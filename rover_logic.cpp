#include <QDebug>

#include "rover_logic.h"

#define TARGET_MIN_SIZE 5
#define CLOCK_INTERVAL 50
#define LOCKER_INTERVAL 1000

const float xPK = 0.3;
const float xIK = 0.005;
const float xDK = 0.03;

RoverLogic::RoverLogic(const RoverEngine& _engine):
m_engine(_engine),
m_chw(true),
m_lastYaw(0),
m_lastChasisSpeed(0),
m_lastX(0),
m_lastArmSpeed(0)
{
  m_clock.setInterval(CLOCK_INTERVAL);
  m_locker.setInterval(LOCKER_INTERVAL);
}

RoverLogic::~RoverLogic()
{}

void RoverLogic::start()
{
  m_state=SEARCH;

  connect(&m_clock, SIGNAL(timeout()), this, SLOT(loop()));
  connect(&m_locker, SIGNAL(timeout()), this, SLOT(readyCop()));

  m_clock.start();
}

void RoverLogic::stop()
{
  m_clock.stop();
  m_locker.stop();

  disconnect(&m_clock, SIGNAL(timeout()), this, SLOT(loop()));
  disconnect(&m_locker, SIGNAL(timeout()), this, SLOT(readyCop()));

  m_engine.stop();
}

void RoverLogic::loop()
{
  switch(m_state)
  {
    case SEARCH:
     if (m_currentLoc.s > TARGET_MIN_SIZE)
      {
        qDebug() << "Found target";
        m_state = TRACK;
        break;
      }
      search();
      break;
    case TRACK:
      if (m_currentLoc.s <= TARGET_MIN_SIZE)
      {
        qDebug() << "Lost target";
        m_state = SEARCH;
        break;
      }
      track();
      break;
    case COP:
      break;
    case RELEASE:
      break;
    default:
      stop();
      break;
  }

}

void RoverLogic::search()
{
  m_engine.moveChasis(-20, 20);
}

void RoverLogic::track()
{
  trackChasis();
  trackArm();
  lockCheck();
}

void RoverLogic::trackChasis()
{
  if (m_chw || m_currentLoc.s < m_zeroLoc.s/3)
  {
    int x = powerProportional(m_currentLoc.x, -100, m_zeroLoc.x, 100);
    int yaw = xPK*m_currentLoc.x;// + xIK*(m_currentLoc.x + m_lastX) + xDK*(m_currentLoc.x - m_lastX);
    int speed = powerProportional(m_currentLoc.s, 0, m_zeroLoc.s+10, 100); // back/forward based on ball size

    m_lastX = x;

    int speedL = (-speed+yaw)/2;
    int speedR = (-speed-yaw)/2;

    qDebug() << "Chasis l: " << sign(speedL)*5+speedL << "x r: " << sign(speedR)*5+speedR;

    m_engine.moveChasis(sign(speedL)*5+speedL, sign(speedR)*5+speedR);
  }
  else
  {
    m_engine.moveChasis(0, 0);
  }
 
}

void RoverLogic::trackArm()
{
  int speed = powerProportional(m_currentLoc.y, -100, m_zeroLoc.y, 100);

  if (abs(speed)>10){
    m_chw = false;
  } else {
    m_chw = true;
  }

 qDebug() << "Arm: " << -speed;
 m_engine.moveArm(-speed);
}

void RoverLogic::lockCheck()
{
  int diffX = powerProportional(m_currentLoc.x, -100, m_zeroLoc.x, 100);
  int diffY = powerProportional(m_currentLoc.y, -100, m_zeroLoc.y, 100);
  int diffMass = powerProportional(m_currentLoc.s, 0, m_zeroLoc.s, 100);

  bool fixed = (abs(diffX) <= 10
             && abs(diffY) <= 10
             && abs(diffMass) <= 10);

  if (fixed)
  {
    if (!m_locker.isActive())
      m_locker.start();
  }
  else
    m_locker.stop();
}

void RoverLogic::readyCop()
{
  disconnect(&m_locker, SIGNAL(timeout()), this, SLOT(readyCop()));
  m_locker.stop();
  m_state=COP;

  cop();
}

void RoverLogic::cop()
{
  qDebug() << "COP";

  m_engine.moveChasis(0, 0);
  m_engine.moveArm(0);
  m_engine.moveHand(90);

  m_state=RELEASE;
  release();
}

void RoverLogic::release()
{
  qDebug() << "RELEASE";

  QTimer::singleShot(500,  this, SLOT(turnLeft()));  
  QTimer::singleShot(2500, this, SLOT(doRelease()));
  QTimer::singleShot(3000, this, SLOT(turnRight()));
  QTimer::singleShot(5000, this, SLOT(start()));

}

void RoverLogic::turnLeft()
{
  m_engine.moveArm(60);
  m_engine.moveChasis(-30, 30);
}

void RoverLogic::doRelease()
{
  m_engine.moveArm(0);
  m_engine.moveChasis(0, 0);
  m_engine.moveHand(-90);
}

void RoverLogic::turnRight()
{
  m_engine.moveChasis(30, -30);
}

int RoverLogic::sign(int _v)
{
  return (_v < 0) ? -1 : ((_v > 0) ? 1 : 0);
}

int RoverLogic::powerIntegral(int _power, int _lastPower, int _percent)
{
  if (sign(_power) == sign(_lastPower))
    _power += (_lastPower * _percent) / 100 + sign(_lastPower);

  return _power;
}

int RoverLogic::powerProportional(int _val, int _min, int _zero, int _max)
{
  int adj = _val - _zero;
  if (adj > 0)
  {
    if (_val >= _max)
      return 100;
    else
      return (+100*(_val-_zero)) / (_max-_zero); // _max!=_zero, otherwise (_val>=_max) matches
  }
  else if (adj < 0)
  {
    if (_val <= _min)
      return -100;
    else
      return (-100*(_val-_zero)) / (_min-_zero); // _min!=_zero, otherwise (_val<=_min) matches
  }
  else
    return 0;
}
