#include <QDebug>

#include "rover.h"

const int key1 = 139;
const int key2 = 103;
const int key3 = 105;
const int key4 = 63;
const int key5 = 64;
const int key6 = 65;

const QString logFifoPath="/tmp/dsp-detector.out.fifo";
const QString cmdFifoPath="/tmp/dsp-detector.in.fifo";

const QString armPort = "JM3";
const QString lPort   = "JM1";
const QString rPort   = "JM4";

const QString handPort  = "JE1";
const QString rotorPort = "JE2";

Rover::Rover(QThread *guiThread, QString configPath, QString soundPath, QString speech):
  m_logFifo(logFifoPath),
  m_cmdFifo(cmdFifoPath),
  m_brick(*guiThread, configPath),
  m_engine(m_brick, lPort, rPort, armPort, handPort),
  m_manual(m_engine),
  m_logic(m_engine)
{
  qDebug() << "ROVER_STARTS";

  void locationParsed(LocationData);
  void colorParsed(ColorData);

  connect(&m_logFifo, SIGNAL(colorParsed(ColorData)),       this, SLOT(setColor(ColorData)));
  connect(&m_logFifo, SIGNAL(locationParsed(LocationData)), this, SLOT(setLocation(LocationData)));

  connect(m_brick.keys(),    SIGNAL(buttonPressed(int,int)), this, SLOT(onBrickButtonChanged(int,int)));

//init state is MANUAL_MODE:
  manualMode();
}

Rover::~Rover()
{}

void Rover::manualMode()
{
  qDebug() << "MANUAL_MODE";
  movementMode = MANUAL_MODE;

  m_logic.stop();

  connect(m_brick.gamepad(), SIGNAL(pad(int,int,int)), &m_manual, SLOT(onPadDown(int,int,int)));
  connect(m_brick.gamepad(), SIGNAL(padUp(int)),       &m_manual, SLOT(onPadUp(int)));
  connect(m_brick.gamepad(), SIGNAL(wheel(int)),       &m_manual, SLOT(onWheel(int)));
}

void Rover::roverMode()
{
  qDebug() << "ROVER_MODE";
  movementMode = ROVER_MODE;

  disconnect(m_brick.gamepad(), SIGNAL(pad(int,int,int)), &m_manual, SLOT(onPadDown(int,int,int)));
  disconnect(m_brick.gamepad(), SIGNAL(padUp(int)),       &m_manual, SLOT(onPadUp(int)));
  disconnect(m_brick.gamepad(), SIGNAL(wheel(int)),       &m_manual, SLOT(onWheel(int)));

  m_logic.start();
}

void Rover::onBrickButtonChanged(int buttonCode, int state)
{
  if (state == 0) return;

  switch (buttonCode)
  {
    case key1:  
      m_cmdFifo.write("detect\n");
      break;
    case key2:  
      if(movementMode != ROVER_MODE)
      {
        roverMode();
      }
      else
      {
        manualMode();
      }
      break;
    case key3:
      qDebug() << "Set target zero location";
      m_logic.setZeroLoc();
      break;
    default:
      manualMode();
  }
}

void Rover::setColor(ColorData _targetCol)
{
  QString s = QString("hsv %1 %2 %3 %4 %5 %6\n").arg(_targetCol.h)
                                                .arg(_targetCol.hT)
                                                .arg(_targetCol.s)
                                                .arg(_targetCol.sT)
                                                .arg(_targetCol.v)
                                                .arg(_targetCol.vT);
//  qDebug() << s;
  m_cmdFifo.write(s);
}

void Rover::setLocation(LocationData _targetLoc)
{
//  qDebug("Ball x, y, s: %d, %d, %d", _targetLoc.x, _targetLoc.y, _targetLoc.s);
  m_logic.setCurrentLoc(_targetLoc);
}
