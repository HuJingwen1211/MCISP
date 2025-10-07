#include "connect_dialog.h"
#include "ui_connect_dialog.h"
#include <QSerialPortInfo>
#include <QDebug>

ConnectDialog::ConnectDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ConnectDialog)
{
    ui->setupUi(this);
    ui->serial_radio->setChecked(true);
    ui->config_stack->setCurrentIndex(0);
    // Radio Button切换
    connect(ui->serial_radio, &QRadioButton::toggled, [this](bool checked) {
        if (checked) ui->config_stack->setCurrentIndex(0);
    });
    connect(ui->network_radio, &QRadioButton::toggled, [this](bool checked) {
        if (checked) ui->config_stack->setCurrentIndex(1);
    });
    
    // Connect按钮切换
    connect(ui->connect_btn, &QPushButton::clicked, [this]() {
        if (ui->connect_btn->text() == "Connect") {
            ui->connect_btn->setText("Disconnect");
            ui->connect_btn->setStyleSheet("background-color: #d32f2f; color: white;");
        } else {
            ui->connect_btn->setText("Connect");
            // 恢复默认样式
            ui->connect_btn->setStyleSheet("");
        }
    });
}

ConnectDialog::~ConnectDialog()
{
    delete ui;
}
