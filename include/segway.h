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
  void onGamepadPadDown(int pd ,int x, int y);
  void onGamepadPadUp(int pd);
  void onGamepadBtnChanged(int code, int state);
  
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

/*
  1 - pk, dk
  2 - pk, ik
  3 - rowrow
*/
  double m_rowrow;
  double m_wewwew;
  int m_state;  
  
  double m_pk;
  double m_dk;
  double m_ik;

  double m_offset;  
  int m_gyroGain;
  int m_gyroDrift;
  int m_gyroDriftCnt;
};
