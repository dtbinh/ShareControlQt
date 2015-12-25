#include "ObstacleItem.h"
#include <QGraphicsEllipseItem>
#include <QGraphicsRectItem>
#include <cmath>

GraphicsObstacleItem::GraphicsObstacleItem(ObstacleShape shape, qreal x, qreal y, qreal rX, qreal rY, QGraphicsItem *parent):
    QGraphicsItem(parent), x(x),y(y),rX(abs(rX)),rY(abs(rY)),myShape(shape)
{
    highlight_brush.setColor(Qt::blue);
    this->setZValue(-10);
    this->setPos(x, y);
}

QRectF GraphicsObstacleItem::boundingRect() const{
    return QRectF(-rX-0.5, -rY-0.5, 2*rX+1, 2*rY+1);
}
void GraphicsObstacleItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){

    painter->setPen(Qt::black);
    if (highlight == true){
        painter->setBrush(QColor(255, 0, 0, 127));
    }
    else{
        painter->setBrush(Qt::NoBrush);
    }

    if (myShape == ObstacleShape::Circle){
        painter->drawEllipse(-rX, -rY, 2*rX, 2*rY);
    }
    else if (myShape == ObstacleShape::Square){
        painter->drawRect(-rX, -rY, 2*rX, 2*rY);
    }
}

QPainterPath GraphicsObstacleItem::shape() const{
    if (myShape == ObstacleShape::Circle){
        QPainterPath path;
        path.addEllipse(boundingRect());
        return path;
    }
    else{
        QPainterPath path;
        path.addRect(boundingRect());
        return path;
    }
}

// 暂时不考虑 (x竖, y横) 的情况
QGraphicsItem* ObstacleItem::make_item(CoordinateSystem* xyz){
    // 在原始的坐标系里 x轴向右 y轴向下
    qreal unitX = !xyz ? 1 :
                   xyz->XHeading() == AxisHeading::right ? xyz->UnitSize() : -xyz->UnitSize();
    qreal unitY = !xyz ? 1 :
                   xyz->YHeading() == AxisHeading::down  ? xyz->UnitSize() : -xyz->UnitSize();

    pItem = std::shared_ptr<GraphicsObstacleItem>(
                new GraphicsObstacleItem(shape, unitX*x, unitY*y, unitX*rX, unitY*rY));
    return pItem.get();
}

bool ObstacleItem::contains(double px, double py){
    if (shape == ObstacleShape::Circle){
        return ((px-x)*(px-x)/rX/rX + (py-y)*(py-y)/rY/rY < 1);
    }
    else if (shape == ObstacleShape::Square){
        return std::max(abs((px-x)/rY), abs((py-y)/rY)) < 1;
    }
    return false;
}
