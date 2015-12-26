#ifndef GRAPHICSROBOTITEM_H
#define GRAPHICSROBOTITEM_H
#pragma warning(disable: 4819 4100)

#include "Util/Auxilary.h"
#include "Util/CoordinateSystem.h"
#include "Util/RobotInfoItem.h"
#include <atomic>
#include <mutex>
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
    QPainterPath shape() const;

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
    qreal radius = 0.25; // 按实际长度度量

    CoordinateSystem* coordination = nullptr;
    static CoordinateSystem defaultCoordination;
};

using pGraphicsRobotItem = std::shared_ptr<GraphicsRobotItem>;


enum class UserInputType{
    none = 0,
    newSlide, doubleClick,
    setFree                 // 当User停止连续控制时（MouseUp）发送此
};

struct RobotItem{
    RobotItem(){}
    RobotItem(int _ID, pGraphicsRobotItem&& _robot, pGraphicsRobotInfoItem&& _info);

    // 因为有了个Mutex所以复制构造函数要重新写
    RobotItem::RobotItem(const RobotItem & it){
        ID = it.ID;
        robot = it.robot;
        info = it.info;
        user_input = it.user_input;
        u_start = it.u_start;
        u_dx = it.u_dx;
    }

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

    void set_userInput(UserInputType u_type){
        guard.lock();
        user_input = u_type;
        guard.unlock();
    }
    void set_userInputSlide(QPointF u_start, QPointF u_dx){
        guard.lock();
        user_input = UserInputType::newSlide;
        this->u_start = u_start;
        this->u_dx = u_dx;
        guard.unlock();
    }

    void get_userInput(UserInputType &type, QPointF& start, QPointF& dx){
        guard.lock();
        type = user_input;
        start = u_start;
        dx    = u_dx;
        guard.unlock();
    }

public:
    int ID = -1;
    pGraphicsRobotItem     robot = nullptr;
    pGraphicsRobotInfoItem info  = nullptr;

protected:
    std::mutex guard;
    UserInputType user_input;
    QPointF u_start, u_dx;
};


/*
 * RobotItem
 */
inline RobotItem::RobotItem(int _ID, pGraphicsRobotItem &&_robot, pGraphicsRobotInfoItem &&_info)
    : ID(_ID),robot(_robot), info(_info){}


#endif // GRAPHICSROBOTITEM_H
