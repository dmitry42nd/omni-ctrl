#pragma once

#include <QObject>
#include <QTimer>

#include "state.h"

class Rover;
using namespace trikControl;

class StateReleasing : public State
{
  Q_OBJECT

public:

  explicit StateReleasing(Rover* rover, State* state, const StateMode mode);
  virtual ~StateReleasing();

  void stop();


public slots:
  virtual void init(); //virtual is not mandatory here
  virtual void run();  //virtual is not mandatory here
  virtual void check();  //virtual is not mandatory here

private slots:
  void runChasisLeft();
  void runChasisRight();
  void runHand();

private:
  QTimer m_releaseTimer;
};
