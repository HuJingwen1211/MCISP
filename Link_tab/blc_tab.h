#pragma once

#include <QWidget>
#include "ui_blc_tab.h"

class BLC_tab : public QWidget
{
	Q_OBJECT

public:
	BLC_tab(QWidget *parent = nullptr);
	~BLC_tab();

private:
	Ui::BLC_tabClass ui;
};
