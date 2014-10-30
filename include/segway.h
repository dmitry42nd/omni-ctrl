#pragma once
#include <trikControl/brick.h>
#include <QTimer>

using namespace trikControl;

class Segway : public QObject
{
  Q_OBJECT

public:
  explicit Segway(QThread *guiThread, QString configPath, QString startDirPath);
  virtual ~Segway();

private slots:
  void onBtnPressed(int code, int state);

  void startDriftAcc();
  void stopDriftAcc();
  void startDancing();
  
  void accDrift();
  void dance();
  
//signals:

private:
  Brick  m_brick;
  QTimer m_mainTicker;
  QTimer m_gdcTicker; //Gyro (zero) drift controller
  
  double m_acceData;
  double m_gyroData;
  double m_outData;
  double m_outDataOld;

  double m_offset;  
  int m_gyroGain;
  int m_gyroDrift;
  int m_gyroDriftCnt;
};
