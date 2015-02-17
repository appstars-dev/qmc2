#include <QApplication>
#include <QTest>
#include <QFont>
#include <QFontInfo>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMap>
#include <QDateTime>
#include <QMessageBox>
#include <QScrollBar>

#include "collectionrebuilder.h"
#include "settings.h"
#include "options.h"
#include "unzip.h"
#include "zip.h"
#include "sevenzipfile.h"
#include "gamelist.h"
#include "macros.h"

extern Settings *qmc2Config;
extern Options *qmc2Options;
extern Gamelist *qmc2Gamelist;

CollectionRebuilder::CollectionRebuilder(ROMAlyzer *myROMAlyzer, QWidget *parent)
#if defined(QMC2_OS_WIN)
	: QDialog(parent, Qt::Dialog)
#else
	: QDialog(parent, Qt::Dialog | Qt::SubWindow)
#endif
{
	m_romAlyzer = myROMAlyzer;

	setupUi(this);

	switch ( romAlyzer()->mode() ) {
		case QMC2_ROMALYZER_MODE_SOFTWARE:
			setWindowTitle(tr("Software Collection Rebuilder"));
			m_settingsKey = "SoftwareCollectionRebuilder";
			m_defaultSetEntity = "software";
			m_defaultRomEntity = "rom";
			m_defaultDiskEntity = "disk";
			checkBoxFilterExpressionSoftwareLists->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/UseFilterExpressionSoftwareLists", false).toBool());
			comboBoxFilterSyntaxSoftwareLists->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterSyntaxSoftwareLists", 0).toInt());
			comboBoxFilterTypeSoftwareLists->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterTypeSoftwareLists", 0).toInt());
			lineEditFilterExpressionSoftwareLists->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterExpressionSoftwareLists", QString()).toString());
			checkBoxFilterExpression->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/UseFilterExpression", false).toBool());
			comboBoxFilterSyntax->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterSyntax", 0).toInt());
			comboBoxFilterType->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterType", 0).toInt());
			lineEditFilterExpression->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterExpression", QString()).toString());
			m_correctIconPixmap = QPixmap(QString::fromUtf8(":/data/img/software_correct.png"));
			m_mostlyCorrectIconPixmap = QPixmap(QString::fromUtf8(":/data/img/software_mostlycorrect.png"));
			m_incorrectIconPixmap = QPixmap(QString::fromUtf8(":/data/img/software_incorrect.png"));
			m_notFoundIconPixmap = QPixmap(QString::fromUtf8(":/data/img/software_notfound.png"));
			m_unknownIconPixmap = QPixmap(QString::fromUtf8(":/data/img/software_unknown.png"));
			hideStateFilter(); // FIXME: add software-state filtering
			break;
		case QMC2_ROMALYZER_MODE_SYSTEM:
		default:
			setWindowTitle(tr("ROM Collection Rebuilder"));
			m_settingsKey = "CollectionRebuilder";
#if defined(QMC2_EMUTYPE_MESS)
			m_defaultSetEntity = "machine";
#else
			m_defaultSetEntity = "game";
#endif
			m_defaultRomEntity = "rom";
			m_defaultDiskEntity = "disk";
			checkBoxFilterExpressionSoftwareLists->setVisible(false);
			comboBoxFilterSyntaxSoftwareLists->setVisible(false);
			comboBoxFilterTypeSoftwareLists->setVisible(false);
			lineEditFilterExpressionSoftwareLists->setVisible(false);
			toolButtonClearFilterExpressionSoftwareLists->setVisible(false);
			checkBoxFilterExpression->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/UseFilterExpression", false).toBool());
			comboBoxFilterSyntax->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterSyntax", 0).toInt());
			comboBoxFilterType->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterType", 0).toInt());
			lineEditFilterExpression->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterExpression", QString()).toString());
			m_correctIconPixmap = QPixmap(QString::fromUtf8(":/data/img/sphere_green.png"));
			m_mostlyCorrectIconPixmap = QPixmap(QString::fromUtf8(":/data/img/sphere_yellowgreen.png"));
			m_incorrectIconPixmap = QPixmap(QString::fromUtf8(":/data/img/sphere_red.png"));
			m_notFoundIconPixmap = QPixmap(QString::fromUtf8(":/data/img/sphere_grey.png"));
			m_unknownIconPixmap = QPixmap(QString::fromUtf8(":/data/img/sphere_blue.png"));
			break;
	}

	checkBoxFilterStates->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/UseStateFilter", false).toBool());
	toolButtonStateC->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/IncludeStateC", true).toBool());
	toolButtonStateM->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/IncludeStateM", true).toBool());
	toolButtonStateI->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/IncludeStateI", true).toBool());
	toolButtonStateN->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/IncludeStateN", true).toBool());
	toolButtonStateU->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/IncludeStateU", true).toBool());

	pushButtonPauseResume->setVisible(false);

	QFont logFont;
	logFont.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/LogFont").toString());
	plainTextEditLog->setFont(logFont);
	spinBoxMaxLogSize->setValue(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/MaxLogSize", 10000).toInt());
	adjustIconSizes();

	m_rebuilderThread = new CollectionRebuilderThread(this);
	connect(rebuilderThread(), SIGNAL(log(const QString &)), this, SLOT(log(const QString &)));
	connect(rebuilderThread(), SIGNAL(rebuildStarted()), this, SLOT(rebuilderThread_rebuildStarted()));
	connect(rebuilderThread(), SIGNAL(rebuildFinished()), this, SLOT(rebuilderThread_rebuildFinished()));
	connect(rebuilderThread(), SIGNAL(rebuildPaused()), this, SLOT(rebuilderThread_rebuildPaused()));
	connect(rebuilderThread(), SIGNAL(rebuildResumed()), this, SLOT(rebuilderThread_rebuildResumed()));
	connect(rebuilderThread(), SIGNAL(progressTextChanged(const QString &)), this, SLOT(rebuilderThread_progressTextChanged(const QString &)));
	connect(rebuilderThread(), SIGNAL(progressRangeChanged(int, int)), this, SLOT(rebuilderThread_progressRangeChanged(int, int)));
	connect(rebuilderThread(), SIGNAL(progressChanged(int)), this, SLOT(rebuilderThread_progressChanged(int)));
	connect(rebuilderThread(), SIGNAL(statusUpdated(quint64, quint64, quint64)), this, SLOT(rebuilderThread_statusUpdated(quint64, quint64, quint64)));
	connect(rebuilderThread(), SIGNAL(newMissing(QString, QString, QString, QString, QString, QString, QString)), this, SLOT(rebuilderThread_newMissing(QString, QString, QString, QString, QString, QString, QString)));

	m_missingDumpsViewer = NULL;

	m_iconCheckpoint = QIcon(QString::fromUtf8(":/data/img/checkpoint.png"));
	m_iconNoCheckpoint = QIcon(QString::fromUtf8(":/data/img/no_checkpoint.png"));

	m_animationSequence = 0;
	connect(&m_animationTimer, SIGNAL(timeout()), this, SLOT(animationTimer_timeout()));

	frameEntities->setVisible(false);
	toolButtonRemoveXmlSource->setVisible(false);
	comboBoxXmlSource->blockSignals(true);
	comboBoxXmlSource->clear();
	comboBoxXmlSource->insertItem(0, tr("Current default emulator"));
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/Checkpoint", -1).toLongLong() >= 0 ) {
		comboBoxXmlSource->setItemIcon(0, m_iconCheckpoint);
		comboBoxXmlSource->setItemData(0, true);
	} else {
		comboBoxXmlSource->setItemIcon(0, m_iconNoCheckpoint);
		comboBoxXmlSource->setItemData(0, false);
	}
	QStringList xmlSources = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSources", QStringList()).toStringList();
	QStringList setEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetEntities", QStringList()).toStringList();
	QStringList romEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/RomEntities", QStringList()).toStringList();
	QStringList diskEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/DiskEntities", QStringList()).toStringList();
	QStringList checkpointList = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceCheckpoints", QStringList()).toStringList();
	QStringList softwareCheckpointList = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceListCheckpoints", QStringList()).toStringList();
	bool softwareCheckpointListOk = romAlyzer()->mode() == QMC2_ROMALYZER_MODE_SOFTWARE ? (softwareCheckpointList.count() == xmlSources.count()) : true;
	QList<qint64> checkpoints;
	foreach (QString cp, checkpointList)
		checkpoints << cp.toLongLong();
	int index = 1;
	if ( xmlSources.count() > 0 && setEntities.count() == xmlSources.count() && romEntities.count() == xmlSources.count() && diskEntities.count() == xmlSources.count() && checkpointList.count() == xmlSources.count() && softwareCheckpointListOk ) {
		for (int i = 0; i < xmlSources.count(); i++) {
			QString xmlSource = xmlSources[i];
			QFileInfo fi(xmlSource);
			if ( fi.exists() && fi.isReadable() ) {
				comboBoxXmlSource->insertItem(index, xmlSource);
				if ( checkpoints[i] >= 0 ) {
					comboBoxXmlSource->setItemIcon(index, m_iconCheckpoint);
					comboBoxXmlSource->setItemData(index, true);
				} else {
					comboBoxXmlSource->setItemIcon(index, m_iconNoCheckpoint);
					comboBoxXmlSource->setItemData(index, false);
				}
				index++;
			} else {
				xmlSources.removeAt(i);
				setEntities.removeAt(i);
				romEntities.removeAt(i);
				diskEntities.removeAt(i);
				checkpointList.removeAt(i);
				checkpoints.removeAt(i);
				if ( romAlyzer()->mode() == QMC2_ROMALYZER_MODE_SOFTWARE )
					softwareCheckpointList.removeAt(i);
				i--;
			}
		}
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSources", xmlSources);
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetEntities", setEntities);
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/RomEntities", romEntities);
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/DiskEntities", diskEntities);
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceCheckpoints", checkpointList);
		if ( romAlyzer()->mode() == QMC2_ROMALYZER_MODE_SOFTWARE )
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceListCheckpoints", softwareCheckpointList);
	} else {
		qmc2Config->remove(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSources");
		qmc2Config->remove(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetEntities");
		qmc2Config->remove(QMC2_FRONTEND_PREFIX + m_settingsKey + "/RomEntities");
		qmc2Config->remove(QMC2_FRONTEND_PREFIX + m_settingsKey + "/DiskEntities");
		qmc2Config->remove(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceCheckpoints");
		qmc2Config->remove(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceListCheckpoints");
	}
	comboBoxXmlSource->insertSeparator(index);
	index++;
	comboBoxXmlSource->insertItem(index, tr("Select XML file..."));
	comboBoxXmlSource->setItemIcon(index, QIcon(QString::fromUtf8(":/data/img/fileopen.png")));
	comboBoxXmlSource->setCurrentIndex(0);
	comboBoxXmlSource->blockSignals(false);
	lineEditSetEntity->setText(m_defaultSetEntity);
	lineEditRomEntity->setText(m_defaultRomEntity);
	lineEditDiskEntity->setText(m_defaultDiskEntity);
	rebuilderThread_statusUpdated(0, 0, 0);
	comboBoxXmlSource->setFocus();
}

CollectionRebuilder::~CollectionRebuilder()
{
	if ( missingDumpsViewer() )
		delete missingDumpsViewer();
	if ( rebuilderThread() )
		delete rebuilderThread();
}

void CollectionRebuilder::on_spinBoxMaxLogSize_valueChanged(int value)
{
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/MaxLogSize", value);
	plainTextEditLog->setMaximumBlockCount(value);
}

void CollectionRebuilder::log(const QString &message)
{
	if ( checkBoxEnableLog->isChecked() )
		plainTextEditLog->appendPlainText(QDateTime::currentDateTime().toString("hh:mm:ss.zzz") + ": " + message);
}

void CollectionRebuilder::scrollToEnd()
{
	plainTextEditLog->horizontalScrollBar()->setValue(plainTextEditLog->horizontalScrollBar()->minimum());
	plainTextEditLog->verticalScrollBar()->setValue(plainTextEditLog->verticalScrollBar()->maximum());
}

void CollectionRebuilder::adjustIconSizes()
{
	QFont f;
	f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	QFontMetrics fm(f);
	QSize iconSize = QSize(fm.height() - 2, fm.height() - 2);

	pushButtonStartStop->setIconSize(iconSize);
	pushButtonPauseResume->setIconSize(iconSize);
	comboBoxXmlSource->setIconSize(iconSize);
	toolButtonRemoveXmlSource->setIconSize(iconSize);
	toolButtonViewMissingList->setIconSize(iconSize);
	toolButtonClearFilterExpression->setIconSize(iconSize);
	toolButtonClearFilterExpressionSoftwareLists->setIconSize(iconSize);

	QSize iconSizeMiddle = QSize(fm.height(), fm.height());
	toolButtonStateC->setIcon(QIcon(m_correctIconPixmap.scaled(iconSizeMiddle, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
	toolButtonStateM->setIcon(QIcon(m_mostlyCorrectIconPixmap.scaled(iconSizeMiddle, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
	toolButtonStateI->setIcon(QIcon(m_incorrectIconPixmap.scaled(iconSizeMiddle, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
	toolButtonStateN->setIcon(QIcon(m_notFoundIconPixmap.scaled(iconSizeMiddle, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
	toolButtonStateU->setIcon(QIcon(m_unknownIconPixmap.scaled(iconSizeMiddle, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
}

void CollectionRebuilder::on_pushButtonStartStop_clicked()
{
	pushButtonStartStop->setEnabled(false);
	pushButtonStartStop->update();
	pushButtonPauseResume->setEnabled(false);
	pushButtonPauseResume->update();
	qApp->processEvents();
	if ( rebuilderThread()->isActive )
		rebuilderThread()->stopRebuilding = true;
	else if ( rebuilderThread()->isWaiting ) {
		newMissingList().clear();
		if ( missingDumpsViewer() )
			missingDumpsViewer()->treeWidget->clear();
		if ( comboBoxXmlSource->itemData(comboBoxXmlSource->currentIndex()).toBool() ) {
			switch ( QMessageBox::question(this, tr("Confirm checkpoint restart"), tr("Restart from stored checkpoint?"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::No) ) {
				case QMessageBox::Yes: {
						qint64 cp = 0;
						int index = comboBoxXmlSource->currentIndex();
						if ( index == 0 )
							cp = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/Checkpoint", -1).toLongLong();
						else {
							index -= 1;
							QStringList checkpointList = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceCheckpoints", QStringList()).toStringList();
							QStringList softwareCheckpointList = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceListCheckpoints", QStringList()).toStringList();
							if ( index >= 0 && index < checkpointList.count() ) {
								cp = checkpointList[index].toLongLong();
								if ( romAlyzer()->mode() == QMC2_ROMALYZER_MODE_SOFTWARE )
									rebuilderThread()->setListCheckpoint(softwareCheckpointList[index], index);
							} else {
								cp = -1;
								if ( romAlyzer()->mode() == QMC2_ROMALYZER_MODE_SOFTWARE )
									rebuilderThread()->setListCheckpoint(QString(), index);
							}
						}
						rebuilderThread()->checkpointRestart(cp);
					}
					break;
				case QMessageBox::No:
					rebuilderThread()->setCheckpoint(-1, comboBoxXmlSource->currentIndex());
					if ( romAlyzer()->mode() == QMC2_ROMALYZER_MODE_SOFTWARE )
						rebuilderThread()->setListCheckpoint(QString(), comboBoxXmlSource->currentIndex());
					break;
				case QMessageBox::Cancel:
					pushButtonStartStop->setEnabled(true);
					pushButtonPauseResume->setEnabled(true);
					return;
			}
		} else {
			rebuilderThread()->setCheckpoint(-1, comboBoxXmlSource->currentIndex());
			if ( romAlyzer()->mode() == QMC2_ROMALYZER_MODE_SOFTWARE )
				rebuilderThread()->setListCheckpoint(QString(), comboBoxXmlSource->currentIndex());
		}
		if ( romAlyzer()->mode() == QMC2_ROMALYZER_MODE_SOFTWARE ) {
			if ( checkBoxFilterExpressionSoftwareLists->isChecked() && !lineEditFilterExpressionSoftwareLists->text().isEmpty() )
				rebuilderThread()->setFilterExpressionSoftware(lineEditFilterExpressionSoftwareLists->text(), comboBoxFilterSyntaxSoftwareLists->currentIndex(), comboBoxFilterTypeSoftwareLists->currentIndex());
			else
				rebuilderThread()->clearFilterExpressionSoftware();
		}
		if ( checkBoxFilterExpression->isChecked() && !lineEditFilterExpression->text().isEmpty() )
			rebuilderThread()->setFilterExpression(lineEditFilterExpression->text(), comboBoxFilterSyntax->currentIndex(), comboBoxFilterType->currentIndex());
		else
			rebuilderThread()->clearFilterExpression();
		if ( checkBoxFilterStates->isChecked() )
			rebuilderThread()->setStateFilter(true, toolButtonStateC->isChecked(), toolButtonStateM->isChecked(), toolButtonStateI->isChecked(), toolButtonStateN->isChecked(), toolButtonStateU->isChecked());
		else
			rebuilderThread()->clearStateFilter();
		rebuilderThread()->waitCondition.wakeAll();
	}
}

void CollectionRebuilder::on_pushButtonPauseResume_clicked()
{
	pushButtonPauseResume->setEnabled(false);
	if ( rebuilderThread()->isPaused )
		QTimer::singleShot(0, rebuilderThread(), SLOT(resume()));
	else
		QTimer::singleShot(0, rebuilderThread(), SLOT(pause()));
}

void CollectionRebuilder::setStateFilterVisibility(bool visible)
{
	checkBoxFilterStates->setVisible(visible);
	toolButtonStateC->setVisible(visible);
	toolButtonStateM->setVisible(visible);
	toolButtonStateI->setVisible(visible);
	toolButtonStateN->setVisible(visible);
	toolButtonStateU->setVisible(visible);
}

void CollectionRebuilder::on_comboBoxXmlSource_currentIndexChanged(int index)
{
	static int lastIndex = -1;

	if ( index == 0 ) {
		if ( lastIndex >= 0 ) {
			QStringList setEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetEntities", QStringList()).toStringList();
			QStringList romEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/RomEntities", QStringList()).toStringList();
			QStringList diskEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/DiskEntities", QStringList()).toStringList();
			if ( lastIndex < setEntities.count() ) {
				setEntities.replace(lastIndex, lineEditSetEntity->text());
				qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetEntities", setEntities);
			}
			if ( lastIndex < romEntities.count() ) {
				romEntities.replace(lastIndex, lineEditRomEntity->text());
				qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/RomEntities", romEntities);
			}
			if ( lastIndex < diskEntities.count() ) {
				diskEntities.replace(lastIndex, lineEditDiskEntity->text());
				qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/DiskEntities", diskEntities);
			}
		}
		lineEditSetEntity->setText(m_defaultSetEntity);
		lineEditRomEntity->setText(m_defaultRomEntity);
		lineEditDiskEntity->setText(m_defaultDiskEntity);
		frameEntities->setVisible(false);
		if ( romAlyzer()->mode() == QMC2_ROMALYZER_MODE_SYSTEM )
			showStateFilter(); // FIXME: add software-state filteirng
		QTimer::singleShot(0, this, SLOT(scrollToEnd()));
		toolButtonRemoveXmlSource->setVisible(false);
		lastIndex = -1;
	} else if ( index == comboBoxXmlSource->count() - 1 ) {
		QString xmlSource = QFileDialog::getOpenFileName(this, tr("Choose source XML file"), QString(), tr("Data and XML files (*.dat *.xml)") + ";;" + tr("Data files (*.dat)") + ";;" + tr("XML files (*.xml)") + ";;" + tr("All files (*)"), 0, qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
		if ( !xmlSource.isNull() ) {
			int foundAtIndex = comboBoxXmlSource->findText(xmlSource);
			if ( foundAtIndex < 0 ) {
				comboBoxXmlSource->blockSignals(true);
				QStringList xmlSources = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSources", QStringList()).toStringList();
				xmlSources << xmlSource;
				qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSources", xmlSources);
				QStringList setEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetEntities", QStringList()).toStringList();
				setEntities << m_defaultSetEntity;
				qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetEntities", setEntities);
				lineEditSetEntity->setText(m_defaultSetEntity);
				QStringList romEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/RomEntities", QStringList()).toStringList();
				romEntities << m_defaultRomEntity;
				qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/RomEntities", romEntities);
				lineEditRomEntity->setText(m_defaultRomEntity);
				QStringList diskEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/DiskEntities", QStringList()).toStringList();
				diskEntities << m_defaultDiskEntity;
				qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/DiskEntities", diskEntities);
				lineEditDiskEntity->setText(m_defaultDiskEntity);
				QStringList checkpointList = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceCheckpoints", QStringList()).toStringList();
				checkpointList << "-1";
				qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceCheckpoints", checkpointList);
				if ( romAlyzer()->mode() == QMC2_ROMALYZER_MODE_SOFTWARE ) {
					QStringList softwareCheckpointList = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceListCheckpoints", QStringList()).toStringList();
					softwareCheckpointList << QString();
					qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceListCheckpoints", softwareCheckpointList);
				}
				int insertIndex = comboBoxXmlSource->count() - 2;
				lastIndex = insertIndex - 1;
				comboBoxXmlSource->insertItem(insertIndex, xmlSource);
				comboBoxXmlSource->setItemIcon(insertIndex, m_iconNoCheckpoint);
				comboBoxXmlSource->setItemData(insertIndex, false);
				comboBoxXmlSource->setCurrentIndex(insertIndex);
				comboBoxXmlSource->blockSignals(false);
				frameEntities->setVisible(true);
				hideStateFilter();
				toolButtonRemoveXmlSource->setVisible(true);
				QTimer::singleShot(0, this, SLOT(scrollToEnd()));
			} else
				comboBoxXmlSource->setCurrentIndex(foundAtIndex);
		} else
			comboBoxXmlSource->setCurrentIndex(0);
		raise();
	} else {
		index -= 1;
		if ( index >= 0 ) {
			QStringList setEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetEntities", QStringList()).toStringList();
			QStringList romEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/RomEntities", QStringList()).toStringList();
			QStringList diskEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/DiskEntities", QStringList()).toStringList();
			if ( lastIndex >= 0 ) {
				if ( lastIndex < setEntities.count() ) {
					setEntities.replace(lastIndex, lineEditSetEntity->text());
					qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetEntities", setEntities);
				}
				if ( lastIndex < romEntities.count() ) {
					romEntities.replace(lastIndex, lineEditRomEntity->text());
					qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/RomEntities", romEntities);
				}
				if ( lastIndex < diskEntities.count() ) {
					diskEntities.replace(lastIndex, lineEditDiskEntity->text());
					qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/DiskEntities", diskEntities);
				}
			}
			lastIndex = index;
			lineEditSetEntity->setText(setEntities[index]);
			lineEditRomEntity->setText(romEntities[index]);
			lineEditDiskEntity->setText(diskEntities[index]);
			frameEntities->setVisible(true);
			hideStateFilter();
			toolButtonRemoveXmlSource->setVisible(true);
			QTimer::singleShot(0, this, SLOT(scrollToEnd()));
		}
	}
}

void CollectionRebuilder::on_toolButtonRemoveXmlSource_clicked()
{
	int index = comboBoxXmlSource->currentIndex() - 1;
	comboBoxXmlSource->setCurrentIndex(0);
	QStringList xmlSources = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSources", QStringList()).toStringList();
	xmlSources.removeAt(index);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSources", xmlSources);
	QStringList setEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetEntities", QStringList()).toStringList();
	setEntities.removeAt(index);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetEntities", setEntities);
	QStringList romEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/RomEntities", QStringList()).toStringList();
	romEntities.removeAt(index);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/RomEntities", romEntities);
	QStringList diskEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/DiskEntities", QStringList()).toStringList();
	diskEntities.removeAt(index);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/DiskEntities", diskEntities);
	QStringList checkpointList = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceCheckpoints", QStringList()).toStringList();
	checkpointList.removeAt(index);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceCheckpoints", checkpointList);
	if ( romAlyzer()->mode() == QMC2_ROMALYZER_MODE_SOFTWARE ) {
		QStringList softwareCheckpointList = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceListCheckpoints", QStringList()).toStringList();
		softwareCheckpointList.removeAt(index);
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceListCheckpoints", softwareCheckpointList);
	}
	comboBoxXmlSource->blockSignals(true);
	comboBoxXmlSource->removeItem(index + 1);
	comboBoxXmlSource->blockSignals(false);
}

void CollectionRebuilder::on_toolButtonViewMissingList_clicked()
{
	if ( missingDumpsViewer() ) {
		if ( missingDumpsViewer()->isVisible() )
			missingDumpsViewer()->hide();
		else
			missingDumpsViewer()->show();
	} else {
		m_missingDumpsViewer = new MissingDumpsViewer(0);
		if ( !newMissingList().isEmpty() && missingDumpsViewer() )
			QTimer::singleShot(0, this, SLOT(updateMissingList()));
		missingDumpsViewer()->show();
	}
}

void CollectionRebuilder::rebuilderThread_rebuildStarted()
{
	pushButtonStartStop->setIcon(QIcon(QString::fromUtf8(":/data/img/halt.png")));
	pushButtonStartStop->setText(tr("Stop rebuilding"));
	pushButtonPauseResume->setText(tr("Pause"));
	pushButtonPauseResume->show();
	pushButtonStartStop->setEnabled(true);
	pushButtonPauseResume->setEnabled(true);
	comboBoxXmlSource->setEnabled(false);
	comboBoxXmlSource->setItemIcon(comboBoxXmlSource->currentIndex(), m_iconCheckpoint);
	comboBoxXmlSource->setItemData(comboBoxXmlSource->currentIndex(), true);
	labelXmlSource->setEnabled(false);
	toolButtonRemoveXmlSource->setEnabled(false);
	checkBoxFilterExpression->setEnabled(false);
	checkBoxFilterExpressionSoftwareLists->setEnabled(false);
	comboBoxFilterSyntax->setEnabled(false);
	comboBoxFilterSyntaxSoftwareLists->setEnabled(false);
	comboBoxFilterType->setEnabled(false);
	comboBoxFilterTypeSoftwareLists->setEnabled(false);
	lineEditFilterExpression->setEnabled(false);
	lineEditFilterExpressionSoftwareLists->setEnabled(false);
	toolButtonClearFilterExpression->setEnabled(false);
	toolButtonClearFilterExpressionSoftwareLists->setEnabled(false);
	checkBoxFilterStates->setEnabled(false);
	toolButtonStateC->setEnabled(false);
	toolButtonStateM->setEnabled(false);
	toolButtonStateI->setEnabled(false);
	toolButtonStateN->setEnabled(false);
	toolButtonStateU->setEnabled(false);
	frameEntities->setEnabled(false);
	romAlyzer()->groupBoxCheckSumDatabase->setEnabled(false);
	switch ( romAlyzer()->mode() ) {
		case QMC2_ROMALYZER_MODE_SOFTWARE:
			romAlyzer()->pushButtonRomCollectionRebuilder->setText(tr("Rebuilding software collection..."));
			break;
		case QMC2_ROMALYZER_MODE_SYSTEM:
		default:
			romAlyzer()->pushButtonRomCollectionRebuilder->setText(tr("Rebuilding ROM collection..."));
			break;
	}
	m_animationSequence = 0;
	m_animationTimer.start(QMC2_ROMALYZER_REBUILD_ANIM_SPEED);
	qApp->processEvents();
}

void CollectionRebuilder::rebuilderThread_rebuildFinished()
{
	int index = comboBoxXmlSource->currentIndex();
	qint64 cp = rebuilderThread()->checkpoint();
	if ( index == 0 )
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/Checkpoint", cp);
	else {
		int xmlSourceIndex = index - 1;
		QStringList checkpointList = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceCheckpoints", QStringList()).toStringList();
		if ( xmlSourceIndex >= 0 && xmlSourceIndex < checkpointList.count() ) {
			checkpointList.replace(xmlSourceIndex, QString::number(cp));
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceCheckpoints", checkpointList);
			if ( romAlyzer()->mode() == QMC2_ROMALYZER_MODE_SOFTWARE ) {
				QStringList softwareCheckpointList = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceListCheckpoints", QStringList()).toStringList();
				if ( xmlSourceIndex < softwareCheckpointList.count() ) {
					softwareCheckpointList.replace(xmlSourceIndex, rebuilderThread()->currentListName());
					qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceListCheckpoints", softwareCheckpointList);
				}
			}
		}
	}
	if ( index < 0 )
		return;
	if ( cp >= 0 ) {
		comboBoxXmlSource->setItemIcon(index, m_iconCheckpoint);
		comboBoxXmlSource->setItemData(index, true);
	} else {
		comboBoxXmlSource->setItemIcon(index, m_iconNoCheckpoint);
		comboBoxXmlSource->setItemData(index, false);
	}
	pushButtonStartStop->setIcon(QIcon(QString::fromUtf8(":/data/img/refresh.png")));
	pushButtonStartStop->setText(tr("Start rebuilding"));
	pushButtonPauseResume->hide();
	pushButtonStartStop->setEnabled(true);
	pushButtonPauseResume->setEnabled(true);
	comboBoxXmlSource->setEnabled(true);
	labelXmlSource->setEnabled(true);
	toolButtonRemoveXmlSource->setEnabled(true);
	checkBoxFilterExpression->setEnabled(true);
	checkBoxFilterExpressionSoftwareLists->setEnabled(true);
	comboBoxFilterSyntax->setEnabled(checkBoxFilterExpression->isChecked());
	comboBoxFilterSyntaxSoftwareLists->setEnabled(checkBoxFilterExpressionSoftwareLists->isChecked());
	comboBoxFilterType->setEnabled(checkBoxFilterExpression->isChecked());
	comboBoxFilterTypeSoftwareLists->setEnabled(checkBoxFilterExpressionSoftwareLists->isChecked());
	lineEditFilterExpression->setEnabled(checkBoxFilterExpression->isChecked());
	lineEditFilterExpressionSoftwareLists->setEnabled(checkBoxFilterExpressionSoftwareLists->isChecked());
	toolButtonClearFilterExpression->setEnabled(checkBoxFilterExpression->isChecked());
	toolButtonClearFilterExpressionSoftwareLists->setEnabled(checkBoxFilterExpressionSoftwareLists->isChecked());
	checkBoxFilterStates->setEnabled(true);
	toolButtonStateC->setEnabled(checkBoxFilterStates->isChecked());
	toolButtonStateM->setEnabled(checkBoxFilterStates->isChecked());
	toolButtonStateI->setEnabled(checkBoxFilterStates->isChecked());
	toolButtonStateN->setEnabled(checkBoxFilterStates->isChecked());
	toolButtonStateU->setEnabled(checkBoxFilterStates->isChecked());
	frameEntities->setEnabled(true);
	romAlyzer()->groupBoxCheckSumDatabase->setEnabled(true);
	m_animationTimer.stop();
	romAlyzer()->pushButtonRomCollectionRebuilder->setIcon(QIcon(QString::fromUtf8(":/data/img/rebuild.png")));
	switch ( romAlyzer()->mode() ) {
		case QMC2_ROMALYZER_MODE_SOFTWARE:
			romAlyzer()->pushButtonRomCollectionRebuilder->setText(tr("Rebuild software collection..."));
			break;
		case QMC2_ROMALYZER_MODE_SYSTEM:
		default:
			romAlyzer()->pushButtonRomCollectionRebuilder->setText(tr("Rebuild ROM collection..."));
			break;
	}
	qApp->processEvents();
}

void CollectionRebuilder::rebuilderThread_rebuildPaused()
{
	pushButtonPauseResume->setText(tr("Resume"));
	pushButtonPauseResume->setEnabled(true);
}

void CollectionRebuilder::rebuilderThread_rebuildResumed()
{
	pushButtonPauseResume->setText(tr("Pause"));
	pushButtonPauseResume->setEnabled(true);
}

void CollectionRebuilder::rebuilderThread_progressTextChanged(const QString &text)
{
	progressBar->setFormat(text);
}

void CollectionRebuilder::rebuilderThread_progressRangeChanged(int min, int max)
{
	progressBar->setRange(min, max);
}

void CollectionRebuilder::rebuilderThread_progressChanged(int progress)
{
	progressBar->setValue(progress);
}

void CollectionRebuilder::rebuilderThread_statusUpdated(quint64 setsProcessed, quint64 missingDumps, quint64 missingDisks)
{
	QString statusString = "<center><table border=\"0\" cellpadding=\"0\" cellspacing=\"4\"><tr>";
	statusString += "<td nowrap width=\"16.5%\" align=\"left\"><b>" + tr("Sets processed") + ":</b></td><td nowrap width=\"16.5%\" align=\"right\">" + QString::number(setsProcessed) + "</td>";
	statusString += "<td nowrap align=\"center\">|</td>";
	statusString += "<td nowrap width=\"16.5%\" align=\"left\"><b>" + tr("Missing ROMs") + ":</b></td><td nowrap width=\"16.5%\" align=\"right\">" + QString::number(missingDumps) + "</td>";
	statusString += "<td nowrap align=\"center\">|</td>";
	statusString += "<td nowrap width=\"16.5%\" align=\"left\"><b>" + tr("Missing disks") + ":</b></td><td nowrap width=\"16.5%\" align=\"right\">" + QString::number(missingDisks) + "</td>";
	statusString += "<td nowrap align=\"right\">|</td>";
	statusString += "</tr></table></center>";
	labelRebuildStatus->setText(statusString);
}

void CollectionRebuilder::rebuilderThread_newMissing(QString id, QString type, QString name, QString size, QString crc, QString sha1, QString reason)
{
	newMissingList() << id + "|" + type + "|" + name + "|" + size + "|" + crc + "|" + sha1 + "|" + reason;
}

void CollectionRebuilder::animationTimer_timeout()
{
	m_animationTimer.stop();
	QPixmap pixmap(QString::fromUtf8(":/data/img/rebuild.png"));
	QPixmap rotatedPixmap(pixmap.size());
	rotatedPixmap.fill(Qt::transparent);
	QPainter p(&rotatedPixmap); 
	QSize size = pixmap.size();
	p.translate(size.height()/2,size.height()/2);
	p.rotate(24 * ++m_animationSequence);
	if ( m_animationSequence > 14 )
		m_animationSequence = 0;
	p.translate(-size.height()/2,-size.height()/2);
	p.drawPixmap(0, 0, pixmap);
	p.end();
	romAlyzer()->pushButtonRomCollectionRebuilder->setIcon(QIcon(rotatedPixmap));
	m_animationTimer.start(QMC2_ROMALYZER_REBUILD_ANIM_SPEED);
	if ( !newMissingList().isEmpty() && missingDumpsViewer() )
		QTimer::singleShot(0, this, SLOT(updateMissingList()));
}

void CollectionRebuilder::updateMissingList()
{
	QList<QTreeWidgetItem *> itemList;
	foreach (QString newMissing, newMissingList()) {
		QStringList missingWords = newMissing.split("|");
		if ( missingWords.count() >= 7 ) {
			QTreeWidgetItem *newItem = new QTreeWidgetItem(0);
			newItem->setText(QMC2_MDV_COLUMN_ID, missingWords[0]);
			newItem->setText(QMC2_MDV_COLUMN_TYPE, missingWords[1]);
			newItem->setText(QMC2_MDV_COLUMN_NAME, missingWords[2]);
			newItem->setText(QMC2_MDV_COLUMN_SIZE, missingWords[3]);
			newItem->setText(QMC2_MDV_COLUMN_CRC, missingWords[4]);
			newItem->setText(QMC2_MDV_COLUMN_SHA1, missingWords[5]);
			newItem->setText(QMC2_MDV_COLUMN_REASON, missingWords[6]);
			itemList << newItem;
		}
	}
	missingDumpsViewer()->treeWidget->insertTopLevelItems(0, itemList);
	newMissingList().clear();
}

void CollectionRebuilder::showEvent(QShowEvent *e)
{
	restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + QString("Layout/%1/Geometry").arg(m_settingsKey), QByteArray()).toByteArray());
	if ( e )
		QDialog::showEvent(e);
}

void CollectionRebuilder::hideEvent(QHideEvent *e)
{
	if ( isVisible() )
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + QString("Layout/%1/Geometry").arg(m_settingsKey), saveGeometry());
	if ( e )
		QDialog::hideEvent(e);

	if ( missingDumpsViewer() )
		missingDumpsViewer()->hide();
}

void CollectionRebuilder::closeEvent(QCloseEvent *e)
{
	hideEvent(0);
	QDialog::closeEvent(e);

	switch ( romAlyzer()->mode() ) {
		case QMC2_ROMALYZER_MODE_SOFTWARE:
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/UseFilterExpressionSoftwareLists", checkBoxFilterExpressionSoftwareLists->isChecked());
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterSyntaxSoftwareLists", comboBoxFilterSyntaxSoftwareLists->currentIndex());
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterTypeSoftwareLists", comboBoxFilterTypeSoftwareLists->currentIndex());
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterExpressionSoftwareLists", lineEditFilterExpressionSoftwareLists->text());
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/UseFilterExpression", checkBoxFilterExpression->isChecked());
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterSyntax", comboBoxFilterSyntax->currentIndex());
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterType", comboBoxFilterType->currentIndex());
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterExpression", lineEditFilterExpression->text());
			break;
		case QMC2_ROMALYZER_MODE_SYSTEM:
		default:
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/UseFilterExpression", checkBoxFilterExpression->isChecked());
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterSyntax", comboBoxFilterSyntax->currentIndex());
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterType", comboBoxFilterType->currentIndex());
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterExpression", lineEditFilterExpression->text());
			break;
	}

	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/UseStateFilter", checkBoxFilterStates->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/IncludeStateC", toolButtonStateC->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/IncludeStateM", toolButtonStateM->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/IncludeStateI", toolButtonStateI->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/IncludeStateN", toolButtonStateN->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/IncludeStateU", toolButtonStateU->isChecked());

	if ( missingDumpsViewer() )
		missingDumpsViewer()->close();
}

void CollectionRebuilder::keyPressEvent(QKeyEvent *e)
{
	if ( e->key() == Qt::Key_Escape )
		close();
	else
		QDialog::keyPressEvent(e);
}

CollectionRebuilderThread::CollectionRebuilderThread(QObject *parent)
	: QThread(parent)
{
	isActive = exitThread = isWaiting = isPaused = pauseRequested = stopRebuilding = doFilter = doFilterSoftware = isIncludeFilter = isIncludeFilterSoftware = doFilterState = false;
	includeStateC = includeStateM = includeStateI = includeStateN = includeStateU = true;
	m_rebuilderDialog = (CollectionRebuilder *)parent;
	m_checkSumDb = NULL;
	m_xmlIndex = m_xmlIndexCount = m_checkpoint = -1;
	reopenDatabase();
	switch ( rebuilderDialog()->romAlyzer()->mode() ) {
		case QMC2_ROMALYZER_MODE_SOFTWARE:
			m_swlDb = new SoftwareListXmlDatabaseManager(this);
			m_xmlDb = NULL;
			break;
		case QMC2_ROMALYZER_MODE_SYSTEM:
		default:
			m_xmlDb = new XmlDatabaseManager(this);
			m_swlDb = NULL;
			break;
	}
	start();
}

CollectionRebuilderThread::~CollectionRebuilderThread()
{
	exitThread = true;
	waitCondition.wakeAll();
	wait();
	if ( checkSumDb() ) {
		checkSumDb()->disconnect(rebuilderDialog());
		delete checkSumDb();
	}
	if ( xmlDb() )
		delete xmlDb();
	if ( swlDb() )
		delete swlDb();
}

void CollectionRebuilderThread::reopenDatabase()
{
	if ( checkSumDb() ) {
		checkSumDb()->disconnect(rebuilderDialog());
		delete checkSumDb();
	}
	m_checkSumDb = new CheckSumDatabaseManager(this, rebuilderDialog()->romAlyzer()->settingsKey());
	connect(checkSumDb(), SIGNAL(log(const QString &)), rebuilderDialog(), SLOT(log(const QString &)));
}

bool CollectionRebuilderThread::parseXml(QString xml, QString *id, QStringList *romNameList, QStringList *romSha1List, QStringList *romCrcList, QStringList *romSizeList, QStringList *diskNameList, QStringList *diskSha1List, QStringList *diskSizeList)
{
	if ( xml.isEmpty() )
		return false;

	QString setEntityPattern("<" + rebuilderDialog()->lineEditSetEntity->text() + " name=\"");
	QString romEntityPattern("<" + rebuilderDialog()->lineEditRomEntity->text() + " name=\"");
	QString diskEntityPattern("<" + rebuilderDialog()->lineEditDiskEntity->text() + " name=\"");
	bool merge = !rebuilderDialog()->romAlyzer()->checkBoxSetRewriterSelfContainedSets->isChecked();
	int startIndex = -1;
	int endIndex = -1;
	QStringList xmlLines = xml.split("\n");
	QString xmlLine = xmlLines[0];
	startIndex = xmlLine.indexOf(setEntityPattern);
	if ( startIndex >= 0 ) {
		startIndex += setEntityPattern.length();
		endIndex = xmlLine.indexOf("\"", startIndex);
		if ( endIndex >= 0 ) {
			*id = xmlLine.mid(startIndex, endIndex - startIndex);
			for (int i = 1; i < xmlLines.count(); i++) {
				xmlLine = xmlLines[i];
				bool romFound = false;
				startIndex = xmlLine.indexOf(romEntityPattern);
				if ( startIndex >= 0 ) {
					startIndex += romEntityPattern.length();
					endIndex = xmlLine.indexOf("\"", startIndex);
					if ( endIndex >= 0 ) {
						romFound = true;
						QString romName = xmlLine.mid(startIndex, endIndex - startIndex);
						QString mergeName;
						QString status;
						startIndex = xmlLine.indexOf("status=\"");
						if ( startIndex >= 0 ) {
							startIndex += 8;
							endIndex = xmlLine.indexOf("\"", startIndex);
							if ( endIndex >= 0 )
								status = xmlLine.mid(startIndex, endIndex - startIndex);
						}
						startIndex = xmlLine.indexOf("merge=\"");
						if ( startIndex >= 0 ) {
							startIndex += 7;
							endIndex = xmlLine.indexOf("\"", startIndex);
							if ( endIndex >= 0 )
								mergeName = xmlLine.mid(startIndex, endIndex - startIndex);
						}
						if ( status != "nodump" && (!merge || mergeName.isEmpty()) ) {
							QString romSha1, romCrc, romSize;
							startIndex = xmlLine.indexOf("sha1=\"");
							if ( startIndex >= 0 ) {
								startIndex += 6;
								endIndex = xmlLine.indexOf("\"", startIndex);
								if ( endIndex >= 0 )
									romSha1 = xmlLine.mid(startIndex, endIndex - startIndex);
							}
							startIndex = xmlLine.indexOf("crc=\"");
							if ( startIndex >= 0 ) {
								startIndex += 5;
								endIndex = xmlLine.indexOf("\"", startIndex);
								if ( endIndex >= 0 )
									romCrc = xmlLine.mid(startIndex, endIndex - startIndex);
							}
							startIndex = xmlLine.indexOf("size=\"");
							if ( startIndex >= 0 ) {
								startIndex += 6;
								endIndex = xmlLine.indexOf("\"", startIndex);
								if ( endIndex >= 0 )
									romSize = xmlLine.mid(startIndex, endIndex - startIndex);
							}
							if ( !romSha1.isEmpty() || !romCrc.isEmpty() ) {
								*romNameList << romName.replace("&amp;", "&").replace("&lt;", "<").replace("&gt;", ">").replace("&quot;", "\"").replace("&apos;", "'");
								*romSha1List << romSha1;
								*romCrcList << romCrc;
								*romSizeList << romSize;
							}
						}
					}
				}
				if ( romFound )
					continue;
				startIndex = xmlLine.indexOf(diskEntityPattern);
				if ( startIndex >= 0 ) {
					startIndex += diskEntityPattern.length();
					endIndex = xmlLine.indexOf("\"", startIndex);
					if ( endIndex >= 0 ) {
						QString diskName = xmlLine.mid(startIndex, endIndex - startIndex);
						QString mergeName;
						startIndex = xmlLine.indexOf("merge=\"");
						if ( startIndex >= 0 ) {
							startIndex += 7;
							endIndex = xmlLine.indexOf("\"", startIndex);
							if ( endIndex >= 0 )
								mergeName = xmlLine.mid(startIndex, endIndex - startIndex);
						}
						if ( !merge || mergeName.isEmpty() ) {
							QString diskSha1;
							startIndex = xmlLine.indexOf("sha1=\"");
							if ( startIndex >= 0 ) {
								startIndex += 6;
								endIndex = xmlLine.indexOf("\"", startIndex);
								if ( endIndex >= 0 )
									diskSha1 = xmlLine.mid(startIndex, endIndex - startIndex);
							}
							QString diskSize;
							startIndex = xmlLine.indexOf("size=\"");
							if ( startIndex >= 0 ) {
								startIndex += 6;
								endIndex = xmlLine.indexOf("\"", startIndex);
								if ( endIndex >= 0 )
									diskSize = xmlLine.mid(startIndex, endIndex - startIndex);
							}
							if ( !diskSha1.isEmpty() ) {
								*diskNameList << diskName.replace("&amp;", "&").replace("&lt;", "<").replace("&gt;", ">").replace("&quot;", "\"").replace("&apos;", "'");;
								*diskSha1List << diskSha1;
								*diskSizeList << diskSize;
							}
						}
					}
				}
			}
			return true;
		} else
			return false;
	} else
		return false;
}

bool CollectionRebuilderThread::nextId(QString *id, QStringList *romNameList, QStringList *romSha1List, QStringList *romCrcList, QStringList *romSizeList, QStringList *diskNameList, QStringList *diskSha1List, QStringList *diskSizeList)
{
	id->clear();
	romNameList->clear();
	romSha1List->clear();
	romCrcList->clear();
	romSizeList->clear();
	diskNameList->clear();
	diskSha1List->clear();
	diskSizeList->clear();
	if ( m_xmlIndex < 0 || m_xmlIndexCount < 0 ) {
		if ( rebuilderDialog()->comboBoxXmlSource->currentIndex() == 0 ) {
			m_xmlIndex = 1;
			switch ( rebuilderDialog()->romAlyzer()->mode() ) {
				case QMC2_ROMALYZER_MODE_SOFTWARE:
					m_xmlIndexCount = swlDb()->swlRowCount();
					break;
				case QMC2_ROMALYZER_MODE_SYSTEM:
				default:
					m_xmlIndexCount = xmlDb()->xmlRowCount();
					break;
			}
			emit progressRangeChanged(m_xmlIndex, m_xmlIndexCount);
			emit progressChanged(m_xmlIndex);
			setCheckpoint(m_xmlIndex, rebuilderDialog()->comboBoxXmlSource->currentIndex());
			return true;
		} else {
			m_xmlFile.setFileName(rebuilderDialog()->comboBoxXmlSource->currentText());
			if ( m_xmlFile.open(QIODevice::ReadOnly | QIODevice::Text) ) {
				m_xmlIndex = 0;
				m_xmlIndexCount = m_xmlFile.size() - 1;
				emit progressRangeChanged(m_xmlIndex, m_xmlIndexCount);
				emit progressChanged(m_xmlIndex);
				setCheckpoint(0, rebuilderDialog()->comboBoxXmlSource->currentIndex());
				return true;
			} else {
				emit log(tr("FATAL: can't open XML file '%1' for reading, please check permissions").arg(rebuilderDialog()->comboBoxXmlSource->currentText()));
				setCheckpoint(-1, rebuilderDialog()->comboBoxXmlSource->currentIndex());
				return false;
			}
		}
	} else {
		if ( m_xmlIndex > m_xmlIndexCount ) {
			emit progressChanged(m_xmlIndexCount);
			setCheckpoint(-1, rebuilderDialog()->comboBoxXmlSource->currentIndex());
			return false;
		}
		if ( rebuilderDialog()->comboBoxXmlSource->currentIndex() == 0 ) {
			switch ( rebuilderDialog()->romAlyzer()->mode() ) {
				case QMC2_ROMALYZER_MODE_SOFTWARE:
					if ( parseXml(swlDb()->xml(m_xmlIndex), id, romNameList, romSha1List, romCrcList, romSizeList, diskNameList, diskSha1List, diskSizeList) ) {
						id->prepend(swlDb()->list(m_xmlIndex) + ":");
						setCheckpoint(m_xmlIndex, rebuilderDialog()->comboBoxXmlSource->currentIndex());
						m_xmlIndex++;
						emit progressChanged(m_xmlIndex);
						return true;
					} else {
						emit log(tr("FATAL: XML parsing failed"));
						setCheckpoint(-1, rebuilderDialog()->comboBoxXmlSource->currentIndex());
						return false;
					}
				case QMC2_ROMALYZER_MODE_SYSTEM:
				default:
					if ( parseXml(xmlDb()->xml(m_xmlIndex), id, romNameList, romSha1List, romCrcList, romSizeList, diskNameList, diskSha1List, diskSizeList) ) {
						setCheckpoint(m_xmlIndex, rebuilderDialog()->comboBoxXmlSource->currentIndex());
						m_xmlIndex++;
						emit progressChanged(m_xmlIndex);
						return true;
					} else {
						emit log(tr("FATAL: XML parsing failed"));
						setCheckpoint(-1, rebuilderDialog()->comboBoxXmlSource->currentIndex());
						return false;
					}
			}
		} else {
			QString listEntityStartPattern("<softwarelist name=\"");
			QString setEntityStartPattern("<" + rebuilderDialog()->lineEditSetEntity->text() + " name=\"");
			QByteArray line = m_xmlFile.readLine();
			while ( !m_xmlFile.atEnd() && line.indexOf(setEntityStartPattern) < 0 && (rebuilderDialog()->romAlyzer()->mode() == QMC2_ROMALYZER_MODE_SOFTWARE ? line.indexOf(listEntityStartPattern) < 0 : true) && !exitThread )
				line = m_xmlFile.readLine();
			if ( m_xmlFile.atEnd() ) {
				emit progressChanged(m_xmlIndexCount);
				setCheckpoint(-1, rebuilderDialog()->comboBoxXmlSource->currentIndex());
				return false;
			} else if ( !exitThread ) {
				QString xmlString;
				if ( rebuilderDialog()->romAlyzer()->mode() == QMC2_ROMALYZER_MODE_SOFTWARE ) {
					int startIndex = line.indexOf(listEntityStartPattern);
					if ( startIndex >= 0 ) {
						startIndex += listEntityStartPattern.length();
						setListCheckpoint(line.mid(startIndex, line.indexOf("\"", startIndex) - startIndex), rebuilderDialog()->comboBoxXmlSource->currentIndex());
						return true;
					}
				}
				QString setEntityEndPattern("</" + rebuilderDialog()->lineEditSetEntity->text() + ">");
				while ( !m_xmlFile.atEnd() && line.indexOf(setEntityEndPattern) < 0 && !exitThread ) {
					xmlString += line;
					line = m_xmlFile.readLine();
				}
				if ( !m_xmlFile.atEnd() && !exitThread ) {
					xmlString += line;
					if ( parseXml(xmlString, id, romNameList, romSha1List, romCrcList, romSizeList, diskNameList, diskSha1List, diskSizeList) ) {
						if ( rebuilderDialog()->romAlyzer()->mode() == QMC2_ROMALYZER_MODE_SOFTWARE && !m_currentListName.isEmpty() )
							id->prepend(m_currentListName + ":");
						setCheckpoint(m_xmlIndex, rebuilderDialog()->comboBoxXmlSource->currentIndex());
						m_xmlIndex = m_xmlFile.pos();
						emit progressChanged(m_xmlIndex);
						return true;
					} else {
						emit log(tr("FATAL: XML parsing failed"));
						setCheckpoint(-1, rebuilderDialog()->comboBoxXmlSource->currentIndex());
						return false;
					}
				} else {
					emit log(tr("FATAL: XML parsing failed"));
					setCheckpoint(-1, rebuilderDialog()->comboBoxXmlSource->currentIndex());
					return false;
				}
			} else {
				setCheckpoint(-1, rebuilderDialog()->comboBoxXmlSource->currentIndex());
				return false;
			}
		}
	}
}

void CollectionRebuilderThread::setCheckpoint(qint64 cp, int xmlSourceIndex)
{
	m_checkpoint = cp;
	if ( xmlSourceIndex == 0 )
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + rebuilderDialog()->settingsKey() + "/Checkpoint", checkpoint());
	else if ( xmlSourceIndex > 0 && xmlSourceIndex < rebuilderDialog()->comboBoxXmlSource->count() - 1 ) {
		xmlSourceIndex -= 1;
		QStringList checkpointList = qmc2Config->value(QMC2_FRONTEND_PREFIX + rebuilderDialog()->settingsKey() + "/XmlSourceCheckpoints", QStringList()).toStringList();
		checkpointList.replace(xmlSourceIndex, QString::number(checkpoint()));
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + rebuilderDialog()->settingsKey() + "/XmlSourceCheckpoints", checkpointList);
	}
}

void CollectionRebuilderThread::setListCheckpoint(QString list, int xmlSourceIndex)
{
	m_currentListName = list;
	if ( xmlSourceIndex > 0 && xmlSourceIndex < rebuilderDialog()->comboBoxXmlSource->count() - 1 ) {
		xmlSourceIndex -= 1;
		QStringList checkpointList = qmc2Config->value(QMC2_FRONTEND_PREFIX + rebuilderDialog()->settingsKey() + "/XmlSourceListCheckpoints", QStringList()).toStringList();
		checkpointList.replace(xmlSourceIndex, list);
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + rebuilderDialog()->settingsKey() + "/XmlSourceListCheckpoints", checkpointList);
	}
}

void CollectionRebuilderThread::checkpointRestart(qint64 cp)
{
	if ( cp < 0 ) {
		m_xmlIndex = m_xmlIndexCount = -1;
		return;
	}
	m_xmlIndex = cp;
	if ( rebuilderDialog()->comboBoxXmlSource->currentIndex() == 0 ) {
		emit log(tr("restarting from checkpoint '%1'").arg(m_xmlIndex));
		switch ( rebuilderDialog()->romAlyzer()->mode() ) {
			case QMC2_ROMALYZER_MODE_SOFTWARE:
				m_xmlIndexCount = swlDb()->swlRowCount();
				break;
			case QMC2_ROMALYZER_MODE_SYSTEM:
			default:
				m_xmlIndexCount = xmlDb()->xmlRowCount();
				break;
		}
		emit progressRangeChanged(m_xmlIndex, m_xmlIndexCount);
		emit progressChanged(m_xmlIndex);
	} else {
		if ( m_xmlFile.isOpen() )
			m_xmlFile.close();
		m_xmlFile.setFileName(rebuilderDialog()->comboBoxXmlSource->currentText());
		if ( m_xmlFile.open(QIODevice::ReadOnly | QIODevice::Text) ) {
			emit log(tr("restarting from checkpoint '%1'").arg(m_xmlIndex));
			m_xmlIndexCount = m_xmlFile.size() - 1;
			m_xmlFile.seek(m_xmlIndex);
			emit progressRangeChanged(m_xmlIndex, m_xmlIndexCount);
			emit progressChanged(m_xmlIndex);
		} else {
			emit log(tr("FATAL: can't open XML file '%1' for reading, please check permissions").arg(rebuilderDialog()->comboBoxXmlSource->currentText()));
			m_xmlIndex = m_xmlIndexCount = -1;
		}
	}
	setCheckpoint(m_xmlIndex, rebuilderDialog()->comboBoxXmlSource->currentIndex());
}

bool CollectionRebuilderThread::rewriteSet(QString *setKey, QStringList *romNameList, QStringList *romSha1List, QStringList *romCrcList, QStringList *romSizeList, QStringList *diskNameList, QStringList *diskSha1List)
{
	QString set, baseDir = rebuilderDialog()->romAlyzer()->lineEditSetRewriterOutputPath->text();
	if ( rebuilderDialog()->romAlyzer()->mode() == QMC2_ROMALYZER_MODE_SOFTWARE ) {
		QStringList setKeyTokens = setKey->split(":", QString::SkipEmptyParts);
		if ( setKeyTokens.count() < 2 )
			return false;
		else {
			baseDir += "/" + setKeyTokens[0];
			set = setKeyTokens[1];
		}
	} else
		set = *setKey;
	if ( rebuilderDialog()->romAlyzer()->radioButtonSetRewriterZipArchives->isChecked() )
		return writeAllZipData(baseDir, set, romNameList, romSha1List, romCrcList, romSizeList, diskNameList, diskSha1List);
	else
		return writeAllFileData(baseDir, set, romNameList, romSha1List, romCrcList, romSizeList, diskNameList, diskSha1List);
}

bool CollectionRebuilderThread::writeAllFileData(QString baseDir, QString id, QStringList *romNameList, QStringList *romSha1List, QStringList *romCrcList, QStringList *romSizeList, QStringList * /*diskNameList*/, QStringList * /*diskSha1List*/)
{
	// FIXME: no support for disks
	bool success = true;
	QDir d(QDir::cleanPath(baseDir + "/" + id));
	if ( !d.exists() )
		success = d.mkdir(QDir::cleanPath(baseDir + "/" + id));
	int reproducedDumps = 0;
	bool ignoreErrors = !rebuilderDialog()->romAlyzer()->checkBoxSetRewriterAbortOnError->isChecked();
	for (int i = 0; i < romNameList->count() && !exitThread && success; i++) {
		QString fileName = d.absoluteFilePath(romNameList->at(i));
		if ( !createBackup(fileName) ) {
			emit log(tr("FATAL: backup creation failed"));
			success = false;
		}
		QFile f(fileName);
		QString errorReason = tr("file error");
		if ( success && f.open(QIODevice::WriteOnly) ) {
			QByteArray data;
			quint64 size = romSizeList->at(i).toULongLong();
			QString path, member, type;
			if ( checkSumDb()->getData(romSha1List->at(i), romCrcList->at(i), &size, &path, &member, &type) ) {
				if ( type == "ZIP" )
					success = readZipFileData(path, romCrcList->at(i), &data);
				else if ( type == "7Z" )
					success = readSevenZipFileData(path, romCrcList->at(i), &data);
				else if ( type == "FILE" )
					success = readFileData(path, &data);
				else {
					success = false;
					errorReason = tr("unknown file type '%1'").arg(type);
				}
				if ( success ) {
					emit log(tr("writing '%1' (size: %2)").arg(fileName).arg(ROMAlyzer::humanReadable(data.length())));
					quint64 bytesWritten = 0;
					while ( bytesWritten < (quint64)data.length() && !exitThread && success ) {
						quint64 bufferLength = QMC2_ZIP_BUFFER_SIZE;
						if ( bytesWritten + bufferLength > (quint64)data.length() )
							bufferLength = data.length() - bytesWritten;
						qint64 len = f.write(data.mid(bytesWritten, bufferLength));
						success = (len >= 0);
						if ( success ) {
							bytesWritten += len;
						} else
							log(tr("FATAL: failed writing '%1'").arg(fileName));
					}
				}
				f.close();
				reproducedDumps++;
			} else {
				f.close();
				f.remove();
			}
		} else {
			emit log(tr("FATAL: failed opening '%1' for writing"));
			success = false;
		}
		if ( !success ) {
			emit statusUpdated(setsProcessed, ++missingROMs, missingDisks);
			emit newMissing(id, tr("ROM"), romNameList->at(i), romSizeList->at(i), romCrcList->at(i), romSha1List->at(i), errorReason);
		}
		if ( ignoreErrors )
			success = true;
	}
	if ( reproducedDumps == 0 )
		d.rmdir(d.absolutePath());
	return success;
}

bool CollectionRebuilderThread::writeAllZipData(QString baseDir, QString id, QStringList *romNameList, QStringList *romSha1List, QStringList *romCrcList, QStringList *romSizeList, QStringList * /*diskNameList*/, QStringList * /*diskSha1List*/)
{
	// FIXME: no support for disks
	QDir d(QDir::cleanPath(baseDir));
	if ( !d.exists() )
		if ( !d.mkdir(QDir::cleanPath(baseDir)) )
			return false;
	QString fileName = QDir::cleanPath(baseDir) + "/" + id + ".zip";
	if ( !createBackup(fileName) ) {
		emit log(tr("FATAL: backup creation failed"));
		return false;
	}
	QFile f(fileName);
	if ( f.exists() )
		if ( !f.remove() )
			return false;
	bool success = true;
	bool uniqueCRCs = rebuilderDialog()->romAlyzer()->checkBoxSetRewriterUniqueCRCs->isChecked();
	bool ignoreErrors = !rebuilderDialog()->romAlyzer()->checkBoxSetRewriterAbortOnError->isChecked();
	int zipLevel = rebuilderDialog()->romAlyzer()->spinBoxSetRewriterZipLevel->value();
	zipFile zip = zipOpen(fileName.toLocal8Bit().constData(), APPEND_STATUS_CREATE);
	if ( zip ) {
		emit log(tr("creating new ZIP archive '%1'").arg(fileName));
		zip_fileinfo zipInfo;
		QDateTime cDT = QDateTime::currentDateTime();
		zipInfo.tmz_date.tm_sec = cDT.time().second();
		zipInfo.tmz_date.tm_min = cDT.time().minute();
		zipInfo.tmz_date.tm_hour = cDT.time().hour();
		zipInfo.tmz_date.tm_mday = cDT.date().day();
		zipInfo.tmz_date.tm_mon = cDT.date().month() - 1;
		zipInfo.tmz_date.tm_year = cDT.date().year();
		zipInfo.dosDate = zipInfo.internal_fa = zipInfo.external_fa = 0;
		QStringList storedCRCs;
		int reproducedDumps = 0;
		for (int i = 0; i < romNameList->count() && !exitThread && success; i++) {
			if ( uniqueCRCs && storedCRCs.contains(romCrcList->at(i)) ) {
				emit log(tr("skipping '%1'").arg(romNameList->at(i)) + " ("+ tr("a dump with CRC '%1' already exists").arg(romCrcList->at(i)) + ")");
				continue;
			}
			QString file = romNameList->at(i);
			QByteArray data;
			quint64 size = romSizeList->at(i).toULongLong();
			QString path, member, type;
			QString errorReason = tr("file error");
			if ( checkSumDb()->getData(romSha1List->at(i), romCrcList->at(i), &size, &path, &member, &type) ) {
				if ( type == "ZIP" )
					success = readZipFileData(path, romCrcList->at(i), &data);
				else if ( type == "7Z" )
					success = readSevenZipFileData(path, romCrcList->at(i), &data);
				else if ( type == "FILE" )
					success = readFileData(path, &data);
				else {
					success = false;
					errorReason = tr("unknown file type '%1'").arg(type);
				}
				if ( success && zipOpenNewFileInZip(zip, file.toLocal8Bit().constData(), &zipInfo, (const void *)file.toLocal8Bit().constData(), file.length(), 0, 0, 0, Z_DEFLATED, zipLevel) == ZIP_OK ) {
					emit log(tr("writing '%1' to ZIP archive '%2' (uncompressed size: %3)").arg(file).arg(fileName).arg(ROMAlyzer::humanReadable(data.length())));
					quint64 bytesWritten = 0;
					while ( bytesWritten < (quint64)data.length() && !exitThread && success ) {
						quint64 bufferLength = QMC2_ZIP_BUFFER_SIZE;
						if ( bytesWritten + bufferLength > (quint64)data.length() )
							bufferLength = data.length() - bytesWritten;
						QByteArray writeBuffer = data.mid(bytesWritten, bufferLength);
						success = (zipWriteInFileInZip(zip, (const void *)writeBuffer.data(), bufferLength) == ZIP_OK);
						if ( success )
							bytesWritten += bufferLength;
						else {
							emit log(tr("FATAL: failed writing '%1' to ZIP archive '%2'").arg(file).arg(fileName));
							success = false;
						}
					}
					storedCRCs << romCrcList->at(i);
					zipCloseFileInZip(zip);
					if ( success )
						reproducedDumps++;
				}
			}
			if ( !success ) {
				emit statusUpdated(setsProcessed, ++missingROMs, missingDisks);
				emit newMissing(id, tr("ROM"), romNameList->at(i), romSizeList->at(i), romCrcList->at(i), romSha1List->at(i), errorReason);
			}
			if ( ignoreErrors )
				success = true;
		}
		if ( rebuilderDialog()->romAlyzer()->checkBoxAddZipComment->isChecked() )
			zipClose(zip, tr("Created by QMC2 v%1 (%2)").arg(XSTR(QMC2_VERSION)).arg(cDT.toString(Qt::SystemLocaleShortDate)).toLocal8Bit().constData());
		else
			zipClose(zip, "");
		if ( reproducedDumps == 0 )
			f.remove();
		emit log(tr("done (creating new ZIP archive '%1')").arg(fileName));
	} else {
		emit log(tr("FATAL: failed creating ZIP archive '%1'").arg(fileName));
		success = false;
	}
	return success;
}

bool CollectionRebuilderThread::readFileData(QString fileName, QByteArray *data)
{
	QFile file(fileName);
	data->clear();
	if ( file.open(QIODevice::ReadOnly) ) {
  		char ioBuffer[QMC2_ROMALYZER_FILE_BUFFER_SIZE];
		int len = 0;
		emit log(tr("reading '%1' (size: %2)").arg(fileName).arg(ROMAlyzer::humanReadable(file.size())));
		while ( (len = file.read(ioBuffer, QMC2_FILE_BUFFER_SIZE)) > 0 && !exitThread )
			data->append(QByteArray((const char *)ioBuffer, len));
		file.close();
		return true;
	} else {
		emit log(tr("FATAL: failed reading '%1'").arg(fileName));
		return false;
	}
}

bool CollectionRebuilderThread::readSevenZipFileData(QString fileName, QString crc, QByteArray *data)
{
	SevenZipFile sevenZipFile(fileName);
	if ( sevenZipFile.open() ) {
		int index = sevenZipFile.indexOfCrc(crc);
		if ( index >= 0 ) {
			SevenZipMetaData metaData = sevenZipFile.itemList()[index];
			emit log(tr("reading '%1' from 7Z archive '%2' (uncompressed size: %3)").arg(metaData.name()).arg(fileName).arg(ROMAlyzer::humanReadable(metaData.size())));
			quint64 readLength = sevenZipFile.read(index, data); // can't be interrupted!
			if ( sevenZipFile.hasError() ) {
				emit log(tr("FATAL: failed reading '%1' from 7Z archive '%2'").arg(metaData.name()).arg(fileName));
				return false;
			}
			if ( readLength != metaData.size() ) {
				emit log(tr("FATAL: failed reading '%1' from 7Z archive '%2'").arg(metaData.name()).arg(fileName));
				return false;
			}
			return true;
		} else {
			emit log(tr("FATAL: failed reading from 7Z archive '%1'").arg(fileName));
			return false;
		}
	} else {
		emit log(tr("FATAL: failed reading from 7Z archive '%1'").arg(fileName));
		return false;
	}
}

bool CollectionRebuilderThread::readZipFileData(QString fileName, QString crc, QByteArray *data)
{
	bool success = true;
	unzFile zipFile = unzOpen(fileName.toLocal8Bit().constData());
	if ( zipFile ) {
  		char ioBuffer[QMC2_ZIP_BUFFER_SIZE];
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
			if ( unzLocateFile(zipFile, fn.toLocal8Bit().constData(), 2) == UNZ_OK ) {
				if ( unzOpenCurrentFile(zipFile) == UNZ_OK ) {
					emit log(tr("reading '%1' from ZIP archive '%2' (uncompressed size: %3)").arg(fn).arg(fileName).arg(ROMAlyzer::humanReadable(zipInfo.uncompressed_size)));
					qint64 len;
					while ( (len = unzReadCurrentFile(zipFile, ioBuffer, QMC2_ROMALYZER_ZIP_BUFFER_SIZE)) > 0 && !exitThread )
						data->append(QByteArray((const char *)ioBuffer, len));
					unzCloseCurrentFile(zipFile);
				} else {
					emit log(tr("FATAL: failed reading '%1' from ZIP archive '%2'").arg(fn).arg(fileName));
					success = false;
				}
			} else {
				emit log(tr("FATAL: failed reading '%1' from ZIP archive '%2'").arg(fn).arg(fileName));
				success = false;
			}
		} else {
			emit log(tr("FATAL: CRC '%1' not found in ZIP archive '%2'").arg(crc).arg(fileName));
			success = false;
		}
		unzClose(zipFile);
	} else {
		emit log(tr("FATAL: failed reading from ZIP archive '%1'").arg(fileName));
		success = false;
	}
	return success;
}

bool CollectionRebuilderThread::createBackup(QString filePath)
{
	if ( !rebuilderDialog()->romAlyzer()->checkBoxCreateBackups->isChecked() || rebuilderDialog()->romAlyzer()->lineEditBackupFolder->text().isEmpty() )
		return true;
	QFile sourceFile(filePath);
	if ( !sourceFile.exists() )
		return true;
	QDir backupDir(rebuilderDialog()->romAlyzer()->lineEditBackupFolder->text());
	QFileInfo backupDirInfo(backupDir.absolutePath());
	if ( backupDirInfo.exists() ) {
		if ( backupDirInfo.isWritable() ) {
#if defined(QMC2_OS_WIN)
			QString filePathCopy = filePath;
			QString destinationPath = QDir::cleanPath(QString(backupDir.absolutePath() + "/" + filePathCopy.replace(":", "")));
#else
			QString destinationPath = QDir::cleanPath(backupDir.absolutePath() + "/" + filePath);
#endif
			QFileInfo destinationPathInfo(destinationPath);
			if ( !destinationPathInfo.dir().exists() ) {
				if ( !backupDir.mkpath(destinationPathInfo.dir().absolutePath()) ) {
					emit log(tr("backup") + ": " + tr("FATAL: target path '%1' cannot be created").arg(destinationPathInfo.dir().absolutePath()));
					return false;
				}
			}
			if ( !sourceFile.open(QIODevice::ReadOnly) ) {
				emit log(tr("backup") + ": " + tr("FATAL: source file '%1' cannot be opened for reading").arg(filePath));
				return false;
			}
			QFile destinationFile(destinationPath);
			if ( destinationFile.open(QIODevice::WriteOnly) ) {
				emit log(tr("backup") + ": " + tr("creating backup copy of '%1' as '%2'").arg(filePath).arg(destinationPath));
				char ioBuffer[QMC2_ROMALYZER_FILE_BUFFER_SIZE];
				int count = 0;
				int len = 0;
				bool success = true;
				while ( success && (len = sourceFile.read(ioBuffer, QMC2_ROMALYZER_FILE_BUFFER_SIZE)) > 0 ) {
					if ( count++ % QMC2_BACKUP_IO_RESPONSE == 0 )
						qApp->processEvents();
					if ( destinationFile.write(ioBuffer, len) != len ) {
						emit log(tr("backup") + ": " + tr("FATAL: I/O error while writing to '%1'").arg(destinationPath));
						success = false;
					}
				}
				sourceFile.close();
				destinationFile.close();
				if ( success ) {
					emit log(tr("backup") + ": " + tr("done (creating backup copy of '%1' as '%2')").arg(filePath).arg(destinationPath));
					return true;
				} else
					return false;
			} else {
				emit log(tr("backup") + ": " + tr("FATAL: destination file '%1' cannot be opened for writing").arg(destinationPath));
				sourceFile.close();
				return false;
			}
		} else {
			emit log(tr("backup") + ": " + tr("FATAL: backup folder '%1' isn't writable").arg(backupDir.absolutePath()));
			return false;
		}
	} else {
		emit log(tr("backup") + ": " + tr("FATAL: backup folder '%1' doesn't exist").arg(backupDir.absolutePath()));
		return false;
	}
}

void CollectionRebuilderThread::setFilterExpression(QString expression, int syntax, int type)
{
	doFilter = !expression.isEmpty();
	isIncludeFilter = (type == 0);
	QRegExp::PatternSyntax ps;
	switch ( syntax ) {
		case 1:
			ps = QRegExp::RegExp2;
			break;
		case 2:
			ps = QRegExp::Wildcard;
			break;
		case 3:
			ps = QRegExp::WildcardUnix;
			break;
		case 4:
			ps = QRegExp::FixedString;
			break;
		case 5:
			ps = QRegExp::W3CXmlSchema11;
			break;
		case 0:
		default:
			ps = QRegExp::RegExp;
			break;
	}
	filterRx = QRegExp(expression, Qt::CaseSensitive, ps);
	if ( doFilter && !filterRx.isValid() ) {
		emit log(tr("WARNING: invalid filter expression '%1' ignored").arg(expression));
		doFilter = false;
	}
}

void CollectionRebuilderThread::setFilterExpressionSoftware(QString expression, int syntax, int type)
{
	doFilterSoftware = !expression.isEmpty();
	isIncludeFilterSoftware = (type == 0);
	QRegExp::PatternSyntax ps;
	switch ( syntax ) {
		case 1:
			ps = QRegExp::RegExp2;
			break;
		case 2:
			ps = QRegExp::Wildcard;
			break;
		case 3:
			ps = QRegExp::WildcardUnix;
			break;
		case 4:
			ps = QRegExp::FixedString;
			break;
		case 5:
			ps = QRegExp::W3CXmlSchema11;
			break;
		case 0:
		default:
			ps = QRegExp::RegExp;
			break;
	}
	filterRxSoftware = QRegExp(expression, Qt::CaseSensitive, ps);
	if ( doFilterSoftware && !filterRxSoftware.isValid() ) {
		emit log(tr("WARNING: invalid filter expression '%1' ignored").arg(expression));
		doFilterSoftware = false;
	}
}

void CollectionRebuilderThread::pause()
{
	pauseRequested = true;
}

void CollectionRebuilderThread::resume()
{
	isPaused = false;
}

void CollectionRebuilderThread::run()
{
	emit log(tr("rebuilder thread started"));
	while ( !exitThread ) {
		emit log(tr("waiting for work"));
		mutex.lock();
		isWaiting = true;
		isActive = isPaused = stopRebuilding = false;
		waitCondition.wait(&mutex);
		isActive = true;
		isWaiting = false;
		mutex.unlock();
		if ( !exitThread && !stopRebuilding ) {
			setsProcessed = missingROMs = missingDisks = 0;
			emit log(tr("rebuilding started"));
			emit statusUpdated(0, 0, 0);
			emit rebuildStarted();
			emit progressTextChanged(tr("Rebuilding"));
			QTime rebuildTimer, elapsedTime(0, 0, 0, 0);
			rebuildTimer.start();
			if ( checkpoint() < 0 )
				m_xmlIndex = m_xmlIndexCount = -1;
			QString setKey, list, set;
			QStringList romNameList, romSha1List, romCrcList, romSizeList, diskNameList, diskSha1List, diskSizeList;
			while ( !exitThread && !stopRebuilding && nextId(&setKey, &romNameList, &romSha1List, &romCrcList, &romSizeList, &diskNameList, &diskSha1List, &diskSizeList) ) {
				bool pauseMessageLogged = false;
				while ( (pauseRequested || isPaused) && !exitThread && !stopRebuilding ) {
					if ( !pauseMessageLogged ) {
						pauseMessageLogged = true;
						isPaused = true;
						pauseRequested = false;
						emit progressTextChanged(tr("Paused"));
						emit rebuildPaused();
						emit log(tr("rebuilding paused"));
					}
					QTest::qWait(100);
				}
				if ( pauseMessageLogged && !exitThread && !stopRebuilding ) {
					isPaused = false;
					emit progressTextChanged(tr("Rebuilding"));
					emit rebuildResumed();
					emit log(tr("rebuilding resumed"));
				}

				if ( setKey.isEmpty() )
					continue;

				switch ( rebuilderDialog()->romAlyzer()->mode() ) {
					case QMC2_ROMALYZER_MODE_SOFTWARE: {
							QStringList setKeyTokens = setKey.split(":", QString::SkipEmptyParts);
							if ( setKeyTokens.count() < 2 )
								continue;
							list = setKeyTokens[0];
							set = setKeyTokens[1];
							if ( doFilterSoftware ) {
								if ( isIncludeFilterSoftware ) {
									if ( filterRxSoftware.indexIn(list) < 0 )
										continue;
								} else {
									if ( filterRxSoftware.indexIn(list) >= 0 )
										continue;
								}
							}
						}
						break;
					case QMC2_ROMALYZER_MODE_SYSTEM:
					default:
						set = setKey;
						if ( doFilterState ) {
							switch ( qmc2Gamelist->romState(set) ) {
								case 'C':
									if ( !includeStateC )
										continue;
									break;
								case 'M':
									if ( !includeStateM )
										continue;
									break;
								case 'I':
									if ( !includeStateI )
										continue;
									break;
								case 'N':
									if ( !includeStateN )
										continue;
									break;
								case 'U':
								default:
									if ( !includeStateU )
										continue;
									break;
							}
						}
						break;
				}
				if ( doFilter ) {
					if ( isIncludeFilter ) {
						if ( filterRx.indexIn(set) < 0 )
							continue;
					} else {
						if ( filterRx.indexIn(set) >= 0 )
							continue;
					}
				}
				if ( !exitThread && !stopRebuilding && (!romNameList.isEmpty() || !diskNameList.isEmpty()) ) {
					emit log(tr("set rebuilding started for '%1'").arg(setKey));
					for (int i = 0; i < romNameList.count(); i++) {
						bool dbStatusGood = checkSumDb()->exists(romSha1List[i], romCrcList[i], romSizeList[i].toULongLong());
						emit log(tr("required ROM") + ": " + tr("name = '%1', crc = '%2', sha1 = '%3', database status = '%4'").arg(romNameList[i]).arg(romCrcList[i]).arg(romSha1List[i]).arg(dbStatusGood ? tr("available") : tr("not available")));
						if ( !dbStatusGood ) {
							missingROMs++;
							emit newMissing(setKey, tr("ROM"), romNameList[i], romSizeList[i], romCrcList[i], romSha1List[i], tr("check-sum not available in database"));
						}
					}
					for (int i = 0; i < diskNameList.count(); i++) {
						bool dbStatusGood = checkSumDb()->exists(diskSha1List[i], QString());
						emit log(tr("required disk") + ": " + tr("name = '%1', sha1 = '%2', database status = '%3'").arg(diskNameList[i]).arg(diskSha1List[i]).arg(dbStatusGood ? tr("available") : tr("not available")));
						if ( !dbStatusGood ) {
							missingDisks++;
							emit newMissing(setKey, tr("DISK"), diskNameList[i], diskSizeList[i], QString(), diskSha1List[i], tr("check-sum not available in database"));
						}
					}
					emit statusUpdated(setsProcessed, missingROMs, missingDisks);
					bool rewriteOkay = true;
					if ( !romNameList.isEmpty() )
						rewriteOkay = rewriteSet(&setKey, &romNameList, &romSha1List, &romCrcList, &romSizeList, &diskNameList, &diskSha1List);
					if ( rewriteOkay )
						emit log(tr("set rebuilding finished for '%1'").arg(setKey));
					else
						emit log(tr("set rebuilding failed for '%1'").arg(setKey));
					emit statusUpdated(++setsProcessed, missingROMs, missingDisks);
				}
				QTest::qWait(0);
			}
			elapsedTime = elapsedTime.addMSecs(rebuildTimer.elapsed());
			emit log(tr("rebuilding finished - total rebuild time = %1, sets processed = %2, missing ROMs = %3, missing disks = %4").arg(elapsedTime.toString("hh:mm:ss.zzz")).arg(setsProcessed).arg(missingROMs).arg(missingDisks));
			emit progressRangeChanged(0, 100);
			emit progressChanged(0);
			emit progressTextChanged(tr("Idle"));
			if ( m_xmlFile.isOpen() )
				m_xmlFile.close();
			emit rebuildFinished();
		}
	}
	emit log(tr("rebuilder thread ended"));
}
