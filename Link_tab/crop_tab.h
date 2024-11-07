#pragma once

#include <QWidget>
#include "ui_crop_tab.h"

class CROP_tab : public QWidget
{
	Q_OBJECT

public:
	CROP_tab(QWidget *parent = nullptr);
	~CROP_tab();

private:
	Ui::CROP_tabClass ui;
};
