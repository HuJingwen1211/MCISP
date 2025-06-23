#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QStyle>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFileInfo>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    QFont menuFont("Times New Roman", 12, QFont::Normal); // 创建字体对象，设置字体类型、大小和加粗属性
    w.setFont(menuFont); // 设置主窗口标题字体

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
    // 检查是否有文件路径参数
    QString filePath;
    if (a.arguments().count() > 1) {
        filePath = a.arguments().at(1);
    }
    qDebug()<<filePath;

    // 如果通过双击文件启动，弹出格式选择对话框
    if (!filePath.isEmpty()) {
        // 确保窗口已显示
        // w.openRawFileWithDialog(filePath);

        QString suffix = QFileInfo(filePath).suffix().toLower(); //获取后缀名
        if(suffix == "raw"){
            if(!w.openRawFileWithDialog(filePath)){
                a.quit();
                return 0;
            }

        }else if(suffix == "rgb"){
            if(!w.openRgbFileWithDialog(filePath)){
                a.quit();
                return 0;
            }
        }else if(suffix == "yuv"){
            if(!w.openYuvFileWithDialog(filePath)){
                a.quit();
                return 0;
            }
        }else{
            QMessageBox::information(nullptr,"INFO","Only support to open .raw/.rgb/.yuv image !!!");
            a.quit();
            return 0;
           //message 该软件只能打开 raw /rgb /yuv文件
        }
    }
    ///////////////////////////////////////////////////////////////////////
    w.show();
    return a.exec();
}
