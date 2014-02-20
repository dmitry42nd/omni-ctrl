#pragma once

#include <QObject>
#include <QTimer>
#include <QVector2D>
#include <QStringList>

#include <cmath>

#include "log_fifo.h"
#include "cmd_fifo.h"
#include "motor_controller.h"

using namespace trikControl;

class Omni : public QObject
{
    Q_OBJECT
public:
    explicit Omni(QThread *guiThread, QString configPath);
    virtual ~Omni();

protected:
    void manualMode();
    void linetraceMode();

    void startMotorsWorker();
    void stopMotorsWorker();

public slots:

private slots:
  void setLineColorData(int hue, int hueTol, int sat, int satTol, int val, int valTol);
  void setLineTargetData(int x, int angle, int mass);

  void onGamepadPadDown(int padNum, int vx, int vy);
  void onGamepadPadUp(int padNum);
  void onGamepadButtonChanged(int buttonNum, int value);
  void onBrickButtonChanged(int buttonCode, int value);

private:

    enum { MANUAL_MODE,
           LINETRACE_MODE
    } movementMode;

    LogFifo          m_logFifo;
    CmdFifo          m_cmdFifo;
    Brick            m_brick;
    MotorController  m_motorControllerL;
    MotorController  m_motorControllerR;
    QThread          m_motorsWorkerThread;

    //target location data
    int m_tgtX;
    int m_prevTgtX;
    int m_tgtAngle;
    int m_tgtMass;

    //target HSV data
    int m_hue;
    int m_hueTol;
    int m_sat;
    int m_satTol;
    int m_val;
    int m_valTol;
};
