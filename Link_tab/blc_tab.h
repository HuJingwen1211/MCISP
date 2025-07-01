#pragma once

#include <QWidget>
#include "ui_blc_tab.h"
#include "configurable_tab.h"

class BLC_tab : public ConfigurableTab
{
	Q_OBJECT

public:
	BLC_tab(QWidget *parent = nullptr);
	~BLC_tab();

    // 实现基类的虚函数
    QMap<QString, int> getAllParams() const override;
    QString getModuleName() const override { return "BLC"; }
    void setParams(const QMap<QString, int>& params) override;

private:
	Ui::BLC_tabClass ui;
};
