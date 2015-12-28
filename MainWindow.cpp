#include "MainWindow.h"
#include "ui_mainwindow.h"
#include <QGraphicsSceneMouseEvent>
#include <QWheelEvent>


#define GET_STR(x) #x
QString getModeString(SceneMode mode){
    if (mode == SceneMode::control_interface)
        return GET_STR(control_interface);
    if (mode == SceneMode::placing_robots)
        return GET_STR(placing_robots);
    if (mode == SceneMode::control_forTunnel)
        return GET_STR(control_forTunnel);
    return "";
}



/*
 * MainScene
 */
MainScene::MainScene(QObject *parent) :
    QGraphicsScene(parent)
{
    myCoordination = new CoordinateSystem(150);
    myCoordination->setAxisHedaing(AxisHeading::right, AxisHeading::up);
    myInvert = myCoordination->inverted();
    qreal ratio = 1.0 / myCoordination->UnitSize();
    myInvert.scale(ratio, ratio);
    mode_changing = false;
}

MainScene::~MainScene(){

}

pGraphicsRobotItem MainScene::getRobot(int id){
    auto it = data->IDtoIndex.find(id);
    if (it == data->IDtoIndex.end())
        return nullptr;
    size_t index = it->second;
    return data->robots[index].robot;
}
RobotItem* MainScene::getRobotItem(int id){
    auto it = data->IDtoIndex.find(id);
    if (it == data->IDtoIndex.end())
        return nullptr;
    size_t index = it->second;
    return &(data->robots[index]);
}
RobotItem* MainScene::getNearestRobot(QPointF ref){
    if (data->robots.size() == 0) return nullptr;

    auto distanceTo = [&](size_t k)->double{
        auto p = data->robots[k].robot->pos() - ref;
        return sqrt(p.x() * p.x() + p.y() * p.y());
    };

    size_t index = 0;
    double best = distanceTo(0);
    for (size_t i=1; i < data->robots.size(); ++i){
        double d = distanceTo(i);
        if (d < best){
            best = d;
            index = i;
        }
    }
    return &(data->robots[index]);
}
QPointF MainScene::positionOf(int id) const{
    size_t index = data->IDtoIndex.at(id);
    return data->robots[index].robot->RealPos();
}
qreal MainScene::headingOf(int id) const{
    size_t index = data->IDtoIndex.at(id);
    return data->robots[index].robot->Heading();
}

/*
 * SceneMode Change
 */

void MainScene::onSetupTunnel(double R, double r){
    if (traceLine){
        this->removeItem(traceLine);
    }

    // 画障碍物
    if (data->obstacles.empty())
        data->generateObstacles();

    addObstacles(data->obstacles);

    // 画轨道
    if (tunnelLines.empty()){
        QPen pen;
        QGraphicsItem* added_line;

#define __define_point(var, x, y) QPointF var##(x, y); \
        var = this->myCoordination->map(var) * this->myCoordination->UnitSize();

        __define_point(A1, -(R+r), -(R+r));
        __define_point(B1, -(R+r), (R+r));
        __define_point(A2, -(R-r), -(R-r));
        __define_point(B2, -(R-r), (R-r));
        __define_point(C1, (R+r), (R+r));
        __define_point(D1, (R+r), -(R+r));
        __define_point(C2, (R-r), (R-r));
        __define_point(D2, (R-r), -(R-r));


#define __kidding_me(pA, pB) do{\
        added_line = this->addLine(pA ## .x(), pA ##.y(), pB ##.x(), pB ##.y(), pen);\
        tunnelLines.push_back(added_line); \
    }while(0)

        __kidding_me(A1, B1);
        __kidding_me(A2, B2);
        __kidding_me(B1, C1);
        __kidding_me(B2, C2);
        __kidding_me(C1, D1);
        __kidding_me(C2, D2);
        __kidding_me(D1, A1);
        __kidding_me(D2, A2);
    }
}

void MainScene::setSceneMode(SceneMode modeNew){
    mode_changing = true;
    if (modeNew != mode){
        if (modeNew == SceneMode::placing_robots){
            if (userArrow.scene())
                this->removeItem(&userArrow);

            if (robotSelected != nullptr){
                robotSelected->robot->setHighlight(false);
            }
        }
        if (modeNew == SceneMode::control_forTunnel){
            // 跟control_interface做同样的事情
            for (auto& it : data->robots){
                it.robot->setFlag(QGraphicsItem::ItemIsMovable, false);
                it.robot->setHighlight(false);
            }
            mousePressed = false;
            robotSelected = nullptr;
        }
        mode = modeNew;

        update();   // 更新整个视图
        qDebug() << "Mode changed to " << getModeString(mode);
    }
    mode_changing = false;
}

void MainScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent){
    if (mode_changing) return;

    // 此时跟 control_interface 一样
    if (mode == SceneMode::control_forTunnel){
        if (mouseEvent->button() == Qt::LeftButton){
            // 找到被选中的Robot
            if (robotSelected != nullptr){
                robotSelected->robot->setHighlight(false);
            }
            robotSelected = getNearestRobot(mouseEvent->scenePos());

            if (robotSelected != nullptr){
                // 如果能找到才合法
                mousePressed = true;
                robotSelected->robot->setHighlight(true);

                // 设置User箭头
                userArrow.setEndpoint(0, 0);
                if (!userArrow.scene())
                    this->addItem(&userArrow);
                userArrow.setPos(mouseEvent->scenePos());
            }
        }
        mouseEvent->accept();
    }
    return QGraphicsScene::mousePressEvent(mouseEvent);
}

void MainScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent){
    if (mode_changing) return;
    if (mode == SceneMode::control_forTunnel && mousePressed){
        userArrow.setEndpoint(mouseEvent->scenePos() - userArrow.pos());
        if (robotSelected){
            robotSelected->set_userInputSlide(myInvert.map(userArrow.scenePos()),
                                                myInvert.map(userArrow.Endpoint()));
        }
        mouseEvent->accept();
    }
    else QGraphicsScene::mouseMoveEvent(mouseEvent);
}

/*
 * 产生两个userEvent的地方
 */
void MainScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent){
    if (mode_changing) return;
    if (mode == SceneMode::control_forTunnel){
         if (mouseEvent->button() == Qt::LeftButton){
             if (doubleClicked){
                 // 双击的时候跟control_interface一样
                 doubleClicked = false;
                 qDebug() << "double clicked";

                 if (robotSelected != nullptr){
                     robotSelected->set_userInput(UserInputType::doubleClick);
                     robotSelected->setHighlight(false);
                     robotSelected = nullptr;
                 }
             }
             else if (robotSelected != nullptr){
                 // 单击而放下的时候
                 qDebug() << QString("Releasing Control for Robot %1").arg(robotSelected->ID);
                 robotSelected->set_userInput(UserInputType::setFree);
                 robotSelected->setHighlight(false);
                 robotSelected = nullptr;
                 mousePressed = false;
                 if (userArrow.scene()){
                     this->removeItem(&userArrow);
                 }
             }
         }
    }
    else QGraphicsScene::mouseReleaseEvent(mouseEvent);
}

void MainScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event){
    if (mode_changing) return;

    if (event->button() == Qt::LeftButton){
        doubleClicked = true;
        event->accept();
    }
    QGraphicsScene::mouseDoubleClickEvent(event);
}

// 更新Robot位置于此
void MainScene::updateRobot(int ID, qreal x, qreal y, qreal heading){
    auto robot = this->getRobot(ID);
    if (robot != nullptr){
        robot->setRealPos(x, y);
        robot->setHeading(heading);
    }
    else{
        qDebug()<<"Invalid ID"<<ID;
    }
}

void MainScene::timerEvent(QTimerEvent *event){
    if (event->timerId() == this->myObstacleUpdateTimer){
        data->update_obstacle();
    }
}

/*
 * MainWindow
 */

MainWindow::MainWindow(QWidget *parent) :
    QGraphicsView(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow(){
    delete ui;
}
