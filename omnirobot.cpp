#include <QDebug>
//#include <QStringList>

#include "omnirobot.h"

//#include <cmath>


const qreal gyroConvConst = 818.511136;
//818.511136
//938.736454707
//838.736454707
//820.846931
const QString logFifoPath="/tmp/dsp-detector.out.fifo";
const QString cmdFifoPath="/tmp/dsp-detector.in.fifo";

const qreal c1 = sqrt(3) * 0.5;
const qreal c2 = 0.5;

const int speed = 80;
const qreal stopK = 1;
const qreal PK = 0.42;
const qreal IK = 0.006;
const qreal DK = -0.009;

#define rt_SATURATE(sig,ll,ul)     (((sig) >= (ul)) ? (ul) : (((sig) <= (ll)) ? (ll) : (sig)) )
#define sign(x)     x >= 0 ? 1 : -1
#define f_abs(y)    (y >= 0 ? y : -y)

OmniRobot::OmniRobot(QThread *guiThread, QString configPath):
  brick(*guiThread, configPath),
  m_logFifo(logFifoPath),
  m_cmdFifo(cmdFifoPath),
  Mt(),
  cmd(),
  alpha(0.0),
  rotateSpeed(0),
  m_logStruct()
{
  qDebug() << "OMNI_STARTS";

  init();


  //a bit of SciFi to speed up logFifo.readFifio() (A long time ago in a galaxy far, far away QSocketNotifier appeared...)
  guiThread->connect(&m_logFifo, SIGNAL(finished()), guiThread, SLOT(quit()));
  m_logFifo.moveToThread(&m_logFifo);
  m_logFifo.start();

  m_logFifo.openFifo();
  m_cmdFifo.openFifo();
  connect(&m_logFifo, SIGNAL(fifoRead(QString)), this, SLOT(parseLogFifo(QString)));

  connect(brick.gamepad(), SIGNAL(button(int,int)),        this, SLOT(gamepadButton(int, int)));
  connect(brick.gamepad(), SIGNAL(pad(int,int,int)),       this, SLOT(gamepadPad(int,int,int)));
  connect(brick.gamepad(), SIGNAL(padUp(int)),             this, SLOT(gamepadPadUp(int)));
  connect(brick.keys(),    SIGNAL(buttonPressed(int,int)), this, SLOT(getButton(int,int)));
}

OmniRobot::~OmniRobot()
{
  disconnect(&m_logFifo, SIGNAL(fifoRead(QString)), this, SLOT(parseLogFifo(QString)));
}

void OmniRobot::init()
{
  period = 1000;
  xw = 110.0; //mm
  yw = 110.0; //mm
  Dw = 0.02 * 2.0 * 20.0; // 1/50mm

  QVector2D u1(0.0, 1.0);
  QVector2D u2(c1, -c2);
  QVector2D u3(-1.0, 0.0);

  QVector2D n1(-1.0, 0.0);
  QVector2D n2(c2, c1);
  QVector2D n3(c1, -c2);

  QVector2D b1(0.0, yw);
  QVector2D b2(c1*xw, -c2*yw);
  QVector2D b3(-c1*xw, -c2*yw);

  Mt(0,0) = n1.x(); Mt(0,1) = n1.y(); Mt(0,2) = b1.x()*u1.x() + b1.y()*u1.y();
  Mt(1,0) = n2.x(); Mt(1,1) = n2.y(); Mt(1,2) = b2.x()*u2.x() + b2.y()*u3.y();
  Mt(2,0) = n3.x(); Mt(2,1) = n3.y(); Mt(2,2) = b3.x()*u3.x() + b3.y()*u3.y();

  Mt *= -1.0;

  pwm.setX(0.0); pwm.setY(0.0); pwm.setZ(0.0);
  cmd.setX(0.0); cmd.setY(0.0); cmd.setZ(0.0);

  omniState = INIT_MODE;
  movementMode = ROTATE_MODE;
}

void OmniRobot::gamepadPad(int pad, int vx, int vy)
{
  if (pad != 1) return;
  cmd.setX((qreal)vx);
  cmd.setY((qreal)vy);

}

void OmniRobot::gamepadPadUp(int pad)
{
  if (pad != 1) return;
  cmd.setX(0.0);
  cmd.setY(0.0);
  brick.stop();
}

void OmniRobot::gamepadButton(int button, int pressed)
{
  if (pressed == 0) return;

  switch (omniState)
  {
  case INIT_MODE:
    qDebug() << "CONTROL_MODE";
    omniState = CONTROL_MODE;

    switch (button)
    {
      case 1: 
        qDebug() << "ANDROID_MODE";
        movementMode = ANDROID_MODE; 
        period       = 20; 
        break;
      case 2: 
        movementMode = LINE_TRACE_MODE; 
        period       = 30; 
        break;
      case 3: 
        movementMode = ROTATE_MODE; 
        alpha = 0;
        targetAngle = 90;
        rotateSpeed = 0;
        period      = 20; 
        break;
      case 4: 
        movementMode = ROTATE_MAX_MODE; 
        break;
      case 5: 
        movementMode = ROTATE_POINT_MODE; 
        break;
    }
    startControl();

    break;
  case CONTROL_MODE:
    qDebug() << "INIT_MODE";
    omniState = INIT_MODE;

    init();
    brick.stop();
    timer.stop();
    disconnect(&timer, SIGNAL(timeout()), this, SLOT(omniControl()));

    break;
  default:
    brick.stop();
    timer.stop();
    disconnect(&timer, SIGNAL(timeout()), this, SLOT(omniControl()));
  }

}

void OmniRobot::getButton(int code, int value)
{
  if (value != 1)
    return;

  switch (code)
  {
    case 64:  
      m_cmdFifo.writeFifo("detect\n");
      break;
    case 62:  
      if(movementMode != LINE_TRACE_MODE)
      {
        qDebug() << "CONTROL_MODE";
        omniState    = CONTROL_MODE;
        movementMode = LINE_TRACE_MODE; 
        period       = 30; 

        startControl();
        break;
      }
    default:
      qDebug() << "INIT_MODE";
      omniState = INIT_MODE;

      init();
      brick.stop();
      timer.stop();
      disconnect(&timer, SIGNAL(timeout()), this, SLOT(omniControl()));
  }
}

void OmniRobot::startControl()
{
  brick.stop();
  timer.stop();
  connect(&timer, SIGNAL(timeout()), this, SLOT(omniControl()));
  timer.start(period);
}

void OmniRobot::omniControl()
{
  switch (movementMode)
  {
    case ANDROID_MODE:
      androidmode();
      break;
    case LINE_TRACE_MODE:
      lineTracerMode();
      break;
    case ROTATE_MODE:
      rotateAtTargetAngle();
      break;
    case ROTATE_POINT_MODE:
      break;
    case ROTATE_MAX_MODE:
      break;
    default:
      brick.stop();
      break;
  }
}

void OmniRobot::lineTracerMode()
{
/*
  qDebug() << "x    : " << m_tgtX;
  qDebug() << "alpha: " << m_tgtY;
*/
  int speed = 60;
  int P = m_tgtX*PK;
  int I = (m_prevTgtX + m_tgtX)*IK;
  int D = (m_prevTgtX - m_tgtX)*DK;
  int yaw = P + I + D;

  //TODO: smart x, y, z cropping
  pwm = (strafe(yaw) + rotate(m_tgtY) + velocity(speed))*Dw;

  qDebug() << "pwm xyz: " << pwm.x() << " " << pwm.y() << " " << pwm.z();
  brickPower();
}

QVector3D OmniRobot::strafe(qreal _x)
{
  QVector3D strafePwm = QVector3D();
  strafePwm.setX(Mt(0,0)*_x);
  strafePwm.setY(Mt(1,0)*_x);
  strafePwm.setZ(Mt(2,0)*_x);

  //qDebug() << "strafe: " << strafePwm.x() << " " << strafePwm.y() << " " << strafePwm.z();
  return strafePwm;
}

QVector3D OmniRobot::velocity(qreal _y)
{
  QVector3D vPwm = QVector3D();
  vPwm.setX(0);
  vPwm.setY(-_y);
  vPwm.setZ(_y);

  //qDebug() << "velocity: " << vPwm.x() << " " << vPwm.y() << " " << vPwm.z();  
  return vPwm;
}

QVector3D OmniRobot::rotate(qreal _angle)
{
//TODO: gyro usage
  QVector3D rotatePwm = QVector3D();

  rotatePwm.setX(-_angle);
  rotatePwm.setY(-_angle);
  rotatePwm.setZ(-_angle);

  //qDebug() << "rotate: " << rotatePwm.x() << " " << rotatePwm.y() << " " << rotatePwm.z();  
  return rotatePwm;
}


void OmniRobot::brickPower()
{

  int m1 = rt_SATURATE((int)pwm.x(), -100, 100);
  int m2 = rt_SATURATE((int)pwm.y(), -100, 100);
  int m4 = rt_SATURATE((int)pwm.z(), -100, 100);

//  qDebug() << "motors: " << m1 << " " << m2 << " " << m4;
  brick.powerMotor("2")->setPower(m2);
  brick.powerMotor("3")->setPower(m1);
  brick.powerMotor("4")->setPower(m4);

}

void OmniRobot::parseLogFifo(QString _logData)
{
  m_logStruct = _logData.split(" ");

  if(m_logStruct[0] == "loc:")
  {
    //assert(logStruct.length == 4)
    m_prevTgtX = m_tgtX;
    m_tgtX    = m_logStruct[1].toInt();
    m_tgtY    = m_logStruct[2].toInt();
//    m_tgtMass = m_logStruct[3].toInt();
  }
  else if(m_logStruct[0] == "hsv:")
  {
    //assert(logStruct.length == 7)
    m_hue    = m_logStruct[1].toInt();
    m_hueTol = m_logStruct[2].toInt();
    m_sat    = m_logStruct[3].toInt();
    m_satTol = m_logStruct[4].toInt();
    m_val    = m_logStruct[5].toInt();
    m_valTol = m_logStruct[6].toInt();

    QString hsvCmd;
    hsvCmd.sprintf("hsv %d %d %d %d %d %d\n", m_hue, m_hueTol, m_sat, m_satTol, m_val, m_valTol);
    m_cmdFifo.writeFifo(hsvCmd);
    emit hsvCmdParsed(hsvCmd);
  }
}

void OmniRobot::androidmode()
{

  pwm.setX(Mt(0,0)*cmd.x() + Mt(0,1)*cmd.y() + Mt(0,2)*cmd.z());
  pwm.setY(Mt(1,0)*cmd.x() + Mt(1,1)*cmd.y() + Mt(1,2)*cmd.z());
  pwm.setZ(Mt(2,0)*cmd.x() + Mt(2,1)*cmd.y() + Mt(2,2)*cmd.z());

  pwm *= 2.0 * 20.0 * Dw ;

  brickPower();

/*
  float gyronew = (brick.gyroscope()->read()[2])/ gyroConvConst; //to rad
  gyronew = abs(gyronew) > 0.001 ? gyronew : 0;
  alpha += (gyronew)*period*0.001*180/3.14159f;
//  gyroLast = gyronew;

  qDebug() << "gyro:  " << brick.gyroscope()->read()[2]/ gyroConvConst;
  qDebug() << "alpha: " << alpha;
  qDebug() << "_______";
*/
}

void OmniRobot::rotateAtTargetAngle()
{
  float gyronew = (brick.gyroscope()->read()[2])/gyroConvConst; //to rad

  gyronew = f_abs(gyronew) > 0.05 ? gyronew : 0;
  alpha += gyronew*period*0.001*180/3.14159f;
  qDebug() << "alpha: " << alpha;
/*
  int speed = 40;

  float diff = alpha - targetAngle;
  qDebug() << "diff: " << diff;
  if(f_abs(diff) < 1)
  {
    qDebug() << "rotate done";

    qDebug() << "INIT_MODE";
    omniState = INIT_MODE;

    init();
    brick.stop();
    timer.stop();
    disconnect(&timer, SIGNAL(timeout()), this, SLOT(omniControl()));
  }
  else if(alpha < targetAngle)
  {
    pwm.setX(-speed);
    pwm.setY(-speed);
    pwm.setZ(-speed);    
  }
  else if(alpha > targetAngle)
  {
    pwm.setX(speed);
    pwm.setY(speed);
    pwm.setZ(speed);    
  } 
*/
  brickPower();
}
