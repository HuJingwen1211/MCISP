#include "nryuv_dialog.h"
#include "ui_nryuv_dialog.h"

nryuv_dialog::nryuv_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::nryuv_dialog)
{
    ui->setupUi(this);
}

nryuv_dialog::~nryuv_dialog()
{
    delete ui;
}

int nryuv_dialog::get_ysigma2()
{
    return ui->y_sigma2->value();
}

int nryuv_dialog::get_uvsigma2()
{
    return ui->uv_sigma2->value();
}

int nryuv_dialog::get_yfilt()
{
    return ui->y_filt->value();
}

int nryuv_dialog::get_uvfilt()
{
    return ui->uv_filt->value();
}

void nryuv_dialog::set_ysigma2(int value)
{
    ui->y_sigma2->setValue(value);
}

void nryuv_dialog::set_uvsigma2(int value)
{
    ui->uv_sigma2->setValue(value);
}

void nryuv_dialog::set_yfilt(int value)
{
    ui->y_filt->setValue(value);
}

void nryuv_dialog::set_uvfilt(int value)
{
    ui->uv_filt->setValue(value);
}
