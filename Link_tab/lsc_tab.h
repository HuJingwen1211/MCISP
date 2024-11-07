#pragma once

#include <QWidget>
#include "ui_lsc_tab.h"

class LSC_tab : public QWidget
{
	Q_OBJECT

public:
	LSC_tab(QWidget *parent = nullptr);
	~LSC_tab();

private:
	Ui::LSC_tabClass ui;
};
