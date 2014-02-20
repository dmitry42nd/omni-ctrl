#include <QDebug>
#include <QRegExp>
#include <QStringList>
#include "log_fifo.h"


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

  bool wasLoc = false;
  bool wasHsv = false;

  for(int i = lines.size() - 2; !(wasHsv && wasLoc) && i >= 0; i--)
  {
    QStringList logStruct = lines[i].split(" ", QString::SkipEmptyParts);
    if(!wasLoc && logStruct[0] == "loc:")
    {
      int x     = logStruct[1].toInt();
      int angle = logStruct[2].toInt();
      int mass  = logStruct[3].toInt();
    
      wasLoc = true;
      emit lineTargetDataParsed(x, angle, mass);
    }
    else if (!wasHsv && logStruct[0] == "hsv:")
    {
      int hue    = logStruct[1].toInt();
      int hueTol = logStruct[2].toInt();
      int sat    = logStruct[3].toInt();
      int satTol = logStruct[4].toInt();
      int val    = logStruct[5].toInt();
      int valTol = logStruct[6].toInt();

      wasHsv = true;
      emit lineColorDataParsed(hue, hueTol, sat, satTol, val, valTol);
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
