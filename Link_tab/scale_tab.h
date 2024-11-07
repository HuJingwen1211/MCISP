#pragma once

#include <QWidget>
#include "ui_scale_tab.h"

class SCALE_tab : public QWidget
{
	Q_OBJECT

public:
	SCALE_tab(QWidget *parent = nullptr);
	~SCALE_tab();

private:
	Ui::SCALE_tabClass ui;
};
