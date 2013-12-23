#include <QDebug>
#include <QRegExp>

#include "log_fifo.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <errno.h>
#include <termios.h>
#include <netdb.h>
#include <linux/input.h>

//#include <assert>

static const int max_fifo_input_size = 32;

logFifo::logFifo(const QString _fifoPath):
m_fifoPath(_fifoPath)
{}

logFifo::~logFifo()
{
  closeFifo();
}

void logFifo::openFifo()
{
  m_fifoFd = open(m_fifoPath.toLocal8Bit().data(), /*O_SYNC,*/ O_RDONLY|O_NONBLOCK);
  if (m_fifoFd < 0)
  {
    qDebug() << m_fifoPath << ": fifo open failed: " << errno;

    return;
  }
  
  m_fifoNotifier = new QSocketNotifier(m_fifoFd, QSocketNotifier::Read, this);
  connect(m_fifoNotifier, SIGNAL(activated(int)), this, SLOT(readFifo()));
  m_fifoNotifier->setEnabled(true);

  emit opened();
}

void logFifo::closeFifo()
{

  disconnect(m_fifoNotifier, SIGNAL(activated(int)), this, SLOT(readFifo()));
  m_fifoNotifier->setEnabled(false);
  if(close(m_fifoFd) != 0)
  {
    qDebug() << m_fifoPath << ": fifo close failed: " << errno;
    
    return;
  }

  emit closed();
}

void logFifo::readFifo()
{
  char indato[max_fifo_input_size];
  if (read(m_fifoFd, indato, max_fifo_input_size) < 0)
  {
    qDebug() << m_fifoPath << ": fifo read failed: " << errno;
    return;
  }

  QString logData(indato);

  emit fifoRead(logData.remove(QRegExp("\n.*$")));
}

