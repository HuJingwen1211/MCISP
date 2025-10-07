#ifndef CONNECT_DIALOG_H
#define CONNECT_DIALOG_H

#include <QObject>
#include <QDialog>

namespace Ui {
class ConnectDialog;
}

class ConnectDialog : public QDialog
{
    // Q_OBJECT
public:
    explicit ConnectDialog(QWidget *parent = nullptr);
    ~ConnectDialog();

private:
    Ui::ConnectDialog *ui;
};

#endif // CONNECT_DIALOG_H
