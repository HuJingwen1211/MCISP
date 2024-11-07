#pragma once

#include <QWidget>
#include "ui_gamma_tab.h"

class GAMMA_tab : public QWidget
{
	Q_OBJECT

public:
	GAMMA_tab(QWidget *parent = nullptr);
	~GAMMA_tab();

private:
	Ui::GAMMA_tabClass ui;
};
