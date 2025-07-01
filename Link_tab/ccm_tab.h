#pragma once

#include <QWidget>
#include "ui_ccm_tab.h"
#include <QStandardItemModel>
#include "link_board.h"
#include "configurable_tab.h"


#define REG_CCM_COEFF1_ADDR      0xA0000000 +  (0x18)
#define REG_CCM_COEFF2_ADDR      0xA0000000 +  (0x1C)
#define REG_CCM_COEFF3_ADDR      0xA0000000 +  (0x20)
#define REG_CCM_COEFF4_ADDR      0xA0000000 +  (0x24)
#define REG_CCM_COEFF5_ADDR      0xA0000000 +  (0x28)


class CCM_tab : public ConfigurableTab
{
	Q_OBJECT

public:
	CCM_tab(QWidget *parent = nullptr);
	~CCM_tab();
    void updateModelFromUI();
    QMap<QString, int> getAllParams() const override;
    QString getModuleName() const override { return "CCM"; }
    void setParams(const QMap<QString, int>& params) override;

private slots:
    void on_ccm_write_btn_clicked();
    void updateUIFromModel();

private:
    link_board *link_tab;
    QStandardItemModel *ccm_model;
    QList<QSpinBox*> spinBoxes;
	Ui::CCM_tabClass ui;
};
