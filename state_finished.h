#pragma once

#include <QObject>

#include "state.h"

class Rover;

class StateFinished : public State
{
  Q_OBJECT

public:

  explicit StateFinished(Rover* rover);
  virtual ~StateFinished();

  void stop();
public slots:
  virtual void init(); //virtual is not mandatory here
  virtual void run();  //virtual is not mandatory here
  virtual void check();  //virtual is not mandatory here

};
