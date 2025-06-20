#include "enable_tab.h"
#include "ui_enable_tab.h"

ENABLE_tab::ENABLE_tab(QWidget* parent) :
	QMainWindow(parent),
	ui(new Ui::ENABLE_tab)
{
	ui->setupUi(this);
	link_tab = qobject_cast<link_board*>(this->parent());
}

ENABLE_tab::~ENABLE_tab()
{
	delete ui;
}

void ENABLE_tab::on_cfg_btn_clicked() {
    uint32_t enable_value = 0;   //初始化为0
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
    if (ui->AWB_check->isChecked()) {
        enable_value |= AWB_MODULE_EN;
    }
    if(ui->WBC_check->isChecked()){
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
    uint8_t databuf[8];
    uint32_t EN_ADDR = REG_MODULE_EN_ADDR;
    uint32_t ISP_RST_ADDR = 0xA0030000;
    uint32_t ISP_RST_VAL  = 0x00;
    memcpy(databuf, &ISP_RST_ADDR, 4);
    memcpy(databuf+4, &ISP_RST_VAL, 4);
    link_tab->send_cmd_data(TEST_RW_CMD, databuf, 8);

    memcpy(databuf, &EN_ADDR, 4);
    memcpy(databuf+4, &enable_value, 4);
    link_tab->send_cmd_data(TEST_RW_CMD, databuf, 8);   ////借用TEST_RW_CMD写一个寄存器

    ISP_RST_VAL = 0x01;
    memcpy(databuf, &ISP_RST_ADDR, 4);
    memcpy(databuf+4, &ISP_RST_VAL, 4);
    link_tab->send_cmd_data(TEST_RW_CMD, databuf, 8);

}


