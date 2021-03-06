#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#pragma warning(disable: 4819 4100)

#include "Util/Auxilary.h"
#include "Util/CoordinateSystem.h"
#include "RobotItem.h"
#include "Util/RobotInfoItem.h"
#include "Util/UserArrow.h"
#include "CoreData.h"
#include "ObstacleItem.h"


enum class SceneMode{
    control_interface,  // 发送User箭头，自动确定离哪个robot最近 不能移动
    control_forTunnel,  // 给Tunnel实验的控制模式
    placing_robots      // 可以移动机器人, 但并不发送User箭头
};

class MainScene  : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit MainScene(QObject *parent = 0);
    ~MainScene();

    SceneMode getSceneMode() const;

    pGraphicsRobotItem getRobot(int ID);
    RobotItem*         getNearestRobot(QPointF ref);
    RobotItem*         getRobotItem(int ID);

    QPointF positionOf(int ID) const;
    qreal   headingOf(int ID) const;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent * mouseEvent);
    void mouseMoveEvent(QGraphicsSceneMouseEvent * mouseEvent);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent * mouseEvent);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void wheelEvent(QGraphicsSceneWheelEvent * wheelEvent);

signals:
    void newUserSlideMotion(int robotID, QPointF start, QPointF dx);
    void newUserDoubleClick(int robotID);

public slots:
    // 添加新的Robot
    // pGraphicsRobotItem addRobot(int ID, qreal x = 0, qreal y = 0, qreal heading = 90, bool show_info = true);

    // 设置Robot位置
    void updateRobot(int robotID, qreal x, qreal y, qreal heading);

    // 改变当前模式
    void setSceneMode(SceneMode _mode);
    void intoPlaceRobot()       {setSceneMode(SceneMode::placing_robots);}
    void intoControlInterface() {setSceneMode(SceneMode::control_interface);}

    // 把Tunnel画上去\擦掉
    void onSetupTunnel(double R, double r);
    void onClearTunnel(){
        for (auto& item : tunnelLines)
            this->removeItem(item);
    }

public:
    // 画障碍物
    void addObstacles(std::vector<ObstacleItem>& obstacles){
        for (size_t i=0;i<obstacles.size();++i){
            addObstacles(obstacles[i]);
            obstacles[i].pItem->ID = i + 1;
            //obstacles[i].pItem->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
        }
    }
    void addObstacles(ObstacleItem& obstacle){
        this->addItem(obstacle.make_item(myCoordination));
    }

    void addRobot(RobotItem& robot){
        robot.setCoordination(myCoordination);  // 用新坐标系
        this->addItem(robot.robot.get());
       // this->addItem(robot.info.get());
    }
    void addRobot(std::vector<RobotItem>& robots){
        for (auto& rbt : robots){
            addRobot(rbt);
        }
    }

public:
    bool mousePressed = false;
    bool doubleClicked = false;
    bool isTraceMode = false;
    QGraphicsItem* traceLine = nullptr;
    std::vector<QGraphicsItem*> tunnelLines;

    UserArrow userArrow;
    RobotItem* robotSelected = nullptr;

    SceneMode mode = SceneMode::placing_robots;
    CoordinateSystem* myCoordination = nullptr;
    QTransform myInvert;

    CoreData* data = nullptr;
};


namespace Ui {
class MainWindow;
}

class MainWindow : public QGraphicsView
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void programExit();

protected:
    void wheelEvent(QWheelEvent * event);
    void closeEvent(QCloseEvent * event){
        emit programExit();
    }

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
