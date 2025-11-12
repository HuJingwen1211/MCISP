#include "xml_module_tab.h"
#include "ui_xml_module_tab.h"
#include "module_edit_dialog.h"
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
#include <cstring>
XMLModuleTab::XMLModuleTab(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::XMLModuleTab)
    , m_commMgr(new CommManager(this))  // 初始化 CommManager
    , m_connected(false)
{
    ui->setupUi(this);
    initializeUI();
    
    // 连接工具栏动作
    connect(ui->edit_mode_action,     &QAction::triggered, this, &XMLModuleTab::setEditMode);
    connect(ui->refresh_action,       &QAction::triggered, this, &XMLModuleTab::refreshToDefault);
    connect(ui->import_xml_action,    &QAction::triggered, this, &XMLModuleTab::importXML);
    connect(ui->export_xml_action,    &QAction::triggered, this, &XMLModuleTab::exportXml);
    connect(ui->import_config_action, &QAction::triggered, this, &XMLModuleTab::importConfig);
    connect(ui->export_config_action, &QAction::triggered, this, &XMLModuleTab::exportConfig);
    connect(ui->all_read_action,      &QAction::triggered, this, &XMLModuleTab::allRead);
    connect(ui->all_write_action,     &QAction::triggered, this, &XMLModuleTab::allWrite);

    // 连接模块树和按钮
    connect(ui->module_list,          &QTreeWidget::itemDoubleClicked, this, &XMLModuleTab::onModuleTreeDoubleClicked);
    connect(ui->add_module_btn,       &QToolButton::clicked, this, &XMLModuleTab::addNewModule);

    // 连接 Connect 区的控件
    connect(ui->link_btn, &QPushButton::clicked, this, &XMLModuleTab::onLinkBtnClicked);
    connect(ui->clear_btn, &QPushButton::clicked, [this]() {
        ui->echo_text->clear();
    });
    connect(ui->serial_radio, &QRadioButton::toggled, [this](bool checked) {
        if (checked) ui->config_stack->setCurrentIndex(0);
    });
    connect(ui->network_radio, &QRadioButton::toggled, [this](bool checked) {
        if (checked) ui->config_stack->setCurrentIndex(1);
    });
    
    // 连接 CommManager 日志信号
    connect(m_commMgr, &CommManager::logMessage, this, &XMLModuleTab::printLog);
    
    // 连接 CommManager 网络专用信号（只用于异步网络连接）
    connect(m_commMgr, &CommManager::networkConnected, this, &XMLModuleTab::onNetworkConnected);
    connect(m_commMgr, &CommManager::networkDisconnected, this, &XMLModuleTab::onNetworkDisconnected);

    // 模块读回包
    connect(m_commMgr, &CommManager::moduleReadReply, this, &XMLModuleTab::onModuleReadReply);
}

XMLModuleTab::~XMLModuleTab()
{
    delete ui;
}

// ============================================================================
// UI 初始化
// ============================================================================

void XMLModuleTab::initializeUI()
{
    // 隐藏编辑模式按钮
    ui->add_module_btn->setVisible(false);
    
    // 设置左右分隔条初始比例（左侧面板 200px，右侧内容区 800px）
    ui->splitter_2->setSizes({200, 800});
    
    // 设置左侧上下分隔条初始比例（OPTION 区 400px，CONNECT 区 180px）
    ui->left_splitter->setSizes({400, 180});

    // 填充串口列表
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        ui->port_combx->addItem(info.portName());
    }
    
    // 设置默认值
    ui->serial_radio->setChecked(true);
    ui->config_stack->setCurrentIndex(0);
}

// ============================================================================
// 工具栏动作处理
// ============================================================================

void XMLModuleTab::setEditMode()
{
    m_isEditMode = !m_isEditMode;
    ui->edit_mode_action->setText(m_isEditMode ? tr("Exit Edit") : tr("Edit Mode"));
    ui->add_module_btn->setVisible(m_isEditMode);

    ui->connect_groupBox->setVisible(!m_isEditMode);
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
    if (!m_commMgr->isOpen()) {
        printLog("未连接，无法执行 All Read");
        return;
    }

    for (const Module &module : m_xmlConfig.modules) {
        QGroupBox *group = m_groupById.value(module.moduleId, nullptr);
        if (group) {
            readModule(group);
        }
    }
}

void XMLModuleTab::allWrite()
{
    if (!m_commMgr->isOpen()) {
        printLog("未连接，无法执行 All Write");
        return;
    }

    for (const Module &module : m_xmlConfig.modules) {
        QGroupBox *group = m_groupById.value(module.moduleId, nullptr);
        if (group) {
            writeModule(group);
        }
    }
}

// ============================================================================
// XML 解析与序列化
// ============================================================================

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
                const auto attrs = reader.attributes();
                curModule.moduleName = attrs.value("id").toString().trimmed();
            
                bool ok = false;
                const QString idAttr = attrs.value("module_id").toString().trimmed();
                curModule.moduleId = static_cast<quint8>(idAttr.toUInt(&ok, 0));
                if (!ok) {
                    printLog(QString("Module %1 missing valid module_id").arg(curModule.moduleName));
                    return false;
                }
            
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
    if (reader.hasError()) {
        printLog(QString("XML parse error: %1").arg(reader.errorString()));
        return false;
    }

    // 直接在这里填映射表
    m_moduleNameMap.clear();
    m_moduleIdMap.clear();

    for (Module &module : m_xmlConfig.modules) {
        if (module.moduleId == 0xFF) continue;
        m_moduleNameMap.insert(module.moduleName, &module);
        m_moduleIdMap.insert(module.moduleId, &module);
    }

    return true;
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
        writer.writeAttribute("module_id", QString::asprintf("0x%02X", module.moduleId));

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

// ============================================================================
// UI 生成与管理
// ============================================================================

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
    group->setProperty("module_id", module.moduleId);
    m_groupById.insert(module.moduleId, group);

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
        QLabel* label = new QLabel(param.paramName + "      (0x" + QString::asprintf("%08X", param.address) + "):", group);
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
    m_groupById.clear();
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

// ============================================================================
// 模块读写操作
// ============================================================================

void XMLModuleTab::readModule(QGroupBox *group)
{
    if (!m_commMgr->isOpen()) {
        printLog(QString("未连接，无法读取 %1").arg(group->title()));
        return;
    }

    const quint8 moduleId = static_cast<quint8>(group->property("module_id").toUInt());
    const auto spinBoxes = group->findChildren<QSpinBox*>();

    QByteArray payload;
    payload.append(static_cast<char>(moduleId));
    for (QSpinBox *spin : spinBoxes) {
        const quint32 address = spin->property("address").toUInt();
        payload.append(reinterpret_cast<const char*>(&address), sizeof(address));
    }

    m_commMgr->sendCmd(READ_REG_CMD,
                       reinterpret_cast<const uint8_t*>(payload.constData()),
                       static_cast<uint16_t>(payload.size()));
}
void XMLModuleTab::writeModule(QGroupBox *group)
{
    if (!m_commMgr->isOpen()) {
        printLog(QString("未连接，无法写入 %1").arg(group->title()));
        return;
    }

    const quint8 moduleId = static_cast<quint8>(group->property("module_id").toUInt());
    const auto spinBoxes = group->findChildren<QSpinBox*>();
    
    // 构造 payload: [moduleId][addr1][val1][addr2][val2]...
    QByteArray payload;
    payload.append(static_cast<char>(moduleId));

    for (QSpinBox *spin : spinBoxes) {
        const quint32 address = spin->property("address").toUInt();
        const quint32 value = static_cast<quint32>(spin->value());
        payload.append(reinterpret_cast<const char*>(&address), sizeof(address));
        payload.append(reinterpret_cast<const char*>(&value), sizeof(value));
    }

    m_commMgr->sendCmd(WRITE_REG_CMD, 
                       reinterpret_cast<const uint8_t*>(payload.constData()), 
                       static_cast<uint16_t>(payload.size()));
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

// ============================================================================
// 配置文件操作
// ============================================================================

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

// ============================================================================
// 通信连接操作
// ============================================================================
void XMLModuleTab::onLinkBtnClicked()
{
    if (ui->link_btn->text() == "Connect") {
        if (ui->serial_radio->isChecked()) {
            handleSerialConnect();
        } else {
            handleNetworkConnect();
        }
    } else if (ui->link_btn->text() == "Disconnect") {
        handleDisconnect();
    }
}

void XMLModuleTab::handleSerialConnect()
{
    const QString port = ui->port_combx->currentText();
    const int baud = ui->baud_combx->currentText().toInt();
    
    if (port.isEmpty() || port == "<None>") {
        QMessageBox::warning(this, "警告", "请选择串口");
        return;
    }
    
    if (m_commMgr->openSerial(port, baud)) {
        m_commMgr->writeRaw(QByteArray("HELLO_FROM_APP\r\n"));
        
        m_connected = true;
        ui->link_btn->setText("Disconnect");
        ui->link_btn->setStyleSheet("background-color: #38815c;");
        ui->port_combx->setEnabled(false);
        ui->baud_combx->setEnabled(false);
        ui->serial_radio->setEnabled(false);
        ui->network_radio->setEnabled(false);
    } else {
        QMessageBox::critical(this, "错误", "串口打开失败");
    }
}

void XMLModuleTab::handleNetworkConnect()
{
    const QString ip = ui->ip_lineEdit->text();
    const int port = ui->tcp_port_spinBox->value();
    
    if (ip.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入 IP 地址");
        return;
    }
    
    m_commMgr->openNetwork(ip, port);
    printLog(QString("正在连接 %1:%2...").arg(ip).arg(port));
}

void XMLModuleTab::handleDisconnect()
{
    if (ui->serial_radio->isChecked()) {
        // 串口：同步断开，直接处理
        m_commMgr->close();
        
        m_connected = false;
        printLog("串口断开");
        ui->link_btn->setText("Connect");
        ui->link_btn->setStyleSheet("");
        ui->port_combx->setEnabled(true);
        ui->baud_combx->setEnabled(true);
        ui->serial_radio->setEnabled(true);
        ui->network_radio->setEnabled(true);
    } else {
        // 网络：异步断开，依赖 networkDisconnected 信号
        m_commMgr->close();
    }
}

void XMLModuleTab::onNetworkConnected()
{
    // 网络异步连接成功
    m_connected = true;
    
    ui->link_btn->setText("Disconnect");
    ui->link_btn->setStyleSheet("background-color: #38815c;");
    ui->ip_lineEdit->setEnabled(false);
    ui->tcp_port_spinBox->setEnabled(false);
    ui->serial_radio->setEnabled(false);
    ui->network_radio->setEnabled(false);
}

void XMLModuleTab::onNetworkDisconnected()
{
    // 网络意外断开（或主动断开）
    m_connected = false;
    
    ui->link_btn->setText("Connect");
    ui->link_btn->setStyleSheet("");
    ui->ip_lineEdit->setEnabled(true);
    ui->tcp_port_spinBox->setEnabled(true);
    ui->serial_radio->setEnabled(true);
    ui->network_radio->setEnabled(true);
}

void XMLModuleTab::onModuleReadReply(quint8 moduleId, const QByteArray &payload)
{
    QGroupBox *group = m_groupById.value(moduleId, nullptr);
    if (!group) {
        return;
    }

    const auto spinBoxes = group->findChildren<QSpinBox*>();
    const char *ptr = payload.constData();
    for (QSpinBox *spin : spinBoxes) {
        quint32 value;
        memcpy(&value, ptr, sizeof(value));
        ptr += sizeof(value);
        spin->setValue(static_cast<int>(value));
    }
}