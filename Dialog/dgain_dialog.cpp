#include "dgain_dialog.h"
#include "ui_dgain_dialog.h"

Dgain_Dialog::Dgain_Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dgain_Dialog)
{
    ui->setupUi(this);
}

Dgain_Dialog::~Dgain_Dialog()
{
    delete ui;
}

double Dgain_Dialog::get_Gain_R()
{
    return  ui->Gain_R->value();
}

double Dgain_Dialog::get_Gain_Gr()
{
    return ui->Gain_Gr->value();
}

double Dgain_Dialog::get_Gain_Gb()
{
    return ui->Gain_Gb->value();
}

double Dgain_Dialog::get_Gain_B()
{
    return ui->Gain_B->value();
}

void Dgain_Dialog::set_Gain(double last_r, double last_gr, double last_gb, double last_b)
{
    ui->Gain_R->setValue(last_r);
    ui->Gain_Gr->setValue(last_gr);
    ui->Gain_Gb->setValue(last_gb);
    ui->Gain_B->setValue(last_b);
}
