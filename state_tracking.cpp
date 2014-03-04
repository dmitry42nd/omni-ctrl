#include <QDebug>

#include "state_tracking.h"

StateTracking::StateTracking(Rover* rover, const State* nextState, const StateMode mode) :
State(rover, nextState, mode),
m_zeroMass(20),
m_zeroY(50),
m_zeroX(0),
m_chw(true),
m_locked(false)
{}

StateTracking::~StateTracking()
{}

void StateTracking::init(StateMode mode)
{
  qDebug() << "START_SEARCHING";

  switch (mode)
  {
    case UntilMass:
      m_mode = UntilMass;
      connect(m_rover, SIGNAL(locationChanged()), this, SLOT(check()));
      connect(m_rover, SIGNAL(locationChanged()), this, SLOT(run()));
      break;
    case UntilLocked:
      connect(m_rover, SIGNAL(locationChanged()), this, SLOT(check()));
      connect(m_rover, SIGNAL(locationChanged()), this, SLOT(run()));  
      m_mode = UntilLocked;
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
}

void StateTracking::runChasis()
{
  int yaw;
  int speed;
  int backSpeed;

  yaw = m_rover->tgtX();
  yaw += m_rover->oldTgtX()/10;

  speed = powerProportional(m_rover->tgtMass(), 0, m_zeroMass, 100); // back/forward based on ball size
  backSpeed = powerProportional(m_rover->tgtY(), -100, m_zeroY, 100); // move back/forward if ball leaves range

  if(m_chw || m_rover->tgtMass() < m_zeroMass/6)
  {
    if (backSpeed >= 30)
      speed += (backSpeed)*3;

    speed = speed/2;

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

    qDebug() << "Chasis l: " << speedL << "x r: " << speedR;
    
    m_rover->manualControlChasis(speedL, -speedR);
  }
}

void StateTracking::runArm()
{
  int speed;

  speed = powerProportional(m_rover->tgtY(), -100, m_zeroY, 100);

  qDebug() << "Arm: " << speed;

  if (abs(speed)>0){ 
    m_chw = false;
    speed += 15;
  } else {
    speed = 0;
    m_chw = true;
  }

  m_rover->manualControlArm(speed);
}

void StateTracking::runHand()
{
  m_rover->manualControlHand(0);

  int diffX = powerProportional(m_rover->tgtX(), -100, m_zeroX, 100);
  int diffY = powerProportional(m_rover->tgtY(), -100, m_zeroY, 100);
  int diffMass = powerProportional(m_rover->tgtMass(), 0, m_zeroMass, 100);

  m_locked = (   abs(diffX) <= 10
                  && abs(diffY) <= 10
                  && abs(diffMass) <= 10);

#warning DEBUG
  qDebug()
  << diffX << " "
  << diffY << " "
  << diffMass << " "
  << (m_locked ? "###LOCK###" : "")
  << "(" << m_rover->tgtX()    << "->"<< m_zeroX    << " " 
         << m_rover->tgtY()    << "->"<< m_zeroY    << " " 
         << m_rover->tgtMass() << "->"<< m_zeroMass << ")";
}

void StateTracking::check()
{
  switch(m_mode)
  {
    case UntilMass:
      if (m_zeroMass > m_rover->tgtMass())
      {
        qDebug() << "MASS REACHED";
        disconnect(m_rover, SIGNAL(massChanged(int)), this, SLOT(run()));
        emit finished(m_nextState);
      }
      break;
    case UntilLocked:
      if (m_locked)
      {
        qDebug() << "TRAGET LOCKED";
        disconnect(m_rover, SIGNAL(massChanged(int)), this, SLOT(run()));
        emit finished(m_nextState);
      }
      break;
      
    default:
      break;
  }
}
