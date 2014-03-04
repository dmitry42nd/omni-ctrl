#pragma once

#include <QObject>
#include "state.h"

class Rover;
using namespace trikControl;

class StateTracking : public State
{
  Q_OBJECT

public:

  explicit StateTracking(Rover* rover, const State* state, const StateMode mode);
  virtual ~StateTracking();

  void setObjectiveMass(int mass);

public slots:
  virtual void init(StateMode mode); //virtual is not mandatory here
  virtual void run();  //virtual is not mandatory here
  virtual void check();  //virtual is not mandatory here

protected:
/*
  int m_objectiveMass;
  int m_objectiveY;
  int m_objectiveX;
*/
  int m_zeroMass;
  int m_zeroY;
  int m_zeroX;

  bool m_chw;
  bool m_locked;
//  int m_objectiveDistance;

  void runChasis();
  void runArm();
  void runHand();
};
