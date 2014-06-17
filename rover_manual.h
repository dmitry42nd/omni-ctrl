#pragma once

#include <QObject>

#include <cmath>
#include <trikControl/brick.h>

using namespace trikControl;

class RoverManual : public QObject
{
  Q_OBJECT

public:
  explicit RoverManual();
  virtual ~RoverManual();

public slots:
  void onButtonChanged(int _btn, int _value);
  void onPadDown(int _pad, int _x, int _y);
  void onPadUp(int _pad);
  void onWheel(int _angle);

signals:
  void moveChasis(int _lS, int _rS);
  void moveArm(int _s);
  void moveHand(int _s);
};
