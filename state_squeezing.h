#pragma once

#include <QObject>
#include <QTimer>

#include "state.h"

class Rover;
using namespace trikControl;

class StateSqueezing : public State
{
  Q_OBJECT

public:

  explicit StateSqueezing(Rover* rover, State* state, const StateMode mode);
  virtual ~StateSqueezing();

  void stop();

public slots:
  virtual void init(); //virtual is not mandatory here
  virtual void run();  //virtual is not mandatory here
  virtual void check();  //virtual is not mandatory here

private:
  QTimer m_squeezeTimer;
};
