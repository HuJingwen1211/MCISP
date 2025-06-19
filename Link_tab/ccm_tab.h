#pragma once

#include <QWidget>
#include "ui_ccm_tab.h"
#include <QDataWidgetMapper>
#include <QStandardItemModel>

class CCM_tab : public QWidget
{
	Q_OBJECT

public:
	CCM_tab(QWidget *parent = nullptr);
	~CCM_tab();
    void updateModelFromUI();

private slots:
    void on_ccm_write_btn_clicked();
    void updateUIFromModel();

private:
    QStandardItemModel *ccm_model;
    QList<QSpinBox*> spinBoxes;
	Ui::CCM_tabClass ui;
};
