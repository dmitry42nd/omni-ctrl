#include <QDebug>

#include "state_searching.h"

const int speed = 100;

StateSearching::StateSearching(const &Rover rover):
{
/*
  connect(&m_logFifo, SIGNAL(ballColorDataParsed(int, int, int, int, int, int)),  
          this, SLOT(setBallColorData(int, int, int, int, int, int)));
  connect(m_brick.gamepad(), SIGNAL(button(int,int)),        this, SLOT(onGamepadButtonChanged(int, int)));
  connect(m_brick.keys(),    SIGNAL(buttonPressed(int,int)), this, SLOT(onBrickButtonChanged(int,int)));
*/
}

StateSearching::~StateSearching()
{

}

StateSearching::start()
{
  qDebug() << "START_ROTATION";
}
