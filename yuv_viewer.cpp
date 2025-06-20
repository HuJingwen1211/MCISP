#include "yuv_viewer.h"
#include "ui_yuv_viewer.h"
#include <QDebug>
YUV_Viewer::YUV_Viewer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::YUV_Viewer)
{
    ui->setupUi(this);
    view=new My_GraphicsView();
    isPacked=1;   ///初始勾选packed
    rgb_image =nullptr;
    yuv_data[0] = nullptr;
    yuv_data[1] = nullptr;
    yuv_data[2] = nullptr;
}

YUV_Viewer::~YUV_Viewer()
{
    delete ui;
    delete view;
    clear_last();
}

void YUV_Viewer::clear_last()
{
    if(rgb_image){
        delete rgb_image;
        rgb_image=nullptr;   // 将指针置为nullptr，避免悬挂指针问题
    }
    for(int i=0;i<3;i++){
        if(yuv_data[i]){
            delete [] yuv_data[i];
            yuv_data[i]=nullptr;
        }
    }
}

int YUV_Viewer::load_yuv_image(QString filepath)
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

    long expectedSize_16 = image_height*image_width*6;  ////判断用户输入的长宽是否与文件大小一致，每个像素点每个通道2字节，共3通道
    long expectedSize_8 = image_height*image_width*3;
    fseek(image_file, 0, SEEK_END); // 定位到文件末尾
    long fileSize = ftell(image_file); // 获取文件大小
    fseek(image_file, 0, SEEK_SET); // 定位回文件开头
    if ((fileSize != expectedSize_8 &&sensorbits==8) || (fileSize != expectedSize_16 && sensorbits>8)){
        QMessageBox::critical(nullptr, "Error", "File size or sensobits not match the width and height,pleasee check and retry!");
        return -1;
    }


    //////初始化yuv_data指向的空间
    yuv_data[Y]=new u16[image_width*image_height]();
    yuv_data[U]=new u16[image_width*image_height]();
    yuv_data[V]=new u16[image_width*image_height]();
    u16 read_val;
    if(isPacked){
        for(int row=0;row<image_height;row++){            /////读取YUV444 packed格式数据
            for(int col=0;col<image_width;col++){
                if(sensorbits==8){
                    for(int i=0;i<3;i++){
                       fread(&read_val, sizeof(u8), 1, image_file); ///读取数据,8bit位宽数据
                       yuv_data[i][row*image_width+col]=static_cast<u8>(read_val); ///导入数据
                    }
                }else{
                    for(int i=0;i<3;i++){
                       fread(&read_val, sizeof(u16), 1, image_file); ///读取数据，大于8bit位宽数据
                       yuv_data[i][row*image_width+col]=read_val; ///导入数据
                    }

                }
            }
        }
    }else{                ////读取planar444数据
        for(int i=0;i<3;i++){
            for(int row=0;row<image_height;row++){
                for(int col=0;col<image_width;col++){
                    if(sensorbits==8){
                        fread(&read_val, sizeof(u8), 1, image_file); ///读取数据,8bit位宽数据
                        yuv_data[i][row*image_width+col]=static_cast<u8>(read_val); ///导入数据
                    }else{
                        fread(&read_val, sizeof(u16), 1, image_file); ///读取数据，大于8bit位宽数据
                        yuv_data[i][row*image_width+col]=read_val; ///导入数据
                    }
                }
            }
        }

    }

    fclose(image_file);
    return 0;
}

void YUV_Viewer::display_image()          /////相比RGB_Viewer多了yuv转rgb的步骤
{
    int offset=sensorbits-8;
    for(int row=0;row<image_height;row++){
        for(int col=0;col<image_width;col++){
            int pix_pos=row*image_width+col;
            int Y_data=yuv_data[Y][pix_pos]>>offset;
            int U_data=yuv_data[U][pix_pos]>>offset;
            int V_data=yuv_data[V][pix_pos]>>offset;
            int temp_R,temp_G,temp_B;
            temp_R=static_cast<int>(Y_data + 1.4746 *(V_data-128));              //////暂时只支持BT2020转换公式
            temp_G=static_cast<int>(Y_data - 0.1645 * (U_data - 128) - 0.5713 * (V_data - 128));
            temp_B=static_cast<int>(Y_data + 1.8814 * (U_data - 128));
            temp_R = qBound(0, temp_R, 255);
            temp_G = qBound(0, temp_G, 255);
            temp_B = qBound(0, temp_B, 255);
            u8 pixel_R= static_cast<u8>(temp_R);      //////将YUV格式的数据转换为RGB进行显示
            u8 pixel_G= static_cast<u8>(temp_G);
            u8 pixel_B= static_cast<u8>(temp_B);
            rgb_image->setPixel(col,row,qRgb(pixel_R,pixel_G,pixel_B));  ////设置显示的值
        }
    }
    view->SetImage(*rgb_image);          ////将Qimage对象传入view
    QGridLayout *layout=ui->image_layout;        //////获取image_preview_box下的layout
    layout->addWidget(view);
}

void YUV_Viewer::packed_planar_changed()
{
    clear_last();
    if(current_file_path.isEmpty()){    ////首次打开就选择planar/packed先判断路径
        return;
    }
    int isOk=load_yuv_image(current_file_path);
    if(isOk!=0){    ////load失败则返回，不进行接下来的操作
        return;
    }
    rgb_image=new QImage(image_width,image_height,QImage::Format_RGB888);
    display_image();   ///显示yuv图像
}



void YUV_Viewer::on_btn_open_clicked()
{
    clear_last();
    image_width=ui->imwidth->text().toInt();    ///宽
    image_height=ui->imheight->text().toInt();  ///长
    sensorbits=ui->spin_bit->value();           ///bit 位宽

    QString file_name = QFileDialog::getOpenFileName(this, "Chose RGB Image", "", "Images(*.yuv)");
    if (file_name.isEmpty()){         //如果一个文件都没选
        QMessageBox::warning(nullptr, "Warning", "You don't select any file!");
        return;
     }

    int isOk=load_yuv_image(file_name);
    if(isOk!=0){    ////load失败则返回，不进行接下来的操作
        return;
    }
    current_file_path=file_name;  ///记录当前文件路径以便更新操作
    ui->image_box->setTitle("yuv image preview: "+file_name);
    rgb_image=new QImage(image_width,image_height,QImage::Format_RGB888);
    display_image();   ///显示yuv图像
}



void YUV_Viewer::on_packed_check_toggled(bool checked)
{
    if(checked){
        isPacked=1;
        ui->planar_check->setChecked(false);
        packed_planar_changed();
    }else{
        ui->planar_check->setChecked(true);
    }

}

void YUV_Viewer::on_planar_check_toggled(bool checked)
{
    if(checked){
        isPacked=0;
        ui->packed_check->setChecked(false);
        packed_planar_changed();
    }else{
        ui->packed_check->setChecked(true);
    }
}


void YUV_Viewer::on_size_combox_currentIndexChanged(int index)
{
    switch(index){
    case 0:
        ui->imwidth->clear();
        ui->imheight->clear();
        ui->imwidth->setEnabled(true);
        ui->imheight->setEnabled(true);
        break;
    case 1:
        ui->imwidth->setText("640");
        ui->imheight->setText("480");
        ui->imwidth->setEnabled(false);
        ui->imheight->setEnabled(false);
        break;
    case 2:
        ui->imwidth->setText("832");
        ui->imheight->setText("480");
        ui->imwidth->setEnabled(false);
        ui->imheight->setEnabled(false);
        break;
    case 3:
        ui->imwidth->setText("720");
        ui->imheight->setText("576");
        ui->imwidth->setEnabled(false);
        ui->imheight->setEnabled(false);
        break;
    case 4:
        ui->imwidth->setText("1280");
        ui->imheight->setText("720");
        ui->imwidth->setEnabled(false);
        ui->imheight->setEnabled(false);
        break;
    case 5:
        ui->imwidth->setText("1920");
        ui->imheight->setText("1080");
        ui->imwidth->setEnabled(false);
        ui->imheight->setEnabled(false);
        break;
    case 6:
        ui->imwidth->setText("3840");
        ui->imheight->setText("2160");
        ui->imwidth->setEnabled(false);
        ui->imheight->setEnabled(false);
        break;
    case 7:
        ui->imwidth->setText("1024");
        ui->imheight->setText("768");
        ui->imwidth->setEnabled(false);
        ui->imheight->setEnabled(false);
        break;
    case 8:
        ui->imwidth->setText("1280");
        ui->imheight->setText("960");
        ui->imwidth->setEnabled(false);
        ui->imheight->setEnabled(false);
        break;
    default:break;
    }
}


void YUV_Viewer::on_save_clicked()
{
    // 首先检查图像是否有效
    if (rgb_image && !rgb_image->isNull()) {
        // 获取保存文件的路径
        QString filePath = QFileDialog::getSaveFileName(
            this,
            tr("Save Image"),
            QDir::homePath(),
            tr("Images (*.png *.jpg *.jpeg *.bmp *.tif *.tiff)")
            );

        // 如果用户没有取消对话框
        if (!filePath.isEmpty()) {
            // 根据文件扩展名确定格式
            QString extension = QFileInfo(filePath).suffix().toLower();

            // 保存图像
            bool saved = rgb_image->save(filePath, extension.toStdString().c_str());

            if (!saved) {
                QMessageBox::warning(this, tr("Error"), tr("Failed to save the image."));
            }else{
                QMessageBox::information(this, "成功", "图像已保存到"+filePath);
            }
        }
    } else {
        QMessageBox::warning(this, tr("Error"), tr("No image to save."));
    }

}

