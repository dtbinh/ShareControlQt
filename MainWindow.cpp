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
    return "";
}

/*
 * RobotItem
 */
RobotItem::RobotItem(int _ID, pGraphicsRobotItem &&_robot, pGraphicsRobotInfoItem &&_info)
    : ID(_ID),robot(_robot), info(_info){}


/*
 * MainScene
 */
MainScene::MainScene(QObject *parent) :
    QGraphicsScene(parent)
{
    myCoordination = new CoordinateSystem(50);
    myCoordination->setAxisHedaing(AxisHeading::right, AxisHeading::up);
}

MainScene::~MainScene(){
    for (auto& item : robots){
        this->removeItem(item.robot.get());
        // 因为robot是info的parent
        // 所以删了robot后面的也被删了
        //this->removeItem(item.info.get());
    }
}

pGraphicsRobotItem MainScene::getRobot(int id){
    auto it = IDtoIndex.find(id);
    if (it == IDtoIndex.end())
        return nullptr;
    size_t index = it->second;
    return robots[index].robot;
}
RobotItem* MainScene::getRobotItem(int id){
    auto it = IDtoIndex.find(id);
    if (it == IDtoIndex.end())
        return nullptr;
    size_t index = it->second;
    return &robots[index];
}
RobotItem* MainScene::getNearestRobot(QPointF ref){
    if (robots.size() == 0) return nullptr;

    auto distanceTo = [&](size_t k)->double{
        auto p = robots[k].robot->pos() - ref;
        return p.x() * p.x() + p.y() * p.y();
    };

    size_t index = 0;
    double best = distanceTo(0);
    for (size_t i=1; i < robots.size(); ++i){
        double d = distanceTo(i);
        if (d < best){
            best = d;
            index = i;
        }
    }
    return &robots[index];
}
QPointF MainScene::positionOf(int id) const{
    size_t index = IDtoIndex.at(id);
    return robots[index].robot->RealPos();
}
qreal MainScene::headingOf(int id) const{
    size_t index = IDtoIndex.at(id);
    return robots[index].robot->Heading();
}

pGraphicsRobotItem MainScene::addRobot(int ID, qreal x, qreal y, qreal heading){
    auto it = IDtoIndex.find(ID);
    if (it != IDtoIndex.end())
        return nullptr;

    IDtoIndex[ID] = robots.size();
    auto item = new GraphicsRobotItem;
    item->setCoordination(myCoordination);
    item->setRealPos(x, y);
    item->setHeading(heading);
    item->setFlag(QGraphicsItem::ItemIsMovable);
    item->setFlag(QGraphicsItem::ItemIsSelectable);

    auto info = new GraphicsRobotInfoItem(item);
    this->addItem(item);
    this->addItem(info);
    robots.push_back({ID,
                      std::move(pGraphicsRobotItem(item)),
                      std::move(pGraphicsRobotInfoItem(info))});
    return robots.back().robot;
}

SceneMode MainScene::getSceneMode() const{
    return this->mode;
}

/*
 * SceneMode Change
 */

void MainScene::onSetupTunnel(double R, double r){
    if (traceLine){
        this->removeItem(traceLine);
    }
    if (tunnelLines.empty()){
        QPen pen;
        QGraphicsItem* added_line;

#define __define_point(var, x, y) QPointF var##(x, y); var = this->myCoordination->map(var) * this->myCoordination->UnitSize();

        __define_point(A1, -(R+r), -(R+r));
        __define_point(B1, -(R+r), (R+r));
        __define_point(A2, -(R-r), -(R-r));
        __define_point(B2, -(R-r), (R-r));
        __define_point(C1, (R+r), (R+r));
        __define_point(D1, (R+r), -(R+r));
        __define_point(C2, (R-r), (R-r));
        __define_point(D2, (R-r), -(R-r));

        /*QPointF A1( -(R+r), -(R+r)); A1 = this->myCoordination->map(A1) * this->myCoordination->UnitSize();
        QPointF B1( -(R+r), (R+r));  B1 = this->myCoordination->map(B1) * this->myCoordination->UnitSize();
        QPointF A2( -(R-r), -(R+r)); A2 = this->myCoordination->map(A2) * this->myCoordination->UnitSize();
        QPointF B2( -(R-r), (R-r));  B2 = this->myCoordination->map(B2) * this->myCoordination->UnitSize();
        QPointF C1( (R+r), (R+r));   C1 = this->myCoordination->map(C1) * this->myCoordination->UnitSize();
        QPointF D1( (R+r), -(R+r));  D1 = this->myCoordination->map(D1) * this->myCoordination->UnitSize();
        QPointF C2( (R-r), (R-r));   C2 = this->myCoordination->map(C2) * this->myCoordination->UnitSize();
        QPointF D2( (R-r), -(R+r));  D2 = this->myCoordination->map(D2) * this->myCoordination->UnitSize();
*/

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
    if (modeNew != mode){
        if (modeNew == SceneMode::control_interface){
            for (auto& it : robots)
                it.robot->setFlag(QGraphicsItem::ItemIsMovable, false);
            mousePressed = false;
        }
        if (modeNew == SceneMode::placing_robots){
            if (userArrow.scene())
                this->removeItem(&userArrow);

            if (robotSelected != nullptr)
                robotSelected->robot->setSelected(false);

            for (auto& it : robots)
                it.robot->setFlag(QGraphicsItem::ItemIsMovable, true);
        }
        mode = modeNew;
        qDebug() << "Mode changed to " << getModeString(mode);
    }
}

void MainScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent){
    if (mode == SceneMode::control_interface){
        if (mouseEvent->button() == Qt::LeftButton){
            // 找到被选中的Robot
            if (robotSelected != nullptr)
                robotSelected->robot->setSelected(false);
            robotSelected = getNearestRobot(mouseEvent->scenePos());
            if (robotSelected != nullptr){
                // 如果能找到才合法
                mousePressed = true;
                robotSelected->robot->setSelected(true);

                // 设置User箭头
                userArrow.setEndpoint(0, 0);
                if (!userArrow.scene())
                    this->addItem(&userArrow);
                userArrow.setPos(mouseEvent->scenePos());
            }
        }
    }
    return QGraphicsScene::mousePressEvent(mouseEvent);
}

void MainScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent){
    if (mode == SceneMode::control_interface && mousePressed){
        userArrow.setEndpoint(mouseEvent->scenePos() - userArrow.pos());
    }
    else QGraphicsScene::mouseMoveEvent(mouseEvent);
}

void MainScene::wheelEvent(QGraphicsSceneWheelEvent * wheelEvent){
//    qDebug()<<"Delta ="<<wheelEvent->delta();
/*   if (wheelEvent > 0){ // 放大
        qreal unit_old = this->myCoordination->UnitSize();
        this->myCoordination->setUnitSize(unit_old + 5);
        this->update();
    }
    else{ // 缩小
        qreal unit_old = this->myCoordination->UnitSize();
        this->myCoordination->setUnitSize(unit_old - 5);
        this->update();
    }
 */
}

void MainWindow::wheelEvent(QWheelEvent * event){
    //event->ignore();
    //if (scene())
    //    QApplication::sendEvent(scene(), event);
    QGraphicsView::wheelEvent(event);
}

/*
 * 产生两个userEvent的地方
 */

void MainScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent){
    if (mode == SceneMode::control_interface){
        if (mouseEvent->button() == Qt::LeftButton){
            if (doubleClicked){
                doubleClicked = false;
                qDebug() << "double clicked";

                if (robotSelected != nullptr){
                    emit this->newUserDoubleClick(robotSelected->ID);
                    robotSelected->robot->setSelected(false);
                    robotSelected = false;
                }
            }
            else{
                mousePressed = false;
            }
        }
        // 右键的时候才确定发送
        if (mouseEvent->button() == Qt::RightButton){
            if (userArrow.scene()){
                auto invert = myCoordination->inverted();
                this->removeItem(&userArrow);

                emit this->newUserSlideMotion(robotSelected->ID,
                                              invert.map(userArrow.scenePos()) / myCoordination->UnitSize(),
                                              invert.map(userArrow.Endpoint()) / myCoordination->UnitSize());

                if (this->isTraceMode){
                    if (traceLine != nullptr)  this->removeItem(traceLine);
                    QLineF line(userArrow.scenePos(), userArrow.scenePos() + userArrow.Endpoint());
                    line.setLength(500);
                    QPen linePen;
                    linePen.setStyle(Qt::DashLine);
                    linePen.setColor(QColor("red"));
                    traceLine = this->addLine(line, linePen);
                    qDebug()<<"is trace mode";
                }
                else
                    qDebug()<<"NOT trace mode";
                //qDebug()<<"Emitted "<<(invert.map(userArrow.scenePos()) / myCoordination->UnitSize())
                //                    <<(invert.map(userArrow.Endpoint())/ myCoordination->UnitSize());
            }
            if (robotSelected != nullptr){
                robotSelected->robot->setSelected(false);
                robotSelected = nullptr;
            }
        }
    }
    else QGraphicsScene::mouseReleaseEvent(mouseEvent);
}

void MainScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event){
    if (event->button() == Qt::LeftButton){
        doubleClicked = true;
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
