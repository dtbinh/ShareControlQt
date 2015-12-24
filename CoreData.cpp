#include "CoreData.h"
#include <QDebug>
#include <QTextStream>
#include <QFile>


void CoreData::add_robot(int ID, qreal x, qreal y, qreal heading, bool show_info){
    auto it = IDtoIndex.find(ID);
    if (it != IDtoIndex.end()){
        qDebug()<<"Robot already added";
        return;
    }

    IDtoIndex[ID] = robots.size();
    auto item = new GraphicsRobotItem;

    item->setRealPos(x, y);
    item->setHeading(heading);
    item->setFlag(QGraphicsItem::ItemIsMovable);
    item->setFlag(QGraphicsItem::ItemIsSelectable);

    auto info = new GraphicsRobotInfoItem(item);
    if (!show_info)
        info->hide();

    robots.push_back({ID,
                      std::move(pGraphicsRobotItem(item)),
                      std::move(pGraphicsRobotInfoItem(info))});
}

void CoreData::generateObstacles(){
    // 产生针对于Tunnel实验的障碍物
    if (experimentID == ExpUsing::expTunnel){
        obstacles.clear();
        double R = tunnel_R;
        double r = tunnel_r;
        double dr = 0.2;

        obstacles = { {-(R+r/2-dr), 0, r/2,  r/2, ObstacleShape::Circle},
                       {-R/2, (R+r/2-dr), r/2, r/2, ObstacleShape::Circle},
                       {R/2, (R+r/2-dr), r/2, r/2, ObstacleShape::Circle},
                       {R, 0, r/4, r/4, ObstacleShape::Circle},
                       //{R,  0, r/4, R, ObstacleShape::Square},
                       {R/2, -(R+r/2-dr), r/2, r/2, ObstacleShape::Circle},
                       {-R/2, -(R-r/2+dr), r/2, r/2, ObstacleShape::Circle},
                      };
    }
}

void CoreData::onNewRobotPositions(int ID, const RobotStateData& state){
    using std::chrono::steady_clock;
    using std::chrono::milliseconds;
    using std::chrono::duration_cast;
    auto tnow = std::chrono::steady_clock::now();

    size_t index = ID - 1;
    if (index >= rec_state.size()){
        qDebug()<<QString("ID and storage size mismatch");
        return;
    }

    rec_state[index].push_back(state);
    rec_state[index].back().t_now = duration_cast<milliseconds>(tnow - t_start);
    rec_state[index].back().inside_obstacle = -1;

    for (size_t i =0;i < obstacles.size();++i){
        if (obstacles[i].contains(state.x, state.y)){
            rec_state[index].back().inside_obstacle = i;
            break;  // 不会在两个Obstacle里面的
        }
    }
}

void CoreData::saveExperimentData(){
    if (!rec_state.empty() && !rec_state.front().empty()){
        qDebug()<<QString("total records : %1").arg(rec_state[0].size());
        qDebug()<<QString("total time    : %1 ms").arg(std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count());

        for (size_t robot_index = 0; robot_index < rec_state.size(); ++robot_index){
            QString fname = QString("expData-%1.txt").arg(robot_index);
            QFile file(fname);
            if (!file.open(QIODevice::WriteOnly)){
                qDebug()<<"Failed to open file";
                return;
            }
            QTextStream out(&file);
            auto& rec = rec_state[robot_index];
            for (size_t i=0;i<rec.size();++i){
                out << QString("%1  %2  %3  %4  %5  %6")
                       .arg(rec[i].t_now.count(), -4)
                       .arg(rec[i].x, -5, 'f', 5)
                       .arg(rec[i].y, -5, 'f', 5)
                       .arg(rec[i].heading)
                       .arg(rec[i].velocity, -5, 'f', 5)
                       .arg(rec[i].inside_obstacle+1);
                out << endl;
            }
            out.flush();
        }
    }
}

void CoreData::analyzeData(){
    using std::chrono::milliseconds;
    using std::chrono::duration_cast;
    qDebug()<<"Inside";
    try{
        if (!rec_state.empty()){
            milliseconds              time_all;    // 实验总时间
            //std::vector<milliseconds> time_good;   // 该Robot没有撞到障碍物的时间
            std::vector<milliseconds> time_bad;    // 该Robot撞到障碍物的时间(用来做Double Check)
            //time_good.resize(rec_state.size());
            time_bad.resize(rec_state.size());

            time_all = duration_cast<milliseconds>(t_end - t_start);

            for (size_t robot_index = 0;robot_index < rec_state.size(); ++robot_index){

                std::vector<StateRecord>& rec = rec_state[robot_index];

                /*
                milliseconds first_good = milliseconds::zero() ; // 当前一连续good中的第一时刻
                bool former_good = true;     // 前一时刻是不是好的
                time_good[robot_index] = milliseconds::zero();  // 0初始化

                for(size_t k=0; k < rec.size() && rec[k].t_now < time_all; ++k){
                    if (rec[k].inside_obstacle == -1){
                        if (!former_good){
                            // 这时候不在Obstacle里面 但前一时刻在
                            first_good = rec[k].t_now;
                            former_good = true;
                        }
                    }
                    else if (former_good){
                       // 现在在Obstacle里但上一时刻还好好的
                       time_good[robot_index] += duration_cast<milliseconds>(rec[k].t_now - first_good);
                       former_good = false;
                    }
                }
                // 结束的时候 如果最后一段是good 就加进去
                if (former_good){
                    time_good[robot_index] += duration_cast<milliseconds>(time_all - first_good);
                }
                */

                // 上方函数的镜像
                // 用来做DoubleCheck
                milliseconds first_bad = milliseconds::zero(); // 当前一连续good中的第一时刻
                bool former_bad = false;   // 前一时刻是不是坏的
                time_bad[robot_index] = milliseconds::zero();  // 0初始化

                for(size_t k=0; k < rec.size() && rec[k].t_now < time_all; ++k){
                    if (rec[k].inside_obstacle != -1){
                        if (!former_bad){
                            // 这时候在Obstacle里面 但前一时刻不在
                            first_bad = rec[k].t_now;
                            former_bad = true;
                        }
                    }
                    else if (former_bad){
                       // 现在在Obstacle里但上一时刻还好好的
                       time_bad[robot_index] += duration_cast<milliseconds>(rec[k].t_now - first_bad);
                       former_bad = false;
                    }
                }
                // 结束的时候 如果最后一段是good 就加进去
                if (former_bad){
                    time_bad[robot_index] += duration_cast<milliseconds>(time_all - first_bad);
                }


                // 输出结果
                qDebug()<<QString("For Robot %1, time inside obstacle = %2 ms / %3 ms")
                          .arg(robot_index+1)
                          .arg(time_bad[robot_index].count())
                          .arg(time_all.count());
            }
        }
    }
    catch (std::exception e){
        qDebug()<<"Error : "<<e.what();
    }
    catch (...){
        qDebug()<<"Error : unexpected...";
    }
}
