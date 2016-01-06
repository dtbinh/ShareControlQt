#include "RobotItem.h"


CoordinateSystem GraphicsRobotItem::defaultCoordination = CoordinateSystem(100);

QPainterPath GraphicsRobotItem::shape() const{
    QPainterPath path;
    path.addEllipse(boundingRect());
    return path;
}

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
    //this->isSelected() ||
    if (this->highlight){
        painter->setPen(QPen(QColor(0, 0, 255, 200), 2));
    }
    else{
        painter->setPen(Qt::black);
        //painter->setBrush(Qt::NoBrush);
    }
    painter->setBackgroundMode(Qt::TransparentMode);
    painter->drawEllipse(QPointF(0,0), _radius, _radius);
    painter->drawLine(QPointF(0,0), _radius * QPointF(cos(_heading), sin(_heading)));
}

GraphicsRobotItem::GraphicsRobotItem(QGraphicsItem *parent): QGraphicsItem(parent){

}

QRectF GraphicsRobotItem::boundingRect() const{
    qreal _radius = radius * (coordination ? coordination->UnitSize()
                                           : defaultCoordination.UnitSize());
    QRectF _boundingRect = {-_radius*1.1, -_radius*1.1, 2.2 * _radius, 2.2 * _radius};
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
    this->setPos(p2);
    return;
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
    qreal   real_heading = this->Heading();
    QPointF real_pos = this->RealPos(); // 记录当前的真实位置
    this->prepareGeometryChange();      // 通知画布准备坐标变换
    coordination = trans;               // 更新坐标系
    this->setRealPos(real_pos);         // 保证真实坐标不变
    this->setHeading(real_heading);     // 保证真实姿态不变
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
