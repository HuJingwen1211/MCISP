#include "dpc_dialog.h"
#include "ui_dpc_dialog.h"

dpc_dialog::dpc_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dpc_dialog)
{
    ui->setupUi(this);
}

dpc_dialog::~dpc_dialog()
{
    delete ui;
}

int dpc_dialog::get_dpc_mode()
{
    return ui->dpc_correct_mode->value();
}

void dpc_dialog::set_dpc_mode(int value)
{
    ui->dpc_correct_mode->setValue(value);
}
