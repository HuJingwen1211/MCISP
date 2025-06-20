#pragma once

#include <QWidget>
#include "ui_awb_tab.h"
#include <QStandardItemModel>
#include "link_board.h"

#define  REG_WBC_GAIN_1_ADDR   0xA0000010
#define  REG_WBC_GAIN_2_ADDR   0xA0000014

#define  REG_AWB_GAIN_1_ADDR   0xA000002C
#define  REG_AWB_GAIN_2_ADDR   0xA0000030

class AWB_tab : public QWidget
{
	Q_OBJECT

public:
	AWB_tab(QWidget *parent = nullptr);
	~AWB_tab();
    void updateModelFromUI();

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


