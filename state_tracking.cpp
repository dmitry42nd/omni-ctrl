//#include <QDebug>

#include "state_tracking.h"
#include "rover.h"

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
  qDebug() << "START_TRACKING";

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
  int tgtMass = m_rover->tgtMass();
  int tgtY = m_rover->tgtY();
  int tgtX = m_rover->tgtX();
  
  int yaw;
  int speed;
  int backSpeed;

  yaw = m_rover->tgtX();
  yaw += m_lastYaw/5;
  m_lastYaw = yaw;

  speed = powerProportional(tgtMass, 0, m_zeroMass, 100); // back/forward based on ball size
  backSpeed = powerProportional(tgtY, -100, m_zeroY, 100); // move back/forward if ball leaves range

  if(m_chw || tgtMass < m_zeroMass/6)
  {

    speed += backSpeed;

    int m_const = 10;
    int m_const2 = 2;
    int speedL = (-speed+yaw);

    if (speedL >= m_const)
      speedL = m_const+(speedL-m_const)/m_const2;
    else if (speedL <= -m_const)
      speedL = -m_const+(speedL+m_const)/m_const2;

    int speedR = (-speed-yaw);

    if (speedR >= m_const)
      speedR = m_const+(speedR-m_const)/m_const2;
    else if (speedR <= -m_const)
      speedR = -m_const+(speedR+m_const)/m_const2;

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


#warning DEBUG
  qDebug()
  << diffX << " "
  << diffY << " "
  << diffMass << " "
  << (m_fixed ? "###LOCK###" : "")
  << "(" << tgtX    << "->" << m_zeroX    << " " 
         << tgtY    << "->" << m_zeroY    << " " 
         << tgtMass << "->" << m_zeroMass << ")";


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
        disconnect(m_rover, SIGNAL(locationChanged()), this, SLOT(run()));
        disconnect(&m_locker, SIGNAL(timeout()), this, SLOT(check()));
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
}
