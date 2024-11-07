#ifndef YUV_VIEWER_H
#define YUV_VIEWER_H

#include <QMainWindow>
#include "my_graphicsview.h"
#include <QMessageBox>
#include <QFileDialog>
typedef uint16_t u16;
typedef uint8_t  u8;

typedef enum{
    Y=0,
    U=1,
    V=2
}YUV_COLOR;


namespace Ui {
class YUV_Viewer;
}

class YUV_Viewer : public QMainWindow
{
    Q_OBJECT

private:
    u16 *yuv_data[3];
    int image_width;
    int image_height;
    int sensorbits;
    QImage *rgb_image;
    My_GraphicsView *view;
    bool isPacked;    //////1:packed   0:planar
    QString current_file_path;

public:
    explicit YUV_Viewer(QWidget *parent = 0);
    ~YUV_Viewer();
    void clear_last();
    int  load_yuv_image(QString filepath);
    void display_image();
    void packed_planar_changed();

private slots:
    void on_btn_open_clicked();

    void on_packed_check_toggled(bool checked);

    void on_planar_check_toggled(bool checked);

private:
    Ui::YUV_Viewer *ui;
};

#endif // YUV_VIEWER_H
