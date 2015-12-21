#ifndef GRAPHICSROBOTITEM_H
#define GRAPHICSROBOTITEM_H
#pragma warning(disable: 4819 4100)

#include "Util/Auxilary.h"
#include "Util/CoordinateSystem.h"
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

    void setCoordination(CoordinateSystem* trans);
    CoordinateSystem* Coordination() const;

public slots:
    void restoreDefaultCoordiantionSystem();
    void onPrepareCoordinateChange(CoordinateSystem* newCoordination);

protected:
    qreal heading = 90; // 按实际朝向度量，x轴始，逆时针为正
    qreal radius = 0.3; // 按实际长度度量

    CoordinateSystem* coordination = nullptr;
    static CoordinateSystem defaultCoordination;
};

using pGraphicsRobotItem = std::shared_ptr<GraphicsRobotItem>;




#endif // GRAPHICSROBOTITEM_H
