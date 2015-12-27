#include "ExpA.h"
#include <chrono>
using _cfg::section;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::steady_clock;
using std::chrono::duration_cast;


void ExpA::run(){
    // 实验设置
    data->tunnel_useTTT = use_TTT;

    // 准备开始
    ext->onSetCurrentConfig();
    data->resetRecords();

    // 画图
    qDebug()<<"1.5 seconds to start";
    emit prepare_start();
    if (this->isExercise)
        data->show_obstacles(1);
    else
        data->show_obstacles(0);

    std::this_thread::sleep_for(milliseconds(1500));

    emit start_experiment();
    s->setSceneMode(SceneMode::control_forTunnel);
    data->t_start = std::chrono::steady_clock::now();
    qDebug()<<"Experiment Started";

    // 实验中
    if (this->isExercise){
        emit new_autoStop(600000);
    }
    else{
        emit new_autoStop(duration_cast<milliseconds>(data->exp_time).count());
    }
}

void ExpA::prepare_stop(){
    data->t_end = steady_clock::now();
    s->setSceneMode(SceneMode::placing_robots);
    emit stop_experiment();
    qDebug()<<"Experiment stopped";

    // 事后：分析、存储数据
    if (!this->isExercise){
        if (use_TTT){
            data->saveExperimentData("expTTT");
        }
        else{
            data->saveExperimentData("expNon");
        }
        data->analyzeData();
    }

    if (this->isExercise)
        data->hide_obstacles(1);
    else
        data->hide_obstacles(0);

}
