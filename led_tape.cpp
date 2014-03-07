#include <QDebug>

#include "led_tape.h"

const QString logFifoPath="/tmp/dsp-detector.out.fifo";
const QString cmdFifoPath="/tmp/dsp-detector.in.fifo";

const QString redPort = "JM1"; //0x14
const QString grePort = "JM3"; //0x17
const QString bluPort = "JM2"; //0x15
const QString powPort = "M1";  //0x16

LedTape::LedTape(QThread *guiThread, QString configPath):
  m_logFifo(logFifoPath),
  m_brick(*guiThread, configPath)
{
  qDebug() << "LED_TAPE_STARTS";
  m_logFifo.open();

  connect(&m_logFifo,     SIGNAL(colorDataParsed(int, int, int, int)), this, SLOT(setColorData(int, int, int, int)));
  connect(m_brick.keys(), SIGNAL(buttonPressed(int,int)),    this, SLOT(onBrickButtonChanged(int,int)));

  pause();
}

LedTape::~LedTape()
{}

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

  m_brick.motor(powPort)->setPower(0);
  start();
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

  if (m_red != red || m_gre != gre || m_blu != blu)
  {
    m_red = red;
    m_gre = gre;
    m_blu = blu;

    emit colorChanged();
  }
}

void LedTape::start()
{
  connect(this, SIGNAL(colorChanged()), this, SLOT(colorStabilityCheck()));
  connect(&m_colorStabilityChecker, SIGNAL(timeout()), this, SLOT(setComingOutColor()));
}

void LedTape::stop()
{
  disconnect(this, SIGNAL(colorChanged()), this, SLOT(colorStabilityCheck()));
  disconnect(&m_colorStabilityChecker, SIGNAL(timeout()), this, SLOT(setComingOutColor()));
}

void LedTape::colorStabilityCheck()
{
  m_colorStabilityChecker.stop();
  m_colorStabilityChecker.start(1000);
}

void LedTape::setComingOutColor()
{
  m_colorStabilityChecker.stop();
  stop();

  connect(&m_colorChangeTimer, SIGNAL(timeout()), this, SLOT(setColorStep()));
  m_colorChangeTimer.start(100);
}
/*
#define rt_SATURATE(sig,ll,ul)     (((sig) >= (ul)) ? (ul) : (((sig) <= (ll)) ? (ll) : (sig)) )
#define sign(x)     x >= 0 ? 1 : -1
#define f_abs(y)    (y >= 0 ? y : -y)
*/
#define delta(a,b)  (a > b ? -1 : (a < b ? 1 : 0))
#define convert(a) ((int)((float)(256-a)/256.0f)*100)
void LedTape::setColorStep()
{
  int redD = delta(m_redOut, m_red);
  int greD = delta(m_greOut, m_gre);
  int bluD = delta(m_bluOut, m_blu);

  m_redOut += redD;
  m_greOut += greD;
  m_bluOut += bluD;

  m_brick.motor(redPort)->setPower(convert(m_redOut));
  m_brick.motor(grePort)->setPower(convert(m_greOut));
  m_brick.motor(bluPort)->setPower(convert(m_bluOut));

  if (redD == 0 && greD == 0 && bluD == 0)
  {
    m_colorChangeTimer.stop();
    disconnect(&m_colorChangeTimer, SIGNAL(timeout()), this, SLOT(setColorStep()));
    start();
  }  
}
