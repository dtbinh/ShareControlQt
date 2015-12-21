#ifndef COREDATA_H
#define COREDATA_H

#include <mutex>
#include <atomic>
#include "basic/app-share.hpp"
#include "RobotItem.h"



class CoreData
    :public BasicExperimentConfig
{
public:
    using atomic_bool = std::atomic_bool;
    CoreData();
    std::mutex      guard;

    // 这些都是当时的设置
    atomic_bool experiment_running;
    float Kusr_scr  = 2.5f;  // 2.5m时对应Kusr_base大小的干预量
    float Kusr_base = 10.0f;
    float decay_time = 2.0f;
    TargetType user_type = TargetType::velocity;

    // 其他的设置
    atomic_bool isPlacingTrace;
    atomic_bool isPlacingRobot;
    std::vector<int> addedUserTarget;

    // Used by
    pGraphicsRobotItem robot = nullptr;
};

inline CoreData::CoreData(){
    experiment_running = false;
    isPlacingRobot = false;
    isPlacingTrace = false;
}

#endif // COREDATA_H
