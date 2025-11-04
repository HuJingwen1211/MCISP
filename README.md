# ECUSTISP

ECUST ISP Tuning Tool

## 项目概述

本工具是 PC 侧 ISP 调试与参数配置软件，通过串口或 TCP 网络与 FPGA 板卡通信，实现寄存器读写、图像采集、参数配置等功能。

**核心职责：**
- 通信管理（串口/网络双模式）
- 协议帧的打包与解析
- 命令分发与数据路由
- 图像分片接收与重组
- 多模块参数管理与配置导入/导出

---

## 目录

1. [协议规范](#协议规范)
2. [帧格式详解](#帧格式详解)
3. [CRC16校验算法](#crc16校验算法)
4. [状态机解析](#状态机解析)
5. [命令系统](#命令系统)
6. [模块寄存器映射](#模块寄存器映射)
7. [通信流程](#通信流程)
8. [读写操作详解](#读写操作详解)
9. [图像采集机制](#图像采集机制)
10. [调试与排错](#调试与排错)
11. [代码实现细节](#代码实现细节)
12. [常见问题](#常见问题)

---

## 协议规范

### 基础约定

**字节序：** 小端序（Little-Endian），即低字节在前，高字节在后。

**数据类型：**
- `uint8_t`: 1字节无符号整数
- `uint16_t`: 2字节无符号整数
- `uint32_t`: 4字节无符号整数

**帧长度限制：**
- 最小帧长：7字节（帧头2 + 长度1 + 命令1 + CRC2 + 帧尾1）
- 最大有效载荷：255字节（len字段为1字节）
- 最大帧长：262字节（帧头2 + 长度1 + 命令1 + 数据255 + CRC2 + 帧尾1）

**传输模式：**
- 串口：同步阻塞式，`open()` 成功即可通信
- 网络：异步非阻塞式，`connectToHost()` 后需等待 `connected` 信号

---

## 帧格式详解

### 帧结构总览

```
+--------+--------+--------+--------+---------+--------+--------+--------+
| 0xA5   | 0x5A   | len    | cmd    | data    | crc_h  | crc_l  | 0xFF   |
| (1B)   | (1B)   | (1B)   | (1B)   | (lenB)  | (1B)   | (1B)   | (1B)   |
+--------+--------+--------+--------+---------+--------+--------+--------+
  帧头1    帧头2    数据长度  命令字   有效数据   CRC高8位 CRC低8位 帧尾
```

### 字段说明

#### 1. 帧头（Frame Header）
- **位置：** 字节0-1
- **固定值：** `0xA5 0x5A`
- **作用：** 帧同步标识，状态机通过此标识进入帧解析状态

#### 2. 数据长度（Length）
- **位置：** 字节2
- **范围：** `0x00 - 0xFF`（0-255）
- **含义：** `data` 字段的字节数，不包含帧头、长度、命令、CRC、帧尾
- **特殊情况：** `len=0` 表示无数据载荷，常见于触发类命令（如 CAPTURE_CMD）

#### 3. 命令字（Command）
- **位置：** 字节3
- **当前定义：**
  ```c
  #define STR_CMD       0x01  // 字符串打印
  #define DEBUG_CMD     0x02  // 调试命令（板端接收，不回传）
  #define WRITE_REG_CMD 0x03  // 写寄存器（板端接收，不回传）
  #define READ_REG_CMD  0x04  // 读寄存器（板端回传数据）
  #define CAPTURE_CMD   0x05  // 图像采集（分片回传）
  #define TEST_RW_CMD   0x06  // 通用读写测试（回传数据）
  ```

#### 4. 数据域（Data）
- **位置：** 字节4 至 字节(4+len-1)
- **长度：** 由 `len` 字段指定
- **内容：** 取决于 `cmd` 的具体要求，详见 [命令系统](#命令系统)

#### 5. CRC16校验（Checksum）
- **位置：** 数据域之后的2字节
- **字节序：** 高字节在前，低字节在后（`crc_h, crc_l`）
- **校验范围：** 从帧头第1字节到数据域最后1字节（不含CRC和帧尾）
  ```
  CRC范围 = [0xA5, 0x5A, len, cmd, data[0], ..., data[len-1]]
  ```
- **算法：** CRC16-Modbus（多项式 0xA001）

#### 6. 帧尾（Frame Tail）
- **位置：** 最后1字节
- **固定值：** `0xFF`
- **作用：** 帧完整性标识，状态机收到此字节后确认帧有效并发出信号

### 示例帧分析

#### 示例1：读取AWB模块寄存器

**帧内容（十六进制）：**
```
A5 5A 09 04 05 00 00 00 A0 04 00 00 A0 3F 21 FF
```

**字段解析：**
```
A5 5A          - 帧头
09             - 数据长度 = 9字节
04             - 命令字 = READ_REG_CMD
05             - 模块号 = AWBC_MODULE (0x05)
00 00 00 A0    - 地址1 = 0xA0000000（小端序）
04 00 00 A0    - 地址2 = 0xA0000004（小端序）
3F 21          - CRC16校验值（高字节0x3F，低字节0x21）
FF             - 帧尾
```

**数据域细节：**
```
字节0: 模块号（AWBC_MODULE = 0x05）
字节1-4: 第一个寄存器地址（uint32_t，小端序）
字节5-8: 第二个寄存器地址（uint32_t，小端序）
```

#### 示例2：写入CCM模块5个寄存器

**数据构造：**
```c
uint8_t databuf[41];
databuf[0] = CCM_MODULE;  // 模块号

// 5对地址-数值
for(int i=0; i<5; i++) {
    memcpy(databuf + 1 + i*8, &addr[i], 4);   // 地址（小端序）
    memcpy(databuf + 5 + i*8, &value[i], 4);  // 数值（小端序）
}

send_cmd_data(WRITE_REG_CMD, databuf, 41);
```

**帧长度计算：**
```
总长度 = 2(帧头) + 1(len) + 1(cmd) + 41(data) + 2(CRC) + 1(帧尾) = 48字节
```

---

## CRC16校验算法

### 算法规范

**多项式：** 0xA001（CRC16-Modbus 反向多项式）  
**初始值：** 0xFFFF  
**数据宽度：** 8位  
**校验宽度：** 16位  

### 实现代码

```c
uint16_t link_board::CRC16_Check(const uint8_t *data, uint8_t len)
{
    uint16_t CRC16 = 0xFFFF;
    uint8_t state, i, j;
    
    for(i = 0; i < len; i++) {
        CRC16 ^= data[i];  // 异或当前字节
        for(j = 0; j < 8; j++) {
            state = CRC16 & 0x01;  // 取最低位
            CRC16 >>= 1;           // 右移1位
            if(state) {
                CRC16 ^= 0xA001;   // 最低位为1则异或多项式
            }
        }
    }
    return CRC16;
}
```

### 算法步骤详解

1. **初始化：** CRC寄存器置为 `0xFFFF`
2. **字节处理循环：**
   - 将当前字节与CRC寄存器异或
   - 进入位处理循环（8次）：
     - 保存最低位
     - CRC寄存器右移1位
     - 若保存的最低位为1，则CRC寄存器异或 `0xA001`
3. **返回结果：** 16位校验值

### 发送时的打包

```c
uint16_t crc16 = CRC16_Check(buf, len+4);  // 计算CRC
buf[cnt++] = crc16 >> 8;    // CRC高字节
buf[cnt++] = crc16 & 0xFF;  // CRC低字节
```

**说明：** `len+4` 是因为CRC校验范围包括 帧头(2) + 长度(1) + 命令(1) + 数据(len)。

### 接收时的校验

```c
// 状态机case 6（接收CRC低字节）
frameState.crc16 <<= 8;     // 高字节左移
frameState.crc16 += byteData;  // 加上低字节

// 计算期望的CRC
uint16_t expectedCRC = CRC16_Check(frameState.Buf, frameState.cnt);

if(frameState.crc16 == expectedCRC) {
    frameState.step++;  // 校验通过，进入帧尾检测
} else {
    frameState.step = 0;  // 校验失败，重置状态机
}
```

### 测试用例

**输入数据：** `A5 5A 04 01 48 45 4C 4C`  
**预期CRC：** `0x1234`（示例值，实际需计算）

**手工计算步骤（前2字节）：**
```
初始：CRC = 0xFFFF

处理 0xA5:
  CRC ^= 0xA5 = 0xFF5A
  位循环8次后 → CRC = 0x7F5D（简化示意）

处理 0x5A:
  CRC ^= 0x5A = ...
  
...（依次处理所有字节）
```

**调试技巧：**
- PC侧与FPGA侧必须使用完全相同的算法
- 测试时可发送已知CRC的数据帧验证实现正确性
- 校验失败率过高时，检查字节序、初始值、多项式是否一致

---

## 状态机解析

### 状态定义

状态机用于逐字节解析数据流，将字节流还原为完整的帧结构。

**状态枚举：**
```c
step = 0: 等待帧头第1字节 (0xA5)
step = 1: 等待帧头第2字节 (0x5A)
step = 2: 接收数据长度字节
step = 3: 接收命令字节
step = 4: 接收数据域（len字节）
step = 5: 接收CRC高字节
step = 6: 接收CRC低字节
step = 7: 接收帧尾 (0xFF)
```

### 状态机结构体

```c
struct {
    uint8_t step;       // 当前状态（0-7）
    uint8_t cnt;        // 缓冲区已接收字节计数
    uint8_t Buf[300];   // 数据缓冲区
    uint8_t len;        // 数据域长度（从帧中解析）
    uint8_t cmd;        // 命令字（从帧中解析）
    uint8_t *data_ptr;  // 数据域起始指针
    uint16_t crc16;     // 接收到的CRC值
} frameState;
```

### 状态转换图

```
     [任意状态收到非预期字节]
              ↓
           step=0 ←──────────────────┐
              ↓ (收到0xA5)            │
           step=1                     │
              ↓ (收到0x5A)            │
           step=2                     │
              ↓ (收到len)             │
           step=3                     │
              ↓ (收到cmd)             │
           step=4 ────→ (len==0时跳过) │
              ↓ (收齐len字节)         │
           step=5                     │
              ↓ (收到CRC高字节)       │
           step=6                     │
              ↓ (CRC校验通过)         │
           step=7                     │
              ↓ (收到0xFF)            │
         发出frameReceived信号       │
              ↓                       │
           step=0 ←────────────────────┘
```

### 状态机核心代码逐行解析

#### Case 0: 等待帧头第1字节

```c
case 0:
    if(byteData == 0xA5) {
        frameState.step++;
        frameState.cnt = 0;
        frameState.Buf[frameState.cnt++] = byteData;
    }
    break;
```

**逻辑：**
- 只接受 `0xA5`，其他字节丢弃
- 进入状态1，重置计数器，保存首字节

#### Case 1: 等待帧头第2字节

```c
case 1:
    if(byteData == 0x5A) {
        frameState.step++;
        frameState.Buf[frameState.cnt++] = byteData;
    }
    else if(byteData == 0xA5) {
        frameState.step = 1;  // 重新开始
    }
    else {
        frameState.step = 0;  // 返回初始状态
    }
    break;
```

**逻辑：**
- 收到 `0x5A`：帧头确认，进入状态2
- 收到 `0xA5`：可能是新帧的开始，重置为状态1
- 其他字节：帧头错误，返回状态0

**关键设计：** 状态1允许连续的 `0xA5` 字节，避免丢失真正的帧头。

#### Case 2: 接收数据长度

```c
case 2:
    frameState.step++;
    frameState.Buf[frameState.cnt++] = byteData;
    frameState.len = byteData;
    break;
```

**逻辑：**
- 直接接受任意值作为长度字段
- 保存到 `frameState.len`

#### Case 3: 接收命令字

```c
case 3:
    frameState.step++;
    frameState.Buf[frameState.cnt++] = byteData;
    frameState.cmd = byteData;
    frameState.data_ptr = &frameState.Buf[frameState.cnt];
    if(frameState.len == 0) 
        frameState.step++;  // 无数据域，跳过状态4
    break;
```

**逻辑：**
- 保存命令字
- 记录数据域起始指针
- **特殊情况：** 若 `len==0`，直接跳到状态5（无需接收数据域）

#### Case 4: 接收数据域

```c
case 4:
    frameState.Buf[frameState.cnt++] = byteData;
    // 利用指针偏移判断是否接收完所有数据
    if(frameState.data_ptr + frameState.len == &frameState.Buf[frameState.cnt]) {
        frameState.step++;
    }
    break;
```

**逻辑：**
- 逐字节累积数据
- 通过指针算术检测是否收齐 `len` 字节
- **指针判断：** `data_ptr` 指向数据域起始，`data_ptr + len` 是数据域结束位置，`&Buf[cnt]` 是当前写入位置

#### Case 5: 接收CRC高字节

```c
case 5:
    frameState.step++;
    frameState.crc16 = byteData;  // 暂存高字节
    break;
```

#### Case 6: 接收CRC低字节并校验

```c
case 6:
    frameState.crc16 <<= 8;      // 高字节左移
    frameState.crc16 += byteData; // 合成16位CRC
    
    if(frameState.crc16 == CRC16_Check(frameState.Buf, frameState.cnt)) {
        frameState.step++;  // 校验通过
    }
    else if(byteData == 0xA5) {
        frameState.step = 1;  // 可能是新帧
    }
    else {
        frameState.step = 0;  // 校验失败，重置
    }
    break;
```

**逻辑：**
- 合成16位CRC值
- 调用 `CRC16_Check` 计算期望值并比对
- 校验失败时的容错：若低字节恰好是 `0xA5`，尝试作为新帧头

#### Case 7: 接收帧尾并发出信号

```c
case 7:
    if(byteData == 0xFF) {
        // 构造QByteArray并发出信号
        QByteArray frameData(
            reinterpret_cast<const char*>(frameState.data_ptr), 
            frameState.len
        );
        emit frameReceived(frameState.cmd, frameData);
        frameState.step = 0;
    }
    else if(byteData == 0xA5) {
        frameState.step = 1;
    }
    else {
        frameState.step = 0;
    }
    break;
```

**逻辑：**
- 验证帧尾 `0xFF`
- 发出 `frameReceived` 信号，参数为 `cmd` 和数据域的 `QByteArray`
- 重置状态机，准备接收下一帧

### 非帧数据处理

```c
if(frameState.step == 0 && byteData != 0xFF) {
    processColorByte(byteData);  // 处理ANSI颜色码或普通字符
}
```

**说明：**
- 当状态机处于初始状态（step=0）且收到的不是帧尾 `0xFF` 时，认为是非协议数据
- 这些数据通常是板端通过串口直接发送的调试信息
- `processColorByte()` 支持ANSI转义序列（彩色输出）

---

## 命令系统

### 命令概览表

| 命令字 | 宏定义 | 方向 | 有无回传 | 用途 |
|--------|--------|------|----------|------|
| 0x01 | STR_CMD | FPGA→PC | - | 字符串打印（调试输出） |
| 0x02 | DEBUG_CMD | PC→FPGA | 否 | 文本菜单控制 |
| 0x03 | WRITE_REG_CMD | PC→FPGA | 否 | 写寄存器 |
| 0x04 | READ_REG_CMD | PC→FPGA→PC | 是 | 读寄存器（返回数据） |
| 0x05 | CAPTURE_CMD | PC→FPGA→PC | 是（分片） | 图像采集 |
| 0x06 | TEST_RW_CMD | PC→FPGA→PC | 是 | 通用寄存器读写测试 |

### 命令详细说明

#### STR_CMD (0x01)

**用途：** FPGA向PC发送字符串，用于调试输出或状态通知。

**数据格式：**
```
len: 字符串长度
data: ASCII字符数组（不含终止符）
```

**示例：**
```
发送 "Hello": A5 5A 05 01 48 65 6C 6C 6F [CRC] FF
```

**PC侧处理：**
```c
case STR_CMD:
    ui->echo_text->appendPlainText(QString(data));
    break;
```

#### DEBUG_CMD (0x02)

**用途：** PC向FPGA发送文本命令，控制TPG模式、ISP开关等。

**数据格式：**
```
len: 命令字符串长度
data: ASCII命令（如 "S\n", "T\n", "I\n", "D\n" 或 "数字\n"）
```

**示例（切换到Sensor模式）：**
```c
uint8_t cmd[] = {'S', '\n'};
send_cmd_data(DEBUG_CMD, cmd, 2);
```

**FPGA侧行为：**
- 解析文本命令
- 执行对应操作（切换数据源、开关模块等）
- 无数据回传

#### WRITE_REG_CMD (0x03)

**用途：** 写入一个或多个寄存器。

**数据格式（单寄存器）：**
```
字节0: 模块号
字节1-4: 地址（uint32_t，小端序）
字节5-8: 数值（uint32_t，小端序）
总长度：9字节
```

**数据格式（多寄存器）：**
```
字节0: 模块号
字节1-4: 地址1
字节5-8: 数值1
字节9-12: 地址2
字节13-16: 数值2
...
总长度：1 + N*8 字节（N为寄存器数）
```

**示例（写入AWB两个寄存器）：**
```c
uint8_t databuf[17];
databuf[0] = AWBC_MODULE;

uint32_t addr1 = 0xA0000000;
uint32_t val1  = 0x10000FFF;  // G增益[31:16] | R增益[15:0]
uint32_t addr2 = 0xA0000004;
uint32_t val2  = 0x00000FFF;  // B增益[15:0]

memcpy(databuf+1, &addr1, 4);
memcpy(databuf+5, &val1, 4);
memcpy(databuf+9, &addr2, 4);
memcpy(databuf+13, &val2, 4);

link_tab->send_cmd_data(WRITE_REG_CMD, databuf, 17);
```

**FPGA侧行为：**
- 解析模块号，路由到对应模块
- 按地址-数值对逐个写入寄存器
- 无数据回传

#### READ_REG_CMD (0x04)

**用途：** 读取一个或多个寄存器，FPGA将数据原样返回。

**发送数据格式（读取请求）：**
```
字节0: 模块号
字节1-4: 地址1（uint32_t，小端序）
字节5-8: 地址2（uint32_t，小端序）
...
总长度：1 + N*4 字节（N为寄存器数）
```

**返回数据格式（读取回复）：**
```
字节0: 模块号
字节1-4: 数值1（uint32_t，小端序）
字节5-8: 数值2（uint32_t，小端序）
...
总长度：1 + N*4 字节
```

**示例（读取AWB两个寄存器）：**
```c
// 发送
uint8_t databuf[9];
databuf[0] = AWBC_MODULE;
uint32_t addr1 = 0xA0000000;
uint32_t addr2 = 0xA0000004;
memcpy(databuf+1, &addr1, 4);
memcpy(databuf+5, &addr2, 4);
link_tab->send_cmd_data(READ_REG_CMD, databuf, 9);

// 接收（在read_reg_process槽函数中）
const uchar *ptr = reinterpret_cast<const uchar*>(regData.constData());
uint32_t val1, val2;
memcpy(&val1, ptr+1, 4);
memcpy(&val2, ptr+5, 4);
```

**PC侧分发机制：**
```c
void link_board::read_reg_process(const QByteArray &data)
{
    uint8_t module = data.constData()[0];
    switch(module) {
        case AWBC_MODULE:
            emit awbc_read_done(data);
            break;
        // 其他模块...
    }
}
```

#### CAPTURE_CMD (0x05)

**用途：** 触发图像采集，FPGA以分片形式回传图像数据。

**发送数据格式（采集请求）：**
```
len: 1
data: 图像类型（0x01=RAW, 0x02=RGB, 0x03=YUV）
```

**返回数据格式（分片回传）：**
```
每帧数据格式：
字节0-3: 帧序号（uint32_t，大端序）
字节4-7: 总帧数（uint32_t，大端序）
字节8-N: 图像数据载荷
```

**流程：**
1. PC发送 CAPTURE_CMD
2. FPGA分片发送图像数据（多帧）
3. PC侧接收所有分片后拼接完整图像
4. 弹出保存对话框

详见 [图像采集机制](#图像采集机制)。

#### TEST_RW_CMD (0x06)

**用途：** 通用寄存器读写测试，用于直接操作任意地址。

**发送数据格式（读请求）：**
```
len: 4
data: 地址（uint32_t，小端序）
```

**发送数据格式（写请求）：**
```
len: 8
data: 地址（uint32_t）+ 数值（uint32_t）
```

**返回数据格式（读回复）：**
```
len: 8
data: 地址（uint32_t）+ 数值（uint32_t）
```

**示例（TEST Tab的读写）：**
```c
// 写操作
uint8_t databuf[8];
memcpy(databuf, &addr, 4);
memcpy(databuf+4, &value, 4);
link_tab->send_cmd_data(TEST_RW_CMD, databuf, 8);

// 读操作
uint8_t databuf[4];
memcpy(databuf, &addr, 4);
link_tab->send_cmd_data(TEST_RW_CMD, databuf, 4);

// 接收回复
void TEST_tab::read_reg_process(const QByteArray &regData) {
    uint32_t addr, value;
    memcpy(&addr, regData.constData(), 4);
    memcpy(&value, regData.constData()+4, 4);
    // 更新UI显示
}
```

---

## 模块寄存器映射

### 模块号定义

```c
#define DPC_MODULE     0x01  // 坏点校正
#define BLC_MODULE     0x02  // 黑电平校正
#define LSC_MODULE     0x03  // 镜头阴影校正
#define NR_RAW_MODULE  0x04  // RAW域降噪
#define AWBC_MODULE    0x05  // 自动白平衡
#define GB_MODULE      0x06  // 绿色平衡
#define DMS_MODULE     0x07  // 去马赛克
#define CCM_MODULE     0x08  // 颜色校正矩阵
#define GAMMA_MODULE   0x09  // 伽马校正
#define CSC_MODULE     0x0A  // 色彩空间转换
#define NR_YUV_MODULE  0x0B  // YUV域降噪
```

### AWB模块寄存器（示例）

**寄存器地址（假设）：**
```c
#define REG_WBC_GAIN_1_ADDR  0xA0000000  // G增益[31:16] | R增益[15:0]
#define REG_WBC_GAIN_2_ADDR  0xA0000004  // B增益[15:0]
#define REG_AWB_GAIN_1_ADDR  0xA0000008  // 读回寄存器1
#define REG_AWB_GAIN_2_ADDR  0xA000000C  // 读回寄存器2
```

**寄存器位域：**
```
REG_WBC_GAIN_1 (0xA0000000):
  [31:16] G_Gain (0-0x3FFF, 14-bit)
  [15:0]  R_Gain (0-0x3FFF, 14-bit)

REG_WBC_GAIN_2 (0xA0000004):
  [15:0]  B_Gain (0-0x3FFF, 14-bit)
```

**写入示例：**
```c
uint32_t R_Gain = 0x0FFF;  // R增益 = 4095
uint32_t G_Gain = 0x1000;  // G增益 = 4096
uint32_t B_Gain = 0x0FFF;  // B增益 = 4095

uint32_t VALUE1 = (G_Gain << 16) | R_Gain;
uint32_t VALUE2 = B_Gain;

// 构造WRITE_REG_CMD数据包
uint8_t databuf[17];
databuf[0] = AWBC_MODULE;
memcpy(databuf+1, &REG_WBC_GAIN_1_ADDR, 4);
memcpy(databuf+5, &VALUE1, 4);
memcpy(databuf+9, &REG_WBC_GAIN_2_ADDR, 4);
memcpy(databuf+13, &VALUE2, 4);

send_cmd_data(WRITE_REG_CMD, databuf, 17);
```

**读取示例：**
```c
// 构造READ_REG_CMD数据包
uint8_t databuf[9];
databuf[0] = AWBC_MODULE;
memcpy(databuf+1, &REG_AWB_GAIN_1_ADDR, 4);
memcpy(databuf+5, &REG_AWB_GAIN_2_ADDR, 4);

send_cmd_data(READ_REG_CMD, databuf, 9);

// 回调处理
void AWB_tab::read_reg_process(const QByteArray &regData) {
    uint32_t VALUE1, VALUE2;
    memcpy(&VALUE1, regData.constData()+1, 4);
    memcpy(&VALUE2, regData.constData()+5, 4);
    
    uint32_t R_Gain = VALUE1 & 0xFFFF;
    uint32_t G_Gain = (VALUE1 >> 16) & 0xFFFF;
    uint32_t B_Gain = VALUE2 & 0xFFFF;
    
    // 更新UI
}
```

### CCM模块寄存器（示例）

**寄存器地址（假设）：**
```c
#define REG_CCM_COEFF1_ADDR  0xB0000000  // a12[31:13] | a11[12:0]
#define REG_CCM_COEFF2_ADDR  0xB0000004  // a21[31:13] | a13[12:0]
#define REG_CCM_COEFF3_ADDR  0xB0000008  // a23[31:13] | a22[12:0]
#define REG_CCM_COEFF4_ADDR  0xB000000C  // a32[31:13] | a31[12:0]
#define REG_CCM_COEFF5_ADDR  0xB0000010  // a33[12:0]
```

**CCM矩阵打包：**

CCM矩阵为3x3，每个系数13位有符号数（S12.0格式）：
```
[ a11  a12  a13 ]
[ a21  a22  a23 ]
[ a31  a32  a33 ]
```

**打包逻辑：**
```c
uint32_t CCM_REG1 = (a12 << 13) | (a11 & 0x1FFF);
uint32_t CCM_REG2 = (a21 << 13) | (a13 & 0x1FFF);
uint32_t CCM_REG3 = (a23 << 13) | (a22 & 0x1FFF);
uint32_t CCM_REG4 = (a32 << 13) | (a31 & 0x1FFF);
uint32_t CCM_REG5 = a33 & 0x1FFF;
```

**写入示例：**
```c
uint8_t databuf[41];
databuf[0] = CCM_MODULE;

uint32_t addr_val_pairs[10] = {
    REG_CCM_COEFF1_ADDR, CCM_REG1,
    REG_CCM_COEFF2_ADDR, CCM_REG2,
    REG_CCM_COEFF3_ADDR, CCM_REG3,
    REG_CCM_COEFF4_ADDR, CCM_REG4,
    REG_CCM_COEFF5_ADDR, CCM_REG5
};

for(int i=0; i<10; i++) {
    memcpy(databuf + 1 + i*4, &addr_val_pairs[i], 4);
}

send_cmd_data(WRITE_REG_CMD, databuf, 41);
```

### NR_YUV模块寄存器（示例）

**寄存器地址（假设）：**
```c
#define REG_NRYUV_CTRL_ADDR   0xC0000000  // 控制寄存器
#define REG_NRYUV_PARAM_ADDR  0xC0000004  // 参数寄存器
```

**寄存器位域：**
```
REG_NRYUV_CTRL:
  [23:16] Y_Gain   (8-bit)
  [15:8]  UV_Gain  (8-bit)
  [7:0]   Threshold (8-bit)
```

**写入示例：**
```c
uint8_t Y_Gain = 128;
uint8_t UV_Gain = 64;
uint8_t Threshold = 16;

uint32_t CTRL_VALUE = (Y_Gain << 16) | (UV_Gain << 8) | Threshold;

uint8_t databuf[9];
databuf[0] = NR_YUV_MODULE;
memcpy(databuf+1, &REG_NRYUV_CTRL_ADDR, 4);
memcpy(databuf+5, &CTRL_VALUE, 4);

send_cmd_data(WRITE_REG_CMD, databuf, 9);
```

---

## 通信流程

### 串口通信流程

#### 连接流程

```
1. 扫描可用串口
   QSerialPortInfo::availablePorts()
   
2. 用户选择端口和波特率
   ui->port_combx, ui->baud_combx
   
3. 点击Connect
   handleSerialConnect()
   
4. 创建并配置串口
   serial = new QSerialPort;
   serial->setPortName(...);
   serial->setBaudRate(...);
   serial->setDataBits(QSerialPort::Data8);
   serial->setParity(QSerialPort::NoParity);
   serial->setStopBits(QSerialPort::OneStop);
   
5. 打开串口
   serial->open(QIODevice::ReadWrite);
   
6. 连接readyRead信号
   connect(serial, SIGNAL(readyRead()), this, SLOT(handle_ready_read()));
   
7. 更新UI状态
   ui->link_btn->setText("Disconnect");
   ui->port_combx->setEnabled(false);
```

#### 发送流程

```
1. 构造数据包
   uint8_t databuf[N];
   databuf[0] = MODULE_ID;
   // 填充地址、数值等
   
2. 调用封装函数
   send_cmd_data(cmd, databuf, len)
   
3. 打包帧
   buf = [A5 5A len cmd data crc_h crc_l FF]
   
4. 发送
   serial->write(buf, total_len)
   serial->waitForBytesWritten(1000)
```

#### 接收流程

```
1. 触发readyRead信号
   handle_ready_read()
   
2. 读取所有可用数据
   QByteArray receivedData = serial->readAll();
   
3. 逐字节喂给状态机
   for(byte in receivedData)
       Receive(byte);
       
4. 状态机解析完整帧
   emit frameReceived(cmd, data);
   
5. 命令分发
   process_cmd_data(cmd, data)
   
6. 模块处理
   switch(cmd) { ... }
```

### 网络通信流程

#### 连接流程

```
1. 用户输入IP和端口
   ui->ip_lineEdit, ui->tcp_port_spinBox
   
2. 点击Connect
   handleNetworkConnect()
   
3. 创建Socket
   tcpSocket = new QTcpSocket(this);
   
4. 连接信号槽
   connect(tcpSocket, &QTcpSocket::connected, ...);
   connect(tcpSocket, &QTcpSocket::disconnected, ...);
   connect(tcpSocket, &QTcpSocket::readyRead, ...);
   
5. 发起连接（异步）
   tcpSocket->connectToHost(ip, port);
   ui->echo_text->appendPlainText("正在连接...");
   
6. 等待connected信号
   [异步回调] → 更新UI为"Disconnect"
```

#### 发送流程（与串口相同）

```
同串口发送流程，底层通过 sendNetworkData() 实现：
tcpSocket->write(buf, len)
tcpSocket->waitForBytesWritten(1000)
```

#### 接收流程（与串口相同）

```
同串口接收流程，readyRead信号触发后：
QByteArray receivedData = tcpSocket->readAll();
// 后续处理完全一致
```

#### 断开流程

```
1. 用户点击Disconnect 或 网络异常
   
2. 主动断开
   tcpSocket->disconnectFromHost();
   
3. 等待disconnected信号
   [异步回调] → 更新UI为"Connect"
   
4. 清理Socket
   if(tcpSocket->state() == QAbstractSocket::UnconnectedState)
       tcpSocket->deleteLater();
   else
       tcpSocket->waitForDisconnected(1000);
```

### 双模式抽象

**统一发送接口：**
```c
int link_board::Send(const uint8_t *data, uint16_t len)
{
    if (isNetworkMode) {
        return sendNetworkData(data, len);
    } else {
        return sendSerialData(data, len);
    }
}
```

**统一接收处理：**
```c
void link_board::handle_ready_read()
{
    QByteArray receivedData;
    if (isNetworkMode) {
        receivedData = tcpSocket->readAll();
    } else {
        receivedData = serial->readAll();
    }
    
    for(int i = 0; i < receivedData.size(); ++i) {
        Receive(static_cast<uint8_t>(receivedData.at(i)));
    }
}
```

**优点：**
- 上层业务代码无需关心通信方式
- 切换串口/网络无需修改命令打包代码
- 状态机与通信层完全解耦

---

## 读写操作详解

### 单个模块读操作

#### AWB读取流程

**1. 用户点击Read按钮**
```c
void AWB_tab::on_awb_read_btn_clicked()
{
    uint32_t ADDR1 = REG_AWB_GAIN_1_ADDR;
    uint32_t ADDR2 = REG_AWB_GAIN_2_ADDR;
    
    uint8_t databuf[9];
    databuf[0] = AWBC_MODULE;  // 模块号
    memcpy(databuf+1, &ADDR1, 4);  // 地址1（小端序）
    memcpy(databuf+5, &ADDR2, 4);  // 地址2（小端序）
    
    link_tab->send_cmd_data(READ_REG_CMD, databuf, 9);
}
```

**2. link_board打包并发送**
```c
bool link_board::send_cmd_data(uint8_t cmd, const uint8_t *datas, uint16_t len)
{
    uint8_t buf[BUFFER_SIZE];
    uint16_t cnt = 0;
    
    buf[cnt++] = 0xA5;
    buf[cnt++] = 0x5A;
    buf[cnt++] = len;         // 9
    buf[cnt++] = cmd;         // READ_REG_CMD
    for(int i=0; i<len; i++)
        buf[cnt++] = datas[i];  // 模块号 + 地址1 + 地址2
    
    uint16_t crc16 = CRC16_Check(buf, len+4);
    buf[cnt++] = crc16 >> 8;
    buf[cnt++] = crc16 & 0xFF;
    buf[cnt++] = 0xFF;
    
    Send(buf, cnt);  // 串口或网络发送
}
```

**3. FPGA处理并回传**
```
FPGA端：
- 接收并解析帧
- 识别READ_REG_CMD
- 读取AWBC_MODULE的两个寄存器
- 构造回传帧：
  [A5 5A 09 04 05 VALUE1(4B) VALUE2(4B) CRC FF]
```

**4. PC侧接收并解析**
```c
// 状态机接收完整帧后
emit frameReceived(READ_REG_CMD, data);

// 命令分发
void link_board::process_cmd_data(uint8_t cmd, const QByteArray &data)
{
    case READ_REG_CMD:
        read_reg_process(data);
        break;
}

// 模块路由
void link_board::read_reg_process(const QByteArray &data)
{
    uint8_t module = data.constData()[0];
    switch(module) {
        case AWBC_MODULE:
            emit awbc_read_done(data);
            break;
    }
}
```

**5. AWB Tab更新UI**
```c
void AWB_tab::read_reg_process(const QByteArray &regData)
{
    const uchar *ptr = reinterpret_cast<const uchar*>(regData.constData());
    
    uint32_t VALUE1, VALUE2;
    memcpy(&VALUE1, ptr+1, 4);  // 跳过模块号
    memcpy(&VALUE2, ptr+5, 4);
    
    // 解包位域
    uint32_t R_Gain = VALUE1 & 0xFFFF;
    uint32_t G_Gain = (VALUE1 >> 16) & 0xFFFF;
    uint32_t B_Gain = VALUE2 & 0xFFFF;
    
    // 更新数据模型（自动触发UI刷新）
    awbc_model->setData(awbc_model->index(0, 0), R_Gain);
    awbc_model->setData(awbc_model->index(0, 1), G_Gain);
    awbc_model->setData(awbc_model->index(0, 2), B_Gain);
}
```

### 单个模块写操作

#### AWB写入流程

**1. 用户修改参数并点击Write按钮**
```c
void AWB_tab::on_awb_write_btn_clicked()
{
    // 从UI读取数据到模型
    updateModelFromUI();
    
    uint32_t R_Gain = awbc_model->data(awbc_model->index(0, 0)).toUInt();
    uint32_t G_Gain = awbc_model->data(awbc_model->index(0, 1)).toUInt();
    uint32_t B_Gain = awbc_model->data(awbc_model->index(0, 2)).toUInt();
    
    // 按寄存器位域打包
    uint32_t ADDR1 = REG_WBC_GAIN_1_ADDR;
    uint32_t ADDR2 = REG_WBC_GAIN_2_ADDR;
    uint32_t VALUE1 = (G_Gain << 16) | R_Gain;
    uint32_t VALUE2 = B_Gain;
    
    uint8_t databuf[17];
    databuf[0] = AWBC_MODULE;
    memcpy(databuf+1, &ADDR1, 4);
    memcpy(databuf+5, &VALUE1, 4);
    memcpy(databuf+9, &ADDR2, 4);
    memcpy(databuf+13, &VALUE2, 4);
    
    link_tab->send_cmd_data(WRITE_REG_CMD, databuf, 17);
}
```

**2. link_board打包并发送**
```
帧结构：
A5 5A 11 03 05 [ADDR1(4B)] [VALUE1(4B)] [ADDR2(4B)] [VALUE2(4B)] [CRC] FF
总长度：2+1+1+17+2+1 = 24字节
```

**3. FPGA处理**
```
FPGA端：
- 接收并解析帧
- 识别WRITE_REG_CMD
- 提取AWBC_MODULE
- 解析地址-数值对
- 写入寄存器：
  MEM[ADDR1] = VALUE1
  MEM[ADDR2] = VALUE2
- 无回传
```

### 批量读写操作（All Read/Write）

#### 设计思路

对于多模块参数配置，可实现批量读取和批量写入功能。

**方案1：逐个发送（简单）**
```c
void XMLModuleTab::allRead()
{
    for(auto &module : m_modules) {
        readModule(module.groupBox);
        QThread::msleep(50);  // 防止数据包过密
    }
}
```

**方案2：打包发送（高效）**
```c
void XMLModuleTab::allRead()
{
    QByteArray combinedData;
    
    for(auto &module : m_modules) {
        uint8_t moduleId = getModuleId(module.name);
        combinedData.append(moduleId);
        
        for(auto &param : module.params) {
            uint32_t addr = param.address;
            combinedData.append((char*)&addr, 4);
        }
    }
    
    m_commMgr->sendCmd(READ_REG_CMD, (uint8_t*)combinedData.data(), combinedData.size());
}
```

**推荐方案1：** 简单可靠，易于调试，延迟可接受。

---

## 图像采集机制

### 采集流程

#### 1. 触发采集

**用户点击Capture按钮：**
```c
void capture_tab::on_capture_btn_clicked()
{
    uint8_t imageType = 0x01;  // RAW=0x01, RGB=0x02, YUV=0x03
    link_tab->send_cmd_data(CAPTURE_CMD, &imageType, 1);
}
```

**发送帧：**
```
A5 5A 01 05 01 [CRC] FF
```

#### 2. FPGA分片发送

**分片策略：**
- 单帧最大载荷：255字节
- 减去8字节帧头（帧序号4B + 总帧数4B）= 247字节实际图像数据
- 图像总大小：假设1920x1080 RAW10 = 2,592,000字节
- 需要分片数：2,592,000 / 247 ≈ 10,486 帧

**每帧格式：**
```
字节0-3: frameIndex（uint32_t，大端序）
字节4-7: totalFrames（uint32_t，大端序）
字节8-N: 图像数据（N-8字节）
```

**示例（第0帧）：**
```
字节0-3: 0x00000000  // 帧序号=0
字节4-7: 0x00002000  // 总帧数=8192（示例）
字节8-254: 实际图像数据（247字节）
```

#### 3. PC侧接收与拼接

**接收状态机：**
```c
struct ImageReception {
    bool active;              // 是否正在接收
    uint32_t totalFrames;     // 总帧数
    uint32_t receivedFrames;  // 已接收帧数
    uint32_t frameDataSize;   // 每帧数据大小
    QVector<QByteArray> frameData;  // 分片数据容器
};
```

**处理函数：**
```c
void link_board::process_recv_image(const QByteArray &data)
{
    // 1. 检查帧头完整性
    if (data.size() < 8) return;
    
    const uchar *ptr = reinterpret_cast<const uchar*>(data.constData());
    
    // 2. 解析帧序号和总帧数（大端序）
    uint32_t frameIndex = (ptr[0]<<24) | (ptr[1]<<16) | (ptr[2]<<8) | ptr[3];
    uint32_t totalFrames = (ptr[4]<<24) | (ptr[5]<<16) | (ptr[6]<<8) | ptr[7];
    
    uint32_t payloadSize = data.size() - 8;
    
    // 3. 检测新传输
    if (frameIndex == 0) {
        if (currentReception.active) {
            qDebug() << "新图像开始，覆盖旧传输";
        }
        startNewReception(totalFrames, payloadSize);
    }
    
    // 4. 验证状态
    if (!currentReception.active) {
        qDebug() << "未初始化接收状态";
        return;
    }
    if (totalFrames != currentReception.totalFrames) {
        qDebug() << "总帧数不匹配";
        resetReception();
        return;
    }
    if (frameIndex >= currentReception.totalFrames) {
        qDebug() << "帧序号越界";
        resetReception();
        return;
    }
    
    // 5. 检查帧大小一致性（最后一帧除外）
    if (frameIndex != totalFrames-1 && payloadSize != currentReception.frameDataSize) {
        qDebug() << "帧大小不一致";
        resetReception();
        return;
    }
    
    // 6. 存储分片数据
    currentReception.frameData[frameIndex] = QByteArray(data.constData()+8, payloadSize);
    currentReception.receivedFrames++;
    
    // 7. 检查是否完成
    if (currentReception.receivedFrames == currentReception.totalFrames) {
        // 拼接所有分片
        QByteArray completeImage;
        for (const QByteArray &frame : currentReception.frameData) {
            completeImage.append(frame);
        }
        
        emit imageReceived(completeImage);
        resetReception();
    }
}
```

**初始化接收：**
```c
void link_board::startNewReception(uint32_t totalFrames, uint32_t frameDataSize)
{
    currentReception.active = true;
    currentReception.totalFrames = totalFrames;
    currentReception.receivedFrames = 0;
    currentReception.frameDataSize = frameDataSize;
    currentReception.frameData.resize(totalFrames);  // 预分配
}
```

**重置接收：**
```c
void link_board::resetReception()
{
    currentReception.active = false;
    currentReception.totalFrames = 0;
    currentReception.receivedFrames = 0;
    currentReception.frameDataSize = 0;
    currentReception.frameData.clear();
}
```

#### 4. 保存图像

**弹出保存对话框：**
```c
void link_board::save_image(const QByteArray &imageData)
{
    QString fileName = QFileDialog::getSaveFileName(
        this, 
        tr("保存图像"),
        QDir::homePath(),
        tr("图片文件 (*.raw *.rgb *.yuv);;所有文件 (*)")
    );
    
    if (fileName.isEmpty()) return;
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, tr("错误"), tr("无法创建文件"));
        return;
    }
    
    qint64 bytesWritten = file.write(imageData);
    file.close();
    
    if (bytesWritten != imageData.size()) {
        QMessageBox::critical(this, tr("错误"), 
            tr("写入不完整\n期望:%1\n实际:%2")
            .arg(imageData.size()).arg(bytesWritten));
        return;
    }
    
    QMessageBox::information(this, tr("成功"), 
        tr("图像已保存:\n%1").arg(fileName));
}
```

### 容错机制

**1. 分片丢失检测**
- 当前实现：按序接收，丢失任意一帧会导致 `receivedFrames < totalFrames` 永远不相等
- 改进方案：添加超时机制，X秒内未收齐则认为传输失败

**2. 帧序号错乱检测**
- 通过 `frameIndex >= totalFrames` 检测越界
- 通过 `totalFrames` 不一致检测传输中断

**3. 帧大小异常检测**
- 最后一帧允许大小不同（可能不满247字节）
- 其他帧必须大小一致

**4. 重复传输处理**
- 检测到 `frameIndex==0` 时，若已有传输在进行，覆盖旧传输

### 性能优化

**1. 预分配内存**
```c
currentReception.frameData.resize(totalFrames);
```
避免动态扩容，提高性能。

**2. 避免拷贝**
```c
currentReception.frameData[frameIndex] = QByteArray(data.constData()+8, payloadSize);
```
直接构造QByteArray，避免中间拷贝。

**3. 进度显示（待实现）**
```c
int progress = (receivedFrames * 100) / totalFrames;
ui->progressBar->setValue(progress);
```

---

## 调试与排错

### 日志输出

#### 协议层调试

**开启状态机详细日志：**
```c
void link_board::Receive(uint8_t byteData)
{
    qDebug() << "State:" << frameState.step << "Byte:" << Qt::hex << byteData;
    
    switch(frameState.step) {
        // ...状态处理
    }
}
```

**CRC校验调试：**
```c
uint16_t computed = CRC16_Check(frameState.Buf, frameState.cnt);
qDebug() << "CRC: received=" << Qt::hex << frameState.crc16 
         << "computed=" << computed;
```

**帧内容打印：**
```c
void printFrame(const uint8_t *buf, int len) {
    QString hexStr;
    for(int i=0; i<len; i++) {
        hexStr += QString("%1 ").arg(buf[i], 2, 16, QChar('0')).toUpper();
    }
    qDebug() << "Frame:" << hexStr;
}
```

#### 通信层调试

**发送数据追踪：**
```c
int link_board::Send(const uint8_t *data, uint16_t len)
{
    // 打印发送的原始数据
    QString hexStr;
    for(int i=0; i<len; i++) {
        hexStr += QString("%1 ").arg(data[i], 2, 16, QChar('0')).toUpper();
    }
    qDebug() << "TX:" << hexStr;
    
    // 实际发送
    if (isNetworkMode) {
        return sendNetworkData(data, len);
    } else {
        return sendSerialData(data, len);
    }
}
```

**接收数据追踪：**
```c
void link_board::handle_ready_read()
{
    QByteArray receivedData = serial->readAll();  // 或 tcpSocket->readAll()
    
    // 打印接收的原始数据
    QString hexStr;
    for(int i=0; i<receivedData.size(); i++) {
        hexStr += QString("%1 ").arg((uint8_t)receivedData.at(i), 2, 16, QChar('0')).toUpper();
    }
    qDebug() << "RX:" << hexStr;
    
    // 逐字节送入状态机
    for(int i = 0; i < receivedData.size(); ++i) {
        Receive(static_cast<uint8_t>(receivedData.at(i)));
    }
}
```

### 常见错误排查

#### 1. CRC校验失败

**症状：** 接收数据但无法解析成完整帧

**排查步骤：**
1. 打印接收到的原始字节流
2. 手工计算CRC值与代码计算值对比
3. 检查PC侧与FPGA侧CRC算法是否一致
4. 确认CRC范围是否正确（不含CRC和帧尾）

**常见原因：**
- 多项式不一致
- 初始值不一致
- 计算范围错误
- 字节序混乱

#### 2. 帧同步丢失

**症状：** 数据流中无法找到帧头

**排查步骤：**
1. 确认发送端是否正确打包
2. 检查帧头定义是否一致
3. 查看是否有非帧数据干扰

**常见原因：**
- 帧头定义错误
- 发送端未发送帧头
- 中间路由/网关修改了数据

#### 3. 数据长度不匹配

**症状：** 解析出的长度与实际数据不符

**排查步骤：**
1. 打印len字段和实际接收的数据长度
2. 检查是否有字节丢失
3. 确认状态机的长度判断逻辑

**常见原因：**
- len字段计算错误（是否包含了不应包含的字段）
- 数据截断
- 缓冲区溢出

#### 4. 模块号路由失败

**症状：** 读回的数据未触发对应模块的回调

**排查步骤：**
1. 打印接收到的模块号
2. 检查模块号定义是否与FPGA一致
3. 确认信号连接是否正确

**常见原因：**
- 模块号定义不一致
- 信号槽未连接
- 回调函数未实现

#### 5. 图像分片接收失败

**症状：** 图像传输中断或数据不完整

**排查步骤：**
1. 打印每帧的 `frameIndex` 和 `totalFrames`
2. 检查是否有分片丢失
3. 查看帧大小是否一致

**常见原因：**
- 网络丢包
- FPGA发送速度过快，PC侧处理不过来
- 帧序号或总帧数错误
- 帧大小计算错误

### 调试工具

#### 串口助手

**推荐软件：**
- SSCOM（支持十六进制收发）
- Serial Port Utility
- HTerm

**使用场景：**
- 验证PC软件发送的数据包
- 模拟FPGA发送测试数据
- 测试CRC算法

**示例测试帧：**
```
发送：A5 5A 04 01 54 45 53 54 XX XX FF
（TEST命令，CRC需自行计算）

接收：PC软件应解析出STR_CMD，数据为"TEST"
```

#### 网络助手

**推荐软件：**
- TCP/UDP调试助手
- NetAssist
- SocketTool

**使用场景：**
- 测试网络连接
- 模拟FPGA回传数据
- 抓包分析

#### Wireshark

**使用场景：**
- 抓取网络数据包
- 分析TCP连接状态
- 查找丢包原因

**过滤器示例：**
```
tcp.port == 8080 && ip.addr == 192.168.1.100
```

### 单元测试建议

#### CRC算法测试

```c
void test_CRC16()
{
    uint8_t testData[] = {0xA5, 0x5A, 0x04, 0x01, 0x48, 0x45, 0x4C, 0x4C};
    uint16_t crc = CRC16_Check(testData, 8);
    
    qDebug() << "CRC=" << Qt::hex << crc;
    Q_ASSERT(crc == 0x1234);  // 替换为实际期望值
}
```

#### 状态机测试

```c
void test_StateMachine()
{
    // 模拟发送完整帧
    uint8_t frame[] = {0xA5, 0x5A, 0x04, 0x01, 
                       0x48, 0x45, 0x4C, 0x4C,  // "HELL"
                       0x12, 0x34,  // CRC（示例）
                       0xFF};
    
    for(int i=0; i<sizeof(frame); i++) {
        Receive(frame[i]);
    }
    
    // 验证是否发出frameReceived信号
}
```

---

## 代码实现细节

### 内存管理

#### 缓冲区大小

```c
#define BUFFER_SIZE 256  // 发送缓冲区
uint8_t Buf[300];        // 状态机接收缓冲区
```

**说明：**
- 发送缓冲区：256字节足够容纳最大帧（262字节）
- 接收缓冲区：300字节留有余量，防止溢出

#### 动态内存分配

**串口对象：**
```c
serial = new QSerialPort;
// 使用完毕
serial->close();
delete serial;
serial = nullptr;
```

**网络对象：**
```c
tcpSocket = new QTcpSocket(this);  // 父对象管理生命周期
// 或
tcpSocket->deleteLater();  // 延迟删除
```

#### 图像数据管理

```c
QVector<QByteArray> frameData;  // 自动管理内存
frameData.resize(totalFrames);  // 预分配

// 拼接时
QByteArray completeImage;
for (const QByteArray &frame : frameData) {
    completeImage.append(frame);  // 可能涉及内存拷贝
}
```

**优化建议：**
- 预计算总大小并 `reserve()`
- 考虑使用 `std::vector<uint8_t>` 替代 `QByteArray`

### 字节序处理

**小端序写入：**
```c
uint32_t addr = 0xA0000000;
memcpy(databuf, &addr, 4);  // 直接memcpy，x86为小端序
```

**大端序写入（如帧序号）：**
```c
uint32_t frameIndex = 1234;
databuf[0] = (frameIndex >> 24) & 0xFF;
databuf[1] = (frameIndex >> 16) & 0xFF;
databuf[2] = (frameIndex >> 8) & 0xFF;
databuf[3] = frameIndex & 0xFF;
```

**小端序读取：**
```c
uint32_t value;
memcpy(&value, data, 4);  // 直接memcpy
```

**大端序读取：**
```c
uint32_t frameIndex = (ptr[0]<<24) | (ptr[1]<<16) | (ptr[2]<<8) | ptr[3];
```

**跨平台建议：**
```c
#include <QtEndian>

uint32_t addr = qToLittleEndian<uint32_t>(0xA0000000);
uint32_t frameIndex = qFromBigEndian<uint32_t>(rawData);
```

### 信号与槽

#### 信号定义

```c
signals:
    void frameReceived(uint8_t cmd, const QByteArray &data);
    void imageReceived(const QByteArray &imageData);
    void test_rw_signal(const QByteArray &regData);
    void awbc_read_done(const QByteArray &regData);
```

#### 槽函数连接

**传统SIGNAL/SLOT宏（不推荐）：**
```c
connect(this, SIGNAL(frameReceived(uint8_t,QByteArray)), 
        this, SLOT(process_cmd_data(uint8_t,QByteArray)));
```

**函数指针方式（推荐）：**
```c
connect(this, &link_board::frameReceived, 
        this, &link_board::process_cmd_data);
```

**Lambda表达式：**
```c
connect(tcpSocket, &QTcpSocket::connected, this, [this]() {
    ui->echo_text->appendPlainText("网络连接成功!");
    ui->link_btn->setText("Disconnect");
});
```

#### 跨线程信号（图像采集场景）

```c
// 若FPGA数据处理在子线程
connect(workerThread, &WorkerThread::imageChunkReceived, 
        this, &link_board::process_recv_image, 
        Qt::QueuedConnection);  // 跨线程，排队连接
```

### UI阻塞问题

#### 问题场景

```c
// 错误示例：阻塞UI线程
for(int i=0; i<10000; i++) {
    send_cmd_data(...);
    QThread::sleep(1);  // UI冻结1秒
}
```

#### 解决方案1：QTimer

```c
void sendNextFrame() {
    static int index = 0;
    if(index < totalFrames) {
        send_cmd_data(...);
        index++;
        QTimer::singleShot(10, this, &MyClass::sendNextFrame);
    }
}
```

#### 解决方案2：QThread

```c
class SendThread : public QThread {
    void run() override {
        for(int i=0; i<totalFrames; i++) {
            emit sendRequest(data);
            msleep(10);
        }
    }
};
```

#### 解决方案3：异步信号

```c
// 串口/网络的readyRead本身就是异步的，无需额外处理
connect(serial, &QSerialPort::readyRead, this, &MyClass::handleData);
```

---

## 常见问题

### Q1: 为什么使用小端序？

**A:** x86/x64架构为小端序，FPGA端通常也配置为小端序以简化通信。使用 `memcpy` 直接拷贝 `uint32_t` 到字节流时，自动遵循平台字节序。

### Q2: CRC校验范围是否包含帧头？

**A:** 是。CRC范围是从帧头第1字节到数据域最后1字节，不包含CRC本身和帧尾。

### Q3: 状态机为什么在case 1中重复判断0xA5？

**A:** 防止丢失真正的帧头。若数据流为 `... 0xA5 0xA5 0x5A ...`，第一个0xA5触发状态1，第二个0xA5再次触发状态1，然后0x5A进入状态2，帧头正确识别。

### Q4: 为什么网络连接是异步的？

**A:** TCP握手需要时间（SYN→SYN-ACK→ACK），Qt使用异步模型避免阻塞UI线程。串口则是同步的，因为 `open()` 只是打开设备文件，无需网络协商。

### Q5: 如何处理TCP TIME_WAIT状态？

**A:** 调用 `waitForDisconnected(1000)` 等待四次挥手完成，确保本地端口释放。或者在Socket上设置 `SO_REUSEADDR` 选项（Qt默认已设置）。

### Q6: 图像分片的帧序号为什么用大端序？

**A:** 约定问题，可能FPGA端习惯大端序发送。只要收发双方一致即可。

### Q7: 如何验证CRC算法正确性？

**A:** 使用在线CRC计算器（CRC16-Modbus），输入相同数据，对比结果。注意输入格式（十六进制）和参数（初始值0xFFFF，多项式0xA001）。

### Q8: 为什么需要非帧数据处理（processColorByte）？

**A:** FPGA可能通过串口直接发送调试信息（如printf），这些数据不符合协议帧格式，需要单独处理并显示在终端。

### Q9: 如何添加新模块？

**步骤：**
1. 在 `link_board.h` 中添加模块号定义
2. 创建模块Tab类（继承 `ConfigurableTab`）
3. 实现 `getAllParams()` 和 `setParams()`
4. 在 `read_reg_process()` 中添加case分支
5. 在 `createModuleTab()` 中添加创建逻辑
6. 在 `defaultParams` 中添加默认值

### Q10: 如何测试协议解析？

**方法：**
1. 使用串口助手发送手工构造的帧
2. 在 `Receive()` 函数中添加日志
3. 验证是否发出 `frameReceived` 信号
4. 检查 `cmd` 和 `data` 是否正确

**测试用例：**
```
发送：A5 5A 04 01 41 42 43 44 [CRC] FF
期望：解析出STR_CMD，数据为"ABCD"
```

---

## 附录

### A. 命令速查表

| 命令 | 值 | 方向 | 数据格式 | 回传 |
|------|-----|------|----------|------|
| STR_CMD | 0x01 | FPGA→PC | ASCII字符串 | - |
| DEBUG_CMD | 0x02 | PC→FPGA | 文本命令 | 否 |
| WRITE_REG_CMD | 0x03 | PC→FPGA | 模块号+地址值对 | 否 |
| READ_REG_CMD | 0x04 | PC⇄FPGA | 请求:模块号+地址  回复:模块号+数值 | 是 |
| CAPTURE_CMD | 0x05 | PC⇄FPGA | 请求:图像类型  回复:分片数据 | 是 |
| TEST_RW_CMD | 0x06 | PC⇄FPGA | 地址+数值 | 是 |

### B. 模块号速查表

| 模块 | 号 | 缩写 | 用途 |
|------|-----|------|------|
| DPC | 0x01 | DPC | 坏点校正 |
| BLC | 0x02 | BLC | 黑电平校正 |
| LSC | 0x03 | LSC | 镜头阴影校正 |
| NR_RAW | 0x04 | NR_RAW | RAW域降噪 |
| AWBC | 0x05 | AWB | 自动白平衡 |
| GB | 0x06 | GB | 绿色平衡 |
| DMS | 0x07 | DMS | 去马赛克 |
| CCM | 0x08 | CCM | 颜色校正矩阵 |
| GAMMA | 0x09 | GAMMA | 伽马校正 |
| CSC | 0x0A | CSC | 色彩空间转换 |
| NR_YUV | 0x0B | NR_YUV | YUV域降噪 |

### C. 寄存器地址示例（需根据实际硬件调整）

```c
// AWB模块
#define REG_WBC_GAIN_1_ADDR   0xA0000000
#define REG_WBC_GAIN_2_ADDR   0xA0000004
#define REG_AWB_GAIN_1_ADDR   0xA0000008
#define REG_AWB_GAIN_2_ADDR   0xA000000C

// CCM模块
#define REG_CCM_COEFF1_ADDR   0xB0000000
#define REG_CCM_COEFF2_ADDR   0xB0000004
#define REG_CCM_COEFF3_ADDR   0xB0000008
#define REG_CCM_COEFF4_ADDR   0xB000000C
#define REG_CCM_COEFF5_ADDR   0xB0000010

// ENABLE模块
#define REG_MODULE_ENABLE_ADDR 0xC0000000
#define REG_ISP_RESET_ADDR     0xC0000004

// TEST模块（通用寄存器访问）
// 地址由用户输入，无固定定义
```

### D. 错误码定义

```c
// Send函数返回值
#define SEND_OK             0   // 发送成功
#define SEND_ERR_NOT_OPEN  -1   // 设备未打开
#define SEND_ERR_WRITE     -2   // 写入失败
#define SEND_ERR_LENGTH    -3   // 长度不匹配
#define SEND_ERR_TIMEOUT   -4   // 发送超时
```

