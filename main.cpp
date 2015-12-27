#include "MainWindow.h"
#include "ControlPanel.h"
#include "ControlNew.h"
#include "ExpA.h"
#include <QApplication>
#include <QPushButton>
#include <QThread>
#include <functional>
#include <exception>
#include <config.hpp>
#include <QObject>

#include "zmq.hpp"
#include "ExternalControl.h"

using std::vector;
using std::string;
using netAgent::Agent;
using netAgent::client_data;
using zmq::socket_t;
using zmq::context_t;
using zmq::message_t;
using _cfg::section;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::steady_clock;
using std::chrono::duration_cast;

void initMainWindow(MainWindow& w, MainScene& s){
    QRectF rct(-400, -400, 800, 800);

    w.setScene(&s);
    w.setSceneRect(rct);
    w.setFixedSize(850, 850);
}
void connect_robots(int n, ExternalControl& ext, MainScene& s, CoreData& data, float R){
    std::vector<client_data> client_list;
    for (int i=0;i<n;++i){
        client_list.push_back(client_data{"127.0.0.1", i+1, 4455+2*i, 4456+2*i});
        //data.add_robot(i+1, 0, 0, 90, show_info);
    }
    ext.agent.me = client_data{"127.0.0.1", 0, 5544, 5545};
    ext.connect(client_list);

    bool show_info = false;
    if (n >=1) data.add_robot(1, -R, -R, 90, show_info);
    if (n >=2) data.add_robot(2, -R,  R, 90, show_info);
    if (n >=3) data.add_robot(3,  R,  R, 90, show_info);
    if (n >=4) data.add_robot(4,  R, -R, 90, show_info);
    s.addRobot(data.robots);
}

void load_configA(section& config, CoreData& data){
    data.experimentID = ExpUsing::expTunnel;
    data.tunnel_continuous = true;
    data.user_type = TargetType::position;

    // Set up environment
    data.tunnel_R = config.get<double>("tunnel_R");
    data.tunnel_r = config.get<double>("tunnel_r");
    data.generateObstacles();
    qDebug()<<QString("R = %1, r = %2").arg( data.tunnel_R).arg( data.tunnel_r);

    // Load data for experimentA
    section expA = config.sub("experiment-1");
    data.exp_time = seconds(expA.get<int>("time"));

    int robot_type;
    if (expA.get_exist("robot_type", robot_type) == true){
        if (robot_type == 1) data.robot_type = RobotType::direct;
        if (robot_type == 2) data.robot_type = RobotType::ideal;
    }

    data.mpcWindowsTime = expA.get<float>("win_size");
    data.Kusr_base     = expA.get<float>("Kusr");
    data.decay_time    = expA.get<float>("decay");
    qDebug()<<"Config Loaded";
}


int main(int argc, char *argv[]){
    QApplication a(argc, argv);
    MainWindow w;
    MainScene  s;
    ControlNew pannel;
    ExternalControl ext;
    CoreData data;
    section config;
    config.load("..\\ExperimentConfig.txt");

    s.data = &data;
    ext.data = &data;
    initMainWindow(w, s);   // 设置画布大小等

    w.show();
    pannel.show();

    qDebug()<<"Loading Config";
    load_configA(config, data);
    ExpA expA(&s, &ext, &data, &config);

    qDebug()<<QString("R = %1, r = %2").arg( data.tunnel_R).arg( data.tunnel_r);
    s.onSetupTunnel(data.tunnel_R, data.tunnel_r);
    s.addObstacles(data.obstacles);

    // 连接、放置机器人
    connect_robots(4, ext, s, data, data.tunnel_R);

    // 连接各个部分
    QObject::connect(&ext, &ExternalControl::newPosition,
                     &s,   &MainScene::updateRobot);
    QObject::connect(&expA, &ExpA::start_experiment,
                     &ext,  &ExternalControl::onStartExperiment);
    QObject::connect(&expA, &ExpA::prepare_start,
                     &s,    &MainScene::onPrepareStart);
    QObject::connect(&expA, &ExpA::stop_experiment,
                     &ext,  &ExternalControl::onStopExperiment);
    QObject::connect(&pannel, &ControlNew::stop_exp,
                     &expA, &ExpA::prepare_stop);
    QObject::connect(&expA,  &ExpA::new_autoStop,
                     &pannel, &ControlNew::on_newAutoStop);

    // 退出信号
    QObject::connect(&pannel, &ControlNew::program_exit,
                     &w,      &MainWindow::close);
    QObject::connect(&w,     &MainWindow::programExit,
                     &pannel, &ControlNew::close);

    // 开始实验的信号
    QObject::connect(&pannel, &ControlNew::start_exp1,
                     [&expA](bool isExercise){
        expA.use_TTT = true;
        expA.isExercise = isExercise;
        expA.start();
    });
    QObject::connect(&pannel, &ControlNew::start_exp2,
                     [&expA](bool isExercise){
        expA.use_TTT = false;
        expA.isExercise = isExercise;
        expA.start();
    });

    return a.exec();
}
