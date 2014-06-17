#include <QDebug>

#include "rover_logic.h"

#define TARGET_MIN_SIZE 5
#define CLOCK_INTERVAL 50

RoverLogic::RoverLogic()
{
  m_clock.setInterval(CLOCK_INTERVAL);
}

RoverLogic::~RoverLogic()
{}

void RoverLogic::start()
{
  connect(&m_clock, SIGNAL(timeout()), this, SLOT(loop()));
  connect(&m_locker, SIGNAL(timeout()), this, SLOT(cop()));

  m_state=SEARCH;
  m_clock.start();
}

void RoverLogic::stop()
{
  emit stopSignal();
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
      cop();
      break;
    case RELEASE:
      release();
      break;
    default:
      stop();
      break;
  }

}

void RoverLogic::search()
{
  emit moveChasis(-40, 40);
}

void RoverLogic::track()
{
  trackChasis();
  trackArm();
  lockCheck();
}

void RoverLogic::trackChasis()
{
  int yaw;
  int speed;
  int backSpeed;

  yaw = powerProportional(m_currentLoc.x, -100, m_zeroLoc.x, 100);
  yaw = powerIntegral(yaw, m_lastYaw, 20);

  speed = powerProportional(m_currentLoc.s, 0, m_zeroLoc.s, 100); // back/forward based on ball size
  backSpeed = powerProportional(m_currentLoc.y, -100, m_zeroLoc.y, 100); // move back/forward if ball leaves range

  if (backSpeed >= 30)
    speed += (backSpeed)*3;

  speed = powerIntegral(speed, m_lastChasisSpeed, 10);

  m_lastYaw = yaw;
  m_lastChasisSpeed = speed;

  int m_const = 10;
  int m_const2 = 2;

  int lS = (-speed+yaw);
  if (lS >= m_const)
    lS = m_const+(lS-m_const)/m_const2;
  else if (lS <= -m_const)
    lS = -m_const+(lS+m_const)/m_const2;

  int rS = (-speed-yaw);
  if (rS >= m_const)
    rS = m_const+(rS-m_const)/m_const2;
  else if (rS <= -m_const)
    rS = -m_const+(rS+m_const)/m_const2;

  qDebug() << "Chasis l r: " << lS << " " << rS;
  emit moveChasis(lS, rS);
}

void RoverLogic::trackArm()
{
  int speed;
  speed = powerProportional(m_currentLoc.y, -100, m_zeroLoc.y, 100);
  speed = powerIntegral(speed, m_lastArmSpeed, 10);
  m_lastArmSpeed = speed;

  qDebug() << "Hand: " << speed;
  emit moveArm(speed);
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
      m_locker.start(1000);
  }
  else
    m_locker.stop();
}

void RoverLogic::cop()
{
  m_locker.stop();
  emit moveArm(90);
}

void RoverLogic::release()
{
  emit moveArm(-90);
 
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
