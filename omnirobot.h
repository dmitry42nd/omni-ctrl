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
    explicit OmniRobot(QThread *guiThread);

protected:
    void startControl();
    void rotatepoint();
    void rotatemax();
    void rotate();
    void init();
    
signals:
    
public slots:

private slots:
    void getButton(int code, int value);
    void omniControl();
    void gamepadChoice(int button, int pressed);

private:
    enum { INIT_MODE,
           CONTROL_MODE
    } omniState;
    enum { ROTATE_MODE,
           ROTATE_MAX_MODE,
           ROTATE_POINT_MODE,
           ANDROID_MODE
    } movementMode;

    Brick   brick;
    QTimer  timer;
    int power;
    int pplus;
};

#endif // OMNIROBOT_H
