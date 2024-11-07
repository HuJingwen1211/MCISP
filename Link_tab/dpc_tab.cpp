#include "dpc_tab.h"
#include "ui_dpc_tab.h"

DPC_tab::DPC_tab(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DPC_tab)
{
    ui->setupUi(this);
}

DPC_tab::~DPC_tab()
{
    delete ui;
}
