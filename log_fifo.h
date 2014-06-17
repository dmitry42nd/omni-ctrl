#pragma once

#include <QObject>

#include <QSharedPointer>
#include <QSocketNotifier>
#include <QString>

#include "common.h"

class LogFifo : public QObject
{
    Q_OBJECT
public:
    explicit LogFifo(const QString fifoPath);
    virtual ~LogFifo();

signals:
  void opened();  
  void closed();

  void locationParsed(LocationData);
  void colorParsed(ColorData);

private slots:
  void readFifo();

private:
  QSharedPointer<QSocketNotifier> m_fifoNotifier;
  QString                   m_fifoPath;
  int                       m_fifoFd;

  QString                   m_rest;

  LocationData m_targetLoc;
  ColorData    m_targetCol;
};
