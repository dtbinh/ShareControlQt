#include "ControlPanel.h"
#include "ui_controlpanel.h"
#include <QTextCodec>

ControlPanel::ControlPanel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ControlPanel)
{
    ui->setupUi(this);
    setWindowFlags(Qt::CustomizeWindowHint);
    setWindowFlags(Qt::WindowTitleHint);
    setWindowFlags(Qt::WindowCloseButtonHint);
    setWindowTitle("ControlPannel");
    this->textKaiShi = ui->btnStartStop->text();

    // SLIDERS
    connect(ui->sliderWinSize,    &QSlider::valueChanged, this, &ControlPanel::onWinSizeChanged);
    connect(ui->sliderQDecayTIme, &QSlider::valueChanged, this, &ControlPanel::onDecayTimeChanged);
    connect(ui->sliderKusr,       &QSlider::valueChanged, this, &ControlPanel::onKusrChanged);

    // RADIOBUTTON
    // ExpUsing
    connect(ui->rtnExpShared, &QRadioButton::clicked, [=](){
        this->onExperimentTypeChanged(ExpUsing::expShared);
    });
    connect(ui->rtnExpSwitch, &QRadioButton::clicked, [=](){
        this->onExperimentTypeChanged(ExpUsing::expSwitch);
    });
    connect(ui->rtnExpDirect, &QRadioButton::clicked, [=](){
        this->onExperimentTypeChanged(ExpUsing::expDirect);
    });
    connect(ui->rtnExpTunnel, &QRadioButton::clicked, [=](){
        this->onExperimentTypeChanged(ExpUsing::expTunnel);
    });


    // RADIOBUTTON
    // TargetType
    connect(ui->rtnByPosition, &QRadioButton::clicked, [=](){
        this->onUserTypeChanged(TargetType::position);
    });
    connect(ui->rtnByVelocity, &QRadioButton::clicked, [=](){
        this->onUserTypeChanged(TargetType::velocity);
    });

    // 界面模式（当前是PlaceRobot还是PlaceTrace）
    connect(ui->rtnSetRobot, &QRadioButton::clicked, [=](){
        if (data){
            data->isPlacingRobot = true;
            data->isPlacingTrace = false;
        }
        emit this->intoRobotMovingMode();
    });
    connect(ui->rtnSetTrace, &QRadioButton::clicked, [=](){
        if (data){
            data->isPlacingRobot = false;
            data->isPlacingTrace = true;
        }
        emit this->intoTraceSettingMode();
    });

    // Robot采用哪种模型：理想(ideal) 还是 仿真(direct)(物理直接控制)
    connect(ui->rtnIdeal, &QRadioButton::clicked, [=](){
        if (data){
            data->robot_type = RobotType::ideal;
        }
    });
    connect(ui->rtnSimu, &QRadioButton::clicked, [=](){
        if (data){
            data->robot_type = RobotType::direct;
        }
    });

    // PUSH BUTTON
    connect(ui->btnSetConfig, &QPushButton::clicked,  [=](){
        if (data){
            // 为兼容以前的实验(只取第一个Robot的位置)
            // (后续实验中robots初始位置不由此设置)
            QPointF pos   = data->robots[0].RealPos();
            qreal heading = data->robots[0].Heading();
            data->robotStartState.x       = pos.x();
            data->robotStartState.y       = pos.y();
        }
        emit this->setCurrentConfig();

        // 清除、显示Tunnel
        if (data->experimentID == ExpUsing::expTunnel){
            // tunnel_R, tunnel_r是在client端设置的
            emit this->setupForTunnel(data->tunnel_R, data->tunnel_r);
        }
        else emit this->clearForTunnel();
    });

    connect(ui->btnResetConfig, &QPushButton::clicked,  [=](){
        emit this->resetCurrentConfig();
    });

    // CHECK BOX
    // Tunnel实验的选项
    connect(ui->cbContinuous, &QCheckBox::clicked, [=](bool checked){
       if (data){data->tunnel_continuous = checked;}
    });
    connect(ui->cbWithTTT,    &QCheckBox::clicked, [=](bool checked){
        if (data){data->tunnel_useTTT    = checked;}
    });

    // 开始/结束实验按钮
    connect(ui->btnStartStop, &QPushButton::clicked, [&](){
        if (data){
            if (data->experiment_running){
                return beforeStopExperiment();
            }
            else{
                return beforeStartingExperiment();
            }
        }
    });

    // 退出程序按钮
    connect(ui->btnExitMe, &QPushButton::clicked, [&](){
        emit programExit();
        this->close();
    });
    connect(ui->btnExitAll, &QPushButton::clicked, [&](){
        emit closeClients();
        emit programExit();
        this->close();
    });
}

ControlPanel::~ControlPanel()
{
    delete ui;
}

void ControlPanel::closeEvent(QCloseEvent *event){
    emit programExit();
}

void ControlPanel::onWinSizeChanged(int pos){
    if (data){
        data->mpcWindowsTime = (pos / 99.0) * (maxWinSize - minWinSize) + minWinSize;
        ui->labWinSize->setText(QString("窗口大小:%1s").arg(data->mpcWindowsTime, 0, 'g', 3));
        //qDebug()<<"new mpcWindowsTime "<<data->mpcWindowsTime;
    }
}

void ControlPanel::onKusrChanged(int pos){
    if (data){
        float power = (pos / 99.0) * (maxKusrPower - minKusrPower) + minKusrPower;
        data->Kusr_base = std::powf(10, power);
        ui->labKusr->setText(QString("Kusr基准:%1").arg(data->Kusr_base, 0, 'g', 3));
        //qDebug()<<"new Kusr base "<<data->Kusr_base;
    }
}

void ControlPanel::onDecayTimeChanged(int pos){
    if (data){
        data->decay_time = (pos / 99.0)* (maxDecayTime - minDecayTime) + minDecayTime;
        ui->labDecayTime->setText(QString("衰减时间:%1s").arg(data->decay_time, 0, 'g', 3));
        //qDebug()<<"new decay time "<<data->decay_time;
    }
}

void ControlPanel::onExperimentTypeChanged(ExpUsing type){
    if (data){
        data->experimentID = type;
        //qDebug()<<"new exp type "<<int(type);
    }
}

void ControlPanel::onUserTypeChanged(TargetType type){
    if (data){
        data->user_type = type;
        //qDebug()<<"new usr type "<<int(type);
    }
}

void ControlPanel::beforeStartingExperiment(){
    ui->gpConfig->setDisabled(true);
    ui->btnStartStop->setText(QString("停止"));

    data->experiment_running = true;
    emit this->startExperiment();
}

void ControlPanel::beforeStopExperiment(){
    ui->gpConfig->setDisabled(false);
    ui->btnStartStop->setText(this->textKaiShi);
    emit this->stopExperiment();

    data->experiment_running = false;
}

void ControlPanel::refreshBasicConfig(){
    if (data){
        // 利用CoreData更新实验设置: 用什么实验、用什么模型、MPC窗大小多少
        bool op;
        op = data->experimentID == ExpUsing::expSwitch;
        ui->rtnExpSwitch->setChecked(op);
        op = data->experimentID == ExpUsing::expShared;
        ui->rtnExpShared->setChecked(op);
        op = data->experimentID == ExpUsing::expDirect;
        ui->rtnExpDirect->setChecked(op);
        op = data->experimentID == ExpUsing::expTunnel;
        ui->rtnExpTunnel->setChecked(op);

        op = data->robot_type == RobotType::ideal;
        ui->rtnIdeal->setChecked(op);
        op = data->robot_type == RobotType::direct;
        ui->rtnSimu->setChecked(op);

        ui->cbContinuous->setChecked(data->tunnel_continuous);
        ui->cbWithTTT->setChecked(data->tunnel_useTTT);

        float winSize = data->mpcWindowsTime;
        int pos = int((winSize - minWinSize) / (maxWinSize - minWinSize) * 100 + 0.5);
        ui->sliderWinSize->setSliderPosition(pos);
        onWinSizeChanged(pos);

        // 重置实验中发送数据时的设置
        onKusrChanged(50);      // 设置在中心点
        onDecayTimeChanged(50); // 设置在中心点
    }
}
