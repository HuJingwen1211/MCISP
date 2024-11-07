#include "nlm_nr_dialog.h"
#include "ui_nlm_nr_dialog.h"

nlm_nr_dialog::nlm_nr_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::nlm_nr_dialog)
{
    ui->setupUi(this);
}

nlm_nr_dialog::~nlm_nr_dialog()
{
    delete ui;
}

int nlm_nr_dialog::get_nlm_h()
{
    return ui->spinBox->value();
}

void nlm_nr_dialog::set_nlm_h(int value)
{
    ui->spinBox->setValue(value);
}
