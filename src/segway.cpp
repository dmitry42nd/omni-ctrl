#include <QDebug>
#include <QVector>

#include <math.h>
#include "segway.h"

const QString logFifoPath="/tmp/dsp-detector.out.fifo";
const QString cmdFifoPath="/tmp/dsp-detector.in.fifo";

const QString l = "M3";
const QString r = "M4";

const int gyroAxis = 0;
const int acceAxis = 2;

const double G = 4096;
const double K = 0.022;

const double fullBattery = 12.7;

const int gdcPeriod  = 4000;
const int mainPeriod = 8;

const int minPow = 5;

const double mainPeriodS = mainPeriod/1000.f;
const double parToDeg    = 0.07;
const double gyroCoeff   = mainPeriodS*parToDeg;

inline double sgn(double x) { return x > 0 ? 1 : (x < 0 ? -1 : 0); }
inline double abs(double x) { return x > 0 ? x : -x; }
inline double sat(double a, double b) { return abs(a) > b ? sgn(a) * b : a; }

Segway::~Segway() {}  

Segway::Segway(QThread *guiThread, QString configPath, QString startDirPath, pk, dk, ik):
  m_brick(*guiThread, configPath, startDirPath),
  m_bc(1),
  m_outData(0),
  m_outDataOld(0),
  m_fbControl(0),
  m_rlControl(0),
  m_state(PID_CONTROL1),
  m_pk(pk),
  m_dk(dk),
  m_ik(ik),
  m_offset(2.9),
  m_cnt(0)
{
  qDebug() << "SEGWAY_STARTS";
  connect(m_brick.keys(), SIGNAL(buttonPressed(int,int)), this, SLOT(onBtnPressed(int,int)));
 
  connect(m_brick.gamepad(), SIGNAL(button(int,int)),  this, SLOT(onGamepadBtnChanged(int, int)));
  connect(m_brick.gamepad(), SIGNAL(pad(int,int,int)), this, SLOT(onGamepadPadDown(int,int,int)));
  connect(m_brick.gamepad(), SIGNAL(padUp(int)),       this, SLOT(onGamepadPadUp(int)));

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
  m_bc = fullBattery / m_brick.battery()->readVoltage();
}


void Segway::startDriftAccumulation()
{
  qDebug() << "START_DRIFT_ACCUMULATION";

  m_gyroDrift    = 0;  
  m_gyroDriftCnt = 0;
  m_gyroGain     = 0;
  
  connect(&m_mainTicker, SIGNAL(timeout()), this, SLOT(accumulateDrift()));
  m_mainTicker.start(mainPeriod);
}

void Segway::stopDriftAccumulation()
{
  qDebug() << "STOP_DRIFT_ACCUMULATION";
  
  m_mainTicker.stop();
  disconnect(&m_mainTicker, SIGNAL(timeout()), this, SLOT(accumulateDrift()));

  m_gyroDrift /= m_gyroDriftCnt;
  qDebug() << "gyro[0] drif is: " << m_gyroDrift << " gain is: " << m_gyroGain;
}

void Segway::accumulateDrift()
{
  int gd = m_brick.gyroscope()->read()[gyroAxis];
  m_gyroDrift += gd;
  m_gyroGain = abs(gd) > m_gyroGain ? abs(gd) : m_gyroGain;
  m_gyroDriftCnt++;
}

void Segway::startDancing()
{
  connect(&m_mainTicker, SIGNAL(timeout()), this, SLOT(dance()));
  m_dbgTicker.restart();
  m_mainTicker.start(mainPeriod);
}

void Segway::dance()
{
  double acceData  = m_brick.accelerometer()->read()[acceAxis] * 180.0/(3.14159*G);

  double gyroData  = (m_brick.gyroscope()->read()[gyroAxis] - m_gyroDrift)*m_dbgTicker.elapsed()*parToDeg/1000.0;
  m_dbgTicker.restart();
  
  m_outData   = (1-K)*(m_outData + gyroData) + K*acceData;
  double tmp  = m_outData - m_offset;
  double tmp2 = tmp + m_fbControl;

  int yaw = m_bc*(sgn(tmp2)*minPow + 2*(tmp2*m_pk + (tmp2-m_outDataOld)*m_dk + (tmp2+m_outDataOld)*m_ik));
  m_outDataOld = tmp; 

  if (abs(yaw) < 200) {
    m_brick.motor(l)->setPower(yaw+m_rlControl);
    m_brick.motor(r)->setPower(yaw-m_rlControl);
  } else {
    m_brick.motor(l)->setPower(0);
    m_brick.motor(r)->setPower(0);
  }

  if (m_cnt == 10) {
    qDebug("data yaw: %1.5f %d pdi: %1.1f %1.1f %1.1f rr: %1.2f bc: %1.2f", tmp, yaw, m_pk, m_dk, m_ik, m_fbControl, m_bc);
    m_cnt = 0;
  }
  m_cnt++;
}


//controls
void Segway::onGamepadPadDown(int pd ,int x, int y) 
{
  if (pd == 1) 
    switch (m_state) {
      case MOVEMENT_CONTROL: 
        m_fbControl = y/100.0; 
        m_rlControl = x/3.0;
        break;
      case PID_CONTROL1: 
        m_pk += x >= 0 ? 0.1 : -0.1;
        break;
      case PID_CONTROL2:
        m_pk += x >= 0 ? 0.1 : -0.1;
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

void Segway::onBtnPressed(int code, int state)
{
  if(state == 0) return;
  
  switch(code) {
    case 104:
      m_offset = 0;
    case 105:
      m_offset = m_outData;
    default : break;
  }
}
