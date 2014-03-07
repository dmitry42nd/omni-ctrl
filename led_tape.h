#pragma once

#include <QObject>
#include <QTimer>
#include <QVector2D>
#include <QStringList>

#include <cmath>

#include "log_fifo.h"
#include <trikControl/brick.h>

using namespace trikControl;

class LedTape : public QObject
{
    Q_OBJECT
public:
    explicit LedTape(QThread *guiThread, QString configPath);
    virtual ~LedTape();

protected:
    void pause();
    void lighting();

signals:
  void colorChanged();
  void outColorChanged();
  
private slots:
  void setColorData(int red, int gre, int blu, int colorEntry);
  void onBrickButtonChanged(int buttonCode, int value);

  void start();
  void stop();

  void setComingOutColor();

  void outColorChangedCheck();

//  void setColorStep(int color, QString colorPort);

  void setColorStepR();
  void setColorStepG();
  void setColorStepB();

private:
    static int convert(float c256);

    enum { PAUSE,
           LIGHTING
    } ledMode;

    LogFifo m_logFifo;
    Brick   m_brick;
    QTimer  m_colorStabilityChecker;
    QTimer  m_colorChangeTimer;

    QTimer  m_cCTimerR;
    QTimer  m_cCTimerG;
    QTimer  m_cCTimerB;

    int m_colorSetCnt;
    //color data
    int m_red;
    int m_gre;
    int m_blu;

    int m_colorEntry;

    int m_redOut;
    int m_greOut;
    int m_bluOut;

    int redD;
    int greD;
    int bluD;

    int redT;
    int greT;
    int bluT;

};

