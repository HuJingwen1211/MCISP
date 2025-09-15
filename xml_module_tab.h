#ifndef XML_MODULE_TAB_H
#define XML_MODULE_TAB_H

#include <QWidget>

namespace Ui {
class XMLModuleTab;
}

class XMLModuleTab : public QWidget
{
    Q_OBJECT

public:
    explicit XMLModuleTab(QWidget *parent = nullptr);
    ~XMLModuleTab();

private:
    Ui::XMLModuleTab *ui;
};

#endif // XML_MODULE_TAB_H
