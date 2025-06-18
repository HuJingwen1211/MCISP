#pragma once

#include <QWidget>
#include "ui_awb_tab.h"

class AWB_tab : public QWidget
{
	Q_OBJECT

public:
	AWB_tab(QWidget *parent = nullptr);
	~AWB_tab();

private:
	Ui::AWB_tabClass ui;
};


