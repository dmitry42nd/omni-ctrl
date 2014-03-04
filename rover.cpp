#include <QDebug>

#include "rover.h"

const QString logFifoPath="/tmp/dsp-detector.out.fifo";
const QString cmdFifoPath="/tmp/dsp-detector.in.fifo";

const QString armServoL = "JE1";
const QString armServoR = "JE2";
const QString handServo = "JE4";

const int speed = 100;
const qreal PK = 0.42;
const qreal IK = 0.006;
const qreal DK = -0.009;

Rover::Rover(QThread *guiThread, QString configPath):
  m_logFifo(logFifoPath),
  m_cmdFifo(cmdFifoPath),
  m_brick(*guiThread, configPath),
  m_motorControllerL(m_brick, "JM1", "JB4"),
  m_motorControllerR(m_brick, "JM3", "JB3"),
  m_motorsWorkerThread()
/*,
  m_searching1(this, *m_tracking2, UntilMass)
  m_tracking2(this, *m_searching1)
*/
{
  m_logFifo.open();
  m_cmdFifo.open();


  qDebug() << "ROVER_STARTS";
  connect(&m_logFifo, SIGNAL(ballColorDataParsed(int, int, int, int, int, int)),  
          this, SLOT(setBallColorData(int, int, int, int, int, int)));
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

Rover::~Rover()
{
  m_motorControllerL.stopAutoControl();
  m_motorControllerR.stopAutoControl();

  m_motorsWorkerThread.quit();
  m_motorsWorkerThread.wait();
}

void Rover::manualMode()
{
  movementMode = MANUAL_MODE;
  qDebug() << "MANUAL_MODE";

  disconnect(&m_logFifo, SIGNAL(ballTargetDataParsed(int, int, int)), this, SLOT(setLineTargetData(int, int, int)));
  m_motorControllerL.setActualSpeed(0);
  m_motorControllerR.setActualSpeed(0);

  connect(m_brick.gamepad(), SIGNAL(pad(int,int,int)), this, SLOT(onGamepadPadDown(int,int,int)));
  connect(m_brick.gamepad(), SIGNAL(padUp(int)),       this, SLOT(onGamepadPadUp(int)));
}

void Rover::roverMode()
{
  qDebug() << "ROVER_MODE";
  movementMode = ROVER_MODE;

  connect(&m_logFifo, SIGNAL(ballTargetDataParsed(int, int, int)), this, SLOT(setBallTargetData(int, int, int)));
  disconnect(m_brick.gamepad(), SIGNAL(pad(int,int,int)), this, SLOT(onGamepadPadDown(int,int,int)));
  disconnect(m_brick.gamepad(), SIGNAL(padUp(int)),       this, SLOT(onGamepadPadUp(int)));
}

void Rover::onGamepadButtonChanged(int buttonNum, int state)
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
      if(movementMode != ROVER_MODE )
      {
        roverMode();
      }
      break;
  }
}

void Rover::onBrickButtonChanged(int buttonCode, int state)
{
  if (state == 0) return;

  switch (buttonCode)
  {
    case 28:  
      m_cmdFifo.write("detect\n");
      break;
    case 105:  
      if(movementMode != ROVER_MODE)
      {
        roverMode();
        break;
      } 
    default:
      manualMode();
  }
}

void Rover::setBallColorData(int hue, int hueTol, int sat, int satTol, int val, int valTol)
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


//AUTO CONTROLL
void Rover::setBallTargetData(int x, int y, int mass)
{
//  qDebug("Ball x, y: %d, %d", x, y);

  if (m_tgtX != x || m_tgtY != y || m_tgtMass != mass)
  {
    m_oldTgtX    = m_tgtX;
    m_tgtX       = x;
    
    m_oldTgtY    = m_tgtY;
    m_tgtY       = y;
    m_oldTgtMass = m_tgtMass;
    m_tgtMass    = mass;

    emit locationChanged();
  }

}

//MANUAL CONTROL
void Rover::onGamepadPadDown(int padNum, int vx, int vy)
{

  switch (padNum)
  {
    case 1:
      manualControlChasis(vy-vx, -vy+vx);
      break;
    case 2:
      manualControlArm(vy);
      manualControlHand(vx);
      break;
    default:
      qDebug() << "More than two pads is not provided";
      break;
  }
}

void Rover::onGamepadPadUp(int padNum)
{

  switch (padNum)
  {
    case 1:
      manualControlChasis(0, 0);
      break;
    case 2:
      manualControlArm(0);
      manualControlHand(0);
      break;
    default:
      qDebug() << "More than two pads is not provided";
      break;
  }
}

void Rover::manualControlChasis(int speedL, int speedR)
{
  m_motorControllerL.setActualSpeed(speedL);
  m_motorControllerR.setActualSpeed(speedR);
}

void Rover::manualControlArm(int speed)
{
  m_brick.motor(armServoL)->setPower(speed);
  m_brick.motor(armServoR)->setPower(-speed);
}

void Rover::manualControlHand(int speed)
{
  m_brick.motor(handServo)->setPower(speed);
}

