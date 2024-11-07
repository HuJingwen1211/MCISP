#ifndef NRYUV_DIALOG_H
#define NRYUV_DIALOG_H

#include <QDialog>

namespace Ui {
class nryuv_dialog;
}

class nryuv_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit nryuv_dialog(QWidget *parent = 0);
    ~nryuv_dialog();
    int get_ysigma2();
    int get_uvsigma2();
    int get_yfilt();
    int get_uvfilt();

    void set_ysigma2(int value);
    void set_uvsigma2(int value);
    void set_yfilt(int value);
    void set_uvfilt(int value);

private:
    Ui::nryuv_dialog *ui;
};

#endif // NRYUV_DIALOG_H
