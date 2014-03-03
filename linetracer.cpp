#include <QDebug>

#include "linetracer.h"

const QString logFifoPath="/tmp/dsp-detector.out.fifo";
const QString cmdFifoPath="/tmp/dsp-detector.in.fifo";

const int speed = 100;
const qreal stopK = 1;
const qreal PK = 0.42;
const qreal IK = 0.006;
const qreal DK = -0.009;
const qreal encC = 1/(334*34); //1 : (num of points of encoder wheel * reductor ratio)

Linetracer::Linetracer(QThread *guiThread, QString configPath):
  m_logFifo(logFifoPath),
  m_cmdFifo(cmdFifoPath),
  m_brick(*guiThread, configPath),
  m_motorControllerL(m_brick, "JM1", "JB4"),
  m_motorControllerR(m_brick, "JM3", "JB3"),
  m_motorsWorkerThread()
{
  m_logFifo.open();
  m_cmdFifo.open();


  qDebug() << "LINETRACER_STARTS";
  connect(&m_logFifo, SIGNAL(lineColorDataParsed(int, int, int, int, int, int)),  
          this, SLOT(setLineColorData(int, int, int, int, int, int)));
  connect(m_brick.gamepad(), SIGNAL(button(int,int)),        this, SLOT(onGamepadButtonChanged(int, int)));
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
    case 28:  
      m_cmdFifo.write("detect\n");
      break;
    case 105:  
      if(movementMode != LINETRACE_MODE)
      {
        linetraceMode();
        break;
      } 
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
//  qDebug() << s;
  m_cmdFifo.write(s);
}

void Linetracer::setLineTargetData(int x, int angle, int mass)
{
//  qDebug("line x, angle: %d, %d", x, angle);
  m_prevTgtX = m_tgtX;
  m_tgtX     = x;
  m_tgtAngle = angle;
  m_tgtMass  = mass;

  m_motorControllerL.setActualSpeed(x);
  m_motorControllerR.setActualSpeed(x);
}

