#ifndef OBSTACLEITEM_H
#define OBSTACLEITEM_H

#include <QObject>
#include <QPointF>
#include <memory>
#include <QGraphicsItem>
#include <Util/CoordinateSystem.h>
#include <Util/Auxilary.h>

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
    bool   highlight = false;

    int ID;
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
        :x(x),y(y),rX(rx),rY(ry),shape(shape){}

    QGraphicsItem* make_item(CoordinateSystem* xyz = nullptr);
    std::shared_ptr<GraphicsObstacleItem> pItem = nullptr;

    bool contains(QPointF pt){
        return contains(pt.x(), pt.y());
    }
    bool contains(double px, double py);

protected:
    double x, y;
    double rX, rY;
    ObstacleShape shape = ObstacleShape::Circle;
};



#endif // OBSTACLEITEM_H
