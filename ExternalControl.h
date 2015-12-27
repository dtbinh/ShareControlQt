
#ifndef EXTERNALCONTROL_H
#define EXTERNALCONTROL_H

#include <QObject>
#include <atomic>
#include <set>
#include "Util/Auxilary.h"
#include "Agent.hpp"
#include "basic/app-share.hpp"
#include "MainWindow.h"
#include "CoreData.h"

using std::vector;
using std::string;
using netAgent::Agent;
using netAgent::client_data;
using zmq::socket_t;
using zmq::context_t;
using zmq::message_t;
using std::atomic_bool;


class ExternalControl:public QObject{
    Q_OBJECT
public:
    explicit ExternalControl(QObject *parent = 0);
    ~ExternalControl(){
        agent.stop();
        agent.join();
    }
signals:
    void configRefreshed();
    void newPosition(int ID, float x, float y, int heading, float velocity);

public slots:
    // Basic Setup

    // void onResetCurrentConfig();

    // void onSetNewTrace(int robotID, QPointF start, QPointF dx);
    // Used by switched experiment
    // void onToAutonomous(int robotID);
    // void onToTeleoperation(int robotID, QPointF start, QPointF dx);

    // Used by share experiment
    // void onNewUserTarget(int robotID, QPointF start, QPointF dx);
    // void onRemoveLastTarget(int robotID);

    // Used by tunnel experiment
    void onNewTunnelTarget(int robotID, QPointF start, QPointF dx);
    void onRemoveAllTunnelTarget(int robotID);
    void onUserSetFree(int robotID);

    // Used by direct blend experiment
    // void onDirectBlend_NewCmd(int robotID, QPointF start, QPointF dx);
    // void onDirectBlend_ReplaceLast(int robotID, QPointF start, QPointF dx);
    // void onDirectBlend_RemoveLast(int robotID);
    // void onDirectBlend_RemoveAll(int robotID);

    // Used to close all programs
public:
    void connect(const client_data& client);
    void connect(const std::vector<client_data>& client_list, const std::set<int>& except = {});

    void start_updating(){
        isUpdating = true;
    }
    void stop_updating(){
        isUpdating = false;
    }

protected:
    void handle_sub(int ID, int type, void* data, size_t size);
    void timerEvent(QTimerEvent *event);    // 异步发送UserInput的方式

public slots:
    void onSetCurrentConfig();
    void onStartExperiment();
    void onStopExperiment();
    void onCloseClients();

public:
    Agent       agent;
    atomic_bool isUpdating;
    CoreData*   data  = nullptr;
    int client_num = 1;
    int myTimerID  = 0;
};

#endif // EXTERNALCONTROL_H
