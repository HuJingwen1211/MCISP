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
    void on_test_read_btn_clicked();

    void on_test_write_btn_clicked();
    void read_reg_process(const QByteArray &regData);

private:
    Ui::TEST_tab *ui;
    link_board *link_tab;


};

#endif // TEST_TAB_H

//qint8 negativeValue = -42;
//QByteArray byteArray;
//byteArray.append(static_cast<char>(negativeValue));

//qint64 bytesWritten = serialPort.write(byteArray);  ///发送负数
//qint8 negativeValue = static_cast<qint8>(receivedData.at(0)); ///接收负数
