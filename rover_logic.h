#pragma once

#include <QObject>
#include <QTimer>

#include <cmath>
#include <trikControl/brick.h>

#include "common.h"

using namespace trikControl;

class RoverLogic : public QObject
{
  Q_OBJECT

public:
  explicit RoverLogic();
  virtual ~RoverLogic();

  void setCurrentLoc(LocationData _currentLoc) { m_currentLoc = _currentLoc; };
  void setZeroLoc() { m_zeroLoc = m_currentLoc; };

public slots:
  void start();
  void stop();

signals:
  void moveChasis(int _lS, int _rS);
  void moveArm(int _s);
  void moveHand(int _s);

  void locked();
  void stopSignal();

private:
  void loop();

  void search();
  void track();
    void trackChasis();
    void trackArm();
    void lockCheck();
  void cop();
  void release();

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

  QTimer m_clock;
  QTimer m_locker;

  int m_lastYaw;
  int m_lastChasisSpeed;
  int m_lastArmSpeed;

  LocationData m_zeroLoc;
  LocationData m_currentLoc;
};
