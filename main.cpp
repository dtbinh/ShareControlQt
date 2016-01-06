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
    QRectF rct(-400, -300, 800, 600);

    w.setScene(&s);
    w.setSceneRect(rct);
    w.setFixedSize(850, 650);
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

// The settings for physical experiment, 2016/1/5
void connect_robots_exp(ExternalControl& ext, MainScene& s, CoreData& data, float R){
    std::vector<client_data> client_list;
    client_list = {
        {"192.168.1.224", 1, 4455, 4456},
        {"192.168.1.245", 2, 4455, 4456}
    };
    ext.agent.me = client_data{"192.168.1.200", 0, 5544, 5545};
    ext.connect(client_list);

    bool show_info = false;
    data.add_robot(1, -R, -R, 90, show_info);
    data.add_robot(2,  R,  R, 90, show_info);
    s.addRobot(data.robots);
}

void load_configA(section& config, CoreData& data){
    data.experimentID = ExpUsing::expTunnel;
    data.tunnel_continuous = true;
    data.user_type = TargetType::position;

    // Set up environment
    data.tunnel_R = config.get<double>("tunnel_R");
    data.tunnel_r = config.get<double>("tunnel_r");

    int ob_type;
    if (config.get_exist("ob_type", ob_type) == true){
        qDebug()<<QString("Obstacle Type = %1").arg(ob_type);

        if (ob_type == 1) data.ob_using = CoreData::ObstacleSet::staticVersion;
        if (ob_type == 2) data.ob_using = CoreData::ObstacleSet::dynamicVersion;
        if (ob_type == 3){
            data.vision.myAddr   = "192.168.1.200";
            data.vision.hostAddr = "192.168.1.200";
            if (data.vision.init()){
                qDebug()<<"Cortex inited";
                if (data.vision.load_setting("..\\VisionSetting.txt")){
                    qDebug()<<"Cortex Setting Loaded";
                }
                else{
                    qDebug()<<"SomethingWrong Cortex";
                }
            }else{
                qDebug()<<"SomethingWrong Cortex";
            }
            data.ob_using = CoreData::ObstacleSet::cortexExperiment;
        }
    }

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
    ExpA expA(&s, &ext,  &pannel, &data, &config);

    qDebug()<<QString("R = %1, r = %2").arg( data.tunnel_R).arg( data.tunnel_r);
    s.onSetupTunnel(data.tunnel_R, data.tunnel_r);
    s.addObstacles(data.obstacles);

    if (data.ob_using == CoreData::ObstacleSet::cortexExperiment){
        qDebug()<<QString("Obstacle Num = %1").arg(data.obstacles.size());
        data.show_obstacles();
        s.beginUpdateObstacles();
    }

    // 连接、放置机器人
    // connect_robots(4, ext, s, data, data.tunnel_R);
    connect_robots_exp(ext, s, data, data.tunnel_R);

    // 连接各个部分
    QObject::connect(&ext, &ExternalControl::newPosition,
                     &s,   &MainScene::updateRobot);
    QObject::connect(&expA, &ExpA::start_experiment,
                     &ext,  &ExternalControl::onStartExperiment); // 向下发送开始命令 开始碰撞检测线程
    QObject::connect(&expA,  &ExpA::new_autoStop,
                     &pannel, &ControlNew::on_newAutoStop);       // 开始实验倒计时的Timer
    QObject::connect(&expA, &ExpA::prepare_start,
                     &s,    &MainScene::onPrepareStart);          // 把轨道和障碍都画出来, reset
    QObject::connect(&expA, &ExpA::start_experiment,
                     &s,    &MainScene::onExperimentStart);       // 开始obstacle移动线程 重置其初态
    QObject::connect(&pannel, &ControlNew::stop_exp,
                     &expA,   &ExpA::prepare_stop);               // expA开始停止实验
    QObject::connect(&expA, &ExpA::stop_experiment,
                     &ext,  &ExternalControl::onStopExperiment);  // 向下发送停止命令
    QObject::connect(&expA, &ExpA::stop_experiment,
                     &s,    &MainScene::onExperimentStop);        // 停止障碍物移动 障碍物隐藏


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
