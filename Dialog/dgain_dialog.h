#ifndef DGAIN_DIALOG_H
#define DGAIN_DIALOG_H

#include <QDialog>

namespace Ui {
class Dgain_Dialog;
}

class Dgain_Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dgain_Dialog(QWidget *parent = 0);
    ~Dgain_Dialog();
    double get_Gain_R();
    double get_Gain_Gr();
    double get_Gain_Gb();
    double get_Gain_B();
    void set_Gain(double last_r,double last_gr,double last_gb,double last_b);////回显上次设置的值


private:
    Ui::Dgain_Dialog *ui;
};

#endif // DGAIN_DIALOG_H
