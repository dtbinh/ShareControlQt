#include "ExternalControl.h"
#include <QDebug>

ExternalControl::ExternalControl(QObject *parent) : QObject(parent)
{}


void ExternalControl::connect(const client_data& target){
    agent.server_start();
    agent.connect_to(target);
    agent.handle_broadcast = [&](int ID, int type, void* data, size_t size){
        this->handle_sub(ID, type, data, size);
    };
    isUpdating = false;
    agent.start(netAgent::sub_thread);
    qDebug()<<"Everything started";
}

void ExternalControl::handle_sub(int ID, int type, void* _data, size_t size){
    myBroadcast::Basic _type = myBroadcast::Basic(type);
    if (_type == myBroadcast::Basic::state){
        if (size == sizeof(RobotStateData)){
            RobotStateData* pstate = (RobotStateData*)_data;
            if (this->isUpdating)
                  emit this->newPosition(ID, pstate->x, pstate->y, pstate->heading, pstate->velocity);
            //qDebug()<<QString("new state %1, (%2, %3) - %4, %5").arg(ID)
            //                                                    .arg(pstate->x).arg(pstate->y)
            //                                                    .arg(pstate->heading).arg(pstate->velocity);
        }
        else{
            qDebug()<<"Unrecognized State Broadcast";
        }
    }
    else{
        qDebug()<<"Strange Broadcast Received";
    }
}

void ExternalControl::onSetCurrentConfig(){
    int robotID = 1;
    if (data){
        //qDebug()<<"set current config";
        data->robotStartState.heading = data->traceDefinition.heading;  // 免去设置初始角度的问题....

        BasicExperimentConfig* config = (BasicExperimentConfig*)data;
        agent.request(robotID, int(myEvent::Basic::updateBasicConfig), config, sizeof(BasicExperimentConfig));
        //qDebug()<<"reply is "<<rep;
    }
}

void ExternalControl::onResetCurrentConfig(){
    int robotID = 1;
    if (data){
        //qDebug()<<"reset current config";
        BasicExperimentConfig* config;
        agent.request(robotID, int(myEvent::Basic::requestBasicConfig));
        config = agent.get_more_reply<BasicExperimentConfig>();
        if (config != nullptr)
            *(BasicExperimentConfig*)data = *config;

        emit this->configRefreshed();
    }
}

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

void ExternalControl::onStartExperiment(){
    int robotID = 1;
    int rep = agent.request(robotID, (int)myEvent::Basic::startExperiment);
    if (rep != (int)myReply::good){
        qDebug()<<"can not start experiment...";
    }
}
void ExternalControl::onStopExperiment(){
    int robotID = 1;
    int rep = agent.request(robotID, (int)myEvent::Basic::stopExperiment);
    if (rep != (int)myReply::good){
        qDebug()<<"can not stop experiment...";
    }
}

void ExternalControl::onToAutonomous(int robotID){
    if (data){
        if (data->experimentID == ExpUsing::expSwitch){
            int rep = agent.request(robotID, (int)myEvent::Switch::toAutonomous);
            if (rep != (int)myReply::good){
                qDebug()<<"Fail to send toAuto Request";
            }
            qDebug()<<"toAuto Request Sended";
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
               qDebug()<<"Fail to send toTele Request";
           }
           qDebug()<<QString("toTele Request Sended (%1, %2) - %3").arg(user_target.x).arg(user_target.y).arg(user_target.heading);
        }
    }
}
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
            data->addedUserTarget.push_back(rep);

            qDebug()<<QString("The %1-th request added").arg(rep);
            qDebug()<<"\t"<<QString("(%1, %2) - %3").arg(user_target.x).arg(user_target.y).arg(user_target.heading);
            qDebug()<<"\t"<<QString("K = %1, decay = %2").arg(user_target.K).arg(user_target.secondsToDieOut);
        }
    }
}

void ExternalControl::onRemoveLastTarget(int robotID){
    if (data){
        if (data->experimentID == ExpUsing::expShared){
            auto& tarList = data->addedUserTarget;
            if (tarList.empty()){
                qDebug()<<"No target has been added";
                return ;
            }
            int target_id = tarList.back();
            tarList.erase(tarList.begin() + tarList.size() - 1);

            int rep = agent.request(robotID, int(myEvent::Shared::removeUserTarget), &target_id, sizeof(int));
            if (rep != (int)myReply::good){
                qDebug()<<"Fail to Remove Last User Target";
            }
            qDebug()<<"Last User Target Removed";
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

            int rep = agent.request(robotID, (int)myEvent::Tunnel::newUserTarget, &user_target, sizeof(TraceDataWithBlend));
            data->addedUserTarget.push_back(rep);

            qDebug()<<QString("The %1-th request added").arg(rep);
            qDebug()<<"\t"<<QString("(%1, %2) - %3").arg(user_target.x).arg(user_target.y).arg(user_target.heading);
            qDebug()<<"\t"<<QString("K = %1, decay = %2").arg(user_target.K).arg(user_target.secondsToDieOut);
        }
    }
}

void ExternalControl::onRemoveAllTunnelTarget(int robotID){
    if (data){
        if (data->experimentID == ExpUsing::expTunnel){
            auto& tarList = data->addedUserTarget;
            if (tarList.empty()){
                qDebug()<<"No target has been added";
                return ;
            }
            tarList.clear();

            int rep = agent.request(robotID, int(myEvent::Tunnel::removeAllUserTargets), nullptr, 0);
            if (rep != (int)myReply::good){
                qDebug()<<"Fail to Remove Tunnel User Target";
            }
            qDebug()<<"All Tunnel User Target Removed";
        }
    }
}


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
        qDebug()<<QString("New direct blend data sended, K = %1").arg(Kusr);
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
        qDebug()<<QString("New direct blend data sended, K = %1").arg(Kusr);
    }
}

void ExternalControl::onDirectBlend_RemoveLast(int robotID){
    if (data){
        int rep = agent.request(robotID, (int)myEvent::DirectBlend::removeLastCommand, nullptr, 0);
        qDebug()<<QString("Direct Blend : Remove Last");
    }
}

void ExternalControl::onDirectBlend_RemoveAll(int robotID){
    if (data){
        int rep = agent.request(robotID, (int)myEvent::DirectBlend::toAtuonomous, nullptr, 0);
        qDebug()<<QString("Direct Blend : Remove All Targets");
    }
}
