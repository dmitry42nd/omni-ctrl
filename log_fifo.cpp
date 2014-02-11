#include <QDebug>
#include <QRegExp>

#include "log_fifo.h"

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
//#include <assert>

static const int max_fifo_input_size = 32;

logFifo::logFifo(const QString _fifoPath):
m_fifoPath(_fifoPath),
m_timer(),
m_myPipe()
{}

logFifo::~logFifo()
{
  closeFifo();
}

void logFifo::openFifo()
{

  m_fifoFd = open(m_fifoPath.toLocal8Bit().data(), O_RDONLY|O_NONBLOCK);
  if (m_fifoFd < 0)
  {
    qDebug() << m_fifoPath << ": fifo open failed: " << errno;

    return;
  }
  
  m_fifoNotifier = new QSocketNotifier(m_fifoFd, QSocketNotifier::Read);
  connect(m_fifoNotifier, SIGNAL(activated(int)), this, SLOT(readFifo(int)));
  m_fifoNotifier->setEnabled(true);

  emit opened();
/*
  connect(&m_myPipe,SIGNAL(connected()),this,SLOT(sloConnected()));
  connect(&m_myPipe,SIGNAL(error(QLocalSocket::LocalSocketError)),this,SLOT(sloConnectionError()));
  m_myPipe.connectToServer(m_fifoPath,QIODevice::ReadOnly);
*/
}

/* 
void logFifo::sloConnected()
{
   connect(&m_timer,SIGNAL(timeout()),this,SLOT(readFifo()));
   m_timer.start(20); //i need to update my clients with the changed/unchanged data at regular intervals
}

void logFifo::sloConnectionError()
{
   qDebug() << "ERROR!!";
}
*/
void logFifo::closeFifo()
{

  disconnect(m_fifoNotifier, SIGNAL(activated(int)), this, SLOT(readFifo(int)));
  m_fifoNotifier->setEnabled(false);
  if(close(m_fifoFd) != 0)
  {
    qDebug() << m_fifoPath << ": fifo close failed: " << errno;
    
    return;
  }

  emit closed();
}

void logFifo::readFifo(int _socket)
{

  if(_socket != 0)
  {
    char indato[max_fifo_input_size];
    if (read(_socket, indato, max_fifo_input_size) < 0)
    {
//      qDebug() << m_fifoPath << ": fifo read failed: " << errno;
      return;
    }

    QString logData(indato);

    emit fifoRead(logData.remove(QRegExp("\n.*$")));
  }

/*
  if(m_myPipe.bytesAvailable())
  {
    QByteArray indato;
    indato=m_myPipe.readAll();

    QString logData(indato);

    emit fifoRead(logData.remove(QRegExp("\n.*$")));
  }
*/
}

