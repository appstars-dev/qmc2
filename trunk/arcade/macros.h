#ifndef MACROS_H
#define MACROS_H

#include <Qt>

// make a string out of a non-string constant
#define STR(s)                        #s
#define XSTR(s)                       STR(s)

// global OS macros for supported target operating systems
#if (defined(Q_OS_UNIX) || defined(Q_OS_LINUX)) && !defined(Q_OS_MAC)
#define QMC2_ARCADE_OS_UNIX
#if defined(Q_OS_LINUX)
#define QMC2_ARCADE_OS_LINUX
#define QMC2_ARCADE_OS_NAME           QString("Linux")
#else
#define QMC2_ARCADE_OS_NAME           QString("UNIX")
#endif
#elif defined(Q_OS_MAC)
#define QMC2_ARCADE_OS_MAC
#define QMC2_ARCADE_OS_NAME           QString("Darwin")
#elif defined(Q_OS_WIN32) || defined(Q_OS_WIN64)
#define QMC2_ARCADE_OS_WIN
#define QMC2_ARCADE_OS_NAME           QString("Windows")
#else
#error "Target OS is not supported -- QMC2 Arcade currently supports Linux/UNIX, Windows and Mac OS X!"
#endif

#include <QStringList>
#include <QString>
#include <QRegExp>
#include <QTime>
#include <QDir>
#include <stdio.h>

// application and ini related
#define QMC2_ARCADE_ORG_DOMAIN        QString("qmc2.arcadehits.net")
#define QMC2_ARCADE_ORG_NAME          QString("qmc2")
#define QMC2_ARCADE_APP_VERSION       QString(XSTR(QMC2_ARCADE_VERSION))
#define QMC2_ARCADE_APP_TITLE         QObject::tr("QMC2 Arcade")
#define QMC2_ARCADE_APP_NAME          QMC2_ARCADE_ORG_NAME

// relevant game list cache data columns
#define QMC2_ARCADE_GLC_ID            0
#define QMC2_ARCADE_GLC_DESCRIPTION   1
#define QMC2_ARCADE_GLC_BIOS          5
#define QMC2_ARCADE_GLC_DEVICE        10

// dot-path related
#if defined(Q_OS_MAC)
#define QMC2_ARCADE_DOT_PATH          (QDir::homePath() + "/Library/Application Support/qmc2")
#else
#define QMC2_ARCADE_DOT_PATH          (QDir::homePath() + "/.qmc2")
#endif
#define QMC2_ARCADE_DYN_DOT_PATH      (argumentList.indexOf("-config_path") >= 0 && argumentList.indexOf("-config_path") + 1 <= argumentList.count() ? argumentList[argumentList.indexOf("-config_path") + 1]: QMC2_ARCADE_DOT_PATH)

// ROM states
#define QMC2_ARCADE_ROMSTATE_C        0
#define QMC2_ARCADE_ROMSTATE_M        1
#define QMC2_ARCADE_ROMSTATE_I        2
#define QMC2_ARCADE_ROMSTATE_N        3
#define QMC2_ARCADE_ROMSTATE_U        4

// emulator modes
#define QMC2_ARCADE_EMUMODE_UNK       -1
#define QMC2_ARCADE_EMUMODE_MAME      0
#define QMC2_ARCADE_EMUMODE_MESS      1
#define QMC2_ARCADE_EMUMODE_UME       2

// console modes
#define QMC2_ARCADE_CONSOLE_UNK       -1
#define QMC2_ARCADE_CONSOLE_TERM      0
#define QMC2_ARCADE_CONSOLE_WIN       1
#define QMC2_ARCADE_CONSOLE_WINMIN    2

// emulator option types
#define QMC2_ARCADE_EMUOPT_UNKNOWN    0
#define QMC2_ARCADE_EMUOPT_BOOL       1
#define QMC2_ARCADE_EMUOPT_INT        2
#define QMC2_ARCADE_EMUOPT_FLOAT      3
#define QMC2_ARCADE_EMUOPT_FLOAT1     QMC2_ARCADE_EMUOPT_FLOAT
#define QMC2_ARCADE_EMUOPT_STRING     4
#define QMC2_ARCADE_EMUOPT_FILE       5
#define QMC2_ARCADE_EMUOPT_FOLDER     6
#define QMC2_ARCADE_EMUOPT_COMBO      7
#define QMC2_ARCADE_EMUOPT_FLOAT2     8
#define QMC2_ARCADE_EMUOPT_FLOAT3     9

// ZIP read buffer size
#define QMC2_ARCADE_ZIP_BUFSIZE       64*1024

// number of image- & pixmap-cache slots
#define QMC2_ARCADE_IMGCACHE_SIZE     100

// indexed QML themes
#define QMC2_ARCADE_THEME_TOXICWASTE  0
#define QMC2_ARCADE_THEME_DARKONE     1

// additional command line arguments
// -emu <emu> ([mame], mess, ume)
#define QMC2_ARCADE_CLI_EMU           (argumentList.indexOf("-emu") >= 0 && argumentList.count() > argumentList.indexOf("-emu") + 1 ? argumentList[argumentList.indexOf("-emu") + 1].toLower() : "mame")
#define QMC2_ARCADE_CLI_EMU_MAME      (argumentList.indexOf("-emu") >= 0 && argumentList.count() > argumentList.indexOf("-emu") + 1 ? argumentList[argumentList.indexOf("-emu") + 1].toLower() == "mame" : false)
#define QMC2_ARCADE_CLI_EMU_MESS      (argumentList.indexOf("-emu") >= 0 && argumentList.count() > argumentList.indexOf("-emu") + 1 ? argumentList[argumentList.indexOf("-emu") + 1].toLower() == "mess" : false)
#define QMC2_ARCADE_CLI_EMU_UME       (argumentList.indexOf("-emu") >= 0 && argumentList.count() > argumentList.indexOf("-emu") + 1 ? argumentList[argumentList.indexOf("-emu") + 1].toLower() == "ume" : false)
#define QMC2_ARCADE_CLI_EMU_UNK       (!QMC2_ARCADE_CLI_EMU_MAME && !QMC2_ARCADE_CLI_EMU_MESS && !QMC2_ARCADE_CLI_EMU_UME)
#define QMC2_ARCADE_CLI_EMU_INV       (argumentList.indexOf("-emu") >= 0 && argumentList.count() == argumentList.indexOf("-emu") + 1)
// -theme <theme> ([ToxicWaste], darkone)
#define QMC2_ARCADE_CLI_THEME         (argumentList.indexOf("-theme") >= 0 && argumentList.count() > argumentList.indexOf("-theme") + 1 ? argumentList[argumentList.indexOf("-theme") + 1] : "ToxicWaste")
#define QMC2_ARCADE_CLI_THEME_INV     (argumentList.indexOf("-theme") >= 0 && argumentList.count() == argumentList.indexOf("-theme") + 1)
#define QMC2_ARCADE_CLI_THEME_VAL     (argumentList.indexOf("-theme") >= 0 && argumentList.count() > argumentList.indexOf("-theme") + 1)
// -console <mode> ([terminal], window, window-minimized)
#define QMC2_ARCADE_CLI_CONS          (argumentList.indexOf("-console") >= 0 && argumentList.count() > argumentList.indexOf("-console") + 1 ? argumentList[argumentList.indexOf("-console") + 1] : "terminal")
#define QMC2_ARCADE_CLI_CONS_INV      (argumentList.indexOf("-console") >= 0 && argumentList.count() == argumentList.indexOf("-console") + 1)
#define QMC2_ARCADE_CLI_CONS_VAL      (argumentList.indexOf("-console") >= 0 && argumentList.count() > argumentList.indexOf("-console") + 1)
// -graphicssystem <gsys> ([raster], native, opengl)
#if QT_VERSION < 0x050000
#define QMC2_ARCADE_CLI_GSYS          (argumentList.indexOf("-graphicssystem") >= 0 && argumentList.count() > argumentList.indexOf("-graphicssystem") + 1 ? argumentList[argumentList.indexOf("-graphicssystem") + 1] : "raster")
#define QMC2_ARCADE_CLI_GSYS_INV      (argumentList.indexOf("-graphicssystem") >= 0 && argumentList.count() == argumentList.indexOf("-graphicssystem") + 1)
#define QMC2_ARCADE_CLI_GSYS_VAL      (argumentList.indexOf("-graphicssystem") >= 0 && argumentList.count() > argumentList.indexOf("-graphicssystem") + 1)
#else
#define QMC2_ARCADE_CLI_GSYS_INV      false
#endif
// -language <lang>
#define QMC2_ARCADE_CLI_LANG          (argumentList.indexOf("-language") >= 0 && argumentList.count() > argumentList.indexOf("-language") + 1 ? argumentList[argumentList.indexOf("-language") + 1] : "us")
#define QMC2_ARCADE_CLI_LANG_INV      (argumentList.indexOf("-language") >= 0 && argumentList.count() == argumentList.indexOf("-language") + 1)
#define QMC2_ARCADE_CLI_LANG_VAL      (argumentList.indexOf("-language") >= 0 && argumentList.count() > argumentList.indexOf("-language") + 1)
// -h|-?|-help
#define QMC2_ARCADE_CLI_HELP          (argumentList.indexOf(QRegExp("(-h|-\\?|-help)")) >= 0)
// argument validation
#define QMC2_ARCADE_CLI_INVALID       (QMC2_ARCADE_CLI_EMU_INV || QMC2_ARCADE_CLI_THEME_INV || QMC2_ARCADE_CLI_CONS_INV || QMC2_ARCADE_CLI_GSYS_INV || QMC2_ARCADE_CLI_LANG_INV)

// console logging macros
#define QMC2_ARCADE_LOG_STR(s)        if ( !consoleWindow ) { printf("%s: %s\n", (const char *)QTime::currentTime().toString("hh:mm:ss.zzz").toLocal8Bit(), (const char *)s.toLocal8Bit()); fflush(stdout); } else { consoleWindow->appendPlainText(QTime::currentTime().toString("hh:mm:ss.zzz") + ": " + s); }
#define QMC2_ARCADE_LOG_STR_NT(s)     if ( !consoleWindow ) { printf("%s\n", (const char *)s.toLocal8Bit()); fflush(stdout); } else { consoleWindow->appendPlainText(s); }
#define QMC2_ARCADE_LOG_CSTR(s)       if ( !consoleWindow ) { printf("%s: %s\n", (const char *)QTime::currentTime().toString("hh:mm:ss.zzz").toLocal8Bit(), (const char *)s); fflush(stdout); } else { consoleWindow->appendPlainText(QTime::currentTime().toString("hh:mm:ss.zzz") + ": " + QString(s)); }
#define QMC2_ARCADE_LOG_CSTR_NT(s)    if ( !consoleWindow ) { printf("%s\n", (const char *)s); fflush(stdout); } else { consoleWindow->appendPlainText(QString(s)); }

// exchangable (de)compression routines
#define QMC2_ARCADE_COMPRESS(data)    qCompress((data))
#define QMC2_ARCADE_UNCOMPRESS(data)  (data).isEmpty() ? QByteArray() : qUncompress(data)

#endif
