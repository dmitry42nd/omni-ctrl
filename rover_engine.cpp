#include <QDebug>

#include "rover_engine.h"

RoverEngine::RoverEngine(const Brick& _brick, QString _lPort, QString _rPort, QString _armPort, QString _handPort):
  m_brick(_brick),
  m_lPort(_lPort),
  m_rPort(_rPort),
  m_armPort(_armPort),
  m_handPort(_handPort)
{}

RoverEngine::~RoverEngine()
{}

void RoverEngine::moveChasis(int _lS, int _rS)
{
  m_brick.motor(m_lPort)->setPower(_lS);
  m_brick.motor(m_rPort)->setPower(_rS);
}

void RoverEngine::moveArm(int _s)
{
  m_brick.motor(m_armPort)->setPower(_s);
}

void RoverEngine::moveHand(int _s)
{
  m_brick.motor(m_handPort)->setPower(_s);
}

void RoverEngine::stop()
{
  m_brick.motor(m_lPort)->powerOff();
  m_brick.motor(m_rPort)->powerOff();
  m_brick.motor(m_armPort)->powerOff();
  m_brick.motor(m_handPort)->powerOff();
}
