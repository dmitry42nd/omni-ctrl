#include <QDebug>

#include <cmath>

#include "state_tracking.h"
#include "rover.h"

const float xPK = 1.1;
const float xIK = 0.05;
const float xDK = 0.3;
/*
const float massPK = 1;
const float massIK = 0.05;
const float massDK = 0.1;
*/
StateTracking::StateTracking(Rover* rover, State* nextState, const StateMode mode) :
State(rover, nextState, mode),
/*
m_zeroMass(20),
m_zeroY(50),
m_zeroX(0),
*/
m_chw(true),
m_fixed(false)
{}

StateTracking::~StateTracking()
{}

void StateTracking::init()
{
  qDebug() << "STAЯT_TЯACKИNГ";

  m_zeroMass = m_rover->zeroMass();
  m_zeroY    = m_rover->zeroY();
  m_zeroX    = m_rover->zeroX();

  switch (m_mode)
  {
    case UntilMass:
      connect(m_rover, SIGNAL(locationChanged()), this, SLOT(check()));
      connect(m_rover, SIGNAL(locationChanged()), this, SLOT(run()));
      break;
    case UntilLocked:
      connect(m_rover, SIGNAL(locationChanged()), this, SLOT(run()));  
      connect(&m_locker, SIGNAL(timeout()), this, SLOT(check()));
      connect(m_rover, SIGNAL(locationChanged()), this, SLOT(massController()));
      break;

    default:
      qDebug() << "This trigger is not supported in searching state";
      break;
      //flag for action access;
  }

  run();
  emit started();
}

void StateTracking::run()
{
  runChasis();
  runArm();
  runHand();
}

void StateTracking::runChasis()
{
/*
  float tgtMass = m_rover->tgtMass() - m_zeroMass;
  float oldTgtMass = m_rover->oldTgtMass() - m_zeroMass;
*/
  float tgtMass = m_rover->tgtMass();
  float tgtY = m_rover->tgtY();
  float tgtX = m_rover->tgtX() - m_zeroX;
  float oldTgtX = m_rover->oldTgtX() - m_zeroX;
  
  if (m_chw || tgtMass < m_zeroMass/3)
  {
    int yaw = xPK*tgtX + xIK*(tgtX + oldTgtX) + xDK*(tgtX - oldTgtX);
//    int speed = xPK*tgtMass + massIK*(tgtMass + oldTgtMass) + massDK*(tgtMass - oldTgtMass);
    int speed = powerProportional(tgtMass, 0, m_zeroMass, 100); // back/forward based on ball size
    int backSpeed = powerProportional(tgtY, -100, m_zeroY, 100); // move back/forward if ball leaves range

    int speedL = (-(speed+backSpeed)+yaw)/2;
    int speedR = (-(speed+backSpeed)-yaw)/2;

//    qDebug() << "Chasis l: " << speedL << "x r: " << speedR;
    m_rover->manualControlChasis(speedL, speedR);
  }
  else
  {
    m_rover->manualControlChasis(0, 0);
  }

}

void StateTracking::runArm()
{
  int speed;

  speed = powerProportional(m_rover->tgtY(), -100, m_zeroY, 100);

//  qDebug() << "Arm: " << speed;

  if (abs(speed)>15){ 
    m_chw = false;
    speed += sign(speed)*20;
  } else {
    speed = 0;
    m_chw = true;
  }

  m_rover->manualControlArm(-speed);
}

void StateTracking::runHand()
{
  m_rover->manualControlHand(0);

  int tgtX = m_rover->tgtX();
  int tgtY = m_rover->tgtY();
  int tgtMass = m_rover->tgtMass();

  int diffX = powerProportional(tgtX, -100, m_zeroX, 100);
  int diffY = powerProportional(tgtY, -100, m_zeroY, 100);
  int diffMass = powerProportional(tgtMass, 0, m_zeroMass, 100);

  m_fixed = (abs(diffX) <= 10
          && abs(diffY) <= 10
          && abs(diffMass) <= 10);

/*
#warning DEBUG
  qDebug()
  << diffX << " "
  << diffY << " "
  << diffMass << " "
  << (m_fixed ? "###LOCK###" : "")
  << "(" << tgtX    << "->" << m_zeroX    << " " 
         << tgtY    << "->" << m_zeroY    << " " 
         << tgtMass << "->" << m_zeroMass << ")";
*/

  if (m_fixed)
  {
    if (!m_locker.isActive())
      m_locker.start(1000);
  }
  else
    m_locker.stop();
}

void StateTracking::check()
{
  switch(m_mode)
  {
    case UntilMass:
      if (m_zeroMass > m_rover->tgtMass())
      {
        qDebug() << "MASS REACHED";
        disconnect(m_rover, SIGNAL(locationChanged()), this, SLOT(check()));
        disconnect(m_rover, SIGNAL(locationChanged()), this, SLOT(run()));
        emit finished(m_nextState);
      }
      break;
    case UntilLocked:
        qDebug() << "TRAGET LOCKED";

        m_rover->stopRover();
//stop()?
        disconnect(m_rover, SIGNAL(locationChanged()), this, SLOT(massController()));
        disconnect(&m_locker, SIGNAL(timeout()), this, SLOT(check()));
        disconnect(m_rover, SIGNAL(locationChanged()), this, SLOT(run()));
//
        emit finished(m_nextState);
    default:
      break;
  }
}

void StateTracking::stop()
{
  qDebug() << "Tracking stoped";
  disconnect(m_rover, SIGNAL(locationChanged()), this, SLOT(run()));
  disconnect(&m_locker, SIGNAL(timeout()), this, SLOT(check()));
  disconnect(m_rover, SIGNAL(locationChanged()), this, SLOT(massController()));
}
