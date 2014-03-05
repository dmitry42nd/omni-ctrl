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
  qDebug() << "FINISHED";

  run();
  emit started();
}

void StateFinished::run()
{
  m_rover->resetScenario();

  emit finished();
}

void StateFinished::check() {}

void StateFinished::stop() {}
