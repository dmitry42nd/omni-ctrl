#pragma once

#include <QDebug>
#include <QObject>
#include <QTimer>
#include <QScopedPointer>

#include <cmath>

#include <trikControl/brick.h>

using namespace trikControl;

class MotorController : public QObject
{
    Q_OBJECT

public:
//    explicit MotorController(QString configPath);
    explicit MotorController(const Brick& brick, const QString port);
    virtual ~MotorController();

public:
  void setLineTargetData(int x, int angle, int mass);   

public slots:
  void start();
  void stop();
  void doStep();
  void setActualSpeed(int speed);

signals:
  void correctionDone();

private:
  QString m_port;
  const Brick& m_brick;

  //target location data
  int m_tgtX;
  int m_prevTgtX;
  int m_tgtAngle;
  int m_tgtMass;

  int m_actualSpeed;
};
