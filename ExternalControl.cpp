#include "ExternalControl.h"
#include "RobotItem.h"
#include <QDebug>

ExternalControl::ExternalControl(QObject *parent) : QObject(parent)
{}

void  ExternalControl::onCloseClients(){
    agent.broadcast(int(myEvent::Basic::programExit), nullptr, 0);
    qDebug()<<QString("programExit signal sended");
}

void ExternalControl::connect(const client_data& target){
    agent.server_start();
    agent.connect_to(target);
    agent.handle_broadcast = [&](int ID, int type, void* data, size_t size){
        this->handle_sub(ID, type, data, size);
    };
    isUpdating = false;
    agent.start(netAgent::sub_thread);
    qDebug()<<QString("Everything started");
}

void ExternalControl::connect(const std::vector<client_data>& client_list, const std::set<int>& except){
    agent.server_start();
    agent.connect_to(client_list, except);

    agent.handle_broadcast = [&](int ID, int type, void* data, size_t size){
        this->handle_sub(ID, type, data, size);
    };

    isUpdating = false;
    agent.start(netAgent::sub_thread);
    qDebug()<<QString("Everything started");
}

void ExternalControl::handle_sub(int ID, int type, void* _data, size_t size){

    myBroadcast::Basic _type = myBroadcast::Basic(type);
    if (_type == myBroadcast::Basic::state){
        if (size == sizeof(RobotStateData) && this->isUpdating){
            RobotStateData* pstate = (RobotStateData*)_data;
            if (data){
                data->onNewRobotPositions(ID, *pstate);
            }
            emit this->newPosition(ID, pstate->x, pstate->y, pstate->heading, pstate->velocity);

            //qDebug()<<QString("new state %1, (%2, %3) - %4, %5").arg(ID)
            //                                                    .arg(pstate->x).arg(pstate->y)
            //                                                    .arg(pstate->heading).arg(pstate->velocity);
        }
    }
    else{
        qDebug()<<"Strange Broadcast Received";
    }
}

/*
 * 处理来自MainScene的NewTrace
*/
/*
void ExternalControl::onSetNewTrace(int robotID, QPointF start, QPointF dx){
    if (data){
        double th = atan2(dx.y(), dx.x());
        int angle = th*180 / 3.141592654;

        qDebug()<<QString("(%1, %2) - %3").arg(start.x()).arg(start.y()).arg(angle);

        data->traceDefinition.x = start.x();
        data->traceDefinition.y = start.y();
        data->traceDefinition.heading = angle;
    }
}
*/
/*
 * 向Client发送全局命令
*/
void ExternalControl::onSetCurrentConfig(){
    if (data){
        //qDebug()<<"set current config";
        data->robotStartState.heading = data->traceDefinition.heading;  // 免去设置初始角度的问题....

        BasicExperimentConfig* config = (BasicExperimentConfig*)data;
        //agent.request(robotID, int(myEvent::Basic::updateBasicConfig), config, sizeof(BasicExperimentConfig));

        // 因为有多个Agent 所以采用广播的方式
        agent.broadcast(int(myEvent::Basic::updateBasicConfig), config, sizeof(*config));
        //qDebug()<<"reply is "<<rep;
    }
}
/*
void ExternalControl::onResetCurrentConfig(){
    int robotID = 1;
    if (data){
        if (data->useDefaultFromClient){
            BasicExperimentConfig* config;

            // 如果要恢复默认设置，则以Robot1的设置为准
            agent.request(robotID, int(myEvent::Basic::requestBasicConfig));

            config = agent.get_more_reply<BasicExperimentConfig>();
            if (config != nullptr){
                *(BasicExperimentConfig*)data = *config;
                emit this->configRefreshed();
            }
            else{
                qDebug()<<QString("Failed to get config from the robot");
            }
        }
        else{
            *(BasicExperimentConfig*)data = BasicExperimentConfig();
            emit this->configRefreshed();
        }
    }
}
*/
void ExternalControl::onStartExperiment(){    
    agent.broadcast(int(myEvent::Basic::startExperiment));
    this->isUpdating = true;

    int checking_T = int(data->mpcSystemDt * 1000 * 2 / 3 + 0.5);

    myTimerID = startTimer(checking_T);
    if (myTimerID != 0)
        qDebug()<<QString("Experiment Started with checking cycle = %1 ms").arg(checking_T);
    else{
        qDebug()<<"Fail to set up timer";
    }
}

void ExternalControl::onStopExperiment(){
   this->isUpdating = false;
   this->killTimer(myTimerID);
   agent.broadcast(int(myEvent::Basic::stopExperiment));
}

/*
 * Swithced Experiment
*/
/*
void ExternalControl::onToAutonomous(int robotID){
    if (data){
        if (data->experimentID == ExpUsing::expSwitch){
            int rep = agent.request(robotID, (int)myEvent::Switch::toAutonomous);
            if (rep != (int)myReply::good){
                qDebug()<<QString("Fail to send toAuto Request");
            }
            qDebug()<<QString("toAuto Request Sended");
        }
    }
}
void ExternalControl::onToTeleoperation(int robotID, QPointF start, QPointF dx){
    if (data){
        if (data->experimentID == ExpUsing::expSwitch){
           double th = atan2(dx.y(), dx.x());
           int angle = th*180 / 3.141592654;

           TraceDataEX user_target;
           user_target.x = start.x();
           user_target.y = start.y();
           user_target.heading = angle;
           user_target.type = data->user_type;

           int rep = agent.request(robotID, (int)myEvent::Switch::toTeleoperation, &user_target, sizeof(TraceDataEX));
           if (rep != (int)myReply::good){
               qDebug()<<QString("Fail to send toTele Request");
           }
           qDebug()<<QString("toTele Request Sended (%1, %2) - %3").arg(user_target.x).arg(user_target.y).arg(user_target.heading);
        }
    }
}
*/
/*
 * Shared Experiment
*/
/*
void ExternalControl::onNewUserTarget(int robotID, QPointF start, QPointF dx){
    if (data){
        if (data->experimentID == ExpUsing::expShared){
            double th = atan2(dx.y(), dx.x());
            double sz = sqrt(dx.x()*dx.x() + dx.y()*dx.y());
            int angle = th*180 / 3.141592654;

            double ratio = (sz / data->Kusr_scr);
            if (ratio >= 2.5) ratio = 2.5;
            float Kusr = data->Kusr_base * ratio;


            TraceDataWithBlend user_target;
            user_target.x = start.x();
            user_target.y = start.y();
            user_target.heading = angle;
            user_target.type = data->user_type;
            user_target.K = Kusr;
            user_target.q0 = 1;
            user_target.secondsToDieOut = data->decay_time;

            int rep = agent.request(robotID, (int)myEvent::Shared::newUserTarget, &user_target, sizeof(TraceDataWithBlend));
            if (rep != (int)myReply::good){
                qDebug()<<QString("Failed to add new user target");
            }
            else{
                qDebug()<<QString("The %1-th request added").arg(rep);
                qDebug()<<"\t"<<QString("(%1, %2) - %3").arg(user_target.x).arg(user_target.y).arg(user_target.heading);
                qDebug()<<"\t"<<QString("K = %1, decay = %2").arg(user_target.K).arg(user_target.secondsToDieOut);
            }
        }
    }
}

void ExternalControl::onRemoveLastTarget(int robotID){
    if (data){
        if (data->experimentID == ExpUsing::expShared){
            int rep = agent.request(robotID, int(myEvent::Shared::removeUserTarget), nullptr, 0);
            if (rep != (int)myReply::good){
                qDebug()<<QString("Fail to Remove The Earliest User Target");
            }
            qDebug()<<QString("User Target Removed");
        }
    }
}
*/
/*
 * Tunnel Experiment
*/
void ExternalControl::timerEvent(QTimerEvent *event){
    using std::chrono::milliseconds;
    using std::chrono::steady_clock;
    using std::chrono::duration_cast;
    auto current_time = steady_clock::now();

    UserInputType type;
    QPointF start, dx;
    if (data){
        for (auto& robot : data->robots){
            robot.get_userInput(type, start, dx);
            robot.set_userInput(UserInputType::none);
            if (type == UserInputType::newSlide){
                this->onNewTunnelTarget(robot.ID, start, dx);
            }
            else if (type == UserInputType::doubleClick){
                this->onRemoveAllTunnelTarget(robot.ID);
            }
            else if (type == UserInputType::setFree){
                this->onUserSetFree(robot.ID);
            }

            if (type != UserInputType::none){
                data->rec_user.push_back({ duration_cast<milliseconds>(current_time - data->t_start),
                                             type, robot.ID,
                                             start.x(), start.y(), dx.x(), dx.y()});
            }
        }
    }
}

void ExternalControl::onUserSetFree(int robotID){
    if (data && data->experimentID == ExpUsing::expTunnel){
        int rep = agent.request(robotID, int(myEvent::Tunnel::setFree), nullptr, 0);
        if (rep != (int)myReply::good){
            qDebug()<<QString("Failed to setFree");
        }
        else{
            qDebug()<<QString("setFree sended to Robot %1").arg(robotID);
        }
    }
}

void ExternalControl::onNewTunnelTarget(int robotID, QPointF start, QPointF dx){
    if (data){
        if (data->experimentID == ExpUsing::expTunnel){
            double th = atan2(dx.y(), dx.x());
            double sz = sqrt(dx.x()*dx.x() + dx.y()*dx.y());
            int angle = th*180 / 3.141592654;

            double ratio = (sz / data->Kusr_scr);
            if (ratio >= 2.5) ratio = 2.5;
            float Kusr = data->Kusr_base * ratio;

            TraceDataWithBlend user_target;
            user_target.x = start.x();
            user_target.y = start.y();
            user_target.heading = angle;
            user_target.type = data->user_type;
            user_target.K = Kusr;
            user_target.q0 = 1;
            user_target.secondsToDieOut = data->decay_time;


            int rep;
            if (data->newTargetAsReplace){
                rep = agent.request(robotID, (int)myEvent::Tunnel::replaceLastTarget, &user_target, sizeof(user_target));
            }
            else{
                rep = agent.request(robotID, (int)myEvent::Tunnel::newUserTarget, &user_target, sizeof(TraceDataWithBlend));
            }

            if (rep != (int)myReply::good){
                qDebug()<<QString("Failed to add new user target");
            }
            else{
                /*
                qDebug()<<QString("Robot %1, request added").arg(robotID);
                qDebug()<<"\t"<<QString("(%1, %2) - %3").arg(user_target.x).arg(user_target.y).arg(user_target.heading);
                qDebug()<<"\t"<<QString("K = %1, decay = %2").arg(user_target.K).arg(user_target.secondsToDieOut);
                */
            }
        }
    }
}

void ExternalControl::onRemoveAllTunnelTarget(int robotID){
    if (data){
        if (data->experimentID == ExpUsing::expTunnel){
            int rep = agent.request(robotID, int(myEvent::Tunnel::removeAllUserTargets), nullptr, 0);
            if (rep != (int)myReply::good){
                qDebug()<<"Fail to Remove Tunnel User Target";
            }
            qDebug()<<QString("All Tunnel User Target Removed");
        }
    }
}

/*
 * Direct Linear Blend Experiment
*/
/*
void ExternalControl::onDirectBlend_NewCmd(int robotID, QPointF start, QPointF dx){
    if (data){
        double th = atan2(dx.y(), dx.x());
        double sz = sqrt(dx.x()*dx.x() + dx.y()*dx.y());
        int angle = th*180 / 3.141592654;

        double ratio = (sz / data->Kusr_scr);
        if (ratio >= 2.5) ratio = 2.5;
        float Kusr = data->Kusr_base * ratio;

        DirectBlendData new_cmd;
        new_cmd.x       = start.x();
        new_cmd.y       = start.y();
        new_cmd.type    = data->user_type;
        new_cmd.heading = angle;
        new_cmd.K       = Kusr;
        new_cmd.secondsToDieOut = data->decay_time;
        new_cmd.q0      = 1;

        int rep = agent.request(robotID, (int)myEvent::DirectBlend::newCommand, &new_cmd, sizeof(DirectBlendData));
        if (rep != (int)myReply::good){
            qDebug()<<QString("Failed to send new command");
        }
        else{
            qDebug()<<QString("New direct blend data sended, K = %1").arg(Kusr);
        }
    }
}

void ExternalControl::onDirectBlend_ReplaceLast(int robotID, QPointF start, QPointF dx){
    if (data){
        double th = atan2(dx.y(), dx.x());
        double sz = sqrt(dx.x()*dx.x() + dx.y()*dx.y());
        int angle = th*180 / 3.141592654;

        double ratio = (sz / data->Kusr_scr);
        if (ratio >= 2.5) ratio = 2.5;
        float Kusr = data->Kusr_base * ratio;

        DirectBlendData new_cmd;
        new_cmd.x       = start.x();
        new_cmd.y       = start.y();
        new_cmd.type    = data->user_type;
        new_cmd.heading = angle;
        new_cmd.K       = Kusr;
        new_cmd.secondsToDieOut = data->decay_time;
        new_cmd.q0      = 1;

        int rep = agent.request(robotID, (int)myEvent::DirectBlend::replaceLastCommand, &new_cmd, sizeof(DirectBlendData));
        if (rep != (int)myReply::good){
            qDebug()<<QString("Failed to replace last command");
        }
        else{
            qDebug()<<QString("New direct blend data sended, K = %1").arg(Kusr);
        }

    }
}

void ExternalControl::onDirectBlend_RemoveLast(int robotID){
    if (data){
        int rep = agent.request(robotID, (int)myEvent::DirectBlend::removeLastCommand, nullptr, 0);
        if (rep != (int)myReply::good){
            qDebug()<<QString("Failed to remove last command");
        }
        else{
            qDebug()<<QString("Direct Blend : Remove Last");
        }
    }
}

void ExternalControl::onDirectBlend_RemoveAll(int robotID){
    if (data){
        int rep = agent.request(robotID, (int)myEvent::DirectBlend::toAtuonomous, nullptr, 0);
        if (rep != (int)myReply::good){
            qDebug()<<"Failed to remove all command";
        }
        else{
            qDebug()<<QString("Direct Blend : Remove All Targets");
        }
    }
}
*/
