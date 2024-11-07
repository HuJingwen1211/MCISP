#include "gamma_dialog.h"
#include "ui_gamma_dialog.h"

gamma_dialog::gamma_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::gamma_dialog)
{
    ui->setupUi(this);
}

gamma_dialog::~gamma_dialog()
{
    delete ui;
}

double gamma_dialog::get_gamma()
{
    return ui->gamma->value();
}

void gamma_dialog::set_gamma(double value)
{
    ui->gamma->setValue(value);
}
