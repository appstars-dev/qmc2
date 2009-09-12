#include <QProcess>
#include <QThread>

#include "about.h"
#include "macros.h"
#include "options.h"
#include "emuopt.h"
#include "qmc2main.h"
#include "gamelist.h"

#if QMC2_JOYSTICK == 1
#if defined(Q_WS_WIN)
#include <SDL.h>
#else
#include <SDL/SDL.h>
#endif
#endif

#if QMC2_USE_PHONON_API
#include "qmc2_phonon.h"
#endif

#if defined(Q_WS_MAC) || defined(Q_WS_WIN)
#include <QSysInfo>
#endif

// external global variables
extern MainWindow *qmc2MainWindow;
extern Gamelist *qmc2Gamelist;
extern EmulatorOptions *qmc2GlobalEmulatorOptions;

About::About(QWidget *parent)
  : QDialog(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: About::About(QWidget *parent = 0x" + QString::number((ulong)parent, 16) + ")");
#endif

  setupUi(this);

  widgetSize = QSize(-1, -1);
  widgetPosValid = FALSE;
  ignoreResizeAndMove = TRUE;

  adjustSize();

#if defined(Q_WS_MAC)
  switch ( QSysInfo::MacintoshVersion ) {
    case QSysInfo::MV_10_3: macVersion = tr("Mac OS X 10.3"); break;
    case QSysInfo::MV_10_4: macVersion = tr("Mac OS X 10.4"); break;
    case QSysInfo::MV_10_5: macVersion = tr("Mac OS X 10.5"); break;
    case QSysInfo::MV_10_6: macVersion = tr("Mac OS X 10.6"); break;
    default: macVersion = tr("Mac (unkown)"); break;
  }
#elif defined(Q_WS_WIN)
  switch ( QSysInfo::WindowsVersion ) {
    case QSysInfo::WV_4_0: winVersion = tr("Windows NT (Windows 4.0)"); break;
    case QSysInfo::WV_5_0: winVersion = tr("Windows 2000 (Windows 5.0)"); break;
    case QSysInfo::WV_5_1: winVersion = tr("Windows XP (Windows 5.1)"); break;
    case QSysInfo::WV_5_2: winVersion = tr("Windows Server 2003, Windows Server 2003 R2, Windows Home Server or Windows XP Professional x64 Edition (Windows 5.2)"); break;
    case QSysInfo::WV_6_0: winVersion = tr("Windows Vista or Windows Server 2008 (Windows 6.0)"); break;
    case QSysInfo::WV_6_1: winVersion = tr("Windows 7 (Windows 6.1)"); break;
    default: winVersion = tr("Windows (unknown)"); break;
  }
#endif
}

About::~About()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: About::~About()");
#endif

}

void About::showEvent(QShowEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: About::showEvent(QShowEvent *e = 0x" + QString::number((ulong)e, 16) + ")");
#endif

  ignoreResizeAndMove = TRUE;

#if defined(QMC2_SHOWMEMINFO)
  // get memory information
  quint64 numPages, pageSize, freePages, totalSize, totalUsed, totalFree;
  numPages = sysconf(_SC_PHYS_PAGES) / 1024;
  pageSize = sysconf(_SC_PAGESIZE) / 1024;
  freePages = sysconf( _SC_AVPHYS_PAGES) / 1024;
  totalSize = numPages * pageSize;
  totalFree = pageSize * freePages;
  totalUsed = totalSize - totalFree;
#endif

#if QMC2_JOYSTICK == 1
  const SDL_version *sdlVersionLinked = SDL_Linked_Version();
  SDL_version sdlVersionCompile;
  SDL_VERSION(&sdlVersionCompile);
#endif

  int numLogicalCores = QThread::idealThreadCount();

  QString titleString =
#if defined(QMC2_EMUTYPE_MAME)
          "<p><font size=\"+1\"><b>QMC2 - M.A.M.E. Catalog / Launcher II</b></font><br>" +
#elif defined(QMC2_EMUTYPE_MESS)
          "<p><font size=\"+1\"><b>QMC2 - M.E.S.S. Catalog / Launcher II</b></font><br>" +
#endif
          tr("Qt 4 based multi-platform/multi-emulator front end") + "<br>" +
          tr("Version ") + QString::number(MAJOR) + "." + QString::number(MINOR) +
#ifdef BETA
          ".b" + QString::number(BETA) +
#endif
          ", " + tr("built for") + 
#if defined(QMC2_SDLMAME)
          " SDLMAME" +
#elif defined(QMC2_SDLMESS)
          " SDLMESS" +
#elif defined(QMC2_MAME)
          " MAME" +
#elif defined(QMC2_MESS)
          " MESS" +
#else
          " ???" +
#endif
          "<br>" + tr("Copyright") + " &copy; 2006 - 2009 R. Reucher, " + tr("Germany") + "</p>";
  labelTitle->setText(titleString);

  QString projectInfoString =
          "<p><b>" + tr("Project homepage:") + "</b><br><a href=\"http://qmc2.arcadehits.net/wordpress\">http://qmc2.arcadehits.net/wordpress</a></p>" +
          "<p><b>" + tr("Development site:") + "</b><br><a href=\"http://sourceforge.net/projects/qmc2\">http://sourceforge.net/projects/qmc2</a></p>" +
          "<p><b>" + tr("QMC2 development mailing list:") + "</b><br>qmc2-devel@lists.sourceforge.net (" + tr("subscription required") +")</p>" +
          "<p><b>" + tr("List subscription:") + "</b><br><a href=\"http://lists.sourceforge.net/lists/listinfo/qmc2-devel\">https://lists.sourceforge.net/lists/listinfo/qmc2-devel</a></p>" +
          "<p><b>" + tr("Bug tracking system:") + "</b><br><a href=\"http://sourceforge.net/apps/mantisbt/qmc2/my_view_page.php\">http://sourceforge.net/apps/mantisbt/qmc2</a></p>";
  labelProjectInfo->setText(projectInfoString);

  QString sysInfoString =
#if !defined(Q_WS_WIN)
          "<p><b>" + tr("Build OS:") + "</b><br>" + XSTR(TARGET_OS_NAME) + " " + XSTR(TARGET_OS_RELEASE) + " " + XSTR(TARGET_MACHINE) + "</p>" +
#endif
#if defined(Q_WS_MAC)
          "<p><b>" + tr("Running OS:") + "</b><br>" + macVersion + "</p>" +
#elif defined(Q_WS_WIN)
          "<p><b>" + tr("Running OS:") + "</b><br>" + winVersion + "</p>" +
#endif
          "<p><b>" + tr("Emulator version:") + "</b><br>" + qmc2Gamelist->emulatorVersion + "</p>" +
          "<p><b>" + tr("Template information:") + "</b><br>" + tr("Emulator:") + " " + qmc2GlobalEmulatorOptions->templateEmulator + "<br>" + tr("Version:") + " " + qmc2GlobalEmulatorOptions->templateVersion + "<br>" + tr("Format:") + " " + qmc2GlobalEmulatorOptions->templateFormat + "</p>" +
          "<p><b>" + tr("Qt version:") + "</b><br>" + tr("Compile-time:") + " " + QT_VERSION_STR + "<br>" + tr("Run-time:") + " " + qVersion() + "</p>" +
#if QMC2_USE_PHONON_API
          "<p><b>" + tr("Phonon version:") + "</b><br>" + tr("Run-time:") + " " + QString("%1").arg(Phonon::phononVersion()) + "</p>" +
#endif
#if QMC2_JOYSTICK == 1
          "<p><b>" + tr("SDL version:") + "</b><br>" + tr("Compile-time:") + " " + QString("%1.%2.%3").arg(sdlVersionCompile.major).arg(sdlVersionCompile.minor).arg(sdlVersionCompile.patch) + "<br>" + tr("Run-time:") + " " + QString("%1.%2.%3").arg(sdlVersionLinked->major).arg(sdlVersionLinked->minor).arg(sdlVersionLinked->patch) + "</p>" +
#endif
#if defined(QMC2_SHOWMEMINFO)
          "<p><b>" + tr("Physical memory:") + "</b><br>" + tr("Total: %1 MB").arg(totalSize) + "<br>" + tr("Free: %1 MB").arg(totalFree) + "<br>" + tr("Used: %1 MB").arg(totalUsed) + "</p>"
#endif
          "<p><b>" + tr("Number of CPUs:") + "</b><br>" + (numLogicalCores != -1 ? QString("%1").arg(numLogicalCores) : tr("unknown")) + "</p>"
          "<p><b>" + tr("Environment variables:") + "</b>";
  QStringList envVars = QProcess::systemEnvironment();
  envVars.sort();
  foreach (QString ev, envVars)
    sysInfoString += "<br>" + ev;
  sysInfoString += "</p>";
  textBrowserSystemInformation->setText(sysInfoString);

  // ensure a black background for the logo image
  labelLogoPixmap->setPalette(QPalette(QColor(0, 0, 0)));

  if ( widgetSize.isValid() )
    resize(widgetSize);
  if ( widgetPosValid )
    move(widgetPos);

  ignoreResizeAndMove = FALSE;

  e->accept();
}

void About::resizeEvent(QResizeEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: About::resizeEvent(QResizeEvent *e = 0x" + QString::number((ulong)e, 16) + ")");
#endif

  if ( !ignoreResizeAndMove )
    widgetSize = size();

  e->accept();
}

void About::moveEvent(QMoveEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: About::moveEvent(QMoveEvent *e = 0x" + QString::number((ulong)e, 16) + ")");
#endif

  if ( !ignoreResizeAndMove ) {
    widgetPos = pos();
    widgetPosValid = TRUE;
  }

  e->accept();
}
