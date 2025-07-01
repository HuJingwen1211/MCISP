#pragma once

#include <QWidget>
#include "ui_lsc_tab.h"
#include "configurable_tab.h"

class LSC_tab :  public ConfigurableTab
{
	Q_OBJECT

public:
	LSC_tab(QWidget *parent = nullptr);
	~LSC_tab();
    // 实现基类的虚函数
    QMap<QString, int> getAllParams() const override;
    QString getModuleName() const override { return "LSC"; }
    void setParams(const QMap<QString, int>& params) override;

private:
	Ui::LSC_tabClass ui;
};
