#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAction>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void paintEvent(QPaintEvent *envent);
    bool openRawFileWithDialog(QString filePath);
    bool openRgbFileWithDialog(QString filePath);
    bool openYuvFileWithDialog(QString filePath);

private slots:
    void Theme_changed(QAction *theme_act);

private slots:
    void on_act_RAW_Tuning_Tool_triggered();
    void on_Tab_MainWindow_tabCloseRequested(int index);
    void on_Tab_MainWindow_currentChanged(int index);
    void on_act_RGB_Viewer_triggered();
    void on_act_YUV_Viewer_triggered();
    void on_act_linkboard_triggered();
    void on_act_About_triggered();
    void on_actionXML_Module_Tool_triggered();

    void on_actionUser_Manual_triggered();

private:
    QActionGroup *Theme;
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
