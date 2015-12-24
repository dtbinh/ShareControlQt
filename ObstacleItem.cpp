#include "ObstacleItem.h"
#include <QGraphicsEllipseItem>
#include <QGraphicsRectItem>
#include <cmath>

ObstacleItem::ObstacleItem()
{

}


// 暂时不考虑 (x竖, y横) 的情况
std::shared_ptr<QGraphicsItem> ObstacleItem::make_item(CoordinateSystem* xyz){
    // 在原始的坐标系里 x轴向右 y轴向下
    qreal unitX = !xyz ? 1 :
                   xyz->XHeading() == AxisHeading::right ? xyz->UnitSize() : -xyz->UnitSize();
    qreal unitY = !xyz ? 1 :
                   xyz->YHeading() == AxisHeading::down  ? xyz->UnitSize() : -xyz->UnitSize();

    if (shape == ObstacleShape::Circle){
        // 这是Bounding Rect 不是XXX
        pItem = std::shared_ptr<QGraphicsItem>(new QGraphicsEllipseItem(unitX*(x-rX), unitY*(y-rY), unitX*rX*2, unitY*rY*2));
    }
    else if (shape == ObstacleShape::Square){
        pItem = std::shared_ptr<QGraphicsItem>(new QGraphicsRectItem(unitX*(x-rX), unitY*(y-rY), unitX*rX*2, unitY*rY*2));
    }
    return pItem;
}

bool ObstacleItem::contains(double px, double py){
    if (shape == ObstacleShape::Circle){
        return ((px-x)*(px-x)/rX/rX + (py-y)*(py-y)/rY/rY < 1);
    }
    else if (shape == ObstacleShape::Square){
        return std::max(abs(px-x)/rY, abs(py-y)/rY) < 1;
    }
    return false;
}
