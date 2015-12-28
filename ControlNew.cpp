#include "ControlNew.h"
#include "ui_controlnew.h"

ControlNew::ControlNew(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ControlNew)
{
    ui->setupUi(this);
    setWindowFlags(Qt::CustomizeWindowHint);
    setWindowFlags(Qt::WindowTitleHint);
    setWindowFlags(Qt::WindowCloseButtonHint);
    ui->rbExp1->setChecked(true);

    QObject::connect(ui->pbStartStop, &QPushButton::clicked,
                     [&](){
        if (this->isStartButton){
            isStartButton = false;
            ui->pbStartStop->setText(QString("Stop"));
            if (this->ui->rbExp1->isChecked()){
                emit this->start_exp1(this->ui->cbExercise->isChecked());
            }
            else{
                emit this->start_exp2(this->ui->cbExercise->isChecked());
            }
        }
        else{
            on_expStopped();
        }
    });

    QObject::connect(ui->pbExit, &QPushButton::clicked,
                     [&](){
        emit this->program_exit();
        this->close();
    });
}

QString ControlNew::getName(){
    return this->ui->leName->text();
}

void ControlNew::on_expStopped(){   // 当实验自动停止时
    if (myAutoStopTimer){
        this->killTimer(myAutoStopTimer);
        myAutoStopTimer = 0;
    }
    emit this->stop_exp();

    isStartButton = true;
    ui->pbStartStop->setText(QString("Start"));
}

ControlNew::~ControlNew()
{
    delete ui;
}

void ControlNew::on_newAutoStop(int ms){
    myAutoStopTimer = this->startTimer(ms);
    if (myAutoStopTimer == 0){
        qDebug()<<"Fail to start autoStopTimer";
    }
    else{
        qDebug()<<QString("AutoStopTimer started for %1 ms").arg(ms);
    }
}

void ControlNew::timerEvent(QTimerEvent *event){
    if (event->timerId() == this->myAutoStopTimer){
        on_expStopped();
    }
}
