#ifndef BLC_DIALOG_H
#define BLC_DIALOG_H

#include <QDialog>

namespace Ui {
class blc_dialog;
}

class blc_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit blc_dialog(QWidget *parent = 0);
    ~blc_dialog();
    int* get_blc_paramter();
    void set_blc_paramter(const int value[]);

private:
    Ui::blc_dialog *ui;
};

#endif // BLC_DIALOG_H
