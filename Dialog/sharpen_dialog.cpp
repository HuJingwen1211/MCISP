#include "sharpen_dialog.h"
#include "ui_sharpen_dialog.h"

sharpen_dialog::sharpen_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::sharpen_dialog)
{
    ui->setupUi(this);
}

sharpen_dialog::~sharpen_dialog()
{
    delete ui;
}

int sharpen_dialog::get_sharpen_param()
{
    return ui->sharpen_param->value();
}

void sharpen_dialog::set_sharpen_param(int value)
{
    ui->sharpen_param->setValue(value);
}
