#include "yuv_viewer.h"
#include "ui_yuv_viewer.h"
#include <QDebug>
#include <QFormLayout>


double BT2020_Coeff[9]={
    1.0,  0,       1.4746,
    1.0,  -0.1646, -0.5714,
    1.0,  1.8814,  0
};


double REC709_Coeff[9]={
    1.0, 0.0, 1.5748,
    1.0, -0.1873, -0.4681,
    1.0, 1.8556, 0.0
};

double BT601_Coeff[9]={
    1.0, -0.0, 1.402,
    1.0, -0.3441, -0.7141,
    1.0, 1.772, -0.0
};


YUV_Viewer::YUV_Viewer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::YUV_Viewer)
{
    ui->setupUi(this);
    view=new My_GraphicsView();
    YUVData =nullptr;
    rgb_image =nullptr;

    ////初始化配置
    yuvformat = YUV444;
    y2rstd    = BT2020;
    dispcolor = YCbCr;
    yuvpacktype = PACKED;

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
    SAFE_FREE(YUVData);
    current_file_path.clear();
}


qint64 YUV_Viewer::getfilesize(QString filepath)
{
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(nullptr, "Error", "Failed to open file!");
        return -1;
    }
    file.close();

    return file.size();  // 获取文件大小（字节数）
}

int YUV_Viewer::readYUVImage(QString filepath)
{
    qint64 pixelsize = image_width*image_height;
    qint64 filesize = getfilesize(filepath);    //尝试打开文件获取文件大小
    if(filesize == -1){
        current_file_path.clear();     //打开失败，对应字符串路径无法打开
        return -1;
    }
    YUVData = new PixelYUV[pixelsize];   //创建空间
    if(yuvformat == YUV444 && (filesize == pixelsize*3 || filesize == pixelsize*6)){
        int ret = readYUV444(filepath,filesize,pixelsize);
        if(ret!=0){
            return -1;
        }
    }else if(yuvformat == YUV422 && (filesize == pixelsize*2 || filesize == pixelsize*4)){
        int ret = readYUV422(filepath,filesize,pixelsize);
        if(ret!=0){
            return -1;
        }
    }else if(yuvformat == YUV420 && (filesize ==pixelsize*1.5 || filesize == pixelsize*3)){
        int ret = readYUV420(filepath,filesize,pixelsize);
        if(ret!=0){
            return -1;
        }
    }else{
        SAFE_FREE(YUVData);
        QMessageBox::critical(nullptr, "Error", "Please Check the YUVFormat(YUV444/422/420) or Size of image (width,height), File Size not match!!! ");  ///文件大小与所选尺寸不匹配
        return -2;
    }
    return 0;

}

int YUV_Viewer::readYUV444(QString filepath,qint64 filesize, qint64 pixelsize)
{
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(nullptr, "Error", "Failed to open file!");
        SAFE_FREE(YUVData);
        return -1;
    }
    if(filesize == pixelsize *3){ ///单字节读取
        if(sensorbits > 8){
            QMessageBox::critical(nullptr, "Error", "Error Check sensorbits or file,file size not match sensorbits!");
            SAFE_FREE(YUVData);
            file.close();
            return -2;
        }
        QDataStream stream(&file);
        stream.setByteOrder(QDataStream::LittleEndian); // 设置字节序（可选）
        switch(yuvpacktype){
        case PACKED:{
            for(int i=0;i<pixelsize;i++){
                uint8_t Y,U,V;
                stream >> Y >> U >> V;
                YUVData[i].Y = static_cast<uint16_t>(Y);
                YUVData[i].U = static_cast<uint16_t>(U);
                YUVData[i].V = static_cast<uint16_t>(V);
            }
        }break;
        case PLANAR:{
            for(int i=0;i<pixelsize;i++){
                uint8_t Y;
                stream >> Y;
                YUVData[i].Y = static_cast<uint16_t>(Y);
            }
            for(int i=0;i<pixelsize;i++){
                uint8_t U;
                stream >> U;
                YUVData[i].U = static_cast<uint16_t>(U);
            }
            for(int i=0;i<pixelsize;i++){
                uint8_t V;
                stream >> V;
                YUVData[i].V = static_cast<uint16_t>(V);
            }
        }break;
        default:break;
        }
    }else{    ///双字节读取
        if(sensorbits <= 8){
            QMessageBox::information(nullptr, "INFO", "sensorbit cconfigured <=8,but file is 2 byte per pixel component!!!");
        }
        QDataStream stream(&file);
        stream.setByteOrder(QDataStream::LittleEndian); // 设置字节序（可选）
        switch(yuvpacktype){
        case PACKED:{
            for(int i=0;i<pixelsize;i++){
                uint16_t Y,U,V;
                stream >> Y >> U >> V;
                YUVData[i].Y = Y;
                YUVData[i].U = U;
                YUVData[i].V = V;
            }
        }break;
        case PLANAR:{
            for(int i=0;i<pixelsize;i++){
                uint16_t Y;
                stream >> Y;
                YUVData[i].Y = Y;
            }
            for(int i=0;i<pixelsize;i++){
                uint16_t U;
                stream >> U;
                YUVData[i].U = U;
            }
            for(int i=0;i<pixelsize;i++){
                uint16_t V;
                stream >> V;
                YUVData[i].V = V;
            }
        }break;
        default:break;
        }
    }
    file.close();
    return 0;
}

int YUV_Viewer::readYUV422(QString filepath,qint64 filesize, qint64 pixelsize)
{
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(nullptr, "Error", "Failed to open file!");
        SAFE_FREE(YUVData);
        return -1;
    }
    if(filesize == pixelsize*2){   //单字节读取
        if(sensorbits > 8){
            QMessageBox::critical(nullptr, "Error", "Error Check sensorbits or file,file size not match sensorbits!");
            SAFE_FREE(YUVData);
            file.close();
            return -2;
        }
        uint8_t *YUV422Data = new uint8_t[filesize];           //暂存422数据
        file.read(reinterpret_cast<char*>(YUV422Data), filesize);  //读取所有YUV422数据
        switch(yuvpacktype){
        case PACKED:{
            for(int i=0;i<pixelsize;i=i+2){          //四个字节为Y1 U1 Y2 V2两个像素
                uint8_t Y1 = YUV422Data[2*i];
                uint8_t U1 = YUV422Data[2*i+1];
                uint8_t Y2 = YUV422Data[2*i+2];
                uint8_t V2 = YUV422Data[2*i+3];
                YUVData[i].Y = static_cast<uint16_t>(Y1);    //两个像素共用一个Y
                YUVData[i].U = static_cast<uint16_t>(U1);
                YUVData[i].V = static_cast<uint16_t>(V2);
                YUVData[i+1].Y = static_cast<uint16_t>(Y2);
                YUVData[i+1].U = static_cast<uint16_t>(U1);
                YUVData[i+1].V = static_cast<uint16_t>(V2);
            }
        }break;
        case PLANAR:{
            uint8_t *YPtr=YUV422Data;
            uint8_t *UPtr=YUV422Data + pixelsize;
            uint8_t *VPtr=YUV422Data + pixelsize + pixelsize/2;
            for(int i=0;i<pixelsize;i++){
                YUVData[i].Y = static_cast<uint16_t>(YPtr[i]);
            }
            for(int i=0;i<pixelsize/2;i++){
                YUVData[2*i].U = static_cast<uint16_t>(UPtr[i]);
                YUVData[2*i+1].U = static_cast<uint16_t>(UPtr[i]);
            }
            for(int i=0;i<pixelsize/2;i++){
                YUVData[2*i].V = static_cast<uint16_t>(VPtr[i]);
                YUVData[2*i+1].V = static_cast<uint16_t>(VPtr[i]);
            }
        }break;
        default:break;
        }
        delete []YUV422Data;  //用完释放暂存空间
    }else{     //双字节读取
        if(sensorbits <= 8){
            QMessageBox::information(nullptr, "INFO", "sensorbit cconfigured <=8,but file is 2 byte per pixel component!!!");
        }
        uint16_t *YUV422Data = new uint16_t[filesize];     //暂存数据
        // 使用 QDataStream 读取
        QDataStream stream(&file);
        stream.setByteOrder(QDataStream::LittleEndian); // 如果是小端数据
        qint64 numU16 = filesize / sizeof(uint16_t);    ///注意此处读取的是uint16_t的数据，需要字节数/2
        for (qint64 i = 0; i < numU16; ++i) {
            stream >> YUV422Data[i];
        }
        switch(yuvpacktype){
        case PACKED:{
            for(int i=0;i<pixelsize;i=i+2){
                uint16_t Y1 = YUV422Data[2*i];
                uint16_t U1 = YUV422Data[2*i+1];
                uint16_t Y2 = YUV422Data[2*i+2];
                uint16_t V2 = YUV422Data[2*i+3];
                YUVData[i].Y = Y1;
                YUVData[i].U = U1;
                YUVData[i].V = V2;
                YUVData[i+1].Y = Y2;
                YUVData[i+1].U = U1;
                YUVData[i+1].V = V2;
            }
        }break;
        case PLANAR:{
            uint16_t *YPtr=YUV422Data;
            uint16_t *UPtr=YUV422Data + pixelsize;
            uint16_t *VPtr=YUV422Data + pixelsize + pixelsize/2;
            for(int i=0;i<pixelsize;i++){
                YUVData[i].Y = YPtr[i];
            }
            for(int i=0;i<pixelsize/2;i++){
                YUVData[2*i].U = UPtr[i];
                YUVData[2*i+1].U = UPtr[i];
            }
            for(int i=0;i<pixelsize/2;i++){
                YUVData[2*i].V = VPtr[i];
                YUVData[2*i+1].V = VPtr[i];
            }
        }break;
        default:break;
        }
        delete []YUV422Data;
    }
    file.close();
    return 0;
}

int YUV_Viewer::readYUV420(QString filepath,qint64 filesize, qint64 pixelsize)
{
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(nullptr, "Error", "Failed to open file!");
        SAFE_FREE(YUVData);
        return -1;
    }
    if(yuvpacktype == PACKED){
        QMessageBox::warning(nullptr, "INFO", "Only support to read YUV420 PLANAR format,Please chose PLANR to Open!!!");
        SAFE_FREE(YUVData);
        file.close();
        return -2;
        // ui->packtype_combx->setCurrentIndex(1);   //设置为PLANAR格式
    }
    if(filesize == pixelsize*1.5){
        if(sensorbits > 8){
            QMessageBox::critical(nullptr, "Error", "Error Check sensorbits or file,file size not match sensorbits!");
            SAFE_FREE(YUVData);
            file.close();
            return -2;
        }
        uint8_t *YUV420Data = new uint8_t[filesize];       //暂存420数据
        file.read(reinterpret_cast<char*>(YUV420Data), filesize);  //读取所有YUV422数据
        uint8_t *YPtr=YUV420Data;
        uint8_t *UPtr=YUV420Data + pixelsize;
        uint8_t *VPtr=YUV420Data + pixelsize + pixelsize/4;
        for(int i=0;i<pixelsize;i++){
            this->YUVData[i].Y = static_cast<uint16_t>(YPtr[i]);
        }
        int k = 0;
        for(int i=0;i<image_height;i=i+2){
            for(int j=0;j<image_width;j=j+2){
                this->YUVData[i*image_width+j].U= static_cast<uint16_t>(UPtr[k]);         //四个像素共用一个U,V分量
                this->YUVData[i*image_width+j].V= static_cast<uint16_t>(VPtr[k]);

                this->YUVData[i*image_width+j+1].U= static_cast<uint16_t>(UPtr[k]);
                this->YUVData[i*image_width+j+1].V= static_cast<uint16_t>(VPtr[k]);

                this->YUVData[(i+1)*image_width+j].U = static_cast<uint16_t>(UPtr[k]);
                this->YUVData[(i+1)*image_width+j].V = static_cast<uint16_t>(VPtr[k]);

                this->YUVData[(i+1)*image_width+j+1].U = static_cast<uint16_t>(UPtr[k]);
                this->YUVData[(i+1)*image_width+j+1].V = static_cast<uint16_t>(VPtr[k]);
                k++;
            }
        }
        delete []YUV420Data;   //用完释放暂存空间
    }else{
        if(sensorbits <= 8){
            QMessageBox::information(nullptr, "INFO", "sensorbit cconfigured <=8,but file is 2 byte per pixel component!!!");
        }
        uint16_t *YUV420Data = new uint16_t[filesize];
        // 使用 QDataStream 读取
        QDataStream stream(&file);
        stream.setByteOrder(QDataStream::LittleEndian); // 如果是小端数据
        qint64 numU16 = filesize / sizeof(uint16_t);    ////注意此处读取的是uint16_t的数据，需要字节数/2
        for (qint64 i = 0; i < numU16; ++i) {
            stream >> YUV420Data[i];
        }
        uint16_t *YPtr=YUV420Data;
        uint16_t *UPtr=YUV420Data + pixelsize;
        uint16_t *VPtr=YUV420Data + pixelsize + pixelsize/4;
        for(int i=0;i<pixelsize;i++){
            this->YUVData[i].Y = YPtr[i];
        }
        int k = 0;
        for(int i=0;i<image_height;i=i+2){
            for(int j=0;j<image_width;j=j+2){
                this->YUVData[i*image_width+j].U= UPtr[k];
                this->YUVData[i*image_width+j].V= VPtr[k];

                this->YUVData[i*image_width+j+1].U= UPtr[k];
                this->YUVData[i*image_width+j+1].V= VPtr[k];

                this->YUVData[(i+1)*image_width+j].U = UPtr[k];
                this->YUVData[(i+1)*image_width+j].V = VPtr[k];

                this->YUVData[(i+1)*image_width+j+1].U = UPtr[k];
                this->YUVData[(i+1)*image_width+j+1].V = VPtr[k];
                k++;

            }
        }
        delete []YUV420Data;  //用完释放暂存空间
    }
    file.close();
    return 0;
}

void YUV_Viewer::yuv2rgb_display()
{
    if(rgb_image){
        delete rgb_image;
        rgb_image=nullptr;              // 将指针置为nullptr，避免悬挂指针问题
    }
    rgb_image = new QImage(image_width,image_height,QImage::Format_RGB888);
    int offset = sensorbits - 8;
    double *coeff=nullptr;
    switch(y2rstd){      ////选择转换的标准
    case BT2020:
        coeff = BT2020_Coeff;
        break;
    case BT601 :
        coeff = BT601_Coeff;
        break;
    case REC709:
        coeff = REC709_Coeff;
        break;
    default:break;
    }
    for(int row=0;row<image_height;row++){
        for(int col=0;col<image_width;col++){
            int pix_pos=row*image_width+col;
            int Y_value,U_value,V_value;

            Y_value = YUVData[pix_pos].Y >> offset;
            U_value = YUVData[pix_pos].U >> offset;
            V_value = YUVData[pix_pos].V >> offset;

            // switch(dispcolor){
            //     case YCbCr:
            //         Y_value = YUVData[pix_pos].Y >> offset;
            //         U_value = YUVData[pix_pos].U >> offset;
            //         V_value = YUVData[pix_pos].V >> offset;
            //         break;
            //     case Y:
            //         Y_value = YUVData[pix_pos].Y >> offset;
            //         U_value = 128;
            //         V_value = 128;
            //         break;
            //     case Cb:
            //         Y_value = 128;
            //         U_value = YUVData[pix_pos].U >> offset;
            //         V_value = 0;
            //         break;
            //     case Cr:
            //         Y_value = 128;
            //         U_value = 0;
            //         V_value = YUVData[pix_pos].V >> offset;
            //         break;
            //     default:break;
            // }
            int temp_R,temp_G,temp_B;

            switch(dispcolor){
                case YCbCr:
                    temp_R = static_cast<int>(Y_value*coeff[0] + (U_value-128)*coeff[1] + (V_value-128)*coeff[2]);
                    temp_G = static_cast<int>(Y_value*coeff[3] + (U_value-128)*coeff[4] + (V_value-128)*coeff[5]);
                    temp_B = static_cast<int>(Y_value*coeff[6] + (U_value-128)*coeff[7] + (V_value-128)*coeff[8]);
                    break;
                case Y:
                    temp_R = Y_value;
                    temp_G = Y_value;
                    temp_B = Y_value;
                    break;
                case Cb:
                    UVRGB ucolor;
                    ucolor= mapU2color(U_value);
                    temp_R = ucolor.r;
                    temp_G = ucolor.g;
                    temp_B = ucolor.b;
                    break;
                case Cr:
                    UVRGB vcolor;
                    vcolor = mapV2color(V_value);
                    temp_R = vcolor.r;
                    temp_G = vcolor.g;
                    temp_B = vcolor.b;
                    break;
                default:break;
            }
            // temp_R = static_cast<int>(Y_value*coeff[0] + (U_value-128)*coeff[1] + (V_value-128)*coeff[2]);
            // temp_G = static_cast<int>(Y_value*coeff[3] + (U_value-128)*coeff[4] + (V_value-128)*coeff[5]);
            // temp_B = static_cast<int>(Y_value*coeff[6] + (U_value-128)*coeff[7] + (V_value-128)*coeff[8]);
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



UVRGB YUV_Viewer::mapU2color(int U)         ////可视化U分量
{
    int val = U % 256;
    return UVRGB{0,val,255-val};
}

UVRGB YUV_Viewer::mapV2color(int V)         ////可视化V分量
{
    int val = V%256;
    return UVRGB{val,0,255-val};
}

bool YUV_Viewer::ShowSaveDialog(YUVFormat &saveformat, YUVFileType &savepacktype, SaveType &datatype)
{
    QDialog dialog(this);
    dialog.setWindowTitle("选择YUV保存参数");
    QComboBox comboBoxSaveFormat, comboBoxPackFormat, comboBoxDataType;
    QPushButton btnOk("保存", &dialog), btnCancel("取消", &dialog);
    btnOk.setMinimumSize(200, 30);
    btnCancel.setMinimumSize(80, 30);
    comboBoxSaveFormat.setMinimumSize(200, 30);  // 宽度 200px，高度 30px
    comboBoxPackFormat.setMinimumSize(200, 30);
    comboBoxDataType.setMinimumSize(200, 30);

    // 添加选项
    comboBoxSaveFormat.addItems({"YUV444", "YUV422", "YUV420"});
    comboBoxPackFormat.addItems({"PACKED", "PLANAR"});
    comboBoxDataType.addItems({"UINT8", "UINT16"});

    // 布局
    QFormLayout layout(&dialog);
    layout.addRow("保存格式:", &comboBoxSaveFormat);
    layout.addRow("打包格式:", &comboBoxPackFormat);
    layout.addRow("数据类型:", &comboBoxDataType);
    layout.addRow(&btnOk, &btnCancel);

    // 连接按钮信号
    connect(&btnOk, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(&btnCancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    // 显示对话框并等待用户选择
    if (dialog.exec() == QDialog::Accepted) {

        switch(comboBoxSaveFormat.currentIndex()){
            case 0: saveformat = YUV444;break;
            case 1: saveformat = YUV422;break;
            case 2: saveformat = YUV420;break;
            default:break;
        }
        switch(comboBoxPackFormat.currentIndex()){
            case 0:savepacktype = PACKED;break;
            case 1:savepacktype = PLANAR;break;
            default:break;
        }
        switch(comboBoxDataType.currentIndex()){
            case 0:datatype = UINT8;break;
            case 1:datatype = UINT16;break;
            default:break;
        }
        return true;
    }
    return false;
}

int YUV_Viewer::writeYUV444(QString filepath, YUVFileType savepacktype, SaveType datatype)
{
    QFile file(filepath);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(nullptr, "Error", "Failed to open file!");
        return -1;
    }
    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian); // 设置字节序（可选）
    if(savepacktype == PACKED){    ///保存packed
        switch(datatype){
        case UINT8:{
            if(sensorbits > 8){
                QMessageBox::warning(nullptr,"Warning","sensorbits > 8, But Save as uint8! all the value will be shift to 8bit, lower bit will be discard!!!");
            }
            for(int i=0; i < image_width * image_height; i++){
                PixelYUV pixel = this->YUVData[i];
                uint8_t Y = static_cast<uint8_t>(pixel.Y>>(sensorbits-8));      //移位丢弃低bit位
                uint8_t U = static_cast<uint8_t>(pixel.U>>(sensorbits-8));
                uint8_t V = static_cast<uint8_t>(pixel.V>>(sensorbits-8));
                stream << Y << U << V;
            }
        }break;
        case UINT16:{
            for(int i=0; i < image_width * image_height; i++){
                PixelYUV pixel = this->YUVData[i];
                stream << pixel.Y << pixel.U << pixel.V;
            }
        }break;
        default:break;
        }
    }else{      ///保存planar
        switch(datatype){
        case UINT8:{
            if(sensorbits > 8){
                QMessageBox::warning(nullptr,"Warning","sensorbits > 8, But Save as uint8! all the value will be shift to 8bit, lower bit will be discard!!!");
            }
            for(int i=0; i < image_width * image_height; i++){          //Y
                PixelYUV pixel = this->YUVData[i];
                uint8_t Y = static_cast<uint8_t>(pixel.Y>>(sensorbits-8));
                stream << Y;
            }
            for(int i=0; i < image_width * image_height; i++){          //U
                PixelYUV pixel = this->YUVData[i];
                uint8_t U = static_cast<uint8_t>(pixel.U>>(sensorbits-8));
                stream << U;
            }
            for(int i=0; i < image_width * image_height; i++){          //V
                PixelYUV pixel = this->YUVData[i];
                uint8_t V = static_cast<uint8_t>(pixel.V>>(sensorbits-8));
                stream << V;
            }
        }break;
        case UINT16:{
            for(int i=0; i < image_width * image_height; i++){
                PixelYUV pixel = this->YUVData[i];
                stream << pixel.Y;
            }
            for(int i=0; i < image_width * image_height; i++){
                PixelYUV pixel = this->YUVData[i];
                stream << pixel.U;
            }
            for(int i=0; i < image_width * image_height; i++){
                PixelYUV pixel = this->YUVData[i];
                stream << pixel.V;
            }
        }break;
        default:break;
        }
    }
    file.close();
    return 0;
}

int YUV_Viewer::writeYUV422(QString filepath, YUVFileType savepacktype, SaveType datatype)
{
    QFile file(filepath);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(nullptr, "Error", "Failed to open file!");
        return -1;
    }
    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian); // 设置字节序（可选）
    if(datatype == UINT8){     ///保存为单字节
        if(sensorbits > 8){
            QMessageBox::warning(nullptr,"Warning","sensorbits > 8, But Save as uint8! all the value will be shift to 8bit, lower bit will be discard!!!");
        }
        switch(savepacktype){
        case PACKED:{
            for(int i=0; i < image_width * image_height; i=i+2){    //两个像素存成Y1 U1 Y2 V2
                PixelYUV pixel1 = this->YUVData[i];
                PixelYUV pixel2 = this->YUVData[i+1];
                uint8_t Y1 = static_cast<uint8_t>(pixel1.Y>>(sensorbits-8));
                uint8_t U1 = static_cast<uint8_t>(pixel1.U>>(sensorbits-8));
                uint8_t Y2 = static_cast<uint8_t>(pixel2.Y>>(sensorbits-8));
                uint8_t V2 = static_cast<uint8_t>(pixel2.V>>(sensorbits-8));
                stream << Y1 << U1 << Y2 << V2;
            }
        }break;
        case PLANAR:{
            for(int i=0; i < image_width * image_height; i++){       //Y
                PixelYUV pixel = this->YUVData[i];
                uint8_t Y = static_cast<uint8_t>(pixel.Y>>(sensorbits-8));
                stream << Y;
            }
            for(int i=0; i < image_width * image_height; i=i+2){     //U 从U1开始
                PixelYUV pixel = this->YUVData[i];
                uint8_t U = static_cast<uint8_t>(pixel.U>>(sensorbits-8));
                stream << U;
            }
            for(int i=1; i < image_width * image_height; i=i+2){     //V 从V2开始
                PixelYUV pixel = this->YUVData[i];
                uint8_t V = static_cast<uint8_t>(pixel.V>>(sensorbits-8));
                stream << V;
            }
        }break;
        // default:break;
        }
    }else{                     ///保存为双字节
        switch(savepacktype){
        case PACKED:{
            for(int i=0; i < image_width * image_height; i=i+2){
                PixelYUV pixel1 = this->YUVData[i];
                PixelYUV pixel2 = this->YUVData[i+1];
                stream << pixel1.Y << pixel1.U << pixel2.Y << pixel2.V;
            }
        }break;
        case PLANAR:{
            for(int i=0; i < image_width * image_height; i++){
                PixelYUV pixel = this->YUVData[i];
                stream << pixel.Y;
            }
            for(int i=0; i < image_width * image_height; i=i+2){
                PixelYUV pixel = this->YUVData[i];
                stream << pixel.U;
            }
            for(int i=1; i < image_width * image_height; i=i+2){
                PixelYUV pixel = this->YUVData[i];
                stream << pixel.V;
            }
        }break;
        }
    }
    file.close();
    return 0;

}

int YUV_Viewer::writeYUV420(QString filepath, YUVFileType savepacktype, SaveType datatype)
{
    QFile file(filepath);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(nullptr, "Error", "Failed to open file!");
        return -1;
    }
    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian); // 设置字节序（可选）
    if(datatype == UINT8){     ///单字节保存
        if(sensorbits > 8){
            QMessageBox::warning(nullptr,"Warning","sensorbits > 8, But Save as uint8! all the value will be shift to 8bit, lower bit will be discard!!!");
        }
        switch(savepacktype){
        case PLANAR:{
            for(int i=0; i < image_width * image_height; i++){     //Y
                PixelYUV pixel = this->YUVData[i];
                uint8_t Y = static_cast<uint8_t>(pixel.Y>>(sensorbits-8));
                stream << Y;
            }
            for(int i=0;i<image_height;i=i+2){                      //U从第一行开始
                for(int j=0;j<image_width;j=j+2){
                    PixelYUV pixel = this->YUVData[i*image_width+j];
                    uint8_t U = static_cast<uint8_t>(pixel.U>>(sensorbits-8));
                    stream << U;
                }
            }
            for(int i=1;i<image_height;i=i+2){                       //V从第二行开始
                for(int j=0;j<image_width;j=j+2){
                    PixelYUV pixel = this->YUVData[i*image_width+j];
                    uint8_t V = static_cast<uint8_t>(pixel.V>>(sensorbits-8));
                    stream << V;
                }
            }
        }break;
        case PACKED:
            QMessageBox::critical(nullptr,"ERROR","YUV420 format only support planar save!!!");
            file.close();
            return -1;
            break;
        }
    }else{                     ///双字节保存
        switch(savepacktype){
        case PLANAR:{
            for(int i=0; i < image_width * image_height; i++){       ///Y
                PixelYUV pixel = this->YUVData[i];
                stream << pixel.Y;
            }
            for(int i=0;i<image_height;i=i+2){                       ///U
                for(int j=0;j<image_width;j=j+2){
                    PixelYUV pixel = this->YUVData[i*image_width+j];
                    stream << pixel.U;
                }
            }
            for(int i=1;i<image_height;i=i+2){                       //V
                for(int j=0;j<image_width;j=j+2){
                    PixelYUV pixel = this->YUVData[i*image_width+j];
                    stream << pixel.V;
                }
            }
        }break;
        case PACKED:
            QMessageBox::critical(nullptr,"ERROR","YUV420 format only support planar save!!!");
            file.close();
            return -1;
            break;
        }
    }
    file.close();
    return 0;
}

void YUV_Viewer::on_btn_open_clicked()
{
    if(ui->imwidth->text() == nullptr || ui->imheight->text() == nullptr){
        QMessageBox::information(nullptr, "INFO", "Please configure the image size first!!!");
        return;
    }
    clear_last();
    image_width=ui->imwidth->text().toInt();    ///宽
    image_height=ui->imheight->text().toInt();  ///长
    sensorbits=ui->spin_bit->value();           ///bit 位宽

    QString file_name = QFileDialog::getOpenFileName(this, "Chose YUV Image", "", "Images(*.yuv *.rgb)");   //获取一个字符串作为文件名
    if (file_name.isEmpty()){         //如果一个文件都没选
        QMessageBox::warning(nullptr, "Warning", "You don't select any file!");
        return;
    }

    int ret = readYUVImage(file_name);
    if(ret!=0){
        return;
    }
    current_file_path = file_name;  ///打开成功后记录当前文件路径
    ui->image_box->setTitle("yuv image preview: "+file_name);
    yuv2rgb_display();
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


void YUV_Viewer::on_conversion_combx_currentIndexChanged(int index)
{
    switch(index){
        case 0: y2rstd = BT2020;break;
        case 1: y2rstd = BT601;break;
        case 2: y2rstd = REC709;break;
        default:break;
    }
    if(YUVData == nullptr){
        return;
    }
    yuv2rgb_display();   //不用重新读取图像，更改显示即可
}


void YUV_Viewer::on_component_combx_currentIndexChanged(int index)
{
    switch(index){
        case 0:dispcolor = YCbCr;break;
        case 1:dispcolor = Y;break;
        case 2:dispcolor = Cb;break;
        case 3:dispcolor = Cr;break;
        default:break;
    }
    if(YUVData == nullptr){
        return;
    }
    yuv2rgb_display();     //不用重新读取图像，更改显示即可
}




void YUV_Viewer::on_convert_btn_clicked()
{
    ///读取保存格式 YUV444 YUV422 YUV420
    ///读取打包格式 PACKED  PLANAR
    //读取保存类型  UINT8  UINT16  单字节/双字节
    //读取保存路径
    ///初始化保存参数

    if(!YUVData){
        QMessageBox::critical(this, "ERROR", "You have not open a file,can not to convert save!!!");
        return;
    }

    YUVFormat  saveformat = YUV444;
    YUVFileType savepacktype = PACKED;
    SaveType   datatype = UINT16;

    if (!ShowSaveDialog(saveformat,savepacktype,datatype)) {     //选择对话框
        return; // 用户取消
    }
    QString savePath = QFileDialog::getSaveFileName(
        this,
        "选择保存路径",
        "",
        "YUV Files (*.yuv);;All Files (*)"
    );
    if (savePath.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Invalid or Empty File path!!!");
        return;
    }
    int ret;
    switch(saveformat){
        case YUV444:{
            ret = writeYUV444(savePath,savepacktype,datatype);
        } break;
        case YUV422:{
            ret = writeYUV422(savePath,savepacktype,datatype);
        }break;
        case YUV420:{
            ret = writeYUV420(savePath,savepacktype,datatype);
        }break;
    }
    if(ret!=0){      ////保存失败移除创建的文件
        QFile file(savePath);
        file.remove();
        return;
    }
    QMessageBox::information(this, "INFO", "yuv file saved to "+savePath+" successfully");
}


void YUV_Viewer::on_yuv_format_currentIndexChanged(int index)
{
    ////重新选择格式后清除一切,猜测用户想重新打开YUV其他格式图像
    if(YUVData!=nullptr){
        clear_last();
        view->SetImage(QImage());  // 传入空 QImage清空画面
    }else{
        switch(index){
        case 0: yuvformat = YUV444;break;
        case 1: yuvformat = YUV422;break;
        case 2: yuvformat = YUV420;break;
        default:break;
        }
    }
}


void YUV_Viewer::on_packtype_combx_currentIndexChanged(int index)
{
    switch(index){
        case 0:yuvpacktype = PACKED;break;
        case 1:yuvpacktype = PLANAR;break;
        default:break;
    }
    if(current_file_path == nullptr){
        return;
    }
    int ret = readYUVImage(current_file_path);     ////PACKED和PLANAR需要重新读取图像
    if(ret!=0){
        return;
    }
    yuv2rgb_display();
}

