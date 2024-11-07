#pragma once

#include <QWidget>
#include "ui_nr_raw_tab.h"

class NR_RAW_tab : public QWidget
{
	Q_OBJECT

public:
	NR_RAW_tab(QWidget *parent = nullptr);
	~NR_RAW_tab();

private:
	Ui::NR_RAW_tabClass ui;
};
