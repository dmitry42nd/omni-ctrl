#include <QDebug>

#include "omnirobot.h"

#include <cmath>
#include <linux/input.h>

const float gyroConvConst = 820.846931;

OmniRobot::OmniRobot(QThread *guiThread):
    brick(*guiThread)
{
    qDebug() << "INIT_MODE";
    init();
    connect(brick.gamepad(), SIGNAL(trikGamepad_button(int,int)), this, SLOT(gamepadChoice(int, int)));
    connect(brick.keys(), SIGNAL(buttonPressed(int,int)), this, SLOT(getButton(int,int)));
}

void OmniRobot::init()
{
    omniState = INIT_MODE;
    movementMode = ROTATE_MODE;
    power = 20;
    pplus = 1;
}

void OmniRobot::gamepadChoice(int button, int pressed)
{
    if (pressed == 0) return;
    switch (omniState)
    {
    case INIT_MODE:
        switch (button)
        {
        case 1: movementMode = ROTATE_MAX_MODE; break;
        case 2: movementMode = ROTATE_POINT_MODE; break;
        case 3: movementMode = ROTATE_MODE; break;
        default: movementMode = ROTATE_MODE;
        }
        omniState = CONTROL_MODE;
        startControl();
        break;
    case CONTROL_MODE:
        qDebug() << "INIT_MODE";
        init();
        omniState = INIT_MODE;
        brick.stop();
        timer.stop();
        break;
    default:
        brick.stop();
    }

}

void OmniRobot::getButton(int code, int value)
{
    if (value != 1)
        return;

    switch (omniState)
    {
    case INIT_MODE:
        switch (code)
        {
        case KEY_F5: movementMode = ROTATE_MAX_MODE; break;
        case KEY_F7: movementMode = ROTATE_POINT_MODE; break;
        case KEY_F3: movementMode = ROTATE_MODE; break;
        }
        omniState = CONTROL_MODE;
        startControl();
        break;
    case CONTROL_MODE:
        qDebug() << "INIT_MODE";
        init();
        omniState = INIT_MODE;
        brick.stop();
        timer.stop();
        disconnect(&timer, SIGNAL(timeout()), this, SLOT(omniControl()));
        break;
    default:
        brick.stop();
    }
}

void OmniRobot::startControl()
{
    brick.stop();
    timer.stop();
    qDebug() << "CONTROL_MODE";
    connect(&timer, SIGNAL(timeout()), this, SLOT(omniControl()));
    timer.start(100);
}

void OmniRobot::omniControl()
{
    switch (movementMode)
    {
    case ROTATE_POINT_MODE:
        rotatepoint();
        break;
    case ROTATE_MODE:
        rotate();
        break;
    case ROTATE_MAX_MODE:
        rotatemax();
        break;
    default:
        break;
    }
}

void OmniRobot::rotate()
{
    if (power == 20)
        pplus = 2;
    if (power == 100)
        pplus = -2;
    power = power + pplus;

    brick.powerMotor("1")->setPower(power);
    brick.powerMotor("2")->setPower(power);
    brick.powerMotor("3")->setPower(power);
    brick.powerMotor("4")->setPower(power);
}

void OmniRobot::rotatepoint()
{
    brick.powerMotor("2")->setPower(100);
    brick.powerMotor("3")->setPower(100);
    brick.powerMotor("4")->setPower(100);
}

void OmniRobot::rotatemax()
{
    brick.powerMotor("1")->setPower(100);
    brick.powerMotor("2")->setPower(100);
    brick.powerMotor("3")->setPower(100);
    brick.powerMotor("4")->setPower(100);
}
