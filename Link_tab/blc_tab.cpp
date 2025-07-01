#include "blc_tab.h"

BLC_tab::BLC_tab(QWidget *parent)
    : ConfigurableTab(parent)
{
	ui.setupUi(this);
}



BLC_tab::~BLC_tab()
{}


QMap<QString, int> BLC_tab::getAllParams() const
{
    QMap<QString, int> params;
    params["BLC_R"] = ui.BLC_R->value();
    params["BLC_Gr"] = ui.BLC_Gr->value();
    params["BLC_Gb"] = ui.BLC_Gb->value();
    params["BLC_B"] = ui.BLC_B->value();
    return params;
}

void BLC_tab::setParams(const QMap<QString, int>& params)
{
    if (params.contains("BLC_R")) ui.BLC_R->setValue(params["BLC_R"]);
    if (params.contains("BLC_Gr")) ui.BLC_Gr->setValue(params["BLC_Gr"]);
    if (params.contains("BLC_Gb")) ui.BLC_Gb->setValue(params["BLC_Gb"]);
    if (params.contains("BLC_B")) ui.BLC_B->setValue(params["BLC_B"]);
}
