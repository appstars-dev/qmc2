#ifndef TWEAKEDQMLAPPVIEWER_H
#define TWEAKEDQMLAPPVIEWER_H

#include <QTimer>
#include <QStringList>
#include <QMap>

#if QT_VERSION < 0x050000
#include "qmlapplicationviewer.h"
#else
#include <QQuickView>
#include <QWindow>
#include <QByteArray>
#endif

#include "processmanager.h"
#include "imageprovider.h"
#include "infoprovider.h"
#include "keysequencemap.h"
#if defined(QMC2_ARCADE_ENABLE_JOYSTICK)
#include "joyfunctionmap.h"
#endif

#define QMC2_ARCADE_PARAM_THEME     0
#if QT_VERSION < 0x050000
#define QMC2_ARCADE_PARAM_GRASYS    1
#define QMC2_ARCADE_PARAM_CONSOLE   2
#define QMC2_ARCADE_PARAM_LANGUAGE  3
#else
#define QMC2_ARCADE_PARAM_CONSOLE   1
#define QMC2_ARCADE_PARAM_LANGUAGE  2
#endif

#if QT_VERSION < 0x050000
class TweakedQmlApplicationViewer : public QmlApplicationViewer
#else
class TweakedQmlApplicationViewer : public QQuickView
#endif
{
    Q_OBJECT

public:
    int numFrames;
    QTimer frameCheckTimer;
    QByteArray savedGeometry;
    bool savedMaximized;
    QList<QObject *> gameList;
    ProcessManager *processManager;
    ImageProvider *imageProvider;
    InfoProvider *infoProvider;
    bool windowModeSwitching;
    QMap<QString, QStringList> cliAllowedParameterValues;
    QMap<QString, QString> cliParameterDescriptions;
    QStringList cliParams;
    QStringList infoClasses;
    KeySequenceMap *keySequenceMap;
#if defined(QMC2_ARCADE_ENABLE_JOYSTICK)
    JoyFunctionMap *joyFunctionMap;
#endif

#if QT_VERSION < 0x050000
    explicit TweakedQmlApplicationViewer(QWidget *parent = 0);
#else
    explicit TweakedQmlApplicationViewer(QWindow *parent = 0);
#endif
    virtual ~TweakedQmlApplicationViewer();

#if QT_VERSION >= 0x050000
    bool isFullScreen()
    {
        return windowState() & Qt::WindowFullScreen;
    }
    bool isMaximized()
    {
        return windowState() & Qt::WindowMaximized;
    }
    QByteArray saveGeometry()
    {
        // FIXME
        return QByteArray();
    }
    void restoreGeometry(const QByteArray &geom)
    {
        // FIXME
    }
#endif

    int themeIndex();

signals:
    void emulatorStarted(int);
    void emulatorFinished(int);

public slots:
    void fpsReady();
    void loadSettings();
    void saveSettings();
    void goFullScreen();
    void switchToFullScreen(bool initially = false);
    void switchToWindowed(bool initially = false);
    QString romStateText(int);
    int romStateCharToInt(char);
    void loadGamelist();
    void launchEmulator(QString);
    QString loadImage(const QString &);
    QString requestInfo(const QString&, const QString&);
    int findIndex(QString, int startIndex = 0);
    void log(QString);
    QStringList cliParamNames();
    QString cliParamDescription(QString);
    QString cliParamValue(QString);
    QStringList cliParamAllowedValues(QString);
    void setCliParamValue(QString, QString);
    void linkActivated(QString);
    QString emuMode();
#if QT_VERSION >= 0x050000
    void frameBufferSwapped();
    void handleQuit();
#endif
    int runningEmulators() { return processManager->runningProcesses(); }

private:
    bool initialised;

#if QT_VERSION < 0x050000
protected:
    void paintEvent(QPaintEvent *);
    void closeEvent(QCloseEvent *);
#endif
};

#endif
