#ifndef CONTROLNEW_H
#define CONTROLNEW_H

#include <QWidget>
#include "CoreData.h"
#include "ExternalControl.h"
#include "MainWindow.h"

namespace Ui {
class ControlNew;
}

class ControlNew : public QWidget
{
    Q_OBJECT
    void timerEvent(QTimerEvent*);
    void closeEvent(QCloseEvent * event){
        emit program_exit();
    }
public:
    explicit ControlNew(QWidget *parent = 0);
    ~ControlNew();
    QString getName();
signals:
    void start_exp1(bool isExercise);
    void start_exp2(bool isExercise);
    void program_exit();
    void stop_exp();

public slots:
    void on_expStopped();
    void on_newAutoStop(int ms);
private:
    Ui::ControlNew *ui;
    bool isStartButton = true;
    int myAutoStopTimer = 0;
};

#endif // CONTROLNEW_H
