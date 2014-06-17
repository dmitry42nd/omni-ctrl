#pragma once

#include <QObject>
#include <QTimer>

#include <cmath>

#include "common.h"
#include "log_fifo.h"
#include "cmd_fifo.h"
#include "rover_engine.h"
#include "rover_manual.h"
#include "rover_logic.h"
#include <trikControl/brick.h>

using namespace trikControl;

class Rover : public QObject
{
  Q_OBJECT

public:
  explicit Rover(QThread *guiThread, QString configPath, QString soundPath, QString speech);
  virtual ~Rover();

public slots:
  void roverMode();
  void manualMode();

private slots:
  void setColor(ColorData _targetCol);
  void setLocation(LocationData _targetLoc);

  void onBrickButtonChanged(int _btn, int _value);
signals:

private:

  enum { 
    MANUAL_MODE,
    ROVER_MODE
  } movementMode;

  LogFifo          m_logFifo;
  CmdFifo          m_cmdFifo;
  Brick            m_brick;
  RoverEngine      m_engine;
  RoverManual      m_manual;
  RoverLogic       m_logic;

  const QString    m_soundPath;
  const QString    m_speech;
};
