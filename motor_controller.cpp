#include <QDebug>

#include "motor_controller.h"

const double max_ppms = 23.833f; //201rpm*(334points*21.3reductor ration)/60000ms
MotorController::MotorController(const Brick& brick, const QString port, const QString enc_port) :
m_port(port),
m_enc_port(enc_port),
m_brick(brick),
m_actualSpeed(0),
m_currentSpeed(0),
m_timer()
{}

MotorController::~MotorController()
{}

void MotorController::doStep()
{
  m_brick.powerMotor(m_port)->setPower(m_actualSpeed);
}

void MotorController::startAutoControl()
{
  connect(this, SIGNAL(correctionDone()), this, SLOT(doStep()));
}

void MotorController::stopAutoControl()
{
  disconnect(this, SIGNAL(correctionDone()), this, SLOT(doStep()));
}


void MotorController::startSpeedometer()
{
  m_brick.encoder(m_enc_port)->reset();
  connect(this, SIGNAL(speedometerDone()), this, SLOT(countSpeed()), Qt::QueuedConnection);
  countSpeed();
}

void MotorController::stopSpeedometer()
{
  disconnect(this, SIGNAL(speedometerDone()), this, SLOT(countSpeed()));
}

int MotorController::countSpeed()
{
  int encPoints = m_brick.encoder(m_enc_port)->read();
  timespec time;
  clock_gettime(CLOCK_MONOTONIC, &time);

  double period = (time.tv_sec - m_time.tv_sec)*1000.0 +  (time.tv_nsec - m_time.tv_nsec)/1000000.0;
//  int ppms = (encPoints - m_encPoints)/period;
  float speed = (encPoints - m_encPoints)*100.0/(period*max_ppms);

  if(m_currentSpeed != speed)
  {
    qDebug() << "current speed: " << speed << " period: " << period;
  }
  
  m_currentSpeed = speed;
  m_time = time;
  m_encPoints = encPoints;

  emit speedometerDone();

  return speed;
}

void MotorController::setActualSpeed(int speed)
{ 
  if (m_actualSpeed != speed)
  {
    m_actualSpeed = speed;
    emit correctionDone();
  }
}

