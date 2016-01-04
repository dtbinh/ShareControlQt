#include "ExpA.h"
#include <chrono>
#include <QTextCodec>
#include <Shlwapi.h>
#include <iostream>
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

BOOL DirectoryExists(LPCTSTR szPath)
{
  DWORD dwAttrib = GetFileAttributes(szPath);

  return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
         (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

void ExpA::prepare_stop(){
    data->t_end = steady_clock::now();
    s->setSceneMode(SceneMode::placing_robots);
    emit stop_experiment();
    qDebug()<<"Experiment stopped";


    // 事后：分析、存储数据
    if (!this->isExercise){
        QString dir = c->getName();
        if (dir != "")
            CreateDirectory(dir.toStdWString().c_str(), nullptr);
        else
            dir = ".";  // 用当前路径
        if (use_TTT){
            if (!data->saveExperimentData(QString("%1\\expTTT").arg(dir))){
                qDebug()<<"Error Creating File for TTT, use default ";
                data->saveExperimentData(QString("expTTT"));
            }

        }
        else{
            if (!data->saveExperimentData(QString("%1\\expNon").arg(dir))){
                qDebug()<<"Error Creating File for TTT, use default ";
                data->saveExperimentData(QString("expNon"));
            }
        }
        data->analyzeData();
    }

    if (this->isExercise)
        data->hide_obstacles(1);
    else
        data->hide_obstacles(0);

}
