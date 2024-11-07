#ifndef SHARPEN_DIALOG_H
#define SHARPEN_DIALOG_H

#include <QDialog>

namespace Ui {
class sharpen_dialog;
}

class sharpen_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit sharpen_dialog(QWidget *parent = 0);
    ~sharpen_dialog();
    int get_sharpen_param();
    void set_sharpen_param(int value);

private:
    Ui::sharpen_dialog *ui;
};

#endif // SHARPEN_DIALOG_H
