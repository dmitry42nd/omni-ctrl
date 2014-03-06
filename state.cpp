#include <QDebug>

#include "state_finished.h"
#include "rover.h"

void State::massController()
{
  if(m_rover->tgtMass() < 2)
  {
    qDebug() << "TARGET LOST";

    m_rover->stopRover();
    stop();

    emit failed();
  }
}
