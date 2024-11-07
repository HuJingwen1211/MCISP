#ifndef GAMMA_DIALOG_H
#define GAMMA_DIALOG_H

#include <QDialog>

namespace Ui {
class gamma_dialog;
}

class gamma_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit gamma_dialog(QWidget *parent = 0);
    ~gamma_dialog();
    double get_gamma();
    void set_gamma(double value);

private:
    Ui::gamma_dialog *ui;
};

#endif // GAMMA_DIALOG_H
