#include <QDebug>
#include <QRegExp>

#include <unistd.h>
#include <fcntl.h>

#include <errno.h>
#include <linux/input.h>

#include "cmd_fifo.h"
//#include <assert>

CmdFifo::CmdFifo(const QString _fifoPath):
m_fifoFile(_fifoPath)
{
  if(!(m_fifoFile.open(QIODevice::WriteOnly|QIODevice::Truncate|QIODevice::Unbuffered|QIODevice::Text)))
  {
    qDebug() << m_fifoFile.fileName() << ": fifo open failed";
  }

  emit opened();
}

CmdFifo::~CmdFifo()
{
  m_fifoFile.close();

  emit closed();
}

void CmdFifo::write(QString _cmd)
{
    m_fifoFile.write(_cmd.toLocal8Bit().data());
    m_fifoFile.flush();
}
