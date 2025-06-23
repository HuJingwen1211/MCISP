#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "isp_tunning.h"
#include "rgb_viewer.h"
#include "yuv_viewer.h"
#include "link_board.h"
#include <QPainter>
#include <QActionGroup>
#include <QLabel>
#include <QSpinBox>
#include <QFormLayout>
#include <QPushButton>
#include <QDesktopServices>
#include <QTextBrowser>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(tr("MCISP")); // 可被翻译
    setStyleSheet(
        "QWidget {"
        "   font-size: 14px;"  // 可能影响标题栏字体（取决于系统）
        "}"
    );

    Theme=new QActionGroup(this);
    Theme->addAction(ui->act_Dark_Theme);
    Theme->addAction(ui->act_Light_Theme);
    setCentralWidget(ui->Tab_MainWindow);
    ui->Tab_MainWindow->setVisible(false);//初始设置不可见
    ui->Tab_MainWindow->clear();//清除所有页面
    ui->Tab_MainWindow->setTabsClosable(true);

    /////连接Theme Action触发信号与主题切换的操作的函数，参数传递为被触发的action指针
    QObject::connect(Theme, SIGNAL(triggered(QAction*)), this,SLOT(Theme_changed(QAction*)));
    ui->Tab_MainWindow->setStyleSheet(
        "QTabBar::tab {"
        "   font-weight: bold;"
        "   font-size: 14px;"  // 设置字号为 14px
        "   padding: 4px;"     // 可选：调整内边距
        "}"
    );
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
    // QImage image(":/images/images/ecust.png"); ///读取图像
    QImage image(":/images/images/mc.png"); ///读取图像
    // QSize imageSize = image.size();
    // QSize scaledSize = imageSize.scaled(width(), height(), Qt::KeepAspectRatio);///缩放图像
    // QRect drawRect(QPoint(0, 100), scaledSize);  ///划分范围
    // painter.setOpacity(0.5);  ///设置透明度
    // painter.drawImage(drawRect, image); ///绘制图像
    // 计算缩放后的尺寸（保持宽高比）
    QSize scaledSize = image.size().scaled(width(), height(), Qt::KeepAspectRatio);

    // 计算居中位置
    int x = (width() - scaledSize.width()) / 2;
    int y = (height() - scaledSize.height()) / 2;

    // 创建居中矩形
    QRect drawRect(QPoint(x, y), scaledSize);

    painter.setOpacity(0.5);  ///设置透明度
    painter.drawImage(drawRect, image); ///绘制图像
}

bool MainWindow::openRawFileWithDialog(QString filePath)
{
    // 创建格式选择对话框
    QDialog dialog(this);
    dialog.setWindowTitle("选择 RAW 参数");
    dialog.setMinimumWidth(300);

    QList<QPair<QString, QSize>> sizePresets ={
        {"<Custom Size>",QSize(-1,-1)},
        {"VGA (640x480)",QSize(640,480)},
        {"WVGA (832x480)",QSize(832,480)},
        {"ITU (720x576)",QSize(720,576)},
        {"720P (1280x720)",QSize(1280,720)},
        {"1080P (1920x1080)",QSize(1920,1080)},
        {"4K (3840x2160)",QSize(3840,2160)},
        {"XGA+ (1280x960)",QSize(1280,960)}
    };
    QComboBox sizeCombo;
    for (auto it = sizePresets.begin(); it != sizePresets.end(); ++it) {
        sizeCombo.addItem(it->first);
    }
    QSpinBox widthSpin, heightSpin;
    sizeCombo.setCurrentIndex(0);
    widthSpin.setEnabled(true);
    heightSpin.setEnabled(true);
    widthSpin.setRange(0, 10000);
    heightSpin.setRange(0, 10000);
    widthSpin.setValue(0);
    heightSpin.setValue(0);
    // 初始设置
    auto updateSizeControls = [&](int index) {
        if(index == 0){
            widthSpin.setEnabled(true);
            heightSpin.setEnabled(true);
        }else{
            widthSpin.setEnabled(false);
            heightSpin.setEnabled(false);
            QSize size = sizePresets[index].second;
            widthSpin.setValue(size.width());
            heightSpin.setValue(size.height());
        }
    };
    // 尺寸选择变化时的处理
    connect(&sizeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int index) {
        updateSizeControls(index);
    });
    // 创建控件
    QComboBox bayerFormatCombo;
    bayerFormatCombo.addItems({"RGGB", "GRBG", "GBRG", "BGGR"});
    QPushButton btnOpen("OPEN", &dialog), btnCancel("Cancel",&dialog);
    btnOpen.setMinimumSize(200, 30);
    btnCancel.setMinimumSize(80, 30);

    QSpinBox sensorbitSpin;
    sensorbitSpin.setRange(8, 16);    /// 8 - 16 bit
    sensorbitSpin.setValue(12);
    // 布局
    QFormLayout layout(&dialog);
    layout.addRow("Image Size:", &sizeCombo);
    layout.addRow("Imgae Width:", &widthSpin);
    layout.addRow("Image Height:", &heightSpin);
    layout.addRow("Image SensorBits:",&sensorbitSpin);
    layout.addRow("Bayer Pattern:", &bayerFormatCombo);
    layout.addRow(&btnOpen,&btnCancel);
    // 连接按钮信号
    connect(&btnOpen, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(&btnCancel, &QPushButton::clicked, &dialog, &QDialog::reject);

    if(dialog.exec()!=QDialog::Accepted){
        return false;
    }
    Tunning_Tab *tab=new Tunning_Tab(this);   ///创建tunning tab
    tab->setAttribute(Qt::WA_DeleteOnClose);
    int cur=ui->Tab_MainWindow->addTab(tab,QString::asprintf("ISP Tuning"));
    ui->Tab_MainWindow->setCurrentIndex(cur);
    ui->Tab_MainWindow->setVisible(true);

    int width  = widthSpin.value();
    int height = heightSpin.value();
    int sensorbits = sensorbitSpin.value();
    int bayerpattern = bayerFormatCombo.currentIndex();

    int ret = tab->open_with_click_init(filePath,width, height,sensorbits, bayerpattern);
    if(ret!=0){
        return false;
    }
    return true;
}

bool MainWindow::openRgbFileWithDialog(QString filePath)
{
    // 创建格式选择对话框
    QDialog dialog(this);
    dialog.setWindowTitle("选择 RGB 参数");
    dialog.setMinimumWidth(300);

    QList<QPair<QString, QSize>> sizePresets ={
        {"<Custom Size>",QSize(-1,-1)},
        {"VGA (640x480)",QSize(640,480)},
        {"WVGA (832x480)",QSize(832,480)},
        {"ITU (720x576)",QSize(720,576)},
        {"720P (1280x720)",QSize(1280,720)},
        {"1080P (1920x1080)",QSize(1920,1080)},
        {"4K (3840x2160)",QSize(3840,2160)},
        {"XGA+ (1280x960)",QSize(1280,960)}
    };
    QComboBox sizeCombo;
    for (auto it = sizePresets.begin(); it != sizePresets.end(); ++it) {
        sizeCombo.addItem(it->first);
    }
    QSpinBox widthSpin, heightSpin;
    sizeCombo.setCurrentIndex(0);
    widthSpin.setEnabled(true);
    heightSpin.setEnabled(true);
    widthSpin.setRange(0, 10000);
    heightSpin.setRange(0, 10000);
    widthSpin.setValue(0);
    heightSpin.setValue(0);
    // 初始设置
    auto updateSizeControls = [&](int index) {
        if(index == 0){
            widthSpin.setEnabled(true);
            heightSpin.setEnabled(true);
        }else{
            widthSpin.setEnabled(false);
            heightSpin.setEnabled(false);
            QSize size = sizePresets[index].second;
            widthSpin.setValue(size.width());
            heightSpin.setValue(size.height());
        }
    };
    // 尺寸选择变化时的处理
    connect(&sizeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int index) {
        updateSizeControls(index);
    });
    QPushButton btnOpen("OPEN", &dialog), btnCancel("Cancel",&dialog);
    btnOpen.setMinimumSize(200, 30);
    btnCancel.setMinimumSize(80, 30);

    QSpinBox sensorbitSpin;
    sensorbitSpin.setRange(8, 16);    /// 8 - 16 bit
    sensorbitSpin.setValue(12);
    // 布局
    QFormLayout layout(&dialog);
    layout.addRow("Image Size:", &sizeCombo);
    layout.addRow("Imgae Width:", &widthSpin);
    layout.addRow("Image Height:", &heightSpin);
    layout.addRow("Image SensorBits:",&sensorbitSpin);
    layout.addRow(&btnOpen,&btnCancel);
    // 连接按钮信号
    connect(&btnOpen, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(&btnCancel, &QPushButton::clicked, &dialog, &QDialog::reject);

    if(dialog.exec()!=QDialog::Accepted){
        return false;
    }

    RGB_Viewer *tab=new RGB_Viewer(this);
    tab->setAttribute(Qt::WA_DeleteOnClose);
    int cur=ui->Tab_MainWindow->addTab(tab,QString::asprintf("RGB Viewer"));
    ui->Tab_MainWindow->setCurrentIndex(cur);
    ui->Tab_MainWindow->setVisible(true);

    int width  = widthSpin.value();
    int height = heightSpin.value();
    int sensorbits = sensorbitSpin.value();
    int ret = tab->open_with_click_init(filePath,width,height,sensorbits);
    if(ret != 0){
        return false;
    }
    return true;
}

bool MainWindow::openYuvFileWithDialog(QString filePath)
{
    // 创建格式选择对话框
    QDialog dialog(this);
    dialog.setWindowTitle("选择 YUV 参数");
    dialog.setMinimumWidth(300);

    QList<QPair<QString, QSize>> sizePresets ={
        {"<Custom Size>",QSize(-1,-1)},
        {"VGA (640x480)",QSize(640,480)},
        {"WVGA (832x480)",QSize(832,480)},
        {"ITU (720x576)",QSize(720,576)},
        {"720P (1280x720)",QSize(1280,720)},
        {"1080P (1920x1080)",QSize(1920,1080)},
        {"4K (3840x2160)",QSize(3840,2160)},
        {"XGA+ (1280x960)",QSize(1280,960)}
    };
    QComboBox sizeCombo;
    for (auto it = sizePresets.begin(); it != sizePresets.end(); ++it) {
        sizeCombo.addItem(it->first);
    }
    QSpinBox widthSpin, heightSpin;
    sizeCombo.setCurrentIndex(0);
    widthSpin.setEnabled(true);
    heightSpin.setEnabled(true);
    widthSpin.setRange(0, 10000);
    heightSpin.setRange(0, 10000);
    widthSpin.setValue(0);
    heightSpin.setValue(0);
    // 初始设置
    auto updateSizeControls = [&](int index) {
        if(index == 0){
            widthSpin.setEnabled(true);
            heightSpin.setEnabled(true);
        }else{
            widthSpin.setEnabled(false);
            heightSpin.setEnabled(false);
            QSize size = sizePresets[index].second;
            widthSpin.setValue(size.width());
            heightSpin.setValue(size.height());
        }
    };
    // 尺寸选择变化时的处理
    connect(&sizeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int index) {
        updateSizeControls(index);
    });
    QComboBox comboBoxYUVFormat, comboBoxPackFormat;
    comboBoxYUVFormat.setMinimumSize(200, 30);  // 宽度 200px，高度 30px
    comboBoxPackFormat.setMinimumSize(200, 30);
    comboBoxYUVFormat.addItems({"YUV444", "YUV422", "YUV420"});
    comboBoxPackFormat.addItems({"PACKED", "PLANAR"});

    QPushButton btnOpen("OPEN", &dialog), btnCancel("Cancel",&dialog);
    btnOpen.setMinimumSize(200, 30);
    btnCancel.setMinimumSize(80, 30);

    QSpinBox sensorbitSpin;
    sensorbitSpin.setRange(8, 16);    /// 8 - 16 bit
    sensorbitSpin.setValue(10);
    // 布局
    QFormLayout layout(&dialog);
    layout.addRow("Image Size:", &sizeCombo);
    layout.addRow("Imgae Width:", &widthSpin);
    layout.addRow("Image Height:", &heightSpin);
    layout.addRow("Image SensorBits:",&sensorbitSpin);
    layout.addRow("YUV Format:",&comboBoxYUVFormat);
    layout.addRow("Pixel Format:",&comboBoxPackFormat);
    layout.addRow(&btnOpen,&btnCancel);
    // 连接按钮信号
    connect(&btnOpen, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(&btnCancel, &QPushButton::clicked, &dialog, &QDialog::reject);

    if(dialog.exec()!=QDialog::Accepted){
        return false;
    }

    YUV_Viewer *tab=new YUV_Viewer(this);
    tab->setAttribute(Qt::WA_DeleteOnClose);
    int cur=ui->Tab_MainWindow->addTab(tab,QString::asprintf("YUV Viewer"));
    ui->Tab_MainWindow->setCurrentIndex(cur);
    ui->Tab_MainWindow->setVisible(true);

    int width  = widthSpin.value();
    int height = heightSpin.value();
    int sensorbits = sensorbitSpin.value();
    YUV_Viewer::YUVFormat yuvformat = YUV_Viewer::YUV444;
    YUV_Viewer::YUVFileType yuvpacktype=YUV_Viewer::PACKED;
    switch(comboBoxYUVFormat.currentIndex()){
    case 0: yuvformat = YUV_Viewer::YUV444;break;
    case 1: yuvformat = YUV_Viewer::YUV422;break;
    case 2: yuvformat = YUV_Viewer::YUV420;break;
    default:break;
    }
    switch(comboBoxPackFormat.currentIndex()){
    case 0:yuvpacktype = YUV_Viewer::PACKED;break;
    case 1:yuvpacktype = YUV_Viewer::PLANAR;break;
    default:break;
    }
    int ret = tab->open_with_click_init(filePath,width,height,sensorbits,yuvformat,yuvpacktype);
    if(ret != 0){
        return false;
    }
    return true;
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
        int cur=ui->Tab_MainWindow->addTab(tab,QString::asprintf("Connect to Board"));
        ui->Tab_MainWindow->setCurrentIndex(cur);
        ui->Tab_MainWindow->setVisible(true);
    }
}

void MainWindow::on_act_About_triggered()
{
    //构建富文本信息
    QString content =
        u8R"(
        <style>
            h2 { color: #2c3e50; margin-bottom: 2px; }
            h3 { color: #3498db; margin-top: 5px; margin-bottom: 5px; }
            li { margin: 5px 0; }
            a { color: #2980b9; text-decoration: none; }
            .highlight { background-color: #f8f9fa; padding: 10px; border-radius: 5px; }
        </style>

        <div class='highlight'>
            <h2>MCISP v1.0 软件工具特性</h2>

            <h3>📷 RAW Tuning Tool</h3>
            <ul>
                <li>支持 <b>8-16 bit</b> 位宽的RAW图像显示，离线算法参数调试，运行ISP Pipeline</li>
                <li>支持RAW运行结果的保存（以 <code>uint16_t</code> 格式保存）</li>
            </ul>

            <h3>🌈 YUV Image Viewer</h3>
            <ul>
                <li>支持 <b>8-16 bit</b> 位宽的YUV444/YUV422/YUV420图像显示</li>
                <li>YUV444/YUV422支持 <b>Packed/Planar</b> 解析，YUV420支持Planar</li>
                <li>支持色彩标准选择：BT2020 Full Range、BT601 Full Range、REC709</li>
                <li>支持 <b>YCbCr/Y/Cb/Cr</b> 通道独立显示</li>
                <li>支持另存为 PNG/JPG/BMP/TIFF 等通用图像格式</li>
                <li>支持YUV格式转换存储(YUV444、YUV422、YUV420)(Packed、Planar)(UINT8、UINT16)</li>
            </ul>

            <h3>🎨 RGB Image Viewer</h3>
            <ul>
                <li>支持 <b>8-16 bit</b> 位宽的RGB域图像显示</li>
                <li>支持另存为 PNG/JPG/BMP/TIFF 等通用格式</li>
            </ul>

            <h3>🔌 Connect to Board</h3>
            <ul>
                <li>支持串口连接开发板进行<b>在线参数调试</b></li>
                <li>支持ISP模块参数在线配置/读取</li>
                <li>提供测试接口，可单独读写指定地址寄存器</li>
            </ul>

            <h3>💡 使用技巧</h3>
            <ul>
                <li>关联文件类型：将 <code>.raw/.rgb/.yuv</code> 设置为默认用本软件打开，即可双击文件打开图像</li>
                <li>主题切换：支持 <b>暗黑(Dark)/亮白(Light)</b> 主题</li>
            </ul>
            <p>🔗<a href='https://blog.csdn.net/qq_46144191?spm=1000.2115.3001.10640'>个人博客</a> |✉️ <a href='mailto:470951044@qq.com'>技术支持</a></p>
        </div>
        )";

    // 1. 创建自定义对话框（替代QMessageBox）
    QDialog dialog(this);
    dialog.setWindowTitle(tr("软件信息"));
    dialog.setMinimumSize(600, 400);  // 固定对话框大小

    // 2. 创建文本显示区域（使用QTextBrowser支持完整HTML）
    QTextBrowser *textBrowser = new QTextBrowser(&dialog);
    textBrowser->setOpenExternalLinks(true);

    // dialog.setStyleSheet("QDialog { background-color: #f8f9fa; }");
    textBrowser->setStyleSheet("QTextBrowser { background-color: #f8f9fa; }");

    // 3. 设置精确的样式表
    QString styleSheet =
        u8R"(
        html, body {
            font-family: "Microsoft YaHei", Arial, sans-serif;
            font-size: 10px;
            color: #333333;
            margin: 0;
            padding: 0;
        }
        h2 {
            font-size: 12px;
            color: #2c3e50;
            margin-top: 15px;
            margin-bottom: 8px;
            font-weight: bold;
        }
        h3 {
            font-size: 11px;
            color: #3498db;
            margin-top: 12px;
            margin-bottom: 6px;
        }
        ul {
            margin: 5px 0;
            padding-left: 18px;
        }
        li {
            margin: 4px 0;
            line-height: 1.3;
        }
        a {
            color: #2980b9;
            text-decoration: none;
        }
        .highlight {
            background-color: #f8f9fa;
            padding: 8px;
            border-radius: 4px;
        }
        )";

    // 5. 设置完整HTML文档
    textBrowser->setHtml(QString("<html><head><style>%1</style></head><body>%2</body></html>").arg(styleSheet).arg(content));

    // 7. 布局设置
    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    layout->addWidget(textBrowser);

    // 8. 禁用文本区域的滚动条（可选）
    textBrowser->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    textBrowser->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // 9. 执行对话框
    dialog.exec();
}


void MainWindow::on_actionUser_Manual_triggered()
{
    // 1. 构建文档路径（exe同级目录/Document/user_manual.pdf）
    QString pdfPath = QCoreApplication::applicationDirPath() + "/Document/MCISP用户指南V1.0.pdf";
    // 3. 检查文件是否存在
    if(!QFile::exists(pdfPath)) {
        QMessageBox::information(this, tr("文档缺失"),tr("用户手册未找到，请检查以下路径：\n%1").arg(pdfPath));
        return;
    }
    QUrl pdfUrl = QUrl::fromLocalFile(QDir::toNativeSeparators(pdfPath));

    if(!QDesktopServices::openUrl(pdfUrl)) {
        // 打开失败的回退方案
        QMessageBox::warning(this,tr("打开失败"),tr("无法自动打开PDF文档，请尝试：\n""1. 确保已安装PDF阅读器\n""2. 手动打开文件：\n%1").arg(pdfPath));
    }
}

