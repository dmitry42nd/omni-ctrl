#pragma once

#include <QObject>
#include <QString>
#include <QFile>

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
  void write(QString cmd);


private slots:

private:
  QString                   m_fifoPath;
  QFile                     m_fifoFile;
};
