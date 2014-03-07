#include <QDebug>
#include <QRegExp>
#include <QStringList>
#include "log_fifo.h"

#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
//#include <assert>

static const int max_fifo_input_size = 4000;

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
  if (m_fifoFd == -1)
  {
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
	m_fifoNotifier->setEnabled(false);

  static char indata[max_fifo_input_size];

  int size;
  if ((size = read(m_fifoFd, indata, max_fifo_input_size)) < 0)
  {
    qDebug() << m_fifoPath << ": fifo read failed: " << errno;
    return;
  }
  m_rest.append(QByteArray(indata, size));

  QStringList lines = m_rest.split('\n');

  m_rest = lines.last();

  bool wasColor = false;

  for(int i = lines.size() - 2; !(wasColor) && i >= 0; i--)
  {
    QStringList logStruct = lines[i].split(" ", QString::SkipEmptyParts);
    if(!wasColor && logStruct[0] == "color:")
    {
      int color      = logStruct[1].toInt();
      int colorEntry = logStruct[2].toInt();
    
      wasColor = true;

      int red = (uint8_t)(color >> 16);
      int gre = (uint8_t)(color >> 8);
      int blu = (uint8_t)color;
      emit colorDataParsed(red, gre, blu, colorEntry);
    }
  }

	m_fifoNotifier->setEnabled(true);
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
