#ifndef GRAPHICSROBOTITEM_H
#define GRAPHICSROBOTITEM_H
#pragma warning(disable: 4819 4100)

#include "Util/Auxilary.h"
#include "Util/CoordinateSystem.h"
#include "Util/RobotInfoItem.h"
#include <atomic>
/*
 * 说明：
 *     通过setHeading/Heading, setRealPos/RealPos
 *     设置/读取该机器人对应的真实朝向、坐标
 */
class GraphicsRobotItem
        :public QGraphicsItem
{
public:
    explicit GraphicsRobotItem(QGraphicsItem *parent = 0);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);


    void setHeading(qreal _heading);
    qreal Heading() const;

    void setRealPos(qreal x, qreal y);
    void setRealPos(const QPointF& pos);
    QPointF RealPos() const;

    void setRadius(qreal _radius);
    qreal Radius() const;

    void setHighlight(bool highlight = true){
        this->highlight =highlight;
    }
    bool isHighlighted() const{
        return highlight;
    }

    void setCoordination(CoordinateSystem* trans);
    CoordinateSystem* Coordination() const;

public slots:
    void restoreDefaultCoordiantionSystem();
    void onPrepareCoordinateChange(CoordinateSystem* newCoordination);

protected:
    bool  highlight = false;
    qreal heading = 90; // 按实际朝向度量，x轴始，逆时针为正
    qreal radius = 0.3; // 按实际长度度量

    CoordinateSystem* coordination = nullptr;
    static CoordinateSystem defaultCoordination;
};

using pGraphicsRobotItem = std::shared_ptr<GraphicsRobotItem>;

struct RobotItem{
    RobotItem(){}
    RobotItem(int _ID, pGraphicsRobotItem&& _robot, pGraphicsRobotInfoItem&& _info);

    // 一些兼容GraphicsRobotItem的接口

    void setHeading(qreal _heading){
        return robot->setHeading(_heading);
    }
    qreal Heading() const{
        return robot->Heading();
    }

    void setRealPos(qreal x, qreal y){
        return robot->setRealPos(x, y);
    }
    void setRealPos(const QPointF& pos){
        return robot->setRealPos(pos);
    }
    QPointF RealPos() const{
        return robot->RealPos();
    }

    void setRadius(qreal _radius){
        return robot->setRadius(_radius);
    }
    qreal Radius() const{
        return robot->Radius();
    }

    void setHighlight(bool highlight = true){
        return robot->setHighlight(highlight);
    }
    bool isHighlighted() const{
        return robot->isHighlighted();
    }

    void setCoordination(CoordinateSystem* trans){
        return robot->setCoordination(trans);
    }
    CoordinateSystem* Coordination() const{
        return robot->Coordination();
    }

    int ID = -1;
    pGraphicsRobotItem     robot = nullptr;
    pGraphicsRobotInfoItem info  = nullptr;
};




#endif // GRAPHICSROBOTITEM_H
