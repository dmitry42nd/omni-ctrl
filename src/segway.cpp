#include <QDebug>

#include "segway.h"

const QString logFifoPath="/tmp/dsp-detector.out.fifo";
const QString cmdFifoPath="/tmp/dsp-detector.in.fifo";

const QString armMotor    = "JM1";
const QString handServo   = "JE1";
const QString rangeFinder = "JA6";

Segway::Segway(QThread *guiThread, QString configPath, QString startDirPath):
  m_brick(*guiThread, configPath, startDirPath)
{
  qDebug() << "SEGWAY_STARTS";
  
  dance();
}

void Segway::dance()
{
}
