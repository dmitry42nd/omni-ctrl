#pragma once

#include <QObject>

#include <QtCore/QSharedPointer>

#include <trikControl/brick.h>

using namespace trikControl;

class LogFifo : public QObject
{
    Q_OBJECT
public:
    explicit LogFifo(const QString fifoPath);
    virtual ~LogFifo();

signals:
  void opened();  
  void closed();
  void lineTargetDataParsed(int, int, int);
  void lineColorDataParsed(int, int, int, int, int, int);

public slots:
  void open();
  void close();

private slots:
  void readFifo();

private:
  QSharedPointer<QSocketNotifier> m_fifoNotifier;
  QString                   m_fifoPath;
  int                       m_fifoFd;
  QString                   m_rest;
};
