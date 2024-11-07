#ifndef LSC_DIALOG_H
#define LSC_DIALOG_H

#include <QDialog>

namespace Ui {
class lsc_dialog;
}

class lsc_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit lsc_dialog(QWidget *parent = 0);
    ~lsc_dialog();
    int* get_R_Gain_Mat();
    int* get_Gr_Gain_Mat();
    int* get_Gb_Gain_Mat();
    int* get_B_Gain_Mat();

    void set_R_Gain_Mat(const int value[]);
    void set_Gr_Gain_Mat(const int value[]);
    void set_Gb_Gain_Mat(const int value[]);
    void set_B_Gain_Mat(const int value[]);

private:
    Ui::lsc_dialog *ui;
};

#endif // LSC_DIALOG_H
