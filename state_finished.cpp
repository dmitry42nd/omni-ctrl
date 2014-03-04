#include <QDebug>

#include "state_finished.h"
#include "rover.h"

const int speed = 50;

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
  m_rover->manualControlChasis(0, 0);
  m_rover->manualControlArm(0);
  m_rover->manualControlHand(0);

  emit finished();
}

void StateFinished::check() {}

