#include <QDebug>
#include <QRegExp>

#include "log_fifo.h"

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
//#include <assert>

static const int max_fifo_input_size = 32;

LogFifo::LogFifo(const QString _fifoPath):
m_fifoPath(_fifoPath)
{}

LogFifo::~LogFifo()
{
  close();
}

void LogFifo::open()
{
  m_fifoFd = ::open(m_fifoPath.toLocal8Bit().data(), O_SYNC|O_NONBLOCK, O_RDONLY); //O_SYNC ?
  {
  if (m_fifoFd == -1)
    qDebug() << m_fifoPath << ": fifo open failed: " << errno;
    return;
  }
  
  m_fifoNotifier = QSharedPointer<QSocketNotifier>(new QSocketNotifier(m_fifoFd, QSocketNotifier::Read, this));

  connect(m_fifoNotifier.data(), SIGNAL(activated(int)), this, SLOT(readFifo()));
  m_fifoNotifier->setEnabled(true);

  emit opened();
}

void LogFifo::readFifo()
{
  char indato[max_fifo_input_size];
  if (read(m_fifoFd, indato, max_fifo_input_size) < 0)
  {
    qDebug() << m_fifoPath << ": fifo read failed: " << errno;
    return;
  }

  QString s(indato);
  qDebug() << s;
  QStringList logStruct = s.remove(QRegExp("\n.*$")).split(' ');

  if(logStruct[0] == "loc:")
  {
    int x     = logStruct[1].toInt();
    int angle = logStruct[2].toInt();
    int mass  = logStruct[3].toInt();
  
    emit lineTargetDataParsed(x, angle, mass);
  }
  else if (logStruct[0] == "hsv:")
  {
    int hue    = logStruct[1].toInt();
    int hueTol = logStruct[2].toInt();
    int sat    = logStruct[3].toInt();
    int satTol = logStruct[4].toInt();
    int val    = logStruct[5].toInt();
    int valTol = logStruct[6].toInt();

    emit lineColorDataParsed(hue, hueTol, sat, satTol, val, valTol);
  }
}

void LogFifo::close()
{

  disconnect(m_fifoNotifier.data(), SIGNAL(activated(int)), this, SLOT(readFifo()));
  m_fifoNotifier->setEnabled(false);
  if(::close(m_fifoFd) != 0)
  {
    qDebug() << m_fifoPath << ": fifo close failed: " << errno;
    
    return;
  }

  emit closed();
}
