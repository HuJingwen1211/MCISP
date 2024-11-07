#include "awb_manual_dialog.h"
#include "ui_awb_manual_dialog.h"

awb_manual_dialog::awb_manual_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::awb_manual_dialog)
{
    ui->setupUi(this);
}

awb_manual_dialog::~awb_manual_dialog()
{
    delete ui;
}

int awb_manual_dialog::get_RGain()
{
    return ui->R_Gain->value();
}

int awb_manual_dialog::get_GrGain()
{
    return ui->Gr_Gain->value();
}

int awb_manual_dialog::get_GbGain()
{
    return ui->Gb_Gain->value();
}

int awb_manual_dialog::get_BGain()
{
    return ui->B_Gain->value();
}

void awb_manual_dialog::set_RGain(int value)
{
    ui->R_Gain->setValue(value);
}

void awb_manual_dialog::set_GrGain(int value)
{
    ui->Gr_Gain->setValue(value);
}

void awb_manual_dialog::set_GbGain(int value)
{
    ui->Gb_Gain->setValue(value);
}

void awb_manual_dialog::set_BGain(int value)
{
    ui->B_Gain->setValue(value);
}
