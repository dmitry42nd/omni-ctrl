#ifndef OMNIROBOT_H
#define OMNIROBOT_H

#include <QObject>
#include <QTimer>
#include <trikControl/brick.h>

using namespace trikControl;

class OmniRobot : public QObject
{
    Q_OBJECT
public:
    explicit OmniRobot();
    
signals:
    
public slots:

private slots:
    void getButton(int code, int value);
    void rotate();

protected:
    void startControl();

private:
    enum { INIT_MODE,
           CONTROL_MODE
    } omniState;
    enum { ROTATE_MODE,
           ANDROID_MODE
    } movementMode;
    Brick   brick;
    QTimer  timer;
    int power;
    int pplus;
};

#endif // OMNIROBOT_H
