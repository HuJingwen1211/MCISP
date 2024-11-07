#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "isp_tunning.h"
#include "rgb_viewer.h"
#include "yuv_viewer.h"
#include "link_board.h"
#include <QPainter>
#include <QActionGroup>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    Theme=new QActionGroup(this);
    Theme->addAction(ui->act_Dark_Theme);
    Theme->addAction(ui->act_Light_Theme);
    setCentralWidget(ui->Tab_MainWindow);
    ui->Tab_MainWindow->setVisible(false);//初始设置不可见
    ui->Tab_MainWindow->clear();//清除所有页面
    ui->Tab_MainWindow->setTabsClosable(true);

    /////连接Theme Action触发信号与主题切换的操作的函数，参数传递为被触发的action指针
    QObject::connect(Theme, SIGNAL(triggered(QAction*)), this,SLOT(Theme_changed(QAction*)));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete Theme;
}

/////设置软件的底板水印
void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    QImage image(":/images/images/ecust.png"); ///读取图像
    QSize imageSize = image.size();
    QSize scaledSize = imageSize.scaled(width(), height(), Qt::KeepAspectRatio);///缩放图像
    QRect drawRect(QPoint(0, 100), scaledSize);  ///划分范围
    painter.setOpacity(0.5);  ///设置透明度
    painter.drawImage(drawRect, image); ///绘制图像
}

//////设置主题
void MainWindow::Theme_changed(QAction *theme_act)
{
    if(theme_act==ui->act_Dark_Theme){
        //qDebug()<<"Dark";
        QFile f(":/qdarkstyle/dark/darkstyle.qss");
        if (!f.exists())   {
            qDebug()<<"Unable to set darkstylesheet, file not found";
        }
        else   {
            f.open(QFile::ReadOnly | QFile::Text);
            QTextStream ts(&f);
            qApp->setStyleSheet(ts.readAll());
        }
        f.close();


    }else if(theme_act==ui->act_Light_Theme){
        //qDebug()<<"Light";
        QFile f(":/qdarkstyle/light/lightstyle.qss");
        if (!f.exists())   {
            qDebug()<<"Unable to set lightstylesheet, file not found";
        }
        else   {
            f.open(QFile::ReadOnly | QFile::Text);
            QTextStream ts(&f);
            qApp->setStyleSheet(ts.readAll());
        }
        f.close();
    }
}

//////点击RAW_Tuning_Tool后新建Tab
void MainWindow::on_act_RAW_Tuning_Tool_triggered()
{
    Tunning_Tab *tab=new Tunning_Tab(this);   ///创建tunning tab
    tab->setAttribute(Qt::WA_DeleteOnClose);
    int cur=ui->Tab_MainWindow->addTab(tab,QString::asprintf("ISP Tuning"));
    ui->Tab_MainWindow->setCurrentIndex(cur);
    ui->Tab_MainWindow->setVisible(true);
}

//////点击Tab中的关闭时调用
void MainWindow::on_Tab_MainWindow_tabCloseRequested(int index)
{
    QWidget *tab=ui->Tab_MainWindow->widget(index);//拿到当前Tab
    ui->Tab_MainWindow->removeTab(index);
    delete tab;                            ////关闭Tab后释放Tab占用的资源
    //tab->close();
}

/////Tab变化时调用函数,当全部关闭后显示水印
void MainWindow::on_Tab_MainWindow_currentChanged(int index)
{
    Q_UNUSED(index);
    bool  en=ui->Tab_MainWindow->count()>0; //再无页面时，actions禁用
    ui->Tab_MainWindow->setVisible(en);
}

///////////////////RGB Viewer Tab////////////////////
void MainWindow::on_act_RGB_Viewer_triggered()
{
    RGB_Viewer *tab=new RGB_Viewer(this);
    tab->setAttribute(Qt::WA_DeleteOnClose);
    int cur=ui->Tab_MainWindow->addTab(tab,QString::asprintf("RGB Viewer"));
    ui->Tab_MainWindow->setCurrentIndex(cur);
    ui->Tab_MainWindow->setVisible(true);
}


////////////////YUV Viewer Tab/////////////////////////
void MainWindow::on_act_YUV_Viewer_triggered()
{
    YUV_Viewer *tab=new YUV_Viewer(this);
    tab->setAttribute(Qt::WA_DeleteOnClose);
    int cur=ui->Tab_MainWindow->addTab(tab,QString::asprintf("YUV Viewer"));
    ui->Tab_MainWindow->setCurrentIndex(cur);
    ui->Tab_MainWindow->setVisible(true);
}

///////////////Link to Board Tab//////////////////
void MainWindow::on_act_linkboard_triggered()
{
    int tabcnt = ui->Tab_MainWindow->count();
    bool is_exist=0;
    for (int i = 0; i < tabcnt; ++i) {                   ///////用于设置该Tab只能有一个
        QWidget *tabWidget = ui->Tab_MainWindow->widget(i);    /////用了以后不能delete因为是指针
        if(link_board *tab=dynamic_cast<link_board*>(tabWidget)){   ///遍历当前Tab判断是否已经存在,如果动态转换成功则表示有该类型
            ui->Tab_MainWindow->setCurrentIndex(i);     /////如果之前创建了就定位到之前的tab
            is_exist=1;
            break;
        }
    }
    if(!is_exist){      ////如果不存在该类型的tab才新建一个
        link_board *tab=new link_board(this);
        tab->setAttribute(Qt::WA_DeleteOnClose);
        int cur=ui->Tab_MainWindow->addTab(tab,QString::asprintf("Link to Board"));
        ui->Tab_MainWindow->setCurrentIndex(cur);
        ui->Tab_MainWindow->setVisible(true);
    }
}
