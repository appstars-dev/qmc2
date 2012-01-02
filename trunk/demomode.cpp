#include <QMap>
#include <QTreeWidgetItem>
#include <QStringList>
#include <QSettings>

#include "demomode.h"
#include "qmc2main.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;
extern QMap<QString, QTreeWidgetItem *> qmc2GamelistItemMap;
extern QMap<QString, QString> qmc2GamelistDescriptionMap;
extern QStringList qmc2BiosROMs;
extern QString qmc2DemoGame;
extern QStringList qmc2DemoArgs;
extern bool qmc2ReloadActive;
extern bool qmc2VerifyActive;
extern QSettings *qmc2Config;

DemoModeDialog::DemoModeDialog(QWidget *parent)
  : QDialog(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: DemoModeDialog::DemoModeDialog(QWidget *parent = %1").arg((qulonglong) parent));
#endif

  setupUi(this);
  demoModeRunning = false;
  emuProcess = NULL;
#if !defined(Q_WS_X11) && !defined(Q_WS_WIN)
  checkBoxEmbedded->setVisible(false);
#endif

  clearStatus();

  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/RestoreLayout").toBool() )
    restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + "DemoMode/Geometry").toByteArray());
  toolButtonSelectC->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "DemoMode/SelectC", true).toBool());
  toolButtonSelectM->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "DemoMode/SelectM", true).toBool());
  toolButtonSelectI->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "DemoMode/SelectI", false).toBool());
  toolButtonSelectN->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "DemoMode/SelectN", false).toBool());
  toolButtonSelectU->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "DemoMode/SelectU", false).toBool());
  checkBoxFullScreen->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "DemoMode/FullScreen", true).toBool());
  checkBoxMaximized->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "DemoMode/Maximized", false).toBool());
#if defined(Q_WS_X11) || defined(Q_WS_WIN)
  checkBoxEmbedded->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "DemoMode/Embedded", false).toBool());
#endif
  checkBoxTagged->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "DemoMode/Tagged", false).toBool());
  spinBoxSecondsToRun->setValue(qmc2Config->value(QMC2_FRONTEND_PREFIX + "DemoMode/SecondsToRun", 60).toInt());
  spinBoxPauseSeconds->setValue(qmc2Config->value(QMC2_FRONTEND_PREFIX + "DemoMode/PauseSeconds", 2).toInt());
}

DemoModeDialog::~DemoModeDialog()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DemoModeDialog::~DemoModeDialog()");
#endif

}

void DemoModeDialog::showEvent(QShowEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: DemoModeDialog::showEvent(QShowEvent *e = %1)").arg((qulonglong) e));
#endif

  // try to "grab" the input focus...
  activateWindow();
  setFocus();
}

void DemoModeDialog::closeEvent(QCloseEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: DemoModeDialog::closeEvent(QCloseEvent *e = %1)").arg((qulonglong) e));
#endif

  if ( demoModeRunning )
    pushButtonRunDemo->animateClick();

  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "DemoMode/SelectC", toolButtonSelectC->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "DemoMode/SelectM", toolButtonSelectM->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "DemoMode/SelectI", toolButtonSelectI->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "DemoMode/SelectN", toolButtonSelectN->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "DemoMode/SelectU", toolButtonSelectU->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "DemoMode/FullScreen", checkBoxFullScreen->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "DemoMode/Maximized", checkBoxMaximized->isChecked());
#if defined(Q_WS_X11) || defined(Q_WS_WIN)
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "DemoMode/Embedded",checkBoxEmbedded->isChecked());
#endif
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "DemoMode/Tagged",checkBoxTagged->isChecked());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "DemoMode/SecondsToRun", spinBoxSecondsToRun->value());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "DemoMode/PauseSeconds", spinBoxPauseSeconds->value());
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "DemoMode/Geometry", saveGeometry());
}

void DemoModeDialog::on_pushButtonRunDemo_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DemoModeDialog::on_pushButtonRunDemo_clicked()");
#endif

  if ( demoModeRunning ) {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("demo mode stopped"));
    demoModeRunning = false;
    pushButtonRunDemo->setText(tr("Run &demo"));
    pushButtonRunDemo->setToolTip(tr("Run demo now"));
    qmc2DemoGame = "";
    qmc2DemoArgs.clear();
    if ( emuProcess ) {
      emuProcess->terminate();
      emuProcess = NULL;
    }
    qmc2MainWindow->actionCheckROMs->setEnabled(true);
    qmc2MainWindow->actionPlay->setEnabled(true);
#if defined(Q_WS_X11) || defined(Q_WS_WIN)
    qmc2MainWindow->actionPlayEmbedded->setEnabled(true);
#endif
    qmc2MainWindow->enableContextMenuPlayActions(true);
    clearStatus();
  } else {
    if ( qmc2ReloadActive ) {
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("please wait for reload to finish and try again"));
      return;
    }
    if ( qmc2VerifyActive ) {
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("please wait for ROM verification to finish and try again"));
      return;
    }
    selectedGames.clear();
    if ( checkBoxTagged->isChecked() ) {
	    foreach (QString game, qmc2GamelistItemMap.keys()) {
		    if ( qmc2BiosROMs.contains(game) ) continue;
		    QTreeWidgetItem *gameItem = qmc2GamelistItemMap[game];
		    if ( !gameItem ) continue;
		    if ( gameItem->checkState(QMC2_GAMELIST_COLUMN_TAG) == Qt::Checked )
			    selectedGames << game;
	    }
    } else {
	    foreach (QString game, qmc2GamelistItemMap.keys()) {
	      if ( qmc2BiosROMs.contains(game) ) continue;
	      QTreeWidgetItem *gameItem = qmc2GamelistItemMap[game];
	      if ( !gameItem ) continue;
	      switch ( gameItem->whatsThis(QMC2_GAMELIST_COLUMN_GAME).at(0).toAscii() ) {
		case QMC2_ROMSTATE_CHAR_C:
		  if ( toolButtonSelectC->isChecked() ) selectedGames << game;
		  break;

		case QMC2_ROMSTATE_CHAR_M:
		  if ( toolButtonSelectM->isChecked() ) selectedGames << game;
		  break;

		case QMC2_ROMSTATE_CHAR_I:
		  if ( toolButtonSelectI->isChecked() ) selectedGames << game;
		  break;

		case QMC2_ROMSTATE_CHAR_N:
		  if ( toolButtonSelectN->isChecked() ) selectedGames << game;
		  break;

		case QMC2_ROMSTATE_CHAR_U:
		default:
		  if ( toolButtonSelectU->isChecked() ) selectedGames << game;
		  break;
	      }
	    }
    }
    if ( selectedGames.count() > 0 )
	    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("demo mode started -- %n game(s) selected by filter", "", selectedGames.count()));
    else {
	    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("demo mode cannot start -- no games selected by filter"));
	    return;
    }
    demoModeRunning = true;
    pushButtonRunDemo->setText(tr("Stop &demo"));
    pushButtonRunDemo->setToolTip(tr("Stop demo now"));
    qmc2MainWindow->actionCheckROMs->setEnabled(false);
    qmc2MainWindow->actionPlay->setEnabled(false);
#if defined(Q_WS_X11) || defined(Q_WS_WIN)
    qmc2MainWindow->actionPlayEmbedded->setEnabled(false);
#endif
    qmc2MainWindow->enableContextMenuPlayActions(false);
    QTimer::singleShot(0, this, SLOT(startNextEmu()));
  }
}

void DemoModeDialog::emuStarted()
{
  emuProcess = (QProcess *)sender();
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DemoModeDialog::emuStarted()");
#endif

}

void DemoModeDialog::emuFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DemoModeDialog::emuFinished(int exitCode = ..., QProcess::ExitStatus exitStatus = ...)");
#endif

  // try to "grab" the input focus...
  activateWindow();
  setFocus();

  qmc2DemoArgs.clear();
  qmc2DemoGame.clear();
  emuProcess = NULL;

  if ( demoModeRunning ) {
    clearStatus();
    QTimer::singleShot(spinBoxPauseSeconds->value() * 1000, this, SLOT(startNextEmu()));
  }
}

void DemoModeDialog::startNextEmu()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DemoModeDialog::startNextEmu()");
#endif

  if ( !demoModeRunning )
    return;

  qmc2DemoArgs.clear();
  qmc2DemoArgs << "-str" << QString::number(spinBoxSecondsToRun->value());
  emuProcess = NULL;
  if ( checkBoxFullScreen->isChecked() ) {
    qmc2DemoArgs << "-nowindow";
  } else {
    qmc2DemoArgs << "-window";
    if ( checkBoxMaximized )
      qmc2DemoArgs << "-maximize";
    else
      qmc2DemoArgs << "-nomaximize";
  }
  qmc2DemoGame = selectedGames[qrand() % selectedGames.count()];
  QString gameDescription = qmc2GamelistDescriptionMap[qmc2DemoGame];
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("starting emulation in demo mode for '%1'").arg(gameDescription));
  setStatus(gameDescription);
#if defined(Q_WS_X11) || defined(Q_WS_WIN)
  if ( checkBoxEmbedded->isChecked() && !checkBoxFullScreen->isChecked() )
    QTimer::singleShot(0, qmc2MainWindow, SLOT(on_actionPlayEmbedded_activated()));
  else
    QTimer::singleShot(0, qmc2MainWindow, SLOT(on_actionPlay_activated()));
#else
  QTimer::singleShot(0, qmc2MainWindow, SLOT(on_actionPlay_activated()));
#endif
}

void DemoModeDialog::adjustIconSizes()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DemoModeDialog::adjustIconSizes()");
#endif

  QFontMetrics fm(qApp->font());
  QSize iconSize(fm.height() - 2, fm.height() - 2);

  toolButtonSelectC->setIconSize(iconSize);
  toolButtonSelectM->setIconSize(iconSize);
  toolButtonSelectI->setIconSize(iconSize);
  toolButtonSelectN->setIconSize(iconSize);
  toolButtonSelectU->setIconSize(iconSize);

  adjustSize();
}

void DemoModeDialog::setStatus(QString statusString)
{
	if ( statusString.isEmpty() ) {
		labelDemoStatus->clear();
		labelDemoStatus->hide();
	} else {
		labelDemoStatus->setText("<font size=\"+1\"><b>" + statusString + "</b></font>");
		labelDemoStatus->show();
	}
	adjustSize();
}
