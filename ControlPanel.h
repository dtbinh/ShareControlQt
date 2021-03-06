#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H
#pragma warning(disable: 4819 4100)

#include <QWidget>
#include <Agent.hpp>
#include "MainWindow.h"
#include "CoreData.h"

namespace Ui {
class ControlPanel;
}

class ControlPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ControlPanel(QWidget *parent = 0);
    ~ControlPanel();

signals:
    void intoTraceSettingMode();
    void intoRobotMovingMode();

    void setCurrentConfig();
    void resetCurrentConfig();
    void startExperiment();
    void stopExperiment();
    void newScreenPrecision(int precision);
    void setupForTunnel(double, double);
    void clearForTunnel();
    void programExit();     // 自己退出时的信号
    void closeClients();    // 关闭下位Client

public slots:
    void refreshBasicConfig();      // 根据CoreData更新各按钮、Slider的信息
    void onWinSizeChanged(int pos);
    void onKusrChanged(int pos);
    void onDecayTimeChanged(int pos);

    void onExperimentTypeChanged(ExpUsing type);
    void onUserTypeChanged(TargetType type);

    void beforeStartingExperiment();
    void beforeStopExperiment();

public:
    CoreData* data = nullptr;
    float minWinSize = 1;
    float maxWinSize = 15;
    float minKusrPower = 0;
    float maxKusrPower = 2;
    float minDecayTime = 1;
    float maxDecayTime = 20;

protected:
    void closeEvent(QCloseEvent *event);
private:
    Ui::ControlPanel *ui;
    QString textKaiShi;
};



#endif // CONTROLPANEL_H
