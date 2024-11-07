#ifndef NLM_NR_DIALOG_H
#define NLM_NR_DIALOG_H

#include <QDialog>

namespace Ui {
class nlm_nr_dialog;
}

class nlm_nr_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit nlm_nr_dialog(QWidget *parent = 0);
    ~nlm_nr_dialog();
    int get_nlm_h();
    void set_nlm_h(int value);

private:
    Ui::nlm_nr_dialog *ui;
};

#endif // NLM_NR_DIALOG_H
