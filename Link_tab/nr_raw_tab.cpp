#include "nr_raw_tab.h"

NR_RAW_tab::NR_RAW_tab(QWidget *parent)
    : ConfigurableTab(parent)
{
	ui.setupUi(this);
}

NR_RAW_tab::~NR_RAW_tab()
{}


QMap<QString, int> NR_RAW_tab::getAllParams() const
{
    QMap<QString, int> params;
    params["SIGMA"] = ui.nr_raw_sigma->value();
    params["NR_RAW_H"] = ui.nr_raw_H->value();
    return params;
}

void NR_RAW_tab::setParams(const QMap<QString, int>& params)
{
    if (params.contains("SIGMA")) ui.nr_raw_sigma->setValue(params["SIGMA"]);
    if (params.contains("NR_RAW_H")) ui.nr_raw_H->setValue(params["NR_RAW_H"]);
}
