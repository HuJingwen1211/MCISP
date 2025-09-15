#include "xml_module_tab.h"
#include "ui_xml_module_tab.h"

XMLModuleTab::XMLModuleTab(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::XMLModuleTab)
{
    ui->setupUi(this);
}

XMLModuleTab::~XMLModuleTab()
{
    delete ui;
}
