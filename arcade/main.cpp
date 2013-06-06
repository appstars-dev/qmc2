#include <QApplication>
#include <QTranslator>
#include <QIcon>

#include "arcadesettings.h"
#include "tweakedqmlappviewer.h"
#include "consolewindow.h"
#include "macros.h"
#include "joystick.h"

ArcadeSettings *globalConfig = NULL;
ConsoleWindow *consoleWindow = NULL;
int emulatorMode = QMC2_ARCADE_EMUMODE_MAME;
int consoleMode = QMC2_ARCADE_CONSOLE_TERM;
QStringList emulatorModes;
QStringList arcadeThemes;
QStringList mameThemes;
QStringList messThemes;
QStringList umeThemes;
QStringList consoleModes;
QStringList graphicsSystems;
QStringList argumentList;
bool runApp = true;

#if QT_VERSION < 0x050000
void qtMessageHandler(QtMsgType type, const char *msg)
#else
void qtMessageHandler(QtMsgType type, const QMessageLogContext &, const QString &msg)
#endif
{
    if ( !runApp )
        return;

    QString msgString;

    switch ( type ) {
    case QtDebugMsg:
        msgString = "QtDebugMsg: " + QString(msg);
        break;
    case QtWarningMsg:
        msgString = "QtWarningMsg: " + QString(msg);
        break;
    case QtCriticalMsg:
        msgString = "QtCriticalMsg: " + QString(msg);
        break;
    case QtFatalMsg:
        msgString = "QtFatalMsg: " + QString(msg);
        break;
    default:
        return;
    }

    QMC2_ARCADE_LOG_STR(msgString);
}

void showHelp(ArcadeSettings *settings = NULL)
{
#if defined(QMC2_ARCADE_OS_WIN)
    // we need the console window to display the help text on Windows because we have no terminal connection
    if ( !consoleWindow ) {
        consoleWindow = new ConsoleWindow(0);
        consoleWindow->show();
    }
#endif

    QString defTheme = "ToxicWaste";
    QString defConsole = "terminal";
    QString defGSys = "raster";

    if ( settings ) {
        defTheme = settings->defaultTheme();
        defConsole = settings->defaultConsoleType();
        defGSys = settings->defaultGraphicsSystem();
    }

    QStringList themeList;
    foreach (QString theme, arcadeThemes) {
        if ( defTheme == theme )
            themeList << "[" + theme + "]";
        else
            themeList << theme;
    }
    QString availableThemes = themeList.join(", ");

    QStringList consoleList;
    foreach (QString console, consoleModes) {
        if ( defConsole == console )
            consoleList << "[" + console + "]";
        else
            consoleList << console;
    }
    QString availableConsoles = consoleList.join(", ");

    QStringList gSysList;
    foreach (QString gSys, graphicsSystems) {
        if ( defGSys == gSys )
            gSysList << "[" + gSys + "]";
        else
            gSysList << gSys;
    }
    QString availableGraphicsSystems = gSysList.join(", ");

    QString helpMessage;
    helpMessage  = "Usage: qmc2-arcade [-emu <emulator>] [-theme <theme>] [-console <type>] [-graphicssystem <engine>] [-config_path <path>] [-h|-?|-help]\n\n"
                   "Option           Meaning             Possible values ([..] = default)\n"
                   "---------------  ------------------  ------------------------------------\n"
                   "-emu             Emulator mode       [mame], mess, ume\n";
    helpMessage += "-theme           Theme selection     " + availableThemes + "\n";
    helpMessage += "-console         Console type        " + availableConsoles + "\n";
    helpMessage += "-graphicssystem  Graphics engine     " + availableGraphicsSystems + "\n";
    helpMessage += QString("-config_path     Configuration path  [%1], ...\n").arg(QMC2_ARCADE_DOT_PATH);

    QMC2_ARCADE_LOG_STR_NT(helpMessage);
}

#if defined(QMC2_ARCADE_OS_WIN)
#if defined(TCOD_VISUAL_STUDIO)
int SDL_main(int argc, char *argv[]) {
    return main(argc, argv);
}
#endif
#if defined(QMC2_ARCADE_MINGW)
#undef main
#endif
#endif

int main(int argc, char *argv[])
{
    qsrand(QDateTime::currentDateTime().toTime_t());
#if QT_VERSION < 0x050000
    qInstallMsgHandler(qtMessageHandler);
#else
    qInstallMessageHandler(qtMessageHandler);
#endif

    // available emulator-modes, themes, console-modes and graphics-systems
    emulatorModes << "mame" << "mess" << "ume";
    arcadeThemes << "ToxicWaste" << "darkone";
    mameThemes << "ToxicWaste" << "darkone";
    // messThemes << "..."
    umeThemes << "ToxicWaste" << "darkone";
    consoleModes << "terminal" << "window" << "window-minimized";
    graphicsSystems << "raster" << "native" << "opengl";

    // we have to make a copy of the command line arguments since QApplication's constructor "eats" -graphicssystem and its value (and we really need to know if it has been set!)
    for (int i = 0; i < argc; i++)
        argumentList << argv[i];

    QApplication *tempApp = new QApplication(argc, argv);

    QCoreApplication::setOrganizationName(QMC2_ARCADE_ORG_NAME);
    QCoreApplication::setOrganizationDomain(QMC2_ARCADE_ORG_DOMAIN);
    QCoreApplication::setApplicationName(QMC2_ARCADE_APP_NAME);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, QMC2_ARCADE_DYN_DOT_PATH);
    ArcadeSettings *startupConfig = new ArcadeSettings("");

    QString gSys = startupConfig->defaultGraphicsSystem();
    if ( QMC2_ARCADE_CLI_GSYS_VAL )
        gSys = QMC2_ARCADE_CLI_GSYS;

    delete startupConfig;
    delete tempApp;

    if ( !graphicsSystems.contains(gSys) ) {
        QMC2_ARCADE_LOG_STR_NT(QObject::tr("%1 is not a valid graphics-system - available graphics-systems: %2").arg(gSys).arg(graphicsSystems.join(", ")));
        return 1;
    }

    QApplication::setGraphicsSystem(gSys);

    // create the actual application instance
    QScopedPointer<QApplication> app(createApplication(argc, argv));

    if ( !QMC2_ARCADE_CLI_EMU_UNK ) {
        emulatorMode = QMC2_ARCADE_CLI_EMU_MAME ? QMC2_ARCADE_EMUMODE_MAME : QMC2_ARCADE_CLI_EMU_MESS ? QMC2_ARCADE_EMUMODE_MESS : QMC2_ARCADE_CLI_EMU_UME ? QMC2_ARCADE_EMUMODE_UME : QMC2_ARCADE_EMUMODE_UNK;
        if ( emulatorMode == QMC2_ARCADE_EMUMODE_UNK ) {
            showHelp();
            return 1;
        }
    } else if ( !emulatorModes.contains(QMC2_ARCADE_CLI_EMU) ) {
        QMC2_ARCADE_LOG_STR_NT(QObject::tr("%1 is not a valid emulator-mode - available emulator-modes: %2").arg(QMC2_ARCADE_CLI_EMU).arg(emulatorModes.join(", ")));
        return 1;
    }

    startupConfig = new ArcadeSettings("");

    QString console = startupConfig->defaultConsoleType();
    if ( QMC2_ARCADE_CLI_CONS_VAL )
        console = QMC2_ARCADE_CLI_CONS;

    if ( console == "window" || console == "window-minimized" ) {
        consoleMode = console == "window" ? QMC2_ARCADE_CONSOLE_WIN : QMC2_ARCADE_CONSOLE_WINMIN;
        consoleWindow = new ConsoleWindow(0);
        if ( consoleMode == QMC2_ARCADE_CONSOLE_WINMIN )
            consoleWindow->showMinimized();
        else
            consoleWindow->show();
    } else if ( !consoleModes.contains(console) ) {
        QMC2_ARCADE_LOG_STR_NT(QObject::tr("%1 is not a valid console-mode - available console-modes: %2").arg(console).arg(consoleModes.join(", ")));
        return 1;
    }

    if ( QMC2_ARCADE_CLI_HELP || QMC2_ARCADE_CLI_INVALID ) {
        showHelp(startupConfig);
        if ( !consoleWindow ) {
            delete startupConfig;
            return 1;
        } else
            runApp = false;
    }

    QString theme = startupConfig->defaultTheme();
    if ( QMC2_ARCADE_CLI_THEME_VAL )
        theme = QMC2_ARCADE_CLI_THEME;

    if ( !arcadeThemes.contains(theme) && runApp ) {
        QMC2_ARCADE_LOG_STR_NT(QObject::tr("%1 is not valid theme - available themes: %2").arg(theme).arg(arcadeThemes.join(", ")));
        if ( !consoleWindow ) {
            delete startupConfig;
            return 1;
        } else
            runApp = false;
    }

    delete startupConfig;

    switch ( emulatorMode ) {
    case QMC2_ARCADE_EMUMODE_MAME:
        if ( !mameThemes.contains(theme) && runApp ) {
            QMC2_ARCADE_LOG_STR_NT(QObject::tr("%1 is not a valid %2 theme - available %2 themes: %3").arg(theme).arg(emulatorModes[QMC2_ARCADE_EMUMODE_MAME]).arg(mameThemes.isEmpty() ? QObject::tr("(none)") : mameThemes.join(", ")));
            if ( !consoleWindow )
                return 1;
            else
                runApp = false;
        }
        break;
    case QMC2_ARCADE_EMUMODE_MESS:
        if ( !messThemes.contains(theme) && runApp ) {
            QMC2_ARCADE_LOG_STR_NT(QObject::tr("%1 is not a valid %2 theme - available %2 themes: %3").arg(theme).arg(emulatorModes[QMC2_ARCADE_EMUMODE_MESS]).arg(messThemes.isEmpty() ? QObject::tr("(none)") : messThemes.join(", ")));
            if ( !consoleWindow )
                return 1;
            else
                runApp = false;
        }
        break;
    case QMC2_ARCADE_EMUMODE_UME:
        if ( !umeThemes.contains(theme) && runApp ) {
            QMC2_ARCADE_LOG_STR_NT(QObject::tr("%1 is not a valid %2 theme - available %2 themes: %3").arg(theme).arg(emulatorModes[QMC2_ARCADE_EMUMODE_UME]).arg(umeThemes.isEmpty() ? QObject::tr("(none)") : umeThemes.join(", ")));
            if ( !consoleWindow )
                return 1;
            else
                runApp = false;
        }
        break;
    }

    // create global settings
    globalConfig = new ArcadeSettings(theme);
    globalConfig->setApplicationVersion(QMC2_ARCADE_APP_VERSION);

    // load translator
    QString language = globalConfig->language();
    if ( !globalConfig->languageMap.contains(language) )
        language = "us";
    QTranslator qmc2ArcadeTranslator;
    if ( qmc2ArcadeTranslator.load(QString("qmc2-arcade_%1").arg(language), ":/translations") )
        app->installTranslator(&qmc2ArcadeTranslator);

    int returnCode;

    if ( runApp ) {
        // log banner message
        QString bannerMessage = QString("%1 %2 (%3)").
                                arg(QMC2_ARCADE_APP_TITLE).
#if defined(QMC2_ARCADE_SVN_REV)
                                arg(QMC2_ARCADE_APP_VERSION + QString(", SVN r%1").arg(XSTR(QMC2_ARCADE_SVN_REV))).
#else
                                arg(QMC2_ARCADE_APP_VERSION).
#endif
                                arg(QString("Qt ") + qVersion() + ", " +
                                    QObject::tr("emulator-mode: %1").arg(emulatorModes[emulatorMode]) + ", " +
                                    QObject::tr("theme: %1").arg(theme) + ", " +
                                    QObject::tr("graphics-system: %1").arg(gSys) + ", " +
                                    QObject::tr("console-mode: %1").arg(consoleModes[consoleMode]));

        QMC2_ARCADE_LOG_STR(bannerMessage);

        if ( consoleWindow )
            consoleWindow->loadSettings();

        // set up the main QML app viewer window
        TweakedQmlApplicationViewer *viewer = new TweakedQmlApplicationViewer();
        viewer->setWindowIcon(QIcon(QLatin1String(":/images/qmc2-arcade.png")));
        viewer->setWindowTitle(QMC2_ARCADE_APP_TITLE + " " + QMC2_ARCADE_APP_VERSION);
        viewer->setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
        QMC2_ARCADE_LOG_STR(QObject::tr("Starting QML viewer using theme '%1'").arg(theme));
        viewer->setSource(QString("qrc:/qml/%1/%1.qml").arg(theme));

        // set up display mode initially...
        if ( globalConfig->fullScreen() )
            viewer->switchToFullScreen(true);
        else
            viewer->switchToWindowed(true);

        // ... and run the application
        returnCode = app->exec();

        delete viewer;
    } else {
        if ( consoleWindow ) {
            consoleWindow->loadSettings();
            QString consoleMessage(QObject::tr("QML viewer not started - please close the console window to exit"));
            QMC2_ARCADE_LOG_STR_NT(QString("-").repeated(consoleMessage.length()));
            QMC2_ARCADE_LOG_STR_NT(consoleMessage);
            QMC2_ARCADE_LOG_STR_NT(QString("-").repeated(consoleMessage.length()));
            consoleWindow->showNormal();
            consoleWindow->raise();
            app->exec();
        }
        returnCode = 1;
    }

    if ( consoleWindow ) {
        consoleWindow->saveSettings();
        delete consoleWindow;
    }

    delete globalConfig;

    return returnCode;
}
