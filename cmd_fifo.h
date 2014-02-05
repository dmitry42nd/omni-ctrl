#pragma once

#include <QObject>
#include <trikControl/brick.h>


using namespace trikControl;

class CmdFifo : public QObject
{
    Q_OBJECT
public:
    explicit CmdFifo(const QString fifoPath);
    virtual ~CmdFifo();

protected:

signals:
  void opened();  
  void closed();

public slots:
  void open();
  void close();
  void write(QString cmd);


private slots:

private:
  QString                   m_fifoPath;
  QFile                     m_fifoFile;
};
