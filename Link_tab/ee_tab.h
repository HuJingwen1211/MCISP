#pragma once

#include <QWidget>
#include "ui_ee_tab.h"

class EE_tab : public QWidget
{
	Q_OBJECT

public:
	EE_tab(QWidget *parent = nullptr);
	~EE_tab();

private:
	Ui::EE_tabClass ui;
};
