#pragma once

#include <QWidget>
#include "ui_nr_raw_tab.h"
#include "configurable_tab.h"

class NR_RAW_tab :  public ConfigurableTab
{
	Q_OBJECT

public:
	NR_RAW_tab(QWidget *parent = nullptr);
	~NR_RAW_tab();

    QMap<QString, int> getAllParams() const override;
    QString getModuleName() const override { return "NR_RAW"; }
    void setParams(const QMap<QString, int>& params) override;

private:
	Ui::NR_RAW_tabClass ui;
};
