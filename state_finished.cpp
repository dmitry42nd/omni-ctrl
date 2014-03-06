#include <QDebug>

#include "state_finished.h"
#include "rover.h"

StateFinished::StateFinished(Rover* rover) :
State(rover)
{}

StateFinished::~StateFinished()
{}

void StateFinished::init()
{
  qDebug() << "FINISHING";

  run();
  emit started();
}

void StateFinished::run()
{
  m_rover->resetScenario();
  m_rover->manualMode();

  check(); //just for codestyle accordance
}

void StateFinished::check() 
{
  qDebug() << "FINISHED";

  m_rover->stopRover();
  stop();

  emit finished();
}

void StateFinished::stop() {}
