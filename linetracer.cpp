#include <QDebug>

#include "linetracer.h"

const QString logFifoPath="/tmp/dsp-detector.out.fifo";
const QString cmdFifoPath="/tmp/dsp-detector.in.fifo";

const int SPEED = 60;
const qreal PK = 0.35;
const qreal IK = 0.003;
const qreal DK = -0.009;

Linetracer::Linetracer(QThread *guiThread, QString configPath):
  m_logFifo(logFifoPath),
  m_cmdFifo(cmdFifoPath),
  m_brick(*guiThread, configPath),
  m_motorControllerL(m_brick, "JM1", "JB4"),
  m_motorControllerR(m_brick, "JM3", "JB3"),
  m_motorsWorkerThread(),
  m_zeroX(-20)
{
  m_logFifo.open();
  m_cmdFifo.open();

  qDebug() << "LINETRACER_STARTS";

  connect(&m_logFifo, SIGNAL(lineColorDataParsed(int, int, int, int, int, int)),  
          this, SLOT(setLineColorData(int, int, int, int, int, int)));
  connect(m_brick.keys(),    SIGNAL(buttonPressed(int,int)), this, SLOT(onBrickButtonChanged(int,int)));

  m_motorControllerL.moveToThread(&m_motorsWorkerThread);
  m_motorControllerR.moveToThread(&m_motorsWorkerThread);
  m_motorControllerL.startAutoControl();
  m_motorControllerR.startAutoControl();
  m_motorsWorkerThread.start();

//init state is MANUAL_MODE:
  manualMode();
}

Linetracer::~Linetracer()
{
  m_motorControllerL.stopAutoControl();
  m_motorControllerR.stopAutoControl();

  m_motorsWorkerThread.quit();
  m_motorsWorkerThread.wait();
}

void Linetracer::manualMode()
{
  movementMode = MANUAL_MODE;
  qDebug() << "MANUAL_MODE";

  disconnect(&m_logFifo, SIGNAL(lineTargetDataParsed(int, int, int)), this, SLOT(setLineTargetData(int, int, int)));
  m_motorControllerL.setActualSpeed(0);
  m_motorControllerR.setActualSpeed(0);

  connect(m_brick.gamepad(), SIGNAL(pad(int,int,int)), this, SLOT(onGamepadPadDown(int,int,int)));
  connect(m_brick.gamepad(), SIGNAL(padUp(int)),       this, SLOT(onGamepadPadUp(int)));
}

void Linetracer::linetraceMode()
{
  qDebug() << "LINETRACE_MODE";
  movementMode = LINETRACE_MODE;

  connect(&m_logFifo, SIGNAL(lineTargetDataParsed(int, int, int)), this, SLOT(setLineTargetData(int, int, int)));
  disconnect(m_brick.gamepad(), SIGNAL(pad(int,int,int)), this, SLOT(onGamepadPadDown(int,int,int)));
  disconnect(m_brick.gamepad(), SIGNAL(padUp(int)),       this, SLOT(onGamepadPadUp(int)));
}

void Linetracer::onGamepadPadDown(int padNum, int vx, int vy)
{
  if (padNum != 1) return;

  m_motorControllerL.setActualSpeed(-vy+vx);
  m_motorControllerR.setActualSpeed(vy-vx);
}

void Linetracer::onGamepadPadUp(int padNum)
{
  if (padNum != 1) return;

  m_motorControllerL.setActualSpeed(0);
  m_motorControllerR.setActualSpeed(0);
}

void Linetracer::onGamepadButtonChanged(int buttonNum, int state)
{
  if (state == 0) return; //in case of
  
  switch (buttonNum)
  {
    case 1: 
      if(movementMode != MANUAL_MODE)
      {
        manualMode();
      }
      break;
    case 2: 
      if(movementMode != LINETRACE_MODE )
      {
        linetraceMode();
      }
      break;
  }
}

void Linetracer::onBrickButtonChanged(int buttonCode, int state)
{
  if (state == 0) return;

  switch (buttonCode)
  {
    case 62:  
      m_cmdFifo.write("detect\n");
      break;
    case 60:  
      if(movementMode != LINETRACE_MODE)
      {
        linetraceMode();
      } else
      {
        manualMode();
      }
      break;
    case 64:  
      m_zeroX = m_X;
      qDebug() << "zero x: " << m_zeroX;
      break;
    default:
      manualMode();
  }
}

void Linetracer::setLineColorData(int hue, int hueTol, int sat, int satTol, int val, int valTol)
{
  QString s = QString("hsv %1 %2 %3 %4 %5 %6\n").arg(hue)
                                                .arg(hueTol)
                                                .arg(sat)
                                                .arg(satTol)
                                                .arg(val)
                                                .arg(valTol);
  qDebug() << s;
  m_cmdFifo.write(s);
}


static int m_max(int a, int b)
{
  return a >= b ? a : b;
}


static int m_min(int a, int b)
{
  return a <= b ? a : b;
}

static int saturate(int min, int val, int max)
{
  return (val <= max) ? ((val >= min) ? val : min) : max;
}


static int powerProportional(int _val, int _min, int _zero, int _max)
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

void Linetracer::setLineTargetData(int x, int angle, int size)
{
  qDebug("xyz: %d, %d, %d", x, angle, size);

  m_oldX  = m_X;
  m_X     = powerProportional(x, -100, m_zeroX, 100);
  m_angle = angle;
  m_size  = size;

  if(abs(x) < 10 && abs(m_oldX) < 10) //just move straight
  {
    m_motorControllerL.setActualSpeed(SPEED);
    m_motorControllerR.setActualSpeed(SPEED);
  } 
  else
  {    
    float P, I, D;

    P = x * PK;
    D = (x - m_oldX) * DK;
    I = (x + m_oldX) * IK;

    float yaw;
    yaw = P + I + D;

  #if 1
    int speedL = SPEED+yaw;  
    int speedR = SPEED-yaw;

    int maxSpeed = m_max(speedL, speedR);
    int minSpeed = m_min(speedL, speedR);

    if (maxSpeed > 100)
    {
      speedL = saturate(0, speedL - (maxSpeed - 100),100);
      speedR = saturate(0, speedR - (maxSpeed - 100),100);
    }

    if (minSpeed < -100)
    {
      speedL = saturate(0, speedL - (minSpeed + 100),100);
      speedR = saturate(0, speedR - (minSpeed + 100),100);
    }
  #endif

    qDebug("lr: %d, %d", speedL, speedR);
    m_motorControllerL.setActualSpeed(speedL);
    m_motorControllerR.setActualSpeed(speedR);
  }
}

