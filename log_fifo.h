#pragma once

#include <QObject>
#include <QPointer>
#include <QTimer>
#include <QSocketNotifier>
#include <trikControl/brick.h>


using namespace trikControl;

class logFifo : public QObject
{
    Q_OBJECT
public:
    explicit logFifo(const QString fifoPath);
    virtual ~logFifo();

protected:

signals:
  void opened();  
  void closed();
  void fifoRead(QString line);

public slots:
  void openFifo();
  void closeFifo();
  void readFifo();
//  void writeFifo(QString cmd);


private slots:

private:
  QPointer<QSocketNotifier> m_fifoNotifier;
  QString                   m_fifoPath;
  int                       m_fifoFd;

};
