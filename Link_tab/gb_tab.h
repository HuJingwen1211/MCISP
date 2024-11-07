#pragma once

#include <QWidget>
#include "ui_gb_tab.h"

class GB_tab : public QWidget
{
	Q_OBJECT

public:
	GB_tab(QWidget *parent = nullptr);
	~GB_tab();

private:
	Ui::GB_tabClass ui;
};
