#pragma once
#include <trikControl/brick.h>
#include <QTimer>
#include <QElapsedTimer>

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
  
  void startDriftAccumulation();
  void stopDriftAccumulation();
  void accumulateDrift();

  void startDancing();
  void dance();

  void startUpdatingBC();  
  void updateBC();
  
//signals:

private:
  Brick  m_brick;
  QTimer m_mainTicker;
  QTimer m_gdcTicker; //Gyro (zero) drift controller
  QTimer m_bcTicker;
  QElapsedTimer m_dbgTicker;
  
  double m_bc; //battery coeff
  
  double m_acceData;
  double m_gyroData;
  double m_outData;
  double m_outDataOld;

  double m_fbControl;
  double m_rlControl;
  
  enum {PID_CONTROL1, PID_CONTROL2, MOVEMENT_CONTROL} m_state;  
  double m_pk;
  double m_dk;
  double m_ik;

  double m_offset;
  
  int m_cnt;
  int m_gyroGain;
  int m_gyroDrift;
  int m_gyroDriftCnt;
};
