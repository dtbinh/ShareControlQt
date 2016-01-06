#ifndef OBSTACLEITEM_H
#define OBSTACLEITEM_H

#include <QObject>
#include <QPointF>
#include <memory>
#include <QGraphicsItem>
#include <Util/CoordinateSystem.h>
#include <Util/Auxilary.h>
#include <mutex>

enum class ObstacleShape{
    Circle, Square
};

class GraphicsObstacleItem
        :public QGraphicsItem
{
public:
    explicit GraphicsObstacleItem(ObstacleShape shape, qreal x, qreal y, qreal rX, qreal rY,  QGraphicsItem *parent = 0);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QPainterPath shape() const;

public:
    bool highlight = false;
    int  ID;

protected:
    qreal x,  y;
    qreal rX, rY;
    ObstacleShape myShape;
    QBrush highlight_brush;
};

class ObstacleItem
{
public:
    ObstacleItem(){}
    ObstacleItem(double x, double y, double rx, double ry, ObstacleShape shape)
        :x(x),y(y),rX(rx),rY(ry),shape(shape)
    {
        qDebug()<<QString("new obstacle at (%1, %2) - %3")
                  .arg(x, 0, 'f', 3)
                  .arg(y, 0, 'f', 3)
                  .arg(rx, 0, 'f', 3);

    }
    ObstacleItem(const ObstacleItem& it){
        x = it.x; y = it.y; rX = it.rX; rY = it.rY; shape = it.shape; xyz = it.xyz;
    }
    ObstacleItem& operator = (const ObstacleItem& it){
        x = it.x; y = it.y; rX = it.rX; rY = it.rY; shape = it.shape; xyz = it.xyz;
        return *this;
    }

    QGraphicsItem* make_item(CoordinateSystem* xyz = nullptr);
    std::shared_ptr<GraphicsObstacleItem> pItem = nullptr;

    bool contains(QPointF pt){
        return contains(pt.x(), pt.y());
    }
    bool contains(double px, double py);

    void setRealPos(qreal x, qreal y){
        if (xyz) pItem->setPos(xyz->map(QPointF(x,y)) * xyz->UnitSize());
        else pItem->setPos(x, y);
        lock.lock();
        this->x = x;    // 只是用来方便以后判断contains的
        this->y = y;    // 只是用来方便以后判断contains的
        lock.unlock();
    }

    void getRealPos(double& x, double& y){
        lock.lock();
        x = this->x;
        y = this->y;
        lock.unlock();
    }

protected:
    std::mutex lock;
    double x, y;
    double rX, rY;
    ObstacleShape shape   = ObstacleShape::Circle;
    CoordinateSystem* xyz = nullptr;
};



#endif // OBSTACLEITEM_H
