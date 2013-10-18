#include <QDebug>

#include "omnirobot.h"

const float gyroConvConst = 820.846931;

OmniRobot::OmniRobot() :
    omniState(INIT_MODE),
    movementMode(ROTATE_MODE),
    power(20),
    pplus(1)
{
    qDebug() << "INIT_MODE";

    connect(brick.keys(), SIGNAL(buttonPressed(int,int)), this, SLOT(getButton(int,int)));
}

void OmniRobot::getButton(int code, int value)
{
    if ((code == KEY_F3) && (value == 1))
    {
        switch (omniState)
        {
        case INIT_MODE:
            omniState = CONTROL_MODE;
            startControl();
            break;
        case CONTROL_MODE:
            omniState = INIT_MODE;
            brick.stop();
            timer.stop();
            break;
        default:
            brick.stop();
        }
    }
}

void OmniRobot::startControl()
{
    switch (movementMode)
    {
    case ROTATE_MODE:
        connect(timer, SIGNAL(timeout()), this, SLOT(rotate()));
        break;
    }
    timer.start(40);
}

void OmniRobot::rotate()
{
    if (power == 20)
        pplus = 1;
    if (power == 100)
        pplus = -1;
    power = power + pplus;

    brick.powerMotor("1")->setPower(power);
    brick.powerMotor("2")->setPower(power);
    brick.powerMotor("3")->setPower(power);
    brick.powerMotor("4")->setPower(power);
}
