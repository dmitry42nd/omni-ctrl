#pragma once

#include <QObject>
#include <QTimer>

#include "state.h"

class Rover;

class StateStart : public State
{
  Q_OBJECT

public:

  explicit StateStart(Rover* rover, State* state, const StateMode mode);
  virtual ~StateStart();

  void stop();

public slots:
  virtual void init(); //virtual is not mandatory here
  virtual void run();  //virtual is not mandatory here
  virtual void check();  //virtual is not mandatory here

};
