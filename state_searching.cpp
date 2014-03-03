#include <QDebug>

#include "state_searching.h"

const int speed = 100;

StateSearching::StateSearching():
  m_logFifo(logFifoPath),
  m_cmdFifo(cmdFifoPath),
  m_brick(*guiThread, configPath),
  m_motorControllerL(m_brick, "JM1", "JB4"),
  m_motorControllerR(m_brick, "JM3", "JB3"),
  m_motorsWorkerThread()
{
  qDebug() << "START_ROTATION";
  connect(&m_logFifo, SIGNAL(ballColorDataParsed(int, int, int, int, int, int)),  
          this, SLOT(setBallColorData(int, int, int, int, int, int)));
  connect(m_brick.gamepad(), SIGNAL(button(int,int)),        this, SLOT(onGamepadButtonChanged(int, int)));
  connect(m_brick.keys(),    SIGNAL(buttonPressed(int,int)), this, SLOT(onBrickButtonChanged(int,int)));
}

StateSearching::~StateSearching()
{

}

StateSearching::start()
{

}
