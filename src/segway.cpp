#include <QDebug>
#include <QVector>

#include <math.h>
#include "segway.h"

const QString logFifoPath="/tmp/dsp-detector.out.fifo";
const QString cmdFifoPath="/tmp/dsp-detector.in.fifo";

const QString l = "M3";
const QString r = "M4";


const double pk = 8.5;
const double dk = 12;
const double ik = 1;

const int gyroAxis = 0;
const int acceAxis = 2;

const double G = 4096;
const double K = 0.02; //0.02 

const int gdcPeriod  = 4000;
const int mainPeriod = 10;

const int minPow = 5;

const double mainPeriodS = mainPeriod/1000.f;
const double parToDeg    = 0.07;
const double gyroCoeff   = mainPeriodS*parToDeg;

inline double sgn(double x) { return x > 0 ? 1 : (x < 0 ? -1 : 0); }
inline double abs(double x) { return x > 0 ? x : -x; }
inline double sat(double a, double b) { return abs(a) > b ? sgn(a) * b : a; }

Segway::~Segway() {}  

Segway::Segway(QThread *guiThread, QString configPath, QString startDirPath):
  m_brick(*guiThread, configPath, startDirPath),
  m_outData(),
  m_outDataOld(),
  m_offset(2.9),
  m_state(1),
  m_pk(6.5),
  m_dk(11.2),
  m_ik(0.2),
  m_rowrow(0)
{
  qDebug() << "SEGWAY_STARTS";
  connect(m_brick.keys(), SIGNAL(buttonPressed(int,int)), this, SLOT(onBtnPressed(int,int)));
  
  connect(m_brick.keys(), SIGNAL(buttonPressed(int,int)), this, SLOT(onBtnPressed(int,int)));
  
  connect(m_brick.gamepad(), SIGNAL(button(int,int)), this, SLOT(onGamepadBtnChanged(int, int)));
  connect(m_brick.gamepad(), SIGNAL(pad(int,int,int)), this, SLOT(onGamepadPadDown(int,int,int)));
  connect(m_brick.gamepad(), SIGNAL(padUp(int)),       this, SLOT(onGamepadPadUp(int)));

  startDriftAcc();
  QTimer::singleShot(gdcPeriod     , this, SLOT(stopDriftAcc()));
  QTimer::singleShot(gdcPeriod + 10, this, SLOT(startDancing()));
}


void Segway::onGamepadPadDown(int pd ,int x, int y) 
{
  if(m_state == 3)
    if(pd == 1)
      m_rowrow = y/50.0;
      m_wewwew = x/10.0;
  else {
    if (pd == 1)
      m_pk += x >= 0 ? 0.1 : -0.1;
    else //if (pd == 2)
      switch(m_state) {
        case 1: m_dk += x >= 0 ? 0.1 : -0.1; break;
        case 2: m_ik += x >= 0 ? 0.1 : -0.1; break;
      }
  }


void Segway::onGamepadPadUp(int pd) 
{
  if(pd == 1)
    m_rowrow = 0;
    m_wewwew = 0;
}

void Segway::onGamepadBtnChanged(int code, int state)
{
  if (state == 0) return;
  
  switch(code) {
    case 1 : m_state = 1; break;
    case 2 : m_state = 2; break;
    case 3 : m_state = 3; break;
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

void Segway::startDriftAcc()
{
  qDebug() << "START_DRIFT_ACCUMULATION";

  m_gyroDrift     = 0;  
  m_gyroDriftCnt  = 0;
  m_gyroGain = 0;
  connect(&m_mainTicker, SIGNAL(timeout()), this, SLOT(accDrift()));
  m_mainTicker.start(mainPeriod);
}

void Segway::stopDriftAcc()
{
  qDebug() << "STOP_DRIFT_ACCUMULATION";
  
  m_mainTicker.stop();
  disconnect(&m_mainTicker, SIGNAL(timeout()), this, SLOT(accDrift()));

  m_gyroDrift /= m_gyroDriftCnt;
  
  qDebug() << "gyro[0] drif is: " << m_gyroDrift << " gain is: " << m_gyroGain;
}

void Segway::accDrift()
{
  int gd = m_brick.gyroscope()->read()[gyroAxis];
  m_gyroDrift += gd;
  m_gyroGain = abs(gd) > m_gyroGain ? abs(gd) : m_gyroGain;
  m_gyroDriftCnt++;
}

void Segway::startDancing()
{
  connect(&m_mainTicker, SIGNAL(timeout()), this, SLOT(dance()));
  m_mainTicker.start(mainPeriod);
}

void Segway::dance()
{
  int gyroData = m_brick.gyroscope()->read()[gyroAxis] - m_gyroDrift;
  int acceData = m_brick.accelerometer()->read()[acceAxis];
 
  m_gyroData = gyroData*parToDeg*mainPeriodS;
  m_acceData  = acceData*(180/3.14159*G);
  
  m_outData = ((1-K)*(m_outData + m_gyroData) + K*m_acceData);
  double tmp = m_outData - m_offset;
  double tmp2 = tmp + m_rowrow;
  int yaw  = 2*(tmp2*m_pk + (tmp2-m_outDataOld)*m_dk + (tmp2+m_outDataOld)*ik);
  m_outDataOld = tmp;

  m_brick.motor(l)->setPower(yaw+m_wewwew);
  m_brick.motor(r)->setPower(yaw-m_wewwew);

  qDebug("data yaw: %1.5f %d pdi: %1.1f %1.1f %1.1f rr: %1.2f\n", tmp, yaw, m_pk, m_dk, m_ik,m_rowrow);
}
