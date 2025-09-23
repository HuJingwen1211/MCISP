#include "xml_module_tab.h"
#include "ui_xml_module_tab.h"
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>

XMLModuleTab::XMLModuleTab(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::XMLModuleTab)
    , m_connected(false)
    , m_moduleLayout(nullptr)
{
    ui->setupUi(this);
    m_moduleLayout = new QVBoxLayout(ui->scrollAreaWidgetContents);
    createTestModule();
    m_moduleLayout->addStretch();

}

XMLModuleTab::~XMLModuleTab()
{
    delete ui;
}

void XMLModuleTab::createTestModule()
{
    // 创建测试模块1 - AWB模块
    QGroupBox* awbGroup = new QGroupBox("AWB模块", ui->scrollAreaWidgetContents);
    awbGroup->setCheckable(true);  // 可折叠
    awbGroup->setChecked(true);    // 默认展开
    
    QGridLayout* awbLayout = new QGridLayout(awbGroup);
    
    // 添加一些测试控件
    QLabel* enableLabel = new QLabel("Enable:", awbGroup);
    QCheckBox* enableCheck = new QCheckBox(awbGroup);
    enableCheck->setChecked(true);
    
    QLabel* gainLabel = new QLabel("R Gain:", awbGroup);
    QSpinBox* gainSpin = new QSpinBox(awbGroup);
    gainSpin->setRange(0, 4095);
    gainSpin->setValue(1024);
    
    QPushButton* readBtn = new QPushButton("Read", awbGroup);
    QPushButton* writeBtn = new QPushButton("Write", awbGroup);
    
    // 布局控件
    awbLayout->addWidget(enableLabel, 0, 0);
    awbLayout->addWidget(enableCheck, 0, 1);
    awbLayout->addWidget(gainLabel, 1, 0);
    awbLayout->addWidget(gainSpin, 1, 1);
    awbLayout->addWidget(readBtn, 0, 2);
    awbLayout->addWidget(writeBtn, 1, 2);
    
    // 添加到主布局
    m_moduleLayout->addWidget(awbGroup);
    
    // 创建测试模块2 - BLC模块
    QGroupBox* blcGroup = new QGroupBox("BLC模块", ui->scrollAreaWidgetContents);
    blcGroup->setCheckable(true);
    blcGroup->setChecked(true);
    
    QGridLayout* blcLayout = new QGridLayout(blcGroup);
    
    QLabel* thresholdLabel = new QLabel("Threshold:", blcGroup);
    QSpinBox* thresholdSpin = new QSpinBox(blcGroup);
    thresholdSpin->setRange(0, 1023);
    thresholdSpin->setValue(64);
    
    QPushButton* blcReadBtn = new QPushButton("Read", blcGroup);
    QPushButton* blcWriteBtn = new QPushButton("Write", blcGroup);
    
    blcLayout->addWidget(thresholdLabel, 0, 0);
    blcLayout->addWidget(thresholdSpin, 0, 1);
    blcLayout->addWidget(blcReadBtn, 0, 2);
    blcLayout->addWidget(blcWriteBtn, 1, 2);
    
    m_moduleLayout->addWidget(blcGroup);
}
