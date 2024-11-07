#ifndef TEST_TAB_H
#define TEST_TAB_H

#include <QMainWindow>
#include "link_board.h"

class link_board; //// 前向声明link_board类

namespace Ui {
class TEST_tab;
}

class TEST_tab : public QMainWindow
{
    Q_OBJECT

public:
    explicit TEST_tab(QWidget *parent = 0);
    ~TEST_tab();

private slots:
    void on_config_btn_clicked();

private:
    Ui::TEST_tab *ui;
    link_board *link_tab;
    quint32  BaseAddr;   ///4字节及基地址
    quint16  OffsetAddr; ///2字节偏移地址
    qint32   test_value;        ///4字节有符号值
    //QString hexString = hexInput.text();
    //int decimalValue = hexString.toInt(&ok, 16);   if(ok)
};

#endif // TEST_TAB_H

//qint8 negativeValue = -42;
//QByteArray byteArray;
//byteArray.append(static_cast<char>(negativeValue));

//qint64 bytesWritten = serialPort.write(byteArray);  ///发送负数
//qint8 negativeValue = static_cast<qint8>(receivedData.at(0)); ///接收负数
