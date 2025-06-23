#ifndef YUV_VIEWER_H
#define YUV_VIEWER_H

#include <QMainWindow>
#include "my_graphicsview.h"
#include <QMessageBox>
#include <QFileDialog>


typedef uint16_t u16;
typedef uint8_t  u8;


#define SAFE_FREE(arr)     \
do                     \
    {                      \
            if (arr)           \
        {                  \
                delete[] arr;  \
                arr = nullptr; \
        }                  \
    } while (0)


namespace Ui {
class YUV_Viewer;
}

typedef struct {
    uint16_t Y;
    uint16_t U;
    uint16_t V;
}PixelYUV;


typedef struct{
    int r, g, b;
}UVRGB ;

class YUV_Viewer : public QMainWindow
{
    Q_OBJECT

public:
    enum YUVFormat {  //YUV格式:目前支持444、422、420
        YUV444 =0,
        YUV422 =1,
        YUV420 =2    //YUV420只支持PLANAR格式
    };
    enum YUVFileType {  //YUV文件类型:目前支持打包和平面格式
        PACKED =0,  //打包格式
        PLANAR =1   //平面格式
    };
    enum SaveType{
        UINT8 =0,
        UINT16 =1
    };
    enum YUV2RGB_Standard{
        BT2020 = 0,
        BT601  = 1,
        REC709 = 2
    };
    typedef enum{
        YCbCr=0,
        Y    =1,
        Cb   =2,
        Cr   =3
    }YUV_COLOR;

private:
    PixelYUV* YUVData = nullptr;
    int image_width;
    int image_height;
    int sensorbits;
    YUVFormat yuvformat;
    YUVFileType yuvpacktype;
    YUV2RGB_Standard y2rstd;
    YUV_COLOR dispcolor;
    QImage *rgb_image = nullptr;
    My_GraphicsView *view;
    QString current_file_path=nullptr;

private:
    void clear_last();
    qint64 getfilesize(QString filepath);
    int readYUVImage(QString filepath);
    int readYUV444(QString filepath,qint64 filesize,qint64 pixelsize);
    int readYUV422(QString filepath,qint64 filesize,qint64 pixelsize);
    int readYUV420(QString filepath,qint64 filesize,qint64 pixelsize);
    void yuv2rgb_display();
    UVRGB mapU2color(int U);
    UVRGB mapV2color(int V);
    bool ShowSaveDialog(YUVFormat  &saveformat,YUVFileType &savepacktype,SaveType   &datatype);
    int writeYUV444(QString filepath ,YUVFileType savepacktype,SaveType   datatype);
    int writeYUV422(QString filepath ,YUVFileType savepacktype,SaveType   datatype);
    int writeYUV420(QString filepath ,YUVFileType savepacktype,SaveType   datatype);

public:
    explicit YUV_Viewer(QWidget *parent = 0);
    ~YUV_Viewer();
    int open_with_click_init(QString filePath,int width,int height ,int sensorbits,YUVFormat format,YUVFileType filetype);

private slots:
    void on_btn_open_clicked();
    void on_size_combox_currentIndexChanged(int index);
    void on_save_clicked();
    void on_conversion_combx_currentIndexChanged(int index);
    void on_component_combx_currentIndexChanged(int index);
    void on_convert_btn_clicked();
    void on_yuv_format_currentIndexChanged(int index);
    void on_packtype_combx_currentIndexChanged(int index);

private:
    Ui::YUV_Viewer *ui;
};



#endif // YUV_VIEWER_H
