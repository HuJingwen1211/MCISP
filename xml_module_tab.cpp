#include "xml_module_tab.h"
#include "ui_xml_module_tab.h"
#include "module_edit_dialog.h"
#include "Comm/connect_dialog.h"
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
#include <QDebug>
#include <QInputDialog>
#include <QHeaderView>
XMLModuleTab::XMLModuleTab(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::XMLModuleTab)
    , m_connected(false)
{
    ui->setupUi(this);
    ui->add_module_btn->setVisible(false);

    connect(ui->connect_action,       &QAction::triggered, this, &XMLModuleTab::showConnectDialog);
    connect(ui->edit_mode_action,     &QAction::triggered, this, &XMLModuleTab::setEditMode);
    connect(ui->refresh_action,       &QAction::triggered, this, &XMLModuleTab::refreshToDefault);
    connect(ui->import_xml_action,    &QAction::triggered, this, &XMLModuleTab::importXML);
    connect(ui->export_xml_action,    &QAction::triggered, this, &XMLModuleTab::exportXml);
    connect(ui->import_config_action, &QAction::triggered, this, &XMLModuleTab::importConfig);
    connect(ui->export_config_action, &QAction::triggered, this, &XMLModuleTab::exportConfig);
    connect(ui->all_read_action,      &QAction::triggered, this, &XMLModuleTab::allRead);
    connect(ui->all_write_action,     &QAction::triggered, this, &XMLModuleTab::allWrite);


    connect(ui->module_list,          &QTreeWidget::itemDoubleClicked, this, &XMLModuleTab::onModuleTreeDoubleClicked);

    connect(ui->add_module_btn,       &QToolButton::clicked, this, &XMLModuleTab::addNewModule);


    // 关键：给scrollAreaWidgetContents设置布局
    ui->scrollAreaWidgetContents->setLayout(new QVBoxLayout());
    
}

XMLModuleTab::~XMLModuleTab()
{
    delete ui;
}



void XMLModuleTab::showConnectDialog()
{
    // 连接到板子
    // 弹出连接对话框
    // 复用link_board的连接对话框，可以选择串口或网络连接
    ConnectDialog connectDialog(this);
    connectDialog.exec();
}

void XMLModuleTab::setEditMode()
{
    // 设置编辑模式
    m_isEditMode = !m_isEditMode;

    ui->edit_mode_action->setText(m_isEditMode ? tr("Exit Edit") : tr("Edit Mode"));
    ui->add_module_btn->setVisible(m_isEditMode);
    generateUI();
}

void XMLModuleTab::refreshToDefault()
{
    if (QMessageBox::question(this, tr("Refresh"), 
                              tr("All parameters will be reset to default values. Continue?")) == QMessageBox::Yes) {
        generateUI();
        printLog(QString("Refresh to default values"));
    }
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
        QMessageBox::information(this, tr("Success"), tr("XML imported successfully"));
        printLog(QString("Imported XML: %1").arg(path));
    } else {
        QMessageBox::warning(this, tr("Warning"), tr("Failed to parse XML: %1").arg(path));
        printLog(QString("Failed to parse XML: %1").arg(path));
    }
}
void XMLModuleTab::exportXml()
{
    // 导出XML，路径自动加上.xml
    QString path = QFileDialog::getSaveFileName(
        this,
        tr("Export XML"),
        QDir::homePath(),
        tr("XML Files (*.xml);;All Files (*.*)")
    );
    if (path.isEmpty()) return;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, tr("Warning"), tr("Failed to open XML: %1").arg(path));
        return;
    }
    file.write(serializeXML());
    file.close();
    printLog(QString("Exported XML: %1").arg(path));
    QMessageBox::information(this, tr("Success"), tr("XML exported successfully"));
}
void XMLModuleTab::importConfig()
{
    // 导入配置
    QString path = QFileDialog::getOpenFileName(
        this,
        tr("Import Config"),
        QDir::homePath(),
        tr("Config Files (*.cfg);;All Files (*.*)")
    );
    if (path.isEmpty()) return;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("Warning"), tr("Failed to open Config: %1").arg(path));
        return;
    }
    QByteArray data = file.readAll();
    file.close();
    if (parseConfigFile(data)) {
        QMessageBox::information(this, tr("Success"), tr("Config imported successfully"));
        printLog(QString("Imported Config: %1").arg(path));
    } else {
        QMessageBox::warning(this, tr("Warning"), tr("Failed to parse Config: %1").arg(path));
        printLog(QString("Failed to parse Config: %1").arg(path));
    }
}

void XMLModuleTab::exportConfig()
{
    // 导出配置
    QString path = QFileDialog::getSaveFileName(
        this,
        tr("Export Config"),
        QDir::homePath(),
        tr("Config Files (*.cfg);;All Files (*.*)")
    );
    if (path.isEmpty()) return;
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, tr("Warning"), tr("Failed to open Config: %1").arg(path));
        return;
    }
    QByteArray data = collectConfigFromUI();
    file.write(data);
    file.close();
    printLog(QString("Exported Config: %1").arg(path));
    QMessageBox::information(this, tr("Success"), tr("Config exported successfully"));
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
                    .address = attrs.value("address").toString().trimmed().toUInt(nullptr, 0)
                };
                curModule.params.append(curParam);
            }
        }
        // end
        else if (reader.isEndElement() && tag == "MODULE" && inModule) {
            if (!curModule.moduleName.isEmpty()) {
                m_xmlConfig.modules.append(curModule);
            }
            inModule = false;
        }
    }
    return !reader.hasError();
}

QByteArray XMLModuleTab::serializeXML() const
{
    QByteArray data;
    QXmlStreamWriter writer(&data);
    writer.setAutoFormatting(true);
    writer.setAutoFormattingIndent(4);

    writer.writeStartDocument();
    writer.writeStartElement("REGISTER_CONFIG");

    for (const Module& module : m_xmlConfig.modules) {
        
        if (module.moduleName.isEmpty()) continue;

        writer.writeStartElement("MODULE");
        writer.writeAttribute("id", module.moduleName);

        for (const Param& param : module.params) {
            writer.writeEmptyElement("PARAM");
            writer.writeAttribute("id", param.paramName);
            writer.writeAttribute("min", QString::number(param.min));
            writer.writeAttribute("max", QString::number(param.max));
            writer.writeAttribute("default", QString::number(param.defaultVal));
            writer.writeAttribute("address", QString::asprintf("0x%08X", param.address));
        }

        writer.writeEndElement(); // MODULE
    }

    writer.writeEndElement(); // REGISTER_CONFIG
    writer.writeEndDocument();

    return data;
}
void XMLModuleTab::onModuleTreeDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    if (!item) return;

    QString targetTitle;
    if (item->parent()) {
        targetTitle = item->parent()->text(0);
    } else {
        targetTitle = item->text(0);
    }
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
    printLog(QString("=== 解析结果 ==="));
    printLog(QString("模块总数: %1").arg(m_xmlConfig.modules.size()));
    printLog("");
    
    // 详细打印每个模块
    for (const Module& module : m_xmlConfig.modules) {
        printLog(QString("模块: %1").arg(module.moduleName));
        printLog(QString("  参数数量: %1").arg(module.params.size()));
        
        for (const Param& param : module.params) {
            printLog(QString("    - %1: min=%2, max=%3, default=%4, addr=0x%5")
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
    ui->module_list->setColumnCount(3);

    ui->module_list->setHeaderLabels({"Module", "", ""});
    // 设置按钮列宽度
    ui->module_list->header()->setStretchLastSection(false);
    ui->module_list->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->module_list->header()->setSectionResizeMode(1, QHeaderView::Fixed);
	ui->module_list->header()->setSectionResizeMode(2, QHeaderView::Fixed);
    
    ui->module_list->setColumnWidth(1, 20);
    ui->module_list->setColumnWidth(2, 20);
    for (int i = 0; i < m_xmlConfig.modules.size(); i++) {
        const Module& module = m_xmlConfig.modules[i];
        QTreeWidgetItem* moduleItem = new QTreeWidgetItem(ui->module_list);
        moduleItem->setText(0, module.moduleName);
        
        // edit button + delete button
        if (m_isEditMode) {
            QToolButton* editBtn = new QToolButton(ui->module_list);
            editBtn->setIcon(QIcon::fromTheme("document-properties"));
            editBtn->setStyleSheet(
                "QToolButton {"
                "    background-color: transparent;"
                "    border: none;"
                "    font-size: 16px;"
                "}"
            );
            ui->module_list->setItemWidget(moduleItem, 1, editBtn);
            // delete button
            QToolButton* deleteBtn = new QToolButton(ui->module_list);
            deleteBtn->setIcon(QIcon::fromTheme("edit-delete"));
            deleteBtn->setStyleSheet(
                "QToolButton {"
                "    background-color: transparent;"
                "    border: none;"
                "    font-size: 16px;"
                "}"
            );
            ui->module_list->setItemWidget(moduleItem, 2, deleteBtn);

            // edit module
            //
            connect(editBtn, &QToolButton::clicked, this, [this, i]() {
                Module editedModule = ModuleEditDialog::editModule(m_xmlConfig.modules[i], this);
                if (!editedModule.moduleName.isEmpty()) {
                    m_xmlConfig.modules[i] = editedModule;
                    generateUI();
                }
            });
            connect(deleteBtn, &QToolButton::clicked, this, [this, i]() {
                if (QMessageBox::question(this, "Delete Module", "Are you sure you want to delete this module?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
                    m_xmlConfig.modules.removeAt(i);
                    generateUI();
                }
            });
        }
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
    QGridLayout* paramLayout = new QGridLayout();

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
    mainLayout->addStretch();

    ui->scrollAreaWidgetContents->layout()->addWidget(group);
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
    
    printLog(QString("%1: Read module").arg(group->title()));
    
}
void XMLModuleTab::writeModule(QGroupBox *group)
{
    printLog(QString("%1: Write module").arg(group->title()));
    
}
void XMLModuleTab::addNewModule()
{
    QString moduleName = QInputDialog::getText(this, tr("Add New Module"), tr("Module Name:"), QLineEdit::Normal, "");
    if (moduleName.isEmpty()) {
        return;
    }
    Module newModule{
        .moduleName = moduleName
    };
    m_xmlConfig.modules.append(newModule);

    printLog(QString("Add new module: %1").arg(moduleName));
    // printXMLConfig();
    generateUI();
}

void XMLModuleTab::printLog(const QString &message)
{
    if (ui->echo_text) {
        ui->echo_text->appendPlainText(QString("[%1] %2").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"), message));
    }
}

QByteArray XMLModuleTab::collectConfigFromUI()
{
    QByteArray data;
    QTextStream stream(&data);
    
    QWidget* container = ui->scrollAreaWidgetContents;
    const auto groups = container->findChildren<QGroupBox*>();
    
    for (QGroupBox* group : groups) {
        QString moduleName = group->title();
        stream << "[" << moduleName << "]" << Qt::endl;
        
        const auto spinBoxes = group->findChildren<QSpinBox*>();
        for (QSpinBox* spinBox : spinBoxes) {
            QLabel* label = findLabelForSpinBox(group, spinBox);
            if (!label) continue;
            
            QString paramName = label->text().chopped(1);
            stream << paramName << "=" << spinBox->value() << Qt::endl;
        }
        stream << Qt::endl;
    }
    
    return data;
}
// xml_module_tab.cpp 第520行后添加：
QLabel* XMLModuleTab::findLabelForSpinBox(QGroupBox* group, QSpinBox* spinBox)
{
    QGridLayout* layout = qobject_cast<QGridLayout*>(group->layout()->itemAt(1)->layout());
    if (!layout) return nullptr;
    
    int row, col, rowSpan, colSpan;
    layout->getItemPosition(layout->indexOf(spinBox), &row, &col, &rowSpan, &colSpan);
    
    QLayoutItem* item = layout->itemAtPosition(row, 0);
    return item ? qobject_cast<QLabel*>(item->widget()) : nullptr;
}
bool XMLModuleTab::parseConfigFile(const QByteArray &data)
{
    QTextStream stream(data);
    QString currentModule;
    QStringList missingModules;
    QStringList missingParams;
    
    while (!stream.atEnd()) {
        QString line = stream.readLine().trimmed();
        if (line.isEmpty()) continue;
        
        if (line.startsWith("[") && line.endsWith("]")) {
            currentModule = line.mid(1, line.length() - 2);
        } else if (line.contains("=")) {
            QStringList parts = line.split("=");
            if (parts.size() == 2 && !currentModule.isEmpty()) {
                QString paramName = parts[0].trimmed();
                int value = parts[1].trimmed().toInt();
                
                // 查找对应的SpinBox
                bool found = false;
                QWidget* container = ui->scrollAreaWidgetContents;
                const auto groups = container->findChildren<QGroupBox*>();
                
                // 先检查模块是否存在
                bool moduleExists = false;
                for (QGroupBox* group : groups) {
                    if (group->title() == currentModule) {
                        moduleExists = true;
                        break;
                    }
                }
                
                if (!moduleExists) {
                    if (!missingModules.contains(currentModule)) {
                        missingModules.append(currentModule);
                    }
                    continue;
                }
                
                // 查找参数
                for (QGroupBox* group : groups) {
                    if (group->title() == currentModule) {
                        const auto spinBoxes = group->findChildren<QSpinBox*>();
                        for (QSpinBox* spinBox : spinBoxes) {
                            QLabel* label = findLabelForSpinBox(group, spinBox);
                            if (label && label->text().chopped(1) == paramName) {
                                spinBox->setValue(value);
                                found = true;
                                break;
                            }
                        }
                        break;
                    }
                }
                
                if (!found) {
                    missingParams.append(QString("[%1] %2").arg(currentModule).arg(paramName));
                }
            }
        }
    }
    
    
    // 输出缺失的模块
    if (!missingModules.isEmpty()) {
        printLog(QString("Missing modules: %1").arg(missingModules.join(", ")));
    }
    
    // 输出缺失的参数
    if (!missingParams.isEmpty()) {
        printLog(QString("Missing parameters: %1").arg(missingParams.join(", ")));
    }
    
    return missingModules.isEmpty() && missingParams.isEmpty();
}