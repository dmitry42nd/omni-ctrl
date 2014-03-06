//#include <QDebug>

#include "state_start.h"
#include "rover.h"

StateStart::StateStart(Rover* rover, State* nextState, const StateMode mode) :
State(rover, nextState, mode)
{}

StateStart::~StateStart()
{}

void StateStart::init()
{
  run();
  emit started();
}

void StateStart::run()
{
  m_rover->stopRover();
}

void StateStart::check()
{
  qDebug() << "ROVER STAЯTEД";

  emit finished(m_nextState);
} 

void StateStart::stop()
{}

