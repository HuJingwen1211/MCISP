#pragma once

#include <QWidget>
#include "ui_nr_yuv_tab.h"
#include "link_board.h"
#include <QStandardItemModel>

#include "configurable_tab.h"



#define REG_NR_YUV_0_ADDR   0xA0000000 +  (0x58)
#define REG_NR_YUV_1_ADDR   0xA0000000 +  (0x5C)

class NR_YUV_tab : public ConfigurableTab
{
	Q_OBJECT

public:
	NR_YUV_tab(QWidget *parent = nullptr);
	~NR_YUV_tab();
    void updateModelFromUI();

    QMap<QString, int> getAllParams() const override;
    QString getModuleName() const override { return "NR_YUV"; }
    void setParams(const QMap<QString, int>& params) override;


private slots:
    void on_nryuv_write_btn_clicked();
    void updateUIFromModel();

private:
    QStandardItemModel *nryuv_model;
    QList<QSpinBox*> spinBoxes;
    link_board *link_tab;
	Ui::NR_YUV_tabClass ui;
};
