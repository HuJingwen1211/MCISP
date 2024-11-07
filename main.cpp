#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QStyle>
#include <QPushButton>
#include <QVBoxLayout>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    QFont menuFont("Times New Roman", 12, QFont::Normal); // 创建字体对象，设置字体类型、大小和加粗属性
    w.setFont(menuFont); // 设置主窗口标题字体
//    QIcon icon(":/images/images/app.icon");
//    QSize iconSize(80, 80);
//    icon = icon.pixmap(iconSize);
//    a.setWindowIcon(icon);
//    w.setWindowIcon(icon);
    //w.setWindowTitle("ECUST");

    //////////////////////设置软件初始主题为暗黑///////////////////////////////////////
    QFile f(":/qdarkstyle/dark/darkstyle.qss");
    if (!f.exists())   {
        qDebug()<<"Unable to set stylesheet, file not found";
        //printf("Unable to set stylesheet, file not found\n");
    }
    else   {
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        qApp->setStyleSheet(ts.readAll());
    }
    f.close();
    ///////////////////////////////////////////////////////////////////////
    w.show();
    return a.exec();
}
