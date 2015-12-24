#ifndef COREDATA_H
#define COREDATA_H

#include <mutex>
#include <atomic>
#include <chrono>
#include "basic/app-share.hpp"
#include "RobotItem.h"
#include "ObstacleItem.h"

struct StateRecord : public RobotStateData{
    StateRecord(){}
    StateRecord(const RobotStateData& dt) : RobotStateData(dt){}
    StateRecord(const RobotStateData& dt, std::chrono::milliseconds t)
        :RobotStateData(dt), t_now(t){}
    std::chrono::milliseconds t_now;
    int inside_obstacle = -1;
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

public:
    // 产生障碍物于obstacles中
    void generateObstacles();
    // 重置记录
    void resetRecords(){
        for (auto& rec : rec_state){
            rec.clear();
        }
        rec_state.resize(robots.size());
    }

    // 记录数据 检查有没有撞上障碍等
    void onNewRobotPositions(int ID, const RobotStateData& state);
    // 保存数据
    void saveExperimentData();
    // 分析数据
    void analyzeData();

    std::vector<pGraphicsRobotItem> robots;
    std::vector<ObstacleItem>       obstacles;

    // 记录实验过程
    std::vector<std::vector<StateRecord>> rec_state;
    std::chrono::steady_clock::time_point t_start;
    std::chrono::steady_clock::time_point t_end;

    // 其他的设置
    bool newTargetAsReplace = false;  // 新的UserTarget是按Replace还是按叠加
    bool useDefaultFromClient = true; // 当ResetConfig的时候 是否是从Client处获得(否则就按默认构造函数处理)
    bool useObstacles = false;         // 要不要显示障碍物
    bool showNearObOnly = false;       // 障碍物要不要只在靠近的时候显示
    float detect_range = 1;            // 如果动态显示障碍物的话，那么在距离多远的时候显示
};


inline CoreData::CoreData(){
    experiment_running = false;
    isPlacingRobot = false;
    isPlacingTrace = false;
}

#endif // COREDATA_H
