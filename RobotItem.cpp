#include "RobotItem.h"


CoordinateSystem GraphicsRobotItem::defaultCoordination = CoordinateSystem(100);

void GraphicsRobotItem::paint(
        QPainter *painter,
        const QStyleOptionGraphicsItem *option,
        QWidget *widget)
{
    qreal _heading = heading * 3.14159265 / 180;
    qreal _radius = radius * (coordination ? coordination->UnitSize()
                                           : defaultCoordination.UnitSize());

    // 先旋转 再画图
    // 如果试图把小的放大了 就会黑糊糊
    if (coordination)
        painter->setTransform(*coordination, true);
    else
        painter->setTransform(defaultCoordination, true);

    //QPen pen;
    //QBrush brush;
    if (this->isSelected()){
        //painter->setPen(Qt::darkCyan);
        painter->setBrush(Qt::yellow);
    }
    else{
        //painter->setPen(Qt::black);
        painter->setBrush(Qt::NoBrush);
    }
    painter->drawEllipse(QPointF(0,0), _radius, _radius);
    painter->drawLine(QPointF(0,0), _radius * QPointF(cos(_heading), sin(_heading)));
}

GraphicsRobotItem::GraphicsRobotItem(QGraphicsItem *parent): QGraphicsItem(parent){

}

QRectF GraphicsRobotItem::boundingRect() const{
    qreal _radius = radius * (coordination ? coordination->UnitSize()
                                           : defaultCoordination.UnitSize());
    QRectF _boundingRect = {-_radius, -_radius, 2 * _radius, 2 * _radius};
    return coordination ? coordination->mapRect(_boundingRect) :
                          defaultCoordination.mapRect(_boundingRect);
}

void GraphicsRobotItem::setHeading(qreal _heading){
    heading = _heading;
}
qreal GraphicsRobotItem::Heading() const {
    return heading;
}

void GraphicsRobotItem::setRealPos(const QPointF& pos){
    qreal unit = coordination ? coordination->UnitSize()
                              : defaultCoordination.UnitSize();
    QPointF p2 = coordination ? coordination->map(pos) * unit
                              : defaultCoordination.map(pos) * unit;
    return this->setPos(p2);
}
void GraphicsRobotItem::setRealPos(qreal x, qreal y){
    return this->setRealPos(QPointF(x, y));
}
QPointF GraphicsRobotItem::RealPos() const{
    qreal unit = coordination ? coordination->UnitSize()
                              : defaultCoordination.UnitSize();
    return coordination ? coordination->inverted().map(this->pos()) / unit
                        : defaultCoordination.inverted().map(this->pos()) / unit;
}

void GraphicsRobotItem::setRadius(qreal _radius){
    if (_radius != radius){
        prepareGeometryChange();
        radius = _radius;
    }
}
qreal GraphicsRobotItem::Radius() const{
    return radius;
}

void GraphicsRobotItem::setCoordination(CoordinateSystem *trans){
    this->prepareGeometryChange();
    coordination = trans;
}
CoordinateSystem* GraphicsRobotItem::Coordination() const{
    return coordination;
}

void GraphicsRobotItem::restoreDefaultCoordiantionSystem(){
    if (coordination == nullptr) return;
    this->prepareGeometryChange();
    coordination = nullptr;
}
void GraphicsRobotItem::onPrepareCoordinateChange(CoordinateSystem* newCoordination){
    this->prepareGeometryChange();
    coordination = newCoordination;
}
