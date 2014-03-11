#pragma once

#include <QObject>

#include <QSharedPointer>
#include <QSocketNotifier>
#include <QString>

class LogFifo : public QObject
{
    Q_OBJECT
public:
    explicit LogFifo(const QString fifoPath);
    virtual ~LogFifo();

signals:
  void opened();  
  void closed();
  void ballTargetDataParsed(int, int, int);
  void ballColorDataParsed(int, int, int, int, int, int);

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
