#include <QTextStream>
#include <QHeaderView>
#include <QTreeWidgetItem>
#include <QStringList>
#include <QFile>
#include <QFontMetrics>
#include <QFont>
#include <QTimer>
#include <QMap>
#include <QSet>
#include <QDir>
#include <QBitArray>
#include <QByteArray>
#include <QWebView>

#include "gamelist.h"
#include "emuopt.h"
#include "qmc2main.h"
#include "options.h"
#include "preview.h"
#include "flyer.h"
#include "cabinet.h"
#include "controller.h"
#include "marquee.h"
#include "title.h"
#include "pcb.h"
#include "romstatusexport.h"
#include "miniwebbrowser.h"
#include "romalyzer.h"
#include "macros.h"
#include "unzip.h"
#if defined(QMC2_EMUTYPE_MESS)
#include "messdevcfg.h"
#endif
#include "softwarelist.h"
#if defined(QMC2_YOUTUBE_ENABLED)
#include "youtubevideoplayer.h"
#endif

// external global variables
extern MainWindow *qmc2MainWindow;
extern Options *qmc2Options;
extern QSettings *qmc2Config;
extern EmulatorOptions *qmc2EmulatorOptions;
extern ROMStatusExporter *qmc2ROMStatusExporter;
extern ROMAlyzer *qmc2ROMAlyzer;
extern bool qmc2ReloadActive;
extern bool qmc2EarlyReloadActive;
extern bool qmc2StopParser;
extern bool qmc2StartingUp;
extern bool qmc2VerifyActive;
extern bool qmc2FilterActive;
extern bool qmc2UseIconFile;
extern bool qmc2IconsPreloaded;
extern bool qmc2WidgetsEnabled;
extern bool qmc2StatesTogglesEnabled;
extern int qmc2GamelistResponsiveness;
extern Preview *qmc2Preview;
extern Flyer *qmc2Flyer;
extern Cabinet *qmc2Cabinet;
extern Controller *qmc2Controller;
extern Marquee *qmc2Marquee;
extern Title *qmc2Title;
extern PCB *qmc2PCB;
extern QTreeWidgetItem *qmc2CurrentItem;
extern QTreeWidgetItem *qmc2LastGameInfoItem;
extern QTreeWidgetItem *qmc2LastEmuInfoItem;
extern QTreeWidgetItem *qmc2LastSoftwareListItem;
#if defined(QMC2_EMUTYPE_MESS)
extern QTreeWidgetItem *qmc2LastDeviceConfigItem;
extern MESSDeviceConfigurator *qmc2MESSDeviceConfigurator;
extern QMap<QString, QString> messXmlDataCache;
extern QMap<QString, QMap<QString, QStringList> > messSystemSlotMap;
extern QMap<QString, QString> messSlotNameMap;
extern bool messSystemSlotsSupported;
#endif
extern SoftwareList *qmc2SoftwareList;
extern QMap<QString, QStringList> systemSoftwareListMap;
extern QMap<QString, QStringList> systemSoftwareFilterMap;
extern QMap<QString, QString> softwareListXmlDataCache;
extern QString swlBuffer;
extern bool swlSupported;
extern QMap<QString, QTreeWidgetItem *> qmc2GamelistItemMap;
extern QMap<QString, QTreeWidgetItem *> qmc2HierarchyItemMap;
extern QMap<QString, QTreeWidgetItem *> qmc2GamelistItemByDescriptionMap;
extern QMap<QString, QString> qmc2GamelistNameMap;
extern QMap<QString, QString> qmc2GamelistDescriptionMap;
extern QMap<QString, QString> qmc2GamelistStatusMap;
extern QMap<QString, QStringList> qmc2HierarchyMap;
extern QMap<QString, QString> qmc2ParentMap;
extern int qmc2SortCriteria;
extern Qt::SortOrder qmc2SortOrder;
extern QBitArray qmc2Filter;
extern unzFile qmc2IconFile;
extern QMap<QString, QIcon> qmc2IconMap;
extern QStringList qmc2BiosROMs;
extern QStringList qmc2DeviceROMs;
extern QMap<QString, QByteArray *> qmc2EmuInfoDB;
#if defined(QMC2_EMUTYPE_MAME)
extern QTreeWidgetItem *qmc2LastMAWSItem;
extern MiniWebBrowser *qmc2MAWSLookup;
extern QMap<QString, QString> qmc2CategoryMap;
extern QMap<QString, QString> qmc2VersionMap;
extern QMap<QString, QTreeWidgetItem *> qmc2CategoryItemMap;
extern QMap<QString, QTreeWidgetItem *> qmc2VersionItemMap;
#endif
#if defined(QMC2_YOUTUBE_ENABLED)
extern YouTubeVideoPlayer *qmc2YouTubeWidget;
extern QTreeWidgetItem *qmc2LastYouTubeItem;
#endif
extern QMap<QString, int> qmc2XmlGamePositionMap;

// local global variables
QStringList Gamelist::phraseTranslatorList;
int numVerifyRoms = 0;
QString verifyLastLine;
QStringList verifiedList;
QMap<QString, int> xmlGamePositionMap;

Gamelist::Gamelist(QObject *parent)
  : QObject(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::Gamelist()");
#endif

  numGames = numTotalGames = numCorrectGames = numMostlyCorrectGames = numIncorrectGames = numUnknownGames = numNotFoundGames = numSearchGames = numDevices = -1;
  cachedGamesCounter = numTaggedSets = 0;
  loadProc = verifyProc = NULL;
  emulatorVersion = tr("unknown");
  autoRomCheck = false;

  QString imgDir = qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/DataDirectory", "data/").toString() + "img/";
  qmc2UnknownImageIcon.addFile(imgDir + "sphere_blue.png");
  qmc2UnknownBIOSImageIcon.addFile(imgDir + "sphere_blue_bios.png");
  qmc2UnknownDeviceImageIcon.addFile(imgDir + "sphere_blue_device.png");
  qmc2CorrectImageIcon.addFile(imgDir + "sphere_green.png");
  qmc2CorrectBIOSImageIcon.addFile(imgDir + "sphere_green_bios.png");
  qmc2CorrectDeviceImageIcon.addFile(imgDir + "sphere_green_device.png");
  qmc2MostlyCorrectImageIcon.addFile(imgDir + "sphere_yellowgreen.png");
  qmc2MostlyCorrectBIOSImageIcon.addFile(imgDir + "sphere_yellowgreen_bios.png");
  qmc2MostlyCorrectDeviceImageIcon.addFile(imgDir + "sphere_yellowgreen_device.png");
  qmc2IncorrectImageIcon.addFile(imgDir + "sphere_red.png");
  qmc2IncorrectBIOSImageIcon.addFile(imgDir + "sphere_red_bios.png");
  qmc2IncorrectDeviceImageIcon.addFile(imgDir + "sphere_red_device.png");
  qmc2NotFoundImageIcon.addFile(imgDir + "sphere_grey.png");
  qmc2NotFoundBIOSImageIcon.addFile(imgDir + "sphere_grey_bios.png");
  qmc2NotFoundDeviceImageIcon.addFile(imgDir + "sphere_grey_device.png");

  if ( phraseTranslatorList.isEmpty() )
    phraseTranslatorList << tr("good") << tr("bad") << tr("preliminary") << tr("supported") << tr("unsupported")
                         << tr("imperfect") << tr("yes") << tr("no") << tr("baddump") << tr("nodump")
                         << tr("vertical") << tr("horizontal") << tr("raster") << tr("unknown") << tr("Unknown") 
                         << tr("On") << tr("Off") << tr("audio") << tr("unused") << tr("Unused") << tr("cpu")
                         << tr("vector") << tr("lcd") << tr("joy4way") << tr("joy8way") << tr("trackball")
                         << tr("joy2way") << tr("doublejoy8way") << tr("dial") << tr("paddle") << tr("pedal")
                         << tr("stick") << tr("vjoy2way") << tr("lightgun") << tr("doublejoy4way") << tr("vdoublejoy2way")
                         << tr("doublejoy2way") << tr("printer") << tr("cdrom") << tr("cartridge") << tr("cassette")
                         << tr("quickload") << tr("floppydisk") << tr("serial") << tr("snapshot") << tr("original")
			 << tr("compatible");

  if ( qmc2UseIconFile ) {
#if defined(QMC2_EMUTYPE_MAME)
    qmc2IconFile = unzOpen((const char *)qmc2Config->value("MAME/FilesAndDirectories/IconFile").toString().toAscii());
    if ( qmc2IconFile == NULL )
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open icon file, please check access permissions for %1").arg(qmc2Config->value("MAME/FilesAndDirectories/IconFile").toString()));
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2IconFile = unzOpen((const char *)qmc2Config->value("MESS/FilesAndDirectories/IconFile").toString().toAscii());
    if ( qmc2IconFile == NULL )
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open icon file, please check access permissions for %1").arg(qmc2Config->value("MESS/FilesAndDirectories/IconFile").toString()));
#endif
  }
}

Gamelist::~Gamelist()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::~Gamelist()");
#endif

  if ( loadProc )
    loadProc->terminate();

  if ( verifyProc )
    verifyProc->terminate();
}

void Gamelist::enableWidgets(bool enable)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::enableWidgets(bool enable = " + QString(enable ? "true" : "false") + ")");
#endif

  // store widget enablement flag for later dialog setups
  qmc2WidgetsEnabled = enable;

  qmc2Options->toolButtonBrowseStyleSheet->setEnabled(enable);
  qmc2Options->toolButtonBrowseFont->setEnabled(enable);
  qmc2Options->toolButtonBrowseLogFont->setEnabled(enable);
  qmc2Options->toolButtonBrowseTemporaryFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseFrontendLogFile->setEnabled(enable);
  qmc2Options->toolButtonBrowsePreviewDirectory->setEnabled(enable);
  qmc2Options->toolButtonBrowsePreviewFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseDataDirectory->setEnabled(enable);
  qmc2Options->toolButtonBrowseGameInfoDB->setEnabled(enable);
  qmc2Options->toolButtonCompressGameInfoDB->setEnabled(enable);
  qmc2Options->checkBoxProcessGameInfoDB->setEnabled(enable);
  qmc2Options->toolButtonBrowseEmuInfoDB->setEnabled(enable);
  qmc2Options->toolButtonCompressEmuInfoDB->setEnabled(enable);
  qmc2Options->checkBoxProcessEmuInfoDB->setEnabled(enable);
#if defined(QMC2_EMUTYPE_MAME)
  qmc2Options->toolButtonBrowseMAWSCacheDirectory->setEnabled(enable);
  qmc2Options->toolButtonBrowseCatverIniFile->setEnabled(enable);
  qmc2Options->checkBoxUseCatverIni->setEnabled(enable);
#endif
  qmc2Options->checkBoxShowROMStatusIcons->setEnabled(enable);
  qmc2Options->checkBoxRomStateFilter->setEnabled(enable);
  qmc2Options->checkBoxShowBiosSets->setEnabled(enable);
  qmc2Options->checkBoxShowDeviceSets->setEnabled(enable);
  qmc2Options->toolButtonBrowseSoftwareListCache->setEnabled(enable);
#if defined(QMC2_EMUTYPE_MESS)
  qmc2Options->toolButtonBrowseGeneralSoftwareFolder->setEnabled(enable);
#endif
  qmc2Options->toolButtonBrowseExecutableFile->setEnabled(enable);
#if defined(QMC2_VARIANT_LAUNCHER) && defined(Q_WS_WIN)
  qmc2Options->toolButtonBrowseMESSVariantExe->setEnabled(enable);
  qmc2Options->toolButtonBrowseMESSVariantExe->setEnabled(enable);
#endif
  qmc2Options->lineEditExecutableFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseWorkingDirectory->setEnabled(enable);
  qmc2Options->toolButtonBrowseEmulatorLogFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseOptionsTemplateFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseListXMLCache->setEnabled(enable);
  qmc2Options->toolButtonBrowseFavoritesFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseHistoryFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseGamelistCacheFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseROMStateCacheFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseFlyerDirectory->setEnabled(enable);
  qmc2Options->toolButtonBrowseFlyerFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseIconDirectory->setEnabled(enable);
  qmc2Options->toolButtonBrowseIconFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseCabinetDirectory->setEnabled(enable);
  qmc2Options->toolButtonBrowseCabinetFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseControllerDirectory->setEnabled(enable);
  qmc2Options->toolButtonBrowseControllerFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseMarqueeDirectory->setEnabled(enable);
  qmc2Options->toolButtonBrowseMarqueeFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseTitleDirectory->setEnabled(enable);
  qmc2Options->toolButtonBrowseTitleFile->setEnabled(enable);
  qmc2Options->toolButtonBrowsePCBDirectory->setEnabled(enable);
  qmc2Options->toolButtonBrowsePCBFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseSoftwareSnapDirectory->setEnabled(enable);
  qmc2Options->toolButtonBrowseSoftwareSnapFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseSoftwareNotesFolder->setEnabled(enable);
  qmc2Options->toolButtonBrowseSoftwareNotesTemplate->setEnabled(enable);
  qmc2Options->toolButtonShowC->setEnabled(enable);
  qmc2Options->toolButtonShowM->setEnabled(enable);
  qmc2Options->toolButtonShowI->setEnabled(enable);
  qmc2Options->toolButtonShowN->setEnabled(enable);
  qmc2Options->toolButtonShowU->setEnabled(enable);
  qmc2Options->comboBoxSortCriteria->setEnabled(enable);
  qmc2Options->comboBoxSortOrder->setEnabled(enable);
  qmc2Options->treeWidgetShortcuts->clearSelection();
  qmc2Options->treeWidgetShortcuts->setEnabled(enable);
  qmc2Options->treeWidgetJoystickMappings->clearSelection();
  qmc2Options->treeWidgetJoystickMappings->setEnabled(enable);
  qmc2Options->toolButtonBrowseZipTool->setEnabled(enable);
  qmc2Options->toolButtonBrowseFileRemovalTool->setEnabled(enable);
  qmc2Options->toolButtonBrowseRomTool->setEnabled(enable);
  qmc2Options->toolButtonBrowseRomToolWorkingDirectory->setEnabled(enable);
  qmc2Options->toolButtonBrowseAdditionalEmulatorExecutable->setEnabled(enable);
  qmc2Options->toolButtonBrowseAdditionalEmulatorWorkingDirectory->setEnabled(enable);
  for (int row = 0; row < qmc2Options->tableWidgetRegisteredEmulators->rowCount(); row++) {
	  QWidget *w = qmc2Options->tableWidgetRegisteredEmulators->cellWidget(row, QMC2_ADDTLEMUS_COLUMN_CUID);
	  if ( w ) w->setEnabled(enable);
  }
#if QMC2_USE_PHONON_API
  qmc2MainWindow->toolButtonAudioAddTracks->setEnabled(enable);
  qmc2MainWindow->toolButtonAudioAddURL->setEnabled(enable);
#endif
  if ( qmc2ROMStatusExporter )
    qmc2ROMStatusExporter->pushButtonExport->setEnabled(enable);
  if ( qmc2ROMAlyzer ) {
    if ( qmc2ROMAlyzer->groupBoxCHDManager->isChecked() ) {
      qmc2ROMAlyzer->toolButtonBrowseCHDManagerExecutableFile->setEnabled(enable);
      qmc2ROMAlyzer->toolButtonBrowseTemporaryWorkingDirectory->setEnabled(enable);
    }
    if ( qmc2ROMAlyzer->groupBoxSetRewriter->isChecked() )
      qmc2ROMAlyzer->toolButtonBrowseSetRewriterOutputPath->setEnabled(enable);
      qmc2ROMAlyzer->toolButtonBrowseSetRewriterAdditionalRomPath->setEnabled(enable);
#if defined(QMC2_DATABASE_ENABLED)
    if ( qmc2ROMAlyzer->groupBoxDatabase->isChecked() )
      qmc2ROMAlyzer->toolButtonBrowseDatabaseOutputPath->setEnabled(enable);
#endif
  }
  qmc2MainWindow->pushButtonSelectRomFilter->setEnabled(enable);
}

void Gamelist::load()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::load()");
#endif

  QString userScopePath = QMC2_DYNAMIC_DOT_PATH;

  qmc2ReloadActive = qmc2EarlyReloadActive = true;
  qmc2StopParser = false;
  qmc2GamelistItemMap.clear();
  qmc2GamelistNameMap.clear();
  qmc2GamelistItemByDescriptionMap.clear();
  qmc2GamelistDescriptionMap.clear();
  qmc2GamelistStatusMap.clear();
  qmc2BiosROMs.clear();
  qmc2DeviceROMs.clear();
  qmc2HierarchyItemMap.clear();

  enableWidgets(false);

  qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_DEFAULT_PAGE);

  numGames = numTotalGames = numCorrectGames = numMostlyCorrectGames = numIncorrectGames = numUnknownGames = numNotFoundGames = numSearchGames = numDevices = -1;
  numTaggedSets = 0;
  qmc2MainWindow->treeWidgetGamelist->clear();
  qmc2MainWindow->treeWidgetHierarchy->clear();
#if defined(QMC2_EMUTYPE_MAME)
  qmc2CategoryItemMap.clear();
  qmc2VersionItemMap.clear();
  qmc2MainWindow->treeWidgetCategoryView->clear();
  qmc2MainWindow->treeWidgetVersionView->clear();
#endif
  qmc2MainWindow->listWidgetSearch->clear();
  qmc2MainWindow->textBrowserGameInfo->clear();
  qmc2MainWindow->textBrowserEmuInfo->clear();
  qmc2MainWindow->labelGameStatus->setPalette(MainWindow::qmc2StatusColorBlue);
  qmc2CurrentItem = NULL;
#if defined(QMC2_EMUTYPE_MESS)
  if ( qmc2MESSDeviceConfigurator ) {
    qmc2MESSDeviceConfigurator->save();
    qmc2MESSDeviceConfigurator->setVisible(false);
    QLayout *vbl = qmc2MainWindow->tabDevices->layout();
    if ( vbl ) delete vbl;
    delete qmc2MESSDeviceConfigurator;
    qmc2MESSDeviceConfigurator = NULL;
  }
  qmc2LastDeviceConfigItem = NULL;
  messXmlDataCache.clear();
  messSystemSlotsSupported = true;
  messSystemSlotMap.clear();
  messSlotNameMap.clear();
#endif
  if ( qmc2SoftwareList ) {
    if ( qmc2SoftwareList->isLoading ) {
      qmc2SoftwareList->interruptLoad = true;
      qmc2LastSoftwareListItem = NULL;
      QTimer::singleShot(0, this, SLOT(load()));
      return;
    }
    qmc2SoftwareList->save();
    qmc2SoftwareList->setVisible(false);
    QLayout *vbl = qmc2MainWindow->tabSoftwareList->layout();
    if ( vbl ) delete vbl;
    delete qmc2SoftwareList;
    qmc2SoftwareList = NULL;
  }
  qmc2LastSoftwareListItem = NULL;
  swlSupported = true;
  systemSoftwareListMap.clear();
  systemSoftwareFilterMap.clear();
  softwareListXmlDataCache.clear();
  swlBuffer.clear();
  qmc2LastGameInfoItem = NULL;
  qmc2LastEmuInfoItem = NULL;
#if defined(QMC2_EMUTYPE_MAME)
  if ( qmc2MAWSLookup ) {
    qmc2MAWSLookup->setVisible(false);
    QLayout *vbl = qmc2MainWindow->tabMAWS->layout();
    if ( vbl ) delete vbl;
    delete qmc2MAWSLookup;
    qmc2MAWSLookup = NULL;
  }
  qmc2LastMAWSItem = NULL;
#endif

#if defined(QMC2_YOUTUBE_ENABLED)
  qmc2LastYouTubeItem = NULL;
  if ( qmc2YouTubeWidget ) {
      qmc2YouTubeWidget->setVisible(false);
      QLayout *vbl = qmc2MainWindow->tabYouTube->layout();
      if ( vbl ) delete vbl;
      delete qmc2YouTubeWidget;
      qmc2YouTubeWidget = NULL;
  }
#endif

  qmc2Preview->update();
  qmc2Flyer->update();
  qmc2Cabinet->update();
  qmc2Controller->update();
  qmc2Marquee->update();
  qmc2Title->update();
  qmc2PCB->update();

  qApp->processEvents();
  QTreeWidgetItem *dummyItem = new QTreeWidgetItem(qmc2MainWindow->treeWidgetGamelist);
  dummyItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Waiting for data..."));
  dummyItem = new QTreeWidgetItem(qmc2MainWindow->treeWidgetHierarchy);
  dummyItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Waiting for data..."));
#if defined(QMC2_EMUTYPE_MAME)
  dummyItem = new QTreeWidgetItem(qmc2MainWindow->treeWidgetCategoryView);
  dummyItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Waiting for data..."));
  dummyItem = new QTreeWidgetItem(qmc2MainWindow->treeWidgetVersionView);
  dummyItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Waiting for data..."));
#endif
  if ( qmc2EmulatorOptions ) {
    qmc2EmulatorOptions->save();
    QLayout *vbl = qmc2MainWindow->tabConfiguration->layout();
    if ( vbl ) delete vbl;
    delete qmc2MainWindow->labelEmuSelector;
    delete qmc2MainWindow->comboBoxEmuSelector;
    delete qmc2EmulatorOptions;
    delete qmc2MainWindow->pushButtonCurrentEmulatorOptionsExportToFile;
    delete qmc2MainWindow->pushButtonCurrentEmulatorOptionsImportFromFile;
    qmc2EmulatorOptions = NULL;
  }
  qmc2MainWindow->labelGamelistStatus->setText(status());

#if defined(QMC2_EMUTYPE_MAME)
  switch ( qmc2MainWindow->stackedWidgetView->currentIndex() ) {
    case QMC2_VIEW_CATEGORY_INDEX:
      QTimer::singleShot(0, qmc2MainWindow, SLOT(viewByCategory()));
      break;
    case QMC2_VIEW_VERSION_INDEX:
      QTimer::singleShot(0, qmc2MainWindow, SLOT(viewByVersion()));
      break;
  }
#endif

  // determine emulator version and supported sets
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("determining emulator version and supported sets"));

  QStringList args;
  QTime elapsedTime;
  parseTimer.start();
  QString command;

  // emulator version
  QProcess commandProc;
#if defined(QMC2_SDLMAME)
  commandProc.setStandardOutputFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmame.tmp").toString());
#elif defined(QMC2_MAME)
  commandProc.setStandardOutputFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mame.tmp").toString());
#elif defined(QMC2_SDLMESS)
  commandProc.setStandardOutputFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmess.tmp").toString());
#elif defined(QMC2_MESS)
  commandProc.setStandardOutputFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mess.tmp").toString());
#else
  commandProc.setStandardOutputFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-unknown.tmp").toString());
#endif

#if !defined(Q_WS_WIN)
  commandProc.setStandardErrorFile("/dev/null");
#endif

#if defined(QMC2_SDLMAME) || defined(QMC2_SDLMESS) || defined(QMC2_MAME) || defined(QMC2_MESS)
  args << "-help";
#endif
  qApp->processEvents();
  bool commandProcStarted = false;
  int retries = 0;
#if defined(QMC2_EMUTYPE_MAME)
  commandProc.start(qmc2Config->value("MAME/FilesAndDirectories/ExecutableFile").toString(), args);
#elif defined(QMC2_EMUTYPE_MESS)
  commandProc.start(qmc2Config->value("MESS/FilesAndDirectories/ExecutableFile").toString(), args);
#endif
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
#if defined(QMC2_EMUTYPE_MAME)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't start MAME executable within a reasonable time frame, giving up"));
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't start MESS executable within a reasonable time frame, giving up"));
#endif
    qmc2ReloadActive = qmc2EarlyReloadActive = false;
    qmc2StopParser = true;
    return;
  }

#if defined(QMC2_SDLMAME)
  QFile qmc2TempVersion(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmame.tmp").toString());
#elif defined(QMC2_MAME)
  QFile qmc2TempVersion(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mame.tmp").toString());
#elif defined(QMC2_SDLMESS)
  QFile qmc2TempVersion(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmess.tmp").toString());
#elif defined(QMC2_MESS)
  QFile qmc2TempVersion(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mess.tmp").toString());
#else
  QFile qmc2TempVersion(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-unknown.tmp").toString());
#endif
  if ( commandProcStarted && qmc2TempVersion.open(QFile::ReadOnly) ) {
    QTextStream ts(&qmc2TempVersion);
    qApp->processEvents();
    QString s = ts.readAll();
    qApp->processEvents();
    qmc2TempVersion.close();
    qmc2TempVersion.remove();
#if defined(Q_WS_WIN)
    s.replace("\r\n", "\n"); // convert WinDOS's "0x0D 0x0A" to just "0x0A" 
#endif
    QStringList versionLines = s.split("\n");
#if defined(QMC2_EMUTYPE_MAME)
    QStringList versionWords = versionLines.first().split(" ");
    if ( versionWords.count() > 1 ) {
      if ( versionWords[0] == "M.A.M.E." ) {
        emulatorVersion = versionWords[1].remove("v");
        emulatorType = "MAME";
      } else {
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: selected executable file is not MAME"));
        emulatorVersion = tr("unknown");
        emulatorType = versionWords[0];
      }
    } else {
      emulatorVersion = tr("unknown");
      emulatorType = tr("unknown");
    }
#elif defined(QMC2_EMUTYPE_MESS)
    QStringList versionWords = versionLines.first().split(" ");
    if ( versionWords.count() > 1 ) {
      if ( versionWords[0] == "MESS" || versionWords[0] == "M.E.S.S.") {
        emulatorVersion = versionWords[1].remove("v");
        emulatorType = "MESS";
      } else {
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: selected executable file is not MESS"));
        emulatorVersion = tr("unknown");
        emulatorType = versionWords[0];
      }
    } else {
      emulatorVersion = tr("unknown");
      emulatorType = tr("unknown");
    }
#endif
  } else {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't create temporary file, please check emulator executable and permissions"));
    emulatorVersion = tr("unknown");
    emulatorType = tr("unknown");
  }

  // supported games/machines
  args.clear();
  args << "-listfull";
#if defined(QMC2_AUDIT_WILDCARD)
  args << "*";
#endif
  qApp->processEvents();
  commandProcStarted = false;
  retries = 0;
#if defined(QMC2_EMUTYPE_MAME)
  commandProc.start(qmc2Config->value("MAME/FilesAndDirectories/ExecutableFile").toString(), args);
#elif defined(QMC2_EMUTYPE_MESS)
  commandProc.start(qmc2Config->value("MESS/FilesAndDirectories/ExecutableFile").toString(), args);
#endif
  started = commandProc.waitForStarted(QMC2_PROCESS_POLL_TIME);
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
#if defined(QMC2_EMUTYPE_MAME)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't start MAME executable within a reasonable time frame, giving up"));
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't start MESS executable within a reasonable time frame, giving up"));
#endif
    qmc2ReloadActive = qmc2EarlyReloadActive = false;
    qmc2StopParser = true;
    return;
  }

#if defined(QMC2_SDLMAME)
  QFile qmc2Temp(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmame.tmp").toString());
#elif defined(QMC2_MAME)
  QFile qmc2Temp(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mame.tmp").toString());
#elif defined(QMC2_SDLMESS)
  QFile qmc2Temp(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmess.tmp").toString());
#elif defined(QMC2_MESS)
  QFile qmc2Temp(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mess.tmp").toString());
#else
  QFile qmc2Temp(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-unknown.tmp").toString());
#endif
  if ( commandProcStarted && qmc2Temp.open(QFile::ReadOnly) ) {
    QTextStream ts(&qmc2Temp);
    qApp->processEvents();
    QString s = ts.readAll();
    qApp->processEvents();
    qmc2Temp.close();
    qmc2Temp.remove();
    numTotalGames = s.split("\n").count() - 2;
    elapsedTime = elapsedTime.addMSecs(parseTimer.elapsed());
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (determining emulator version and supported sets, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
  } else {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't create temporary file, please check emulator executable and permissions"));
  }
  qmc2MainWindow->labelGamelistStatus->setText(status());

  if ( emulatorVersion != tr("unknown") )
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("emulator info: type = %1, version = %2").arg(emulatorType).arg(emulatorVersion));
  else {
    if ( emulatorType == tr("unknown") )
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: couldn't determine emulator type and version"));
    else
#if defined(QMC2_EMUTYPE_MAME)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: couldn't determine emulator version, type identification string is '%1' -- please inform developers if you're sure that this is a valid MAME binary"));
#elif defined(QMC2_EMUTYPE_MESS)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: couldn't determine emulator version, type identification string is '%1' -- please inform developers if you're sure that this is a valid MESS binary"));
#else
      ;
#endif
    qmc2ReloadActive = false;
    enableWidgets(true);
    return;
  }

  if ( numTotalGames > 0 )
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("%n supported set(s)", "", numTotalGames));
  else {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: couldn't determine the number of supported sets"));
    qmc2ReloadActive = false;
    enableWidgets(true);
    return;
  }

#if defined(QMC2_EMUTYPE_MAME)
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/UseCatverIni").toBool() )
    loadCatverIni();
#endif

  gamelistBuffer.clear();

  // try reading XML output from cache
  bool xmlCacheOkay = false;
#if defined(QMC2_EMUTYPE_MAME)
  listXMLCache.setFileName(qmc2Config->value("MAME/FilesAndDirectories/ListXMLCache", userScopePath + "/mame.lxc").toString());
#elif defined(QMC2_EMUTYPE_MESS)
  listXMLCache.setFileName(qmc2Config->value("MESS/FilesAndDirectories/ListXMLCache", userScopePath + "/mess.lxc").toString());
#endif
  listXMLCache.open(QIODevice::ReadOnly | QIODevice::Text);
  if ( listXMLCache.isOpen() ) {
    QTextStream ts(&listXMLCache);
    QString singleXMLLine = ts.readLine();
    singleXMLLine = ts.readLine();
#if defined(QMC2_EMUTYPE_MAME)
    if ( singleXMLLine.startsWith("MAME_VERSION") ) {
#elif defined(QMC2_EMUTYPE_MESS)
    if ( singleXMLLine.startsWith("MESS_VERSION") ) {
#endif
      QStringList words = singleXMLLine.split("\t");
      if ( words.count() > 1 ) { 
          if ( emulatorVersion == words[1] )
            xmlCacheOkay = true;
      }
    }
    if ( xmlCacheOkay ) {
      QTime xmlElapsedTime;
      parseTimer.start();
#if defined(QMC2_EMUTYPE_MAME)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading XML game list data from cache"));
#elif defined(QMC2_EMUTYPE_MESS)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading XML machine list data from cache"));
#endif
      if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
        qmc2MainWindow->progressBarGamelist->setFormat(tr("XML cache - %p%"));
      else
        qmc2MainWindow->progressBarGamelist->setFormat("%p%");
      qmc2MainWindow->progressBarGamelist->setRange(0, numTotalGames);
      qmc2MainWindow->progressBarGamelist->reset();
      int i = 0;
      int gameCount = 0;
      QString readBuffer;
      while ( !ts.atEnd() || !readBuffer.isEmpty() ) {
        readBuffer += ts.read(QMC2_FILE_BUFFER_SIZE);
        bool endsWithNewLine = readBuffer.endsWith("\n");
        QStringList lines = readBuffer.split("\n");
        int l, lc = lines.count();
        if ( !endsWithNewLine )
          lc -= 1;
        for (l = 0; l < lc; l++) {
          if ( !lines[l].isEmpty() ) {
            singleXMLLine = lines[l];
            gamelistBuffer += singleXMLLine + "\n";
#if defined(QMC2_EMUTYPE_MAME)
            gameCount += singleXMLLine.count("<game name=");
#elif defined(QMC2_EMUTYPE_MESS)
            gameCount += singleXMLLine.count("<machine name=");
#endif
          }
        }

        if ( endsWithNewLine )
          readBuffer.clear();
        else
          readBuffer = lines.last();

        if ( i++ % QMC2_XMLCACHE_RESPONSIVENESS == 0 )
          qmc2MainWindow->progressBarGamelist->setValue(gameCount);
      }
      gamelistBuffer += "\n";
      xmlElapsedTime = xmlElapsedTime.addMSecs(parseTimer.elapsed());
#if defined(QMC2_EMUTYPE_MAME)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading XML game list data from cache, elapsed time = %1)").arg(xmlElapsedTime.toString("mm:ss.zzz")));
      if ( singleXMLLine != "</mame>" ) {
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: XML game list cache is incomplete, invalidating XML game list cache"));
#elif defined(QMC2_EMUTYPE_MESS)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading XML machine list data from cache, elapsed time = %1)").arg(xmlElapsedTime.toString("mm:ss.zzz")));
      if ( singleXMLLine != "</mess>" ) {
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: XML machine list cache is incomplete, invalidating XML machine list cache"));
#endif
        xmlCacheOkay = false;
      } else
        qmc2EarlyReloadActive = false;
    }
  }

  if ( listXMLCache.isOpen() )
    listXMLCache.close();

  if ( qmc2StopParser ) {
    qmc2MainWindow->progressBarGamelist->reset();
    qmc2ReloadActive = false;
    enableWidgets(true);
    return;
  }

  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/HideWhileLoading", true).toBool() ) {
    // hide game list
    qmc2MainWindow->treeWidgetGamelist->setVisible(false);
    qmc2MainWindow->labelLoadingGamelist->setVisible(true);
    qmc2MainWindow->treeWidgetHierarchy->setVisible(false);
    qmc2MainWindow->labelLoadingHierarchy->setVisible(true);
    qApp->processEvents();
  }

  if ( xmlCacheOkay ) {
    parse();
    loadFavorites();
    loadPlayHistory();

    // show game list
    qmc2MainWindow->labelLoadingGamelist->setVisible(false);
    qmc2MainWindow->treeWidgetGamelist->setVisible(true);
    qmc2MainWindow->labelLoadingHierarchy->setVisible(false);
    qmc2MainWindow->treeWidgetHierarchy->setVisible(true);

    if ( qmc2MainWindow->tabWidgetGamelist->currentIndex() == QMC2_GAMELIST_INDEX ) {
      switch ( qmc2MainWindow->stackedWidgetView->currentIndex() ) {
	      case QMC2_VIEW_DETAIL_INDEX:
		      qmc2MainWindow->treeWidgetGamelist->setFocus();
		      break;
	      case QMC2_VIEW_TREE_INDEX:
		      qmc2MainWindow->treeWidgetHierarchy->setFocus();
		      break;
#if defined(QMC2_EMUTYPE_MAME)
	      case QMC2_VIEW_CATEGORY_INDEX:
		      qmc2MainWindow->treeWidgetCategoryView->setFocus();
		      break;
	      case QMC2_VIEW_VERSION_INDEX:
		      qmc2MainWindow->treeWidgetVersionView->setFocus();
		      break;
#endif
	      default:
		      qmc2MainWindow->treeWidgetGamelist->setFocus();
		      break;
      }
    }

    qApp->processEvents();
  } else {
    loadTimer.start();
#if defined(QMC2_EMUTYPE_MAME)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading XML game list data and (re)creating cache"));
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading XML machine list data and (re)creating cache"));
#endif
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
      qmc2MainWindow->progressBarGamelist->setFormat(tr("XML data - %p%"));
    else
      qmc2MainWindow->progressBarGamelist->setFormat("%p%");
#if defined(QMC2_EMUTYPE_MAME)
    command = qmc2Config->value("MAME/FilesAndDirectories/ExecutableFile").toString();
#elif defined(QMC2_EMUTYPE_MESS)
    command = qmc2Config->value("MESS/FilesAndDirectories/ExecutableFile").toString();
#endif
    args.clear();
    args << "-listxml";
#if defined(QMC2_AUDIT_WILDCARD)
    args << "*";
#endif

    listXMLCache.open(QIODevice::WriteOnly | QIODevice::Text);
    if ( !listXMLCache.isOpen() ) {
#if defined(QMC2_EMUTYPE_MAME)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: can't open XML game list cache for writing, please check permissions"));
#elif defined(QMC2_EMUTYPE_MESS)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: can't open XML machine list cache for writing, please check permissions"));
#endif
    } else {
      tsListXMLCache.setDevice(&listXMLCache);
      tsListXMLCache.reset();
      tsListXMLCache << "# THIS FILE IS AUTO-GENERATED - PLEASE DO NOT EDIT!\n";
#if defined(QMC2_EMUTYPE_MAME)
      tsListXMLCache << "MAME_VERSION\t" + emulatorVersion + "\n";
#elif defined(QMC2_EMUTYPE_MESS)
      tsListXMLCache << "MESS_VERSION\t" + emulatorVersion + "\n";
#endif
    }
    loadProc = new QProcess(this);
    connect(loadProc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(loadError(QProcess::ProcessError)));
    connect(loadProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(loadFinished(int, QProcess::ExitStatus)));
    connect(loadProc, SIGNAL(readyReadStandardOutput()), this, SLOT(loadReadyReadStandardOutput()));
    connect(loadProc, SIGNAL(started()), this, SLOT(loadStarted()));
    connect(loadProc, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(loadStateChanged(QProcess::ProcessState)));
    loadProc->setProcessChannelMode(QProcess::MergedChannels);
    loadProc->start(command, args);
  }
}

void Gamelist::verify(bool currentOnly)
{
  if ( currentOnly )
    if ( !qmc2CurrentItem )
      return;

#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Gamelist::verify(bool currentOnly = %1)").arg(currentOnly));
#endif

  verifyCurrentOnly = currentOnly;
  qmc2VerifyActive = true;
  qmc2StopParser = false;

  enableWidgets(false);

  verifiedList.clear();
  verifyLastLine.clear();
  verifyTimer.start();
  numVerifyRoms = 0;
  if ( verifyCurrentOnly ) {
    checkedItem = qmc2CurrentItem;
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("verifying ROM status for '%1'").arg(checkedItem->text(QMC2_GAMELIST_COLUMN_GAME)));
    // decrease counter for current game's/machine's state
    switch ( checkedItem->whatsThis(QMC2_GAMELIST_COLUMN_GAME).at(0).toAscii() ) {
      case QMC2_ROMSTATE_CHAR_C:
        numCorrectGames--;
        numUnknownGames++;
        break;

      case QMC2_ROMSTATE_CHAR_M:
        numMostlyCorrectGames--;
        numUnknownGames++;
        break;

      case QMC2_ROMSTATE_CHAR_I:
        numIncorrectGames--;
        numUnknownGames++;
        break;

      case QMC2_ROMSTATE_CHAR_N:
        numNotFoundGames--;
        numUnknownGames++;
        break;

      case QMC2_ROMSTATE_CHAR_U:
      default:
        break;
    }
#if defined(QMC2_EMUTYPE_MAME)
    romCache.setFileName(qmc2Config->value("MAME/FilesAndDirectories/ROMStateCacheFile").toString());
#elif defined(QMC2_EMUTYPE_MESS)
    romCache.setFileName(qmc2Config->value("MESS/FilesAndDirectories/ROMStateCacheFile").toString());
#endif
    romCache.open(QIODevice::WriteOnly | QIODevice::Text);
    if ( !romCache.isOpen() )
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ERROR: can't open ROM state cache for writing, path = %1").arg(romCache.fileName()));
    else {
      tsRomCache.setDevice(&romCache);
      tsRomCache.reset();
      tsRomCache << "# THIS FILE IS AUTO-GENERATED - PLEASE DO NOT EDIT!\n";
    }
  } else {
#if defined(QMC2_EMUTYPE_MAME)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("verifying ROM status for all games"));
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("verifying ROM status for all machines"));
#endif

    numCorrectGames = numMostlyCorrectGames = numIncorrectGames = numNotFoundGames = numUnknownGames = 0;
    qmc2MainWindow->labelGamelistStatus->setText(status());
#if defined(QMC2_EMUTYPE_MAME)
    romCache.setFileName(qmc2Config->value("MAME/FilesAndDirectories/ROMStateCacheFile").toString());
#elif defined(QMC2_EMUTYPE_MESS)
    romCache.setFileName(qmc2Config->value("MESS/FilesAndDirectories/ROMStateCacheFile").toString());
#endif
    romCache.open(QIODevice::WriteOnly | QIODevice::Text);
    if ( !romCache.isOpen() )
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ERROR: can't open ROM state cache for writing, path = %1").arg(romCache.fileName()));
    else {
      tsRomCache.setDevice(&romCache);
      tsRomCache.reset();
      tsRomCache << "# THIS FILE IS AUTO-GENERATED - PLEASE DO NOT EDIT!\n";
    }
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
      qmc2MainWindow->progressBarGamelist->setFormat(tr("ROM check - %p%"));
    else
      qmc2MainWindow->progressBarGamelist->setFormat("%p%");
    qmc2MainWindow->progressBarGamelist->setRange(0, numTotalGames + numDevices);
    qmc2MainWindow->progressBarGamelist->reset();
  }
  
  QStringList args;
#if defined(QMC2_EMUTYPE_MAME)
  QString command = qmc2Config->value("MAME/FilesAndDirectories/ExecutableFile").toString();
  if ( qmc2Config->contains("MAME/Configuration/Global/rompath") )
    args << "-rompath" << qmc2Config->value("MAME/Configuration/Global/rompath").toString().replace("~", "$HOME");
#elif defined(QMC2_EMUTYPE_MESS)
  QString command = qmc2Config->value("MESS/FilesAndDirectories/ExecutableFile").toString();
  if ( qmc2Config->contains("MESS/Configuration/Global/rompath") )
    args << "-rompath" << qmc2Config->value("MESS/Configuration/Global/rompath").toString().replace("~", "$HOME");
#endif
  args << "-verifyroms";
  if ( verifyCurrentOnly )
    args << checkedItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON);
#if defined(QMC2_AUDIT_WILDCARD)
  else
    args << "*";
#endif

  verifyProc = new QProcess(this);
  connect(verifyProc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(verifyError(QProcess::ProcessError)));
  connect(verifyProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(verifyFinished(int, QProcess::ExitStatus)));
  connect(verifyProc, SIGNAL(readyReadStandardOutput()), this, SLOT(verifyReadyReadStandardOutput()));
  connect(verifyProc, SIGNAL(started()), this, SLOT(verifyStarted()));
  connect(verifyProc, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(verifyStateChanged(QProcess::ProcessState)));
  verifyProc->setProcessChannelMode(QProcess::MergedChannels);
  verifyProc->start(command, args);
}

QString Gamelist::value(QString element, QString attribute, bool translate)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::value(QString element = " + element + ", QString attribute = \"" + attribute + "\", translate = " + QString(translate ? "true" : "false") + ")");
#endif

  QString attributePattern = " " + attribute + "=\"";
  if ( element.contains(attributePattern) ) {
    QString valueString = element.remove(0, element.indexOf(attributePattern) + attributePattern.length());
    valueString = valueString.remove(valueString.indexOf("\""), valueString.lastIndexOf(">")).replace("&amp;", "&").replace("&lt;", "<").replace("&gt;", ">").replace("&quot;", "\"");
    if ( translate )
      return tr(valueString.toAscii());
    else
      return valueString;
  } else
    return QString::null;
}

void Gamelist::insertAttributeItems(QTreeWidgetItem *parent, QString element, QStringList attributes, QStringList descriptions, bool translate)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Gamelist::insertAttributeItems(QTreeWidgetItem *parent = %1, QString element = %2, QStringList attributes = ..., QStringList descriptions = ..., translate = %3)").arg((qulonglong)parent).arg(element).arg(translate));
#endif

  int i;
  for (i = 0; i < attributes.count(); i++) {
    QString valueString = value(element, attributes.at(i), translate);
    if ( !valueString.isEmpty() ) {
      QTreeWidgetItem *attributeItem = new QTreeWidgetItem(parent);
      attributeItem->setText(QMC2_GAMELIST_COLUMN_GAME, descriptions.at(i));
      attributeItem->setText(QMC2_GAMELIST_COLUMN_ICON, tr(valueString.toAscii()));
    }
  }
}

void Gamelist::parseGameDetail(QTreeWidgetItem *item)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Gamelist::parseGameDetail(QTreeWidgetItem *item = %1)").arg((qulonglong)item));
#endif

  QString gameName = item->child(0)->text(QMC2_GAMELIST_COLUMN_ICON);
  int gamePos = xmlGamePositionMap[gameName];
  if ( gamePos <= 0 ) {
    QString gameDescription = item->text(QMC2_GAMELIST_COLUMN_GAME);
#if defined(QMC2_EMUTYPE_MAME)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("retrieving game information for '%1'").arg(gameDescription));
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("retrieving machine information for '%1'").arg(gameDescription));
#endif
    qApp->processEvents();
    gameDescription.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;").replace("\"", "&quot;");
    QString s = "<description>" + gameDescription + "</description>";
    gamePos = 0;
    int xmlLinesCount = xmlLines.count();
    while ( !xmlLines[gamePos].contains(s) ) {
      gamePos++;
      if ( gamePos > xmlLinesCount ) break;
    }
    if ( gamePos < xmlLinesCount && xmlLines[gamePos].contains(s) ) {
      xmlGamePositionMap[gameName] = gamePos;
    } else {
#if defined(QMC2_EMUTYPE_MAME)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: couldn't find game information for '%1'").arg(gameDescription));
#elif defined(QMC2_EMUTYPE_MESS)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: couldn't find machine information for '%1'").arg(gameDescription));
#endif
      return;
    }
  }
  qApp->processEvents();

  QTreeWidgetItem *childItem = item->takeChild(0);
  delete childItem;

  QString element, content;
  QStringList attributes, descriptions;

  // game/machine element
  attributes << "name" << "sourcefile" << "isbios" << "isdevice" << "runnable" << "cloneof" << "romof" << "sampleof";
  descriptions << tr("Name") << tr("Source file") << tr("Is BIOS?") << tr("Is device?") << tr("Runnable") << tr("Clone of") << tr("ROM of") << tr("Sample of");
  element = xmlLines.at(gamePos - 1).simplified();
  insertAttributeItems(item, element, attributes, descriptions, true);

#if defined(QMC2_EMUTYPE_MAME)
  while ( !xmlLines[gamePos].contains("</game>") ) {
#elif defined(QMC2_EMUTYPE_MESS)
  while ( !xmlLines[gamePos].contains("</machine>") ) {
#endif
    element = xmlLines[gamePos].simplified();
    if ( element.contains("<year>") ) {
      content = element.remove("<year>").remove("</year>");
      childItem = new QTreeWidgetItem(item);
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Year"));
      childItem->setText(QMC2_GAMELIST_COLUMN_ICON, content);
    }
    if ( element.contains("<manufacturer>") ) {
      content = element.remove("<manufacturer>").remove("</manufacturer>");
      content.replace("&amp;", "&").replace("&lt;", "<").replace("&gt;", ">").replace("&quot;", "\"");
      childItem = new QTreeWidgetItem(item);
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Manufacturer"));
      childItem->setText(QMC2_GAMELIST_COLUMN_ICON, content);
    }
    if ( element.contains("<rom ") ) {
      childItem = new QTreeWidgetItem(item);
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("ROM"));
      childItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(element, "name"));
      attributes.clear();
      attributes << "bios" << "size" << "crc" << "sha1" << "merge" << "region" << "offset" << "status" << "optional";
      descriptions.clear();
      descriptions << tr("BIOS") << tr("Size") << tr("CRC") << tr("SHA1") << tr("Merge") << tr("Region") << tr("Offset") << tr("Status") << tr("Optional");
      insertAttributeItems(childItem, element, attributes, descriptions, true);
    }
    if ( element.contains("<device_ref ") ) {
      childItem = new QTreeWidgetItem(item);
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Device reference"));
      childItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(element, "name"));
    }
    if ( element.contains("<chip ") ) {
      childItem = new QTreeWidgetItem(item);
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Chip"));
      childItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(element, "name"));
      attributes.clear();
      attributes << "tag" << "type" << "clock";
      descriptions.clear();
      descriptions << tr("Tag") << tr("Type") << tr("Clock");
      insertAttributeItems(childItem, element, attributes, descriptions, true);
    }
    if ( element.contains("<display ") ) {
      childItem = new QTreeWidgetItem(item);
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Display"));
      attributes.clear();
      attributes << "type" << "rotate" << "flipx" << "width" << "height" << "refresh" << "pixclock" << "htotal" << "hbend" << "hbstart" << "vtotal" << "vbend" << "vbstart";
      descriptions.clear();
      descriptions << tr("Type") << tr("Rotate") << tr("Flip-X") << tr("Width") << tr("Height") << tr("Refresh") << tr("Pixel clock") << tr("H-Total") << tr("H-Bend") << tr("HB-Start") << tr("V-Total") << tr("V-Bend") << tr("VB-Start");
      insertAttributeItems(childItem, element, attributes, descriptions, true);
    }
    if ( element.contains("<sound ") ) {
      childItem = new QTreeWidgetItem(item);
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Sound"));
      attributes.clear();
      attributes << "channels";
      descriptions.clear();
      descriptions << tr("Channels");
      insertAttributeItems(childItem, element, attributes, descriptions, true);
    }
    if ( element.contains("<input ") ) {
      childItem = new QTreeWidgetItem(item);
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Input"));
      attributes.clear();
      attributes << "service" << "tilt" << "players" << "buttons" << "coins";
      descriptions.clear();
      descriptions << tr("Service") << tr("Tilt") << tr("Players") << tr("Buttons") << tr("Coins");
      insertAttributeItems(childItem, element, attributes, descriptions, true);

      gamePos++;
      while ( xmlLines[gamePos].contains("<control ") ) {
        QString subElement = xmlLines[gamePos].simplified();
        QTreeWidgetItem *nextChildItem = new QTreeWidgetItem(childItem);
        nextChildItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Control"));
        nextChildItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(subElement, "type", true));
        attributes.clear();
        attributes << "minimum" << "maximum" << "sensitivity" << "keydelta" << "reverse";
        descriptions.clear();
        descriptions << tr("Minimum") << tr("Maximum") << tr("Sensitivity") << tr("Key Delta") << tr("Reverse");
        insertAttributeItems(nextChildItem, subElement, attributes, descriptions, true);
        gamePos++;
      }
    }
    if ( element.contains("<dipswitch ") ) {
      childItem = new QTreeWidgetItem(item);
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("DIP switch"));
      childItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(element, "name", true));

      gamePos++;
      while ( xmlLines[gamePos].contains("<dipvalue ") ) {
        QString subElement = xmlLines[gamePos].simplified();
        QTreeWidgetItem *secondChildItem = new QTreeWidgetItem(childItem);
        secondChildItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("DIP value"));
        secondChildItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(subElement, "name", true));
        attributes.clear();
        attributes << "default";
        descriptions.clear();
        descriptions << tr("Default");
        insertAttributeItems(secondChildItem, subElement, attributes, descriptions, true);
        gamePos++;
      }
    }
    if ( element.contains("<configuration ") ) {
      childItem = new QTreeWidgetItem(item);
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Configuration"));
      childItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(element, "name", true));
      attributes.clear();
      attributes << "tag" << "mask";
      descriptions.clear();
      descriptions << tr("Tag") << tr("Mask");
      insertAttributeItems(childItem, element, attributes, descriptions, true);

      gamePos++;
      while ( xmlLines[gamePos].contains("<confsetting ") ) {
        QString subElement = xmlLines[gamePos].simplified();
        QTreeWidgetItem *secondChildItem = new QTreeWidgetItem(childItem);
        secondChildItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Setting"));
        secondChildItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(subElement, "name", true));
        attributes.clear();
        attributes << "value" << "default";
        descriptions.clear();
        descriptions << tr("Value") << tr("Default");
        insertAttributeItems(secondChildItem, subElement, attributes, descriptions, true);
        gamePos++;
      }
    }
    if ( element.contains("<driver ") ) {
      childItem = new QTreeWidgetItem(item);
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Driver"));
      attributes.clear();
      attributes << "status" << "emulation" << "color" << "sound" << "graphic" << "cocktail" << "protection" << "savestate" << "palettesize";
      descriptions.clear();
      descriptions << tr("Status") << tr("Emulation") << tr("Color") << tr("Sound") << tr("Graphic") << tr("Cocktail") << tr("Protection") << tr("Save state") << tr("Palette size");
      insertAttributeItems(childItem, element, attributes, descriptions, true);
    }
    if ( element.contains("<biosset ") ) {
      childItem = new QTreeWidgetItem(item);
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("BIOS set"));
      childItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(element, "name"));
      attributes.clear();
      attributes << "description" << "default";
      descriptions.clear();
      descriptions << tr("Description") << tr("Default");
      insertAttributeItems(childItem, element, attributes, descriptions, true);
    }
    if ( element.contains("<sample ") ) {
      childItem = new QTreeWidgetItem(item);
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Sample"));
      childItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(element, "name"));
    }
    if ( element.contains("<disk ") ) {
      childItem = new QTreeWidgetItem(item);
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Disk"));
      childItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(element, "name"));
      attributes.clear();
      attributes << "md5" << "sha1" << "merge" << "region" << "index" << "status" << "optional";
      descriptions.clear();
      descriptions << tr("MD5") << tr("SHA1") << tr("Merge") << tr("Region") << tr("Index") << tr("Status") << tr("Optional");
      insertAttributeItems(childItem, element, attributes, descriptions, true);
    }
    if ( element.contains("<adjuster ") ) {
      childItem = new QTreeWidgetItem(item);
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Adjuster"));
      childItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(element, "name"));
      attributes.clear();
      attributes << "default";
      descriptions.clear();
      descriptions << tr("Default");
      insertAttributeItems(childItem, element, attributes, descriptions, true);
    }
    if ( element.contains("<softwarelist ") ) {
      childItem = new QTreeWidgetItem(item);
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Software list"));
      childItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(element, "name"));
      attributes.clear();
      attributes << "status";
      descriptions.clear();
      descriptions << tr("Status");
      insertAttributeItems(childItem, element, attributes, descriptions, true);
    }
    if ( element.contains("<category ") ) {
      childItem = new QTreeWidgetItem(item);
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Category"));
      childItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(element, "name", true));

      gamePos++;
      while ( xmlLines[gamePos].contains("<item ") ) {
        QString subElement = xmlLines[gamePos].simplified();
        QTreeWidgetItem *secondChildItem = new QTreeWidgetItem(childItem);
        secondChildItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Item"));
        secondChildItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(subElement, "name", true));
        attributes.clear();
        attributes << "default";
        descriptions.clear();
        descriptions << tr("Default");
        insertAttributeItems(secondChildItem, subElement, attributes, descriptions, true);
        gamePos++;
      }
    }
    if ( element.contains("<device ") ) {
      childItem = new QTreeWidgetItem(item);
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Device"));
      childItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(element, "type", true));
      attributes.clear();
      attributes << "tag" << "mandatory" << "interface";
      descriptions.clear();
      descriptions << tr("Tag") << tr("Mandatory") << tr("Interface");
      insertAttributeItems(childItem, element, attributes, descriptions, false);

      gamePos++;
      while ( xmlLines[gamePos].contains("<instance ") ) {
        QString subElement = xmlLines[gamePos].simplified();
        QTreeWidgetItem *secondChildItem = new QTreeWidgetItem(childItem);
        secondChildItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Instance"));
        secondChildItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(subElement, "name", false));
        attributes.clear();
        attributes << "briefname";
        descriptions.clear();
        descriptions << tr("Brief name");
        insertAttributeItems(secondChildItem, element, attributes, descriptions, false);
        gamePos++;
      }
      while ( xmlLines[gamePos].contains("<extension ") ) {
        QString subElement = xmlLines[gamePos].simplified();
        QTreeWidgetItem *secondChildItem = new QTreeWidgetItem(childItem);
        secondChildItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Extension"));
        secondChildItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(subElement, "name", false));
        gamePos++;
      }
    }
    if ( element.contains("<ramoption") ) {
      childItem = new QTreeWidgetItem(item);
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("RAM options"));
      while ( xmlLines[gamePos].contains("<ramoption") ) {
        QString subElement = xmlLines[gamePos].simplified();
        QTreeWidgetItem *secondChildItem = new QTreeWidgetItem(childItem);
        secondChildItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Option"));
        int fromIndex = subElement.indexOf('>') + 1;
        int toIndex = subElement.indexOf('<', fromIndex);
        QString ramOptionValue = subElement.mid(fromIndex, toIndex - fromIndex);
        secondChildItem->setText(QMC2_GAMELIST_COLUMN_ICON, ramOptionValue);
        attributes.clear();
        attributes << "default";
        descriptions.clear();
        descriptions << tr("Default");
        insertAttributeItems(secondChildItem, subElement, attributes, descriptions, false);
        gamePos++;
      }
      if ( xmlLines[gamePos].contains("</machine>") )
        gamePos--;
    }
    gamePos++;
  }
  qmc2MainWindow->treeWidgetGamelist->scrollToItem(item);
}

void Gamelist::parse()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::parse()");
#endif

  if ( qmc2StopParser ) {
    qmc2ReloadActive = false;
    enableWidgets(true);
    return;
  }

  bool showROMStatusIcons = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowROMStatusIcons", true).toBool();
  bool showDeviceSets = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowDeviceSets", true).toBool();
  bool showBiosSets = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowBiosSets", true).toBool();

  QTime elapsedTime;
  qmc2MainWindow->progressBarGamelist->setRange(0, numTotalGames);
#if defined(QMC2_EMUTYPE_MAME)
  romCache.setFileName(qmc2Config->value("MAME/FilesAndDirectories/ROMStateCacheFile").toString());
#elif defined(QMC2_EMUTYPE_MESS)
  romCache.setFileName(qmc2Config->value("MESS/FilesAndDirectories/ROMStateCacheFile").toString());
#endif
  romCache.open(QIODevice::ReadOnly | QIODevice::Text);
  if ( !romCache.isOpen() ) {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: can't open ROM state cache, please check ROMs"));
  } else {
    parseTimer.start();
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading ROM state from cache"));
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
      qmc2MainWindow->progressBarGamelist->setFormat(tr("ROM states - %p%"));
    else
      qmc2MainWindow->progressBarGamelist->setFormat("%p%");
    qmc2MainWindow->progressBarGamelist->reset();
    qApp->processEvents();
    tsRomCache.setDevice(&romCache);
    tsRomCache.reset();
    cachedGamesCounter = 0;
    while ( !tsRomCache.atEnd() ) {
      QString line = tsRomCache.readLine();
      if ( !line.isNull() && !line.startsWith("#") ) {
        QStringList words = line.split(" ");
        qmc2GamelistStatusMap[words[0]] = words[1];
        cachedGamesCounter++;
      }
      if ( cachedGamesCounter % QMC2_ROMCACHE_RESPONSIVENESS == 0 ) {
        qmc2MainWindow->progressBarGamelist->setValue(cachedGamesCounter);
        qApp->processEvents();
      }
    }
    numCorrectGames = numMostlyCorrectGames = numIncorrectGames = numNotFoundGames = 0;
    elapsedTime = elapsedTime.addMSecs(parseTimer.elapsed());
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading ROM state from cache, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("%n cached ROM state(s) loaded", "", cachedGamesCounter));

    romCache.close();
    qApp->processEvents();
  }

  QTime processGamelistElapsedTimer;
  parseTimer.start();
#if defined(QMC2_EMUTYPE_MAME)
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("processing game list"));
#elif defined(QMC2_EMUTYPE_MESS)
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("processing machine list"));
#endif
  qmc2MainWindow->treeWidgetGamelist->clear();
  qmc2HierarchyMap.clear();
  qmc2ParentMap.clear();
  qmc2MainWindow->progressBarGamelist->reset();

#if defined(QMC2_EMUTYPE_MAME)
  gamelistCache.setFileName(qmc2Config->value("MAME/FilesAndDirectories/GamelistCacheFile").toString());
#elif defined(QMC2_EMUTYPE_MESS)
  gamelistCache.setFileName(qmc2Config->value("MESS/FilesAndDirectories/GamelistCacheFile").toString());
#endif
  gamelistCache.open(QIODevice::ReadOnly | QIODevice::Text);
  bool reparseGamelist = true;

  if ( gamelistCache.isOpen() ) {
    QString line;
    tsGamelistCache.setDevice(&gamelistCache);
    tsGamelistCache.seek(0);
    
    if ( !tsGamelistCache.atEnd() ) {
      line = tsGamelistCache.readLine();
      while ( line.startsWith("#") && !tsGamelistCache.atEnd() )
        line = tsGamelistCache.readLine();
      QStringList words = line.split("\t");
      if ( words.count() >= 2 ) {
#if defined(QMC2_EMUTYPE_MAME)
        if ( words[0] == "MAME_VERSION" ) {
#elif defined(QMC2_EMUTYPE_MESS)
        if ( words[0] == "MESS_VERSION" ) {
#endif
          reparseGamelist = (words[1] != emulatorVersion);
        }
      } else {
#if defined(QMC2_EMUTYPE_MAME)
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: couldn't determine emulator version of game list cache"));
#elif defined(QMC2_EMUTYPE_MESS)
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: couldn't determine emulator version of machine list cache"));
#endif
      }
      if ( words.count() < 4 ) {
#if defined(QMC2_EMUTYPE_MAME)
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("INFORMATION: the game list cache will now be updated due to a new format"));
#elif defined(QMC2_EMUTYPE_MESS)
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("INFORMATION: the machine list cache will now be updated due to a new format"));
#endif
	reparseGamelist = true;
      } else {
	      int cacheGlcVersion = words[3].toInt();
	      if ( cacheGlcVersion < QMC2_GLC_VERSION ) {
#if defined(QMC2_EMUTYPE_MAME)
		      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("INFORMATION: the game list cache will now be updated due to a new format"));
#elif defined(QMC2_EMUTYPE_MESS)
		      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("INFORMATION: the machine list cache will now be updated due to a new format"));
#endif
		      reparseGamelist = true;
	      }
      }
    }

#if defined(QMC2_EMUTYPE_MAME)
    bool useCatverIni = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/UseCatverIni").toBool();
#endif

    if ( !reparseGamelist && !qmc2StopParser ) {
#if defined(QMC2_EMUTYPE_MAME)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading game data from game list cache"));
      if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
        qmc2MainWindow->progressBarGamelist->setFormat(tr("Game data - %p%"));
#elif defined(QMC2_EMUTYPE_MESS)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading machine data from machine list cache"));
      if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
        qmc2MainWindow->progressBarGamelist->setFormat(tr("Machine data - %p%"));
#endif
      else
        qmc2MainWindow->progressBarGamelist->setFormat("%p%");
      QTime gameDataCacheElapsedTime;
      miscTimer.start();
      numGames = numUnknownGames = numDevices = 0;
      qmc2MainWindow->progressBarGamelist->reset();
      qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(false);
      QString readBuffer;
      while ( (!tsGamelistCache.atEnd() || !readBuffer.isEmpty() ) && !qmc2StopParser ) {
        readBuffer += tsGamelistCache.read(QMC2_FILE_BUFFER_SIZE);
        bool endsWithNewLine = readBuffer.endsWith("\n");
        QStringList lines = readBuffer.split("\n");
        int l, lc = lines.count();
        if ( !endsWithNewLine )
          lc -= 1;
        for (l = 0; l < lc; l++) {
          line = lines[l];
          if ( !line.isEmpty() && !line.startsWith("#") ) {
            QStringList words = line.split("\t");
            QString gameName = words[0];
            QString gameDescription = words[1];
            QString gameManufacturer = words[2];
            QString gameYear = words[3];
            QString gameCloneOf = words[4];
            bool isBIOS = (words[5] == "1");
            bool hasROMs = (words[6] == "1");
            bool hasCHDs = (words[7] == "1");
            QString gamePlayers = words[8];
            QString gameStatus = words[9];
            bool isDevice = (words[10] == "1");

#ifdef QMC2_DEBUG
            qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Gamelist::parse(): gameName = %1, gameDescription = %2, gameManufacturer = %3, gameYear = %4, gameCloneOf = %5, isBIOS = %6, hasROMs = %7, hasCHDs = %8, gamePlayers = %9, gameStatus = %10, isDevice = %11").
                            arg(gameName).arg(gameDescription).arg(gameManufacturer).arg(gameYear).arg(gameCloneOf).arg(isBIOS).arg(hasROMs).arg(hasCHDs).arg(gamePlayers).arg(gameStatus).arg(isDevice));
#endif

            GamelistItem *gameDescriptionItem = new GamelistItem(qmc2MainWindow->treeWidgetGamelist);
	    gameDescriptionItem->setHidden((isBIOS && !showBiosSets) || (isDevice && !showDeviceSets));
            gameDescriptionItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
            gameDescriptionItem->setCheckState(QMC2_GAMELIST_COLUMN_TAG, Qt::Unchecked);

            if ( !gameCloneOf.isEmpty() )
              qmc2HierarchyMap[gameCloneOf].append(gameName);
            else if ( !qmc2HierarchyMap.contains(gameName) )
              qmc2HierarchyMap.insert(gameName, QStringList());

            gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_GAME, gameDescription);
            gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_YEAR, gameYear);
            gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_MANU, gameManufacturer);
            gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_NAME, gameName);
	    if ( hasROMs && hasCHDs )
              gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_RTYPES, tr("ROM, CHD"));
            else if ( hasROMs )
              gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_RTYPES, tr("ROM"));
            else if ( hasCHDs )
              gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_RTYPES, tr("CHD"));
            else
              gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_RTYPES, "--");
            if ( isDevice ) {
	      gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_PLAYERS, tr("N/A"));
	      gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_DRVSTAT, tr("N/A"));
            }
	    else {
	      gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_PLAYERS, gamePlayers);
	      gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_DRVSTAT, tr(gameStatus.toAscii()));
            }
#if defined(QMC2_EMUTYPE_MAME)
            if ( useCatverIni ) {
              QString categoryString = qmc2CategoryMap[gameName];
              QString versionString = qmc2VersionMap[gameName];
              gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_CATEGORY, categoryString.isEmpty() ? tr("Unknown") : categoryString);
              gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_VERSION, versionString.isEmpty() ? tr("?") : versionString);
            }
#endif
            switch ( qmc2GamelistStatusMap[gameName][0].toAscii() ) {
              case 'C': 
                numCorrectGames++;
                if ( isBIOS ) {
			if ( showROMStatusIcons )
				gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectBIOSImageIcon);
			qmc2BiosROMs << gameName;
		} else if ( isDevice ) {
			if ( showROMStatusIcons )
				gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectDeviceImageIcon);
			qmc2DeviceROMs << gameName;
		} else if ( showROMStatusIcons )
			gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectImageIcon);
                gameDescriptionItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_C);
                break;

              case 'M': 
                numMostlyCorrectGames++;
                if ( isBIOS ) {
			if ( showROMStatusIcons )
				gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectBIOSImageIcon);
			qmc2BiosROMs << gameName;
		} else if ( isDevice ) {
			if ( showROMStatusIcons )
				gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectDeviceImageIcon);
			qmc2DeviceROMs << gameName;
		} else if ( showROMStatusIcons )
			gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectImageIcon);
                gameDescriptionItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_M);
                break;

              case 'I':
                numIncorrectGames++;
                if ( isBIOS ) {
			if ( showROMStatusIcons )
				gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectBIOSImageIcon);
			qmc2BiosROMs << gameName;
		} else if ( isDevice ) {
			if ( showROMStatusIcons )
				gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectDeviceImageIcon);
			qmc2DeviceROMs << gameName;
		} else if ( showROMStatusIcons )
			gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectImageIcon);
                gameDescriptionItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_I);
                break;

              case 'N':
                numNotFoundGames++;
                if ( isBIOS ) {
			if ( showROMStatusIcons )
				gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundBIOSImageIcon);
			qmc2BiosROMs << gameName;
		} else if ( isDevice ) {
			if ( showROMStatusIcons )
				gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundDeviceImageIcon);
			qmc2DeviceROMs << gameName;
		} else if ( showROMStatusIcons )
			gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundImageIcon);
                gameDescriptionItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_N);
                break;

              default:
                numUnknownGames++;
                qmc2GamelistStatusMap[gameName] = "U";
                if ( isBIOS ) {
			if ( showROMStatusIcons )
				gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownBIOSImageIcon);
			qmc2BiosROMs << gameName;
		} else if ( isDevice ) {
			if ( showROMStatusIcons )
				gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownDeviceImageIcon);
			qmc2DeviceROMs << gameName;
		} else if ( showROMStatusIcons )
			gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownImageIcon);
                gameDescriptionItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_U);
                break;
            }

            QTreeWidgetItem *nameItem = new QTreeWidgetItem(gameDescriptionItem);
            nameItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Waiting for data..."));
            nameItem->setText(QMC2_GAMELIST_COLUMN_ICON, gameName);
            qmc2GamelistItemMap[gameName] = gameDescriptionItem;
            qmc2GamelistItemByDescriptionMap[gameDescription] = gameDescriptionItem;
            qmc2GamelistDescriptionMap[gameName] = gameDescription;
            qmc2GamelistNameMap[gameDescription] = gameName;

            loadIcon(gameName, gameDescriptionItem);

            numGames++;
            if ( isDevice ) numDevices++;
          }

          if ( numGames % qmc2GamelistResponsiveness == 0 ) {
            qmc2MainWindow->progressBarGamelist->setValue(numGames);
            qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(true);
            qmc2MainWindow->labelGamelistStatus->setText(status());
            if ( qmc2Options->config->value(QMC2_FRONTEND_PREFIX + "Gamelist/SortOnline").toBool() )
              qmc2MainWindow->treeWidgetGamelist->sortItems(qmc2MainWindow->sortCriteriaLogicalIndex(), qmc2SortOrder);
            qApp->processEvents();
            qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(false);
          }
        }

        if ( endsWithNewLine )
          readBuffer.clear();
        else
          readBuffer = lines.last();
      }
      qmc2MainWindow->progressBarGamelist->setValue(numGames);
      qApp->processEvents();

      gameDataCacheElapsedTime = gameDataCacheElapsedTime.addMSecs(miscTimer.elapsed());
#if defined(QMC2_EMUTYPE_MAME)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading game data from game list cache, elapsed time = %1)").arg(gameDataCacheElapsedTime.toString("mm:ss.zzz")));
#elif defined(QMC2_EMUTYPE_MESS)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading machine data from machine list cache, elapsed time = %1)").arg(gameDataCacheElapsedTime.toString("mm:ss.zzz")));
#endif
    }
  } 
  if ( gamelistCache.isOpen() )
    gamelistCache.close();

  xmlLines.clear();
  xmlGamePositionMap.clear();
  qmc2XmlGamePositionMap.clear();
#if defined(QMC2_EMUTYPE_MAME)
  xmlLines = gamelistBuffer.remove(0, gamelistBuffer.indexOf("<mame build")).split("\n");
#elif defined(QMC2_EMUTYPE_MESS)
  xmlLines = gamelistBuffer.remove(0, gamelistBuffer.indexOf("<mess build")).split("\n");
#endif
  gamelistBuffer.clear();

  if ( reparseGamelist && !qmc2StopParser ) {
#if defined(QMC2_EMUTYPE_MAME)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("parsing game data and (re)creating game list cache"));
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
      qmc2MainWindow->progressBarGamelist->setFormat(tr("Game data - %p%"));
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("parsing machine data and (re)creating machine list cache"));
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
      qmc2MainWindow->progressBarGamelist->setFormat(tr("Machine data - %p%"));
#endif
    else
      qmc2MainWindow->progressBarGamelist->setFormat("%p%");
    gamelistCache.open(QIODevice::WriteOnly | QIODevice::Text);
    if ( !gamelistCache.isOpen() ) {
#if defined(QMC2_EMUTYPE_MAME)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ERROR: can't open game list cache for writing, path = %1").arg(gamelistCache.fileName()));
#elif defined(QMC2_EMUTYPE_MESS)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ERROR: can't open machine list cache for writing, path = %1").arg(gamelistCache.fileName()));
#endif
    } else {
      tsGamelistCache.setDevice(&gamelistCache);
      tsGamelistCache.reset();
      tsGamelistCache << "# THIS FILE IS AUTO-GENERATED - PLEASE DO NOT EDIT!\n";
#if defined(QMC2_EMUTYPE_MAME)
      tsGamelistCache << "MAME_VERSION\t" + emulatorVersion + "\tGLC_VERSION\t" + QString::number(QMC2_GLC_VERSION) + "\n";
#elif defined(QMC2_EMUTYPE_MESS)
      tsGamelistCache << "MESS_VERSION\t" + emulatorVersion + "\tGLC_VERSION\t" + QString::number(QMC2_GLC_VERSION) + "\n";
#endif
    }

    // parse XML gamelist data
    int lineCounter;
    numGames = numUnknownGames = numDevices = 0;
    bool endParser = qmc2StopParser;
    qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(false);

    for (lineCounter = 0; lineCounter < xmlLines.count() && !endParser; lineCounter++) {
      while ( !endParser && !xmlLines[lineCounter].contains("<description>") ) {
        lineCounter++;
#if defined(QMC2_EMUTYPE_MAME)
        endParser = xmlLines[lineCounter].contains("</mame>") || qmc2StopParser;
#elif defined(QMC2_EMUTYPE_MESS)
        endParser = xmlLines[lineCounter].contains("</mess>") || qmc2StopParser;
#endif
      }
      if ( !endParser ) {
        QString descriptionElement = xmlLines[lineCounter].simplified();
        QString gameElement = xmlLines[lineCounter - 1].simplified();
	if ( !gameElement.contains(" name=\"") )
		continue;
        bool isBIOS = ( value(gameElement, "isbios") == "yes" );
        bool isDevice = ( value(gameElement, "isdevice") == "yes" );
        QString gameName = value(gameElement, "name");
        QString gameCloneOf = value(gameElement, "cloneof");
        QString gameDescription = descriptionElement.remove("<description>").remove("</description>").replace("&amp;", "&").replace("&lt;", "<").replace("&gt;", ">").replace("&quot;", "\"");
        GamelistItem *gameDescriptionItem = new GamelistItem(qmc2MainWindow->treeWidgetGamelist);
        gameDescriptionItem->setHidden((isBIOS && !showBiosSets) || (isDevice && !showDeviceSets));
        gameDescriptionItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
        gameDescriptionItem->setCheckState(QMC2_GAMELIST_COLUMN_TAG, Qt::Unchecked);

#if defined(QMC2_EMUTYPE_MAME)
        bool useCatverIni = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/UseCatverIni").toBool();
#endif

        // find year & manufacturer and determine ROM/CHD requirements
        bool endGame = false;
        int i = lineCounter;
        QString gameYear = "?", gameManufacturer = "?", gamePlayers = "?", gameStatus = "?";
        bool yearFound = false, manufacturerFound = false, hasROMs = false, hasCHDs = false, playersFound = false, statusFound = false;
        while ( !endGame ) {
          QString xmlLine = xmlLines[i];
          if ( xmlLine.contains("<year>") ) {
            gameYear = xmlLine.simplified().remove("<year>").remove("</year>");
            yearFound = true;
          } else if ( xmlLine.contains("<manufacturer>") ) {
            gameManufacturer = xmlLine.simplified().remove("<manufacturer>").remove("</manufacturer>").replace("&amp;", "&").replace("&lt;", "<").replace("&gt;", ">").replace("&quot;", "\"");
            manufacturerFound = true;
          } else if ( xmlLine.contains("<rom name") ) {
            hasROMs = true;
          } else if ( xmlLine.contains("<disk name") ) {
            hasCHDs = true;
          } else if ( xmlLine.contains("<input players") ) {
            int playersPos = xmlLine.indexOf("input players=\"") + 15;
            if ( playersPos >= 0 ) {
              gamePlayers = xmlLine.mid(playersPos, xmlLine.indexOf("\"", playersPos) - playersPos);
              playersFound = true;
            }
          } else if ( xmlLine.contains("<driver status") ) {
            int statusPos = xmlLine.indexOf("driver status=\"") + 15;
            if ( statusPos >= 0 ) {
              gameStatus = xmlLine.mid(statusPos, xmlLine.indexOf("\"", statusPos) - statusPos);
              statusFound = true;
            }
          }
#if defined(QMC2_EMUTYPE_MAME)
          endGame = xmlLine.contains("</game>") || (yearFound && manufacturerFound && hasROMs && hasCHDs && playersFound && statusFound);
#elif defined(QMC2_EMUTYPE_MESS)
          endGame = xmlLine.contains("</machine>") || (yearFound && manufacturerFound && hasROMs && hasCHDs && playersFound && statusFound);
#endif
          i++;
        }

        if ( !gameCloneOf.isEmpty() )
          qmc2HierarchyMap[gameCloneOf].append(gameName);
        else if ( !qmc2HierarchyMap.contains(gameName) )
          qmc2HierarchyMap.insert(gameName, QStringList());

        gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_GAME, gameDescription);
        gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_YEAR, gameYear);
        gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_MANU, gameManufacturer);
        gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_NAME, gameName);
	if ( hasROMs && hasCHDs )
          gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_RTYPES, tr("ROM, CHD"));
        else if ( hasROMs )
          gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_RTYPES, tr("ROM"));
        else if ( hasCHDs )
          gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_RTYPES, tr("CHD"));
        else
          gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_RTYPES, "--");
        if ( isDevice ) {
	  gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_PLAYERS, tr("N/A"));
	  gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_DRVSTAT, tr("N/A"));
	} else {
	  gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_PLAYERS, gamePlayers);
	  gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_DRVSTAT, tr(gameStatus.toAscii()));
        }
#if defined(QMC2_EMUTYPE_MAME)
        if ( useCatverIni ) {
          QString categoryString = qmc2CategoryMap[gameName];
          QString versionString = qmc2VersionMap[gameName];
          gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_CATEGORY, categoryString.isEmpty() ? tr("Unknown") : categoryString);
          gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_VERSION, versionString.isEmpty() ? tr("?") : versionString);
        }
#endif
        switch ( qmc2GamelistStatusMap[gameName][0].toAscii() ) {
          case 'C': 
            numCorrectGames++;
            if ( isBIOS ) {
		    if ( showROMStatusIcons )
			    gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectBIOSImageIcon);
		    qmc2BiosROMs << gameName;
	    } else if ( isDevice ) {
		    if ( showROMStatusIcons )
			    gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectDeviceImageIcon);
		    qmc2DeviceROMs << gameName;
	    } else if ( showROMStatusIcons )
		    gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectImageIcon);
            gameDescriptionItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_C);
            break;

          case 'M': 
            numMostlyCorrectGames++;
            if ( isBIOS ) {
		    if ( showROMStatusIcons )
			    gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectBIOSImageIcon);
		    qmc2BiosROMs << gameName;
	    } else if ( isDevice ) {
		    if ( showROMStatusIcons )
			    gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectDeviceImageIcon);
		    qmc2DeviceROMs << gameName;
	    } else if ( showROMStatusIcons )
		    gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectImageIcon);
            gameDescriptionItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_M);
            break;

          case 'I':
            numIncorrectGames++;
            if ( isBIOS ) {
		    if ( showROMStatusIcons )
			    gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectBIOSImageIcon);
		    qmc2BiosROMs << gameName;
	    } else if ( isDevice ) {
		    if ( showROMStatusIcons )
			    gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectDeviceImageIcon);
		    qmc2DeviceROMs << gameName;
	    } else if ( showROMStatusIcons )
		    gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectImageIcon);
            gameDescriptionItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_I);
            break;

          case 'N':
            numNotFoundGames++;
            if ( isBIOS ) {
		    if ( showROMStatusIcons )
			    gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundBIOSImageIcon);
		    qmc2BiosROMs << gameName;
	    } else if ( isDevice ) {
		    if ( showROMStatusIcons )
			    gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundDeviceImageIcon);
		    qmc2DeviceROMs << gameName;
	    } else if ( showROMStatusIcons )
		    gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundImageIcon);
            gameDescriptionItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_N);
            break;

          default:
            numUnknownGames++;
            qmc2GamelistStatusMap[gameName] = "U";
            if ( isBIOS ) {
		    if ( showROMStatusIcons )
			    gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownBIOSImageIcon);
		    qmc2BiosROMs << gameName;
	    } else if ( isDevice ) {
		    if ( showROMStatusIcons )
			    gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownDeviceImageIcon);
		    qmc2DeviceROMs << gameName;
	    } else if ( showROMStatusIcons )
		    gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownImageIcon);
            gameDescriptionItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_U);
            break;
        }

        QTreeWidgetItem *nameItem = new QTreeWidgetItem(gameDescriptionItem);
        nameItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Waiting for data..."));
        nameItem->setText(QMC2_GAMELIST_COLUMN_ICON, gameName);
        qmc2GamelistItemMap[gameName] = gameDescriptionItem;
        qmc2GamelistItemByDescriptionMap[gameDescription] = gameDescriptionItem;
        qmc2GamelistDescriptionMap[gameName] = gameDescription;
        qmc2GamelistNameMap[gameDescription] = gameName;

        loadIcon(gameName, gameDescriptionItem);

        if ( gamelistCache.isOpen() )
          tsGamelistCache << gameName << "\t" << gameDescription << "\t" << gameManufacturer << "\t"
                          << gameYear << "\t" << gameCloneOf << "\t" << (isBIOS ? "1": "0") << "\t"
			  << (hasROMs ? "1" : "0") << "\t" << (hasCHDs ? "1": "0") << "\t"
			  << gamePlayers << "\t" << gameStatus << "\t" << (isDevice ? "1": "0") <<"\n";

        numGames++;
        if ( isDevice ) numDevices++;
      }

      qmc2MainWindow->progressBarGamelist->setValue(numGames);

      if ( numGames % qmc2GamelistResponsiveness == 0 ) {
        qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(true);
        qmc2MainWindow->labelGamelistStatus->setText(status());
        if ( qmc2Options->config->value(QMC2_FRONTEND_PREFIX + "Gamelist/SortOnline").toBool() )
          qmc2MainWindow->treeWidgetGamelist->sortItems(qmc2MainWindow->sortCriteriaLogicalIndex(), qmc2SortOrder);
        qApp->processEvents();
        qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(false);
      }
    }
  }
  if ( gamelistCache.isOpen() )
    gamelistCache.close();

#if defined(QMC2_EMUTYPE_MAME)
  bool useCatverIni = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/UseCatverIni").toBool();
#endif

  // create parent/clone hierarchy tree
  qmc2MainWindow->treeWidgetHierarchy->clear();
  QMapIterator<QString, QStringList> i(qmc2HierarchyMap);
  while ( i.hasNext() ) {
    i.next();
    QString iValue = i.key();
    QString iDescription = qmc2GamelistDescriptionMap[iValue];
    if ( iDescription.isEmpty() )
      continue;
    bool isBIOS = qmc2BiosROMs.contains(iValue);
    bool isDevice = qmc2DeviceROMs.contains(iValue);
    GamelistItem *hierarchyItem = new GamelistItem(qmc2MainWindow->treeWidgetHierarchy);
    hierarchyItem->setHidden((isBIOS && !showBiosSets) || (isDevice && !showDeviceSets));
    hierarchyItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
    hierarchyItem->setCheckState(QMC2_GAMELIST_COLUMN_TAG, Qt::Unchecked);
    hierarchyItem->setText(QMC2_GAMELIST_COLUMN_GAME, iDescription);
    QTreeWidgetItem *baseItem = qmc2GamelistItemMap[iValue];
    hierarchyItem->setText(QMC2_GAMELIST_COLUMN_YEAR, baseItem->text(QMC2_GAMELIST_COLUMN_YEAR));
    hierarchyItem->setText(QMC2_GAMELIST_COLUMN_MANU, baseItem->text(QMC2_GAMELIST_COLUMN_MANU));
    hierarchyItem->setText(QMC2_GAMELIST_COLUMN_NAME, baseItem->text(QMC2_GAMELIST_COLUMN_NAME));
    hierarchyItem->setText(QMC2_GAMELIST_COLUMN_RTYPES, baseItem->text(QMC2_GAMELIST_COLUMN_RTYPES));
    hierarchyItem->setText(QMC2_GAMELIST_COLUMN_PLAYERS, baseItem->text(QMC2_GAMELIST_COLUMN_PLAYERS));
    hierarchyItem->setText(QMC2_GAMELIST_COLUMN_DRVSTAT, baseItem->text(QMC2_GAMELIST_COLUMN_DRVSTAT));
#if defined(QMC2_EMUTYPE_MAME)
    if ( useCatverIni ) {
      hierarchyItem->setText(QMC2_GAMELIST_COLUMN_CATEGORY, baseItem->text(QMC2_GAMELIST_COLUMN_CATEGORY));
      hierarchyItem->setText(QMC2_GAMELIST_COLUMN_VERSION, baseItem->text(QMC2_GAMELIST_COLUMN_VERSION));
    }
#endif
    qmc2HierarchyItemMap[iValue] = hierarchyItem;
    switch ( qmc2GamelistStatusMap[iValue][0].toAscii() ) {
      case 'C': 
        if ( isBIOS ) {
          if ( showROMStatusIcons ) hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectBIOSImageIcon);
        } else if ( isDevice ) {
          if ( showROMStatusIcons ) hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectDeviceImageIcon);
	} else {
          if ( showROMStatusIcons ) hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectImageIcon);
	}
        hierarchyItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_C);
        break;

      case 'M': 
        if ( isBIOS ) {
          if ( showROMStatusIcons ) hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectBIOSImageIcon);
        } else if ( isDevice ) {
          if ( showROMStatusIcons ) hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectDeviceImageIcon);
        } else {
          if ( showROMStatusIcons ) hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectImageIcon);
	}
        hierarchyItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_M);
        break;

      case 'I':
        if ( isBIOS ) {
          if ( showROMStatusIcons ) hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectBIOSImageIcon);
        } else if ( isDevice ) {
          if ( showROMStatusIcons ) hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectDeviceImageIcon);
	} else {
          if ( showROMStatusIcons ) hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectImageIcon);
	}
        hierarchyItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_I);
        break;

      case 'N':
        if ( isBIOS ) {
          if ( showROMStatusIcons ) hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundBIOSImageIcon);
        } else if ( isDevice ) {
          if ( showROMStatusIcons ) hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundDeviceImageIcon);
	} else {
          if ( showROMStatusIcons ) hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundImageIcon);
	}
        hierarchyItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_N);
        break;

      default:
        if ( isBIOS ) {
          if ( showROMStatusIcons ) hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownBIOSImageIcon);
        } else if ( isDevice ) {
          if ( showROMStatusIcons ) hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownDeviceImageIcon);
	} else {
          if ( showROMStatusIcons ) hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownImageIcon);
	}
        hierarchyItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_U);
        break;
    }

    loadIcon(iValue, hierarchyItem);

    int j;
    for (j = 0; j < i.value().count(); j++) {
      QString jValue = i.value().at(j);
      QString jDescription = qmc2GamelistDescriptionMap[jValue];
      if ( jDescription.isEmpty() )
        continue;
      GamelistItem *hierarchySubItem = new GamelistItem(hierarchyItem);
      isBIOS = qmc2BiosROMs.contains(jValue);
      isDevice = qmc2DeviceROMs.contains(jValue);
      hierarchySubItem->setHidden((isBIOS && !showBiosSets) || (isDevice && !showDeviceSets));
      hierarchySubItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
      hierarchySubItem->setCheckState(QMC2_GAMELIST_COLUMN_TAG, Qt::Unchecked);
      hierarchySubItem->setText(QMC2_GAMELIST_COLUMN_GAME, jDescription);
      baseItem = qmc2GamelistItemMap[jValue];
      hierarchySubItem->setText(QMC2_GAMELIST_COLUMN_YEAR, baseItem->text(QMC2_GAMELIST_COLUMN_YEAR));
      hierarchySubItem->setText(QMC2_GAMELIST_COLUMN_MANU, baseItem->text(QMC2_GAMELIST_COLUMN_MANU));
      hierarchySubItem->setText(QMC2_GAMELIST_COLUMN_NAME, baseItem->text(QMC2_GAMELIST_COLUMN_NAME));
      hierarchySubItem->setText(QMC2_GAMELIST_COLUMN_RTYPES, baseItem->text(QMC2_GAMELIST_COLUMN_RTYPES));
      hierarchySubItem->setText(QMC2_GAMELIST_COLUMN_PLAYERS, baseItem->text(QMC2_GAMELIST_COLUMN_PLAYERS));
      hierarchySubItem->setText(QMC2_GAMELIST_COLUMN_DRVSTAT, baseItem->text(QMC2_GAMELIST_COLUMN_DRVSTAT));
#if defined(QMC2_EMUTYPE_MAME)
      if ( useCatverIni ) {
        hierarchySubItem->setText(QMC2_GAMELIST_COLUMN_CATEGORY, baseItem->text(QMC2_GAMELIST_COLUMN_CATEGORY));
        hierarchySubItem->setText(QMC2_GAMELIST_COLUMN_VERSION, baseItem->text(QMC2_GAMELIST_COLUMN_VERSION));
      }
#endif
      qmc2HierarchyItemMap[jValue] = hierarchySubItem;
      qmc2ParentMap[jValue] = iValue;
      // "fill up" emulator info data for clones
      if ( !qmc2EmuInfoDB.isEmpty() ) {
        QByteArray *p = qmc2EmuInfoDB[hierarchyItem->text(QMC2_GAMELIST_COLUMN_NAME)];
        if ( p )
          if ( !qmc2EmuInfoDB.contains(baseItem->text(QMC2_GAMELIST_COLUMN_NAME)) )
            qmc2EmuInfoDB[baseItem->text(QMC2_GAMELIST_COLUMN_NAME)] = p;
      }
      switch ( qmc2GamelistStatusMap[jValue][0].toAscii() ) {
        case 'C': 
          if ( isBIOS ) {
            if ( showROMStatusIcons ) hierarchySubItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectBIOSImageIcon);
          } else if ( isDevice ) {
            if ( showROMStatusIcons ) hierarchySubItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectDeviceImageIcon);
	  } else {
            if ( showROMStatusIcons ) hierarchySubItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectImageIcon);
	  }
          hierarchySubItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_C);
          break;

        case 'M': 
          if ( isBIOS ) {
            if ( showROMStatusIcons ) hierarchySubItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectBIOSImageIcon);
          } else if ( isDevice ) {
            if ( showROMStatusIcons ) hierarchySubItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectDeviceImageIcon);
	  } else {
            if ( showROMStatusIcons ) hierarchySubItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectImageIcon);
	  }
          hierarchySubItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_M);
          break;

        case 'I':
          if ( isBIOS ) {
            if ( showROMStatusIcons ) hierarchySubItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectBIOSImageIcon);
          } else if ( isDevice ) {
            if ( showROMStatusIcons ) hierarchySubItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectDeviceImageIcon);
	  } else {
            if ( showROMStatusIcons ) hierarchySubItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectImageIcon);
	  }
          hierarchySubItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_I);
          break;

        case 'N':
          if ( isBIOS ) {
            if ( showROMStatusIcons ) hierarchySubItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundBIOSImageIcon);
          } else if ( isDevice ) {
            if ( showROMStatusIcons ) hierarchySubItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundDeviceImageIcon);
	  } else {
            if ( showROMStatusIcons ) hierarchySubItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundImageIcon);
	  }
          hierarchySubItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_N);
          break;

        default:
          if ( isBIOS ) {
            if ( showROMStatusIcons ) hierarchySubItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownBIOSImageIcon);
          } else if ( isDevice ) {
            if ( showROMStatusIcons ) hierarchySubItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownDeviceImageIcon);
	  } else {
            if ( showROMStatusIcons ) hierarchySubItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownImageIcon);
	  }
          hierarchySubItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_U);
          break;
      }

      loadIcon(jValue, hierarchySubItem);
    }
  }

  QString sortCriteria = "?";
  switch ( qmc2SortCriteria ) {
    case QMC2_SORT_BY_DESCRIPTION:
#if defined(QMC2_EMUTYPE_MAME)
      sortCriteria = QObject::tr("game description");
#elif defined(QMC2_EMUTYPE_MESS)
      sortCriteria = QObject::tr("machine description");
#endif
      break;
    case QMC2_SORT_BY_ROM_STATE:
      sortCriteria = QObject::tr("ROM state");
      break;
    case QMC2_SORT_BY_TAG:
      sortCriteria = QObject::tr("tag");
      break;
    case QMC2_SORT_BY_YEAR:
      sortCriteria = QObject::tr("year");
      break;
    case QMC2_SORT_BY_MANUFACTURER:
      sortCriteria = QObject::tr("manufacturer");
      break;
    case QMC2_SORT_BY_NAME:
#if defined(QMC2_EMUTYPE_MAME)
      sortCriteria = QObject::tr("game name");
#elif defined(QMC2_EMUTYPE_MESS)
      sortCriteria = QObject::tr("machine name");
#endif
    case QMC2_SORT_BY_ROMTYPES:
      sortCriteria = QObject::tr("ROM types");
      break;
    case QMC2_SORT_BY_PLAYERS:
      sortCriteria = QObject::tr("players");
      break;
    case QMC2_SORT_BY_DRVSTAT:
      sortCriteria = QObject::tr("driver status");
      break;
#if defined(QMC2_EMUTYPE_MAME)
    case QMC2_SORT_BY_CATEGORY:
      sortCriteria = QObject::tr("category");
      break;
    case QMC2_SORT_BY_VERSION:
      sortCriteria = QObject::tr("version");
      break;
#endif
  }
#if defined(QMC2_EMUTYPE_MAME)
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("sorting game list by %1 in %2 order").arg(sortCriteria).arg(qmc2SortOrder == Qt::AscendingOrder ? tr("ascending") : tr("descending")));
#elif defined(QMC2_EMUTYPE_MESS)
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("sorting machine list by %1 in %2 order").arg(sortCriteria).arg(qmc2SortOrder == Qt::AscendingOrder ? tr("ascending") : tr("descending")));
#endif
  qApp->processEvents();
  QList<QTreeWidgetItem *> itemList = qmc2MainWindow->treeWidgetGamelist->findItems("*", Qt::MatchContains | Qt::MatchWildcard);
  for (int i = 0; i < itemList.count(); i++) {
    if ( itemList[i]->childCount() > 1 ) {
      qmc2MainWindow->treeWidgetGamelist->collapseItem(itemList[i]);
      QList<QTreeWidgetItem *> childrenList = itemList[i]->takeChildren();
      int j;
      for (j = 0; j < childrenList.count(); j++)
        delete childrenList[j];
      QTreeWidgetItem *nameItem = new QTreeWidgetItem(itemList[i]);
      nameItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Waiting for data..."));
      nameItem->setText(QMC2_GAMELIST_COLUMN_ICON, qmc2GamelistNameMap[itemList[i]->text(QMC2_GAMELIST_COLUMN_GAME)]);
      qApp->processEvents();
    }
  }
  qmc2MainWindow->treeWidgetGamelist->sortItems(qmc2MainWindow->sortCriteriaLogicalIndex(), qmc2SortOrder);
  qmc2MainWindow->treeWidgetHierarchy->sortItems(qmc2MainWindow->sortCriteriaLogicalIndex(), qmc2SortOrder);
  qApp->processEvents();
  QTreeWidgetItem *ci = qmc2MainWindow->treeWidgetGamelist->currentItem();
  if ( ci ) {
    if ( ci->isSelected() ) {
      QTimer::singleShot(0, qmc2MainWindow, SLOT(scrollToCurrentItem()));
    } else if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/RestoreGameSelection").toBool() ) {
#if defined(QMC2_EMUTYPE_MAME)
      QTreeWidgetItem *glItem = qmc2GamelistItemByDescriptionMap[qmc2Config->value("MAME/SelectedGame").toString()];
#elif defined(QMC2_EMUTYPE_MESS)
      QTreeWidgetItem *glItem = qmc2GamelistItemByDescriptionMap[qmc2Config->value("MESS/SelectedGame").toString()];
#endif
      if ( glItem ) {
#if defined(QMC2_EMUTYPE_MAME)
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("restoring game selection"));
#elif defined(QMC2_EMUTYPE_MESS)
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("restoring machine selection"));
#endif
        qmc2MainWindow->treeWidgetGamelist->setCurrentItem(glItem);
        QTimer::singleShot(0, qmc2MainWindow, SLOT(scrollToCurrentItem()));
      }
    }
  } else if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/RestoreGameSelection").toBool() ) {
#if defined(QMC2_EMUTYPE_MAME)
    QTreeWidgetItem *glItem = qmc2GamelistItemByDescriptionMap[qmc2Config->value("MAME/SelectedGame").toString()];
#elif defined(QMC2_EMUTYPE_MESS)
    QTreeWidgetItem *glItem = qmc2GamelistItemByDescriptionMap[qmc2Config->value("MESS/SelectedGame").toString()];
#endif
    if ( glItem ) {
#if defined(QMC2_EMUTYPE_MAME)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("restoring game selection"));
#elif defined(QMC2_EMUTYPE_MESS)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("restoring machine selection"));
#endif
      qmc2MainWindow->treeWidgetGamelist->setCurrentItem(glItem);
      QTimer::singleShot(0, qmc2MainWindow, SLOT(scrollToCurrentItem()));
    }
  }
  qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(true);
  qmc2MainWindow->labelGamelistStatus->setText(status());

  processGamelistElapsedTimer = processGamelistElapsedTimer.addMSecs(parseTimer.elapsed());
  int numBIOSs = qmc2BiosROMs.count();
#if defined(QMC2_EMUTYPE_MAME)
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (processing game list, elapsed time = %1)").arg(processGamelistElapsedTimer.toString("mm:ss.zzz")));
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("%n game(s)", "", numTotalGames - numBIOSs) + tr(", %n BIOS set(s)", "", numBIOSs) + tr(" and %n device(s) loaded", "", numDevices));
#elif defined(QMC2_EMUTYPE_MESS)
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (processing machine list, elapsed time = %1)").arg(processGamelistElapsedTimer.toString("mm:ss.zzz")));
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("%n machine(s)", "", numTotalGames - numBIOSs) + tr(", %n BIOS set(s)", "", numBIOSs) + tr(" and %n device(s) loaded", "", numDevices));
#endif

  if ( numGames - numDevices != numTotalGames ) {
    if ( reparseGamelist && qmc2StopParser ) {
#if defined(QMC2_EMUTYPE_MAME)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: game list not fully parsed, invalidating game list cache"));
      QFile f(qmc2Config->value("MAME/FilesAndDirectories/GamelistCacheFile").toString());
#elif defined(QMC2_EMUTYPE_MESS)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: machine list not fully parsed, invalidating machine list cache"));
      QFile f(qmc2Config->value("MESS/FilesAndDirectories/GamelistCacheFile").toString());
#endif
      f.remove();
    } else if ( !qmc2StopParser) {
#if defined(QMC2_EMUTYPE_MAME)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: game list cache is out of date, invalidating game list cache"));
      QFile f(qmc2Config->value("MAME/FilesAndDirectories/GamelistCacheFile").toString());
#elif defined(QMC2_EMUTYPE_MESS)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: machine list cache is out of date, invalidating machine list cache"));
      QFile f(qmc2Config->value("MESS/FilesAndDirectories/GamelistCacheFile").toString());
#endif
      f.remove();
    }
  }
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ROM state info: L:%1 C:%2 M:%3 I:%4 N:%5 U:%6").arg(numTotalGames + numDevices).arg(numCorrectGames).arg(numMostlyCorrectGames).arg(numIncorrectGames).arg(numNotFoundGames).arg(numUnknownGames));
  qmc2MainWindow->progressBarGamelist->reset();

  qmc2ReloadActive = false;
  qmc2StartingUp = false;

  if ( qmc2StopParser ) {
	  if ( loadProc )
		  loadProc->terminate();
  } else {
	  if ( cachedGamesCounter - numDevices != numTotalGames ) {
		  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/AutoTriggerROMCheck").toBool() ) {
			  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: ROM state cache is incomplete or not up to date, triggering an automatic ROM check"));
			  autoRomCheck = true;
		  } else {
			  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: ROM state cache is incomplete or not up to date, please re-check ROMs"));
		  }
	  }
  }

  if ( autoRomCheck )
	  QTimer::singleShot(QMC2_AUTOROMCHECK_DELAY, qmc2MainWindow->actionCheckROMs, SLOT(trigger()));
  else if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/EnableRomStateFilter", true).toBool() )
	  filter();

  enableWidgets(true);
}

void Gamelist::filter()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::filter()");
#endif

  if ( qmc2FilterActive ) {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ROM state filter already active"));
    return;
  }

  if ( qmc2VerifyActive ) {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("please wait for ROM verification to finish and try again"));
    return;
  }

  if ( qmc2ReloadActive ) {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("please wait for reload to finish and try again"));
    return;
  }

  QTime elapsedTime;
  qmc2StopParser = false;
  parseTimer.start();
  qmc2FilterActive = true;
  enableWidgets(false);
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("applying ROM state filter"));
  qmc2MainWindow->progressBarGamelist->reset();
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
    qmc2MainWindow->progressBarGamelist->setFormat(tr("State filter - %p%"));
  else
    qmc2MainWindow->progressBarGamelist->setFormat("%p%");

  qmc2MainWindow->progressBarGamelist->setRange(0, numGames - 1);

  bool showC = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowC").toBool();
  bool showM = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowM").toBool();
  bool showI = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowI").toBool();
  bool showN = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowN").toBool();
  bool showU = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowU").toBool();
  bool showDeviceSets = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowDeviceSets", true).toBool();
  bool showBiosSets = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowBiosSets", true).toBool();

  QMapIterator<QString, QTreeWidgetItem *> it(qmc2GamelistItemMap);
  int i = 0;
  int filterResponse = numGames / QMC2_STATEFILTER_UPDATES;
  qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(false);
  while ( it.hasNext() && !qmc2StopParser ) {
    if ( i++ % filterResponse == 0 ) {
      qmc2MainWindow->progressBarGamelist->setValue(i);
      qApp->processEvents();
    }
    it.next();
    QTreeWidgetItem *item = it.value();
    if ( item ) {
      QString gameName = item->text(QMC2_GAMELIST_COLUMN_NAME);
      if ( !showBiosSets && qmc2BiosROMs.contains(gameName) )
        item->setHidden(true);
      else if ( !showDeviceSets && qmc2DeviceROMs.contains(gameName) )
        item->setHidden(true);
      else switch ( item->whatsThis(QMC2_GAMELIST_COLUMN_GAME).at(0).toAscii() ) {
        case QMC2_ROMSTATE_CHAR_C:
          item->setHidden(!showC);
          break;
        case QMC2_ROMSTATE_CHAR_M:
          item->setHidden(!showM);
          break;
        case QMC2_ROMSTATE_CHAR_I:
          item->setHidden(!showI);
          break;
        case QMC2_ROMSTATE_CHAR_N:
          item->setHidden(!showN);
          break;
        case QMC2_ROMSTATE_CHAR_U:
        default:
          item->setHidden(!showU);
          break;
      }
    }
  }
  qmc2MainWindow->progressBarGamelist->setValue(numGames - 1);
  qApp->processEvents();
  qmc2MainWindow->scrollToCurrentItem();
  qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(true);
  qmc2FilterActive = false;
  elapsedTime = elapsedTime.addMSecs(parseTimer.elapsed());
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (applying ROM state filter, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
  enableWidgets(true);
  qmc2StatesTogglesEnabled = true;
  QTimer::singleShot(0, qmc2MainWindow->progressBarGamelist, SLOT(reset()));
}

void Gamelist::save()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::save()");
#endif

#if defined(QMC2_EMUTYPE_MAME)
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("saving game list"));
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (saving game list)"));
#elif defined(QMC2_EMUTYPE_MESS)
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("saving machine list"));
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (saving machine list)"));
#endif
}

void Gamelist::loadFavorites()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::loadFavorites()");
#endif

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading favorites"));

  qmc2MainWindow->listWidgetFavorites->clear();
#if defined(QMC2_EMUTYPE_MAME)
  QFile f(qmc2Config->value("MAME/FilesAndDirectories/FavoritesFile").toString());
#elif defined(QMC2_EMUTYPE_MESS)
  QFile f(qmc2Config->value("MESS/FilesAndDirectories/FavoritesFile").toString());
#endif
  if ( f.open(QIODevice::ReadOnly | QIODevice::Text) ) {
    QTextStream ts(&f);
    while ( !ts.atEnd() ) {
      QString gameName = ts.readLine();
      if ( !gameName.isEmpty() ) {
        QTreeWidgetItem *gameItem = qmc2GamelistItemMap[gameName];
        if ( gameItem ) {
          QListWidgetItem *item = new QListWidgetItem(qmc2MainWindow->listWidgetFavorites);
          item->setText(gameItem->text(QMC2_GAMELIST_COLUMN_GAME));
        }
      }
    }
    f.close();
  }

  qmc2MainWindow->listWidgetFavorites->sortItems();
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading favorites)"));
  if ( qmc2MainWindow->tabWidgetGamelist->currentIndex() == QMC2_FAVORITES_INDEX )
    QTimer::singleShot(0, qmc2MainWindow, SLOT(checkCurrentFavoritesSelection()));
  else
    qmc2MainWindow->listWidgetFavorites->setCurrentIndex(QModelIndex());
}

void Gamelist::saveFavorites()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::saveFavorites()");
#endif

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("saving favorites"));

#if defined(QMC2_EMUTYPE_MAME)
  QFile f(qmc2Config->value("MAME/FilesAndDirectories/FavoritesFile").toString());
#elif defined(QMC2_EMUTYPE_MESS)
  QFile f(qmc2Config->value("MESS/FilesAndDirectories/FavoritesFile").toString());
#endif
  if ( f.open(QIODevice::WriteOnly | QIODevice::Text) ) {
    QTextStream ts(&f);
    int i;
    for (i = 0; i < qmc2MainWindow->listWidgetFavorites->count(); i++) {
      ts << qmc2GamelistNameMap[qmc2MainWindow->listWidgetFavorites->item(i)->text()] << "\n";
    }
    f.close();
  } else {
#if defined(QMC2_EMUTYPE_MAME)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open favorites file for writing, path = %1").arg(qmc2Config->value("MAME/FilesAndDirectories/FavoritesFile").toString()));
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open favorites file for writing, path = %1").arg(qmc2Config->value("MESS/FilesAndDirectories/FavoritesFile").toString()));
#endif
  }

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (saving favorites)"));
}

void Gamelist::loadPlayHistory()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::loadPlayHistory()");
#endif

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading play history"));

  qmc2MainWindow->listWidgetPlayed->clear();
#if defined(QMC2_EMUTYPE_MAME)
  QFile f(qmc2Config->value("MAME/FilesAndDirectories/HistoryFile").toString());
#elif defined(QMC2_EMUTYPE_MESS)
  QFile f(qmc2Config->value("MESS/FilesAndDirectories/HistoryFile").toString());
#endif
  if ( f.open(QIODevice::ReadOnly | QIODevice::Text) ) {
    QTextStream ts(&f);
    while ( !ts.atEnd() ) {
      QString gameName = ts.readLine();
      if ( !gameName.isEmpty() ) {
        QTreeWidgetItem *gameItem = qmc2GamelistItemMap[gameName];
        if ( gameItem ) {
          QListWidgetItem *item = new QListWidgetItem(qmc2MainWindow->listWidgetPlayed);
          item->setText(gameItem->text(QMC2_GAMELIST_COLUMN_GAME));
        }
      }
    }
    f.close();
  }

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading play history)"));
  if ( qmc2MainWindow->tabWidgetGamelist->currentIndex() == QMC2_PLAYED_INDEX )
    QTimer::singleShot(0, qmc2MainWindow, SLOT(checkCurrentPlayedSelection()));
  else
    qmc2MainWindow->listWidgetPlayed->setCurrentIndex(QModelIndex());
}

void Gamelist::savePlayHistory()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::savePlayHistory()");
#endif

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("saving play history"));

#if defined(QMC2_EMUTYPE_MAME)
  QFile f(qmc2Config->value("MAME/FilesAndDirectories/HistoryFile").toString());
#elif defined(QMC2_EMUTYPE_MESS)
  QFile f(qmc2Config->value("MESS/FilesAndDirectories/HistoryFile").toString());
#endif
  if ( f.open(QIODevice::WriteOnly | QIODevice::Text) ) {
    QTextStream ts(&f);
    int i;
    for (i = 0; i < qmc2MainWindow->listWidgetPlayed->count(); i++) {
      ts << qmc2GamelistNameMap[qmc2MainWindow->listWidgetPlayed->item(i)->text()] << "\n";
    }
    f.close();
  } else {
#if defined(QMC2_EMUTYPE_MAME)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open play history file for writing, path = %1").arg(qmc2Config->value("MAME/FilesAndDirectories/HistoryFile").toString()));
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open play history file for writing, path = %1").arg(qmc2Config->value("MESS/FilesAndDirectories/HistoryFile").toString()));
#endif
  }

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (saving play history)"));
}

QString Gamelist::status()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::status()");
#endif

  QLocale locale;
  QString statusString = "<b>";
  statusString += "<font color=black>" + tr("L:") + QString(numGames > -1 ? locale.toString(numGames) : "?") + "</font>\n";
  statusString += "<font color=#00cc00>" + tr("C:") + QString(numCorrectGames > -1 ? locale.toString(numCorrectGames) : "?") + "</font>\n";
  statusString += "<font color=#a2c743>" + tr("M:") + QString(numMostlyCorrectGames > -1 ? locale.toString(numMostlyCorrectGames) : "?") + "</font>\n";
  statusString += "<font color=#f90000>" + tr("I:") + QString(numIncorrectGames > -1 ? locale.toString(numIncorrectGames) : "?") + "</font>\n";
  statusString += "<font color=#7f7f7f>" + tr("N:") + QString(numNotFoundGames > -1 ? locale.toString(numNotFoundGames) : "?") + "</font>\n";
  statusString += "<font color=#0000f9>" + tr("U:") + QString(numUnknownGames > -1 ? locale.toString(numUnknownGames) : "?") + "</font>\n";
  statusString += "<font color=chocolate>" + tr("S:") + QString(numSearchGames > -1 ? locale.toString(numSearchGames) : "?") + "</font>\n";
  statusString += "<font color=sandybrown>" + tr("T:") + QString(numTaggedSets > -1 ? locale.toString(numTaggedSets) : "?") + "</font>";
  statusString += "</b>";

  return statusString;
}

void Gamelist::loadStarted()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::loadStarted()");
#endif

  qmc2MainWindow->progressBarGamelist->setRange(0, numTotalGames);
  qmc2MainWindow->progressBarGamelist->reset();
}

void Gamelist::loadFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Gamelist::loadFinished(int exitCode = %1, QProcess::ExitStatus exitStatus = %2): proc = %3").arg(exitCode).arg(exitStatus).arg((qulonglong)loadProc));
#endif

  if ( exitStatus != QProcess::NormalExit && !qmc2StopParser )
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: emulator audit call didn't exit cleanly -- exitCode = %1, exitStatus = %2").arg(exitCode).arg(QString(exitStatus == QProcess::NormalExit ? tr("normal") : tr("crashed"))));

  QTime elapsedTime;
  elapsedTime = elapsedTime.addMSecs(loadTimer.elapsed());
#if defined(QMC2_EMUTYPE_MAME)
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading XML game list data and (re)creating cache, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
#elif defined(QMC2_SDLMAME) || defined(QMC2_MESS)
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading XML machine list data and (re)creating cache, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
#endif
  qmc2MainWindow->progressBarGamelist->reset();
  qmc2EarlyReloadActive = false;
  if ( loadProc )
    delete loadProc;
  loadProc = NULL;

  if ( romCache.isOpen() )
    romCache.close();

  if ( listXMLCache.isOpen() )
    listXMLCache.close();

  parse();
  loadFavorites();
  loadPlayHistory();

  // show game list
  qmc2MainWindow->labelLoadingGamelist->setVisible(false);
  qmc2MainWindow->treeWidgetGamelist->setVisible(true);
  qmc2MainWindow->labelLoadingHierarchy->setVisible(false);
  qmc2MainWindow->treeWidgetHierarchy->setVisible(true);

  if ( qmc2MainWindow->tabWidgetGamelist->currentIndex() == QMC2_GAMELIST_INDEX ) {
	  switch ( qmc2MainWindow->stackedWidgetView->currentIndex() ) {
		  case QMC2_VIEW_DETAIL_INDEX:
			  qmc2MainWindow->treeWidgetGamelist->setFocus();
		      	  break;
    		  case QMC2_VIEW_TREE_INDEX:
			  qmc2MainWindow->treeWidgetHierarchy->setFocus();
			  break;
#if defined(QMC2_EMUTYPE_MAME)
    		  case QMC2_VIEW_CATEGORY_INDEX:
			  qmc2MainWindow->treeWidgetCategoryView->setFocus();
    			  break;
    		  case QMC2_VIEW_VERSION_INDEX:
    			  qmc2MainWindow->treeWidgetVersionView->setFocus();
    			  break;
#endif
    		  default:
    			  qmc2MainWindow->treeWidgetGamelist->setFocus();
    			  break;
    	  }
  }

  qApp->processEvents();
}

void Gamelist::loadReadyReadStandardOutput()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Gamelist::loadReadyReadStandardOutput(): proc = %1)").arg((qulonglong)loadProc));
#endif

  QString s = loadProc->readAllStandardOutput();
  bool endsWithSpace = s.endsWith(" ");
  bool startWithSpace = s.startsWith(" ");

  s = s.simplified();
  if ( startWithSpace )
    s.prepend(" ");

  // ensure XML elements are on individual lines
  int i;
  for (i = 0; i < s.length(); i++) {
    if ( s[i] == '>' )
      if ( i + 1 < s.length() ) {
        if ( s[i + 1] == '<' )
          s.insert(i + 1, "\n");
        else if ( s[i + 1] == ' ' )
          if ( i + 2 < s.length() )
            if ( s[i + 2] == '<' )
              s.replace(i + 1, 1, "\n");
      }
  }

  QStringList sl = s.split("\n");
  int l, lc = sl.count();
  for (l = 0; l < lc; l++) {
    QString singleXMLLine = sl[l];
    if ( !singleXMLLine.startsWith("<!") && !singleXMLLine.startsWith("<?") && !singleXMLLine.startsWith("]>") ) {
      bool newLine = singleXMLLine.endsWith(">");
      if ( newLine ) {
        if ( singleXMLLine.endsWith("<description>") )
          newLine = false;
        else if ( singleXMLLine.endsWith("<year>") )
          newLine = false;
        else if ( singleXMLLine.endsWith("<manufacturer>") )
          newLine = false;
        if ( newLine ) {
          bool found = false;
          for (i = singleXMLLine.length() - 2; i > 0 && !found; i--)
            found = ( singleXMLLine[i] == '<' );
          if ( found && i == 0 )
            newLine = false;
        }
      }
      bool needsSpace = singleXMLLine.endsWith("\"");
      if ( needsSpace ) {
        bool found = false;
        bool stop = false;
        for (i = singleXMLLine.length() - 2; i > 1 && !found && !stop; i--) {
          if ( singleXMLLine[i] == '\"' ) {
            if ( singleXMLLine[i - 1] == '=' )
              found = true;
            else
              stop = true;
          }
        }
        if ( !found )
          needsSpace = false;
      }
      needsSpace |= endsWithSpace;
      if ( newLine ) {
        if ( singleXMLLine[singleXMLLine.length() - 1].isSpace() )
          singleXMLLine.remove(singleXMLLine.length() - 1, 1);
        needsSpace = false;
      }
      gamelistBuffer += singleXMLLine + QString(needsSpace ? " " : "") + QString(newLine ? "\n" : "");
      if ( listXMLCache.isOpen() )
        tsListXMLCache << singleXMLLine << QString(needsSpace ? " " : "") << QString(newLine ? "\n" : "");
    }
  }

#if defined(QMC2_EMUTYPE_MAME)
  qmc2MainWindow->progressBarGamelist->setValue(qmc2MainWindow->progressBarGamelist->value() + s.count("<game name="));
#elif defined(QMC2_SDLMAME) || defined(QMC2_MESS)
  qmc2MainWindow->progressBarGamelist->setValue(qmc2MainWindow->progressBarGamelist->value() + s.count("<machine name="));
#endif
}

void Gamelist::loadError(QProcess::ProcessError processError)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::loadError(QProcess::ProcessError processError = " + QString::number(processError) + ")");
#endif

}

void Gamelist::loadStateChanged(QProcess::ProcessState processState)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::loadStateChanged(QProcess::ProcessState = " + QString::number(processState) + ")");
#endif

}

void Gamelist::verifyStarted()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::verifyStarted()");
#endif

}

void Gamelist::verifyFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Gamelist::verifyFinished(int exitCode = %1, QProcess::ExitStatus exitStatus = %2): proc = %3").arg(exitCode).arg(exitStatus).arg((qulonglong)verifyProc));
#endif

  if ( !verifyProc->atEnd() )
	  verifyReadyReadStandardOutput();

  bool cleanExit = true;
  if ( exitStatus != QProcess::NormalExit && !qmc2StopParser ) {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: emulator audit call didn't exit cleanly -- exitCode = %1, exitStatus = %2").arg(exitCode).arg(QString(exitStatus == QProcess::NormalExit ? tr("normal") : tr("crashed"))));
    cleanExit = false;
  }

  bool showROMStatusIcons = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowROMStatusIcons", true).toBool();
  if ( !verifyCurrentOnly ) {
    QSet<QString> gameSet = QSet<QString>::fromList(qmc2GamelistItemMap.uniqueKeys());
    QList<QString> remainingGames = gameSet.subtract(QSet<QString>::fromList(verifiedList)).values();
    int i;
    if ( qmc2StopParser || !cleanExit ) {
      for (i = 0; i < remainingGames.count(); i++) {
        qmc2MainWindow->progressBarGamelist->setValue(qmc2MainWindow->progressBarGamelist->value() + 1);
        QString gameName = remainingGames[i];
        bool isBIOS = qmc2BiosROMs.contains(gameName);
        bool isDevice = qmc2DeviceROMs.contains(gameName);
        QTreeWidgetItem *romItem = qmc2GamelistItemMap[gameName];
        QTreeWidgetItem *hierarchyItem = qmc2HierarchyItemMap[gameName];
#if defined(QMC2_EMUTYPE_MAME)
	QTreeWidgetItem *categoryItem = qmc2CategoryItemMap[gameName];
	QTreeWidgetItem *versionItem = qmc2VersionItemMap[gameName];
#endif
        if ( romItem && hierarchyItem ) {
          if ( romCache.isOpen() )
            tsRomCache << gameName << " U\n";
          numUnknownGames++;
          if ( isBIOS ) {
            if ( showROMStatusIcons ) {
              romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownBIOSImageIcon);
              hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownBIOSImageIcon);
#if defined(QMC2_EMUTYPE_MAME)
	      if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownBIOSImageIcon);
	      if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownBIOSImageIcon);
#endif
	    }
          } else if ( isDevice ) {
            if ( showROMStatusIcons ) {
              romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownDeviceImageIcon);
              hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownDeviceImageIcon);
#if defined(QMC2_EMUTYPE_MAME)
	      if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownDeviceImageIcon);
	      if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownDeviceImageIcon);
#endif
	    }
          } else {
            if ( showROMStatusIcons ) {
              romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownImageIcon);
              hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownImageIcon);
#if defined(QMC2_EMUTYPE_MAME)
	      if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownImageIcon);
	      if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownImageIcon);
#endif
            }
          }
          romItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_U);
          hierarchyItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_U);
        } else {
          qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: can't find item map entry for '%1' - ROM state cannot be determined").arg(gameName));
          if ( romCache.isOpen() )
            tsRomCache << gameName << " U\n";
          numUnknownGames++;
        }
      }
    } else {
      QMap<QString, int> systemPosMap;
      for (i = 0; i < remainingGames.count(); i++) {
        qmc2MainWindow->progressBarGamelist->setValue(qmc2MainWindow->progressBarGamelist->value() + 1);
        QString gameName = remainingGames[i];
        bool isBIOS = qmc2BiosROMs.contains(gameName);
        bool isDevice = qmc2DeviceROMs.contains(gameName);
        QTreeWidgetItem *romItem = qmc2GamelistItemMap[gameName];
        QTreeWidgetItem *hierarchyItem = qmc2HierarchyItemMap[gameName];
#if defined(QMC2_EMUTYPE_MAME)
	QTreeWidgetItem *categoryItem = qmc2CategoryItemMap[gameName];
	QTreeWidgetItem *versionItem = qmc2VersionItemMap[gameName];
#endif
	// there are a number of machines in MESS and some in MAME that don't require any ROMs...
	bool romRequired = true;
	int xmlCounter = 0;
	bool xmlFound = false;
	if ( systemPosMap.contains(gameName) ) {
		xmlCounter = systemPosMap[gameName];
		xmlFound = true;
	} else {
#if defined(QMC2_EMUTYPE_MESS)
		while ( !xmlFound && xmlCounter < xmlLines.count() ) {
			xmlFound = (xmlLines[xmlCounter].contains(QString("<machine name=\"%1\"").arg(gameName)));
			if ( !xmlFound ) {
				if ( xmlLines[xmlCounter].contains("<machine name=\"") ) {
					QString xmlLine = xmlLines[xmlCounter];
					int gameNamePos = xmlLine.indexOf("machine name=\"") + 14;
					QString currentGame = xmlLine.mid(gameNamePos, xmlLine.indexOf("\"", gameNamePos) - gameNamePos);
					systemPosMap[currentGame] = xmlCounter + 1;
				}
			}
			xmlCounter++;
		}
#elif defined(QMC2_EMUTYPE_MAME)
		while ( !xmlFound && xmlCounter < xmlLines.count() ) {
			xmlFound = (xmlLines[xmlCounter].contains(QString("<game name=\"%1\"").arg(gameName)));
			if ( !xmlFound ) {
				if ( xmlLines[xmlCounter].contains("<game name=\"") ) {
					QString xmlLine = xmlLines[xmlCounter];
					int gameNamePos = xmlLine.indexOf("game name=\"") + 11;
					QString currentGame = xmlLine.mid(gameNamePos, xmlLine.indexOf("\"", gameNamePos) - gameNamePos);
					systemPosMap[currentGame] = xmlCounter + 1;
				}
			}
			xmlCounter++;
		}
#endif
	}
	if ( xmlFound ) {
		int romCounter = 0;
		bool endFound = false;
		while ( !endFound && xmlCounter < xmlLines.count() ) {
#if defined(QMC2_EMUTYPE_MESS)
			if ( xmlLines[xmlCounter].contains("<rom name=\"") ) {
				romCounter++;
				endFound = true;
			} else if ( xmlLines[xmlCounter].contains("</machine>") )
				endFound = true;
#elif defined(QMC2_EMUTYPE_MAME)
			if ( xmlLines[xmlCounter].contains("<rom name=\"") ) {
				romCounter++;
				endFound = true;
			} else if ( xmlLines[xmlCounter].contains("</game>") )
				endFound = true;
#endif
			xmlCounter++;
		}
		romRequired = (romCounter > 0);
	}
        if ( romItem && hierarchyItem ) {
	  if ( romCache.isOpen() ) {
		  if ( romRequired ) {
			  tsRomCache << gameName << " N\n";
			  numNotFoundGames++;
		  } else {
			  tsRomCache << gameName << " C\n";
			  numCorrectGames++;
		  }
	  }
          if ( isBIOS ) {
            if ( showROMStatusIcons ) {
              if ( romRequired ) {
                romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundBIOSImageIcon);
                hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundBIOSImageIcon);
#if defined(QMC2_EMUTYPE_MAME)
	        if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundBIOSImageIcon);
	        if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundBIOSImageIcon);
#endif
              } else {
                romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectBIOSImageIcon);
                hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectBIOSImageIcon);
#if defined(QMC2_EMUTYPE_MAME)
	        if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectBIOSImageIcon);
	        if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectBIOSImageIcon);
#endif
              }
            }
          } else if ( isDevice ) {
              if ( romRequired ) {
                romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundDeviceImageIcon);
                hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundDeviceImageIcon);
#if defined(QMC2_EMUTYPE_MAME)
	        if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundDeviceImageIcon);
	        if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundDeviceImageIcon);
#endif
              } else {
                romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectDeviceImageIcon);
                hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectDeviceImageIcon);
#if defined(QMC2_EMUTYPE_MAME)
	        if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectDeviceImageIcon);
	        if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectDeviceImageIcon);
#endif
              }
          } else {
            if ( showROMStatusIcons ) {
              if ( romRequired ) {
                romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundImageIcon);
                hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundImageIcon);
#if defined(QMC2_EMUTYPE_MAME)
                if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundImageIcon);
                if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundImageIcon);
#endif
              } else {
                romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectImageIcon);
                hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectImageIcon);
#if defined(QMC2_EMUTYPE_MAME)
                if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectImageIcon);
                if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectImageIcon);
#endif
              }
            }
          }
	  if ( romRequired ) {
		  romItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_N);
		  hierarchyItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_N);
	  } else {
		  romItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_C);
		  hierarchyItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_C);
	  }
        } else {
          qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: can't find item map entry for '%1' - ROM state cannot be determined").arg(gameName));
          if ( romCache.isOpen() )
            tsRomCache << gameName << " U\n";
          numUnknownGames++;
        }
      }
    }
    qmc2MainWindow->labelGamelistStatus->setText(status());
  }

  if ( verifyCurrentOnly && romCache.isOpen() ) {
    QMapIterator<QString, QTreeWidgetItem *> it(qmc2GamelistItemMap);
    while ( it.hasNext() ) {
      it.next();
      QTreeWidgetItem *item = it.value();
      QString gameID = it.key();
      if ( !item || gameID.isEmpty() ) continue;
      tsRomCache << gameID << " ";
      switch ( item->whatsThis(QMC2_GAMELIST_COLUMN_GAME)[0].toAscii() ) {
        case QMC2_ROMSTATE_CHAR_C:
          tsRomCache << "C\n";
          break;
        case QMC2_ROMSTATE_CHAR_M:
          tsRomCache << "M\n";
          break;
        case QMC2_ROMSTATE_CHAR_U:
          tsRomCache << "U\n";
          break;
        case QMC2_ROMSTATE_CHAR_I:
          tsRomCache << "I\n";
          break;
        case QMC2_ROMSTATE_CHAR_N:
          tsRomCache << "N\n";
          break;
      }
    }
  }

  QTime elapsedTime;
  elapsedTime = elapsedTime.addMSecs(verifyTimer.elapsed());
  if ( verifyCurrentOnly )
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (verifying ROM status for '%1', elapsed time = %2)").arg(checkedItem->text(QMC2_GAMELIST_COLUMN_GAME)).arg(elapsedTime.toString("mm:ss.zzz")));
  else {
#if defined(QMC2_EMUTYPE_MAME)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (verifying ROM status for all games, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (verifying ROM status for all machines, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
#endif
  }
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ROM state info: L:%1 C:%2 M:%3 I:%4 N:%5 U:%6").arg(numTotalGames + numDevices).arg(numCorrectGames).arg(numMostlyCorrectGames).arg(numIncorrectGames).arg(numNotFoundGames).arg(numUnknownGames));
  qmc2MainWindow->progressBarGamelist->reset();
  qmc2VerifyActive = false;
  if ( verifyProc )
    delete verifyProc;
  verifyProc = NULL;

  if ( romCache.isOpen() ) {
    tsRomCache.flush();
    romCache.close();
  }

  if ( qmc2SortCriteria == QMC2_SORT_BY_ROM_STATE ) {
#if defined(QMC2_EMUTYPE_MAME)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("sorting game list by %1 in %2 order").arg(tr("ROM state")).arg(qmc2SortOrder == Qt::AscendingOrder ? tr("ascending") : tr("descending")));
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("sorting machine list by %1 in %2 order").arg(tr("ROM state")).arg(qmc2SortOrder == Qt::AscendingOrder ? tr("ascending") : tr("descending")));
#endif
    qApp->processEvents();
    QList<QTreeWidgetItem *> itemList = qmc2MainWindow->treeWidgetGamelist->findItems("*", Qt::MatchContains | Qt::MatchWildcard);
    for (int i = 0; i < itemList.count(); i++) {
      if ( itemList[i]->childCount() > 1 ) {
        qmc2MainWindow->treeWidgetGamelist->collapseItem(itemList[i]);
        QList<QTreeWidgetItem *> childrenList = itemList[i]->takeChildren();
        int j;
        for (j = 0; j < childrenList.count(); j++)
          delete childrenList[j];
        QTreeWidgetItem *nameItem = new QTreeWidgetItem(itemList[i]);
        nameItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Waiting for data..."));
        nameItem->setText(QMC2_GAMELIST_COLUMN_ICON, qmc2GamelistNameMap[itemList[i]->text(QMC2_GAMELIST_COLUMN_GAME)]);
        qApp->processEvents();
      }
    }
    qApp->processEvents();
    qmc2MainWindow->treeWidgetGamelist->sortItems(qmc2MainWindow->sortCriteriaLogicalIndex(), qmc2SortOrder);
    QTreeWidgetItem *ci = qmc2MainWindow->treeWidgetGamelist->currentItem();
    if ( ci )
      QTimer::singleShot(0, qmc2MainWindow, SLOT(scrollToCurrentItem()));
  }

  enableWidgets(true);

  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/EnableRomStateFilter", true).toBool() ) filter();
}

void Gamelist::verifyReadyReadStandardOutput()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Gamelist::verifyReadyReadStandardOutput(): proc = %1").arg((qulonglong) verifyProc));
#endif

  // process rom verification output
  int i;
  QString romName, romState, romStateLong; 
  QString s = verifyLastLine + verifyProc->readAllStandardOutput();
#if defined(Q_WS_WIN)
  s.replace("\r\n", "\n"); // convert WinDOS's "0x0D 0x0A" to just "0x0A" 
#endif
  QStringList lines = s.split("\n");

  if ( s.endsWith("\n") ) {
    verifyLastLine.clear();
  } else {
    verifyLastLine = lines.last();
    lines.removeLast();
  }

  bool showROMStatusIcons = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowROMStatusIcons", true).toBool();
  for (i = 0; i < lines.count(); i++) {
    if ( lines[i].startsWith("romset ") ) {
      QStringList words = lines[i].split(" ");
      numVerifyRoms++;
      if ( words.count() > 2 ) {
        romName = words[1].remove("\"");
        bool isBIOS = qmc2BiosROMs.contains(romName);
        bool isDevice = qmc2DeviceROMs.contains(romName);
        if ( qmc2GamelistItemMap.count(romName) == 1 ) {
          QTreeWidgetItem *romItem = qmc2GamelistItemMap[romName];
          QTreeWidgetItem *hierarchyItem = qmc2HierarchyItemMap[romName];
#if defined(QMC2_EMUTYPE_MAME)
          QTreeWidgetItem *categoryItem = qmc2CategoryItemMap[romName];
          QTreeWidgetItem *versionItem = qmc2VersionItemMap[romName];
#endif
          if ( romItem && hierarchyItem ) {
#if defined(QMC2_SDLMAME) || defined(QMC2_SDLMESS) || defined(QMC2_MAME) || defined(QMC2_MESS)
            if ( words.last() == "good" || lines[i].endsWith("has no roms!") ) {
              romState = "C";
              romStateLong = QObject::tr("correct");
              numCorrectGames++;
              if ( isBIOS ) {
                if ( showROMStatusIcons ) {
                  romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectBIOSImageIcon);
                  hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectBIOSImageIcon);
#if defined(QMC2_EMUTYPE_MAME)
                  if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectBIOSImageIcon);
                  if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectBIOSImageIcon);
#endif
                }
              } else if ( isDevice ) {
                if ( showROMStatusIcons ) {
                  romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectDeviceImageIcon);
                  hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectDeviceImageIcon);
#if defined(QMC2_EMUTYPE_MAME)
                  if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectDeviceImageIcon);
                  if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectDeviceImageIcon);
#endif
                }
              } else {
                if ( showROMStatusIcons ) {
                  romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectImageIcon);
                  hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectImageIcon);
#if defined(QMC2_EMUTYPE_MAME)
                  if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectImageIcon);
                  if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectImageIcon);
#endif
                }
              }
              romItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_C);
              hierarchyItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_C);
	      if ( romItem == qmc2CurrentItem ) qmc2MainWindow->labelGameStatus->setPalette(MainWindow::qmc2StatusColorGreen);
            } else if ( words.last() == "bad" ) {
              romState = "I";
              romStateLong = QObject::tr("incorrect");
              numIncorrectGames++;
              if ( isBIOS ) {
                if ( showROMStatusIcons ) {
                  romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectBIOSImageIcon);
                  hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectBIOSImageIcon);
#if defined(QMC2_EMUTYPE_MAME)
                  if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectBIOSImageIcon);
                  if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectBIOSImageIcon);
#endif
                }
              } else if ( isDevice ) {
                if ( showROMStatusIcons ) {
                  romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectDeviceImageIcon);
                  hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectDeviceImageIcon);
#if defined(QMC2_EMUTYPE_MAME)
                  if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectDeviceImageIcon);
                  if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectDeviceImageIcon);
#endif
                }
              } else {
                if ( showROMStatusIcons ) {
                  romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectImageIcon);
                  hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectImageIcon);
#if defined(QMC2_EMUTYPE_MAME)
                  if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectImageIcon);
                  if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectImageIcon);
#endif
                }
              }
              romItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_I);
              hierarchyItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_I);
	      if ( romItem == qmc2CurrentItem ) qmc2MainWindow->labelGameStatus->setPalette(MainWindow::qmc2StatusColorRed);
            } else if ( words.last() == "available" ) {
              romState = "M";
              romStateLong = QObject::tr("mostly correct");
              numMostlyCorrectGames++;
              if ( isBIOS ) {
                if ( showROMStatusIcons ) {
                  romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectBIOSImageIcon);
                  hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectBIOSImageIcon);
#if defined(QMC2_EMUTYPE_MAME)
                  if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectBIOSImageIcon);
                  if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectBIOSImageIcon);
#endif
                }
              } else if ( isDevice ) {
                if ( showROMStatusIcons ) {
                  romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectDeviceImageIcon);
                  hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectDeviceImageIcon);
#if defined(QMC2_EMUTYPE_MAME)
                  if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectDeviceImageIcon);
                  if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectDeviceImageIcon);
#endif
                }
              } else {
                if ( showROMStatusIcons ) {
                  romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectImageIcon);
                  hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectImageIcon);
#if defined(QMC2_EMUTYPE_MAME)
                  if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectImageIcon);
                  if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectImageIcon);
#endif
                }
              }
              romItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_M);
              hierarchyItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_M);
	      if ( romItem == qmc2CurrentItem ) qmc2MainWindow->labelGameStatus->setPalette(MainWindow::qmc2StatusColorYellowGreen);
            } else if ( words.last() == "missing" || words.last() == "found!" ) {
              romState = "N";
              romStateLong = QObject::tr("not found");
              numNotFoundGames++;
              if ( isBIOS ) {
                if ( showROMStatusIcons ) {
                  romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundBIOSImageIcon);
                  hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundBIOSImageIcon);
#if defined(QMC2_EMUTYPE_MAME)
                  if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundBIOSImageIcon);
                  if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundBIOSImageIcon);
#endif
                }
              } else if ( isDevice ) {
                if ( showROMStatusIcons ) {
                  romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundDeviceImageIcon);
                  hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundDeviceImageIcon);
#if defined(QMC2_EMUTYPE_MAME)
                  if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundDeviceImageIcon);
                  if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundDeviceImageIcon);
#endif
                }
              } else {
                if ( showROMStatusIcons ) {
                  romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundImageIcon);
                  hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundImageIcon);
#if defined(QMC2_EMUTYPE_MAME)
                  if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundImageIcon);
                  if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundImageIcon);
#endif
                }
              }
              romItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_N);
              hierarchyItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_N);
	      if ( romItem == qmc2CurrentItem ) qmc2MainWindow->labelGameStatus->setPalette(MainWindow::qmc2StatusColorGrey);
            } else {
              romState = "U";
              romStateLong = QObject::tr("unknown");
              numUnknownGames++;
              if ( isBIOS ) {
                if ( showROMStatusIcons ) {
                  romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownBIOSImageIcon);
                  hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownBIOSImageIcon);
#if defined(QMC2_EMUTYPE_MAME)
                  if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownBIOSImageIcon);
                  if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownBIOSImageIcon);
#endif
                }
              } else if ( isDevice ) {
                if ( showROMStatusIcons ) {
                  romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownDeviceImageIcon);
                  hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownDeviceImageIcon);
#if defined(QMC2_EMUTYPE_MAME)
                  if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownDeviceImageIcon);
                  if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownDeviceImageIcon);
#endif
                }
              } else {
                if ( showROMStatusIcons ) {
                  romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownImageIcon);
                  hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownImageIcon);
#if defined(QMC2_EMUTYPE_MAME)
                  if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownImageIcon);
                  if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownImageIcon);
#endif
                }
              }
              romItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_U);
              hierarchyItem->setWhatsThis(QMC2_GAMELIST_COLUMN_GAME, QMC2_ROMSTATE_STRING_U);
	      if ( romItem == qmc2CurrentItem ) qmc2MainWindow->labelGameStatus->setPalette(MainWindow::qmc2StatusColorBlue);
            }
#endif
          } else {
            qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: can't find item map entry for '%1' - ROM state cannot be determined").arg(romName));
            romState = "U";
            romStateLong = QObject::tr("unknown");
            numUnknownGames++;
          }

#ifdef QMC2_DEBUG
          qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::verifyReadyReadStandardOutput(): " + romName + " " + romState);
#endif

          qmc2GamelistStatusMap[romName] = romState;

          verifiedList << romName;

          if ( verifyCurrentOnly ) {
            qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ROM status for '%1' is '%2'").arg(checkedItem->text(QMC2_GAMELIST_COLUMN_GAME)).arg(romStateLong));
            numUnknownGames--;
          } else if ( romCache.isOpen() )
            tsRomCache << romName << " " << romState << "\n";
        }
      }
    }
  }

  if ( romCache.isOpen() && !verifyCurrentOnly )
    tsRomCache.flush();

  if ( qmc2StopParser && verifyProc )
    verifyProc->terminate();

  qmc2MainWindow->progressBarGamelist->setValue(numVerifyRoms);
  qmc2MainWindow->labelGamelistStatus->setText(status());
}

void Gamelist::verifyError(QProcess::ProcessError processError)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::verifyError(QProcess::ProcessError processError = " + QString::number(processError) + ")");
#endif

}

void Gamelist::verifyStateChanged(QProcess::ProcessState processState)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::verifyStateChanged(QProcess::ProcessState = " + QString::number(processState) + ")");
#endif

}

bool Gamelist::loadIcon(QString gameName, QTreeWidgetItem *item, bool checkOnly, QString *fileName)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Gamelist::loadIcon(QString gameName = %1, QTreeWidgetItem *item = %2, bool checkOnly = %3, QString *fileName = %4)").arg(gameName).arg((qulonglong)item).arg(checkOnly).arg((qulonglong)fileName));
#endif

  QIcon icon;
  char imageBuffer[QMC2_ZIP_BUFFER_SIZE];

  if ( fileName )
    *fileName = gameName;

  if ( qmc2IconMap.contains(gameName) ) {
    // use cached icon
    if ( !checkOnly )
      item->setIcon(QMC2_GAMELIST_COLUMN_ICON, qmc2IconMap.value(gameName));
    else
      qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(true);

    return true;
  } else if ( qmc2IconsPreloaded ) {
    // icon wasn't found
    if ( !checkOnly ) {
      icon = QIcon();
      qmc2IconMap[gameName] = icon;
      item->setIcon(QMC2_GAMELIST_COLUMN_ICON, icon);
    } else
      qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(true);

    return false;
  }

  if ( qmc2UseIconFile ) {
    // use icon file
    QByteArray imageData;
    int len, i;
    if ( !qmc2IconsPreloaded ) {
      QTime preloadTimer, elapsedTime;
      int iconCount = 0;
      preloadTimer.start();
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("pre-caching icons from ZIP archive"));
      unz_global_info unzGlobalInfo;
      if ( unzGetGlobalInfo(qmc2IconFile, &unzGlobalInfo) == UNZ_OK ) {
        int currentMax = qmc2MainWindow->progressBarGamelist->maximum();
        QString oldFormat = qmc2MainWindow->progressBarGamelist->format();
        if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
          qmc2MainWindow->progressBarGamelist->setFormat(tr("Icon cache - %p%"));
        else
          qmc2MainWindow->progressBarGamelist->setFormat("%p%");
        qmc2MainWindow->progressBarGamelist->setRange(0, unzGlobalInfo.number_entry);
        qmc2MainWindow->progressBarGamelist->reset();
        qApp->processEvents();
        if ( unzGoToFirstFile(qmc2IconFile) == UNZ_OK ) {
          do {
            char unzFileName[QMC2_MAX_PATH_LENGTH];
            iconCount++;
            if ( iconCount % QMC2_ICONCACHE_RESPONSIVENESS == 0 ) {
              qmc2MainWindow->progressBarGamelist->setValue(iconCount);
              qApp->processEvents();
            }
            if ( unzGetCurrentFileInfo(qmc2IconFile, NULL, unzFileName, QMC2_MAX_PATH_LENGTH, NULL, 0, NULL, 0) == UNZ_OK ) {
              QFileInfo fi(unzFileName);
              QString gameFileName = fi.fileName();
#ifdef QMC2_DEBUG
              qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Gamelist::loadIcon(): loading %1").arg(gameFileName));
#endif
              imageData.clear();
              if ( unzOpenCurrentFile(qmc2IconFile) == UNZ_OK ) {
                while ( (len = unzReadCurrentFile(qmc2IconFile, &imageBuffer, QMC2_ZIP_BUFFER_SIZE)) > 0 )
                  for (i = 0; i < len; i++)
                    imageData += imageBuffer[i];
                unzCloseCurrentFile(qmc2IconFile);
                QPixmap iconPixmap;
                if ( iconPixmap.loadFromData(imageData) ) {
                  QFileInfo fi(gameFileName.toLower());
                  qmc2IconMap[fi.baseName()] = QIcon(iconPixmap);
                }
              }
            }
            if ( iconCount % qmc2GamelistResponsiveness == 0 ) {
              qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(true);
              qApp->processEvents();
              qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(false);
            }
          } while ( unzGoToNextFile(qmc2IconFile) != UNZ_END_OF_LIST_OF_FILE );
        }
        qmc2MainWindow->progressBarGamelist->setRange(0, currentMax);
        if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
          qmc2MainWindow->progressBarGamelist->setFormat(oldFormat);
        else
          qmc2MainWindow->progressBarGamelist->setFormat("%p%");
      }
      elapsedTime = elapsedTime.addMSecs(preloadTimer.elapsed());
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (pre-caching icons from ZIP archive, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("%n icon(s) loaded", "", iconCount));
      qmc2IconsPreloaded = true;

      if ( checkOnly )
        qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(true);

      return loadIcon(gameName, item, checkOnly);
    }
  } else {
    // use icon directory
    if ( !qmc2IconsPreloaded ) {
      QTime preloadTimer, elapsedTime;
      preloadTimer.start();
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("pre-caching icons from directory"));
      qApp->processEvents();
#if defined(QMC2_EMUTYPE_MAME)
      QString icoDir = qmc2Config->value("MAME/FilesAndDirectories/IconDirectory").toString();
#elif defined(QMC2_EMUTYPE_MESS)
      QString icoDir = qmc2Config->value("MESS/FilesAndDirectories/IconDirectory").toString();
#endif
      QDir iconDirectory(icoDir);
      QStringList nameFilter;
      nameFilter << "*.png";
      QStringList iconFiles = iconDirectory.entryList(nameFilter, QDir::Files | QDir::Readable);
      int iconCount;
      int currentMax = qmc2MainWindow->progressBarGamelist->maximum();
      QString oldFormat = qmc2MainWindow->progressBarGamelist->format();
      if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
        qmc2MainWindow->progressBarGamelist->setFormat(tr("Icon cache - %p%"));
      else
        qmc2MainWindow->progressBarGamelist->setFormat("%p%");
      qmc2MainWindow->progressBarGamelist->setRange(0, iconFiles.count());
      qmc2MainWindow->progressBarGamelist->reset();
      qApp->processEvents();
      for (iconCount = 0; iconCount < iconFiles.count(); iconCount++) {
        qmc2MainWindow->progressBarGamelist->setValue(iconCount);
        if ( iconCount % 25 == 0 )
          qApp->processEvents();
#ifdef QMC2_DEBUG
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Gamelist::loadIcon(): loading %1").arg(iconFiles[iconCount]));
#endif
        QPixmap iconPixmap;
        if ( iconPixmap.load(icoDir + iconFiles[iconCount]) )
          icon = QIcon(iconPixmap);
        else
          icon = QIcon();
        qmc2IconMap[iconFiles[iconCount].toLower().remove(".png")] = icon;
        if ( iconCount % qmc2GamelistResponsiveness == 0 ) {
          qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(true);
          qApp->processEvents();
          qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(false);
        }
      }
      qmc2MainWindow->progressBarGamelist->setRange(0, currentMax);
      if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
        qmc2MainWindow->progressBarGamelist->setFormat(oldFormat);
      else
        qmc2MainWindow->progressBarGamelist->setFormat("%p%");
      elapsedTime = elapsedTime.addMSecs(preloadTimer.elapsed());
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (pre-caching icons from directory, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("%n icon(s) loaded", "", iconCount));
      qmc2IconsPreloaded = true;

      if ( checkOnly )
        qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(true);

      return loadIcon(gameName, item, checkOnly);
    }
  }

  if ( checkOnly )
    qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(true);

  return false;
}

#if defined(QMC2_EMUTYPE_MAME)
void Gamelist::loadCatverIni()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::loadCatverIni()");
#endif

  qmc2CategoryMap.clear();
  qmc2VersionMap.clear();

  QTime loadTimer, elapsedTime;
  loadTimer.start();
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading catver.ini"));
  qApp->processEvents();

  int currentMax = qmc2MainWindow->progressBarGamelist->maximum();
  QString oldFormat = qmc2MainWindow->progressBarGamelist->format();
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
    qmc2MainWindow->progressBarGamelist->setFormat(tr("Catver.ini - %p%"));
  else
    qmc2MainWindow->progressBarGamelist->setFormat("%p%");
  qmc2MainWindow->progressBarGamelist->setRange(0, 2 * numTotalGames); // we can't assume that catver.ini has exactly this number of games, though!
  qmc2MainWindow->progressBarGamelist->reset();
  qApp->processEvents();

  QFile catverIniFile(qmc2Config->value("MAME/FilesAndDirectories/CatverIni").toString());
  int entryCounter = 0;
  if ( catverIniFile.open(QFile::ReadOnly) ) {
    QTextStream tsCatverIni(&catverIniFile);
    bool isVersion = false, isCategory = false;
    while ( !tsCatverIni.atEnd() ) {
      QString catverLine = tsCatverIni.readLine().simplified().trimmed();
      if ( catverLine.isEmpty() )
        continue;
      if ( catverLine.contains("[Category]") ) {
        isCategory = true;
        isVersion = false;
      } else if ( catverLine.contains("[VerAdded]") ) {
        isCategory = false;
        isVersion = true;
      } else {
        QStringList tokens = catverLine.split("=");
        if ( tokens.count() >= 2 ) {
          qmc2MainWindow->progressBarGamelist->setValue(++entryCounter);
          if ( isCategory )
            qmc2CategoryMap.insert(tokens[0], tokens[1]);
          else if ( isVersion ) {
            QString verStr = tokens[1];
            if ( verStr.startsWith(".") ) verStr.prepend("0");
            qmc2VersionMap.insert(tokens[0], verStr);
          }
        }
      }
    }
    catverIniFile.close();
  } else
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ERROR: can't open '%1' for reading -- no catver.ini data available").arg(qmc2Config->value("MAME/FilesAndDirectories/CatverIni").toString()));

  qmc2MainWindow->progressBarGamelist->setRange(0, currentMax);
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
    qmc2MainWindow->progressBarGamelist->setFormat(oldFormat);
  else
    qmc2MainWindow->progressBarGamelist->setFormat("%p%");

  elapsedTime = elapsedTime.addMSecs(loadTimer.elapsed());
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading catver.ini, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("%1 category / %2 version records loaded").arg(qmc2CategoryMap.count()).arg(qmc2VersionMap.count()));
}

void Gamelist::createCategoryView()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::createCategoryView()");
#endif

	qmc2CategoryItemMap.clear();
	qmc2MainWindow->treeWidgetCategoryView->hide();
	qmc2MainWindow->labelCreatingCategoryView->show();

	if ( qmc2ReloadActive && !qmc2StopParser && qmc2MainWindow->stackedWidgetView->currentIndex() == QMC2_VIEW_CATEGORY_INDEX ) {
		QTimer::singleShot(QMC2_RELOAD_POLL_INTERVAL, this, SLOT(createCategoryView()));
		qApp->processEvents();
		return;
	} else if ( qmc2MainWindow->stackedWidgetView->currentIndex() != QMC2_VIEW_CATEGORY_INDEX && qmc2MainWindow->stackedWidgetView->currentIndex() == QMC2_VIEW_CATEGORY_INDEX ) {
		qmc2MainWindow->stackedWidgetView->setCurrentIndex(QMC2_VIEW_CATEGORY_INDEX);
		qmc2MainWindow->stackedWidgetView->update();
		qApp->processEvents();
		QTimer::singleShot(QMC2_RELOAD_POLL_INTERVAL, this, SLOT(createCategoryView()));
		return;
	}

	qmc2MainWindow->stackedWidgetView->setCurrentIndex(QMC2_VIEW_CATEGORY_INDEX);
	qmc2MainWindow->stackedWidgetView->update();
	qApp->processEvents();

	if ( !qmc2StopParser ) {
		qmc2MainWindow->treeWidgetCategoryView->clear();
		QString oldFormat = qmc2MainWindow->progressBarGamelist->format();
		if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
			qmc2MainWindow->progressBarGamelist->setFormat(tr("Category view - %p%"));
		else
			qmc2MainWindow->progressBarGamelist->setFormat("%p%");
		qmc2MainWindow->progressBarGamelist->setRange(0, qmc2CategoryMap.count());
		qmc2MainWindow->progressBarGamelist->reset();
		QMapIterator<QString, QString> it(qmc2CategoryMap);
		int counter = 0;
		bool showDeviceSets = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowDeviceSets", true).toBool();
		bool showBiosSets = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowBiosSets", true).toBool();
		while ( it.hasNext() ) {
			it.next();
			qmc2MainWindow->progressBarGamelist->setValue(counter++);
			QString gameName = it.key();
			QString category = it.value();
			if ( gameName.isEmpty() )
				continue;
			if ( !qmc2GamelistItemMap.contains(gameName) )
				continue;
			if ( category.isEmpty() )
				category = tr("?");
			QTreeWidgetItem *baseItem = qmc2GamelistItemMap[gameName];
			if ( baseItem ) {
				QList<QTreeWidgetItem *> matchItems = qmc2MainWindow->treeWidgetCategoryView->findItems(category, Qt::MatchExactly);
				QTreeWidgetItem *categoryItem = NULL;
				if ( matchItems.count() > 0 )
					categoryItem = matchItems[0];
				if ( categoryItem == NULL ) {
					categoryItem = new QTreeWidgetItem(qmc2MainWindow->treeWidgetCategoryView);
					categoryItem->setText(QMC2_GAMELIST_COLUMN_GAME, category);
				}
				QTreeWidgetItem *gameItem = new QTreeWidgetItem(categoryItem);
				bool isBIOS = qmc2BiosROMs.contains(gameName);
				bool isDevice = qmc2DeviceROMs.contains(gameName);
				gameItem->setHidden((isBIOS && !showBiosSets) || (isDevice && !showDeviceSets));
				gameItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
				gameItem->setCheckState(QMC2_GAMELIST_COLUMN_TAG, baseItem->checkState(QMC2_GAMELIST_COLUMN_TAG));
				gameItem->setText(QMC2_GAMELIST_COLUMN_GAME, baseItem->text(QMC2_GAMELIST_COLUMN_GAME));
				gameItem->setText(QMC2_GAMELIST_COLUMN_YEAR, baseItem->text(QMC2_GAMELIST_COLUMN_YEAR));
				gameItem->setText(QMC2_GAMELIST_COLUMN_MANU, baseItem->text(QMC2_GAMELIST_COLUMN_MANU));
				gameItem->setText(QMC2_GAMELIST_COLUMN_NAME, baseItem->text(QMC2_GAMELIST_COLUMN_NAME));
				gameItem->setText(QMC2_GAMELIST_COLUMN_RTYPES, baseItem->text(QMC2_GAMELIST_COLUMN_RTYPES));
				gameItem->setText(QMC2_GAMELIST_COLUMN_PLAYERS, baseItem->text(QMC2_GAMELIST_COLUMN_PLAYERS));
				gameItem->setText(QMC2_GAMELIST_COLUMN_DRVSTAT, baseItem->text(QMC2_GAMELIST_COLUMN_DRVSTAT));
				gameItem->setText(QMC2_GAMELIST_COLUMN_VERSION, baseItem->text(QMC2_GAMELIST_COLUMN_VERSION));
				if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowROMStatusIcons", true).toBool() ) {
					switch ( qmc2GamelistStatusMap[gameName][0].toAscii() ) {
						case 'C':
							if ( isBIOS )
								gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectBIOSImageIcon);
							else if ( isDevice )
								gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectDeviceImageIcon);
							else
								gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectImageIcon);
							break;
						case 'M':
							if ( isBIOS )
								gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectBIOSImageIcon);
							else if ( isDevice )
								gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectDeviceImageIcon);
							else
								gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectImageIcon);
							break;
						case 'I':
							if ( isBIOS )
								gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectBIOSImageIcon);
							else if ( isDevice )
								gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectDeviceImageIcon);
							else
								gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectImageIcon);
							break;
						case 'N':
							if ( isBIOS )
								gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundBIOSImageIcon);
							else if ( isDevice )
								gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundDeviceImageIcon);
							else
								gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundImageIcon);
							break;
						default:
							if ( isBIOS )
								gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownBIOSImageIcon);
							else if ( isDevice )
								gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownDeviceImageIcon);
							else
								gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownImageIcon);
							break;
					}
				}
				loadIcon(gameName, gameItem);
				qmc2CategoryItemMap[gameName] = gameItem;
			}
		}
		qmc2MainWindow->treeWidgetCategoryView->sortItems(qmc2MainWindow->sortCriteriaLogicalIndex(), qmc2SortOrder);
		qmc2MainWindow->progressBarGamelist->reset();
		qmc2MainWindow->progressBarGamelist->setFormat(oldFormat);
	}

	qmc2MainWindow->labelCreatingCategoryView->hide();
	qmc2MainWindow->treeWidgetCategoryView->show();

	QTimer::singleShot(0, qmc2MainWindow, SLOT(scrollToCurrentItem()));
}

void Gamelist::createVersionView()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::createVersionView()");
#endif

	qmc2VersionItemMap.clear();
	qmc2MainWindow->treeWidgetVersionView->hide();
	qmc2MainWindow->labelCreatingVersionView->show();

	if ( qmc2ReloadActive && !qmc2StopParser && qmc2MainWindow->stackedWidgetView->currentIndex() == QMC2_VIEW_VERSION_INDEX ) {
		QTimer::singleShot(QMC2_RELOAD_POLL_INTERVAL, this, SLOT(createVersionView()));
		qApp->processEvents();
		return;
	} else if ( qmc2MainWindow->stackedWidgetView->currentIndex() != QMC2_VIEW_VERSION_INDEX && qmc2MainWindow->stackedWidgetView->currentIndex() == QMC2_VIEW_VERSION_INDEX ) {
		qmc2MainWindow->stackedWidgetView->setCurrentIndex(QMC2_VIEW_VERSION_INDEX);
		qmc2MainWindow->stackedWidgetView->update();
		qApp->processEvents();
		QTimer::singleShot(QMC2_RELOAD_POLL_INTERVAL, this, SLOT(createVersionView()));
		return;
	}

	qmc2MainWindow->stackedWidgetView->setCurrentIndex(QMC2_VIEW_VERSION_INDEX);
	qmc2MainWindow->stackedWidgetView->update();
	qApp->processEvents();

	if ( !qmc2StopParser ) {
		qmc2MainWindow->treeWidgetVersionView->clear();
		QString oldFormat = qmc2MainWindow->progressBarGamelist->format();
		if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
			qmc2MainWindow->progressBarGamelist->setFormat(tr("Version view - %p%"));
		else
			qmc2MainWindow->progressBarGamelist->setFormat("%p%");
		qmc2MainWindow->progressBarGamelist->setRange(0, qmc2VersionMap.count());
		qmc2MainWindow->progressBarGamelist->reset();
		QMapIterator<QString, QString> it(qmc2VersionMap);
		int counter = 0;
		bool showDeviceSets = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowDeviceSets", true).toBool();
		bool showBiosSets = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowBiosSets", true).toBool();
		while ( it.hasNext() ) {
			it.next();
			qmc2MainWindow->progressBarGamelist->setValue(counter++);
			QString gameName = it.key();
			QString version = it.value();
			if ( gameName.isEmpty() )
				continue;
			if ( !qmc2GamelistItemMap.contains(gameName) )
				continue;
			if ( version.isEmpty() )
				version = tr("?");
			QTreeWidgetItem *baseItem = qmc2GamelistItemMap[gameName];
			if ( baseItem ) {
				QList<QTreeWidgetItem *> matchItems = qmc2MainWindow->treeWidgetVersionView->findItems(version, Qt::MatchExactly);
				QTreeWidgetItem *versionItem = NULL;
				if ( matchItems.count() > 0 )
					versionItem = matchItems[0];
				if ( versionItem == NULL ) {
					versionItem = new QTreeWidgetItem(qmc2MainWindow->treeWidgetVersionView);
					versionItem->setText(QMC2_GAMELIST_COLUMN_GAME, version);
				}
				QTreeWidgetItem *gameItem = new QTreeWidgetItem(versionItem);
				bool isBIOS = qmc2BiosROMs.contains(gameName);
				bool isDevice = qmc2DeviceROMs.contains(gameName);
				gameItem->setHidden((isBIOS && !showBiosSets) || (isDevice && !showDeviceSets));
				gameItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
				gameItem->setCheckState(QMC2_GAMELIST_COLUMN_TAG, baseItem->checkState(QMC2_GAMELIST_COLUMN_TAG));
				gameItem->setText(QMC2_GAMELIST_COLUMN_GAME, baseItem->text(QMC2_GAMELIST_COLUMN_GAME));
				gameItem->setText(QMC2_GAMELIST_COLUMN_YEAR, baseItem->text(QMC2_GAMELIST_COLUMN_YEAR));
				gameItem->setText(QMC2_GAMELIST_COLUMN_MANU, baseItem->text(QMC2_GAMELIST_COLUMN_MANU));
				gameItem->setText(QMC2_GAMELIST_COLUMN_NAME, baseItem->text(QMC2_GAMELIST_COLUMN_NAME));
				gameItem->setText(QMC2_GAMELIST_COLUMN_RTYPES, baseItem->text(QMC2_GAMELIST_COLUMN_RTYPES));
				gameItem->setText(QMC2_GAMELIST_COLUMN_PLAYERS, baseItem->text(QMC2_GAMELIST_COLUMN_PLAYERS));
				gameItem->setText(QMC2_GAMELIST_COLUMN_DRVSTAT, baseItem->text(QMC2_GAMELIST_COLUMN_DRVSTAT));
				gameItem->setText(QMC2_GAMELIST_COLUMN_CATEGORY, baseItem->text(QMC2_GAMELIST_COLUMN_CATEGORY));
				if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowROMStatusIcons", true).toBool() ) {
					switch ( qmc2GamelistStatusMap[gameName][0].toAscii() ) {
						case 'C':
							if ( isBIOS )
								gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectBIOSImageIcon);
							else if ( isDevice )
								gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectDeviceImageIcon);
							else
								gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectImageIcon);
							break;
						case 'M':
							if ( isBIOS )
								gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectBIOSImageIcon);
							else if ( isDevice )
								gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectDeviceImageIcon);
							else
								gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectImageIcon);
							break;
						case 'I':
							if ( isBIOS )
								gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectBIOSImageIcon);
							else if ( isDevice )
								gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectDeviceImageIcon);
							else
								gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectImageIcon);
							break;
						case 'N':
							if ( isBIOS )
								gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundBIOSImageIcon);
							else if ( isDevice )
								gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundDeviceImageIcon);
							else
								gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundImageIcon);
							break;
						default:
							if ( isBIOS )
								gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownBIOSImageIcon);
							else if ( isDevice )
								gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownDeviceImageIcon);
							else
								gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownImageIcon);
							break;
					}
				}
				loadIcon(gameName, gameItem);
				qmc2VersionItemMap[gameName] = gameItem;
			}
		}
		qmc2MainWindow->treeWidgetVersionView->sortItems(qmc2MainWindow->sortCriteriaLogicalIndex(), qmc2SortOrder);
		qmc2MainWindow->progressBarGamelist->reset();
		qmc2MainWindow->progressBarGamelist->setFormat(oldFormat);
	}

	qmc2MainWindow->labelCreatingVersionView->hide();
	qmc2MainWindow->treeWidgetVersionView->show();

	QTimer::singleShot(0, qmc2MainWindow, SLOT(scrollToCurrentItem()));
}
#endif

QString Gamelist::lookupDriverName(QString systemName)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Gamelist::lookupDriverName(QString systemName = %1)").arg(systemName));
#endif

	QString driverName = driverNameMap[systemName];

	if ( driverName.isEmpty() ) {
		int i = 0;
#if defined(QMC2_EMUTYPE_MAME) 
		QString s = "<game name=\"" + systemName + "\"";
		while ( !xmlLines[i].contains(s) ) i++;
		QString line = xmlLines[i].simplified();
		int startIndex = line.indexOf("sourcefile=\"");
		if ( startIndex > 0 ) {
			startIndex += 12;
			int endIndex = line.indexOf("\"", startIndex);
			driverName = line.mid(startIndex, endIndex - startIndex); 
		}
#elif defined(QMC2_EMUTYPE_MESS)
		QString s = "<machine name=\"" + systemName + "\"";
		while ( !xmlLines[i].contains(s) ) i++;
		QString line = xmlLines[i].simplified();
		int startIndex = line.indexOf("sourcefile=\"");
		if ( startIndex > 0 ) {
			startIndex += 12;
			int endIndex = line.indexOf("\"", startIndex);
			driverName = line.mid(startIndex, endIndex - startIndex); 
		}
#endif

		if ( !driverName.isEmpty() ) {
			QFileInfo fi(driverName);
			driverName = fi.baseName();
			driverNameMap[systemName] = driverName;
		}
	}

	return driverName;
}

bool GamelistItem::operator<(const QTreeWidgetItem &otherItem) const
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: GamelistItem::operator<(const GamelistItem &otherItem = ...)");
#endif
  
  switch ( qmc2SortCriteria ) {
    case QMC2_SORT_BY_DESCRIPTION:
      return (text(QMC2_GAMELIST_COLUMN_GAME).toUpper() < otherItem.text(QMC2_GAMELIST_COLUMN_GAME).toUpper());

    case QMC2_SORT_BY_ROM_STATE:
      return (whatsThis(QMC2_GAMELIST_COLUMN_GAME).at(0).toAscii() < otherItem.whatsThis(QMC2_GAMELIST_COLUMN_GAME).at(0).toAscii());

    case QMC2_SORT_BY_TAG:
      return (int(checkState(QMC2_GAMELIST_COLUMN_TAG)) < int(otherItem.checkState(QMC2_GAMELIST_COLUMN_TAG)));

    case QMC2_SORT_BY_YEAR:
      return (text(QMC2_GAMELIST_COLUMN_YEAR) < otherItem.text(QMC2_GAMELIST_COLUMN_YEAR));

    case QMC2_SORT_BY_MANUFACTURER:
      return (text(QMC2_GAMELIST_COLUMN_MANU).toUpper() < otherItem.text(QMC2_GAMELIST_COLUMN_MANU).toUpper());

    case QMC2_SORT_BY_NAME:
      return (text(QMC2_GAMELIST_COLUMN_NAME).toUpper() < otherItem.text(QMC2_GAMELIST_COLUMN_NAME).toUpper());

    case QMC2_SORT_BY_ROMTYPES:
      return (text(QMC2_GAMELIST_COLUMN_RTYPES).toUpper() < otherItem.text(QMC2_GAMELIST_COLUMN_RTYPES).toUpper());

    case QMC2_SORT_BY_PLAYERS:
      return (text(QMC2_GAMELIST_COLUMN_PLAYERS).toUpper() < otherItem.text(QMC2_GAMELIST_COLUMN_PLAYERS).toUpper());

    case QMC2_SORT_BY_DRVSTAT:
      return (text(QMC2_GAMELIST_COLUMN_DRVSTAT).toUpper() < otherItem.text(QMC2_GAMELIST_COLUMN_DRVSTAT).toUpper());

#if defined(QMC2_EMUTYPE_MAME)
    case QMC2_SORT_BY_CATEGORY:
      return (text(QMC2_GAMELIST_COLUMN_CATEGORY).toUpper() < otherItem.text(QMC2_GAMELIST_COLUMN_CATEGORY).toUpper());

    case QMC2_SORT_BY_VERSION:
      return (text(QMC2_GAMELIST_COLUMN_VERSION).toUpper() < otherItem.text(QMC2_GAMELIST_COLUMN_VERSION).toUpper());
#endif

    default:
      return false;
  }
}
