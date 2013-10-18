#include <QtCore/QCoreApplication>
#include "omnirobot.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    OmniRobot robot;

    return a.exec();
}
