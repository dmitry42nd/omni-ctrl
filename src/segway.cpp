#include <QDebug>
#include <QVector>
#include <trikControl/brickFactory.h>
#include <math.h>
#include "segway.h"

const QString l = "M3";
const QString r = "M4";
const QString le = "B3";
const QString re = "B4";

const double fullBattery = 12.7;
const double parToDeg = 0.07;
const int gdcPeriod  = 4000;
const int mainPeriod = 6;
const int minPow = 2;

inline double sgn(double x) { return x > 0 ? 1 : (x < 0 ? -1 : 0); }
inline double abs(double x) { return x > 0 ? x : -x; }
inline double sat(double a, double b) { return abs(a) > b ? sgn(a) * b : a; }

Segway::~Segway() {
  disconnectAll();
  m_brick->stop();
}  

Segway::Segway(QApplication *app, 
               QString systemConfigPath, QString modelConfigPath, QString startDirPath, 
               double pk, double dk, double ik, double ck, double ofs,
               int accGAxis, int accOAxis, int gyroAxis):
  m_app(app),
//  m_brick(*(app->thread()), configPath, startDirPath),
  m_fbControl(),
  m_rlControl(),
  m_state(PID_CONTROL1),
  m_pk(pk),
  m_dk(dk),
  m_ik(ik),
  m_ck(ck),
  m_offset(ofs),
  m_accGAxis(accGAxis),
  m_accOAxis(accOAxis),
  m_gyroAxis(gyroAxis),
  m_cnt()
{
  m_brick = BrickFactory::create(*(app->thread()), systemConfigPath, modelConfigPath, startDirPath);
  qDebug() << "SEGWAY_STARTS";
  connect(m_brick->keys(), SIGNAL(buttonPressed(int,int)), this, SLOT(onBtnPressed(int,int)));
 
 /*
  connect(m_brick->gamepad(), SIGNAL(button(int,int)),  this, SLOT(onGamepadBtnChanged(int, int)));
  connect(m_brick->gamepad(), SIGNAL(pad(int,int,int)), this, SLOT(onGamepadPadDown(int,int,int)));
  connect(m_brick->gamepad(), SIGNAL(padUp(int)),       this, SLOT(onGamepadPadUp(int)));
*/
  startUpdatingBC();  
  startDriftAccumulation();
  QTimer::singleShot(gdcPeriod,      this, SLOT(stopDriftAccumulation()));
  QTimer::singleShot(gdcPeriod + 10, this, SLOT(startDancing()));
}

void Segway::startUpdatingBC()
{
  m_bc = 1;  
  connect(&m_bcTicker, SIGNAL(timeout()), this, SLOT(updateBC()));
  m_bcTicker.start(5000);
}

void Segway::updateBC()
{
  m_bc = fullBattery / m_brick->battery()->readVoltage();
}

void Segway::startDriftAccumulation()
{
  qDebug() << "START_DRIFT_ACCUMULATION";

  m_gyroDrift    = 0;  
  m_gyroDriftCnt = 0;
  
  connect(&m_mainTicker, SIGNAL(timeout()), this, SLOT(accumulateDrift()));
  m_mainTicker.start(mainPeriod);
}

void Segway::stopDriftAccumulation()
{
  qDebug() << "STOP_DRIFT_ACCUMULATION";
  
  m_mainTicker.stop();
  disconnect(&m_mainTicker, SIGNAL(timeout()), this, SLOT(accumulateDrift()));

  m_gyroDrift /= m_gyroDriftCnt;
  qDebug() << "gyro[0] drift is: " << m_gyroDrift;
}

void Segway::accumulateDrift()
{
  int gd = m_brick->gyroscope()->read()[m_gyroAxis];
  m_gyroDrift += gd;
  m_gyroDriftCnt++;
}

void Segway::startDancing()
{
  qDebug() << "START_DANCING";
  m_outData    = 0;
  m_outDataOld = 0;
  m_brick->encoder(le)->reset();
  m_brick->encoder(re)->reset();
  
  connect(&m_mainTicker, SIGNAL(timeout()), this, SLOT(dance()));
  m_dbgTicker.restart();
  m_mainTicker.start(mainPeriod);
}

void Segway::dance()
{
 
  QVector<int> acc = m_brick->accelerometer()->read();
  double acceData  = atan2(acc[m_accOAxis],acc[m_accGAxis]) * 180.0/3.14159;
//or double acceData  = m_brick->accelerometer()->read()[acceAxis] * 180.0/(3.14159*4096);
  int tmpElapsed = m_dbgTicker.elapsed();
  double gyroData  = (m_brick->gyroscope()->read()[m_gyroAxis] - m_gyroDrift)*tmpElapsed*parToDeg/1000.0;
  m_dbgTicker.restart();
  
  m_outData     = (1 - m_ck)*(m_outData + gyroData) + m_ck*acceData;
  double angle  = m_outData - m_offset;
  double leverage = angle + m_fbControl;

  int yaw = m_bc*(sgn(leverage)*minPow + leverage*m_pk + (leverage-m_outDataOld)*m_dk + (leverage+m_outDataOld)*m_ik);
  m_outDataOld = angle; 

  if (abs(angle) < 45) {
    m_brick->motor(l)->setPower(yaw + m_rlControl);
    m_brick->motor(r)->setPower(yaw - m_rlControl);
  } else {
    m_brick->motor(l)->setPower(0);
    m_brick->motor(r)->setPower(0);
  }


  if (m_cnt == 10) {
    //qDebug("angle speed: %1.5f %d encoder l r: %d %d", angle, yaw, m_brick->encoder(le)->readRawData(), -m_brick->encoder(re)->readRawData());
    qDebug("angle speed: %1.5f elapsed: %d", angle, tmpElapsed);
    m_cnt = 0;
  }
  m_cnt++;

}

void Segway::disconnectAll() 
{
  disconnect(&m_mainTicker, SIGNAL(timeout()), this, SLOT(accumulateDrift()));
  disconnect(&m_mainTicker, SIGNAL(timeout()), this, SLOT(dance()));
  disconnect(m_brick->keys(), SIGNAL(buttonPressed(int,int)), this, SLOT(onBtnPressed(int,int)));
 /*
  disconnect(m_brick->gamepad(), SIGNAL(button(int,int)),  this, SLOT(onGamepadBtnChanged(int, int)));
  disconnect(m_brick->gamepad(), SIGNAL(pad(int,int,int)), this, SLOT(onGamepadPadDown(int,int,int)));
  disconnect(m_brick->gamepad(), SIGNAL(padUp(int)),       this, SLOT(onGamepadPadUp(int)));
*/
  disconnect(&m_bcTicker, SIGNAL(timeout()), this, SLOT(updateBC()));
}

//controls
void Segway::onBtnPressed(int code, int state)
{
  if(state == 0) return;
  
  switch(code) {
    case 103:
      m_app->quit();
    case 108:
      m_brick->encoder(le)->reset();
      m_brick->encoder(re)->reset();
    case 105:
      m_offset = m_outData;
    default : break;
  }
}

/*
void Segway::onGamepadPadDown(int pd ,int x, int y) 
{
  if (pd == 1) 
    switch (m_state) {
      case MOVEMENT_CONTROL: 
        m_fbControl = y/200.0; 
        m_rlControl = x/3.0;
         break;
      case PID_CONTROL1: 
        m_pk += x >= 0 ? 0.1 : -0.1;
        break;
      case PID_CONTROL2:
        m_ck += x >= 0 ? 0.001 : -0.001;
        break;
    }
  else
    switch (m_state) {
      case MOVEMENT_CONTROL: 
        break;
      case PID_CONTROL1: 
        m_dk += x >= 0 ? 0.1 : -0.1; 
        break;
      case PID_CONTROL2:
        m_ik += x >= 0 ? 0.1 : -0.1; 
        break;
    }
}

void Segway::onGamepadPadUp(int pd) 
{
  if (pd == 1) 
    switch (m_state) {
      case MOVEMENT_CONTROL: 
        m_fbControl = 0; 
        m_rlControl = 0;
        break;
      case PID_CONTROL1: 
      case PID_CONTROL2:
        break;
    }
  else
    switch (m_state) {
      case MOVEMENT_CONTROL: 
      case PID_CONTROL1: 
      case PID_CONTROL2:
        break;
    }
}

void Segway::onGamepadBtnChanged(int code, int state)
{
  if (state == 0) return;
  
  switch(code) {
    case 1 : m_state = PID_CONTROL1; break;
    case 2 : m_state = PID_CONTROL2; break;
    case 3 : m_state = MOVEMENT_CONTROL; break;
    default : break;
  }
}
*/
