#ifndef EXPA_H
#define EXPA_H

#include <atomic>
#include <QObject>
#include <QThread>
#include "MainWindow.h"
#include "CoreData.h"
#include <config.hpp>
#include "ExternalControl.h"


class ExpA : public QThread
{
    Q_OBJECT
    void run();
    void timerEvent(QTimerEvent *);
public slots:
    void prepare_stop();
signals:
    void start_experiment();
    void stop_experiment();
    void new_autoStop(int ms);
public:
    explicit ExpA(QObject *parent = 0):QThread(parent){}
    explicit ExpA(MainScene* s, ExternalControl* ext, CoreData *data, _cfg::section *config, QObject *parent = 0):
        QThread(parent), s(s), ext(ext),data(data),config(config){}


    MainScene *s;
    ExternalControl *ext;
    CoreData *data;
    _cfg::section *config;

    bool use_TTT;
    bool isExercise;
    std::atomic<int> exp_running;
    int myTimer;
};

#endif // EXPA_H
