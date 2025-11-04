#include "comm_manager.h"
#include <QDebug>

CommManager::CommManager(QObject *parent) : QObject(parent) {
    // 初始化ANSI颜色格式
    m_defaultFormat.setForeground(Qt::white);
    m_currentFormat = m_defaultFormat;
    
    // 初始化图像接收状态
    resetReception();
    
    // 连接内部信号（参考 link_board.cpp 313行）
    connect(this, &CommManager::frameReceived, this, &CommManager::process_cmd_data);
}

CommManager::~CommManager() {
    close();
}

// ==================== 连接管理 ====================

bool CommManager::openSerial(const QString& portName, int baudRate) {
    close();  // 先关闭之前的连接
    
    serial = new QSerialPort(this);
    serial->setPortName(portName);
    serial->setBaudRate(baudRate);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);
    
    if (!serial->open(QIODevice::ReadWrite)) {
        emit logMessage(QString("串口打开失败: %1 @ %2，原因：%3").arg(portName).arg(baudRate).arg(serial->errorString()));
        cleanupSerial();
        return false;
    }
    
    // 统一抽象：设置device指针
    device = serial;
    
    // 连接信号：串口数据到达 → 发射统一信号
    connect(serial, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    
    emit logMessage(QString("串口连接成功: %1 @ %2").arg(portName).arg(baudRate));
    return true;
}

bool CommManager::openNetwork(const QString& ip, int port) {
    close();
    
    tcpSocket = new QTcpSocket(this);
    
    // 连接信号
    connect(tcpSocket, SIGNAL(connected()), this, SLOT(onNetworkConnected()));
    connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(onNetworkDisconnected()));
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    
    tcpSocket->connectToHost(ip, port);
    return true;
}

void CommManager::close() {
    cleanupSerial();
    cleanupNetwork();
    device = nullptr;
}

bool CommManager::isOpen() const {
    if (serial && serial->isOpen()) return true;
    if (tcpSocket && tcpSocket->state() == QAbstractSocket::ConnectedState) return true;
    return false;
}

// ==================== 数据发送 ====================

bool CommManager::sendCmd(uint8_t cmd, const uint8_t *datas, uint16_t len) {
    uint8_t buf[BUFFER_SIZE];
    uint8_t i;
    uint16_t cnt = 0;
    uint16_t crc16;
    
    // 组装帧：[A5 5A][len][cmd][data...][crc16_H][crc16_L][FF]
    buf[cnt++] = 0xA5;
    buf[cnt++] = 0x5A;
    buf[cnt++] = len;
    buf[cnt++] = cmd;
    
    for(i = 0; i < len; i++) {
        buf[cnt++] = datas[i];
    }
    
    crc16 = CRC16_Check(buf, len + 4);
    buf[cnt++] = crc16 >> 8;
    buf[cnt++] = crc16 & 0xFF;
    buf[cnt++] = 0xFF;
    
    return writeRaw(QByteArray(reinterpret_cast<const char*>(buf), cnt)) == 0;
}

int CommManager::writeRaw(const QByteArray& bytes) {
    if (!device || !isOpen()) {
        return -1;
    }
    
    qint64 bytesWritten = device->write(bytes);
    if (bytesWritten == -1) {
        return -2;
    } else if (bytesWritten != bytes.size()) {
        return -3;
    }
    
    if (!device->waitForBytesWritten(1000)) {
        return -4;
    }
    
    emit logMessage("发送成功");
    return 0;
}

// ==================== 数据接收 ====================

void CommManager::onReadyRead() {
    if (!device) return;
    
    QByteArray receivedData = device->readAll();

    if (receivedData.isEmpty()) return;

    // // 调试：原始字节流调试输出（便于与串口助手互测）
    const QString hex = receivedData.toHex(' ');
    const QString ascii = QString::fromLatin1(receivedData);
    emit logMessage(QString("RX HEX: %1").arg(hex));
    emit logMessage(QString("RX ASCII: %1").arg(ascii));
    
    
    // 协议解析：逐字节送入状态机
    for(int i = 0; i < receivedData.size(); ++i) {
        Receive(static_cast<uint8_t>(receivedData.at(i)));
    }
}

void CommManager::onNetworkConnected() {
    device = tcpSocket;  // 统一抽象：设置device指针
    emit logMessage(QString("网络连接成功: %1:%2").arg(tcpSocket->peerAddress().toString()).arg(tcpSocket->peerPort()));
    emit networkConnected();  // 发射网络专用信号
}

void CommManager::onNetworkDisconnected() {
    emit networkDisconnected();  // 发射网络专用信号
}

// ==================== CRC校验 ====================

uint16_t CommManager::CRC16_Check(const uint8_t *data, uint8_t len) {
    uint16_t CRC16 = 0xFFFF;
    uint8_t state, i, j;
    
    for(i = 0; i < len; i++) {
        CRC16 ^= data[i];
        for(j = 0; j < 8; j++) {
            state = CRC16 & 0x01;
            CRC16 >>= 1;
            if(state) {
                CRC16 ^= 0xA001;
            }
        }
    }
    return CRC16;
}

// ==================== 帧解析状态机 ====================

void CommManager::Receive(uint8_t byteData) {
    // 进行数据解析 状态机
    switch(frameState.step) {
    case 0: // 接收帧头1状态
        if(byteData == 0xA5) {
            frameState.step++;
            frameState.cnt = 0;
            frameState.Buf[frameState.cnt++] = byteData;
        }
        break;

    case 1: // 接收帧头2状态
        if(byteData == 0x5A) {
            frameState.step++;
            frameState.Buf[frameState.cnt++] = byteData;
        }
        else if(byteData == 0xA5) {
            frameState.step = 1;
        }
        else {
            frameState.step = 0;
        }
        break;

    case 2: // 接收数据长度字节状态
        frameState.step++;
        frameState.Buf[frameState.cnt++] = byteData;
        frameState.len = byteData;
        break;

    case 3: // 接收命令字节状态
        frameState.step++;
        frameState.Buf[frameState.cnt++] = byteData;
        frameState.cmd = byteData;
        frameState.data_ptr = &frameState.Buf[frameState.cnt]; // 记录数据指针首地址
        if(frameState.len == 0) frameState.step++; // 数据字节长度为0则跳过数据接收状态
        break;

    case 4: // 接收len字节数据状态
        frameState.Buf[frameState.cnt++] = byteData;
        if(frameState.data_ptr + frameState.len == &frameState.Buf[frameState.cnt]) { // 利用指针地址偏移判断是否接收完len位数据
            frameState.step++;
        }
        break;

    case 5: // 接收crc16校验高8位字节
        frameState.step++;
        frameState.crc16 = byteData;
        break;

    case 6: // 接收crc16校验低8位字节
        frameState.crc16 <<= 8;
        frameState.crc16 += byteData;
        if(frameState.crc16 == CRC16_Check(frameState.Buf, frameState.cnt)) { // 校验正确进入下一状态
            frameState.step++;
        }
        else if(byteData == 0xA5) {
            frameState.step = 1;
        }
        else {
            frameState.step = 0;
        }
        break;

    case 7: // 接收帧尾
        if(byteData == 0xFF) { // 帧尾接收正确
            QByteArray frameData(reinterpret_cast<const char*>(frameState.data_ptr), frameState.len);
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

    default:
        frameState.step = 0;
        break;
    }
    
    // 处理非帧数据（ANSI颜色等）
    if(frameState.step == 0 && byteData != 0xFF) {
        processColorByte(byteData); // 逐字节处理
    }
}

// ==================== 命令处理 ====================

void CommManager::process_cmd_data(uint8_t cmd, const QByteArray &data) {
    switch(cmd) {
        case STR_CMD:
            emit logMessage(QString("接收字符串: %1").arg(QString(data)));
            break;       // ZYNQ发送的字符串
        case DEBUG_CMD:
            break;       // ZYNQ接收,不返回
        case WRITE_REG_CMD:
            break;       // ZYNQ接收并配置寄存器，不返回
        case READ_REG_CMD:
            read_reg_process(data);
            break;       // 返回读取的寄存器值
        case CAPTURE_CMD:
            process_recv_image(data);
            break;       // 返回捕捉的视频帧
        case TEST_RW_CMD:
            emit test_rw_signal(data);
            break;
        default:
            break;
    }
}

void CommManager::read_reg_process(const QByteArray &data) {
    uint8_t module = data.constData()[0];
    switch(module) {
        case DPC_MODULE:
            break;
        case BLC_MODULE:
            break;
        case LSC_MODULE:
            break;
        case NR_RAW_MODULE:
            break;
        case AWBC_MODULE:
            emit awbc_read_done(data);
            break;
        case GB_MODULE:
            break;
        case DMS_MODULE:
            break;
        case CCM_MODULE:
            break;
        case GAMMA_MODULE:
            break;
        case CSC_MODULE:
            break;
        case NR_YUV_MODULE:
            break;
        default:
            break;
    }
}

// ==================== 图像接收处理 ====================

void CommManager::process_recv_image(const QByteArray &data) {
    // 检查数据是否包含完整的帧头(8字节: 4字节帧序号 + 4字节总帧数)
    if (data.size() < 8) {
        emit logMessage("图像帧太小，无法包含完整帧头");
        return;
    }
    
    const uchar *ptr = reinterpret_cast<const uchar*>(data.constData());
    uint32_t frameIndex = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3];
    uint32_t totalFrames = (ptr[4] << 24) | (ptr[5] << 16) | (ptr[6] << 8) | ptr[7];
    const uint32_t payloadSize = data.size() - 8;

    emit logMessage(QString("接收图像帧: %1/%2").arg(frameIndex).arg(totalFrames));

    // 检查是否是新的图像传输
    if (frameIndex == 0) {
        if (currentReception.active) {
            emit logMessage("新图像开始，但上一个图像还未完成");
        }
        startNewReception(totalFrames, payloadSize);
    }
    
    // 验证当前接收状态
    if (!currentReception.active) {
        emit logMessage("收到图像帧但没有有效的接收状态");
        return;
    }
    
    if (totalFrames != currentReception.totalFrames) {
        emit logMessage(QString("总帧数不匹配。期望: %1, 收到: %2").arg(currentReception.totalFrames).arg(totalFrames));
        resetReception();
        return;
    }
    
    if (frameIndex >= currentReception.totalFrames) {
        emit logMessage(QString("帧索引超出范围。最大: %1, 收到: %2").arg(currentReception.totalFrames - 1).arg(frameIndex));
        resetReception();
        return;
    }
    
    // 检查帧数据大小是否一致(第一帧除外)
    if (frameIndex != totalFrames-1 && payloadSize != currentReception.frameDataSize) {
        emit logMessage(QString("帧大小不匹配。期望: %1, 收到: %2").arg(currentReception.frameDataSize).arg(payloadSize));
        resetReception();
        return;
    }
    
    // 存储帧数据
    currentReception.frameData[frameIndex] = QByteArray(data.constData() + 8, payloadSize);
    currentReception.receivedFrames++;
    
    // 检查是否完成接收
    if (currentReception.receivedFrames == currentReception.totalFrames) {
        // 组合所有帧数据
        QByteArray completeImage;
        for (const QByteArray &frame : currentReception.frameData) {
            completeImage.append(frame);
        }

        emit imageReceived(completeImage);
        resetReception();
    }
}

void CommManager::resetReception() {
    currentReception.active = false;
    currentReception.totalFrames = 0;
    currentReception.receivedFrames = 0;
    currentReception.frameDataSize = 0;
    currentReception.frameData.clear();
}

void CommManager::startNewReception(uint32_t totalFrames, uint32_t frameDataSize) {
    currentReception.active = true;
    currentReception.totalFrames = totalFrames;
    currentReception.receivedFrames = 0;
    currentReception.frameDataSize = frameDataSize;
    currentReception.frameData.resize(totalFrames);
}

// ==================== ANSI颜色处理 ====================

void CommManager::processColorByte(uint8_t byte) {
    if (m_inEscapeSequence) {
        m_escapeSequence += static_cast<char>(byte);
        // 检查是否到达ANSI序列的结束符 'm'
        if (byte == 'm') {
            applyAnsiColorFormat(m_escapeSequence);
            m_escapeSequence.clear();
            m_inEscapeSequence = false;
        }
    } else {
        if (byte == 0x1B) { // 检测到开始符 '\x1b'
            m_inEscapeSequence = true;
            m_escapeSequence = "\x1b";
        } else {
            // 普通字符，可以发射日志信号
            emit logMessage(QString(static_cast<char>(byte)));
        }
    }
}

void CommManager::resetColorFormat() {
    m_currentFormat = m_defaultFormat;
}

void CommManager::applyAnsiColorFormat(const QString &ansiCode) {
    if (ansiCode == "\x1b[31m") m_currentFormat.setForeground(Qt::red);
    else if (ansiCode == "\x1b[32m") m_currentFormat.setForeground(Qt::green);
    else if (ansiCode == "\x1b[33m") m_currentFormat.setForeground(Qt::yellow);
    else if (ansiCode == "\x1b[34m") m_currentFormat.setForeground(Qt::blue);
    else if (ansiCode == "\x1b[35m") m_currentFormat.setForeground(Qt::magenta);
    else if (ansiCode == "\x1b[36m") m_currentFormat.setForeground(Qt::cyan);
    else if (ansiCode == "\x1b[0m") m_currentFormat = m_defaultFormat; // 重置
}

// ==================== 内部辅助函数 ====================

void CommManager::cleanupSerial() {
    if (serial) {
        serial->close();
        serial->deleteLater();
        serial = nullptr;
    }
}

void CommManager::cleanupNetwork() {
    if (tcpSocket) {
        tcpSocket->disconnectFromHost();  // 发送 FIN
        
        // 等待四次挥手完成（最多1秒）
        if (tcpSocket->state() != QAbstractSocket::UnconnectedState) {
            tcpSocket->waitForDisconnected(1000);
        }
        
        tcpSocket->deleteLater();
        tcpSocket = nullptr;
    }
}
