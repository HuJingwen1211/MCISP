#include "enable_tab.h"
#include "ui_enable_tab.h"
#include <iostream>

ENABLE_tab::ENABLE_tab(QWidget* parent) :
	QMainWindow(parent),
	ui(new Ui::ENABLE_tab)
{
	ui->setupUi(this);
	link_tab = qobject_cast<link_board*>(this->parent());
    enable_value = 0;
}

ENABLE_tab::~ENABLE_tab()
{
	delete ui;
}


void ENABLE_tab::on_cfg_btn_clicked() {

    if (link_tab->serial == NULL || link_tab->serial->isOpen() == false) {
        qDebug()<<"串口未打开";
        return;
    }
    bool ok = false;
    QString Baseaddr_str = ui->baseaddr->text();
    BaseAddr = Baseaddr_str.toUInt(&ok, 16);         /////将字符串看作16进制读取
    if (!ok) {
        link_tab->set_echo_text("Error: Plese check BaseAddr ,and use Hex format!");
        return;
    }
    ok = false;
    QString offsetaddr_str = ui->offsetaddr->text();
    OffsetAddr = static_cast<quint16>(offsetaddr_str.toUInt(&ok, 16));
    if (!ok) {
        link_tab->set_echo_text("Error: Plese check OffsetAddr ,and use Hex format!");
        return;
    }
    ok = false;
    enable_value = 0;
    //enable_value |= DMS_MODULE_EN;
    //enable_value |= STA_MODULE_EN;
    if (ui->DMS_check->isChecked()) {
        enable_value |= DMS_MODULE_EN;
    }
    if (ui->STA_check->isChecked()) {
        enable_value |= STA_MODULE_EN;
    }
    if (ui->TPG_check->isChecked()) {
        enable_value |= TPG_MODULE_EN;
    }
    if (ui->DPC_check->isChecked()) {
        enable_value |= DPC_MODULE_EN;
    }
    if (ui->BLC_check->isChecked()) {
        enable_value |= BLC_MODULE_EN;
    }
    if (ui->LSC_check->isChecked()) {
        enable_value |= LSC_MODULE_EN;
    }
    if (ui->NR_RAW_check->isChecked()) {
        enable_value |= NR_RAW_MODULE_EN;
    }
    if (ui->AWBC_check->isChecked()) {
        enable_value |= AWB_MODULE_EN;
        enable_value |= WBC_MODULE_EN;
    }
    if (ui->GB_check->isChecked()) {
        enable_value |= GB_MODULE_EN;
    }
    if (ui->CCM_check->isChecked()) {
        enable_value |= CCM_MODULE_EN;
    }
    if (ui->GAMMA_check->isChecked()) {
        enable_value |= GAMMA_MODULE_EN;
    }
    if (ui->CSC_check->isChecked()) {
        enable_value |= CSC_MODULE_EN;
    }
    if (ui->NR_YUV_check->isChecked()){
        enable_value |= NR_YUV_MODULE_EN;
    }
    QByteArray transdata;
    QDataStream stream(&transdata, QIODevice::WriteOnly);   /////将数据导入到字节数组中
    stream << BaseAddr << OffsetAddr << enable_value;
    link_tab->Write_Data(transdata);        /////发送组合数据

}


