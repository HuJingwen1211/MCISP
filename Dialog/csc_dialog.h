#ifndef CSC_DIALOG_H
#define CSC_DIALOG_H

#include <QDialog>

namespace Ui {
class csc_dialog;
}

class csc_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit csc_dialog(QWidget *parent = 0);
    ~csc_dialog();
    int* get_offsetin();
    int* get_csccoeff();
    int* get_offsetout();

    void set_offsetin(const int offsetin[]);
    void set_csccoeff(const int csccoeff[]);
    void set_offsetout(const int offsetout[]);

private:
    Ui::csc_dialog *ui;
};

#endif // CSC_DIALOG_H
