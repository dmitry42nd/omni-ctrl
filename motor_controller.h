#pragma once

#include <QDebug>
#include <QObject>
#include <QTimer>
#include <QScopedPointer>

#include <cmath>

#include <trikControl/brick.h>
#include "stopwatch.h"

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

signals:
  void correctionDone();

private:
  QString m_port;
  QString m_enc_port;
  const Brick& m_brick;
  QTimer m_timer;
  Stopwatch m_stopwatch;
  

  int m_actualSpeed;
  int m_currentSpeed;
  int m_encOldData;
  int m_encData;
  float m_old_ppms;

  struct timespec m_time, m_oldTime;
  struct timespec m_realTime, m_oldRealTime;
};
