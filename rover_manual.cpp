#include <QDebug>

#include "rover_manual.h"

RoverManual::RoverManual()
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
      emit moveChasis(_y+_x, _y-_x);
      break;
    case 2:
      emit moveArm(_y);
      emit moveHand(_x);
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
      emit moveChasis(0, 0);
      break;
    case 2:
      emit moveArm(0);
      emit moveHand(0);
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
