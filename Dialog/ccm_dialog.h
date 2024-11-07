#ifndef CCM_DIALOG_H
#define CCM_DIALOG_H

#include <QDialog>

namespace Ui {
class ccm_dialog;
}

class ccm_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit ccm_dialog(QWidget *parent = 0);
    ~ccm_dialog();
    int* get_ccm_coeff();
    int* get_offset_out();
    void set_ccm_coeff(const int coeff[]);
    void set_offset_out(const int offset[]);

private:
    Ui::ccm_dialog *ui;
};

#endif // CCM_DIALOG_H
