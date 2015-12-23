#ifndef COREDATA_H
#define COREDATA_H

#include <mutex>
#include <atomic>
#include <chrono>
#include "basic/app-share.hpp"
#include "RobotItem.h"

struct StateRecord : public RobotStateData{
    StateRecord(){}
    StateRecord(const RobotStateData& dt) : RobotStateData(dt){}
    std::chrono::milliseconds t_now;
};

struct SharedExpCommandRecord{
    std::chrono::milliseconds t_now;

};

class CoreData
    :public BasicExperimentConfig
{
public:
    using atomic_bool = std::atomic_bool;
    CoreData();
    std::mutex      guard;

    // 这些都是运行中的设置
    atomic_bool experiment_running;
    float Kusr_scr  = 2.5f;  // 2.5m时对应Kusr_base大小的干预量
    float Kusr_base = 10.0f;
    float decay_time = 2.0f;
    TargetType user_type = TargetType::velocity;

    // 其他的设置
    atomic_bool isPlacingTrace;
    atomic_bool isPlacingRobot;
    //std::vector<int> addedUserTarget;

    // Used by
    //pGraphicsRobotItem robot = nullptr;         // 存放MainScene里Add之后的Roobot, 用于在需要的时候读取其屏幕位置
    std::vector<pGraphicsRobotItem> robots;

    // 记录实验过程
    std::vector<std::vector<StateRecord>> rec_state;
    std::chrono::steady_clock::time_point t_start;
    std::chrono::steady_clock::time_point t_end;

    // 其他的设置
    bool newTargetAsReplace = false;  // 新的UserTarget是按Replace还是按叠加
    bool useDefaultFromClient = true; // 当ResetConfig的时候 是否是从Client处获得(否则就按默认构造函数处理)
};


inline CoreData::CoreData(){
    experiment_running = false;
    isPlacingRobot = false;
    isPlacingTrace = false;
}

#endif // COREDATA_H
