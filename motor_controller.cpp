#include <QDebug>

#include "motor_controller.h"

MotorController::MotorController(const Brick& brick, const QString port) :
m_port(port),
m_brick(brick),
m_actualSpeed(0)
{}

MotorController::~MotorController()
{
  stop();
}

void MotorController::doStep()
{
//  qDebug() << "and my data: " << m_tgtX << " " << m_tgtAngle;

  m_brick.powerMotor(m_port)->setPower(m_actualSpeed);

  emit correctionDone();
}

void MotorController::start()
{
  connect(this, SIGNAL(correctionDone()), this, SLOT(doStep()), Qt::QueuedConnection);
  emit correctionDone();
}

void MotorController::stop()
{
  disconnect(this, SIGNAL(correctionDone()), this, SLOT(doStep()));
}

void MotorController::setLineTargetData(int x, int angle, int mass)
{
  m_prevTgtX = m_tgtX;
  m_tgtX     = x;
  m_tgtAngle = angle;
  m_tgtMass  = mass;
}

void MotorController::setActualSpeed(int speed)
{ 
  m_actualSpeed = speed;
}

#if 0

void Linetracer::linetracerMode()
{
/*
  int P = m_tgtX*PK;
  int I = (m_prevTgtX + m_tgtX)*IK;
  int D = (m_prevTgtX - m_tgtX)*DK;
  int yaw = P + I + D;

  int enc_r = (m_brick.encoder("3")->read()); //enc_r
  int enc_l = (m_brick.encoder("4")->read()); //enc_l

  qDebug() << sizeof(int);
  qDebug() << "encs: " << enc_r  << " " << enc_l;

  pwm = velocity(yaw, speed);
  brickPower();

*/
}

QVector2D Linetracer::velocity(int _yaw, int _speed)
{

  QVector2D vPwm = QVector2D();
/*
  int speedL = (_speed+_yaw);  
  int speedR = (_speed-_yaw);
  
  if (speedL > 100)
  {
    speedR -= speedL - 100;
    speedL = 100;
  }
  else if (speedR > 100)
  {
    speedL -= speedR - 100;
    speedR = 100;
  }

  vPwm.setX(speedL);
  vPwm.setY(speedR);

  //qDebug() << "velocity: " << vPwm.x() << " " << vPwm.y() << " " << vPwm.z();  
*/
  return vPwm;

}

#endif

