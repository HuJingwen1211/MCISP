#ifndef MY_GRAPHICSVIEW_H
#define MY_GRAPHICSVIEW_H
#include <QGraphicsView>
#include <qevent.h>
#include <QGraphicsPixmapItem>
class My_GraphicsView : public QGraphicsView
{
public:
    My_GraphicsView(QWidget *parent = 0);
    ~My_GraphicsView();
    void SetImage(const QImage & image);

protected:
    virtual void wheelEvent(QWheelEvent *event) override;  ///鼠标滚轮事件
    virtual void mouseMoveEvent(QMouseEvent *event) override; ///鼠标移动事件
    virtual void mousePressEvent(QMouseEvent *event) override; ///鼠标按下事件
    virtual void mouseReleaseEvent(QMouseEvent *event) override; ///鼠标松开事件
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override; ///鼠标双击事件

public slots:
    void ZoomIn();
    void ZoomOut();
    void Zoom(double scaleFactor);
    void Translate(QPointF delta);  /////拖动效果

private:
    bool m_isTranslate;  ////////按下鼠标左键的标志信号
    QPoint m_lastMousePos;  //////上一次坐标位置
    QGraphicsScene * m_scene;  /////场景
    QGraphicsPixmapItem * m_imageItem;  /////图像项目容器
};

#endif // MY_GRAPHICSVIEW_H
