#pragma once

#include <QWidget>
#include "ui_ccm_tab.h"

class CCM_tab : public QWidget
{
	Q_OBJECT

public:
	CCM_tab(QWidget *parent = nullptr);
	~CCM_tab();

private:
	Ui::CCM_tabClass ui;
};
