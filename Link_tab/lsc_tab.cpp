#include "lsc_tab.h"

LSC_tab::LSC_tab(QWidget *parent)
    : ConfigurableTab(parent)
{
	ui.setupUi(this);
}

LSC_tab::~LSC_tab()
{
}


QMap<QString, int> LSC_tab::getAllParams() const
{
    QMap<QString, int> params;
    params["LSC_R"] = ui.LSC_R->toPlainText().toInt();
    params["LSC_Gr"] = ui.LSC_Gr->toPlainText().toInt();
    params["LSC_Gb"] = ui.LSC_Gb->toPlainText().toInt();
    params["LSC_B"] = ui.LSC_B->toPlainText().toInt();
    return params;
}

void LSC_tab::setParams(const QMap<QString, int>& params)
{
    if (params.contains("LSC_R")) ui.LSC_R->setPlainText(QString::number(params["LSC_R"]));
    if (params.contains("LSC_Gr")) ui.LSC_Gr->setPlainText(QString::number(params["LSC_Gr"]));
    if (params.contains("LSC_Gb")) ui.LSC_Gb->setPlainText(QString::number(params["LSC_Gb"]));
    if (params.contains("LSC_B")) ui.LSC_B->setPlainText(QString::number(params["LSC_B"]));
}
