#include "RobotInfoItem.h"
#include "..\RobotItem.h"

GraphicsRobotInfoItem::GraphicsRobotInfoItem(GraphicsRobotItem* parent):
    QGraphicsSimpleTextItem(parent)
{
    if (parent){
        //qreal r = parent->Radius();
        this->setPos(-35, 15);
    }
    this->setFlag(GraphicsItemFlag::ItemIsMovable);
}

void GraphicsRobotInfoItem::update(){
    if (parentItem()){
        QString s = QString("(%1, %2) - %3");
        GraphicsRobotItem* parent = (GraphicsRobotItem*)parentItem();
        auto pos = parent->RealPos();
        s = s.arg(pos.x(), 0, 'g', 4).arg(pos.y(), 0, 'g', 4).arg(parent->Heading());
        this->setText(s);
    }
}

void GraphicsRobotInfoItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    update();
    this->QGraphicsSimpleTextItem::paint(painter, option, widget);
}
