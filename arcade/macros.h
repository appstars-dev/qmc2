#ifndef MACROS_H
#define MACROS_H

#include <stdio.h>

// make a string out of a non-string constant
#define STR(s)                      #s
#define XSTR(s)                     STR(s)

// application and ini related
#define QMC2_ARCADE_ORG_DOMAIN      QString("qmc2.arcadehits.net")
#define QMC2_ARCADE_ORG_NAME        QString("qmc2")
#define QMC2_ARCADE_APP_VERSION     QString(XSTR(QMC2_ARCADE_VERSION))
#define QMC2_ARCADE_APP_TITLE       QObject::tr("QMC2 Arcade Mode")
#define QMC2_ARCADE_APP_NAME        QMC2_ARCADE_ORG_NAME

// dot-path related
#if defined(Q_OS_MAC)
#define QMC2_ARCADE_DOT_PATH        (QDir::homePath() + "/Library/Application Support/qmc2")
#else
#define QMC2_ARCADE_DOT_PATH        (QDir::homePath() + "/.qmc2")
#endif
#define QMC2_ARCADE_DYN_DOT_PATH    (qApp->arguments().indexOf("-config_path") >= 0 && qApp->arguments().indexOf("-config_path") + 1 <= qApp->arguments().count() ? qApp->arguments()[qApp->arguments().indexOf("-config_path") + 1]: QMC2_ARCADE_DOT_PATH)

// ROM states
#define QMC2_ARCADE_ROMSTATE_C      0
#define QMC2_ARCADE_ROMSTATE_M      1
#define QMC2_ARCADE_ROMSTATE_I      2
#define QMC2_ARCADE_ROMSTATE_N      3
#define QMC2_ARCADE_ROMSTATE_U      4

// debuggung macros
#define QMC2_PRINT_TXT(t)           printf("%s\n", #t)
#define QMC2_PRINT_STR(s)           printf("%s = %s\n", #s, (const char *)s.toLocal8Bit())
#define QMC2_PRINT_CSTR(s)          printf("%s = %s\n", #s, (const char *)s)
#define QMC2_PRINT_PTR(p)           printf("%s = %p\n", #p, p)
#define QMC2_PRINT_INT(i)           printf("%s = %ld\n", #i, i)
#define QMC2_PRINT_HEX(x)           printf("%s = %x\n", #x, x)
#define QMC2_PRINT_BOOL(b)          printf("%s = %s\n", #b, b ? "true" : "false")
#define QMC2_PRINT_STRLST(l)        for (int i = 0; i < l.count(); i++) printf("%s[%ld] = %s\n", #l, i, (const char *)l[i].toLocal8Bit())

#endif
