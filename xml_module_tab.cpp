#include "xml_module_tab.h"
#include "ui_xml_module_tab.h"
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QTimer>
#include <QXmlStreamReader>
#include <QToolButton>

XMLModuleTab::XMLModuleTab(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::XMLModuleTab)
    , m_connected(false)
    , m_moduleLayout(nullptr)
{
    ui->setupUi(this);


    connect(ui->connect_action,       &QAction::triggered, this, &XMLModuleTab::connectToBoard);
    connect(ui->import_xml_action,    &QAction::triggered, this, &XMLModuleTab::importXML);
    // connect(ui->export_xml_action,    &QAction::triggered, this, &XMLModuleTab::exportXml);
    connect(ui->import_config_action, &QAction::triggered, this, &XMLModuleTab::importConfig);
    connect(ui->export_config_action, &QAction::triggered, this, &XMLModuleTab::exportConfig);
    connect(ui->all_read_action,      &QAction::triggered, this, &XMLModuleTab::allRead);
    connect(ui->all_write_action,     &QAction::triggered, this, &XMLModuleTab::allWrite);


    connect(ui->module_list,          &QTreeWidget::itemDoubleClicked, this, &XMLModuleTab::onModuleTreeDoubleClicked);

    m_moduleLayout = new QVBoxLayout(ui->scrollAreaWidgetContents);
    m_moduleLayout->addStretch();

}

XMLModuleTab::~XMLModuleTab()
{
    delete ui;
}



void XMLModuleTab::connectToBoard()
{
    // 连接到板子
    // 弹出连接对话框
    // 复用link_board的连接对话框，可以选择串口或网络连接
}

void XMLModuleTab::importXML()
{
    QString path = QFileDialog::getOpenFileName(
        this,
        tr("import XML"),
        QDir::homePath(),
        tr("XML Files (*.xml);;All Files (*.*)")
        );
    if (path.isEmpty()) {
        return;
    }
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("Warning"), tr("Failed to open XML: %1").arg(path));
        return;
    }

    QByteArray data = file.readAll();
    file.close();
    if (parseXML(data)) {
        generateUI();
        if (ui->echo_text) {
            ui->echo_text->appendPlainText(QString("[%1] Imported XML: %2")
                .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"), path));
        }
        // QMessageBox::information(this, tr("Success"), tr("XML imported:\n%1").arg(path));

    } else {
        QMessageBox::warning(this, tr("Warning"), tr("Failed to parse XML: %1").arg(path));
    }
}

void XMLModuleTab::importConfig()
{
    // 导入配置
}

void XMLModuleTab::exportConfig()
{
    // 导出配置
}

void XMLModuleTab::allRead()
{
    // 全部读取
}

void XMLModuleTab::allWrite()
{
    // 全部写入
}


bool XMLModuleTab::parseXML(const QByteArray &data)
{
    m_xmlConfig = XMLConfig{};
    QXmlStreamReader reader(data);
    Module curModule;
    bool inModule = false;

    while (!reader.atEnd()) {
        reader.readNext();
        const QString tag = reader.name().toString();
        if (reader.isStartElement()) {
            
            if (tag == "MODULE") {
                curModule = Module{};
                curModule.moduleName = reader.attributes().value("id").toString();
                inModule = true;
            }
            else if (tag == "PARAM" && inModule) {
                const auto attrs = reader.attributes();
                Param curParam{
                    .paramName = attrs.value("id").toString().trimmed(),
                    .min = attrs.value("min").toString().trimmed().toInt(),
                    .max = attrs.value("max").toString().trimmed().toInt(),
                    .defaultVal = attrs.value("default").toString().trimmed().toInt(),
                    .address = attrs.value("address").toString().trimmed().toInt(nullptr, 0)
                };
                curModule.params.append(curParam);
            }
        }
        // end
        else if (reader.isEndElement() && tag == "MODULE" && inModule) {
            if (!curModule.moduleName.isEmpty()) {
                m_xmlConfig.modules.append(curModule);
                m_xmlConfig.moduleOrder.append(curModule.moduleName);
            }
            inModule = false;
        }
    }
    return !reader.hasError();
}

void XMLModuleTab::onModuleTreeDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    if (!item) return;

    const QString targetTitle = item->text(0);
    QWidget* container = ui->scrollAreaWidgetContents;
    if (!container) return;

    QGroupBox* targetGroup = nullptr;
    const auto children = container->findChildren<QGroupBox*>();
    for (QGroupBox* gb : children) {
        if (gb && gb->title() == targetTitle) {
            targetGroup = gb;
            break;
        }
    }
    if (!targetGroup) return;

    ui->scrollArea->ensureWidgetVisible(targetGroup);

    // 可选高亮0.5秒
    targetGroup->setStyleSheet("QGroupBox{border:2px solid #0078d4;}");
    QTimer::singleShot(500, [targetGroup]() { targetGroup->setStyleSheet(""); });
}

void XMLModuleTab::printXMLConfig()
{
    // 打印解析结果
    ui->echo_text->appendPlainText(QString("=== 解析结果 ==="));
    ui->echo_text->appendPlainText(QString("模块总数: %1").arg(m_xmlConfig.modules.size()));
    ui->echo_text->appendPlainText(QString("模块顺序: %1").arg(m_xmlConfig.moduleOrder.join(", ")));
    ui->echo_text->appendPlainText("");
    
    // 详细打印每个模块
    for (const Module& module : m_xmlConfig.modules) {
        ui->echo_text->appendPlainText(QString("模块: %1").arg(module.moduleName));
        ui->echo_text->appendPlainText(QString("  参数数量: %1").arg(module.params.size()));
        
        for (const Param& param : module.params) {
            ui->echo_text->appendPlainText(QString("    - %1: min=%2, max=%3, default=%4, addr=0x%5")
                .arg(param.paramName)
                .arg(param.min)
                .arg(param.max)
                .arg(param.defaultVal)
                .arg(param.address, 0, 16));  // 16进制显示地址
        }
    }
    return;
}

void XMLModuleTab::generateUI()
{
    clearUI();
    generateModuleTree();
    // printXMLConfig();
    for (const Module& module : m_xmlConfig.modules) {
        generateModuleGroup(module);
    }
}
void XMLModuleTab::generateModuleTree()
{
    for (const QString& module : m_xmlConfig.moduleOrder) {
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->module_list);
        item->setText(0, module);
    }
}
void XMLModuleTab::generateModuleGroup(const Module &module)
{
    QGroupBox* group = new QGroupBox(module.moduleName, ui->scrollAreaWidgetContents);
    group->setCheckable(true);
    group->setChecked(true);

    // 主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(group);

    // button
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();

    QToolButton* readBtn = new QToolButton(group);
    QToolButton* writeBtn = new QToolButton(group);
    readBtn->setIcon(QIcon::fromTheme("media-tape"));
    writeBtn->setIcon(QIcon::fromTheme("mail-message-new"));
    buttonLayout->addWidget(readBtn);
    buttonLayout->addWidget(writeBtn);

    connect(readBtn, &QToolButton::clicked, [this, group]() {
        readModule(group);
    });
    connect(writeBtn, &QToolButton::clicked, [this, group]() {
        writeModule(group);
    });
    mainLayout->addLayout(buttonLayout);

    // param
    QGridLayout* paramLayout = new QGridLayout(group);

    int row = 0;
    for (const Param& param : module.params) {
        QLabel* label = new QLabel(param.paramName + ":", group);
        QSpinBox* spinBox = new QSpinBox(group);
        spinBox->setRange(param.min, param.max);
        spinBox->setValue(param.defaultVal);
        spinBox->setProperty("address", param.address);
        
        paramLayout->addWidget(label, row, 0);    // 第row行，第0列
        paramLayout->addWidget(spinBox, row, 1);  // 第row行，第1列
        row++;
    }

    mainLayout->addLayout(paramLayout);

    m_moduleLayout->addWidget(group);
}

void XMLModuleTab::clearUI()
{
    // tree
    ui->module_list->clear();
    // group
    QLayout* layout = ui->scrollAreaWidgetContents->layout();
    if (!layout) return;
    
    QLayoutItem* item;
    while ((item = layout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
}

void XMLModuleTab::readModule(QGroupBox *group)
{
    if (ui->echo_text) {
        ui->echo_text->appendPlainText(QString("[%1] %2 : Read module").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"), group->title()));
    }
}
void XMLModuleTab::writeModule(QGroupBox *group)
{
    if (ui->echo_text) {
        ui->echo_text->appendPlainText(QString("[%1] %2 : Write module").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"), group->title()));
    }
}
