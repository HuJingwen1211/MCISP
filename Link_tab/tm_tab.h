#pragma once

#include <QWidget>
#include "ui_tm_tab.h"

class TM_tab : public QWidget
{
	Q_OBJECT

public:
	TM_tab(QWidget *parent = nullptr);
	~TM_tab();

private:
	Ui::TM_tabClass ui;
};
