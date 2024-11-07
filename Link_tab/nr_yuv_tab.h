#pragma once

#include <QWidget>
#include "ui_nr_yuv_tab.h"

class NR_YUV_tab : public QWidget
{
	Q_OBJECT

public:
	NR_YUV_tab(QWidget *parent = nullptr);
	~NR_YUV_tab();

private:
	Ui::NR_YUV_tabClass ui;
};
