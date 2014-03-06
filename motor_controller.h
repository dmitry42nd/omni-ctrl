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
    explicit MotorController(const Brick& brick, const QString port, const QString enc_port);
    virtual ~MotorController();

public slots:
  void startAutoControl();
  void stopAutoControl();
  void doStep();
  void setActualSpeed(int speed);

  void startSpeedometer();
  void stopSpeedometer();
  int countSpeed();

signals:
  void correctionDone();
  void speedometerDone();

private:
  QString m_port;
  QString m_enc_port;
  const Brick& m_brick;
//  QTimer m_timer;
/*
  Stopwatch m_stopwatch;
*/

  int m_actualSpeed;
  float m_currentSpeed;

  int m_encPoints;
  struct timespec m_time, m_oldTime;

/*
  int m_encOldData;
  float m_old_ppms;

  struct timespec m_time, m_oldTime;
  struct timespec m_realTime, m_oldRealTime;
*/
  static int sign(int _v)
  {
    return (_v < 0) ? -1 : ((_v > 0) ? 1 : 0);
  }

};
