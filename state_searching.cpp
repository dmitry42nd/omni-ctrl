//#include <QDebug>

#include "state_searching.h"
#include "rover.h"

const int speed = 50;

StateSearching::StateSearching(Rover* rover, const State* nextState, const StateMode mode) :
State(rover, nextState, mode),
m_zeroMass(0)
{}

StateSearching::~StateSearching()
{}

void StateSearching::init()
{
  qDebug() << "START_SEARCHING";

  switch (m_mode)
  {
    case UntilMass:
      m_mode = UntilMass;
      connect(m_rover, SIGNAL(locationChanged()), this, SLOT(check()));  
      break;
/*
    case UntilTimeout:
      connect(m_rover, SIGNAL(timeout()), this, SLOT(check()));  
      m_mode = UntilMass;
      break;
*/
    default:
      qDebug() << "This trigger is not supported in searching state";
      //set access flag
      break;
  }

  //check access flag
  run();
  emit started();
}

void StateSearching::run()
{
  m_rover->manualControlChasis(speed, -speed);
}

void StateSearching::check()
{
  switch(m_mode)
  {
    case UntilMass:
      if (m_zeroMass > m_rover->tgtMass())
      {
        qDebug() << "FOUND";
        disconnect(m_rover, SIGNAL(massChanged(int)), this, SLOT(run()));
        emit finished(m_nextState);
      }
      break;
    default:
      break;
  }
}

/*
void StateSearching::setObjectiveMass(int mass)
{
  m_objectiveMass = mass;
}
*/
