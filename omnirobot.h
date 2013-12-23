#pragma once


#include <QObject>
#include <QTimer>
#include <QMatrix3x3>
#include <QVector2D>
#include <QVector3D>
#include <QStringList>

#include <cmath>

#include <trikControl/brick.h>
#include "log_fifo.h"
#include "cmd_fifo.h"

using namespace trikControl;

class OmniRobot : public QObject
{
    Q_OBJECT
public:
    explicit OmniRobot(QThread *guiThread, QString configPath);
    virtual ~OmniRobot();

protected:
    void init();
    void brickPower();
    void startControl();
    void androidmode();
    void lineTracerMode();
    QVector3D strafe(qreal x);
    QVector3D rotate(qreal angle/*, qreal speed*/);
    
signals:
  void hsvCmdParsed();
public slots:

private slots:
  void getButton(int code, int value);
  void gamepadButton(int button, int pressed);
  void omniControl();
  void gamepadPad(int pad, int vx, int vy);
  void gamepadPadUp(int pad);
  void parseLogFifo(QString logData);
  void accumGyroError();
  
private:
    int period;
    enum { INIT_MODE,
           CONTROL_MODE
    } omniState;

    enum { ROTATE_MODE,
           ROTATE_MAX_MODE,
           ROTATE_POINT_MODE,
           LINE_TRACE_MODE,
           ANDROID_MODE
    } movementMode;

    Brick   brick;
    QTimer  timer;
    logFifo m_logFifo;
    cmdFifo m_cmdFifo;

    qreal Dw;
    qreal xw;
    qreal yw;

  
    int64_t gyroCntr;//tmp

    qreal gyrolast;
    int gyroError;
    qreal alpha;

    QMatrix3x3 Mt;
    QVector3D cmd;
    QVector3D pwm;

    //target location data
    int m_tgtX;
    int m_prevTgtX;
    int m_tgtY;
    int m_tgtMass;

    //target HSV data
    int m_hue;
    int m_hueTol;
    int m_sat;
    int m_satTol;
    int m_val;
    int m_valTol;

    QStringList m_logStruct;
    //matrix<float> Rot;
    //vector<float> cmd;
    //vector<float> pwm;
    //vector<float> mov;
};
