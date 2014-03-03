#pragma once

#include <QObject>

using namespace trikControl;

class StateSearching : public QObject
{
  Q_OBJECT

public:
  explicit StateSearching(const *Rover rover);
  virtual ~StateSearching();

signals:
  void started();
  void finished();

public slots:
  void start();

};
