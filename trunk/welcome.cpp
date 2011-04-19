#include <QTimer>
#include <QMessageBox>
#include <QFileInfo>
#include <QFileDialog>
#include <QTranslator>
#include <QLocale>

#include "welcome.h"
#include "macros.h"

#ifdef QMC2_DEBUG
#include "qmc2main.h"
extern MainWindow *qmc2MainWindow;
#endif

// external global variables
extern QTranslator *qmc2Translator;
extern QTranslator *qmc2QtTranslator;

Welcome::Welcome(QWidget *parent)
  : QDialog(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Welcome::Welcome(QWidget *parent = 0x" + QString::number((ulong)parent, 16) + ")");
#endif

  checkOkay = FALSE;
  hide();
  if ( !checkConfig() ) {
    setupUi(this);

#if defined(QMC2_SDLMAME)
    QString emulatorName = tr("SDLMAME");
#elif defined(QMC2_SDLMESS)
    QString emulatorName = tr("SDLMESS");
    labelSamplePath->setVisible(FALSE);
    lineEditSamplePath->setVisible(FALSE);
    toolButtonBrowseSamplePath->setVisible(FALSE);
#elif defined(QMC2_MAME)
    QString emulatorName = tr("MAME");
#elif defined(QMC2_MESS)
    QString emulatorName = tr("MESS");
    labelSamplePath->setVisible(FALSE);
    lineEditSamplePath->setVisible(FALSE);
    toolButtonBrowseSamplePath->setVisible(FALSE);
#else
    QString emulatorName = tr("Unsupported emulator");
#endif
    labelExecutableFile->setText(tr("%1 executable file").arg(emulatorName));
    show();
    adjustSize();
  } else {
    checkOkay = TRUE;
    QTimer::singleShot(0, this, SLOT(on_pushButtonOkay_clicked()));
  }
}

Welcome::~Welcome()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Welcome::~Welcome()");
#endif

}

void Welcome::on_pushButtonOkay_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Welcome::on_pushButtonOkay_clicked()");
#endif

  if ( !checkOkay ) {
    QFileInfo fileInfo(lineEditExecutableFile->text());
    if ( fileInfo.isExecutable() && fileInfo.isReadable() && fileInfo.isFile() ) {
#if defined(QMC2_EMUTYPE_MAME)
      startupConfig->setValue("MAME/FilesAndDirectories/ExecutableFile", lineEditExecutableFile->text());
      if ( !lineEditROMPath->text().isEmpty() )
        startupConfig->setValue("MAME/Configuration/Global/rompath", lineEditROMPath->text());
      if ( !lineEditSamplePath->text().isEmpty() )
        startupConfig->setValue("MAME/Configuration/Global/samplepath", lineEditSamplePath->text());
      if ( !lineEditHashPath->text().isEmpty() )
        startupConfig->setValue("MAME/Configuration/Global/hashpath", lineEditHashPath->text());
#elif defined(QMC2_EMUTYPE_MESS)
      startupConfig->setValue("MESS/FilesAndDirectories/ExecutableFile", lineEditExecutableFile->text());
      if ( !lineEditROMPath->text().isEmpty() )
        startupConfig->setValue("MESS/Configuration/Global/rompath", lineEditROMPath->text());
      if ( !lineEditHashPath->text().isEmpty() )
        startupConfig->setValue("MESS/Configuration/Global/hashpath", lineEditHashPath->text());
#endif
      startupConfig->sync();
      delete startupConfig;
      emit accept();
    } else {
      QMessageBox::critical(this, tr("Error"), tr("The specified file isn't executable!"));
    }
  } else {
    delete startupConfig;
    emit accept();
  }
}

void Welcome::on_toolButtonBrowseExecutableFile_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Welcome::on_toolButtonBrowseExecutableFile_clicked()");
#endif

  QFileInfo fileInfo(lineEditExecutableFile->text());
  QString s = QFileDialog::getOpenFileName(this, tr("Choose emulator executable file"), fileInfo.absolutePath(), tr("All files (*)"));
  if ( !s.isNull() )
    lineEditExecutableFile->setText(s);
  raise();
}

void Welcome::on_toolButtonBrowseROMPath_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Welcome::on_toolButtonBrowseROMPath_clicked()");
#endif

  QString s = QFileDialog::getExistingDirectory(this, tr("Choose ROM path"), lineEditROMPath->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  if ( !s.isNull() )
    lineEditROMPath->setText(s);
  raise();
}

void Welcome::on_toolButtonBrowseSamplePath_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Welcome::on_toolButtonBrowseSamplePath_clicked()");
#endif

#if defined(QMC2_EMUTYPE_MAME)
  QString s = QFileDialog::getExistingDirectory(this, tr("Choose sample path"), lineEditSamplePath->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  if ( !s.isNull() )
    lineEditSamplePath->setText(s);
  raise();
#endif
}

void Welcome::on_toolButtonBrowseHashPath_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Welcome::on_toolButtonBrowseHashPath_clicked()");
#endif

  QString s = QFileDialog::getExistingDirectory(this, tr("Choose hash path"), lineEditHashPath->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  if ( !s.isNull() )
    lineEditHashPath->setText(s);
  raise();
}

void Welcome::setupLanguage()
{
  QString lang = startupConfig->value("GUI/Language").toString();

  QStringList availableLanguages;
  availableLanguages << "us" << "de" << "pl" << "fr" << "gr" << "pt";

  if ( lang.isEmpty() || !availableLanguages.contains(lang) ) {
    // try to use default system locale - use "us" if a translation is not available for the system locale
    switch ( QLocale::system().language() ) {
      case QLocale::French: lang = "fr"; break;
      case QLocale::German: lang = "de"; break;
      case QLocale::Polish: lang = "pl"; break;
      case QLocale::Portuguese: lang = "pt"; break;
      default: lang = "us"; break;
    }
    startupConfig->setValue("GUI/Language", lang);
  }

  QString directory = startupConfig->value("FilesAndDirectories/DataDirectory", "data/").toString() + "lng/";
  qmc2QtTranslator = new QTranslator(0);
  qmc2QtTranslator->load(directory + QString("qt_") + lang + ".qm");
  qApp->installTranslator(qmc2QtTranslator);
  qmc2Translator = new QTranslator(0);
  qmc2Translator->load(directory + QString("qmc2_") + lang + ".qm");
  qApp->installTranslator(qmc2Translator);
}

bool Welcome::checkConfig()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Welcome::checkConfig()");
#endif

  bool configOkay = TRUE;

#if !defined(Q_WS_WIN)
  QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, QMC2_SYSCONF_PATH);
#endif
  QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, QMC2_DYNAMIC_DOT_PATH);

  qApp->setOrganizationName("qmc2");
  startupConfig = new QSettings(QSettings::IniFormat, QSettings::UserScope, "qmc2");

#if defined(QMC2_SDLMAME)
  variant = "qmc2-sdlmame";
  fallbackVariant = "qmc2-sdlmess";
#elif defined(QMC2_SDLMESS)
  variant = "qmc2-sdlmess";
  fallbackVariant = "qmc2-sdlmame";
#elif defined(QMC2_MAME)
  variant = "qmc2-mame";
  fallbackVariant = "qmc2-mess";
#elif defined(QMC2_MESS)
  variant = "qmc2-mess";
  fallbackVariant = "qmc2-mame";
#else
  variant = "qmc2-???";
  fallbackVariant = "qmc2-sdlmame";
#endif

  startupConfig->beginGroup(QString("Frontend/%1").arg(variant));

  setupLanguage();

  if ( startupConfig->value("GUI/CheckSingleInstance", TRUE).toBool() ) {
    if ( startupConfig->value(QString("InstanceRunning")).toBool() ) {
      switch ( QMessageBox::question(this, tr("Single-instance check"),
                                           tr("It appears that another instance of %1 is already running.\nHowever, this can also be the leftover of a previous crash.\n\nExit now, accept once or ignore completely?").arg(variant),
                                           tr("&Exit"), tr("&Once"), tr("&Ignore"), 0, 0) ) {
         case 0:
           startupConfig->setValue("GUI/CheckSingleInstance", TRUE);
           qApp->quit();
           return FALSE;
           break;

         case 1:
           startupConfig->setValue("GUI/CheckSingleInstance", TRUE);
           break;

         case 2: 
         default:
           startupConfig->setValue("GUI/CheckSingleInstance", FALSE);
           break;
      }
    }
  }

  startupConfig->endGroup();

  int oldMajor = 0, oldMinor = 0, oldBeta = 0;
  QStringList versionList = startupConfig->value("Version").toString().split(".");
  int oldSvnRevision = startupConfig->value("SVN_Revision").toInt();
  if ( versionList.count() > 1 ) {
	  oldMajor = versionList[0].toInt();
	  oldMinor = versionList[1].toInt();
	  if ( versionList.count() > 2 )
		  oldBeta = versionList[2].mid(1).toInt();
#if defined(QMC2_EMUTYPE_MAME)
	  if ( oldMinor == 2 ) {
		  if ( oldBeta < 19 || (oldSvnRevision < 2559 && oldSvnRevision > 0 ) ) {
			  // GLC format change (V3) in QMC2 0.2.b19 / SVN r2559 -- any saved header states for the category- and version-views must be invalidated!
			  startupConfig->remove(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/CategoryViewHeaderState");
			  startupConfig->remove(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/VersionViewHeaderState");
		  }
	  }
#endif
  }

#if defined(QMC2_EMUTYPE_MAME)
  configOkay &= !startupConfig->value("MAME/FilesAndDirectories/ExecutableFile").toString().isEmpty();
#elif defined(QMC2_EMUTYPE_MESS)
  configOkay &= !startupConfig->value("MESS/FilesAndDirectories/ExecutableFile").toString().isEmpty();
#endif

  return configOkay;
}
