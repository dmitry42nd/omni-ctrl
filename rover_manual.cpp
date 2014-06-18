#include <QDebug>

#include "rover_manual.h"

RoverManual::RoverManual(const RoverEngine& _engine):
m_engine(_engine)
{}

RoverManual::~RoverManual()
{}

void RoverManual::onButtonChanged(int _btn, int _value)
{
  if (_value == 0) return; //in case of
  
  qDebug() << "Button: " << _btn;
}

void RoverManual::onPadDown(int _pad, int _x, int _y)
{
  switch (_pad)
  {
    case 1:
      m_engine.moveChasis(_y+_x, _y-_x);
      break;
    case 2:
      m_engine.moveArm(_y);
      m_engine.moveHand(_x);
      break;
    default:
      qDebug() << "More than two pads is not provided";
      break;
  }
}

void RoverManual::onPadUp(int _pad)
{
  switch (_pad)
  {
    case 1:
      m_engine.moveChasis(0, 0);
      break;
    case 2:
      m_engine.moveArm(0);
      m_engine.moveHand(0);
      break;
    default:
      qDebug() << "More than two pads is not provided";
      break;
  }
}

void RoverManual::onWheel (int _angle)
{
  qDebug() << "Wheel: " << _angle;
}
