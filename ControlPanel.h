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
    void setupTunnel(double, double);
    void clearTunnel();
public slots:
    void refreshBasicConfig();
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

private:
    Ui::ControlPanel *ui;
    QString textKaiShi;
};



#endif // CONTROLPANEL_H
