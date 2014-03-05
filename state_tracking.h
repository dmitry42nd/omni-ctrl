#pragma once

#include <QObject>
#include <QTimer>

#include "state.h"

class Rover;
using namespace trikControl;

class StateTracking : public State
{
  Q_OBJECT

public:

  explicit StateTracking(Rover* rover, State* state, const StateMode mode);
  virtual ~StateTracking();

  void stop();

public slots:
  virtual void init(); //virtual is not mandatory here
  virtual void run();  //virtual is not mandatory here
  virtual void check();  //virtual is not mandatory here

protected:
  int m_zeroMass;
  int m_zeroY;
  int m_zeroX;
//  int m_zeroDistance;

  bool m_chw;
  bool m_fixed;

  void runChasis();
  void runArm();
  void runHand();

private:
  QTimer m_locker;

  int m_lastYaw;

};
