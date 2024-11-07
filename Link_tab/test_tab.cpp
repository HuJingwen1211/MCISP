#include "test_tab.h"
#include "ui_test_tab.h"

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

///////配置按钮
void TEST_tab::on_config_btn_clicked()
{
    bool ok=false;
    QString Baseaddr_str = ui->baseaddr->text();
    BaseAddr=Baseaddr_str.toUInt(&ok, 16);         /////将字符串看作16进制读取
    if(!ok){
        link_tab->set_echo_text("Error: Plese check BaseAddr ,and use Hex format!");
        return;
    }
    ok=false;
    QString offsetaddr_str=ui->offsetaddr->text();
    OffsetAddr= static_cast<quint16>(offsetaddr_str.toUInt(&ok,16));
    if(!ok){
        link_tab->set_echo_text("Error: Plese check OffsetAddr ,and use Hex format!");
        return;
    }
    ok=false;
    test_value=ui->paramvalue->text().toInt(&ok);
    if(!ok){
        link_tab->set_echo_text("Error: Plese check ParamValue ,and use Decimal format!");
        return;
    }
    ////////组合发送的数据流
    QByteArray transdata;
    QDataStream stream(&transdata, QIODevice::WriteOnly);   /////将数据导入到字节数组中
    stream << BaseAddr << OffsetAddr << test_value;
    link_tab->Write_Data(transdata);        /////发送组合数据
}
