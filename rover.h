#pragma once

#include <QObject>
#include <QTimer>
#include <QVector2D>
#include <QStringList>

#include <cmath>

#include "log_fifo.h"
#include "cmd_fifo.h"
#include "motor_controller.h"

#include "state_searching.h"
#include "state_tracking.h"
#include "state_finished.h"
#include "state_squeezing.h"

#include <trikControl/brick.h>

using namespace trikControl;

class Rover : public QObject
{
  Q_OBJECT

public:
  explicit Rover(QThread *guiThread, QString configPath);
  virtual ~Rover();

  int tgtMass() { return m_tgtMass; }
  int tgtX() { return m_tgtX; }
  int tgtY() { return m_tgtY; }

  int oldTgtMass() { return m_oldTgtMass; }
  int oldTgtX() { return m_oldTgtX; }
  int oldTgtY() { return m_oldTgtY; }

  int zeroMass() { return m_zeroMass; }
  int zeroX() { return m_zeroX; }
  int zeroY() { return m_zeroY; }

  void manualControlChasis(int speedL, int speedR);
  void manualControlArm(int speed);
  void manualControlHand(int speed); 
  
  void stopRover();
  void resetScenario();
  
protected:

private slots:
  void setBallColorData(int hue, int hueTol, int sat, int satTol, int val, int valTol);
  void setBallTargetData(int x, int angle, int mass);

  void onGamepadPadDown(int padNum, int vx, int vy);
  void onGamepadPadUp(int padNum);
  void onGamepadButtonChanged(int buttonNum, int value);
  void onBrickButtonChanged(int buttonCode, int value);

  void nextStep(State* state);

  void manualMode();
  void roverMode();

signals:
/*
  void xChanged();
  void yChanged();
  void massChanged();
*/
  void distanceChanged();
  void locationChanged();

private:

  enum { MANUAL_MODE,
         ROVER_MODE
  } movementMode;

  LogFifo          m_logFifo;
  CmdFifo          m_cmdFifo;
  Brick            m_brick;
  MotorController  m_motorControllerL;
  MotorController  m_motorControllerR;
  QThread          m_motorsWorkerThread;
  State*           m_currentState;
  
  //target location data
  int m_tgtX;
  int m_tgtMass;
  int m_tgtY;

  int m_oldTgtX;
  int m_oldTgtY;
  int m_oldTgtMass;

  //zero location
  int m_zeroMass;
  int m_zeroY;
  int m_zeroX;

  //target HSV data
  int m_hue;
  int m_hueTol;
  int m_sat;
  int m_satTol;
  int m_val;
  int m_valTol;

  //Scenario
  StateSearching m_searching1;
  StateTracking m_tracking2;
  StateSqueezing m_squeezing3;
  StateFinished m_finished;

};
