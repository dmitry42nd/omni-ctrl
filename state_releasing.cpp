//#include <QDebug>

#include <QTimer>
#include "state_releasing.h"
#include "rover.h"

StateReleasing::StateReleasing(Rover* rover, State* nextState, const StateMode mode) :
State(rover, nextState, mode)
{}

StateReleasing::~StateReleasing()
{}

void StateReleasing::init()
{
  qDebug() << "START_RELEASING";

//  connect(&m_releaseTimer, SIGNAL(timeout()), this, SLOT(check()));

  run();
  emit started();
}

void StateReleasing::run()
{
  runChasisLeft();
}

void StateReleasing::runChasisLeft()
{
  m_rover->manualControlArm(100);
  m_rover->manualControlChasis(-40, 40);
  QTimer::singleShot(2500, this, SLOT(runHand()));
}

void StateReleasing::runHand()
{
  m_rover->stopRover();
  m_rover->manualControlHand(-100);
  QTimer::singleShot(5000, this, SLOT(runChasisRight()));
}

void StateReleasing::runChasisRight()
{
  m_rover->manualControlArm(-50);
  m_rover->manualControlChasis(40, -40);
  QTimer::singleShot(2500, this, SLOT(check()));
}

void StateReleasing::check()
{
  qDebug() << "TARGET RELEASED";

  m_rover->stopRover();
  stop();

  emit finished(m_nextState);
}

void StateReleasing::stop()
{
  //disconnect(&m_releaseTimer, SIGNAL(timeout()), this, SLOT(check()));
}

