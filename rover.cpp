#include <QDebug>

#include "rover.h"
/*
#include "../trikRuntime/trikControl/include/trikControl/brick.h"
using namespace trikControl;
*/
const QString logFifoPath="/tmp/dsp-detector.out.fifo";
const QString cmdFifoPath="/tmp/dsp-detector.in.fifo";

const QString armServoL = "JC2";
const QString armServoR = "JC1";
const QString handServo = "JE1";

const int speed = 100;
const qreal PK = 0.42;
const qreal IK = 0.006;
const qreal DK = -0.009;

Rover::Rover(QThread *guiThread, QString configPath):
  m_logFifo(logFifoPath),
  m_cmdFifo(cmdFifoPath),
  m_brick(*guiThread, configPath),
  m_motorControllerL(m_brick, "JM1", "JB4"),
  m_motorControllerR(m_brick, "M1", "JB3"),
  m_motorsWorkerThread(),
//rover mode scenario:
  m_searching1(this, &m_tracking2, UntilMass),
  m_tracking2(this, &m_squeezing3, UntilLocked),
  m_squeezing3(this, &m_releasing4, None),
  m_releasing4(this, &m_finished, None),
  m_finished(this)
//TODO:: disconnect/connect states stuff in resetScenario, and refactor all rover.cpp stuff
{
  m_logFifo.open();
  m_cmdFifo.open();

  m_currentState = &m_searching1;

  qDebug() << "ROVER_STARTS";

  connect(&m_logFifo, SIGNAL(ballColorDataParsed(int, int, int, int, int, int)),  
          this,         SLOT(setBallColorData(int, int, int, int, int, int)));
  connect(&m_logFifo, SIGNAL(ballTargetDataParsed(int, int, int)), this, SLOT(setBallTargetData(int, int, int)));

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

  m_currentState->stop();
  stopRover();

  connect(m_brick.gamepad(), SIGNAL(pad(int,int,int)), this, SLOT(onGamepadPadDown(int,int,int)));
  connect(m_brick.gamepad(), SIGNAL(padUp(int)),       this, SLOT(onGamepadPadUp(int)));
}

void Rover::roverMode()
{
  qDebug() << "ROVER_MODE";
  movementMode = ROVER_MODE;

  disconnect(m_brick.gamepad(), SIGNAL(pad(int,int,int)), this, SLOT(onGamepadPadDown(int,int,int)));
  disconnect(m_brick.gamepad(), SIGNAL(padUp(int)),       this, SLOT(onGamepadPadUp(int)));

  connect(&m_searching1, SIGNAL(finished(State*)), this, SLOT(nextStep(State*)));
  connect(&m_searching1, SIGNAL(failed()), this, SLOT(restart()));

  m_currentState->init();
}

void Rover::nextStep(State* state)
{
  disconnect(m_currentState, SIGNAL(finished(State*)), this, SLOT(nextStep(State*)));
  disconnect(m_currentState, SIGNAL(failed()), this, SLOT(restart()));  

  m_currentState = state;

  connect(m_currentState, SIGNAL(finished(State*)), this, SLOT(nextStep(State*)));
  connect(m_currentState, SIGNAL(failed()), this, SLOT(restart()));

  m_currentState->init();
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
    case 139:  
      if(movementMode != ROVER_MODE)
      {
        roverMode();
      }
      else
      {
        manualMode();
      }
      break;
    case 105:
      m_zeroMass = m_tgtMass;
      m_zeroY    = m_tgtY;
      m_zeroX    = m_tgtX;
      qDebug() << "x y mass: " << m_zeroX << " " << m_zeroY << " " << m_zeroMass;
      break;
    case 103:
      resetScenario();
      break;
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
      manualControlChasis(vy+vx, vy-vx); //seems to be right
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
      m_brick.motor(armServoL)->powerOff();
      m_brick.motor(armServoR)->powerOff();
      m_brick.motor(handServo)->powerOff();
      break;
    default:
      qDebug() << "More than two pads is not provided";
      break;
  }
}

void Rover::manualControlChasis(int speedL, int speedR)
{
  m_motorControllerL.setActualSpeed(-speedL);
  m_motorControllerR.setActualSpeed(-speedR);
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

void Rover::stopRover()
{
  m_brick.motor(handServo)->powerOff();
  m_brick.motor(armServoL)->powerOff();
  m_brick.motor(armServoR)->powerOff();
  m_motorControllerL.setActualSpeed(0);
  m_motorControllerR.setActualSpeed(0);
}

void Rover::restart()
{
  qDebug() << "reseting";
  resetScenario();
  m_currentState->init();
}

void Rover::resetScenario()
{
  m_currentState=&m_searching1;

  m_brick.motor(handServo)->setPower(-100);
  QTimer::singleShot(3000, this, SLOT(stopRover()));
}

