#include <QDebug>

#include "state_squeezing.h"
#include "rover.h"

StateSqueezing::StateSqueezing(Rover* rover, State* nextState, const StateMode mode) :
State(rover, nextState, mode)
{}

StateSqueezing::~StateSqueezing()
{}

void StateSqueezing::init()
{
  qDebug() << "START_SQUEEZING";

  connect(&m_squeezeTimer, SIGNAL(timeout()), this, SLOT(check()));
  connect(m_rover, SIGNAL(locationChanged()), this, SLOT(massController()));

  run();
  emit started();
}

void StateSqueezing::run()
{
  m_rover->manualControlHand(100);
  m_squeezeTimer.start(5000);
}

void StateSqueezing::check()
{
    qDebug() << "TARGET SQUEEЗEД";

    m_rover->stopRover();
    stop();

    emit finished(m_nextState);
}

void StateSqueezing::stop()
{
//  qDebug() << "Squeezing stoped";

  disconnect(m_rover, SIGNAL(locationChanged()), this, SLOT(massController()));
  disconnect(&m_squeezeTimer, SIGNAL(timeout()), this, SLOT(check()));
}

