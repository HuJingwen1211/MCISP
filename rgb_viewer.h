#ifndef RGB_VIEWER_H
#define RGB_VIEWER_H

#include <QMainWindow>
#include "my_graphicsview.h"
#include <QMessageBox>
#include <QFileDialog>

typedef uint16_t u16;
typedef uint8_t u8;

typedef enum{
    R=0,
    G=1,
    B=2
}RGB_COLOR;

namespace Ui {
class RGB_Viewer;
}

class RGB_Viewer : public QMainWindow
{
    Q_OBJECT

 private:
    u16 *rgb_data[3];  ////保存RGB数据
    int image_width;
    int image_height;
    int sensorbits;
    QImage *rgb_image;
    My_GraphicsView *view;

public:
    explicit RGB_Viewer(QWidget *parent = 0);
    ~RGB_Viewer();
    void clear_last();
    int  load_rgb_iamge(QString filepath);
    void display_image();


private slots:
    void on_btn_open_clicked();

private:
    Ui::RGB_Viewer *ui;
};

#endif // RGB_VIEWER_H
