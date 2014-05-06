#include <QDebug>

#include "led_tape.h"

const QString logFifoPath="/tmp/dsp-detector.out.fifo";
const QString cmdFifoPath="/tmp/dsp-detector.in.fifo";

static const int period = 500;

const QString redPort = "JM1"; //0x14
const QString bluPort = "JM2"; //0x15
const QString grePort = "JM3"; //0x17
const QString powPort = "M1";  //0x16

LedTape::LedTape(QThread *guiThread, QString configPath):
  m_logFifo(logFifoPath),
  m_brick(*guiThread, configPath)
{
  m_red =0;
  m_gre =0;
  m_blu =0;

  m_redOut =0;
  m_greOut =0;
  m_bluOut =0;

  m_colorSetCnt = 0;
  qDebug() << "LED_TAPE_STARTS";
  m_logFifo.open();

  connect(m_brick.keys(), SIGNAL(buttonPressed(int,int)),    this, SLOT(onBrickButtonChanged(int,int)));

  m_colorStabilityChecker.setInterval(period);
  pause();
}

LedTape::~LedTape()
{}

int LedTape::convert(float c256)
{
  return 100-(int)((((float)(256-c256))/256.0f)*100.0f);
}

void LedTape::pause()
{
  ledMode = PAUSE;
  qDebug() << "PAUSE";

  m_brick.motor(powPort)->setPower(0);
  m_colorStabilityChecker.stop();
  stop();
}

void LedTape::lighting()
{
  ledMode = LIGHTING;
  qDebug() << "LIGHTING";

  start();

  m_redOut = m_red;
  m_greOut = m_gre;
  m_bluOut = m_blu;

  m_brick.motor(powPort)->setPower(100);
  m_brick.motor(redPort)->setPower(m_redOut);
  m_brick.motor(grePort)->setPower(m_greOut);
  m_brick.motor(bluPort)->setPower(m_bluOut);
}

void LedTape::onBrickButtonChanged(int buttonCode, int state)
{
  if (state == 0) return;

  switch (buttonCode)
  {
    case 105:  
      if(ledMode != LIGHTING)
      {
        lighting();
        break;
      } 
    default:
      pause();
  }
}

void LedTape::setColorData(int red, int gre, int blu, int colorEntry)
{
//  qDebug("line x, angle: %d, %d", x, angle);
  m_colorEntry = colorEntry; //colorEntry is not signal emiting factor  

  int tmpRed = convert(red);
  int tmpGre = convert(gre);
  int tmpBlu = convert(blu);

  if (m_red != tmpRed || m_gre != tmpGre || m_blu != tmpBlu)
  {
    m_red = tmpRed;
    m_gre = tmpGre;
    m_blu = tmpBlu;

    emit colorChanged();
    qDebug() << "color changed";
    qDebug() << "r g b: " << m_red << " " << m_gre << " " << m_blu;
  }
}

void LedTape::start()
{
  connect(&m_logFifo,     SIGNAL(colorDataParsed(int, int, int, int)), this, SLOT(setColorData(int, int, int, int)));
  connect(this, SIGNAL(colorChanged()), &m_colorStabilityChecker, SLOT(start()));
  connect(&m_colorStabilityChecker, SIGNAL(timeout()), this, SLOT(setComingOutColor()));
}

void LedTape::stop()
{
  disconnect(&m_logFifo,     SIGNAL(colorDataParsed(int, int, int, int)), this, SLOT(setColorData(int, int, int, int)));
  disconnect(this, SIGNAL(colorChanged()), &m_colorStabilityChecker, SLOT(start()));
  disconnect(&m_colorStabilityChecker, SIGNAL(timeout()), this, SLOT(setComingOutColor()));
}

#define delta(a,b)  (a != b ? period/abs(a - b) : period)
#define deltaSign(a,b)  (a > b ? -1 : (a < b ? 1 : 0))
void LedTape::setComingOutColor()
{
  m_colorStabilityChecker.stop();
  stop();

  qDebug() << "old color: " << m_redOut << " " << m_greOut << " " << m_bluOut;
  qDebug() << "new color: " << m_red << " " << m_gre << " " << m_blu;

  redT = delta(m_redOut, m_red);
  greT = delta(m_greOut, m_gre);
  bluT = delta(m_bluOut, m_blu);

  qDebug() << "Ts: " << redT << " " << greT << " " << bluT;

  redD = deltaSign(m_redOut, m_red);
  greD = deltaSign(m_greOut, m_gre);
  bluD = deltaSign(m_bluOut, m_blu);

  qDebug() << "Ds: " << redD << " " << greD << " " << bluD;

  connect(&m_cCTimerR, SIGNAL(timeout()), this, SLOT(setColorStepR()));
  connect(&m_cCTimerG, SIGNAL(timeout()), this, SLOT(setColorStepG()));
  connect(&m_cCTimerB, SIGNAL(timeout()), this, SLOT(setColorStepB()));

  connect(this, SIGNAL(outColorChanged()), this, SLOT(outColorChangedCheck()));

  m_cCTimerR.start(redT);
  m_cCTimerG.start(greT);
  m_cCTimerB.start(bluT);
}

void LedTape::setColorStepR()
{
  if(m_redOut == m_red)
  {
    m_cCTimerR.stop();
    disconnect(&m_cCTimerR, SIGNAL(timeout()), this, SLOT(setColorStepR()));

    emit outColorChanged();
  }
  else
  {
    m_redOut += redD;
    m_brick.motor(redPort)->setPower(m_redOut);
  }
}

void LedTape::setColorStepG()
{
  if(m_greOut == m_gre)
  {
    m_cCTimerG.stop();
    disconnect(&m_cCTimerG, SIGNAL(timeout()), this, SLOT(setColorStepG()));

    emit outColorChanged();
  }
  else
  {
    m_greOut += greD;
    m_brick.motor(grePort)->setPower(m_greOut);
  }
}

void LedTape::setColorStepB()
{
  if(m_bluOut == m_blu)
  {
    m_cCTimerB.stop();
    disconnect(&m_cCTimerB, SIGNAL(timeout()), this, SLOT(setColorStepB()));

    emit outColorChanged();
  }
  else
  {
    m_bluOut += bluD;
    m_brick.motor(bluPort)->setPower(m_bluOut);
  }
}

void LedTape::outColorChangedCheck()
{
  if(!m_cCTimerR.isActive() && !m_cCTimerG.isActive() && !m_cCTimerB.isActive())
  {
    qDebug() << "Color changed";
    disconnect(this, SIGNAL(outColorChanged()), this, SLOT(outColorChangedCheck()));
    start();
  }
}



/*
void LedTape::setColorStep()
{
  if (m_colorSetCnt < tpp)
  {
    m_redOut -= redD;
    m_greOut -= greD;
    m_bluOut -= bluD;

    m_brick.motor(redPort)->setPower(convert(m_redOut));
    m_brick.motor(grePort)->setPower(convert(m_greOut));
    m_brick.motor(bluPort)->setPower(convert(m_bluOut));
  
    m_colorSetCnt++;

    qDebug() << "out r g b: " << m_redOut << " " << m_greOut << " " << m_bluOut;
    qDebug() << "out conv r g b: " << convert(m_redOut) << " " << convert(m_greOut) << " " << convert(m_bluOut);
  }
  else
  {
    m_colorSetCnt = 0;
    m_colorChangeTimer.stop();
    disconnect(&m_colorChangeTimer, SIGNAL(timeout()), this, SLOT(setColorStep()));
    start();
  }  
}
*/
