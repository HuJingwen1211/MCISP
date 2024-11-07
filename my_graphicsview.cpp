#include "my_graphicsview.h"

My_GraphicsView::My_GraphicsView(QWidget *parent):
    QGraphicsView(parent),
    m_isTranslate(false),
    m_scene(new QGraphicsScene()),          ////初始化图形项场景，作为容器控制内部项目
    m_imageItem(new QGraphicsPixmapItem())  ///初始化图像项目
{
    m_scene->addItem(m_imageItem);  ////将图像项目添加到项目当中，如果只显示一张，在显示之前需要clear
    setScene(m_scene);   /////将场景添加到视图中

    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);////设置滚动条的显示策略为超出显示范围才可见
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setDragMode(QGraphicsView::ScrollHandDrag);        ////光标变为手形，拖动鼠标将滚动滚动条
    setRenderHint(QPainter::SmoothPixmapTransform);    /////设置试图的渲染方式
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);///设置场景中任何部分发生变化时采取的更新模式
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);////将鼠标所在位置作为锚点，在进行变换时，鼠标所在位置的点将作为变换的中心点
    setResizeAnchor(QGraphicsView::AnchorUnderMouse); ///将鼠标所在位置作为锚点，在调整大小时，鼠标所在位置的点将保持固定。
    setInteractive(true);
    setMouseTracking(true);
    centerOn(0, 0);    /////用于将视图的中心定位到指定的场景坐标或图形项上,(0,0)表示不移动

}

My_GraphicsView::~My_GraphicsView()
{
    m_scene->deleteLater();
    delete m_imageItem;
}

/////将QImage图像项放置到Item容器中
void My_GraphicsView::SetImage(const QImage &image)
{
//    m_scene->clear();
//    m_imageItem=new QGraphicsPixmapItem();
//    m_scene->addItem(m_imageItem);
    m_imageItem->setPixmap(QPixmap::fromImage(image));
    QPoint newCenter(image.width() / 2 ,image.height()/2);
    //centerOn(newCenter);//设置scene中心到图像中点
    QRectF bounds = m_imageItem->boundingRect();
    fitInView(bounds, Qt::KeepAspectRatio);     //将视图调整为图像的大小，并居中显示
    show();  ///显示视图
}

///////鼠标滚轮监听事件进行放大缩小
void My_GraphicsView::wheelEvent(QWheelEvent *event)
{
    QPoint scrollAmount = event->angleDelta();  ///获取滚轮的滚动量
    scrollAmount.y() > 0 ? ZoomIn() : ZoomOut();///正值表示滚轮远离使用者放大,负值表示朝向使用者缩小
}

////鼠标移动事件
void My_GraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    if(m_isTranslate)     ////只有当鼠标左键按下时才进行鼠标移动量的计算和视图的拖拽移动
    {
        QPointF mouseDelta = event->pos()-m_lastMousePos;
        Translate(mouseDelta);
    }
    m_lastMousePos = event->pos();   ///更新鼠标上一次位置
    QGraphicsView::mouseMoveEvent(event);
}

////鼠标按下事件
void My_GraphicsView::mousePressEvent(QMouseEvent *event)
{
    /////判断鼠标的按键情况
    if(event->button() == Qt::LeftButton)   /////鼠标左键
    {
        m_isTranslate = true;   ////按下鼠标左键的标志信号
        m_lastMousePos = event->pos();
    }
    else if(event->button()==Qt::RightButton)  ///鼠标右键，后期配置选取ROI
    {
        //QPointF point = mapToScene(event->pos());
        //////发送鼠标右键点击处的坐标
    }
    QGraphicsView::mousePressEvent(event);// 调用基类的处理函数，确保其他默认行为继续生效
}

///鼠标松开事件
void My_GraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        m_isTranslate = false;   ///设置鼠标左键为松开状态
    QGraphicsView::mouseReleaseEvent(event);
}

/////鼠标左键双击事件
void My_GraphicsView::mouseDoubleClickEvent(QMouseEvent *event)
{
    centerOn(m_imageItem->pixmap().width()/2,m_imageItem->pixmap().height()/2);
    QGraphicsView::mouseDoubleClickEvent(event);
}

////放大
void My_GraphicsView::ZoomIn()
{
    Zoom(1.1);
}

///缩小
void My_GraphicsView::ZoomOut()
{
    Zoom(0.9);
}

/////缩放因子,基于当下进行缩放
void My_GraphicsView::Zoom(double scaleFactor)
{
    //////transform()用于获取视图的当前变换矩阵,变换矩阵描述了应用于视图的所有变换操作，例如平移、缩放和旋转
    qreal factor = transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
    if (factor < 0.07 || factor > 150)   ////限制缩放的极限倍数(宽度),防止无意义放大缩小操作
        return;
    scale(scaleFactor, scaleFactor);     ///保持视图长宽比进行缩放
}

/////按鼠标拖拽的偏移量进行图片的拖动效果
void My_GraphicsView::Translate(QPointF delta)
{
    ////viewport用于获取视图的视口对象，视口是指视图的可见区域，用于显示场景内容。
    int w = viewport()->rect().width();
    int h = viewport()->rect().height();
    /////拖拽的方向与scollbar的移动方向相反，所以下面是相减
    QPoint newCenter(int(w / 2. - delta.x()+0.5),  int(h / 2. - delta.y()+0.5));
    centerOn(mapToScene(newCenter));    ////mapToScene将视图坐标转换为场景坐标
}









