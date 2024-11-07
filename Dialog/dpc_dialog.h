#ifndef DPC_DIALOG_H
#define DPC_DIALOG_H

#include <QDialog>

namespace Ui {
class dpc_dialog;
}

class dpc_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit dpc_dialog(QWidget *parent = 0);
    ~dpc_dialog();
    int get_dpc_mode();
    void set_dpc_mode(int value);

private:
    Ui::dpc_dialog *ui;
};

#endif // DPC_DIALOG_H
