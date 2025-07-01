#pragma once

#include <QWidget>
#include "ui_awb_tab.h"
#include <QStandardItemModel>
#include "link_board.h"
#include "configurable_tab.h"


#define  REG_WBC_GAIN_1_ADDR   0xA0000010
#define  REG_WBC_GAIN_2_ADDR   0xA0000014

#define  REG_AWB_GAIN_1_ADDR   0xA000002C
#define  REG_AWB_GAIN_2_ADDR   0xA0000030

class AWB_tab : public ConfigurableTab
{
	Q_OBJECT

public:
	AWB_tab(QWidget *parent = nullptr);
	~AWB_tab();
    void updateModelFromUI();
    // 实现基类的虚函数
    QMap<QString, int> getAllParams() const override;
    QString getModuleName() const override { return "AWB"; }
    void setParams(const QMap<QString, int>& params) override;

private slots:
    void on_awb_read_btn_clicked();
    void updateUIFromModel();
    void on_awb_write_btn_clicked();
    void read_reg_process(const QByteArray &regData);

private:
    QStandardItemModel *awbc_model;
    QList<QSpinBox*> spinBoxes;
    link_board *link_tab;
	Ui::AWB_tabClass ui;
};


