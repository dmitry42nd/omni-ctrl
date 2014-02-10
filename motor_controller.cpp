#include <QDebug>

#include "motor_controller.h"

const double max_ppms = 23.833f; //201rpm*(334points*21.3reductor ration)/60000ms
MotorController::MotorController(const Brick& brick, const QString port, const QString enc_port) :
m_port(port),
m_enc_port(enc_port),
m_brick(brick),
m_actualSpeed(0),
m_currentSpeed(0),
m_encOldData(0),
m_encData(0),
m_old_ppms(0)
{}

MotorController::~MotorController()
{}

void MotorController::doStep()
{
  long long outer =  m_stopwatch.lap()/1000;
  m_stopwatch.start();

//  double period = ((m_time.tv_sec - m_oldTime.tv_sec)*1000000000.0 +  m_time.tv_nsec - m_oldTime.tv_nsec)/1000000.0;
//  float ppms = (m_encData-m_encOldData)/period;

//  if(m_actualSpeed != 0 && m_port == "4")
//    qDebug() << m_port << " " << period << ": " << m_time.tv_sec << " " << m_time.tv_nsec/1000000.0 << " " << ppms;
/*
  m_encOldData = m_encData;
  m_oldTime = m_time;
  m_oldRealTime = m_realTime;
*/
//  m_old_ppms = ppms;

//  clock_gettime(CLOCK_MONOTONIC, &m_time);

//  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &m_realTime);
  //long long tmp1 = m_stopwatch.lap();

  m_encData = m_brick.encoder(m_enc_port)->read();
  long long tmp1 = m_stopwatch.lap()/1000;

  if(m_actualSpeed == 0)
  {
      m_brick.powerMotor(m_port)->setPower(0);
      m_timer.stop();
  } else {
    //  emit correctionDone();      
    m_brick.powerMotor(m_port)->setPower(m_actualSpeed);
  }

  long long tmp2 = m_stopwatch.lap()/1000;
  qDebug() << QString("%1: %2 %3 %4").arg(m_port).arg(outer).arg(tmp1).arg(tmp2);

}

void MotorController::startAutoControl()
{
  m_actualSpeed = 0;
  m_currentSpeed = 0;
  m_encOldData = 0;
  m_encData = 0;
  memset(&m_time,0, sizeof(m_time));
  memset(&m_oldTime,0, sizeof(m_oldTime));
  memset(&m_realTime,0, sizeof(m_realTime));
  memset(&m_oldRealTime,0, sizeof(m_oldRealTime));
  m_brick.encoder(m_enc_port)->reset();

  connect(&m_timer, SIGNAL(timeout()), this, SLOT(doStep()), Qt::QueuedConnection);
  m_stopwatch.start();
//  m_timer.start(0);
}

void MotorController::stopAutoControl()
{
  m_timer.stop();
  disconnect(this, SIGNAL(correctionDone()), this, SLOT(doStep()));
}

void MotorController::setActualSpeed(int speed)
{ 
  m_actualSpeed = speed;

  m_timer.start(5);
}

