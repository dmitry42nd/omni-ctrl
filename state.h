#pragma once

#include <QDebug>
#include <QObject>
#include <trikControl/brick.h>
#include "enum_state.h"

class Rover;

using namespace trikControl;

class State : public QObject
{
  Q_OBJECT

public:
  State(Rover* rover, State* nextState, const StateMode mode) :
  m_rover(rover),
  m_nextState(nextState),
  m_mode(mode)
  {}

  State(Rover* rover) :
  m_rover(rover)
  {}

  ~State()
  {}

  virtual void stop()
  {
    qDebug() << "Sorry, Mario, your stop() is in another castle!"; 
  }

signals:
  void started();
  void finished(State* state);
  void finished();
  void failed();

public slots:
  virtual void init() 
  { 
    qDebug() << "Sorry, Mario, your init() is in another castle!"; 
  }

  virtual void run() { qDebug() << "Sorry, Mario, your run() is in another castle!"; }
  virtual void check() { qDebug() << "Sorry, Mario, your check() is in another castle!"; }
  virtual void massController();

protected:
  Rover* m_rover;
  State* m_nextState;
  StateMode m_mode;

  static int powerProportional(int _val, int _min, int _zero, int _max)
  {
    int adj = _val - _zero;
    if (adj > 0)
    {
      if (_val >= _max)
        return 100;
      else
        return (+100*(_val-_zero)) / (_max-_zero); // _max!=_zero, otherwise (_val>=_max) matches
    }
    else if (adj < 0)
    {
      if (_val <= _min)
        return -100;
      else
        return (-100*(_val-_zero)) / (_min-_zero); // _min!=_zero, otherwise (_val<=_min) matches
    }
    else
      return 0;
  }

  static int sign(int _v)
  {
    return (_v < 0) ? -1 : ((_v > 0) ? 1 : 0);
  }
};
/*
#include "state_searching.h"
#include "state_tracking.h"
#include "state_finished.h"
*/
