#ifndef AWB_MANUAL_DIALOG_H
#define AWB_MANUAL_DIALOG_H

#include <QDialog>

namespace Ui {
class awb_manual_dialog;
}

class awb_manual_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit awb_manual_dialog(QWidget *parent = 0);
    ~awb_manual_dialog();
    int get_RGain();
    int get_GrGain();
    int get_GbGain();
    int get_BGain();

    void set_RGain(int value);
    void set_GrGain(int value);
    void set_GbGain(int value);
    void set_BGain(int value);


private:
    Ui::awb_manual_dialog *ui;
};

#endif // AWB_MANUAL_DIALOG_H
