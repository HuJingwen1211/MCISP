#pragma once

#include <QWidget>
#include "ui_csc_tab.h"

class CSC_tab : public QWidget
{
	Q_OBJECT

public:
	CSC_tab(QWidget *parent = nullptr);
	~CSC_tab();

private:
	Ui::CSC_tabClass ui;
};
