#pragma once

#include <QObject>

#include <cmath>
#include <trikControl/brick.h>

using namespace trikControl;

class RoverEngine : public QObject
{
  Q_OBJECT

public:
  explicit RoverEngine(const Brick& _brick, QString _lPort, QString _rPort, QString _armPort, QString _handPort);
  virtual ~RoverEngine();

public slots:
  void moveChasis(int _lS, int _rS);
  void moveArm(int _s);
  void moveHand(int _s); //catch, release?

  void stop();
private:
  const Brick& m_brick;

  QString m_lPort;
  QString m_rPort;
  QString m_armPort;
  QString m_handPort;
};
