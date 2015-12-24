#ifndef OBSTACLEITEM_H
#define OBSTACLEITEM_H

#include <QObject>
#include <QPointF>
#include <memory>
#include <QGraphicsItem>
#include <Util/CoordinateSystem.h>

enum class ObstacleShape{
    Circle, Square
};

class ObstacleItem
{
public:
    ObstacleItem();
    ObstacleItem(double x, double y, double rx, double ry, ObstacleShape shape)
        :x(x),y(y),rX(rx),rY(ry),shape(shape){}
    double x, y;
    double rX, rY;
    ObstacleShape shape = ObstacleShape::Circle;

    std::shared_ptr<QGraphicsItem> make_item(CoordinateSystem* xyz = nullptr);
    std::shared_ptr<QGraphicsItem> pItem = nullptr;

    bool contains(QPointF pt){
        return contains(pt.x(), pt.y());
    }
    bool contains(double x, double y);
};



#endif // OBSTACLEITEM_H
