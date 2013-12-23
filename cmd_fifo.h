#pragma once

#include <QObject>
#include <trikControl/brick.h>


using namespace trikControl;

class cmdFifo : public QObject
{
    Q_OBJECT
public:
    explicit cmdFifo(const QString fifoPath);
    virtual ~cmdFifo();

protected:

signals:
  void opened();  
  void closed();

public slots:
  void openFifo();
  void closeFifo();
  void writeFifo(QString cmd);


private slots:

private:
  QString                   m_fifoPath;
  QFile                     m_fifoFile;
};
