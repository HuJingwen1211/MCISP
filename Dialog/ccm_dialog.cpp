#include "ccm_dialog.h"
#include "ui_ccm_dialog.h"

ccm_dialog::ccm_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ccm_dialog)
{
    ui->setupUi(this);
}

ccm_dialog::~ccm_dialog()
{
    delete ui;
}

int *ccm_dialog::get_ccm_coeff()
{
    int* ccm_coeff=new int[9];
    ccm_coeff[0]=ui->ccm_coeff0->value();
    ccm_coeff[1]=ui->ccm_coeff1->value();
    ccm_coeff[2]=ui->ccm_coeff2->value();
    ccm_coeff[3]=ui->ccm_coeff3->value();
    ccm_coeff[4]=ui->ccm_coeff4->value();
    ccm_coeff[5]=ui->ccm_coeff5->value();
    ccm_coeff[6]=ui->ccm_coeff6->value();
    ccm_coeff[7]=ui->ccm_coeff7->value();
    ccm_coeff[8]=ui->ccm_coeff8->value();
    return ccm_coeff;
}

int *ccm_dialog::get_offset_out()
{
    int* offest_out=new int[3];
    offest_out[0]=ui->offset_out0->value();
    offest_out[1]=ui->offset_out1->value();
    offest_out[2]=ui->offset_out2->value();
    return offest_out;
}

void ccm_dialog::set_ccm_coeff(const int coeff[])
{
    ui->ccm_coeff0->setValue(coeff[0]);
    ui->ccm_coeff1->setValue(coeff[1]);
    ui->ccm_coeff2->setValue(coeff[2]);
    ui->ccm_coeff3->setValue(coeff[3]);
    ui->ccm_coeff4->setValue(coeff[4]);
    ui->ccm_coeff5->setValue(coeff[5]);
    ui->ccm_coeff6->setValue(coeff[6]);
    ui->ccm_coeff7->setValue(coeff[7]);
    ui->ccm_coeff8->setValue(coeff[8]);
}

void ccm_dialog::set_offset_out(const int offset[])
{
    ui->offset_out0->setValue(offset[0]);
    ui->offset_out1->setValue(offset[1]);
    ui->offset_out2->setValue(offset[2]);
}


