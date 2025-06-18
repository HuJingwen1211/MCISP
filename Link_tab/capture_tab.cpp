#include "capture_tab.h"
#include "ui_capture_tab.h"
#include <QFileDialog>

capture_tab::capture_tab(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::capture_tab)
{
    ui->setupUi(this);
    link_tab=qobject_cast<link_board*>(this->parent());
    // connect(this,&capture_tab::capture_path_signal,);
}

capture_tab::~capture_tab()
{
    delete ui;
}

void capture_tab::on_capture_btn_clicked()
{
    ///0x01:RAW 0x02:RGB  0x03:YUV
    uint8_t databuf[1]={0x01};
    link_tab->send_cmd_data(CAPTURE_CMD,databuf,1);
    // 弹出文件保存对话框
    // QString fileName = QFileDialog::getSaveFileName(
    //     this,                           // 父窗口
    //     tr("保存文件"),                 // 对话框标题
    //     QDir::homePath(),               // 默认打开目录（用户主目录）
    //     tr("图片文件 (*.png *.jpg *.bmp);;所有文件 (*)") // 文件过滤器
    //     );
}

