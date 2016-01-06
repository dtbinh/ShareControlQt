#ifndef EXPA_H
#define EXPA_H

#include <atomic>
#include <QObject>
#include <QThread>
#include "MainWindow.h"
#include "CoreData.h"
#include <config.hpp>
#include "ExternalControl.h"
#include "ControlNew.h"


class ExpA : public QThread
{
    Q_OBJECT
    void run();
public slots:
    void prepare_stop();
signals:
    void prepare_start();
    void start_experiment();
    void stop_experiment();
    void new_autoStop(int ms);
public:
    explicit ExpA(QObject *parent = 0):QThread(parent){}
    explicit ExpA(MainScene* s, ExternalControl* ext, ControlNew* c, CoreData *data, _cfg::section *config, QObject *parent = 0):
        QThread(parent), s(s), ext(ext), c(c), data(data),config(config){}


    MainScene *s;
    ExternalControl *ext;
    ControlNew* c;
    CoreData *data;
    _cfg::section *config;

    bool use_TTT;       // TTT : Temporary Tracking Task
    bool isExercise;
};

#endif // EXPA_H
