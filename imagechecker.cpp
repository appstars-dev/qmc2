#include <QSettings>
#include "imagechecker.h"
#include "preview.h"
#include "flyer.h"
#include "cabinet.h"
#include "controller.h"
#include "marquee.h"
#include "title.h"
#include "pcb.h"
#include "gamelist.h"
#include "qmc2main.h"
#include "macros.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern QMap<QString, QTreeWidgetItem *> qmc2GamelistItemMap;
extern QMap<QString, QTreeWidgetItem *> qmc2HierarchyItemMap;
extern QMap<QString, QTreeWidgetItem *> qmc2CategoryItemMap;
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
extern QMap<QString, QTreeWidgetItem *> qmc2VersionItemMap;
#endif
extern QMap<QString, QIcon> qmc2IconMap;
extern QSettings *qmc2Config;
extern Gamelist *qmc2Gamelist;
extern QAbstractItemView::ScrollHint qmc2CursorPositioningMode;
extern bool qmc2ImageCheckActive;
extern bool qmc2StopParser;
extern Preview *qmc2Preview;
extern Flyer *qmc2Flyer;
extern Cabinet *qmc2Cabinet;
extern Controller *qmc2Controller;
extern Marquee *qmc2Marquee;
extern Title *qmc2Title;
extern PCB *qmc2PCB;

#define QMC2_DEBUG

ImageCheckerThread::ImageCheckerThread(int tNum, ImageWidget *imgWidget, QObject *parent)
	: QThread(parent)
{
	threadNumber = tNum;
	imageWidget = imgWidget;
	isActive = exitThread = false;
}

ImageCheckerThread::~ImageCheckerThread()
{
	// NOP
}

void ImageCheckerThread::run()
{
#ifdef QMC2_DEBUG
	emit log(QString("DEBUG: ImageCheckerThread[%1]: started").arg(threadNumber));
#endif
	while ( !exitThread && !qmc2StopParser ) {
#ifdef QMC2_DEBUG
		emit log(QString("DEBUG: ImageCheckerThread[%1]: going to sleep").arg(threadNumber));
#endif

		mutex.lock();
		isActive = false;
		waitCondition.wait(&mutex);
		isActive = true;
		mutex.unlock();

#ifdef QMC2_DEBUG
		emit log(QString("DEBUG: ImageCheckerThread[%1]: woke up").arg(threadNumber));
#endif

		if ( !exitThread && !qmc2StopParser ) {
			if ( workUnitMutex.tryLock() ) {
#ifdef QMC2_DEBUG
				emit log(QString("DEBUG: ImageCheckerThread[%1]: processing work unit with %2 entries").arg(threadNumber).arg(workUnit.count()));
#endif
				foundList.clear();
				missingList.clear();
				foreach (QString gameName, workUnit) {
					if ( exitThread || qmc2StopParser )
						break;
					QString fileName;
					if ( imageWidget->checkImage(gameName, &fileName) ) {
						foundList << gameName;
#ifdef QMC2_DEBUG
						emit log(QString("DEBUG: ImageCheckerThread[%1]: image for '%2' found, loaded from '%3'").arg(threadNumber).arg(gameName).arg(fileName));
#endif
					} else {
						missingList << gameName;
#ifdef QMC2_DEBUG
						emit log(QString("DEBUG: ImageCheckerThread[%1]: image for '%2' missing").arg(threadNumber).arg(gameName));
#endif
					}
				}
				workUnit.clear();
				workUnitMutex.unlock();
				emit resultsReady(foundList, missingList);
			}
		}
	}

#ifdef QMC2_DEBUG
	emit log(QString("DEBUG: ImageCheckerThread[%1]: ended").arg(threadNumber));
#endif
}

ImageChecker::ImageChecker(QWidget *parent)
#if defined(Q_WS_WIN)
	: QDialog(parent, Qt::Dialog)
#else
	: QDialog(parent, Qt::Dialog | Qt::SubWindow)
#endif
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ImageChecker::ImageChecker(QWidget *parent = %1)").arg((qulonglong)parent));
#endif

	setupUi(this);

	isRunning = false;
	comboBoxImageType->insertSeparator(QMC2_IMGCHK_INDEX_SEPARATOR);
	connect(&updateTimer, SIGNAL(timeout()), this, SLOT(updateResults()));
}

ImageChecker::~ImageChecker()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageChecker::~ImageChecker()");
#endif

}

void ImageChecker::adjustIconSizes()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageChecker::adjustIconSizes()");
#endif

	QFont f;
	f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	QFontMetrics fm(f);
	QSize iconSize = QSize(fm.height() - 2, fm.height() - 2);
	QSize iconSizeMiddle = iconSize + QSize(2, 2);
	QSize iconSizeLarge = iconSize + QSize(4, 4);

	comboBoxImageType->setIconSize(iconSize);
	toolButtonSelectSets->setIconSize(iconSize);
	toolButtonClearCaches->setIconSize(iconSize);
	toolButtonStartStop->setIconSize(iconSize);
	toolButtonRemoveObsolete->setIconSize(iconSize);

	QFont logFont;
	logFont.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/LogFont").toString());
	plainTextEditLog->setFont(logFont);
}

void ImageChecker::on_listWidgetFound_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageChecker::on_listWidgetFound_itemSelectionChanged()");
#endif

	if ( toolButtonSelectSets->isChecked() ) {
		QList<QListWidgetItem *> items = listWidgetFound->selectedItems();
		if ( items.count() > 0 )
			selectItem(items[0]->text());
	}
}

void ImageChecker::on_listWidgetMissing_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageChecker::on_listWidgetMissing_itemSelectionChanged()");
#endif

	if ( toolButtonSelectSets->isChecked() ) {
		QList<QListWidgetItem *> items = listWidgetMissing->selectedItems();
		if ( items.count() > 0 )
			selectItem(items[0]->text());
	}
}

void ImageChecker::on_toolButtonStartStop_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageChecker::on_toolButtonStartStop_clicked()");
#endif

	if ( isRunning ) {
		foreach (ImageCheckerThread *thread, threadMap) {
			thread->exitThread = true;
			thread->waitCondition.wakeAll();
			thread->quit();
			thread->wait();
			delete thread;
		}
		threadMap.clear();
		isRunning = false;
		qmc2ImageCheckActive = false;
		toolButtonStartStop->setIcon(QIcon(QString::fromUtf8(":/data/img/refresh.png")));
		progressBar->setRange(0, 100);
		progressBar->setValue(0);
		progressBar->setFormat(tr("Idle"));
		updateTimer.stop();
		updateResults();
	} else {
		threadMap.clear();
		plainTextEditLog->clear();
		ImageWidget *imageWidget;
		switch ( comboBoxImageType->currentIndex() ) {
			case QMC2_IMGCHK_INDEX_PREVIEW:
				imageWidget = qmc2Preview;
				break;
			case QMC2_IMGCHK_INDEX_FLYER:
				imageWidget = qmc2Flyer;
				break;
			case QMC2_IMGCHK_INDEX_CABINET:
				imageWidget = qmc2Cabinet;
				break;
			case QMC2_IMGCHK_INDEX_CONTROLLER:
				imageWidget = qmc2Controller;
				break;
			case QMC2_IMGCHK_INDEX_MARQUEE:
				imageWidget = qmc2Marquee;
				break;
			case QMC2_IMGCHK_INDEX_TITLE:
				imageWidget = qmc2Title;
				break;
			case QMC2_IMGCHK_INDEX_PCB:
				imageWidget = qmc2PCB;
				break;
		}
		for (int t = 0; t < spinBoxThreads->value(); t++) {
			ImageCheckerThread *thread = new ImageCheckerThread(t, imageWidget, this);
			connect(thread, SIGNAL(log(const QString &)), this, SLOT(log(const QString &)));
			connect(thread, SIGNAL(resultsReady(const QStringList &, const QStringList &)), this, SLOT(resultsReady(const QStringList &, const QStringList &)));
			threadMap[t] = thread;
			thread->start();
		}
		isRunning = true;
		toolButtonStartStop->setIcon(QIcon(QString::fromUtf8(":/data/img/halt.png")));
		qmc2ImageCheckActive = true;
		listWidgetFound->clear();
		labelFound->setText(tr("Found:") + " 0");
		listWidgetMissing->clear();
		labelMissing->setText(tr("Missing:") + " 0");
		listWidgetObsolete->clear();
		labelObsolete->setText(tr("Obsolete:") + " 0");
		progressBar->setRange(0, qmc2GamelistItemMap.count());
		progressBar->setValue(0);
		progressBar->setFormat(tr("Pass #1"));
		bufferedFoundList.clear();
		bufferedMissingList.clear();
		QTimer::singleShot(0, this, SLOT(feedWorkerThreads()));
		updateTimer.start(QMC2_CHECK_UPDATE_FAST);
	}
}

void ImageChecker::feedWorkerThreads()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageChecker::feedWorkerThreads()");
#endif

	QMapIterator<QString, QTreeWidgetItem *> it(qmc2GamelistItemMap);
	int lastThreadID = -1;
#ifdef QMC2_DEBUG
	int count = 0;
#endif
	while ( it.hasNext() && qmc2ImageCheckActive && !qmc2StopParser ) {
		int selectedThread = -1;
		if ( threadMap.count() > 1 ) {
			for (int t = lastThreadID + 1; t < threadMap.count() && selectedThread == -1; t++)
				if ( !threadMap[t]->isActive )
					selectedThread = t;
			for (int t = 0; t < lastThreadID && selectedThread == -1; t++)
				if ( !threadMap[t]->isActive )
					selectedThread = t;
		} else
			selectedThread = 0;
		if ( selectedThread >= 0 ) {
			if ( threadMap[selectedThread]->workUnitMutex.tryLock() ) {
				QStringList workUnit;
				while ( it.hasNext() && qmc2ImageCheckActive && workUnit.count() < QMC2_IMGCHK_WORKUNIT_SIZE && !qmc2StopParser ) {
					it.next();
					workUnit << it.key();
				}
				threadMap[selectedThread]->workUnitMutex.unlock();
				if ( qmc2ImageCheckActive ) {
					threadMap[selectedThread]->workUnit += workUnit;
					threadMap[selectedThread]->waitCondition.wakeAll();
#ifdef QMC2_DEBUG
					count += workUnit.count();
					log(QString("DEBUG: ImageChecker::feedWorkerThreads(): count = %1").arg(count));
#endif
				}
			}
			lastThreadID = selectedThread;
		}
		qApp->processEvents();
	}
}

void ImageChecker::on_toolButtonRemoveObsolete_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageChecker::on_toolButtonRemoveObsolete_clicked()");
#endif

	// FIXME
}

void ImageChecker::log(const QString &message)
{
	QString msg = QTime::currentTime().toString("hh:mm:ss.zzz") + ": " + message;

	bool scrollBarMaximum = (plainTextEditLog->verticalScrollBar()->value() == plainTextEditLog->verticalScrollBar()->maximum());
	plainTextEditLog->appendPlainText(msg);
	if ( scrollBarMaximum ) {
		plainTextEditLog->update();
		qApp->processEvents();
		plainTextEditLog->verticalScrollBar()->setValue(plainTextEditLog->verticalScrollBar()->maximum());
	}
}

void ImageChecker::resultsReady(const QStringList &foundList, const QStringList &missingList)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageChecker::resultsReady(const QStringList &foundList = ..., const QStringList &missingList = ...)");
#endif

	bufferedFoundList += foundList;
	bufferedMissingList += missingList;
	progressBar->setValue(progressBar->value() + foundList.count() + missingList.count());
	qApp->processEvents();
}

void ImageChecker::updateResults()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageChecker::updateResults()");
#endif

	listWidgetFound->insertItems(listWidgetFound->count(), bufferedFoundList);
	listWidgetMissing->insertItems(listWidgetMissing->count(), bufferedMissingList);
	bufferedFoundList.clear();
	bufferedMissingList.clear();
	labelFound->setText(tr("Found:") + " " + QString::number(listWidgetFound->count()));
	labelMissing->setText(tr("Missing:") + " " + QString::number(listWidgetMissing->count()));
	labelObsolete->setText(tr("Obsolete:") + " " + QString::number(listWidgetObsolete->count()));
	qApp->processEvents();
	if ( listWidgetFound->count() + listWidgetMissing->count() >= qmc2GamelistItemMap.count() && isRunning )
		QTimer::singleShot(0, toolButtonStartStop, SLOT(animateClick()));
}

void ImageChecker::selectItem(QString setName)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ImageChecker::selectItem(QString setName = %1)").arg(setName));
#endif

	switch ( qmc2MainWindow->stackedWidgetView->currentIndex() ) {
		case QMC2_VIEWGAMELIST_INDEX: {
			QTreeWidgetItem *gameItem = qmc2GamelistItemMap[setName];
			if ( gameItem ) {
				qmc2MainWindow->treeWidgetGamelist->clearSelection();
				qmc2MainWindow->treeWidgetGamelist->setCurrentItem(gameItem);
				qmc2MainWindow->treeWidgetGamelist->scrollToItem(gameItem, qmc2CursorPositioningMode);
				gameItem->setSelected(true);
			}
			break;
		}

		case QMC2_VIEWHIERARCHY_INDEX: {
			QTreeWidgetItem *hierarchyItem = qmc2HierarchyItemMap[setName];
			if ( hierarchyItem ) {
				qmc2MainWindow->treeWidgetHierarchy->clearSelection();
				qmc2MainWindow->treeWidgetHierarchy->setCurrentItem(hierarchyItem);
				qmc2MainWindow->treeWidgetHierarchy->scrollToItem(hierarchyItem, qmc2CursorPositioningMode);
				hierarchyItem->setSelected(true);
			}
			break;
		}

		case QMC2_VIEWCATEGORY_INDEX: {
			QTreeWidgetItem *categoryItem = qmc2CategoryItemMap[setName];
			if ( categoryItem ) {
				qmc2MainWindow->treeWidgetCategoryView->clearSelection();
				qmc2MainWindow->treeWidgetCategoryView->setCurrentItem(categoryItem);
				qmc2MainWindow->treeWidgetCategoryView->scrollToItem(categoryItem, qmc2CursorPositioningMode);
				categoryItem->setSelected(true);
			}
			break;
		}

#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
		case QMC2_VIEWVERSION_INDEX: {
			QTreeWidgetItem *versionItem = qmc2VersionItemMap[setName];
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

void ImageChecker::closeEvent(QCloseEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ImageChecker::closeEvent(QCloseEvent *e = %1)").arg((qulonglong)e));
#endif

	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/ImageChecker/Geometry", saveGeometry());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ImageChecker/ImageType", comboBoxImageType->currentIndex());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ImageChecker/Threads", spinBoxThreads->value());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ImageChecker/SelectSets", toolButtonSelectSets->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ImageChecker/ClearCaches", toolButtonClearCaches->isChecked());

	if ( e )
		QDialog::closeEvent(e);
}

void ImageChecker::hideEvent(QHideEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ImageChecker::hideEvent(QHideEvent *e = %1)").arg((qulonglong)e));
#endif

	closeEvent(NULL);
	QDialog::hideEvent(e);
}

void ImageChecker::showEvent(QShowEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ImageChecker::showEvent(QShowEvent *e = %1)").arg((qulonglong)e));
#endif

	adjustIconSizes();

	restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/ImageChecker/Geometry", QByteArray()).toByteArray());
	comboBoxImageType->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ImageChecker/ImageType", QMC2_IMGCHK_INDEX_PREVIEW).toInt());
	spinBoxThreads->setValue(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ImageChecker/Threads", 1).toInt());
	toolButtonSelectSets->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ImageChecker/SelectSets", true).toBool());
	toolButtonClearCaches->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ImageChecker/ClearCaches", true).toBool());

	QDialog::showEvent(e);
}
