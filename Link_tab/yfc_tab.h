#pragma once

#include <QWidget>
#include "ui_yfc_tab.h"

class YFC_tab : public QWidget
{
	Q_OBJECT

public:
	YFC_tab(QWidget *parent = nullptr);
	~YFC_tab();

private:
	Ui::YFC_tabClass ui;
};
