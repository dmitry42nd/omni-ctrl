#include <QtGui/QApplication>
#include "omnirobot.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    OmniRobot robot(a.thread());

    return a.exec();
}
