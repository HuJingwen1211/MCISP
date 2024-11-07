#pragma once

#include <QWidget>
#include "ui_dms_tab.h"

class DMS_tab : public QWidget
{
	Q_OBJECT

public:
	DMS_tab(QWidget *parent = nullptr);
	~DMS_tab();

private:
	Ui::DMS_tabClass ui;
};
