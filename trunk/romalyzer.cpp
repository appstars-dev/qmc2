#include <QCryptographicHash>
#include <QHeaderView>
#include <QDir>
#include <QFileInfo>
#include <QScrollBar>
#include <QTest>
#include <QMap>
#include <QFileDialog>
#include <QClipboard>

#include "romalyzer.h"
#include "qmc2main.h"
#include "options.h"
#include "gamelist.h"
#include "macros.h"
#include "unzip.h"
#include "zip.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern QSettings *qmc2Config;
extern Gamelist *qmc2Gamelist;
extern ROMAlyzer *qmc2ROMAlyzer;
extern bool qmc2ReloadActive;
extern bool qmc2ROMAlyzerActive;
extern bool qmc2ROMAlyzerPaused;
extern bool qmc2CleaningUp;
extern bool qmc2EarlyStartup;
extern bool qmc2StopParser;
extern QMap<QString, QTreeWidgetItem *> qmc2GamelistItemMap;
extern QMap<QString, QTreeWidgetItem *> qmc2HierarchyItemMap;
#if defined(QMC2_EMUTYPE_MAME)
extern QMap<QString, QTreeWidgetItem *> qmc2CategoryItemMap;
extern QMap<QString, QTreeWidgetItem *> qmc2VersionItemMap;
#endif
extern QAbstractItemView::ScrollHint qmc2CursorPositioningMode;

QCache<QString, int> romalyzerXmlGamePositionCache;

/*
  HOWTO: Calculate the 32-bit CRC of a QByteArray with zlib:

  #include <QByteArray>
  #include <zlib.h>
  ...
  QByteArray ba("This is a test -- 123 :)!");
  ulong crc = crc32(0, NULL, 0);
  crc = crc32(crc, (const Bytef *)ba.data(), ba.size());
  printf("CRC-32 = 0x%x\n", crc);
*/

/*
  INFO: How MAME searches for ROMs & CHDs of a game

  <rompath> = <first_rompath>
  for all ROM and CHD <file>'s {
    1) try <rompath>/<game>/<file> - if okay skip to 6)
    2) try <file> from <rompath1>/<game>.zip - if okay skip to 6)
    3) if more rompaths exists, retry 1) and 2) for <next_rompath>...
    4) if a <merge> exists, retry 1), 2) and 3) for <romof>/<merge> instead of <game>/<file>
    5) <file> was not found - stop
    6) load <file> and check CRC
    7) ...
  }

  Backward engineering powered by strace :)! 
*/

ROMAlyzer::ROMAlyzer(QWidget *parent)
#if defined(Q_WS_WIN)
  : QDialog(parent, Qt::Dialog)
#else
  : QDialog(parent, Qt::Dialog | Qt::SubWindow)
#endif
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::ROMAlyzer(QWidget *parent = %1)").arg((qulonglong)parent));
#endif
  
  setupUi(this);

  checkBoxFixCHDs->setVisible(QMC2_CHD_CURRENT_VERSION < 5);

#if defined(QMC2_SDLMESS)
  treeWidgetChecksums->headerItem()->setText(0, tr("Machine / File"));
  checkBoxSelectGame->setText(tr("Select machine"));
  checkBoxSelectGame->setToolTip(tr("Select machine in machine list if selected in analysis report?"));
  checkBoxAutoScroll->setToolTip(tr("Automatically scroll to the currently analyzed machine"));
  lineEditGames->setToolTip(tr("Shortname of machine to be analyzed - wildcards allowed, use space as delimiter for multiple machines"));
#endif

  treeWidgetChecksums->header()->setSortIndicatorShown(false);
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/RestoreLayout").toBool() ) {
    treeWidgetChecksums->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/ROMAlyzer/ReportHeaderState", QByteArray()).toByteArray());
    tabWidgetAnalysis->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/ROMAlyzer/AnalysisTab", 0).toInt());
    treeWidgetChecksumWizardSearchResult->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/ROMAlyzer/ChecksumWizardHeaderState", QByteArray()).toByteArray());
    move(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/ROMAlyzer/Position", QPoint()).toPoint());
    resize(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/ROMAlyzer/Size", QSize()).toSize());
  }

  // compression types for CHD v3 and v4
  chdCompressionTypes << tr("none") << tr("zlib") << tr("zlib+") << tr("A/V codec");

  // compression types for CHD v5
  chdCompressionTypesV5["zlib"] = tr("zlib (Deflate)");
  chdCompressionTypesV5["lzma"] = tr("lzma (LZMA)");
  chdCompressionTypesV5["huff"] = tr("huff (Huffman)");
  chdCompressionTypesV5["flcb"] = tr("flcb (FLAC, big-endian)");
  chdCompressionTypesV5["flcl"] = tr("flcl (FLAC, little-endian)");
  chdCompressionTypesV5["cdfl"] = tr("cdfl (CD FLAC)");
  chdCompressionTypesV5["avhu"] = tr("avhu (A/V Huffman)");

  chdManagerRunning = chdManagerMD5Success = chdManagerSHA1Success = false;
#if QMC2_CHD_CURRENT_VERSION < 5
  chdManagerCurrentHunk = chdManagerTotalHunks = 0;
#else
  chdManagerCurrentHunk = 0;
  chdManagerTotalHunks = 100;
  checkBoxVerifyCHDs->setToolTip(tr("Verify CHDs through 'chdman verify'"));
  checkBoxUpdateCHDs->setToolTip(tr("Try to update CHDs if their header indicates an older version ('chdman copy')"));
#endif

  adjustIconSizes();
  pushButtonPause->setVisible(false);

  wizardSearch = quickSearch = false;

  QFont logFont;
  logFont.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/LogFont").toString());
  textBrowserLog->setFont(logFont);

  connect(&animTimer, SIGNAL(timeout()), this, SLOT(animationTimeout()));

#if !defined(QMC2_DATABASE_ENABLED)
  groupBoxDatabase->setChecked(false);
  groupBoxDatabase->setVisible(false);
#else
  connectionCheckRunning = false;
  dbManager = new ROMDatabaseManager(this);
#endif

  QString s;
  QAction *action;

  romFileContextMenu = new QMenu(this);
  romFileContextMenu->hide();
  
  s = tr("Search checksum");
  action = romFileContextMenu->addAction(s);
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/filefind.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(runChecksumWizard()));

  romSetContextMenu = new QMenu(this);
  romSetContextMenu->hide();
  
  s = tr("Rewrite set");
  action = romSetContextMenu->addAction(s);
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/filesave.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(runSetRewriter()));
  actionRewriteSet = action;

  s = tr("Copy to clipboard");
  action = romSetContextMenu->addAction(s);
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/editcopy.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(copyToClipboard()));

#if defined(Q_WS_MAC)
  setParent(qmc2MainWindow, Qt::Dialog);
#endif
}

ROMAlyzer::~ROMAlyzer()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::~ROMAlyzer()");
#endif

#if defined(QMC2_DATABASE_ENABLED)
  if ( dbManager )
    delete dbManager;
#endif
}

void ROMAlyzer::adjustIconSizes()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::adjustIconSizes()"));
#endif

  QFont f;
  f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
  QFontMetrics fm(f);
  QSize iconSize = QSize(fm.height() - 2, fm.height() - 2);
  QSize iconSizeMiddle = iconSize + QSize(2, 2);
  QSize iconSizeLarge = iconSize + QSize(4, 4);
  pushButtonAnalyze->setIconSize(iconSize);
  pushButtonPause->setIconSize(iconSize);
  pushButtonClose->setIconSize(iconSize);
  pushButtonSearchForward->setIconSize(iconSize);
  pushButtonSearchBackward->setIconSize(iconSize);
  toolButtonBrowseCHDManagerExecutableFile->setIconSize(iconSize);
  toolButtonBrowseTemporaryWorkingDirectory->setIconSize(iconSize);
  toolButtonBrowseSetRewriterOutputPath->setIconSize(iconSize);
  toolButtonBrowseSetRewriterAdditionalRomPath->setIconSize(iconSize);
  pushButtonChecksumWizardAnalyzeSelectedSets->setIconSize(iconSize);
  pushButtonChecksumWizardRepairBadSets->setIconSize(iconSize);
#if defined(QMC2_DATABASE_ENABLED)
  toolButtonBrowseDatabaseOutputPath->setIconSize(iconSize);
#endif
  treeWidgetChecksums->setIconSize(iconSizeMiddle);
  pushButtonChecksumWizardSearch->setIconSize(iconSize);
}

void ROMAlyzer::on_pushButtonClose_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_pushButtonClose_clicked()");
#endif

  if ( qmc2ROMAlyzerActive )
    on_pushButtonAnalyze_clicked();
}

void ROMAlyzer::on_pushButtonAnalyze_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_pushButtonAnalyze_clicked()");
#endif

  if ( qmc2ReloadActive ) {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("please wait for reload to finish and try again"));
  } else if ( qmc2ROMAlyzerActive ) {
    // stop ROMAlyzer
    log(tr("stopping analysis"));
    qmc2StopParser = true;
  } else if ( qmc2Gamelist->numGames > 0 ) {
    // start ROMAlyzer
    log(tr("starting analysis"));
    qmc2StopParser = false;
    QTimer::singleShot(0, this, SLOT(analyze()));
  }
}

void ROMAlyzer::on_pushButtonPause_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_pushButtonPause_clicked()");
#endif

  qmc2ROMAlyzerPaused = !qmc2ROMAlyzerPaused;
  if ( qmc2ROMAlyzerPaused ) {
    log(tr("pausing analysis"));
    pushButtonPause->setEnabled(false);
  } else {
    log(tr("resuming analysis"));
    pushButtonPause->setText(tr("&Pause"));
  }
}

void ROMAlyzer::on_pushButtonSearchForward_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_pushButtonSearchForward_clicked()");
#endif

  if ( !textBrowserLog->find(lineEditSearchString->text()) ) {
    lineEditSearchString->setEnabled(false);
    QTimer::singleShot(QMC2_ROMALYZER_FLASH_TIME, this, SLOT(enableSearchEdit()));
  }
}

void ROMAlyzer::on_pushButtonSearchBackward_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_pushButtonSearchBackward_clicked()");
#endif

  if ( !textBrowserLog->find(lineEditSearchString->text(), QTextDocument::FindBackward) ) {
    lineEditSearchString->setEnabled(false);
    QTimer::singleShot(QMC2_ROMALYZER_FLASH_TIME, this, SLOT(enableSearchEdit()));
  }
}

void ROMAlyzer::on_lineEditGames_textChanged(QString text)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::on_lineEditGames_textChanged(QString text = %1)").arg(text));
#endif

	if ( !qmc2ROMAlyzerActive )
		pushButtonAnalyze->setEnabled(!text.isEmpty());
}

void ROMAlyzer::on_checkBoxCalculateCRC_toggled(bool enable)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::on_checkBoxCalculateCRC_toggled(bool enable = %1)").arg(enable));
#endif

	treeWidgetChecksums->setColumnHidden(QMC2_ROMALYZER_COLUMN_CRC, !enable);
}

void ROMAlyzer::on_checkBoxCalculateMD5_toggled(bool enable)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::on_checkBoxCalculateMD5_toggled(bool enable = %1)").arg(enable));
#endif

	treeWidgetChecksums->setColumnHidden(QMC2_ROMALYZER_COLUMN_MD5, !enable);
}

void ROMAlyzer::on_checkBoxCalculateSHA1_toggled(bool enable)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::on_checkBoxCalculateSHA1_toggled(bool enable = %1)").arg(enable));
#endif

	treeWidgetChecksums->setColumnHidden(QMC2_ROMALYZER_COLUMN_SHA1, !enable);
	if ( groupBoxDatabase->isChecked() )
		groupBoxDatabase->setChecked(enable);
	tabChecksumWizard->setEnabled(enable);
}

void ROMAlyzer::animationTimeout()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::animationTimeout()");
#endif

  switch ( ++animSeq ) {
    case 0:
    case 1:
      pushButtonAnalyze->setIcon(QIcon(QString::fromUtf8(":/data/img/viewmag+.png")));
      break;

    case 2:
    case 3:
      pushButtonAnalyze->setIcon(QIcon(QString::fromUtf8(":/data/img/viewmag-.png")));
      break;

    default:
      break;
  }
  if ( animSeq > 2 )
    animSeq = -1;
}

void ROMAlyzer::closeEvent(QCloseEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::closeEvent(QCloseEvent *e = %1)").arg((qulonglong)e));
#endif

  if ( e && qmc2ROMAlyzerActive )
    on_pushButtonAnalyze_clicked();

  // save settings
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMAlyzer/AppendReport", checkBoxAppendReport->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMAlyzer/ExpandFiles", checkBoxExpandFiles->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMAlyzer/ExpandChecksums", checkBoxExpandChecksums->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMAlyzer/AutoScroll", checkBoxAutoScroll->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMAlyzer/CalculateCRC", checkBoxCalculateCRC->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMAlyzer/CalculateSHA1", checkBoxCalculateSHA1->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMAlyzer/CalculateMD5", checkBoxCalculateMD5->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMAlyzer/SelectGame", checkBoxSelectGame->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMAlyzer/MaxFileSize", spinBoxMaxFileSize->value());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMAlyzer/MaxLogSize", spinBoxMaxLogSize->value());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMAlyzer/MaxReports", spinBoxMaxReports->value());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMAlyzer/EnableCHDManager", groupBoxCHDManager->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMAlyzer/CHDManagerExecutableFile", lineEditCHDManagerExecutableFile->text());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMAlyzer/TemporaryWorkingDirectory", lineEditTemporaryWorkingDirectory->text());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMAlyzer/EnableSetRewriter", groupBoxSetRewriter->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMAlyzer/SetRewriterWhileAnalyzing", checkBoxSetRewriterWhileAnalyzing->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMAlyzer/SetRewriterSelfContainedSets", checkBoxSetRewriterSelfContainedSets->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMAlyzer/SetRewriterGoodSetsOnly", checkBoxSetRewriterGoodSetsOnly->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMAlyzer/SetRewriterZipArchives", radioButtonSetRewriterZipArchives->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMAlyzer/SetRewriterZipLevel", spinBoxSetRewriterZipLevel->value());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMAlyzer/SetRewriterUniqueCRCs", checkBoxSetRewriterUniqueCRCs->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMAlyzer/SetRewriterIndividualDirectories", radioButtonSetRewriterIndividualDirectories->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMAlyzer/SetRewriterOutputPath", lineEditSetRewriterOutputPath->text());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMAlyzer/SetRewriterAdditionalRomPath", lineEditSetRewriterAdditionalRomPath->text());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMAlyzer/ChecksumWizardHashType", comboBoxChecksumWizardHashType->currentIndex());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMAlyzer/ChecksumWizardAutomationLevel", comboBoxChecksumWizardAutomationLevel->currentIndex());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMAlyzer/VerifyCHDs", checkBoxVerifyCHDs->isChecked());
#if QMC2_CHD_CURRENT_VERSION < 5
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMAlyzer/FixCHDs", checkBoxFixCHDs->isChecked());
#endif
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMAlyzer/UpdateCHDs", checkBoxUpdateCHDs->isChecked());

#if defined(QMC2_DATABASE_ENABLED)
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMAlyzer/EnableDatabase", groupBoxDatabase->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMAlyzer/DatabaseServer", lineEditDatabaseServer->text());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMAlyzer/DatabaseName", lineEditDatabaseName->text());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMAlyzer/DatabasePort", spinBoxDatabasePort->value());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMAlyzer/DatabaseUser", lineEditDatabaseUser->text());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMAlyzer/DatabasePassword", QMC2_COMPRESS(lineEditDatabasePassword->text().toLatin1()));
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMAlyzer/DatabaseDownload", checkBoxDatabaseDownload->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMAlyzer/DatabaseUpload", checkBoxDatabaseUpload->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMAlyzer/DatabaseOverwrite", checkBoxDatabaseOverwrite->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMAlyzer/DatabaseDriver", comboBoxDatabaseDriver->currentIndex());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ROMAlyzer/DatabaseOutputPath", lineEditDatabaseOutputPath->text());
#endif

  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() ) {
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/ROMAlyzer/ReportHeaderState", treeWidgetChecksums->header()->saveState());
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/ROMAlyzer/AnalysisTab", tabWidgetAnalysis->currentIndex());
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/ROMAlyzer/ChecksumWizardHeaderState", treeWidgetChecksumWizardSearchResult->header()->saveState());
    if ( !qmc2CleaningUp ) {
      qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/ROMAlyzer/Visible", false);
      qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/ROMAlyzer/Position", pos());
      qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/ROMAlyzer/Size", size());
    }
  }

  if ( e )
    e->accept();
}

void ROMAlyzer::hideEvent(QHideEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::hideEvent(QHideEvent *e = %1)").arg((qulonglong)e));
#endif

  closeEvent(NULL);

  if ( e )
    e->accept();
}

void ROMAlyzer::showEvent(QShowEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::showEvent(QShowEvent *e = %1)").arg((qulonglong)e));
#endif

  // restore settings
  checkBoxAppendReport->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/AppendReport", false).toBool());
  checkBoxExpandFiles->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/ExpandFiles", false).toBool());
  checkBoxExpandChecksums->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/ExpandChecksums", false).toBool());
  checkBoxAutoScroll->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/AutoScroll", true).toBool());
  checkBoxCalculateCRC->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/CalculateCRC", true).toBool());
  checkBoxCalculateSHA1->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/CalculateSHA1", true).toBool());
  checkBoxCalculateMD5->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/CalculateMD5", true).toBool());
  checkBoxSelectGame->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/SelectGame", true).toBool());
  spinBoxMaxFileSize->setValue(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/MaxFileSize", 0).toInt());
  spinBoxMaxLogSize->setValue(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/MaxLogSize", 10000).toInt());
  spinBoxMaxReports->setValue(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/MaxReports", 1000).toInt());
  lineEditCHDManagerExecutableFile->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/CHDManagerExecutableFile", "").toString());
  lineEditTemporaryWorkingDirectory->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/TemporaryWorkingDirectory", "").toString());
  lineEditSetRewriterOutputPath->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/SetRewriterOutputPath", "").toString());
  lineEditSetRewriterAdditionalRomPath->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/SetRewriterAdditionalRomPath", "").toString());
  groupBoxSetRewriter->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/EnableSetRewriter", false).toBool());
  checkBoxSetRewriterWhileAnalyzing->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/SetRewriterWhileAnalyzing", false).toBool());
  checkBoxSetRewriterSelfContainedSets->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/SetRewriterSelfContainedSets", false).toBool());
  checkBoxSetRewriterGoodSetsOnly->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/SetRewriterGoodSetsOnly", true).toBool());
  radioButtonSetRewriterZipArchives->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/SetRewriterZipArchives", true).toBool());
  spinBoxSetRewriterZipLevel->setValue(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/SetRewriterZipLevel", Z_DEFAULT_COMPRESSION).toInt());
  checkBoxSetRewriterUniqueCRCs->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/SetRewriterUniqueCRCs", false).toBool());
  comboBoxChecksumWizardHashType->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/ChecksumWizardHashType", QMC2_ROMALYZER_CSWIZ_HASHTYPE_SHA1).toInt());
  comboBoxChecksumWizardAutomationLevel->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/ChecksumWizardAutomationLevel", QMC2_ROMALYZER_CSWIZ_AMLVL_NONE).toInt());
  radioButtonSetRewriterIndividualDirectories->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/SetRewriterIndividualDirectories", false).toBool());
  checkBoxVerifyCHDs->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/VerifyCHDs", true).toBool());
#if QMC2_CHD_CURRENT_VERSION < 5
  checkBoxFixCHDs->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/FixCHDs", false).toBool());
#endif
  checkBoxUpdateCHDs->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/UpdateCHDs", false).toBool());
  groupBoxCHDManager->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/EnableCHDManager", false).toBool());

#if defined(QMC2_DATABASE_ENABLED)
  lineEditDatabaseServer->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/DatabaseServer", "").toString());
#if defined(QMC2_EMUTYPE_MAME)
  lineEditDatabaseName->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/DatabaseName", "mame_romdb").toString());
#elif defined(QMC2_EMUTYPE_MESS)
  lineEditDatabaseName->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/DatabaseName", "mess_romdb").toString());
#endif
  spinBoxDatabasePort->setValue(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/DatabasePort", 0).toInt());
  lineEditDatabaseUser->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/DatabaseUser", "").toString());
  lineEditDatabasePassword->setText(QString(QMC2_UNCOMPRESS(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/DatabasePassword", "").toByteArray())));
  checkBoxDatabaseDownload->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/DatabaseDownload", true).toBool());
  checkBoxDatabaseUpload->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/DatabaseUpload", false).toBool());
  checkBoxDatabaseOverwrite->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/DatabaseOverwrite", false).toBool());
  comboBoxDatabaseDriver->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/DatabaseDriver", QMC2_DB_DRIVER_MYSQL).toInt());
  lineEditDatabaseOutputPath->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/DatabaseOutputPath", "").toString());
  groupBoxDatabase->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/EnableDatabase", false).toBool());
#endif

  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/ROMAlyzer/Visible", true);

  if ( e )
    e->accept();
}

void ROMAlyzer::on_spinBoxMaxLogSize_valueChanged(int value)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::on_spinBoxMaxLogSize_valueChanged(int value = %1)").arg(value));
#endif

  textBrowserLog->setMaximumBlockCount(spinBoxMaxLogSize->value());
}

void ROMAlyzer::moveEvent(QMoveEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::moveEvent(QMoveEvent *e = %1)").arg((qulonglong)e));
#endif

  if ( !qmc2CleaningUp && !qmc2EarlyStartup )
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
      qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/ROMAlyzer/Position", pos());

  if ( e )
    e->accept();
}

void ROMAlyzer::resizeEvent(QResizeEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::resizeEvent(QResizeEvent *e = %1)").arg((qulonglong)e));
#endif

  if ( !qmc2CleaningUp && !qmc2EarlyStartup )
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
      qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/ROMAlyzer/Size", size());

  if ( e )
    e->accept();
}

void ROMAlyzer::analyze()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::analyze()");
#endif

  qmc2ROMAlyzerActive = true;

  QString myRomPath;
#if defined(QMC2_EMUTYPE_MAME)
  if ( qmc2Config->contains("MAME/Configuration/Global/rompath") )
    myRomPath = qmc2Config->value("MAME/Configuration/Global/rompath").toString();
  else
    myRomPath = "roms";
#elif defined(QMC2_EMUTYPE_MESS)
  if ( qmc2Config->contains("MESS/Configuration/Global/rompath") )
    myRomPath = qmc2Config->value("MESS/Configuration/Global/rompath").toString();
  else
    myRomPath = "roms";
#endif

  if ( groupBoxSetRewriter->isChecked() )
    if ( !lineEditSetRewriterAdditionalRomPath->text().isEmpty() )
      myRomPath = lineEditSetRewriterAdditionalRomPath->text() + ";" + myRomPath;

  myRomPath.replace("~", QDir::homePath());
  myRomPath.replace("$HOME", QDir::homePath());
  romPaths = myRomPath.split(";", QString::SkipEmptyParts);

  int i;
  QStringList analyzerList;
  QStringList patternList = lineEditGames->text().simplified().split(" ");

  if ( !checkBoxAppendReport->isChecked() ) {
    treeWidgetChecksums->clear();
    textBrowserLog->clear();
  }
  qmc2ROMAlyzerPaused = false;
  animSeq = -1;
  animationTimeout();
  animTimer.start(QMC2_ANIMATION_TIMEOUT);
  pushButtonAnalyze->setText(tr("&Stop"));
  pushButtonPause->setVisible(true);
  pushButtonPause->setEnabled(true);
  pushButtonPause->setText(tr("&Pause"));
  lineEditGames->setEnabled(false);
  if ( checkBoxCalculateSHA1->isChecked() ) tabChecksumWizard->setEnabled(false);
  QTime analysisTimer, elapsedTime;
  romalyzerXmlGamePositionCache.clear();
  romalyzerXmlGamePositionCache.setMaxCost(QMC2_ROMALYZER_XMLPOSCACHE_SIZE);
  analysisTimer.start();
  log(tr("analysis started"));
#if defined(QMC2_EMUTYPE_MAME)
  log(tr("determining list of games to analyze"));
#elif defined(QMC2_EMUTYPE_MESS)
  log(tr("determining list of machines to analyze"));
#endif

  if ( wizardSearch || quickSearch ) {
    // two simple cases: the checksum wizard doesn't use any wild-cards, same holds for tagged sets --> no need to search!
    analyzerList = patternList;
    analyzerList.sort();
  } else {
    if ( patternList.count() == 1 ) {
      if ( qmc2GamelistItemMap.contains(patternList[0]) ) {
        // special case for exactly ONE matching game/machine -- no need to search
        analyzerList << patternList[0];
      }
    }

    if ( analyzerList.empty() ) {
      // determine list of games to analyze
#if defined(QMC2_EMUTYPE_MAME)
      labelStatus->setText(tr("Searching games"));
#elif defined(QMC2_EMUTYPE_MESS)
      labelStatus->setText(tr("Searching machines"));
#endif
      progressBar->setRange(0, qmc2Gamelist->numGames);
      progressBar->reset();
      QMapIterator<QString, QTreeWidgetItem *> it(qmc2GamelistItemMap);
      i = 0;
      while ( it.hasNext() && !qmc2StopParser ) {
        qApp->processEvents();
        it.next();
        progressBar->setValue(++i);
        foreach (QString pattern, patternList) {
          QRegExp regexp(pattern, Qt::CaseSensitive, QRegExp::Wildcard);
          if ( regexp.exactMatch(it.key()) )
            if ( !analyzerList.contains(it.key()) )
              analyzerList << it.key();
        }
      }
      progressBar->reset();
      labelStatus->setText(tr("Idle"));
    }
  }

  quickSearch = false;

  if ( !qmc2StopParser ) {
#if defined(QMC2_EMUTYPE_MAME)
    log(tr("done (determining list of games to analyze)"));
    log(tr("%n game(s) to analyze", "", analyzerList.count()));
#elif defined(QMC2_EMUTYPE_MESS)
    log(tr("done (determining list of machines to analyze)"));
    log(tr("%n machine(s) to analyze", "", analyzerList.count()));
#endif

    i = 0;
    int setsInMemory = 0;
    foreach (QString gameName, analyzerList) {
      // wait if paused...
      for (quint64 waitCounter = 0; qmc2ROMAlyzerPaused && !qmc2StopParser; waitCounter++) {
        if ( waitCounter == 0 ) {
          log(tr("analysis paused"));
          pushButtonPause->setText(tr("&Resume"));
          pushButtonPause->setEnabled(true);
          progressBar->reset();
          labelStatus->setText(tr("Paused"));
        }
        QTest::qWait(QMC2_ROMALYZER_PAUSE_TIMEOUT);
      }

      bool filesSkipped = false;
      bool filesUnknown = false;
      bool filesError = false;

      if ( qmc2StopParser )
        break;

      // remove the 'oldest' sets from the report if the report limit has been reached
      int reportLimit = spinBoxMaxReports->value();
      if ( reportLimit > 0 ) {
	      if ( setsInMemory >= reportLimit ) {
		      int setsToBeRemoved = setsInMemory - reportLimit + 1;
		      log(tr("report limit reached, removing %n set(s) from the report", "", setsToBeRemoved));
		      for (int j = 0; j < setsToBeRemoved; j++) {
			      QTreeWidgetItem *ti = treeWidgetChecksums->topLevelItem(0);
			      if ( ti ) {
				      if ( ti->isSelected() )
					      treeWidgetChecksums->selectionModel()->clear();
				      delete treeWidgetChecksums->takeTopLevelItem(0);
				      setsInMemory--;
			      }
		      }
	      }
      }
      setsInMemory++;

      QLocale locale;

      // analyze game
      log(tr("analyzing '%1'").arg(gameName));
      setRewriterSetCount = analyzerList.count() - i;
      labelStatus->setText(tr("Analyzing '%1'").arg(gameName) + QString(" - %1").arg(locale.toString(setRewriterSetCount)));

      // step 1: retrieve XML data, insert item with game name
      QTreeWidgetItem *item = new QTreeWidgetItem(treeWidgetChecksums);
      item->setText(QMC2_ROMALYZER_COLUMN_GAME, gameName);
      QString xmlBuffer = getXmlData(gameName);

      if ( qmc2StopParser )
	break;

      // step 2: parse XML data, insert ROMs / CHDs and checksums as they _should_ be
      log(tr("parsing XML data for '%1'").arg(gameName));
      QXmlInputSource xmlInputSource;
      xmlInputSource.setData(xmlBuffer);
      ROMAlyzerXmlHandler xmlHandler(item, checkBoxExpandFiles->isChecked(), checkBoxAutoScroll->isChecked());
      QXmlSimpleReader xmlReader;
      xmlReader.setContentHandler(&xmlHandler);
      if ( xmlReader.parse(xmlInputSource) )
	log(tr("done (parsing XML data for '%1')").arg(gameName));
      else
	log(tr("error (parsing XML data for '%1')").arg(gameName));
      if ( qmc2StopParser )
	break;

      int numWizardFiles = 1;
      if ( wizardSearch )
	numWizardFiles = treeWidgetChecksumWizardSearchResult->findItems(gameName, Qt::MatchExactly, QMC2_ROMALYZER_CSWIZ_COLUMN_ID).count();

      // step 3: check file status of ROMs and CHDs, recalculate checksums
      log(tr("checking %n file(s) for '%1'", "", wizardSearch ? numWizardFiles : xmlHandler.fileCounter).arg(gameName));
      progressBar->reset();
      progressBar->setRange(0, xmlHandler.fileCounter);
      int fileCounter;
      int notFoundCounter = 0;
      int noDumpCounter = 0;
      bool gameOkay = true;
      int mergeStatus = QMC2_ROMALYZER_MERGE_STATUS_OK;

      setRewriterFileMap.clear();
      setRewriterSetName = gameName;
      setRewriterItem = item;

      for (fileCounter = 0; fileCounter < xmlHandler.fileCounter && !qmc2StopParser; fileCounter++) {
	progressBar->setValue(fileCounter + 1);
	qApp->processEvents();
	QByteArray data;
	bool zipped = false;
	bool merged = false;
	QTreeWidgetItem *childItem = xmlHandler.childItems.at(fileCounter);
	QTreeWidgetItem *parentItem = xmlHandler.parentItem;
	QString sha1Calculated, md5Calculated, fallbackPath;

	if ( wizardSearch ) {
		QList<QTreeWidgetItem *> il = treeWidgetChecksumWizardSearchResult->findItems(gameName, Qt::MatchExactly, QMC2_ROMALYZER_CSWIZ_COLUMN_ID);
		bool itemFound = false;
		foreach (QTreeWidgetItem *it, il)
			if ( it->text(QMC2_ROMALYZER_CSWIZ_COLUMN_FILENAME) == childItem->text(QMC2_ROMALYZER_COLUMN_GAME) ) {
				itemFound = true;
				break;
			}
		if ( !itemFound ) {
			childItem->setText(QMC2_ROMALYZER_COLUMN_FILESTATUS, tr("skipped"));
			childItem->setForeground(QMC2_ROMALYZER_COLUMN_FILESTATUS, xmlHandler.blueBrush);
			continue;
		}
	}

	QString effectiveFile = getEffectiveFile(childItem, gameName, 
						 childItem->text(QMC2_ROMALYZER_COLUMN_GAME),
						 childItem->text(QMC2_ROMALYZER_COLUMN_CRC),
						 parentItem->text(QMC2_ROMALYZER_COLUMN_MERGE),
						 childItem->text(QMC2_ROMALYZER_COLUMN_MERGE),
						 childItem->text(QMC2_ROMALYZER_COLUMN_TYPE),
						 &data, &sha1Calculated, &md5Calculated,
						 &zipped, &merged, fileCounter, &fallbackPath); 

#ifdef QMC2_DEBUG
	log(QString("DEBUG: fileName = %1 [%2], isZipped = %3, fileType = %4, crcExpected = %5, sha1Calculated = %6")
		    .arg(childItem->text(QMC2_ROMALYZER_COLUMN_GAME))
		    .arg(effectiveFile)
		    .arg(zipped ? "true" : "false")
		    .arg(childItem->text(QMC2_ROMALYZER_COLUMN_TYPE).startsWith(tr("ROM")) ? "ROM" : "CHD")
		    .arg(childItem->text(QMC2_ROMALYZER_COLUMN_CRC).isEmpty() ? QString("--") : childItem->text(QMC2_ROMALYZER_COLUMN_CRC))
		    .arg(sha1Calculated.isEmpty() ? QString("--") : sha1Calculated));
#endif

	if ( qmc2StopParser )
	  continue;

	if ( effectiveFile.isEmpty() ) {
	  childItem->setText(QMC2_ROMALYZER_COLUMN_FILESTATUS, tr("not found"));
	  childItem->setForeground(QMC2_ROMALYZER_COLUMN_FILESTATUS, xmlHandler.greyBrush);
	  childItem->setIcon(QMC2_ROMALYZER_COLUMN_GAME, QIcon(QString::fromUtf8(":/data/img/remove.png")));
	  notFoundCounter++;
	} else {
	  QString fileStatus;
	  bool somethingsWrong = false;
	  bool eligibleForDatabaseUpload = false;
	  bool isCHD = childItem->text(QMC2_ROMALYZER_COLUMN_TYPE).split(" ")[0] == QObject::tr("CHD");
	  bool hasDump = childItem->text(QMC2_ROMALYZER_COLUMN_EMUSTATUS) != QObject::tr("no dump");

	  if ( effectiveFile != QMC2_ROMALYZER_FILE_NOT_FOUND ) {
	    if ( zipped )
	      childItem->setIcon(QMC2_ROMALYZER_COLUMN_GAME, QIcon(QString::fromUtf8(":/data/img/zip.png")));
	    else if ( isCHD )
	      childItem->setIcon(QMC2_ROMALYZER_COLUMN_GAME, QIcon(QString::fromUtf8(":/data/img/disk2.png")));
	    else if ( hasDump )
	      childItem->setIcon(QMC2_ROMALYZER_COLUMN_GAME, QIcon(QString::fromUtf8(":/data/img/fileopen.png")));
	    else
	      childItem->setIcon(QMC2_ROMALYZER_COLUMN_GAME, QIcon(QString::fromUtf8(":/data/img/wip.png")));
	  }

	  if ( effectiveFile == QMC2_ROMALYZER_FILE_TOO_BIG ) {
	    fileStatus = tr("skipped");
	    filesSkipped = true;
	  } else if ( effectiveFile == QMC2_ROMALYZER_FILE_NOT_SUPPORTED ) {
	    fileStatus = tr("skipped");
	    filesUnknown = true;
	  } else if ( effectiveFile == QMC2_ROMALYZER_FILE_ERROR ) {
	    fileStatus = tr("error");
	    childItem->setIcon(QMC2_ROMALYZER_COLUMN_GAME, QIcon(QString::fromUtf8(":/data/img/warning.png")));

	    QString mergeName = childItem->text(QMC2_ROMALYZER_COLUMN_MERGE);
	    if ( !mergeName.isEmpty() ) {
	      if ( mergeStatus < QMC2_ROMALYZER_MERGE_STATUS_CRIT ) mergeStatus = QMC2_ROMALYZER_MERGE_STATUS_CRIT;
	      childItem->setIcon(QMC2_ROMALYZER_COLUMN_MERGE, QIcon(QString::fromUtf8(":/data/img/merge_nok.png")));
	    }

	    filesError = true;
	    if ( wizardSelectedSets.contains(gameName) ) {
	      QList<QTreeWidgetItem *> il = treeWidgetChecksumWizardSearchResult->findItems(gameName, Qt::MatchExactly, QMC2_ROMALYZER_CSWIZ_COLUMN_ID);
	      foreach (QTreeWidgetItem *item, il)
		if ( item->text(QMC2_ROMALYZER_CSWIZ_COLUMN_FILENAME) == childItem->text(QMC2_ROMALYZER_COLUMN_GAME) ||
		     item->whatsThis(QMC2_ROMALYZER_CSWIZ_COLUMN_FILENAME) == childItem->text(QMC2_ROMALYZER_COLUMN_CRC) ) {
		  item->setText(QMC2_ROMALYZER_CSWIZ_COLUMN_STATUS, tr("bad"));
		  item->setForeground(QMC2_ROMALYZER_CSWIZ_COLUMN_STATUS, xmlHandler.redBrush);
		  item->setText(QMC2_ROMALYZER_CSWIZ_COLUMN_PATH, fallbackPath);
		}
	      on_treeWidgetChecksumWizardSearchResult_itemSelectionChanged();
	    }
	  } else if ( effectiveFile == QMC2_ROMALYZER_FILE_NOT_FOUND || effectiveFile == QMC2_ROMALYZER_NO_DUMP ) {
	    if ( hasDump ) {
		    fileStatus = tr("not found");
		    childItem->setIcon(QMC2_ROMALYZER_COLUMN_GAME, QIcon(QString::fromUtf8(":/data/img/remove.png")));
		    notFoundCounter++;
		    QString mergeName = childItem->text(QMC2_ROMALYZER_COLUMN_MERGE);
		    if ( !mergeName.isEmpty() ) {
			    if ( mergeStatus < QMC2_ROMALYZER_MERGE_STATUS_CRIT ) mergeStatus = QMC2_ROMALYZER_MERGE_STATUS_CRIT;
			    childItem->setIcon(QMC2_ROMALYZER_COLUMN_MERGE, QIcon(QString::fromUtf8(":/data/img/merge_nok.png")));
		    }

		    if ( wizardSelectedSets.contains(gameName) ) {
			    QList<QTreeWidgetItem *> il = treeWidgetChecksumWizardSearchResult->findItems(gameName, Qt::MatchExactly, QMC2_ROMALYZER_CSWIZ_COLUMN_ID);
			    foreach (QTreeWidgetItem *item, il) {
				    if ( item->text(QMC2_ROMALYZER_CSWIZ_COLUMN_FILENAME) == childItem->text(QMC2_ROMALYZER_COLUMN_GAME) || item->whatsThis(QMC2_ROMALYZER_CSWIZ_COLUMN_FILENAME) == childItem->text(QMC2_ROMALYZER_COLUMN_CRC) ) {
					    item->setText(QMC2_ROMALYZER_CSWIZ_COLUMN_STATUS, tr("bad"));
					    item->setForeground(QMC2_ROMALYZER_CSWIZ_COLUMN_STATUS, xmlHandler.redBrush);
					    item->setText(QMC2_ROMALYZER_CSWIZ_COLUMN_PATH, fallbackPath);
				    }
			    }
			    on_treeWidgetChecksumWizardSearchResult_itemSelectionChanged();
		    }
	    } else {
		    fileStatus = tr("no dump");
		    childItem->setIcon(QMC2_ROMALYZER_COLUMN_GAME, QIcon(QString::fromUtf8(":/data/img/wip.png")));
		    noDumpCounter++;
	    }
	  } else {
	    QTreeWidgetItem *fileItem = new QTreeWidgetItem(childItem);
	    fileItem->setText(QMC2_ROMALYZER_COLUMN_GAME, tr("Checksums"));
	    childItem->setExpanded(false);

	    QString mergeName = childItem->text(QMC2_ROMALYZER_COLUMN_MERGE);
	    if ( !mergeName.isEmpty() ) {
	      if ( !merged ) {
		log(tr("WARNING: %1 file '%2' loaded from '%3' may be obsolete, should be merged from parent set '%4'").arg(isCHD ? tr("CHD") : tr("ROM")).arg(childItem->text(QMC2_ROMALYZER_COLUMN_GAME)).arg(effectiveFile).arg(parentItem->text(QMC2_ROMALYZER_COLUMN_MERGE)));
		childItem->setIcon(QMC2_ROMALYZER_COLUMN_MERGE, QIcon(QString::fromUtf8(":/data/img/merge.png")));
		if ( mergeStatus < QMC2_ROMALYZER_MERGE_STATUS_WARN ) mergeStatus = QMC2_ROMALYZER_MERGE_STATUS_WARN;
	      } else 
		childItem->setIcon(QMC2_ROMALYZER_COLUMN_MERGE, QIcon(QString::fromUtf8(":/data/img/merge_ok.png")));
	    }

	    // Size
	    QString sizeStr = childItem->text(QMC2_ROMALYZER_COLUMN_SIZE);
	    if ( !sizeStr.isEmpty() ){
	      fileItem->setText(QMC2_ROMALYZER_COLUMN_SIZE, QString::number(data.size()));
	      fileStatus += tr("SIZE ");
	      if ( data.size() != sizeStr.toLongLong() && hasDump ) {
		somethingsWrong = true;
		fileItem->setForeground(QMC2_ROMALYZER_COLUMN_SIZE, xmlHandler.redBrush);
	      }
	      pushButtonPause->setIcon(QIcon(QString::fromUtf8(":/data/img/time.png")));
	      qApp->processEvents();
	    }

	    // CRC
	    QString crcStr = childItem->text(QMC2_ROMALYZER_COLUMN_CRC);
	    if ( !crcStr.isEmpty() && checkBoxCalculateCRC->isChecked() ) {
	      ulong crc = crc32(0, NULL, 0);
	      crc = crc32(crc, (const Bytef *)data.data(), data.size());
	      fileItem->setText(QMC2_ROMALYZER_COLUMN_CRC, QString::number(crc, 16).rightJustified(8, '0'));
	      fileStatus += tr("CRC ");
	      if ( crc != crcStr.toULongLong(0, 16) && hasDump ) {
		somethingsWrong = true;
		fileItem->setForeground(QMC2_ROMALYZER_COLUMN_CRC, xmlHandler.redBrush);
	      }
	      qApp->processEvents();
	    }

	    // SHA1
	    QString sha1Str = childItem->text(QMC2_ROMALYZER_COLUMN_SHA1);
	    if ( !sha1Str.isEmpty() && checkBoxCalculateSHA1->isChecked() ) {
	      fileItem->setText(QMC2_ROMALYZER_COLUMN_SHA1, sha1Calculated);
	      fileStatus += tr("SHA1 ");
	      if ( sha1Str != sha1Calculated && hasDump ) {
		somethingsWrong = true;
		fileItem->setForeground(QMC2_ROMALYZER_COLUMN_SHA1, xmlHandler.redBrush);
	      } else if ( hasDump )
		eligibleForDatabaseUpload = true;
	      if ( wizardSelectedSets.contains(gameName) ) {
		QList<QTreeWidgetItem *> il = treeWidgetChecksumWizardSearchResult->findItems(gameName, Qt::MatchExactly, QMC2_ROMALYZER_CSWIZ_COLUMN_ID);
		foreach (QTreeWidgetItem *item, il)
		  if ( item->text(QMC2_ROMALYZER_CSWIZ_COLUMN_FILENAME) == childItem->text(QMC2_ROMALYZER_COLUMN_GAME) ||
		       item->whatsThis(QMC2_ROMALYZER_CSWIZ_COLUMN_FILENAME) == childItem->text(QMC2_ROMALYZER_COLUMN_CRC) ) {
		    item->setText(QMC2_ROMALYZER_CSWIZ_COLUMN_STATUS, eligibleForDatabaseUpload ? tr("good") : tr("bad"));
		    item->setForeground(QMC2_ROMALYZER_CSWIZ_COLUMN_STATUS, eligibleForDatabaseUpload ? xmlHandler.greenBrush : xmlHandler.redBrush);
		    item->setText(QMC2_ROMALYZER_CSWIZ_COLUMN_PATH, effectiveFile);
		    if ( eligibleForDatabaseUpload && zipped ) item->setWhatsThis(QMC2_ROMALYZER_CSWIZ_COLUMN_FILENAME, childItem->text(QMC2_ROMALYZER_COLUMN_CRC));
		  }
	        on_treeWidgetChecksumWizardSearchResult_itemSelectionChanged();
	      }
	      qApp->processEvents();
	    }

	    // MD5
	    QString md5Str = childItem->text(QMC2_ROMALYZER_COLUMN_MD5);
	    if ( !md5Str.isEmpty() && checkBoxCalculateMD5->isChecked() ) {
	      fileItem->setText(QMC2_ROMALYZER_COLUMN_MD5, md5Calculated);
	      fileStatus += tr("MD5 ");
	      if ( md5Str != md5Calculated && hasDump ) {
		somethingsWrong = true;
		fileItem->setForeground(QMC2_ROMALYZER_COLUMN_MD5, xmlHandler.redBrush);
	      }
	      qApp->processEvents();
	    }
	  }

	  childItem->setText(QMC2_ROMALYZER_COLUMN_FILESTATUS, fileStatus);
	  if ( somethingsWrong ) {
	    gameOkay = false;
	    childItem->setForeground(QMC2_ROMALYZER_COLUMN_FILESTATUS, xmlHandler.redBrush);
	    childItem->setIcon(QMC2_ROMALYZER_COLUMN_GAME, QIcon(QString::fromUtf8(":/data/img/warning.png")));
	    log(tr("WARNING: %1 file '%2' loaded from '%3' has incorrect / unexpected checksums").arg(isCHD ? tr("CHD") : tr("ROM")).arg(childItem->text(QMC2_ROMALYZER_COLUMN_GAME)).arg(effectiveFile));
	  } else {
	    if ( fileStatus == tr("skipped") ) {
	      childItem->setForeground(QMC2_ROMALYZER_COLUMN_FILESTATUS, xmlHandler.blueBrush);
	    } else if ( fileStatus == tr("not found") ) {
	      childItem->setForeground(QMC2_ROMALYZER_COLUMN_FILESTATUS, xmlHandler.greyBrush);
	    } else if ( fileStatus == tr("no dump") ) {
	      childItem->setForeground(QMC2_ROMALYZER_COLUMN_FILESTATUS, xmlHandler.brownBrush);
	    } else {
	      childItem->setForeground(QMC2_ROMALYZER_COLUMN_FILESTATUS, xmlHandler.greenBrush);

	      if ( eligibleForDatabaseUpload ) {
		// FIXME: add optional DB upload through ROM database manager here...
	      }
	    }
	  }

	  if ( checkBoxExpandChecksums->isChecked() && checkBoxExpandFiles->isChecked() )
	    childItem->setExpanded(true);

	  qApp->processEvents();
	}
      }

      if ( xmlHandler.fileCounter == 0 )
	progressBar->setRange(0, 1);
      progressBar->reset();
      qApp->processEvents();

      if ( qmc2StopParser ) 
	log(tr("interrupted (checking %n file(s) for '%1')", "", wizardSearch ? numWizardFiles : xmlHandler.fileCounter).arg(gameName));
      else {
	gameOkay |= filesError;
	filesSkipped |= filesUnknown;
	if ( !wizardSearch ) {
		if ( gameOkay ) {
		  if ( notFoundCounter == xmlHandler.fileCounter ) {
#if defined(QMC2_EMUTYPE_MAME)
		    xmlHandler.parentItem->setText(QMC2_ROMALYZER_COLUMN_FILESTATUS, tr("not found"));
		    xmlHandler.parentItem->setForeground(QMC2_ROMALYZER_COLUMN_FILESTATUS, xmlHandler.greyBrush);
#elif defined(QMC2_EMUTYPE_MESS)
		    if ( xmlHandler.fileCounter == 0 ) {
			    xmlHandler.parentItem->setText(QMC2_ROMALYZER_COLUMN_FILESTATUS, tr("good"));
			    xmlHandler.parentItem->setForeground(QMC2_ROMALYZER_COLUMN_FILESTATUS, xmlHandler.greenBrush);
		    } else {
			    xmlHandler.parentItem->setText(QMC2_ROMALYZER_COLUMN_FILESTATUS, tr("not found"));
			    xmlHandler.parentItem->setForeground(QMC2_ROMALYZER_COLUMN_FILESTATUS, xmlHandler.greyBrush);
		    }
#endif
		  } else if ( notFoundCounter > 0 ) {
		    if ( filesSkipped )
		      xmlHandler.parentItem->setText(QMC2_ROMALYZER_COLUMN_FILESTATUS, tr("good / not found / skipped"));
		    else
		      xmlHandler.parentItem->setText(QMC2_ROMALYZER_COLUMN_FILESTATUS, tr("good / not found"));
		    xmlHandler.parentItem->setForeground(QMC2_ROMALYZER_COLUMN_FILESTATUS, xmlHandler.brownBrush);
		  } else if ( noDumpCounter > 0 ) {
			  if ( filesSkipped )
				  xmlHandler.parentItem->setText(QMC2_ROMALYZER_COLUMN_FILESTATUS, tr("good / no dump / skipped"));
			  else
				  xmlHandler.parentItem->setText(QMC2_ROMALYZER_COLUMN_FILESTATUS, tr("good / no dump"));
			  xmlHandler.parentItem->setForeground(QMC2_ROMALYZER_COLUMN_FILESTATUS, xmlHandler.brownBrush);
		  } else {
		    if ( filesSkipped )
		      xmlHandler.parentItem->setText(QMC2_ROMALYZER_COLUMN_FILESTATUS, tr("good / skipped"));
		    else
		      xmlHandler.parentItem->setText(QMC2_ROMALYZER_COLUMN_FILESTATUS, tr("good"));
		    xmlHandler.parentItem->setForeground(QMC2_ROMALYZER_COLUMN_FILESTATUS, xmlHandler.greenBrush);
		  }
		} else {
		  if ( notFoundCounter > 0 ) {
		    if ( filesSkipped )
		      xmlHandler.parentItem->setText(QMC2_ROMALYZER_COLUMN_FILESTATUS, tr("bad / not found / skipped"));
		    else
		      xmlHandler.parentItem->setText(QMC2_ROMALYZER_COLUMN_FILESTATUS, tr("bad / not found"));
		    xmlHandler.parentItem->setForeground(QMC2_ROMALYZER_COLUMN_FILESTATUS, xmlHandler.redBrush);
		  } else if ( noDumpCounter > 0 ) {
			  if ( filesSkipped )
				  xmlHandler.parentItem->setText(QMC2_ROMALYZER_COLUMN_FILESTATUS, tr("bad / no dump / skipped"));
			  else
				  xmlHandler.parentItem->setText(QMC2_ROMALYZER_COLUMN_FILESTATUS, tr("bad / no dump"));
			  xmlHandler.parentItem->setForeground(QMC2_ROMALYZER_COLUMN_FILESTATUS, xmlHandler.redBrush);
		  } else {
		    if ( filesSkipped )
		      xmlHandler.parentItem->setText(QMC2_ROMALYZER_COLUMN_FILESTATUS, tr("bad / skipped"));
		    else
		      xmlHandler.parentItem->setText(QMC2_ROMALYZER_COLUMN_FILESTATUS, tr("bad"));
		    xmlHandler.parentItem->setForeground(QMC2_ROMALYZER_COLUMN_FILESTATUS, xmlHandler.redBrush);
		  }
		}
		if ( !xmlHandler.parentItem->text(QMC2_ROMALYZER_COLUMN_MERGE).isEmpty() ) {
			switch ( mergeStatus ) {
				case QMC2_ROMALYZER_MERGE_STATUS_OK:
					xmlHandler.parentItem->setIcon(QMC2_ROMALYZER_COLUMN_MERGE, QIcon(QString::fromUtf8(":/data/img/merge_ok.png")));
					break;
				case QMC2_ROMALYZER_MERGE_STATUS_WARN:
					xmlHandler.parentItem->setIcon(QMC2_ROMALYZER_COLUMN_MERGE, QIcon(QString::fromUtf8(":/data/img/merge.png")));
					break;
				case QMC2_ROMALYZER_MERGE_STATUS_CRIT:
					xmlHandler.parentItem->setIcon(QMC2_ROMALYZER_COLUMN_MERGE, QIcon(QString::fromUtf8(":/data/img/merge_nok.png")));
					break;
				default:
					break;
			}
		}
	}
	log(tr("done (checking %n file(s) for '%1')", "", wizardSearch ? numWizardFiles : xmlHandler.fileCounter).arg(gameName));

	if ( gameOkay || !checkBoxSetRewriterGoodSetsOnly->isChecked() )
		if ( groupBoxSetRewriter->isChecked() )
			if ( checkBoxSetRewriterWhileAnalyzing->isChecked() && !qmc2StopParser )
				runSetRewriter();
      }
      if ( qmc2StopParser )
	break;

      treeWidgetChecksums->update();

      i++;
      log(tr("done (analyzing '%1')").arg(gameName));
#if defined(QMC2_EMUTYPE_MAME)
      log(tr("%n game(s) left", "", analyzerList.count() - i));
#elif defined(QMC2_EMUTYPE_MESS)
      log(tr("%n machine(s) left", "", analyzerList.count() - i));
#endif
    }
  }

  animTimer.stop();
  pushButtonAnalyze->setText(tr("&Analyze"));
  pushButtonPause->setVisible(false);
  pushButtonAnalyze->setIcon(QIcon(QString::fromUtf8(":/data/img/find.png")));
  lineEditGames->setEnabled(true);
  if ( checkBoxCalculateSHA1->isChecked() ) tabChecksumWizard->setEnabled(true);

  progressBar->reset();
  labelStatus->setText(tr("Idle"));
  qApp->processEvents();
  elapsedTime = elapsedTime.addMSecs(analysisTimer.elapsed());
  log(tr("analysis ended") + " - " + tr("elapsed time = %1").arg(elapsedTime.toString("hh:mm:ss.zzz")));

  if ( wizardSearch && wizardAutomationLevel >= QMC2_ROMALYZER_CSWIZ_AMLVL_REPAIR && !qmc2StopParser ) {
    if ( pushButtonChecksumWizardRepairBadSets->isEnabled() )
      on_pushButtonChecksumWizardRepairBadSets_clicked();
  }

  qmc2ROMAlyzerActive = false;
  wizardSearch = false;
}

QString &ROMAlyzer::getXmlData(QString gameName)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::getXmlData(QString gameName = %1)").arg(gameName));
#endif

  static QString xmlBuffer;

  xmlBuffer.clear();
  int i = 0;
  int *iCached = romalyzerXmlGamePositionCache[gameName];
  if ( iCached ) i = *iCached;
  int xmlLinesCount = qmc2Gamelist->xmlLines.count();
#if defined(QMC2_EMUTYPE_MAME)
  QString s = "<game name=\"" + gameName + "\"";
#elif defined(QMC2_EMUTYPE_MESS)
  QString s = "<machine name=\"" + gameName + "\"";
#endif
  while ( !qmc2Gamelist->xmlLines[i].contains(s) ) {
    i++;
    if ( i > xmlLinesCount ) break;
  }
  if ( i < xmlLinesCount && qmc2Gamelist->xmlLines[i].contains(s) ) {
    romalyzerXmlGamePositionCache.insert(gameName, new int(i));
    xmlBuffer = "<?xml version=\"1.0\"?>\n";
#if defined(QMC2_EMUTYPE_MAME)
    while ( !qmc2Gamelist->xmlLines[i].contains("</game>") )
#elif defined(QMC2_EMUTYPE_MESS)
    while ( !qmc2Gamelist->xmlLines[i].contains("</machine>") )
#endif
      xmlBuffer += qmc2Gamelist->xmlLines[i++].simplified() + "\n";
#if defined(QMC2_EMUTYPE_MAME)
    xmlBuffer += "</game>\n";
#elif defined(QMC2_EMUTYPE_MESS)
    xmlBuffer += "</machine>\n";
#endif
  }

  return xmlBuffer;
}

QString &ROMAlyzer::getEffectiveFile(QTreeWidgetItem *myItem, QString gameName, QString fileName, QString wantedCRC, QString merge, QString mergeFile, QString type, QByteArray *fileData, QString *sha1Str, QString *md5Str, bool *isZipped, bool *mergeUsed, int fileCounter, QString *fallbackPath)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::getEffectiveFile(QTreeWidgetItem *myItem = %1, QString gameName = %2, QString fileName = %3, QString wantedCRC = %4, QString merge = %5, QString mergeFile = %6, QString type = %7, QByteArray *fileData = ..., QString *sha1Str = ..., QString md5Str = ..., bool *isZipped = ..., bool *mergeUsed = ..., int fileCounter = ..., QString *fallbackPath = ...)")
                     .arg((qulonglong)myItem).arg(gameName).arg(fileName).arg(wantedCRC).arg(merge).arg(mergeFile).arg(type));
#endif

  static QCryptographicHash sha1Hash(QCryptographicHash::Sha1);
  static QCryptographicHash md5Hash(QCryptographicHash::Md5);
  static QString effectiveFile;
  static char buffer[MAX(QMC2_ROMALYZER_ZIP_BUFFER_SIZE, QMC2_ROMALYZER_FILE_BUFFER_SIZE)];

  effectiveFile.clear();
  fileData->clear();

  bool calcMD5 = checkBoxCalculateMD5->isChecked();
  bool calcSHA1 = checkBoxCalculateSHA1->isChecked();
  bool isCHD = type.split(" ")[0] == tr("CHD");
  bool sizeLimited = spinBoxMaxFileSize->value() > 0;
  bool chdManagerVerifyCHDs = checkBoxVerifyCHDs->isChecked();
#if QMC2_CHD_CURRENT_VERSION < 5
  bool chdManagerFixCHDs = checkBoxFixCHDs->isChecked();
#endif
  bool chdManagerUpdateCHDs = checkBoxUpdateCHDs->isChecked();
  bool chdManagerEnabled = groupBoxCHDManager->isChecked() && (chdManagerVerifyCHDs || chdManagerUpdateCHDs);
  bool needProgressWidget;
  QProgressBar *progressWidget;
  qint64 totalSize, myProgress, sizeLeft, len;

  // search for file in ROM paths (first search for "game/file", then "file" in "game.zip"), load file data when found
  foreach (QString romPath, romPaths) {
    progressWidget = NULL;
    needProgressWidget = false;
    QString filePath(romPath + "/" + gameName + "/" + fileName);
    if ( isCHD ) {
      filePath += ".chd";
      if ( fallbackPath->isEmpty() ) *fallbackPath = filePath;
    }
    if ( QFile::exists(filePath) ) {
      QFileInfo fi(filePath);
      if ( fi.isReadable() ) {
        totalSize = fi.size();
        // load data from a regular file
        if ( sizeLimited ) {
          if ( totalSize > (qint64) spinBoxMaxFileSize->value() * QMC2_ONE_MEGABYTE ) {
            log(tr("size of '%1' is greater than allowed maximum -- skipped").arg(filePath));
            *isZipped = false;
            progressBarFileIO->reset();
            effectiveFile = QMC2_ROMALYZER_FILE_TOO_BIG;
            continue;
          }
        }
        QFile romFile(filePath);
        if ( romFile.open(QIODevice::ReadOnly) ) {
          log(tr("loading '%1'%2").arg(filePath).arg(*mergeUsed ? tr(" (merged)") : ""));
          progressBarFileIO->setRange(0, totalSize);
	  progressBarFileIO->reset();
          if ( totalSize > QMC2_ROMALYZER_PROGRESS_THRESHOLD ) {
            if ( isCHD && !chdManagerEnabled )
              needProgressWidget = false;
            else
              needProgressWidget = true;
            if ( needProgressWidget ) {
              progressWidget = new QProgressBar(0);
#if QMC2_CHD_CURRENT_VERSION >= 5
              if ( isCHD ) {
		      progressWidget->setRange(0, 100);
		      progressWidget->setValue(0);
              } else {
		      progressWidget->setRange(0, totalSize);
		      progressWidget->setValue(0);
	      }
#else
              progressWidget->setRange(0, totalSize);
              progressWidget->setValue(0);
#endif
              treeWidgetChecksums->setItemWidget(myItem, QMC2_ROMALYZER_COLUMN_FILESTATUS, progressWidget);
            }
          } else
            needProgressWidget = false;
          sizeLeft = totalSize;
          if ( calcSHA1 )
            sha1Hash.reset();
          if ( calcMD5 )
            md5Hash.reset();
          if ( isCHD ) {
            QString chdFilePath;
            quint32 chdTotalHunks = 0;
            if ( (len = romFile.read(buffer, QMC2_CHD_HEADER_V3_LENGTH)) > 0 ) {
              log(tr("CHD header information:"));
              QByteArray chdTag(buffer + QMC2_CHD_HEADER_TAG_OFFSET, QMC2_CHD_HEADER_TAG_LENGTH);
              log(tr("  tag: %1").arg(chdTag.constData()));
              quint32 chdVersion = QMC2_TO_UINT32(buffer + QMC2_CHD_HEADER_VERSION_OFFSET);
              log(tr("  version: %1").arg(chdVersion));
              myItem->setText(QMC2_ROMALYZER_COLUMN_TYPE, tr("CHD v%1").arg(chdVersion));
	      QLocale locale;
              switch ( chdVersion ) {
                case 3: {
                  quint32 chdCompression = QMC2_TO_UINT32(buffer + QMC2_CHD_HEADER_COMPRESSION_OFFSET);
                  log(tr("  compression: %1").arg(chdCompressionTypes[chdCompression]));
                  quint32 chdFlags = QMC2_TO_UINT32(buffer + QMC2_CHD_HEADER_FLAGS_OFFSET);
                  log(tr("  flags: %1, %2").arg(chdFlags & QMC2_CHD_HEADER_FLAG_HASPARENT ? tr("has parent") : tr("no parent")).arg(chdFlags & QMC2_CHD_HEADER_FLAG_ALLOWSWRITES ? tr("allows writes") : tr("read only")));
                  chdTotalHunks = QMC2_TO_UINT32(buffer + QMC2_CHD_HEADER_V3_TOTALHUNKS_OFFSET);
                  log(tr("  number of total hunks: %1").arg(locale.toString(chdTotalHunks)));
                  quint32 chdHunkBytes = QMC2_TO_UINT32(buffer + QMC2_CHD_HEADER_V3_HUNKBYTES_OFFSET);
                  log(tr("  number of bytes per hunk: %1").arg(locale.toString(chdHunkBytes)));
                  quint64 chdLogicalBytes = QMC2_TO_UINT64(buffer + QMC2_CHD_HEADER_V3_LOGICALBYTES_OFFSET);
                  log(tr("  logical size: %1 (%2 B)").arg(humanReadable(chdLogicalBytes)).arg(locale.toString(chdLogicalBytes)));
                  QByteArray md5Data((const char *)(buffer + QMC2_CHD_HEADER_V3_MD5_OFFSET), QMC2_CHD_HEADER_V3_MD5_LENGTH);
                  log(tr("  MD5 checksum: %1").arg(QString(md5Data.toHex())));
                  QByteArray sha1Data((const char *)(buffer + QMC2_CHD_HEADER_V3_SHA1_OFFSET), QMC2_CHD_HEADER_V3_SHA1_LENGTH);
                  log(tr("  SHA1 checksum: %1").arg(QString(sha1Data.toHex())));
                  if ( chdFlags & QMC2_CHD_HEADER_FLAG_HASPARENT ) {
                    QByteArray md5Data((const char *)(buffer + QMC2_CHD_HEADER_V3_PARENTMD5_OFFSET), QMC2_CHD_HEADER_V3_PARENTMD5_LENGTH);
                    log(tr("  parent CHD's MD5 checksum: %1").arg(QString(md5Data.toHex())));
                    QByteArray sha1Data((const char *)(buffer + QMC2_CHD_HEADER_V3_PARENTSHA1_OFFSET), QMC2_CHD_HEADER_V3_PARENTSHA1_LENGTH);
                    log(tr("  parent CHD's SHA1 checksum: %1").arg(QString(sha1Data.toHex())));
                  }
                }
                break;

              case 4: {
                  quint32 chdCompression = QMC2_TO_UINT32(buffer + QMC2_CHD_HEADER_COMPRESSION_OFFSET);
                  log(tr("  compression: %1").arg(chdCompressionTypes[chdCompression]));
                  quint32 chdFlags = QMC2_TO_UINT32(buffer + QMC2_CHD_HEADER_FLAGS_OFFSET);
                  log(tr("  flags: %1, %2").arg(chdFlags & QMC2_CHD_HEADER_FLAG_HASPARENT ? tr("has parent") : tr("no parent")).arg(chdFlags & QMC2_CHD_HEADER_FLAG_ALLOWSWRITES ? tr("allows writes") : tr("read only")));
                  chdTotalHunks = QMC2_TO_UINT32(buffer + QMC2_CHD_HEADER_V4_TOTALHUNKS_OFFSET);
                  log(tr("  number of total hunks: %1").arg(locale.toString(chdTotalHunks)));
                  quint32 chdHunkBytes = QMC2_TO_UINT32(buffer + QMC2_CHD_HEADER_V4_HUNKBYTES_OFFSET);
                  log(tr("  number of bytes per hunk: %1").arg(locale.toString(chdHunkBytes)));
                  quint64 chdLogicalBytes = QMC2_TO_UINT64(buffer + QMC2_CHD_HEADER_V4_LOGICALBYTES_OFFSET);
                  log(tr("  logical size: %1 (%2 B)").arg(humanReadable(chdLogicalBytes)).arg(locale.toString(chdLogicalBytes)));
                  QByteArray sha1Data((const char *)(buffer + QMC2_CHD_HEADER_V4_SHA1_OFFSET), QMC2_CHD_HEADER_V4_SHA1_LENGTH);
                  log(tr("  SHA1 checksum: %1").arg(QString(sha1Data.toHex())));
                  if ( chdFlags & QMC2_CHD_HEADER_FLAG_HASPARENT ) {
                    QByteArray sha1Data((const char *)(buffer + QMC2_CHD_HEADER_V4_PARENTSHA1_OFFSET), QMC2_CHD_HEADER_V4_PARENTSHA1_LENGTH);
                    log(tr("  parent CHD's SHA1 checksum: %1").arg(QString(sha1Data.toHex())));
                  }
                  QByteArray rawsha1Data((const char *)(buffer + QMC2_CHD_HEADER_V4_RAWSHA1_OFFSET), QMC2_CHD_HEADER_V4_SHA1_LENGTH);
                  log(tr("  raw SHA1 checksum: %1").arg(QString(rawsha1Data.toHex())));
                }
                break;

              case 5: {
		  QString chdCompressors;
		  for (int i = 0; i < QMC2_CHD_HEADER_V5_COMPRESSORS_COUNT; i++) {
			  QString compressor = QString::fromAscii(buffer + i * 4 + QMC2_CHD_HEADER_V5_COMPRESSORS_OFFSET, 4);
			  if ( chdCompressionTypesV5.contains(compressor) ) {
				  if ( i > 0 ) chdCompressors += ", ";
				  chdCompressors += chdCompressionTypesV5[compressor];
			  } else if ( i == 0 ) {
				  chdCompressors = tr("none (uncompressed)");
				  break;
			  } else
				  break;
		  }
		  log(tr("  compressors: %1").arg(chdCompressors));
		  quint32 chdHunkBytes = QMC2_TO_UINT32(buffer + QMC2_CHD_HEADER_V5_HUNKBYTES_OFFSET);
		  log(tr("  number of bytes per hunk: %1").arg(locale.toString(chdHunkBytes)));
		  quint32 chdUnitBytes = QMC2_TO_UINT32(buffer + QMC2_CHD_HEADER_V5_UNITBYTES_OFFSET);
		  log(tr("  number of bytes per unit: %1").arg(locale.toString(chdUnitBytes)));
		  quint64 chdLogicalBytes = QMC2_TO_UINT64(buffer + QMC2_CHD_HEADER_V5_LOGICALBYTES_OFFSET);
		  log(tr("  logical size: %1 (%2 B)").arg(humanReadable(chdLogicalBytes)).arg(locale.toString(chdLogicalBytes)));
		  QByteArray sha1Data((const char *)(buffer + QMC2_CHD_HEADER_V5_SHA1_OFFSET), QMC2_CHD_HEADER_V5_SHA1_LENGTH);
		  log(tr("  SHA1 checksum: %1").arg(QString(sha1Data.toHex())));
		  QByteArray parentSha1DataHex = QByteArray((const char *)(buffer + QMC2_CHD_HEADER_V5_PARENTSHA1_OFFSET), QMC2_CHD_HEADER_V5_PARENTSHA1_LENGTH).toHex();
		  if ( parentSha1DataHex.toInt(0, 16) != 0 )
			  log(tr("  parent CHD's SHA1 checksum: %1").arg(QString(parentSha1DataHex)));
		  QByteArray rawsha1Data((const char *)(buffer + QMC2_CHD_HEADER_V5_RAWSHA1_OFFSET), QMC2_CHD_HEADER_V5_SHA1_LENGTH);
		  log(tr("  raw SHA1 checksum: %1").arg(QString(rawsha1Data.toHex())));
		}
		break;

               default:
                 log(tr("CHD v%1 not supported -- rest of header information skipped").arg(chdVersion));
                 break;
              }

              if ( calcSHA1 || calcMD5 || chdManagerEnabled ) {
                chdFilePath = fi.absoluteFilePath();
                QString chdTempFilePath = lineEditTemporaryWorkingDirectory->text() + fi.baseName() + "-chdman-update.chd";
                if ( chdManagerEnabled ) {
                  romFile.close();
                  chdManagerCurrentHunk = 0;
#if QMC2_CHD_CURRENT_VERSION >= 5
                  chdTotalHunks = 100;
#endif
                  chdManagerTotalHunks = chdTotalHunks;
                  if ( progressWidget ) {
                    progressWidget->setRange(0, chdTotalHunks);
                    progressWidget->setValue(0);
                  }
                  progressBarFileIO->setRange(0, chdTotalHunks);
                  progressBarFileIO->reset();
                  int step;
                  for (step = 0; step < 2 && !qmc2StopParser; step++) {
                    QStringList args;
                    QString oldFormat;
                    if ( progressWidget ) oldFormat = progressWidget->format();
                    switch ( step ) {
                      case 0:
                        if ( chdManagerVerifyCHDs ) {
                          if ( progressWidget ) progressWidget->setFormat(tr("Verify - %p%"));
#if QMC2_CHD_CURRENT_VERSION >= 5
			  log(tr("CHD manager: verifying CHD"));
			  args << "verify" << "--input" << chdFilePath;
#else
			  if ( chdManagerFixCHDs ) {
				  log(tr("CHD manager: verifying and fixing CHD"));
				  args << "-verifyfix" << chdFilePath;
			  } else {
				  log(tr("CHD manager: verifying CHD"));
				  args << "-verify" << chdFilePath;
			  }
#endif
                        } else
                          continue;
                        break;

                      case 1:
                        if ( chdManagerUpdateCHDs ) {
                          if ( chdVersion < QMC2_CHD_CURRENT_VERSION ) {
                            if ( progressWidget ) progressWidget->setFormat(tr("Update - %p%"));
                            log(tr("CHD manager: updating CHD (v%1 -> v%2)").arg(chdVersion).arg(QMC2_CHD_CURRENT_VERSION));
#if QMC2_CHD_CURRENT_VERSION >= 5
                            args << "copy" << "--input" << chdFilePath << "--output" << chdTempFilePath;
#else
                            args << "-update" << chdFilePath << chdTempFilePath;
#endif
                          } else if ( !chdManagerVerifyCHDs ) {
                            switch ( chdVersion ) {
                              case 3:
                                log(tr("CHD manager: using header checksums for CHD verification"));
                                if ( calcSHA1 ) {
                                  QByteArray sha1Data((const char *)(buffer + QMC2_CHD_HEADER_V3_SHA1_OFFSET), QMC2_CHD_HEADER_V3_SHA1_LENGTH);
                                  *sha1Str = QString(sha1Data.toHex());
                                }
                                if ( calcMD5 ) {
                                  QByteArray md5Data((const char *)(buffer + QMC2_CHD_HEADER_V3_MD5_OFFSET), QMC2_CHD_HEADER_V3_MD5_LENGTH);
                                  *md5Str = QString(md5Data.toHex());
                                }
                                break;

                              case 4:
                                log(tr("CHD manager: using header checksums for CHD verification"));
                                if ( calcSHA1 ) {
                                  QByteArray sha1Data((const char *)(buffer + QMC2_CHD_HEADER_V4_SHA1_OFFSET), QMC2_CHD_HEADER_V4_SHA1_LENGTH);
                                  *sha1Str = QString(sha1Data.toHex());
                                }
                                break;

                              case 5:
                                log(tr("CHD manager: using header checksums for CHD verification"));
                                if ( calcSHA1 ) {
                                  QByteArray sha1Data((const char *)(buffer + QMC2_CHD_HEADER_V5_SHA1_OFFSET), QMC2_CHD_HEADER_V5_SHA1_LENGTH);
                                  *sha1Str = QString(sha1Data.toHex());
                                }
                                break;

                              default:
                                log(tr("CHD manager: no header checksums available for CHD verification"));
                                effectiveFile = QMC2_ROMALYZER_FILE_NOT_SUPPORTED;
                                if ( fallbackPath->isEmpty() ) *fallbackPath = chdFilePath;
                                break;
                            }
                            continue;
                          } else
                            continue;
                        } else
                          continue;
                        break;
                    }
                    QString command = lineEditCHDManagerExecutableFile->text();
                    QProcess *chdManagerProc = new QProcess(this);
                    connect(chdManagerProc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(chdManagerError(QProcess::ProcessError)));
                    connect(chdManagerProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(chdManagerFinished(int, QProcess::ExitStatus)));
                    connect(chdManagerProc, SIGNAL(readyReadStandardOutput()), this, SLOT(chdManagerReadyReadStandardOutput()));
                    connect(chdManagerProc, SIGNAL(readyReadStandardError()), this, SLOT(chdManagerReadyReadStandardError()));
                    connect(chdManagerProc, SIGNAL(started()), this, SLOT(chdManagerStarted()));
                    connect(chdManagerProc, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(chdManagerStateChanged(QProcess::ProcessState)));
                    chdManagerProc->start(command, args);
                    chdManagerRunning = true;
                    chdManagerMD5Success = chdManagerSHA1Success = false;

                    // wait for CHD manager to finish...
                    while ( chdManagerRunning && !qmc2StopParser ) {
                      QTest::qWait(QMC2_ROMALYZER_PAUSE_TIMEOUT);
                      if ( qmc2StopParser ) {
                        log(tr("CHD manager: terminating external process"));
                        chdManagerProc->kill();
                        chdManagerProc->waitForFinished();
			disconnect(chdManagerProc);
			chdManagerProc->deleteLater();
                      } else {
                        if ( progressWidget ) {
                          if ( chdManagerTotalHunks != (quint64)progressWidget->maximum() )
                            progressWidget->setRange(0, chdManagerTotalHunks);
                          if ( chdManagerCurrentHunk != (quint64)progressWidget->value() )
                            progressWidget->setValue(chdManagerCurrentHunk);
                        }
                        if ( chdManagerTotalHunks != (quint64)progressBarFileIO->maximum() )
                          progressBarFileIO->setRange(0, chdManagerTotalHunks);
                        if ( chdManagerCurrentHunk != (quint64)progressBarFileIO->value() )
                          progressBarFileIO->setValue(chdManagerCurrentHunk);
                        progressBarFileIO->update();
			qApp->processEvents();
                      }
                    }
                    chdManagerRunning = false;
                    if ( !qmc2StopParser ) {
                      if ( chdManagerMD5Success && calcMD5 )
                        log(tr("CHD manager: CHD file integrity is good"));
		      else if ( chdManagerSHA1Success && calcSHA1 )
                        log(tr("CHD manager: CHD file integrity is good"));
                      else
                        log(tr("CHD manager: WARNING: CHD file integrity is bad"));

                      switch ( chdVersion ) {
                        case 3:
                          log(tr("CHD manager: using CHD v%1 header checksums for CHD verification").arg(chdVersion));
                          if ( chdManagerSHA1Success && calcSHA1 ) {
                            QByteArray sha1Data((const char *)(buffer + QMC2_CHD_HEADER_V3_SHA1_OFFSET), QMC2_CHD_HEADER_V3_SHA1_LENGTH);
                            *sha1Str = QString(sha1Data.toHex());
                          }
                          if ( chdManagerMD5Success && calcMD5 ) {
                            QByteArray md5Data((const char *)(buffer + QMC2_CHD_HEADER_V3_MD5_OFFSET), QMC2_CHD_HEADER_V3_MD5_LENGTH);
                            *md5Str = QString(md5Data.toHex());
                          }
                          break;

                        case 4:
                          log(tr("CHD manager: using CHD v%1 header checksums for CHD verification").arg(chdVersion));
                          if ( chdManagerSHA1Success && calcSHA1 ) {
                            QByteArray sha1Data((const char *)(buffer + QMC2_CHD_HEADER_V4_SHA1_OFFSET), QMC2_CHD_HEADER_V4_SHA1_LENGTH);
                            *sha1Str = QString(sha1Data.toHex());
                          }
                          break;

			case 5:
			  log(tr("CHD manager: using CHD v%1 header checksums for CHD verification").arg(chdVersion));
			  if ( chdManagerSHA1Success && calcSHA1 ) {
				  QByteArray sha1Data((const char *)(buffer + QMC2_CHD_HEADER_V5_SHA1_OFFSET), QMC2_CHD_HEADER_V5_SHA1_LENGTH);
				  *sha1Str = QString(sha1Data.toHex());
			  }
			  break;

                        default:
                          log(tr("CHD manager: WARNING: no header checksums available for CHD verification"));
                          effectiveFile = QMC2_ROMALYZER_FILE_NOT_SUPPORTED;
                          if ( fallbackPath->isEmpty() ) *fallbackPath = chdFilePath;
                          break;
                      }

                      if ( step == 1 && (chdManagerMD5Success || chdManagerSHA1Success) ) {
                        log(tr("CHD manager: replacing CHD"));
                        if ( progressWidget ) {
                          progressWidget->setFormat(tr("Copy"));
                          progressWidget->setRange(-1, -1);
                          progressWidget->setValue(-1);
                        }
                        QFile::remove(chdFilePath);
                        if ( QFile::rename(chdTempFilePath, chdFilePath) ) {
                          log(tr("CHD manager: CHD replaced"));
                          myItem->setText(QMC2_ROMALYZER_COLUMN_TYPE, tr("CHD v%1").arg(QMC2_CHD_CURRENT_VERSION));
                        } else
                          log(tr("CHD manager: FATAL: failed to replace CHD -- updated CHD preserved as '%1', please copy it to '%2' manually!").arg(chdTempFilePath).arg(chdFilePath));
                      }
                    }
                    if ( QFile::exists(chdTempFilePath) ) {
                      log(tr("CHD manager: cleaning up"));
                      QFile::remove(chdTempFilePath);
                    }
                    if ( progressWidget ) {
                      progressWidget->setFormat(oldFormat);
                      progressWidget->reset();
                    }
                    progressBarFileIO->reset();
                  }
                } else {
                  switch ( chdVersion ) {
                    case 3:
                      log(tr("using CHD v%1 header checksums for CHD verification").arg(chdVersion));
                      if ( calcSHA1 ) {
                        QByteArray sha1Data((const char *)(buffer + QMC2_CHD_HEADER_V3_SHA1_OFFSET), QMC2_CHD_HEADER_V3_SHA1_LENGTH);
                        *sha1Str = QString(sha1Data.toHex());
                      }
                      if ( calcMD5 ) {
                        QByteArray md5Data((const char *)(buffer + QMC2_CHD_HEADER_V3_MD5_OFFSET), QMC2_CHD_HEADER_V3_MD5_LENGTH);
                        *md5Str = QString(md5Data.toHex());
                      }
                      break;

                    case 4:
                      log(tr("using CHD v%1 header checksums for CHD verification").arg(chdVersion));
                      if ( calcSHA1 ) {
                        QByteArray sha1Data((const char *)(buffer + QMC2_CHD_HEADER_V4_SHA1_OFFSET), QMC2_CHD_HEADER_V4_SHA1_LENGTH);
                        *sha1Str = QString(sha1Data.toHex());
                      }
                      break;

		    case 5:
		      log(tr("CHD manager: using CHD v%1 header checksums for CHD verification").arg(chdVersion));
		      if ( calcSHA1 ) {
			      QByteArray sha1Data((const char *)(buffer + QMC2_CHD_HEADER_V5_SHA1_OFFSET), QMC2_CHD_HEADER_V5_SHA1_LENGTH);
			      *sha1Str = QString(sha1Data.toHex());
		      }
		      break;

                    default:
                      log(tr("WARNING: no header checksums available for CHD verification"));
                      effectiveFile = QMC2_ROMALYZER_FILE_NOT_SUPPORTED;
                      if ( fallbackPath->isEmpty() ) *fallbackPath = chdFilePath;
                      break;
                  }
                }
              }
            } else {
              log(tr("WARNING: can't read CHD header information"));
              effectiveFile = QMC2_ROMALYZER_FILE_ERROR;
              if ( fallbackPath->isEmpty() ) *fallbackPath = chdFilePath;
            }
          } else {
            while ( (len = romFile.read(buffer, QMC2_ROMALYZER_FILE_BUFFER_SIZE)) > 0 && !qmc2StopParser ) {
              QByteArray readData((const char *)buffer, len);
              fileData->append(readData);
              if ( calcSHA1 )
                sha1Hash.addData(readData);
              if ( calcMD5 )
                md5Hash.addData(readData);
              sizeLeft -= len;
              myProgress = totalSize - sizeLeft;
              progressBarFileIO->setValue(myProgress);
              if ( needProgressWidget )
                if ( progressWidget ) progressWidget->setValue(myProgress);
              progressBarFileIO->update();
              qApp->processEvents();
            }
            if ( calcSHA1 )
              *sha1Str = sha1Hash.result().toHex();
            if ( calcMD5 )
              *md5Str = md5Hash.result().toHex();
          }
          romFile.close();
          effectiveFile = filePath;
          *isZipped = false;
          if ( needProgressWidget ) {
            if ( progressWidget ) {
              progressWidget->reset();
              treeWidgetChecksums->removeItemWidget(myItem, QMC2_ROMALYZER_COLUMN_FILESTATUS);
            }
            if ( progressWidget ) delete progressWidget;
          }
          progressBarFileIO->reset();
        } else {
          log(tr("WARNING: found '%1' but can't read from it although permissions seem okay - check file integrity").arg(filePath));
        }
      } else
        log(tr("WARNING: found '%1' but can't read from it - check permission").arg(filePath));
    } else {
      if ( isCHD ) {
        log(tr("WARNING: CHD file '%1' not found").arg(filePath));
        if ( mergeFile.isEmpty() && merge.isEmpty() )
          if ( fallbackPath->isEmpty() ) *fallbackPath = filePath;
      }
    }

    if ( effectiveFile.isEmpty() && !qmc2StopParser ) {
      filePath = romPath + "/" + gameName + ".zip";
      if ( fallbackPath->isEmpty() ) *fallbackPath = filePath;
      if ( QFile::exists(filePath) ) {
        QFileInfo fi(filePath);
        if ( fi.isReadable() ) {
          // load data from ZIP
          unzFile zipFile = unzOpen((const char *)filePath.toAscii());
          if ( zipFile ) {
            // identify file by CRC!
            unz_file_info zipInfo;
            QMap<uLong, QString> crcIdentMap;
            uLong ulCRC = wantedCRC.toULong(0, 16);
            do {
              if ( unzGetCurrentFileInfo(zipFile, &zipInfo, buffer, QMC2_ROMALYZER_ZIP_BUFFER_SIZE, 0, 0, 0, 0) == UNZ_OK )
                crcIdentMap[zipInfo.crc] = QString((const char *)buffer);
            } while ( unzGoToNextFile(zipFile) == UNZ_OK && !crcIdentMap.contains(ulCRC) );
            unzGoToFirstFile(zipFile);
            QString fn = "QMC2_DUMMY_FILENAME";
            if ( crcIdentMap.contains(ulCRC) )
              fn = crcIdentMap[ulCRC];
            else if ( mergeFile.isEmpty() ) {
              if ( !isCHD ) {
		      if ( wantedCRC.isEmpty() ) {
			      log(tr("WARNING: unable to identify '%1' from '%2' by CRC (no dump exists / CRC unknown)").arg(fileName).arg(filePath));
			      effectiveFile = QMC2_ROMALYZER_NO_DUMP;
		      } else
			      log(tr("WARNING: unable to identify '%1' from '%2' by CRC '%3'").arg(fileName).arg(filePath).arg(wantedCRC));
              }
              fn = fileName;
            }

            if ( unzLocateFile(zipFile, (const char *)fn.toAscii(), 2) == UNZ_OK ) { // NOT case-sensitive filename compare!
              totalSize = 0;
              if ( unzGetCurrentFileInfo(zipFile, &zipInfo, 0, 0, 0, 0, 0, 0) == UNZ_OK ) 
                totalSize = zipInfo.uncompressed_size;
              if ( sizeLimited ) {
                if ( totalSize > (qint64) spinBoxMaxFileSize->value() * QMC2_ONE_MEGABYTE ) {
                  log(tr("size of '%1' from '%2' is greater than allowed maximum -- skipped").arg(fn).arg(filePath));
                  *isZipped = true;
                  progressBarFileIO->reset();
                  effectiveFile = QMC2_ROMALYZER_FILE_TOO_BIG;
                  if ( fallbackPath->isEmpty() ) *fallbackPath = filePath;
                  continue;
                }
              }
              if ( unzOpenCurrentFile(zipFile) == UNZ_OK ) {
                log(tr("loading '%1' with CRC '%2' from '%3' as '%4'%5").arg(fn).arg(wantedCRC).arg(filePath).arg(myItem->text(QMC2_ROMALYZER_COLUMN_GAME)).arg(*mergeUsed ? tr(" (merged)") : ""));
                progressBarFileIO->setRange(0, totalSize);
                progressBarFileIO->reset();
		needProgressWidget = totalSize > QMC2_ROMALYZER_PROGRESS_THRESHOLD;
                if ( needProgressWidget ) {
                  progressWidget = new QProgressBar(0);
                  progressWidget->setRange(0, totalSize);
                  progressWidget->setValue(0);
                  treeWidgetChecksums->setItemWidget(myItem, QMC2_ROMALYZER_COLUMN_FILESTATUS, progressWidget);
                }
                sizeLeft = totalSize;
                if ( calcSHA1 )
                  sha1Hash.reset();
                if ( calcMD5 )
                  md5Hash.reset();
                while ( (len = unzReadCurrentFile(zipFile, buffer, QMC2_ROMALYZER_ZIP_BUFFER_SIZE)) > 0 && !qmc2StopParser ) {
                  QByteArray readData((const char *)buffer, len);
                  if ( !isCHD )
                    fileData->append(readData);
                  if ( calcSHA1 )
                    sha1Hash.addData(readData);
                  if ( calcMD5 )
                    md5Hash.addData(readData);
                  sizeLeft -= len;
                  myProgress = totalSize - sizeLeft;
                  progressBarFileIO->setValue(myProgress);
                  if ( needProgressWidget )
                    progressWidget->setValue(myProgress);
                  progressBarFileIO->update();
                  qApp->processEvents();
                }
                unzCloseCurrentFile(zipFile);
                effectiveFile = filePath;
                if ( fallbackPath->isEmpty() ) *fallbackPath = filePath;
		QString fromName = fileName;
                if ( fn != "QMC2_DUMMY_FILENAME" ) fromName = fn;
                if ( !wantedCRC.isEmpty() ) {
                  QStringList sl;
                  //    fromName    fromPath    toName
                  sl << fromName << filePath << myItem->text(QMC2_ROMALYZER_COLUMN_GAME);
                  setRewriterFileMap.insert(wantedCRC, sl); 
                } else {
                  ulong crc = crc32(0, NULL, 0);
	          crc = crc32(crc, (const Bytef *)fileData->data(), fileData->size());
                  QString fallbackCRC = QString::number(crc, 16).rightJustified(8, '0');
                  if ( !fallbackCRC.isEmpty() ) {
                    QStringList sl;
                    //    fromName    fromPath    toName
                    sl << fromName << filePath << myItem->text(QMC2_ROMALYZER_COLUMN_GAME);
                    setRewriterFileMap.insert(fallbackCRC, sl); 
                    if ( groupBoxSetRewriter->isChecked() )
                      log(tr("WARNING: the CRC for '%1' from '%2' is unknown to the emulator, the set rewriter will use the recalculated CRC '%3' to qualify the file").arg(fileName).arg(filePath).arg(fallbackCRC));
                  } else if ( groupBoxSetRewriter->isChecked() )
                    log(tr("WARNING: unable to determine the CRC for '%1' from '%2', the set rewriter will NOT store this file in the new set").arg(fileName).arg(filePath));
		}
                *isZipped = true;
                if ( calcSHA1 )
                  *sha1Str = sha1Hash.result().toHex();
                if ( calcMD5 )
                  *md5Str = md5Hash.result().toHex();
                if ( needProgressWidget ) {
                  if ( progressWidget ) {
                    progressWidget->reset();
                    treeWidgetChecksums->removeItemWidget(myItem, QMC2_ROMALYZER_COLUMN_FILESTATUS);
                  }
                  if ( progressWidget ) delete progressWidget;
                }
                progressBarFileIO->reset();
              } else
                log(tr("WARNING: unable to decompress '%1' from '%2' - check file integrity").arg(fn).arg(filePath));
            }
            unzClose(zipFile);
          } else
            log(tr("WARNING: found '%1' but can't open it for decompression - check file integrity").arg(filePath));
        } else
          log(tr("WARNING: found '%1' but can't read from it - check permission").arg(gameName));
      }
    }

    if ( !effectiveFile.isEmpty() || qmc2StopParser )
      break;
  }

  // try merges, if applicable...
  if ( effectiveFile.isEmpty() && !qmc2StopParser ) {
    if ( mergeFile.isEmpty() && !merge.isEmpty() ) {
      // romof is set, but the merge's file name is missing... use the same file name for the merge
      mergeFile = fileName;
    }
    if ( !mergeFile.isEmpty() && !qmc2StopParser ) {
      // romof is set, and the merge's file name is given
      QString nextMerge = getXmlData(merge).split("\n")[1];
      int romofPosition = nextMerge.indexOf("romof=");
      *mergeUsed = true;
      if ( romofPosition > -1 ) {
        nextMerge = nextMerge.mid(romofPosition + 7);
        nextMerge = nextMerge.left(nextMerge.indexOf("\""));
        effectiveFile = getEffectiveFile(myItem, merge, mergeFile, wantedCRC, nextMerge, mergeFile, type, fileData, sha1Str, md5Str, isZipped, mergeUsed, fileCounter, fallbackPath);
      } else
        effectiveFile = getEffectiveFile(myItem, merge, mergeFile, wantedCRC, "", "", type, fileData, sha1Str, md5Str, isZipped, mergeUsed, fileCounter, fallbackPath);
    }
  }

  if ( effectiveFile.isEmpty() )
    effectiveFile = QMC2_ROMALYZER_FILE_NOT_FOUND;

  return effectiveFile;
}

void ROMAlyzer::on_tabWidgetAnalysis_currentChanged(int index)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::on_tabWidgetAnalysis_currentChanged(int index = %1)").arg(index));
#endif

	switch ( index ) {
		case QMC2_ROMALYZER_PAGE_LOG:
			textBrowserLog->horizontalScrollBar()->setValue(0);
			break;
		default:
			break;
	}
}

void ROMAlyzer::on_treeWidgetChecksums_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_treeWidgetChecksums_itemSelectionChanged()");
#endif

  if ( checkBoxSelectGame->isChecked() ) {
    QList<QTreeWidgetItem *> items = treeWidgetChecksums->selectedItems();
    if ( items.count() > 0 ) {
      QTreeWidgetItem *item = items[0];
      while ( (void*)item->parent() != (void *)treeWidgetChecksums && item->parent() != 0 ) item = item->parent();
      QStringList words = item->text(QMC2_ROMALYZER_COLUMN_GAME).split(" ");
      selectItem(words[0]);
    }
  }
}

void ROMAlyzer::selectItem(QString gameName)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::selectItem(QString gameName = %1)").arg(gameName));
#endif

  switch ( qmc2MainWindow->stackedWidgetView->currentIndex() ) {
    case QMC2_VIEWGAMELIST_INDEX: {
      QTreeWidgetItem *gameItem = qmc2GamelistItemMap[gameName];
      if ( gameItem ) {
        qmc2MainWindow->treeWidgetGamelist->clearSelection();
        qmc2MainWindow->treeWidgetGamelist->setCurrentItem(gameItem);
        qmc2MainWindow->treeWidgetGamelist->scrollToItem(gameItem, qmc2CursorPositioningMode);
        gameItem->setSelected(true);
      }
      break;
    }
    case QMC2_VIEWHIERARCHY_INDEX: {
      QTreeWidgetItem *hierarchyItem = qmc2HierarchyItemMap[gameName];
      if ( hierarchyItem ) {
        qmc2MainWindow->treeWidgetHierarchy->clearSelection();
        qmc2MainWindow->treeWidgetHierarchy->setCurrentItem(hierarchyItem);
        qmc2MainWindow->treeWidgetHierarchy->scrollToItem(hierarchyItem, qmc2CursorPositioningMode);
        hierarchyItem->setSelected(true);
      }
      break;
    }
#if defined(QMC2_EMUTYPE_MAME)
    case QMC2_VIEWCATEGORY_INDEX: {
      QTreeWidgetItem *categoryItem = qmc2CategoryItemMap[gameName];
      if ( categoryItem ) {
        qmc2MainWindow->treeWidgetCategoryView->clearSelection();
        qmc2MainWindow->treeWidgetCategoryView->setCurrentItem(categoryItem);
        qmc2MainWindow->treeWidgetCategoryView->scrollToItem(categoryItem, qmc2CursorPositioningMode);
        categoryItem->setSelected(true);
      }
      break;
    }
    case QMC2_VIEWVERSION_INDEX: {
      QTreeWidgetItem *versionItem = qmc2VersionItemMap[gameName];
      if ( versionItem ) {
        qmc2MainWindow->treeWidgetVersionView->clearSelection();
        qmc2MainWindow->treeWidgetVersionView->setCurrentItem(versionItem);
        qmc2MainWindow->treeWidgetVersionView->scrollToItem(versionItem, qmc2CursorPositioningMode);
        versionItem->setSelected(true);
      }
      break;
    }
#endif
  }
}

QString ROMAlyzer::humanReadable(quint64 value)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::humanReadable(quint64 value = %1)").arg(value));
#endif

  static QString humanReadableString;
  static qreal humanReadableValue;
  QLocale locale;

#if __WORDSIZE == 64
  if ( (qreal)value / (qreal)QMC2_ONE_KILOBYTE < (qreal)QMC2_ONE_KILOBYTE ) {
    humanReadableValue = (qreal)value / (qreal)QMC2_ONE_KILOBYTE;
    humanReadableString = locale.toString(humanReadableValue, 'f', 2) + QString(tr(" KB"));
  } else if ( (qreal)value / (qreal)QMC2_ONE_MEGABYTE < (qreal)QMC2_ONE_KILOBYTE ) {
    humanReadableValue = (qreal)value / (qreal)QMC2_ONE_MEGABYTE;
    humanReadableString = locale.toString(humanReadableValue, 'f', 2) + QString(tr(" MB"));
  } else if ( (qreal)value / (qreal)QMC2_ONE_GIGABYTE < (qreal)QMC2_ONE_KILOBYTE ) {
    humanReadableValue = (qreal)value / (qreal)QMC2_ONE_GIGABYTE;
    humanReadableString = locale.toString(humanReadableValue, 'f', 2) + QString(tr(" GB"));
  } else {
    humanReadableValue = (qreal)value / (qreal)QMC2_ONE_TERABYTE;
    humanReadableString = locale.toString(humanReadableValue, 'f', 2) + QString(tr(" TB"));
  }
#else
  if ( (qreal)value / (qreal)QMC2_ONE_KILOBYTE < (qreal)QMC2_ONE_KILOBYTE ) {
    humanReadableValue = (qreal)value / (qreal)QMC2_ONE_KILOBYTE;
    humanReadableString = locale.toString(humanReadableValue, 'f', 2) + QString(tr(" KB"));
  } else if ( (qreal)value / (qreal)QMC2_ONE_MEGABYTE < (qreal)QMC2_ONE_KILOBYTE ) {
    humanReadableValue = (qreal)value / (qreal)QMC2_ONE_MEGABYTE;
    humanReadableString = locale.toString(humanReadableValue, 'f', 2) + QString(tr(" MB"));
  } else {
    humanReadableValue = (qreal)value / (qreal)QMC2_ONE_GIGABYTE;
    humanReadableString = locale.toString(humanReadableValue, 'f', 2) + QString(tr(" GB"));
  }
#endif

  return humanReadableString;
}

void ROMAlyzer::log(QString message)
{
  QString msg = QTime::currentTime().toString("hh:mm:ss.zzz") + ": " + message;

  bool scrollBarMaximum = (textBrowserLog->verticalScrollBar()->value() == textBrowserLog->verticalScrollBar()->maximum());
  textBrowserLog->appendPlainText(msg);
  if ( scrollBarMaximum ) {
    textBrowserLog->update();
    qApp->processEvents();
    textBrowserLog->verticalScrollBar()->setValue(textBrowserLog->verticalScrollBar()->maximum());
  }
}

void ROMAlyzer::on_toolButtonBrowseCHDManagerExecutableFile_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_toolButtonBrowseCHDManagerExecutableFile_clicked()");
#endif

  QString s = QFileDialog::getOpenFileName(this, tr("Choose CHD manager executable file"), lineEditCHDManagerExecutableFile->text(), tr("All files (*)"));
  if ( !s.isNull() )
    lineEditCHDManagerExecutableFile->setText(s);
  raise();
}

void ROMAlyzer::on_toolButtonBrowseTemporaryWorkingDirectory_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_toolButtonBrowseTemporaryWorkingDirectory_clicked()");
#endif

  QString s = QFileDialog::getExistingDirectory(this, tr("Choose temporary working directory"), lineEditTemporaryWorkingDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  if ( !s.isNull() ) {
    if ( !s.endsWith("/") ) s += "/";
    lineEditTemporaryWorkingDirectory->setText(s);
  }
  raise();
}

void ROMAlyzer::on_toolButtonBrowseSetRewriterOutputPath_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_toolButtonBrowseSetRewriterOutputPath_clicked()");
#endif

	QString s = QFileDialog::getExistingDirectory(this, tr("Choose output directory"), lineEditSetRewriterOutputPath->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if ( !s.isNull() ) {
		if ( !s.endsWith("/") ) s += "/";
		lineEditSetRewriterOutputPath->setText(s);
	}
	raise();
}

void ROMAlyzer::on_toolButtonBrowseSetRewriterAdditionalRomPath_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_toolButtonBrowseSetRewriterAdditionalRomPath_clicked()");
#endif

	QString s = QFileDialog::getExistingDirectory(this, tr("Choose additional ROM path"), lineEditSetRewriterAdditionalRomPath->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if ( !s.isNull() )
		lineEditSetRewriterAdditionalRomPath->setText(s);
	raise();
}

void ROMAlyzer::chdManagerStarted()
{
#ifdef QMC2_DEBUG
  QProcess *proc = (QProcess *)sender();
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::chdManagerStarted(), proc = %1").arg((qulonglong)proc));
#endif

  chdManagerRunning = true;
  chdManagerCurrentHunk = 0;
  log(tr("CHD manager: external process started"));
}

void ROMAlyzer::chdManagerFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
#ifdef QMC2_DEBUG
  QProcess *proc = (QProcess *)sender();
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::chdManagerFinished(int exitCode = %1, QProcess::ExitStatus exitStatus = %2), proc = %3").arg(exitCode).arg((int)exitStatus).arg((qulonglong)proc));
#endif

  chdManagerRunning = false;
  QString statusString = tr("unknown");
  switch ( exitStatus ) {
    case QProcess::NormalExit: statusString = tr("normal"); break;
    case QProcess::CrashExit: statusString = tr("crashed"); break;
  }
  log(tr("CHD manager: external process finished (exit code = %1, exit status = %2)").arg(exitCode).arg(statusString));
}

void ROMAlyzer::chdManagerReadyReadStandardOutput()
{
  QProcess *proc = (QProcess *)sender();

#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::chdManagerReadyReadStandardOutput(), proc = %1").arg((qulonglong)proc));
#endif

  QString output = proc->readAllStandardOutput();
  QStringList sl = output.split("\n");
  foreach (QString s, sl) {
    s = s.trimmed();
    if ( !s.isEmpty() ) {
      log(tr("CHD manager: stdout: %1").arg(s));
      if ( s.contains("MD5 verification successful") )
        chdManagerMD5Success = true;
      if ( s.contains("SHA1 verification successful") )
        chdManagerSHA1Success = true;
    }
  }
}

void ROMAlyzer::chdManagerReadyReadStandardError()
{
  QProcess *proc = (QProcess *)sender();

#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::chdManagerReadyReadStandardError(), proc = %1").arg((qulonglong)proc));
#endif

  QString output = proc->readAllStandardError();
  QStringList sl = output.split("\n");
  foreach (QString s, sl) {
    s = s.trimmed();
    if ( !s.isEmpty() ) {
      log(tr("CHD manager: stderr: %1").arg(s));
#if QMC2_CHD_CURRENT_VERSION >= 5
      if ( s.contains(QRegExp(", \\d+\\.\\d+\\%\\ complete\\.\\.\\.")) ) {
        QRegExp rx(", (\\d+)\\.(\\d+)\\%\\ complete\\.\\.\\.");
        int pos = rx.indexIn(s);
        if ( pos > -1 ) {
          chdManagerCurrentHunk = rx.cap(1).toInt(); // 'current hunk' is misused as a percentage value, and 'total hunks' is thus set to 100 constantly
          int decimal = rx.cap(2).toInt();
	  if ( decimal >= 5 ) chdManagerCurrentHunk += 1;
	}
      } else {
        if ( s.contains("Compression complete ... final ratio =") )
          chdManagerSHA1Success = true;
      }
#else
      if ( s.contains(QRegExp("hunk \\d+/\\d+\\.\\.\\.")) ) {
        QRegExp rx("(\\d+)/(\\d+)");
        int pos = rx.indexIn(s);
        if ( pos > -1 ) {
          chdManagerCurrentHunk = rx.cap(1).toInt();
          chdManagerTotalHunks = rx.cap(2).toInt();
        }
      } else {
        if ( s.contains("Input MD5 verified") )
          chdManagerMD5Success = true;
        if ( s.contains("Input SHA1 verified") )
          chdManagerSHA1Success = true;
      }
#endif
    }
  }
}

void ROMAlyzer::chdManagerError(QProcess::ProcessError processError)
{
#ifdef QMC2_DEBUG
  QProcess *proc = (QProcess *)sender();
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::chdManagerError(QProcess::ProcessError processError = %1), proc = %2").arg((int)processError).arg((qulonglong)proc));
#endif

  switch ( processError ) {
    case QProcess::FailedToStart:
      log(tr("CHD manager: failed to start"));
      break;

    case QProcess::Crashed:
      log(tr("CHD manager: crashed"));
      break;

    case QProcess::WriteError:
      log(tr("CHD manager: write error"));
      break;

    case QProcess::ReadError:
      log(tr("CHD manager: read error"));
      break;

    default:
      log(tr("CHD manager: unknown error %1").arg(processError));
      break;
  }

  chdManagerRunning = false;
}

void ROMAlyzer::chdManagerStateChanged(QProcess::ProcessState processState)
{
#ifdef QMC2_DEBUG
  QProcess *proc = (QProcess *)sender();
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::chdManagerStateChanged(QProcess::ProcessState processState = %1), proc = %2").arg((int)processState).arg((qulonglong)proc));
#endif

}

void ROMAlyzer::on_lineEditChecksumWizardHash_textEdited(const QString &text)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::on_lineEditChecksumWizardHash_textEdited(const QString &text = %1)").arg(text));
#endif

	switch ( comboBoxChecksumWizardHashType->currentIndex() ) {
		case QMC2_ROMALYZER_CSWIZ_HASHTYPE_CRC:
			currentFilesSHA1Checksum.clear();
			currentFilesCrcChecksum = text;
			break;

		default:
		case QMC2_ROMALYZER_CSWIZ_HASHTYPE_SHA1:
			currentFilesCrcChecksum.clear();
			currentFilesSHA1Checksum = text;
			break;
	}
}

void ROMAlyzer::on_comboBoxChecksumWizardHashType_currentIndexChanged(int index)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::on_comboBoxChecksumWizardHashType_currentIndexChanged(int index)").arg(index));
#endif

	switch ( index ) {
		case QMC2_ROMALYZER_CSWIZ_HASHTYPE_CRC:
			if ( !currentFilesCrcChecksum.isEmpty() && !lineEditChecksumWizardHash->text().isEmpty() )
				lineEditChecksumWizardHash->setText(currentFilesCrcChecksum);
			else
				lineEditChecksumWizardHash->clear();
			break;

		default:
		case QMC2_ROMALYZER_CSWIZ_HASHTYPE_SHA1:
			if ( !currentFilesSHA1Checksum.isEmpty() && !lineEditChecksumWizardHash->text().isEmpty() )
				lineEditChecksumWizardHash->setText(currentFilesSHA1Checksum);
			else
				lineEditChecksumWizardHash->clear();
			break;
	}
}

void ROMAlyzer::on_pushButtonChecksumWizardSearch_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_pushButtonChecksumWizardSearch_clicked()");
#endif

	qmc2StopParser = false;

	treeWidgetChecksumWizardSearchResult->clear();
	QString searchedChecksum = lineEditChecksumWizardHash->text().toLower();
	if ( searchedChecksum.isEmpty() ) return;

	pushButtonChecksumWizardSearch->setEnabled(false);
	lineEditChecksumWizardHash->setEnabled(false);

	int numXmlLines = qmc2Gamelist->xmlLines.count();

	progressBar->setRange(0, numXmlLines);
	labelStatus->setText(tr("Checksum search"));

	QString hashStartString;
	int hashStartOffset;
	switch ( comboBoxChecksumWizardHashType->currentIndex() ) {
		case QMC2_ROMALYZER_CSWIZ_HASHTYPE_CRC:
			hashStartString = "crc=\"";
			hashStartOffset = 5;
			break;

		default:
		case QMC2_ROMALYZER_CSWIZ_HASHTYPE_SHA1:
			hashStartString = "sha1=\"";
			hashStartOffset = 6;
			break;
	}

	for (int i = 0; i < numXmlLines; i++) {
		progressBar->setValue(i);
		qApp->processEvents();
		QString xmlLine = qmc2Gamelist->xmlLines[i];
#if defined(QMC2_EMUTYPE_MAME)
		int gameNamePos = xmlLine.indexOf("<game name=\"");
		if ( gameNamePos == 0 ) gameNamePos = 12;
#elif defined(QMC2_EMUTYPE_MESS)
		int gameNamePos = xmlLine.indexOf("<machine name=\"");
		if ( gameNamePos == 0 ) gameNamePos = 15;
#else
		int gameNamePos = -1;
#endif
		if ( gameNamePos >= 0 ) {
			QString currentGame = xmlLine.mid(gameNamePos, xmlLine.indexOf("\"", gameNamePos) - gameNamePos);
			bool gameEnd = false;
			int j;
			for (j = i + 1; j < numXmlLines && !gameEnd; j++) {
				xmlLine = qmc2Gamelist->xmlLines[j];
#if defined(QMC2_EMUTYPE_MAME)
				if ( xmlLine.startsWith("</game>") ) {
					gameEnd = true;
					continue;
				}
#elif defined(QMC2_EMUTYPE_MESS)
				if ( xmlLine.startsWith("</machine>") ) {
					gameEnd = true;
					continue;
				}
#else
				continue;
#endif
				int hashPos = xmlLine.indexOf(hashStartString) + hashStartOffset;
				if ( hashPos >= 0 ) {
					QString currentChecksum = xmlLine.mid(hashPos, xmlLine.indexOf("\"", hashPos) - hashPos).toLower();
					if ( currentChecksum == searchedChecksum ) {
						int fileNamePos;
						QString fileType;
						if ( xmlLine.startsWith("<disk name=\"") ) {
							fileType = tr("CHD");
							fileNamePos = xmlLine.indexOf("<disk name=\"") + 12;
						} else {
							fileType = tr("ROM");
							fileNamePos = xmlLine.indexOf("<rom name=\"") + 11;
						}
						QString fileName = xmlLine.mid(fileNamePos, xmlLine.indexOf("\"", fileNamePos) - fileNamePos);
						QTreeWidgetItem *item = new QTreeWidgetItem(treeWidgetChecksumWizardSearchResult);
						item->setText(QMC2_ROMALYZER_CSWIZ_COLUMN_ID, currentGame);
						item->setText(QMC2_ROMALYZER_CSWIZ_COLUMN_FILENAME, fileName.replace("&amp;", "&").replace("&lt;", "<").replace("&gt;", ">"));
						item->setText(QMC2_ROMALYZER_CSWIZ_COLUMN_TYPE, fileType);
						item->setText(QMC2_ROMALYZER_CSWIZ_COLUMN_STATUS, tr("unknown"));
						if ( wizardAutomationLevel >= QMC2_ROMALYZER_CSWIZ_AMLVL_SELECT ) item->setSelected(true);
					}
				}
			}
			i = j - 1;
		}
	}
	progressBar->reset();
	labelStatus->setText(tr("Idle"));
	pushButtonChecksumWizardSearch->setEnabled(true);
	lineEditChecksumWizardHash->setEnabled(true);
	qApp->processEvents();

	if ( wizardAutomationLevel >= QMC2_ROMALYZER_CSWIZ_AMLVL_ANALYZE && !qmc2StopParser ) {
		if ( pushButtonChecksumWizardAnalyzeSelectedSets->isEnabled() )
			on_pushButtonChecksumWizardAnalyzeSelectedSets_clicked();
	}
}

void ROMAlyzer::runChecksumWizard()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::runChecksumWizard()");
#endif

	if ( !currentFilesSHA1Checksum.isEmpty() ) {
		comboBoxChecksumWizardHashType->setCurrentIndex(QMC2_ROMALYZER_CSWIZ_HASHTYPE_SHA1);
		lineEditChecksumWizardHash->setText(currentFilesSHA1Checksum);
		tabWidgetAnalysis->setCurrentWidget(tabChecksumWizard);
		pushButtonChecksumWizardSearch->animateClick();
	} else if ( !currentFilesCrcChecksum.isEmpty() ) {
		comboBoxChecksumWizardHashType->setCurrentIndex(QMC2_ROMALYZER_CSWIZ_HASHTYPE_CRC);
		lineEditChecksumWizardHash->setText(currentFilesCrcChecksum);
		tabWidgetAnalysis->setCurrentWidget(tabChecksumWizard);
		pushButtonChecksumWizardSearch->animateClick();
	}
}

void ROMAlyzer::runSetRewriter()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::runSetRewriter()");
#endif

	// when called through the context menu and multiple sets are in the report, rerun the analyzer on the respective set (to make sure that pre-check data is current)
	if ( setRewriterItem == NULL ) {
		QList<QTreeWidgetItem *> il = treeWidgetChecksums->selectedItems();
		if ( il.count() > 0 ) {
			QTreeWidgetItem *item = il[0];
			while ( item->parent() != NULL ) item = item->parent();
			if ( item != NULL ) {
				if ( treeWidgetChecksums->topLevelItemCount() > 1 || qmc2StopParser ) {
					groupBoxSetRewriter->setEnabled(false);
					bool savedSRWA = checkBoxSetRewriterWhileAnalyzing->isChecked();
					checkBoxSetRewriterWhileAnalyzing->setChecked(false);
					lineEditGames->setText(item->text(QMC2_ROMALYZER_COLUMN_GAME).split(" ")[0]);
					qmc2StopParser = false;
					analyze();
					checkBoxSetRewriterWhileAnalyzing->setChecked(savedSRWA);
					groupBoxSetRewriter->setEnabled(true);
					setRewriterItem = treeWidgetChecksums->topLevelItem(0);
					if ( setRewriterItem == NULL ) return;
				} else
					setRewriterItem = item;
			} else
				return;
		} else
			return;
	}

	// check output path and write permission
	QString outPath = lineEditSetRewriterOutputPath->text();
	if ( !outPath.isEmpty() ) {
		QDir dir(outPath);
		if ( dir.exists() ) {
			QFileInfo dirInfo(outPath);
			if ( !dirInfo.isDir() ) {
				log(tr("set rewriter: WARNING: can't rewrite set '%1', output path is not a directory").arg(setRewriterSetName));
				return;
			}
			if ( !dirInfo.isWritable() ) {
				log(tr("set rewriter: WARNING: can't rewrite set '%1', output path is not writable").arg(setRewriterSetName));
				return;
			}

		} else {
			log(tr("set rewriter: WARNING: can't rewrite set '%1', output path does not exist").arg(setRewriterSetName));
			return;
		}
	} else {
		log(tr("set rewriter: WARNING: can't rewrite set '%1', output path is empty").arg(setRewriterSetName));
		return;
	}

	if ( !outPath.endsWith("/") ) outPath += "/";
	outPath += setRewriterSetName + ".zip";

	QLocale locale;

	QString savedStatusText = labelStatus->text();
	labelStatus->setText(tr("Reading '%1' - %2").arg(setRewriterSetName).arg(locale.toString(setRewriterSetCount)));
	progressBar->setRange(0, setRewriterFileMap.count());
	progressBar->reset();
	qApp->processEvents();
	QString modeString = tr("space-efficient");
	if ( checkBoxSetRewriterSelfContainedSets->isChecked() ) modeString = tr("self-contained");

	log(tr("set rewriter: rewriting %1 set '%2' to '%3'").arg(modeString).arg(setRewriterSetName).arg(outPath));

	bool loadOkay = true;
	QMapIterator<QString, QStringList> it(setRewriterFileMap);
	QMap<QString, QByteArray> outputDataMap;
	int count = 0;
	QStringList uniqueCRCs;
	while ( it.hasNext() && loadOkay ) {
		progressBar->setValue(++count);
		it.next();
		QString fileCRC = it.key();
		QString fileName = it.value()[0];
		QString filePath = it.value()[1];
		QString outputFileName = it.value()[2];
		if ( checkBoxSetRewriterUniqueCRCs->isChecked() ) {
			if ( uniqueCRCs.contains(fileCRC) ) {
				log(tr("set rewriter: skipping '%1' with CRC '%2' from '%3' as '%4'").arg(fileName).arg(fileCRC).arg(filePath).arg(outputFileName));
				continue;
			}
		}
		log(tr("set rewriter: loading '%1' with CRC '%2' from '%3' as '%4'").arg(fileName).arg(fileCRC).arg(filePath).arg(outputFileName));
		QByteArray fileData;
		if ( readZipFileData(filePath, fileCRC, &fileData) ) {
			outputDataMap[outputFileName] = fileData;
			uniqueCRCs << fileCRC;
		} else {
			if ( checkBoxSetRewriterGoodSetsOnly->isChecked() ) {
				log(tr("set rewriter: FATAL: can't load '%1' with CRC '%2' from '%3', aborting").arg(fileName).arg(fileCRC).arg(filePath));
				loadOkay = false;
			} else
				log(tr("set rewriter: WARNING: can't load '%1' with CRC '%2' from '%3', ignoring this file").arg(fileName).arg(fileCRC).arg(filePath));
		}
	}
	progressBar->reset();

	if ( loadOkay ) {
		if ( !checkBoxSetRewriterSelfContainedSets->isChecked() ) {
			// remove redundant files (if applicable)
			for (int i = 0; i < setRewriterItem->childCount(); i++) {
				QTreeWidgetItem *childItem = setRewriterItem->child(i);
				if ( !childItem->text(QMC2_ROMALYZER_COLUMN_MERGE).isEmpty() ) {
					if ( !childItem->text(QMC2_ROMALYZER_COLUMN_CRC).isEmpty() ) {
						QList<QStringList> fileEntries = setRewriterFileMap.values(childItem->text(QMC2_ROMALYZER_COLUMN_CRC));
						foreach (QStringList fileEntry, fileEntries) {
							if ( fileEntry.count() == 3 ) { // valid entry?
								QString localName = fileEntry[2];
								if ( outputDataMap.contains(localName) ) {
									log(tr("set rewriter: removing redundant file '%1' with CRC '%2' from output data").arg(localName).arg(childItem->text(QMC2_ROMALYZER_COLUMN_CRC)));
									outputDataMap.remove(localName);
								}
							}
						}
					}
				}
			}
		}
		if ( !outputDataMap.isEmpty() ) {
			log(tr("set rewriter: writing new %1 set '%2' in '%3'").arg(modeString).arg(setRewriterSetName).arg(outPath));
			labelStatus->setText(tr("Writing '%1' - %2").arg(setRewriterSetName).arg(locale.toString(setRewriterSetCount)));
			if ( writeAllZipData(outPath, &outputDataMap, true, progressBar) )
				log(tr("set rewriter: new %1 set '%2' in '%3' successfully created").arg(modeString).arg(setRewriterSetName).arg(outPath));
			else {
				log(tr("set rewriter: FATAL: failed to create new %1 set '%2' in '%3'").arg(modeString).arg(setRewriterSetName).arg(outPath));
				loadOkay = false;
			}
		} else {
			log(tr("set rewriter: INFORMATION: no output data available, thus not rewriting set '%1' to '%2'").arg(setRewriterSetName).arg(outPath));
			loadOkay = false;
		}
	}

	if ( loadOkay ) {
		log(tr("set rewriter: done (rewriting %1 set '%2' to '%3')").arg(modeString).arg(setRewriterSetName).arg(outPath));
		setRewriterItem->setIcon(QMC2_ROMALYZER_COLUMN_GAME, QIcon(QString::fromUtf8(":/data/img/filesaveas.png")));
	}

	labelStatus->setText(savedStatusText);
	progressBar->setRange(0, 100);
	progressBar->setValue(0);
	progressBar->reset();
	qApp->processEvents();
}

void ROMAlyzer::copyToClipboard()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::copyToClipboard()");
#endif

	QList<QTreeWidgetItem *> il = treeWidgetChecksums->selectedItems();
	if ( !il.isEmpty() ) {
		QHeaderView *header = treeWidgetChecksums->header();
		QTreeWidgetItem *headerItem = treeWidgetChecksums->headerItem();
		QTreeWidgetItem *item = il[0];
		QStringList columnTitles;
		QList<int> columnWidths;
		QList<QStringList> rows;
		QStringList firstRow;
		for (int i = 0; i < treeWidgetChecksums->columnCount(); i++)
			if ( !treeWidgetChecksums->isColumnHidden(header->logicalIndex(i)) ) {
				QString h = headerItem->text(header->logicalIndex(i));
				columnTitles << h;
				QString t = item->text(header->logicalIndex(i));
				firstRow << t;
				columnWidths.append(MAX(t.length(), h.length()));
			}
		rows.append(firstRow);
		for (int i = 0; i < item->childCount(); i++) {
			QTreeWidgetItem *childItem = item->child(i);
			QStringList row;
			for (int j = 0; j < treeWidgetChecksums->columnCount(); j++) {
				if ( !treeWidgetChecksums->isColumnHidden(header->logicalIndex(j)) ) {
					QString t = childItem->text(header->logicalIndex(j));
					if ( j == 0 ) t.prepend("\\ ");
					row << t;
					if ( columnWidths[j] < t.length() ) columnWidths[j] = t.length();
				}
			}
			rows.append(row);
		}

		QString cbText;
		for (int i = 0; i < columnTitles.count(); i++)
			cbText += columnTitles[i].leftJustified(columnWidths[i] + 2, ' ');
		cbText += "\n";
		for (int i = 0; i < columnTitles.count(); i++)
			cbText += QString().fill('-', columnWidths[i]) + "  ";
		cbText += "\n";
		foreach (QStringList row, rows) {
			for (int i = 0; i < row.count(); i++)
				cbText += row[i].leftJustified(columnWidths[i] + 2, ' ');
			cbText += "\n";
		}

		qApp->clipboard()->setText(cbText);
	}
}

void ROMAlyzer::on_treeWidgetChecksumWizardSearchResult_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_treeWidgetChecksumWizardSearchResult_itemSelectionChanged()");
#endif

	QList<QTreeWidgetItem *> il = treeWidgetChecksumWizardSearchResult->selectedItems();
	pushButtonChecksumWizardAnalyzeSelectedSets->setEnabled(!il.isEmpty());
	wizardSelectedSets.clear();
	int selectedGoodSets = 0;
	int selectedBadSets = 0;
	foreach (QTreeWidgetItem *item, il) {
		wizardSelectedSets << item->text(QMC2_ROMALYZER_CSWIZ_COLUMN_ID);
		if ( item->text(QMC2_ROMALYZER_CSWIZ_COLUMN_STATUS) == tr("good") ) selectedGoodSets++;
		if ( item->text(QMC2_ROMALYZER_CSWIZ_COLUMN_STATUS) == tr("bad") ) selectedBadSets++;
	}
	wizardSelectedSets.removeDuplicates();
	pushButtonChecksumWizardRepairBadSets->setEnabled(selectedBadSets > 0 && selectedGoodSets > 0);
}

void ROMAlyzer::on_pushButtonChecksumWizardAnalyzeSelectedSets_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_pushButtonChecksumWizardAnalyzeSelectedSets_clicked()");
#endif

	lineEditGames->setText(wizardSelectedSets.join(" "));
	wizardSearch = true;
	pushButtonAnalyze->animateClick();
}

// reads all files in the ZIP 'fileName' and maps the data:
// - CRC codes are mapped to their data in 'dataMap'
// - CRC codes are mapped to their file names in 'fileMap'
// - will also read files with incorrect CRCs (compared to their header CRCs)
bool ROMAlyzer::readAllZipData(QString fileName, QMap<QString, QByteArray> *dataMap, QMap<QString, QString> *fileMap)
{
	bool success = true;
	unzFile zipFile = unzOpen((const char *)fileName.toAscii());

	if ( zipFile ) {
  		char ioBuffer[QMC2_ROMALYZER_ZIP_BUFFER_SIZE];
		unz_file_info zipInfo;
		do {
			if ( unzGetCurrentFileInfo(zipFile, &zipInfo, ioBuffer, QMC2_ROMALYZER_ZIP_BUFFER_SIZE, 0, 0, 0, 0) == UNZ_OK ) {
				fileMap->insert(QString::number(zipInfo.crc), QString((const char *)ioBuffer));
				if ( unzOpenCurrentFile(zipFile) == UNZ_OK ) {
					qint64 len;
					QByteArray fileData;
					progressBarFileIO->setRange(0, zipInfo.uncompressed_size);
					progressBarFileIO->reset();
					while ( (len = unzReadCurrentFile(zipFile, ioBuffer, QMC2_ROMALYZER_ZIP_BUFFER_SIZE)) > 0 ) {
						QByteArray readData((const char *)ioBuffer, len);
						fileData += readData;
						progressBarFileIO->setValue(fileData.length());
						progressBarFileIO->update();
						progressBar->update();
						if ( fileData.length() % QMC2_128K == 0 || fileData.length() == zipInfo.uncompressed_size ) qApp->processEvents();
					}
					dataMap->insert(QString::number(zipInfo.crc), fileData);
					unzCloseCurrentFile(zipFile);
				}
			}
		} while ( unzGoToNextFile(zipFile) == UNZ_OK );
		unzClose(zipFile);
	} else
		success = false;

	progressBarFileIO->reset();
	return success;
}

// reads the file with the CRC 'crc' in the ZIP 'fileName' and returns its data in 'data'
bool ROMAlyzer::readZipFileData(QString fileName, QString crc, QByteArray *data)
{
	bool success = true;
	unzFile zipFile = unzOpen((const char *)fileName.toAscii());

	if ( zipFile ) {
  		char ioBuffer[QMC2_ROMALYZER_ZIP_BUFFER_SIZE];

		// identify file by CRC
		unz_file_info zipInfo;
		QMap<uLong, QString> crcIdentMap;
		uLong ulCRC = crc.toULong(0, 16);
		do {
			if ( unzGetCurrentFileInfo(zipFile, &zipInfo, ioBuffer, QMC2_ROMALYZER_ZIP_BUFFER_SIZE, 0, 0, 0, 0) == UNZ_OK )
				crcIdentMap[zipInfo.crc] = QString((const char *)ioBuffer);
		} while ( unzGoToNextFile(zipFile) == UNZ_OK && !crcIdentMap.contains(ulCRC) );
		unzGoToFirstFile(zipFile);
		if ( crcIdentMap.contains(ulCRC) ) {
			QString fn = crcIdentMap[ulCRC];
			if ( unzLocateFile(zipFile, (const char *)fn.toAscii(), 2) == UNZ_OK ) { // NOT case-sensitive filename compare!
				if ( unzOpenCurrentFile(zipFile) == UNZ_OK ) {
					qint64 len;
					progressBarFileIO->setRange(0, zipInfo.uncompressed_size);
					progressBarFileIO->reset();
					while ( (len = unzReadCurrentFile(zipFile, ioBuffer, QMC2_ROMALYZER_ZIP_BUFFER_SIZE)) > 0 ) {
						QByteArray readData((const char *)ioBuffer, len);
						*data += readData;
						progressBarFileIO->setValue(data->length());
						progressBarFileIO->update();
						progressBar->update();
						if ( data->length() % QMC2_128K == 0 || data->length() == zipInfo.uncompressed_size ) qApp->processEvents();
					}
					unzCloseCurrentFile(zipFile);
				} else
					success = false;
			} else
				success = false;
		} else
			success = false;

		unzClose(zipFile);
	} else
		success = false;

	progressBarFileIO->reset();
	return success;
}

// creates the new ZIP 'fileName' (overwrites an existing file w/o creating a backup!)
// and stores the data found in 'fileDataMap' into the ZIP:
// - 'fileDataMap' maps file names to their data
bool ROMAlyzer::writeAllZipData(QString fileName, QMap<QString, QByteArray> *fileDataMap, bool writeLog, QProgressBar *pBar)
{
	bool success = true;

	QFile f(fileName);
	if (  f.exists() )
		success = f.remove();

	zipFile zip = NULL;
	if ( success )
		zip = zipOpen((const char *)fileName.toAscii(), APPEND_STATUS_CREATE);

	if ( zip ) {
		zip_fileinfo zipInfo;
		QDateTime cDT = QDateTime::currentDateTime();
		zipInfo.tmz_date.tm_sec = cDT.time().second();
		zipInfo.tmz_date.tm_min = cDT.time().minute();
		zipInfo.tmz_date.tm_hour = cDT.time().hour();
		zipInfo.tmz_date.tm_mday = cDT.date().day();
		zipInfo.tmz_date.tm_mon = cDT.date().month() - 1;
		zipInfo.tmz_date.tm_year = cDT.date().year();
		zipInfo.dosDate = zipInfo.internal_fa = zipInfo.external_fa = 0;
		if ( pBar ) {
			pBar->setRange(0, fileDataMap->count());
			pBar->reset();
		}
		QMapIterator<QString, QByteArray> it(*fileDataMap);
		int count = 0;
		while ( it.hasNext() && success ) {
			if ( pBar ) pBar->setValue(++count);
			it.next();
			QString file = it.key();
			QByteArray data = it.value();
			if ( writeLog )
				log(tr("set rewriter: deflating '%1' (uncompressed size: %2)").arg(file).arg(humanReadable(data.length())));
			if ( zipOpenNewFileInZip(zip, (const char *)file.toAscii(), &zipInfo, (const void *)file.toAscii(), file.length(), 0, 0, 0, Z_DEFLATED, spinBoxSetRewriterZipLevel->value()) == ZIP_OK ) {
				quint64 bytesWritten = 0;
				progressBarFileIO->setInvertedAppearance(true);
				progressBarFileIO->setRange(0, data.length());
				progressBarFileIO->reset();
				qApp->processEvents();
				while ( bytesWritten < data.length() && success ) {
					quint64 bufferLength = QMC2_ZIP_BUFFER_SIZE;
					if ( bytesWritten + bufferLength > data.length() )
						bufferLength = data.length() - bytesWritten;
					QByteArray writeBuffer = data.mid(bytesWritten, bufferLength);
					success = (zipWriteInFileInZip(zip, (const void *)writeBuffer.data(), bufferLength) == ZIP_OK);
					if ( success ) {
						bytesWritten += bufferLength;
						progressBarFileIO->setValue(bytesWritten);
						progressBarFileIO->update();
						progressBar->update();
						if ( bytesWritten % QMC2_128K == 0 || bytesWritten == data.length() ) qApp->processEvents();
					} else if ( writeLog )
						log(tr("set rewriter: WARNING: failed to deflate '%1'").arg(file));
				}
				zipCloseFileInZip(zip);
			} else
				success = false;
		}
		zipClose(zip, (const char *)tr("Created by QMC2 v%1 (%2)").arg(XSTR(QMC2_VERSION)).arg(cDT.toString(Qt::SystemLocaleShortDate)).toAscii());
	} else
		success = false;

	if ( pBar ) pBar->reset();
	progressBarFileIO->reset();
	progressBarFileIO->setInvertedAppearance(false);
	return success;
}

void ROMAlyzer::on_pushButtonChecksumWizardRepairBadSets_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_pushButtonChecksumWizardRepairBadSets_clicked()");
#endif

	QList<QTreeWidgetItem *> badList;
	QTreeWidgetItem *goodItem = NULL;
	foreach (QTreeWidgetItem *item, treeWidgetChecksumWizardSearchResult->selectedItems()) {
		if ( item->text(QMC2_ROMALYZER_CSWIZ_COLUMN_STATUS) == tr("bad") )
			badList << item;
		else if ( goodItem == NULL && item->text(QMC2_ROMALYZER_CSWIZ_COLUMN_STATUS) == tr("good") )
			goodItem = item;
	}
	int numBadSets = badList.count();

	// this shouldn't happen, but you never know :)
	if ( numBadSets <= 0  || goodItem == NULL) return;

	// only one repair at a time
	if ( !pushButtonChecksumWizardRepairBadSets->isEnabled() ) return;
	pushButtonChecksumWizardRepairBadSets->setEnabled(false);

	log(tr("checksum wizard: repairing %n bad set(s)", "", numBadSets));
	if ( goodItem != NULL ) {
		QString sourceType = goodItem->text(QMC2_ROMALYZER_CSWIZ_COLUMN_TYPE);
		QString sourceFile = goodItem->text(QMC2_ROMALYZER_CSWIZ_COLUMN_FILENAME);
		QString sourceCRC  = goodItem->whatsThis(QMC2_ROMALYZER_CSWIZ_COLUMN_FILENAME); // we need the CRC for file identification in ZIPs
		QString sourcePath = goodItem->text(QMC2_ROMALYZER_CSWIZ_COLUMN_PATH);
		log(tr("checksum wizard: using %1 file '%2' from '%3' as repro template").arg(sourceType).arg(sourceFile).arg(sourcePath));

		bool loadOkay = true;
		QByteArray templateData;
		QString fn;
		if ( sourceType == tr("ROM") ) {
  			char ioBuffer[MAX(QMC2_ROMALYZER_ZIP_BUFFER_SIZE, QMC2_ROMALYZER_FILE_BUFFER_SIZE)];
			// load ROM image
			if ( sourcePath.indexOf(QRegExp("^.*\\.[zZ][iI][pP]$")) == 0 ) {
				// file from a ZIP archive
				unzFile zipFile = unzOpen((const char *)sourcePath.toAscii());
				if ( zipFile ) {
					// identify file by CRC
					unz_file_info zipInfo;
					QMap<uLong, QString> crcIdentMap;
					uLong ulCRC = sourceCRC.toULong(0, 16);
					do {
						if ( unzGetCurrentFileInfo(zipFile, &zipInfo, ioBuffer, QMC2_ROMALYZER_ZIP_BUFFER_SIZE, 0, 0, 0, 0) == UNZ_OK )
							crcIdentMap[zipInfo.crc] = QString((const char *)ioBuffer);
					} while ( unzGoToNextFile(zipFile) == UNZ_OK && !crcIdentMap.contains(ulCRC) );
					unzGoToFirstFile(zipFile);
					if ( crcIdentMap.contains(ulCRC) ) {
						fn = crcIdentMap[ulCRC];
						log(tr("checksum wizard: successfully identified '%1' from '%2' by CRC, filename in ZIP archive is '%3'").arg(sourceFile).arg(sourcePath).arg(fn));
						if ( unzLocateFile(zipFile, (const char *)fn.toAscii(), 2) == UNZ_OK ) { // NOT case-sensitive filename compare!
							if ( unzOpenCurrentFile(zipFile) == UNZ_OK ) {
								qint64 len;
								while ( (len = unzReadCurrentFile(zipFile, ioBuffer, QMC2_ROMALYZER_ZIP_BUFFER_SIZE)) > 0 ) {
									QByteArray readData((const char *)ioBuffer, len);
									templateData += readData;
								}
								unzCloseCurrentFile(zipFile);
								log(tr("checksum wizard: template data loaded, uncompressed size = %1").arg(humanReadable(templateData.length())));
							}
						}
					} else {
						log(tr("checksum wizard: FATAL: unable to identify '%1' from '%2' by CRC").arg(sourceFile).arg(sourcePath));
						loadOkay = false;
					}
					unzClose(zipFile);
				} else {
					log(tr("checksum wizard: FATAL: can't open ZIP archive '%1' for reading").arg(sourcePath));
					loadOkay = false;
				}
			} else {
				// FIXME: no support for regular files yet
				log(tr("checksum wizard: sorry, no support for regular files yet"));
				loadOkay = false;
			}
		} else {
			// FIXME: no support for CHDs yet (probably not necessary)
			log(tr("checksum wizard: sorry, no support for CHD files yet"));
			loadOkay = false;
		}

		if ( loadOkay ) {
			progressBar->setRange(0, badList.count());
			progressBar->reset();
			quint32 counter = 0;
			foreach (QTreeWidgetItem *badItem, badList) {
				bool saveOkay = true;
				QString targetType = badItem->text(QMC2_ROMALYZER_CSWIZ_COLUMN_TYPE);
				QString targetFile = badItem->text(QMC2_ROMALYZER_CSWIZ_COLUMN_FILENAME);
				QString targetPath = badItem->text(QMC2_ROMALYZER_CSWIZ_COLUMN_PATH);
				labelStatus->setText(tr("Repairing set '%1' - %2").arg(badItem->text(QMC2_ROMALYZER_CSWIZ_COLUMN_ID)).arg(badList.count() - counter));
				log(tr("checksum wizard: repairing %1 file '%2' in '%3' from repro template").arg(targetType).arg(targetFile).arg(targetPath));
				qApp->processEvents();

				if ( targetType == tr("ROM") ) {
					// save ROM image
					if ( targetPath.indexOf(QRegExp("^.*\\.[zZ][iI][pP]$")) == 0 ) {
						bool copyTargetData = false;
						QMap <QString, QByteArray> targetDataMap;
						QMap <QString, QString> targetFileMap;
						QFile f(targetPath);
						int appendType = APPEND_STATUS_ADDINZIP;
						if ( f.exists() ) {
							log(tr("checksum wizard: target ZIP exists, loading complete data and structure"));
							if ( readAllZipData(targetPath, &targetDataMap, &targetFileMap) ) {
								log(tr("checksum wizard: target ZIP successfully loaded"));
								if ( targetDataMap.contains(sourceCRC) ) {
									log(tr("checksum wizard: an entry with the CRC '%1' already exists, recreating the ZIP from scratch to replace the bad file").arg(sourceCRC));
									QString newName = targetPath + QString(".qmc2-backup.%1").arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz"));
									if ( f.rename(newName) ) {
										log(tr("checksum wizard: backup file '%1' successfully created").arg(newName));
										copyTargetData = true;
										appendType = APPEND_STATUS_CREATE;
									} else {
										log(tr("checksum wizard: FATAL: failed to create backup file '%1', aborting"));
										saveOkay = false;
									}
								} else {
									log(tr("checksum wizard: no entry with the CRC '%1' was found, adding the missing file to the existing ZIP").arg(sourceCRC));
								}
							} else {
								log(tr("checksum wizard: FATAL: failed to load target ZIP, aborting"));
								saveOkay = false;
							}
						} else {
							appendType = APPEND_STATUS_CREATE;
							log(tr("checksum wizard: the target ZIP does not exist, creating a new ZIP with just the missing file"));
						}

						zipFile zip = NULL;
					        if ( saveOkay )
							zip = zipOpen((const char *)targetPath.toAscii(), appendType);

						if ( zip ) {
							if ( !copyTargetData ) {
								targetDataMap.clear();
								targetFileMap.clear();
							}

							targetDataMap[sourceCRC] = templateData;
							targetFileMap[sourceCRC] = targetFile;

							zip_fileinfo zipInfo;
							QDateTime cDT = QDateTime::currentDateTime();
							zipInfo.tmz_date.tm_sec = cDT.time().second();
							zipInfo.tmz_date.tm_min = cDT.time().minute();
							zipInfo.tmz_date.tm_hour = cDT.time().hour();
							zipInfo.tmz_date.tm_mday = cDT.date().day();
							zipInfo.tmz_date.tm_mon = cDT.date().month() - 1;
							zipInfo.tmz_date.tm_year = cDT.date().year();
							zipInfo.dosDate = zipInfo.internal_fa = zipInfo.external_fa = 0;

							QMapIterator<QString, QByteArray> it(targetDataMap);
							while ( it.hasNext() ) {
								it.next();
								QString tFile = targetFileMap[it.key()];
								QByteArray tData = it.value();
								if ( zipOpenNewFileInZip(zip, (const char *)tFile.toAscii(), &zipInfo, (const void *)tFile.toAscii(), tFile.length(), 0, 0, 0, Z_DEFLATED, Z_DEFAULT_COMPRESSION) == ZIP_OK ) {
									quint64 bytesWritten = 0;
									progressBarFileIO->setRange(0, tData.length());
									progressBarFileIO->reset();
									while ( bytesWritten < tData.length() && saveOkay ) {
										quint64 bufferLength = QMC2_ZIP_BUFFER_SIZE;
										if ( bytesWritten + bufferLength > tData.length() )
											bufferLength = tData.length() - bytesWritten;
										QByteArray writeBuffer = tData.mid(bytesWritten, bufferLength);
										saveOkay = (zipWriteInFileInZip(zip, (const void *)writeBuffer.data(), bufferLength) == ZIP_OK);
										if ( saveOkay )
											bytesWritten += bufferLength;
										progressBarFileIO->setValue(bytesWritten);
										progressBarFileIO->update();
									}
									progressBarFileIO->reset();
									qApp->processEvents();
									zipCloseFileInZip(zip);
								} else {
									log(tr("checksum wizard: FATAL: can't open file '%1' in ZIP archive '%2' for writing").arg(targetFile).arg(targetPath));
									saveOkay = false;
								}
							}
							if ( saveOkay )
								if ( appendType == APPEND_STATUS_ADDINZIP )
									zipClose(zip, (const char *)tr("Fixed by QMC2 v%1 (%2)").arg(XSTR(QMC2_VERSION)).arg(cDT.toString(Qt::SystemLocaleShortDate)).toAscii());
								else
									zipClose(zip, (const char *)tr("Created by QMC2 v%1 (%2)").arg(XSTR(QMC2_VERSION)).arg(cDT.toString(Qt::SystemLocaleShortDate)).toAscii());
							else
								zipClose(zip, 0);
						} else {
							log(tr("checksum wizard: FATAL: can't open ZIP archive '%1' for writing").arg(targetPath));
							saveOkay = false;
						}
					} else {
						// FIXME: no support for regular files yet
						log(tr("checksum wizard: sorry, no support for regular files yet"));
						saveOkay = false;
					}
				} else {
					// FIXME: no support for CHDs yet (probably not necessary)
					log(tr("checksum wizard: sorry, no support for CHD files yet"));
					saveOkay = false;
				}

				if ( saveOkay ) {
					badItem->setText(QMC2_ROMALYZER_CSWIZ_COLUMN_STATUS, tr("repaired"));
					badItem->setForeground(QMC2_ROMALYZER_CSWIZ_COLUMN_STATUS, QBrush(QColor(0, 255, 0))); // green
					log(tr("checksum wizard: successfully repaired %1 file '%2' in '%3' from repro template").arg(targetType).arg(targetFile).arg(targetPath));
				} else {
					badItem->setText(QMC2_ROMALYZER_CSWIZ_COLUMN_STATUS, tr("repair failed"));
					badItem->setForeground(QMC2_ROMALYZER_CSWIZ_COLUMN_STATUS, QBrush(QColor(255, 0, 0))); // red
					log(tr("checksum wizard: FATAL: failed to repair %1 file '%2' in '%3' from repro template").arg(targetType).arg(targetFile).arg(targetPath));
				}

				progressBar->setValue(++counter);
				qApp->processEvents();
			}
			labelStatus->setText(tr("Idle"));
			progressBar->reset();
		}
	} else
		log(tr("checksum wizard: FATAL: can't find any good set"));

	log(tr("checksum wizard: done (repairing %n bad set(s))", "", numBadSets));
	on_treeWidgetChecksumWizardSearchResult_itemSelectionChanged();
}

void ROMAlyzer::on_treeWidgetChecksums_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_treeWidgetChecksums_customContextMenuRequested(const QPoint &p = ...)");
#endif

	if ( qmc2ROMAlyzerActive )
		return;

	QTreeWidgetItem *item = treeWidgetChecksums->itemAt(p);
	if ( item )
		if ( item->parent() != NULL ) {
			currentFilesSHA1Checksum = item->text(QMC2_ROMALYZER_COLUMN_SHA1);
			currentFilesCrcChecksum = item->text(QMC2_ROMALYZER_COLUMN_CRC);
			if ( !currentFilesSHA1Checksum.isEmpty() || !currentFilesCrcChecksum.isEmpty() ) {
				treeWidgetChecksums->setItemSelected(item, true);
				romFileContextMenu->move(qmc2MainWindow->adjustedWidgetPosition(treeWidgetChecksums->viewport()->mapToGlobal(p), romFileContextMenu));
				romFileContextMenu->show();
			}
		} else {
			actionRewriteSet->setVisible(groupBoxSetRewriter->isChecked());
			actionRewriteSet->setEnabled(groupBoxSetRewriter->isChecked());
			treeWidgetChecksums->setItemSelected(item, true);
			setRewriterItem = NULL;
			romSetContextMenu->move(qmc2MainWindow->adjustedWidgetPosition(treeWidgetChecksums->viewport()->mapToGlobal(p), romSetContextMenu));
			romSetContextMenu->show();
		}
}

#if defined(QMC2_DATABASE_ENABLED)
void ROMAlyzer::on_pushButtonDatabaseCheckConnection_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_pushButtonDatabaseCheckConnection_clicked()");
#endif

	if ( !dbManager || connectionCheckRunning )
		return;

	if ( dbManager->isConnected() )
		return;

	connectionCheckRunning = true;
	savedCheckButtonPalette = pushButtonDatabaseCheckConnection->palette();
	bool hadFocus = pushButtonDatabaseCheckConnection->hasFocus();
	pushButtonDatabaseCheckConnection->setEnabled(false);

	if ( dbManager->checkConnection(comboBoxDatabaseDriver->currentIndex(),
				lineEditDatabaseUser->text(),
				lineEditDatabasePassword->text(),
				lineEditDatabaseName->text(),
				lineEditDatabaseServer->text(),
				spinBoxDatabasePort->value()) ) {
		if ( qApp->styleSheet().isEmpty() ) {
			pushButtonDatabaseCheckConnection->setStyleSheet("");
			QPalette pal = pushButtonDatabaseCheckConnection->palette();
			pal.setColor(QPalette::Button, QColor(0, 255, 0));
			pushButtonDatabaseCheckConnection->setPalette(pal);
		} else
			pushButtonDatabaseCheckConnection->setStyleSheet("background: #00ff00; color: black");
		pushButtonDatabaseCheckConnection->setText(tr("Connection check -- succeeded!"));
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("database connection check successful"));
	} else {
		if ( qApp->styleSheet().isEmpty() ) {
			pushButtonDatabaseCheckConnection->setStyleSheet("");
			QPalette pal = pushButtonDatabaseCheckConnection->palette();
			pal.setColor(QPalette::Button, QColor(255, 0, 0));
			pushButtonDatabaseCheckConnection->setPalette(pal);
		} else
			pushButtonDatabaseCheckConnection->setStyleSheet("background: #ff0000; color: black");
		pushButtonDatabaseCheckConnection->setText(tr("Connection check -- failed!"));
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("database connection check failed -- errorNumber = %1, errorText = '%2'").arg(dbManager->errorNumber()).arg(dbManager->errorText()));
	}

	pushButtonDatabaseCheckConnection->setEnabled(true);
	if ( hadFocus ) pushButtonDatabaseCheckConnection->setFocus(Qt::OtherFocusReason);
	QTimer::singleShot(QMC2_DB_RESET_CCB_DELAY, this, SLOT(resetDatabaseConnectionCheckButton()));
}

void ROMAlyzer::resetDatabaseConnectionCheckButton()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::resetDatabaseConnectionCheckButton()");
#endif

	pushButtonDatabaseCheckConnection->setStyleSheet("");
	pushButtonDatabaseCheckConnection->setPalette(savedCheckButtonPalette);
	pushButtonDatabaseCheckConnection->setText(tr("Connection check"));

	connectionCheckRunning = false;
}

void ROMAlyzer::on_toolButtonBrowseDatabaseOutputPath_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_toolButtonBrowseDatabaseOutputPath_clicked()");
#endif

	QString s = QFileDialog::getExistingDirectory(this, tr("Choose local DB output path"), lineEditDatabaseOutputPath->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

	if ( !s.isNull() ) {
		if ( !s.endsWith("/") ) s += "/";
		lineEditDatabaseOutputPath->setText(s);
	}

	raise();
}
#endif

ROMAlyzerXmlHandler::ROMAlyzerXmlHandler(QTreeWidgetItem *parent, bool expand, bool scroll)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzerXmlHandler::ROMAlyzerXmlHandler(QTreeWidgetItem *parent = %1, bool expand = %2, bool scroll = %3)").arg((qulonglong)parent).arg(expand).arg(scroll));
#endif

  parentItem = parent;
  autoExpand = expand;
  autoScroll = scroll;

  redBrush = QBrush(QColor(255, 0, 0));
  greenBrush = QBrush(QColor(0, 255, 0));
  blueBrush = QBrush(QColor(0, 0, 255));
  yellowBrush = QBrush(QColor(255, 255, 0));
  brownBrush = QBrush(QColor(128, 128, 0));
  greyBrush = QBrush(QColor(128, 128, 128));
}

ROMAlyzerXmlHandler::~ROMAlyzerXmlHandler()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzerXmlHandler::~ROMAlyzerXmlHandler()");
#endif

}

bool ROMAlyzerXmlHandler::startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &attributes)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzerXmlHandler::startElement(...)");
#endif

  QString s;

#if defined(QMC2_EMUTYPE_MAME)
  if ( qName == "game" ) {
#elif defined(QMC2_EMUTYPE_MESS)
  if ( qName == "machine" ) {
#endif
    parentItem->setText(QMC2_ROMALYZER_COLUMN_MERGE, attributes.value("romof"));
    parentItem->setExpanded(false);
    emuStatus = 0;
    fileCounter = 0;
    currentText.clear();
    childItems.clear();
  } else if ( qName == "rom" || qName == "disk" ) {
    fileCounter++;
    childItem = new QTreeWidgetItem(parentItem);
    childItems << childItem;
    childItem->setText(QMC2_ROMALYZER_COLUMN_GAME, attributes.value("name"));
    childItem->setText(QMC2_ROMALYZER_COLUMN_TYPE, qName == "rom" ? QObject::tr("ROM") : QObject::tr("CHD"));
    childItem->setText(QMC2_ROMALYZER_COLUMN_MERGE, attributes.value("merge"));
    s = attributes.value("status");
    if ( s.isEmpty() || s == "good" ) {
      childItem->setText(QMC2_ROMALYZER_COLUMN_EMUSTATUS, QObject::tr("good"));
      childItem->setForeground(QMC2_ROMALYZER_COLUMN_EMUSTATUS, greenBrush);
      emuStatus |= QMC2_ROMALYZER_EMUSTATUS_GOOD;
    } else if ( s == "nodump" ) {
      childItem->setText(QMC2_ROMALYZER_COLUMN_EMUSTATUS, QObject::tr("no dump"));
      childItem->setForeground(QMC2_ROMALYZER_COLUMN_EMUSTATUS, brownBrush);
      emuStatus |= QMC2_ROMALYZER_EMUSTATUS_NODUMP;
    } else if ( s == "baddump" ) {
      childItem->setText(QMC2_ROMALYZER_COLUMN_EMUSTATUS, QObject::tr("bad dump"));
      childItem->setForeground(QMC2_ROMALYZER_COLUMN_EMUSTATUS, brownBrush);
      emuStatus |= QMC2_ROMALYZER_EMUSTATUS_BADDUMP;
    } else {
      childItem->setText(QMC2_ROMALYZER_COLUMN_EMUSTATUS, QObject::tr("unknown"));
      childItem->setForeground(QMC2_ROMALYZER_COLUMN_EMUSTATUS, blueBrush);
      emuStatus |= QMC2_ROMALYZER_EMUSTATUS_UNKNOWN;
    }
    childItem->setText(QMC2_ROMALYZER_COLUMN_SIZE, attributes.value("size"));
    childItem->setText(QMC2_ROMALYZER_COLUMN_CRC, attributes.value("crc"));
    childItem->setText(QMC2_ROMALYZER_COLUMN_SHA1, attributes.value("sha1"));
    childItem->setText(QMC2_ROMALYZER_COLUMN_MD5, attributes.value("md5"));
  }

  return true;
}

bool ROMAlyzerXmlHandler::endElement(const QString &namespaceURI, const QString &localName, const QString &qName)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzerXmlHandler::endElement(...)");
#endif

#if defined(QMC2_EMUTYPE_MAME)
  if ( qName == "game" ) {
#elif defined(QMC2_EMUTYPE_MESS)
  if ( qName == "machine" ) {
#endif
    QString s(parentItem->text(QMC2_ROMALYZER_COLUMN_GAME));
    s += " [" + QString::number(fileCounter) + "]";
    parentItem->setText(QMC2_ROMALYZER_COLUMN_GAME, s);
    QString emuStatusStr;
    QBrush myBrush;
    if ( emuStatus == QMC2_ROMALYZER_EMUSTATUS_GOOD ) {
      emuStatusStr = QObject::tr("good");
      myBrush = greenBrush;
    } else if ( emuStatus & QMC2_ROMALYZER_EMUSTATUS_UNKNOWN ) {
      emuStatusStr = QObject::tr("unknown");
      myBrush = blueBrush;
    } else if ( emuStatus & QMC2_ROMALYZER_EMUSTATUS_NODUMP && emuStatus & QMC2_ROMALYZER_EMUSTATUS_BADDUMP ) {
      emuStatusStr = QObject::tr("no / bad dump");
      myBrush = brownBrush;
    } else if ( emuStatus & QMC2_ROMALYZER_EMUSTATUS_NODUMP ) {
      emuStatusStr = QObject::tr("no dump");
      myBrush = brownBrush;
    } else if ( emuStatus & QMC2_ROMALYZER_EMUSTATUS_BADDUMP ) {
      emuStatusStr = QObject::tr("bad dump");
      myBrush = brownBrush;
    } else {
      emuStatusStr = QObject::tr("unknown");
      myBrush = blueBrush;
    }
#if defined(QMC2_EMUTYPE_MESS)
    if ( fileCounter == 0 ) {
       emuStatusStr = QObject::tr("good");
      myBrush = greenBrush;
    }
#endif
    parentItem->setText(QMC2_ROMALYZER_COLUMN_EMUSTATUS, emuStatusStr);
    parentItem->setForeground(QMC2_ROMALYZER_COLUMN_EMUSTATUS, myBrush);
    if ( autoExpand )
      parentItem->setExpanded(true);
    if ( autoScroll )
      qmc2ROMAlyzer->treeWidgetChecksums->scrollToItem(parentItem, QAbstractItemView::PositionAtTop);
  }

  return true;
}

bool ROMAlyzerXmlHandler::characters(const QString &str)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzerXmlHandler::characters(...)");
#endif

  currentText += QString::fromUtf8(str.toAscii());
  return true;
}
