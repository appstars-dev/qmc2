#if defined(QMC2_ARCADE_ENABLE_JOYSTICK)

#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <QObject>
#include <QMap>
#include <QTime>
#include <QTimer>
#include <QStringList>
#if defined(QMC2_ARCADE_OS_WIN)
#if defined(QMC2_ARCADE_MINGW)
#include <SDL/SDL.h>
#else
#include <SDL.h>
#endif
#else
#include <SDL/SDL.h>
#endif

#define QMC2_ARCADE_SDL_JOYSTICK_DEFAULT_EVENT_TIMEOUT		25
#define QMC2_ARCADE_SDL_JOYSTICK_DEFAULT_AUTOREPEAT_DELAY	250

class Joystick : public QObject
{
    Q_OBJECT

public:
    QStringList joystickNames;
    SDL_Joystick *joystick;
    int numAxes;
    int numButtons;
    int numHats;
    int numTrackballs;
    int eventTimeout;
    int autoRepeatDelay;
    bool autoRepeat;
    QTimer joystickTimer;
    QMap<int, int> deadzones;
    QMap<int, int> sensitivities;

    Joystick(QObject *parent = 0,
             int joystickEventTimeout = QMC2_ARCADE_SDL_JOYSTICK_DEFAULT_EVENT_TIMEOUT,
             bool doAutoRepeat = true,
             int autoRepeatDelay = QMC2_ARCADE_SDL_JOYSTICK_DEFAULT_AUTOREPEAT_DELAY);
    ~Joystick();
    bool open(int);
    void close();
    bool isOpen() { return joystick != NULL; }
    int getAxisValue(int);

private:
    QMap<int, Sint16> axes;
    QMap<int, Uint8> buttons;
    QMap<int, Uint8> hats;
    QMap<int, QTime> axisRepeatTimers;
    QMap<int, QTime> buttonRepeatTimers;
    QMap<int, QTime> hatRepeatTimers;

signals:
    void axisValueChanged(int axis, int value);
    void buttonValueChanged(int button, bool value);
    void hatValueChanged(int hat, int value);
    void trackballValueChanged(int trackball, int deltaX, int deltaY);

public slots:
    void processEvents();
};

#endif

#endif
