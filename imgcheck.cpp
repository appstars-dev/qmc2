#include <QMap>
#include <QTime>
#include <QIcon>
#include <QSettings>
#include <QTreeWidget>
#include <QTimer>
#include <QDir>
#include <QFileInfo>

#include "imgcheck.h"
#include "qmc2main.h"
#include "flyer.h"
#include "preview.h"
#include "gamelist.h"
#include "options.h"
#include "toolexec.h"
#include "unzip.h"
#include "macros.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern Preview *qmc2Preview;
extern Flyer *qmc2Flyer;
extern bool qmc2ReloadActive;
extern bool qmc2ROMAlyzerActive;
extern bool qmc2ImageCheckActive;
extern bool qmc2SampleCheckActive;
extern bool qmc2EarlyReloadActive;
extern bool qmc2VerifyActive;
extern bool qmc2FilterActive;
extern bool qmc2StopParser;
extern bool qmc2UsePreviewFile;
extern bool qmc2UseFlyerFile;
extern bool qmc2UseIconFile;
extern bool qmc2IconsPreloaded;
extern bool qmc2CleaningUp;
extern bool qmc2EarlyStartup;
extern QMap<QString, QTreeWidgetItem *> qmc2GamelistItemMap;
extern QMap<QString, QTreeWidgetItem *> qmc2HierarchyItemMap;
#if defined(QMC2_EMUTYPE_MAME)
extern QMap<QString, QTreeWidgetItem *> qmc2CategoryItemMap;
extern QMap<QString, QTreeWidgetItem *> qmc2VersionItemMap;
#endif
extern QMap<QString, QIcon> qmc2IconMap;
extern Options *qmc2Options;
extern Gamelist *qmc2Gamelist;
extern Preview *qmc2Preview;
extern Flyer *qmc2Flyer;
extern int qmc2GamelistResponsiveness;
extern QSettings *qmc2Config;
extern unzFile qmc2PreviewFile;
extern unzFile qmc2FlyerFile;
extern unzFile qmc2IconFile;
extern QAbstractItemView::ScrollHint qmc2CursorPositioningMode;

ImageChecker::ImageChecker(QWidget *parent)
#if defined(Q_WS_WIN)
  : QDialog(parent, Qt::Dialog)
#else
  : QDialog(parent, Qt::Dialog | Qt::SubWindow)
#endif
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ImageChecker::ImageChecker(QWidget *parent = %1").arg((qulonglong)parent));
#endif

  setupUi(this);

#if defined(QMC2_SDLMESS)
  checkBoxPreviewsSelectGame->setText(tr("Select machine"));
  checkBoxPreviewsSelectGame->setToolTip(tr("Select machine in machine list if selected in check lists?"));
  checkBoxFlyersSelectGame->setText(tr("Select machine"));
  checkBoxFlyersSelectGame->setToolTip(tr("Select machine in machine list if selected in check lists?"));
  tabWidgetImageChecker->removeTab(QMC2_ICON_INDEX);
#endif

  QFont f;
  f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
  QFontMetrics fm(f);
  QSize iconSize(fm.height() - 2, fm.height() - 2);
  QTabBar *tabBar = tabWidgetImageChecker->findChild<QTabBar *>();
  if ( tabBar ) tabBar->setIconSize(iconSize);
}

ImageChecker::~ImageChecker()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageChecker::~ImageChecker()");
#endif

}

void ImageChecker::on_pushButtonPreviewsCheck_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageChecker::on_pushButtonPreviewsCheck_clicked()");
#endif

  if ( lockProcessing() ) {
    // check previews
    QTime checkTimer, elapsedTime;
    pushButtonPreviewsRemoveObsolete->setEnabled(FALSE);
    checkTimer.start();
    if ( qmc2UsePreviewFile )
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("checking previews from ZIP archive"));
    else
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("checking previews from directory"));

    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
      qmc2MainWindow->progressBarGamelist->setFormat(tr("Preview check - %p%"));
    else
      qmc2MainWindow->progressBarGamelist->setFormat("%p%");
    qmc2MainWindow->progressBarGamelist->setRange(0, qmc2Gamelist->numTotalGames + qmc2Gamelist->numDevices);
    qmc2MainWindow->progressBarGamelist->reset();

    if ( qmc2Gamelist->numTotalGames + qmc2Gamelist->numDevices != qmc2Gamelist->numGames )
#if defined(QMC2_EMUTYPE_MAME)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: game list not fully loaded, check results may be misleading"));
#elif defined(QMC2_EMUTYPE_MESS)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: machine list not fully loaded, check results may be misleading"));
#endif

    // change check buttons to stop buttons
    pushButtonPreviewsCheck->setText(tr("&Stop check"));
    pushButtonFlyersCheck->setText(tr("&Stop check"));
    pushButtonIconsCheck->setText(tr("&Stop check"));

    // avoid changes to preview source during check
    qmc2Options->radioButtonPreviewSelect->setEnabled(FALSE);
    qmc2Options->stackedWidgetPreview->setEnabled(FALSE);
    qmc2Options->lineEditPreviewFile->setEnabled(FALSE);
    qmc2Options->lineEditPreviewDirectory->setEnabled(FALSE);
    qmc2Options->toolButtonBrowsePreviewFile->setEnabled(FALSE);
    qmc2Options->toolButtonBrowsePreviewDirectory->setEnabled(FALSE);

    // check pass 1: found and missing previews
    listWidgetPreviewsFound->clear();
    labelPreviewsFound->setText(tr("Found: 0"));
    listWidgetPreviewsMissing->clear();
    labelPreviewsMissing->setText(tr("Missing: 0"));
    listWidgetPreviewsObsolete->clear();
    labelPreviewsObsolete->setText(tr("Obsolete: 0"));
    QMapIterator<QString, QTreeWidgetItem *> it(qmc2GamelistItemMap);
    int i = -1, lastCheckTimer = 0;
    qmc2StopParser = FALSE;
    QStringList foundPreviews, missingPreviews;
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("check pass 1: found and missing previews"));
    QStringList fileNames;
    while ( it.hasNext() && !qmc2StopParser ) {
      i++;
      if ( checkTimer.elapsed() - lastCheckTimer >= QMC2_CHECK_UPDATE ) {
        lastCheckTimer = checkTimer.elapsed();
        qmc2MainWindow->progressBarGamelist->setValue(i);
        listWidgetPreviewsFound->addItems(foundPreviews);
        labelPreviewsFound->setText(tr("Found: %1").arg(listWidgetPreviewsFound->count()));
        listWidgetPreviewsMissing->addItems(missingPreviews);
        labelPreviewsMissing->setText(tr("Missing: %1").arg(listWidgetPreviewsMissing->count()));
        foundPreviews.clear();
        missingPreviews.clear();
      }
      it.next();
      QString gameName = it.key();
      QString fileName;
      if ( qmc2Preview->loadPreview(gameName, gameName, TRUE, &fileName) )
        foundPreviews << gameName;
      else
        missingPreviews << gameName;
      if ( !fileName.isEmpty() )
#if defined(Q_WS_WIN)
        fileNames << fileName.toLower();
#else
        fileNames << fileName;
#endif
      qApp->processEvents();
    }
    listWidgetPreviewsFound->addItems(foundPreviews);
    labelPreviewsFound->setText(tr("Found: %1").arg(listWidgetPreviewsFound->count()));
    listWidgetPreviewsFound->sortItems(Qt::AscendingOrder);
    listWidgetPreviewsMissing->addItems(missingPreviews);
    labelPreviewsMissing->setText(tr("Missing: %1").arg(listWidgetPreviewsMissing->count()));
    listWidgetPreviewsMissing->sortItems(Qt::AscendingOrder);

    // check pass 2: obsolete previews
    qmc2MainWindow->progressBarGamelist->reset();
    QStringList obsoleteFiles;
    if ( qmc2UsePreviewFile ) {
      // check from ZIP archive
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("check pass 2: obsolete files: reading ZIP directory"));
      qApp->processEvents();

      QStringList fileList;
      i = 0;
      unz_global_info unzGlobalInfo;
      if ( unzGetGlobalInfo(qmc2Preview->previewFile, &unzGlobalInfo) == UNZ_OK ) {
        qmc2MainWindow->progressBarGamelist->setRange(0, unzGlobalInfo.number_entry);
        qmc2MainWindow->progressBarGamelist->reset();
        qApp->processEvents();
        if ( unzGoToFirstFile(qmc2Preview->previewFile) == UNZ_OK ) {
          do {
            char unzFileName[QMC2_MAX_PATH_LENGTH];
            i++;
            if ( i % 25 == 0 ) {
              qmc2MainWindow->progressBarGamelist->setValue(i);
              qApp->processEvents();
            }
            if ( unzGetCurrentFileInfo(qmc2Preview->previewFile, NULL, unzFileName, QMC2_MAX_PATH_LENGTH, NULL, 0, NULL, 0) == UNZ_OK ) {
              fileList << unzFileName;
            }
          } while ( unzGoToNextFile(qmc2Preview->previewFile) != UNZ_END_OF_LIST_OF_FILE );
        }
        qmc2MainWindow->progressBarGamelist->setRange(0, fileList.count());
        qmc2MainWindow->progressBarGamelist->reset();
      }

      for (i = 0; i < fileList.count() && !qmc2StopParser; i++) {
        if ( checkTimer.elapsed() - lastCheckTimer >= QMC2_CHECK_UPDATE ) {
          lastCheckTimer = checkTimer.elapsed();
          qmc2MainWindow->progressBarGamelist->setValue(i);
          listWidgetPreviewsObsolete->addItems(obsoleteFiles);
          labelPreviewsObsolete->setText(tr("Obsolete: %1").arg(listWidgetPreviewsObsolete->count()));
          obsoleteFiles.clear();
        }
#if defined(Q_WS_WIN)
        if ( !fileNames.contains(fileList[i], Qt::CaseInsensitive) )
          obsoleteFiles << fileList[i];
#else
        if ( !fileNames.contains(fileList[i]) )
          obsoleteFiles << fileList[i];
#endif
        qApp->processEvents();
      }
      listWidgetPreviewsObsolete->addItems(obsoleteFiles);
      labelPreviewsObsolete->setText(tr("Obsolete: %1").arg(listWidgetPreviewsObsolete->count()));
      listWidgetPreviewsObsolete->sortItems(Qt::AscendingOrder);
    } else {
      // check from directory
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("check pass 2: obsolete files: reading directory structure"));
      qApp->processEvents();
      QStringList fileList;
#if defined(QMC2_EMUTYPE_MAME)
      QString previewDir = qmc2Config->value("MAME/FilesAndDirectories/PreviewDirectory").toString();
#elif defined(QMC2_EMUTYPE_MESS)
      QString previewDir = qmc2Config->value("MESS/FilesAndDirectories/PreviewDirectory").toString();
#endif
      recursiveFileList(previewDir, fileList);
      qmc2MainWindow->progressBarGamelist->setRange(0, fileList.count());
      qmc2MainWindow->progressBarGamelist->reset();
      for (i = 0; i < fileList.count() && !qmc2StopParser; i++) {
        if ( checkTimer.elapsed() - lastCheckTimer >= QMC2_CHECK_UPDATE ) {
          lastCheckTimer = checkTimer.elapsed();
          qmc2MainWindow->progressBarGamelist->setValue(i);
          listWidgetPreviewsObsolete->addItems(obsoleteFiles);
          labelPreviewsObsolete->setText(tr("Obsolete: %1").arg(listWidgetPreviewsObsolete->count()));
          obsoleteFiles.clear();
        }
        QString relativeFilePath = fileList[i].remove(previewDir);
        if ( !fileNames.contains(relativeFilePath) )
          obsoleteFiles << relativeFilePath;
        qApp->processEvents();
      }
      listWidgetPreviewsObsolete->addItems(obsoleteFiles);
      labelPreviewsObsolete->setText(tr("Obsolete: %1").arg(listWidgetPreviewsObsolete->count()));
      listWidgetPreviewsObsolete->sortItems(Qt::AscendingOrder);
    }

    qmc2MainWindow->progressBarGamelist->reset();

    // reallow changes to preview source after check
    qmc2Options->radioButtonPreviewSelect->setEnabled(TRUE);
    qmc2Options->stackedWidgetPreview->setEnabled(TRUE);
    qmc2Options->lineEditPreviewFile->setEnabled(TRUE);
    qmc2Options->lineEditPreviewDirectory->setEnabled(TRUE);
    qmc2Options->toolButtonBrowsePreviewFile->setEnabled(TRUE);
    qmc2Options->toolButtonBrowsePreviewDirectory->setEnabled(TRUE);

    // log check timing
    elapsedTime = elapsedTime.addMSecs(checkTimer.elapsed());
    if ( qmc2UsePreviewFile )
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (checking previews from ZIP archive, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
    else
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (checking previews from directory, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("%1 found, %2 missing, %3 obsolete").arg(listWidgetPreviewsFound->count()).arg(listWidgetPreviewsMissing->count()).arg(listWidgetPreviewsObsolete->count()));

    // enable removal button if obsolete images exist
    pushButtonPreviewsRemoveObsolete->setEnabled(listWidgetPreviewsObsolete->count() > 0);

    // reset check buttons
    pushButtonPreviewsCheck->setText(tr("&Check previews"));
    pushButtonFlyersCheck->setText(tr("&Check flyers"));
    pushButtonIconsCheck->setText(tr("&Check icons"));

    // release lock
    qmc2ImageCheckActive = FALSE;
  }
}

void ImageChecker::on_pushButtonPreviewsRemoveObsolete_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageChecker::on_pushButtonPreviewsRemoveObsolete_clicked()");
#endif

  if ( qmc2UsePreviewFile ) {
#if defined(Q_WS_WIN)
    QString command = "cmd.exe";
    QStringList args;
    args << "/c" << qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/ZipTool").toString().replace('/', '\\')
         << qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/ZipToolRemovalArguments").toString().split(" ");
#else
    QString command = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/ZipTool").toString();
    QStringList args = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/ZipToolRemovalArguments").toString().split(" ");
#endif

    int i, j;
    QStringList addArgs;
    for (i = 0; i < args.count(); i++) {
      if ( args[i] == "$ARCHIVE$" ) {
#if defined(QMC2_EMUTYPE_MAME)
        args[i] = qmc2Config->value("MAME/FilesAndDirectories/PreviewFile").toString();
#elif defined(QMC2_EMUTYPE_MESS)
        args[i] = qmc2Config->value("MESS/FilesAndDirectories/PreviewFile").toString();
#endif
#if defined(Q_WS_WIN)
        args[i] = args[i].replace('/', '\\');
#endif
      } else if ( args[i] == "$FILELIST$" ) {
        QList<QListWidgetItem *> items = listWidgetPreviewsObsolete->findItems("*", Qt::MatchWildcard); 
        for (j = 0; j < items.count(); j++) {
#if defined(Q_WS_WIN)
          addArgs << items[j]->text().replace('/', '\\');
#else
          addArgs << items[j]->text();
#endif
        }
        args.removeAt(i);
        args << addArgs;
      }
    }

    qmc2Preview->setUpdatesEnabled(FALSE);
    unzClose(qmc2Preview->previewFile);
    ToolExecutor zipRemovalTool(this, command, args);
    zipRemovalTool.exec();
#if defined(QMC2_EMUTYPE_MAME)
    qmc2Preview->previewFile = unzOpen((const char *)qmc2Config->value("MAME/FilesAndDirectories/PreviewFile").toString().toAscii());
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2Preview->previewFile = unzOpen((const char *)qmc2Config->value("MESS/FilesAndDirectories/PreviewFile").toString().toAscii());
#endif
    qmc2Preview->setUpdatesEnabled(TRUE);
  } else {
#if defined(Q_WS_WIN)
    QString command = "cmd.exe";
    QStringList args;
    args << "/c" << qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/FileRemovalTool").toString().replace('/', '\\')
         << qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/FileRemovalToolArguments").toString().split(" ");
#else
    QString command = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/FileRemovalTool").toString();
    QStringList args = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/FileRemovalToolArguments").toString().split(" ");
#endif

    int i, j;
    QStringList addArgs;
    for (i = 0; i < args.count(); i++) {
      if ( args[i] == "$FILELIST$" ) {
        QList<QListWidgetItem *> items = listWidgetPreviewsObsolete->findItems("*", Qt::MatchWildcard); 
        for (j = 0; j < items.count(); j++) {
#if defined(Q_WS_WIN)
#if defined(QMC2_EMUTYPE_MAME)
          addArgs << qmc2Config->value("MAME/FilesAndDirectories/PreviewDirectory").toString().replace('/', '\\') + items[j]->text().replace('/', '\\');
#elif defined(QMC2_EMUTYPE_MESS)
          addArgs << qmc2Config->value("MESS/FilesAndDirectories/PreviewDirectory").toString().replace('/', '\\') + items[j]->text().replace('/', '\\');
#endif
#else
#if defined(QMC2_EMUTYPE_MAME)
          addArgs << qmc2Config->value("MAME/FilesAndDirectories/PreviewDirectory").toString() + items[j]->text();
#elif defined(QMC2_EMUTYPE_MESS)
          addArgs << qmc2Config->value("MESS/FilesAndDirectories/PreviewDirectory").toString() + items[j]->text();
#endif
#endif
        }
        args.removeAt(i);
        args << addArgs;
      }
    }

    ToolExecutor fileRemovalTool(this, command, args);
    fileRemovalTool.exec();
  }
}

void ImageChecker::on_listWidgetPreviewsFound_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageChecker::on_listWidgetPreviewsFound_itemSelectionChanged()");
#endif

  if ( checkBoxPreviewsSelectGame->isChecked() ) {
    QList<QListWidgetItem *> items = listWidgetPreviewsFound->selectedItems();
    if ( items.count() > 0 )
      selectItem(items[0]->text());
  }
}

void ImageChecker::on_listWidgetPreviewsMissing_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageChecker::on_listWidgetPreviewsMissing_itemSelectionChanged()");
#endif

  if ( checkBoxPreviewsSelectGame->isChecked() ) {
    QList<QListWidgetItem *> items = listWidgetPreviewsMissing->selectedItems();
    if ( items.count() > 0 )
      selectItem(items[0]->text());
  }
}

void ImageChecker::on_pushButtonFlyersCheck_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageChecker::on_pushButtonFlyersCheck_clicked()");
#endif

  if ( lockProcessing() ) {
    // check flyers
    QTime checkTimer, elapsedTime;
    pushButtonFlyersRemoveObsolete->setEnabled(FALSE);
    checkTimer.start();
    if ( qmc2UseFlyerFile )
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("checking flyers from ZIP archive"));
    else
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("checking flyers from directory"));

    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
      qmc2MainWindow->progressBarGamelist->setFormat(tr("Flyer check - %p%"));
    else
      qmc2MainWindow->progressBarGamelist->setFormat("%p%");
    qmc2MainWindow->progressBarGamelist->setRange(0, qmc2Gamelist->numTotalGames + qmc2Gamelist->numDevices);
    qmc2MainWindow->progressBarGamelist->reset();

    if ( qmc2Gamelist->numTotalGames + qmc2Gamelist->numDevices != qmc2Gamelist->numGames )
#if defined(QMC2_EMUTYPE_MAME)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: game list not fully loaded, check results may be misleading"));
#elif defined(QMC2_EMUTYPE_MESS)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: machine list not fully loaded, check results may be misleading"));
#endif

    // change check buttons to stop buttons
    pushButtonPreviewsCheck->setText(tr("&Stop check"));
    pushButtonFlyersCheck->setText(tr("&Stop check"));
    pushButtonIconsCheck->setText(tr("&Stop check"));

    // avoid changes to flyer source during check
    qmc2Options->radioButtonFlyerSelect->setEnabled(FALSE);
    qmc2Options->stackedWidgetFlyer->setEnabled(FALSE);
    qmc2Options->lineEditFlyerFile->setEnabled(FALSE);
    qmc2Options->lineEditFlyerDirectory->setEnabled(FALSE);
    qmc2Options->toolButtonBrowseFlyerFile->setEnabled(FALSE);
    qmc2Options->toolButtonBrowseFlyerDirectory->setEnabled(FALSE);

    // check pass 1: found and missing flyers
    listWidgetFlyersFound->clear();
    labelFlyersFound->setText(tr("Found: 0"));
    listWidgetFlyersMissing->clear();
    labelFlyersMissing->setText(tr("Missing: 0"));
    listWidgetFlyersObsolete->clear();
    labelFlyersObsolete->setText(tr("Obsolete: 0"));
    QMapIterator<QString, QTreeWidgetItem *> it(qmc2GamelistItemMap);
    int i = -1, lastCheckTimer = 0;
    qmc2StopParser = FALSE;
    QStringList foundFlyers, missingFlyers;
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("check pass 1: found and missing flyers"));
    QStringList fileNames;
    while ( it.hasNext() && !qmc2StopParser ) {
      i++;
      if ( checkTimer.elapsed() - lastCheckTimer >= QMC2_CHECK_UPDATE ) {
        lastCheckTimer = checkTimer.elapsed();
        qmc2MainWindow->progressBarGamelist->setValue(i);
        listWidgetFlyersFound->addItems(foundFlyers);
        labelFlyersFound->setText(tr("Found: %1").arg(listWidgetFlyersFound->count()));
        listWidgetFlyersMissing->addItems(missingFlyers);
        labelFlyersMissing->setText(tr("Missing: %1").arg(listWidgetFlyersMissing->count()));
        foundFlyers.clear();
        missingFlyers.clear();
      }
      it.next();
      QString gameName = it.key();
      QString fileName;
      if ( qmc2Flyer->loadFlyer(gameName, gameName, TRUE, &fileName) )
        foundFlyers << gameName;
      else
        missingFlyers << gameName;
      if ( !fileName.isEmpty() )
#if defined(Q_WS_WIN)
        fileNames << fileName.toLower();
#else
        fileNames << fileName;
#endif
      qApp->processEvents();
    }
    listWidgetFlyersFound->addItems(foundFlyers);
    labelFlyersFound->setText(tr("Found: %1").arg(listWidgetFlyersFound->count()));
    listWidgetFlyersFound->sortItems(Qt::AscendingOrder);
    listWidgetFlyersMissing->addItems(missingFlyers);
    labelFlyersMissing->setText(tr("Missing: %1").arg(listWidgetFlyersMissing->count()));
    listWidgetFlyersMissing->sortItems(Qt::AscendingOrder);

    // check pass 2: obsolete flyers
    qmc2MainWindow->progressBarGamelist->reset();
    QStringList obsoleteFiles;
    if ( qmc2UseFlyerFile ) {
      // check from ZIP archive
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("check pass 2: obsolete files: reading ZIP directory"));
      qApp->processEvents();

      QStringList fileList;
      i = 0;
      unz_global_info unzGlobalInfo;
      if ( unzGetGlobalInfo(qmc2Flyer->flyerFile, &unzGlobalInfo) == UNZ_OK ) {
        qmc2MainWindow->progressBarGamelist->setRange(0, unzGlobalInfo.number_entry);
        qmc2MainWindow->progressBarGamelist->reset();
        qApp->processEvents();
        if ( unzGoToFirstFile(qmc2Flyer->flyerFile) == UNZ_OK ) {
          do {
            char unzFileName[QMC2_MAX_PATH_LENGTH];
            i++;
            if ( i % 25 == 0 ) {
              qmc2MainWindow->progressBarGamelist->setValue(i);
              qApp->processEvents();
            }
            if ( unzGetCurrentFileInfo(qmc2Flyer->flyerFile, NULL, unzFileName, QMC2_MAX_PATH_LENGTH, NULL, 0, NULL, 0) == UNZ_OK ) {
              fileList << unzFileName;
            }
          } while ( unzGoToNextFile(qmc2Flyer->flyerFile) != UNZ_END_OF_LIST_OF_FILE );
        }
        qmc2MainWindow->progressBarGamelist->setRange(0, fileList.count());
        qmc2MainWindow->progressBarGamelist->reset();
      }

      for (i = 0; i < fileList.count() && !qmc2StopParser; i++) {
        if ( checkTimer.elapsed() - lastCheckTimer >= QMC2_CHECK_UPDATE ) {
          lastCheckTimer = checkTimer.elapsed();
          qmc2MainWindow->progressBarGamelist->setValue(i);
          listWidgetFlyersObsolete->addItems(obsoleteFiles);
          labelFlyersObsolete->setText(tr("Obsolete: %1").arg(listWidgetFlyersObsolete->count()));
          obsoleteFiles.clear();
        }
#if defined(Q_WS_WIN)
        if ( !fileNames.contains(fileList[i], Qt::CaseInsensitive) )
          obsoleteFiles << fileList[i];
#else
        if ( !fileNames.contains(fileList[i]) )
          obsoleteFiles << fileList[i];
#endif
        qApp->processEvents();
      }
      listWidgetFlyersObsolete->addItems(obsoleteFiles);
      labelFlyersObsolete->setText(tr("Obsolete: %1").arg(listWidgetFlyersObsolete->count()));
      listWidgetFlyersObsolete->sortItems(Qt::AscendingOrder);
    } else {
      // check from directory
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("check pass 2: obsolete files: reading directory structure"));
      qApp->processEvents();
      QStringList fileList;
#if defined(QMC2_EMUTYPE_MAME)
      QString flyerDir = qmc2Config->value("MAME/FilesAndDirectories/FlyerDirectory").toString();
#elif defined(QMC2_EMUTYPE_MESS)
      QString flyerDir = qmc2Config->value("MESS/FilesAndDirectories/FlyerDirectory").toString();
#endif
      recursiveFileList(flyerDir, fileList);
      qmc2MainWindow->progressBarGamelist->setRange(0, fileList.count());
      qmc2MainWindow->progressBarGamelist->reset();
      for (i = 0; i < fileList.count() && !qmc2StopParser; i++) {
        if ( checkTimer.elapsed() - lastCheckTimer >= QMC2_CHECK_UPDATE ) {
          lastCheckTimer = checkTimer.elapsed();
          qmc2MainWindow->progressBarGamelist->setValue(i);
          listWidgetFlyersObsolete->addItems(obsoleteFiles);
          labelFlyersObsolete->setText(tr("Obsolete: %1").arg(listWidgetFlyersObsolete->count()));
          obsoleteFiles.clear();
        }
        QString relativeFilePath = fileList[i].remove(flyerDir);
        if ( !fileNames.contains(relativeFilePath) )
          obsoleteFiles << relativeFilePath;
        qApp->processEvents();
      }
      listWidgetFlyersObsolete->addItems(obsoleteFiles);
      labelFlyersObsolete->setText(tr("Obsolete: %1").arg(listWidgetFlyersObsolete->count()));
      listWidgetFlyersObsolete->sortItems(Qt::AscendingOrder);
    }

    qmc2MainWindow->progressBarGamelist->reset();

    // reallow changes to flyer source after check
    qmc2Options->radioButtonFlyerSelect->setEnabled(TRUE);
    qmc2Options->stackedWidgetFlyer->setEnabled(TRUE);
    qmc2Options->lineEditFlyerFile->setEnabled(TRUE);
    qmc2Options->lineEditFlyerDirectory->setEnabled(TRUE);
    qmc2Options->toolButtonBrowseFlyerFile->setEnabled(TRUE);
    qmc2Options->toolButtonBrowseFlyerDirectory->setEnabled(TRUE);

    // log check timing
    elapsedTime = elapsedTime.addMSecs(checkTimer.elapsed());
    if ( qmc2UseFlyerFile )
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (checking flyers from ZIP archive, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
    else
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (checking flyers from directory, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("%1 found, %2 missing, %3 obsolete").arg(listWidgetFlyersFound->count()).arg(listWidgetFlyersMissing->count()).arg(listWidgetFlyersObsolete->count()));

    // enable removal button if obsolete images exist
    pushButtonFlyersRemoveObsolete->setEnabled(listWidgetFlyersObsolete->count() > 0);

    // reset check buttons
    pushButtonPreviewsCheck->setText(tr("&Check previews"));
    pushButtonFlyersCheck->setText(tr("&Check flyers"));
    pushButtonIconsCheck->setText(tr("&Check icons"));

    // release lock
    qmc2ImageCheckActive = FALSE;
  }
}

void ImageChecker::on_pushButtonFlyersRemoveObsolete_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageChecker::on_pushButtonFlyersRemoveObsolete_clicked()");
#endif

  if ( qmc2UseFlyerFile ) {
#if defined(Q_WS_WIN)
    QString command = "cmd.exe";
    QStringList args;
    args << "/c" << qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/ZipTool").toString().replace('/', '\\')
         << qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/ZipToolRemovalArguments").toString().split(" ");
#else
    QString command = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/ZipTool").toString();
    QStringList args = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/ZipToolRemovalArguments").toString().split(" ");
#endif

    int i, j;
    QStringList addArgs;
    for (i = 0; i < args.count(); i++) {
      if ( args[i] == "$ARCHIVE$" ) {
#if defined(QMC2_EMUTYPE_MAME)
        args[i] = qmc2Config->value("MAME/FilesAndDirectories/FlyerFile").toString();
#elif defined(QMC2_EMUTYPE_MESS)
        args[i] = qmc2Config->value("MESS/FilesAndDirectories/FlyerFile").toString();
#endif
#if defined(Q_WS_WIN)
        args[i] = args[i].replace('/', '\\');
#endif
      } else if ( args[i] == "$FILELIST$" ) {
        QList<QListWidgetItem *> items = listWidgetFlyersObsolete->findItems("*", Qt::MatchWildcard); 
        for (j = 0; j < items.count(); j++) {
#if defined(Q_WS_WIN)
          addArgs << items[j]->text().replace('/', '\\');
#else
          addArgs << items[j]->text();
#endif
        }
        args.removeAt(i);
        args << addArgs;
      }
    }

    qmc2Flyer->setUpdatesEnabled(FALSE);
    unzClose(qmc2Flyer->flyerFile);
    ToolExecutor zipRemovalTool(this, command, args);
    zipRemovalTool.exec();
#if defined(QMC2_EMUTYPE_MAME)
    qmc2Flyer->flyerFile = unzOpen((const char *)qmc2Config->value("MAME/FilesAndDirectories/FlyerFile").toString().toAscii());
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2Flyer->flyerFile = unzOpen((const char *)qmc2Config->value("MESS/FilesAndDirectories/FlyerFile").toString().toAscii());
#endif
    qmc2Flyer->setUpdatesEnabled(TRUE);
  } else {
#if defined(Q_WS_WIN)
    QString command = "cmd.exe";
    QStringList args;
    args << "/c" << qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/FileRemovalTool").toString().replace('/', '\\')
         << qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/FileRemovalToolArguments").toString().split(" ");
#else
    QString command = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/FileRemovalTool").toString();
    QStringList args = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/FileRemovalToolArguments").toString().split(" ");
#endif

    int i, j;
    QStringList addArgs;
    for (i = 0; i < args.count(); i++) {
      if ( args[i] == "$FILELIST$" ) {
        QList<QListWidgetItem *> items = listWidgetFlyersObsolete->findItems("*", Qt::MatchWildcard); 
        for (j = 0; j < items.count(); j++) {
#if defined(Q_WS_WIN)
#if defined(QMC2_EMUTYPE_MAME)
          addArgs << qmc2Config->value("MAME/FilesAndDirectories/FlyerDirectory").toString().replace('/', '\\') + items[j]->text().replace('/', '\\');
#elif defined(QMC2_EMUTYPE_MESS)
          addArgs << qmc2Config->value("MESS/FilesAndDirectories/FlyerDirectory").toString().replace('/', '\\') + items[j]->text().replace('/', '\\');
#endif
#else
#if defined(QMC2_EMUTYPE_MAME)
          addArgs << qmc2Config->value("MAME/FilesAndDirectories/FlyerDirectory").toString() + items[j]->text();
#elif defined(QMC2_EMUTYPE_MESS)
          addArgs << qmc2Config->value("MESS/FilesAndDirectories/FlyerDirectory").toString() + items[j]->text();
#endif
#endif
        }
        args.removeAt(i);
        args << addArgs;
      }
    }

    ToolExecutor fileRemovalTool(this, command, args);
    fileRemovalTool.exec();
  }
}

void ImageChecker::on_listWidgetFlyersFound_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageChecker::on_listWidgetFlyersFound_itemSelectionChanged()");
#endif

  if ( checkBoxFlyersSelectGame->isChecked() ) {
    QList<QListWidgetItem *> items = listWidgetFlyersFound->selectedItems();
    if ( items.count() > 0 )
      selectItem(items[0]->text());
  }
}

void ImageChecker::on_listWidgetFlyersMissing_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageChecker::on_listWidgetFlyersMissing_itemSelectionChanged()");
#endif

  if ( checkBoxFlyersSelectGame->isChecked() ) {
    QList<QListWidgetItem *> items = listWidgetFlyersMissing->selectedItems();
    if ( items.count() > 0 )
      selectItem(items[0]->text());
  }
}

void ImageChecker::on_pushButtonIconsCheck_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageChecker::on_pushButtonIconsCheck_clicked()");
#endif

  if ( lockProcessing() ) {
    // check icons
    QTime checkTimer, elapsedTime;
    pushButtonIconsRemoveObsolete->setEnabled(FALSE);
    checkTimer.start();
    if ( checkBoxIconsClearCache->isChecked() )
      qmc2MainWindow->on_actionClearIconCache_activated();
    if ( qmc2UseIconFile )
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("checking icons from ZIP archive"));
    else
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("checking icons from directory"));

    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
      qmc2MainWindow->progressBarGamelist->setFormat(tr("Icon check - %p%"));
    else
      qmc2MainWindow->progressBarGamelist->setFormat("%p%");
    qmc2MainWindow->progressBarGamelist->setRange(0, qmc2Gamelist->numTotalGames + qmc2Gamelist->numDevices);
    qmc2MainWindow->progressBarGamelist->reset();

    if ( qmc2Gamelist->numTotalGames + qmc2Gamelist->numDevices != qmc2Gamelist->numGames )
#if defined(QMC2_EMUTYPE_MAME)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: game list not fully loaded, check results may be misleading"));
#elif defined(QMC2_EMUTYPE_MESS)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: machine list not fully loaded, check results may be misleading"));
#endif

    // change check buttons to stop buttons
    pushButtonPreviewsCheck->setText(tr("&Stop check"));
    pushButtonFlyersCheck->setText(tr("&Stop check"));
    pushButtonIconsCheck->setText(tr("&Stop check"));

    // avoid changes to icon source during check
    qmc2Options->radioButtonIconSelect->setEnabled(FALSE);
    qmc2Options->stackedWidgetIcon->setEnabled(FALSE);
    qmc2Options->lineEditIconFile->setEnabled(FALSE);
    qmc2Options->lineEditIconDirectory->setEnabled(FALSE);
    qmc2Options->toolButtonBrowseIconFile->setEnabled(FALSE);
    qmc2Options->toolButtonBrowseIconDirectory->setEnabled(FALSE);

    // check pass 1: found and missing icons
    listWidgetIconsFound->clear();
    labelIconsFound->setText(tr("Found: 0"));
    listWidgetIconsMissing->clear();
    labelIconsMissing->setText(tr("Missing: 0"));
    listWidgetIconsObsolete->clear();
    labelIconsObsolete->setText(tr("Obsolete: 0"));
    QMapIterator<QString, QTreeWidgetItem *> it(qmc2GamelistItemMap);
    int i = -1, lastCheckTimer = 0;
    qmc2StopParser = FALSE;
    QStringList foundIcons, missingIcons;
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("check pass 1: found and missing icons"));
    QStringList fileNames;
    while ( it.hasNext() && !qmc2StopParser ) {
      i++;
      if ( checkTimer.elapsed() - lastCheckTimer >= QMC2_CHECK_UPDATE ) {
        lastCheckTimer = checkTimer.elapsed();
        qmc2MainWindow->progressBarGamelist->setValue(i);
        listWidgetIconsFound->addItems(foundIcons);
        labelIconsFound->setText(tr("Found: %1").arg(listWidgetIconsFound->count()));
        listWidgetIconsMissing->addItems(missingIcons);
        labelIconsMissing->setText(tr("Missing: %1").arg(listWidgetIconsMissing->count()));
        foundIcons.clear();
        missingIcons.clear();
      }
      it.next();
      QString gameName = it.key();
      QString fileName;
      if ( qmc2Gamelist->loadIcon(gameName, NULL, TRUE, &fileName) )
        foundIcons << gameName;
      else
        missingIcons << gameName;
      if ( !fileName.isEmpty() )
#if defined(Q_WS_WIN)
        fileNames << fileName.toLower();
#else
        fileNames << fileName;
#endif
      qApp->processEvents();
    }
    listWidgetIconsFound->addItems(foundIcons);
    labelIconsFound->setText(tr("Found: %1").arg(listWidgetIconsFound->count()));
    listWidgetIconsFound->sortItems(Qt::AscendingOrder);
    listWidgetIconsMissing->addItems(missingIcons);
    labelIconsMissing->setText(tr("Missing: %1").arg(listWidgetIconsMissing->count()));
    listWidgetIconsMissing->sortItems(Qt::AscendingOrder);

    // check pass 2: obsolete icons
    qmc2MainWindow->progressBarGamelist->reset();
    QStringList obsoleteFiles;
    if ( qmc2UseIconFile ) {
      // check from ZIP archive
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("check pass 2: obsolete files: reading ZIP directory"));
      qApp->processEvents();
      QStringList fileList;
      i = 0;
      unz_global_info unzGlobalInfo;
      if ( unzGetGlobalInfo(qmc2IconFile, &unzGlobalInfo) == UNZ_OK ) {
        qmc2MainWindow->progressBarGamelist->setRange(0, unzGlobalInfo.number_entry);
        qmc2MainWindow->progressBarGamelist->reset();
        qApp->processEvents();
        if ( unzGoToFirstFile(qmc2IconFile) == UNZ_OK ) {
          do {
            char unzFileName[QMC2_MAX_PATH_LENGTH];
            i++;
            if ( i % 25 == 0 ) {
              qmc2MainWindow->progressBarGamelist->setValue(i);
              qApp->processEvents();
            }
            if ( unzGetCurrentFileInfo(qmc2IconFile, NULL, unzFileName, QMC2_MAX_PATH_LENGTH, NULL, 0, NULL, 0) == UNZ_OK )
              fileList << unzFileName;
          } while ( unzGoToNextFile(qmc2IconFile) != UNZ_END_OF_LIST_OF_FILE );
        }
        qmc2MainWindow->progressBarGamelist->setRange(0, fileList.count());
        qmc2MainWindow->progressBarGamelist->reset();
      }

      for (i = 0; i < fileList.count() && !qmc2StopParser; i++) {
        if ( checkTimer.elapsed() - lastCheckTimer >= QMC2_CHECK_UPDATE ) {
          lastCheckTimer = checkTimer.elapsed();
          qmc2MainWindow->progressBarGamelist->setValue(i);
          listWidgetIconsObsolete->addItems(obsoleteFiles);
          labelIconsObsolete->setText(tr("Obsolete: %1").arg(listWidgetIconsObsolete->count()));
          obsoleteFiles.clear();
        }
#if defined(Q_WS_WIN)
        if ( !fileNames.contains(fileList[i], Qt::CaseInsensitive) )
          obsoleteFiles << fileList[i];
#else
        if ( !fileNames.contains(fileList[i]) )
          obsoleteFiles << fileList[i];
#endif
        qApp->processEvents();
      }
      listWidgetIconsObsolete->addItems(obsoleteFiles);
      labelIconsObsolete->setText(tr("Obsolete: %1").arg(listWidgetIconsObsolete->count()));
      listWidgetIconsObsolete->sortItems(Qt::AscendingOrder);
    } else {
      // check from directory
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("check pass 2: obsolete files: reading directory structure"));
      qApp->processEvents();
      QStringList fileList;
#if defined(QMC2_EMUTYPE_MAME)
      QString iconDir = qmc2Config->value("MAME/FilesAndDirectories/IconDirectory").toString();
#elif defined(QMC2_EMUTYPE_MESS)
      QString iconDir = qmc2Config->value("MESS/FilesAndDirectories/IconDirectory").toString();
#endif
      recursiveFileList(iconDir, fileList);
      qmc2MainWindow->progressBarGamelist->setRange(0, fileList.count());
      qmc2MainWindow->progressBarGamelist->reset();
      for (i = 0; i < fileList.count() && !qmc2StopParser; i++) {
        if ( checkTimer.elapsed() - lastCheckTimer >= QMC2_CHECK_UPDATE ) {
          lastCheckTimer = checkTimer.elapsed();
          qmc2MainWindow->progressBarGamelist->setValue(i);
          listWidgetIconsObsolete->addItems(obsoleteFiles);
          labelIconsObsolete->setText(tr("Obsolete: %1").arg(listWidgetIconsObsolete->count()));
          obsoleteFiles.clear();
        }
        QString relativeFilePath = fileList[i].remove(iconDir);
        if ( !fileNames.contains(relativeFilePath) )
          obsoleteFiles << relativeFilePath;
        qApp->processEvents();
      }
      listWidgetIconsObsolete->addItems(obsoleteFiles);
      labelIconsObsolete->setText(tr("Obsolete: %1").arg(listWidgetIconsObsolete->count()));
      listWidgetIconsObsolete->sortItems(Qt::AscendingOrder);
    }

    qmc2MainWindow->progressBarGamelist->reset();

    // reallow changes to icon source after check
    qmc2Options->radioButtonIconSelect->setEnabled(TRUE);
    qmc2Options->stackedWidgetIcon->setEnabled(TRUE);
    qmc2Options->lineEditIconFile->setEnabled(TRUE);
    qmc2Options->lineEditIconDirectory->setEnabled(TRUE);
    qmc2Options->toolButtonBrowseIconFile->setEnabled(TRUE);
    qmc2Options->toolButtonBrowseIconDirectory->setEnabled(TRUE);

    // log check timing
    elapsedTime = elapsedTime.addMSecs(checkTimer.elapsed());
    if ( qmc2UseIconFile )
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (checking icons from ZIP archive, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
    else
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (checking icons from directory, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("%1 found, %2 missing, %3 obsolete").arg(listWidgetIconsFound->count()).arg(listWidgetIconsMissing->count()).arg(listWidgetIconsObsolete->count()));

    // enable removal button if obsolete images exist
    pushButtonIconsRemoveObsolete->setEnabled(listWidgetIconsObsolete->count() > 0);

    // reset check buttons
    pushButtonPreviewsCheck->setText(tr("&Check previews"));
    pushButtonFlyersCheck->setText(tr("&Check flyers"));
    pushButtonIconsCheck->setText(tr("&Check icons"));

    // release lock
    qmc2ImageCheckActive = FALSE;
  }
}

void ImageChecker::on_pushButtonIconsRemoveObsolete_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageChecker::on_pushButtonIconsRemoveObsolete_clicked()");
#endif

  if ( qmc2UseIconFile ) {
#if defined(Q_WS_WIN)
    QString command = "cmd.exe";
    QStringList args;
    args << "/c" << qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/ZipTool").toString().replace('/', '\\')
         << qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/ZipToolRemovalArguments").toString().split(" ");
#else
    QString command = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/ZipTool").toString();
    QStringList args = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/ZipToolRemovalArguments").toString().split(" ");
#endif

    int i, j;
    QStringList addArgs;
    for (i = 0; i < args.count(); i++) {
      if ( args[i] == "$ARCHIVE$" ) {
#if defined(QMC2_EMUTYPE_MAME)
        args[i] = qmc2Config->value("MAME/FilesAndDirectories/IconFile").toString();
#elif defined(QMC2_EMUTYPE_MESS)
        args[i] = qmc2Config->value("MESS/FilesAndDirectories/IconFile").toString();
#endif
#if defined(Q_WS_WIN)
        args[i] = args[i].replace('/', '\\');
#endif
      } else if ( args[i] == "$FILELIST$" ) {
        QList<QListWidgetItem *> items = listWidgetIconsObsolete->findItems("*", Qt::MatchWildcard); 
        for (j = 0; j < items.count(); j++) {
#if defined(Q_WS_WIN)
          addArgs << items[j]->text().replace('/', '\\');
#else
          addArgs << items[j]->text();
#endif
        }
        args.removeAt(i);
        args << addArgs;
      }
    }

    unzClose(qmc2IconFile);
    ToolExecutor zipRemovalTool(this, command, args);
    zipRemovalTool.exec();
#if defined(QMC2_EMUTYPE_MAME)
    qmc2IconFile = unzOpen((const char *)qmc2Config->value("MAME/FilesAndDirectories/IconFile").toString().toAscii());
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2IconFile = unzOpen((const char *)qmc2Config->value("MESS/FilesAndDirectories/IconFile").toString().toAscii());
#endif
  } else {
#if defined(Q_WS_WIN)
    QString command = "cmd.exe";
    QStringList args;
    args << "/c" << qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/FileRemovalTool").toString().replace('/', '\\')
         << qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/FileRemovalToolArguments").toString().split(" ");
#else
    QString command = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/FileRemovalTool").toString();
    QStringList args = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/FileRemovalToolArguments").toString().split(" ");
#endif

    int i, j;
    QStringList addArgs;
    for (i = 0; i < args.count(); i++) {
      if ( args[i] == "$FILELIST$" ) {
        QList<QListWidgetItem *> items = listWidgetIconsObsolete->findItems("*", Qt::MatchWildcard); 
        for (j = 0; j < items.count(); j++) {
#if defined(Q_WS_WIN)
#if defined(QMC2_EMUTYPE_MAME)
          addArgs << qmc2Config->value("MAME/FilesAndDirectories/IconDirectory").toString().replace('/', '\\') + items[j]->text().replace('/', '\\');
#elif defined(QMC2_EMUTYPE_MESS)
          addArgs << qmc2Config->value("MESS/FilesAndDirectories/IconDirectory").toString().replace('/', '\\') + items[j]->text().replace('/', '\\');
#endif
#else
#if defined(QMC2_EMUTYPE_MAME)
          addArgs << qmc2Config->value("MAME/FilesAndDirectories/IconDirectory").toString() + items[j]->text();
#elif defined(QMC2_EMUTYPE_MESS)
          addArgs << qmc2Config->value("MESS/FilesAndDirectories/IconDirectory").toString() + items[j]->text();
#endif
#endif
        }
        args.removeAt(i);
        args << addArgs;
      }
    }

    ToolExecutor fileRemovalTool(this, command, args);
    fileRemovalTool.exec();
  }
}

void ImageChecker::on_listWidgetIconsFound_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageChecker::on_listWidgetIconsFound_itemSelectionChanged()()");
#endif

  if ( checkBoxIconsSelectGame->isChecked() ) {
    QList<QListWidgetItem *> items = listWidgetIconsFound->selectedItems();
    if ( items.count() > 0 )
      selectItem(items[0]->text());
  }
}

void ImageChecker::on_listWidgetIconsMissing_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageChecker::on_listWidgetIconsMissing_itemSelectionChanged()");
#endif

  if ( checkBoxIconsSelectGame->isChecked() ) {
    QList<QListWidgetItem *> items = listWidgetIconsMissing->selectedItems();
    if ( items.count() > 0 )
      selectItem(items[0]->text());
  }
}

bool ImageChecker::lockProcessing()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageChecker::lockProcessing()");
#endif

  if ( qmc2ReloadActive ) {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("please wait for reload to finish and try again"));
    return FALSE;
  }

  if ( qmc2ROMAlyzerActive ) {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("please wait for ROMAlyzer to finish the current analysis and try again"));
    return FALSE;
  }

  if ( qmc2FilterActive ) {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("please wait for ROM state filter to finish and try again"));
    return FALSE;
  }

  if ( qmc2VerifyActive ) {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("please wait for ROM verification to finish and try again"));
    return FALSE;
  }

  if ( qmc2SampleCheckActive ) {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("please wait for sample check to finish and try again"));
    return FALSE;
  }

  if ( qmc2ImageCheckActive ) {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("stopping image check upon user request"));
    qmc2StopParser = TRUE;
    return FALSE;
  }

  qmc2ImageCheckActive = TRUE;
  return TRUE;
}

void ImageChecker::selectItem(QString gameName)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ImageChecker::selectItem(QString gameName = %1)").arg(gameName));
#endif

  switch ( qmc2MainWindow->stackedWidgetView->currentIndex() ) {
    case QMC2_VIEWGAMELIST_INDEX: {
      QTreeWidgetItem *gameItem = qmc2GamelistItemMap[gameName];
      if ( gameItem ) {
        qmc2MainWindow->treeWidgetGamelist->clearSelection();
        qmc2MainWindow->treeWidgetGamelist->setCurrentItem(gameItem);
        qmc2MainWindow->treeWidgetGamelist->scrollToItem(gameItem, qmc2CursorPositioningMode);
        gameItem->setSelected(TRUE);
      }
      break;
    }
    case QMC2_VIEWHIERARCHY_INDEX: {
      QTreeWidgetItem *hierarchyItem = qmc2HierarchyItemMap[gameName];
      if ( hierarchyItem ) {
        qmc2MainWindow->treeWidgetHierarchy->clearSelection();
        qmc2MainWindow->treeWidgetHierarchy->setCurrentItem(hierarchyItem);
        qmc2MainWindow->treeWidgetHierarchy->scrollToItem(hierarchyItem, qmc2CursorPositioningMode);
        hierarchyItem->setSelected(TRUE);
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
        categoryItem->setSelected(TRUE);
      }
      break;
    }
    case QMC2_VIEWVERSION_INDEX: {
      QTreeWidgetItem *versionItem = qmc2VersionItemMap[gameName];
      if ( versionItem ) {
        qmc2MainWindow->treeWidgetVersionView->clearSelection();
        qmc2MainWindow->treeWidgetVersionView->setCurrentItem(versionItem);
        qmc2MainWindow->treeWidgetVersionView->scrollToItem(versionItem, qmc2CursorPositioningMode);
        versionItem->setSelected(TRUE);
      }
      break;
    }
#endif
  }
}

void ImageChecker::restoreLayout()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageChecker::restoreLayout()");
#endif

  if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "Layout/ImageChecker/Position") )
    move(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/ImageChecker/Position", pos()).toPoint());
  if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "Layout/ImageChecker/Size") )
    resize(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/ImageChecker/Size", size()).toSize());
}

void ImageChecker::closeEvent(QCloseEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ImageChecker::closeEvent(QCloseEvent *e = %1)").arg((qulonglong)e));
#endif

  // save settings
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ImageChecker/Previews/SelectGame", checkBoxPreviewsSelectGame->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ImageChecker/Flyers/SelectGame", checkBoxFlyersSelectGame->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ImageChecker/Icons/SelectGame", checkBoxIconsSelectGame->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ImageChecker/Icons/ClearCache", checkBoxIconsClearCache->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ImageChecker/CurrentTab", tabWidgetImageChecker->currentIndex());
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() ) {
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/ImageChecker/Position", pos());
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/ImageChecker/Size", size());
    if ( !qmc2CleaningUp )
      qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/ImageChecker/Visible", FALSE);
  }

  if ( e )
    e->accept();
}

void ImageChecker::hideEvent(QHideEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ImageChecker::hideEvent(QHideEvent *e = %1)").arg((qulonglong)e));
#endif

  closeEvent(NULL);
  e->accept();
}

void ImageChecker::showEvent(QShowEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ImageChecker::showEvent(QShowEvent *e = %1)").arg((qulonglong)e));
#endif

  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/RestoreLayout").toBool() )
    restoreLayout();

  // restore settings
  checkBoxPreviewsSelectGame->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ImageChecker/Previews/SelectGame", TRUE).toBool());
  checkBoxFlyersSelectGame->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ImageChecker/Flyers/SelectGame", TRUE).toBool());
  checkBoxIconsSelectGame->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ImageChecker/Icons/SelectGame", TRUE).toBool());
  checkBoxIconsClearCache->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ImageChecker/Icons/ClearCache", TRUE).toBool());

  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/ImageChecker/Visible", TRUE);

  if ( e )
    e->accept();
}

void ImageChecker::recursiveFileList(const QString &sDir, QStringList &fileNames)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ImageChecker::recursiveFileList(const QString& sDir = %1, QStringList &fileNames)").arg(sDir));
#endif

  QDir dir(sDir);
  QFileInfoList list = dir.entryInfoList();
  int i;
  for (i = 0; i < list.count(); i++) {
    QFileInfo info = list[i];
    QString path = info.filePath();
    if ( info.isDir() ) {
      // directory recursion
      if ( info.fileName() != ".." && info.fileName() != "." ) {
        recursiveFileList(path, fileNames);
        qApp->processEvents();
      }
    } else
      fileNames << path;
  }
} 
