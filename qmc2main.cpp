#include <QMessageBox>
#include <QPixmapCache>
#include <QCache>
#include <QHeaderView>
#include <QTextStream>
#include <QScrollBar>
#include <QProcess>
#include <QTimer>
#include <QTime>
#include <QFile>
#include <QMap>
#include <QStyleFactory>
#include <QBitArray>
#include <QFileInfo>
#include <QFileDialog>
#include <QAction>
#include <QPair>
#include <QClipboard>
#include <QDateTime>
#include <QMutex>
#include <QtWebKit>
#include <QNetworkAccessManager>
#include <QSplashScreen>
#if defined(QMC2_SDLMAME) || defined(QMC2_SDLMESS)
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

#include "macros.h"
#include "qmc2main.h"
#include "options.h"
#include "emuopt.h"
#include "procmgr.h"
#include "gamelist.h"
#include "preview.h"
#include "flyer.h"
#include "cabinet.h"
#include "controller.h"
#include "marquee.h"
#include "title.h"
#include "pcb.h"
#include "docbrowser.h"
#include "about.h"
#include "welcome.h"
#include "imgcheck.h"
#include "sampcheck.h"
#include "romalyzer.h"
#include "romstatusexport.h"
#include "detailsetup.h"
#include "miniwebbrowser.h"
#include "unzip.h"
#include "downloaditem.h"
#include "mawsqdlsetup.h"
#include "embedder.h"
#include "demomode.h"
#include "softwarelist.h"
#if QMC2_JOYSTICK == 1
#include "joystick.h"
#endif
#if defined(QMC2_EMUTYPE_MESS)
#include "messdevcfg.h"
#endif
#if QMC2_USE_PHONON_API
#include <QTest>
#include "audioeffects.h"
#endif
#include "toolexec.h"
#include "arcade/arcadeview.h"
#include "arcade/arcadesetupdialog.h"
#if defined(Q_WS_X11)
#include "keyseqscan.h"
#endif
#if defined(QMC2_YOUTUBE_ENABLED)
#include "youtubevideoplayer.h"
#endif
#if defined(Q_WS_WIN)
#include "windows_tools.h"
#endif
#include "htmleditor/htmleditor.h"

#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#if defined(verify) // verify() is #defined in AssertMacros.h on OS X
#undef verify
#endif
#endif

// global variables
MainWindow *qmc2MainWindow = NULL;
Gamelist *qmc2Gamelist = NULL;
Options *qmc2Options = NULL;
QSettings *qmc2Config = NULL;
EmulatorOptions *qmc2GlobalEmulatorOptions = NULL;
EmulatorOptions *qmc2EmulatorOptions = NULL;
ProcessManager *qmc2ProcessManager = NULL;
Preview *qmc2Preview = NULL;
Flyer *qmc2Flyer = NULL;
Cabinet *qmc2Cabinet = NULL;
Controller *qmc2Controller = NULL;
Marquee *qmc2Marquee = NULL;
Title *qmc2Title = NULL;
PCB *qmc2PCB = NULL;
About *qmc2About = NULL;
DocBrowser *qmc2DocBrowser = NULL;
Welcome *qmc2Welcome = NULL;
ImageChecker *qmc2ImageChecker = NULL;
SampleChecker *qmc2SampleChecker = NULL;
ROMAlyzer *qmc2ROMAlyzer = NULL;
ROMStatusExporter *qmc2ROMStatusExporter = NULL;
DetailSetup *qmc2DetailSetup = NULL;
QWidget *qmc2DetailSetupParent = NULL;
ArcadeView *qmc2ArcadeView = NULL;
ArcadeSetupDialog *qmc2ArcadeSetupDialog = NULL;
SoftwareList *qmc2SoftwareList = NULL;
SoftwareSnap *qmc2SoftwareSnap = NULL;
SoftwareSnapshot *qmc2SoftwareSnapshot = NULL;
QString qmc2DriverName = "";
#if defined(QMC2_EMUTYPE_MESS)
MESSDeviceConfigurator *qmc2MESSDeviceConfigurator = NULL;
QTreeWidgetItem *qmc2LastDeviceConfigItem = NULL;
#endif
QTreeWidgetItem *qmc2LastSoftwareListItem = NULL;
#if QMC2_USE_PHONON_API
AudioEffectDialog *qmc2AudioEffectDialog = NULL;
QString qmc2AudioLastIndividualTrack = "";
#endif
DemoModeDialog *qmc2DemoModeDialog = NULL;
bool qmc2ReloadActive = false;
bool qmc2ImageCheckActive = false;
bool qmc2SampleCheckActive = false;
bool qmc2EarlyReloadActive = false;
bool qmc2VerifyActive = false;
bool qmc2FilterActive = false;
bool qmc2ROMAlyzerActive = false;
bool qmc2ROMAlyzerPaused = false;
bool qmc2GuiReady = false;
bool qmc2CleaningUp = false;
bool qmc2StartingUp = true;
bool qmc2EarlyStartup = true;
bool qmc2ScaledPreview = true;
bool qmc2ScaledFlyer = true;
bool qmc2ScaledCabinet = true;
bool qmc2ScaledController = true;
bool qmc2ScaledMarquee = true;
bool qmc2ScaledTitle = true;
bool qmc2ScaledPCB = true;
bool qmc2SmoothScaling = false;
bool qmc2RetryLoadingImages = true;
bool qmc2ParentImageFallback = false;
bool qmc2UsePreviewFile = false;
bool qmc2UseFlyerFile = false;
bool qmc2UseCabinetFile = false;
bool qmc2UseControllerFile = false;
bool qmc2UseIconFile = false;
bool qmc2UseMarqueeFile = false;
bool qmc2UseTitleFile = false;
bool qmc2UsePCBFile = false;
bool qmc2UseSoftwareSnapFile = false;
bool qmc2IconsPreloaded = false;
bool qmc2CheckItemVisibility = true;
bool qmc2AutomaticReload = false;
bool qmc2LoadingGameInfoDB = false;
bool qmc2WidgetsEnabled = true;
bool qmc2ExportingROMStatus = false;
bool qmc2StatesTogglesEnabled = true;
bool qmc2VariantSwitchReady = false;
bool qmc2DestroyingArcadeView = false;
bool qmc2IgnoreItemActivation = false;
bool qmc2StartEmbedded = false;
bool qmc2FifoIsOpen = false;
bool qmc2SuppressQtMessages = false;
int qmc2GamelistResponsiveness = 100;
int qmc2UpdateDelay = 10;
QFile *qmc2FrontendLogFile = NULL;
QFile *qmc2EmulatorLogFile = NULL;
QFile *qmc2FifoFile = NULL;
QTextStream qmc2FrontendLogStream;
QTextStream qmc2EmulatorLogStream;
QTranslator *qmc2Translator = NULL;
QTranslator *qmc2QtTranslator = NULL;
QString qmc2LastFrontendLogMessage = "";
quint64 qmc2FrontendLogMessageRepeatCount = 0;
QString qmc2LastEmulatorLogMessage = "";
quint64 qmc2EmulatorLogMessageRepeatCount = 0;
bool qmc2StopParser = false;
QTreeWidgetItem *qmc2CurrentItem = NULL;
QTreeWidgetItem *qmc2LastConfigItem = NULL;
QTreeWidgetItem *qmc2LastGameInfoItem = NULL;
bool qmc2LoadingEmuInfoDB = false;
QTreeWidgetItem *qmc2LastEmuInfoItem = NULL;
QMap<QString, QByteArray *> qmc2EmuInfoDB;
MiniWebBrowser *qmc2MAWSLookup = NULL;
QTreeWidgetItem *qmc2LastMAWSItem = NULL;
QCache<QString, QByteArray> qmc2MAWSCache;
#if defined(QMC2_EMUTYPE_MAME)
MawsQuickDownloadSetup *qmc2MawsQuickDownloadSetup = NULL;
QMap<QString, QString> qmc2CategoryMap;
QMap<QString, QString> qmc2VersionMap;
QMap<QString, QTreeWidgetItem *> qmc2CategoryItemMap;
QMap<QString, QTreeWidgetItem *> qmc2VersionItemMap;
QTreeWidgetItem *qmc2CategoryViewSelectedItem = NULL;
QTreeWidgetItem *qmc2VersionViewSelectedItem = NULL;
#elif defined(QMC2_EMUTYPE_MESS)
MiniWebBrowser *qmc2ProjectMESS = NULL;
QTreeWidgetItem *qmc2LastProjectMESSItem = NULL;
QCache<QString, QByteArray> qmc2ProjectMESSCache;
#endif
QTreeWidgetItem *qmc2LastSoftwareNotesItem = NULL;
HtmlEditor *qmc2SoftwareNotesEditor = NULL;
#if defined(QMC2_YOUTUBE_ENABLED)
YouTubeVideoPlayer *qmc2YouTubeWidget = NULL;
QTreeWidgetItem *qmc2LastYouTubeItem = NULL;
QMap <QString, YouTubeVideoInfo> qmc2YouTubeVideoInfoMap;
bool qmc2YouTubeVideoInfoMapChanged = false;
#endif
QTreeWidgetItem *qmc2HierarchySelectedItem = NULL;
QMenu *qmc2EmulatorMenu = NULL,
      *qmc2GameMenu = NULL,
      *qmc2FavoritesMenu = NULL,
      *qmc2PlayedMenu = NULL,
      *qmc2SearchMenu = NULL;
QMap<QString, QTreeWidgetItem *> qmc2GamelistItemMap;
QMap<QString, QTreeWidgetItem *> qmc2HierarchyItemMap;
QMap<QString, QTreeWidgetItem *> qmc2GamelistItemByDescriptionMap;
QMap<QString, QString> qmc2GamelistNameMap;
QMap<QString, QString> qmc2GamelistDescriptionMap;
QMap<QString, QString> qmc2GamelistStatusMap;
QMap<QString, QStringList> qmc2HierarchyMap;
QMap<QString, QString> qmc2ParentMap;
QMap<QString, QIcon> qmc2IconMap;
QMap<QString, QPair<QString, QAction *> > qmc2ShortcutMap;
QMap<QString, QString> qmc2CustomShortcutMap;
QMap<QString, QString> qmc2JoystickFunctionMap;
QMap<QString, QByteArray *> qmc2GameInfoDB;
QString qmc2DemoGame;
QStringList qmc2DemoArgs;
int qmc2SortCriteria = QMC2_SORT_BY_DESCRIPTION;
Qt::SortOrder qmc2SortOrder = Qt::AscendingOrder;
QBitArray qmc2Filter;
unzFile qmc2IconFile = NULL;
QStringList qmc2BiosROMs;
QStringList qmc2DeviceROMs;
KeyPressFilter *qmc2KeyPressFilter = NULL;
QMap<QString, int> qmc2QtKeyMap;
#if QMC2_JOYSTICK == 1
Joystick *qmc2Joystick = NULL;
#endif
QString qmc2DefaultStyle;
QSocketNotifier *qmc2FifoNotifier = NULL;
bool qmc2ShowGameName = false;
bool qmc2ShowGameNameOnlyWhenRequired = true;
QMutex qmc2LogFrontendMutex;
QMutex qmc2LogEmulatorMutex;
QString qmc2FileEditStartPath = "";
QString qmc2DirectoryEditStartPath = "";
QNetworkAccessManager *qmc2NetworkAccessManager = NULL;
int qmc2LastListIndex = 0;
QAbstractItemView::ScrollHint qmc2CursorPositioningMode = QAbstractItemView::PositionAtTop;
QMap<QString, int> qmc2XmlGamePositionMap;
QFont qmc2StartupDefaultFont;
int qmc2SoftwareSnapPosition = 0;
QWidgetList qmc2AutoMinimizedWidgets;
QSplashScreen *qmc2SplashScreen = NULL;
int qmc2DefaultLaunchMode = QMC2_LAUNCH_MODE_INDEPENDENT;

// game status colors 
QColor MainWindow::qmc2StatusColorGreen = QColor("#00cc00");
QColor MainWindow::qmc2StatusColorYellowGreen = QColor("#a2c743");
QColor MainWindow::qmc2StatusColorRed = QColor("#f90000");
QColor MainWindow::qmc2StatusColorBlue = QColor("#0000f9");
QColor MainWindow::qmc2StatusColorGrey = QColor("#7f7f7f");

// extern global variables
extern bool exitArcade;
extern QMap<QString, QStringList> systemSoftwareListMap;
extern QMap<QString, QStringList> systemSoftwareFilterMap;
extern QMap<QString, QString> softwareListXmlDataCache;
#if defined(Q_WS_WIN)
extern QMap<HWND, QString> winWindowMap;
#endif

void MainWindow::log(char logOrigin, QString message)
{
  if ( !qmc2GuiReady )
    return;

  QString timeString = QTime::currentTime().toString("hh:mm:ss.zzz");

  // count subsequent message duplicates
  switch ( logOrigin ) {
    case QMC2_LOG_FRONTEND:
      if ( !qmc2LogFrontendMutex.tryLock(QMC2_LOG_MUTEX_LOCK_TIMEOUT) ) return;
      if ( message == qmc2LastFrontendLogMessage ) {
        qmc2FrontendLogMessageRepeatCount++;
	qmc2LogFrontendMutex.unlock();
        return;
      } else {
        qmc2LastFrontendLogMessage = message;
        if ( qmc2FrontendLogMessageRepeatCount > 0 )
          message = tr("last message repeated %n time(s)", "", qmc2FrontendLogMessageRepeatCount) + "\n" + timeString + ": " + qmc2LastFrontendLogMessage;
        qmc2FrontendLogMessageRepeatCount = 0;
      }
      break;

    case QMC2_LOG_EMULATOR:
      if( !qmc2LogEmulatorMutex.tryLock(QMC2_LOG_MUTEX_LOCK_TIMEOUT) ) return;
      if ( message == qmc2LastEmulatorLogMessage ) {
        qmc2EmulatorLogMessageRepeatCount++;
        qmc2LogEmulatorMutex.unlock();
        return;
      } else {
        qmc2LastEmulatorLogMessage = message;
        if ( qmc2EmulatorLogMessageRepeatCount > 0 )
          message = tr("last message repeated %n time(s)", "", qmc2EmulatorLogMessageRepeatCount) + "\n" + timeString + ": " + qmc2LastEmulatorLogMessage;
        qmc2EmulatorLogMessageRepeatCount = 0;
      }
      break;

    default:
      break;
  }

  QString msg = timeString + ": " + message;

  QString userScopePath = QMC2_DYNAMIC_DOT_PATH;
  QString defaultEmuLogPath, defaultFrontendLogPath;
#if defined(QMC2_SDLMAME)
  defaultFrontendLogPath = userScopePath + "/qmc2-sdlmame.log";
  defaultEmuLogPath = userScopePath + "/mame.log";
#elif defined(QMC2_SDLMESS)
  defaultFrontendLogPath = userScopePath + "/qmc2-sdlmess.log";
  defaultEmuLogPath = userScopePath + "/mess.log";
#elif defined(QMC2_MAME)
  defaultFrontendLogPath = userScopePath + "/qmc2-mame.log";
  defaultEmuLogPath = userScopePath + "/mame.log";
#elif defined(QMC2_MESS)
  defaultFrontendLogPath = userScopePath + "/qmc2-mess.log";
  defaultEmuLogPath = userScopePath + "/mess.log";
#endif

  switch ( logOrigin ) {
    case QMC2_LOG_FRONTEND:
      textBrowserFrontendLog->appendPlainText(msg);
      if ( !qmc2FrontendLogFile )
        if ( (qmc2FrontendLogFile = new QFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/LogFile", defaultFrontendLogPath).toString(), this)) == NULL ) {
          qmc2LogFrontendMutex.unlock();
          return;
        }
      if ( qmc2FrontendLogFile->handle() == -1 ) {
        if ( qmc2FrontendLogFile->open(QIODevice::WriteOnly | QIODevice::Text) )
          qmc2FrontendLogStream.setDevice(qmc2FrontendLogFile);
        else {
          qmc2LogFrontendMutex.unlock();
          return;
        }
      }
      qmc2FrontendLogStream << msg << "\n";
      qmc2FrontendLogStream.flush();
      break;

    case QMC2_LOG_EMULATOR:
      textBrowserEmulatorLog->appendPlainText(msg);
      if ( !qmc2EmulatorLogFile ) {
#if defined(QMC2_EMUTYPE_MAME)
        if ( (qmc2EmulatorLogFile = new QFile(qmc2Config->value("MAME/FilesAndDirectories/LogFile", defaultEmuLogPath).toString(), this)) == NULL ) {
#elif defined(QMC2_EMUTYPE_MESS)
        if ( (qmc2EmulatorLogFile = new QFile(qmc2Config->value("MESS/FilesAndDirectories/LogFile", defaultEmuLogPath).toString(), this)) == NULL ) {
#endif
          qmc2LogEmulatorMutex.unlock();
          return;
        }
      }
      if ( qmc2EmulatorLogFile->handle() == -1 ) {
        if ( qmc2EmulatorLogFile->open(QIODevice::WriteOnly | QIODevice::Text) )
          qmc2EmulatorLogStream.setDevice(qmc2EmulatorLogFile);
        else {
          qmc2LogEmulatorMutex.unlock();
          return;
        }
      }
      qmc2EmulatorLogStream << msg << "\n";
      qmc2EmulatorLogStream.flush();
      break;

    default:
      break;
  }

  qmc2LogFrontendMutex.unlock();
  qmc2LogEmulatorMutex.unlock();
}

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::MainWindow(QWidget *parent = %1)").arg((qulonglong)parent));
#endif

  qmc2Config->setValue(QString(QMC2_FRONTEND_PREFIX + "InstanceRunning"), true);
 
  qmc2StartupDefaultFont = qApp->font();
  desktopGeometry = qApp->desktop()->geometry();
  isActiveState = false;

  // remember the default style
  qmc2DefaultStyle = QApplication::style()->objectName();

  // initial setup of the application style
  QString myStyle = qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Style", tr("Default")).toString();
  setupStyle(myStyle);

  setupUi(this);

  // toolbar's search combo-box
  comboBoxToolbarSearch = new QComboBox(this);
  comboBoxToolbarSearch->setEditable(true);
  comboBoxToolbarSearch->lineEdit()->setPlaceholderText(tr("Enter search string"));
#if defined(QMC2_EMUTYPE_MAME)
  comboBoxToolbarSearch->setToolTip(tr("Search for games (not case-sensitive)"));
  comboBoxToolbarSearch->setStatusTip(tr("Search for games"));
#elif defined(QMC2_EMUTYPE_MESS)
  comboBoxToolbarSearch->setToolTip(tr("Search for machines (not case-sensitive)"));
  comboBoxToolbarSearch->setStatusTip(tr("Search for machines"));
#endif
  comboBoxToolbarSearch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
  widgetActionToolbarSearch = new QWidgetAction(this);
  widgetActionToolbarSearch->setDefaultWidget(comboBoxToolbarSearch);
  toolbar->addSeparator();
  toolbar->addAction(widgetActionToolbarSearch);
  connect(comboBoxToolbarSearch, SIGNAL(activated(const QString &)), this, SLOT(comboBoxToolbarSearch_activated(const QString &)));
  connect(comboBoxToolbarSearch, SIGNAL(editTextChanged(const QString &)), this, SLOT(comboBoxToolbarSearch_textChanged(const QString &)));

#if defined(QMC2_EMUTYPE_MAME)
  tabWidgetSoftwareDetail->removeTab(tabWidgetSoftwareDetail->indexOf(tabProjectMESS));
#endif

  // save splitter widgets at index 0 for later comparison
  hSplitterWidget0 = hSplitter->widget(0);
  vSplitterWidget0 = vSplitter->widget(0);

  // updated later when all initialization is finished
  setUpdatesEnabled(false);

  // disable the menu-bar's default context menu (may be irritating)
  menuBar()->setContextMenuPolicy(Qt::PreventContextMenu);

  // hide "loading game list" labels initially
  labelLoadingGamelist->setVisible(false);
  labelLoadingHierarchy->setVisible(false);

  // hide memory indicator initially
  progressBarMemory->setVisible(false);

  checkBoxAudioFade->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

  treeWidgetGamelist->setMouseTracking(true);
  treeWidgetHierarchy->setMouseTracking(true);
#if defined(QMC2_EMUTYPE_MAME)
  treeWidgetCategoryView->setMouseTracking(true);
  treeWidgetVersionView->setMouseTracking(true);
#endif

#if defined(Q_WS_WIN)
  actionLaunchQMC2MAME->setText(tr("QMC2 for MAME"));
  actionLaunchQMC2MAME->setToolTip(tr("Launch QMC2 for MAME"));
  actionLaunchQMC2MAME->setStatusTip(tr("Launch QMC2 for MAME"));
  actionLaunchQMC2MESS->setText(tr("QMC2 for MESS"));
  actionLaunchQMC2MESS->setToolTip(tr("Launch QMC2 for MESS"));
  actionLaunchQMC2MESS->setStatusTip(tr("Launch QMC2 for MESS"));
#endif

  // FIXME: remove this WIP clause when arcade mode is ready (probably never ;)?)
#if !defined(QMC2_WIP_ENABLED)
  menu_Display->removeAction(menuArcade->menuAction());
  actionArcadeToggle->setVisible(false);
#endif

  labelGameStatus->setVisible(false);
  labelGameStatus->setPalette(qmc2StatusColorBlue);

#if defined(Q_WS_X11) || defined(Q_WS_WIN)
  embedderCornerWidget = new QWidget(tabWidgetEmbeddedEmulators);
  embedderCornerLayout = new QHBoxLayout(embedderCornerWidget);
  embedderCornerLayout->setContentsMargins(0, 0, 0, 0);

#if defined(Q_WS_X11)
  toolButtonEmbedderAutoPause = new QToolButton(embedderCornerWidget);
  toolButtonEmbedderAutoPause->setIcon(QIcon(QString::fromUtf8(":/data/img/sleep.png")));
  toolButtonEmbedderAutoPause->setToolTip(tr("Toggle automatic pausing of embedded emulators (hold down for menu)"));
  toolButtonEmbedderAutoPause->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
  toolButtonEmbedderAutoPause->setToolButtonStyle(Qt::ToolButtonIconOnly);
  toolButtonEmbedderAutoPause->setAutoRaise(true);
  toolButtonEmbedderAutoPause->setCheckable(true);
  toolButtonEmbedderAutoPause->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/Embedder/AutoPause", false).toBool());
  embedderCornerLayout->addWidget(toolButtonEmbedderAutoPause);

  menuAutoPause = new QMenu(0);
  QString apMenuString = tr("Scan the pause key used by the emulator");
  QAction *apMenuAction = menuAutoPause->addAction(tr("Scan pause key..."));
  apMenuAction->setIcon(QIcon(QString::fromUtf8(":/data/img/keyboard.png")));
  apMenuAction->setToolTip(apMenuString); apMenuAction->setStatusTip(apMenuString);
  connect(apMenuAction, SIGNAL(triggered()), this, SLOT(action_embedderScanPauseKey_triggered()));
  toolButtonEmbedderAutoPause->setMenu(menuAutoPause);
#endif

  toolButtonEmbedderMaximizeToggle = new QToolButton(embedderCornerWidget);
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/Embedder/Maximize", false).toBool() )
    toolButtonEmbedderMaximizeToggle->setIcon(QIcon(QString::fromUtf8(":/data/img/minimize.png")));
  else
    toolButtonEmbedderMaximizeToggle->setIcon(QIcon(QString::fromUtf8(":/data/img/maximize.png")));
  toolButtonEmbedderMaximizeToggle->setToolTip(tr("Toggle maximization of embedded emulator windows"));
  toolButtonEmbedderMaximizeToggle->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
  toolButtonEmbedderMaximizeToggle->setToolButtonStyle(Qt::ToolButtonIconOnly);
  toolButtonEmbedderMaximizeToggle->setAutoRaise(true);
  toolButtonEmbedderMaximizeToggle->setCheckable(true);
  toolButtonEmbedderMaximizeToggle->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/Embedder/Maximize", false).toBool());
  connect(toolButtonEmbedderMaximizeToggle, SIGNAL(toggled(bool)), this, SLOT(on_toolButtonEmbedderMaximizeToggle_toggled(bool)));
  embedderCornerLayout->addWidget(toolButtonEmbedderMaximizeToggle);

  embedderCornerWidget->setLayout(embedderCornerLayout);
  tabWidgetEmbeddedEmulators->setCornerWidget(embedderCornerWidget, Qt::TopRightCorner);

  widgetEmbeddedEmus = tabWidgetGamelist->widget(tabWidgetGamelist->indexOf(tabEmbeddedEmus));
#else
  actionPlayEmbedded->setVisible(false);
  actionPlayEmbeddedTagged->setVisible(false);
#endif
  tabWidgetEmbeddedEmulators->removeTab(0);
  tabWidgetGamelist->removeTab(tabWidgetGamelist->indexOf(tabEmbeddedEmus));

#if !defined(QMC2_VARIANT_LAUNCHER)
  actionLaunchQMC2MAME->setVisible(false);
  actionLaunchQMC2MESS->setVisible(false);
#endif

#if !defined(QMC2_YOUTUBE_ENABLED)
  actionClearYouTubeCache->setVisible(false);
#endif

#if defined(QMC2_EMUTYPE_MESS)
  qmc2ProjectMESSCache.setMaxCost(QMC2_PROJECT_MESS_CACHE_SIZE);
#endif

  floatToggleButtonSoftwareDetail = new QToolButton(tabWidgetSoftwareDetail);
  floatToggleButtonSoftwareDetail->setCheckable(true);
  floatToggleButtonSoftwareDetail->setToolTip(tr("Dock / undock this widget"));
  floatToggleButtonSoftwareDetail->setIcon(QIcon(QString::fromUtf8(":/data/img/dock.png")));
  tabWidgetSoftwareDetail->setCornerWidget(floatToggleButtonSoftwareDetail, Qt::TopRightCorner);

#if defined(QMC2_EMUTYPE_MAME)
  actionLaunchQMC2MAME->setVisible(false);
  qmc2MAWSCache.setMaxCost(QMC2_MAWS_CACHE_SIZE);
#if defined(Q_WS_WIN)
  treeWidgetEmulators->headerItem()->setText(QMC2_EMUCONTROL_COLUMN_GAME, tr("Game"));
#endif
#elif defined(QMC2_EMUTYPE_MESS)
  qmc2MAWSCache.setMaxCost(QMC2_MESS_WIKI_CACHE_SIZE);
  actionLaunchQMC2MESS->setVisible(false);
  actionDemoMode->setVisible(false);
  setWindowTitle(tr("M.E.S.S. Catalog / Launcher II"));
  treeWidgetGamelist->headerItem()->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Machine / Attribute"));
  treeWidgetHierarchy->headerItem()->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Machine / Clones"));
  tabWidgetGamelist->setTabText(QMC2_GAMELIST_INDEX, tr("M&achine list"));
  tabWidgetGameDetail->setTabText(QMC2_GAMEINFO_INDEX, tr("Machine &info"));
  textBrowserGameInfo->setToolTip(tr("Detailed machine info"));
  textBrowserGameInfo->setStatusTip(tr("Detailed machine info"));
  listWidgetFavorites->setToolTip(tr("Favorite machines"));
  listWidgetFavorites->setStatusTip(tr("Favorite machines"));
  listWidgetPlayed->setToolTip(tr("Machines last played"));
  listWidgetPlayed->setStatusTip(tr("Machines last played"));
  actionPlay->setToolTip(tr("Play current machine"));
  actionPlay->setStatusTip(tr("Play current machine"));
  actionPlayTagged->setToolTip(tr("Play all tagged machines"));
  actionPlayTagged->setStatusTip(tr("Play all tagged machines"));
  actionClearGamelistCache->setText(tr("Clear machine list cache"));
  actionClearGamelistCache->setIconText(tr("Clear machine list cache"));
  actionClearGamelistCache->setToolTip(tr("Forcedly clear (remove) the machine list cache"));
  actionClearGamelistCache->setStatusTip(tr("Forcedly clear (remove) the machine list cache"));
  treeWidgetGamelist->setStatusTip(tr("List of all supported machines"));
  actionClearMAWSCache->setText(tr("Clear MESS wiki cache"));
  actionClearMAWSCache->setIconText(tr("Clear MESS wiki cache"));
  actionClearMAWSCache->setToolTip(tr("Clear MESS wiki cache"));
  actionClearMAWSCache->setStatusTip(tr("Clear MESS wiki cache"));
#if defined(Q_WS_X11) || defined(Q_WS_WIN)
  actionPlayEmbedded->setToolTip(tr("Play current machine (embedded)"));
  actionPlayEmbedded->setStatusTip(tr("Play current machine (embedded)"));
  actionPlayEmbeddedTagged->setToolTip(tr("Play all tagged machines (embedded)"));
  actionPlayEmbeddedTagged->setStatusTip(tr("Play all tagged machines (embedded)"));
#endif
#if defined(Q_WS_WIN)
  treeWidgetEmulators->headerItem()->setText(QMC2_EMUCONTROL_COLUMN_MACHINE, tr("Machine"));
#else
  treeWidgetEmulators->headerItem()->setText(QMC2_EMUCONTROL_COLUMN_MACHINE, tr("Machine / Notifier"));
#endif
  actionToFavorites->setToolTip(tr("Add current machine to favorites"));
  actionToFavorites->setStatusTip(tr("Add current machine to favorites"));
  actionToFavoritesTagged->setToolTip(tr("Add all tagged machines to favorites"));
  actionToFavoritesTagged->setStatusTip(tr("Add all tagged machines to favorites"));
  actionReload->setToolTip(tr("Reload entire machine list"));
  actionReload->setStatusTip(tr("Reload entire machine list"));
  actionViewFullDetail->setToolTip(tr("View machine list with full detail"));
  actionViewFullDetail->setStatusTip(tr("View machine list with full detail"));
  actionViewByCategory->setEnabled(false);
  actionViewByCategory->setVisible(false);
  actionViewByVersion->setEnabled(false);
  actionViewByVersion->setVisible(false);
  actionCheckCurrentROM->setToolTip(tr("Check current machine's ROM state"));
  actionCheckCurrentROM->setStatusTip(tr("Check current machine's ROM state"));
  actionAnalyseCurrentROM->setToolTip(tr("Analyse current machine with ROMAlyzer"));
  actionAnalyseCurrentROM->setStatusTip(tr("Analyse current machine with ROMAlyzer"));
  menu_Game->setTitle(tr("M&achine"));
  comboBoxViewSelect->setItemText(QMC2_VIEW_DETAIL_INDEX, tr("Machine list with full detail (filtered)"));
  comboBoxViewSelect->setToolTip(tr("Select between detailed machine list and parent / clone hierarchy"));
  comboBoxViewSelect->setStatusTip(tr("Select between detailed machine list and parent / clone hierarchy"));
  labelGameStatus->setToolTip(tr("Machine status indicator"));
  labelGameStatus->setStatusTip(tr("Machine status indicator"));
  actionCheckSamples->setEnabled(false);
  actionCheckSamples->setVisible(false);
  progressBarGamelist->setToolTip(tr("Progress indicator for machine list processing"));
  progressBarGamelist->setStatusTip(tr("Progress indicator for machine list processing"));
  qmc2Options->checkBoxGameStatusIndicator->setText(tr("Machine status indicator"));
  qmc2Options->checkBoxGameStatusIndicator->setToolTip(tr("Show vertical machine status indicator in machine details"));
  qmc2Options->checkBoxGameStatusIndicatorOnlyWhenRequired->setToolTip(tr("Show the machine status indicator only when the machine list is not visible due to the current layout"));
  qmc2Options->checkBoxShowGameName->setText(tr("Show machine name"));
  qmc2Options->checkBoxShowGameName->setToolTip(tr("Show machine's description at the bottom of any images"));
  qmc2Options->checkBoxShowGameNameOnlyWhenRequired->setToolTip(tr("Show machine's description only when the machine list is not visible due to the current layout"));
  labelLoadingGamelist->setText(tr("Loading machine list, please wait..."));
  labelLoadingHierarchy->setText(tr("Loading machine list, please wait..."));
  comboBoxSearch->setToolTip(tr("Search for machines (not case-sensitive)"));
  comboBoxSearch->setStatusTip(tr("Search for machines"));
#endif

#if defined(QMC2_EMUTYPE_MAME)
  qmc2Options->checkBoxShowGameName->setText(tr("Show game/software titles"));
  qmc2Options->checkBoxShowGameName->setToolTip(tr("Show game- or software-titles at the bottom of any images"));
  qmc2Options->checkBoxShowGameNameOnlyWhenRequired->setToolTip(tr("Show game titles only when the game list is not visible due to the current layout"));
#elif defined(QMC2_EMUTYPE_MESS)
  qmc2Options->checkBoxShowGameName->setText(tr("Show machine/software titles"));
  qmc2Options->checkBoxShowGameName->setToolTip(tr("Show machine- or software-titles at the bottom of any images"));
  qmc2Options->checkBoxShowGameNameOnlyWhenRequired->setToolTip(tr("Show machine titles only when the machine list is not visible due to the current layout"));
#endif

  qmc2Gamelist = new Gamelist(this);
  labelGamelistStatus->setText(qmc2Gamelist->status());

  statusBar()->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Statusbar").toBool());

  // game/machine image widgets
  QHBoxLayout *previewLayout = new QHBoxLayout, 
              *flyerLayout = new QHBoxLayout,
              *cabinetLayout = new QHBoxLayout,
              *controllerLayout = new QHBoxLayout,
              *marqueeLayout = new QHBoxLayout,
              *titleLayout = new QHBoxLayout,
              *pcbLayout = new QHBoxLayout;

  int left, top, right, bottom;
  gridLayout->getContentsMargins(&left, &top, &right, &bottom);

  qmc2Preview = new Preview(tabPreview);
  previewLayout->addWidget(qmc2Preview);
  previewLayout->setContentsMargins(left, top, right, bottom);
  tabPreview->setLayout(previewLayout);
  qmc2Flyer = new Flyer(tabFlyer);
  flyerLayout->addWidget(qmc2Flyer);
  flyerLayout->setContentsMargins(left, top, right, bottom);
  tabFlyer->setLayout(flyerLayout);
  qmc2Cabinet = new Cabinet(tabCabinet);
  cabinetLayout->addWidget(qmc2Cabinet);
  cabinetLayout->setContentsMargins(left, top, right, bottom);
  tabCabinet->setLayout(cabinetLayout);
  qmc2Controller = new Controller(tabController);
  controllerLayout->addWidget(qmc2Controller);
  controllerLayout->setContentsMargins(left, top, right, bottom);
  tabController->setLayout(controllerLayout);
  qmc2Marquee = new Marquee(tabMarquee);
  marqueeLayout->addWidget(qmc2Marquee);
  marqueeLayout->setContentsMargins(left, top, right, bottom);
  tabMarquee->setLayout(marqueeLayout);
  qmc2Title = new Title(tabTitle);
  titleLayout->addWidget(qmc2Title);
  titleLayout->setContentsMargins(left, top, right, bottom);
  tabTitle->setLayout(titleLayout);
  qmc2PCB = new PCB(tabPCB);
  pcbLayout->addWidget(qmc2PCB);
  pcbLayout->setContentsMargins(left, top, right, bottom);
  tabPCB->setLayout(pcbLayout);

  // remove column-width restrictions
  treeWidgetGamelist->header()->setMinimumSectionSize(0);
  treeWidgetHierarchy->header()->setMinimumSectionSize(0);
#if defined(QMC2_EMUTYPE_MAME)
  treeWidgetCategoryView->header()->setMinimumSectionSize(0);
  treeWidgetVersionView->header()->setMinimumSectionSize(0);
#endif

  // restore layout
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/RestoreLayout").toBool() ) {
    log(QMC2_LOG_FRONTEND, tr("restoring main widget layout"));
    menuBar()->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ShowMenuBar", true).toBool());
    QList<int> hSplitterSizes, vSplitterSizes;
    QSize hSplitterSize = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/hSplitter").toSize();
    QSize vSplitterSize = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/vSplitter").toSize();
    bool hSplitterFlipped = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/hSplitterFlipped", false).toBool();
    bool vSplitterFlipped = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/vSplitterFlipped", false).toBool();
    bool hSplitterSwapped = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/hSplitterSwapped", false).toBool();
    bool vSplitterSwapped = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/vSplitterSwapped", false).toBool();
    if ( hSplitterSize.width() > 0 || hSplitterSize.height() > 0 )
      hSplitterSizes << hSplitterSize.width() << hSplitterSize.height();
    else
      hSplitterSizes << 100 << 100;
    if ( vSplitterSize.width() > 0 || vSplitterSize.height() > 0 )
      vSplitterSizes << vSplitterSize.width() << vSplitterSize.height();
    else
      vSplitterSizes << 100 << 100;
    if ( hSplitterSwapped ) hSplitterSizes.swap(0, 1);
    if ( vSplitterSwapped ) vSplitterSizes.swap(0, 1);
    hSplitter->setSizes(hSplitterSizes);
    vSplitter->setSizes(vSplitterSizes);
    if ( hSplitterFlipped ) on_menuHorizontalSplitter_FlipOrientation_activated();
    if ( vSplitterFlipped ) on_menuVerticalSplitter_FlipOrientation_activated();
    if ( hSplitterSwapped ) on_menuHorizontalSplitter_SwapLayouts_activated();
    if ( vSplitterSwapped ) on_menuVerticalSplitter_SwapWidgets_activated();
    tabWidgetGamelist->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/GamelistTab", 0).toInt());
    tabWidgetLogsAndEmulators->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/LogsAndEmulatorsTab", 0).toInt());
    on_tabWidgetLogsAndEmulators_currentChanged(tabWidgetLogsAndEmulators->currentIndex());
    tabWidgetSoftwareDetail->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/SoftwareDetailTab", 0).toInt());
    treeWidgetHierarchy->header()->setDefaultAlignment(Qt::AlignLeft);
    treeWidgetGamelist->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/GamelistHeaderState").toByteArray());
    treeWidgetHierarchy->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/HierarchyHeaderState").toByteArray());
#if defined(QMC2_EMUTYPE_MAME)
    treeWidgetCategoryView->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/CategoryViewHeaderState").toByteArray());
    treeWidgetCategoryView->hideColumn(QMC2_GAMELIST_COLUMN_CATEGORY);
    treeWidgetVersionView->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/VersionViewHeaderState").toByteArray());
    treeWidgetVersionView->hideColumn(QMC2_GAMELIST_COLUMN_VERSION);
#endif
    treeWidgetEmulators->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/EmulatorControlHeaderState").toByteArray());
#if defined(Q_WS_WIN)
    // output notifiers are not supported on Windows
    treeWidgetEmulators->hideColumn(QMC2_EMUCONTROL_COLUMN_STATUS);
    treeWidgetEmulators->hideColumn(QMC2_EMUCONTROL_COLUMN_LED0);
    treeWidgetEmulators->hideColumn(QMC2_EMUCONTROL_COLUMN_LED1);
#endif
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/ImageChecker/Visible").toBool() ) {
      on_actionCheckPreviews_activated();
      qmc2ImageChecker->tabWidgetImageChecker->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ImageChecker/CurrentTab", 0).toInt());
    }
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SampleChecker/Visible").toBool() )
      on_actionCheckSamples_activated();
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/ROMAlyzer/Visible").toBool() )
      on_actionROMAlyzer_activated();
    actionFullscreenToggle->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Fullscreen", false).toBool());
    tabWidgetGamelist->setTabPosition((QTabWidget::TabPosition)qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/Gamelist/TabPosition", QTabWidget::North).toInt());
    tabWidgetGameDetail->setTabPosition((QTabWidget::TabPosition)qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/GameDetail/TabPosition", QTabWidget::North).toInt());
    tabWidgetLogsAndEmulators->setTabPosition((QTabWidget::TabPosition)qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/LogsAndEmulators/TabPosition", QTabWidget::North).toInt());
    tabWidgetSoftwareDetail->setTabPosition((QTabWidget::TabPosition)qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareDetail/TabPosition", QTabWidget::North).toInt());
    floatToggleButtonSoftwareDetail->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/SoftwareDetailDocked", true).toBool());
  } else {
    QList<int> splitterSizes;
    splitterSizes << 100 << 100;
    hSplitter->setSizes(splitterSizes);
    vSplitter->setSizes(splitterSizes);
    floatToggleButtonSoftwareDetail->setChecked(true);
  }

  on_actionFullscreenToggle_activated();

  // context menus
  QAction *action;
  QString s;

  qmc2EmulatorMenu = new QMenu(0);
#if defined(Q_WS_X11) || defined(Q_WS_WIN)
  s = tr("Embed emulator widget");
  action = qmc2EmulatorMenu->addAction(tr("&Embed"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/embed.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(action_embedEmulator_triggered()));
  qmc2EmulatorMenu->addSeparator();
#endif
  s = tr("Terminate selected emulator(s) (sends TERM signal to emulator process(es))");
  action = qmc2EmulatorMenu->addAction(tr("&Terminate"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/terminate.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(action_terminateEmulator_triggered()));
  s = tr("Kill selected emulator(s) (sends KILL signal to emulator process(es))");
  action = qmc2EmulatorMenu->addAction(tr("&Kill"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/kill.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(action_killEmulator_triggered()));
  qmc2EmulatorMenu->addSeparator();
  s = tr("Copy emulator command line to clipboard");
  action = qmc2EmulatorMenu->addAction(tr("&Copy command"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/editcopy.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(action_copyEmulatorCommand_triggered()));

  qmc2GameMenu = new QMenu(0);
#if defined(QMC2_EMUTYPE_MAME)
  s = tr("Play selected game");
#elif defined(QMC2_EMUTYPE_MESS)
  s = tr("Start selected machine");
#endif
  action = qmc2GameMenu->addAction(tr("&Play"));
  contextMenuPlayActions.append(action);
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/launch.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_actionPlay_activated()));
#if defined(Q_WS_X11) || defined(Q_WS_WIN)
#if defined(QMC2_EMUTYPE_MAME)
  s = tr("Play selected game (embedded)");
#elif defined(QMC2_EMUTYPE_MESS)
  s = tr("Start selected machine (embedded)");
#endif
  action = qmc2GameMenu->addAction(tr("Play &embedded"));
  contextMenuPlayActions.append(action);
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/embed.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_actionPlayEmbedded_activated()));
#endif
#if defined(QMC2_EMUTYPE_MAME)
  s = tr("Add current game to favorites");
#elif defined(QMC2_EMUTYPE_MESS)
  s = tr("Add current machine to favorites");
#endif
  action = qmc2GameMenu->addAction(tr("To &favorites"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/favorites.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_actionToFavorites_activated()));
  qmc2GameMenu->addSeparator();
#if defined(QMC2_EMUTYPE_MAME)
  s = tr("Check current game's ROM state");
#elif defined(QMC2_EMUTYPE_MESS)
  s = tr("Check current machine's ROM state");
#endif
  action = qmc2GameMenu->addAction(tr("Check &ROM state"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/rom.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_actionCheckCurrentROM_activated()));
#if defined(QMC2_EMUTYPE_MAME)
  s = tr("Analyse current game's ROM set with ROMAlyzer");
#elif defined(QMC2_EMUTYPE_MESS)
  s = tr("Analyse current machine's ROM set with ROMAlyzer");
#endif
  action = qmc2GameMenu->addAction(tr("&Analyse ROM..."));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/search.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_actionAnalyseCurrentROM_activated()));

  qmc2SearchMenu = new QMenu(0);
#if defined(QMC2_EMUTYPE_MAME)
  s = tr("Play selected game");
#elif defined(QMC2_EMUTYPE_MESS)
  s = tr("Start selected machine");
#endif
  action = qmc2SearchMenu->addAction(tr("&Play"));
  contextMenuPlayActions.append(action);
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/launch.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_actionPlay_activated()));
#if defined(Q_WS_X11) || defined(Q_WS_WIN)
#if defined(QMC2_EMUTYPE_MAME)
  s = tr("Play selected game (embedded)");
#elif defined(QMC2_EMUTYPE_MESS)
  s = tr("Start selected machine (embedded)");
#endif
  action = qmc2SearchMenu->addAction(tr("Play &embedded"));
  contextMenuPlayActions.append(action);
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/embed.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_actionPlayEmbedded_activated()));
#endif
#if defined(QMC2_EMUTYPE_MAME)
  s = tr("Add current game to favorites");
#elif defined(QMC2_EMUTYPE_MESS)
  s = tr("Add current machine to favorites");
#endif
  action = qmc2SearchMenu->addAction(tr("To &favorites"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/favorites.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_actionToFavorites_activated()));
  qmc2SearchMenu->addSeparator();
#if defined(QMC2_EMUTYPE_MAME)
  s = tr("Check current game's ROM state");
#elif defined(QMC2_EMUTYPE_MESS)
  s = tr("Check current machine's ROM state");
#endif
  action = qmc2SearchMenu->addAction(tr("Check &ROM state"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/rom.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_actionCheckCurrentROM_activated()));
#if defined(QMC2_EMUTYPE_MAME)
  s = tr("Analyse current game's ROM set with ROMAlyzer");
#elif defined(QMC2_EMUTYPE_MESS)
  s = tr("Analyse current machine's ROM set with ROMAlyzer");
#endif
  action = qmc2SearchMenu->addAction(tr("&Analyse ROM..."));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/search.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_actionAnalyseCurrentROM_activated()));

  qmc2FavoritesMenu = new QMenu(0);
#if defined(QMC2_EMUTYPE_MAME)
  s = tr("Play selected game");
#elif defined(QMC2_EMUTYPE_MESS)
  s = tr("Start selected machine");
#endif
  action = qmc2FavoritesMenu->addAction(tr("&Play"));
  contextMenuPlayActions.append(action);
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/launch.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_actionPlay_activated()));
#if defined(Q_WS_X11) || defined(Q_WS_WIN)
#if defined(QMC2_EMUTYPE_MAME)
  s = tr("Play selected game (embedded)");
#elif defined(QMC2_EMUTYPE_MESS)
  s = tr("Start selected machine (embedded)");
#endif
  action = qmc2FavoritesMenu->addAction(tr("Play &embedded"));
  contextMenuPlayActions.append(action);
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/embed.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_actionPlayEmbedded_activated()));
#endif
  qmc2FavoritesMenu->addSeparator();
#if defined(QMC2_EMUTYPE_MAME)
  s = tr("Check current game's ROM state");
#elif defined(QMC2_EMUTYPE_MESS)
  s = tr("Check current machine's ROM state");
#endif
  action = qmc2FavoritesMenu->addAction(tr("Check &ROM state"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/rom.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_actionCheckCurrentROM_activated()));
#if defined(QMC2_EMUTYPE_MAME)
  s = tr("Analyse current game's ROM set with ROMAlyzer");
#elif defined(QMC2_EMUTYPE_MESS)
  s = tr("Analyse current machine's ROM set with ROMAlyzer");
#endif
  action = qmc2FavoritesMenu->addAction(tr("&Analyse ROM..."));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/search.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_actionAnalyseCurrentROM_activated()));
  qmc2FavoritesMenu->addSeparator();
  s = tr("Remove from favorites");
  action = qmc2FavoritesMenu->addAction(tr("&Remove"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/editdelete.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(action_removeFromFavorites_triggered()));
  s = tr("Clear all favorites");
  action = qmc2FavoritesMenu->addAction(tr("&Clear"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/kill.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(action_clearAllFavorites_triggered()));
  s = tr("Save favorites now");
  action = qmc2FavoritesMenu->addAction(tr("&Save"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/filesave.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(action_saveFavorites_triggered()));

  qmc2PlayedMenu = new QMenu(0);
#if defined(QMC2_EMUTYPE_MAME)
  s = tr("Play selected game");
#elif defined(QMC2_EMUTYPE_MESS)
  s = tr("Start selected machine");
#endif
  action = qmc2PlayedMenu->addAction(tr("&Play"));
  contextMenuPlayActions.append(action);
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/launch.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_actionPlay_activated()));
#if defined(Q_WS_X11) || defined(Q_WS_WIN)
#if defined(QMC2_EMUTYPE_MAME)
  s = tr("Play selected game (embedded)");
#elif defined(QMC2_EMUTYPE_MESS)
  s = tr("Start selected machine (embedded)");
#endif
  action = qmc2PlayedMenu->addAction(tr("Play &embedded"));
  contextMenuPlayActions.append(action);
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/embed.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_actionPlayEmbedded_activated()));
#endif
#if defined(QMC2_EMUTYPE_MAME)
  s = tr("Add current game to favorites");
#elif defined(QMC2_EMUTYPE_MESS)
  s = tr("Add current machine to favorites");
#endif
  action = qmc2PlayedMenu->addAction(tr("To &favorites"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/favorites.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_actionToFavorites_activated()));
  qmc2PlayedMenu->addSeparator();
#if defined(QMC2_EMUTYPE_MAME)
  s = tr("Check current game's ROM state");
#elif defined(QMC2_EMUTYPE_MESS)
  s = tr("Check current machine's ROM state");
#endif
  action = qmc2PlayedMenu->addAction(tr("Check &ROM state"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/rom.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_actionCheckCurrentROM_activated()));
#if defined(QMC2_EMUTYPE_MAME)
  s = tr("Analyse current game's ROM set with ROMAlyzer");
#elif defined(QMC2_EMUTYPE_MESS)
  s = tr("Analyse current machine's ROM set with ROMAlyzer");
#endif
  action = qmc2PlayedMenu->addAction(tr("&Analyse ROM..."));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/search.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_actionAnalyseCurrentROM_activated()));
  qmc2PlayedMenu->addSeparator();
  s = tr("Remove from played");
  action = qmc2PlayedMenu->addAction(tr("&Remove"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/editdelete.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(action_removeFromPlayed_triggered()));
  s = tr("Clear all played");
  action = qmc2PlayedMenu->addAction(tr("&Clear"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/kill.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(action_clearAllPlayed_triggered()));
  s = tr("Save play-history now");
  action = qmc2PlayedMenu->addAction(tr("&Save"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/filesave.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(action_savePlayed_triggered()));

  // tab widget position menus
  menuTabWidgetGamelist = new QMenu(0);
  s = tr("Set tab position north");
  action = menuTabWidgetGamelist->addAction(tr("&North"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/north.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_menuTabWidgetGamelist_North_activated()));
  s = tr("Set tab position south");
  action = menuTabWidgetGamelist->addAction(tr("&South"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/south.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_menuTabWidgetGamelist_South_activated()));
  s = tr("Set tab position west");
  action = menuTabWidgetGamelist->addAction(tr("&West"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/west.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_menuTabWidgetGamelist_West_activated()));
  s = tr("Set tab position east");
  action = menuTabWidgetGamelist->addAction(tr("&East"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/east.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_menuTabWidgetGamelist_East_activated()));

  menuTabWidgetGameDetail = new QMenu(0);
  s = tr("Set tab position north");
  action = menuTabWidgetGameDetail->addAction(tr("&North"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/north.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_menuTabWidgetGameDetail_North_activated()));
  s = tr("Set tab position south");
  action = menuTabWidgetGameDetail->addAction(tr("&South"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/south.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_menuTabWidgetGameDetail_South_activated()));
  s = tr("Set tab position west");
  action = menuTabWidgetGameDetail->addAction(tr("&West"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/west.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_menuTabWidgetGameDetail_West_activated()));
  s = tr("Set tab position east");
  action = menuTabWidgetGameDetail->addAction(tr("&East"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/east.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_menuTabWidgetGameDetail_East_activated()));
  menuTabWidgetGameDetail->addSeparator();
  s = tr("Detail setup");
  action = menuTabWidgetGameDetail->addAction(tr("&Setup..."));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/work.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_menuTabWidgetGameDetail_Setup_activated()));

  menuTabWidgetLogsAndEmulators = new QMenu(0);
  s = tr("Set tab position north");
  action = menuTabWidgetLogsAndEmulators->addAction(tr("&North"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/north.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_menuTabWidgetLogsAndEmulators_North_activated()));
  s = tr("Set tab position south");
  action = menuTabWidgetLogsAndEmulators->addAction(tr("&South"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/south.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_menuTabWidgetLogsAndEmulators_South_activated()));
  s = tr("Set tab position west");
  action = menuTabWidgetLogsAndEmulators->addAction(tr("&West"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/west.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_menuTabWidgetLogsAndEmulators_West_activated()));
  s = tr("Set tab position east");
  action = menuTabWidgetLogsAndEmulators->addAction(tr("&East"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/east.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_menuTabWidgetLogsAndEmulators_East_activated()));

  menuTabWidgetSoftwareDetail = new QMenu(0);
  s = tr("Set tab position north");
  action = menuTabWidgetSoftwareDetail->addAction(tr("&North"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/north.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_menuTabWidgetSoftwareDetail_North_activated()));
  s = tr("Set tab position south");
  action = menuTabWidgetSoftwareDetail->addAction(tr("&South"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/south.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_menuTabWidgetSoftwareDetail_South_activated()));
  s = tr("Set tab position west");
  action = menuTabWidgetSoftwareDetail->addAction(tr("&West"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/west.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_menuTabWidgetSoftwareDetail_West_activated()));
  s = tr("Set tab position east");
  action = menuTabWidgetSoftwareDetail->addAction(tr("&East"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/east.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_menuTabWidgetSoftwareDetail_East_activated()));

  // splitter context menus
  menuHorizontalSplitter = new QMenu(0);
  s = tr("Flip splitter orientation");
  action = menuHorizontalSplitter->addAction(tr("&Flip splitter orientation"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/flip.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_menuHorizontalSplitter_FlipOrientation_activated()));
  s = tr("Swap splitter's sub-layouts");
  action = menuHorizontalSplitter->addAction(tr("&Swap splitter's sub-layouts"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/swap.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_menuHorizontalSplitter_SwapLayouts_activated()));

  menuVerticalSplitter = new QMenu(0);
  s = tr("Flip splitter orientation");
  action = menuVerticalSplitter->addAction(tr("&Flip splitter orientation"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/flip.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_menuVerticalSplitter_FlipOrientation_activated()));
  s = tr("Swap splitter's sub-widgets");
  action = menuVerticalSplitter->addAction(tr("&Swap splitter's sub-widgets"));
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/swap.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(on_menuVerticalSplitter_SwapWidgets_activated()));

  QHeaderView *header;

  menuGamelistHeader = new QMenu(0);
  header = treeWidgetGamelist->header();
#if defined(QMC2_EMUTYPE_MAME)
  action = menuGamelistHeader->addAction(tr("Game / Attribute"), this, SLOT(actionGamelistHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_GAME);
  action->setChecked(!treeWidgetGamelist->isColumnHidden(QMC2_GAMELIST_COLUMN_GAME));
  action = menuGamelistHeader->addAction(tr("Tag"), this, SLOT(actionGamelistHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_TAG);
  action->setChecked(!treeWidgetGamelist->isColumnHidden(QMC2_GAMELIST_COLUMN_TAG));
  action = menuGamelistHeader->addAction(tr("Icon / Value"), this, SLOT(actionGamelistHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_ICON);
  action->setChecked(!treeWidgetGamelist->isColumnHidden(QMC2_GAMELIST_COLUMN_ICON));
#elif defined(QMC2_EMUTYPE_MESS)
  action = menuGamelistHeader->addAction(tr("Machine / Attribute"), this, SLOT(actionGamelistHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_GAME);
  action->setChecked(!treeWidgetGamelist->isColumnHidden(QMC2_MACHINELIST_COLUMN_GAME));
  action = menuGamelistHeader->addAction(tr("Tag"), this, SLOT(actionGamelistHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_TAG);
  action->setChecked(!treeWidgetGamelist->isColumnHidden(QMC2_MACHINELIST_COLUMN_TAG));
  action = menuGamelistHeader->addAction(tr("Icon / Value"), this, SLOT(actionGamelistHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_ICON);
  action->setChecked(!treeWidgetGamelist->isColumnHidden(QMC2_MACHINELIST_COLUMN_ICON));
#endif
  action = menuGamelistHeader->addAction(tr("Year"), this, SLOT(actionGamelistHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_YEAR);
  action->setChecked(!treeWidgetGamelist->isColumnHidden(QMC2_GAMELIST_COLUMN_YEAR));
  action = menuGamelistHeader->addAction(tr("Manufacturer"), this, SLOT(actionGamelistHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_MANU);
  action->setChecked(!treeWidgetGamelist->isColumnHidden(QMC2_GAMELIST_COLUMN_MANU));
  action = menuGamelistHeader->addAction(tr("Name"), this, SLOT(actionGamelistHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_NAME);
  action->setChecked(!treeWidgetGamelist->isColumnHidden(QMC2_GAMELIST_COLUMN_NAME));
  action = menuGamelistHeader->addAction(tr("ROM types"), this, SLOT(actionGamelistHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_RTYPES);
  action->setChecked(!treeWidgetGamelist->isColumnHidden(QMC2_GAMELIST_COLUMN_RTYPES));
  action = menuGamelistHeader->addAction(tr("Players"), this, SLOT(actionGamelistHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_PLAYERS);
  action->setChecked(!treeWidgetGamelist->isColumnHidden(QMC2_GAMELIST_COLUMN_PLAYERS));
  action = menuGamelistHeader->addAction(tr("Driver status"), this, SLOT(actionGamelistHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_DRVSTAT);
  action->setChecked(!treeWidgetGamelist->isColumnHidden(QMC2_GAMELIST_COLUMN_DRVSTAT));
#if defined(QMC2_EMUTYPE_MAME)
  action = menuGamelistHeader->addAction(tr("Category"), this, SLOT(actionGamelistHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_CATEGORY);
  action->setChecked(!treeWidgetGamelist->isColumnHidden(QMC2_GAMELIST_COLUMN_CATEGORY));
  actionMenuGamelistHeaderCategory = action;
  action = menuGamelistHeader->addAction(tr("Version"), this, SLOT(actionGamelistHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_VERSION);
  action->setChecked(!treeWidgetGamelist->isColumnHidden(QMC2_GAMELIST_COLUMN_VERSION));
  actionMenuGamelistHeaderVersion = action;
#endif
  header->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(header, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(treeWidgetGamelistHeader_customContextMenuRequested(const QPoint &)));

  menuHierarchyHeader = new QMenu(0);
  header = treeWidgetHierarchy->header();
#if defined(QMC2_EMUTYPE_MAME)
  action = menuHierarchyHeader->addAction(tr("Game / Clones"), this, SLOT(actionHierarchyHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_GAME);
  action->setChecked(!treeWidgetHierarchy->isColumnHidden(QMC2_GAMELIST_COLUMN_GAME));
  action = menuHierarchyHeader->addAction(tr("Tag"), this, SLOT(actionHierarchyHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_TAG);
  action->setChecked(!treeWidgetHierarchy->isColumnHidden(QMC2_GAMELIST_COLUMN_TAG));
  action = menuHierarchyHeader->addAction(tr("Icon"), this, SLOT(actionHierarchyHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_ICON);
  action->setChecked(!treeWidgetHierarchy->isColumnHidden(QMC2_GAMELIST_COLUMN_ICON));
#elif defined(QMC2_EMUTYPE_MESS)
  action = menuHierarchyHeader->addAction(tr("Machine / Clones"), this, SLOT(actionHierarchyHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_GAME);
  action->setChecked(!treeWidgetHierarchy->isColumnHidden(QMC2_MACHINELIST_COLUMN_GAME));
  action = menuHierarchyHeader->addAction(tr("Tag"), this, SLOT(actionHierarchyHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_TAG);
  action->setChecked(!treeWidgetHierarchy->isColumnHidden(QMC2_MACHINELIST_COLUMN_TAG));
  action = menuHierarchyHeader->addAction(tr("Icon"), this, SLOT(actionHierarchyHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_ICON);
  action->setChecked(!treeWidgetHierarchy->isColumnHidden(QMC2_MACHINELIST_COLUMN_ICON));
#endif
  action = menuHierarchyHeader->addAction(tr("Year"), this, SLOT(actionHierarchyHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_YEAR);
  action->setChecked(!treeWidgetHierarchy->isColumnHidden(QMC2_GAMELIST_COLUMN_YEAR));
  action = menuHierarchyHeader->addAction(tr("Manufacturer"), this, SLOT(actionHierarchyHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_MANU);
  action->setChecked(!treeWidgetHierarchy->isColumnHidden(QMC2_GAMELIST_COLUMN_MANU));
  action = menuHierarchyHeader->addAction(tr("Name"), this, SLOT(actionHierarchyHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_NAME);
  action->setChecked(!treeWidgetHierarchy->isColumnHidden(QMC2_GAMELIST_COLUMN_NAME));
  action = menuHierarchyHeader->addAction(tr("ROM types"), this, SLOT(actionHierarchyHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_RTYPES);
  action->setChecked(!treeWidgetHierarchy->isColumnHidden(QMC2_GAMELIST_COLUMN_RTYPES));
  action = menuHierarchyHeader->addAction(tr("Players"), this, SLOT(actionHierarchyHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_PLAYERS);
  action->setChecked(!treeWidgetHierarchy->isColumnHidden(QMC2_GAMELIST_COLUMN_PLAYERS));
  action = menuHierarchyHeader->addAction(tr("Driver status"), this, SLOT(actionHierarchyHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_DRVSTAT);
  action->setChecked(!treeWidgetHierarchy->isColumnHidden(QMC2_GAMELIST_COLUMN_DRVSTAT));
#if defined(QMC2_EMUTYPE_MAME)
  action = menuHierarchyHeader->addAction(tr("Category"), this, SLOT(actionHierarchyHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_CATEGORY);
  action->setChecked(!treeWidgetHierarchy->isColumnHidden(QMC2_GAMELIST_COLUMN_CATEGORY));
  actionMenuHierarchyHeaderCategory = action;
  action = menuHierarchyHeader->addAction(tr("Version"), this, SLOT(actionHierarchyHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_VERSION);
  action->setChecked(!treeWidgetHierarchy->isColumnHidden(QMC2_GAMELIST_COLUMN_VERSION));
  actionMenuHierarchyHeaderVersion = action;
#endif
  header->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(header, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(treeWidgetHierarchyHeader_customContextMenuRequested(const QPoint &)));

#if defined(QMC2_EMUTYPE_MAME)
  menuCategoryHeader = new QMenu(0);
  header = treeWidgetCategoryView->header();
  action = menuCategoryHeader->addAction(tr("Category / Game"), this, SLOT(actionCategoryHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_GAME);
  action->setChecked(!treeWidgetCategoryView->isColumnHidden(QMC2_GAMELIST_COLUMN_GAME));
  action = menuCategoryHeader->addAction(tr("Tag"), this, SLOT(actionCategoryHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_TAG);
  action->setChecked(!treeWidgetCategoryView->isColumnHidden(QMC2_GAMELIST_COLUMN_TAG));
  action = menuCategoryHeader->addAction(tr("Icon"), this, SLOT(actionCategoryHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_ICON);
  action->setChecked(!treeWidgetCategoryView->isColumnHidden(QMC2_GAMELIST_COLUMN_ICON));
  action = menuCategoryHeader->addAction(tr("Year"), this, SLOT(actionCategoryHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_YEAR);
  action->setChecked(!treeWidgetCategoryView->isColumnHidden(QMC2_GAMELIST_COLUMN_YEAR));
  action = menuCategoryHeader->addAction(tr("Manufacturer"), this, SLOT(actionCategoryHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_MANU);
  action->setChecked(!treeWidgetCategoryView->isColumnHidden(QMC2_GAMELIST_COLUMN_MANU));
  action = menuCategoryHeader->addAction(tr("Name"), this, SLOT(actionCategoryHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_NAME);
  action->setChecked(!treeWidgetCategoryView->isColumnHidden(QMC2_GAMELIST_COLUMN_NAME));
  action = menuCategoryHeader->addAction(tr("ROM types"), this, SLOT(actionCategoryHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_RTYPES);
  action->setChecked(!treeWidgetCategoryView->isColumnHidden(QMC2_GAMELIST_COLUMN_RTYPES));
  action = menuCategoryHeader->addAction(tr("Players"), this, SLOT(actionCategoryHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_PLAYERS);
  action->setChecked(!treeWidgetCategoryView->isColumnHidden(QMC2_GAMELIST_COLUMN_PLAYERS));
  action = menuCategoryHeader->addAction(tr("Driver status"), this, SLOT(actionCategoryHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_DRVSTAT);
  action->setChecked(!treeWidgetCategoryView->isColumnHidden(QMC2_GAMELIST_COLUMN_DRVSTAT));
  action = menuCategoryHeader->addAction(tr("Version"), this, SLOT(actionCategoryHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_VERSION);
  action->setChecked(!treeWidgetCategoryView->isColumnHidden(QMC2_GAMELIST_COLUMN_VERSION));
  header->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(header, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(treeWidgetCategoryViewHeader_customContextMenuRequested(const QPoint &)));

  menuVersionHeader = new QMenu(0);
  header = treeWidgetVersionView->header();
  action = menuVersionHeader->addAction(tr("Version / Game"), this, SLOT(actionVersionHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_GAME);
  action->setChecked(!treeWidgetVersionView->isColumnHidden(QMC2_GAMELIST_COLUMN_GAME));
  action = menuVersionHeader->addAction(tr("Tag"), this, SLOT(actionVersionHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_TAG);
  action->setChecked(!treeWidgetVersionView->isColumnHidden(QMC2_GAMELIST_COLUMN_TAG));
  action = menuVersionHeader->addAction(tr("Icon"), this, SLOT(actionVersionHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_ICON);
  action->setChecked(!treeWidgetVersionView->isColumnHidden(QMC2_GAMELIST_COLUMN_ICON));
  action = menuVersionHeader->addAction(tr("Year"), this, SLOT(actionVersionHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_YEAR);
  action->setChecked(!treeWidgetVersionView->isColumnHidden(QMC2_GAMELIST_COLUMN_YEAR));
  action = menuVersionHeader->addAction(tr("Manufacturer"), this, SLOT(actionVersionHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_MANU);
  action->setChecked(!treeWidgetVersionView->isColumnHidden(QMC2_GAMELIST_COLUMN_MANU));
  action = menuVersionHeader->addAction(tr("Name"), this, SLOT(actionVersionHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_NAME);
  action->setChecked(!treeWidgetVersionView->isColumnHidden(QMC2_GAMELIST_COLUMN_NAME));
  action = menuVersionHeader->addAction(tr("ROM types"), this, SLOT(actionVersionHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_RTYPES);
  action->setChecked(!treeWidgetVersionView->isColumnHidden(QMC2_GAMELIST_COLUMN_RTYPES));
  action = menuVersionHeader->addAction(tr("Players"), this, SLOT(actionVersionHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_PLAYERS);
  action->setChecked(!treeWidgetVersionView->isColumnHidden(QMC2_GAMELIST_COLUMN_PLAYERS));
  action = menuVersionHeader->addAction(tr("Driver status"), this, SLOT(actionVersionHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_DRVSTAT);
  action->setChecked(!treeWidgetVersionView->isColumnHidden(QMC2_GAMELIST_COLUMN_DRVSTAT));
  action = menuVersionHeader->addAction(tr("Category"), this, SLOT(actionVersionHeader_triggered())); action->setCheckable(true); action->setData(QMC2_GAMELIST_COLUMN_CATEGORY);
  action->setChecked(!treeWidgetVersionView->isColumnHidden(QMC2_GAMELIST_COLUMN_CATEGORY));
  header->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(header, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(treeWidgetVersionViewHeader_customContextMenuRequested(const QPoint &)));
#endif

  // other actions
  connect(actionViewFullDetail, SIGNAL(triggered()), this, SLOT(viewFullDetail()));
  connect(actionViewParentClones, SIGNAL(triggered()), this, SLOT(viewParentClones()));
#if defined(QMC2_EMUTYPE_MAME)
  connect(actionViewByCategory, SIGNAL(triggered()), this, SLOT(viewByCategory()));
  connect(actionViewByVersion, SIGNAL(triggered()), this, SLOT(viewByVersion()));
#endif
  connect(comboBoxViewSelect, SIGNAL(currentIndexChanged(int)), stackedWidgetView, SLOT(setCurrentIndex(int)));
  connect(&searchTimer, SIGNAL(timeout()), this, SLOT(on_comboBoxSearch_editTextChanged_delayed()));
  connect(&updateTimer, SIGNAL(timeout()), this, SLOT(on_treeWidgetGamelist_itemSelectionChanged_delayed()));
  connect(&activityCheckTimer, SIGNAL(timeout()), this, SLOT(checkActivity()));
  activityState = false;

  comboBoxSearch->lineEdit()->setPlaceholderText(tr("Enter search string"));

  // restore toolbar state
  restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/ToolbarState", QByteArray()).toByteArray());
#if defined(Q_WS_MAC)
  setUnifiedTitleAndToolBarOnMac(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/UnifiedTitleAndToolBarOnMac", false).toBool());
#endif

#if QMC2_JOYSTICK == 1
  joyIndex = -1;
#endif

#if !(QMC2_USE_PHONON_API)
  tabWidgetLogsAndEmulators->removeTab(tabWidgetLogsAndEmulators->indexOf(tabAudioPlayer));
  menu_Tools->removeAction(menuAudio_player->menuAction());
#else
  audioState = Phonon::StoppedState;
  phononAudioPlayer = new Phonon::MediaObject(this);
  phononAudioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
  phononAudioPath = Phonon::createPath(phononAudioPlayer, phononAudioOutput);
  listWidgetAudioPlaylist->setTextElideMode(Qt::ElideMiddle);
  listWidgetAudioPlaylist->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  QStringList psl = qmc2Config->value(QMC2_FRONTEND_PREFIX + "AudioPlayer/PlayList").toStringList();
  listWidgetAudioPlaylist->addItems(psl);
  QList<QListWidgetItem *> sl = listWidgetAudioPlaylist->findItems(qmc2Config->value(QMC2_FRONTEND_PREFIX + "AudioPlayer/LastTrack", QString()).toString(), Qt::MatchExactly);
  if ( sl.count() > 0 ) {
    listWidgetAudioPlaylist->setCurrentItem(sl[0]);
    listWidgetAudioPlaylist->scrollToItem(sl[0], qmc2CursorPositioningMode);
    qmc2AudioLastIndividualTrack = sl[0]->text();
  }
  checkBoxAudioPlayOnStart->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "AudioPlayer/PlayOnStart", false).toBool());
  checkBoxAudioShuffle->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "AudioPlayer/Shuffle", false).toBool());
  checkBoxAudioPause->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "AudioPlayer/Pause", true).toBool());
  checkBoxAudioFade->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "AudioPlayer/Fade", true).toBool());
  dialAudioVolume->setValue(qmc2Config->value(QMC2_FRONTEND_PREFIX + "AudioPlayer/Volume", 50).toInt());
  phononAudioOutput->setVolume((qreal)dialAudioVolume->value()/100.0);
  toolButtonAudioPreviousTrack->setDefaultAction(actionAudioPreviousTrack);
  toolButtonAudioNextTrack->setDefaultAction(actionAudioNextTrack);
  toolButtonAudioStopTrack->setDefaultAction(actionAudioStopTrack);
  toolButtonAudioPauseTrack->setDefaultAction(actionAudioPauseTrack);
  toolButtonAudioPlayTrack->setDefaultAction(actionAudioPlayTrack);
  phononAudioPlayer->setTickInterval(1000);
  connect(phononAudioPlayer, SIGNAL(tick(qint64)), this, SLOT(audioTick(qint64)));
  connect(phononAudioPlayer, SIGNAL(totalTimeChanged(qint64)), this, SLOT(audioTotalTimeChanged(qint64)));
  connect(phononAudioPlayer, SIGNAL(finished()), this, SLOT(audioFinished()));
  connect(phononAudioPlayer, SIGNAL(metaDataChanged()), this, SLOT(audioMetaDataChanged()));
  connect(phononAudioPlayer, SIGNAL(bufferStatus(int)), this, SLOT(audioBufferStatus(int)));
  audioFastForwarding = audioFastBackwarding = audioSkippingTracks = false;
  if ( checkBoxAudioPlayOnStart->isChecked() ) {
    audioState = Phonon::PlayingState;
    QTimer::singleShot(0, this, SLOT(on_actionAudioPlayTrack_triggered()));
  } else
    QTimer::singleShot(0, this, SLOT(on_actionAudioStopTrack_triggered()));
#endif

  // download manager widget
  checkBoxRemoveFinishedDownloads->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Downloads/RemoveFinished", false).toBool());

  // setup ROM state filter selector menu & toggle actions / short cuts
  menuRomStatusFilter = new QMenu(pushButtonSelectRomFilter);
  actionRomStatusFilterC = menuRomStatusFilter->addAction(QIcon(QString::fromUtf8(":/data/img/sphere_green.png")), tr("&Correct"));
  actionRomStatusFilterC->setCheckable(true);
  actionRomStatusFilterC->setShortcut(QKeySequence("Ctrl+Alt+C"));
  actionRomStatusFilterC->setShortcutContext(Qt::ApplicationShortcut);
  actionRomStatusFilterC->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowC").toBool());
  connect(actionRomStatusFilterC, SIGNAL(toggled(bool)), this, SLOT(on_romStateFilterC_toggled(bool)));
  actionRomStatusFilterM = menuRomStatusFilter->addAction(QIcon(QString::fromUtf8(":/data/img/sphere_yellowgreen.png")), tr("&Mostly correct"));
  actionRomStatusFilterM->setCheckable(true);
  actionRomStatusFilterM->setShortcut(QKeySequence("Ctrl+Alt+M"));
  actionRomStatusFilterM->setShortcutContext(Qt::ApplicationShortcut);
  actionRomStatusFilterM->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowM").toBool());
  connect(actionRomStatusFilterM, SIGNAL(toggled(bool)), this, SLOT(on_romStateFilterM_toggled(bool)));
  actionRomStatusFilterI = menuRomStatusFilter->addAction(QIcon(QString::fromUtf8(":/data/img/sphere_red.png")), tr("&Incorrect"));
  actionRomStatusFilterI->setCheckable(true);
  actionRomStatusFilterI->setShortcut(QKeySequence("Ctrl+Alt+I"));
  actionRomStatusFilterI->setShortcutContext(Qt::ApplicationShortcut);
  actionRomStatusFilterI->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowI").toBool());
  connect(actionRomStatusFilterI, SIGNAL(toggled(bool)), this, SLOT(on_romStateFilterI_toggled(bool)));
  actionRomStatusFilterN = menuRomStatusFilter->addAction(QIcon(QString::fromUtf8(":/data/img/sphere_grey.png")), tr("&Not found"));
  actionRomStatusFilterN->setCheckable(true);
  actionRomStatusFilterN->setShortcut(QKeySequence("Ctrl+Alt+N"));
  actionRomStatusFilterN->setShortcutContext(Qt::ApplicationShortcut);
  actionRomStatusFilterN->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowN").toBool());
  connect(actionRomStatusFilterN, SIGNAL(toggled(bool)), this, SLOT(on_romStateFilterN_toggled(bool)));
  actionRomStatusFilterU = menuRomStatusFilter->addAction(QIcon(QString::fromUtf8(":/data/img/sphere_blue.png")), tr("&Unknown"));
  actionRomStatusFilterU->setCheckable(true);
  actionRomStatusFilterU->setShortcut(QKeySequence("Ctrl+Alt+U"));
  actionRomStatusFilterU->setShortcutContext(Qt::ApplicationShortcut);
  actionRomStatusFilterU->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowU").toBool());
  connect(actionRomStatusFilterU, SIGNAL(toggled(bool)), this, SLOT(on_romStateFilterU_toggled(bool)));
  pushButtonSelectRomFilter->setMenu(menuRomStatusFilter);

  // initialize ROM state toggles
  qmc2StatesTogglesEnabled = false;
  actionRomStatusFilterC->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_C]);
  actionRomStatusFilterM->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_M]);
  actionRomStatusFilterI->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_I]);
  actionRomStatusFilterN->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_N]);
  actionRomStatusFilterU->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_U]);

  // connect header click signals
  connect(treeWidgetGamelist->header(), SIGNAL(sectionClicked(int)), this, SLOT(on_treeWidgetGamelist_headerSectionClicked(int)));
  connect(treeWidgetHierarchy->header(), SIGNAL(sectionClicked(int)), this, SLOT(on_treeWidgetHierarchy_headerSectionClicked(int)));
  treeWidgetGamelist->header()->setClickable(true);
  treeWidgetHierarchy->header()->setClickable(true);
#if defined(QMC2_EMUTYPE_MAME)
  connect(treeWidgetCategoryView->header(), SIGNAL(sectionClicked(int)), this, SLOT(on_treeWidgetCategoryView_headerSectionClicked(int)));
  connect(treeWidgetVersionView->header(), SIGNAL(sectionClicked(int)), this, SLOT(on_treeWidgetVersionView_headerSectionClicked(int)));
  treeWidgetCategoryView->header()->setClickable(true);
  treeWidgetVersionView->header()->setClickable(true);
#endif

  // connections for dock/undock buttons
  connect(floatToggleButtonSoftwareDetail, SIGNAL(toggled(bool)), this, SLOT(floatToggleButtonSoftwareDetail_toggled(bool)));

  // setup the global network access manager
  qmc2NetworkAccessManager = new QNetworkAccessManager(this);

  // URL replacement regexp
  QString urlChar = QLatin1String("\\+\\-\\w\\./#@&;:=\\?~%_,\\!\\$\\*");
  urlSectionRegExp = QString("[%1]+").arg(urlChar);

  QTimer::singleShot(0, this, SLOT(init()));
}

MainWindow::~MainWindow()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::~MainWindow()");
#endif

}

void MainWindow::on_actionPlayEmbedded_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionPlayEmbedded_activated()");
#endif

  qmc2StartEmbedded = true;
  on_actionPlay_activated();
}

void MainWindow::on_actionPlay_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionPlay_activated()");
#endif

  if ( qmc2EarlyReloadActive )
    return;

#if defined(QMC2_EMUTYPE_MAME)
  if ( !qmc2CurrentItem && qmc2DemoGame.isEmpty() )
#else
  if ( !qmc2CurrentItem )
#endif
    return;

  QString gameName;

#if defined(QMC2_EMUTYPE_MAME)
  if ( !qmc2DemoGame.isEmpty() )
    gameName = qmc2DemoGame;
  else if ( !qmc2CurrentItem )
    return;
  else
    gameName = qmc2CurrentItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON);
#else
  gameName = qmc2CurrentItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON);
#endif

  if ( qmc2DeviceROMs.contains(gameName) ) {
	   log(QMC2_LOG_FRONTEND, tr("sorry, devices cannot run standalone"));
	   return;
  }

#if defined(QMC2_EMUTYPE_MAME)
  if ( qmc2DemoGame.isEmpty() ) {
#endif
    qmc2LastConfigItem = NULL;
    on_tabWidgetGameDetail_currentChanged(qmc2DetailSetup->appliedDetailList.indexOf(QMC2_CONFIG_INDEX));
#if defined(QMC2_EMUTYPE_MAME)
  }
#endif

  // check if a foreign emulator is to be used and if it CAN be used; if yes (both), start it instead of the default emulator...
#if defined(QMC2_EMUTYPE_MAME)
  qmc2Config->beginGroup("MAME/RegisteredEmulators");
#elif defined(QMC2_EMUTYPE_MESS)
  qmc2Config->beginGroup("MESS/RegisteredEmulators");
#endif
  QStringList registeredEmulators = qmc2Config->childGroups();
  qmc2Config->endGroup();
  bool foreignEmulator = false;
#if defined(QMC2_EMUTYPE_MAME)
  if ( registeredEmulators.count() > 0 && qmc2DemoGame.isEmpty() ) {
#else
  if ( registeredEmulators.count() > 0 ) {
#endif
    if ( qmc2Config->contains(qmc2EmulatorOptions->settingsGroup + "/SelectedEmulator") ) {
      QString selectedEmulator = qmc2Config->value(qmc2EmulatorOptions->settingsGroup + "/SelectedEmulator").toString();
      if ( !selectedEmulator.isEmpty() && registeredEmulators.contains(selectedEmulator) ) {
        foreignEmulator = true;
#if defined(QMC2_EMUTYPE_MAME)
        QString emuCommand = qmc2Config->value(QString("MAME/RegisteredEmulators/%1/Executable").arg(selectedEmulator)).toString();
        QString emuWorkDir = qmc2Config->value(QString("MAME/RegisteredEmulators/%1/WorkingDirectory").arg(selectedEmulator)).toString();
        QStringList emuArgs = qmc2Config->value(QString("MAME/RegisteredEmulators/%1/Arguments").arg(selectedEmulator)).toString().replace("$ID$", gameName).split(" ");
#elif defined(QMC2_EMUTYPE_MESS)
        QString emuCommand = qmc2Config->value(QString("MESS/RegisteredEmulators/%1/Executable").arg(selectedEmulator)).toString();
        QString emuWorkDir = qmc2Config->value(QString("MESS/RegisteredEmulators/%1/WorkingDirectory").arg(selectedEmulator)).toString();
        QStringList emuArgs = qmc2Config->value(QString("MESS/RegisteredEmulators/%1/Arguments").arg(selectedEmulator)).toString().replace("$ID$", gameName).split(" ");
#endif
        // start game/machine
        qmc2ProcessManager->process(qmc2ProcessManager->start(emuCommand, emuArgs, true, emuWorkDir));

        qmc2AutoMinimizedWidgets.clear();
        if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/MinimizeOnEmuLaunch", false).toBool() && !qmc2StartEmbedded ) {
          foreach (QWidget *w, qApp->topLevelWidgets()) {
            if ( w->isVisible () ) {
              qmc2AutoMinimizedWidgets << w;
              w->showMinimized();
            }
          }
        }
      }
    }
  }

  if ( foreignEmulator )
    return;

  QStringList args;
  QString sectionTitle;
  EmulatorOptions *emuOptions = qmc2EmulatorOptions;

#if defined(QMC2_EMUTYPE_MAME)
  EmulatorOptions *demoOpts = NULL;
  if ( !qmc2DemoGame.isEmpty() ) {
    demoOpts = new EmulatorOptions("MAME/Configuration/" + gameName, 0);
    demoOpts->hide();
    demoOpts->load();
    emuOptions = demoOpts;
  }
#endif

  foreach (sectionTitle, emuOptions->optionsMap.uniqueKeys()) {
    int i;
    for (i = 0; i < emuOptions->optionsMap[sectionTitle].count(); i++) {
      EmulatorOption option = emuOptions->optionsMap[sectionTitle][i];
#if defined(QMC2_EMUTYPE_MAME)
      QString globalOptionKey = "MAME/Configuration/Global/" + option.name;
#elif defined(QMC2_EMUTYPE_MESS)
      QString globalOptionKey = "MESS/Configuration/Global/" + option.name;
#endif
      switch ( option.type ) {
        case QMC2_EMUOPT_TYPE_INT: {
          int  v = option.value.toInt();
          int dv = option.dvalue.toInt();
          int gv = qmc2Config->value(globalOptionKey, dv).toInt();
          if ( !option.valid )
            v = gv;
          if ( v != dv )
            args << QString("-%1").arg(option.name) << QString("%1").arg(v);
          break;
        }

        case QMC2_EMUOPT_TYPE_FLOAT: {
          double  v = option.value.toDouble();
          double dv = option.dvalue.toDouble();
          double gv = qmc2Config->value(globalOptionKey, dv).toDouble();
          if ( !option.valid )
            v = gv;
          if ( v != dv ) {
            QString val;
	    val.setNum(v, 'f', option.decimals);
            args << QString("-%1").arg(option.name) << val;
	  }
          break;
        }

        case QMC2_EMUOPT_TYPE_BOOL: {
          bool dv = EmulatorOptionDelegate::stringToBool(option.dvalue);
          bool v = EmulatorOptionDelegate::stringToBool(option.value);
          bool gv = qmc2Config->value(globalOptionKey, dv).toBool();
          if ( !option.valid )
            v = gv;
          if ( v != dv ) {
            if ( v )
              args << QString("-%1").arg(option.name);
            else
              args << QString("-no%1").arg(option.name);
          }
          break;
        }

        case QMC2_EMUOPT_TYPE_STRING:
        default: {
          QString  v = option.value;
          QString dv = option.dvalue;
          QString gv = qmc2Config->value(globalOptionKey, dv).toString();
          if ( !option.valid )
            v = gv;
          if ( v != dv )
            args << QString("-%1").arg(option.name) << v.replace("~", "$HOME");
          break;
        }
      }
    }
  }

#if defined(Q_WS_X11) || defined(Q_WS_WIN)
  if ( qmc2StartEmbedded )
    args << "-window" << "-nomaximize" << "-keepaspect" << "-rotate" << "-noror" << "-norol";
#endif

#if defined(QMC2_EMUTYPE_MAME)
  if ( !qmc2DemoGame.isEmpty() )
    args << qmc2DemoArgs;
#endif

  args << gameName;

  qmc2DriverName = gameName;

  if ( qmc2SoftwareList && tabWidgetGameDetail->currentIndex() == qmc2DetailSetup->appliedDetailList.indexOf(QMC2_SOFTWARE_LIST_INDEX) )
    args << qmc2SoftwareList->arguments();
#if defined(QMC2_EMUTYPE_MESS)
  else if ( qmc2MESSDeviceConfigurator && tabWidgetGameDetail->currentIndex() == qmc2DetailSetup->appliedDetailList.indexOf(QMC2_DEVICE_INDEX) ) {
	  switch ( qmc2MESSDeviceConfigurator->tabWidgetDeviceSetup->currentIndex() ) {
		case QMC2_DEVSETUP_TAB_FILECHOOSER: {
				QString instance = qmc2MESSDeviceConfigurator->comboBoxDeviceInstanceChooser->currentText();
				QItemSelectionModel *selectionModel = qmc2MESSDeviceConfigurator->treeViewFileChooser->selectionModel();
				QModelIndexList indexList;
				if ( selectionModel )
					indexList = selectionModel->selectedIndexes();
				if ( indexList.count() > 0 && instance != tr("No devices available") ) {
#if !defined(QMC2_ALTERNATE_FSM)
					QString file = qmc2MESSDeviceConfigurator->fileModel->fileInfo(indexList[0]).absoluteFilePath();
#else
					QString file = qmc2MESSDeviceConfigurator->fileModel->absolutePath(indexList[0]);
#endif

#if defined(Q_WS_WIN)
					args << QString("-%1").arg(instance) << file.replace('/', '\\');
#else
					args << QString("-%1").arg(instance) << file.replace("~", "$HOME");
#endif
					QList<QTreeWidgetItem *> allSlotItems = qmc2MESSDeviceConfigurator->treeWidgetSlotOptions->findItems("*", Qt::MatchWildcard);
					foreach (QTreeWidgetItem *item, allSlotItems) {
						QString slotName = item->data(QMC2_SLOTCONFIG_COLUMN_SLOT, Qt::EditRole).toString();
						if ( !slotName.isEmpty() ) {
							QComboBox *cb = (QComboBox *)qmc2MESSDeviceConfigurator->treeWidgetSlotOptions->itemWidget(item, QMC2_SLOTCONFIG_COLUMN_OPTION);
							if ( cb )
								if ( cb->currentIndex() > 0 )
									args << QString("-%1").arg(slotName) << cb->currentText().split(" ")[0];
						}
					}
				}
			}
			break;

		default: {
				QString configName = qmc2MESSDeviceConfigurator->lineEditConfigurationName->text();
				if ( configName != tr("No devices") ) {
					// make sure the currently edited data is up to date
					qmc2MESSDeviceConfigurator->on_toolButtonSaveConfiguration_clicked();
					if ( qmc2MESSDeviceConfigurator->configurationMap.contains(configName) ) {
						QPair<QStringList, QStringList> valuePair = qmc2MESSDeviceConfigurator->configurationMap[configName];
						int i;
						for (i = 0; i < valuePair.first.count(); i++)
#if defined(Q_WS_WIN)
							args << QString("-%1").arg(valuePair.first[i]) << valuePair.second[i].replace('/', '\\');
#else
							args << QString("-%1").arg(valuePair.first[i]) << valuePair.second[i].replace("~", "$HOME");
#endif
						valuePair = qmc2MESSDeviceConfigurator->slotMap[configName];
						for (i = 0; i < valuePair.first.count(); i++)
							args << QString("-%1").arg(valuePair.first[i]) << valuePair.second[i];
					}
				} else {
					QList<QTreeWidgetItem *> allSlotItems = qmc2MESSDeviceConfigurator->treeWidgetSlotOptions->findItems("*", Qt::MatchWildcard);
					foreach (QTreeWidgetItem *item, allSlotItems) {
						QString slotName = item->data(QMC2_SLOTCONFIG_COLUMN_SLOT, Qt::EditRole).toString();
						if ( !slotName.isEmpty() ) {
							QComboBox *cb = (QComboBox *)qmc2MESSDeviceConfigurator->treeWidgetSlotOptions->itemWidget(item, QMC2_SLOTCONFIG_COLUMN_OPTION);
							if ( cb )
								if ( cb->currentIndex() > 0 )
									args << QString("-%1").arg(slotName) << cb->currentText().split(" ")[0];
						}
					}
				}
			}
			break;
	  }
  }
#endif

#if defined(QMC2_EMUTYPE_MAME)
  QString command = qmc2Config->value("MAME/FilesAndDirectories/ExecutableFile").toString();
  QString workingDirectory = qmc2Config->value("MAME/FilesAndDirectories/WorkingDirectory").toString();
#elif defined(QMC2_EMUTYPE_MESS)
  QString command = qmc2Config->value("MESS/FilesAndDirectories/ExecutableFile").toString();
  QString workingDirectory = qmc2Config->value("MESS/FilesAndDirectories/WorkingDirectory").toString();
#endif

  // start game/machine
  qmc2ProcessManager->process(qmc2ProcessManager->start(command, args, true, workingDirectory));

  qmc2AutoMinimizedWidgets.clear();
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/MinimizeOnEmuLaunch", false).toBool() && !qmc2StartEmbedded ) {
    foreach (QWidget *w, qApp->topLevelWidgets()) {
      if ( w->isVisible () ) {
        qmc2AutoMinimizedWidgets << w;
        w->showMinimized();
      }
    }
  }

#if defined(QMC2_EMUTYPE_MAME)
  if ( qmc2DemoGame.isEmpty() ) {
#endif
    // add game/machine to played list
    listWidgetPlayed->blockSignals(true);
    QList<QListWidgetItem *> matches = listWidgetPlayed->findItems(qmc2GamelistItemMap[gameName]->text(QMC2_GAMELIST_COLUMN_GAME), Qt::MatchExactly);
    QListWidgetItem *playedItem;
    if ( matches.count() > 0 ) {
      playedItem = listWidgetPlayed->takeItem(listWidgetPlayed->row(matches[0]));
    } else {
      playedItem = new QListWidgetItem();
      playedItem->setText(qmc2GamelistItemMap[gameName]->text(QMC2_GAMELIST_COLUMN_GAME));
    }
    listWidgetPlayed->insertItem(0, playedItem);
    listWidgetPlayed->setCurrentItem(playedItem);
    listWidgetPlayed->blockSignals(false);
#if defined(QMC2_EMUTYPE_MAME)
  } else {
    if ( qmc2DemoModeDialog ) {
      QProcess *proc = qmc2ProcessManager->process(qmc2ProcessManager->procCount - 1);
      if ( proc ) {
        connect(proc, SIGNAL(finished(int, QProcess::ExitStatus)), qmc2DemoModeDialog, SLOT(emuFinished(int, QProcess::ExitStatus)));
        connect(proc, SIGNAL(started()), qmc2DemoModeDialog, SLOT(emuStarted()));
      }
      if ( demoOpts )
        delete demoOpts;
    }
  }
#endif
}

void MainWindow::on_hSplitter_splitterMoved(int pos, int index)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_hSplitter_splitterMoved(int pos = %1, int index = %2)").arg(pos).arg(index));
#endif

#if defined(Q_WS_X11) || defined(Q_WS_WIN)
  if ( tabWidgetGamelist->currentIndex() != QMC2_EMBED_INDEX || (tabWidgetGamelist->currentIndex() == QMC2_EMBED_INDEX && !toolButtonEmbedderMaximizeToggle->isChecked()) ) {
    hSplitterSizes = hSplitter->sizes();
  } else if ( tabWidgetGamelist->currentIndex() == QMC2_EMBED_INDEX && toolButtonEmbedderMaximizeToggle->isChecked() ) {
    toolButtonEmbedderMaximizeToggle->setChecked(false);
    menuBar()->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ShowMenuBar", true).toBool());
    statusBar()->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Statusbar", true).toBool());
    toolbar->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Toolbar", true).toBool());
    frameStatus->show();
  }
#endif

  // show / hide game status indicator
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/GameStatusIndicator").toBool() ) {
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/GameStatusIndicatorOnlyWhenRequired").toBool() ) {
      if ( pos == 0 ) {
          if ( !labelGameStatus->isVisible() )
            labelGameStatus->setVisible(true);
      } else {
        if ( labelGameStatus->isVisible() )
          labelGameStatus->setVisible(false);
      }
    } else {
      if ( !labelGameStatus->isVisible() )
        labelGameStatus->setVisible(true);
    }
  } else
    if ( labelGameStatus->isVisible() )
      labelGameStatus->setVisible(false);
}

void MainWindow::on_actionToFavorites_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionToFavorites_activated()");
#endif

  if ( !qmc2CurrentItem )
    return;

  QString itemText = qmc2CurrentItem->text(QMC2_GAMELIST_COLUMN_GAME);

  if ( itemText == tr("Waiting for data...") )
    return;

  QList<QListWidgetItem *> matches = listWidgetFavorites->findItems(itemText, Qt::MatchExactly);
  if ( matches.count() <= 0 ) {
    QListWidgetItem *item = new QListWidgetItem(listWidgetFavorites);
    item->setText(itemText);
    listWidgetFavorites->sortItems();
  }
}

void MainWindow::on_actionReload_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionReload_activated()");
#endif

  if ( qmc2FilterActive ) {
    log(QMC2_LOG_FRONTEND, tr("please wait for ROM state filter to finish and try again"));
    return;
  }

  if ( qmc2VerifyActive ) {
    log(QMC2_LOG_FRONTEND, tr("please wait for ROM verification to finish and try again"));
    return;
  }

  if ( qmc2ImageCheckActive ) {
    log(QMC2_LOG_FRONTEND, tr("please wait for image check to finish and try again"));
    return;
  }

  if ( qmc2SampleCheckActive ) {
    log(QMC2_LOG_FRONTEND, tr("please wait for sample check to finish and try again"));
    return;
  }

  if ( qmc2ROMAlyzerActive ) {
    log(QMC2_LOG_FRONTEND, tr("please wait for ROMAlyzer to finish the current analysis and try again"));
    return;
  }

  if ( qmc2ReloadActive ) {
#if defined(QMC2_EMUTYPE_MAME)
    log(QMC2_LOG_FRONTEND, tr("game list reload is already active"));
#elif defined(QMC2_EMUTYPE_MESS)
    log(QMC2_LOG_FRONTEND, tr("machine list reload is already active"));
#endif
  } else {
    qmc2StopParser = false;
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProcessGameInfoDB").toBool() && qmc2DetailSetup->appliedDetailList.contains(QMC2_GAMEINFO_INDEX) )
      if ( qmc2GameInfoDB.isEmpty() && !qmc2StopParser ) {
        qmc2Gamelist->enableWidgets(false);
        loadGameInfoDB();
        qmc2Gamelist->enableWidgets(true);
      }

    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProcessEmuInfoDB").toBool() && qmc2DetailSetup->appliedDetailList.contains(QMC2_EMUINFO_INDEX) )
      if ( qmc2EmuInfoDB.isEmpty() && !qmc2StopParser ) {
        qmc2Gamelist->enableWidgets(false);
        loadEmuInfoDB();
        qmc2Gamelist->enableWidgets(true);
      }

    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveGameSelection").toBool() && !qmc2StartingUp ) {
      if ( qmc2CurrentItem ) {
        log(QMC2_LOG_FRONTEND, tr("saving game selection"));
#if defined(QMC2_EMUTYPE_MAME)
        qmc2Config->setValue("MAME/SelectedGame", qmc2CurrentItem->text(QMC2_GAMELIST_COLUMN_GAME));
#elif defined(QMC2_EMUTYPE_MESS)
        qmc2Config->setValue("MESS/SelectedGame", qmc2CurrentItem->text(QMC2_GAMELIST_COLUMN_GAME));
#endif
      } else
#if defined(QMC2_EMUTYPE_MAME)
        qmc2Config->remove("MAME/SelectedGame");
#elif defined(QMC2_EMUTYPE_MESS)
        qmc2Config->remove("MESS/SelectedGame");
#endif
    }
    if ( !qmc2StopParser )
      qmc2Gamelist->load();
  }
}

void MainWindow::on_actionExitStop_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionExitStop_activated()");
#endif

  close();
}

void MainWindow::on_actionCheckCurrentROM_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionCheckCurrentROM_activated()");
#endif

  if ( qmc2FilterActive ) {
    log(QMC2_LOG_FRONTEND, tr("please wait for ROM state filter to finish and try again"));
  } else if ( qmc2VerifyActive ) {
    log(QMC2_LOG_FRONTEND, tr("ROM verification already active"));
  } else if ( qmc2ReloadActive ) {
    log(QMC2_LOG_FRONTEND, tr("please wait for reload to finish and try again"));
  } else if ( qmc2ImageCheckActive ) {
    log(QMC2_LOG_FRONTEND, tr("please wait for image check to finish and try again"));
  } else if ( qmc2SampleCheckActive ) {
    log(QMC2_LOG_FRONTEND, tr("please wait for sample check to finish and try again"));
  } else if ( qmc2ROMAlyzerActive ) {
    log(QMC2_LOG_FRONTEND, tr("please wait for ROMAlyzer to finish the current analysis and try again"));
  } else
    qmc2Gamelist->verify(true);
}

void MainWindow::on_actionCheckROMs_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionCheckROMs_activated()");
#endif

  if ( qmc2FilterActive ) {
    log(QMC2_LOG_FRONTEND, tr("please wait for ROM state filter to finish and try again"));
  } else if ( qmc2VerifyActive ) {
    log(QMC2_LOG_FRONTEND, tr("ROM verification already active"));
  } else if ( qmc2ReloadActive ) {
    log(QMC2_LOG_FRONTEND, tr("please wait for reload to finish and try again"));
  } else if ( qmc2ImageCheckActive ) {
    log(QMC2_LOG_FRONTEND, tr("please wait for image check to finish and try again"));
  } else if ( qmc2SampleCheckActive ) {
    log(QMC2_LOG_FRONTEND, tr("please wait for sample check to finish and try again"));
  } else if ( qmc2ROMAlyzerActive ) {
    log(QMC2_LOG_FRONTEND, tr("please wait for ROMAlyzer to finish the current analysis and try again"));
  } else {
    if ( !qmc2Gamelist->autoRomCheck ) {
      switch ( QMessageBox::question(this,
                                     tr("Confirm"),
                                     tr("The ROM verification process may be very time-consuming.\nIt will overwrite existing cached data.\n\nDo you really want to check all ROM states now?"),
                                     QMessageBox::Yes | QMessageBox::No, QMessageBox::No) ) {
          case QMessageBox::Yes:
            qmc2Gamelist->verify();
            break;

          default:
            break;
      }
    } else {
      if ( qmc2AutomaticReload )
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("automatic ROM check triggered"));
      qmc2Gamelist->verify();
    }
  }
  qmc2Gamelist->autoRomCheck = false;
  qmc2AutomaticReload = false;
}

void MainWindow::on_actionExportROMStatus_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionExportROMStatus_activated()");
#endif

  if ( !qmc2ROMStatusExporter )
    qmc2ROMStatusExporter = new ROMStatusExporter(this);

  qmc2ROMStatusExporter->adjustIconSizes();

  if ( qmc2ROMStatusExporter->isHidden() )
    qmc2ROMStatusExporter->show();
  else if ( qmc2ROMStatusExporter->isMinimized() )
    qmc2ROMStatusExporter->showNormal();

  QTimer::singleShot(0, qmc2ROMStatusExporter, SLOT(raise()));
}

void MainWindow::on_actionDemoMode_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionDemoMode_activated()");
#endif

  if ( !qmc2DemoModeDialog )
    qmc2DemoModeDialog = new DemoModeDialog(this);

  qmc2DemoModeDialog->adjustIconSizes();

  if ( qmc2DemoModeDialog->isHidden() )
    qmc2DemoModeDialog->show();
  else if ( qmc2DemoModeDialog->isMinimized() )
    qmc2DemoModeDialog->showNormal();

  QTimer::singleShot(0, qmc2DemoModeDialog, SLOT(raise()));
}

void MainWindow::on_actionCheckSamples_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionCheckSamples_activated()");
#endif

  if ( !qmc2SampleChecker )
    qmc2SampleChecker = new SampleChecker(this);

  if ( qmc2SampleChecker->isHidden() )
    qmc2SampleChecker->show();
  else if ( qmc2SampleChecker->isMinimized() )
    qmc2SampleChecker->showNormal();

  QTimer::singleShot(0, qmc2SampleChecker, SLOT(raise()));
}

void MainWindow::on_actionCheckPreviews_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionCheckPreviews_activated()");
#endif

  if ( !qmc2ImageChecker )
    qmc2ImageChecker = new ImageChecker(this);

  qmc2ImageChecker->tabWidgetImageChecker->setCurrentIndex(QMC2_PREVIEW_INDEX);
  if ( qmc2ImageChecker->isHidden() )
    qmc2ImageChecker->show();
  else if ( qmc2ImageChecker->isMinimized() )
    qmc2ImageChecker->showNormal();

  QTimer::singleShot(0, qmc2ImageChecker, SLOT(raise()));
}

void MainWindow::on_actionCheckFlyers_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionCheckFlyers_activated()");
#endif

  if ( !qmc2ImageChecker )
    qmc2ImageChecker = new ImageChecker(this);

  qmc2ImageChecker->tabWidgetImageChecker->setCurrentIndex(QMC2_FLYER_INDEX);
  if ( qmc2ImageChecker->isHidden() )
    qmc2ImageChecker->show();
  else if ( qmc2ImageChecker->isMinimized() )
    qmc2ImageChecker->showNormal();

  QTimer::singleShot(0, qmc2ImageChecker, SLOT(raise()));
}

void MainWindow::on_actionCheckIcons_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionCheckIcons_activated()");
#endif

  if ( !qmc2ImageChecker )
    qmc2ImageChecker = new ImageChecker(this);

  qmc2ImageChecker->tabWidgetImageChecker->setCurrentIndex(QMC2_ICON_INDEX);
  if ( qmc2ImageChecker->isHidden() )
    qmc2ImageChecker->show();
  else if ( qmc2ImageChecker->isMinimized() )
    qmc2ImageChecker->showNormal();

  QTimer::singleShot(0, qmc2ImageChecker, SLOT(raise()));
}

void MainWindow::on_actionROMAlyzer_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionROMAlyzer_activated()");
#endif

  if ( !qmc2ROMAlyzer )
    qmc2ROMAlyzer = new ROMAlyzer(this);

  if ( !qmc2ROMAlyzerActive )
    qmc2ROMAlyzer->lineEditGames->setText("*");

  if ( qmc2ROMAlyzer->isHidden() )
    qmc2ROMAlyzer->show();
  else if ( qmc2ROMAlyzer->isMinimized() )
    qmc2ROMAlyzer->showNormal();

  QTimer::singleShot(0, qmc2ROMAlyzer, SLOT(raise()));
}

void MainWindow::on_actionRunRomTool_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionRunRomTool_activated()");
#endif

  if ( !qmc2CurrentItem )
    return;

  if ( qmc2CurrentItem->text(QMC2_GAMELIST_COLUMN_GAME) == tr("Waiting for data...") )
    return;

  QString command = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/RomTool").toString();
  QStringList args = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/RomToolArguments").toString().split(" ");
  QString wd = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/RomToolWorkingDirectory").toString();
  QString gameID = qmc2CurrentItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON);
  QString gameDescription = qmc2GamelistDescriptionMap[gameID];
  QStringList newArgs;
  foreach (QString argument, args)
    newArgs << argument.replace("$ID$", gameID).replace("$DESCRIPTION$", gameDescription);
  ToolExecutor romTool(this, command, newArgs, wd);
  romTool.exec();
}

void MainWindow::on_actionAnalyseCurrentROM_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionAnalyseCurrentROM_activated()");
#endif

  if ( !qmc2CurrentItem )
    return;

  if ( qmc2CurrentItem->text(QMC2_GAMELIST_COLUMN_GAME) == tr("Waiting for data...") )
    return;

  if ( qmc2ROMAlyzerActive ) {
    log(QMC2_LOG_FRONTEND, tr("please wait for ROMAlyzer to finish the current analysis and try again"));
    return;
  }

  if ( !qmc2ROMAlyzer )
    qmc2ROMAlyzer = new ROMAlyzer(this);

  qmc2ROMAlyzer->lineEditGames->setText(qmc2CurrentItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON));

  if ( qmc2ROMAlyzer->isHidden() )
    qmc2ROMAlyzer->show();
  else if ( qmc2ROMAlyzer->isMinimized() )
    qmc2ROMAlyzer->showNormal();

  if ( qmc2ROMAlyzer->tabWidgetAnalysis->currentWidget() != qmc2ROMAlyzer->tabReport && qmc2ROMAlyzer->tabWidgetAnalysis->currentWidget() != qmc2ROMAlyzer->tabLog )
    qmc2ROMAlyzer->tabWidgetAnalysis->setCurrentWidget(qmc2ROMAlyzer->tabReport);

  QTimer::singleShot(0, qmc2ROMAlyzer, SLOT(raise()));
  QTimer::singleShot(0, qmc2ROMAlyzer->pushButtonAnalyze, SLOT(click()));
}

void MainWindow::on_actionClearImageCache_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionClearImageCache_activated()");
#endif

  QPixmapCache::clear();
  log(QMC2_LOG_FRONTEND, tr("image cache cleared"));
}

void MainWindow::on_actionClearIconCache_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionClearIconCache_activated()");
#endif

  qmc2IconMap.clear();
  qmc2IconsPreloaded = false;
  log(QMC2_LOG_FRONTEND, tr("icon cache cleared"));
}

void MainWindow::on_actionClearMAWSCache_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionClearMAWSCache_activated()");
#endif

  QString cacheStatus = tr("freed %n byte(s) in %1", "", qmc2MAWSCache.totalCost()).arg(tr("%n entry(s)", "", qmc2MAWSCache.count()));
  qmc2MAWSCache.clear();
#if defined(QMC2_EMUTYPE_MAME)
  log(QMC2_LOG_FRONTEND, tr("MAWS in-memory cache cleared (%1)").arg(cacheStatus));
  QDir mawsCacheDir(qmc2Config->value("MAME/FilesAndDirectories/MAWSCacheDirectory").toString());
  qulonglong removedBytes = 0;
  qulonglong removedFiles = 0;
  if ( mawsCacheDir.exists() ) {
    QStringList webCacheFiles = mawsCacheDir.entryList(QStringList("*.wc"));
    foreach (QString webCacheFile, webCacheFiles) {
      QFileInfo fi(mawsCacheDir.filePath(webCacheFile));
      qint64 fSize = fi.size();
      if ( mawsCacheDir.remove(webCacheFile) ) {
        removedBytes += fSize;
        removedFiles++;
      }
      qApp->processEvents();
    }
  }
  cacheStatus = tr("removed %n byte(s) in %1", "", removedBytes).arg(tr("%n file(s)", "", removedFiles));
  log(QMC2_LOG_FRONTEND, tr("MAWS on-disk cache cleared (%1)").arg(cacheStatus));
#elif defined(QMC2_EMUTYPE_MESS)
  log(QMC2_LOG_FRONTEND, tr("MESS wiki in-memory cache cleared (%1)").arg(cacheStatus));
#endif
}

#if defined(QMC2_YOUTUBE_ENABLED)
void MainWindow::on_actionClearYouTubeCache_activated()
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionClearYouTubeCache_activated()");
#endif

	QDir youTubeCacheDir(qmc2Config->value(QMC2_FRONTEND_PREFIX + "YouTubeWidget/CacheDirectory").toString());
	quint64 removedBytes = 0;
	quint64 removedFiles = 0;
	if ( youTubeCacheDir.exists() ) {
		QStringList youTubeCacheFiles = youTubeCacheDir.entryList(QStringList("*"));
		foreach (QString youTubeCacheFile, youTubeCacheFiles) {
			QFileInfo fi(youTubeCacheDir.filePath(youTubeCacheFile));
			qint64 fSize = fi.size();
			if ( youTubeCacheDir.remove(youTubeCacheFile) ) {
				removedBytes += fSize;
				removedFiles++;
			}
			qApp->processEvents();
		}
	}
	QString removalInfo = tr("removed %n byte(s) in %1", "", removedBytes).arg(tr("%n file(s)", "", removedFiles));
	log(QMC2_LOG_FRONTEND, tr("YouTube on-disk cache cleared (%1)").arg(removalInfo));
}
#endif

void MainWindow::on_actionClearROMStateCache_activated()
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionClearROMStateCache_activated()");
#endif

	if ( qmc2ReloadActive ) {
		log(QMC2_LOG_FRONTEND, tr("please wait for reload to finish and try again"));
		return;
	}
	if ( qmc2VerifyActive ) {
		log(QMC2_LOG_FRONTEND, tr("please wait for ROM verification to finish and try again"));
		return;
	}

	QString userScopePath = QMC2_DYNAMIC_DOT_PATH;

#if defined(QMC2_EMUTYPE_MAME)
	QString fileName = qmc2Config->value("MAME/FilesAndDirectories/ROMStateCacheFile", userScopePath + "/mame.rsc").toString();
#elif defined(QMC2_EMUTYPE_MESS)
	QString fileName = qmc2Config->value("MESS/FilesAndDirectories/ROMStateCacheFile", userScopePath + "/mess.rsc").toString();
#else
	return;
#endif

	QFile f(fileName);
	if ( f.exists() ) {
		if ( f.remove() )
			log(QMC2_LOG_FRONTEND, tr("ROM state cache file '%1' forcedly removed upon user request").arg(fileName));
		else
			log(QMC2_LOG_FRONTEND, tr("WARNING: cannot remove the ROM state cache file '%1', please check permissions").arg(fileName));
	}

	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/AutoTriggerROMCheck").toBool() ) {
		qmc2Gamelist->autoRomCheck = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/AutoTriggerROMCheck").toBool();
		log(QMC2_LOG_FRONTEND, tr("triggering an automatic ROM check on next reload"));
	}
}

void MainWindow::on_actionClearGamelistCache_activated()
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionClearGamelistCache_activated()");
#endif

	if ( qmc2ReloadActive ) {
		log(QMC2_LOG_FRONTEND, tr("please wait for reload to finish and try again"));
		return;
	}

	QString userScopePath = QMC2_DYNAMIC_DOT_PATH;

#if defined(QMC2_EMUTYPE_MAME)
	QString fileName = qmc2Config->value("MAME/FilesAndDirectories/GamelistCacheFile", userScopePath + "/mame.glc").toString();
#elif defined(QMC2_EMUTYPE_MESS)
	QString fileName = qmc2Config->value("MESS/FilesAndDirectories/GamelistCacheFile", userScopePath + "/mess.glc").toString();
#else
	return;
#endif

	QFile f(fileName);
	if ( f.exists() ) {
#if defined(QMC2_EMUTYPE_MAME)
		if ( f.remove() )
			log(QMC2_LOG_FRONTEND, tr("game list cache file '%1' forcedly removed upon user request").arg(fileName));
		else
			log(QMC2_LOG_FRONTEND, tr("WARNING: cannot remove the game list cache file '%1', please check permissions").arg(fileName));
#elif defined(QMC2_EMUTYPE_MESS)
		if ( f.remove() )
			log(QMC2_LOG_FRONTEND, tr("machine list cache file '%1' forcedly removed upon user request").arg(fileName));
		else
			log(QMC2_LOG_FRONTEND, tr("WARNING: cannot remove the machine list cache file '%1', please check permissions").arg(fileName));
#endif
	}
}

void MainWindow::on_actionClearXMLCache_activated()
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionClearXMLCache_activated()");
#endif

	if ( qmc2ReloadActive ) {
		log(QMC2_LOG_FRONTEND, tr("please wait for reload to finish and try again"));
		return;
	}

	QString userScopePath = QMC2_DYNAMIC_DOT_PATH;

#if defined(QMC2_EMUTYPE_MAME)
	QString fileName = qmc2Config->value("MAME/FilesAndDirectories/ListXMLCache", userScopePath + "/mame.lxc").toString();
#elif defined(QMC2_EMUTYPE_MESS)
	QString fileName = qmc2Config->value("MESS/FilesAndDirectories/ListXMLCache", userScopePath + "/mess.lxc").toString();
#else
	return;
#endif

	QFile f(fileName);
	if ( f.exists() ) {
		if ( f.remove() )
			log(QMC2_LOG_FRONTEND, tr("XML cache file '%1' forcedly removed upon user request").arg(fileName));
		else
			log(QMC2_LOG_FRONTEND, tr("WARNING: cannot remove the XML cache file '%1', please check permissions").arg(fileName));
	}
	softwareListXmlDataCache.clear();
	systemSoftwareListMap.clear();
	systemSoftwareFilterMap.clear();
}

void MainWindow::on_actionClearSoftwareListCache_activated()
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionClearSoftwareListCache_activated()");
#endif

	if ( qmc2ReloadActive ) {
		log(QMC2_LOG_FRONTEND, tr("please wait for reload to finish and try again"));
		return;
	}

	QString userScopePath = QMC2_DYNAMIC_DOT_PATH;

#if defined(QMC2_EMUTYPE_MAME)
	QString fileName = qmc2Config->value("MAME/FilesAndDirectories/SoftwareListCache", userScopePath + "/mame.swl").toString();
#elif defined(QMC2_EMUTYPE_MESS)
	QString fileName = qmc2Config->value("MESS/FilesAndDirectories/SoftwareListCache", userScopePath + "/mess.swl").toString();
#else
	return;
#endif

	QFile f(fileName);
	if ( f.exists() ) {
		if ( f.remove() )
			log(QMC2_LOG_FRONTEND, tr("software list cache file '%1' forcedly removed upon user request").arg(fileName));
		else
			log(QMC2_LOG_FRONTEND, tr("WARNING: cannot remove the software list cache file '%1', please check permissions").arg(fileName));
	}
}

void MainWindow::on_actionClearAllEmulatorCaches_activated()
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionClearAllEmulatorCaches_activated()");
#endif

	actionClearROMStateCache->trigger();
	actionClearGamelistCache->trigger();
	actionClearXMLCache->trigger();
	actionClearSoftwareListCache->trigger();
}

void MainWindow::on_actionRecreateTemplateMap_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionRecreateTemplateMap_activated()");
#endif

  if ( qmc2GlobalEmulatorOptions != NULL )
    qmc2GlobalEmulatorOptions->createTemplateMap();
}

void MainWindow::on_actionCheckTemplateMap_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionCheckTemplateMap_activated()");
#endif

  if ( qmc2GlobalEmulatorOptions != NULL )
    qmc2GlobalEmulatorOptions->checkTemplateMap();
}

void MainWindow::on_actionOptions_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionOptions_activated()");
#endif

  if ( qmc2Options->isHidden() )
    qmc2Options->show();
  else if ( qmc2Options->isMinimized() )
    qmc2Options->showNormal();

  QTimer::singleShot(0, qmc2Options, SLOT(raise()));
}

void MainWindow::on_actionFullscreenToggle_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionFullscreenToggle_activated()");
#endif

#if defined(QMC2_YOUTUBE_ENABLED)
  if ( qmc2YouTubeWidget )
	  if ( qmc2YouTubeWidget->videoPlayer->videoWidget()->isFullScreen() ) {
		  qmc2YouTubeWidget->videoPlayer->videoWidget()->setFullScreen(false);
		  qmc2YouTubeWidget->videoOverlayWidget->clearMessage();
		  qApp->processEvents();
		  if ( windowState() & Qt::WindowFullScreen )
			  actionFullscreenToggle->setChecked(true);
		  else
			  actionFullscreenToggle->setChecked(false);
		  return;
	  }
#endif

  if ( qmc2ArcadeView )
    if ( qmc2ArcadeView->isActiveWindow() ) {
      qmc2ArcadeView->raise();
      qApp->processEvents();
      qmc2ArcadeView->toggleFullscreen();
      if ( windowState() & Qt::WindowFullScreen )
        actionFullscreenToggle->setChecked(true);
      else
        actionFullscreenToggle->setChecked(false);
      return;
    }

  if ( !qmc2EarlyStartup ) {
    // saftey checks
    if ( windowState() & Qt::WindowFullScreen )
      actionFullscreenToggle->setChecked(false);
    else
      actionFullscreenToggle->setChecked(true);
  }

  bool feLogScrollBarMaximum = (textBrowserFrontendLog->verticalScrollBar()->value() == textBrowserFrontendLog->verticalScrollBar()->maximum());
  bool emuLogScrollBarMaximum = (textBrowserEmulatorLog->verticalScrollBar()->value() == textBrowserEmulatorLog->verticalScrollBar()->maximum());

  qApp->processEvents();

  if ( actionFullscreenToggle->isChecked() ) {
    if ( isVisible() && !(windowState() & Qt::WindowFullScreen) ) {
      if ( isMaximized() ) {
        qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Maximized", true);
      } else {
        qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Geometry", saveGeometry());
        qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Position", pos());
        qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Size", size());
        qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Maximized", false);
      }
    } else {
      if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Size") )
        resize(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Size").toSize());
      if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Position") )
        move(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Position").toPoint());
      if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Geometry") )
        restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Geometry").toByteArray());
    }
    // we need to tweak full screen startup to ensure that raising the window works
    if ( qmc2EarlyStartup ) {
      setUpdatesEnabled(true);
      showNormal();
      raise();
    }
    showFullScreen();
    if ( qmc2EarlyStartup ) {
      qApp->processEvents();
      setUpdatesEnabled(false);
    }
  } else {
#if defined(QMC2_YOUTUBE_ENABLED)
    bool youTubeWasPlaying = false;
    if ( qmc2YouTubeWidget )
      youTubeWasPlaying = qmc2YouTubeWidget->videoPlayer->isPlaying();
#endif
    hide();
    qApp->processEvents();
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/RestoreLayout").toBool() ) {
      if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Maximized", false).toBool() ) {
        showMaximized();
      } else {
        if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Size") )
          resize(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Size").toSize());
        if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Position") )
          move(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Position").toPoint());
        if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Geometry") )
          restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Geometry").toByteArray());
        showNormal();
      }
    } else {
      resize(640, 480);
      move((qApp->desktop()->availableGeometry().width() - width()) / 2, (qApp->desktop()->availableGeometry().height() - height()) / 2);
      showNormal();
    }
#if defined(QMC2_YOUTUBE_ENABLED)
    if ( qmc2YouTubeWidget && youTubeWasPlaying )
      qmc2YouTubeWidget->videoPlayer->play();
#endif
  }

  activateWindow();
  raise();

  if ( feLogScrollBarMaximum )
    textBrowserFrontendLog->verticalScrollBar()->setValue(textBrowserFrontendLog->verticalScrollBar()->maximum());
  if ( emuLogScrollBarMaximum )
    textBrowserEmulatorLog->verticalScrollBar()->setValue(textBrowserEmulatorLog->verticalScrollBar()->maximum());
}

void MainWindow::on_actionLaunchQMC2MAME_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: on_actionLaunchQMC2MAME_activated()");
#endif

  if ( !qmc2VariantSwitchReady )
    return;

  bool launched = false;

#if defined(Q_WS_MAC)
  OSStatus err;
  FSRef appRef;
  err = LSFindApplicationForInfo(kLSUnknownCreator, CFSTR(QMC2_VARIANT_SDLMAME_BUNDLE_ID), NULL, &appRef, NULL);
  if ( err == noErr )
    err = LSOpenFSRef(&appRef, NULL);
  launched = err == noErr;
#elif defined(Q_WS_WIN)
  QString otherVariantExe = qmc2Config->value("MESS/FilesAndDirectories/MAMEVariantExe", QString()).toString();
  QStringList otherVariantArgs = qmc2Config->value("MESS/FilesAndDirectories/MAMEVariantExeArguments", QString()).toString().split(" ", QString::SkipEmptyParts);
  if ( otherVariantArgs.isEmpty() )
	  otherVariantArgs = qApp->arguments();
  HANDLE procHandle = NULL;
  if ( otherVariantExe.isEmpty() )
	  procHandle = winFindProcessHandle(QMC2_VARIANT_MAME_NAME);
  else {
	  QFileInfo fi(otherVariantExe);
	  procHandle = winFindProcessHandle(fi.fileName());
  }
  if ( procHandle != NULL ) {
	HWND windowHandle = winFindWindowHandle(QMC2_VARIANT_MAME_TITLE);
	if ( windowHandle )
		launched = BringWindowToTop(windowHandle);
  } else {
	// if not stated otherwise, we search the other variant in the directory we were started from
	if ( otherVariantExe.isEmpty() ) {
		WCHAR myExecPath[MAX_PATH + 1];
		GetModuleFileName(NULL, myExecPath, MAX_PATH + 1);
		QFileInfo fi(QString::fromWCharArray(myExecPath));
		QDir execDir(fi.path());
		if ( execDir.exists(QMC2_VARIANT_MAME_NAME) )
			launched = QProcess::startDetached(execDir.path() + "/" + QMC2_VARIANT_MAME_NAME, otherVariantArgs);
	} else
		launched = QProcess::startDetached(otherVariantExe, otherVariantArgs);
  }
#else
  QStringList args;
  args << QMC2_VARIANT_SDLMAME_NAME << QMC2_VARIANT_SDLMAME_TITLE << QMC2_VARIANT_SDLMAME_NAME << qApp->arguments();
  launched = QProcess::startDetached(QMC2_COMMAND_RUNONCE, args);
#endif

  if ( launched ) {
#if defined(Q_WS_WIN)
    log(QMC2_LOG_FRONTEND, tr("variant '%1' launched").arg(QMC2_VARIANT_MAME_NAME));
#else
    log(QMC2_LOG_FRONTEND, tr("variant '%1' launched").arg(QMC2_VARIANT_SDLMAME_NAME));
#endif

    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ExitOnVariantLaunch").toBool() ) {
      // we need to call this twice to make sure active processing gets stopped before QMC2 exits...
      QTimer::singleShot(0, actionExitStop, SLOT(trigger()));
      QTimer::singleShot(100, actionExitStop, SLOT(trigger()));
    } else if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/MinimizeOnVariantLaunch").toBool() ) {
      if ( qmc2Options )
        if ( qmc2Options->isVisible() )
          qmc2Options->showMinimized();
      if ( qmc2ROMAlyzer )
        if ( qmc2ROMAlyzer->isVisible() )
          qmc2ROMAlyzer->showMinimized();
      if ( qmc2ImageChecker )
        if ( qmc2ImageChecker->isVisible() )
          qmc2ImageChecker->showMinimized();
      if ( qmc2SampleChecker )
        if ( qmc2SampleChecker->isVisible() )
          qmc2SampleChecker->showMinimized();
      if ( qmc2ArcadeView )
        if ( qmc2ArcadeView->isVisible() )
          qmc2ArcadeView->close();
      if ( qmc2ArcadeSetupDialog )
        if ( qmc2ArcadeSetupDialog->isVisible() )
          qmc2ArcadeSetupDialog->showMinimized();
      if ( qmc2DocBrowser )
        if ( qmc2DocBrowser->isVisible() )
          qmc2DocBrowser->showMinimized();
#if QMC2_USE_PHONON_API
      if ( qmc2AudioEffectDialog )
	if ( qmc2AudioEffectDialog->isVisible() )
	  qmc2AudioEffectDialog->showMinimized();
#endif
      showMinimized();
    }
  } else {
#if defined(Q_WS_WIN)
    log(QMC2_LOG_FRONTEND, tr("WARNING: failed to launch variant '%1'").arg(QMC2_VARIANT_MAME_NAME));
#else
    log(QMC2_LOG_FRONTEND, tr("WARNING: failed to launch variant '%1'").arg(QMC2_VARIANT_SDLMAME_NAME));
#endif
  }
}

void MainWindow::on_actionLaunchQMC2MESS_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: on_actionLaunchQMC2MESS_activated()");
#endif

  if ( !qmc2VariantSwitchReady )
    return;

  bool launched = false;

#if defined(Q_WS_MAC)
  OSStatus err;
  FSRef appRef;
  err = LSFindApplicationForInfo(kLSUnknownCreator, CFSTR(QMC2_VARIANT_SDLMESS_BUNDLE_ID), NULL, &appRef, NULL);
  if ( err == noErr )
    err = LSOpenFSRef(&appRef, NULL);
  launched = err == noErr;
#elif defined(Q_WS_WIN)
  QString otherVariantExe = qmc2Config->value("MAME/FilesAndDirectories/MESSVariantExe", QString()).toString();
  QStringList otherVariantArgs = qmc2Config->value("MAME/FilesAndDirectories/MESSVariantExeArguments", QString()).toString().split(" ", QString::SkipEmptyParts);
  if ( otherVariantArgs.isEmpty() )
	  otherVariantArgs = qApp->arguments();
  HANDLE procHandle = NULL;
  if ( otherVariantExe.isEmpty() )
	  procHandle = winFindProcessHandle(QMC2_VARIANT_MESS_NAME);
  else {
	  QFileInfo fi(otherVariantExe);
	  procHandle = winFindProcessHandle(fi.fileName());
  }
  if ( procHandle != NULL ) {
	HWND windowHandle = winFindWindowHandle(QMC2_VARIANT_MESS_TITLE);
	if ( windowHandle )
		launched = BringWindowToTop(windowHandle);
  } else {
	// if not stated otherwise, we search the other variant in the directory we were started from
	if ( otherVariantExe.isEmpty() ) {
		WCHAR myExecPath[MAX_PATH + 1];
		GetModuleFileName(NULL, myExecPath, MAX_PATH + 1);
		QFileInfo fi(QString::fromWCharArray(myExecPath));
		QDir execDir(fi.path());
		if ( execDir.exists(QMC2_VARIANT_MESS_NAME) )
			launched = QProcess::startDetached(execDir.path() + "/" + QMC2_VARIANT_MESS_NAME, otherVariantArgs);
	} else
		launched = QProcess::startDetached(otherVariantExe, otherVariantArgs);
  }
#else
  QStringList args;
  args << QMC2_VARIANT_SDLMESS_NAME << QMC2_VARIANT_SDLMESS_TITLE << QMC2_VARIANT_SDLMESS_NAME << qApp->arguments();
  launched = QProcess::startDetached(QMC2_COMMAND_RUNONCE, args);
#endif

  if ( launched ) {
#if defined(Q_WS_WIN)
    log(QMC2_LOG_FRONTEND, tr("variant '%1' launched").arg(QMC2_VARIANT_MESS_NAME));
#else
    log(QMC2_LOG_FRONTEND, tr("variant '%1' launched").arg(QMC2_VARIANT_SDLMESS_NAME));
#endif

    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ExitOnVariantLaunch").toBool() ) {
      // we need to call this twice to make sure active processing gets stopped before QMC2 exits...
      QTimer::singleShot(0, actionExitStop, SLOT(trigger()));
      QTimer::singleShot(100, actionExitStop, SLOT(trigger()));
    } else if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/MinimizeOnVariantLaunch").toBool() ) {
      if ( qmc2Options )
        if ( qmc2Options->isVisible() )
          qmc2Options->showMinimized();
      if ( qmc2ROMAlyzer )
        if ( qmc2ROMAlyzer->isVisible() )
          qmc2ROMAlyzer->showMinimized();
      if ( qmc2ImageChecker )
        if ( qmc2ImageChecker->isVisible() )
          qmc2ImageChecker->showMinimized();
      if ( qmc2SampleChecker )
        if ( qmc2SampleChecker->isVisible() )
          qmc2SampleChecker->showMinimized();
      if ( qmc2ArcadeView )
        if ( qmc2ArcadeView->isVisible() )
          qmc2ArcadeView->close();
      if ( qmc2ArcadeSetupDialog )
        if ( qmc2ArcadeSetupDialog->isVisible() )
          qmc2ArcadeSetupDialog->showMinimized();
      if ( qmc2DocBrowser )
        if ( qmc2DocBrowser->isVisible() )
          qmc2DocBrowser->showMinimized();
#if QMC2_USE_PHONON_API
      if ( qmc2AudioEffectDialog )
	if ( qmc2AudioEffectDialog->isVisible() )
	  qmc2AudioEffectDialog->showMinimized();
#endif
      showMinimized();
    }
  } else {
#if defined(Q_WS_WIN)
    log(QMC2_LOG_FRONTEND, tr("WARNING: failed to launch variant '%1'").arg(QMC2_VARIANT_MESS_NAME));
#else
    log(QMC2_LOG_FRONTEND, tr("WARNING: failed to launch variant '%1'").arg(QMC2_VARIANT_SDLMESS_NAME));
#endif
  }
}

void MainWindow::on_actionDocumentation_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionDocumentation_activated()");
#endif

  if ( !qmc2DocBrowser ) {
    qmc2DocBrowser = new DocBrowser(this);
    QString searchPath;
    searchPath = qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/DataDirectory").toString() +
                 "doc/html/" +
                 qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Language").toString();
#if defined(Q_WS_WIN)
    QDir searchDir(searchPath);
    QUrl docUrl("file:///" + searchDir.absolutePath() + "/index.html");
#else
    QUrl docUrl("file://" + searchPath + "/index.html");
#endif
    qmc2DocBrowser->browser->webViewBrowser->load(docUrl);
  }

  if ( qmc2DocBrowser->isMinimized() )
    qmc2DocBrowser->showNormal();
  else
    qmc2DocBrowser->show();
  qmc2DocBrowser->raise();
}

void MainWindow::on_actionAbout_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionAbout_activated()");
#endif

  if ( !qmc2About )
    qmc2About = new About(this);

  qmc2About->show();
  qmc2About->raise();
}

void MainWindow::on_actionAboutQt_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionAboutQt_activated()");
#endif

  QMessageBox::aboutQt(this, tr("About Qt"));
}

void MainWindow::on_actionArcadeSetup_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionArcadeSetup_activated()");
#endif

  log(QMC2_LOG_FRONTEND, tr("WARNING: this feature is not yet working!"));

  if ( !qmc2ArcadeSetupDialog )
    qmc2ArcadeSetupDialog = new ArcadeSetupDialog(this);

  qmc2ArcadeSetupDialog->show();
  qmc2ArcadeSetupDialog->raise();
}

void MainWindow::on_actionArcadeToggle_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionArcadeToggle_activated()");
#endif

  if ( qmc2DestroyingArcadeView )
    return;

  if ( !qmc2ArcadeView ) {
    log(QMC2_LOG_FRONTEND, tr("WARNING: this feature is not yet working!"));
    qmc2ArcadeView = new ArcadeView(0);
  }

  if ( qmc2ArcadeView->isVisible() ) {
    qmc2DestroyingArcadeView = true;
    qmc2ArcadeView->close();
    QTimer::singleShot(0, this, SLOT(destroyArcadeView()));
  } else {
    qmc2ArcadeView->show();
    qmc2ArcadeView->raise();
  }
}

void MainWindow::destroyArcadeView()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::destroyArcadeView()");
#endif

  if ( qmc2ArcadeView ) {
    delete qmc2ArcadeView;
    qmc2ArcadeView = NULL;
    qmc2DestroyingArcadeView = false;
  }
}

void MainWindow::on_comboBoxSearch_editTextChanged(const QString &)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_comboBoxSearch_editTextChanged(const QString &)");
#endif

  searchTimer.start(QMC2_SEARCH_DELAY);
}

void MainWindow::on_comboBoxSearch_editTextChanged_delayed()
{
  searchTimer.stop();

  QString pattern = comboBoxSearch->currentText();

  // easy pattern match
  if ( !pattern.isEmpty() ) {
    pattern = "*" + pattern.replace(' ', "* *") + "*";
    pattern.replace(QString("*^"), "");
    pattern.replace(QString("$*"), "");
  }

  listWidgetSearch->clear();
  QList<QTreeWidgetItem *> matches = treeWidgetGamelist->findItems(pattern, Qt::MatchContains | Qt::MatchWildcard, QMC2_GAMELIST_COLUMN_GAME);
  QList<QTreeWidgetItem *> matchesByShortName = treeWidgetGamelist->findItems(pattern, Qt::MatchContains | Qt::MatchWildcard, QMC2_GAMELIST_COLUMN_NAME);

  int i;
  for (i = 0; i < matchesByShortName.count(); i++) {
    QTreeWidgetItem *item = matchesByShortName[i];
    if ( !matches.contains(item) ) matches.append(item);
  }
  
  for (i = 0; i < matches.count(); i++) {
    QListWidgetItem *item = new QListWidgetItem(listWidgetSearch);
    item->setText(matches.at(i)->text(QMC2_GAMELIST_COLUMN_GAME));
  }

  listWidgetSearch->sortItems();

  qmc2Gamelist->numSearchGames = matches.count();
  labelGamelistStatus->setText(qmc2Gamelist->status());

  QTimer::singleShot(0, this, SLOT(checkCurrentSearchSelection()));
}

void MainWindow::on_comboBoxSearch_activated(const QString &pattern)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_comboBoxSearch_activated(const QString &pattern = \"" + pattern + "\")");
#endif

  on_comboBoxSearch_editTextChanged_delayed();
  if ( tabWidgetGamelist->currentWidget() != tabSearch ) {
    tabWidgetGamelist->blockSignals(true);
    tabWidgetGamelist->setCurrentWidget(tabSearch);
    tabWidgetGamelist->blockSignals(false);
  }
  QTimer::singleShot(0, listWidgetSearch, SLOT(setFocus()));
}

void MainWindow::on_listWidgetSearch_currentTextChanged(QString s)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_listWidgetSearch_currentTextChanged(QString s = \""+ s + "\")");
#endif

  QTreeWidgetItem *matchItem = qmc2GamelistItemByDescriptionMap[s];
  if ( matchItem ) {
    qmc2CheckItemVisibility = false;
    treeWidgetGamelist->clearSelection();
    treeWidgetGamelist->setCurrentItem(matchItem);
    qmc2CurrentItem = matchItem;
  }
#ifdef QMC2_DEBUG
  else
    log(QMC2_LOG_FRONTEND, "DEBUG: ERROR: no match found (?)");
#endif
}

void MainWindow::on_listWidgetSearch_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_listWidgetSearch_currentItemChanged(QListWidgetItem *current = %1, QListWidgetItem *previous = %2)").arg((qulonglong)current).arg((qulonglong)previous));
#endif

  QTreeWidgetItem *glItem = NULL;
  if ( current )
    glItem = qmc2GamelistItemByDescriptionMap[current->text()];
  if ( glItem ) {
    qmc2CheckItemVisibility = false;
    treeWidgetGamelist->clearSelection();
    qmc2CurrentItem = glItem;
    treeWidgetGamelist->setCurrentItem(glItem);
    processEvents();
  }
}

void MainWindow::on_listWidgetSearch_itemPressed(QListWidgetItem *current)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_listWidgetSearch_itemPressed(QListWidgetItem *current = %1)").arg((qulonglong)current));
#endif

  on_listWidgetSearch_currentItemChanged(current, NULL);
}

void MainWindow::on_listWidgetSearch_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_listWidgetSearch_itemSelectionChanged()");
#endif

  QList<QListWidgetItem *> selected = listWidgetSearch->selectedItems();
  if ( selected.count() > 0 )
    on_listWidgetSearch_currentItemChanged(selected[0], NULL);
}

void MainWindow::on_listWidgetSearch_itemActivated(QListWidgetItem *item)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_listWidgetSearch_itemActivated(QListWidgetItem *item = %1)").arg((qulonglong)item));
#endif

  if ( item == NULL )
    return;

  QTreeWidgetItem *matchItem = qmc2GamelistItemByDescriptionMap[item->text()];
  if ( matchItem ) {
    qmc2CheckItemVisibility = false;
    treeWidgetGamelist->clearSelection();
    treeWidgetGamelist->setCurrentItem(matchItem);
    treeWidgetGamelist->scrollToItem(matchItem, qmc2CursorPositioningMode);
    qmc2CurrentItem = matchItem;
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/PlayOnSublistActivation").toBool() ) {
      if ( qmc2DemoModeDialog )
	      if ( qmc2DemoModeDialog->demoModeRunning )
		      return;
      		switch ( qmc2DefaultLaunchMode ) {
#if defined(Q_WS_X11) || defined(Q_WS_WIN)
			case QMC2_LAUNCH_MODE_EMBEDDED:
				QTimer::singleShot(0, this, SLOT(on_actionPlayEmbedded_activated()));
				break;
#endif
			case QMC2_LAUNCH_MODE_INDEPENDENT:
			default:
				QTimer::singleShot(0, this, SLOT(on_actionPlay_activated()));
				break;
		}
    } else {
      tabWidgetGamelist->setCurrentIndex(0);
      if ( !qmc2ReloadActive )
        treeWidgetGamelist->expandItem(matchItem);
    }
  }
  else
    log(QMC2_LOG_FRONTEND, tr("ERROR: no match found (?)"));
}

void MainWindow::on_listWidgetFavorites_currentTextChanged(QString s)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_listWidgetFavorites_currentTextChanged(QString s = \""+ s + "\")");
#endif

  on_listWidgetSearch_currentTextChanged(s);
}

void MainWindow::on_listWidgetFavorites_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_listWidgetFavorites_itemSelectionChanged()");
#endif

  QList<QListWidgetItem *> selected = listWidgetFavorites->selectedItems();
  if ( selected.count() > 0 )
    on_listWidgetSearch_currentItemChanged(selected[0], NULL);
}

void MainWindow::on_listWidgetFavorites_itemActivated(QListWidgetItem *item)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_listWidgetFavorites_itemActivated(QListWidgetItem *item = %1)").arg((qulonglong)item));
#endif

  on_listWidgetSearch_itemActivated(item);
}

void MainWindow::on_listWidgetPlayed_currentTextChanged(QString s)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_listWidgetPlayed_currentTextChanged(QString s = \""+ s + "\")");
#endif

  on_listWidgetSearch_currentTextChanged(s);
}

void MainWindow::on_listWidgetPlayed_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_listWidgetPlayed_itemSelectionChanged()");
#endif

  QList<QListWidgetItem *> selected = listWidgetPlayed->selectedItems();
  if ( selected.count() > 0 )
    on_listWidgetSearch_currentItemChanged(selected[0], NULL);
}

void MainWindow::on_listWidgetPlayed_itemActivated(QListWidgetItem *item)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_listWidgetPlayed_itemActivated(QListWidgetItem *item = %1)").arg((qulonglong)item));
#endif

  on_listWidgetSearch_itemActivated(item);
}

void MainWindow::on_tabWidgetGamelist_currentChanged(int currentIndex)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_tabWidgetGamelist_currentChanged(int i = " + QString::number(currentIndex) + ")");
#endif

  static int lastTabWidgetGamelistIndex = -1;

  if ( !qmc2EarlyStartup ) {
    menuGamelistHeader->hide();
    menuHierarchyHeader->hide();
#if defined(QMC2_EMUTYPE_MAME)
    menuCategoryHeader->hide();
    menuVersionHeader->hide();
#endif
  }

#if defined(Q_WS_X11) || defined(Q_WS_WIN)
  if ( hSplitterSizes.count() > 1 && currentIndex != QMC2_EMBED_INDEX )
    hSplitter->setSizes(hSplitterSizes);
#endif

  switch ( currentIndex ) {
    case QMC2_GAMELIST_INDEX:
#if defined(Q_WS_X11) || defined(Q_WS_WIN)
      if ( lastTabWidgetGamelistIndex != QMC2_EMBED_INDEX )
        QTimer::singleShot(0, this, SLOT(scrollToCurrentItem()));
      menuBar()->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ShowMenuBar", true).toBool());
      statusBar()->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Statusbar", true).toBool());
      toolbar->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Toolbar", true).toBool());
      frameStatus->show();
#else
      QTimer::singleShot(0, this, SLOT(scrollToCurrentItem()));
#endif
      switch ( stackedWidgetView->currentIndex() ) {
	      case QMC2_VIEW_TREE_INDEX:
		      treeWidgetHierarchy->activateWindow();
		      treeWidgetHierarchy->setFocus();
		      break;
#if defined(QMC2_EMUTYPE_MAME)
	      case QMC2_VIEW_CATEGORY_INDEX:
		      treeWidgetCategoryView->activateWindow();
		      treeWidgetCategoryView->setFocus();
		      break;
	      case QMC2_VIEW_VERSION_INDEX:
		      treeWidgetVersionView->activateWindow();
		      treeWidgetVersionView->setFocus();
		      break;
#endif
	      case QMC2_VIEW_DETAIL_INDEX:
	      default:
		      treeWidgetGamelist->activateWindow();
		      treeWidgetGamelist->setFocus();
		      break;
      }
      break;

    case QMC2_SEARCH_INDEX:
#if defined(Q_WS_X11) || defined(Q_WS_WIN)
      if ( lastTabWidgetGamelistIndex != QMC2_EMBED_INDEX )
        QTimer::singleShot(0, this, SLOT(checkCurrentSearchSelection()));
      menuBar()->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ShowMenuBar", true).toBool());
      statusBar()->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Statusbar", true).toBool());
      toolbar->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Toolbar", true).toBool());
      frameStatus->show();
#else
      QTimer::singleShot(0, this, SLOT(checkCurrentSearchSelection()));
#endif
      if ( listWidgetSearch->count() > 0 ) {
        listWidgetSearch->activateWindow();
        listWidgetSearch->setFocus();
      } else {
        comboBoxSearch->activateWindow();
	comboBoxSearch->setFocus();
      }
      break;

    case QMC2_FAVORITES_INDEX:
#if defined(Q_WS_X11) || defined(Q_WS_WIN)
      if ( lastTabWidgetGamelistIndex != QMC2_EMBED_INDEX )
        QTimer::singleShot(0, this, SLOT(checkCurrentFavoritesSelection()));
      menuBar()->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ShowMenuBar", true).toBool());
      statusBar()->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Statusbar", true).toBool());
      toolbar->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Toolbar", true).toBool());
      frameStatus->show();
#else
      QTimer::singleShot(0, this, SLOT(checkCurrentFavoritesSelection()));
#endif
      if ( listWidgetFavorites->count() > 0 ) {
        listWidgetFavorites->activateWindow();
        listWidgetFavorites->setFocus();
      }
      break;

    case QMC2_PLAYED_INDEX:
#if defined(Q_WS_X11) || defined(Q_WS_WIN)
      if ( lastTabWidgetGamelistIndex != QMC2_EMBED_INDEX )
        QTimer::singleShot(0, this, SLOT(checkCurrentPlayedSelection()));
      menuBar()->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ShowMenuBar", true).toBool());
      statusBar()->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Statusbar", true).toBool());
      toolbar->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Toolbar", true).toBool());
      frameStatus->show();
#else
      QTimer::singleShot(0, this, SLOT(checkCurrentPlayedSelection()));
#endif
      if ( listWidgetPlayed->count() > 0 ) {
        listWidgetPlayed->activateWindow();
        listWidgetPlayed->setFocus();
      }
      break;

#if defined(Q_WS_X11) || defined(Q_WS_WIN)
    case QMC2_EMBED_INDEX: {
        if ( toolButtonEmbedderMaximizeToggle->isChecked() ) {
          menuBar()->hide();
          statusBar()->hide();
          toolbar->hide();
          frameStatus->hide();
          qApp->processEvents();
          hSplitterSizes = hSplitter->sizes();
          QList<int> maximizedSizes;
          if ( hSplitter->widget(0) == hSplitterWidget0 )
            maximizedSizes << desktopGeometry.width() << 0;
          else
            maximizedSizes << 0 << desktopGeometry.width();
          hSplitter->setSizes(maximizedSizes);
        }
      }
      break;
#endif

    default:
      break;
  }

  qmc2Preview->update();
  qmc2Flyer->update();
  qmc2Cabinet->update();
  qmc2Controller->update();
  qmc2Marquee->update();
  qmc2Title->update();
  qmc2PCB->update();

  // show / hide game status indicator
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/GameStatusIndicator").toBool() ) {
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/GameStatusIndicatorOnlyWhenRequired").toBool() ) {
      if ( currentIndex != QMC2_GAMELIST_INDEX ) {
        labelGameStatus->setVisible(true);
      } else {
        labelGameStatus->setVisible(false);
      }
    } else {
      labelGameStatus->setVisible(true);
    }
  } else
    labelGameStatus->setVisible(false);

  lastTabWidgetGamelistIndex = currentIndex;
}

void MainWindow::on_tabWidgetLogsAndEmulators_currentChanged(int currentIndex)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_tabWidgetLogsAndEmulators_currentChanged(int i = " + QString::number(currentIndex) + ")");
#endif

  switch ( currentIndex ) {
    case QMC2_FRONTENDLOG_INDEX:
      textBrowserFrontendLog->horizontalScrollBar()->setValue(0);
      break;

    case QMC2_EMULATORLOG_INDEX:
      textBrowserEmulatorLog->horizontalScrollBar()->setValue(0);
      break;

    default:
      break;
  }
}

void MainWindow::on_tabWidgetSoftwareDetail_currentChanged(int currentIndex)
{
#ifdef QMC2_DEBUG
       	log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_tabWidgetSoftwareDetail_currentChanged(int i = " + QString::number(currentIndex) + ")");
#endif

	if ( !qmc2SoftwareList )
		return;

	if ( !qmc2SoftwareList->currentItem )
		return;

	int left, top, right, bottom;
	gridLayout->getContentsMargins(&left, &top, &right, &bottom);

	switch ( currentIndex ) {
		case QMC2_SWINFO_SNAPSHOT_PAGE:
			if ( !qmc2SoftwareSnapshot ) {
				qmc2SoftwareSnapshot = new SoftwareSnapshot(tabSnapshot);
				QHBoxLayout *layout = new QHBoxLayout;
				layout->addWidget(qmc2SoftwareSnapshot);
				layout->setContentsMargins(left, top, right, bottom);
				tabSnapshot->setLayout(layout);
			}
			qmc2SoftwareSnapshot->update();
			break;

#if defined(QMC2_EMUTYPE_MESS)
		case QMC2_SWINFO_PROJECTMESS_PAGE:
			if ( qmc2SoftwareList->currentItem != qmc2LastProjectMESSItem ) {
				if ( !qmc2ProjectMESS ) {
					QVBoxLayout *layout = new QVBoxLayout;
					layout->setContentsMargins(left, top, right, bottom);
					qmc2ProjectMESS = new MiniWebBrowser(tabProjectMESS);
					layout->addWidget(qmc2ProjectMESS);
					tabProjectMESS->setLayout(layout);
					connect(qmc2ProjectMESS->webViewBrowser, SIGNAL(loadStarted()), this, SLOT(projectMessLoadStarted()));
				}
				QString entryName = qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_NAME);
				QString entryTitle = qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_TITLE);
				QString listName = qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_LIST);
				QString projectMessUrl = qmc2Config->value(QMC2_FRONTEND_PREFIX + "ProjectMESS/BaseURL", QMC2_PROJECT_MESS_BASE_URL).toString().arg(entryName).arg(listName);
				qmc2ProjectMESS->webViewBrowser->settings()->setFontFamily(QWebSettings::StandardFont, qApp->font().family());
				qmc2ProjectMESS->webViewBrowser->settings()->setFontSize(QWebSettings::MinimumFontSize, qApp->font().pointSize());
				qmc2ProjectMESS->webViewBrowser->setStatusTip(tr("ProjectMESS page for '%1' / '%2'").arg(listName).arg(entryTitle));
				if ( !qmc2ProjectMESSCache.contains(listName + "_" + entryName) ) {
					QColor color = qmc2ProjectMESS->webViewBrowser->palette().color(QPalette::WindowText);
					qmc2ProjectMESS->webViewBrowser->setHtml(
								QString("<html><head></head><body><center><p><font color=\"#%1%2%3\"<b>").arg(color.red()).arg(color.green()).arg(color.blue()) +
									tr("Fetching ProjectMESS page for '%1' / '%2', please wait...").arg(listName).arg(entryTitle) + "</font></b></p><p>" +
									QString("(<a href=\"%1\">%1</a>)").arg(projectMessUrl) + "</p></center></body></html>",
								QUrl(projectMessUrl)
								);
					connect(qmc2ProjectMESS->webViewBrowser, SIGNAL(loadFinished(bool)), this, SLOT(projectMessLoadFinished(bool)));
					qmc2ProjectMESS->webViewBrowser->load(QUrl(projectMessUrl));
				} else {
					// FIXME: There's currently a bug in QWebView::setHtml() so that it executes JavaScript twice.
					qmc2ProjectMESS->webViewBrowser->setHtml(QString(QMC2_UNCOMPRESS(*qmc2ProjectMESSCache[listName + "_" + entryName])), QUrl(projectMessUrl));
					qmc2ProjectMESS->webViewBrowser->load(QUrl(projectMessUrl));
				}
				qmc2ProjectMESS->homeUrl = QUrl(projectMessUrl);
				qmc2LastProjectMESSItem = qmc2SoftwareList->currentItem;
			}
			break;
#endif

		case QMC2_SWINFO_NOTES_PAGE:
			if ( qmc2SoftwareList->currentItem != qmc2LastSoftwareNotesItem ) {
				if ( !qmc2SoftwareNotesEditor ) {
					QVBoxLayout *layout = new QVBoxLayout;
					layout->setContentsMargins(left, top, right, bottom);
					qmc2SoftwareNotesEditor = new HtmlEditor(tabNotes);
					layout->addWidget(qmc2SoftwareNotesEditor);
					tabNotes->setLayout(layout);
				} else
					qmc2SoftwareNotesEditor->save();
#if defined(QMC2_EMUTYPE_MAME)
				QString softwareNotesFolder = qmc2Config->value("MAME/FilesAndDirectories/SoftwareNotesFolder").toString();
				QString softwareNotesTemplate = qmc2Config->value("MAME/FilesAndDirectories/SoftwareNotesTemplate").toString();
				bool useSoftwareNotesTemplate = qmc2Config->value("MAME/FilesAndDirectories/UseSoftwareNotesTemplate").toBool();
#elif defined(QMC2_EMUTYPE_MESS)
				QString softwareNotesFolder = qmc2Config->value("MESS/FilesAndDirectories/SoftwareNotesFolder").toString();
				QString softwareNotesTemplate = qmc2Config->value("MESS/FilesAndDirectories/SoftwareNotesTemplate").toString();
				bool useSoftwareNotesTemplate = qmc2Config->value("MESS/FilesAndDirectories/UseSoftwareNotesTemplate").toBool();
#endif
				QString fileName = softwareNotesFolder + qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_LIST) + "/" + qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_NAME) + ".html";
				qmc2SoftwareNotesEditor->setCurrentFileName(fileName);
				if ( QFile::exists(fileName) ) {
					QTimer::singleShot(25, qmc2SoftwareNotesEditor, SLOT(loadCurrent()));
				} else {
					if ( useSoftwareNotesTemplate ) {
						qmc2SoftwareNotesEditor->templateMap.clear();
						qmc2SoftwareNotesEditor->templateMap["$SOFTWARE_TITLE$"] = qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_TITLE);
						qmc2SoftwareNotesEditor->templateMap["$SOFTWARE_NAME$"] = qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_NAME);
						qmc2SoftwareNotesEditor->templateMap["$SOFTWARE_LIST$"] = qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_LIST);
						qmc2SoftwareNotesEditor->templateMap["$SOFTWARE_PUBLISHER$"] = qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_PUBLISHER);
						qmc2SoftwareNotesEditor->templateMap["$SOFTWARE_YEAR$"] = qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_YEAR);
						qmc2SoftwareNotesEditor->setCurrentTemplateName(softwareNotesTemplate);
						QTimer::singleShot(25, qmc2SoftwareNotesEditor, SLOT(loadCurrentTemplate()));
					} else
						qmc2SoftwareNotesEditor->fileNew();
				}
				qmc2SoftwareNotesEditor->setCurrentFileName(fileName);
				qmc2LastSoftwareNotesItem = qmc2SoftwareList->currentItem;
			}
			break;

		default:
			break;
	}
}

void MainWindow::scrollToCurrentItem()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::scrollToCurrentItem()");
#endif

  QTreeWidgetItem *ci;

  if ( qmc2CurrentItem )
    ci = qmc2CurrentItem;
  else
    ci = treeWidgetGamelist->currentItem();

  if ( ci ) {
    if ( ci->text(QMC2_GAMELIST_COLUMN_GAME) == tr("Waiting for data...") )
      return;

    switch ( stackedWidgetView->currentIndex() ) {
      case QMC2_VIEWHIERARCHY_INDEX:
        ci = qmc2HierarchyItemMap[ci->child(0)->text(QMC2_GAMELIST_COLUMN_ICON)];
        if ( ci ) {
          treeWidgetHierarchy->clearSelection();
          treeWidgetHierarchy->setCurrentItem(ci);
          treeWidgetHierarchy->scrollToItem(ci, qmc2CursorPositioningMode);
        }
        break;

#if defined(QMC2_EMUTYPE_MAME)
      case QMC2_VIEWCATEGORY_INDEX:
        ci = qmc2CategoryItemMap[ci->child(0)->text(QMC2_GAMELIST_COLUMN_ICON)];
        if ( ci ) {
          treeWidgetCategoryView->clearSelection();
          treeWidgetCategoryView->setCurrentItem(ci);
          treeWidgetCategoryView->scrollToItem(ci, qmc2CursorPositioningMode);
        }
        break;

      case QMC2_VIEWVERSION_INDEX:
        ci = qmc2VersionItemMap[ci->child(0)->text(QMC2_GAMELIST_COLUMN_ICON)];
        if ( ci ) {
          treeWidgetVersionView->clearSelection();
          treeWidgetVersionView->setCurrentItem(ci);
          treeWidgetVersionView->scrollToItem(ci, qmc2CursorPositioningMode);
        }
        break;
#endif

      case QMC2_VIEWGAMELIST_INDEX:
      default:
        qmc2CheckItemVisibility = false;
        treeWidgetGamelist->clearSelection();
        if ( !qmc2ReloadActive )
          treeWidgetGamelist->setCurrentItem(ci);
        treeWidgetGamelist->scrollToItem(ci, qmc2CursorPositioningMode);
        break;
    }
    if ( !qmc2ReloadActive && ci )
      ci->setSelected(true);
  }
}

void MainWindow::checkCurrentSearchSelection()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::checkCurrentSearchSelection()");
#endif

  listWidgetSearch->setCurrentIndex(QModelIndex());
  listWidgetSearch->clearSelection();

  if ( !qmc2CurrentItem )
    return;

  listWidgetSearch->blockSignals(true);
  QList<QListWidgetItem *> searchMatches = listWidgetSearch->findItems(qmc2CurrentItem->text(QMC2_GAMELIST_COLUMN_GAME), Qt::MatchExactly);
  if ( searchMatches.count() > 0 ) {
    QListWidgetItem *matchedItem = searchMatches[0];
    if ( matchedItem != NULL ) {
      listWidgetSearch->setCurrentItem(matchedItem, QItemSelectionModel::ClearAndSelect);
      listWidgetSearch->scrollToItem(matchedItem, qmc2CursorPositioningMode);
      qApp->processEvents();
    }
  }
  listWidgetSearch->blockSignals(false);
}

void MainWindow::checkCurrentFavoritesSelection()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::checkCurrentFavoritesSelection()");
#endif

  listWidgetFavorites->setCurrentIndex(QModelIndex());
  listWidgetFavorites->clearSelection();

  if ( !qmc2CurrentItem )
    return;

  listWidgetFavorites->blockSignals(true);
  QList<QListWidgetItem *> favoritesMatches = listWidgetFavorites->findItems(qmc2CurrentItem->text(QMC2_GAMELIST_COLUMN_GAME), Qt::MatchExactly);
  if ( favoritesMatches.count() > 0 ) {
    QListWidgetItem *matchedItem = favoritesMatches[0];
    if ( matchedItem != NULL ) {
      listWidgetFavorites->setCurrentItem(matchedItem, QItemSelectionModel::ClearAndSelect);
      listWidgetFavorites->scrollToItem(matchedItem, qmc2CursorPositioningMode);
      qApp->processEvents();
    }
  }
  listWidgetFavorites->setFocus();
  listWidgetFavorites->blockSignals(false);
}

void MainWindow::checkCurrentPlayedSelection()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::checkCurrentPlayedSelection()");
#endif

  listWidgetPlayed->setCurrentIndex(QModelIndex());
  listWidgetPlayed->clearSelection();

  if ( !qmc2CurrentItem )
    return;

  listWidgetPlayed->blockSignals(true);
  QList<QListWidgetItem *> playedMatches = listWidgetPlayed->findItems(qmc2CurrentItem->text(QMC2_GAMELIST_COLUMN_GAME), Qt::MatchExactly);
  if ( playedMatches.count() > 0 ) {
    QListWidgetItem *matchedItem = playedMatches[0];
    if ( matchedItem != NULL ) {
      listWidgetPlayed->setCurrentItem(matchedItem, QItemSelectionModel::ClearAndSelect);
      listWidgetPlayed->scrollToItem(matchedItem, qmc2CursorPositioningMode);
      qApp->processEvents();
    }
  }
  listWidgetPlayed->setFocus();
  listWidgetPlayed->blockSignals(false);
}

void MainWindow::softwareLoadInterrupted()
{
  on_tabWidgetGameDetail_currentChanged(qmc2DetailSetup->appliedDetailList.indexOf(QMC2_SOFTWARE_LIST_INDEX));
}

void MainWindow::on_tabWidgetGameDetail_currentChanged(int currentIndex)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_tabWidgetGameDetail_currentChanged(int currentIndex = " + QString::number(currentIndex) + ")");
#endif

  if ( !qmc2CurrentItem || qmc2EarlyReloadActive ) {
#if defined(QMC2_EMUTYPE_MAME)
    qmc2LastGameInfoItem = qmc2LastEmuInfoItem = qmc2LastConfigItem = qmc2LastSoftwareListItem = NULL;
#else
    qmc2LastGameInfoItem = qmc2LastEmuInfoItem = qmc2LastConfigItem = qmc2LastDeviceConfigItem = qmc2LastSoftwareListItem = NULL;
#endif
#if QMC2_OPENGL == 1
    // images painted through OpenGL need extra "clear()'s", otherwise garbage is displayed
    switch ( qmc2DetailSetup->appliedDetailList[currentIndex] ) {
      case QMC2_PREVIEW_INDEX: {
          QPainter pPreview(qmc2Preview);
          qmc2Preview->drawCenteredImage(0, &pPreview);
        }
        break;

      case QMC2_FLYER_INDEX: {
          QPainter pFlyer(qmc2Flyer);
          qmc2Flyer->drawCenteredImage(0, &pFlyer);
        }
        break;

      case QMC2_CABINET_INDEX: {
          QPainter pCabinet(qmc2Cabinet);
          qmc2Cabinet->drawCenteredImage(0, &pCabinet);
        }
        break;

      case QMC2_CONTROLLER_INDEX: {
          QPainter pController(qmc2Controller);
          qmc2Controller->drawCenteredImage(0, &pController);
        }
        break;

      case QMC2_MARQUEE_INDEX: {
          QPainter pMarquee(qmc2Marquee);
          qmc2Marquee->drawCenteredImage(0, &pMarquee);
        }
        break;

      case QMC2_TITLE_INDEX: {
          QPainter pTitle(qmc2Title);
          qmc2Title->drawCenteredImage(0, &pTitle);
        }
        break;

      case QMC2_PCB_INDEX: {
          QPainter pPCB(qmc2PCB);
          qmc2PCB->drawCenteredImage(0, &pPCB);
        }
        break;

      default:
        break;
    }
#endif
    return;
  }

  // paranoia :)
  QTreeWidgetItem *ci = treeWidgetGamelist->currentItem();
  if ( !ci )
    return;
  if ( !ci->isSelected() )
    return;
  QTreeWidgetItem *topLevelItem = ci;
  while ( topLevelItem->parent() )
    topLevelItem = topLevelItem->parent();
  if ( topLevelItem )
    qmc2CurrentItem = topLevelItem;
  else
    return;
  if ( qmc2CurrentItem->childCount() <= 0 )
    return;
  if ( qmc2CurrentItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON) == tr("Waiting for data...") )
    return;
 
  // show / hide game status indicator
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/GameStatusIndicator").toBool() ) {
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/GameStatusIndicatorOnlyWhenRequired").toBool() ) {
      if ( hSplitter->sizes()[0] == 0 || tabWidgetGamelist->currentIndex() != QMC2_GAMELIST_INDEX) {
        labelGameStatus->setVisible(true);
      } else {
        labelGameStatus->setVisible(false);
      }
    } else {
      labelGameStatus->setVisible(true);
    }
  } else
    labelGameStatus->setVisible(false);

  // setup status indicator color
  switch ( qmc2CurrentItem->whatsThis(QMC2_GAMELIST_COLUMN_GAME).at(0).toAscii() ) {
    case QMC2_ROMSTATE_CHAR_C:
      labelGameStatus->setPalette(qmc2StatusColorGreen);
      break;

    case QMC2_ROMSTATE_CHAR_M:
      labelGameStatus->setPalette(qmc2StatusColorYellowGreen);
      break;

    case QMC2_ROMSTATE_CHAR_I:
      labelGameStatus->setPalette(qmc2StatusColorRed);
      break;

    case QMC2_ROMSTATE_CHAR_N:
      labelGameStatus->setPalette(qmc2StatusColorGrey);
      break;

    case QMC2_ROMSTATE_CHAR_U:
      labelGameStatus->setPalette(qmc2StatusColorBlue);
    default:
      break;
  }

  // 'special widgets': switch back to the default page, if applicable
  switch ( qmc2DetailSetup->appliedDetailList[tabWidgetGameDetail->currentIndex()] )
  {
	  case QMC2_SOFTWARE_LIST_INDEX:
		  if ( qmc2SoftwareList && qmc2CurrentItem == qmc2LastSoftwareListItem )
			  qmc2SoftwareList->on_toolButtonToggleSoftwareInfo_clicked(qmc2SoftwareList->toolButtonToggleSoftwareInfo->isChecked());
		  else
			  stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_DEFAULT_PAGE);
		  break;

	  default:
		  stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_DEFAULT_PAGE);
		  break;
  }

  int left, top, right, bottom;
  switch ( qmc2DetailSetup->appliedDetailList[currentIndex] ) {
#if defined(QMC2_YOUTUBE_ENABLED)
    case QMC2_YOUTUBE_INDEX:
      if ( qmc2YouTubeVideoInfoMap.isEmpty() )
        loadYouTubeVideoInfoMap();
      if ( qmc2CurrentItem != qmc2LastYouTubeItem ) {
          tabYouTube->setUpdatesEnabled(false);
          if ( qmc2YouTubeWidget ) {
            qmc2YouTubeWidget->saveSettings();
            qmc2YouTubeWidget->forcedExit = true;
            if ( qmc2YouTubeWidget->videoPlayer->isPlaying() || qmc2YouTubeWidget->videoPlayer->isPaused() )
              qmc2YouTubeWidget->videoPlayer->stop();
            QLayout *vbl = tabYouTube->layout();
            if ( vbl ) delete vbl;
            qmc2YouTubeWidget->close();
            delete qmc2YouTubeWidget;
            qmc2YouTubeWidget = NULL;
          }
          gridLayout->getContentsMargins(&left, &top, &right, &bottom);
          QVBoxLayout *layout = new QVBoxLayout;
          layout->setContentsMargins(left, top, right, bottom);
	  QString setID = qmc2CurrentItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON);
          qmc2YouTubeWidget = new YouTubeVideoPlayer(setID, qmc2GamelistDescriptionMap[setID], tabYouTube);
          layout->addWidget(qmc2YouTubeWidget);
          tabYouTube->setLayout(layout);
          qmc2YouTubeWidget->show();
          qmc2LastYouTubeItem = qmc2CurrentItem;
          tabYouTube->setUpdatesEnabled(true);
      }
      break;
#endif

    case QMC2_SOFTWARE_LIST_INDEX:
#if defined(QMC2_YOUTUBE_ENABLED)
      if ( qmc2YouTubeWidget )
	      qmc2YouTubeWidget->videoOverlayWidget->clearMessage();
#endif
      if ( qmc2SoftwareList ) {
        if ( qmc2SoftwareList->isLoading ) {
          qmc2SoftwareList->interruptLoad = true;
          qmc2LastSoftwareListItem = NULL;
	  QTimer::singleShot(0, this, SLOT(softwareLoadInterrupted()));
	  return;
        }
      }
      if ( qmc2CurrentItem != qmc2LastSoftwareListItem ) {
        tabSoftwareList->setUpdatesEnabled(false);
        if ( qmc2SoftwareList ) {
          qmc2SoftwareList->save();
          QLayout *vbl = tabSoftwareList->layout();
          if ( vbl ) delete vbl;
          delete qmc2SoftwareList;
          qmc2SoftwareList = NULL;
        }
        QString machineName = qmc2CurrentItem->child(0)->text(QMC2_MACHINELIST_COLUMN_ICON);
        gridLayout->getContentsMargins(&left, &top, &right, &bottom);
        QVBoxLayout *layout = new QVBoxLayout;
        layout->setContentsMargins(left, top, right, bottom);
        qmc2SoftwareList = new SoftwareList(machineName, tabSoftwareList);
        layout->addWidget(qmc2SoftwareList);
        tabSoftwareList->setLayout(layout);
        qmc2LastSoftwareListItem = qmc2CurrentItem;
        qmc2SoftwareList->show();
        tabSoftwareList->setUpdatesEnabled(true);
        qmc2SoftwareList->load();
      }
      break;

#if defined(QMC2_EMUTYPE_MESS)
    case QMC2_DEVICE_INDEX:
#if defined(QMC2_YOUTUBE_ENABLED)
      if ( qmc2YouTubeWidget )
	      qmc2YouTubeWidget->videoOverlayWidget->clearMessage();
#endif
      if ( qmc2CurrentItem != qmc2LastDeviceConfigItem ) {
        tabDevices->setUpdatesEnabled(false);
        if ( qmc2MESSDeviceConfigurator ) {
          qmc2MESSDeviceConfigurator->save();
          QLayout *vbl = tabDevices->layout();
          if ( vbl ) delete vbl;
          delete qmc2MESSDeviceConfigurator;
          qmc2MESSDeviceConfigurator = NULL;
        }
        gridLayout->getContentsMargins(&left, &top, &right, &bottom);
        QString machineName = qmc2CurrentItem->child(0)->text(QMC2_MACHINELIST_COLUMN_ICON);
        QVBoxLayout *layout = new QVBoxLayout;
        layout->setContentsMargins(left, top, right, bottom);
        qmc2MESSDeviceConfigurator = new MESSDeviceConfigurator(machineName, tabDevices);
	QTimer::singleShot(0, qmc2MESSDeviceConfigurator, SLOT(load()));
        layout->addWidget(qmc2MESSDeviceConfigurator);
        if ( !tabDevices->layout() )
          tabDevices->setLayout(layout);
        qmc2MESSDeviceConfigurator->show();
        qmc2LastDeviceConfigItem = qmc2CurrentItem;
        tabDevices->setUpdatesEnabled(true);
      }
      break;

    case QMC2_MESS_WIKI_INDEX:
#if defined(QMC2_YOUTUBE_ENABLED)
      if ( qmc2YouTubeWidget )
	      qmc2YouTubeWidget->videoOverlayWidget->clearMessage();
#endif
      if ( qmc2CurrentItem != qmc2LastMAWSItem ) {
        tabMAWS->setUpdatesEnabled(false);
        if ( qmc2MAWSLookup ) {
          QLayout *vbl = tabMAWS->layout();
          if ( vbl ) delete vbl;
          delete qmc2MAWSLookup;
          qmc2MAWSLookup = NULL;
        }
        gridLayout->getContentsMargins(&left, &top, &right, &bottom);
        QVBoxLayout *layout = new QVBoxLayout;
        layout->setContentsMargins(left, top, right, bottom);
        qmc2MAWSLookup = new MiniWebBrowser(tabMAWS);
        qmc2MAWSLookup->webViewBrowser->settings()->setFontFamily(QWebSettings::StandardFont, qApp->font().family());
        qmc2MAWSLookup->webViewBrowser->settings()->setFontSize(QWebSettings::MinimumFontSize, qApp->font().pointSize());
        qmc2MAWSLookup->webViewBrowser->setStatusTip(tr("MESS wiki page for all drivers"));
        layout->addWidget(qmc2MAWSLookup);
        tabMAWS->setLayout(layout);
        QString messWikiUrl = QMC2_MESS_WIKI_BASE_URL;
        QColor color = qmc2MAWSLookup->webViewBrowser->palette().color(QPalette::WindowText);
        QString driverName = qmc2Gamelist->lookupDriverName(qmc2CurrentItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON));
        if ( driverName.isEmpty() ) {
          if ( qmc2MAWSCache.contains("QMC2_MESS_WIKI_ALL_DRIVERS") )
            qmc2MAWSLookup->webViewBrowser->setHtml(QString(QMC2_UNCOMPRESS(*qmc2MAWSCache["QMC2_MESS_WIKI_ALL_DRIVERS"])), QUrl(messWikiUrl));
          else
            qmc2MAWSLookup->webViewBrowser->setHtml(
                                    QString("<html><head></head><body><center><p><font color=\"#%1%2%3\"<b>").arg(color.red()).arg(color.green()).arg(color.blue()) +
                                    tr("Fetching MESS wiki page for all drivers, please wait...") +
                                    "</font></b></p><p>" + QString("(<a href=\"%1\">%1</a>)").arg(messWikiUrl) + "</p></center></body></html>",
                                    QUrl(messWikiUrl));
          qmc2MAWSLookup->webViewBrowser->load(QUrl(messWikiUrl));
	} else if ( !qmc2MAWSCache.contains(driverName) ) {
          qmc2MAWSLookup->webViewBrowser->setStatusTip(tr("MESS wiki page for driver '%1'").arg(driverName));
          messWikiUrl = QString(QMC2_MESS_WIKI_PATTERN_URL).arg(driverName);
          qmc2MAWSLookup->webViewBrowser->setHtml(
                                  QString("<html><head></head><body><center><p><font color=\"#%1%2%3\"<b>").arg(color.red()).arg(color.green()).arg(color.blue()) +
                                  tr("Fetching MESS wiki page for driver '%1', please wait...").arg(driverName) +
                                  "</font></b></p><p>" + QString("(<a href=\"%1\">%1</a>)").arg(messWikiUrl) + "</p></center></body></html>",
                                  QUrl(messWikiUrl));
          qmc2MAWSLookup->webViewBrowser->load(QUrl(messWikiUrl));
        } else {
          // FIXME: There's currently a bug in QWebView::setHtml() so that it executes JavaScript twice.
          qmc2MAWSLookup->webViewBrowser->setStatusTip(tr("MESS wiki page for driver '%1'").arg(driverName));
          messWikiUrl = QString(QMC2_MESS_WIKI_PATTERN_URL).arg(driverName);
          qmc2MAWSLookup->webViewBrowser->setHtml(QString(QMC2_UNCOMPRESS(*qmc2MAWSCache[driverName])), QUrl(messWikiUrl));
          qmc2MAWSLookup->webViewBrowser->load(QUrl(messWikiUrl));
        }
        qmc2LastMAWSItem = qmc2CurrentItem;
        connect(qmc2MAWSLookup->webViewBrowser, SIGNAL(loadFinished(bool)), this, SLOT(messWikiLoadFinished(bool)));
        connect(qmc2MAWSLookup->webViewBrowser, SIGNAL(loadStarted()), this, SLOT(messWikiLoadStarted()));
        tabMAWS->setUpdatesEnabled(true);
      }
      break;

#elif defined(QMC2_EMUTYPE_MAME)
    case QMC2_MAWS_INDEX:
#if defined(QMC2_YOUTUBE_ENABLED)
      if ( qmc2YouTubeWidget )
	      qmc2YouTubeWidget->videoOverlayWidget->clearMessage();
#endif
      if ( qmc2CurrentItem != qmc2LastMAWSItem ) {
        tabMAWS->setUpdatesEnabled(false);
        if ( qmc2MAWSLookup ) {
          QLayout *vbl = tabMAWS->layout();
          if ( vbl ) delete vbl;
          delete qmc2MAWSLookup;
          qmc2MAWSLookup = NULL;
        }
        QString gameName = qmc2CurrentItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON);
        gridLayout->getContentsMargins(&left, &top, &right, &bottom);
        QVBoxLayout *layout = new QVBoxLayout;
        layout->setContentsMargins(left, top, right, bottom);
        qmc2MAWSLookup = new MiniWebBrowser(tabMAWS);
        qmc2MAWSLookup->webViewBrowser->settings()->setFontFamily(QWebSettings::StandardFont, qApp->font().family());
        qmc2MAWSLookup->webViewBrowser->settings()->setFontSize(QWebSettings::MinimumFontSize, qApp->font().pointSize());
        qmc2MAWSLookup->webViewBrowser->setStatusTip(tr("MAWS page for '%1'").arg(qmc2GamelistDescriptionMap[gameName]));
        layout->addWidget(qmc2MAWSLookup);
        tabMAWS->setLayout(layout);
        QString mawsUrl = qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/BaseURL", QMC2_MAWS_BASE_URL).toString().arg(gameName);

        menuMAWSQuickLinks = NULL;
        toolButtonMAWSQuickLinks = NULL;

        // lookup in disk cache first
        bool foundInDiskCache = false;
        QDir mawsCacheDir(qmc2Config->value("MAME/FilesAndDirectories/MAWSCacheDirectory").toString());
        if ( mawsCacheDir.exists(gameName + ".wc") ) {
          QFile mawsCacheFile(mawsCacheDir.filePath(gameName + ".wc"));
          if ( mawsCacheFile.open(QIODevice::ReadOnly) ) {
            mawsCacheFile.readLine();
            QString mawsCacheAge = QString(mawsCacheFile.readLine().trimmed()).split('\t')[1];
            if ( QDateTime::currentDateTime().toTime_t() - mawsCacheAge.toULong() < QMC2_MAWS_MAX_CACHE_AGE ) {
              if ( !qmc2MAWSCache.contains(gameName) ) {
                QByteArray data = mawsCacheFile.read(QMC2_ONE_MEGABYTE);
#if defined(QMC2_WC_COMPRESSION_ENABLED)
		QByteArray uncompressedData = QMC2_UNCOMPRESS(data);
                qmc2MAWSLookup->webViewBrowser->setHtml(QString::fromLatin1(uncompressedData.constData()), QUrl(mawsUrl));
#else
                qmc2MAWSLookup->webViewBrowser->setHtml(QString(data), QUrl(mawsUrl));
#endif
                foundInDiskCache = true;
              }
            }
            mawsCacheFile.close();
          }
        }

        if ( !foundInDiskCache ) {
          // now check in memory cache and fetch data if unavailable
          if ( !qmc2MAWSCache.contains(gameName) ) {
            QColor color = qmc2MAWSLookup->webViewBrowser->palette().color(QPalette::WindowText);
            qmc2MAWSLookup->webViewBrowser->setHtml(
                                    QString("<html><head></head><body><center><p><font color=\"#%1%2%3\"<b>").arg(color.red()).arg(color.green()).arg(color.blue()) +
                                    tr("Fetching MAWS page for '%1', please wait...").arg(qmc2GamelistDescriptionMap[gameName]) +
                                    "</font></b></p><p>" + QString("(<a href=\"%1\">%1</a>)").arg(mawsUrl) + "</p></center></body></html>",
                                    QUrl(mawsUrl));
            qmc2MAWSLookup->webViewBrowser->load(QUrl(mawsUrl));
          } else {
            qmc2MAWSLookup->webViewBrowser->setHtml(QString(QMC2_UNCOMPRESS(*qmc2MAWSCache[gameName])), QUrl(mawsUrl));
            if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/QuickDownload", true).toBool() )
              QTimer::singleShot(QMC2_MAWS_QDL_DELAY, this, SLOT(createMawsQuickLinksMenu()));
          }
        }
        qmc2LastMAWSItem = qmc2CurrentItem;
        connect(qmc2MAWSLookup->webViewBrowser, SIGNAL(loadFinished(bool)), this, SLOT(mawsLoadFinished(bool)));
        connect(qmc2MAWSLookup->webViewBrowser, SIGNAL(loadStarted()), this, SLOT(mawsLoadStarted()));
        tabMAWS->setUpdatesEnabled(true);
      }
      break;
#endif

    case QMC2_CONFIG_INDEX:
#if defined(QMC2_YOUTUBE_ENABLED)
      if ( qmc2YouTubeWidget )
	      qmc2YouTubeWidget->videoOverlayWidget->clearMessage();
#endif
      if ( qmc2CurrentItem != qmc2LastConfigItem ) {
        QWidget *configWidget = qmc2DetailSetup->tabWidgetsMap[QMC2_CONFIG_INDEX];

        configWidget->setUpdatesEnabled(false);

        // save & cleanup existing game/machine specific emulator settings
        QString selectedEmulator;
        if ( qmc2EmulatorOptions ) {
          selectedEmulator = comboBoxEmuSelector->currentText();
          if ( selectedEmulator == tr("Default") || selectedEmulator.isEmpty() )
            qmc2Config->remove(qmc2EmulatorOptions->settingsGroup + "/SelectedEmulator");
          else
            qmc2Config->setValue(qmc2EmulatorOptions->settingsGroup + "/SelectedEmulator", selectedEmulator);
          qmc2EmulatorOptions->save();
          QLayout *vbl = configWidget->layout();
          if ( vbl ) delete vbl;
          delete labelEmuSelector;
          delete comboBoxEmuSelector;
          delete qmc2EmulatorOptions;
          delete pushButtonCurrentEmulatorOptionsExportToFile;
          delete pushButtonCurrentEmulatorOptionsImportFromFile;
          qmc2EmulatorOptions = NULL;
        }
        QString gameName = qmc2CurrentItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON);
        gridLayout->getContentsMargins(&left, &top, &right, &bottom);
        QVBoxLayout *layout = new QVBoxLayout;
        layout->setContentsMargins(left, top, right, bottom);
        configWidget->setLayout(layout);

        // emulator selector (default, or one of the registered emulators)
#if defined(QMC2_EMUTYPE_MAME)
        labelEmuSelector = new QLabel(tr("Emulator for this game") + ":", configWidget);
#elif defined(QMC2_EMUTYPE_MESS)
        labelEmuSelector = new QLabel(tr("Emulator for this machine") + ":", configWidget);
#endif
        labelEmuSelector->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        comboBoxEmuSelector = new QComboBox(configWidget);
        comboBoxEmuSelector->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
#if defined(QMC2_EMUTYPE_MAME)
        qmc2Config->beginGroup("MAME/RegisteredEmulators");
#elif defined(QMC2_EMUTYPE_MESS)
        qmc2Config->beginGroup("MESS/RegisteredEmulators");
#endif
        QStringList registeredEmulators = qmc2Config->childGroups();
        qmc2Config->endGroup();
	registeredEmulators.prepend(tr("Default"));
        comboBoxEmuSelector->insertItems(0, registeredEmulators);
        QHBoxLayout *emuSelectorLayout = new QHBoxLayout();
        emuSelectorLayout->addWidget(labelEmuSelector);
        emuSelectorLayout->addWidget(comboBoxEmuSelector);
        layout->addLayout(emuSelectorLayout);
        connect(comboBoxEmuSelector, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(on_emuSelector_currentIndexChanged(const QString &)));

        // (default) emulator options
#if defined(QMC2_EMUTYPE_MAME)
        qmc2EmulatorOptions = new EmulatorOptions("MAME/Configuration/" + gameName, configWidget);
#elif defined(QMC2_EMUTYPE_MESS)
        qmc2EmulatorOptions = new EmulatorOptions("MESS/Configuration/" + gameName, configWidget);
#endif

        qmc2EmulatorOptions->load();

#if defined(QMC2_EMUTYPE_MAME)
        qmc2EmulatorOptions->addChoices("bios", getXmlChoices(gameName, "biosset", "name"));
#elif defined(QMC2_EMUTYPE_MESS)
        qmc2EmulatorOptions->addChoices("bios", getXmlChoices(gameName, "biosset", "name"));
        qmc2EmulatorOptions->addChoices("ramsize", getXmlChoices(gameName, "ramoption"));
#endif

        layout->addWidget(qmc2EmulatorOptions);

        // import/export buttons
#if defined(QMC2_SDLMAME) || defined(QMC2_SDLMESS) || defined(QMC2_MAME) || defined(QMC2_MESS)
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        pushButtonCurrentEmulatorOptionsExportToFile = new QPushButton(tr("Export to..."), this);
#if defined(QMC2_EMUTYPE_MAME)
        pushButtonCurrentEmulatorOptionsExportToFile->setToolTip(QObject::tr("Export game-specific MAME configuration"));
        pushButtonCurrentEmulatorOptionsExportToFile->setStatusTip(QObject::tr("Export game-specific MAME configuration"));
#elif defined(QMC2_EMUTYPE_MESS)
        pushButtonCurrentEmulatorOptionsExportToFile->setToolTip(QObject::tr("Export machine-specific MESS configuration"));
        pushButtonCurrentEmulatorOptionsExportToFile->setStatusTip(QObject::tr("Export machine-specific MESS configuration"));
#endif
        pushButtonCurrentEmulatorOptionsImportFromFile = new QPushButton(QObject::tr("Import from..."), this);
#if defined(QMC2_EMUTYPE_MAME)
        pushButtonCurrentEmulatorOptionsImportFromFile->setToolTip(QObject::tr("Import game-specific MAME configuration"));
        pushButtonCurrentEmulatorOptionsImportFromFile->setStatusTip(QObject::tr("Import game-specific MAME configuration"));
#elif defined(QMC2_EMUTYPE_MESS)
        pushButtonCurrentEmulatorOptionsImportFromFile->setToolTip(QObject::tr("Import machine-specific MESS configuration"));
        pushButtonCurrentEmulatorOptionsImportFromFile->setStatusTip(QObject::tr("Import machine-specific MESS configuration"));
#endif
        buttonLayout->addWidget(pushButtonCurrentEmulatorOptionsExportToFile);
        buttonLayout->addWidget(pushButtonCurrentEmulatorOptionsImportFromFile);
        layout->addLayout(buttonLayout);

        // import/export menus
        qmc2MainWindow->selectMenuCurrentEmulatorOptionsExportToFile = new QMenu(qmc2MainWindow->pushButtonCurrentEmulatorOptionsExportToFile);
        connect(qmc2MainWindow->selectMenuCurrentEmulatorOptionsExportToFile->addAction(QIcon(QString::fromUtf8(":/data/img/work.png")), tr("<inipath>/%1.ini").arg(gameName)), SIGNAL(triggered()), qmc2MainWindow, SLOT(on_pushButtonCurrentEmulatorOptionsExportToFile_clicked()));
        connect(qmc2MainWindow->selectMenuCurrentEmulatorOptionsExportToFile->addAction(QIcon(QString::fromUtf8(":/data/img/fileopen.png")), tr("Select file...")), SIGNAL(triggered()), qmc2MainWindow, SLOT(on_pushButtonCurrentEmulatorOptionsSelectExportFile_clicked()));
        qmc2MainWindow->pushButtonCurrentEmulatorOptionsExportToFile->setMenu(qmc2MainWindow->selectMenuCurrentEmulatorOptionsExportToFile);
        qmc2MainWindow->selectMenuCurrentEmulatorOptionsImportFromFile = new QMenu(qmc2MainWindow->pushButtonCurrentEmulatorOptionsImportFromFile);
        connect(qmc2MainWindow->selectMenuCurrentEmulatorOptionsImportFromFile->addAction(QIcon(QString::fromUtf8(":/data/img/work.png")), tr("<inipath>/%1.ini").arg(gameName)), SIGNAL(triggered()), qmc2MainWindow, SLOT(on_pushButtonCurrentEmulatorOptionsImportFromFile_clicked()));
        connect(qmc2MainWindow->selectMenuCurrentEmulatorOptionsImportFromFile->addAction(QIcon(QString::fromUtf8(":/data/img/fileopen.png")), tr("Select file...")), SIGNAL(triggered()), qmc2MainWindow, SLOT(on_pushButtonCurrentEmulatorOptionsSelectImportFile_clicked()));
        qmc2MainWindow->pushButtonCurrentEmulatorOptionsImportFromFile->setMenu(qmc2MainWindow->selectMenuCurrentEmulatorOptionsImportFromFile);
#endif
        qmc2EmulatorOptions->resizeColumnToContents(0);
        qmc2EmulatorOptions->pseudoConstructor();
        qmc2LastConfigItem = qmc2CurrentItem;

        // select the emulator to be used, if applicable
        if ( registeredEmulators.count() > 1 ) {
          if ( qmc2Config->contains(qmc2EmulatorOptions->settingsGroup + "/SelectedEmulator") ) {
            selectedEmulator = qmc2Config->value(qmc2EmulatorOptions->settingsGroup + "/SelectedEmulator").toString();
            if ( !selectedEmulator.isEmpty() && registeredEmulators.contains(selectedEmulator) ) {
              int emuIndex = comboBoxEmuSelector->findText(selectedEmulator);
              if ( emuIndex >= 0 )
                comboBoxEmuSelector->setCurrentIndex(emuIndex);
              else
                comboBoxEmuSelector->setCurrentIndex(0);
            }
          }
        }

	// finally show the widgets...
        labelEmuSelector->show();
        comboBoxEmuSelector->show();
        qmc2EmulatorOptions->show();
        pushButtonCurrentEmulatorOptionsExportToFile->show();
        pushButtonCurrentEmulatorOptionsImportFromFile->show();

        configWidget->setUpdatesEnabled(true);
        configWidget->update();

        qmc2EmulatorOptions->horizontalScrollBar()->setSliderPosition(qmc2EmulatorOptions->horizontalScrollPosition);
        qmc2EmulatorOptions->verticalScrollBar()->setSliderPosition(qmc2EmulatorOptions->verticalScrollPosition);
      }
      break;

    case QMC2_GAMEINFO_INDEX:
#if defined(QMC2_YOUTUBE_ENABLED)
      if ( qmc2YouTubeWidget )
	      qmc2YouTubeWidget->videoOverlayWidget->clearMessage();
#endif
      if ( qmc2CurrentItem != qmc2LastGameInfoItem ) {
        tabGameInfo->setUpdatesEnabled(false);
        QString gameName = qmc2CurrentItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON);
        if ( qmc2GameInfoDB.contains(gameName) || qmc2GameInfoDB.contains(qmc2ParentMap[gameName]) ) {
          // update game/machine info if it points to a different DB record
          bool updateInfo = true;
          QByteArray *newGameInfo = qmc2GameInfoDB[gameName];
          if ( !newGameInfo ) {
            // fall back to parent's game/machine info, if applicable
            newGameInfo = qmc2GameInfoDB[qmc2ParentMap[gameName]];
          }
          if ( qmc2LastGameInfoItem )
            if ( qmc2LastGameInfoItem->child(0) ) {
              QByteArray *oldGameInfo = qmc2GameInfoDB[qmc2LastGameInfoItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON)];
              if ( !oldGameInfo ) {
                // fall back to parent's game/machine info, if applicable
		oldGameInfo = qmc2GameInfoDB[qmc2ParentMap[qmc2LastGameInfoItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON)]];
              }
              updateInfo = (newGameInfo != oldGameInfo || !oldGameInfo);
            }
          if ( updateInfo ) {
            if ( newGameInfo ) {
              QString gameInfoText;
#if defined(QMC2_EMUTYPE_MESS)
              if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/CompressGameInfoDB").toBool() )
                gameInfoText = QString(QMC2_UNCOMPRESS(*newGameInfo));
              else
                gameInfoText = QString(*newGameInfo);
              textBrowserGameInfo->setHtml(messWikiToHtml(gameInfoText));
#else
              if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/CompressGameInfoDB").toBool() )
                gameInfoText = QString(QMC2_UNCOMPRESS(*newGameInfo)).replace(QRegExp(QString("((http|https|ftp)://%1)").arg(urlSectionRegExp)), QLatin1String("<a href=\"\\1\">\\1</a>"));
              else
                gameInfoText = QString(*newGameInfo).replace(QRegExp(QString("((http|https|ftp)://%1)").arg(urlSectionRegExp)), QLatin1String("<a href=\"\\1\">\\1</a>"));
              textBrowserGameInfo->setHtml(gameInfoText);
#endif
            } else
#if defined(QMC2_EMUTYPE_MESS)
              textBrowserGameInfo->setHtml("<h2>" + qmc2GamelistDescriptionMap[gameName] + "</h2>" + tr("<p>No data available</p>"));
#else
              textBrowserGameInfo->setHtml("<b>" + qmc2GamelistDescriptionMap[gameName] + "</b><p>" + tr("No data available"));
#endif
          }
        } else
#if defined(QMC2_EMUTYPE_MESS)
          textBrowserGameInfo->setHtml("<h2>" + qmc2GamelistDescriptionMap[gameName] + "</h2>" + tr("<p>No data available</p>"));
#else
          textBrowserGameInfo->setHtml("<b>" + qmc2GamelistDescriptionMap[gameName] + "</b><p>" + tr("No data available"));
#endif
        qmc2LastGameInfoItem = qmc2CurrentItem;
        tabGameInfo->setUpdatesEnabled(true);
      }
      break;

    case QMC2_EMUINFO_INDEX:
#if defined(QMC2_YOUTUBE_ENABLED)
      if ( qmc2YouTubeWidget )
	      qmc2YouTubeWidget->videoOverlayWidget->clearMessage();
#endif
      if ( qmc2CurrentItem != qmc2LastEmuInfoItem ) {
        tabEmuInfo->setUpdatesEnabled(false);
        QString gameName = qmc2CurrentItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON);
        if ( qmc2EmuInfoDB.contains(gameName) || qmc2EmuInfoDB.contains(qmc2ParentMap[gameName]) ) {
          // update emulator info if it points to a different DB record
          bool updateInfo = true;
          QByteArray *newEmuInfo = qmc2EmuInfoDB[gameName];
          if ( !newEmuInfo ) {
            // fall back to parent's emulator info, if applicable
            newEmuInfo = qmc2EmuInfoDB[qmc2ParentMap[gameName]];
          }
          if ( qmc2LastEmuInfoItem )
            if ( qmc2LastEmuInfoItem->child(0) ) {
              QByteArray *oldEmuInfo = qmc2EmuInfoDB[qmc2LastEmuInfoItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON)];
              if ( !oldEmuInfo ) {
                // fall back to parent's emulator info, if applicable
		oldEmuInfo = qmc2EmuInfoDB[qmc2ParentMap[qmc2LastEmuInfoItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON)]];
              }
              updateInfo = (newEmuInfo != oldEmuInfo || !oldEmuInfo);
            }
          if ( updateInfo ) {
            if ( newEmuInfo ) {
              if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/CompressEmuInfoDB").toBool() )
                textBrowserEmuInfo->setHtml(QString(QMC2_UNCOMPRESS(*newEmuInfo)).replace(QRegExp(QString("(\\w+://%1)").arg(urlSectionRegExp)), QLatin1String("<a href=\"\\1\">\\1</a>")));
              else
                textBrowserEmuInfo->setHtml(QString(*newEmuInfo).replace(QRegExp(QString("(\\w+://%1)").arg(urlSectionRegExp)), QLatin1String("<a href=\"\\1\">\\1</a>")));
            } else
              textBrowserEmuInfo->setHtml(tr("No data available"));
          }
        } else
          textBrowserEmuInfo->setHtml(tr("No data available"));
        qmc2LastEmuInfoItem = qmc2CurrentItem;
        tabEmuInfo->setUpdatesEnabled(true);
      }
      break;

    default:
#if defined(QMC2_YOUTUBE_ENABLED)
      if ( qmc2YouTubeWidget )
	      qmc2YouTubeWidget->videoOverlayWidget->clearMessage();
#endif
      // if local emulator options exits and they are no longer needed, close & destroy them...
      if ( qmc2EmulatorOptions ) {
        QWidget *configWidget = qmc2DetailSetup->tabWidgetsMap[QMC2_CONFIG_INDEX];
        QString selectedEmulator = comboBoxEmuSelector->currentText();
        if ( selectedEmulator == tr("Default") || selectedEmulator.isEmpty() )
          qmc2Config->remove(qmc2EmulatorOptions->settingsGroup + "/SelectedEmulator");
        else
          qmc2Config->setValue(qmc2EmulatorOptions->settingsGroup + "/SelectedEmulator", selectedEmulator);
        qmc2EmulatorOptions->save();
        QLayout *vbl = configWidget->layout();
        if ( vbl ) delete vbl;
        delete labelEmuSelector;
        delete comboBoxEmuSelector;
        delete qmc2EmulatorOptions;
        delete pushButtonCurrentEmulatorOptionsExportToFile;
        delete pushButtonCurrentEmulatorOptionsImportFromFile;
        qmc2EmulatorOptions = NULL;
      }
      qmc2LastConfigItem = NULL;
      break;
  }
}

QStringList &MainWindow::getXmlChoices(QString gameName, QString optionElement, QString optionAttribute)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: &MainWindow::getXmlChoices(QString gameName = %1, QString optionElement = %2, QString optionAttribute = %3)").arg(gameName).arg(optionElement).arg(optionAttribute));
#endif

	static QStringList xmlChoices;
	xmlChoices.clear();
	int i = qmc2XmlGamePositionMap[gameName];
	if ( i <= 0 ) {
#if defined(QMC2_EMUTYPE_MAME)
		QString s = "<game name=\"" + gameName + "\"";
#elif defined(QMC2_EMUTYPE_MESS)
		QString s = "<machine name=\"" + gameName + "\"";
#endif
		int xmlLinesCount = qmc2Gamelist->xmlLines.count();
		i = 0;
		while ( !qmc2Gamelist->xmlLines[i].contains(s) ) {
			i++;
			if ( i >= xmlLinesCount )
				break;
		}
		if ( i < xmlLinesCount )
			if ( qmc2Gamelist->xmlLines[i].contains(s) )
				qmc2XmlGamePositionMap[gameName] = i;
	}
	if ( i > 0 ) {
#if defined(QMC2_EMUTYPE_MAME)
		while ( !qmc2Gamelist->xmlLines[i].contains("</game>") ) {
#elif defined(QMC2_EMUTYPE_MESS)
		while ( !qmc2Gamelist->xmlLines[i].contains("</machine>") ) {
#endif
			QString xmlLine = qmc2Gamelist->xmlLines[i++].simplified();

			int index = xmlLine.indexOf("<" + optionElement);
			if ( index >= 0 ) {
				if ( optionAttribute.isEmpty() ) {
					index = xmlLine.indexOf(">", index);
					if ( index >= 0 ) {
						xmlLine.remove(0, index + 1);
						xmlLine.replace("</" + optionElement + ">", "");
						xmlChoices << xmlLine;
					}
				} else {
					QString prefix = optionAttribute + "=\"";
					index = xmlLine.indexOf(prefix);
					if ( index >= 0 ) {
						xmlLine.remove(0, index + prefix.length());
						index = xmlLine.indexOf("\"");
						if ( index >= 0 )
							xmlChoices << xmlLine.left(index);
					}
				}
			}
		}
	}
	if ( !xmlChoices.isEmpty() )
		xmlChoices.insert(0, QString());
	return xmlChoices;
}

void MainWindow::on_emuSelector_currentIndexChanged(const QString &text)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_emuSelector_currentIndexChanged(const QString &text = %1)").arg(text));
#endif

  if ( !qmc2EmulatorOptions )
    return;

  if ( text == tr("Default") ) {
    qmc2EmulatorOptions->setEnabled(true);
    pushButtonCurrentEmulatorOptionsExportToFile->setEnabled(true);
    pushButtonCurrentEmulatorOptionsImportFromFile->setEnabled(true);
  } else {
    qmc2EmulatorOptions->setEnabled(false);
    pushButtonCurrentEmulatorOptionsExportToFile->setEnabled(false);
    pushButtonCurrentEmulatorOptionsImportFromFile->setEnabled(false);
  }
}

void MainWindow::on_treeWidgetGamelist_itemActivated(QTreeWidgetItem *item, int column)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_treeWidgetGamelist_itemActivated(QTreeWidgetItem *item = %1, int column = %2)").arg((qulonglong)item).arg(column));
#endif

  if ( qmc2DemoModeDialog )
	  if ( qmc2DemoModeDialog->demoModeRunning )
		  return;

  qmc2StartEmbedded = false;
  if ( !qmc2IgnoreItemActivation ) {
	  switch ( qmc2DefaultLaunchMode ) {
#if defined(Q_WS_X11) || defined(Q_WS_WIN)
		  case QMC2_LAUNCH_MODE_EMBEDDED:
			  on_actionPlayEmbedded_activated();
			  break;
#endif
		  case QMC2_LAUNCH_MODE_INDEPENDENT:
		  default:
			  on_actionPlay_activated();
			  break;
	  }
  }
  qmc2IgnoreItemActivation = false;
}

void MainWindow::on_treeWidgetHierarchy_itemActivated(QTreeWidgetItem *item, int column)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_treeWidgetHierarchy_itemActivated(QTreeWidgetItem *item = %1, int column = %2)").arg((qulonglong)item).arg(column));
#endif

  if ( qmc2DemoModeDialog )
	  if ( qmc2DemoModeDialog->demoModeRunning )
		  return;

  qmc2StartEmbedded = false;
  if ( !qmc2IgnoreItemActivation ) {
	  switch ( qmc2DefaultLaunchMode ) {
#if defined(Q_WS_X11) || defined(Q_WS_WIN)
		  case QMC2_LAUNCH_MODE_EMBEDDED:
			  on_actionPlayEmbedded_activated();
			  break;
#endif
		  case QMC2_LAUNCH_MODE_INDEPENDENT:
		  default:
			  on_actionPlay_activated();
			  break;
	  }
  }
  qmc2IgnoreItemActivation = false;
}

void MainWindow::on_treeWidgetGamelist_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_treeWidgetGamelist_itemDoubleClicked(QTreeWidgetItem *item = %1, int column = %2)").arg((qulonglong)item).arg(column));
#endif

  if ( !qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/DoubleClickActivation").toBool() )
    qmc2IgnoreItemActivation = true;
}

void MainWindow::on_treeWidgetHierarchy_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_treeWidgetHierarchy_itemDoubleClicked(QTreeWidgetItem *item = %1, int column = %2)").arg((qulonglong)item).arg(column));
#endif

  if ( !qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/DoubleClickActivation").toBool() )
    qmc2IgnoreItemActivation = true;
}

void MainWindow::on_treeWidgetGamelist_itemExpanded(QTreeWidgetItem *item)
{
  if ( !item ) return;

  if ( qmc2ReloadActive ) {
    treeWidgetGamelist->collapseItem(item);
    log(QMC2_LOG_FRONTEND, tr("please wait for reload to finish and try again"));
    return;
  }

#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_treeWidgetGamelist_itemExpanded(QTreeWidgetItem *item = %1)").arg((qulonglong)item));
#endif

  if ( item->child(0) ) {
    if ( item->child(0)->text(QMC2_GAMELIST_COLUMN_GAME) == tr("Waiting for data...") )
      qmc2Gamelist->parseGameDetail(item);
  }
}

void MainWindow::on_treeWidgetGamelist_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_treeWidgetGamelist_currentItemChanged(QTreeWidgetItem *current = %1, QTreeWidgetItem *previous = %2").arg((qulonglong)current).arg((qulonglong)previous));
#endif

  // workaround for a Qt bug: when POS1/Home is pressed, QTreeWidget & QTreeView don't correctly select the first VISIBLE item,
  // if the top item is HIDDEN
  if ( current ) {
    if ( qmc2CheckItemVisibility ) {
      if ( current->isHidden() ) {
        int i = treeWidgetGamelist->indexOfTopLevelItem(current);
        if ( i >= 0 ) {
          while ( current->isHidden() && i < treeWidgetGamelist->topLevelItemCount() ) {
            current = treeWidgetGamelist->topLevelItem(++i);
            if ( current == NULL ) 
              break;
          }
          if ( current )
            treeWidgetGamelist->setCurrentItem(current);
        }
      }
    } else
      qmc2CurrentItem = current;
  }
  qmc2CheckItemVisibility = true;
  if ( qmc2UpdateDelay > 0 )
    updateTimer.start(qmc2UpdateDelay);
  else
    on_treeWidgetGamelist_itemSelectionChanged_delayed();
}

void MainWindow::on_treeWidgetHierarchy_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_treeWidgetHierarchy_currentItemChanged(QTreeWidgetItem *current = %1, QTreeWidgetItem *previous = %2").arg((qulonglong)current).arg((qulonglong)previous));
#endif

  qmc2CheckItemVisibility = false;
  if ( qmc2UpdateDelay > 0 )
    updateTimer.start(qmc2UpdateDelay);
  else
    on_treeWidgetGamelist_itemSelectionChanged_delayed();
}

void MainWindow::on_treeWidgetGamelist_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_treeWidgetGamelist_itemSelectionChanged()");
#endif

  if ( qmc2UpdateDelay > 0 )
    updateTimer.start(qmc2UpdateDelay);
  else
    on_treeWidgetGamelist_itemSelectionChanged_delayed();
}

void MainWindow::on_treeWidgetGamelist_itemSelectionChanged_delayed()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_treeWidgetGamelist_itemSelectionChanged_delayed()");
#endif

  updateTimer.stop();
  QList<QTreeWidgetItem *>selected;
  if ( qmc2HierarchySelectedItem != NULL )
    selected.append(qmc2HierarchySelectedItem);
  else
    selected = treeWidgetGamelist->selectedItems();
  if ( selected.count() == 0 )
    if ( treeWidgetGamelist->currentItem() )
      selected.append(treeWidgetGamelist->currentItem());
  if ( selected.count() > 0 ) {
    QTreeWidgetItem *topLevelItem = selected.at(0);
    while ( topLevelItem->parent() )
      topLevelItem = topLevelItem->parent();
    if ( topLevelItem ) {
      qmc2CurrentItem = topLevelItem;

      qmc2Preview->update();
      qmc2Flyer->update();
      qmc2Cabinet->update();
      qmc2Controller->update();
      qmc2Marquee->update();
      qmc2Title->update();
      qmc2PCB->update();

      on_tabWidgetGameDetail_currentChanged(tabWidgetGameDetail->currentIndex());
    }
  } else
    qmc2CurrentItem = NULL;
  qmc2HierarchySelectedItem = NULL;
}

void MainWindow::on_treeWidgetHierarchy_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_treeWidgetHierarchy_itemSelectionChanged()");
#endif

  qmc2HierarchySelectedItem = NULL;
  QList<QTreeWidgetItem *>selected = treeWidgetHierarchy->selectedItems();
  if ( selected.count() > 0 ) {
    if ( selected.at(0)->text(QMC2_GAMELIST_COLUMN_GAME) == tr("Waiting for data...") )
      return;
    qmc2HierarchySelectedItem = qmc2GamelistItemByDescriptionMap[selected.at(0)->text(QMC2_GAMELIST_COLUMN_GAME)];
    qmc2CheckItemVisibility = false;
    treeWidgetGamelist->setCurrentItem(qmc2HierarchySelectedItem);
  }
}

void MainWindow::on_treeWidgetEmulators_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_treeWidgetEmulators_customContextMenuRequested(const QPoint &p = ...)");
#endif

  QTreeWidgetItem *item = treeWidgetEmulators->itemAt(p);
  if ( item ) {
    treeWidgetEmulators->setItemSelected(item, true);
    qmc2EmulatorMenu->move(adjustedWidgetPosition(treeWidgetEmulators->viewport()->mapToGlobal(p), qmc2EmulatorMenu));
    qmc2EmulatorMenu->show();
  }
}

#if defined(Q_WS_X11) || defined(Q_WS_WIN)
void MainWindow::action_embedEmulator_triggered()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::action_embedEmulator_triggered()");
#endif

  qmc2StartEmbedded = false;

  QStringList gameList;
  QStringList idList;
#if defined(Q_WS_X11)
  QStringList statusList;
#endif

  QList<QTreeWidgetItem *> sl = treeWidgetEmulators->selectedItems();
  int i;
  for (i = 0; i < sl.count(); i++) {
    QTreeWidgetItem *item = sl[i];
    while ( item->parent() ) item = item->parent();
    gameList << item->text(QMC2_EMUCONTROL_COLUMN_GAME).split(":")[0];
    idList << item->text(QMC2_EMUCONTROL_COLUMN_NUMBER);
#if defined(Q_WS_X11)
    statusList << item->text(QMC2_EMUCONTROL_COLUMN_STATUS);
#endif
  }

  if ( sl.count() == 0 )
    if ( treeWidgetEmulators->currentItem() ) {
      QTreeWidgetItem *item = treeWidgetEmulators->currentItem();
      while ( item->parent() ) item = item->parent();
      gameList << item->text(QMC2_EMUCONTROL_COLUMN_GAME).split(":")[0];
      idList << item->text(QMC2_EMUCONTROL_COLUMN_NUMBER);
#if defined(Q_WS_X11)
      statusList << item->text(QMC2_EMUCONTROL_COLUMN_STATUS);
#endif
    }

  bool success = true;
  for (i = 0; i < gameList.count(); i++) {
    QString gameName = gameList[i];
    QString gameID = idList[i];
#if defined(Q_WS_X11)
    QString gameStatus = statusList[i];
#endif

    if ( gameName.isEmpty() || gameID.isEmpty() )
      continue;

    // check if the emulator window is already embedded
    bool found = false;
    int j;
    for (j = 0; j < tabWidgetEmbeddedEmulators->count() && !found; j++)
      found = tabWidgetEmbeddedEmulators->tabText(j).startsWith(QString("#%1 - ").arg(gameID));
    if ( found ) {                                                                  
      log(QMC2_LOG_FRONTEND, tr("emulator #%1 is already embedded").arg(gameID));
      tabWidgetGamelist->setCurrentIndex(tabWidgetGamelist->indexOf(widgetEmbeddedEmus));
      tabWidgetEmbeddedEmulators->setCurrentIndex(j - 1);
      continue;
    }

#if defined(Q_WS_X11)
    // run "xwininfo -root -all" to find the window ID
    QString command = QString(XSTR(QMC2_XWININFO));
    QStringList args, winIdList;
    args << "-root" << "-all";
    int xwininfoRetries = 0;
    while ( winIdList.count() < 1 && xwininfoRetries++ < QMC2_MAX_XWININFO_RETRIES ) {
      QProcess commandProc;
      bool commandProcStarted = false;
      int retries = 0;
      commandProc.start(command, args);
      bool started = commandProc.waitForStarted(QMC2_PROCESS_POLL_TIME);
      while ( !started && retries++ < QMC2_PROCESS_POLL_RETRIES ) {
        started = commandProc.waitForStarted(QMC2_PROCESS_POLL_TIME_LONG);
	qApp->processEvents();
      }
      if ( started ) {
        commandProcStarted = true;
        bool commandProcRunning = (commandProc.state() == QProcess::Running);
        while ( !commandProc.waitForFinished(QMC2_PROCESS_POLL_TIME) && commandProcRunning ) {
          qApp->processEvents();
          commandProcRunning = (commandProc.state() == QProcess::Running);
        }
      } else {
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't start XWININFO within a reasonable time frame, giving up"));
        break;
      }
      QStringList ssl = QString(commandProc.readAllStandardOutput()).split("\n");
#if defined(QMC2_EMUTYPE_MAME)
      QString regExp = QString("*MAME:*%1*\"QMC2-MAME-ID-%2\"*").arg(gameName).arg(gameID);
#elif defined(QMC2_EMUTYPE_MESS)
      QString regExp = QString("*MESS:*%1*\"QMC2-MESS-ID-%2\"*").arg(gameName).arg(gameID);
#else
      QString regExp;
#endif
      foreach (QString s, ssl) {
        if ( s.contains(QRegExp(regExp, Qt::CaseSensitive, QRegExp::Wildcard)) ) {
          s = s.trimmed().split(" ")[0];
          winIdList << s;
        }
      }
    }
#elif defined(Q_WS_WIN)
    QList<WId> winIdList;
    int wininfoRetries = 0;
    while ( winIdList.isEmpty() && wininfoRetries++ < QMC2_MAX_WININFO_RETRIES ) {
	Q_PID gamePid = qmc2ProcessManager->getPid(gameID.toInt());
	if ( gamePid ) {
#if defined(QMC2_EMUTYPE_MAME)
		HWND hwnd = winFindWindowHandleOfProcess(gamePid, "MAME:");
#elif defined(QMC2_EMUTYPE_MESS)
		HWND hwnd = winFindWindowHandleOfProcess(gamePid, "MESS:");
#endif
		if ( hwnd )
			winIdList << (WId)hwnd;
	}
	if ( winIdList.isEmpty() && wininfoRetries <= QMC2_MAX_WININFO_RETRIES )
		QTest::qWait(QMC2_WININFO_DELAY);
    }
#endif

#if defined(Q_WS_X11)
    if ( winIdList.count() > 1 )
      log(QMC2_LOG_FRONTEND, tr("WARNING: multiple windows for emulator #%1 found, choosing window ID %2 for embedding").arg(gameID).arg(winIdList[0]));
#elif defined(Q_WS_WIN)
    if ( winIdList.count() > 1 )
      log(QMC2_LOG_FRONTEND, tr("WARNING: multiple windows for emulator #%1 found, choosing window ID %2 for embedding").arg(gameID).arg((qulonglong)winIdList[0]));
#endif

    if ( !winIdList.isEmpty() ) {
      int embeddedEmusIndex = tabWidgetGamelist->indexOf(widgetEmbeddedEmus);
      if ( tabWidgetGamelist->currentIndex() != embeddedEmusIndex )
        qmc2LastListIndex = tabWidgetGamelist->currentIndex();
      if ( embeddedEmusIndex < 0 )
        tabWidgetGamelist->addTab(widgetEmbeddedEmus, QIcon(QString::fromUtf8(":/data/img/embed.png")), tr("Embedded emulators"));
#if defined(Q_WS_X11)
      log(QMC2_LOG_FRONTEND, tr("embedding emulator #%1, window ID = %2").arg(gameID).arg(winIdList[0]));
      Embedder *embedder = new Embedder(gameName, gameID, winIdList[0].toInt(0, 16), (gameStatus == tr("paused")), this, qmc2IconMap[gameName]);
#elif defined(Q_WS_WIN)
      log(QMC2_LOG_FRONTEND, tr("embedding emulator #%1, window ID = %2").arg(gameID).arg((qulonglong)winIdList[0]));
      Embedder *embedder = new Embedder(gameName, gameID, winIdList[0], false, this, qmc2IconMap[gameName]);
#endif
      connect(embedder, SIGNAL(closing()), this, SLOT(closeEmbeddedEmuTab()));
      tabWidgetEmbeddedEmulators->addTab(embedder, QString("#%1 - %2").arg(gameID).arg(qmc2GamelistDescriptionMap[gameName]));

      // serious hack to access the tab bar without sub-classing from QTabWidget ;)
      QTabBar *tabBar = tabWidgetEmbeddedEmulators->findChild<QTabBar *>();
      if ( tabBar ) {
        int index = tabWidgetEmbeddedEmulators->indexOf(embedder);
        tabBar->tabButton(index, QTabBar::RightSide)->setToolTip(tr("Release emulator"));
	// replace the default 'X' icon with something more suitable (the only way I found to accomplish this is by setting a style-sheet)
	tabBar->setStyleSheet("QTabBar::close-button { image: url(:/data/img/release.png); subcontrol-position: right; } QTabBar::close-button:hover { image: url(:/data/img/release_alternate.png); }");

        QToolButton *optionsButton = new QToolButton(tabBar);
        optionsButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	optionsButton->setText(" ");
        optionsButton->setIcon(QIcon(QString::fromUtf8(":/data/img/work.png")));
        optionsButton->setToolTip(tr("Toggle embedder options (hold down for menu)"));
        optionsButton->setStatusTip(tr("Toggle embedder options (hold down for menu)"));
        optionsButton->setCheckable(true);
        connect(optionsButton, SIGNAL(toggled(bool)), this, SLOT(on_embedderOptions_toggled(bool)));

	QMenu *optionsMenu = new QMenu(optionsButton);
        QString s;
        QAction *action;
        s = tr("To favorites");
        action = optionsMenu->addAction(tr("To &favorites"));
        action->setIcon(QIcon(QString::fromUtf8(":/data/img/favorites.png")));
        action->setToolTip(s); action->setStatusTip(s);
        connect(action, SIGNAL(triggered()), this, SLOT(on_embedderOptionsMenu_ToFavorites_activated()));
        optionsMenu->addSeparator();
        s = tr("Terminate emulator");
        action = optionsMenu->addAction(tr("&Terminate emulator"));
        action->setIcon(QIcon(QString::fromUtf8(":/data/img/terminate.png")));
        action->setToolTip(s); action->setStatusTip(s);
        connect(action, SIGNAL(triggered()), this, SLOT(on_embedderOptionsMenu_TerminateEmulator_activated()));
        s = tr("Kill emulator");
        action = optionsMenu->addAction(tr("&Kill emulator"));
        action->setIcon(QIcon(QString::fromUtf8(":/data/img/kill.png")));
        action->setToolTip(s); action->setStatusTip(s);
        connect(action, SIGNAL(triggered()), this, SLOT(on_embedderOptionsMenu_KillEmulator_activated()));
        optionsMenu->addSeparator();
        s = tr("Copy emulator command line to clipboard");
        action = optionsMenu->addAction(tr("&Copy command"));
        action->setToolTip(s); action->setStatusTip(s);
        action->setIcon(QIcon(QString::fromUtf8(":/data/img/editcopy.png")));
        connect(action, SIGNAL(triggered()), this, SLOT(on_embedderOptionsMenu_CopyCommand_activated()));

	optionsButton->setMenu(optionsMenu);

        tabBar->setTabButton(index, QTabBar::LeftSide, optionsButton);
        embedder->adjustIconSizes();
      }
      tabWidgetGamelist->setCurrentIndex(tabWidgetGamelist->indexOf(widgetEmbeddedEmus));
      tabWidgetEmbeddedEmulators->setCurrentIndex(tabWidgetEmbeddedEmulators->count() - 1);
      qApp->processEvents();
    } else {
      success = false;
      log(QMC2_LOG_FRONTEND, tr("WARNING: no matching window for emulator #%1 found").arg(gameID));
    }
  }

  sl = treeWidgetEmulators->selectedItems();
  if ( !success && sl.count() > 0 ) {
    switch ( QMessageBox::question(this, tr("Embedding failed"),
                                   tr("Couldn't find the window ID of one or more\nemulator(s) within a reasonable timeframe.\n\nRetry embedding?"),
                                   QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) ) {
      case QMessageBox::No:
        break;

      case QMessageBox::Yes:
        sl = treeWidgetEmulators->selectedItems();
        if ( sl.count() > 0 )
          QTimer::singleShot(0, this, SLOT(action_embedEmulator_triggered()));
	else
          QMessageBox::information(this, tr("Information"), tr("Sorry, the emulator meanwhile died a sorrowful death :(."));
        break;

      default:
        break;
    }
  }
}

#if defined(Q_WS_X11)
void MainWindow::action_embedderScanPauseKey_triggered()
{
	qApp->removeEventFilter(qmc2KeyPressFilter);

	KeySequenceScanner keySeqScanner(this, true, true);
	keySeqScanner.setWindowTitle(tr("Scanning pause key"));
	keySeqScanner.labelStatus->setText(tr("Scanning pause key"));
	if ( keySeqScanner.exec() == QDialog::Accepted )
  		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Embedder/PauseKey", keySeqScanner.keySequence);

	qApp->installEventFilter(qmc2KeyPressFilter);
}
#endif

void MainWindow::on_embedderOptions_toggled(bool enabled)
{
  // serious hack to access the tab bar ;)
  QToolButton *optionsButton = (QToolButton *)sender();
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_embedderOptions_toggled(bool enabled = %1)").arg(enabled));
#endif

  QTabBar *tabBar = (QTabBar *)optionsButton->parent();
  Embedder *embedder = (Embedder *)tabWidgetEmbeddedEmulators->widget(tabBar->tabAt(optionsButton->pos()));
  if ( embedder )
    embedder->toggleOptions();

  if ( enabled )
    tabWidgetEmbeddedEmulators->setCurrentIndex(tabWidgetEmbeddedEmulators->indexOf(embedder));
}

void MainWindow::on_tabWidgetEmbeddedEmulators_tabCloseRequested(int index)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_tabWidgetEmbeddedEmulators_tabCloseRequested(int index = %1)").arg(index));
#endif

  tabWidgetGamelist->setUpdatesEnabled(false);

  QWidget *widget = NULL;

  if ( index >= 0 ) {
    widget = tabWidgetEmbeddedEmulators->widget(index);
    if ( widget )
      tabWidgetEmbeddedEmulators->removeTab(index);
  }

  qApp->processEvents();

  if ( tabWidgetEmbeddedEmulators->count() < 1 ) {
    if ( tabWidgetGamelist->currentIndex() == tabWidgetGamelist->indexOf(tabEmbeddedEmus) ) {
      tabWidgetGamelist->setCurrentIndex(qmc2LastListIndex);
      if ( toolButtonEmbedderMaximizeToggle->isChecked() )
        hSplitter->setSizes(hSplitterSizes);
    }
    tabWidgetGamelist->removeTab(tabWidgetGamelist->indexOf(tabEmbeddedEmus));
    menuBar()->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ShowMenuBar", true).toBool());
    statusBar()->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Statusbar", true).toBool());
    toolbar->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Toolbar", true).toBool());
    frameStatus->show();
  }

  if ( widget ) {
    widget->close();
    widget->deleteLater();
  }

  tabWidgetGamelist->setUpdatesEnabled(true);
}

void MainWindow::closeEmbeddedEmuTab()
{
  Embedder *embedder = (Embedder *)sender();
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::closeEmbeddedEmuTab()");
#endif

  if ( !qmc2CleaningUp )
    on_tabWidgetEmbeddedEmulators_tabCloseRequested(tabWidgetEmbeddedEmulators->indexOf(embedder));
}

void MainWindow::on_toolButtonEmbedderMaximizeToggle_toggled(bool on)
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_toolButtonEmbedderMaximizeToggle_toggled(bool on = %1)").arg(on));
#endif

	if ( on ) {
		menuBar()->hide();
		statusBar()->hide();
		toolbar->hide();
		frameStatus->hide();
		qApp->processEvents();
		hSplitterSizes = hSplitter->sizes();
		QList<int> maximizedSizes;
		if ( hSplitter->widget(0) == hSplitterWidget0 )
			maximizedSizes << desktopGeometry.width() << 0;
		else
			maximizedSizes << 0 << desktopGeometry.width();
		hSplitter->setSizes(maximizedSizes);
		toolButtonEmbedderMaximizeToggle->setIcon(QIcon(QString::fromUtf8(":/data/img/minimize.png")));
	} else {
		hSplitter->setSizes(hSplitterSizes);
		menuBar()->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ShowMenuBar", true).toBool());
		statusBar()->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Statusbar", true).toBool());
		toolbar->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Toolbar", true).toBool());
		frameStatus->show();
		toolButtonEmbedderMaximizeToggle->setIcon(QIcon(QString::fromUtf8(":/data/img/maximize.png")));
	}
}

void MainWindow::on_embedderOptionsMenu_KillEmulator_activated()
{
  // serious hack to access the corresponding embedder ;)
  QAction *action = (QAction *)sender();
  QMenu *menu = (QMenu *)action->parent();
  QToolButton *toolButton = (QToolButton *)menu->parent();
  QTabBar *tabBar = (QTabBar *)toolButton->parent();
  Embedder *embedder = (Embedder *)tabWidgetEmbeddedEmulators->widget(tabBar->tabAt(toolButton->pos()));
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_embedderOptionsMenu_KillEmulator_activated()");
#endif

  if ( embedder )
    qmc2ProcessManager->kill(embedder->gameID.toInt());
}

void MainWindow::on_embedderOptionsMenu_TerminateEmulator_activated()
{
  // serious hack to access the corresponding embedder ;)
  QAction *action = (QAction *)sender();
  QMenu *menu = (QMenu *)action->parent();
  QToolButton *toolButton = (QToolButton *)menu->parent();
  QTabBar *tabBar = (QTabBar *)toolButton->parent();
  Embedder *embedder = (Embedder *)tabWidgetEmbeddedEmulators->widget(tabBar->tabAt(toolButton->pos()));
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_embedderOptionsMenu_TerminateEmulator_activated()");
#endif

  if ( embedder )
    qmc2ProcessManager->terminate(embedder->gameID.toInt());
}

void MainWindow::on_embedderOptionsMenu_ToFavorites_activated()
{
  // serious hack to access the corresponding embedder ;)
  QAction *action = (QAction *)sender();
  QMenu *menu = (QMenu *)action->parent();
  QToolButton *toolButton = (QToolButton *)menu->parent();
  QTabBar *tabBar = (QTabBar *)toolButton->parent();
  Embedder *embedder = (Embedder *)tabWidgetEmbeddedEmulators->widget(tabBar->tabAt(toolButton->pos()));
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_embedderOptionsMenu_ToFavorites_activated()");
#endif

  if ( embedder ) {
    QString gameDescription = qmc2GamelistDescriptionMap[embedder->gameName];
    QList<QListWidgetItem *> matches = listWidgetFavorites->findItems(gameDescription, Qt::MatchExactly);
    if ( matches.count() <= 0 ) {
      QListWidgetItem *item = new QListWidgetItem(listWidgetFavorites);
      item->setText(gameDescription);
      listWidgetFavorites->sortItems();
    }
  }
}

void MainWindow::on_embedderOptionsMenu_CopyCommand_activated()
{
  // serious hack to access the corresponding embedder ;)
  QAction *action = (QAction *)sender();
  QMenu *menu = (QMenu *)action->parent();
  QToolButton *toolButton = (QToolButton *)menu->parent();
  QTabBar *tabBar = (QTabBar *)toolButton->parent();
  Embedder *embedder = (Embedder *)tabWidgetEmbeddedEmulators->widget(tabBar->tabAt(toolButton->pos()));
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_embedderOptionsMenu_CopyCommand_activated()");
#endif

  if ( embedder ) {
    QList<QTreeWidgetItem *> il = treeWidgetEmulators->findItems(embedder->gameID, Qt::MatchExactly, QMC2_EMUCONTROL_COLUMN_ID);
    if ( il.count() > 0 )
      QApplication::clipboard()->setText(il[0]->text(QMC2_EMUCONTROL_COLUMN_COMMAND));
  }
}
#endif

void MainWindow::action_terminateEmulator_triggered()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::action_terminateEmulator_triggered()");
#endif

  QList<QTreeWidgetItem *> sl = treeWidgetEmulators->selectedItems();
  int i;
  for (i = 0; i < sl.count(); i++) {
    QTreeWidgetItem *item = sl[i];
    while ( item->parent() ) item = item->parent();
    qmc2ProcessManager->terminate(item->text(QMC2_EMUCONTROL_COLUMN_NUMBER).toInt());
  }

  if ( sl.count() == 0 )
    if ( treeWidgetEmulators->currentItem() ) {
      QTreeWidgetItem *item = treeWidgetEmulators->currentItem();
      while ( item->parent() ) item = item->parent();
      qmc2ProcessManager->terminate(item->text(QMC2_EMUCONTROL_COLUMN_NUMBER).toInt());
    }
}

void MainWindow::action_killEmulator_triggered()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::action_killEmulator_triggered()");
#endif

  QList<QTreeWidgetItem *> sl = treeWidgetEmulators->selectedItems();
  int i;
  for (i = 0; i < sl.count(); i++) {
    QTreeWidgetItem *item = sl[i];
    while ( item->parent() ) item = item->parent();
    qmc2ProcessManager->kill(item->text(QMC2_EMUCONTROL_COLUMN_NUMBER).toInt());
  }

  if ( sl.count() == 0 )
    if ( treeWidgetEmulators->currentItem() ) {
      QTreeWidgetItem *item = treeWidgetEmulators->currentItem();
      while ( item->parent() ) item = item->parent();
      qmc2ProcessManager->kill(item->text(QMC2_EMUCONTROL_COLUMN_NUMBER).toInt());
    }
}

void MainWindow::action_copyEmulatorCommand_triggered()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::action_copyEmulatorCommand_triggered()");
#endif

  QString commandString;
  QList<QTreeWidgetItem *> sl = treeWidgetEmulators->selectedItems();
  QList<QTreeWidgetItem *> tl;
  int i;

  // find toplevel items...
  for (i = 0; i < sl.count(); i++) {
    QTreeWidgetItem *item = sl[i];
    while ( item->parent() ) item = item->parent();
    if ( !tl.contains(item) )
      tl.append(item);
  }
  // ... and copy their commands
  for (i = 0; i < tl.count(); i++) {
    if ( i > 0 ) commandString += "\n";
    commandString += tl[i]->text(QMC2_EMUCONTROL_COLUMN_COMMAND);
  }

  QApplication::clipboard()->setText(commandString);
}

void MainWindow::action_removeFromFavorites_triggered()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::action_removeFromFavorites_triggered()");
#endif

  QListWidgetItem *i = listWidgetFavorites->currentItem();
  if ( i ) {
    QListWidgetItem *item = listWidgetFavorites->takeItem(listWidgetFavorites->row(i));
    delete item;
  }
}

void MainWindow::action_clearAllFavorites_triggered()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::action_clearAllFavorites_triggered()");
#endif

  switch ( QMessageBox::question(this, tr("Confirm"),
                                 tr("Are you sure you want to clear the favorites list?"),
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::No) ) {
    case QMessageBox::No:
      return;
      break;

    case QMessageBox::Yes:
      listWidgetFavorites->clear();
      qmc2Gamelist->saveFavorites();
      break;

    default:
      break;
  }
}

void MainWindow::action_saveFavorites_triggered()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::action_saveFavorites_triggered()");
#endif

  qmc2Gamelist->saveFavorites();
}

void MainWindow::action_removeFromPlayed_triggered()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::action_removeFromPlayed_triggered()");
#endif

  QListWidgetItem *i = listWidgetPlayed->currentItem();
  if ( i ) {
    QListWidgetItem *item = listWidgetPlayed->takeItem(listWidgetPlayed->row(i));
    delete item;
  }
}

void MainWindow::action_clearAllPlayed_triggered()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::action_clearAllPlayed_triggered()");
#endif

  switch ( QMessageBox::question(this, tr("Confirm"),
                                 tr("Are you sure you want to clear the play history?"),
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::No) ) {
    case QMessageBox::No:
      return;
      break;

    case QMessageBox::Yes:
      listWidgetPlayed->clear();
      qmc2Gamelist->savePlayHistory();
      break;

    default:
      break;
  }
}

void MainWindow::action_savePlayed_triggered()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::action_savePlayed_triggered()");
#endif

  qmc2Gamelist->savePlayHistory();
}

void MainWindow::on_listWidgetSearch_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_listWidgetSearch_customContextMenuRequested(const QPoint &p = ...)");
#endif

  QListWidgetItem *item = listWidgetSearch->itemAt(p);
  if ( item ) {
    listWidgetSearch->setItemSelected(item, true);
    qmc2SearchMenu->move(adjustedWidgetPosition(listWidgetSearch->viewport()->mapToGlobal(p), qmc2SearchMenu));
    qmc2SearchMenu->show();
  }
}

void MainWindow::on_listWidgetFavorites_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_listWidgetFavorites_customContextMenuRequested(const QPoint &p = ...)");
#endif

  QListWidgetItem *item = listWidgetFavorites->itemAt(p);
  if ( item ) {
    listWidgetFavorites->setItemSelected(item, true);
    qmc2FavoritesMenu->move(adjustedWidgetPosition(listWidgetFavorites->viewport()->mapToGlobal(p), qmc2FavoritesMenu));
    qmc2FavoritesMenu->show();
  }
}

void MainWindow::on_listWidgetPlayed_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_listWidgetPlayed_customContextMenuRequested(const QPoint &p = ...)");
#endif

  QListWidgetItem *item = listWidgetPlayed->itemAt(p);
  if ( item ) {
    listWidgetPlayed->setItemSelected(item, true);
    qmc2PlayedMenu->move(adjustedWidgetPosition(listWidgetPlayed->viewport()->mapToGlobal(p), qmc2PlayedMenu));
    qmc2PlayedMenu->show();
  }
}

void MainWindow::on_treeWidgetGamelist_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_treeWidgetGamelist_customContextMenuRequested(const QPoint &p = ...)");
#endif

  QTreeWidgetItem *item = treeWidgetGamelist->itemAt(p);
  if ( !item )
    return;
  if ( item->text(QMC2_GAMELIST_COLUMN_GAME) == tr("Waiting for data...") )
    return;
  treeWidgetGamelist->setItemSelected(item, true);
  qmc2GameMenu->move(adjustedWidgetPosition(treeWidgetGamelist->viewport()->mapToGlobal(p), qmc2GameMenu));
  qmc2GameMenu->show();
}

void MainWindow::on_treeWidgetHierarchy_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_treeWidgetHierarchy_customContextMenuRequested(const QPoint &p = ...)");
#endif

  QTreeWidgetItem *item = treeWidgetHierarchy->itemAt(p);
  if ( !item )
    return;
  if ( item->text(QMC2_GAMELIST_COLUMN_GAME) == tr("Waiting for data...") )
    return;
  if ( item ) {
    treeWidgetHierarchy->setItemSelected(item, true);
    qmc2GameMenu->move(adjustedWidgetPosition(treeWidgetHierarchy->viewport()->mapToGlobal(p), qmc2GameMenu));
    qmc2GameMenu->show();
  }
}

void MainWindow::on_stackedWidgetView_currentChanged(int index)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_tackedWidgetView_currentChanged(int index = %1)").arg(index));
#endif

  if ( !qmc2EarlyStartup ) {
    menuGamelistHeader->hide();
    menuHierarchyHeader->hide();
#if defined(QMC2_EMUTYPE_MAME)
    menuCategoryHeader->hide();
    menuVersionHeader->hide();
#endif
  }

  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "GUI/GamelistView", index);

  if ( !qmc2CurrentItem )
    return;

  QTreeWidgetItem *ci = treeWidgetGamelist->currentItem();
  if ( !ci )
    return;

  if ( qmc2CurrentItem != ci && qmc2ReloadActive )
    qmc2CurrentItem = ci;

  if ( qmc2CurrentItem->childCount() <= 0 )
    return;

  switch ( index ) {
    case QMC2_VIEWHIERARCHY_INDEX:
      if ( !qmc2ReloadActive ) {
        QString gameName = qmc2CurrentItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON);
        QTreeWidgetItem *hierarchyItem = qmc2HierarchyItemMap[gameName];
        treeWidgetHierarchy->clearSelection();
        if ( hierarchyItem ) {
          treeWidgetHierarchy->setCurrentItem(hierarchyItem);
          treeWidgetHierarchy->scrollToItem(hierarchyItem, qmc2CursorPositioningMode);
          hierarchyItem->setSelected(true);
        }
      }
      break;

#if defined(QMC2_EMUTYPE_MAME)
    case QMC2_VIEWCATEGORY_INDEX:
      if ( !qmc2ReloadActive ) {
        QString gameName = qmc2CurrentItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON);
        QTreeWidgetItem *categoryItem = qmc2CategoryItemMap[gameName];
        treeWidgetCategoryView->clearSelection();
        if ( categoryItem ) {
          treeWidgetCategoryView->setCurrentItem(categoryItem);
          treeWidgetCategoryView->scrollToItem(categoryItem, qmc2CursorPositioningMode);
          categoryItem->setSelected(true);
        }
      }
      break;

    case QMC2_VIEWVERSION_INDEX:
      if ( !qmc2ReloadActive ) {
        QString gameName = qmc2CurrentItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON);
        QTreeWidgetItem *versionItem = qmc2VersionItemMap[gameName];
        treeWidgetVersionView->clearSelection();
        if ( versionItem ) {
          treeWidgetVersionView->setCurrentItem(versionItem);
          treeWidgetVersionView->scrollToItem(versionItem, qmc2CursorPositioningMode);
          versionItem->setSelected(true);
        }
      }
      break;
#endif

    case QMC2_VIEWGAMELIST_INDEX:
    default:
      scrollToCurrentItem();
      break;
  }
}

void MainWindow::on_pushButtonGlobalEmulatorOptionsSelectExportFile_clicked()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_pushButtonGlobalEmulatorOptionsSelectExportFile_clicked()");
#endif

#if defined(QMC2_EMUTYPE_MAME)
  QString fileName = qmc2Config->value("MAME/Configuration/Global/inipath", QDir::homePath()).toString().replace("~", QDir::homePath()) + "/mame.ini";
#elif defined(QMC2_EMUTYPE_MESS)
  QString fileName = qmc2Config->value("MESS/Configuration/Global/inipath", QDir::homePath()).toString().replace("~", QDir::homePath()) + "/mess.ini";
#else
  QString fileName;
#endif
  QString s = QFileDialog::getSaveFileName(qmc2Options, tr("Choose export file"), fileName, tr("All files (*)"));
  if ( !s.isNull() )
    on_pushButtonGlobalEmulatorOptionsExportToFile_clicked(s);
}

void MainWindow::on_pushButtonGlobalEmulatorOptionsExportToFile_clicked(QString useFileName)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_pushButtonGlobalEmulatorOptionsExportToFile_clicked(QString useFileName = %1)").arg(useFileName));
#endif

  qmc2GlobalEmulatorOptions->exportToIni(true, useFileName);
}

void MainWindow::on_pushButtonGlobalEmulatorOptionsSelectImportFile_clicked()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_pushButtonGlobalEmulatorOptionsSelectImportFile_clicked()");
#endif

#if defined(QMC2_EMUTYPE_MAME)
  QString fileName = qmc2Config->value("MAME/Configuration/Global/inipath", QDir::homePath()).toString().replace("~", QDir::homePath()) + "/mame.ini";
#elif defined(QMC2_EMUTYPE_MESS)
  QString fileName = qmc2Config->value("MESS/Configuration/Global/inipath", QDir::homePath()).toString().replace("~", QDir::homePath()) + "/mess.ini";
#else
  QString fileName;
#endif
  QString s = QFileDialog::getOpenFileName(qmc2Options, tr("Choose import file"), fileName, tr("All files (*)"));
  if ( !s.isNull() )
    on_pushButtonGlobalEmulatorOptionsImportFromFile_clicked(s);
}

void MainWindow::on_pushButtonGlobalEmulatorOptionsImportFromFile_clicked(QString useFileName)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_pushButtonGlobalEmulatorOptionsImportFromFile_clicked(QString useFileName = %1)").arg(useFileName));
#endif

  qmc2GlobalEmulatorOptions->importFromIni(true, useFileName);
}

void MainWindow::on_pushButtonCurrentEmulatorOptionsSelectExportFile_clicked()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_pushButtonCurrentEmulatorOptionsSelectExportFile_clicked()");
#endif

#if defined(QMC2_EMUTYPE_MAME)
  QStringList iniPaths = qmc2Config->value("MAME/Configuration/Global/inipath", QDir::homePath()).toString().split(";");
#elif defined(QMC2_EMUTYPE_MESS)
  QStringList iniPaths = qmc2Config->value("MESS/Configuration/Global/inipath", QDir::homePath()).toString().split(";");
#endif
  QString iniPath;
  if ( iniPaths.count() > 0 ) {
    iniPath = iniPaths[0].replace("~", QDir::homePath());
  } else {
    iniPath = ".";
    log(QMC2_LOG_FRONTEND, tr("WARNING: invalid inipath"));
  }
  QString s = QFileDialog::getSaveFileName(this, tr("Choose export file"), iniPath, tr("All files (*)"));
  if ( !s.isNull() )
    on_pushButtonCurrentEmulatorOptionsExportToFile_clicked(s);
}

void MainWindow::on_pushButtonCurrentEmulatorOptionsExportToFile_clicked(QString useFileName)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_pushButtonCurrentEmulatorOptionsExportToFile_clicked(QString useFileName = %1)").arg(useFileName));
#endif

  qmc2EmulatorOptions->exportToIni(false, useFileName);
}

void MainWindow::on_pushButtonCurrentEmulatorOptionsSelectImportFile_clicked()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_pushButtonCurrentEmulatorOptionsSelectImportFile_clicked()");
#endif

  if ( !qmc2CurrentItem )
    return;

#if defined(QMC2_EMUTYPE_MAME)
  QStringList iniPaths = qmc2Config->value("MAME/Configuration/Global/inipath", QDir::homePath()).toString().split(";");
#elif defined(QMC2_EMUTYPE_MESS)
  QStringList iniPaths = qmc2Config->value("MESS/Configuration/Global/inipath", QDir::homePath()).toString().split(";");
#endif
  QString iniPath;
  if ( iniPaths.count() > 0 ) {
    iniPath = iniPaths[0].replace("~", QDir::homePath());
  } else {
    iniPath = ".";
    log(QMC2_LOG_FRONTEND, tr("WARNING: invalid inipath"));
  }
  iniPath += "/" + qmc2CurrentItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON) + ".ini";
  QString s = QFileDialog::getOpenFileName(this, tr("Choose import file"), iniPath, tr("All files (*)"));
  if ( !s.isNull() )
    on_pushButtonCurrentEmulatorOptionsImportFromFile_clicked(s);
}

void MainWindow::on_pushButtonCurrentEmulatorOptionsImportFromFile_clicked(QString useFileName)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_pushButtonCurrentEmulatorOptionsImportFromFile_clicked(QString useFileName = %1)").arg(useFileName));
#endif

  qmc2EmulatorOptions->importFromIni(false, useFileName);
}

#if QMC2_JOYSTICK == 1
void MainWindow::mapJoystickFunction(QString function)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::mapJoystickFunction(QString function = %1)").arg(function));
#endif

  if ( qmc2CleaningUp )
    return;

  // don't map joystick functions while calibration or test are active
  if ( qmc2Options->treeWidgetJoystickMappings->isHidden() )
    return;

  QString shortcut = qmc2CustomShortcutMap[qmc2JoystickFunctionMap[function]];

#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: matched function = %1").arg(shortcut.isEmpty() ? "none" : shortcut));
#endif

  if ( shortcut.isEmpty() )
    return;
  
  QWidget *focusWidget = QApplication::focusWidget();

  if ( focusWidget ) {
#if defined(Q_WS_X11) || defined(Q_WS_WIN)
    // don't map joystick functions when they are really meant for an embedded emulator
    if ( focusWidget->objectName() == "QMC2_EMBED_CONTAINER" )
      return;
#endif
    QKeySequence keySeq(shortcut);
    uint i, key = 0;
    for (i = 0; i < keySeq.count(); i++)
      key += keySeq[i];
    QKeyEvent *emulatedKeyEvent = new QKeyEvent(QEvent::KeyPress, key, Qt::NoModifier);
    qApp->postEvent(focusWidget, emulatedKeyEvent);
  }
}

void MainWindow::on_joystickAxisValueChanged(int axis, int value)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_joystickAxisValueChanged(int axis = %1, int value = %2)").arg(axis).arg(value));
#endif

  if ( qmc2Config->value(QString(QMC2_FRONTEND_PREFIX + "Joystick/%1/Axis%2Enabled").arg(joyIndex).arg(axis), true).toBool() ) {
    if ( value != 0 )
      mapJoystickFunction(QString("A%1%2").arg(axis).arg(value < 0 ? "-" : "+"));
  }
}

void MainWindow::on_joystickButtonValueChanged(int button, bool value)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_joystickButtonValueChanged(int button = %1, bool value = %2)").arg(button).arg(value));
#endif

   if ( value )
     mapJoystickFunction(QString("B%1").arg(button));
}

void MainWindow::on_joystickHatValueChanged(int hat, int value)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_joystickHatValueChanged(int hat = %1, int value = %2)").arg(hat).arg(value));
#endif

   if ( value != 0 )
     mapJoystickFunction(QString("H%1:%2").arg(hat).arg(value));
}

void MainWindow::on_joystickTrackballValueChanged(int trackball, int deltaX, int deltaY)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_joystickTrackballValueChanged(int trackball = %1, int deltaX = %2, int deltaY = %3)").arg(trackball).arg(deltaX).arg(deltaY));
#endif

  mapJoystickFunction(QString("T%1:X%2,Y%3").arg(trackball)
                             .arg(deltaX < 0 ? "-" : deltaX > 0 ? "+" : "=")
                             .arg(deltaY < 0 ? "-" : deltaY > 0 ? "+" : "="));
}
#endif

void MainWindow::closeEvent(QCloseEvent *e)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::closeEvent(QCloseEvent *e = 0x" + QString::number((qulonglong)e, 16) + ")");
#endif

  if ( qmc2CleaningUp ) {
    e->ignore();
    return;
  }

#if defined(QMC2_EMUTYPE_MAME)
  if ( qmc2ReloadActive ||
       qmc2VerifyActive ||
       qmc2FilterActive ||
       qmc2ImageCheckActive ||
       qmc2SampleCheckActive ||
       qmc2ROMAlyzerActive ||
       qmc2LoadingGameInfoDB ||
       qmc2LoadingEmuInfoDB )
#else
  if ( qmc2ReloadActive ||
       qmc2VerifyActive ||
       qmc2FilterActive ||
       qmc2ImageCheckActive ||
       qmc2ROMAlyzerActive ||
       qmc2LoadingGameInfoDB ||
       qmc2LoadingEmuInfoDB )
#endif
  {
    qmc2StopParser = true;
    log(QMC2_LOG_FRONTEND, tr("stopping current processing upon user request"));
    e->ignore();
    return;
  }

  if ( qmc2SoftwareList && qmc2SoftwareList->isLoading ) {
    qmc2SoftwareList->interruptLoad = true;
    log(QMC2_LOG_FRONTEND, tr("stopping current processing upon user request"));
    e->ignore();
    return;
  }

  if ( !qmc2Options->applied ) {
    switch ( QMessageBox::question(this, tr("Confirm"),
                                   tr("Your configuration changes have not been applied yet.\nReally quit?"),
				   QMessageBox::Yes | QMessageBox::No, QMessageBox::No) ) {
      case QMessageBox::No:
        e->ignore();
        return;
        break;

      default:
        break;
    }
  }
 
  bool doKillEmulators = qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/KillEmulatorsOnExit").toBool();
  if ( qmc2ProcessManager->procMap.count() > 0 && !doKillEmulators ) {
    switch ( QMessageBox::question(this, tr("Confirm"),
                                   tr("There are one or more emulators still running.\nShould they be killed on exit?"),
                                   QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Cancel) ) {
      case QMessageBox::Yes:
        doKillEmulators = true;
        break;

      case QMessageBox::No:
        doKillEmulators = false;
        break;

      case QMessageBox::Cancel:
        e->ignore();
        return;
        break;

      default:
        break;
    }
  }

  QList<int> indexList;
  int runningDownloads = 0;
  QTreeWidgetItemIterator it(treeWidgetDownloads);
  while ( *it ) {
    indexList << treeWidgetDownloads->indexOfTopLevelItem(*it);
    if ( (*it)->whatsThis(0) == "downloading" || (*it)->whatsThis(0) == "initializing" )
      runningDownloads++;
    it++;
  }
  if ( runningDownloads > 0 ) {
    switch ( QMessageBox::question(this, tr("Confirm"),
                                   tr("There are one or more running downloads. Quit anyway?"),
                                   QMessageBox::Yes | QMessageBox::No, QMessageBox::No) ) {
      case QMessageBox::Yes:
        break;

      case QMessageBox::No:
        e->ignore();
        return;
        break;

      default:
        break;
    }
  }

  log(QMC2_LOG_FRONTEND, tr("cleaning up"));
  qmc2CleaningUp = true;
  
  if ( runningDownloads > 0 )
    log(QMC2_LOG_FRONTEND, tr("aborting running downloads"));
  foreach (int i, indexList) {
    DownloadItem *item = (DownloadItem *)treeWidgetDownloads->takeTopLevelItem(i);
    if ( item ) {
      if ( item->whatsThis(0) == "downloading" || item->whatsThis(0) == "initializing" )
        item->itemDownloader->networkReply->abort();
      delete item;
    }
  }

#if defined(QMC2_YOUTUBE_ENABLED)
  if ( !qmc2YouTubeVideoInfoMap.isEmpty() && qmc2YouTubeVideoInfoMapChanged ) {
	  log(QMC2_LOG_FRONTEND, tr("saving YouTube video info map"));
	  QDir youTubeCacheDir(qmc2Config->value(QMC2_FRONTEND_PREFIX + "YouTubeWidget/CacheDirectory").toString());
	  if ( youTubeCacheDir.exists() ) {
#if defined(QMC2_SDLMAME)
		  QFile f(youTubeCacheDir.canonicalPath() + "/qmc2-sdlmame.yti");
#elif defined(QMC2_SDLMESS)
		  QFile f(youTubeCacheDir.canonicalPath() + "/qmc2-sdlmess.yti");
#elif defined(QMC2_MAME)
		  QFile f(youTubeCacheDir.canonicalPath() + "/qmc2-mame.yti");
#elif defined(QMC2_MESS)
		  QFile f(youTubeCacheDir.canonicalPath() + "/qmc2-mess.yti");
#else
		  QFile f(youTubeCacheDir.canonicalPath() + "/qmc2.yti");
#endif
		  if ( f.open(QIODevice::WriteOnly | QIODevice::Text) ) {
			  QTextStream ts(&f);
			  ts << "# THIS FILE IS AUTO-GENERATED - PLEASE DO NOT EDIT!\n";
			  QMapIterator<QString, YouTubeVideoInfo> it(qmc2YouTubeVideoInfoMap);
			  while ( it.hasNext() ) {
				  it.next();
				  ts << it.key() << "\t" << it.value().author << "\t" << it.value().title << "\n";
			  }
			  f.close();
			  log(QMC2_LOG_FRONTEND, tr("done (saving YouTube video info map)"));
		  } else
			  log(QMC2_LOG_FRONTEND, tr("failed (saving YouTube video info map)"));
	  } else
		  log(QMC2_LOG_FRONTEND, tr("failed (saving YouTube video info map)"));
  }
#endif

#if QMC2_USE_PHONON_API
  log(QMC2_LOG_FRONTEND, tr("disconnecting audio source from audio sink"));
  phononAudioPath.disconnect();
  if ( qmc2AudioEffectDialog ) {
    log(QMC2_LOG_FRONTEND, tr("destroying audio effects dialog"));
    qmc2AudioEffectDialog->close();
    delete qmc2AudioEffectDialog;
  }
#if defined(QMC2_YOUTUBE_ENABLED)
  if ( qmc2YouTubeWidget ) {
    log(QMC2_LOG_FRONTEND, tr("destroying YouTube video widget"));
    qmc2YouTubeWidget->saveSettings();
    qmc2YouTubeWidget->forcedExit = true;
    if ( qmc2YouTubeWidget->videoPlayer->isPlaying() || qmc2YouTubeWidget->videoPlayer->isPaused() )
      qmc2YouTubeWidget->videoPlayer->stop();
    qmc2YouTubeWidget->close();
    qmc2YouTubeWidget->deleteLater();
    qmc2YouTubeWidget = NULL;
  }
#endif
#endif

  if ( listWidgetFavorites->count() > 0 )
    qmc2Gamelist->saveFavorites();

  if ( listWidgetPlayed->count() > 0 )
    qmc2Gamelist->savePlayHistory();

  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "GUI/GamelistView", comboBoxViewSelect->currentIndex());
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveGameSelection").toBool() ) {
    if ( qmc2CurrentItem ) {
      log(QMC2_LOG_FRONTEND, tr("saving game selection"));
#if defined(QMC2_EMUTYPE_MAME)
      qmc2Config->setValue("MAME/SelectedGame", qmc2CurrentItem->text(QMC2_GAMELIST_COLUMN_GAME));
#elif defined(QMC2_EMUTYPE_MESS)
      qmc2Config->setValue("MESS/SelectedGame", qmc2CurrentItem->text(QMC2_GAMELIST_COLUMN_GAME));
#endif
    } else
#if defined(QMC2_EMUTYPE_MAME)
      qmc2Config->remove("MAME/SelectedGame");
#elif defined(QMC2_EMUTYPE_MESS)
      qmc2Config->remove("MESS/SelectedGame");
#endif
  }

  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() ) {
    log(QMC2_LOG_FRONTEND, tr("saving main widget layout"));
    if ( windowState() & Qt::WindowFullScreen ) {
      qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Fullscreen", true);
    } else {
      qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Fullscreen", false);
      if ( isMaximized() ) {
        qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Maximized", true);
      } else {
        qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Maximized", false);
        qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Geometry", saveGeometry());
        qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Position", pos());
        qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Size", size());
      }
    }

#if defined(Q_WS_X11) || defined(Q_WS_WIN)
    if ( toolButtonEmbedderMaximizeToggle->isChecked() && tabWidgetGamelist->currentIndex() == QMC2_EMBED_INDEX )
      qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/hSplitter", QSize(hSplitterSizes[0], hSplitterSizes[1]));
    else
      qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/hSplitter", QSize(hSplitter->sizes().at(0), hSplitter->sizes().at(1)));
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/Embedder/Maximize", toolButtonEmbedderMaximizeToggle->isChecked());
#if defined(Q_WS_X11)
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/Embedder/AutoPause", toolButtonEmbedderAutoPause->isChecked());
#endif
#else
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/hSplitter", QSize(hSplitter->sizes().at(0), hSplitter->sizes().at(1)));
#endif
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/hSplitterFlipped", hSplitter->orientation() != Qt::Horizontal);
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/hSplitterSwapped", hSplitter->widget(0) != hSplitterWidget0);
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/vSplitter", QSize(vSplitter->sizes().at(0), vSplitter->sizes().at(1)));
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/vSplitterFlipped", vSplitter->orientation() != Qt::Vertical);
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/vSplitterSwapped", vSplitter->widget(0) != vSplitterWidget0);
#if defined(Q_WS_X11) || defined(Q_WS_WIN)
    if ( tabWidgetGamelist->currentIndex() == QMC2_EMBED_INDEX )
      qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/GamelistTab", qmc2LastListIndex);
    else
      qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/GamelistTab", tabWidgetGamelist->currentIndex());
#else
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/GamelistTab", tabWidgetGamelist->currentIndex());
#endif
#if defined(QMC2_EMUTYPE_MAME)
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/GameDetailTab", tabWidgetGameDetail->currentIndex());
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/MachineDetailTab", tabWidgetGameDetail->currentIndex());
#endif
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/LogsAndEmulatorsTab", tabWidgetLogsAndEmulators->currentIndex());
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/SoftwareDetailTab", tabWidgetSoftwareDetail->currentIndex());
    // save toolbar state
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/ToolbarState", saveState());
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/GamelistHeaderState", treeWidgetGamelist->header()->saveState());
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/HierarchyHeaderState", treeWidgetHierarchy->header()->saveState());
#if defined(QMC2_EMUTYPE_MAME)
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/CategoryViewHeaderState", treeWidgetCategoryView->header()->saveState());
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/VersionViewHeaderState", treeWidgetVersionView->header()->saveState());
#endif
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/EmulatorControlHeaderState", treeWidgetEmulators->header()->saveState());

    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/SoftwareDetailDocked", floatToggleButtonSoftwareDetail->isChecked());
    if ( !floatToggleButtonSoftwareDetail->isChecked() )
      qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/SoftwareDetailGeometry", tabWidgetSoftwareDetail->saveGeometry());
  }

#if QMC2_USE_PHONON_API
  QList<QListWidgetItem *> pl = listWidgetAudioPlaylist->findItems("*", Qt::MatchWildcard);
  QStringList psl;
  foreach (QListWidgetItem *item, pl) psl << item->text();
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "AudioPlayer/PlayList", psl);
  if ( listWidgetAudioPlaylist->currentItem() )
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "AudioPlayer/LastTrack", listWidgetAudioPlaylist->currentItem()->text());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "AudioPlayer/PlayOnStart", checkBoxAudioPlayOnStart->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "AudioPlayer/Shuffle", checkBoxAudioShuffle->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "AudioPlayer/Pause", checkBoxAudioPause->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "AudioPlayer/Fade", checkBoxAudioFade->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "AudioPlayer/Volume", dialAudioVolume->value());
#endif

  // download manager widget
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Downloads/RemoveFinished", checkBoxRemoveFinishedDownloads->isChecked());

  if ( qmc2ArcadeView ) {
    log(QMC2_LOG_FRONTEND, tr("destroying arcade view"));
    qmc2ArcadeView->close();
    delete qmc2ArcadeView;
  }
  if ( qmc2ArcadeSetupDialog ) {
    log(QMC2_LOG_FRONTEND, tr("destroying arcade setup dialog"));
    qmc2ArcadeSetupDialog->close();
    delete qmc2ArcadeSetupDialog;
  }

  if ( qmc2SoftwareSnap ) {
    qmc2SoftwareSnap->close();
    delete qmc2SoftwareSnap;
  }

#if defined(QMC2_EMUTYPE_MAME)
  if ( qmc2SoftwareList ) {
    if ( qmc2SoftwareNotesEditor ) {
      qmc2SoftwareNotesEditor->save();
      qmc2SoftwareNotesEditor->close();
      delete qmc2SoftwareNotesEditor;
    }
    if ( qmc2SoftwareList->fullyLoaded ) {
      log(QMC2_LOG_FRONTEND, tr("saving current game's favorite software"));
      qmc2SoftwareList->save();
    }
    delete qmc2SoftwareList;
  }
#elif defined(QMC2_EMUTYPE_MESS)
  if ( qmc2SoftwareList ) {
    if ( qmc2SoftwareNotesEditor ) {
      qmc2SoftwareNotesEditor->save();
      qmc2SoftwareNotesEditor->close();
      delete qmc2SoftwareNotesEditor;
    }
    if ( qmc2SoftwareList->fullyLoaded ) {
      log(QMC2_LOG_FRONTEND, tr("saving current machine's favorite software"));
      qmc2SoftwareList->save();
    }
    delete qmc2SoftwareList;
  }
  if ( qmc2MESSDeviceConfigurator ) {
    log(QMC2_LOG_FRONTEND, tr("saving current machine's device configurations"));
    qmc2MESSDeviceConfigurator->save();
    delete qmc2MESSDeviceConfigurator;
  }
#endif

  if ( qmc2EmulatorOptions ) {
#if defined(QMC2_EMUTYPE_MAME)
    log(QMC2_LOG_FRONTEND, tr("destroying current game's emulator configuration"));
#elif defined(QMC2_EMUTYPE_MESS)
    log(QMC2_LOG_FRONTEND, tr("destroying current machine's emulator configuration"));
#endif
    QString selectedEmulator = comboBoxEmuSelector->currentText();
    if ( selectedEmulator == tr("Default") || selectedEmulator.isEmpty() )
      qmc2Config->remove(qmc2EmulatorOptions->settingsGroup + "/SelectedEmulator");
    else
      qmc2Config->setValue(qmc2EmulatorOptions->settingsGroup + "/SelectedEmulator", selectedEmulator);
    qmc2EmulatorOptions->save();
    delete qmc2EmulatorOptions;
  }
  log(QMC2_LOG_FRONTEND, tr("destroying global emulator options"));
  //delete qmc2GlobalEmulatorOptions; <- doing so will end up in heavy CPU load and a close-timeout at exit for Qt 4.3+
  //                              <- qmc2GlobalEmulatorOptions->setParent(0) fixes this (this is strange but true :)
  qmc2GlobalEmulatorOptions->pseudoDestructor();
  qmc2GlobalEmulatorOptions->setParent(0);
#if defined(QMC2_EMUTYPE_MAME)
  log(QMC2_LOG_FRONTEND, tr("destroying game list"));
#elif defined(QMC2_EMUTYPE_MESS)
  log(QMC2_LOG_FRONTEND, tr("destroying machine list"));
#endif
  delete qmc2Gamelist;

  if ( qmc2Preview ) {
    log(QMC2_LOG_FRONTEND, tr("destroying preview"));
    delete qmc2Preview;
  }
  if ( qmc2Flyer ) {
    log(QMC2_LOG_FRONTEND, tr("destroying flyer"));
    delete qmc2Flyer;
  }
  if ( qmc2Cabinet ) {
    log(QMC2_LOG_FRONTEND, tr("destroying cabinet"));
    delete qmc2Cabinet;
  }
  if ( qmc2Controller ) {
    log(QMC2_LOG_FRONTEND, tr("destroying controller"));
    delete qmc2Controller;
  }
  if ( qmc2Marquee ) {
    log(QMC2_LOG_FRONTEND, tr("destroying marquee"));
    delete qmc2Marquee;
  }
  if ( qmc2Title ) {
    log(QMC2_LOG_FRONTEND, tr("destroying title"));
    delete qmc2Title;
  }
  if ( qmc2PCB ) {
    log(QMC2_LOG_FRONTEND, tr("destroying PCB"));
    delete qmc2PCB;
  }
  if ( qmc2About ) {
    log(QMC2_LOG_FRONTEND, tr("destroying about dialog"));
    delete qmc2About;
  }
  if ( qmc2DocBrowser ) {
    log(QMC2_LOG_FRONTEND, tr("destroying MiniWebBrowser"));
    delete qmc2DocBrowser;
  }
#if defined(QMC2_EMUTYPE_MAME)
  if ( qmc2MAWSLookup ) {
    log(QMC2_LOG_FRONTEND, tr("destroying MAWS lookup"));
    delete qmc2MAWSLookup;
  }
  if ( qmc2MawsQuickDownloadSetup ) {
    log(QMC2_LOG_FRONTEND, tr("destroying MAWS quick download setup"));
    qmc2MawsQuickDownloadSetup->close();
    delete qmc2MawsQuickDownloadSetup;
  }
#elif defined(QMC2_EMUTYPE_MESS)
  if ( qmc2MAWSLookup ) {
    log(QMC2_LOG_FRONTEND, tr("destroying MESS wiki lookup"));
    delete qmc2MAWSLookup;
  }
#endif
  if ( qmc2ImageChecker ) {
    log(QMC2_LOG_FRONTEND, tr("destroying image checker"));
    qmc2ImageChecker->close();
    delete qmc2ImageChecker;
  }
  if ( qmc2SampleChecker ) {
    log(QMC2_LOG_FRONTEND, tr("destroying sample checker"));
    qmc2SampleChecker->close();
    delete qmc2SampleChecker;
  }
  if ( qmc2ROMAlyzer ) {
    log(QMC2_LOG_FRONTEND, tr("destroying ROMAlyzer"));
    qmc2ROMAlyzer->saveState();
    delete qmc2ROMAlyzer;
  }
  if ( qmc2ROMStatusExporter ) {
    log(QMC2_LOG_FRONTEND, tr("destroying ROM status exporter"));
    qmc2ROMStatusExporter->close();
    delete qmc2ROMStatusExporter;
  }
  if ( qmc2DetailSetup ) {
    log(QMC2_LOG_FRONTEND, tr("destroying detail setup"));
    qmc2DetailSetup->close();
    delete qmc2DetailSetup;
  }
#if defined(QMC2_EMUTYPE_MAME)
  if ( qmc2DemoModeDialog ) {
    log(QMC2_LOG_FRONTEND, tr("destroying demo mode dialog"));
    qmc2DemoModeDialog->close();
    delete qmc2DemoModeDialog;
  }
#endif

  if ( !qmc2GameInfoDB.isEmpty() ) {
#if defined(QMC2_EMUTYPE_MAME)
    log(QMC2_LOG_FRONTEND, tr("destroying game info DB"));
#elif defined(QMC2_EMUTYPE_MESS)
    log(QMC2_LOG_FRONTEND, tr("destroying machine info DB"));
#endif
    QMapIterator<QString, QByteArray *> it(qmc2GameInfoDB);
    QList<QByteArray *> deletedRecords;
    while ( it.hasNext() ) {
      it.next();
      if ( !deletedRecords.contains(it.value()) ) {
        if ( it.value() )
          delete it.value();
        deletedRecords.append(it.value());
      }
    }
    deletedRecords.clear();
    qmc2GameInfoDB.clear();
  }
  if ( !qmc2EmuInfoDB.isEmpty() ) {
    log(QMC2_LOG_FRONTEND, tr("destroying emulator info DB"));
    QMapIterator<QString, QByteArray *> it(qmc2EmuInfoDB);
    QList<QByteArray *> deletedRecords;
    while ( it.hasNext() ) {
      it.next();
      if ( !deletedRecords.contains(it.value()) ) {
        if ( it.value() )
          delete it.value();
        deletedRecords.append(it.value());
      }
    }
    deletedRecords.clear();
    qmc2EmuInfoDB.clear();
  }

  log(QMC2_LOG_FRONTEND, tr("destroying process manager"));
  if ( qmc2ProcessManager->procMap.count() > 0 ) {
#if defined(Q_WS_X11) || defined(Q_WS_WIN)
    for (int j = 0; j < tabWidgetEmbeddedEmulators->count(); j++) {
      Embedder *embedder = (Embedder *)tabWidgetEmbeddedEmulators->widget(j);
      if ( embedder )
        embedder->release();
    }
#endif
    if ( doKillEmulators ) {
      log(QMC2_LOG_FRONTEND, tr("killing %n running emulator(s) on exit", "", qmc2ProcessManager->procMap.count()));
      delete qmc2ProcessManager;
    } else
      log(QMC2_LOG_FRONTEND, tr("keeping %n running emulator(s) alive", "", qmc2ProcessManager->procMap.count()));
  } else 
    delete qmc2ProcessManager;

#if defined(QMC2_SDLMAME) || defined(QMC2_SDLMESS)
  if ( qmc2FifoFile ) {
    if ( qmc2FifoNotifier )
      delete qmc2FifoNotifier;
    if ( qmc2FifoFile->isOpen() )
      qmc2FifoFile->close();
    delete qmc2FifoFile;
#if defined(QMC2_SDLMAME)
//    ::unlink(QMC2_SDLMAME_OUTPUT_FIFO);
#elif defined(QMC2_SDLMESS)
//    ::unlink(QMC2_SDLMESS_OUTPUT_FIFO);
#endif
  }
#endif

  if ( qmc2NetworkAccessManager ) {
    log(QMC2_LOG_FRONTEND, tr("destroying network access manager"));
    delete qmc2NetworkAccessManager;
  }

  log(QMC2_LOG_FRONTEND, tr("so long and thanks for all the fish"));

  qmc2Config->setValue(QString(QMC2_FRONTEND_PREFIX + "InstanceRunning"), false);

  qInstallMsgHandler(0);
  delete qmc2KeyPressFilter;
  delete qmc2Options;
  e->accept();
}

void MainWindow::init()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::init()");
#endif

  if ( qmc2SplashScreen ) {
	qmc2SplashScreen->showMessage(tr("Welcome to QMC2 v%1!").arg(XSTR(QMC2_VERSION)) + "\n", Qt::AlignHCenter | Qt::AlignBottom, Qt::white);
	qmc2SplashScreen->show();
	qmc2SplashScreen->raise();
	qmc2SplashScreen->repaint();
	qApp->processEvents();
	QTimer::singleShot(QMC2_SPLASH_DURATION, qmc2SplashScreen, SLOT(hide()));
  }

#if defined(Q_WS_MAC)
  bool isShown = qmc2Options->isVisible();
  qmc2Options->setParent(this, Qt::Dialog);
  if ( isShown )
    qmc2Options->show();
#endif

#if defined(QMC2_EMUTYPE_MESS)
  treeWidgetGamelist->hideColumn(QMC2_GAMELIST_COLUMN_VERSION);
  treeWidgetGamelist->hideColumn(QMC2_GAMELIST_COLUMN_CATEGORY);
  treeWidgetHierarchy->hideColumn(QMC2_GAMELIST_COLUMN_VERSION);
  treeWidgetHierarchy->hideColumn(QMC2_GAMELIST_COLUMN_CATEGORY);
#endif

#if QMC2_USE_PHONON_API
  QTimer::singleShot(0, this, SLOT(on_toolButtonAudioSetupEffects_clicked()));
#endif

  createFifo();
  qmc2GhostImagePixmap.load(":/data/img/ghost.png");
  QString myStyle = qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Style", tr("Default")).toString();
  setupStyle(myStyle);
  if ( qApp->styleSheet().isEmpty() ) {
    QString myStyleSheet = qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/StyleSheet", "").toString();
    setupStyleSheet(myStyleSheet);
  }
  progressBarMemory->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/MemoryIndicator", false).toBool());

  // make sure the logs get scrolled to their maxima
  textBrowserFrontendLog->verticalScrollBar()->setValue(textBrowserFrontendLog->verticalScrollBar()->maximum());
  textBrowserEmulatorLog->verticalScrollBar()->setValue(textBrowserEmulatorLog->verticalScrollBar()->maximum());

  setUpdatesEnabled(true);

  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/RestoreLayout").toBool() )
    qmc2LastListIndex = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/GamelistTab", 0).toInt();
  else
    qmc2LastListIndex = 0;

#if defined(Q_WS_X11) || defined(Q_WS_WIN)
  if ( qmc2LastListIndex == QMC2_EMBED_INDEX )
    qmc2LastListIndex = 0;
#endif

  tabWidgetGamelist->setCurrentIndex(qmc2LastListIndex);
  on_tabWidgetGamelist_currentChanged(qmc2LastListIndex);

  qmc2EarlyStartup = false;

  QTimer::singleShot(0, this, SLOT(on_actionReload_activated()));
  activityCheckTimer.start(QMC2_ACTIVITY_CHECK_INTERVAL);

  // make sure the current detail's tab header is shown
  QTimer::singleShot(0, qmc2DetailSetup, SLOT(saveDetail()));

  floatToggleButtonSoftwareDetail_toggled(floatToggleButtonSoftwareDetail->isChecked());
}

void MainWindow::setupStyle(QString styleName)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::setupStyle(QString &styleName = %1").arg(styleName));
#endif

  static QPalette customPalette = QApplication::palette();

  QStyle *newStyle;
  if ( styleName != tr("Default") ) {
    if ( QStyleFactory::keys().contains(styleName) )
      newStyle = QStyleFactory::create(styleName);
    else
      newStyle = QStyleFactory::create(qmc2DefaultStyle);
  } else
    newStyle = QStyleFactory::create(qmc2DefaultStyle);

  QApplication::setStyle(newStyle);
  qApp->processEvents();

  if ( qApp->styleSheet().isEmpty() ) { // custom palettes and style sheets are mutually exclusive
    QPalette newPalette;
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/StandardColorPalette").toBool() )
      newPalette = QApplication::style()->standardPalette();
    else
      newPalette = customPalette;
    QApplication::setPalette(newPalette);
    if ( !qmc2EarlyStartup ) {
      // work around for an annoying Qt bug...
      menuBar()->setPalette(newPalette);
      toolbar->setPalette(newPalette);
    }
    qApp->processEvents();
  }
}

void MainWindow::setupStyleSheet(QString styleSheetName)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::setupStyleSheet(QString &styleSheetName = %1").arg(styleSheetName));
#endif

  static QString oldStyleSheetName = "";

  if ( !styleSheetName.isEmpty() ) {
    QFile f(styleSheetName);
    if ( f.open(QIODevice::ReadOnly) ) {
      if ( styleSheetName != oldStyleSheetName )
        log(QMC2_LOG_FRONTEND, tr("loading style sheet '%1'").arg(styleSheetName));
      QString currentDir = QDir::currentPath();
      QDir::setCurrent(QFileInfo(f).absolutePath());
      qApp->setStyleSheet("");
      qApp->setStyleSheet(f.readAll());
      f.close();
      // FIXME: "cd -" won't work because of relative URLs
      // QDir::setCurrent(currentDir);
    } else
      log(QMC2_LOG_FRONTEND, tr("FATAL: can't open style sheet file '%1', please check").arg(styleSheetName));
  } else {
    if ( !qApp->styleSheet().isEmpty() )
      log(QMC2_LOG_FRONTEND, tr("removing current style sheet"));
    qApp->setStyleSheet("");
  }

  oldStyleSheetName = styleSheetName;

  qApp->processEvents();
}

void MainWindow::viewFullDetail()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::viewFullDetail()");
#endif

  comboBoxViewSelect->setCurrentIndex(QMC2_VIEWGAMELIST_INDEX);
  tabWidgetGamelist->setCurrentIndex(QMC2_GAMELIST_INDEX);
  tabWidgetGamelist->setTabIcon(QMC2_GAMELIST_INDEX, QIcon(QString::fromUtf8(":/data/img/flat.png")));
  menuView->setIcon(QIcon(QString::fromUtf8(":/data/img/flat.png")));
  treeWidgetGamelist->setFocus();
  stackedWidgetView->update();
  qApp->processEvents();
}

void MainWindow::viewParentClones()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::viewParentClones()");
#endif

  comboBoxViewSelect->setCurrentIndex(QMC2_VIEWHIERARCHY_INDEX);
  tabWidgetGamelist->setCurrentIndex(QMC2_GAMELIST_INDEX);
  tabWidgetGamelist->setTabIcon(QMC2_GAMELIST_INDEX, QIcon(QString::fromUtf8(":/data/img/clone.png")));
  menuView->setIcon(QIcon(QString::fromUtf8(":/data/img/clone.png")));
  treeWidgetHierarchy->setFocus();
  stackedWidgetView->update();
  qApp->processEvents();
}

#if defined(QMC2_EMUTYPE_MAME)
void MainWindow::viewByCategory()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::viewByCategory()");
#endif

  comboBoxViewSelect->setCurrentIndex(QMC2_VIEWCATEGORY_INDEX);
  tabWidgetGamelist->setCurrentIndex(QMC2_GAMELIST_INDEX);
  tabWidgetGamelist->setTabIcon(QMC2_GAMELIST_INDEX, QIcon(QString::fromUtf8(":/data/img/category.png")));
  menuView->setIcon(QIcon(QString::fromUtf8(":/data/img/category.png")));
  QTreeWidgetItem *item = treeWidgetCategoryView->topLevelItem(0);
  if ( item ) {
    if ( item->text(QMC2_GAMELIST_COLUMN_GAME) == tr("Waiting for data...") )
      QTimer::singleShot(0, qmc2Gamelist, SLOT(createCategoryView()));
  } else
    QTimer::singleShot(0, qmc2Gamelist, SLOT(createCategoryView()));
  treeWidgetCategoryView->setFocus();
  stackedWidgetView->update();
  qApp->processEvents();
}

void MainWindow::viewByVersion()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::viewByVersion()");
#endif

  comboBoxViewSelect->setCurrentIndex(QMC2_VIEWVERSION_INDEX);
  tabWidgetGamelist->setCurrentIndex(QMC2_GAMELIST_INDEX);
  tabWidgetGamelist->setTabIcon(QMC2_GAMELIST_INDEX, QIcon(QString::fromUtf8(":/data/img/version.png")));
  menuView->setIcon(QIcon(QString::fromUtf8(":/data/img/version.png")));
  QTreeWidgetItem *item = treeWidgetVersionView->topLevelItem(0);
  if ( item ) {
    if ( item->text(QMC2_GAMELIST_COLUMN_GAME) == tr("Waiting for data...") )
      QTimer::singleShot(0, qmc2Gamelist, SLOT(createVersionView()));
  } else
    QTimer::singleShot(0, qmc2Gamelist, SLOT(createVersionView()));
  treeWidgetVersionView->setFocus();
  stackedWidgetView->update();
  qApp->processEvents();
}
#endif

bool KeyPressFilter::eventFilter(QObject *object, QEvent *event)
{
  if (event->type() == QEvent::KeyPress) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

#ifdef QMC2_DEBUG
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: KeyPressFilter::eventFilter(QObject *object = %1, QEvent *event = %2)").arg((qulonglong)object).arg((qulonglong)event));
#endif
    
    if ( keyEvent->text() == QString("QMC2_EMULATED_KEY") ) {
#ifdef QMC2_DEBUG
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: emulated key event"));
#endif
      return false;
    }

    int myKeySeq = 0;
    if ( keyEvent->modifiers() & Qt::ShiftModifier )
      myKeySeq += Qt::SHIFT;
    if ( keyEvent->modifiers() & Qt::ControlModifier )
      myKeySeq += Qt::CTRL;
    if ( keyEvent->modifiers() & Qt::AltModifier )
      myKeySeq += Qt::ALT;
    if ( keyEvent->modifiers() & Qt::MetaModifier )
      myKeySeq += Qt::META;
    myKeySeq += keyEvent->key();

    QString pressedKeySeq = QKeySequence(myKeySeq).toString();
#ifdef QMC2_DEBUG
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: current key-sequence = '%1'").arg(pressedKeySeq));
#endif

    QString matchedKeySeq = qmc2CustomShortcutMap.key(pressedKeySeq);
    if ( !matchedKeySeq.isEmpty() ) {
      if ( !qmc2MainWindow->menuBar()->isVisible() ) {
        QPair<QString, QAction *> actionPair = qmc2ShortcutMap[qmc2CustomShortcutMap.key(pressedKeySeq)];
        if ( actionPair.second )
        {
#ifdef QMC2_DEBUG
          qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: '%1' pressed, emulating key event (due to no menu bar)").arg(pressedKeySeq));
#endif
          actionPair.second->trigger();
          return true;
        }
      }

      if ( matchedKeySeq != pressedKeySeq ) {
#ifdef QMC2_DEBUG
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: emulating key event for '%1'").arg(matchedKeySeq));
#endif
        // emulate a key event for the mapped key
        QKeySequence emulatedKeySequence(matchedKeySeq);
        QKeyEvent *emulatedKeyEvent = new QKeyEvent(QEvent::KeyPress, emulatedKeySequence[0], Qt::NoModifier, QString("QMC2_EMULATED_KEY"));
        qApp->postEvent(object, emulatedKeyEvent);
        return true;
      } else {
#ifdef QMC2_DEBUG
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: '%1' pressed, default key event processing").arg(pressedKeySeq));
#endif
        // default key event processing
        return false;
      }
    }

    QMap<QString, QString>::const_iterator it = qmc2CustomShortcutMap.find(pressedKeySeq);
    if ( it != qmc2CustomShortcutMap.end() ) {
      if ( !it.value().isEmpty() ) {
#ifdef QMC2_DEBUG
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: '%1' pressed, key event suppressed").arg(pressedKeySeq));
#endif
        return true;
      } else {
#ifdef QMC2_DEBUG
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: '%1' pressed, default key event processing").arg(pressedKeySeq));
#endif
        return false;
      }
    }

#ifdef QMC2_DEBUG
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: '%1' pressed, default key event processing").arg(pressedKeySeq));
#endif
    return false;
  } else {
    // default event processing
    return QObject::eventFilter(object, event);
  }
}

#if defined(QMC2_YOUTUBE_ENABLED)
void MainWindow::loadYouTubeVideoInfoMap()
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::loadYouTubeVideoInfoMap()");
#endif

	log(QMC2_LOG_FRONTEND, tr("loading YouTube video info map"));
	QDir youTubeCacheDir(qmc2Config->value(QMC2_FRONTEND_PREFIX + "YouTubeWidget/CacheDirectory").toString());
	if ( youTubeCacheDir.exists() ) {
#if defined(QMC2_SDLMAME)
		QFile f(youTubeCacheDir.canonicalPath() + "/qmc2-sdlmame.yti");
#elif defined(QMC2_SDLMESS)
		QFile f(youTubeCacheDir.canonicalPath() + "/qmc2-sdlmess.yti");
#elif defined(QMC2_MAME)
		QFile f(youTubeCacheDir.canonicalPath() + "/qmc2-mame.yti");
#elif defined(QMC2_MESS)
		QFile f(youTubeCacheDir.canonicalPath() + "/qmc2-mess.yti");
#else
		QFile f(youTubeCacheDir.canonicalPath() + "/qmc2.yti");
#endif
		if ( f.open(QIODevice::ReadOnly | QIODevice::Text) ) {
			QString oldFormat = progressBarGamelist->format();
			if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
				progressBarGamelist->setFormat(tr("YouTube index - %p%"));
			else
				progressBarGamelist->setFormat("%p%");
			QFileInfo fi(f.fileName());
			progressBarGamelist->setRange(0, fi.size());
			progressBarGamelist->setValue(0);
			qmc2YouTubeVideoInfoMap.clear();
			QTextStream ts(&f);
			quint64 viCounter = 0;
			quint64 curLen = 0;
			while ( !ts.atEnd() ) {
				QString line = ts.readLine();
#if defined(Q_WS_WIN)
				curLen += line.length() + 2; // + 0x0d 0x0a
#else
				curLen += line.length() + 1; // + 0x0a
#endif
				if ( viCounter++ % QMC2_YOUTUBE_VIDEO_INFO_RSP == 0 )
					progressBarGamelist->setValue(curLen);
				if ( !line.startsWith("#") ) {
					QStringList tokens = line.split("\t");
					if ( tokens.count() > 2 )
						qmc2YouTubeVideoInfoMap[tokens[0]] = YouTubeVideoInfo(tokens[2], tokens[1]);
				}
			}
			progressBarGamelist->reset();
			progressBarGamelist->setFormat(oldFormat);
		}
	}
	log(QMC2_LOG_FRONTEND, tr("done (loading YouTube video info map)"));
	log(QMC2_LOG_FRONTEND, tr("%n video info record(s) loaded", "", qmc2YouTubeVideoInfoMap.count()));
	qmc2YouTubeVideoInfoMapChanged = false;
}
#endif

void MainWindow::loadGameInfoDB()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::loadGameInfoDB()");
#endif

  QTime gameInfoElapsedTime,
        gameInfoTimer;

  qmc2LoadingGameInfoDB = true;
  qmc2StopParser = false;
#if defined(QMC2_EMUTYPE_MAME)
  log(QMC2_LOG_FRONTEND, tr("loading game info DB"));
#elif defined(QMC2_EMUTYPE_MESS)
  log(QMC2_LOG_FRONTEND, tr("loading machine info DB"));
#endif

  gameInfoTimer.start();

  // clear game/machine info DB
  QMapIterator<QString, QByteArray *> it(qmc2GameInfoDB);
  QList<QByteArray *> deletedRecords;
  while ( it.hasNext() ) {
    it.next();
    if ( !deletedRecords.contains(it.value()) ) {
      if ( it.value() )
        delete it.value();
      deletedRecords.append(it.value());
    }
  }
  deletedRecords.clear();
  qmc2GameInfoDB.clear();

  bool compressData = qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/CompressGameInfoDB").toBool();
#if defined(QMC2_EMUTYPE_MAME)
  QString pathToGameInfoDB = qmc2Config->value("MAME/FilesAndDirectories/GameInfoDB").toString();
#elif defined(QMC2_EMUTYPE_MESS)
  QString pathToGameInfoDB = qmc2Config->value("MESS/FilesAndDirectories/GameInfoDB").toString();
#endif
  QFile gameInfoDB(pathToGameInfoDB);
  gameInfoDB.open(QIODevice::ReadOnly | QIODevice::Text);

  if ( gameInfoDB.isOpen() ) {
    qmc2MainWindow->progressBarGamelist->reset();
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
#if defined(QMC2_EMUTYPE_MAME)
      progressBarGamelist->setFormat(tr("Game info - %p%"));
#elif defined(QMC2_EMUTYPE_MESS)
      progressBarGamelist->setFormat(tr("Machine info - %p%"));
#endif
    else
      progressBarGamelist->setFormat("%p%");
    progressBarGamelist->setRange(0, gameInfoDB.size());
    qApp->processEvents();
    QTextStream ts(&gameInfoDB);
    ts.setCodec(QTextCodec::codecForName("ISO 8859-1"));
    int recordsProcessed = 0;
    while ( !ts.atEnd() && !qmc2StopParser ) {
      QString singleLine = ts.readLine();
      while ( !singleLine.simplified().startsWith("$info=") && !ts.atEnd() ) {
        singleLine = ts.readLine();
        if ( recordsProcessed++ % QMC2_INFOSOURCE_RESPONSIVENESS == 0 ) {
          progressBarGamelist->setValue(gameInfoDB.pos());
          qApp->processEvents();
        }
      }
      if ( singleLine.simplified().startsWith("$info=") ) {
        QStringList gameWords = singleLine.simplified().mid(6).split(",");
        while ( !singleLine.simplified().startsWith("$bio") && !ts.atEnd() ) {
          singleLine = ts.readLine();
          if ( recordsProcessed++ % QMC2_INFOSOURCE_RESPONSIVENESS == 0 ) {
            progressBarGamelist->setValue(gameInfoDB.pos());
            qApp->processEvents();
          }
        }
        if ( singleLine.simplified().startsWith("$bio") ) {
          QString gameInfoString;
          bool firstLine = true;
          while ( !singleLine.simplified().startsWith("$end") && !ts.atEnd() ) {
            singleLine = ts.readLine();
            if ( !singleLine.simplified().startsWith("$end") ) {
#if defined(QMC2_EMUTYPE_MESS)
              if ( !firstLine ) {
                  gameInfoString.append(singleLine + "<br>");
              } else if ( !singleLine.isEmpty() ) {
                gameInfoString.append("<b>" + singleLine + "</b><br>");
                firstLine = false;
              }
#else
              if ( !firstLine ) {
                  gameInfoString.append(singleLine.trimmed() + "<br>");
              } else if ( !singleLine.isEmpty() ) {
                gameInfoString.append("<b>" + singleLine.trimmed() + "</b><br>");
                firstLine = false;
              }
#endif
            }
            if ( recordsProcessed++ % QMC2_INFOSOURCE_RESPONSIVENESS == 0 ) {
              progressBarGamelist->setValue(gameInfoDB.pos());
              qApp->processEvents();
            }
          }
          if ( singleLine.simplified().startsWith("$end") ) {
            // reduce the number of line breaks
            gameInfoString.replace("<br><br><br><br>", "<p>");
            gameInfoString.replace("<br><br><br>", "<p>");
            gameInfoString.replace("<br><br>", "<p>");
            if ( gameInfoString.endsWith("<p>") )
              gameInfoString.remove(gameInfoString.length() - 3, gameInfoString.length() - 1);
            QByteArray *gameInfo;
            if ( compressData )
              gameInfo = new QByteArray(QMC2_COMPRESS(QTextCodec::codecForCStrings()->fromUnicode(gameInfoString))); 
            else
              gameInfo = new QByteArray(QTextCodec::codecForCStrings()->fromUnicode(gameInfoString));
            int i;
            for (i = 0; i < gameWords.count(); i++) {
              if ( !gameWords[i].isEmpty() )
                qmc2GameInfoDB[gameWords[i]] = gameInfo;
            }
          } else {
#if defined(QMC2_EMUTYPE_MAME)
            log(QMC2_LOG_FRONTEND, tr("WARNING: missing '$end' in game info DB %1").arg(pathToGameInfoDB));
#elif defined(QMC2_EMUTYPE_MESS)
            log(QMC2_LOG_FRONTEND, tr("WARNING: missing '$end' in machine info DB %1").arg(pathToGameInfoDB));
#endif
          }
        } else {
#if defined(QMC2_EMUTYPE_MAME)
          log(QMC2_LOG_FRONTEND, tr("WARNING: missing '$bio' in game info DB %1").arg(pathToGameInfoDB));
#elif defined(QMC2_EMUTYPE_MESS)
          log(QMC2_LOG_FRONTEND, tr("WARNING: missing '$bio' in machine info DB %1").arg(pathToGameInfoDB));
#endif
        }
      } else if ( !ts.atEnd() ) {
#if defined(QMC2_EMUTYPE_MAME)
        log(QMC2_LOG_FRONTEND, tr("WARNING: missing '$info' in game info DB %1").arg(pathToGameInfoDB));
#elif defined(QMC2_EMUTYPE_MESS)
        log(QMC2_LOG_FRONTEND, tr("WARNING: missing '$info' in machine info DB %1").arg(pathToGameInfoDB));
#endif
      }
      if ( recordsProcessed++ % QMC2_INFOSOURCE_RESPONSIVENESS == 0 ) {
        progressBarGamelist->setValue(gameInfoDB.pos());
        qApp->processEvents();
      }
    }
    progressBarGamelist->setValue(gameInfoDB.pos());
    gameInfoDB.close();
  } else {
#if defined(QMC2_EMUTYPE_MAME)
    log(QMC2_LOG_FRONTEND, tr("WARNING: can't open game info DB %1").arg(pathToGameInfoDB));
#elif defined(QMC2_EMUTYPE_MESS)
    log(QMC2_LOG_FRONTEND, tr("WARNING: can't open machine info DB %1").arg(pathToGameInfoDB));
#endif
  }

  gameInfoElapsedTime = gameInfoElapsedTime.addMSecs(gameInfoTimer.elapsed());
#if defined(QMC2_EMUTYPE_MAME)
  log(QMC2_LOG_FRONTEND, tr("done (loading game info DB, elapsed time = %1)").arg(gameInfoElapsedTime.toString("mm:ss.zzz")));
  log(QMC2_LOG_FRONTEND, tr("%n game info record(s) loaded", "", qmc2GameInfoDB.count()));
  if ( qmc2StopParser ) {
    log(QMC2_LOG_FRONTEND, tr("invalidating game info DB"));
#elif defined(QMC2_EMUTYPE_MESS)
  log(QMC2_LOG_FRONTEND, tr("done (loading machine info DB, elapsed time = %1)").arg(gameInfoElapsedTime.toString("mm:ss.zzz")));
  log(QMC2_LOG_FRONTEND, tr("%n machine info record(s) loaded", "", qmc2GameInfoDB.count()));
  if ( qmc2StopParser ) {
    log(QMC2_LOG_FRONTEND, tr("invalidating machine info DB"));
#endif
    QMapIterator<QString, QByteArray *> it(qmc2GameInfoDB);
    QList<QByteArray *> deletedRecords;
    while ( it.hasNext() ) {
      it.next();
      if ( !deletedRecords.contains(it.value()) ) {
        if ( it.value() )
          delete it.value();
        deletedRecords.append(it.value());
      }
    }
    deletedRecords.clear();
    qmc2GameInfoDB.clear();
  }
  qmc2LoadingGameInfoDB = false;
  qmc2MainWindow->progressBarGamelist->reset();
}

void MainWindow::loadEmuInfoDB()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::loadEmuInfoDB()");
#endif

  QTime emuInfoElapsedTime,
        emuInfoTimer;

  qmc2LoadingEmuInfoDB = true;
  qmc2StopParser = false;
  log(QMC2_LOG_FRONTEND, tr("loading emulator info DB"));
  emuInfoTimer.start();

  // clear emulator info DB
  QMapIterator<QString, QByteArray *> it(qmc2EmuInfoDB);
  QList<QByteArray *> deletedRecords;
  while ( it.hasNext() ) {
    it.next();
    if ( !deletedRecords.contains(it.value()) ) {
      if ( it.value() )
        delete it.value();
      deletedRecords.append(it.value());
    }
  }
  deletedRecords.clear();
  qmc2EmuInfoDB.clear();

  bool compressData = qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/CompressEmuInfoDB").toBool();
#if defined(QMC2_EMUTYPE_MAME)
  QString pathToEmuInfoDB = qmc2Config->value("MAME/FilesAndDirectories/EmuInfoDB").toString();
#elif defined(QMC2_EMUTYPE_MESS)
  QString pathToEmuInfoDB = qmc2Config->value("MESS/FilesAndDirectories/EmuInfoDB").toString();
#endif
  QFile emuInfoDB(pathToEmuInfoDB);
  emuInfoDB.open(QIODevice::ReadOnly | QIODevice::Text);

  if ( emuInfoDB.isOpen() ) {
    qmc2MainWindow->progressBarGamelist->reset();
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
      progressBarGamelist->setFormat(tr("Emu info - %p%"));
    else
      progressBarGamelist->setFormat("%p%");
    progressBarGamelist->setRange(0, emuInfoDB.size());
    qApp->processEvents();
    QTextStream ts(&emuInfoDB);
    ts.setCodec(QTextCodec::codecForName("ISO 8859-1"));
    int recordsProcessed = 0;
    while ( !ts.atEnd() && !qmc2StopParser ) {
      QString singleLine = ts.readLine();
      while ( !singleLine.simplified().startsWith("$info=") && !ts.atEnd() ) {
        singleLine = ts.readLine();
        if ( recordsProcessed++ % QMC2_INFOSOURCE_RESPONSIVENESS == 0 ) {
          progressBarGamelist->setValue(emuInfoDB.pos());
          qApp->processEvents();
        }
      }
      if ( singleLine.simplified().startsWith("$info=") ) {
        QStringList gameWords = singleLine.simplified().mid(6).split(",");
        while ( !singleLine.simplified().startsWith("$mame") && !ts.atEnd() ) {
          singleLine = ts.readLine();
          if ( recordsProcessed++ % QMC2_INFOSOURCE_RESPONSIVENESS == 0 ) {
            progressBarGamelist->setValue(emuInfoDB.pos());
            qApp->processEvents();
          }
        }
        if ( singleLine.simplified().startsWith("$mame") ) {
          QString emuInfoString;
          while ( !singleLine.simplified().startsWith("$end") && !ts.atEnd() ) {
            singleLine = ts.readLine();
            if ( !singleLine.simplified().startsWith("$end") )
              emuInfoString.append(singleLine + "<br>");
            if ( recordsProcessed++ % QMC2_INFOSOURCE_RESPONSIVENESS == 0 ) {
              progressBarGamelist->setValue(emuInfoDB.pos());
              qApp->processEvents();
            }
          }
          if ( singleLine.simplified().startsWith("$end") ) {
            // convert "two (or more) empty lines" to a paragraph delimiter
            emuInfoString = emuInfoString.replace("<br><br><br>", "<p>").replace("<br><br>", "<p>");
            if ( emuInfoString.startsWith("<br>") )
              emuInfoString.remove(0, 4);
            if ( emuInfoString.endsWith("<p>") )
              emuInfoString.remove(emuInfoString.length() - 3, emuInfoString.length() - 1);
            QByteArray *emuInfo;
            if ( compressData )
              emuInfo = new QByteArray(QMC2_COMPRESS(QTextCodec::codecForCStrings()->fromUnicode(emuInfoString))); 
            else
              emuInfo = new QByteArray(QTextCodec::codecForCStrings()->fromUnicode(emuInfoString));
            int i;
            for (i = 0; i < gameWords.count(); i++) {
              if ( !gameWords[i].isEmpty() )
                qmc2EmuInfoDB[gameWords[i]] = emuInfo;
            }
          } else {
            log(QMC2_LOG_FRONTEND, tr("WARNING: missing '$end' in emulator info DB %1").arg(pathToEmuInfoDB));
          }
        } else if ( !ts.atEnd() ) {
          log(QMC2_LOG_FRONTEND, tr("WARNING: missing '$mame' in emulator info DB %1").arg(pathToEmuInfoDB));
        }
      } else if ( !ts.atEnd() ) {
        log(QMC2_LOG_FRONTEND, tr("WARNING: missing '$info' in emulator info DB %1").arg(pathToEmuInfoDB));
      }
      if ( recordsProcessed++ % QMC2_INFOSOURCE_RESPONSIVENESS == 0 ) {
        progressBarGamelist->setValue(emuInfoDB.pos());
        qApp->processEvents();
      }
    }
    progressBarGamelist->setValue(emuInfoDB.pos());
    emuInfoDB.close();
  } else
    log(QMC2_LOG_FRONTEND, tr("WARNING: can't open emulator info DB %1").arg(pathToEmuInfoDB));

  emuInfoElapsedTime = emuInfoElapsedTime.addMSecs(emuInfoTimer.elapsed());
  log(QMC2_LOG_FRONTEND, tr("done (loading emulator info DB, elapsed time = %1)").arg(emuInfoElapsedTime.toString("mm:ss.zzz")));
  log(QMC2_LOG_FRONTEND, tr("%n emulator info record(s) loaded", "", qmc2EmuInfoDB.count()));
  if ( qmc2StopParser ) {
    log(QMC2_LOG_FRONTEND, tr("invalidating emulator info DB"));
    QMapIterator<QString, QByteArray *> it(qmc2EmuInfoDB);
    QList<QByteArray *> deletedRecords;
    while ( it.hasNext() ) {
      it.next();
      if ( !deletedRecords.contains(it.value()) ) {
        if ( it.value() )
          delete it.value();
        deletedRecords.append(it.value());
      }
    }
    deletedRecords.clear();
    qmc2EmuInfoDB.clear();
  }
  qmc2LoadingEmuInfoDB = false;
  qmc2MainWindow->progressBarGamelist->reset();
}

#if QMC2_USE_PHONON_API
void MainWindow::on_actionAudioPreviousTrack_triggered(bool checked)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionAudioPreviousTrack_triggered(bool checked = ...)");
#endif

  toolButtonAudioPreviousTrack->setDown(true);
  QTimer::singleShot(QMC2_BUTTON_ANIMATION_TIMEOUT, this, SLOT(on_toolButtonAudioPreviousTrack_resetButton()));
  audioFastForwarding = audioFastBackwarding = false;
  audioSkippingTracks = true;

  if ( listWidgetAudioPlaylist->count() > 0 ) {
    QList<QListWidgetItem *> sl = listWidgetAudioPlaylist->selectedItems();
    QListWidgetItem *ci = NULL;
    if ( sl.count() > 0 )
      ci = sl[0];
    if ( !ci )
      ci = listWidgetAudioPlaylist->currentItem();
    int row;
    if ( ci )
      row = listWidgetAudioPlaylist->currentRow() - 1;
    else
      row = listWidgetAudioPlaylist->count() - 1;
    if ( row < 0 )
      row = listWidgetAudioPlaylist->count() - 1;
    listWidgetAudioPlaylist->clearSelection();
    listWidgetAudioPlaylist->setCurrentRow(row);
    ci = listWidgetAudioPlaylist->currentItem();
    switch ( audioState ) {
      case Phonon::PlayingState:
        QTimer::singleShot(0, this, SLOT(on_actionAudioPlayTrack_triggered()));
        break;

      default:
	QTimer::singleShot(0, this, SLOT(on_actionAudioStopTrack_triggered()));
        break;
    }
  }
}

void MainWindow::on_toolButtonAudioPreviousTrack_resetButton()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_toolButtonAudioPreviousTrack_resetButton()");
#endif

  toolButtonAudioPreviousTrack->setDown(false);
}

void MainWindow::on_actionAudioNextTrack_triggered(bool checked)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionAudioNextTrack_triggered(bool checked = ...)");
#endif

  toolButtonAudioNextTrack->setDown(true);
  QTimer::singleShot(QMC2_BUTTON_ANIMATION_TIMEOUT, this, SLOT(on_toolButtonAudioNextTrack_resetButton()));
  audioFastForwarding = audioFastBackwarding = false;
  audioSkippingTracks = true;

  if ( listWidgetAudioPlaylist->count() > 0 ) {
    QList<QListWidgetItem *> sl = listWidgetAudioPlaylist->selectedItems();
    QListWidgetItem *ci = NULL;
    if ( sl.count() > 0 )
      ci = sl[0];  
    if ( !ci )
      ci = listWidgetAudioPlaylist->currentItem();
    int row;
    if ( ci )
      row = listWidgetAudioPlaylist->currentRow() + 1;
    else
      row = 0;
    if ( row > listWidgetAudioPlaylist->count() - 1 )
      row = 0;
    listWidgetAudioPlaylist->clearSelection();
    listWidgetAudioPlaylist->setCurrentRow(row);
    ci = listWidgetAudioPlaylist->currentItem();
    switch ( audioState ) {
      case Phonon::PlayingState:
        QTimer::singleShot(0, this, SLOT(on_actionAudioPlayTrack_triggered()));
        break;

      default:
	QTimer::singleShot(0, this, SLOT(on_actionAudioStopTrack_triggered()));
        break;
    }
  }
}

void MainWindow::on_toolButtonAudioNextTrack_resetButton()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_toolButtonAudioNextTrack_resetButton()");
#endif

  toolButtonAudioNextTrack->setDown(false);
}

void MainWindow::on_actionAudioFastBackward_triggered(bool checked)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionAudioFastBackward_triggered(bool checked = ...)");
#endif

  toolButtonAudioFastBackward->setDown(true);
  QTimer::singleShot(QMC2_BUTTON_ANIMATION_TIMEOUT, this, SLOT(on_toolButtonAudioFastBackward_resetButton()));

  on_toolButtonAudioFastBackward_clicked(checked);
}

void MainWindow::on_toolButtonAudioFastBackward_clicked(bool checked)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_toolButtonAudioFastBackward_clicked(bool checked = ...)");
#endif

  qint64 newTime = phononAudioPlayer->currentTime();
  if ( newTime > 0 ) {
    newTime -= QMC2_AUDIOPLAYER_SEEK_OFFSET;
    audioFastBackwarding = true;
    phononAudioPlayer->seek(newTime);
    audioTick(newTime);
  }
}

void MainWindow::on_toolButtonAudioFastBackward_resetButton()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_toolButtonAudioFastBackward_resetButton()");
#endif

  toolButtonAudioFastBackward->setDown(false);
  audioFastForwarding = false;
}

void MainWindow::on_actionAudioFastForward_triggered(bool checked)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionAudioFastForward_triggered(bool checked = ...)");
#endif

  toolButtonAudioFastForward->setDown(true);
  QTimer::singleShot(QMC2_BUTTON_ANIMATION_TIMEOUT, this, SLOT(on_toolButtonAudioFastForward_resetButton()));

  on_toolButtonAudioFastForward_clicked(checked);
}

void MainWindow::on_toolButtonAudioFastForward_clicked(bool checked)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_toolButtonFastForward_clicked(bool checked = ...)");
#endif

  qint64 newTime = phononAudioPlayer->currentTime();
  if ( newTime > 0 ) {
    newTime += QMC2_AUDIOPLAYER_SEEK_OFFSET;
    audioFastForwarding = true;
    phononAudioPlayer->seek(newTime);
    audioTick(newTime);
  }
}

void MainWindow::on_toolButtonAudioFastForward_resetButton()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_toolButtonAudioFastForward_resetButton()");
#endif

  toolButtonAudioFastForward->setDown(false);
  audioFastForwarding = false;
}

void MainWindow::on_actionAudioStopTrack_triggered(bool checked)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionAudioStopTrack_triggered(bool checked = ...)");
#endif

  actionAudioStopTrack->setChecked(true);
  actionAudioPauseTrack->setChecked(false);
  actionAudioPlayTrack->setChecked(false);
  audioFastForwarding = audioFastBackwarding = audioSkippingTracks = false;
  phononAudioPlayer->stop();
  progressBarAudioProgress->setRange(0, 100);
  progressBarAudioProgress->reset();
  audioState = Phonon::StoppedState;
}

void MainWindow::on_actionAudioPauseTrack_triggered(bool checked)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionAudioPauseTrack_triggered(bool checked = ...)");
#endif

  actionAudioPauseTrack->setChecked(true);
  actionAudioStopTrack->setChecked(false);
  actionAudioPlayTrack->setChecked(false);
  audioFastForwarding = audioFastBackwarding = audioSkippingTracks = false;
  if ( checkBoxAudioFade->isChecked() && audioState == Phonon::PlayingState )
    audioFade(QMC2_AUDIOPLAYER_FADER_PAUSE);
  else
    phononAudioPlayer->pause();
  audioState = Phonon::PausedState;
}

void MainWindow::on_actionAudioPlayTrack_triggered(bool checked)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionAudioPlayTrack_triggered(bool checked = ...)");
#endif

  // if this is a URL media source, force a reconnect to the stream...
  if ( phononAudioPlayer->currentSource().type() == Phonon::MediaSource::Url )
    phononAudioPlayer->setCurrentSource(phononAudioPlayer->currentSource().url());

  static QString audioPlayerCurrentTrack;
  audioFastForwarding = audioFastBackwarding = false;
  if ( audioState == Phonon::PausedState ) {
    if ( qmc2ProcessManager->sentPlaySignal && qmc2ProcessManager->procMap.count() > 0 ) {
      qmc2ProcessManager->musicWasPlaying = true;
    } else if ( checkBoxAudioFade->isChecked() ) {
      audioFade(QMC2_AUDIOPLAYER_FADER_PLAY);
    } else {
      phononAudioPlayer->play();
      actionAudioPlayTrack->setChecked(true);
      actionAudioStopTrack->setChecked(false);
      actionAudioPauseTrack->setChecked(false);
    }
    qmc2ProcessManager->sentPlaySignal = false;
    audioState = Phonon::PlayingState;
  } else if ( listWidgetAudioPlaylist->count() > 0 ) {
    QList<QListWidgetItem *> sl = listWidgetAudioPlaylist->selectedItems();
    QListWidgetItem *ci = NULL;
    if ( sl.count() > 0 )
      ci = sl[0];
    if ( !ci ) {
      if ( !qmc2AudioLastIndividualTrack.isEmpty() ) {
        audioScrollToCurrentItem();
        ci = listWidgetAudioPlaylist->currentItem();
      }
      if ( !ci ) {
        listWidgetAudioPlaylist->setCurrentRow(0);
        ci = listWidgetAudioPlaylist->currentItem();
      }
    }
    if ( ci->text() != audioPlayerCurrentTrack ) {
      progressBarAudioProgress->reset();
      audioPlayerCurrentTrack = ci->text();
      listWidgetAudioPlaylist->scrollToItem(ci, qmc2CursorPositioningMode);
      phononAudioPlayer->setCurrentSource(Phonon::MediaSource(audioPlayerCurrentTrack));
    }
    phononAudioPlayer->play();
    actionAudioPlayTrack->setChecked(true);
    actionAudioStopTrack->setChecked(false);
    actionAudioPauseTrack->setChecked(false);
    audioState = Phonon::PlayingState;
  } else
    on_actionAudioStopTrack_triggered(true);
}

void MainWindow::on_toolButtonAudioAddTracks_clicked()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_toolButtonAudioAddTracks_clicked()");
#endif

  QStringList sl = QFileDialog::getOpenFileNames(this, tr("Select one or more audio files"), QString(), tr("All files (*)"));
  if ( sl.count() > 0 )
    listWidgetAudioPlaylist->addItems(sl);
}

void MainWindow::on_toolButtonAudioAddURL_clicked()
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_toolButtonAudioAddURL_clicked()");
#endif

	bool ok;
	QString streamUrl = QInputDialog::getText(this, tr("Add URL"), tr("Enter valid MP3 stream URL:"), QLineEdit::Normal, "", &ok);
	if ( ok && !streamUrl.isEmpty() )
		listWidgetAudioPlaylist->addItem(streamUrl);
}

void MainWindow::on_toolButtonAudioSetupEffects_clicked()
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_toolButtonAudioSetupEffects_clicked()");
#endif

	static bool audioSetupEffectsFirstCall = true;

	if ( !qmc2AudioEffectDialog )
		qmc2AudioEffectDialog = new AudioEffectDialog(this);

	if ( !audioSetupEffectsFirstCall ) {
		qmc2AudioEffectDialog->show();
		qmc2AudioEffectDialog->raise();
	}

	audioSetupEffectsFirstCall = false;
}

void MainWindow::on_toolButtonAudioRemoveTracks_clicked()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_toolButtonAudioRemoveTracks_clicked()");
#endif

  QList<QListWidgetItem *> sl = listWidgetAudioPlaylist->selectedItems();
  foreach (QListWidgetItem *item, sl) {
    item = listWidgetAudioPlaylist->takeItem(listWidgetAudioPlaylist->row(item));
    delete item;
  }
}

void MainWindow::on_listWidgetAudioPlaylist_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_listWidgetAudioPlaylist_itemSelectionChanged()");
#endif

  QList<QListWidgetItem *> sl = listWidgetAudioPlaylist->selectedItems();
  if ( sl.count() > 0 )
    toolButtonAudioRemoveTracks->setEnabled(true);
  else
    toolButtonAudioRemoveTracks->setEnabled(false);

  audioFastForwarding = audioFastBackwarding = audioSkippingTracks = false;

  if ( sl.count() == 1 && !audioSkippingTracks && !qmc2EarlyStartup ) {
    QListWidgetItem *ci = listWidgetAudioPlaylist->currentItem();
    switch ( audioState ) {
      case Phonon::PlayingState:
        if ( qmc2AudioLastIndividualTrack != sl[0]->text() && ci == sl[0] )
          QTimer::singleShot(0, this, SLOT(on_actionAudioPlayTrack_triggered()));
        break;

      default:
	QTimer::singleShot(0, this, SLOT(on_actionAudioStopTrack_triggered()));
        break;
    }
    if ( ci ) {
      if ( ci->text() == sl[0]->text() )
        qmc2AudioLastIndividualTrack = sl[0]->text();
    }
  } else if ( sl.count() <= 0 )
    QTimer::singleShot(0, this, SLOT(audioScrollToCurrentItem()));
}

void MainWindow::audioScrollToCurrentItem()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::audioScrollToCurrentItem()");
#endif

  if ( !qmc2AudioLastIndividualTrack.isEmpty() ) {
    QList<QListWidgetItem *> itemList = listWidgetAudioPlaylist->findItems(qmc2AudioLastIndividualTrack, Qt::MatchExactly);
    if ( itemList.count() > 0 ) {
      QListWidgetItem *item = itemList[0];
      listWidgetAudioPlaylist->scrollToItem(item, QAbstractItemView::PositionAtCenter);
      listWidgetAudioPlaylist->setCurrentItem(item, QItemSelectionModel::Clear | QItemSelectionModel::SelectCurrent);
    }
  }
}

void MainWindow::on_actionAudioRaiseVolume_triggered(bool checked)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionAudioRaiseVolume_triggered(bool checked = ...)");
#endif

  dialAudioVolume->setValue(dialAudioVolume->value() + dialAudioVolume->pageStep());
}

void MainWindow::on_actionAudioLowerVolume_triggered(bool checked)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionAudioLowerVolume_triggered(bool checked = ...)");
#endif

  dialAudioVolume->setValue(dialAudioVolume->value() - dialAudioVolume->pageStep());
}

void MainWindow::on_dialAudioVolume_valueChanged(int value)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_dialAudioVolume_valueChanged(int value = %1)").arg(value));
#endif

  phononAudioOutput->setVolume((qreal)value/100.0);
}

void MainWindow::audioFinished()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::audioFinished()");
#endif

  static QStringList shuffleSelectionList;

  if ( audioFastBackwarding )
    QTimer::singleShot(0, this, SLOT(on_actionAudioPreviousTrack_triggered()));
  else if ( audioFastForwarding )
    QTimer::singleShot(0, this, SLOT(on_actionAudioNextTrack_triggered()));
  else if ( checkBoxAudioShuffle->isChecked() ) {
    if ( shuffleSelectionList.count() >= listWidgetAudioPlaylist->count() )
      shuffleSelectionList.clear();
    int newTrackIndex = qrand() % listWidgetAudioPlaylist->count();
    while ( shuffleSelectionList.contains(listWidgetAudioPlaylist->item(newTrackIndex)->text()) ) {
      qApp->processEvents();
      newTrackIndex = qrand() % listWidgetAudioPlaylist->count();
    }
    shuffleSelectionList << listWidgetAudioPlaylist->item(newTrackIndex)->text();
    listWidgetAudioPlaylist->setCurrentRow(newTrackIndex);
    QTimer::singleShot(0, this, SLOT(on_actionAudioPlayTrack_triggered()));
  } else {
    QTimer::singleShot(0, this, SLOT(on_actionAudioNextTrack_triggered()));
  }
}

void MainWindow::audioTick(qint64 newTime)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::audioTick(qint64 newTime = %1)").arg(newTime));
#endif

  progressBarAudioProgress->setFormat(tr("%vs (%ms total)"));
  progressBarAudioProgress->setValue(newTime/1000);
}

void MainWindow::audioTotalTimeChanged(qint64 newTotalTime)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::audioTotalTimeChanged(qint64 newTotalTime = %1)").arg(newTotalTime));
#endif

  progressBarAudioProgress->setFormat(tr("%vs (%ms total)"));
  progressBarAudioProgress->setRange(0, newTotalTime/1000);
  progressBarAudioProgress->setValue(phononAudioPlayer->currentTime()/1000);
}

void MainWindow::audioFade(int faderFunction)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::audioFade(int faderFunction = %1)").arg(faderFunction));
#endif

  int currentVolume = dialAudioVolume->value();
  int updateCounter;
  double vol;
  double volStep = (double)currentVolume / (double)QMC2_AUDIOPLAYER_FADER_TIMEOUT;
  audioFastForwarding = audioFastBackwarding = audioSkippingTracks = false;
  switch ( faderFunction ) {
    case QMC2_AUDIOPLAYER_FADER_PAUSE:
      actionAudioPauseTrack->setChecked(true);
      actionAudioStopTrack->setChecked(false);
      actionAudioPlayTrack->setChecked(false);
      actionAudioPauseTrack->setEnabled(false);
      toolButtonAudioPauseTrack->setEnabled(false);
      actionAudioPlayTrack->setEnabled(false);
      toolButtonAudioPlayTrack->setEnabled(false);
      actionAudioStopTrack->setEnabled(false);
      toolButtonAudioStopTrack->setEnabled(false);
      updateCounter = 0;
      for (vol = currentVolume; vol > 0.0; vol -= volStep) {
        updateCounter++;
        dialAudioVolume->setValue((int)vol);
        if ( updateCounter % 10 == 0 )
          qApp->processEvents();
        QTest::qSleep(1);
      }
      phononAudioPlayer->pause();
      audioState = Phonon::PausedState;
      qApp->processEvents();
      actionAudioPauseTrack->setEnabled(true);
      toolButtonAudioPauseTrack->setEnabled(true);
      actionAudioPlayTrack->setEnabled(true);
      toolButtonAudioPlayTrack->setEnabled(true);
      actionAudioStopTrack->setEnabled(true);
      toolButtonAudioStopTrack->setEnabled(true);
      break;

    case QMC2_AUDIOPLAYER_FADER_PLAY:
      dialAudioVolume->setValue(0);
      actionAudioPauseTrack->setChecked(false);
      actionAudioStopTrack->setChecked(false);
      actionAudioPlayTrack->setChecked(true);
      actionAudioPauseTrack->setEnabled(false);
      toolButtonAudioPauseTrack->setEnabled(false);
      actionAudioPlayTrack->setEnabled(false);
      toolButtonAudioPlayTrack->setEnabled(false);
      actionAudioStopTrack->setEnabled(false);
      toolButtonAudioStopTrack->setEnabled(false);
      qApp->processEvents();
      phononAudioPlayer->play();
      audioState = Phonon::PlayingState;
      updateCounter = 0;
      for (vol = 0; vol <= currentVolume; vol += volStep) {
        updateCounter++;
        dialAudioVolume->setValue((int)vol);
        if ( updateCounter % 10 == 0 )
          qApp->processEvents();
        QTest::qSleep(1);
      }
      qApp->processEvents();
      actionAudioPauseTrack->setEnabled(true);
      toolButtonAudioPauseTrack->setEnabled(true);
      actionAudioPlayTrack->setEnabled(true);
      toolButtonAudioPlayTrack->setEnabled(true);
      actionAudioStopTrack->setEnabled(true);
      toolButtonAudioStopTrack->setEnabled(true);
      break;
  }
  dialAudioVolume->setValue(currentVolume);
}

void MainWindow::audioMetaDataChanged()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::audioMetaDataChanged()");
#endif

  QString titleMetaData = phononAudioPlayer->metaData(Phonon::TitleMetaData).join(", ");
  QString artistMetaData = phononAudioPlayer->metaData(Phonon::ArtistMetaData).join(", ");
  QString albumMetaData = phononAudioPlayer->metaData(Phonon::AlbumMetaData).join(", ");
  QString genreMetaData = phononAudioPlayer->metaData(Phonon::GenreMetaData).join(", ");
  
  static QString lastTrackInfo = "";
  QString trackInfo = tr("audio player: track info: title = '%1', artist = '%2', album = '%3', genre = '%4'").arg(titleMetaData).arg(artistMetaData).arg(albumMetaData).arg(genreMetaData);
  if ( trackInfo != lastTrackInfo ) {
    log(QMC2_LOG_FRONTEND, trackInfo);
    lastTrackInfo = trackInfo;
  }
}

void MainWindow::audioBufferStatus(int percentFilled)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::audioBufferStatus(int percentFilled = %1)").arg(percentFilled));
#endif

  progressBarAudioProgress->setRange(0, 100);
  progressBarAudioProgress->setFormat(tr("Buffering %p%"));
  progressBarAudioProgress->setValue(percentFilled);
  if ( percentFilled >= 100 ) {
    if ( audioState == Phonon::StoppedState )
      progressBarAudioProgress->setRange(0, 100);
    else
      progressBarAudioProgress->setRange(0, 0);
    progressBarAudioProgress->reset();
  }
}
#else
void MainWindow::on_actionAudioPreviousTrack_triggered(bool) { ; }
void MainWindow::on_toolButtonAudioPreviousTrack_resetButton() { ; }
void MainWindow::on_actionAudioNextTrack_triggered(bool) { ; }
void MainWindow::on_toolButtonAudioNextTrack_resetButton() { ; }
void MainWindow::on_actionAudioFastBackward_triggered(bool) { ; }
void MainWindow::on_toolButtonAudioFastBackward_clicked(bool) { ; }
void MainWindow::on_toolButtonAudioFastBackward_resetButton() { ; }
void MainWindow::on_actionAudioFastForward_triggered(bool) { ; }
void MainWindow::on_toolButtonAudioFastForward_clicked(bool) { ; }
void MainWindow::on_toolButtonAudioFastForward_resetButton() { ; }
void MainWindow::on_actionAudioStopTrack_triggered(bool) { ; }
void MainWindow::on_actionAudioPauseTrack_triggered(bool) { ; }
void MainWindow::on_actionAudioPlayTrack_triggered(bool) { ; }
void MainWindow::on_toolButtonAudioAddTracks_clicked() { ; }
void MainWindow::on_toolButtonAudioAddURL_clicked() { ; }
void MainWindow::on_toolButtonAudioRemoveTracks_clicked() { ; }
void MainWindow::on_toolButtonAudioSetupEffects_clicked() { ; }
void MainWindow::on_listWidgetAudioPlaylist_itemSelectionChanged() { ; }
void MainWindow::on_dialAudioVolume_valueChanged(int) { ; }
void MainWindow::on_actionAudioRaiseVolume_triggered(bool) { ; }
void MainWindow::on_actionAudioLowerVolume_triggered(bool) { ; }
void MainWindow::audioFinished() { ; }
void MainWindow::audioTick(qint64) { ; }
void MainWindow::audioTotalTimeChanged(qint64) { ; }
void MainWindow::audioFade(int) { ; }
void MainWindow::audioMetaDataChanged() { ; }
void MainWindow::audioBufferStatus(int) { ; }
void MainWindow::audioScrollToCurrentItem() { ; }
#endif

void MainWindow::on_checkBoxRemoveFinishedDownloads_stateChanged(int state)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_checkBoxRemoveFinishedDownloads_stateChanged(int state = ...)");
#endif

  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Downloads/RemoveFinished", checkBoxRemoveFinishedDownloads->isChecked());
}

void MainWindow::createFifo(bool logFifoCreation)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::createFifo(bool logFifoCreation = ...)");
#endif

#if defined(Q_WS_WIN)
  // FIXME: implement Windows specific notifier FIFO support
#else
#if defined(QMC2_SDLMAME) || defined(QMC2_SDLMESS)
#if defined(QMC2_SDLMAME)
  if ( !EXISTS(QMC2_SDLMAME_OUTPUT_FIFO) )
    mkfifo(QMC2_SDLMAME_OUTPUT_FIFO, S_IRUSR | S_IWUSR | S_IRGRP);
  if ( !EXISTS(QMC2_SDLMAME_OUTPUT_FIFO) ) {
    log(QMC2_LOG_FRONTEND, tr("WARNING: can't create SDLMAME output notifier FIFO, path = %1").arg(QMC2_SDLMAME_OUTPUT_FIFO));
  } else {
    qmc2FifoFile = new QFile(QMC2_SDLMAME_OUTPUT_FIFO);
#if defined(O_ASYNC)
    int fd = ::open(QMC2_SDLMAME_OUTPUT_FIFO, O_ASYNC | O_NONBLOCK);
#else
    int fd = ::open(QMC2_SDLMAME_OUTPUT_FIFO, O_NONBLOCK);
#endif
#elif defined(QMC2_SDLMESS)
  if ( !EXISTS(QMC2_SDLMESS_OUTPUT_FIFO) )
    mkfifo(QMC2_SDLMESS_OUTPUT_FIFO, S_IRUSR | S_IWUSR | S_IRGRP);
  if ( !EXISTS(QMC2_SDLMESS_OUTPUT_FIFO) ) {
    log(QMC2_LOG_FRONTEND, tr("WARNING: can't create SDLMESS output notifier FIFO, path = %1").arg(QMC2_SDLMESS_OUTPUT_FIFO));
  } else {
    qmc2FifoFile = new QFile(QMC2_SDLMESS_OUTPUT_FIFO);
#if defined(O_ASYNC)
    int fd = ::open(QMC2_SDLMESS_OUTPUT_FIFO, O_ASYNC | O_NONBLOCK);
#else
    int fd = ::open(QMC2_SDLMESS_OUTPUT_FIFO, O_NONBLOCK);
#endif
#endif
    if ( fd >= 0 ) {
      if ( qmc2FifoFile->open(fd, QIODevice::ReadOnly | QIODevice::Text) ) {
        qmc2FifoNotifier = new QSocketNotifier(qmc2FifoFile->handle(), QSocketNotifier::Read);
        connect(qmc2FifoNotifier, SIGNAL(activated(int)), this, SLOT(processFifoData()));
        qmc2FifoNotifier->setEnabled(true);
        if ( logFifoCreation )
#if defined(QMC2_SDLMAME)
          log(QMC2_LOG_FRONTEND, tr("SDLMAME output notifier FIFO created"));
      } else {
        delete qmc2FifoFile;
        qmc2FifoFile = NULL;
        log(QMC2_LOG_FRONTEND, tr("WARNING: can't open SDLMAME output notifier FIFO for reading, path = %1").arg(QMC2_SDLMAME_OUTPUT_FIFO));
      }
    } else {
      log(QMC2_LOG_FRONTEND, tr("WARNING: can't open SDLMAME output notifier FIFO for reading, path = %1").arg(QMC2_SDLMAME_OUTPUT_FIFO));
    }
  }
#elif defined(QMC2_SDLMESS)
          log(QMC2_LOG_FRONTEND, tr("SDLMESS output notifier FIFO created"));
      } else {
        delete qmc2FifoFile;
        qmc2FifoFile = NULL;
        log(QMC2_LOG_FRONTEND, tr("WARNING: can't open SDLMESS output notifier FIFO for reading, path = %1").arg(QMC2_SDLMESS_OUTPUT_FIFO));
      }
    } else {
      log(QMC2_LOG_FRONTEND, tr("WARNING: can't open SDLMESS output notifier FIFO for reading, path = %1").arg(QMC2_SDLMESS_OUTPUT_FIFO));
    }
  }
#endif
#endif
#endif
  if ( qmc2FifoFile && qmc2FifoNotifier )
    qmc2FifoIsOpen = qmc2FifoFile->isOpen();
  else
    qmc2FifoIsOpen = false;
}

void MainWindow::recreateFifo()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::recreateFifo()");
#endif

#if defined(Q_WS_WIN)
  // FIXME: implement Windows specific notifier FIFO support
#else
#if defined(QMC2_SDLMAME) || defined(QMC2_SDLMESS)
  disconnect(qmc2FifoNotifier);
  delete qmc2FifoNotifier;
  qmc2FifoNotifier = NULL;
  if ( qmc2FifoFile->isOpen() )
    qmc2FifoFile->close();
  delete qmc2FifoFile;
#if defined(QMC2_SDLMAME)
//  ::unlink(QMC2_SDLMAME_OUTPUT_FIFO);
#elif defined(QMC2_SDLMESS)
//  ::unlink(QMC2_SDLMESS_OUTPUT_FIFO);
#endif
  qmc2FifoFile = NULL;
  createFifo(false);
#endif
#endif
}

void MainWindow::processFifoData()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::processFifoData()");
#endif

#if defined(QMC2_SDLMAME) || defined(QMC2_SDLMESS)
  QTextStream ts(qmc2FifoFile);
  QString data = ts.readAll();
  int i;

  if ( data.isEmpty() ) {
    if ( qmc2ProcessManager->procMap.count() <= 0 ) {
      // last emulator exited... recreate & reconnect FIFO to circumvent endless loops due to NULL data
      recreateFifo();
    }
    return;
  }

  QStringList sl = data.split("\n");

  for (i = 0; i < sl.count(); i++) { 
    if ( !sl[i].isEmpty() ) {
      QString msgClass, msgPid, msgWhat, msgState;
      QStringList words = sl[i].trimmed().split(" ");
      if ( words.count() > 0 )
        msgClass = words[0];
      if ( words.count() > 1 )
        msgPid = words[1];
      if ( words.count() > 2 )
        msgWhat = words[2];
      if ( words.count() > 3 )
        msgState = words[3];
      if ( !msgPid.isEmpty() ) {
        QList<QTreeWidgetItem *> il = treeWidgetEmulators->findItems(msgPid, Qt::MatchExactly, QMC2_EMUCONTROL_COLUMN_PID);
        if ( il.count() > 0 ) {
          if ( msgClass == "MAME" ) {
            if ( msgWhat == "START" ) {
              il[0]->setText(QMC2_EMUCONTROL_COLUMN_STATUS, tr("running"));
#if defined(Q_WS_X11)
	      Embedder *embedder = NULL;
	      int embedderIndex = -1;
	      for (int j = 0; j < tabWidgetEmbeddedEmulators->count() && embedder == NULL; j++) {
		      if ( tabWidgetEmbeddedEmulators->tabText(j).startsWith(QString("#%1 - ").arg(il[0]->text(QMC2_EMUCONTROL_COLUMN_NUMBER))) ) {
			      embedder = (Embedder *)tabWidgetEmbeddedEmulators->widget(j);
			      embedderIndex = j;
		      }
	      }
	      if ( embedder ) {
		      embedder->isPaused = false;
		      tabWidgetEmbeddedEmulators->setTabIcon(embedderIndex, embedder->iconRunning);
	      }
#endif
            } else if ( msgWhat == "STOP" ) {
              il[0]->setText(QMC2_EMUCONTROL_COLUMN_STATUS, tr("stopped"));
#if defined(Q_WS_X11)
	      Embedder *embedder = NULL;
	      int embedderIndex = -1;
	      for (int j = 0; j < tabWidgetEmbeddedEmulators->count() && embedder == NULL; j++) {
		      if ( tabWidgetEmbeddedEmulators->tabText(j).startsWith(QString("#%1 - ").arg(il[0]->text(QMC2_EMUCONTROL_COLUMN_NUMBER))) ) {
			      embedder = (Embedder *)tabWidgetEmbeddedEmulators->widget(j);
			      embedderIndex = j;
		      }
	      }
	      if ( embedder ) {
		      embedder->isPaused = false;
		      tabWidgetEmbeddedEmulators->setTabIcon(embedderIndex, embedder->iconStopped);
	      }
#endif
            } else {
#if defined(QMC2_SDLMAME)
              log(QMC2_LOG_FRONTEND, tr("unhandled MAME output notification: game = %1, class = %2, what = %3, state = %4").arg(il[0]->text(QMC2_EMUCONTROL_COLUMN_GAME)).arg(msgClass).arg(msgWhat).arg(msgState));
#elif defined(QMC2_SDLMESS)
              log(QMC2_LOG_FRONTEND, tr("unhandled MESS output notification: game = %1, class = %2, what = %3, state = %4").arg(il[0]->text(QMC2_EMUCONTROL_COLUMN_GAME)).arg(msgClass).arg(msgWhat).arg(msgState));
#endif
            }
          } else if ( msgClass == "OUT" ) {
            // refresh static output notifiers
            if ( msgWhat == "led0" ) {
              if ( msgState == "1" )
                il[0]->setIcon(QMC2_EMUCONTROL_COLUMN_LED0, QIcon(QString::fromUtf8(":/data/img/led_on.png")));
              else
                il[0]->setIcon(QMC2_EMUCONTROL_COLUMN_LED0, QIcon(QString::fromUtf8(":/data/img/led_off.png")));
            } else if ( msgWhat == "led1" ) {
              if ( msgState == "1" )
                il[0]->setIcon(QMC2_EMUCONTROL_COLUMN_LED1, QIcon(QString::fromUtf8(":/data/img/led_on.png")));
              else
                il[0]->setIcon(QMC2_EMUCONTROL_COLUMN_LED1, QIcon(QString::fromUtf8(":/data/img/led_off.png")));
            } else if ( msgWhat == "pause" ) {
#if defined(Q_WS_X11)
	      Embedder *embedder = NULL;
	      int embedderIndex = -1;
	      for (int j = 0; j < tabWidgetEmbeddedEmulators->count() && embedder == NULL; j++) {
		      if ( tabWidgetEmbeddedEmulators->tabText(j).startsWith(QString("#%1 - ").arg(il[0]->text(QMC2_EMUCONTROL_COLUMN_NUMBER))) ) {
			      embedder = (Embedder *)tabWidgetEmbeddedEmulators->widget(j);
			      embedderIndex = j;
		      }
	      }
              if ( msgState == "1" ) {
		      il[0]->setText(QMC2_EMUCONTROL_COLUMN_STATUS, tr("paused"));
		      if ( embedder ) {
			      embedder->isPaused = true;
			      tabWidgetEmbeddedEmulators->setTabIcon(embedderIndex, embedder->iconPaused);
		      }
	      } else {
		      il[0]->setText(QMC2_EMUCONTROL_COLUMN_STATUS, tr("running"));
		      if ( embedder ) {
			      embedder->isPaused = false;
			      tabWidgetEmbeddedEmulators->setTabIcon(embedderIndex, embedder->iconRunning);
		      }
	      }
#else
              if ( msgState == "1" )
                il[0]->setText(QMC2_EMUCONTROL_COLUMN_STATUS, tr("paused"));
              else
                il[0]->setText(QMC2_EMUCONTROL_COLUMN_STATUS, tr("running"));
#endif
            } else {
               // add or refresh dynamic output notifiers
               QTreeWidgetItem *itemFound = NULL;
               int i;
               for (i = 0; i < il[0]->childCount() && itemFound == NULL; i++) {
                 QTreeWidgetItem *item = il[0]->child(i);
                 if ( item->text(QMC2_EMUCONTROL_COLUMN_GAME) == msgWhat )
                   itemFound = item;
               }
               if ( itemFound != NULL ) {
                 itemFound->setText(QMC2_EMUCONTROL_COLUMN_STATUS, msgState);
               } else {
                 itemFound = new QTreeWidgetItem(il[0]);
                 itemFound->setText(QMC2_EMUCONTROL_COLUMN_GAME, msgWhat);
                 itemFound->setText(QMC2_EMUCONTROL_COLUMN_STATUS, msgState);
                 if ( il[0]->childCount() == 1 ) {
                   // this is a workaround for a minor Qt bug: the root decoration
                   // isn't updated correctly on the first child item insertion
                   treeWidgetEmulators->setRootIsDecorated(false);
                   treeWidgetEmulators->setRootIsDecorated(true);
                 }
               }
            }
          } else {
#if defined(QMC2_SDLMAME)
            log(QMC2_LOG_FRONTEND, tr("unhandled MAME output notification: game = %1, class = %2, what = %3, state = %4").arg(il[0]->text(QMC2_EMUCONTROL_COLUMN_GAME)).arg(msgClass).arg(msgWhat).arg(msgState));
#elif defined(QMC2_SDLMESS)
            log(QMC2_LOG_FRONTEND, tr("unhandled MESS output notification: game = %1, class = %2, what = %3, state = %4").arg(il[0]->text(QMC2_EMUCONTROL_COLUMN_GAME)).arg(msgClass).arg(msgWhat).arg(msgState));
#endif
          }
        }
        treeWidgetEmulators->update();
      }
    }
  }
#endif
}

void MainWindow::on_romStateFilterC_toggled(bool on)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_romStateFilterC_toggled(bool on = %1)").arg(on));
#endif

  if ( qmc2StatesTogglesEnabled ) {
    qmc2Options->toolButtonShowC->setChecked(on);
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Gamelist/ShowC", on);
    qmc2Filter.setBit(QMC2_ROMSTATE_INT_C, on);
    qmc2Gamelist->filter();
  }
}

void MainWindow::on_romStateFilterM_toggled(bool on)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_romStateFilterM_toggled(bool on = %1)").arg(on));
#endif

  if ( qmc2StatesTogglesEnabled ) {
    qmc2Options->toolButtonShowM->setChecked(on);
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Gamelist/ShowM", on);
    qmc2Filter.setBit(QMC2_ROMSTATE_INT_M, on);
    qmc2Gamelist->filter();
  }
}

void MainWindow::on_romStateFilterI_toggled(bool on)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_romStateFilterI_toggled(bool on = %1)").arg(on));
#endif

  if ( qmc2StatesTogglesEnabled ) {
    qmc2Options->toolButtonShowI->setChecked(on);
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Gamelist/ShowI", on);
    qmc2Filter.setBit(QMC2_ROMSTATE_INT_I, on);
    qmc2Gamelist->filter();
  }
}

void MainWindow::on_romStateFilterN_toggled(bool on)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_romStateFilterN_toggled(bool on = %1)").arg(on));
#endif

  if ( qmc2StatesTogglesEnabled ) {
    qmc2Options->toolButtonShowN->setChecked(on);
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Gamelist/ShowN", on);
    qmc2Filter.setBit(QMC2_ROMSTATE_INT_N, on);
    qmc2Gamelist->filter();
  }
}

void MainWindow::on_romStateFilterU_toggled(bool on)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_romStateFilterU_toggled(bool on = %1)").arg(on));
#endif

  if ( qmc2StatesTogglesEnabled ) {
    qmc2Options->toolButtonShowU->setChecked(on);
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Gamelist/ShowU", on);
    qmc2Filter.setBit(QMC2_ROMSTATE_INT_U, on);
    qmc2Gamelist->filter();
  }
}

void MainWindow::on_treeWidgetGamelist_headerSectionClicked(int logicalIndex)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_treeWidgetGamelist_headerSectionClicked(int logicalIndex = %1)").arg(logicalIndex));
#endif

  qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicatorShown(false);
  qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicatorShown(false);
#if defined(QMC2_EMUTYPE_MAME)
  qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicatorShown(false);
  qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicatorShown(false);
#endif

  switch ( logicalIndex ) {
    case QMC2_GAMELIST_COLUMN_GAME:
      if ( qmc2Options->comboBoxSortCriteria->currentIndex() == QMC2_SORTCRITERIA_DESCRIPTION )
        qmc2Options->comboBoxSortOrder->setCurrentIndex(qmc2Options->comboBoxSortOrder->currentIndex() == 0 ? 1 : 0);
      else
        qmc2Options->comboBoxSortCriteria->setCurrentIndex(QMC2_SORTCRITERIA_DESCRIPTION);
      break;

    case QMC2_GAMELIST_COLUMN_TAG:
      if ( qmc2Options->comboBoxSortCriteria->currentIndex() == QMC2_SORTCRITERIA_TAG )
        qmc2Options->comboBoxSortOrder->setCurrentIndex(qmc2Options->comboBoxSortOrder->currentIndex() == 0 ? 1 : 0);
      else
        qmc2Options->comboBoxSortCriteria->setCurrentIndex(QMC2_SORTCRITERIA_TAG);
      break;

    case QMC2_GAMELIST_COLUMN_YEAR:
      if ( qmc2Options->comboBoxSortCriteria->currentIndex() == QMC2_SORTCRITERIA_YEAR )
        qmc2Options->comboBoxSortOrder->setCurrentIndex(qmc2Options->comboBoxSortOrder->currentIndex() == 0 ? 1 : 0);
      else
        qmc2Options->comboBoxSortCriteria->setCurrentIndex(QMC2_SORTCRITERIA_YEAR);
      break;

    case QMC2_GAMELIST_COLUMN_MANU:
      if ( qmc2Options->comboBoxSortCriteria->currentIndex() == QMC2_SORTCRITERIA_MANUFACTURER )
        qmc2Options->comboBoxSortOrder->setCurrentIndex(qmc2Options->comboBoxSortOrder->currentIndex() == 0 ? 1 : 0);
      else
        qmc2Options->comboBoxSortCriteria->setCurrentIndex(QMC2_SORTCRITERIA_MANUFACTURER);
      break;

    case QMC2_GAMELIST_COLUMN_NAME:
      if ( qmc2Options->comboBoxSortCriteria->currentIndex() == QMC2_SORTCRITERIA_GAMENAME )
        qmc2Options->comboBoxSortOrder->setCurrentIndex(qmc2Options->comboBoxSortOrder->currentIndex() == 0 ? 1 : 0);
      else
        qmc2Options->comboBoxSortCriteria->setCurrentIndex(QMC2_SORTCRITERIA_GAMENAME);
      break;

    case QMC2_GAMELIST_COLUMN_RTYPES:
      if ( qmc2Options->comboBoxSortCriteria->currentIndex() == QMC2_SORTCRITERIA_ROMTYPES )
        qmc2Options->comboBoxSortOrder->setCurrentIndex(qmc2Options->comboBoxSortOrder->currentIndex() == 0 ? 1 : 0);
      else
        qmc2Options->comboBoxSortCriteria->setCurrentIndex(QMC2_SORTCRITERIA_ROMTYPES);
      break;

    case QMC2_GAMELIST_COLUMN_PLAYERS:
      if ( qmc2Options->comboBoxSortCriteria->currentIndex() == QMC2_SORTCRITERIA_PLAYERS )
        qmc2Options->comboBoxSortOrder->setCurrentIndex(qmc2Options->comboBoxSortOrder->currentIndex() == 0 ? 1 : 0);
      else
        qmc2Options->comboBoxSortCriteria->setCurrentIndex(QMC2_SORTCRITERIA_PLAYERS);
      break;

    case QMC2_GAMELIST_COLUMN_DRVSTAT:
      if ( qmc2Options->comboBoxSortCriteria->currentIndex() == QMC2_SORTCRITERIA_DRVSTAT )
        qmc2Options->comboBoxSortOrder->setCurrentIndex(qmc2Options->comboBoxSortOrder->currentIndex() == 0 ? 1 : 0);
      else
        qmc2Options->comboBoxSortCriteria->setCurrentIndex(QMC2_SORTCRITERIA_DRVSTAT);
      break;

#if defined(QMC2_EMUTYPE_MAME)
    case QMC2_GAMELIST_COLUMN_CATEGORY:
      if ( qmc2Options->comboBoxSortCriteria->currentIndex() == QMC2_SORTCRITERIA_CATEGORY )
        qmc2Options->comboBoxSortOrder->setCurrentIndex(qmc2Options->comboBoxSortOrder->currentIndex() == 0 ? 1 : 0);
      else
        qmc2Options->comboBoxSortCriteria->setCurrentIndex(QMC2_SORTCRITERIA_CATEGORY);
      break;

    case QMC2_GAMELIST_COLUMN_VERSION:
      if ( qmc2Options->comboBoxSortCriteria->currentIndex() == QMC2_SORTCRITERIA_VERSION )
        qmc2Options->comboBoxSortOrder->setCurrentIndex(qmc2Options->comboBoxSortOrder->currentIndex() == 0 ? 1 : 0);
      else
        qmc2Options->comboBoxSortCriteria->setCurrentIndex(QMC2_SORTCRITERIA_VERSION);
      break;
#endif

    default:
      break;
  }

  QTimer::singleShot(0, qmc2Options, SLOT(on_pushButtonApply_clicked()));
}

void MainWindow::on_menuTabWidgetGamelist_North_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_menuTabWidgetGamelist_North_activated()"));
#endif

  tabWidgetGamelist->setTabPosition(QTabWidget::North);
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/Gamelist/TabPosition", QTabWidget::North);
}

void MainWindow::on_menuTabWidgetGamelist_South_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_menuTabWidgetGamelist_South_activated()"));
#endif

  tabWidgetGamelist->setTabPosition(QTabWidget::South);
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/Gamelist/TabPosition", QTabWidget::South);
}

void MainWindow::on_menuTabWidgetGamelist_West_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_menuTabWidgetGamelist_West_activated()"));
#endif

  tabWidgetGamelist->setTabPosition(QTabWidget::West);
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/Gamelist/TabPosition", QTabWidget::West);
}

void MainWindow::on_menuTabWidgetGamelist_East_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_menuTabWidgetGamelist_East_activated()"));
#endif

  tabWidgetGamelist->setTabPosition(QTabWidget::East);
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/Gamelist/TabPosition", QTabWidget::East);
}

void MainWindow::on_tabWidgetGamelist_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_tabWidgetGamelist_customContextMenuRequested(const QPoint &p = ...)");
#endif

  if ( !tabWidgetGamelist->currentWidget()->childrenRect().contains(p, true) ) {
    menuTabWidgetGamelist->move(adjustedWidgetPosition(tabWidgetGamelist->mapToGlobal(p), menuTabWidgetGamelist));
    menuTabWidgetGamelist->show();
  }
}

void MainWindow::on_menuTabWidgetGameDetail_North_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_menuTabWidgetGameDetail_North_activated()"));
#endif

  tabWidgetGameDetail->setTabPosition(QTabWidget::North);
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/GameDetail/TabPosition", QTabWidget::North);
}

void MainWindow::on_menuTabWidgetGameDetail_South_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_menuTabWidgetGameDetail_South_activated()"));
#endif

  tabWidgetGameDetail->setTabPosition(QTabWidget::South);
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/GameDetail/TabPosition", QTabWidget::South);
}

void MainWindow::on_menuTabWidgetGameDetail_West_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_menuTabWidgetGameDetail_West_activated()"));
#endif

  tabWidgetGameDetail->setTabPosition(QTabWidget::West);
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/GameDetail/TabPosition", QTabWidget::West);
}

void MainWindow::on_menuTabWidgetGameDetail_East_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_menuTabWidgetGameDetail_East_activated()"));
#endif

  tabWidgetGameDetail->setTabPosition(QTabWidget::East);
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/GameDetail/TabPosition", QTabWidget::East);
}

void MainWindow::on_menuTabWidgetGameDetail_Setup_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_menuTabWidgetGameDetail_Setup_activated()"));
#endif

  if ( !qmc2DetailSetup )
    return;
 
  qmc2DetailSetup->adjustIconSizes();

  // reparent detail setup dialog to the widget it was called from
  qmc2DetailSetup->setParent(qmc2DetailSetupParent ? qmc2DetailSetupParent : this);
  qmc2DetailSetup->setWindowFlags(Qt::Dialog);
  qmc2DetailSetupParent = NULL;

  if ( qmc2DetailSetup->isHidden() )
    qmc2DetailSetup->show();
  else if ( qmc2DetailSetup->isMinimized() )
    qmc2DetailSetup->showNormal();

  QTimer::singleShot(0, qmc2DetailSetup, SLOT(raise()));
}

void MainWindow::on_tabWidgetGameDetail_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_tabWidgetGameDetail_customContextMenuRequested(const QPoint &p = ...)");
#endif

  if ( tabWidgetGameDetail->currentWidget() ) {
    if ( !tabWidgetGameDetail->currentWidget()->childrenRect().contains(p, true) ) {
      menuTabWidgetGameDetail->move(adjustedWidgetPosition(tabWidgetGameDetail->mapToGlobal(p), menuTabWidgetGameDetail));
      menuTabWidgetGameDetail->show();
    }
  } else {
    menuTabWidgetGameDetail->move(adjustedWidgetPosition(tabWidgetGameDetail->mapToGlobal(p), menuTabWidgetGameDetail));
    menuTabWidgetGameDetail->show();
  }
}

void MainWindow::on_menuTabWidgetLogsAndEmulators_North_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_menuTabWidgetLogsAndEmulators_North_activated()"));
#endif

  tabWidgetLogsAndEmulators->setTabPosition(QTabWidget::North);
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/LogsAndEmulators/TabPosition", QTabWidget::North);
}

void MainWindow::on_menuTabWidgetLogsAndEmulators_South_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_menuTabWidgetLogsAndEmulators_South_activated()"));
#endif

  tabWidgetLogsAndEmulators->setTabPosition(QTabWidget::South);
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/LogsAndEmulators/TabPosition", QTabWidget::South);
}

void MainWindow::on_menuTabWidgetLogsAndEmulators_West_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_menuTabWidgetLogsAndEmulators_West_activated()"));
#endif

  tabWidgetLogsAndEmulators->setTabPosition(QTabWidget::West);
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/LogsAndEmulators/TabPosition", QTabWidget::West);
}

void MainWindow::on_menuTabWidgetLogsAndEmulators_East_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_menuTabWidgetLogsAndEmulators_East_activated()"));
#endif

  tabWidgetLogsAndEmulators->setTabPosition(QTabWidget::East);
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/LogsAndEmulators/TabPosition", QTabWidget::East);
}

void MainWindow::on_tabWidgetLogsAndEmulators_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_tabWidgetLogsAndEmulators_customContextMenuRequested(const QPoint &p = ...)");
#endif

  if ( !tabWidgetLogsAndEmulators->currentWidget()->childrenRect().contains(p, true) ) {
    menuTabWidgetLogsAndEmulators->move(adjustedWidgetPosition(tabWidgetLogsAndEmulators->mapToGlobal(p), menuTabWidgetLogsAndEmulators));
    menuTabWidgetLogsAndEmulators->show();
  }
}

void MainWindow::on_menuTabWidgetSoftwareDetail_North_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_menuTabWidgetSoftwareDetail_North_activated()"));
#endif

  tabWidgetSoftwareDetail->setTabPosition(QTabWidget::North);
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareDetail/TabPosition", QTabWidget::North);
}

void MainWindow::on_menuTabWidgetSoftwareDetail_South_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_menuTabWidgetSoftwareDetail_South_activated()"));
#endif

  tabWidgetSoftwareDetail->setTabPosition(QTabWidget::South);
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareDetail/TabPosition", QTabWidget::South);
}

void MainWindow::on_menuTabWidgetSoftwareDetail_West_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_menuTabWidgetSoftwareDetail_West_activated()"));
#endif

  tabWidgetSoftwareDetail->setTabPosition(QTabWidget::West);
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareDetail/TabPosition", QTabWidget::West);
}

void MainWindow::on_menuTabWidgetSoftwareDetail_East_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_menuTabWidgetSoftwareDetail_East_activated()"));
#endif

  tabWidgetSoftwareDetail->setTabPosition(QTabWidget::East);
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareDetail/TabPosition", QTabWidget::East);
}

void MainWindow::on_tabWidgetSoftwareDetail_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_tabWidgetSoftwareDetail_customContextMenuRequested(const QPoint &p = ...)");
#endif

  if ( !tabWidgetSoftwareDetail->currentWidget()->childrenRect().contains(p, true) ) {
    menuTabWidgetSoftwareDetail->move(adjustedWidgetPosition(tabWidgetSoftwareDetail->mapToGlobal(p), menuTabWidgetSoftwareDetail));
    menuTabWidgetSoftwareDetail->show();
  }
}

void MainWindow::on_hSplitter_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_hSplitter_customContextMenuRequested(const QPoint &p = ...)");
#endif

  menuHorizontalSplitter->move(adjustedWidgetPosition(hSplitter->mapToGlobal(p), menuHorizontalSplitter));
  menuHorizontalSplitter->show();
}

void MainWindow::on_vSplitter_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_vSplitter_customContextMenuRequested(const QPoint &p = ...)");
#endif

  menuVerticalSplitter->move(adjustedWidgetPosition(vSplitter->mapToGlobal(p), menuVerticalSplitter));
  menuVerticalSplitter->show();
}

void MainWindow::on_menuHorizontalSplitter_FlipOrientation_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_menuHorizontalSplitter_FlipOrientation_activated()");
#endif

  if ( hSplitter->orientation() == Qt::Horizontal )
    hSplitter->setOrientation(Qt::Vertical);
  else
    hSplitter->setOrientation(Qt::Horizontal);
}

void MainWindow::on_menuHorizontalSplitter_SwapLayouts_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_menuHorizontalSplitter_SwapLayouts_activated()");
#endif

  hSplitter->insertWidget(0, hSplitter->widget(1));
}

void MainWindow::on_menuVerticalSplitter_FlipOrientation_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_menuVerticalSplitter_FlipOrientation_activated()");
#endif

  if ( vSplitter->orientation() == Qt::Horizontal )
    vSplitter->setOrientation(Qt::Vertical);
  else
    vSplitter->setOrientation(Qt::Horizontal);
}

void MainWindow::on_menuVerticalSplitter_SwapWidgets_activated()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_menuVerticalSplitter_SwapWidgets_activated()");
#endif

  vSplitter->insertWidget(0, vSplitter->widget(1));
}

void MainWindow::on_treeWidgetHierarchy_headerSectionClicked(int logicalIndex)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_treeWidgetHierarchy_headerSectionClicked(int logicalIndex = %1)").arg(logicalIndex));
#endif

  on_treeWidgetGamelist_headerSectionClicked(logicalIndex);
}

#if defined(QMC2_EMUTYPE_MAME)
void MainWindow::on_treeWidgetCategoryView_headerSectionClicked(int logicalIndex)
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_treeWidgetCategoryView_headerSectionClicked(int logicalIndex = %1)").arg(logicalIndex));
#endif

	on_treeWidgetGamelist_headerSectionClicked(logicalIndex);
}

void MainWindow::on_treeWidgetCategoryView_itemActivated(QTreeWidgetItem *item, int column)
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_treeWidgetCategoryView_itemActivated(QTreeWidgetItem *item = %1, int column = %2)").arg((qulonglong)item).arg(column));
#endif

	if ( qmc2DemoModeDialog )
		if ( qmc2DemoModeDialog->demoModeRunning )
			return;
	if ( !item )
		return;
	if ( item->text(QMC2_GAMELIST_COLUMN_NAME).isEmpty() )
		return;
	qmc2StartEmbedded = false;
	if ( !qmc2IgnoreItemActivation ) {
		switch ( qmc2DefaultLaunchMode ) {
#if defined(Q_WS_X11) || defined(Q_WS_WIN)
			case QMC2_LAUNCH_MODE_EMBEDDED:
				on_actionPlayEmbedded_activated();
				break;
#endif
			case QMC2_LAUNCH_MODE_INDEPENDENT:
			default:
				on_actionPlay_activated();
				break;
		}
	}
	qmc2IgnoreItemActivation = false;
}

void MainWindow::on_treeWidgetCategoryView_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_treeWidgetCategoryView_itemDoubleClicked(QTreeWidgetItem *item = %1, int column = %2)").arg((qulonglong)item).arg(column));
#endif

	if ( !item )
		return;
	if ( item->text(QMC2_GAMELIST_COLUMN_NAME).isEmpty() )
		return;
	if ( qmc2DemoModeDialog )
		if ( qmc2DemoModeDialog->demoModeRunning )
			qmc2IgnoreItemActivation = true;
	if ( !qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/DoubleClickActivation").toBool() )
		qmc2IgnoreItemActivation = true;
}

void MainWindow::on_treeWidgetCategoryView_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_treeWidgetCategoryView_currentItemChanged(QTreeWidgetItem *current = %1, QTreeWidgetItem *previous = %2)").arg((qulonglong)current).arg((qulonglong)previous));
#endif

	if ( !current )
		return;
	if ( current->text(QMC2_GAMELIST_COLUMN_NAME).isEmpty() )
		return;
	qmc2CheckItemVisibility = false;
	if ( qmc2UpdateDelay > 0 )
		updateTimer.start(qmc2UpdateDelay);
	else
		on_treeWidgetGamelist_itemSelectionChanged_delayed();
}

void MainWindow::on_treeWidgetCategoryView_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_treeWidgetCategoryView_itemSelectionChanged()"));
#endif

	qmc2CategoryViewSelectedItem = NULL;
	QList<QTreeWidgetItem *>selected = treeWidgetCategoryView->selectedItems();
	if ( selected.count() > 0 ) {
		QTreeWidgetItem *item = selected.at(0);
		QString gameDescription = item->text(QMC2_GAMELIST_COLUMN_GAME);
		if ( gameDescription == tr("Waiting for data...") || item->text(QMC2_GAMELIST_COLUMN_NAME).isEmpty() )
			return;
		qmc2CategoryViewSelectedItem = qmc2GamelistItemByDescriptionMap[gameDescription];
		qmc2CheckItemVisibility = false;
		treeWidgetGamelist->setCurrentItem(qmc2CategoryViewSelectedItem);
	}
}

void MainWindow::on_treeWidgetCategoryView_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_treeWidgetCategoryView_customContextMenuRequested(const QPoint &p = ...)");
#endif

	QTreeWidgetItem *item = treeWidgetCategoryView->itemAt(p);
	if ( !item )
		return;
	if ( item->text(QMC2_GAMELIST_COLUMN_GAME) == tr("Waiting for data...") || item->text(QMC2_GAMELIST_COLUMN_NAME).isEmpty() )
		return;
	treeWidgetCategoryView->setItemSelected(item, true);
	qmc2GameMenu->move(adjustedWidgetPosition(treeWidgetCategoryView->viewport()->mapToGlobal(p), qmc2GameMenu));
	qmc2GameMenu->show();
}

void MainWindow::on_treeWidgetVersionView_headerSectionClicked(int logicalIndex)
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_treeWidgetVersionView_headerSectionClicked(int logicalIndex = %1)").arg(logicalIndex));
#endif

	on_treeWidgetGamelist_headerSectionClicked(logicalIndex);
}

void MainWindow::on_treeWidgetVersionView_itemActivated(QTreeWidgetItem *item, int column)
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_treeWidgetVersionView_itemActivated(QTreeWidgetItem *item = %1, int column = %2)").arg((qulonglong)item).arg(column));
#endif

	if ( qmc2DemoModeDialog )
		if ( qmc2DemoModeDialog->demoModeRunning )
			return;
	if ( !item )
		return;
	if ( item->text(QMC2_GAMELIST_COLUMN_NAME).isEmpty() )
		return;
	qmc2StartEmbedded = false;
	if ( !qmc2IgnoreItemActivation ) {
		switch ( qmc2DefaultLaunchMode ) {
#if defined(Q_WS_X11) || defined(Q_WS_WIN)
			case QMC2_LAUNCH_MODE_EMBEDDED:
				on_actionPlayEmbedded_activated();
				break;
#endif
			case QMC2_LAUNCH_MODE_INDEPENDENT:
			default:
				on_actionPlay_activated();
				break;
		}
	}
	qmc2IgnoreItemActivation = false;
}

void MainWindow::on_treeWidgetVersionView_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_treeWidgetVersionView_itemDoubleClicked(QTreeWidgetItem *item = %1, int column = %2)").arg((qulonglong)item).arg(column));
#endif

	if ( !item )
		return;
	if ( item->text(QMC2_GAMELIST_COLUMN_NAME).isEmpty() )
		return;
	if ( !qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/DoubleClickActivation").toBool() )
		qmc2IgnoreItemActivation = true;
}

void MainWindow::on_treeWidgetVersionView_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_treeWidgetVersionView_currentItemChanged(QTreeWidgetItem *current = %1, QTreeWidgetItem *previous = %2)").arg((qulonglong)current).arg((qulonglong)previous));
#endif

	if ( !current )
		return;
	if ( current->text(QMC2_GAMELIST_COLUMN_NAME).isEmpty() )
		return;
	qmc2CheckItemVisibility = false;
	if ( qmc2UpdateDelay > 0 )
		updateTimer.start(qmc2UpdateDelay);
	else
		on_treeWidgetGamelist_itemSelectionChanged_delayed();
}

void MainWindow::on_treeWidgetVersionView_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_treeWidgetVersionView_itemSelectionChanged()"));
#endif

	qmc2VersionViewSelectedItem = NULL;
	QList<QTreeWidgetItem *>selected = treeWidgetVersionView->selectedItems();
	if ( selected.count() > 0 ) {
		QTreeWidgetItem *item = selected.at(0);
		QString gameDescription = item->text(QMC2_GAMELIST_COLUMN_GAME);
		if ( gameDescription == tr("Waiting for data...") || item->text(QMC2_GAMELIST_COLUMN_NAME).isEmpty() )
			return;
		qmc2VersionViewSelectedItem = qmc2GamelistItemByDescriptionMap[gameDescription];
		qmc2CheckItemVisibility = false;
		treeWidgetGamelist->setCurrentItem(qmc2VersionViewSelectedItem);
	}
}

void MainWindow::on_treeWidgetVersionView_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_treeWidgetVersionView_customContextMenuRequested(const QPoint &p = ...)");
#endif

	QTreeWidgetItem *item = treeWidgetVersionView->itemAt(p);
	if ( !item )
		return;
	if ( item->text(QMC2_GAMELIST_COLUMN_GAME) == tr("Waiting for data...") || item->text(QMC2_GAMELIST_COLUMN_NAME).isEmpty() )
		return;
	treeWidgetVersionView->setItemSelected(item, true);
	qmc2GameMenu->move(adjustedWidgetPosition(treeWidgetVersionView->viewport()->mapToGlobal(p), qmc2GameMenu));
	qmc2GameMenu->show();
}
#endif

void MainWindow::on_actionArcadeShowFPS_toggled(bool on)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_actionArcadeShowFPS_toggled(bool on = %1)").arg(on));
#endif

  if ( qmc2ArcadeView )
    qmc2ArcadeView->menuScene->toggleFps();
}

void MainWindow::on_actionArcadeTakeScreenshot_triggered()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_actionArcadeTakeScreenshot_triggered()"));
#endif

  if ( qmc2ArcadeView ) {
    if ( !exitArcade )
      qmc2ArcadeView->takeScreenshot();
    else
      log(QMC2_LOG_FRONTEND, tr("ArcadeView is not currently active, can't take screen shot"));
  }
}

void MainWindow::on_comboBoxViewSelect_currentIndexChanged(int index)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::on_comboBoxViewSelect_currentIndexChanged(int index = %1)").arg(index));
#endif

  switch ( index ) {
	  case QMC2_VIEWGAMELIST_INDEX:
		  pushButtonSelectRomFilter->setVisible(true);
		  viewFullDetail();
		  break;
	  case QMC2_VIEWHIERARCHY_INDEX:
		  pushButtonSelectRomFilter->setVisible(false);
		  menuRomStatusFilter->setVisible(false);
		  viewParentClones();
		  break;
#if defined(QMC2_EMUTYPE_MAME)
	  case QMC2_VIEWVERSION_INDEX:
		  pushButtonSelectRomFilter->setVisible(false);
		  menuRomStatusFilter->setVisible(false);
		  viewByVersion();
		  break;
	  case QMC2_VIEWCATEGORY_INDEX:
		  pushButtonSelectRomFilter->setVisible(false);
		  menuRomStatusFilter->setVisible(false);
		  viewByCategory();
		  break;
#endif
	  default:
		  break;
  }
}

#if defined(QMC2_EMUTYPE_MESS)
void MainWindow::projectMessLoadStarted()
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::projectMessLoadStarted()");
#endif

}

void MainWindow::projectMessLoadFinished(bool ok)
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::projectMessLoadFinished(bool ok = %1)").arg(ok));
#endif

	if ( qmc2SoftwareList->currentItem && qmc2ProjectMESS && ok ) {
		// store compressed page to in-memory cache
		QString cacheKey = qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_LIST) + "_" + qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_NAME);
		if ( qmc2ProjectMESSCache.contains(cacheKey) ) qmc2ProjectMESSCache.remove(cacheKey);
		QByteArray data = QMC2_COMPRESS(qmc2ProjectMESS->webViewBrowser->page()->mainFrame()->toHtml().toLatin1());
		qmc2ProjectMESSCache.insert(cacheKey, new QByteArray(data), data.size());
	}

	// we only want to know this ONCE
	disconnect(qmc2ProjectMESS->webViewBrowser, SIGNAL(loadFinished(bool)), this, SLOT(projectMessLoadFinished(bool)));
}
#endif

#if defined(QMC2_EMUTYPE_MAME)
void MainWindow::mawsLoadStarted()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::mawsLoadStarted()");
#endif

  if ( menuMAWSQuickLinks )
    delete menuMAWSQuickLinks;
  if ( toolButtonMAWSQuickLinks )
    delete toolButtonMAWSQuickLinks;
  menuMAWSQuickLinks = NULL;
  toolButtonMAWSQuickLinks = NULL;
}

void MainWindow::mawsLoadFinished(bool ok)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::mawsLoadFinished(bool ok = %1)").arg(ok));
#endif

  if ( qmc2CurrentItem && qmc2MAWSLookup && ok ) {
    QString gameName = qmc2CurrentItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON);
    // only cache the ROM set page, don't cache followed pages
    if ( qmc2MAWSLookup->webViewBrowser->url().toString() == qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/BaseURL", QMC2_MAWS_BASE_URL).toString().arg(gameName) ) {

      // the following "mods" will only be done on the original MAWS pages
      if ( qmc2MAWSLookup->webViewBrowser->url().toString().startsWith(QMC2_MAWS_BASE_URL) ) {
        // modify the HTML to contain a "clean" MAWS copyright in the footer
        QString mawsHtml = qmc2MAWSLookup->webViewBrowser->page()->mainFrame()->toHtml();
        mawsHtml.replace("href=\"#top\"", QString("href=\"%1#top\"").arg(qmc2MAWSLookup->webViewBrowser->url().path()));
        int startIndex = mawsHtml.indexOf("<div class=\"ifFoot\"");
        mawsHtml.remove(startIndex, mawsHtml.indexOf("</div>") + 6);
        mawsHtml.insert(startIndex, QString("<p><font size=\"-1\">Copyright &copy; 2004 - %1 <a href=\"%2\"><b>MAWS</b></a>, All Rights Reserved</font></p>").arg(QDate::currentDate().year()).arg(QMC2_MAWS_HOMEPAGE_URL));

        // make sure to scroll to last last scroll position after exchanging the HTML
        QPoint scrollPos = qmc2MAWSLookup->webViewBrowser->page()->mainFrame()->scrollPosition();
        qmc2MAWSLookup->webViewBrowser->setUpdatesEnabled(false);
        qmc2MAWSLookup->webViewBrowser->setHtml(mawsHtml, QUrl(qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/BaseURL", QMC2_MAWS_BASE_URL).toString().arg(gameName)));
        qmc2MAWSLookup->webViewBrowser->stop();
        qmc2MAWSLookup->webViewBrowser->page()->mainFrame()->setScrollPosition(scrollPos);
        qmc2MAWSLookup->webViewBrowser->setUpdatesEnabled(true);
      }

      // store compressed page to in-memory cache
      QByteArray mawsData = QMC2_COMPRESS(qmc2MAWSLookup->webViewBrowser->page()->mainFrame()->toHtml().toLatin1());
      if ( qmc2MAWSCache.contains(gameName) ) {
        qmc2MAWSCache.remove(gameName);
#ifdef QMC2_DEBUG
        log(QMC2_LOG_FRONTEND, QString("DEBUG: MAWS cache: URL exists, updating cache entry for '%1'").arg(qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/BaseURL", QMC2_MAWS_BASE_URL).toString().arg(gameName)));
#endif
      } else {
#ifdef QMC2_DEBUG
        log(QMC2_LOG_FRONTEND, QString("DEBUG: MAWS cache: URL not found, creating cache entry for '%1'").arg(qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/BaseURL", QMC2_MAWS_BASE_URL).toString().arg(gameName)));
#endif
      }
      qmc2MAWSCache.insert(gameName, new QByteArray(mawsData), mawsData.size());
#ifdef QMC2_DEBUG
      log(QMC2_LOG_FRONTEND, QString("DEBUG: MAWS cache: %1% filled").arg((double)100.0 * ((double)qmc2MAWSCache.totalCost()/(double)qmc2MAWSCache.maxCost()), 0, 'f', 2));
#endif

      // save compressed page to MAWS disk cache
      QDir mawsCacheDir(qmc2Config->value("MAME/FilesAndDirectories/MAWSCacheDirectory").toString());
      if ( mawsCacheDir.exists() ) {
        QFile mawsCacheFile(mawsCacheDir.filePath(gameName + ".wc"));
        if ( mawsCacheFile.open(QIODevice::WriteOnly) ) {
          mawsCacheFile.write("# THIS FILE IS AUTO-GENERATED - PLEASE DO NOT EDIT!\n");
	  QString timeStamp = "TIMESTAMP\t" + QString::number(QDateTime::currentDateTime().toTime_t()) + "\n";
	  mawsCacheFile.write((const char  *)timeStamp.toAscii());
#if defined(QMC2_WC_COMPRESSION_ENABLED)
          mawsCacheFile.write(mawsData);
#else
          mawsCacheFile.write((const char *)qmc2MAWSLookup->webViewBrowser->page()->mainFrame()->toHtml().toLatin1());
#endif
          mawsCacheFile.close();
        }
      }
      if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/QuickDownload", true).toBool() )
        QTimer::singleShot(QMC2_MAWS_QDL_DELAY, this, SLOT(createMawsQuickLinksMenu()));
    }
#ifdef QMC2_DEBUG
    else
      log(QMC2_LOG_FRONTEND, QString("DEBUG: MAWS cache: ignoring URL '%1'").arg(qmc2MAWSLookup->webViewBrowser->url().toString()));
#endif
  }
}

void MainWindow::createMawsQuickLinksMenu()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::createMawsQuickLinksMenu()");
#endif

  if ( !qmc2MAWSLookup )
    return;

  mawsQDLActions.clear();

  QFont f;
  f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
  QFontMetrics fm(f);
  QSize iconSize(fm.height() - 2, fm.height() - 2);

  QString mawsHtml = qmc2MAWSLookup->webViewBrowser->page()->mainFrame()->toHtml().simplified();

  toolButtonMAWSQuickLinks = new AutoPopupToolButton(qmc2MAWSLookup->webViewBrowser);
  toolButtonMAWSQuickLinks->setIcon(QIcon(QString::fromUtf8(":/data/img/download.png")));
  toolButtonMAWSQuickLinks->setIconSize(iconSize);
  toolButtonMAWSQuickLinks->setPopupMode(QToolButton::InstantPopup);
  toolButtonMAWSQuickLinks->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
  toolButtonMAWSQuickLinks->setToolButtonStyle(Qt::ToolButtonIconOnly);
  toolButtonMAWSQuickLinks->setToolTip(tr("Quick download links for MAWS data usable by QMC2"));

  connect(qmc2MAWSLookup->webViewBrowser, SIGNAL(mouseOnView(bool)), this, SLOT(mawsQuickLinksSetVisible(bool)));
  connect(toolButtonMAWSQuickLinks, SIGNAL(menuHidden()), this, SLOT(mawsQuickLinksMenuHidden()));

  // we have to force a background widget update when the browser is scrolled to correctly redraw the MAWS QDL tool button
  connect(toolButtonMAWSQuickLinks, SIGNAL(paintFinished()), qmc2MAWSLookup->webViewBrowser, SLOT(delayedUpdate()));

  menuMAWSQuickLinks = new QMenu(toolButtonMAWSQuickLinks);

  QAction *action;

  // icon quick link:
  action = menuMAWSQuickLinks->addAction(tr("Icon"), this, SLOT(storeMawsIcon()));
  action->setIcon(qmc2MAWSLookup->comboBoxURL->itemIcon(qmc2MAWSLookup->comboBoxURL->currentIndex()));
  mawsQDLActions[tr("Icon")] = action;
  menuMAWSQuickLinks->addSeparator();

  // cabinet art quick links:
  QMenu *cabinetArtMenu = menuMAWSQuickLinks->addMenu(QIcon(QString::fromUtf8(":/data/img/arcadecabinet.png")), tr("Cabinet art"));
  QMap<QString, QString> cabinetArtURLs;
  int startIndex = mawsHtml.indexOf("cabinet art");
  int endIndex = mawsHtml.indexOf("</table>", startIndex);
  QString cabinetArtHtml = mawsHtml.mid(startIndex, endIndex - startIndex).remove(QRegExp("</tr>|<table>|</table>|<tbody>|</tbody>")); 
  cabinetArtHtml.replace("<tr>", ":");
  cabinetArtHtml.remove(0, cabinetArtHtml.indexOf(":") + 1);
  cabinetArtHtml = cabinetArtHtml.simplified().trimmed();
  QStringList cabinetArtLines = cabinetArtHtml.split(":");
  int i;
  for (i = 0; i < cabinetArtLines.count(); i++) {
    QString cabArtLine = cabinetArtLines[i];
    if ( cabArtLine.contains(">art</td>") ) {
      continue;
    } else {
      startIndex = cabArtLine.indexOf("href=\"") + 6;
      QString url = QMC2_MAWS_IMGLINKS_BASE_URL + cabArtLine.mid(startIndex, cabArtLine.indexOf("\"", startIndex + 1) - startIndex);
      if ( cabArtLine.contains(">cabinet</td>") ) {
        cabinetArtURLs[tr("Cabinet")] = url;
      } else if ( cabArtLine.contains(">control panel</td>") ) {
        cabinetArtURLs[tr("Controller")] = url;
      } else if ( cabArtLine.contains(">PCB</td>") ) {
        cabinetArtURLs[tr("PCB")] = url;
      } else if ( cabArtLine.contains(">flyer</td>") ) {
        cabinetArtURLs[tr("Flyer")] = url;
      } else if ( cabArtLine.contains(">marquee</td>") ) {
        cabinetArtURLs[tr("Marquee")] = url;
      }
    }
  }
  if ( cabinetArtURLs.isEmpty() ) {
    cabinetArtMenu->setTitle(tr("No cabinet art"));
    cabinetArtMenu->setEnabled(false);
  } else {
    QMapIterator<QString, QString> it(cabinetArtURLs);
    while ( it.hasNext() ) {
      it.next();
      action = cabinetArtMenu->addAction(it.key() + " - " + it.value(), this, SLOT(downloadMawsQuickLink()));
      mawsQDLActions[it.key()] = action;
    }
  }

  // preview quick links:
  QMenu *previewMenu = menuMAWSQuickLinks->addMenu(QIcon(QString::fromUtf8(":/data/img/camera.png")), tr("Previews"));
  QMap<QString, QString> previewURLs;
  // AntoPISA progettoSNAPS
  startIndex = mawsHtml.indexOf("document.snapshot.src='../img/ps/snap/");
  if ( startIndex >= 0 ) {
    endIndex = mawsHtml.indexOf("';", startIndex);
    if ( endIndex > startIndex )
      previewURLs[tr("AntoPISA progettoSNAPS") + " (" + tr("preview") + ")"] = QMC2_MAWS_IMGLINKS_BASE_URL + mawsHtml.mid(startIndex, endIndex - startIndex).split("'")[1];
  }
  // MAME World Snap Collection
  startIndex = mawsHtml.indexOf("document.snapshot.src='img/shots/mwsnap/");
  if ( startIndex >= 0 ) {
    endIndex = mawsHtml.indexOf("';", startIndex);
    if ( endIndex > startIndex )
      previewURLs[tr("MAME World Snap Collection") + " (" + tr("preview") + ")"] = QMC2_MAWS_IMGLINKS_BASE_URL + mawsHtml.mid(startIndex, endIndex - startIndex).split("'")[1];
  }
  // CrashTest Snap Collection
  startIndex = mawsHtml.indexOf("document.snapshot.src='img/shots/snap/");
  if ( startIndex >= 0 ) {
    endIndex = mawsHtml.indexOf("';", startIndex);
    if ( endIndex > startIndex )
      previewURLs[tr("CrashTest Snap Collection") + " (" + tr("preview") + ")"] = QMC2_MAWS_IMGLINKS_BASE_URL + mawsHtml.mid(startIndex, endIndex - startIndex).split("'")[1];
  }
  // Enaitz Jar Snaps
  startIndex = mawsHtml.indexOf("document.snapshot.src='img/shots/ej/");
  if ( startIndex >= 0 ) {
    endIndex = mawsHtml.indexOf("';", startIndex);
    if ( endIndex > startIndex )
      previewURLs[tr("Enaitz Jar Snaps") + " (" + tr("preview") + ")"] = QMC2_MAWS_IMGLINKS_BASE_URL + mawsHtml.mid(startIndex, endIndex - startIndex).split("'")[1];
  }
  if ( previewURLs.isEmpty() ) {
    previewMenu->setTitle(tr("No previews"));
    previewMenu->setEnabled(false);
  } else {
    QMapIterator<QString, QString> it(previewURLs);
    QAction *fallbackAction = NULL;
    while ( it.hasNext() ) {
      bool firstItem = !it.hasPrevious();
      it.next();
      action = previewMenu->addAction(it.key() + " - " + it.value(), this, SLOT(downloadMawsQuickLink()));
      mawsQDLActions[it.key()] = action;
      if ( firstItem )
        fallbackAction = action;
    }
    // use the first menu entry as a fallback action if the preferred image collection isn't available
    if ( fallbackAction ) {
      QString key = qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/PreferredPreviewCollection").toString() + " (" + tr("preview") + ")";
      if ( !previewURLs.contains(key) )
        mawsQDLActions[key] = fallbackAction;
    }
  }

  // title quick links:
  QMenu *titleMenu = menuMAWSQuickLinks->addMenu(QIcon(QString::fromUtf8(":/data/img/arcademode.png")), tr("Titles"));
  QMap<QString, QString> titleURLs;
  // AntoPISA progettoSNAPS
  startIndex = mawsHtml.indexOf("document.snapshot.src='../img/ps/titles/");
  if ( startIndex >= 0 ) {
    endIndex = mawsHtml.indexOf("';", startIndex);
    if ( endIndex > startIndex )
      titleURLs[tr("AntoPISA progettoSNAPS") + " (" + tr("title") + ")"] = QMC2_MAWS_IMGLINKS_BASE_URL + mawsHtml.mid(startIndex, endIndex - startIndex).split("'")[1];
  }
  // CrashTest Snap Collection
  startIndex = mawsHtml.indexOf("document.snapshot.src='img/shots/titles/");
  if ( startIndex >= 0 ) {
    endIndex = mawsHtml.indexOf("';", startIndex);
    if ( endIndex > startIndex ) {
      titleURLs[tr("CrashTest Snap Collection") + " (" + tr("title") + ")"] = QMC2_MAWS_IMGLINKS_BASE_URL + mawsHtml.mid(startIndex, endIndex - startIndex).split("'")[1];
    }
  }
  if ( titleURLs.isEmpty() ) {
    titleMenu->setTitle(tr("No titles"));
    titleMenu->setEnabled(false);
  } else {
    QMapIterator<QString, QString> it(titleURLs);
    QAction *fallbackAction = NULL;
    while ( it.hasNext() ) {
      bool firstItem = !it.hasPrevious();
      it.next();
      action = titleMenu->addAction(it.key() + " - " + it.value(), this, SLOT(downloadMawsQuickLink()));
      mawsQDLActions[it.key()] = action;
      if ( firstItem )
        fallbackAction = action;
    }
    // use the first menu entry as a fallback action if the preferred image collection isn't available
    if ( fallbackAction ) {
      QString key = qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/PreferredTitleCollection").toString() + " (" + tr("title") + ")";
      if ( !titleURLs.contains(key) )
        mawsQDLActions[key] = fallbackAction;
    }
  }
  menuMAWSQuickLinks->addSeparator();

  // quick links setup:
  action = menuMAWSQuickLinks->addAction(tr("Setup..."), this, SLOT(setupMawsQuickLinks()));
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/work.png")));
  toolButtonMAWSQuickLinks->setMenu(menuMAWSQuickLinks);

  mawsQuickLinksSetVisible(qmc2MAWSLookup->webViewBrowser->mouseCurrentlyOnView);

  QTimer::singleShot(QMC2_MAWS_QDL_DELAY, this, SLOT(startMawsAutoDownloads()));
}

void MainWindow::setupMawsQuickLinks()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::setupMawsQuickLinks()");
#endif

  if ( !qmc2MawsQuickDownloadSetup )
    qmc2MawsQuickDownloadSetup = new MawsQuickDownloadSetup(this);

  qmc2MawsQuickDownloadSetup->show();
  qmc2MawsQuickDownloadSetup->raise();
}

void MainWindow::mawsQuickLinksSetVisible(bool visible)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::mawsQuickLinksSetVisible(bool visible = %1)").arg(visible));
#endif

  if ( !qmc2MAWSLookup )
    return;

  if ( toolButtonMAWSQuickLinks && menuMAWSQuickLinks ) {
    if ( visible )
      toolButtonMAWSQuickLinks->setVisible(true);
    else if ( !menuMAWSQuickLinks->isVisible() )
      toolButtonMAWSQuickLinks->setVisible(false);
  }
}

void MainWindow::mawsQuickLinksMenuHidden()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::mawsQuickLinksMenuHidden()");
#endif

  if ( !qmc2MAWSLookup )
    return;

  if ( toolButtonMAWSQuickLinks && menuMAWSQuickLinks )
    toolButtonMAWSQuickLinks->setVisible(qmc2MAWSLookup->webViewBrowser->mouseCurrentlyOnView);
}

void MainWindow::startMawsAutoDownloads()
{
  if ( !qmc2CurrentItem || !menuMAWSQuickLinks )
    return;

#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::startMawsAutoDownloads()");
#endif

  QAction *action;

  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/AutoDownloadIcons", false).toBool() ) {
    action = mawsQDLActions[tr("Icon")];
    if ( action ) QTimer::singleShot(0, action, SLOT(trigger()));
  }
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/AutoDownloadCabinets", false).toBool() ) {
    action = mawsQDLActions[tr("Cabinet")];
    if ( action ) QTimer::singleShot(0, action, SLOT(trigger()));
  }
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/AutoDownloadControllers", false).toBool() ) {
    action = mawsQDLActions[tr("Controller")];
    if ( action ) QTimer::singleShot(0, action, SLOT(trigger()));
  }
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/AutoDownloadFlyers", false).toBool() ) {
    action = mawsQDLActions[tr("Flyer")];
    if ( action ) QTimer::singleShot(0, action, SLOT(trigger()));
  }
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/AutoDownloadMarquees", false).toBool() ) {
    action = mawsQDLActions[tr("Marquee")];
    if ( action ) QTimer::singleShot(0, action, SLOT(trigger()));
  }
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/AutoDownloadPCBs", false).toBool() ) {
    action = mawsQDLActions[tr("PCB")];
    if ( action ) QTimer::singleShot(0, action, SLOT(trigger()));
  }
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/AutoDownloadPreviews", false).toBool() ) {
    action = mawsQDLActions[qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/PreferredPreviewCollection").toString() + " (" + tr("preview") + ")"];
    if ( action ) QTimer::singleShot(0, action, SLOT(trigger()));
  }
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/AutoDownloadTitles", false).toBool() ) {
    action = mawsQDLActions[qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/PreferredTitleCollection").toString() + " (" + tr("title") + ")"];
    if ( action ) QTimer::singleShot(0, action, SLOT(trigger()));
  }
}

void MainWindow::downloadMawsQuickLink()
{
  QAction *action = (QAction *)sender();

  if ( !action || !qmc2CurrentItem )
    return;

#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::downloadMawsQuickLink()");
#endif

  QString saveName = qmc2CurrentItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON) + ".png";
  QString savePath;

  QStringList actionWords = action->text().split(" - ");
  if ( actionWords.count() > 1 ) {
    menuMAWSQuickLinks->setVisible(false);
    QString imageType = actionWords[0];
    if ( imageType == tr("Cabinet") )
      savePath = qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/CabinetDirectory").toString();
    else if ( imageType == tr("Controller") )
      savePath = qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/ControllerDirectory").toString();
    else if ( imageType == tr("Flyer") )
      savePath = qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/FlyerDirectory").toString();
    else if ( imageType == tr("Marquee") )
      savePath = qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/MarqueeDirectory").toString();
    else if ( imageType == tr("PCB") )
      savePath = qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/PCBDirectory").toString();
    else if ( imageType.endsWith(" (" + tr("preview") + ")") )
      savePath = qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/PreviewDirectory").toString();
    else if ( imageType.endsWith(" (" + tr("title") + ")") )
      savePath = qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/TitleDirectory").toString();
    else
      return;
    QNetworkRequest request(actionWords[1]);
    startDownload(qmc2NetworkAccessManager->get(request), saveName, savePath);
  }
}

void MainWindow::storeMawsIcon()
{
  QAction *action = (QAction *)sender();

  if ( !action || !qmc2CurrentItem )
    return;

#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::storeMawsIcon()");
#endif

  menuMAWSQuickLinks->setVisible(false);

  QString gameName = qmc2CurrentItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON);
  QString saveName = gameName + ".png";
  
  QString filePath;

  if ( !qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/IconDirectory").toString().isEmpty() ) {
    filePath = qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/IconDirectory").toString();
    if ( !filePath.endsWith("/") ) filePath.append("/");
    filePath += saveName;
  } else {
    if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "WebBrowser/LastStoragePath") )
      saveName.prepend(qmc2Config->value(QMC2_FRONTEND_PREFIX + "WebBrowser/LastStoragePath").toString());
    filePath = QFileDialog::getSaveFileName(this, tr("Choose file to store the icon"), saveName, tr("All files (*)"));
  }

  if ( !filePath.isEmpty() ) {
    QPixmap iconPixmap = action->icon().pixmap(QSize(64, 64));
    if ( iconPixmap.save(filePath) ) {
      log(QMC2_LOG_FRONTEND, tr("icon image for '%1' stored as '%2'").arg(gameName).arg(filePath));
      QFileInfo fiFilePath(filePath);
      QString storagePath = fiFilePath.absolutePath();
      if ( !storagePath.endsWith("/") )
        storagePath.append("/");
      qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "WebBrowser/LastStoragePath", storagePath);
    } else
      log(QMC2_LOG_FRONTEND, tr("FATAL: icon image for '%1' couldn't be stored as '%2'").arg(gameName).arg(filePath));
  }
}

#elif defined(QMC2_EMUTYPE_MESS)
void MainWindow::messWikiLoadStarted()
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::messWikiLoadStarted()");
#endif

}

void MainWindow::messWikiLoadFinished(bool ok)
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::messWikiLoadFinished(bool ok = %1)").arg(ok));
#endif

	if ( ok ) {
		QByteArray messWikiData = QMC2_COMPRESS(qmc2MAWSLookup->webViewBrowser->page()->mainFrame()->toHtml().toLatin1());
    		QString driverName = qmc2Gamelist->lookupDriverName(qmc2CurrentItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON));
		if ( driverName.isEmpty() )
			driverName = "QMC2_MESS_WIKI_ALL_DRIVERS";
		if ( qmc2MAWSCache.contains(driverName) )
			qmc2MAWSCache.remove(driverName);
		qmc2MAWSCache.insert(driverName, new QByteArray(messWikiData), messWikiData.size());
	}
}
#endif

void MainWindow::startDownload(QNetworkReply *reply, QString saveAsName, QString savePath)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::startDownload(QNetworkReply *reply = %1, QString saveAsName = %2, QString savePath = %3)").arg((qulonglong)reply).arg(saveAsName).arg(savePath));
#endif

  if ( !reply )
    return;

  QFileInfo fi(reply->url().path());
  QString proposedName = fi.baseName();
  if ( !saveAsName.isEmpty() )
    proposedName = saveAsName;
  else if ( !fi.completeSuffix().isEmpty() )
    proposedName += "." + fi.completeSuffix();

  QString filePath;

  if ( !saveAsName.isEmpty() && !savePath.isEmpty() ) {
    if ( !savePath.endsWith("/") ) savePath.append("/");
    filePath = savePath + saveAsName;
  } else {
    if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "WebBrowser/LastStoragePath") )
      proposedName.prepend(qmc2Config->value(QMC2_FRONTEND_PREFIX + "WebBrowser/LastStoragePath").toString());
    filePath = QFileDialog::getSaveFileName(this, tr("Choose file to store download"), proposedName, tr("All files (*)"));
  }

  if ( !filePath.isEmpty() ) {
    DownloadItem *downloadItem = new DownloadItem(reply, filePath, treeWidgetDownloads);
    treeWidgetDownloads->scrollToItem(downloadItem);
    QFileInfo fiFilePath(filePath);
    QString storagePath = fiFilePath.absolutePath();
    if ( !storagePath.endsWith("/") )
      storagePath.append("/");
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "WebBrowser/LastStoragePath", storagePath);
  } else
    reply->close();
}

void MainWindow::on_pushButtonClearFinishedDownloads_clicked()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_pushButtonClearFinishedDownloads_clicked()");
#endif

  static bool downloadCleanupActive = false;

  if ( downloadCleanupActive )
    return;

  downloadCleanupActive = true;

  QList<int> indexList;

  QTreeWidgetItemIterator it(treeWidgetDownloads);
  while ( *it ) {
    if ( (*it)->whatsThis(0) == "finished" || (*it)->whatsThis(0) == "aborted" )
      indexList << treeWidgetDownloads->indexOfTopLevelItem(*it);
    it++;
  }

  int i;
  for (i = indexList.count(); i >= 0; i--) {
    DownloadItem *item = (DownloadItem *)treeWidgetDownloads->takeTopLevelItem(indexList[i]);
    if ( item )
      delete item;
    if ( i % 10 == 0 ) qApp->processEvents();
  }

  downloadCleanupActive = false;
}

void MainWindow::on_pushButtonReloadSelectedDownloads_clicked()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_pushButtonReloadSelectedDownloads_clicked()");
#endif

  QTreeWidgetItemIterator it(treeWidgetDownloads);
  while ( *it ) {
    if ( (*it)->isSelected() )
      if ( (*it)->whatsThis(0) != "downloading" && (*it)->whatsThis(0) != "initializing" )
        ((DownloadItem*)(*it))->itemDownloader->reload();
    it++;
  }
}

void MainWindow::on_pushButtonStopSelectedDownloads_clicked()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_pushButtonStopSelectedDownloads_clicked()");
#endif

  QTreeWidgetItemIterator it(treeWidgetDownloads);
  while ( *it ) {
    if ( (*it)->isSelected() ) {
      if ( (*it)->whatsThis(0) == "downloading" )
        ((DownloadItem*)(*it))->itemDownloader->networkReply->abort();
    }
    it++;
  }
}

void MainWindow::on_memoryUpdateTimer_timeout()
{
#if defined(QMC2_SHOWMEMINFO)
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_memoryUpdateTimer_timeout()");
#endif

  // get memory information
  quint64 numPages, pageSize, freePages, totalSize, totalUsed, totalFree;
  numPages = sysconf(_SC_PHYS_PAGES) / 1024;
  pageSize = sysconf(_SC_PAGESIZE) / 1024;
  freePages = sysconf( _SC_AVPHYS_PAGES) / 1024;
  totalSize = numPages * pageSize;
  totalFree = pageSize * freePages;
  totalUsed = totalSize - totalFree;
  progressBarMemory->setValue(100 * ((double)totalUsed/(double)totalSize));
  progressBarMemory->setToolTip("<b>" + tr("Physical memory:") + "</b><br>" + tr("Total: %1 MB").arg(totalSize) + "<br>" + tr("Free: %1 MB").arg(totalFree) + "<br>" + tr("Used: %1 MB").arg(totalUsed));
#endif
}

void MainWindow::checkActivity()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::checkActivity()");
#endif

  // resync timer (as far as possible)
  activityCheckTimer.start(QMC2_ACTIVITY_CHECK_INTERVAL);

#if defined(QMC2_EMUTYPE_MAME)
  if ( qmc2ReloadActive ||
       qmc2VerifyActive ||
       qmc2FilterActive ||
       qmc2ImageCheckActive ||
       qmc2SampleCheckActive ||
       qmc2ROMAlyzerActive ||
       qmc2LoadingGameInfoDB ||
       qmc2LoadingEmuInfoDB ||
       (qmc2SoftwareList && qmc2SoftwareList->isLoading) )
#else
  if ( qmc2ReloadActive ||
       qmc2VerifyActive ||
       qmc2FilterActive ||
       qmc2ImageCheckActive ||
       qmc2ROMAlyzerActive ||
       qmc2LoadingGameInfoDB ||
       (qmc2SoftwareList && qmc2SoftwareList->isLoading) )
#endif
  {
    activityState = !activityState;
    if ( activityState )
      actionExitStop->setIcon(QIcon(QString::fromUtf8(":/data/img/activity_green.png")));
    else
      actionExitStop->setIcon(QIcon(QString::fromUtf8(":/data/img/activity_red.png")));
    toolbar->update();
    isActiveState = true;
  } else {
    if ( isActiveState ) {
      actionExitStop->setIcon(QIcon(QString::fromUtf8(":/data/img/exit.png")));
      toolbar->update();
      activityState = false;
    }
    isActiveState = false;
  }
}

int MainWindow::sortCriteriaLogicalIndex() {
  switch ( qmc2SortCriteria ) {
    case QMC2_SORT_BY_DESCRIPTION:
    case QMC2_SORT_BY_ROM_STATE:
      return QMC2_GAMELIST_COLUMN_GAME;
      break;

    case QMC2_SORT_BY_TAG:
      return QMC2_GAMELIST_COLUMN_TAG;
      break;

    case QMC2_SORT_BY_YEAR:
      return QMC2_GAMELIST_COLUMN_YEAR;
      break;

    case QMC2_SORT_BY_MANUFACTURER:
      return QMC2_GAMELIST_COLUMN_MANU;
      break;

    case QMC2_SORT_BY_NAME:
      return QMC2_GAMELIST_COLUMN_NAME;
      break;

    case QMC2_SORT_BY_ROMTYPES:
      return QMC2_GAMELIST_COLUMN_RTYPES;
      break;

    case QMC2_SORT_BY_PLAYERS:
      return QMC2_GAMELIST_COLUMN_PLAYERS;
      break;

    case QMC2_SORT_BY_DRVSTAT:
      return QMC2_GAMELIST_COLUMN_DRVSTAT;
      break;

#if defined(QMC2_EMUTYPE_MAME)
    case QMC2_SORT_BY_CATEGORY:
      return QMC2_GAMELIST_COLUMN_CATEGORY;
      break;

    case QMC2_SORT_BY_VERSION:
      return QMC2_GAMELIST_COLUMN_VERSION;
      break;
#endif

    default:
      return QMC2_GAMELIST_COLUMN_GAME;
      break;
  }
}

QPoint MainWindow::adjustedWidgetPosition(QPoint p, QWidget *w)
{
  static QPoint adjustedPosition;
  static QList<QWidget *> adjustedWidgets;

  if ( !adjustedWidgets.contains(w) ) {
    w->move(p);
    w->show();
    adjustedWidgets.append(w);
  }

  adjustedPosition = p;
  if ( p.x() + w->width() > desktopGeometry.width() )
    adjustedPosition.setX(desktopGeometry.width() - w->width());
  if ( p.y() + w->height() > desktopGeometry.height() )
    adjustedPosition.setY(desktopGeometry.height() - w->height());

  return adjustedPosition;
}

void MainWindow::enableContextMenuPlayActions(bool enable)
{
	foreach(QAction *action, contextMenuPlayActions)
		action->setEnabled(enable);
}

void MainWindow::treeWidgetGamelistHeader_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::treeWidgetGamelistHeader_customContextMenuRequested(const QPoint &p = ...)");
#endif

	menuGamelistHeader->move(adjustedWidgetPosition(treeWidgetGamelist->header()->viewport()->mapToGlobal(p), menuGamelistHeader));
	menuGamelistHeader->show();
}

void MainWindow::actionGamelistHeader_triggered()
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::actionGamelistHeader_triggered()");
#endif

	QAction *action = (QAction *)sender();
	int visibleColumns = 0;
	for (int i = 0; i < treeWidgetGamelist->columnCount(); i++) if ( !treeWidgetGamelist->isColumnHidden(i) ) visibleColumns++;
	if ( action ) {
		bool visibility = true;
		if ( action->isChecked() )
			treeWidgetGamelist->setColumnHidden(action->data().toInt(), false);
		else if ( visibleColumns > 1 ) {
			treeWidgetGamelist->setColumnHidden(action->data().toInt(), true);
			visibility = false;
		}
		action->blockSignals(true);
		action->setChecked(visibility);
		action->blockSignals(false);
	}
}

void MainWindow::treeWidgetHierarchyHeader_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::treeWidgetHierarchyHeader_customContextMenuRequested(const QPoint &p = ...)");
#endif

	menuHierarchyHeader->move(adjustedWidgetPosition(treeWidgetHierarchy->header()->viewport()->mapToGlobal(p), menuHierarchyHeader));
	menuHierarchyHeader->show();
}

void MainWindow::actionHierarchyHeader_triggered()
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::actionHierarchyHeader_triggered()");
#endif

	QAction *action = (QAction *)sender();
	int visibleColumns = 0;
	for (int i = 0; i < treeWidgetHierarchy->columnCount(); i++) if ( !treeWidgetHierarchy->isColumnHidden(i) ) visibleColumns++;
	if ( action ) {
		bool visibility = true;
		if ( action->isChecked() )
			treeWidgetHierarchy->setColumnHidden(action->data().toInt(), false);
		else if ( visibleColumns > 1 ) {
			treeWidgetHierarchy->setColumnHidden(action->data().toInt(), true);
			visibility = false;
		}
		action->blockSignals(true);
		action->setChecked(visibility);
		action->blockSignals(false);
	}
}

void MainWindow::on_treeWidgetGamelist_itemEntered(QTreeWidgetItem *item, int column)
{
	if ( column == QMC2_GAMELIST_COLUMN_TAG && item->parent() == NULL ) {
		if ( qApp->mouseButtons() == Qt::LeftButton && qApp->activeWindow() ) {
			QPoint cPos = treeWidgetGamelist->viewport()->mapFromGlobal(QCursor::pos());
			if ( treeWidgetGamelist->itemAt(cPos) == item ) {
				Qt::CheckState cs = (item->checkState(column) == Qt::Unchecked ? Qt::Checked : Qt::Unchecked);
				bool wasTagged = (cs != Qt::Checked);
				item->setCheckState(column, cs);
				QString gameName = item->text(QMC2_GAMELIST_COLUMN_NAME);
				item = qmc2HierarchyItemMap[gameName];
				if ( item )
					item->setCheckState(column, cs);
#if defined(QMC2_EMUTYPE_MAME)
				item = qmc2CategoryItemMap[gameName];
				if ( item )
					item->setCheckState(column, cs);
				item = qmc2VersionItemMap[gameName];
				if ( item )
					item->setCheckState(column, cs);
#endif
				if ( wasTagged )
					qmc2Gamelist->numTaggedSets--;
				else
					qmc2Gamelist->numTaggedSets++;
				labelGamelistStatus->setText(qmc2Gamelist->status());
			}
		}
	}
}

void MainWindow::on_treeWidgetGamelist_itemPressed(QTreeWidgetItem *item, int column)
{
	on_treeWidgetGamelist_itemEntered(item, column);
}

void MainWindow::on_treeWidgetHierarchy_itemEntered(QTreeWidgetItem *item, int column)
{
	if ( column == QMC2_GAMELIST_COLUMN_TAG ) {
		if ( qApp->mouseButtons() == Qt::LeftButton ) {
			QPoint cPos = treeWidgetHierarchy->viewport()->mapFromGlobal(QCursor::pos());
			if ( treeWidgetHierarchy->itemAt(cPos) == item ) {
				Qt::CheckState cs = (item->checkState(column) == Qt::Unchecked ? Qt::Checked : Qt::Unchecked);
				bool wasTagged = (cs != Qt::Checked);
				item->setCheckState(column, cs);
				QString gameName = item->text(QMC2_GAMELIST_COLUMN_NAME);
				item = qmc2GamelistItemMap[gameName];
				if ( item )
					item->setCheckState(column, cs);
#if defined(QMC2_EMUTYPE_MAME)
				item = qmc2CategoryItemMap[gameName];
				if ( item )
					item->setCheckState(column, cs);
				item = qmc2VersionItemMap[gameName];
				if ( item )
					item->setCheckState(column, cs);
#endif
				if ( wasTagged )
					qmc2Gamelist->numTaggedSets--;
				else
					qmc2Gamelist->numTaggedSets++;
				labelGamelistStatus->setText(qmc2Gamelist->status());
			}
		}
	}
}

void MainWindow::on_treeWidgetHierarchy_itemPressed(QTreeWidgetItem *item, int column)
{
	on_treeWidgetHierarchy_itemEntered(item, column);
}

#if defined(QMC2_EMUTYPE_MAME)
void MainWindow::on_treeWidgetCategoryView_itemEntered(QTreeWidgetItem *item, int column)
{
	if ( column == QMC2_GAMELIST_COLUMN_TAG ) {
		if ( qApp->mouseButtons() == Qt::LeftButton ) {
			QPoint cPos = treeWidgetCategoryView->viewport()->mapFromGlobal(QCursor::pos());
			if ( treeWidgetCategoryView->itemAt(cPos) == item ) {
				Qt::CheckState cs = (item->checkState(column) == Qt::Unchecked ? Qt::Checked : Qt::Unchecked);
				bool wasTagged = (cs != Qt::Checked);
				item->setCheckState(column, cs);
				QString gameName = item->text(QMC2_GAMELIST_COLUMN_NAME);
				item = qmc2GamelistItemMap[gameName];
				if ( item )
					item->setCheckState(column, cs);
				item = qmc2HierarchyItemMap[gameName];
				if ( item )
					item->setCheckState(column, cs);
				item = qmc2VersionItemMap[gameName];
				if ( item )
					item->setCheckState(column, cs);
				if ( wasTagged )
					qmc2Gamelist->numTaggedSets--;
				else
					qmc2Gamelist->numTaggedSets++;
				labelGamelistStatus->setText(qmc2Gamelist->status());
			}
		}
	}
}

void MainWindow::on_treeWidgetCategoryView_itemPressed(QTreeWidgetItem *item, int column)
{
	on_treeWidgetCategoryView_itemEntered(item, column);
}

void MainWindow::on_treeWidgetVersionView_itemEntered(QTreeWidgetItem *item, int column)
{
	if ( column == QMC2_GAMELIST_COLUMN_TAG ) {
		if ( qApp->mouseButtons() == Qt::LeftButton ) {
			QPoint cPos = treeWidgetVersionView->viewport()->mapFromGlobal(QCursor::pos());
			if ( treeWidgetVersionView->itemAt(cPos) == item ) {
				Qt::CheckState cs = (item->checkState(column) == Qt::Unchecked ? Qt::Checked : Qt::Unchecked);
				bool wasTagged = (cs != Qt::Checked);
				item->setCheckState(column, cs);
				QString gameName = item->text(QMC2_GAMELIST_COLUMN_NAME);
				item = qmc2GamelistItemMap[gameName];
				if ( item )
					item->setCheckState(column, cs);
				item = qmc2HierarchyItemMap[gameName];
				if ( item )
					item->setCheckState(column, cs);
				item = qmc2CategoryItemMap[gameName];
				if ( item )
					item->setCheckState(column, cs);
				if ( wasTagged )
					qmc2Gamelist->numTaggedSets--;
				else
					qmc2Gamelist->numTaggedSets++;
				labelGamelistStatus->setText(qmc2Gamelist->status());
			}
		}
	}
}

void MainWindow::on_treeWidgetVersionView_itemPressed(QTreeWidgetItem *item, int column)
{
	on_treeWidgetVersionView_itemEntered(item, column);
}

void MainWindow::treeWidgetCategoryViewHeader_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::treeWidgetCategoryViewHeader_customContextMenuRequested(const QPoint &p = ...)");
#endif

	menuCategoryHeader->move(adjustedWidgetPosition(treeWidgetCategoryView->header()->viewport()->mapToGlobal(p), menuCategoryHeader));
	menuCategoryHeader->show();
}

void MainWindow::actionCategoryHeader_triggered()
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::actionCategoryHeader_triggered()");
#endif

	QAction *action = (QAction *)sender();
	int visibleColumns = 0;
	for (int i = 0; i < treeWidgetCategoryView->columnCount(); i++) if ( !treeWidgetCategoryView->isColumnHidden(i) ) visibleColumns++;
	if ( action ) {
		bool visibility = true;
		if ( action->isChecked() )
			treeWidgetCategoryView->setColumnHidden(action->data().toInt(), false);
		else if ( visibleColumns > 1 ) {
			treeWidgetCategoryView->setColumnHidden(action->data().toInt(), true);
			visibility = false;
		}
		action->blockSignals(true);
		action->setChecked(visibility);
		action->blockSignals(false);
	}
}

void MainWindow::treeWidgetVersionViewHeader_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::treeWidgetVersionViewHeader_customContextMenuRequested(const QPoint &p = ...)");
#endif

	menuVersionHeader->move(adjustedWidgetPosition(treeWidgetVersionView->header()->viewport()->mapToGlobal(p), menuVersionHeader));
	menuVersionHeader->show();
}

void MainWindow::actionVersionHeader_triggered()
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::actionVersionHeader_triggered()");
#endif

	QAction *action = (QAction *)sender();
	int visibleColumns = 0;
	for (int i = 0; i < treeWidgetVersionView->columnCount(); i++) if ( !treeWidgetVersionView->isColumnHidden(i) ) visibleColumns++;
	if ( action ) {
		bool visibility = true;
		if ( action->isChecked() )
			treeWidgetVersionView->setColumnHidden(action->data().toInt(), false);
		else if ( visibleColumns > 1 ) {
			treeWidgetVersionView->setColumnHidden(action->data().toInt(), true);
			visibility = false;
		}
		action->blockSignals(true);
		action->setChecked(visibility);
		action->blockSignals(false);
	}
}
#endif

void MainWindow::on_actionPlayTagged_activated()
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionPlayTagged_activated()");
#endif

	progressBarGamelist->reset();
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
		progressBarGamelist->setFormat(tr("Play tagged - %p%"));
	else
		progressBarGamelist->setFormat("%p%");
	progressBarGamelist->setRange(0, qmc2Gamelist->numGames);
	qApp->processEvents();
	int count = 0;

	QTreeWidgetItem *oldCurrentItem = qmc2CurrentItem;
	QMapIterator<QString, QTreeWidgetItem *> it(qmc2GamelistItemMap);
	QTreeWidgetItem *item;
	while ( it.hasNext() ) {
		progressBarGamelist->setValue(count++);
		it.next();
		item = qmc2GamelistItemMap[it.key()];
		if ( item ) {
			if ( item->checkState(QMC2_GAMELIST_COLUMN_TAG) == Qt::Checked ) {
				qApp->processEvents();
				qmc2CurrentItem = item;
				on_actionPlay_activated();
			}
		}
	}
	qmc2CurrentItem = oldCurrentItem;
	progressBarGamelist->reset();
}

void MainWindow::on_actionPlayEmbeddedTagged_activated()
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionPlayEmbeddedTagged_activated()");
#endif

	progressBarGamelist->reset();
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
		progressBarGamelist->setFormat(tr("Play tagged - %p%"));
	else
		progressBarGamelist->setFormat("%p%");
	progressBarGamelist->setRange(0, qmc2Gamelist->numGames);
	qApp->processEvents();
	int count = 0;

	QTreeWidgetItem *oldCurrentItem = qmc2CurrentItem;
	QMapIterator<QString, QTreeWidgetItem *> it(qmc2GamelistItemMap);
	QTreeWidgetItem *item;
	while ( it.hasNext() ) {
		progressBarGamelist->setValue(count++);
		it.next();
		item = qmc2GamelistItemMap[it.key()];
		if ( item ) {
			if ( item->checkState(QMC2_GAMELIST_COLUMN_TAG) == Qt::Checked ) {
				qApp->processEvents();
				qmc2CurrentItem = item;
				qmc2StartEmbedded = true;
				on_actionPlay_activated();
				QTest::qWait(2*QMC2_EMBED_DELAY);
			}
		}
	}
	qmc2CurrentItem = oldCurrentItem;
	progressBarGamelist->reset();
}

void MainWindow::on_actionToFavoritesTagged_activated()
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionToFavoritesTagged_activated()");
#endif

	progressBarGamelist->reset();
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
		progressBarGamelist->setFormat(tr("Add favorites - %p%"));
	else
		progressBarGamelist->setFormat("%p%");
	progressBarGamelist->setRange(0, qmc2Gamelist->numGames);
	qApp->processEvents();
	int count = 0;

	QTreeWidgetItem *oldCurrentItem = qmc2CurrentItem;
	QMapIterator<QString, QTreeWidgetItem *> it(qmc2GamelistItemMap);
	QTreeWidgetItem *item;
	while ( it.hasNext() ) {
		progressBarGamelist->setValue(count++);
		it.next();
		item = qmc2GamelistItemMap[it.key()];
		if ( item ) {
			if ( item->checkState(QMC2_GAMELIST_COLUMN_TAG) == Qt::Checked ) {
				qApp->processEvents();
				qmc2CurrentItem = item;
				on_actionToFavorites_activated();
			}
		}
	}
	qmc2CurrentItem = oldCurrentItem;
	progressBarGamelist->reset();
}

void MainWindow::on_actionCheckROMStateTagged_activated()
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionCheckROMStateTagged_activated()");
#endif

	if ( isActiveState ) {
		log(QMC2_LOG_FRONTEND, tr("please wait for current activity to finish and try again (this batch-mode operation can only run exclusively)"));
		return;
	}

	QMapIterator<QString, QTreeWidgetItem *> it(qmc2GamelistItemMap);
	QTreeWidgetItem *item;
	while ( it.hasNext() && !qmc2StopParser ) {
		it.next();
		item = qmc2GamelistItemMap[it.key()];
		if ( item ) {
			if ( item->checkState(QMC2_GAMELIST_COLUMN_TAG) == Qt::Checked ) {
				qApp->processEvents();
				qmc2CurrentItem = item;
				on_actionCheckCurrentROM_activated();
				while ( qmc2VerifyActive && !qmc2StopParser )
					QTest::qWait(QMC2_TAGGEDROMCHECK_DELAY);
			}
		}
	}
}

void MainWindow::on_actionAnalyseROMTagged_activated()
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionAnalyseROMTagged_activated()");
#endif

	if ( isActiveState ) {
		log(QMC2_LOG_FRONTEND, tr("please wait for current activity to finish and try again (this batch-mode operation can only run exclusively)"));
		return;
	}

	QStringList setsToAnalyze;
	QMapIterator<QString, QTreeWidgetItem *> it(qmc2GamelistItemMap);
	QTreeWidgetItem *item;
	while ( it.hasNext() ) {
		it.next();
		item = qmc2GamelistItemMap[it.key()];
		if ( item )
			if ( item->checkState(QMC2_GAMELIST_COLUMN_TAG) == Qt::Checked )
				setsToAnalyze << item->text(QMC2_GAMELIST_COLUMN_NAME);
	}

	if ( !qmc2ROMAlyzer )
		qmc2ROMAlyzer = new ROMAlyzer(this);

	qmc2ROMAlyzer->quickSearch = true;
	qmc2ROMAlyzer->lineEditGames->setText(setsToAnalyze.join(" "));

	if ( qmc2ROMAlyzer->isHidden() )
		qmc2ROMAlyzer->show();
	else if ( qmc2ROMAlyzer->isMinimized() )
		qmc2ROMAlyzer->showNormal();

	if ( qmc2ROMAlyzer->tabWidgetAnalysis->currentWidget() != qmc2ROMAlyzer->tabReport && qmc2ROMAlyzer->tabWidgetAnalysis->currentWidget() != qmc2ROMAlyzer->tabLog )
		qmc2ROMAlyzer->tabWidgetAnalysis->setCurrentWidget(qmc2ROMAlyzer->tabReport);

	QTimer::singleShot(0, qmc2ROMAlyzer, SLOT(raise()));
	QTimer::singleShot(0, qmc2ROMAlyzer->pushButtonAnalyze, SLOT(click()));
}

void MainWindow::on_actionRunRomToolTagged_activated()
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionRunRomToolTagged_activated()");
#endif

	if ( isActiveState ) {
		log(QMC2_LOG_FRONTEND, tr("please wait for current activity to finish and try again (this batch-mode operation can only run exclusively)"));
		return;
	}

	progressBarGamelist->reset();
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
		progressBarGamelist->setFormat(tr("ROM tool tagged - %p%"));
	else
		progressBarGamelist->setFormat("%p%");
	progressBarGamelist->setRange(0, qmc2Gamelist->numGames);
	qApp->processEvents();
	int count = 0;

	QTreeWidgetItem *oldCurrentItem = qmc2CurrentItem;
	QMapIterator<QString, QTreeWidgetItem *> it(qmc2GamelistItemMap);
	QTreeWidgetItem *item;
	while ( it.hasNext() && !qmc2StopParser ) {
		progressBarGamelist->setValue(count++);
		it.next();
		item = qmc2GamelistItemMap[it.key()];
		if ( item ) {
			if ( item->checkState(QMC2_GAMELIST_COLUMN_TAG) == Qt::Checked ) {
				qApp->processEvents();
				qmc2CurrentItem = item;
				on_actionRunRomTool_activated();
			}
		}
	}
	qmc2CurrentItem = oldCurrentItem;
	progressBarGamelist->reset();
}

void MainWindow::on_actionSetTag_activated()
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionSetTag_activated()");
#endif

	if ( !qmc2CurrentItem )
		return;

	bool wasUntagged = false;
	QString gameName = qmc2CurrentItem->text(QMC2_GAMELIST_COLUMN_NAME);
	QTreeWidgetItem *item = qmc2GamelistItemMap[gameName];
	if ( item ) {
		wasUntagged = (item->checkState(QMC2_GAMELIST_COLUMN_TAG) == Qt::Unchecked);
		item->setCheckState(QMC2_GAMELIST_COLUMN_TAG, Qt::Checked);
	}
	item = qmc2HierarchyItemMap[gameName];
	if ( item )
		item->setCheckState(QMC2_GAMELIST_COLUMN_TAG, Qt::Checked);
#if defined(QMC2_EMUTYPE_MAME)
	item = qmc2CategoryItemMap[gameName];
	if ( item )
		item->setCheckState(QMC2_GAMELIST_COLUMN_TAG, Qt::Checked);
	item = qmc2VersionItemMap[gameName];
	if ( item )
		item->setCheckState(QMC2_GAMELIST_COLUMN_TAG, Qt::Checked);
#endif
	if ( wasUntagged ) {
		qmc2Gamelist->numTaggedSets++;
		labelGamelistStatus->setText(qmc2Gamelist->status());
	}
}

void MainWindow::on_actionUnsetTag_activated()
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionUnsetTag_activated()");
#endif

	if ( !qmc2CurrentItem )
		return;

	bool wasTagged = false;
	QString gameName = qmc2CurrentItem->text(QMC2_GAMELIST_COLUMN_NAME);
	QTreeWidgetItem *item = qmc2GamelistItemMap[gameName];
	if ( item ) {
		wasTagged = (item->checkState(QMC2_GAMELIST_COLUMN_TAG) == Qt::Checked);
		item->setCheckState(QMC2_GAMELIST_COLUMN_TAG, Qt::Unchecked);
	}
	item = qmc2HierarchyItemMap[gameName];
	if ( item )
		item->setCheckState(QMC2_GAMELIST_COLUMN_TAG, Qt::Unchecked);
#if defined(QMC2_EMUTYPE_MAME)
	item = qmc2CategoryItemMap[gameName];
	if ( item )
		item->setCheckState(QMC2_GAMELIST_COLUMN_TAG, Qt::Unchecked);
	item = qmc2VersionItemMap[gameName];
	if ( item )
		item->setCheckState(QMC2_GAMELIST_COLUMN_TAG, Qt::Unchecked);
#endif
	if ( wasTagged ) {
		qmc2Gamelist->numTaggedSets--;
		labelGamelistStatus->setText(qmc2Gamelist->status());
	}
}

void MainWindow::on_actionToggleTag_activated()
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionToggleTag_activated()");
#endif

	if ( !qmc2CurrentItem )
		return;

	QString gameName = qmc2CurrentItem->text(QMC2_GAMELIST_COLUMN_NAME);
	QTreeWidgetItem *item = qmc2GamelistItemMap[gameName];
	if ( item ) {
		Qt::CheckState cs = (item->checkState(QMC2_GAMELIST_COLUMN_TAG) == Qt::Unchecked ? Qt::Checked : Qt::Unchecked);
		bool wasTagged = (cs != Qt::Checked);
		item->setCheckState(QMC2_GAMELIST_COLUMN_TAG, cs);
		item = qmc2HierarchyItemMap[gameName];
		if ( item )
			item->setCheckState(QMC2_GAMELIST_COLUMN_TAG, cs);
#if defined(QMC2_EMUTYPE_MAME)
		item = qmc2CategoryItemMap[gameName];
		if ( item )
			item->setCheckState(QMC2_GAMELIST_COLUMN_TAG, cs);
		item = qmc2VersionItemMap[gameName];
		if ( item )
			item->setCheckState(QMC2_GAMELIST_COLUMN_TAG, cs);
#endif
		if ( wasTagged )
			qmc2Gamelist->numTaggedSets--;
		else
			qmc2Gamelist->numTaggedSets++;
		labelGamelistStatus->setText(qmc2Gamelist->status());
	}
}

void MainWindow::on_actionTagAll_activated()
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionTagAll_activated()");
#endif

	progressBarGamelist->reset();
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
		progressBarGamelist->setFormat(tr("Tag - %p%"));
	else
		progressBarGamelist->setFormat("%p%");
	progressBarGamelist->setRange(0, qmc2Gamelist->numGames);
	qApp->processEvents();
	int count = 0;

	QMapIterator<QString, QTreeWidgetItem *> it(qmc2GamelistItemMap);
	QTreeWidgetItem *item;
	while ( it.hasNext() ) {
		progressBarGamelist->setValue(count++);
		it.next();
		item = qmc2GamelistItemMap[it.key()];
		if ( item )
			item->setCheckState(QMC2_GAMELIST_COLUMN_TAG, Qt::Checked);
		item = qmc2HierarchyItemMap[it.key()];
		if ( item )
			item->setCheckState(QMC2_GAMELIST_COLUMN_TAG, Qt::Checked);
#if defined(QMC2_EMUTYPE_MAME)
		item = qmc2CategoryItemMap[it.key()];
		if ( item )
			item->setCheckState(QMC2_GAMELIST_COLUMN_TAG, Qt::Checked);
		item = qmc2VersionItemMap[it.key()];
		if ( item )
			item->setCheckState(QMC2_GAMELIST_COLUMN_TAG, Qt::Checked);
#endif
		qmc2Gamelist->numTaggedSets++;
		labelGamelistStatus->setText(qmc2Gamelist->status());
	}
	progressBarGamelist->reset();
	qmc2Gamelist->numTaggedSets = qmc2Gamelist->numGames;
	labelGamelistStatus->setText(qmc2Gamelist->status());
}

void MainWindow::on_actionUntagAll_activated()
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionUntagAll_activated()");
#endif

	progressBarGamelist->reset();
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
		progressBarGamelist->setFormat(tr("Untag - %p%"));
	else
		progressBarGamelist->setFormat("%p%");
	progressBarGamelist->setRange(0, qmc2Gamelist->numGames);
	qApp->processEvents();
	int count = 0;

	QMapIterator<QString, QTreeWidgetItem *> it(qmc2GamelistItemMap);
	QTreeWidgetItem *item;
	while ( it.hasNext() ) {
		progressBarGamelist->setValue(count++);
		it.next();
		item = qmc2GamelistItemMap[it.key()];
		if ( item )
			item->setCheckState(QMC2_GAMELIST_COLUMN_TAG, Qt::Unchecked);
		item = qmc2HierarchyItemMap[it.key()];
		if ( item )
			item->setCheckState(QMC2_GAMELIST_COLUMN_TAG, Qt::Unchecked);
#if defined(QMC2_EMUTYPE_MAME)
		item = qmc2CategoryItemMap[it.key()];
		if ( item )
			item->setCheckState(QMC2_GAMELIST_COLUMN_TAG, Qt::Unchecked);
		item = qmc2VersionItemMap[it.key()];
		if ( item )
			item->setCheckState(QMC2_GAMELIST_COLUMN_TAG, Qt::Unchecked);
#endif
		qmc2Gamelist->numTaggedSets--;
		labelGamelistStatus->setText(qmc2Gamelist->status());
	}
	progressBarGamelist->reset();
	qmc2Gamelist->numTaggedSets = 0;
	labelGamelistStatus->setText(qmc2Gamelist->status());
}

void MainWindow::on_actionInvertTags_activated()
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, "DEBUG: MainWindow::on_actionInvertTags_activated()");
#endif

	progressBarGamelist->reset();
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
		progressBarGamelist->setFormat(tr("Invert tag - %p%"));
	else
		progressBarGamelist->setFormat("%p%");
	progressBarGamelist->setRange(0, qmc2Gamelist->numGames);
	qApp->processEvents();
	int count = 0;

	QMapIterator<QString, QTreeWidgetItem *> it(qmc2GamelistItemMap);
	QTreeWidgetItem *item;
	while ( it.hasNext() ) {
		progressBarGamelist->setValue(count++);
		it.next();
		item = qmc2GamelistItemMap[it.key()];
		if ( item ) {
			Qt::CheckState cs = (item->checkState(QMC2_GAMELIST_COLUMN_TAG) == Qt::Unchecked ? Qt::Checked : Qt::Unchecked);
			bool wasTagged = (cs != Qt::Checked);
			item->setCheckState(QMC2_GAMELIST_COLUMN_TAG, cs);
			item = qmc2HierarchyItemMap[it.key()];
			if ( item )
				item->setCheckState(QMC2_GAMELIST_COLUMN_TAG, cs);
#if defined(QMC2_EMUTYPE_MAME)
			item = qmc2CategoryItemMap[it.key()];
			if ( item )
				item->setCheckState(QMC2_GAMELIST_COLUMN_TAG, cs);
			item = qmc2VersionItemMap[it.key()];
			if ( item )
				item->setCheckState(QMC2_GAMELIST_COLUMN_TAG, cs);
#endif
			if ( wasTagged )
				qmc2Gamelist->numTaggedSets--;
			else
				qmc2Gamelist->numTaggedSets++;
			labelGamelistStatus->setText(qmc2Gamelist->status());
		}
	}
	progressBarGamelist->reset();
}

#if defined(QMC2_EMUTYPE_MESS)
// note: - this routine is far from "elegant" but basically works (there may be minor conversion "bugs", though, depending on the quality of the wiki source data)
//       - if someone knows a CLEAN wiki2html converter that's not "bloated" and written in C/C++ (and legally redistributable open source code), please let us know!
QString &MainWindow::messWikiToHtml(QString &wikiText)
{
	int ulLevel = 0;
	int olLevel = 0;
	bool tableOpen = false;
	bool preOn = false;
	bool codeOn = false;
	int preCounter = 0;
	QStringList wikiLines = wikiText.split("<p>");
	wikiText.clear();
	foreach (QString wikiLine, wikiLines) {
		// DEBUG
		//printf("wikiLine = [%s]\n", (const char *)wikiLine.toAscii()); fflush(stdout);
		// DEBUG
		QString wikiLineTrimmed = wikiLine.trimmed();
		if ( wikiLine.indexOf(QRegExp("\\s*<code>")) == 0 ) {
			codeOn = true;
			continue;
		} 
		if ( wikiLine.indexOf(QRegExp("\\s*</code>")) == 0 )
			codeOn = false;
		bool listDetected = ( wikiLineTrimmed.startsWith("* ") || wikiLineTrimmed.startsWith("- ") );
		if ( wikiLine == "  * " || wikiLine == "  - " || wikiLine == "  *" || wikiLine == "  -" ) continue; // this is an "artifact"... ignore :)
		if ( !listDetected && (wikiLine.startsWith("  ") || codeOn) ) {
			if ( tableOpen ) { wikiText += "</table><p>"; tableOpen = false; }
			if ( ulLevel > 0 ) { for (int i = 0; i < ulLevel; i++) wikiText += "</ul>"; ulLevel = 0; wikiText += "<p>"; }
			if ( olLevel > 0 ) { for (int i = 0; i < olLevel; i++) wikiText += "</ol>"; olLevel = 0; wikiText += "<p>"; }
			if ( wikiLine == "  "  && preCounter == 0 ) {
				preCounter++;
				wikiText += "\n";
				continue;
			}
			if ( !preOn ) {
				wikiText += "<p><table border=\"1\"><tr><td><pre>";
				preOn = true;
			}
			if ( wikiLine == "  " ) {
				wikiText += "\n";
				continue;
			}
		} else if ( preOn ) {
			preCounter++;
			wikiText += "</pre></td></tr></table><p>";
			preOn = codeOn = false;
		}
		int listDepth = 0;
		if ( listDetected ) listDepth = wikiLine.indexOf(QRegExp("[\\-\\*]")) / 2;
		if ( !preOn ) {
			wikiLine = wikiLineTrimmed;
			preCounter = 0;
		}
		if ( wikiLine.isEmpty() )
			continue;
		wikiLine.replace("<", "&lt;").replace(">", "&gt;");
		if ( wikiLine.startsWith("//") && wikiLine.endsWith("//") ) {
			wikiLine.replace(0, 2, "<i>");
			wikiLine.replace(wikiLine.length() - 2, 2, "</i>");
		}
		foreach (QString snippet, wikiLine.split("//")) {
			if ( snippet.indexOf(QRegExp("^.*(http:|https:|ftp:)$")) < 0 )
				wikiLine.replace(QString("//%1//").arg(snippet), QString("<i>%1</i>").arg(snippet));
		}
		wikiLine.replace(QRegExp("\\*\\*(.*)\\*\\*"), "<b>\\1</b>");
		wikiLine.replace(QRegExp("__(.*)__"), "<u>\\1</u>");
		wikiLine.replace(QRegExp("\\[\\[wp>([^\\]]*)\\]\\]"), QLatin1String("\\1 -- http://en.wikipedia.org/wiki/\\1"));
		foreach (QString snippet, wikiLine.split("[[")) {
			if ( snippet.indexOf(QRegExp("\\]\\]|\\|")) > 0 ) {
				QStringList subSnippets = snippet.split(QRegExp("\\]\\]|\\|"));
				wikiLine.replace(QString("[[%1]]").arg(snippet), subSnippets[0]);
			}
		}
		wikiLine.replace(QRegExp(QString("((http|https|ftp)://%1)").arg(urlSectionRegExp)), QLatin1String("<a href=\"\\1\">\\1</a>"));
		if ( wikiLine.startsWith("&lt;b&gt;======") && wikiLine.endsWith("======&lt;/b&gt;") ) {
			if ( tableOpen ) { wikiText += "</table><p>"; tableOpen = false; }
			if ( ulLevel > 0 ) { for (int i = 0; i < ulLevel; i++) wikiText += "</ul>"; ulLevel = 0; wikiText += "<p>"; }
			if ( olLevel > 0 ) { for (int i = 0; i < olLevel; i++) wikiText += "</ol>"; olLevel = 0; wikiText += "<p>"; }
			wikiText += "<h2>" + wikiLine.mid(15, wikiLine.length() - 31) + "</h2>";
		} else if ( wikiLine.startsWith("=====") && wikiLine.endsWith("=====") ) {
			if ( tableOpen ) { wikiText += "</table><p>"; tableOpen = false; }
			if ( ulLevel > 0 ) { for (int i = 0; i < ulLevel; i++) wikiText += "</ul>"; ulLevel = 0; wikiText += "<p>"; }
			if ( olLevel > 0 ) { for (int i = 0; i < olLevel; i++) wikiText += "</ol>"; olLevel = 0; wikiText += "<p>"; }
			wikiText += "<h3>" + wikiLine.mid(6, wikiLine.length() - 12) + "</h3>";
		} else if ( wikiLine.startsWith("====") && wikiLine.endsWith("====") ) {
			if ( tableOpen ) { wikiText += "</table><p>"; tableOpen = false; }
			if ( ulLevel > 0 ) { for (int i = 0; i < ulLevel; i++) wikiText += "</ul>"; ulLevel = 0; wikiText += "<p>"; }
			if ( olLevel > 0 ) { for (int i = 0; i < olLevel; i++) wikiText += "</ol>"; olLevel = 0; wikiText += "<p>"; }
			wikiText += "<h4>" + wikiLine.mid(5, wikiLine.length() - 10) + "</h4>";
		} else if ( wikiLine.startsWith("===") && wikiLine.endsWith("===") ) {
			if ( tableOpen ) { wikiText += "</table><p>"; tableOpen = false; }
			if ( ulLevel > 0 ) { for (int i = 0; i < ulLevel; i++) wikiText += "</ul>"; ulLevel = 0; wikiText += "<p>"; }
			if ( olLevel > 0 ) { for (int i = 0; i < olLevel; i++) wikiText += "</ol>"; olLevel = 0; wikiText += "<p>"; }
			wikiText += "<b>" + wikiLine.mid(4, wikiLine.length() - 8) + "</b>";
		} else if ( wikiLine.startsWith("==") && wikiLine.endsWith("==") ) {
			if ( tableOpen ) { wikiText += "</table><p>"; tableOpen = false; }
			if ( ulLevel > 0 ) { for (int i = 0; i < ulLevel; i++) wikiText += "</ul>"; ulLevel = 0; wikiText += "<p>"; }
			if ( olLevel > 0 ) { for (int i = 0; i < olLevel; i++) wikiText += "</ol>"; olLevel = 0; wikiText += "<p>"; }
			wikiText += "<b>" + wikiLine.mid(3, wikiLine.length() - 6) + "</b>";
		} else if ( wikiLine.indexOf(QRegExp("\\* \\S")) == 0 ) {
			if ( tableOpen ) { wikiText += "</table><p>"; tableOpen = false; }
			if ( olLevel > 0 ) { for (int i = 0; i < olLevel; i++) wikiText += "</ol>"; olLevel = 0; wikiText += "<p>"; }
			if ( listDepth > ulLevel ) {
				wikiText += "<ul style=\"list-style-type:square;\">";
				ulLevel++;
			} else if ( listDepth < ulLevel ) {
				wikiText += "</ul>";
				ulLevel--;
			}
			wikiText += "<li>" + wikiLine.mid(2) + "</li>";
		} else if ( wikiLine.indexOf(QRegExp("\\- \\S")) == 0 ) {
			if ( tableOpen ) { wikiText += "</table><p>"; tableOpen = false; }
			if ( ulLevel > 0 ) { for (int i = 0; i < ulLevel; i++) wikiText += "</ul>"; ulLevel = 0; wikiText += "<p>"; }
			if ( listDepth > olLevel ) {
				wikiText += "<ol style=\"list-style-type:decimal;\">";
				olLevel++;
			} else if ( listDepth < olLevel ) {
				wikiText += "</ol>";
				olLevel--;
			}
			wikiText += "<li>" + wikiLine.mid(2) + "</li>";
		} else if ( ( wikiLine.startsWith("| ") && wikiLine.endsWith(" |") ) || ( wikiLine.startsWith("^ ") && wikiLine.endsWith(" |") ) ) {
			if ( ulLevel > 0 ) { for (int i = 0; i < ulLevel; i++) wikiText += "</ul>"; ulLevel = 0; wikiText += "<p>"; }
			if ( olLevel > 0 ) { for (int i = 0; i < olLevel; i++) wikiText += "</ol>"; olLevel = 0; wikiText += "<p>"; }
			if ( !tableOpen ) { wikiText += "<p><table border=\"1\">"; tableOpen = true; }
			wikiText += "<tr>";
			foreach (QString cell, wikiLine.split(QRegExp("\\^|\\|"), QString::SkipEmptyParts)) wikiText += "<td>" + cell + "</td>";
			wikiText += "</tr>";
		} else if ( wikiLine.startsWith("^ ") && wikiLine.endsWith(" ^") ) {
			if ( ulLevel > 0 ) { for (int i = 0; i < ulLevel; i++) wikiText += "</ul>"; ulLevel = 0; wikiText += "<p>"; }
			if ( olLevel > 0 ) { for (int i = 0; i < olLevel; i++) wikiText += "</ol>"; olLevel = 0; wikiText += "<p>"; }
			if ( !tableOpen ) { wikiText += "<p><table border=\"1\">"; tableOpen = true; }
			wikiText += "<tr>";
			foreach (QString cell, wikiLine.split("^", QString::SkipEmptyParts)) wikiText += "<td><b>" + cell + "</b></td>";
			wikiText += "</tr>";
		} else {
			if ( tableOpen ) { wikiText += "</table><p>"; tableOpen = false; }
			if ( ulLevel > 0 ) { for (int i = 0; i < ulLevel; i++) wikiText += "</ul>"; ulLevel = 0; wikiText += "<p>"; }
			if ( olLevel > 0 ) { for (int i = 0; i < olLevel; i++) wikiText += "</ol>"; olLevel = 0; wikiText += "<p>"; }
			if ( preOn ) {
				wikiText += wikiLine.mid(2) + "\n";
			} else if ( codeOn ) {
				wikiText += wikiLine + "\n";
			} else
				wikiText += "<p>" + wikiLine + "</p>";
		}
	}
	return wikiText;
}
#endif

void MainWindow::floatToggleButtonSoftwareDetail_toggled(bool checked)
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::floatToggleButtonSoftwareDetail_toggled(bool checked = %1)").arg(checked));
#endif

	if ( qmc2EarlyStartup )
		return;

	if ( checked ) {
		if ( tabWidgetSoftwareDetail->parent() == this ) {
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/SoftwareDetailGeometry", tabWidgetSoftwareDetail->saveGeometry());
			stackedWidgetSpecial->insertWidget(QMC2_SPECIAL_SOFTWARE_PAGE, tabWidgetSoftwareDetail);
			stackedWidgetSpecial->setCurrentIndex(QMC2_SPECIAL_SOFTWARE_PAGE);
		}
	} else {
		stackedWidgetSpecial->setCurrentIndex(QMC2_SPECIAL_DEFAULT_PAGE);
		stackedWidgetSpecial->removeWidget(tabWidgetSoftwareDetail);
		tabWidgetSoftwareDetail->setParent(this);
		tabWidgetSoftwareDetail->setAttribute(Qt::WA_ShowWithoutActivating);
		tabWidgetSoftwareDetail->setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
		tabWidgetSoftwareDetail->setWindowIcon(qApp->windowIcon());
		tabWidgetSoftwareDetail->setWindowTitle(tr("Software detail"));
		if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/SoftwareDetailGeometry") )
			tabWidgetSoftwareDetail->restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/SoftwareDetailGeometry").toByteArray());
		if ( qmc2SoftwareList ) {
			tabWidgetSoftwareDetail->showNormal();
			tabWidgetSoftwareDetail->raise();
		}
	}
}

void MainWindow::stackedWidgetSpecial_setCurrentIndex(int index)
{
#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, QString("DEBUG: MainWindow::stackedWidgetSpecial_setCurrentIndex(int index = %1)").arg(index));
#endif

	switch ( index ) {
		case QMC2_SPECIAL_DEFAULT_PAGE:
			stackedWidgetSpecial->setCurrentIndex(QMC2_SPECIAL_DEFAULT_PAGE);
			if ( tabWidgetSoftwareDetail->parent() == this )
				tabWidgetSoftwareDetail->hide();
			break;

		case QMC2_SPECIAL_SOFTWARE_PAGE:
			if ( qmc2SoftwareList ) {
				if ( !floatToggleButtonSoftwareDetail->isChecked() ) {
					stackedWidgetSpecial->setCurrentIndex(QMC2_SPECIAL_DEFAULT_PAGE);
					qmc2SoftwareList->detailUpdateTimer.start(qmc2UpdateDelay);
					tabWidgetSoftwareDetail->showNormal();
					tabWidgetSoftwareDetail->raise();
				} else
					stackedWidgetSpecial->setCurrentIndex(QMC2_SPECIAL_SOFTWARE_PAGE);
			} else
				stackedWidgetSpecial->setCurrentIndex(QMC2_SPECIAL_SOFTWARE_PAGE);
			break;

		default:
			stackedWidgetSpecial->setCurrentIndex(index);
			break;
	}
}

void MainWindow::comboBoxToolbarSearch_activated(const QString &text)
{
	comboBoxSearch->lineEdit()->setText(text);
	tabWidgetGamelist->blockSignals(true);
	tabWidgetGamelist->setCurrentIndex(QMC2_SEARCH_INDEX);
	tabWidgetGamelist->blockSignals(false);
	QTimer::singleShot(0, listWidgetSearch, SLOT(setFocus()));
}

void MainWindow::comboBoxToolbarSearch_textChanged(const QString &text)
{
	if ( tabWidgetGamelist->currentIndex() == QMC2_SEARCH_INDEX )
		comboBoxSearch->lineEdit()->setText(text);
}

void myQtMessageHandler(QtMsgType type, const char *msg)
{
  if ( qmc2SuppressQtMessages )
    return;

  if ( !qmc2GuiReady )
    return;

  switch ( type ) {
    case QtDebugMsg:
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, "QtDebugMsg: " + QString(msg));
      break;

    case QtWarningMsg:
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, "QtWarningMsg: " + QString(msg));
      break;

    case QtCriticalMsg:
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, "QtCriticalMsg: " + QString(msg));
      break;

    case QtFatalMsg:
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, "QtFatalMsg: " + QString(msg));
      break;

    default:
      break;
  }
}

void prepareShortcuts()
{
  // shortcuts
  qmc2ShortcutMap["Ctrl+1"].second = qmc2MainWindow->actionCheckROMs;
  qmc2ShortcutMap["Ctrl+2"].second = qmc2MainWindow->actionCheckSamples;
  qmc2ShortcutMap["Ctrl+3"].second = qmc2MainWindow->actionCheckPreviews;
  qmc2ShortcutMap["Ctrl+4"].second = qmc2MainWindow->actionCheckFlyers;
  qmc2ShortcutMap["Ctrl+5"].second = qmc2MainWindow->actionCheckIcons;
  qmc2ShortcutMap["Ctrl+A"].second = qmc2MainWindow->actionAbout;
  qmc2ShortcutMap["Ctrl+D"].second = qmc2MainWindow->actionAnalyseCurrentROM;
  qmc2ShortcutMap["Ctrl+Shift+D"].second = qmc2MainWindow->actionAnalyseROMTagged;
  qmc2ShortcutMap["Ctrl+E"].second = qmc2MainWindow->actionExportROMStatus;
  qmc2ShortcutMap["Ctrl+F"].second = qmc2MainWindow->actionToFavorites;
  qmc2ShortcutMap["Ctrl+Shift+F"].second = qmc2MainWindow->actionToFavoritesTagged;
  qmc2ShortcutMap["Ctrl+H"].second = qmc2MainWindow->actionDocumentation;
  qmc2ShortcutMap["Ctrl+I"].second = qmc2MainWindow->actionClearImageCache;
  qmc2ShortcutMap["Ctrl+Shift+A"].second = qmc2MainWindow->actionArcadeSetup;
  qmc2ShortcutMap["Ctrl+M"].second = qmc2MainWindow->actionClearMAWSCache;
  qmc2ShortcutMap["Ctrl+N"].second = qmc2MainWindow->actionClearIconCache;
  qmc2ShortcutMap["Ctrl+O"].second = qmc2MainWindow->actionOptions;
  qmc2ShortcutMap["Ctrl+P"].second = qmc2MainWindow->actionPlay;
#if defined(Q_WS_X11) || defined(Q_WS_WIN)
  qmc2ShortcutMap["Ctrl+Shift+P"].second = qmc2MainWindow->actionPlayEmbedded;
#endif
  qmc2ShortcutMap["Ctrl+Q"].second = qmc2MainWindow->actionAboutQt;
  qmc2ShortcutMap["Ctrl+R"].second = qmc2MainWindow->actionReload;
  qmc2ShortcutMap["Ctrl+S"].second = qmc2MainWindow->actionCheckCurrentROM;
  qmc2ShortcutMap["Ctrl+Shift+S"].second = qmc2MainWindow->actionCheckROMStateTagged;
  qmc2ShortcutMap["Ctrl+T"].second = qmc2MainWindow->actionRecreateTemplateMap;
  qmc2ShortcutMap["Ctrl+C"].second = qmc2MainWindow->actionCheckTemplateMap;
  qmc2ShortcutMap["Ctrl+X"].second = qmc2MainWindow->actionExitStop;
#if defined(QMC2_YOUTUBE_ENABLED)
  qmc2ShortcutMap["Ctrl+Y"].second = qmc2MainWindow->actionClearYouTubeCache;
#endif
  qmc2ShortcutMap["Ctrl+Z"].second = qmc2MainWindow->actionROMAlyzer;
  qmc2ShortcutMap["Ctrl+Alt+C"].second = qmc2MainWindow->actionRomStatusFilterC;
  qmc2ShortcutMap["Ctrl+Alt+M"].second = qmc2MainWindow->actionRomStatusFilterM;
  qmc2ShortcutMap["Ctrl+Alt+I"].second = qmc2MainWindow->actionRomStatusFilterI;
  qmc2ShortcutMap["Ctrl+Alt+N"].second = qmc2MainWindow->actionRomStatusFilterN;
  qmc2ShortcutMap["Ctrl+Alt+U"].second = qmc2MainWindow->actionRomStatusFilterU;
#if defined(QMC2_VARIANT_LAUNCHER)
  qmc2ShortcutMap["Ctrl+Alt+1"].second = qmc2MainWindow->actionLaunchQMC2MAME;
  qmc2ShortcutMap["Ctrl+Alt+2"].second = qmc2MainWindow->actionLaunchQMC2MESS;
#endif
  qmc2ShortcutMap["Ctrl+Shift+T"].second = qmc2MainWindow->actionSetTag;
  qmc2ShortcutMap["Ctrl+Shift+U"].second = qmc2MainWindow->actionUnsetTag;
  qmc2ShortcutMap["Ctrl+Shift+G"].second = qmc2MainWindow->actionToggleTag;
  qmc2ShortcutMap["Ctrl+Shift+L"].second = qmc2MainWindow->actionTagAll;
  qmc2ShortcutMap["Ctrl+Shift+N"].second = qmc2MainWindow->actionUntagAll;
  qmc2ShortcutMap["Ctrl+Shift+I"].second = qmc2MainWindow->actionInvertTags;
  qmc2ShortcutMap["F5"].second = qmc2MainWindow->actionViewFullDetail;
  qmc2ShortcutMap["F6"].second = qmc2MainWindow->actionViewParentClones;
#if defined(QMC2_EMUTYPE_MAME)
  qmc2ShortcutMap["F7"].second = qmc2MainWindow->actionViewByCategory;
  qmc2ShortcutMap["F8"].second = qmc2MainWindow->actionViewByVersion;
#endif
  qmc2ShortcutMap["F9"].second = qmc2MainWindow->actionRunRomTool;
  qmc2ShortcutMap["Ctrl+Shift+F9"].second = qmc2MainWindow->actionRunRomToolTagged;
  qmc2ShortcutMap["F11"].second = qmc2MainWindow->actionFullscreenToggle;
  qmc2ShortcutMap["F12"].second = qmc2MainWindow->actionArcadeToggle;
  qmc2ShortcutMap["Meta+F"].second = qmc2MainWindow->actionArcadeShowFPS;
  qmc2ShortcutMap["Meta+F12"].second = qmc2MainWindow->actionArcadeTakeScreenshot;
#if QMC2_USE_PHONON_API
  qmc2ShortcutMap["Ctrl+Alt+Left"].second = qmc2MainWindow->actionAudioPreviousTrack;
  qmc2ShortcutMap["Ctrl+Alt+Right"].second = qmc2MainWindow->actionAudioNextTrack;
  qmc2ShortcutMap["Ctrl+Alt+B"].second = qmc2MainWindow->actionAudioFastBackward;
  qmc2ShortcutMap["Ctrl+Alt+F"].second = qmc2MainWindow->actionAudioFastForward;
  qmc2ShortcutMap["Ctrl+Alt+S"].second = qmc2MainWindow->actionAudioStopTrack;
  qmc2ShortcutMap["Ctrl+Alt+#"].second = qmc2MainWindow->actionAudioPauseTrack;
  qmc2ShortcutMap["Ctrl+Alt+P"].second = qmc2MainWindow->actionAudioPlayTrack;
  qmc2ShortcutMap["Ctrl+Alt+PgUp"].second = qmc2MainWindow->actionAudioRaiseVolume;
  qmc2ShortcutMap["Ctrl+Alt+PgDown"].second = qmc2MainWindow->actionAudioLowerVolume;
#endif

  // special keys
  qmc2ShortcutMap["+"].second = NULL;
  qmc2QtKeyMap["+"] = Qt::Key_Plus;
  qmc2ShortcutMap["-"].second = NULL;
  qmc2QtKeyMap["-"] = Qt::Key_Minus;
  qmc2ShortcutMap["Down"].second = NULL;
  qmc2QtKeyMap["Down"] = Qt::Key_Down;
  qmc2ShortcutMap["End"].second = NULL;
  qmc2QtKeyMap["End"] = Qt::Key_End;
  qmc2ShortcutMap["Esc"].second = NULL;
  qmc2QtKeyMap["Esc"] = Qt::Key_Escape;
  qmc2ShortcutMap["Left"].second = NULL;
  qmc2QtKeyMap["Left"] = Qt::Key_Left;
  qmc2ShortcutMap["Home"].second = NULL;
  qmc2QtKeyMap["Home"] = Qt::Key_Home;
  qmc2ShortcutMap["PgDown"].second = NULL;
  qmc2QtKeyMap["PgDown"] = Qt::Key_PageDown;
  qmc2ShortcutMap["PgUp"].second = NULL;
  qmc2QtKeyMap["PgUp"] = Qt::Key_PageUp;
  qmc2ShortcutMap["Return"].second = NULL;
  qmc2QtKeyMap["Return"] = Qt::Key_Return;
  qmc2ShortcutMap["Right"].second = NULL;
  qmc2QtKeyMap["Right"] = Qt::Key_Right;
  qmc2ShortcutMap["Tab"].second = NULL;
  qmc2QtKeyMap["Tab"] = Qt::Key_Tab;
  qmc2ShortcutMap["Up"].second = NULL;
  qmc2QtKeyMap["Up"] = Qt::Key_Up;

  qmc2Options->setupShortcutActions();
}

#if defined(QMC2_MINGW)
#undef main
#endif

int main(int argc, char *argv[])
{
  qsrand(QDateTime::currentDateTime().toTime_t());

  // install message handler
  qInstallMsgHandler(myQtMessageHandler);

  // prepare application and resources
  QApplication qmc2App(argc, argv);
  Q_INIT_RESOURCE(qmc2);
#if defined(QMC2_EMUTYPE_MESS)
  qmc2App.setWindowIcon(QIcon(QString::fromUtf8(":/data/img/mess.png")));
  QPixmap splashPixmap(QString::fromUtf8(":/data/img/qmc2_mess_splash.png"));
#elif defined(QMC2_EMUTYPE_MAME)
  qmc2App.setWindowIcon(QIcon(QString::fromUtf8(":/data/img/mame.png")));
  QPixmap splashPixmap(QString::fromUtf8(":/data/img/qmc2_mame_splash.png"));
#endif

  // check configuration
  int checkReturn = QDialog::Accepted;
  qmc2Welcome = new Welcome(0);
  if ( !qmc2Welcome->checkOkay )
    checkReturn = qmc2Welcome->exec();
  bool showSplashScreen = qmc2Welcome->startupConfig->value(QMC2_FRONTEND_PREFIX + "GUI/ShowSplashScreen", true).toBool();
  QFont splashFont = qApp->font();
  if ( !qmc2Welcome->startupConfig->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString().isEmpty() )
    splashFont.fromString(qmc2Welcome->startupConfig->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
  delete qmc2Welcome;
  if ( checkReturn != QDialog::Accepted )
    return 1;

  // setup splash screen
  if ( showSplashScreen ) {
	qmc2SplashScreen = new QSplashScreen(splashPixmap);
	splashFont.setBold(true);
	qmc2SplashScreen->setFont(splashFont);
	qmc2SplashScreen->setAttribute(Qt::WA_ShowWithoutActivating);
	qmc2SplashScreen->setMask(splashPixmap.mask());
	qmc2SplashScreen->setWindowOpacity(0.8);
#if defined(Q_WS_X11)
	qmc2SplashScreen->showMessage(QObject::tr("Setting up the GUI, please wait...") + "\n", Qt::AlignHCenter | Qt::AlignBottom, Qt::white);
	qmc2SplashScreen->show();
	qmc2SplashScreen->raise();
	qmc2SplashScreen->repaint();
	qApp->processEvents();
#endif
  }

  // setup key event filter
  qmc2KeyPressFilter = new KeyPressFilter(0);
  qmc2App.installEventFilter(qmc2KeyPressFilter);

  // create mandatory objects and prepare shortcuts
  qmc2Options = new Options(0);
  qmc2Config = qmc2Options->config;
  qmc2ProcessManager = new ProcessManager(0);
  qmc2MainWindow = new MainWindow(0);

  // prepare & restore game/machine detail setup
  qmc2DetailSetup = new DetailSetup(qmc2MainWindow);
  qmc2DetailSetup->saveDetail();
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/RestoreLayout").toBool() ) {
#if defined(QMC2_EMUTYPE_MAME)
    qmc2MainWindow->tabWidgetGameDetail->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/GameDetailTab", 0).toInt());
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2MainWindow->tabWidgetGameDetail->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/MachineDetailTab", 0).toInt());
#endif
  }

  // finalize initial setup
  qmc2Options->apply();
  qmc2GuiReady = true;
  prepareShortcuts();
  QTimer::singleShot(0, qmc2Options, SLOT(on_pushButtonApply_clicked()));
  QTimer::singleShot(0, qmc2Options, SLOT(checkShortcuts()));
#if QMC2_JOYSTICK == 1
  QTimer::singleShot(0, qmc2Options, SLOT(checkJoystickMappings()));
#endif

  // this effectively enables support for unicode characters in C strings
  QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());

  // create & show greeting string
  QString greeting;
#if defined(QMC2_EMUTYPE_MESS)
  greeting = QObject::tr("M.E.S.S. Catalog / Launcher II v") +
#elif defined(QMC2_EMUTYPE_MAME)
  greeting = QObject::tr("M.A.M.E. Catalog / Launcher II v") +
#else
  greeting = QObject::tr("M.A.M.E. Catalog / Launcher II v") +
#endif
             QString(XSTR(QMC2_VERSION)) +
#if QMC2_SVN_REV > 0
             ", " + QObject::tr("SVN r%1").arg(QMC2_SVN_REV) +
#endif
             " (Qt " + qVersion() +
#if defined(QMC2_SDLMAME)
             ", SDLMAME, " +
#elif defined(QMC2_SDLMESS)
             ", SDLMESS, " +
#elif defined(QMC2_MAME)
             ", MAME, " +
#elif defined(QMC2_MESS)
             ", MESS, " +
#else
             ", ???, " +
#endif
             qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Language").toString() + ")";
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, greeting);

#if QMC2_OPENGL == 1
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QObject::tr("OpenGL features enabled"));
#endif

#if QMC2_USE_PHONON_API
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QObject::tr("Phonon features enabled - using Phonon v%1").arg(Phonon::phononVersion()));
#endif

#if QMC2_JOYSTICK == 1
  const SDL_version *sdlVersion = SDL_Linked_Version();
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QObject::tr("SDL joystick support enabled - using SDL v%1.%2.%3").arg(sdlVersion->major).arg(sdlVersion->minor).arg(sdlVersion->patch));
#endif

  // process global emulator configuration and create import/export popup menus
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QObject::tr("processing global emulator configuration"));
  int left, top, right, bottom;
  qmc2MainWindow->gridLayout->getContentsMargins(&left, &top, &right, &bottom);
  QVBoxLayout *layout = new QVBoxLayout;
#if defined(QMC2_EMUTYPE_MAME)
  qmc2GlobalEmulatorOptions = new EmulatorOptions("MAME/Configuration/Global", qmc2Options->tabGlobalConfiguration);
#elif defined(QMC2_EMUTYPE_MESS)
  qmc2GlobalEmulatorOptions = new EmulatorOptions("MESS/Configuration/Global", qmc2Options->tabGlobalConfiguration);
#endif
  qmc2GlobalEmulatorOptions->load();
#if defined(QMC2_SDLMAME) || defined(QMC2_SDLMESS) || defined(QMC2_MAME) || defined(QMC2_MESS)
  layout->addWidget(qmc2GlobalEmulatorOptions);
  layout->setContentsMargins(left, top, right, bottom);
  QHBoxLayout *buttonLayout = new QHBoxLayout();
  qmc2MainWindow->pushButtonGlobalEmulatorOptionsExportToFile = new QPushButton(QObject::tr("Export to..."), qmc2Options);
#if defined(QMC2_EMUTYPE_MAME)
  qmc2MainWindow->pushButtonGlobalEmulatorOptionsExportToFile->setToolTip(QObject::tr("Export global MAME configuration"));
  qmc2MainWindow->pushButtonGlobalEmulatorOptionsExportToFile->setToolTip(QObject::tr("Export global MAME configuration"));
#elif defined(QMC2_EMUTYPE_MESS)
  qmc2MainWindow->pushButtonGlobalEmulatorOptionsExportToFile->setToolTip(QObject::tr("Export global MESS configuration"));
  qmc2MainWindow->pushButtonGlobalEmulatorOptionsExportToFile->setToolTip(QObject::tr("Export global MESS configuration"));
#endif
  qmc2MainWindow->pushButtonGlobalEmulatorOptionsImportFromFile = new QPushButton(QObject::tr("Import from..."), qmc2Options);
#if defined(QMC2_EMUTYPE_MAME)
  qmc2MainWindow->pushButtonGlobalEmulatorOptionsImportFromFile->setToolTip(QObject::tr("Import global MAME configuration"));
  qmc2MainWindow->pushButtonGlobalEmulatorOptionsImportFromFile->setStatusTip(QObject::tr("Import global MAME configuration"));
#elif defined(QMC2_EMUTYPE_MESS)
  qmc2MainWindow->pushButtonGlobalEmulatorOptionsImportFromFile->setToolTip(QObject::tr("Import global MESS configuration"));
  qmc2MainWindow->pushButtonGlobalEmulatorOptionsImportFromFile->setStatusTip(QObject::tr("Import global MESS configuration"));
#endif
  buttonLayout->addWidget(qmc2MainWindow->pushButtonGlobalEmulatorOptionsExportToFile);
  buttonLayout->addWidget(qmc2MainWindow->pushButtonGlobalEmulatorOptionsImportFromFile);
  layout->addLayout(buttonLayout);
  qmc2Options->tabGlobalConfiguration->setLayout(layout);
  qmc2GlobalEmulatorOptions->show();
  qmc2MainWindow->pushButtonGlobalEmulatorOptionsExportToFile->show();
  qmc2MainWindow->pushButtonGlobalEmulatorOptionsImportFromFile->show();
  // export/import menus
  qmc2MainWindow->selectMenuGlobalEmulatorOptionsExportToFile = new QMenu(qmc2MainWindow->pushButtonGlobalEmulatorOptionsExportToFile);
#if defined(QMC2_EMUTYPE_MAME)
  QObject::connect(qmc2MainWindow->selectMenuGlobalEmulatorOptionsExportToFile->addAction(QIcon(QString::fromUtf8(":/data/img/work.png")), QObject::tr("<inipath>/mame.ini")), SIGNAL(triggered()), qmc2MainWindow, SLOT(on_pushButtonGlobalEmulatorOptionsExportToFile_clicked()));
#elif defined(QMC2_EMUTYPE_MESS)
  QObject::connect(qmc2MainWindow->selectMenuGlobalEmulatorOptionsExportToFile->addAction(QIcon(QString::fromUtf8(":/data/img/work.png")), QObject::tr("<inipath>/mess.ini")), SIGNAL(triggered()), qmc2MainWindow, SLOT(on_pushButtonGlobalEmulatorOptionsExportToFile_clicked()));
#endif
  QObject::connect(qmc2MainWindow->selectMenuGlobalEmulatorOptionsExportToFile->addAction(QIcon(QString::fromUtf8(":/data/img/fileopen.png")), QObject::tr("Select file...")), SIGNAL(triggered()), qmc2MainWindow, SLOT(on_pushButtonGlobalEmulatorOptionsSelectExportFile_clicked()));
  qmc2MainWindow->pushButtonGlobalEmulatorOptionsExportToFile->setMenu(qmc2MainWindow->selectMenuGlobalEmulatorOptionsExportToFile);
  qmc2MainWindow->selectMenuGlobalEmulatorOptionsImportFromFile = new QMenu(qmc2MainWindow->pushButtonGlobalEmulatorOptionsImportFromFile);
#if defined(QMC2_EMUTYPE_MAME)
  QObject::connect(qmc2MainWindow->selectMenuGlobalEmulatorOptionsImportFromFile->addAction(QIcon(QString::fromUtf8(":/data/img/work.png")), QObject::tr("<inipath>/mame.ini")), SIGNAL(triggered()), qmc2MainWindow, SLOT(on_pushButtonGlobalEmulatorOptionsImportFromFile_clicked()));
#elif defined(QMC2_EMUTYPE_MESS)
  QObject::connect(qmc2MainWindow->selectMenuGlobalEmulatorOptionsImportFromFile->addAction(QIcon(QString::fromUtf8(":/data/img/work.png")), QObject::tr("<inipath>/mess.ini")), SIGNAL(triggered()), qmc2MainWindow, SLOT(on_pushButtonGlobalEmulatorOptionsImportFromFile_clicked()));
#endif
  QObject::connect(qmc2MainWindow->selectMenuGlobalEmulatorOptionsImportFromFile->addAction(QIcon(QString::fromUtf8(":/data/img/fileopen.png")), QObject::tr("Select file...")), SIGNAL(triggered()), qmc2MainWindow, SLOT(on_pushButtonGlobalEmulatorOptionsSelectImportFile_clicked()));
  qmc2MainWindow->pushButtonGlobalEmulatorOptionsImportFromFile->setMenu(qmc2MainWindow->selectMenuGlobalEmulatorOptionsImportFromFile);
#endif
  qmc2GlobalEmulatorOptions->pseudoConstructor();

  // finally run the application
  int retCode = qmc2App.exec();

  if ( qmc2SplashScreen )
  	qmc2SplashScreen->deleteLater();

  // wait for all application threads to finish
  QThreadPool::globalInstance()->waitForDone();

  return retCode;
}
