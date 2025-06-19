#include "rgb_viewer.h"
#include "ui_rgb_viewer.h"
#include <QDebug>

RGB_Viewer::RGB_Viewer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::RGB_Viewer)
{
    ui->setupUi(this);
    view=new My_GraphicsView();   /////新建view
    rgb_image = nullptr;
    rgb_data[0] = nullptr;
    rgb_data[1] = nullptr;
    rgb_data[2] = nullptr;

}

RGB_Viewer::~RGB_Viewer()     ////释放内存空间
{
    clear_last();
    delete view;
    delete ui;                /////凡是通过new 申请的内存都需要进行释放
}

void RGB_Viewer::clear_last()   ////重新选择文件后清除上次申请的内存空间,需要判断是否为空，否则会重复释放报错
{
    if(rgb_image){
        delete rgb_image;
        rgb_image=nullptr;   // 将指针置为nullptr，避免悬挂指针问题
    }
    for(int i=0;i<3;i++){
        if(rgb_data[i]){
            delete [] rgb_data[i];
            rgb_data[i]=nullptr;
        }
    }
}


int  RGB_Viewer::load_rgb_iamge(QString filepath)
{
    QByteArray byteArray = filepath.toUtf8();    ///////将QString类型的字符串转为const char*便于文件读入
    const char* charArray = byteArray.constData();
    FILE* image_file = fopen(charArray, "rb");
    ////判断选择的文件是否能打开
    if(!image_file){
        QMessageBox::critical(nullptr, "Error", "Can not open file!");
        //qDebug()<<"file not found";
        return -1;
    }
    long expectedSize;
    if(sensorbits <= 8){
        expectedSize = image_height*image_width*3;
    }else{
        expectedSize = image_height*image_width*6;
    }

    fseek(image_file, 0, SEEK_END); // 定位到文件末尾
    long fileSize = ftell(image_file); // 获取文件大小
    fseek(image_file, 0, SEEK_SET); // 定位回文件开头
    if (fileSize != expectedSize){
        QMessageBox::critical(nullptr, "Error", "File size or sensorbits not match the width and height,pleasee check and retry!");
        return -1;
    }

    /////初始化指针指向的内存空间
    rgb_data[R]=new u16[image_width*image_height]();
    rgb_data[G]=new u16[image_width*image_height]();
    rgb_data[B]=new u16[image_width*image_height]();
    u16 read_val=0;
    for(int row=0;row<image_height;row++){
        for(int col=0;col<image_width;col++){
            for(int i=0;i<3;i++){
                if(sensorbits <= 8 ){
                    u8 val_u8=0;
                    fread(&val_u8, sizeof(u8), 1, image_file); ///读取数据
                    rgb_data[i][row*image_width+col]=static_cast<u16>(val_u8); ///导入数据
                }else{
                    fread(&read_val, sizeof(u16), 1, image_file); ///读取数据
                    rgb_data[i][row*image_width+col]=read_val; ///导入数据
                }
            }
        }
    }
    fclose(image_file);

    return 0;   ////如果一切正常则返回0
}

void RGB_Viewer::display_image()
{
    for(int row=0;row<image_height;row++){
        for(int col=0;col<image_width;col++){
            u16 pixel_R=rgb_data[R][row*image_width+col]>>(sensorbits-8);
            u16 pixel_G=rgb_data[G][row*image_width+col]>>(sensorbits-8);
            u16 pixel_B=rgb_data[B][row*image_width+col]>>(sensorbits-8);
            int set_R,set_G,set_B;
            set_R=qBound(0,static_cast<int>(pixel_R),255);
            set_G=qBound(0,static_cast<int>(pixel_G),255);
            set_B=qBound(0,static_cast<int>(pixel_B),255);
            rgb_image->setPixel(col,row,qRgb(set_R,set_G,set_B));  ////导入用于显示的值
        }
    }
    //////显示图像
    view->SetImage(*rgb_image);          ////将Qimage对象传入view
    QGridLayout *layout=ui->image_layout;        //////获取image_preview_box下的layout
    layout->addWidget(view);
}


void RGB_Viewer::on_btn_open_clicked()
{
    clear_last();     /////打开之前先清理一遍
    //////导入RGB参数
    image_width=ui->imwidth->text().toInt();
    image_height=ui->imheight->text().toInt();
    sensorbits=ui->spin_sensorbits->value();

    QString file_name = QFileDialog::getOpenFileName(this, "Chose RGB Image", "", "Images(*.rgb *.yuv)");
    if (file_name.isEmpty()){         //如果一个文件都没选
        QMessageBox::warning(nullptr, "Warning", "You don't select any file!");
        //qDebug("empty file!!");
        return;
    }
    int isOk=load_rgb_iamge(file_name);   //////读取.rgb文件,如果读取失败则返回
    if(isOk!=0){    ////load文件失败则返回
        return;
    }
    ui->image_box->setTitle("rgb image preview: "+file_name);
    rgb_image=new QImage(image_width,image_height,QImage::Format_RGB888);    ////创建QImage
    display_image();         //////可视化RGB图像
}

