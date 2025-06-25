#include "link_board.h"
#include "ui_link_board.h"
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

#include "Link_tab/test_tab.h"
#include "Link_tab/dpc_tab.h"
#include "Link_tab/enable_tab.h"
#include "Link_tab/blc_tab.h"
#include "Link_tab/lsc_tab.h"
#include "Link_tab/nr_raw_tab.h"
#include "Link_tab/awb_tab.h"
#include "Link_tab/dms_tab.h"
#include "Link_tab/ccm_tab.h"
#include "Link_tab/ee_tab.h"
#include "Link_tab/tm_tab.h"
#include "Link_tab/gamma_tab.h"
#include "Link_tab/csc_tab.h"
#include "Link_tab/nr_yuv_tab.h"
#include "Link_tab/gb_tab.h"
#include "Link_tab/scale_tab.h"
#include "Link_tab/crop_tab.h"
#include "Link_tab/yfc_tab.h"
#include "Link_tab/debug.h"
#include "Link_tab/capture_tab.h"

int link_board::Send(const uint8_t *data, uint16_t len)
{
    if (!serial || !serial->isOpen()) {
        ui->echo_text->appendPlainText("串口未打开");
        return -1;
    }
    // 将数据写入串口
    qint64 bytesWritten = serial->write(reinterpret_cast<const char*>(data), len);
    if (bytesWritten == -1) {
        ui->echo_text->appendPlainText("串口发送失败");
    } else if (bytesWritten != len) {
        ui->echo_text->appendPlainText("发送长度错误");
    }
    // 确保数据被发送
    if (!serial->waitForBytesWritten(1000)) {
        ui->echo_text->appendPlainText("串口发送延时");
    }
    return 0;
}

uint16_t link_board::CRC16_Check(const uint8_t *data, uint8_t len)
{
    uint16_t CRC16 = 0xFFFF;
    uint8_t state,i,j;
    for(i = 0; i < len; i++ )
    {
        CRC16 ^= data[i];
        for( j = 0; j < 8; j++)
        {
            state = CRC16 & 0x01;
            CRC16 >>= 1;
            if(state)
            {
                CRC16 ^= 0xA001;
            }
        }
    }
    return CRC16;
}

void link_board::Receive(uint8_t byteData)
{
    // 进行数据解析 状态机
    switch(frameState.step)
    {
    case 0://接收帧头1状态
        if(byteData == 0xA5)
        {
            frameState.step++;
            frameState.cnt = 0;
            frameState.Buf[frameState.cnt++] = byteData;
        }
        break;

    case 1://接收帧头2状态
        if(byteData == 0x5A)
        {
            frameState.step++;
            frameState.Buf[frameState.cnt++] = byteData;
        }
        else if(byteData == 0xA5)
        {
            frameState.step = 1;
        }
        else
        {
            frameState.step = 0;
        }
        break;

    case 2://接收数据长度字节状态
        frameState.step++;
        frameState.Buf[frameState.cnt++] = byteData;
        frameState.len = byteData;
        break;

    case 3://接收命令字节状态
        frameState.step++;
        frameState.Buf[frameState.cnt++] = byteData;
        frameState.cmd = byteData;
        frameState.data_ptr = &frameState.Buf[frameState.cnt];//记录数据指针首地址
        if(frameState.len == 0) frameState.step++;//数据字节长度为0则跳过数据接收状态
        break;

    case 4://接收len字节数据状态
        frameState.Buf[frameState.cnt++] = byteData;
        if(frameState.data_ptr + frameState.len == &frameState.Buf[frameState.cnt])//利用指针地址偏移判断是否接收完len位数据
        {
            frameState.step++;
        }
        break;

    case 5://接收crc16校验高8位字节
        frameState.step++;
        frameState.crc16 = byteData;
        break;

    case 6://接收crc16校验低8位字节
        frameState.crc16 <<= 8;
        frameState.crc16 += byteData;
        if(frameState.crc16 == CRC16_Check(frameState.Buf, frameState.cnt))//校验正确进入下一状态
        {
            frameState.step++;
        }
        else if(byteData == 0xA5)
        {
            frameState.step = 1;
        }
        else
        {
            frameState.step = 0;
        }
        break;

    case 7://接收帧尾
        if(byteData == 0xFF)//帧尾接收正确
        {
            QByteArray frameData(reinterpret_cast<const char*>(frameState.data_ptr), frameState.len);
            emit frameReceived(frameState.cmd, frameData);
            // Data_Analysis(frameState.cmd, frameState.data_ptr, frameState.len);
            frameState.step = 0;
        }
        else if(byteData == 0xA5)
        {
            frameState.step = 1;
        }
        else
        {
            frameState.step = 0;
        }
        break;

    default:
        frameState.step = 0;
        break;
    }
    if(frameState.step == 0 && byteData != 0xFF)
    {
        ////若是非帧数据则直接打印字符，不添加回车
        // ui->echo_text->insertPlainText(QString(static_cast<char>(byteData)));
        // qDebug() << "Discarded byte:" << Qt::hex << byteData;
        processColorByte(byteData); // 逐字节处理
    }
}


void link_board::process_recv_image(const QByteArray &data)
{
    // 检查数据是否包含完整的帧头(8字节: 4字节帧序号 + 4字节总帧数)
    if (data.size() < 8) {
        qDebug()<<"Frame too small to contain header";
        return;
    }
    const uchar *ptr = reinterpret_cast<const uchar*>(data.constData());

    uint32_t frameIndex = (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3];
    uint32_t totalFrames = (ptr[4] << 24) | (ptr[5] << 16) | (ptr[6] << 8) | ptr[7];
    // qDebug()<<"datasize:"<<data.size();
    // qDebug() << "first values:" << frameIndex << "sencond:"<<totalFrames;

    const uint32_t payloadSize = data.size() - 8;

    qDebug()<<"frameIndex:"<<frameIndex<<"totalFrames:"<<totalFrames;
    // return;

    // 检查是否是新的图像传输
    if (frameIndex == 0) {
        if (currentReception.active) {
            qDebug()<<"New image started before previous completed";
        }
        startNewReception(totalFrames, payloadSize);
    }
    // 验证当前接收状态
    if (!currentReception.active) {
        qDebug()<<"Received frame without valid reception state";
        return;
    }
    if (totalFrames != currentReception.totalFrames) {
        qDebug()<<(QString("Total frames mismatch. Expected: %1, Received: %2").arg(currentReception.totalFrames).arg(totalFrames));
        resetReception();
        return;
    }
    if (frameIndex >= currentReception.totalFrames) {
        qDebug()<<(QString("Frame index out of range. Max: %1, Received: %2").arg(currentReception.totalFrames - 1).arg(frameIndex));
        resetReception();
        return;
    }
    // 检查帧数据大小是否一致(第一帧除外)
    if (frameIndex !=totalFrames-1 && payloadSize != currentReception.frameDataSize) {
        qDebug()<<(QString("Frame size mismatch. Expected: %1, Received: %2").arg(currentReception.frameDataSize).arg(payloadSize));
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

void link_board::resetReception()
{
    currentReception.active = false;
    currentReception.totalFrames = 0;
    currentReception.receivedFrames = 0;
    currentReception.frameDataSize = 0;
    currentReception.frameData.clear();
}

void link_board::startNewReception(uint32_t totalFrames, uint32_t frameDataSize)
{
    currentReception.active = true;
    currentReception.totalFrames = totalFrames;
    currentReception.receivedFrames = 0;
    currentReception.frameDataSize = frameDataSize;
    currentReception.frameData.resize(totalFrames);
}

void link_board::processColorByte(uint8_t byte)
{
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
            // 普通字符，用当前格式输出
            appendChar(static_cast<char>(byte));
        }
    }

}

void link_board::resetColorFormat()
{
    m_currentFormat = m_defaultFormat;
}

void link_board::applyAnsiColorFormat(const QString &ansiCode)
{
    if (ansiCode == "\x1b[31m") m_currentFormat.setForeground(Qt::red);
    else if (ansiCode == "\x1b[32m") m_currentFormat.setForeground(Qt::green);
    else if (ansiCode == "\x1b[33m") m_currentFormat.setForeground(Qt::yellow);
    else if (ansiCode == "\x1b[34m") m_currentFormat.setForeground(Qt::blue);
    else if (ansiCode == "\x1b[35m") m_currentFormat.setForeground(Qt::magenta);
    else if (ansiCode == "\x1b[36m") m_currentFormat.setForeground(Qt::cyan);
    else if (ansiCode == "\x1b[0m") m_currentFormat = m_defaultFormat; // 重置
}

void link_board::appendChar(char c)
{
    QTextCursor cursor(ui->echo_text->document());
    cursor.movePosition(QTextCursor::End);
    cursor.setCharFormat(m_currentFormat);
    cursor.insertText(QString(c));
}



link_board::link_board(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::link_board)
{
    ui->setupUi(this);
    ui->link_tab->setTabsClosable(true);   /////设置Tab关闭按钮可见
    ui->link_tab->setVisible(false);//初始设置不可见
    ui->link_tab->clear();//清除所有页面
    //查找可用的串口
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        QSerialPort serial_t;
        serial_t.setPort(info);
        if(serial_t.open(QIODevice::ReadWrite))
        {
            ui->port_combx->addItem(serial_t.portName());   ////将查找到的端口添加进combox下拉选项
            serial_t.close();
        }
    }
    //设置波特率下拉菜单默认显示第1项
    ui->baud_combx->setCurrentIndex(0);
    //连接帧收到信号与数据处理函数
    connect(this, SIGNAL(frameReceived(uint8_t,QByteArray)), this, SLOT(process_cmd_data(uint8_t,QByteArray)));
    connect(this, SIGNAL(imageReceived(QByteArray)),SLOT(save_image(QByteArray)));
    resetReception();

    m_defaultFormat.setForeground(Qt::white); // 默认颜色
    m_currentFormat = m_defaultFormat;
}


link_board::~link_board()
{
    delete ui;
}

void link_board::send_cmd_data(uint8_t cmd, const uint8_t *datas, uint16_t len)
{
    uint8_t buf[BUFFER_SIZE];
    uint8_t i;
    uint16_t cnt=0;
    uint16_t crc16;
    buf[cnt++] = 0xA5;
    buf[cnt++] = 0x5A;
    buf[cnt++] = len;
    buf[cnt++] = cmd;
    for(i=0;i<len;i++)
    {
        buf[cnt++] = datas[i];
    }
    crc16 = CRC16_Check(buf,len+4);
    buf[cnt++] = crc16>>8;
    buf[cnt++] = crc16&0xFF;
    buf[cnt++] = 0xFF;
    Send(buf,cnt);    //调用数据帧发送函数将打包好的数据帧发送出去
}

void link_board::read_reg_process(const QByteArray &data)
{
    uint8_t module = data.constData()[0];
    switch(module){
        case DPC_MODULE    :
            break;
        case BLC_MODULE    :
            break;
        case LSC_MODULE    :
            break;
        case NR_RAW_MODULE :
            break;
        case AWBC_MODULE   :
            emit awbc_read_done(data);
            break;
        case GB_MODULE     :
            break;
        case DMS_MODULE    :
            break;
        case CCM_MODULE    :
            break;
        case GAMMA_MODULE  :
            break;
        case CSC_MODULE    :
            break;
        case NR_YUV_MODULE :
            break;
        default:
            break;
    }
}


void link_board::handle_redy_read()
{
    if (!serial || !serial->isOpen()) {
        return;
    }
    QByteArray receivedData = serial->readAll();
    // 将接收到的数据逐个字节送入状态机
    for(int i = 0; i < receivedData.size(); ++i) {
        Receive(static_cast<uint8_t>(receivedData.at(i)));
    }
}

void link_board::process_cmd_data(uint8_t cmd, const QByteArray &data)
{
    switch(cmd){
        case STR_CMD:
            ui->echo_text->appendPlainText(QString(" data:%2").arg(QString(data)));
            break;       ///ZYNQ发送的字符串
        case DEBUG_CMD:break;     //ZYNQ接收,不返回
        case WRITE_REG_CMD:break; //ZYNQ接收并配置寄存器，不返回
        case READ_REG_CMD:
            read_reg_process(data);
            break;  //返回读取的寄存器值
        case CAPTURE_CMD:
            process_recv_image(data);
            break;   //返回捕捉的视频帧
        case TEST_RW_CMD:
            emit test_rw_signal(data);
            break;
        default:break;
    }
}

///清空echo Text
void link_board::on_clear_btn_clicked()
{
    ui->echo_text->clear();
}

////连接开发板串口
void link_board::on_link_btn_clicked()
{
   if(ui->link_btn->text()==tr("Connect")){
       serial = new QSerialPort;             ///创建串口
       serial->setPortName(ui->port_combx->currentText()); ///设置串口名
       serial->setBaudRate(ui->baud_combx->currentText().toInt()); ///设置波特率
       serial->setDataBits(QSerialPort::Data8);  ////设置数据位
       serial->setParity(QSerialPort::NoParity);  ////设置无奇偶校验
       serial->setStopBits(QSerialPort::OneStop); ///设置一位停止位
       serial->setFlowControl(QSerialPort::NoFlowControl);  ///设置流控制
       if (serial->open(QIODevice::ReadWrite)) {             //打开串口
           //qDebug() << "Serial port opened successfully!";
           ui->echo_text->appendPlainText(QString("Serial port opened successfully!  Port:%1").arg(ui->port_combx->currentText()));
       } else {
           ui->echo_text->appendPlainText(QString("Failed to open serial port!"));
           return;
           //qDebug() << "Failed to open serial port.";
       }
       //serial->open(QIODevice::ReadWrite);           ////打开串口

       ui->port_combx->setEnabled(false);
       ui->baud_combx->setEnabled(false);
       ui->link_btn->setText(tr("Disconnect"));
       ui->link_btn->setStyleSheet("background: #38815c;");
       ///ui->sendButton->setEnabled(true);
        connect(serial, SIGNAL(readyRead()), this, SLOT(handle_redy_read()));

   }else if(ui->link_btn->text()==tr("Disconnect")){
       //关闭串口
       serial->clear();
       serial->close();
       // serial->deleteLater();  //稍后清理
       serial =nullptr;
       //恢复设置使能
       ui->port_combx->setEnabled(true);
       ui->baud_combx->setEnabled(true);
       ui->link_btn->setText(tr("Connect"));
       ui->link_btn->setStyleSheet("background: #455364;");
   }
}

///////双击TreeWidget事件
void link_board::on_module_list_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    //qDebug()<<item->text(column);
    if(item->text(column)=="TEST"){
        TEST_DoubleClicked();
    }else if(item->text(column)=="DPC"){
        DPC_DoubleClicked();
    }
    else if (item->text(column) == "ENABLE"){
        ENABLE_DoubleClicked();
    }
    else if (item->text(column) == "BLC") {
        BLC_DoubleClicked();
    }else if(item->text(column) == "LSC"){
        LSC_DoubleClicked();
    }
    else if (item->text(column) == "NR_RAW") {
        NR_RAW_DoubleClicked();
    }
    else if (item->text(column) == "AWB") {
        AWB_DoubleClicked();
    }
    else if (item->text(column) == "DMS") {
        DMS_DoubleClicked();
    }
    else if (item->text(column) == "CCM") {
        CCM_DoubleClicked();
    }
    else if (item->text(column) == "EE") {
        EE_DoubleClicked();
    }
    else if (item->text(column) == "TM") {
        TM_DoubleClicked();
    }
    else if (item->text(column) == "Gamma") {
        GAMMA_DoubleClicked();
    }
    else if (item->text(column) == "CSC") {
        CSC_DoubleClicked();
    }
    else if (item->text(column) == "NR_YUV") {
        NR_YUV_DoubleClicked();
    }
    else if (item->text(column) == "GB") {
        GB_DoubleClicked();
    }
    else if (item->text(column) == "SCALE") {
        SCALE_DoubleClicked();
    }
    else if (item->text(column) == "CROP") {
        CROP_DoubleClicked();
    }
    else if (item->text(column) == "YFC") {
        YFC_DoubleClicked();
    }
    else if(item->text(column) == "Debug"){
        Debug_DoubleClicked();
    }
    else if(item->text(column) == "CAPTURE"){
        Capture_DoubleClicked();
    }
}


////ENABLE
void link_board::ENABLE_DoubleClicked()
{
    bool is_exist = 0;
    for (int i = 0; i < ui->link_tab->count(); i++) {    ///////用于设置该Tab只能有一个
        QWidget* tabWidget = ui->link_tab->widget(i);    /////用了以后不能delete因为是指针
        if (ENABLE_tab* tab = dynamic_cast<ENABLE_tab*>(tabWidget)) {   ///遍历当前Tab判断是否已经存在,如果动态转换成功则表示有该类型
            ui->link_tab->setCurrentIndex(i);     /////如果之前创建了就定位到之前的tab
            is_exist = 1;
            break;
        }
    }
    if (!is_exist) {
        ENABLE_tab* tab = new ENABLE_tab(this);
        tab->setAttribute(Qt::WA_DeleteOnClose);
        // tab->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        int cur = ui->link_tab->addTab(tab, QString::asprintf(" ENABLE "));
        ui->link_tab->setCurrentIndex(cur);
        ui->link_tab->setVisible(true);
        // ENABLE_tab* tab = new ENABLE_tab(this);
        // tab->setAttribute(Qt::WA_DeleteOnClose);

        // // 创建一个容器 widget 并设置布局
        // QWidget *container = new QWidget();
        // QHBoxLayout *layout = new QHBoxLayout(container);
        // layout->addWidget(tab, 0, Qt::AlignCenter); // 居中
        // container->setLayout(layout);

        // // 添加到 link_tab
        // int cur = ui->link_tab->addTab(container, QString::asprintf(" ENABLE "));
        // ui->link_tab->setCurrentIndex(cur);
        // ui->link_tab->setVisible(true);
    }
}


////////TEST
void link_board::TEST_DoubleClicked()
{
    bool is_exist=0;
    for (int i = 0; i < ui->link_tab->count(); i++) {    ///////用于设置该Tab只能有一个
        QWidget *tabWidget = ui->link_tab->widget(i);    /////用了以后不能delete因为是指针
        if(TEST_tab *tab=dynamic_cast<TEST_tab*>(tabWidget)){   ///遍历当前Tab判断是否已经存在,如果动态转换成功则表示有该类型
            ui->link_tab->setCurrentIndex(i);     /////如果之前创建了就定位到之前的tab
            is_exist=1;
            break;
        }
    }
    if(!is_exist){
        TEST_tab *tab=new TEST_tab(this);
        tab->setAttribute(Qt::WA_DeleteOnClose);
        int cur=ui->link_tab->addTab(tab,QString::asprintf(" TEST "));
        ui->link_tab->setCurrentIndex(cur);
        ui->link_tab->setVisible(true);
        connect(this,SIGNAL(test_rw_signal(QByteArray)),tab,SLOT(read_reg_process(QByteArray)));
    }
}

//////DPC
void link_board::DPC_DoubleClicked()
{
    bool is_exist=0;
    for (int i = 0; i < ui->link_tab->count(); ++i) {    ///////用于设置该Tab只能有一个
        QWidget *tabWidget = ui->link_tab->widget(i);    /////用了以后不能delete因为是指针
        if(DPC_tab *tab=dynamic_cast<DPC_tab*>(tabWidget)){   ///遍历当前Tab判断是否已经存在,如果动态转换成功则表示有该类型
            ui->link_tab->setCurrentIndex(i);     /////如果之前创建了就定位到之前的tab
            is_exist=1;
            break;
        }
    }
    if(!is_exist){
        DPC_tab *tab=new DPC_tab(this);
        tab->setAttribute(Qt::WA_DeleteOnClose);
        int cur=ui->link_tab->addTab(tab,QString::asprintf(" DPC "));
        ui->link_tab->setCurrentIndex(cur);
        ui->link_tab->setVisible(true);
    }
}

////BLC
void link_board::BLC_DoubleClicked()
{
    bool is_exist = 0;
    for (int i = 0; i < ui->link_tab->count(); ++i) {    ///////用于设置该Tab只能有一个
        QWidget* tabWidget = ui->link_tab->widget(i);    /////用了以后不能delete因为是指针
        if (BLC_tab* tab = dynamic_cast<BLC_tab*>(tabWidget)) {   ///遍历当前Tab判断是否已经存在,如果动态转换成功则表示有该类型
            ui->link_tab->setCurrentIndex(i);     /////如果之前创建了就定位到之前的tab
            is_exist = 1;
            break;
        }
    }
    if (!is_exist) {
        BLC_tab* tab = new BLC_tab(this);
        tab->setAttribute(Qt::WA_DeleteOnClose);
        int cur = ui->link_tab->addTab(tab, QString::asprintf(" BLC "));
        ui->link_tab->setCurrentIndex(cur);
        ui->link_tab->setVisible(true);
    }
}

///LSC
void link_board::LSC_DoubleClicked()
{
    bool is_exist = 0;
    for (int i = 0; i < ui->link_tab->count(); ++i) {    ///////用于设置该Tab只能有一个
        QWidget* tabWidget = ui->link_tab->widget(i);    /////用了以后不能delete因为是指针
        if (LSC_tab* tab = dynamic_cast<LSC_tab*>(tabWidget)) {   ///遍历当前Tab判断是否已经存在,如果动态转换成功则表示有该类型
            ui->link_tab->setCurrentIndex(i);     /////如果之前创建了就定位到之前的tab
            is_exist = 1;
            break;
        }
    }
    if (!is_exist) {
        LSC_tab* tab = new LSC_tab(this);
        tab->setAttribute(Qt::WA_DeleteOnClose);
        int cur = ui->link_tab->addTab(tab, QString::asprintf(" LSC "));
        ui->link_tab->setCurrentIndex(cur);
        ui->link_tab->setVisible(true);
    }

}


///NR_RAW
void link_board::NR_RAW_DoubleClicked()
{
    bool is_exist = 0;
    for (int i = 0; i < ui->link_tab->count(); ++i) {    ///////用于设置该Tab只能有一个
        QWidget* tabWidget = ui->link_tab->widget(i);    /////用了以后不能delete因为是指针
        if (NR_RAW_tab* tab = dynamic_cast<NR_RAW_tab*>(tabWidget)) {   ///遍历当前Tab判断是否已经存在,如果动态转换成功则表示有该类型
            ui->link_tab->setCurrentIndex(i);     /////如果之前创建了就定位到之前的tab
            is_exist = 1;
            break;
        }
    }
    if (!is_exist) {
        NR_RAW_tab* tab = new NR_RAW_tab(this);
        tab->setAttribute(Qt::WA_DeleteOnClose);
        int cur = ui->link_tab->addTab(tab, QString::asprintf(" NR_RAW "));
        ui->link_tab->setCurrentIndex(cur);
        ui->link_tab->setVisible(true);
    }

}
///AWB
void link_board::AWB_DoubleClicked()
{
    bool is_exist = 0;
    for (int i = 0; i < ui->link_tab->count(); ++i) {    ///////用于设置该Tab只能有一个
        QWidget* tabWidget = ui->link_tab->widget(i);    /////用了以后不能delete因为是指针
        if (AWB_tab* tab = dynamic_cast<AWB_tab*>(tabWidget)) {   ///遍历当前Tab判断是否已经存在,如果动态转换成功则表示有该类型
            ui->link_tab->setCurrentIndex(i);     /////如果之前创建了就定位到之前的tab
            is_exist = 1;
            break;
        }
    }
    if (!is_exist) {
        AWB_tab* tab = new AWB_tab(this);
        tab->setAttribute(Qt::WA_DeleteOnClose);
        int cur = ui->link_tab->addTab(tab, QString::asprintf(" AWB "));
        ui->link_tab->setCurrentIndex(cur);
        ui->link_tab->setVisible(true);
        connect(this,SIGNAL(awbc_read_done(QByteArray)),tab,SLOT(read_reg_process(QByteArray)));
    }
}

void link_board::GB_DoubleClicked()
{
    bool is_exist = 0;
    for (int i = 0; i < ui->link_tab->count(); ++i) {    ///////用于设置该Tab只能有一个
        QWidget* tabWidget = ui->link_tab->widget(i);    /////用了以后不能delete因为是指针
        if (GB_tab* tab = dynamic_cast<GB_tab*>(tabWidget)) {   ///遍历当前Tab判断是否已经存在,如果动态转换成功则表示有该类型
            ui->link_tab->setCurrentIndex(i);     /////如果之前创建了就定位到之前的tab
            is_exist = 1;
            break;
        }
    }
    if (!is_exist) {
        GB_tab* tab = new GB_tab(this);
        tab->setAttribute(Qt::WA_DeleteOnClose);
        int cur = ui->link_tab->addTab(tab, QString::asprintf(" GB "));
        ui->link_tab->setCurrentIndex(cur);
        ui->link_tab->setVisible(true);
    }

}

///DMS
void link_board::DMS_DoubleClicked()
{
    bool is_exist = 0;
    for (int i = 0; i < ui->link_tab->count(); ++i) {    ///////用于设置该Tab只能有一个
        QWidget* tabWidget = ui->link_tab->widget(i);    /////用了以后不能delete因为是指针
        if (DMS_tab* tab = dynamic_cast<DMS_tab*>(tabWidget)) {   ///遍历当前Tab判断是否已经存在,如果动态转换成功则表示有该类型
            ui->link_tab->setCurrentIndex(i);     /////如果之前创建了就定位到之前的tab
            is_exist = 1;
            break;
        }
    }
    if (!is_exist) {
        DMS_tab* tab = new DMS_tab(this);
        tab->setAttribute(Qt::WA_DeleteOnClose);
        int cur = ui->link_tab->addTab(tab, QString::asprintf(" DMS "));
        ui->link_tab->setCurrentIndex(cur);
        ui->link_tab->setVisible(true);
    }
}

///CCM
void link_board::CCM_DoubleClicked()
{
    bool is_exist = 0;
    for (int i = 0; i < ui->link_tab->count(); ++i) {    ///////用于设置该Tab只能有一个
        QWidget* tabWidget = ui->link_tab->widget(i);    /////用了以后不能delete因为是指针
        if (CCM_tab* tab = dynamic_cast<CCM_tab*>(tabWidget)) {   ///遍历当前Tab判断是否已经存在,如果动态转换成功则表示有该类型
            ui->link_tab->setCurrentIndex(i);     /////如果之前创建了就定位到之前的tab
            is_exist = 1;
            break;
        }
    }
    if (!is_exist) {
        CCM_tab* tab = new CCM_tab(this);
        tab->setAttribute(Qt::WA_DeleteOnClose);
        int cur = ui->link_tab->addTab(tab, QString::asprintf(" CCM "));
        ui->link_tab->setCurrentIndex(cur);
        ui->link_tab->setVisible(true);
    }
}
///EE
void link_board::EE_DoubleClicked()
{
    bool is_exist = 0;
    for (int i = 0; i < ui->link_tab->count(); ++i) {    ///////用于设置该Tab只能有一个
        QWidget* tabWidget = ui->link_tab->widget(i);    /////用了以后不能delete因为是指针
        if (EE_tab* tab = dynamic_cast<EE_tab*>(tabWidget)) {   ///遍历当前Tab判断是否已经存在,如果动态转换成功则表示有该类型
            ui->link_tab->setCurrentIndex(i);     /////如果之前创建了就定位到之前的tab
            is_exist = 1;
            break;
        }
    }
    if (!is_exist) {
        EE_tab* tab = new EE_tab(this);
        tab->setAttribute(Qt::WA_DeleteOnClose);
        int cur = ui->link_tab->addTab(tab, QString::asprintf(" EE "));
        ui->link_tab->setCurrentIndex(cur);
        ui->link_tab->setVisible(true);
    }
}
///TM
void link_board::TM_DoubleClicked()
{
    bool is_exist = 0;
    for (int i = 0; i < ui->link_tab->count(); ++i) {    ///////用于设置该Tab只能有一个
        QWidget* tabWidget = ui->link_tab->widget(i);    /////用了以后不能delete因为是指针
        if (TM_tab* tab = dynamic_cast<TM_tab*>(tabWidget)) {   ///遍历当前Tab判断是否已经存在,如果动态转换成功则表示有该类型
            ui->link_tab->setCurrentIndex(i);     /////如果之前创建了就定位到之前的tab
            is_exist = 1;
            break;
        }
    }
    if (!is_exist) {
        TM_tab* tab = new TM_tab(this);
        tab->setAttribute(Qt::WA_DeleteOnClose);
        int cur = ui->link_tab->addTab(tab, QString::asprintf(" TM "));
        ui->link_tab->setCurrentIndex(cur);
        ui->link_tab->setVisible(true);
    }
}
///GAMMA
void link_board::GAMMA_DoubleClicked()
{
    bool is_exist = 0;
    for (int i = 0; i < ui->link_tab->count(); ++i) {    ///////用于设置该Tab只能有一个
        QWidget* tabWidget = ui->link_tab->widget(i);    /////用了以后不能delete因为是指针
        if (GAMMA_tab* tab = dynamic_cast<GAMMA_tab*>(tabWidget)) {   ///遍历当前Tab判断是否已经存在,如果动态转换成功则表示有该类型
            ui->link_tab->setCurrentIndex(i);     /////如果之前创建了就定位到之前的tab
            is_exist = 1;
            break;
        }
    }
    if (!is_exist) {
        GAMMA_tab* tab = new GAMMA_tab(this);
        tab->setAttribute(Qt::WA_DeleteOnClose);
        int cur = ui->link_tab->addTab(tab, QString::asprintf(" GAMMA "));
        ui->link_tab->setCurrentIndex(cur);
        ui->link_tab->setVisible(true);
    }
}
///CSC
void link_board::CSC_DoubleClicked()
{
    bool is_exist = 0;
    for (int i = 0; i < ui->link_tab->count(); ++i) {    ///////用于设置该Tab只能有一个
        QWidget* tabWidget = ui->link_tab->widget(i);    /////用了以后不能delete因为是指针
        if (CSC_tab* tab = dynamic_cast<CSC_tab*>(tabWidget)) {   ///遍历当前Tab判断是否已经存在,如果动态转换成功则表示有该类型
            ui->link_tab->setCurrentIndex(i);     /////如果之前创建了就定位到之前的tab
            is_exist = 1;
            break;
        }
    }
    if (!is_exist) {
        CSC_tab* tab = new CSC_tab(this);
        tab->setAttribute(Qt::WA_DeleteOnClose);
        int cur = ui->link_tab->addTab(tab, QString::asprintf(" CSC "));
        ui->link_tab->setCurrentIndex(cur);
        ui->link_tab->setVisible(true);
    }
}
///NR_YUV
void link_board::NR_YUV_DoubleClicked()
{
    bool is_exist = 0;
    for (int i = 0; i < ui->link_tab->count(); ++i) {    ///////用于设置该Tab只能有一个
        QWidget* tabWidget = ui->link_tab->widget(i);    /////用了以后不能delete因为是指针
        if (NR_YUV_tab* tab = dynamic_cast<NR_YUV_tab*>(tabWidget)) {   ///遍历当前Tab判断是否已经存在,如果动态转换成功则表示有该类型
            ui->link_tab->setCurrentIndex(i);     /////如果之前创建了就定位到之前的tab
            is_exist = 1;
            break;
        }
    }
    if (!is_exist) {
        NR_YUV_tab* tab = new NR_YUV_tab(this);
        tab->setAttribute(Qt::WA_DeleteOnClose);
        int cur = ui->link_tab->addTab(tab, QString::asprintf(" NR_YUV "));
        ui->link_tab->setCurrentIndex(cur);
        ui->link_tab->setVisible(true);
    }
}
//SCALE
void link_board::SCALE_DoubleClicked()
{
    bool is_exist = 0;
    for (int i = 0; i < ui->link_tab->count(); ++i) {    ///////用于设置该Tab只能有一个
        QWidget* tabWidget = ui->link_tab->widget(i);    /////用了以后不能delete因为是指针
        if (SCALE_tab* tab = dynamic_cast<SCALE_tab*>(tabWidget)) {   ///遍历当前Tab判断是否已经存在,如果动态转换成功则表示有该类型
            ui->link_tab->setCurrentIndex(i);     /////如果之前创建了就定位到之前的tab
            is_exist = 1;
            break;
        }
    }
    if (!is_exist) {
        SCALE_tab* tab = new SCALE_tab(this);
        tab->setAttribute(Qt::WA_DeleteOnClose);
        int cur = ui->link_tab->addTab(tab, QString::asprintf(" SCALE "));
        ui->link_tab->setCurrentIndex(cur);
        ui->link_tab->setVisible(true);
    }
}

//CROP
void link_board::CROP_DoubleClicked()
{
    bool is_exist = 0;
    for (int i = 0; i < ui->link_tab->count(); ++i) {    ///////用于设置该Tab只能有一个
        QWidget* tabWidget = ui->link_tab->widget(i);    /////用了以后不能delete因为是指针
        if (CROP_tab* tab = dynamic_cast<CROP_tab*>(tabWidget)) {   ///遍历当前Tab判断是否已经存在,如果动态转换成功则表示有该类型
            ui->link_tab->setCurrentIndex(i);     /////如果之前创建了就定位到之前的tab
            is_exist = 1;
            break;
        }
    }
    if (!is_exist) {
        CROP_tab* tab = new CROP_tab(this);
        tab->setAttribute(Qt::WA_DeleteOnClose);
        int cur = ui->link_tab->addTab(tab, QString::asprintf(" CROP "));
        ui->link_tab->setCurrentIndex(cur);
        ui->link_tab->setVisible(true);
    }
}

//YFC
void link_board::YFC_DoubleClicked()
{
    bool is_exist = 0;
    for (int i = 0; i < ui->link_tab->count(); ++i) {    ///////用于设置该Tab只能有一个
        QWidget* tabWidget = ui->link_tab->widget(i);    /////用了以后不能delete因为是指针
        if (YFC_tab* tab = dynamic_cast<YFC_tab*>(tabWidget)) {   ///遍历当前Tab判断是否已经存在,如果动态转换成功则表示有该类型
            ui->link_tab->setCurrentIndex(i);     /////如果之前创建了就定位到之前的tab
            is_exist = 1;
            break;
        }
    }
    if (!is_exist) {
        YFC_tab* tab = new YFC_tab(this);
        tab->setAttribute(Qt::WA_DeleteOnClose);
        int cur = ui->link_tab->addTab(tab, QString::asprintf(" YFC "));
        ui->link_tab->setCurrentIndex(cur);
        ui->link_tab->setVisible(true);
    }
}


//DEBUG
void link_board::Debug_DoubleClicked()
{
    bool is_exist = 0;
    for (int i = 0; i < ui->link_tab->count(); ++i) {    ///////用于设置该Tab只能有一个
        QWidget* tabWidget = ui->link_tab->widget(i);    /////用了以后不能delete因为是指针
        if (Debug* tab = dynamic_cast<Debug*>(tabWidget)) {   ///遍历当前Tab判断是否已经存在,如果动态转换成功则表示有该类型
            ui->link_tab->setCurrentIndex(i);            /////如果之前创建了就定位到之前的tab
            is_exist = 1;
            break;
        }
    }
    if (!is_exist) {
        Debug* tab = new Debug(this);
        tab->setAttribute(Qt::WA_DeleteOnClose);
        int cur = ui->link_tab->addTab(tab, QString::asprintf(" Debug "));
        ui->link_tab->setCurrentIndex(cur);
        ui->link_tab->setVisible(true);
    }
}

void link_board::Capture_DoubleClicked()
{
    bool is_exist = 0;
    for (int i = 0; i < ui->link_tab->count(); ++i) {    ///////用于设置该Tab只能有一个
        QWidget* tabWidget = ui->link_tab->widget(i);    /////用了以后不能delete因为是指针
        if (capture_tab* tab = dynamic_cast<capture_tab*>(tabWidget)) {   ///遍历当前Tab判断是否已经存在,如果动态转换成功则表示有该类型
            ui->link_tab->setCurrentIndex(i);            /////如果之前创建了就定位到之前的tab
            is_exist = 1;
            break;
        }
    }
    if (!is_exist) {
        capture_tab* tab = new capture_tab(this);
        tab->setAttribute(Qt::WA_DeleteOnClose);
        int cur = ui->link_tab->addTab(tab, QString::asprintf(" Capture "));
        ui->link_tab->setCurrentIndex(cur);
        ui->link_tab->setVisible(true);
    }
}

//////关闭Tab时清除内存
void link_board::on_link_tab_tabCloseRequested(int index)
{
    QWidget *tab=ui->link_tab->widget(index);//拿到当前Tab
    ui->link_tab->removeTab(index);
    delete tab;                            ////关闭Tab后释放Tab占用的资源
    //tab->close();
}

void link_board::save_image(const QByteArray &imageData)
{
    //弹出文件保存对话框
    QString fileName = QFileDialog::getSaveFileName(
        this,                           // 父窗口
        tr("保存文件"),                 // 对话框标题
        QDir::homePath(),               // 默认打开目录（用户主目录）
        tr("图片文件 (*.raw *.rgb *.yuv);;所有文件 (*)") // 文件过滤器
        );
    // 如果用户取消选择，fileName为空
    if (fileName.isEmpty()) {
        qDebug() << "用户取消了保存操作";
        return;
    }

    // 创建并打开文件
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this,tr("错误"),tr("无法创建文件:\n%1").arg(file.errorString()));
        return;
    }

    // 写入二进制数据
    qint64 bytesWritten = file.write(imageData);
    file.close();

    // 检查写入是否成功
    if (bytesWritten != imageData.size()) {
        QMessageBox::critical(this, tr("错误"), tr("写入文件不完整\n期望写入:%1字节\n实际写入:%2字节").arg(imageData.size()).arg(bytesWritten));
        return;
    }
    // 保存成功提示
    QMessageBox::information(this, tr("成功"), tr("图像已成功保存到:\n%1").arg(fileName));
}

void link_board::set_echo_text(QString str)
{
    ui->echo_text->appendPlainText(str);
    ui->echo_text->ensureCursorVisible(); // 确保光标可见（即滚动到底部）
}


////导入cfg配置文件
void link_board::on_import_cfg_btn_clicked()
{

}




////导出cfg配置文件
void link_board::on_export_cfg_btn_clicked()
{

}





//////将配置文件更新到开发板的SD卡中
void link_board::on_boot_cfg_btn_clicked()
{

}

