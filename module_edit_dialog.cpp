#include "module_edit_dialog.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QScrollArea>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QToolButton>
#include <QHeaderView>
ModuleEditDialog::ModuleEditDialog(const Module &module, QWidget *parent)
    : QDialog(parent)
    , m_module(module)
{
    setupDialog();
}

ModuleEditDialog::~ModuleEditDialog() {}

Module ModuleEditDialog::getEditedModule() const
{
    return m_module;
}

Module ModuleEditDialog::editModule(const Module &module, QWidget *parent)
{
    // 传入xmlconfig，用m_module做拷贝，编辑完成后，返回编辑后的module
    ModuleEditDialog dialog(module, parent);
    return dialog.exec() == QDialog::Accepted ? dialog.getEditedModule() : Module{};
}

void ModuleEditDialog::addParam()
{
    Param newParam{
        .paramName = "",
        .min = 0,
        .max = 0,
        .defaultVal = 0,
        .address = 0
    };
    m_module.params.append(newParam);
    updateParamTable();

    int newRow = m_module.params.size() - 1;
    m_paramTable->selectRow(newRow);
}



bool ModuleEditDialog::saveModule()
{
    for (int row = 0; row < m_paramTable->rowCount(); ++row) {
        // 先校验这一行
        if (!validateRow(row)) {
            QMessageBox::warning(this, "Save Failed", 
                QString("Row %1 has invalid data").arg(row + 1));
            m_paramTable->setCurrentCell(row, 0);  // 定位到错误行
            return false;  // 校验失败就停止保存
        }
        Param p;
        // 收集数据
        if (QTableWidgetItem* nameItem = m_paramTable->item(row, 0)) {
            p.paramName = nameItem->text().trimmed();
        }
        
        if (QTableWidgetItem* defItem = m_paramTable->item(row, 1)) {
            p.defaultVal = defItem->text().toInt();
        }
        
        if (QTableWidgetItem* minItem = m_paramTable->item(row, 2)) {
            p.min = minItem->text().toInt();
        }
        
        if (QTableWidgetItem* maxItem = m_paramTable->item(row, 3)) {
            p.max = maxItem->text().toInt();
        }
        if (QTableWidgetItem* addrItem = m_paramTable->item(row, 4)) {
            bool ok = false;
            p.address = addrItem->text().toUInt(&ok, 0);
            if (!ok) p.address = 0;
        }
        
        // 写回
        m_module.params[row] = p;
    }
    return true;
}




void ModuleEditDialog::setupDialog()
{
    // 只做一次性初始化（创建表格，设置列，添加按钮，连接信号）
    // 新增模块的时候：点击add module，弹出模块名输入框，输入模块名，点击确定，这时候创建了module对象。
    // 编辑模块的时候：点击模块后面的编辑按钮，弹出模块编辑对话框
    // 标题：EDIT: module name
    // 

    setWindowTitle(QString("Edit: %1").arg(m_module.moduleName));
    QVBoxLayout* lay = new QVBoxLayout(this);
    
    // 内容：表格（参数名|默认值|最小值|最大值|地址） + 操作（编辑|删除）
    m_paramTable = new QTableWidget(this);
    m_paramTable->setColumnCount(6);
    m_paramTable->setHorizontalHeaderLabels({"Parameter", "Default", "Min", "Max", "Address", "Delete"});
    m_paramTable->setRowCount(m_module.params.size());
    

    m_paramTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_paramTable->setSelectionMode(QAbstractItemView::SingleSelection);
	m_paramTable->setEditTriggers(QAbstractItemView::NoEditTriggers);   // 初始只读
	m_paramTable->setAlternatingRowColors(true);

// 填充数据
    updateParamTable();
    lay->addWidget(m_paramTable);


    	// 新增：无滚动条 + 按内容自适应 + 布局固定窗口大小
	m_paramTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_paramTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_paramTable->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
	m_paramTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	m_paramTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	lay->setSizeConstraint(QLayout::SetFixedSize);
	m_paramTable->resizeColumnsToContents();
	m_paramTable->resizeRowsToContents();
	this->adjustSize();

// 按钮
    QHBoxLayout* row1 = new QHBoxLayout();
    QPushButton* addBtn = new QPushButton("Add New Param", this);
    row1->addWidget(addBtn);

    QHBoxLayout* row2 = new QHBoxLayout();
    QPushButton* editBtn = new QPushButton("Edit", this);
    QPushButton* okBtn = new QPushButton("OK", this);
    QPushButton* cancelBtn = new QPushButton("Cancel", this);
    row2->addWidget(editBtn);
    row2->addWidget(okBtn);   
    row2->addWidget(cancelBtn);

    lay->addLayout(row1);
    lay->addLayout(row2);


    //connect
    connect(addBtn, &QPushButton::clicked, this, &ModuleEditDialog::addParam);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    // connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);

    connect(okBtn, &QPushButton::clicked, this, [this]() {
        m_paramTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_paramTable->setFocus();
        if (!saveModule()) return;
        QDialog::accept();
    });

    connect(editBtn, &QPushButton::clicked, this, [this, editBtn]() {
        // 进入编辑状态
        if (!m_isEditing) {
            m_paramTable->setEditTriggers(QAbstractItemView::AllEditTriggers);
            // 按钮变绿
            editBtn->setText("Save");
            editBtn->setStyleSheet("background-color:green;");
            // 禁用delete按钮
            for (int r = 0; r < m_paramTable->rowCount(); r++) {
                if (auto *w = m_paramTable->cellWidget(r, 5)) {
					w->setEnabled(false);
					w->setStyleSheet("");
				}
            }
            m_isEditing = true;
        } else {
            // 保存并退出编辑状态
            if (!saveModule()) return;
            m_paramTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
            editBtn->setText("Edit");
            editBtn->setStyleSheet("");
            // 启用delete按钮，设为深红色
			for (int r = 0; r < m_paramTable->rowCount(); r++) {
				if (auto *w = m_paramTable->cellWidget(r, 5)) {
					w->setEnabled(true);
					w->setStyleSheet("background-color:#d93026;");
				}
			}
            m_isEditing = false;
        }
    });

}

void ModuleEditDialog::updateParamTable()
{
    //只负责填充数据
    if (!m_paramTable) return;
    m_paramTable->setRowCount(m_module.params.size());
    for (int i = 0; i < m_module.params.size(); i++) {
        const Param& param = m_module.params[i];
        m_paramTable->setItem(i, 0, new QTableWidgetItem(param.paramName));
        m_paramTable->setItem(i, 1, new QTableWidgetItem(QString::number(param.defaultVal)));
        m_paramTable->setItem(i, 2, new QTableWidgetItem(QString::number(param.min)));
        m_paramTable->setItem(i, 3, new QTableWidgetItem(QString::number(param.max)));
        m_paramTable->setItem(i, 4, new QTableWidgetItem(QString::asprintf("0x%08X", param.address)));

        // 删除按钮
        if (!m_paramTable->cellWidget(i, 5)) {
            QToolButton* delBtn = new QToolButton(m_paramTable);
            delBtn->setStyleSheet("background-color:#d93026;");
            delBtn->setIcon(QIcon::fromTheme("edit-delete"));
            delBtn->setAutoRaise(true);
            delBtn->setProperty("row", i);
            m_paramTable->setCellWidget(i, 5, delBtn);
            connect(delBtn, &QToolButton::clicked, this, [this, i]() {
                if (QMessageBox::question(this, "Delete Param", "Are you sure you want to delete this param?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
                    m_module.params.removeAt(i);
                    updateParamTable();
                }
            });
        }
    }
    	// 新增：按内容调整尺寸并固定对话框大小
	m_paramTable->resizeColumnsToContents();
	m_paramTable->resizeRowsToContents();
	this->adjustSize();
}

bool ModuleEditDialog::validateRow(int row) const
{
    // 从表格读取当前值，不是从 m_module
    auto getText = [this](int r, int c) -> QString {
        if (auto* item = m_paramTable->item(r, c)) return item->text().trimmed();
        return {};
    };
    
    QString name = getText(row, 0);
    QString defStr = getText(row, 1);
    QString minStr = getText(row, 2);
    QString maxStr = getText(row, 3);
    QString addrStr = getText(row, 4);
    
    // 基本校验
    if (name.isEmpty()) return false;
    
    bool okDef, okMin, okMax, okAddr;
    int def = defStr.toInt(&okDef);
    int min = minStr.toInt(&okMin);
    int max = maxStr.toInt(&okMax);
    int addr = addrStr.toUInt(&okAddr, 0);  // 支持 0x 格式
    
    if (!okDef || !okMin || !okMax || !okAddr) return false;
    if (min > max) return false;
    if (def < min || def > max) return false;
    
    return true;
}