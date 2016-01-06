#include "UserArrow.h"


UserArrow::UserArrow(QGraphicsItem *parent)
    :QGraphicsPathItem(parent){}
UserArrow::UserArrow(QPointF _end, QGraphicsItem *parent)
    :QGraphicsPathItem(parent), end(_end){}

void UserArrow::setEndpoint(qreal x, qreal y){
    return setEndpoint(QPointF(x, y));
}
void UserArrow::setEndpoint(QPointF _end){
    if (end != _end){
        this->prepareGeometryChange();
        end = _end;

        QPainterPath path;
        path.moveTo(0,0);
        path.lineTo(end);
        path.moveTo(end);
        path.addEllipse(end, endPointRadius, endPointRadius);
        this->setPath(path);
    }
}
QPointF UserArrow::Endpoint()const{
    return end;
}

/*
QRectF UserArrow::boundingRect() const{
    qreal x2 = (abs(end.x())+endPointRadius) + 1;   // 额外的margin
    qreal y2 = (abs(end.y())+endPointRadius) + 1;
    if (end.x()>=0 && end.y()>=0)
        return QRectF(0, 0, x2, y2);
    if (end.x()<0 && end.y()>=0)
        return QRectF(-x2, 0, 0, y2);
    if (end.x()>=0 && end.y()<0)
        return QRectF(0, -y2, x2, 0);
    return QRectF(-x2, -y2, 0, 0);
}

void UserArrow::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    painter->drawLine(QPointF(0,0), end);
    painter->drawEllipse(end, endPointRadius, endPointRadius);
}
*/
