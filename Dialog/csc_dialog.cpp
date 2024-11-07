#include "csc_dialog.h"
#include "ui_csc_dialog.h"

csc_dialog::csc_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::csc_dialog)
{
    ui->setupUi(this);
}

csc_dialog::~csc_dialog()
{
    delete ui;
}

int *csc_dialog::get_offsetin()
{
    int *offsetin=new int[3];
    offsetin[0]=ui->offset_in0->value();
    offsetin[1]=ui->offset_in1->value();
    offsetin[2]=ui->offset_in2->value();
    return offsetin;
}

int *csc_dialog::get_csccoeff()
{
    int *csccoeff=new int[9];
    csccoeff[0]=ui->csc_coeff0->value();
    csccoeff[1]=ui->csc_coeff1->value();
    csccoeff[2]=ui->csc_coeff2->value();
    csccoeff[3]=ui->csc_coeff3->value();
    csccoeff[4]=ui->csc_coeff4->value();
    csccoeff[5]=ui->csc_coeff5->value();
    csccoeff[6]=ui->csc_coeff6->value();
    csccoeff[7]=ui->csc_coeff7->value();
    csccoeff[8]=ui->csc_coeff8->value();
    return csccoeff;
}

int *csc_dialog::get_offsetout()
{
    int *offsetout=new int[3];
    offsetout[0]=ui->offset_out0->value();
    offsetout[1]=ui->offset_out1->value();
    offsetout[2]=ui->offset_out2->value();
    return offsetout;
}

void csc_dialog::set_offsetin(const int offsetin[])
{
    ui->offset_in0->setValue(offsetin[0]);
    ui->offset_in1->setValue(offsetin[1]);
    ui->offset_in2->setValue(offsetin[2]);
}

void csc_dialog::set_csccoeff(const int csccoeff[])
{
    ui->csc_coeff0->setValue(csccoeff[0]);
    ui->csc_coeff1->setValue(csccoeff[1]);
    ui->csc_coeff2->setValue(csccoeff[2]);
    ui->csc_coeff3->setValue(csccoeff[3]);
    ui->csc_coeff4->setValue(csccoeff[4]);
    ui->csc_coeff5->setValue(csccoeff[5]);
    ui->csc_coeff6->setValue(csccoeff[6]);
    ui->csc_coeff7->setValue(csccoeff[7]);
    ui->csc_coeff8->setValue(csccoeff[8]);
}

void csc_dialog::set_offsetout(const int offsetout[])
{
    ui->offset_out0->setValue(offsetout[0]);
    ui->offset_out1->setValue(offsetout[1]);
    ui->offset_out2->setValue(offsetout[2]);
}
