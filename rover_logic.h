#pragma once

#include <QObject>
#include <QTimer>

#include <cmath>
#include <trikControl/brick.h>

#include "common.h"
#include "rover_engine.h"

using namespace trikControl;

class RoverLogic : public QObject
{
  Q_OBJECT

public:
  explicit RoverLogic(const RoverEngine& _engine);
  virtual ~RoverLogic();

  void setCurrentLoc(LocationData _currentLoc) { m_currentLoc = _currentLoc; };
  void setZeroLoc() { m_zeroLoc = m_currentLoc; };

public slots:
  void start();
  void stop();

private slots:
  void loop();

  void readyCop();
    void cop();

  void readyRelease();
    void release();

private:
  void search();
  void track();
    void trackChasis();
    void trackArm();
    void lockCheck();

  static int sign(int _v);
  static int powerIntegral(int _power, int _lastPower, int _percent);
  static int powerProportional(int _val, int _min, int _zero, int _max);

  enum { 
    STOP,
    SEARCH,
    TRACK,
    COP,
    RELEASE
  } m_state;

  const RoverEngine& m_engine;

  QTimer m_clock;
  QTimer m_locker;

  bool m_chw;
  int m_lastYaw;
  int m_lastChasisSpeed;
  int m_lastX;
  int m_lastArmSpeed;

  LocationData m_zeroLoc;
  LocationData m_currentLoc;
};
