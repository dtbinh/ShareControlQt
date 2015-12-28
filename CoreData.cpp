#include "CoreData.h"
#include <QDebug>
#include <QTextStream>
#include <QFile>

// 静态类型声明
std::vector<ObstacleLineData> ObstacleMotionData::line;

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

void CoreData::reset_robot_positions(){
    qreal R = tunnel_R;
    size_t i;
    i=0;
    if (robots.size() > i) {
        robots[i].setRealPos(-R, -R);
        robots[i].setHeading(90);
    }
    i=1;
    if (robots.size() > i) {
        robots[i].setRealPos(-R, R);
        robots[i].setHeading(0);
    }
    i=2;
    if (robots.size() > i) {
        robots[i].setRealPos(R, R);
        robots[i].setHeading(-90);  // 似乎-90会有bug?
    }
    i=3;
    if (robots.size() > i) {
        robots[i].setRealPos(R, -R);
        robots[i].setHeading(180);
    }
}

void CoreData::reset_obstacle_position(){
    using std::chrono::milliseconds;
    if (ob_using == ObstacleSet::dynamicVersion){
        auto t0 = std::chrono::steady_clock::now();
        auto& line = ObstacleMotionData::line;
        ob_motion[0].t0 = t0 - line[ob_motion[0].lineID].T / 2;
        ob_motion[1].t0 = t0 - line[ob_motion[1].lineID].T / 4;
        ob_motion[2].t0 = t0 - line[ob_motion[2].lineID].T / 2;
        ob_motion[3].t0 = t0 - line[ob_motion[3].lineID].T / 4;
    }
}

void CoreData::generateObstacles(){
    if (ob_using == ObstacleSet::staticVersion){
        obstacles.clear();
        double R = tunnel_R;
        double r = tunnel_r;
        double r2 = r * 1.2;
        double dr = 0;

        ObstacleShape shape = ObstacleShape::Circle;
        //ObstacleShape shape = ObstacleShape::Square;
        obstacles = { {-(R+r-dr), 0, r2,  r2, shape},
                       {-R/2, (R+r-dr), r2, r2, shape},
                       {R/2, (R+r-dr), r2, r2, shape},
                       {R, 0, r2/2, r2/2, shape},
                       //{R,  0, r/4, R, ObstacleShape::Square},
                       {R/2, -(R+r-dr), r2, r2, shape},
                       {-R/2, -(R-r+dr), r2, r2, shape},
                      };
    }

    if (ob_using == ObstacleSet::dynamicVersion){
        obstacles.clear();
        ObstacleMotionData::line.clear();

        double R = tunnel_R;
        double r = tunnel_r;
        double r2 = r / 3;
        ObstacleShape shape = ObstacleShape::Circle;

        // 定义障碍物的轨道信息
        double v0 = this->standardVelocity / 1000.0; // 原先是mm/s 变成m/s
        int T = int(2*R/v0 * 1000 + 0.5); // 与小车速度相同
        ObstacleMotionData::line = {
            {3, -R, R, T}, // this is 0, next is 3, anti-clockwise
            {2, R, R, T},
            {1, R, -R, T},
            {0, -R, -R, T}
        };

        // 定义障碍物
        int total_num = 4;
        obstacles.resize(total_num);
        ob_motion.resize(total_num);
        ob_move = {0, 1, 2, 3};

        double amplitude = r/6;  // 振动幅值
        int    n_cycle = 2;     // 每条线上面振多少圈
        const double pi = 3.14159265358979324;
        double w = 2*pi / (T/1000.0) * n_cycle; // 即每一圈的周期是T / n_cycle
                                                // T之前是ms单位的

        for (int i=0;i<4;++i){
            obstacles[i] = {0,0, r2,r2,shape}; // 坐标不重要 会由之后的位置决定
            ob_motion[i].lineID = i;
            ob_motion[i].amplitude = amplitude;
            ob_motion[i].line_spd = v0;
            ob_motion[i].angl_spd = w;
        }
    }
}

void CoreData::update_obstacle(){
    using std::chrono::milliseconds;
    using std::chrono::seconds;
    using std::chrono::duration_cast;
    using std::chrono::steady_clock;
    auto& line = ObstacleMotionData::line;

    if (ob_using == ObstacleSet::dynamicVersion){
        for (int i : ob_move){
            if (!obstacles[i].pItem->isVisible())
                continue;
            int& lineID = ob_motion[i].lineID;
            auto t_now = steady_clock::now();
            milliseconds t = duration_cast<milliseconds>(t_now - ob_motion[i].t0);
            if (t > line[lineID].T){
                // 这条线跑完了
                t = t - line[lineID].T;
                ob_motion[i].t0 = t_now - t;
                lineID = line[lineID].line_next;
            }
            qreal x = line[lineID].start_point.x();
            qreal y = line[lineID].start_point.y();
            if (lineID == 0){
                y -= ob_motion[i].line_spd * (t.count() / 1000.0);
                x += ob_motion[i].amplitude * sin(ob_motion[i].angl_spd * (t.count() /1000.0));
            }
            else if (lineID == 1){
                x -= ob_motion[i].line_spd * (t.count() / 1000.0);
                y += ob_motion[i].amplitude * sin(ob_motion[i].angl_spd * (t.count() /1000.0));
            }
            else if (lineID == 2){
                y += ob_motion[i].line_spd * (t.count() / 1000.0);
                x += ob_motion[i].amplitude * sin(ob_motion[i].angl_spd * (t.count() /1000.0));
            }
            else if (lineID == 3){
                x += ob_motion[i].line_spd * (t.count() / 1000.0);
                y += ob_motion[i].amplitude * sin(ob_motion[i].angl_spd * (t.count() /1000.0));
            }
            else{
                qDebug()<<"Invaild lineID for obstacle update...";
            }
            obstacles[i].setRealPos(x, y);
        }
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

    int last_inside = -1, this_inside = -1;
    if (!rec_state[index].empty()){
         last_inside = rec_state[index].back().inside_obstacle;
    }

    rec_state[index].push_back(state);
    rec_state[index].back().t_now = duration_cast<milliseconds>(tnow - t_start);
    rec_state[index].back().inside_obstacle = -1;

    for (size_t i =0;i < obstacles.size();++i){
        if (!obstacles[i].pItem->isVisible())
            continue;
        if (obstacles[i].contains(state.x, state.y)){
            rec_state[index].back().inside_obstacle = i;
            obstacles[i].pItem->highlight = true;
            obstacles[i].pItem->update();
            this_inside = i;
            break;  // 不会在两个Obstacle里面的
        }
    }
    if (last_inside != -1 && last_inside != this_inside){
        obstacles[last_inside].pItem->highlight = false;
        obstacles[last_inside].pItem->update();
    }
}

bool CoreData::saveExperimentData(QString prefix){
    if (!rec_state.empty() && !rec_state.front().empty()){
        qDebug()<<QString("total records : %1").arg(rec_state[0].size());
        qDebug()<<QString("total time    : %1 ms").arg(std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count());

        for (size_t robot_index = 0; robot_index < rec_state.size(); ++robot_index){
            QString fname = QString("%1-state-%2.txt").arg(prefix).arg(robot_index);
            QFile file(fname);
            if (!file.open(QIODevice::WriteOnly)){
                qDebug()<<"Failed to open file";
                return false;
            }
            QTextStream out(&file);
            auto& rec = rec_state[robot_index];
            out << "t  x  y  th  vel  obstacle" << endl;
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
            file.close();
        }
    }
    else{
        qDebug()<<"No state data has been recorded";
    }
    if (!rec_user.empty()){
        qDebug()<<QString("total user_input record : %1").arg(rec_user.size());
        QString fname = QString("%1-user.txt").arg(prefix);
        QFile file(fname);
        if (!file.open(QIODevice::WriteOnly)){
            qDebug()<<"Failed to open file";
            return false;
        }
        QTextStream out(&file);
        out << "t  ID  type  x  y  dx  dy"<<endl;
        for (size_t i=0;i<rec_user.size();++i){
            auto& rec = rec_user[i];
            out << QString("%1  %2  %3  %4  %5  %6  %7")
                   .arg(rec.t_now.count(), -4)
                   .arg(rec.robotID)
                   .arg(int(rec.type))
                   .arg(rec.x, -5, 'f', 5).arg(rec.y, -5, 'f', 5)
                   .arg(rec.dx, -5, 'f', 5).arg(rec.dy, -5, 'f', 5);
            out << endl;
        }
        out.flush();
        file.close();
    }else{
        qDebug()<<"No user input has been recorded";
    }
    return true;
}

void CoreData::analyzeData(){
    using std::chrono::milliseconds;
    using std::chrono::duration_cast;
    try{
        if (!rec_state.empty()){
            milliseconds              time_all;    // 实验总时间
            std::vector<milliseconds> time_bad;    // 该Robot撞到障碍物的时间(用来做Double Check)
            time_bad.resize(rec_state.size());

            time_all = duration_cast<milliseconds>(t_end - t_start);

            // 计算每个Robot有多长时间在障碍里面
            for (size_t robot_index = 0;robot_index < rec_state.size(); ++robot_index){
                std::vector<StateRecord>& rec = rec_state[robot_index];

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

            // 求均值
            double total_inside = 0;
            for (auto& t : time_bad)
                total_inside += t.count();
            total_inside /= time_bad.size();
            qDebug()<<QString("Average : time inside obstacle = %1 ms / %2 ms (%3 %)")
                      .arg(int(total_inside+0.5))
                      .arg(time_all.count())
                      .arg(int(total_inside / time_all.count() * 100 + 0.5));
        }
    }
    catch (std::exception e){
        qDebug()<<"Error : "<<e.what();
    }
    catch (...){
        qDebug()<<"Error : unexpected...";
    }
}
