#pragma once
#include <trikControl/brick.h>
#include <QTimer>

using namespace trikControl;

class Segway : public QObject
{
  Q_OBJECT

public:
  explicit Segway(QThread *guiThread, QString configPath, QString startDirPath);
  virtual ~Segway() {};

//slots:

//signals:

private:
  Brick  m_brick;
  QTimer m_ticker;

  void dance();
};
