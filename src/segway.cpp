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
const double K = 0.025; //0.02 

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
  m_offset()
{
  qDebug() << "SEGWAY_STARTS";
  connect(m_brick.keys(), SIGNAL(buttonPressed(int,int)), this, SLOT(onBtnPressed(int,int)));

  startDriftAcc();
  QTimer::singleShot(gdcPeriod     , this, SLOT(stopDriftAcc()));
  QTimer::singleShot(gdcPeriod + 10, this, SLOT(startDancing()));
}

void Segway::onBtnPressed(int code, int state)
{
  if(state == 0) return;
  
  switch(code) {
    case 105:
      qDebug() << "pew";
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

const double pk = 4.5;
const double dk = 5;
const double ik = 1;

void Segway::dance()
{
  int gyroData = m_brick.gyroscope()->read()[gyroAxis] - m_gyroDrift;
  int acceData = m_brick.accelerometer()->read()[acceAxis];
 
  m_gyroData = gyroData*parToDeg*mainPeriodS;
  m_acceData  = asin(sat(acceData/G, 1))*180/3.14159;
  
  m_outData = ((1-K)*(m_outData+ m_gyroData) + K*m_acceData);
  int yaw  = (sgn(m_outData)*minPow + 2*m_outData*pk + (m_outData-m_outDataOld)*dk + (m_outData+m_outDataOld)*ik);
  m_outDataOld = m_outData;

  m_brick.motor(l)->setPower(yaw);
  m_brick.motor(r)->setPower(yaw);

  
  qDebug() << m_outData << " " << yaw;
}

