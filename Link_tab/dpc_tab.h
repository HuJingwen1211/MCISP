#ifndef DPC_TAB_H
#define DPC_TAB_H

#include <QMainWindow>

namespace Ui {
class DPC_tab;
}

class DPC_tab : public QMainWindow
{
    Q_OBJECT

public:
    explicit DPC_tab(QWidget *parent = 0);
    ~DPC_tab();

private:
    Ui::DPC_tab *ui;
};

#endif // DPC_TAB_H
