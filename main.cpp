#include "MainWindow.h"
#include "ControlPanel.h"
#include <QApplication>
#include <QPushButton>
#include <functional>
#include <exception>

#include "zmq.hpp"
#include "ExternalControl.h"

using std::vector;
using std::string;
using netAgent::Agent;
using netAgent::client_data;
using zmq::socket_t;
using zmq::context_t;
using zmq::message_t;

void initMainWindow(MainWindow& w, MainScene& s){
//    QRectF rct(0,0, 1600, 1600);
//    rct.moveCenter(QPointF(0, -400));
    QRectF rct(-400, -400, 800, 800);

    w.setScene(&s);
    w.setSceneRect(rct);
    w.setFixedSize(850, 850);
}

void onExperimentStart(MainWindow& w, MainScene&s, ControlPanel& c, ExternalControl& ext, CoreData& data){
    QObject::disconnect(&s,   &MainScene::newUserSlideMotion,
                        &ext, &ExternalControl::onSetNewTrace);

    if (data.experimentID == ExpUsing::expSwitch){
        QObject::connect(&s,   &MainScene::newUserDoubleClick,
                         &ext, &ExternalControl::onToAutonomous);
        QObject::connect(&s,   &MainScene::newUserSlideMotion,
                         &ext, &ExternalControl::onToTeleoperation);
    }
    if (data.experimentID == ExpUsing::expShared){
        QObject::connect(&s,   &MainScene::newUserDoubleClick,
                         &ext, &ExternalControl::onRemoveLastTarget);
        QObject::connect(&s,   &MainScene::newUserSlideMotion,
                         &ext, &ExternalControl::onNewUserTarget);
    }
    if (data.experimentID == ExpUsing::expTunnel){
        QObject::connect(&s,   &MainScene::newUserDoubleClick,
                         &ext, &ExternalControl::onRemoveAllTunnelTarget);
        QObject::connect(&s,   &MainScene::newUserSlideMotion,
                         &ext, &ExternalControl::onNewTunnelTarget);
    }
    if (data.experimentID == ExpUsing::expDirect){
        QObject::connect(&s,    &MainScene::newUserDoubleClick,
                         &ext,  &ExternalControl::onRemoveLastTarget);
        QObject::connect(&s,    &MainScene::newUserSlideMotion,
                         &ext,  &ExternalControl::onDirectBlend_NewCmd);
    }

    data.resetRecords();

    s.isTraceMode = false;
    ext.isUpdating = true;
    ext.onStartExperiment();

    if (data.tunnel_continuous){
        s.setSceneMode(SceneMode::control_forTunnel);
    }
    else{
        s.setSceneMode(SceneMode::control_interface);
    }


    int checking_T = int(data.mpcSystemDt * 1000 * 2 / 3 + 0.5);
    ext.myTimerID = ext.startTimer(checking_T);
    if (ext.myTimerID != 0)
        qDebug()<<QString("Experiment Started with checking cycle = %1 ms").arg(checking_T);
    else{
        qDebug()<<"Fail to set up timer";
    }

    data.t_start = std::chrono::steady_clock::now();
}


void onExperimentStop(MainWindow& w, MainScene&s, ControlPanel& c, ExternalControl& ext, CoreData& data){
    data.t_end = std::chrono::steady_clock::now();
    if (ext.myTimerID != 0)
        ext.killTimer(ext.myTimerID);

    ext.onStopExperiment();
    ext.isUpdating = false;
    s.isTraceMode = true;

    QObject::disconnect(&s, &MainScene::newUserDoubleClick, 0, 0);
    QObject::disconnect(&s, &MainScene::newUserSlideMotion, 0, 0);
    QObject::connect(&s,   &MainScene::newUserSlideMotion,
                     &ext, &ExternalControl::onSetNewTrace);

    if (data.isPlacingRobot)
        s.setSceneMode(SceneMode::placing_robots);

    // 分析、存储数据
    data.saveExperimentData();
    data.analyzeData();
    data.obstacles.clear();
}

void connectPannel(ControlPanel& c, ExternalControl& ext){
    QObject::connect(&c,   &ControlPanel::setCurrentConfig,
                     &ext, &ExternalControl::onSetCurrentConfig);
    QObject::connect(&c,   &ControlPanel::resetCurrentConfig,
                     &ext, &ExternalControl::onResetCurrentConfig);
    QObject::connect(&ext, &ExternalControl::configRefreshed,
                     &c,   &ControlPanel::refreshBasicConfig);
}

void connectPannel(ControlPanel &c, MainScene& s){
    QObject::connect(&c,   &ControlPanel::setupForTunnel,
                     &s,   &MainScene::onSetupTunnel);
    QObject::connect(&c,   &ControlPanel::clearForTunnel,
                     &s,   &MainScene::onClearTunnel);
    QObject::connect(&c,  &ControlPanel::intoRobotMovingMode,
                     &s,  &MainScene::intoPlaceRobot);
    QObject::connect(&c,  &ControlPanel::intoTraceSettingMode,
                     &s,  &MainScene::intoControlInterface);
}

void connectPannel(ExternalControl& ext, MainScene& s, CoreData& data){
    QObject::connect(&ext, &ExternalControl::configRefreshed,
                     [&](){
        s.updateRobot(1, data.robotStartState.x,
                         data.robotStartState.y,
                         data.robotStartState.heading);
        qDebug()<<"reset to "<<data.robotStartState.x<<data.robotStartState.y;
    });
    QObject::connect(&ext, &ExternalControl::newPosition,
                     &s,   &MainScene::updateRobot);
    QObject::connect(&s,   &MainScene::newUserSlideMotion,
                     &ext, &ExternalControl::onSetNewTrace);
}

void connect_robots(int n, ExternalControl& ext, MainScene& s, CoreData& data, bool show_info = false){
    std::vector<client_data> client_list;
    for (int i=0;i<n;++i){
        client_list.push_back(client_data{"127.0.0.1", i+1, 4455+2*i, 4456+2*i});
        //data.add_robot(i+1, 0, 0, 90, show_info);
    }
    ext.agent.me = client_data{"127.0.0.1", 0, 5544, 5545};
    ext.connect(client_list);

    if (n >=1) data.add_robot(1, 0, 0, 90, show_info);
    if (n >=2) data.add_robot(2, 0, 1, 90, show_info);
    if (n >=3) data.add_robot(3, 1, 1, 90, show_info);
    if (n >=4) data.add_robot(4, 1, 0, 90, show_info);
    s.addRobot(data.robots);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    MainScene  s;
    ControlPanel c;
    ExternalControl ext;
    CoreData data;

    s.data = &data;
    c.data = &data;
    ext.data = &data;

    // 功能配置：是否向Client求取默认设置
    data.useDefaultFromClient = true;
    // 功能配置：是否每次用ReplaceLastTarget的方式
    data.newTargetAsReplace   = true;
    // 功能配置：画布是否可以放置轨线
    s.isTraceMode = true;

    c.refreshBasicConfig(); // 根据data的数据更新界面
    initMainWindow(w, s);   // 设置画布大小等

    // Pannel之间的连接
    connectPannel(c, ext);
    connectPannel(c, s);
    connectPannel(ext, s, data);

    // 设置退出信号
    QObject::connect(&c,  &ControlPanel::programExit,
                     &w,  &MainWindow::close);
    QObject::connect(&c,  &ControlPanel::closeClients,
                     &ext,&ExternalControl::onCloseClients);
    QObject::connect(&w,  &MainWindow::programExit,
                     &c,  &ControlPanel::close);

    // 设置模式->开始实验
    QObject::connect(&c,  &ControlPanel::startExperiment,
                     [&](){ onExperimentStart(w, s, c, ext, data);});

    // 实验中->设置模式
    QObject::connect(&c, &ControlPanel::stopExperiment,
                     [&](){  onExperimentStop(w, s, c, ext, data);  });

    // 连接、放置机器人
    bool show_info = false;
    connect_robots(4, ext, s, data, show_info);

    /*
    data.add_robot(1, 0, 0, 90, show_info);
    data.add_robot(2, 0, 1, 90, show_info);
    data.add_robot(3, 1, 1, 90, show_info);
    data.add_robot(4, 1, 0, 90, show_info);
    s.addRobot(data.robots);
    */
    /*
    data.robots.push_back(s.addRobot(1, 0, 0, 90, show_info));
    data.robots.push_back(s.addRobot(2, 0, 1, 90, show_info));
    data.robots.push_back(s.addRobot(3, 1, 1, 90, show_info));
    data.robots.push_back(s.addRobot(4, 1, 0, 90, show_info));
    */
    // 测试：放一些障碍物....
    /*
    data.obstacles = { {2, 0, 1,  1, ObstacleShape::Circle},
                        {0, 0, 1, 0.5, ObstacleShape::Square},
                        {0, 2, 0.5, 1, ObstacleShape::Circle}
                      };
    //
    //s.addObstacles(data.obstacles);
    //data.obstacles[0].pItem->highlight = true;
    //data.obstacles[2].pItem->highlight = true;
    //s.update();
    */

    w.show();
    c.show();
    return a.exec();
}
