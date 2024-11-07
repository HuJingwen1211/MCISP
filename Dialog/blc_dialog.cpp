#include "blc_dialog.h"
#include "ui_blc_dialog.h"

blc_dialog::blc_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::blc_dialog)
{
    ui->setupUi(this);
}

blc_dialog::~blc_dialog()
{
    delete ui;
}

int *blc_dialog::get_blc_paramter()
{
    int* parameter=new int[4];
    parameter[0]=ui->blc_R->value();
    parameter[1]=ui->blc_Gr->value();
    parameter[2]=ui->blc_Gb->value();
    parameter[3]=ui->blc_B->value();

    return parameter;
}

void blc_dialog::set_blc_paramter(const int value[])
{
    ui->blc_R->setValue(value[0]);
    ui->blc_Gr->setValue(value[1]);
    ui->blc_Gb->setValue(value[2]);
    ui->blc_B->setValue(value[3]);
}
