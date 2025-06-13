#include "link_board.h"
#include "ui_link_board.h"
#include <QDebug>

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
    ui->link_statu->setStyleSheet("background-color: #FF3030;"); ////指示灯为红
}


link_board::~link_board()
{
    delete ui;
}

/////接收串口数据
void link_board::Read_Data()
{
    QByteArray receivedData;
    receivedData = serial->readAll();
    if(!receivedData.isEmpty())
    {
//        QDataStream stream(&buf, QIODevice::ReadOnly);
//        quint32 receivedBaseAddr;
//        quint16 receivedOffset;
//        qint32 receivedValue;
//        stream >> receivedBaseAddr >> receivedOffset >> receivedValue;
//        set_echo_text("BaseAddr:"+QString::number(receivedBaseAddr, 16).toUpper());
//        set_echo_text("OffsetAddr:"+QString::number(receivedOffset, 16).toUpper());
//        set_echo_text("ParamValue:"+QString::number(receivedValue, 10).toUpper());
        QString receivedString = QString::fromUtf8(receivedData);    ////接收来自串口的数据
        set_echo_text(receivedString);
    }
}

int link_board::Write_Data(QByteArray transdata)
{
    if(!serial || !serial->isOpen()){     /////判断当前串口是否打开
        set_echo_text("Serial port dose not open, please check and retry!");
        return -1;
    }
    qint64 bytesWritten =serial->write(transdata);   /////发送字节数组,bytesWritten指示发送的字节数
//    qDebug()<<"transdata is "<<transdata;
//    qDebug()<<"Writedata is "<<bytesWritten;
    if(bytesWritten==-1){
        set_echo_text("Failed to write to serial port:"+serial->errorString());
        return -1;
    }else{
        set_echo_text(tr("Data sent successfully !"));
    }
    return 0;
}

//发送字节
void link_board::SendByte(char transdata){
    if (serial->isOpen()) {
        serial->putChar(transdata);
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
   if(ui->link_btn->text()==tr("Link Board")){
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
           ui->link_statu->setStyleSheet("background-color: #00FF7F;");
       } else {
           ui->echo_text->appendPlainText(QString("Failed to open serial port!"));
           return;
           //qDebug() << "Failed to open serial port.";
       }
       //serial->open(QIODevice::ReadWrite);           ////打开串口

       ui->port_combx->setEnabled(false);
       ui->baud_combx->setEnabled(false);
       ui->link_btn->setText(tr("Disconnect"));
       ///ui->sendButton->setEnabled(true);

       ////设置串口接收数据ready
       QObject::connect(serial, &QSerialPort::readyRead, this, &link_board::Read_Data);  ////连接接收信号槽函数
       this->SendByte(CONNECT_CODE);   //发送连接代码
   }else{
       //关闭串口
       serial->clear();
       serial->close();
       serial->deleteLater();  //稍后清理
       //恢复设置使能
       ui->port_combx->setEnabled(true);
       ui->baud_combx->setEnabled(true);
       ui->link_btn->setText(tr("Link Board"));
       ui->link_statu->setStyleSheet("background-color: #FF3030;");
       ///ui->sendButton->setEnabled(false);
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
        int cur = ui->link_tab->addTab(tab, QString::asprintf(" ENABLE "));
        ui->link_tab->setCurrentIndex(cur);
        ui->link_tab->setVisible(true);
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



//////关闭Tab时清除内存
void link_board::on_link_tab_tabCloseRequested(int index)
{
    QWidget *tab=ui->link_tab->widget(index);//拿到当前Tab
    ui->link_tab->removeTab(index);
    delete tab;                            ////关闭Tab后释放Tab占用的资源
    //tab->close();
}



void link_board::set_echo_text(QString str)
{
    ui->echo_text->appendPlainText(str);
}


