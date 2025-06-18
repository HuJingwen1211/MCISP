#ifndef CAPTURE_TAB_H
#define CAPTURE_TAB_H

#include <QWidget>
#include "link_board.h"

namespace Ui {
class capture_tab;
}

class capture_tab : public QWidget
{
    Q_OBJECT

public:
    explicit capture_tab(QWidget *parent = nullptr);
    ~capture_tab();

private slots:
    void on_capture_btn_clicked();

// signals:
//     void capture_path_signal(QString fileName);

private:
    link_board *link_tab;
    Ui::capture_tab *ui;
};

#endif // CAPTURE_TAB_H
