#pragma once
#include <trikControl/brickInterface.h>
#include <trikNetwork/gamepadInterface.h>
#include <QtGui/QApplication>
#include <QTimer>
#include <QElapsedTimer>

using namespace trikControl;

class Segway : public QObject
{
  Q_OBJECT

public:
//  explicit Segway(QThread *guiThread, QString configPath, QString startDirPath);
  Segway(QApplication *app, 
               QString systemConfigPath, QString modelConfigPath, QString startDirPath,
               double pk, double dk, double ik, double ck, double ofs,
               int accGAxis, int accOAxis, int gyroAxis);
  virtual ~Segway();

private slots:
  void disconnectAll();
  void onBtnPressed(int code, int state);
  /*
  void onGamepadPadDown(int pd ,int x, int y);
  void onGamepadPadUp(int pd);
  void onGamepadBtnChanged(int code, int state);
  */
  
  void startDriftAccumulation();
  void stopDriftAccumulation();
  void accumulateDrift();

  void startDancing();
  void dance();

  void startUpdatingBC();  
  void updateBC();
  
//signals:

private:
  QApplication* m_app;
  BrickInterface* m_brick;
//  GamepadInterface* m_gamepad;
  QTimer m_mainTicker;
  QTimer m_gdcTicker; //Gyro (zero) drift controller
  QTimer m_bcTicker;
  QElapsedTimer m_dbgTicker;
  
  double m_bc; //battery coeff
  
  double m_outData;
  double m_outDataOld;

  double m_fbControl;
  double m_rlControl;
  
  enum { PID_CONTROL1, PID_CONTROL2, MOVEMENT_CONTROL } m_state;  
  double m_pk;
  double m_dk;
  double m_ik;
  double m_ck;

  double m_offset;

  int m_accGAxis;
  int m_accOAxis;
  int m_gyroAxis;
  
  int m_cnt;
  int m_gyroDrift;
  int m_gyroDriftCnt;
  double m_rowGyroData;
};
