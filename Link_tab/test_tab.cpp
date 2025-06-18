#include "test_tab.h"
#include "ui_test_tab.h"
#include <QMessageBox>

TEST_tab::TEST_tab(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TEST_tab)
{
    ui->setupUi(this);
    link_tab=qobject_cast<link_board*>(this->parent());   ////父对象指针,new的时候传入,拿取以后便于对象之间的通信，无需对其进行清除
}

TEST_tab::~TEST_tab()
{
    delete ui;
}

void TEST_tab::on_test_read_btn_clicked()
{
    QString addrText = ui->Addr->text().trimmed();
    bool ok;
    uint32_t Addr = addrText.toULong(&ok, 0);
    if (!ok) {
        QMessageBox::warning(this, "Error", "Invalid Addr format for numeric conversion!");
        return;
    }
    qDebug()<<"Read Addr:"<<Qt::uppercasedigits << Qt::hex<<Addr;
    uint8_t databuf[4];
    memcpy(databuf, &Addr, 4);
    link_tab->send_cmd_data(TEST_RW_CMD, databuf, 4);
}


void TEST_tab::on_test_write_btn_clicked()
{
    QString addrText = ui->Addr->text().trimmed();
    QString valueText = ui->Value->text().trimmed();
    bool ok;
    uint32_t Addr = addrText.toULong(&ok, 0);
    if (!ok) {
        QMessageBox::warning(this, "Error", "Invalid Addr format for numeric conversion!");
        return;
    }
    uint32_t value = valueText.toULong(&ok, 0);
    if (!ok) {
        QMessageBox::warning(this, "Error", "Invalid value format for numeric conversion!");
        return;
    }
    qDebug()<<"Write Addr:"<<Qt::uppercasedigits << Qt::hex<<Addr;
    qDebug()<<"Write Value:"<<Qt::uppercasedigits << Qt::hex<<value;
    uint8_t databuf[8];
    memcpy(databuf, &Addr, 4);
    memcpy(databuf+4, &value, 4);
    link_tab->send_cmd_data(TEST_RW_CMD, databuf, 8);
}

void TEST_tab::read_reg_process(const QByteArray &regData)
{
    qDebug()<<"reg read finish";
    if(regData.size()==4){
        const uchar *ptr = reinterpret_cast<const uchar*>(regData.constData());
        uint32_t value = (ptr[3] << 24 | ptr[2] << 16 | ptr[1] << 8 | ptr[0]);
        // uint32_t value = static_cast<uint32_t>(regData[3] << 24 | regData[2] << 16 | regData[1] << 8 | regData[0]);
        ui->Value->setText(QString("0x%1").arg(value, 8, 16, QLatin1Char('0')));
    }else{
        qDebug()<<"Error read reg len";
    }
}





