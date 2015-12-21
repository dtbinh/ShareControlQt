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
    QRectF rct(0,0, 1600, 1600);
    rct.moveCenter(QPointF(0, -400));

    w.setScene(&s);
    w.setSceneRect(rct);
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
    initMainWindow(w, s);

    s.isTraceMode = true;
    //ext.agent.me = client_data{"192.168.1.200", 0, 5544, 5545};
    //ext.connect({"192.168.1.224", 1, 4455, 4456});

    ext.agent.me = client_data{"127.0.0.1", 0, 5544, 5545};
    ext.connect({"127.0.0.1", 1, 4455, 4456});

    // Control Pannel 与 ExtControl的连接
    QObject::connect(&c,   &ControlPanel::setCurrentConfig,
                     &ext, &ExternalControl::onSetCurrentConfig);
    QObject::connect(&c,   &ControlPanel::setupTunnel,
                     &s,   &MainScene::onSetupTunnel);
    QObject::connect(&c,   &ControlPanel::clearTunnel,
                     &s,   &MainScene::onClearTunnel);
    QObject::connect(&c,   &ControlPanel::resetCurrentConfig,
                     &ext, &ExternalControl::onResetCurrentConfig);
    QObject::connect(&ext, &ExternalControl::configRefreshed,
                     [&](){
        c.refreshBasicConfig();
        s.updateRobot(1, data.robotStartState.x,
                         data.robotStartState.y,
                         data.robotStartState.heading);
        //s.update();
        qDebug()<<"reset to "<<data.robotStartState.x<<data.robotStartState.y;
    });
    QObject::connect(&c,  &ControlPanel::intoRobotMovingMode,
                     &s,  &MainScene::intoPlaceRobot);
    QObject::connect(&c,  &ControlPanel::intoTraceSettingMode,
                     &s,  &MainScene::intoControlInterface);

    // ExtControl 与 MainScene的连接
    QObject::connect(&ext, &ExternalControl::newPosition,
                     &s,   &MainScene::updateRobot);
    QObject::connect(&s,   &MainScene::newUserSlideMotion,
                     &ext, &ExternalControl::onSetNewTrace);

    // 设置模式->开始实验
    QObject::connect(&c,  &ControlPanel::startExperiment,
                     [&](){
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

        s.isTraceMode = false;
        data.addedUserTarget.clear();
        ext.isUpdating = true;
        ext.onStartExperiment();
        s.setSceneMode(SceneMode::control_interface);
    });

    // 实验中->设置模式
    QObject::connect(&c, &ControlPanel::stopExperiment,
                     [&](){
        ext.onStopExperiment();
        ext.isUpdating = false;
        s.isTraceMode = true;
        QObject::disconnect(&s, &MainScene::newUserDoubleClick, 0, 0);
        QObject::disconnect(&s, &MainScene::newUserSlideMotion, 0, 0);
        QObject::connect(&s,   &MainScene::newUserSlideMotion,
                         &ext, &ExternalControl::onSetNewTrace);

        if (data.isPlacingRobot)
            s.setSceneMode(SceneMode::placing_robots);
    });

    // Test
    data.robot = s.addRobot(1, 0, 0, 90);

    QObject::connect(&c, &ControlPanel::destroyed,
                     &w, &MainWindow::close);

    w.show();
    c.show();
    return a.exec();
}
