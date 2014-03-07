#pragma once

#include <QObject>
#include <QTimer>
#include <QVector2D>
#include <QStringList>

#include <cmath>

#include "log_fifo.h"

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
  
private slots:
  void setColorData(int red, int gre, int blu, int colorEntry);
  void onBrickButtonChanged(int buttonCode, int value);

  void start();
  void stop();
  void colorStabilityCheck();
  void setComingOutColor();
  void setColorStep();

private:

    enum { PAUSE,
           LIGHTING
    } ledMode;

    LogFifo m_logFifo;
    Brick   m_brick;
    QTimer  m_colorStabilityChecker;
    QTimer  m_colorChangeTimer;

    //color data
    int m_red;
    int m_gre;
    int m_blu;

    int m_redOut;
    int m_greOut;
    int m_bluOut;
/*
    int m_hueP;
    int m_satP;
    int m_valP;
*/
    int m_colorEntry;
};

