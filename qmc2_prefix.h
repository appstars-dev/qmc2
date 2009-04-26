#ifdef __cplusplus

#include <QtGui>
#include <QAction>
#include <QApplication>
#include <QBitArray>
#include <QByteArray>
#include <QClipboard>
#include <QComboBox>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QFontDialog>
#include <QFontMetrics>
#include <QHeaderView>
#include <QIcon>
#include <QImage>
#include <QLineEdit>
#include <QLocale>
#include <QMap>
#include <QMatrix>
#include <QMessageBox>
#include <QMutex>
#include <QPair>
#include <QPixmap>
#include <QPixmapCache>
#include <QProcess>
#include <QScrollBar>
#include <QSet>
#include <QSettings>
#include <QSpinBox>
#include <QStringList>
#include <QStyleFactory>
#include <QTest>
#include <QTextStream>
#include <QThread>
#include <QTimer>
#include <QTranslator>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#endif /* __cplusplus */

#if defined(Q_WS_WIN)
#if QMC2_JOYSTICK == 1
#include <SDL.h>
#endif
#include <io.h>
#else
#if QMC2_JOYSTICK == 1
#include <SDL/SDL.h>
#endif
#include <unistd.h>
#endif

#ifdef __OBJC__

#import <Cocoa/Cocoa.h>

#endif /* __OBJC__ */
