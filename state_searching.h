#pragma once

#include <QObject>
#include "state.h"

class Rover;

using namespace trikControl;

class StateSearching : public State
{
  Q_OBJECT

public:

  explicit StateSearching(Rover* rover, State* state, const StateMode);
  virtual ~StateSearching();

 void stop();

public slots:
  virtual void init(); //virtual is not mandatory here
  virtual void run();  //virtual is not mandatory here
  virtual void check();  //virtual is not mandatory here

protected:
  int m_zeroMass;
};
