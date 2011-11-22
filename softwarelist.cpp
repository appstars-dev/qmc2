#include <QFileInfo>
#include <QPainter>
#include <QPixmapCache>
#include <QCursor>
#include <QDir>
#include <QKeyEvent>
#include <QClipboard>
#if defined(Q_WS_MAC)
#include <QTest>
#endif

#include "softwarelist.h"
#include "gamelist.h"
#include "macros.h"
#include "qmc2main.h"
#include "options.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern QSettings *qmc2Config;
extern Gamelist *qmc2Gamelist;
extern bool qmc2CleaningUp;
extern bool qmc2EarlyStartup;
extern bool qmc2UseSoftwareSnapFile;
extern SoftwareList *qmc2SoftwareList;
extern SoftwareSnap *qmc2SoftwareSnap;
extern int qmc2SoftwareSnapPosition;

QMap<QString, QStringList> systemSoftwareListMap;
QMap<QString, QString> softwareListXmlDataCache;
QMap<QString, int> deviceLookupPositionMap;
QString swlBuffer;
QString swlLastLine;
QString swlSelectedMountDevice;
bool swlSupported = true;

SoftwareList::SoftwareList(QString sysName, QWidget *parent)
	: QWidget(parent)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::SoftwareList(QString sysName = %1, QWidget *parent = %2)").arg(sysName).arg((qulonglong)parent));
#endif

	setupUi(this);

	if ( !qmc2SoftwareSnap )
		qmc2SoftwareSnap = new SoftwareSnap();

	qmc2SoftwareSnap->hide();
	snapTimer.setSingleShot(true);
	connect(&snapTimer, SIGNAL(timeout()), qmc2SoftwareSnap, SLOT(loadSnapshot()));

	systemName = sysName;
	loadProc = NULL;
	validData = snapForced = autoSelectSearchItem = false;
	autoMounted = true;

#if defined(QMC2_EMUTYPE_MAME)
	comboBoxDeviceConfiguration->setVisible(false);
	QString altText = tr("Add the currently selected software to the favorites list");
	toolButtonAddToFavorites->setToolTip(altText); toolButtonAddToFavorites->setStatusTip(altText);
	treeWidgetFavoriteSoftware->setColumnCount(QMC2_SWLIST_COLUMN_DEVICECFG);
#elif defined(QMC2_EMUTYPE_MESS)
	horizontalLayout->removeItem(horizontalSpacer);
#endif

	comboBoxSearch->lineEdit()->setPlaceholderText(tr("Enter search string"));

	QFontMetrics fm(QApplication::font());
	QSize iconSize(fm.height() - 2, fm.height() - 2);
	toolButtonAddToFavorites->setIconSize(iconSize);
	toolButtonRemoveFromFavorites->setIconSize(iconSize);
	toolButtonPlay->setIconSize(iconSize);
	toolButtonReload->setIconSize(iconSize);
#if defined(Q_WS_X11)
	toolButtonPlayEmbedded->setIconSize(iconSize);
#else
	toolButtonPlayEmbedded->setVisible(false);
#endif
	toolBoxSoftwareList->setItemIcon(QMC2_SWLIST_KNOWN_SW_PAGE, QIcon(QPixmap(QString::fromUtf8(":/data/img/flat.png")).scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
	toolBoxSoftwareList->setItemIcon(QMC2_SWLIST_FAVORITES_PAGE, QIcon(QPixmap(QString::fromUtf8(":/data/img/favorites.png")).scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
	toolBoxSoftwareList->setItemIcon(QMC2_SWLIST_SEARCH_PAGE, QIcon(QPixmap(QString::fromUtf8(":/data/img/hint.png")).scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));

	toolBoxSoftwareList->setEnabled(false);
	toolButtonAddToFavorites->setEnabled(false);
	toolButtonRemoveFromFavorites->setEnabled(false);
	toolButtonPlay->setEnabled(false);
	toolButtonPlayEmbedded->setEnabled(false);
	comboBoxDeviceConfiguration->setEnabled(false);

	// software list context menu
	softwareListMenu = new QMenu(this);
	QString s = tr("Play selected software");
	QAction *action = softwareListMenu->addAction(tr("&Play"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/launch.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(playActivated()));
#if defined(Q_WS_X11)
	s = tr("Play selected software (embedded)");
	action = softwareListMenu->addAction(tr("Play &embedded"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/embed.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(playEmbeddedActivated()));
#endif
	softwareListMenu->addSeparator();
	s = tr("Add to favorite software list");
	actionAddToFavorites = softwareListMenu->addAction(tr("&Add to favorites"));
	actionAddToFavorites->setToolTip(s); actionAddToFavorites->setStatusTip(s);
	actionAddToFavorites->setIcon(QIcon(QString::fromUtf8(":/data/img/add_to_favorites.png")));
	connect(actionAddToFavorites, SIGNAL(triggered()), this, SLOT(addToFavorites()));
	s = tr("Remove from favorite software list");
	actionRemoveFromFavorites = softwareListMenu->addAction(tr("&Remove from favorites"));
	actionRemoveFromFavorites->setToolTip(s); actionRemoveFromFavorites->setStatusTip(s);
	actionRemoveFromFavorites->setIcon(QIcon(QString::fromUtf8(":/data/img/remove_from_favorites.png")));
	connect(actionRemoveFromFavorites, SIGNAL(triggered()), this, SLOT(removeFromFavorites()));

	// restore widget states
	treeWidgetKnownSoftware->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/KnownSoftwareHeaderState").toByteArray());
	treeWidgetFavoriteSoftware->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/FavoriteSoftwareHeaderState").toByteArray());
	treeWidgetSearchResults->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SearchResultsHeaderState").toByteArray());
	toolBoxSoftwareList->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/PageIndex").toInt());

	connect(treeWidgetKnownSoftware->header(), SIGNAL(sectionClicked(int)), this, SLOT(treeWidgetKnownSoftware_headerSectionClicked(int)));
	connect(treeWidgetFavoriteSoftware->header(), SIGNAL(sectionClicked(int)), this, SLOT(treeWidgetFavoriteSoftware_headerSectionClicked(int)));
	connect(treeWidgetSearchResults->header(), SIGNAL(sectionClicked(int)), this, SLOT(treeWidgetSearchResults_headerSectionClicked(int)));
	connect(&searchTimer, SIGNAL(timeout()), this, SLOT(comboBoxSearch_textChanged_delayed()));

	QHeaderView *header;

	// header context menus
	menuKnownSoftwareHeader = new QMenu(0);
	header = treeWidgetKnownSoftware->header();
	action = menuKnownSoftwareHeader->addAction(tr("Title"), this, SLOT(actionKnownSoftwareHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_TITLE);
	action->setChecked(!treeWidgetKnownSoftware->isColumnHidden(QMC2_SWLIST_COLUMN_TITLE));
	action = menuKnownSoftwareHeader->addAction(tr("Name"), this, SLOT(actionKnownSoftwareHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_NAME);
	action->setChecked(!treeWidgetKnownSoftware->isColumnHidden(QMC2_SWLIST_COLUMN_NAME));
	action = menuKnownSoftwareHeader->addAction(tr("Publisher"), this, SLOT(actionKnownSoftwareHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_PUBLISHER);
	action->setChecked(!treeWidgetKnownSoftware->isColumnHidden(QMC2_SWLIST_COLUMN_PUBLISHER));
	action = menuKnownSoftwareHeader->addAction(tr("Year"), this, SLOT(actionKnownSoftwareHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_YEAR);
	action->setChecked(!treeWidgetKnownSoftware->isColumnHidden(QMC2_SWLIST_COLUMN_YEAR));
	action = menuKnownSoftwareHeader->addAction(tr("Part"), this, SLOT(actionKnownSoftwareHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_PART);
	action->setChecked(!treeWidgetKnownSoftware->isColumnHidden(QMC2_SWLIST_COLUMN_PART));
	action = menuKnownSoftwareHeader->addAction(tr("Interface"), this, SLOT(actionKnownSoftwareHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_INTERFACE);
	action->setChecked(!treeWidgetKnownSoftware->isColumnHidden(QMC2_SWLIST_COLUMN_INTERFACE));
	action = menuKnownSoftwareHeader->addAction(tr("List"), this, SLOT(actionKnownSoftwareHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_LIST);
	action->setChecked(!treeWidgetKnownSoftware->isColumnHidden(QMC2_SWLIST_COLUMN_LIST));
	header->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(header, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(treeWidgetKnownSoftwareHeader_customContextMenuRequested(const QPoint &)));

	menuFavoriteSoftwareHeader = new QMenu(0);
	header = treeWidgetFavoriteSoftware->header();
	action = menuFavoriteSoftwareHeader->addAction(tr("Title"), this, SLOT(actionFavoriteSoftwareHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_TITLE);
	action->setChecked(!treeWidgetFavoriteSoftware->isColumnHidden(QMC2_SWLIST_COLUMN_TITLE));
	action = menuFavoriteSoftwareHeader->addAction(tr("Name"), this, SLOT(actionFavoriteSoftwareHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_NAME);
	action->setChecked(!treeWidgetFavoriteSoftware->isColumnHidden(QMC2_SWLIST_COLUMN_NAME));
	action = menuFavoriteSoftwareHeader->addAction(tr("Publisher"), this, SLOT(actionFavoriteSoftwareHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_PUBLISHER);
	action->setChecked(!treeWidgetFavoriteSoftware->isColumnHidden(QMC2_SWLIST_COLUMN_PUBLISHER));
	action = menuFavoriteSoftwareHeader->addAction(tr("Year"), this, SLOT(actionFavoriteSoftwareHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_YEAR);
	action->setChecked(!treeWidgetFavoriteSoftware->isColumnHidden(QMC2_SWLIST_COLUMN_YEAR));
	action = menuFavoriteSoftwareHeader->addAction(tr("Part"), this, SLOT(actionFavoriteSoftwareHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_PART);
	action->setChecked(!treeWidgetFavoriteSoftware->isColumnHidden(QMC2_SWLIST_COLUMN_PART));
	action = menuFavoriteSoftwareHeader->addAction(tr("Interface"), this, SLOT(actionFavoriteSoftwareHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_INTERFACE);
	action->setChecked(!treeWidgetFavoriteSoftware->isColumnHidden(QMC2_SWLIST_COLUMN_INTERFACE));
	action = menuFavoriteSoftwareHeader->addAction(tr("List"), this, SLOT(actionFavoriteSoftwareHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_LIST);
	action->setChecked(!treeWidgetFavoriteSoftware->isColumnHidden(QMC2_SWLIST_COLUMN_LIST));
	action = menuFavoriteSoftwareHeader->addAction(tr("Device configuration"), this, SLOT(actionFavoriteSoftwareHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_DEVICECFG);
	action->setChecked(!treeWidgetFavoriteSoftware->isColumnHidden(QMC2_SWLIST_COLUMN_DEVICECFG));
	header->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(header, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(treeWidgetFavoriteSoftwareHeader_customContextMenuRequested(const QPoint &)));

	menuSearchResultsHeader = new QMenu(0);
	header = treeWidgetSearchResults->header();
	action = menuSearchResultsHeader->addAction(tr("Title"), this, SLOT(actionSearchResultsHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_TITLE);
	action->setChecked(!treeWidgetSearchResults->isColumnHidden(QMC2_SWLIST_COLUMN_TITLE));
	action = menuSearchResultsHeader->addAction(tr("Name"), this, SLOT(actionSearchResultsHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_NAME);
	action->setChecked(!treeWidgetSearchResults->isColumnHidden(QMC2_SWLIST_COLUMN_NAME));
	action = menuSearchResultsHeader->addAction(tr("Publisher"), this, SLOT(actionSearchResultsHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_PUBLISHER);
	action->setChecked(!treeWidgetSearchResults->isColumnHidden(QMC2_SWLIST_COLUMN_PUBLISHER));
	action = menuSearchResultsHeader->addAction(tr("Year"), this, SLOT(actionSearchResultsHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_YEAR);
	action->setChecked(!treeWidgetSearchResults->isColumnHidden(QMC2_SWLIST_COLUMN_YEAR));
	action = menuSearchResultsHeader->addAction(tr("Part"), this, SLOT(actionSearchResultsHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_PART);
	action->setChecked(!treeWidgetSearchResults->isColumnHidden(QMC2_SWLIST_COLUMN_PART));
	action = menuSearchResultsHeader->addAction(tr("Interface"), this, SLOT(actionSearchResultsHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_INTERFACE);
	action->setChecked(!treeWidgetSearchResults->isColumnHidden(QMC2_SWLIST_COLUMN_INTERFACE));
	action = menuSearchResultsHeader->addAction(tr("List"), this, SLOT(actionSearchResultsHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_LIST);
	action->setChecked(!treeWidgetSearchResults->isColumnHidden(QMC2_SWLIST_COLUMN_LIST));
	header->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(header, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(treeWidgetSearchResultsHeader_customContextMenuRequested(const QPoint &)));
}

SoftwareList::~SoftwareList()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::~SoftwareList()");
#endif

	// save widget states
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/KnownSoftwareHeaderState", treeWidgetKnownSoftware->header()->saveState());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/FavoriteSoftwareHeaderState", treeWidgetFavoriteSoftware->header()->saveState());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SearchResultsHeaderState", treeWidgetSearchResults->header()->saveState());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/PageIndex", toolBoxSoftwareList->currentIndex());
}

QString &SoftwareList::getSoftwareListXmlData(QString listName)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::getSoftwareListXmlData(QString listName = %1)").arg(listName));
#endif

	static QString softwareListXmlBuffer;

	softwareListXmlBuffer = softwareListXmlDataCache[listName];

	if ( softwareListXmlBuffer.isEmpty() ) {
		int i = 0;
		int swlLinesMax = swlLines.count() - 1;
		QString s = "<softwarelist name=\"" + listName + "\"";
		while ( !swlLines[i].startsWith(s) && i < swlLinesMax ) i++;
		softwareListXmlBuffer = "<?xml version=\"1.0\"?>\n";
		while ( !swlLines[i].startsWith("</softwarelist>") && i < swlLinesMax )
			softwareListXmlBuffer += swlLines[i++].simplified() + "\n";
		softwareListXmlBuffer += "</softwarelist>";
		if ( i < swlLinesMax ) {
			softwareListXmlDataCache[listName] = softwareListXmlBuffer;
		} else
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: software list '%1' not found").arg(listName));
	}

	return softwareListXmlBuffer;
}

QString &SoftwareList::lookupMountDevice(QString device, QString interface, QStringList *mountList)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::lookupMountDevice(QString device = %1, QString interface = %2, QStringList *mountList = %3)").arg(device).arg(interface).arg((qulonglong)mountList));
#endif

	static QString softwareListDeviceName;

	QMap<QString, QStringList> deviceInstanceMap;
	int i = deviceLookupPositionMap[systemName];

	softwareListDeviceName.clear();

#if defined(QMC2_EMUTYPE_MAME)
	QString s = "<game name=\"" + systemName + "\"";
	while ( !qmc2Gamelist->xmlLines[i].contains(s) ) i++;
	if ( qmc2Gamelist->xmlLines[i].contains(s) ) deviceLookupPositionMap[systemName] = i - 1;
	while ( !qmc2Gamelist->xmlLines[i].contains("</game>") ) {
		QString line = qmc2Gamelist->xmlLines[i++].simplified();
		if ( line.startsWith("<device type=\"") ) {
			int startIndex = line.indexOf("interface=\"");
			int endIndex;
			if ( startIndex >= 0 ) {
				startIndex += 11;
				int endIndex = line.indexOf("\"", startIndex);
				QString devInterface = line.mid(startIndex, endIndex - startIndex);
				line = qmc2Gamelist->xmlLines[i++].simplified();
				startIndex = line.indexOf("briefname=\"") + 11;
				endIndex = line.indexOf("\"", startIndex);
				QString devName = line.mid(startIndex, endIndex - startIndex);
				if ( !devName.isEmpty() )
					deviceInstanceMap[devInterface] << devName;
			} else {
				line = qmc2Gamelist->xmlLines[i++].simplified();
				startIndex = line.indexOf("briefname=\"") + 11;
				endIndex = line.indexOf("\"", startIndex);
				QString devName = line.mid(startIndex, endIndex - startIndex);
				if ( !devName.isEmpty() )
					deviceInstanceMap[devName] << devName;
			}
		}
	}
#elif defined(QMC2_EMUTYPE_MESS)
	QString s = "<machine name=\"" + systemName + "\"";
	while ( !qmc2Gamelist->xmlLines[i].contains(s) ) i++;
	if ( qmc2Gamelist->xmlLines[i].contains(s) ) deviceLookupPositionMap[systemName] = i - 1;
	while ( !qmc2Gamelist->xmlLines[i].contains("</machine>") ) {
		QString line = qmc2Gamelist->xmlLines[i++].simplified();
		if ( line.startsWith("<device type=\"") ) {
			int startIndex = line.indexOf("interface=\"");
			int endIndex;
			if ( startIndex >= 0 ) {
				startIndex += 11;
				int endIndex = line.indexOf("\"", startIndex);
				QString devInterface = line.mid(startIndex, endIndex - startIndex);
				line = qmc2Gamelist->xmlLines[i++].simplified();
				startIndex = line.indexOf("briefname=\"") + 11;
				endIndex = line.indexOf("\"", startIndex);
				QString devName = line.mid(startIndex, endIndex - startIndex);
				if ( !devName.isEmpty() )
					deviceInstanceMap[devInterface] << devName;
			} else {
				line = qmc2Gamelist->xmlLines[i++].simplified();
				startIndex = line.indexOf("briefname=\"") + 11;
				endIndex = line.indexOf("\"", startIndex);
				QString devName = line.mid(startIndex, endIndex - startIndex);
				if ( !devName.isEmpty() )
					deviceInstanceMap[devName] << devName;
			}
		}
	}
#endif

	QStringList briefNames = deviceInstanceMap[interface];

	if ( briefNames.contains(device) )
		softwareListDeviceName = device;
	else if ( briefNames.count() > 0 )
		softwareListDeviceName = briefNames[0];

	if ( successfulLookups.contains(softwareListDeviceName) )
		softwareListDeviceName.clear();

	if ( mountList != NULL )
		*mountList = briefNames;

	if ( !softwareListDeviceName.isEmpty() )
		successfulLookups << softwareListDeviceName;

	return softwareListDeviceName;
}

QString &SoftwareList::getXmlData()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::getXmlData()");
#endif

	static QString xmlBuffer;

	QStringList softwareList = systemSoftwareListMap[systemName];
	if ( softwareList.isEmpty() ) {
		int i = 0;
#if defined(QMC2_EMUTYPE_MAME)
		QString s = "<game name=\"" + systemName + "\"";
		while ( !qmc2Gamelist->xmlLines[i].contains(s) ) i++;
		while ( !qmc2Gamelist->xmlLines[i].contains("</game>") ) {
			QString line = qmc2Gamelist->xmlLines[i++].simplified();
			if ( line.startsWith("<softwarelist name=\"") ) {
				int startIndex = line.indexOf("\"") + 1;
				int endIndex = line.indexOf("\"", startIndex);
				softwareList << line.mid(startIndex, endIndex - startIndex); 
			}
		}
#elif defined(QMC2_EMUTYPE_MESS)
		QString s = "<machine name=\"" + systemName + "\"";
		while ( !qmc2Gamelist->xmlLines[i].contains(s) ) i++;
		while ( !qmc2Gamelist->xmlLines[i].contains("</machine>") ) {
			QString line = qmc2Gamelist->xmlLines[i++].simplified();
			if ( line.startsWith("<softwarelist name=\"") ) {
				int startIndex = line.indexOf("\"") + 1;
				int endIndex = line.indexOf("\"", startIndex);
				softwareList << line.mid(startIndex, endIndex - startIndex); 
			}
		}
#endif
		if ( softwareList.isEmpty() )
			softwareList << "NO_SOFTWARE_LIST";
		systemSoftwareListMap[systemName] = softwareList;
#ifdef QMC2_DEBUG
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: systemSoftwareListMap[%1] = %2").arg(systemName).arg(softwareList.join(", ")));
#endif
	}
#ifdef QMC2_DEBUG
	else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: systemSoftwareListMap[%1] = %2 (cached)").arg(systemName).arg(systemSoftwareListMap[systemName].join(", ")));
#endif

	xmlBuffer.clear();

	if ( !softwareList.isEmpty() && !softwareList.contains("NO_SOFTWARE_LIST") ) {
		QString swlString = softwareList.join(", ");
		toolBoxSoftwareList->setItemText(QMC2_SWLIST_KNOWN_SW_PAGE, tr("Known software (%1)").arg(swlString));
		toolBoxSoftwareList->setItemText(QMC2_SWLIST_FAVORITES_PAGE, tr("Favorites (%1)").arg(swlString));
		toolBoxSoftwareList->setItemText(QMC2_SWLIST_SEARCH_PAGE, tr("Search (%1)").arg(swlString));
		toolBoxSoftwareList->setEnabled(true);

#if defined(QMC2_EMUTYPE_MESS)
		// load available device configurations, if any...
		qmc2Config->beginGroup(QString("MESS/Configuration/Devices/%1").arg(systemName));
		QStringList configurationList = qmc2Config->childGroups();
		qmc2Config->endGroup();
		if ( !configurationList.isEmpty() ) {
			comboBoxDeviceConfiguration->insertItems(1, configurationList);
			comboBoxDeviceConfiguration->setEnabled(true);
		}
#endif
	} else {
		toolBoxSoftwareList->setItemText(QMC2_SWLIST_KNOWN_SW_PAGE, tr("Known software (no data available)"));
		toolBoxSoftwareList->setItemText(QMC2_SWLIST_FAVORITES_PAGE, tr("Favorites (no data available)"));
		toolBoxSoftwareList->setItemText(QMC2_SWLIST_SEARCH_PAGE, tr("Search (no data available)"));
	}

	return xmlBuffer;
}

bool SoftwareList::load()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::load()");
#endif

	bool swlCacheOkay = true;
	autoMounted = true;
	validData = swlSupported;
#if defined(QMC2_EMUTYPE_MAME)
	QString swlCachePath = qmc2Config->value("MAME/FilesAndDirectories/SoftwareListCache").toString();
#elif defined(QMC2_EMUTYPE_MESS)
	QString swlCachePath = qmc2Config->value("MESS/FilesAndDirectories/SoftwareListCache").toString();
#endif

	toolButtonReload->setEnabled(false);

	treeWidgetKnownSoftware->clear();
	treeWidgetFavoriteSoftware->clear();
	treeWidgetSearchResults->clear();

	treeWidgetKnownSoftware->setSortingEnabled(false);
	treeWidgetKnownSoftware->header()->setSortIndicatorShown(false);
	treeWidgetFavoriteSoftware->setSortingEnabled(false);
	treeWidgetFavoriteSoftware->header()->setSortIndicatorShown(false);
	treeWidgetSearchResults->setSortingEnabled(false);
	treeWidgetSearchResults->header()->setSortIndicatorShown(false);

	deviceLookupPositionMap.clear();

	if ( swlBuffer.isEmpty() && swlSupported ) {
          	qmc2MainWindow->tabSoftwareList->setUpdatesEnabled(true);
		labelLoadingSoftwareLists->setVisible(true);
		toolBoxSoftwareList->setVisible(false);
		show();

		swlLines.clear();
		validData = false;
		swlCacheOkay = false;
		if ( !swlCachePath.isEmpty() ) {
			fileSWLCache.setFileName(swlCachePath);
			if ( fileSWLCache.open(QIODevice::ReadOnly | QIODevice::Text) ) {
				QTextStream ts(&fileSWLCache);
				QString line = ts.readLine();
				line = ts.readLine();
#if defined(QMC2_EMUTYPE_MAME)
				if ( line.startsWith("MAME_VERSION") ) {
					QStringList words = line.split("\t");
					if ( words.count() > 1 ) {
						if ( qmc2Gamelist->emulatorVersion == words[1] )
							swlCacheOkay = true;
					}
				}
#elif defined(QMC2_EMUTYPE_MESS)
				if ( line.startsWith("MESS_VERSION") ) {
					QStringList words = line.split("\t");
					if ( words.count() > 1 ) {
						if ( qmc2Gamelist->emulatorVersion == words[1] )
							swlCacheOkay = true;
					}
				}
#endif
				if ( swlCacheOkay ) {
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading XML software list data from cache"));
					QTime elapsedTime;
					loadTimer.start();
					
					if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
						qmc2MainWindow->progressBarGamelist->setFormat(tr("SWL cache - %p%"));
					else
						qmc2MainWindow->progressBarGamelist->setFormat("%p%");
					QFileInfo fi(swlCachePath);
					qmc2MainWindow->progressBarGamelist->setRange(0, fi.size());
					qmc2MainWindow->progressBarGamelist->setValue(0);
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
								line = lines[l];
								swlBuffer += line + "\n";
							}
						}
						if ( endsWithNewLine )
							readBuffer.clear();
						else
							readBuffer = lines.last();
						qmc2MainWindow->progressBarGamelist->setValue(swlBuffer.length());
					}
					qmc2MainWindow->progressBarGamelist->reset();
					elapsedTime = elapsedTime.addMSecs(loadTimer.elapsed());
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading XML software list data from cache, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
					validData = true;
				}
				if ( fileSWLCache.isOpen() )
					fileSWLCache.close();
			}
		} else {
#if defined(QMC2_EMUTYPE_MAME)
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ERROR: the file name for the MAME software list cache is empty -- please correct this and reload the game list afterwards"));
#elif defined(QMC2_EMUTYPE_MESS)
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ERROR: the file name for the MESS software list cache is empty -- please correct this and reload the machine list afterwards"));
#endif
			labelLoadingSoftwareLists->setVisible(false);
			toolBoxSoftwareList->setVisible(true);

			return false;
		}
        }

	if ( !swlCacheOkay ) {
          	qmc2MainWindow->tabSoftwareList->setUpdatesEnabled(true);
		labelLoadingSoftwareLists->setVisible(true);
		toolBoxSoftwareList->setVisible(false);
		show();

		loadTimer.start();
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading XML software list data and (re)creating cache"));

		if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
			qmc2MainWindow->progressBarGamelist->setFormat(tr("SWL data - %p%"));
		else
			qmc2MainWindow->progressBarGamelist->setFormat("%p%");

		if ( !fileSWLCache.open(QIODevice::WriteOnly | QIODevice::Text) ) {
#if defined(QMC2_EMUTYPE_MAME)
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ERROR: can't open the MAME software list cache for writing, path = %1 -- please check/correct access permissions and reload the game list afterwards").arg(swlCachePath));
#elif defined(QMC2_EMUTYPE_MESS)
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ERROR: can't open the MESS software list cache for writing, path = %1 -- please check/correct access permissions and reload the machine list afterwards").arg(swlCachePath));
#endif
			return false;
		}

		swlBuffer.clear();
		swlLastLine.clear();

		tsSWLCache.setDevice(&fileSWLCache);
		tsSWLCache.reset();
		tsSWLCache << "# THIS FILE IS AUTO-GENERATED - PLEASE DO NOT EDIT!\n";
#if defined(QMC2_EMUTYPE_MAME)
		tsSWLCache << "MAME_VERSION\t" + qmc2Gamelist->emulatorVersion + "\n";
#elif defined(QMC2_EMUTYPE_MESS)
		tsSWLCache << "MESS_VERSION\t" + qmc2Gamelist->emulatorVersion + "\n";
#endif
		
		loadProc = new QProcess(this);

		connect(loadProc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(loadError(QProcess::ProcessError)));
		connect(loadProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(loadFinished(int, QProcess::ExitStatus)));
		connect(loadProc, SIGNAL(readyReadStandardOutput()), this, SLOT(loadReadyReadStandardOutput()));
		connect(loadProc, SIGNAL(readyReadStandardError()), this, SLOT(loadReadyReadStandardError()));
		connect(loadProc, SIGNAL(started()), this, SLOT(loadStarted()));
		connect(loadProc, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(loadStateChanged(QProcess::ProcessState)));

#if defined(QMC2_EMUTYPE_MAME)
		QString command = qmc2Config->value("MAME/FilesAndDirectories/ExecutableFile").toString();
		QStringList args;
		args << "-listsoftware";
		QString hashPath = qmc2Config->value("MAME/Configuration/Global/hashpath").toString().replace("~", "$HOME");
		if ( !hashPath.isEmpty() )
			args << "-hashpath" << hashPath;
#elif defined(QMC2_EMUTYPE_MESS)
		QString command = qmc2Config->value("MESS/FilesAndDirectories/ExecutableFile").toString();
		QStringList args;
		args << "-listsoftware";
		QString hashPath = qmc2Config->value("MESS/Configuration/Global/hashpath").toString().replace("~", "$HOME");
		if ( !hashPath.isEmpty() )
			args << "-hashpath" << hashPath;
#endif

		loadProc->start(command, args);

		if ( loadProc ) {
			// FIXME: this is blocking the GUI shortly
			if ( loadProc->waitForStarted() ) {
				while ( loadProc->state() == QProcess::Running ) {
#if defined(Q_WS_MAC)
					QTest::qWait(10);
#else
					loadProc->waitForFinished(10);
#endif
					qApp->processEvents();
				}
			} else
				validData = false;
		}

		if ( !validData ) {
			toolBoxSoftwareList->setItemText(QMC2_SWLIST_KNOWN_SW_PAGE, tr("Known software (no data available)"));
			toolBoxSoftwareList->setItemText(QMC2_SWLIST_FAVORITES_PAGE, tr("Favorites (no data available)"));
			toolBoxSoftwareList->setItemText(QMC2_SWLIST_SEARCH_PAGE, tr("Search (no data available)"));

			labelLoadingSoftwareLists->setVisible(false);
			toolBoxSoftwareList->setVisible(true);

			return false;
		}
	}

#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::load(): validData = %1").arg(validData ? "true" : "false"));
#endif

	labelLoadingSoftwareLists->setVisible(false);
	toolBoxSoftwareList->setVisible(true);

	QString xmlData = getXmlData();

	QStringList softwareList = systemSoftwareListMap[systemName];
	if ( !softwareList.contains("NO_SOFTWARE_LIST") ) {
		swlLines = swlBuffer.split("\n");
		foreach (QString swList, softwareList) {
			QString softwareListXml = getSoftwareListXmlData(swList);
#ifdef QMC2_DEBUG
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::load(): XML data for software list '%1' follows:\n%2").arg(swList).arg(softwareListXml));
#endif
			QXmlInputSource xmlInputSource;
			xmlInputSource.setData(softwareListXml);
			SoftwareListXmlHandler xmlHandler(treeWidgetKnownSoftware);
			QXmlSimpleReader xmlReader;
			xmlReader.setContentHandler(&xmlHandler);
			if ( !xmlReader.parse(xmlInputSource) )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: error while parsing XML data for software list '%1'").arg(swList));
#ifdef QMC2_DEBUG
			else
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::load(): successfully parsed the XML data for software list '%1'").arg(swList));
#endif
		}

		// load favorites
#if defined(QMC2_EMUTYPE_MAME)
		QStringList softwareNames = qmc2Config->value(QString("MAME/Favorites/%1/SoftwareNames").arg(systemName)).toStringList();
#elif defined(QMC2_EMUTYPE_MESS)
		QStringList softwareNames = qmc2Config->value(QString("MESS/Favorites/%1/SoftwareNames").arg(systemName)).toStringList();
		QStringList configNames = qmc2Config->value(QString("MESS/Favorites/%1/DeviceConfigs").arg(systemName)).toStringList();
#endif
		for (int i = 0; i < softwareNames.count(); i++) {
			QString software = softwareNames[i];
			QList<QTreeWidgetItem *> matchedSoftware = treeWidgetKnownSoftware->findItems(software, Qt::MatchExactly | Qt::MatchCaseSensitive, QMC2_SWLIST_COLUMN_NAME);
			QTreeWidgetItem *swItem = NULL;
			if ( matchedSoftware.count() > 0 ) swItem = matchedSoftware.at(0);
			if ( swItem ) {
				SoftwareItem *item = new SoftwareItem(treeWidgetFavoriteSoftware);
				item->setText(QMC2_SWLIST_COLUMN_TITLE, swItem->text(QMC2_SWLIST_COLUMN_TITLE));
				item->setText(QMC2_SWLIST_COLUMN_NAME, swItem->text(QMC2_SWLIST_COLUMN_NAME));
				item->setText(QMC2_SWLIST_COLUMN_PUBLISHER, swItem->text(QMC2_SWLIST_COLUMN_PUBLISHER));
				item->setText(QMC2_SWLIST_COLUMN_YEAR, swItem->text(QMC2_SWLIST_COLUMN_YEAR));
				item->setText(QMC2_SWLIST_COLUMN_PART, swItem->text(QMC2_SWLIST_COLUMN_PART));
				item->setText(QMC2_SWLIST_COLUMN_INTERFACE, swItem->text(QMC2_SWLIST_COLUMN_INTERFACE));
				item->setText(QMC2_SWLIST_COLUMN_LIST, swItem->text(QMC2_SWLIST_COLUMN_LIST));
				SoftwareItem *subItem = new SoftwareItem(item);
				subItem->setText(QMC2_SWLIST_COLUMN_TITLE, tr("Waiting for data..."));
#if defined(QMC2_EMUTYPE_MESS)
				if ( configNames.count() > i )
					item->setText(QMC2_SWLIST_COLUMN_DEVICECFG, configNames[i]);
#endif
			}
		}
	}

	treeWidgetKnownSoftware->setSortingEnabled(true);
	treeWidgetKnownSoftware->header()->setSortIndicatorShown(true);
	treeWidgetFavoriteSoftware->setSortingEnabled(true);
	treeWidgetFavoriteSoftware->header()->setSortIndicatorShown(true);
	treeWidgetSearchResults->setSortingEnabled(true);
	treeWidgetSearchResults->header()->setSortIndicatorShown(true);

	toolButtonReload->setEnabled(true);

	return true;
}

bool SoftwareList::save()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::save()");
#endif

#if defined(QMC2_EMUTYPE_MAME)
	qmc2Config->remove(QString("MAME/Favorites/%1").arg(systemName));
#elif defined(QMC2_EMUTYPE_MESS)
	qmc2Config->remove(QString("MESS/Favorites/%1").arg(systemName));
#endif

	QList<QTreeWidgetItem *> itemList = treeWidgetFavoriteSoftware->findItems("*", Qt::MatchWildcard);

	QStringList softwareNames;
#if defined(QMC2_EMUTYPE_MESS)
	QStringList configNames;
	bool onlyEmptyConfigNames = true;
#endif

	foreach (QTreeWidgetItem *item, itemList) {
		softwareNames << item->text(QMC2_SWLIST_COLUMN_NAME);
#if defined(QMC2_EMUTYPE_MESS)
		QString s = item->text(QMC2_SWLIST_COLUMN_DEVICECFG);
		if ( !s.isEmpty() ) onlyEmptyConfigNames = false;
		configNames << s;
#endif
	}

	if ( !softwareNames.isEmpty() ) {
#if defined(QMC2_EMUTYPE_MAME)
		qmc2Config->setValue(QString("MAME/Favorites/%1/SoftwareNames").arg(systemName), softwareNames);
#elif defined(QMC2_EMUTYPE_MESS)
		qmc2Config->setValue(QString("MESS/Favorites/%1/SoftwareNames").arg(systemName), softwareNames);
		if ( onlyEmptyConfigNames )
			qmc2Config->remove(QString("MESS/Favorites/%1/DeviceConfigs").arg(systemName));
		else
			qmc2Config->setValue(QString("MESS/Favorites/%1/DeviceConfigs").arg(systemName), configNames);
#endif
	}

	return true;
}

void SoftwareList::closeEvent(QCloseEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::closeEvent(QCloseEvent *e = %1)").arg((qulonglong)e));
#endif

	QWidget::closeEvent(e);
}

void SoftwareList::hideEvent(QHideEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::hideEvent(QHideEvent *e = %1)").arg((qulonglong)e));
#endif

	QWidget::hideEvent(e);
}

void SoftwareList::leaveEvent(QEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::leaveEvent(QEvent *e = %1)").arg((qulonglong)e));
#endif

	if ( qmc2SoftwareSnap )
		if ( qmc2SoftwareSnap->geometry().contains(QCursor::pos()) ) {
			snapForced = true;
			snapTimer.start(QMC2_SWSNAP_DELAY);
		}

	if ( !snapForced )
		cancelSoftwareSnap();

	QWidget::leaveEvent(e);
}

void SoftwareList::resizeEvent(QResizeEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::resizeEvent(QResizeEvent *e = %1)").arg((qulonglong)e));
#endif

	QWidget::resizeEvent(e);
}

void SoftwareList::mouseMoveEvent(QMouseEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::mouseMoveEvent(QMouseEvent *e = %1)").arg((qulonglong)e));
#endif

	cancelSoftwareSnap();

	QWidget::mouseMoveEvent(e);
}

void SoftwareList::showEvent(QShowEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::showEvent(QShowEvent *e = %1)").arg((qulonglong)e));
#endif

	QWidget::showEvent(e);
}

void SoftwareList::loadStarted()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::loadStarted()"));
#endif

	// we don't know how many items there are...
	qmc2MainWindow->progressBarGamelist->setRange(0, 0);
	qmc2MainWindow->progressBarGamelist->reset();
}

void SoftwareList::loadFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::loadFinished(int exitCode = %1, QProcess::ExitStatus exitStatus = %2)").arg(exitCode).arg(exitStatus));
#endif

	QTime elapsedTime;
	elapsedTime = elapsedTime.addMSecs(loadTimer.elapsed());
	if ( exitStatus == QProcess::NormalExit && exitCode == 0 ) {
		validData = true;
	} else {
#if defined(QMC2_EMUTYPE_MAME)
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the external process called to load the MAME software lists didn't exit cleanly -- exitCode = %1, exitStatus = %2").arg(exitCode).arg(exitStatus == QProcess::NormalExit ? tr("normal") : tr("crashed")));
#elif defined(QMC2_EMUTYPE_MESS)
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the external process called to load the MESS software lists didn't exit cleanly -- exitCode = %1, exitStatus = %2").arg(exitCode).arg(exitStatus == QProcess::NormalExit ? tr("normal") : tr("crashed")));
#endif
		validData = false;
	}
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading XML software list data and (re)creating cache, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));

	if ( fileSWLCache.isOpen() )
		fileSWLCache.close();

	qmc2MainWindow->progressBarGamelist->setRange(0, 1);
	qmc2MainWindow->progressBarGamelist->reset();
}

void SoftwareList::loadReadyReadStandardOutput()
{
	QProcess *proc = (QProcess *)sender();

#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::loadReadyReadStandardOutput()"));
#endif

	QString s = swlLastLine + proc->readAllStandardOutput();
#if defined(Q_WS_WIN)
	s.replace("\r\n", "\n"); // convert WinDOS's "0x0D 0x0A" to just "0x0A" 
#endif
	QStringList lines = s.split("\n");

	if ( s.endsWith("\n") ) {
		swlLastLine.clear();
	} else {
		swlLastLine = lines.last();
		lines.removeLast();
	}

	foreach (QString line, lines) {
		line = line.trimmed();
		if ( !line.isEmpty() )
			if ( !line.startsWith("<!") && !line.startsWith("<?xml") && !line.startsWith("]>") ) {
				tsSWLCache << line << "\n";
				swlBuffer += line + "\n";
			}
	}
}

void SoftwareList::loadReadyReadStandardError()
{
	QProcess *proc = (QProcess *)sender();

	QString data = proc->readAllStandardError();
	data = data.trimmed();

#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::loadReadyReadStandardError(): data = '%1'").arg(data));
#endif

	if ( data.contains("unknown option: -listsoftware") || data.contains("Unknown command 'listsoftware' specified") ) {
#if defined(QMC2_EMUTYPE_MAME)
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the currently selected MAME emulator doesn't support software lists"));
#elif defined(QMC2_EMUTYPE_MESS)
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the currently selected MESS emulator doesn't support software lists"));
#endif
		swlSupported = false;
		if ( fileSWLCache.isOpen() )
			fileSWLCache.close();
		fileSWLCache.remove();
	}
}

void SoftwareList::loadError(QProcess::ProcessError processError)
{
#ifdef QMC2_DEBUG
	QProcess *proc = (QProcess *)sender();
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::loadError(QProcess::ProcessError processError = %1)").arg(processError));
#endif

#if defined(QMC2_EMUTYPE_MAME)
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the external process called to load the MAME software lists caused an error -- processError = %1").arg(processError));
#elif defined(QMC2_EMUTYPE_MESS)
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the external process called to load the MESS software lists caused an error -- processError = %1").arg(processError));
#endif
	validData = false;

	if ( fileSWLCache.isOpen() )
		fileSWLCache.close();

	qmc2MainWindow->progressBarGamelist->setRange(0, 1);
	qmc2MainWindow->progressBarGamelist->reset();
}

void SoftwareList::loadStateChanged(QProcess::ProcessState processState)
{
#ifdef QMC2_DEBUG
	QProcess *proc = (QProcess *)sender();
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::loadStateChanged(QProcess::ProcessState processState = %1)").arg(processState));
#endif

}

void SoftwareList::on_toolButtonReload_clicked(bool checked)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_toolButtonReload_clicked(bool checked = %1)").arg(checked));
#endif

	save();

	treeWidgetKnownSoftware->clear();
	treeWidgetFavoriteSoftware->clear();
	treeWidgetSearchResults->clear();
	toolBoxSoftwareList->setEnabled(false);
	toolButtonAddToFavorites->setEnabled(false);
	toolButtonRemoveFromFavorites->setEnabled(false);
	toolButtonPlay->setEnabled(false);
	toolButtonPlayEmbedded->setEnabled(false);
	comboBoxDeviceConfiguration->setEnabled(false);
	comboBoxDeviceConfiguration->clear();
	comboBoxDeviceConfiguration->insertItem(0, tr("No additional devices"));

	QTimer::singleShot(0, this, SLOT(load()));
}

void SoftwareList::on_toolButtonAddToFavorites_clicked(bool checked)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_toolButtonAddToFavorites_clicked(bool checked = %1)").arg(checked));
#endif

	QList<QTreeWidgetItem *> selectedItems;

	switch ( toolBoxSoftwareList->currentIndex() ) {
		case QMC2_SWLIST_KNOWN_SW_PAGE:
			selectedItems = treeWidgetKnownSoftware->selectedItems();
			break;
		case QMC2_SWLIST_SEARCH_PAGE:
			selectedItems = treeWidgetSearchResults->selectedItems();
			break;
		case QMC2_SWLIST_FAVORITES_PAGE:
			selectedItems = treeWidgetFavoriteSoftware->selectedItems();
			break;
	}

	QTreeWidgetItem *si = NULL;

	if ( selectedItems.count() > 0 )
		si = selectedItems.at(0);

	if ( si ) {
		SoftwareItem *item = NULL;
		QList<QTreeWidgetItem *> matchedItems = treeWidgetFavoriteSoftware->findItems(si->text(QMC2_SWLIST_COLUMN_NAME), Qt::MatchExactly | Qt::MatchCaseSensitive, QMC2_SWLIST_COLUMN_NAME);
		if ( matchedItems.count() > 0 )
			item = (SoftwareItem *)matchedItems.at(0);
		else {
			item = new SoftwareItem(treeWidgetFavoriteSoftware);
			SoftwareItem *subItem = new SoftwareItem(item);
			subItem->setText(QMC2_SWLIST_COLUMN_TITLE, tr("Waiting for data..."));
		}
		if ( item ) {
			item->setText(QMC2_SWLIST_COLUMN_TITLE, si->text(QMC2_SWLIST_COLUMN_TITLE));
			item->setText(QMC2_SWLIST_COLUMN_NAME, si->text(QMC2_SWLIST_COLUMN_NAME));
			item->setText(QMC2_SWLIST_COLUMN_PUBLISHER, si->text(QMC2_SWLIST_COLUMN_PUBLISHER));
			item->setText(QMC2_SWLIST_COLUMN_YEAR, si->text(QMC2_SWLIST_COLUMN_YEAR));
			item->setText(QMC2_SWLIST_COLUMN_PART, si->text(QMC2_SWLIST_COLUMN_PART));
			item->setText(QMC2_SWLIST_COLUMN_INTERFACE, si->text(QMC2_SWLIST_COLUMN_INTERFACE));
			item->setText(QMC2_SWLIST_COLUMN_LIST, si->text(QMC2_SWLIST_COLUMN_LIST));
#if defined(QMC2_EMUTYPE_MESS)
			if ( comboBoxDeviceConfiguration->currentIndex() > QMC2_SWLIST_MSEL_AUTO_MOUNT )
				item->setText(QMC2_SWLIST_COLUMN_DEVICECFG, comboBoxDeviceConfiguration->currentText());
			else
				item->setText(QMC2_SWLIST_COLUMN_DEVICECFG, QString());
#endif
		}
	}
}

void SoftwareList::on_toolButtonRemoveFromFavorites_clicked(bool checked)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_toolButtonRemoveFromFavorites_clicked(bool checked = %1)").arg(checked));
#endif

	if ( toolBoxSoftwareList->currentIndex() != QMC2_SWLIST_FAVORITES_PAGE )
		return;

	QList<QTreeWidgetItem *> selectedItems = treeWidgetFavoriteSoftware->selectedItems();
	QTreeWidgetItem *si = NULL;

	if ( selectedItems.count() > 0 ) {
		si = selectedItems.at(0);
		while ( si->parent() ) si = si->parent();
	}

	if ( si ) {
		QTreeWidgetItem *itemToBeRemoved = treeWidgetFavoriteSoftware->takeTopLevelItem(treeWidgetFavoriteSoftware->indexOfTopLevelItem(si));
		if ( itemToBeRemoved )
			delete itemToBeRemoved;
	}
}

void SoftwareList::on_toolButtonPlay_clicked(bool checked)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_toolButtonPlay_clicked(bool checked = %1)").arg(checked));
#endif

	QTimer::singleShot(0, qmc2MainWindow, SLOT(on_actionPlay_activated()));
}

void SoftwareList::on_toolButtonPlayEmbedded_clicked(bool checked)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_toolButtonPlayEmbedded_clicked(bool checked = %1)").arg(checked));
#endif

	QTimer::singleShot(0, qmc2MainWindow, SLOT(on_actionPlayEmbedded_activated()));
}

void SoftwareList::treeWidgetKnownSoftware_headerSectionClicked(int index)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_treeWidgetKnownSoftware_headerSectionClicked(int index = %1)").arg(index));
#endif

	QList<QTreeWidgetItem *> selectedItems = treeWidgetKnownSoftware->selectedItems();
	if ( selectedItems.count() > 0 )
		treeWidgetKnownSoftware->scrollToItem(selectedItems[0]);
}

void SoftwareList::treeWidgetFavoriteSoftware_headerSectionClicked(int index)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_treeWidgetFavoriteSoftware_headerSectionClicked(int index = %1)").arg(index));
#endif

	QList<QTreeWidgetItem *> selectedItems = treeWidgetFavoriteSoftware->selectedItems();
	if ( selectedItems.count() > 0 )
		treeWidgetFavoriteSoftware->scrollToItem(selectedItems[0]);
}

void SoftwareList::treeWidgetSearchResults_headerSectionClicked(int index)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_treeWidgetSearchResults_headerSectionClicked(int index = %1)").arg(index));
#endif

	QList<QTreeWidgetItem *> selectedItems = treeWidgetSearchResults->selectedItems();
	if ( selectedItems.count() > 0 )
		treeWidgetSearchResults->scrollToItem(selectedItems[0]);
}

void SoftwareList::on_treeWidgetKnownSoftware_itemExpanded(QTreeWidgetItem *item)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_treeWidgetKnownSoftware_itemExpanded(QTreeWidgetItem *item = %1)").arg((qulonglong)item));
#endif

	if ( item->child(0)->text(QMC2_SWLIST_COLUMN_TITLE) == tr("Waiting for data...") ) {
		QString softwareListXml = getSoftwareListXmlData(item->text(QMC2_SWLIST_COLUMN_LIST));
		if ( !softwareListXml.isEmpty() ) {
			QTreeWidgetItem *childItem = item->takeChild(0);
			delete childItem;
			QXmlInputSource xmlInputSource;
			xmlInputSource.setData(softwareListXml);
			successfulLookups.clear();
			SoftwareEntryXmlHandler xmlHandler(item);
			QXmlSimpleReader xmlReader;
			xmlReader.setContentHandler(&xmlHandler);
			if ( !xmlReader.parse(xmlInputSource) )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: error while parsing XML data for software list entry '%1:%2'").arg(item->text(QMC2_SWLIST_COLUMN_LIST)).arg(item->text(QMC2_SWLIST_COLUMN_NAME)));
		} else {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: couldn't find XML data for software list '%1'").arg(item->text(QMC2_SWLIST_COLUMN_LIST)));
		}
	}
}

void SoftwareList::on_treeWidgetFavoriteSoftware_itemExpanded(QTreeWidgetItem *item)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_treeWidgetFavoriteSoftware_itemExpanded(QTreeWidgetItem *item = %1)").arg((qulonglong)item));
#endif

	if ( item->child(0)->text(QMC2_SWLIST_COLUMN_TITLE) == tr("Waiting for data...") ) {
		QString softwareListXml = getSoftwareListXmlData(item->text(QMC2_SWLIST_COLUMN_LIST));
		if ( !softwareListXml.isEmpty() ) {
			QTreeWidgetItem *childItem = item->takeChild(0);
			delete childItem;
			QXmlInputSource xmlInputSource;
			xmlInputSource.setData(softwareListXml);
			successfulLookups.clear();
			SoftwareEntryXmlHandler xmlHandler(item);
			QXmlSimpleReader xmlReader;
			xmlReader.setContentHandler(&xmlHandler);
			if ( !xmlReader.parse(xmlInputSource) )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: error while parsing XML data for software list entry '%1:%2'").arg(item->text(QMC2_SWLIST_COLUMN_LIST)).arg(item->text(QMC2_SWLIST_COLUMN_NAME)));
		} else {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: couldn't find XML data for software list '%1'").arg(item->text(QMC2_SWLIST_COLUMN_LIST)));
		}
	}
}

void SoftwareList::on_treeWidgetSearchResults_itemExpanded(QTreeWidgetItem *item)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_treeWidgetSearchResults_itemExpanded(QTreeWidgetItem *item = %1)").arg((qulonglong)item));
#endif

	if ( item->child(0)->text(QMC2_SWLIST_COLUMN_TITLE) == tr("Waiting for data...") ) {
		QString softwareListXml = getSoftwareListXmlData(item->text(QMC2_SWLIST_COLUMN_LIST));
		if ( !softwareListXml.isEmpty() ) {
			QTreeWidgetItem *childItem = item->takeChild(0);
			delete childItem;
			QXmlInputSource xmlInputSource;
			xmlInputSource.setData(softwareListXml);
			successfulLookups.clear();
			SoftwareEntryXmlHandler xmlHandler(item);
			QXmlSimpleReader xmlReader;
			xmlReader.setContentHandler(&xmlHandler);
			if ( !xmlReader.parse(xmlInputSource) )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: error while parsing XML data for software list entry '%1:%2'").arg(item->text(QMC2_SWLIST_COLUMN_LIST)).arg(item->text(QMC2_SWLIST_COLUMN_NAME)));
		} else {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: couldn't find XML data for software list '%1'").arg(item->text(QMC2_SWLIST_COLUMN_LIST)));
		}
	}
}

void SoftwareList::on_toolBoxSoftwareList_currentChanged(int index)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_toolBoxSoftwareList_currentChanged(int index = %1)").arg(index));
#endif

	comboBoxDeviceConfiguration->setCurrentIndex(0);
	switch ( index ) {
		case QMC2_SWLIST_KNOWN_SW_PAGE:
			on_treeWidgetKnownSoftware_itemSelectionChanged();
			break;
		case QMC2_SWLIST_FAVORITES_PAGE:
			on_treeWidgetFavoriteSoftware_itemSelectionChanged();
			break;
		case QMC2_SWLIST_SEARCH_PAGE:
			on_treeWidgetSearchResults_itemSelectionChanged();
			break;
		default:
			break;
	}
}

void SoftwareList::on_treeWidgetKnownSoftware_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::on_treeWidgetKnownSoftware_itemSelectionChanged()");
#endif

	QList<QTreeWidgetItem *> selectedItems = treeWidgetKnownSoftware->selectedItems();
	bool enable = (selectedItems.count() > 0);
	toolButtonPlay->setEnabled(enable);
	toolButtonPlayEmbedded->setEnabled(enable);
	toolButtonAddToFavorites->setEnabled(enable);
	if ( enable && qmc2SoftwareSnap ) {
		SoftwareItem *item = (SoftwareItem *)selectedItems[0];
		if ( item != qmc2SoftwareSnap->myItem )
			cancelSoftwareSnap();
		qmc2SoftwareSnap->snapForcedResetTimer.stop();
		snapForced = true;
		qmc2SoftwareSnap->myItem = item;
		snapTimer.start(QMC2_SWSNAP_DELAY);
	} else
		cancelSoftwareSnap();
}

void SoftwareList::on_treeWidgetFavoriteSoftware_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::on_treeWidgetFavoriteSoftware_itemSelectionChanged()");
#endif

	QList<QTreeWidgetItem *> selectedItems = treeWidgetFavoriteSoftware->selectedItems();
	bool enable = (selectedItems.count() > 0);
	toolButtonPlay->setEnabled(enable);
	toolButtonPlayEmbedded->setEnabled(enable);
	toolButtonRemoveFromFavorites->setEnabled(enable);
	toolButtonAddToFavorites->setEnabled(enable);
	if ( enable && qmc2SoftwareSnap ) {
		SoftwareItem *item = (SoftwareItem *)selectedItems[0];
		if ( item != qmc2SoftwareSnap->myItem )
			cancelSoftwareSnap();
		qmc2SoftwareSnap->snapForcedResetTimer.stop();
		snapForced = true;
		qmc2SoftwareSnap->myItem = item;
		snapTimer.start(QMC2_SWSNAP_DELAY);
	} else
		cancelSoftwareSnap();
	if ( enable ) {
		QTreeWidgetItem *item = selectedItems[0];
		while ( item->parent() ) item = item->parent();
		QString s = item->text(QMC2_SWLIST_COLUMN_DEVICECFG);
		if ( !s.isEmpty() ) {
			int index = comboBoxDeviceConfiguration->findText(s, Qt::MatchExactly | Qt::MatchCaseSensitive);
			if ( index > 0 )
				comboBoxDeviceConfiguration->setCurrentIndex(index);
			else
				comboBoxDeviceConfiguration->setCurrentIndex(0);
		} else
			comboBoxDeviceConfiguration->setCurrentIndex(0);
	}
}

void SoftwareList::on_treeWidgetSearchResults_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::on_treeWidgetSearchResults_itemSelectionChanged()");
#endif

	QList<QTreeWidgetItem *> selectedItems = treeWidgetSearchResults->selectedItems();
	bool enable = (selectedItems.count() > 0);
	toolButtonPlay->setEnabled(enable);
	toolButtonPlayEmbedded->setEnabled(enable);
	toolButtonAddToFavorites->setEnabled(enable);
	toolButtonRemoveFromFavorites->setEnabled(false);
	if ( selectedItems.count() > 0 && qmc2SoftwareSnap ) {
		SoftwareItem *item = (SoftwareItem *)selectedItems[0];
		if ( item != qmc2SoftwareSnap->myItem )
			cancelSoftwareSnap();
		qmc2SoftwareSnap->snapForcedResetTimer.stop();
		snapForced = true;
		qmc2SoftwareSnap->myItem = item;
		snapTimer.start(QMC2_SWSNAP_DELAY);
	} else
		cancelSoftwareSnap();
}

void SoftwareList::on_treeWidgetKnownSoftware_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::on_treeWidgetKnownSoftware_customContextMenuRequested(const QPoint &p = ...)");
#endif

	QTreeWidgetItem *item = treeWidgetKnownSoftware->itemAt(p);

	if ( !item )
		return;

	treeWidgetKnownSoftware->setItemSelected(item, true);
	actionAddToFavorites->setVisible(true);
	actionRemoveFromFavorites->setVisible(false);
	softwareListMenu->move(qmc2MainWindow->adjustedWidgetPosition(treeWidgetKnownSoftware->viewport()->mapToGlobal(p), softwareListMenu));
	softwareListMenu->show();
}

void SoftwareList::on_treeWidgetFavoriteSoftware_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::on_treeWidgetFavoriteSoftware_customContextMenuRequested(const QPoint &p = ...)");
#endif

	QTreeWidgetItem *item = treeWidgetFavoriteSoftware->itemAt(p);

	if ( !item )
		return;

	treeWidgetFavoriteSoftware->setItemSelected(item, true);
	actionAddToFavorites->setVisible(false);
	actionRemoveFromFavorites->setVisible(true);
	softwareListMenu->move(qmc2MainWindow->adjustedWidgetPosition(treeWidgetFavoriteSoftware->viewport()->mapToGlobal(p), softwareListMenu));
	softwareListMenu->show();
}

void SoftwareList::on_treeWidgetSearchResults_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::on_treeWidgetSearchResults_customContextMenuRequested(const QPoint &p = ...)");
#endif

	QTreeWidgetItem *item = treeWidgetSearchResults->itemAt(p);

	if ( !item )
		return;

	treeWidgetSearchResults->setItemSelected(item, true);
	actionAddToFavorites->setVisible(true);
	actionRemoveFromFavorites->setVisible(false);
	softwareListMenu->move(qmc2MainWindow->adjustedWidgetPosition(treeWidgetSearchResults->viewport()->mapToGlobal(p), softwareListMenu));
	softwareListMenu->show();
}

void SoftwareList::cancelSoftwareSnap()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::cancelSoftwareSnap()");
#endif

	snapForced = false;
	snapTimer.stop();
	if ( qmc2SoftwareSnap ) {
		qmc2SoftwareSnap->myItem = NULL;
		qmc2SoftwareSnap->hide();
	}
}

void SoftwareList::on_treeWidgetKnownSoftware_itemEntered(QTreeWidgetItem *item, int column)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_treeWidgetKnownSoftware_itemEntered(QTreeWidgetItem *item = %1, int column = %2)").arg((qulonglong)item).arg(column));
#endif

	if ( !qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SoftwareSnapOnMouseHover").toBool() )
		return;

	if ( !snapForced ) {
		if ( qmc2SoftwareSnap ) {
			if ( qmc2SoftwareSnap->myItem != item ) {
				cancelSoftwareSnap();
				qmc2SoftwareSnap->myItem = (SoftwareItem *)item;
				snapTimer.start(QMC2_SWSNAP_DELAY);
			}
		}
	}
}

void SoftwareList::on_treeWidgetFavoriteSoftware_itemEntered(QTreeWidgetItem *item, int column)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_treeWidgetFavoriteSoftware_itemEntered(QTreeWidgetItem *item = %1, int column = %2)").arg((qulonglong)item).arg(column));
#endif

	if ( !qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SoftwareSnapOnMouseHover").toBool() )
		return;

	if ( !snapForced ) {
		if ( qmc2SoftwareSnap ) {
			if ( qmc2SoftwareSnap->myItem != item ) {
				cancelSoftwareSnap();
				qmc2SoftwareSnap->myItem = (SoftwareItem *)item;
				snapTimer.start(QMC2_SWSNAP_DELAY);
			}
		}
	}
}

void SoftwareList::on_treeWidgetSearchResults_itemEntered(QTreeWidgetItem *item, int column)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_treeWidgetSearchResults_itemEntered(QTreeWidgetItem *item = %1, int column = %2)").arg((qulonglong)item).arg(column));
#endif

	if ( !qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SoftwareSnapOnMouseHover").toBool() )
		return;

	if ( !snapForced ) {
		if ( qmc2SoftwareSnap ) {
			if ( qmc2SoftwareSnap->myItem != item ) {
				cancelSoftwareSnap();
				qmc2SoftwareSnap->myItem = (SoftwareItem *)item;
				snapTimer.start(QMC2_SWSNAP_DELAY);
			}
		}
	}
}

void SoftwareList::on_treeWidgetKnownSoftware_itemActivated(QTreeWidgetItem *item, int column)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_treeWidgetKnownSoftware_itemActivated(QTreeWidgetItem *item = %1, int column = %2)").arg((qulonglong)item).arg(column));
#endif

	cancelSoftwareSnap();
	QTimer::singleShot(0, this, SLOT(playActivated()));
}

void SoftwareList::on_treeWidgetFavoriteSoftware_itemActivated(QTreeWidgetItem *item, int column)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_treeWidgetFavoriteSoftware_itemActivated(QTreeWidgetItem *item = %1, int column = %2)").arg((qulonglong)item).arg(column));
#endif

	cancelSoftwareSnap();
	QTimer::singleShot(0, this, SLOT(playActivated()));
}

void SoftwareList::on_treeWidgetSearchResults_itemActivated(QTreeWidgetItem *item, int column)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_treeWidgetSearchResults_itemActivated(QTreeWidgetItem *item = %1, int column = %2)").arg((qulonglong)item).arg(column));
#endif

	cancelSoftwareSnap();
	QTimer::singleShot(0, this, SLOT(playActivated()));
}

void SoftwareList::on_comboBoxSearch_textChanged(QString)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::on_comboBoxSearch_textChanged(QString)");
#endif

	searchTimer.start(QMC2_SEARCH_DELAY);
}

void SoftwareList::comboBoxSearch_textChanged_delayed()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::comboBoxSearch_textChanged_delayed()");
#endif

	searchTimer.stop();

	QString pattern = comboBoxSearch->currentText();

	// easy pattern match
	if ( !pattern.isEmpty() ) {
		pattern = "*" + pattern.replace(' ', "* *") + "*";
		pattern.replace(QString("*^"), "");
		pattern.replace(QString("$*"), "");
	}

	treeWidgetSearchResults->clear();

	QList<QTreeWidgetItem *> matches = treeWidgetKnownSoftware->findItems(pattern, Qt::MatchContains | Qt::MatchWildcard, QMC2_SWLIST_COLUMN_TITLE);
	QList<QTreeWidgetItem *> matchesByShortName = treeWidgetKnownSoftware->findItems(pattern, Qt::MatchContains | Qt::MatchWildcard, QMC2_SWLIST_COLUMN_NAME);

	int i;

	for (i = 0; i < matchesByShortName.count(); i++) {
		QTreeWidgetItem *item = matchesByShortName[i];
		if ( !matches.contains(item) )
			matches.append(item);
	}

	for (i = 0; i < matches.count(); i++) {
		SoftwareItem *item = new SoftwareItem(treeWidgetSearchResults);
		SoftwareItem *subItem = new SoftwareItem(item);
		subItem->setText(QMC2_SWLIST_COLUMN_TITLE, tr("Waiting for data..."));
		QTreeWidgetItem *matchItem = matches.at(i);
		item->setText(QMC2_SWLIST_COLUMN_TITLE, matchItem->text(QMC2_SWLIST_COLUMN_TITLE));
		item->setText(QMC2_SWLIST_COLUMN_NAME, matchItem->text(QMC2_SWLIST_COLUMN_NAME));
		item->setText(QMC2_SWLIST_COLUMN_PUBLISHER, matchItem->text(QMC2_SWLIST_COLUMN_PUBLISHER));
		item->setText(QMC2_SWLIST_COLUMN_YEAR, matchItem->text(QMC2_SWLIST_COLUMN_YEAR));
		item->setText(QMC2_SWLIST_COLUMN_PART, matchItem->text(QMC2_SWLIST_COLUMN_PART));
		item->setText(QMC2_SWLIST_COLUMN_INTERFACE, matchItem->text(QMC2_SWLIST_COLUMN_INTERFACE));
		item->setText(QMC2_SWLIST_COLUMN_LIST, matchItem->text(QMC2_SWLIST_COLUMN_LIST));
	}

	if ( autoSelectSearchItem ) {
  		treeWidgetSearchResults->setFocus();
		if ( treeWidgetSearchResults->currentItem() )
			treeWidgetSearchResults->currentItem()->setSelected(true);
	}

	autoSelectSearchItem = false;
}

void SoftwareList::on_comboBoxSearch_activated(QString pattern)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_comboBoxSearch_activated(QString pattern = %1)").arg(pattern));
#endif

	autoSelectSearchItem = true;
	comboBoxSearch_textChanged_delayed();
}

QStringList &SoftwareList::arguments()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::arguments()");
#endif

	static QStringList swlArgs;

	swlArgs.clear();

	// arguments to start a software list entry
	QTreeWidget *treeWidget;
	switch ( toolBoxSoftwareList->currentIndex() ) {
		case QMC2_SWLIST_FAVORITES_PAGE:
			treeWidget = treeWidgetFavoriteSoftware;
			break;
		case QMC2_SWLIST_SEARCH_PAGE:
			treeWidget = treeWidgetSearchResults;
			break;
		case QMC2_SWLIST_KNOWN_SW_PAGE:
		default:
			treeWidget = treeWidgetKnownSoftware;
			break;
	}

	QList<QTreeWidgetItem *> selectedItems = treeWidget->selectedItems();

	if ( selectedItems.count() > 0 ) {
		QTreeWidgetItemIterator it(treeWidget);
		QStringList manualMounts;
		if ( !autoMounted ) {
			// manually mounted
			while ( *it ) {
				QComboBox *comboBox = (QComboBox *)treeWidget->itemWidget(*it, QMC2_SWLIST_COLUMN_PUBLISHER);
				if ( comboBox ) {
					if ( comboBox->currentIndex() > QMC2_SWLIST_MSEL_DONT_MOUNT ) {
						swlArgs << QString("-%1").arg(comboBox->currentText());
						QTreeWidgetItem *item = *it;
						while ( item->parent() ) item = item->parent();
						swlArgs << QString("%1:%2:%3").arg((*it)->text(QMC2_SWLIST_COLUMN_LIST)).arg(item->text(QMC2_SWLIST_COLUMN_NAME)).arg((*it)->text(QMC2_SWLIST_COLUMN_PART));
					}
				}
				it++;
			}
		} else {
			// automatically mounted
			QTreeWidgetItem *item = selectedItems[0];
			while ( item->parent() ) item = item->parent();
			QStringList interfaces = item->text(QMC2_SWLIST_COLUMN_INTERFACE).split(",");
			QStringList parts = item->text(QMC2_SWLIST_COLUMN_PART).split(",");
			successfulLookups.clear();
			for (int i = 0; i < parts.count(); i++) {
				QString mountDev = lookupMountDevice(parts[i], interfaces[i]);
				if ( !mountDev.isEmpty() ) {
					swlArgs << QString("-%1").arg(mountDev);
					swlArgs << QString("%1:%2:%3").arg(item->text(QMC2_SWLIST_COLUMN_LIST)).arg(item->text(QMC2_SWLIST_COLUMN_NAME)).arg(parts[i]);
				}
			}
		}
	}

#if defined(QMC2_EMUTYPE_MESS)
	// optionally add arguments for the selected device configuration
	QString devConfigName = comboBoxDeviceConfiguration->currentText();
	if ( devConfigName != tr("No additional devices") ) {
		qmc2Config->beginGroup(QString("MESS/Configuration/Devices/%1/%2").arg(systemName).arg(devConfigName));
		QStringList instances = qmc2Config->value("Instances").toStringList();
		QStringList files = qmc2Config->value("Files").toStringList();
		QStringList slotNames = qmc2Config->value("Slots").toStringList();
		QStringList slotOptions = qmc2Config->value("SlotOptions").toStringList();
		qmc2Config->endGroup();
		for (int i = 0; i < instances.count(); i++) {
#if defined(Q_WS_WIN)
			swlArgs << QString("-%1").arg(instances[i]) << files[i].replace('/', '\\');
#else
			swlArgs << QString("-%1").arg(instances[i]) << files[i].replace("~", "$HOME");
#endif
		}
		for (int i = 0; i < slotNames.count(); i++)
			swlArgs << QString("-%1").arg(slotNames[i]) << slotOptions[i];
	}
#endif

	return swlArgs;
}

void SoftwareList::treeWidgetKnownSoftwareHeader_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::treeWidgetKnownSoftwareHeader_customContextMenuRequested(const QPoint &p = ...)");
#endif

	menuKnownSoftwareHeader->move(qmc2MainWindow->adjustedWidgetPosition(treeWidgetKnownSoftware->header()->viewport()->mapToGlobal(p), menuKnownSoftwareHeader));
	menuKnownSoftwareHeader->show();
}

void SoftwareList::actionKnownSoftwareHeader_triggered()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::actionKnownSoftwareHeader_triggered()");
#endif

	QAction *action = (QAction *)sender();
	int visibleColumns = 0;
	for (int i = 0; i < treeWidgetKnownSoftware->columnCount(); i++) if ( !treeWidgetKnownSoftware->isColumnHidden(i) ) visibleColumns++;
	if ( action ) {
		bool visibility = true;
		if ( action->isChecked() )
			treeWidgetKnownSoftware->setColumnHidden(action->data().toInt(), false);
		else if ( visibleColumns > 1 ) {
			treeWidgetKnownSoftware->setColumnHidden(action->data().toInt(), true);
			visibility = false;
		}
		action->blockSignals(true);
		action->setChecked(visibility);
		action->blockSignals(false);
	}
}

void SoftwareList::treeWidgetFavoriteSoftwareHeader_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::treeWidgetFavoriteSoftwareHeader_customContextMenuRequested(const QPoint &p = ...)");
#endif

	menuFavoriteSoftwareHeader->move(qmc2MainWindow->adjustedWidgetPosition(treeWidgetFavoriteSoftware->header()->viewport()->mapToGlobal(p), menuFavoriteSoftwareHeader));
	menuFavoriteSoftwareHeader->show();
}

void SoftwareList::actionFavoriteSoftwareHeader_triggered()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::actionFavoriteSoftwareHeader_triggered()");
#endif

	QAction *action = (QAction *)sender();
	int visibleColumns = 0;
	for (int i = 0; i < treeWidgetFavoriteSoftware->columnCount(); i++) if ( !treeWidgetFavoriteSoftware->isColumnHidden(i) ) visibleColumns++;
	if ( action ) {
		bool visibility = true;
		if ( action->isChecked() )
			treeWidgetFavoriteSoftware->setColumnHidden(action->data().toInt(), false);
		else if ( visibleColumns > 1 ) {
			treeWidgetFavoriteSoftware->setColumnHidden(action->data().toInt(), true);
			visibility = false;
		}
		action->blockSignals(true);
		action->setChecked(visibility);
		action->blockSignals(false);
	}
}

void SoftwareList::treeWidgetSearchResultsHeader_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::treeWidgetSearchResultsHeader_customContextMenuRequested(const QPoint &p = ...)");
#endif

	menuSearchResultsHeader->move(qmc2MainWindow->adjustedWidgetPosition(treeWidgetSearchResults->header()->viewport()->mapToGlobal(p), menuSearchResultsHeader));
	menuSearchResultsHeader->show();
}

void SoftwareList::actionSearchResultsHeader_triggered()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::actionSearchResultsHeader_triggered()");
#endif

	QAction *action = (QAction *)sender();
	int visibleColumns = 0;
	for (int i = 0; i < treeWidgetSearchResults->columnCount(); i++) if ( !treeWidgetSearchResults->isColumnHidden(i) ) visibleColumns++;
	if ( action ) {
		bool visibility = true;
		if ( action->isChecked() )
			treeWidgetSearchResults->setColumnHidden(action->data().toInt(), false);
		else if ( visibleColumns > 1 ) {
			treeWidgetSearchResults->setColumnHidden(action->data().toInt(), true);
			visibility = false;
		}
		action->blockSignals(true);
		action->setChecked(visibility);
		action->blockSignals(false);
	}
}

void SoftwareList::checkMountDeviceSelection()
{
	QComboBox *comboBoxSender = (QComboBox *)sender();

#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::checkMountDeviceSelection()");
#endif

	QTreeWidget *treeWidget;
	switch ( qmc2SoftwareList->toolBoxSoftwareList->currentIndex() ) {
		case QMC2_SWLIST_KNOWN_SW_PAGE:
			treeWidget = treeWidgetKnownSoftware;
			break;
		case QMC2_SWLIST_FAVORITES_PAGE:
			treeWidget = treeWidgetFavoriteSoftware;
			break;
		case QMC2_SWLIST_SEARCH_PAGE:
			treeWidget = treeWidgetSearchResults;
			break;
	}

	QString mountDevice = comboBoxSender->currentText();

	QTreeWidgetItemIterator it(treeWidget);

	if ( mountDevice == QObject::tr("Auto mount") ) {
		while ( *it ) {
			if ( !(*it)->parent() ) successfulLookups.clear();
			QComboBox *comboBox = (QComboBox *)treeWidget->itemWidget(*it, QMC2_SWLIST_COLUMN_PUBLISHER);
			if ( comboBox ) {
				comboBox->blockSignals(true);
				comboBox->setCurrentIndex(QMC2_SWLIST_MSEL_AUTO_MOUNT); // => auto mount
				comboBox->blockSignals(false);
				QString itemMountDev = lookupMountDevice((*it)->text(QMC2_SWLIST_COLUMN_PART), (*it)->text(QMC2_SWLIST_COLUMN_INTERFACE));
				if ( itemMountDev.isEmpty() )
					(*it)->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("Not mounted"));
				else
					(*it)->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("Mounted on:") + " " + itemMountDev);
			}
			it++;
		}
		autoMounted = true;
	} else if ( mountDevice == QObject::tr("Don't mount") ) {
		while ( *it ) {
			QComboBox *comboBox = (QComboBox *)treeWidget->itemWidget(*it, QMC2_SWLIST_COLUMN_PUBLISHER);
			if ( comboBox ) {
				if ( comboBox == comboBoxSender )
					(*it)->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("Not mounted"));
				else if ( comboBox->currentIndex() == QMC2_SWLIST_MSEL_AUTO_MOUNT ) {
					comboBox->blockSignals(true);
					comboBox->setCurrentIndex(QMC2_SWLIST_MSEL_DONT_MOUNT); // => don't mount
					comboBox->blockSignals(false);
					(*it)->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("Not mounted"));
				}
			}
			it++;
		}
		autoMounted = false;
	} else {
		while ( *it ) {
			QComboBox *comboBox = (QComboBox *)treeWidget->itemWidget(*it, QMC2_SWLIST_COLUMN_PUBLISHER);
			if ( comboBox ) {
				if ( comboBox != comboBoxSender ) {
					if ( comboBox->currentText() == mountDevice || comboBox->currentText() == QObject::tr("Auto mount") ) {
						comboBox->blockSignals(true);
						comboBox->setCurrentIndex(QMC2_SWLIST_MSEL_DONT_MOUNT); // => don't mount
						comboBox->blockSignals(false);
						(*it)->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("Not mounted"));
					}
				} else
					(*it)->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("Mounted on:") + " " + mountDevice);
			}
			it++;
		}
		autoMounted = false;
	}
}

SoftwareListXmlHandler::SoftwareListXmlHandler(QTreeWidget *parent)
{
#ifdef QMC2_DEBUG
//	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareListXmlHandler::SoftwareListXmlHandler(QTreeWidget *parent = %1)").arg((qulonglong)parent));
#endif

	parentTreeWidget = parent;
}

SoftwareListXmlHandler::~SoftwareListXmlHandler()
{
#ifdef QMC2_DEBUG
//	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareListXmlHandler::~SoftwareListXmlHandler()");
#endif

}

bool SoftwareListXmlHandler::startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &attributes)
{
#ifdef QMC2_DEBUG
//	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareListXmlHandler::startElement(const QString &namespaceURI = ..., const QString &localName = %1, const QString &qName = %2, const QXmlAttributes &attributes = ...)").arg(localName).arg(qName));
#endif

	if ( qName == "softwarelist" ) {
		softwareListName = attributes.value("name");
	} else if ( qName == "software" ) {
		softwareName = attributes.value("name");
		softwareItem = new SoftwareItem(parentTreeWidget);
		SoftwareItem *subItem = new SoftwareItem(softwareItem);
		subItem->setText(QMC2_SWLIST_COLUMN_TITLE, QObject::tr("Waiting for data..."));
		softwareItem->setText(QMC2_SWLIST_COLUMN_NAME, softwareName);
		softwareItem->setText(QMC2_SWLIST_COLUMN_LIST, softwareListName);
	} else if ( qName == "part" ) {
		softwarePart = attributes.value("name");
		QString parts = softwareItem->text(QMC2_SWLIST_COLUMN_PART);
		softwareInterface = attributes.value("interface");
		QString interfaces = softwareItem->text(QMC2_SWLIST_COLUMN_INTERFACE);
		if ( parts.isEmpty() )
			softwareItem->setText(QMC2_SWLIST_COLUMN_PART, softwarePart);
		else
			softwareItem->setText(QMC2_SWLIST_COLUMN_PART, parts + "," + softwarePart);
		if ( interfaces.isEmpty() )
			softwareItem->setText(QMC2_SWLIST_COLUMN_INTERFACE, softwareInterface);
		else
			softwareItem->setText(QMC2_SWLIST_COLUMN_INTERFACE, interfaces + "," + softwareInterface);
	} else if ( qName == "description" || qName == "year" || qName == "publisher" ) {
		currentText.clear();
	}

	return true;
}

bool SoftwareListXmlHandler::endElement(const QString &namespaceURI, const QString &localName, const QString &qName)
{
#ifdef QMC2_DEBUG
//	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareListXmlHandler::endElement(const QString &namespaceURI = ..., const QString &localName = %1, const QString &qName = %2)").arg(localName).arg(qName));
#endif

	if ( qName == "description" ) {
		softwareTitle = currentText;
		softwareItem->setText(QMC2_SWLIST_COLUMN_TITLE, softwareTitle);
	} else if ( qName == "year" ) {
		softwareYear = currentText;
		softwareItem->setText(QMC2_SWLIST_COLUMN_YEAR, softwareYear);
	} else if ( qName == "publisher" ) {
		softwarePublisher = currentText;
		softwareItem->setText(QMC2_SWLIST_COLUMN_PUBLISHER, softwarePublisher);
	}

	return true;
}

bool SoftwareListXmlHandler::characters(const QString &str)
{
#ifdef QMC2_DEBUG
//	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareListXmlHandler::characters(const QString &str = ...)"));
#endif

	currentText += QString::fromUtf8(str.toAscii());
	return true;
}

SoftwareSnap::SoftwareSnap(QWidget *parent)
	: QWidget(parent, Qt::Tool | Qt::CustomizeWindowHint | Qt::FramelessWindowHint)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareSnap::SoftwareSnap(QWidget *parent = %1)").arg((qulonglong)parent));
#endif

	setWindowTitle(tr("Snapshot viewer"));
	setFocusPolicy(Qt::NoFocus);
	focusWidget = QApplication::focusWidget();
	snapForcedResetTimer.setSingleShot(true);
	connect(&snapForcedResetTimer, SIGNAL(timeout()), this, SLOT(resetSnapForced()));

	ctxMenuRequested = false;
	contextMenu = new QMenu(this);
	contextMenu->hide();
	
	QString s;
	QAction *action;

	s = tr("Copy to clipboard");
	action = contextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/editcopy.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(copyToClipboard()));
}

void SoftwareSnap::keyPressEvent(QKeyEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareSnap::keyPressEvent(QKeyEvent *e = %1)").arg((qulonglong)e));
#endif

	// pass the key press event to the software list (to allow for clean cursor movement)
	if ( focusWidget ) {
		QKeyEvent *keyEvent = new QKeyEvent(QEvent::KeyPress, e->key(), e->modifiers(), e->text(), e->isAutoRepeat(), e->count());
		qApp->postEvent(focusWidget, keyEvent);
		if ( qmc2SoftwareList ) {
			qmc2SoftwareList->snapForced = true;
			myItem = NULL;
			snapForcedResetTimer.start(QMC2_SWSNAP_UNFORCE_DELAY);
		}
	} else if ( qmc2SoftwareList ) {
		switch ( qmc2SoftwareList->toolBoxSoftwareList->currentIndex() ) {
			case QMC2_SWLIST_KNOWN_SW_PAGE:
				focusWidget = qmc2SoftwareList->treeWidgetKnownSoftware;
				break;
			case QMC2_SWLIST_FAVORITES_PAGE:
				focusWidget = qmc2SoftwareList->treeWidgetFavoriteSoftware;
				break;
			case QMC2_SWLIST_SEARCH_PAGE:
				focusWidget = qmc2SoftwareList->treeWidgetSearchResults;
				break;
		}
		if ( focusWidget ) {
			QKeyEvent *keyEvent = new QKeyEvent(QEvent::KeyPress, e->key(), e->modifiers(), e->text(), e->isAutoRepeat(), e->count());
			qApp->postEvent(focusWidget, keyEvent);
			qmc2SoftwareList->snapForced = true;
			myItem = NULL;
			snapForcedResetTimer.start(QMC2_SWSNAP_UNFORCE_DELAY);
		}
	} else
		hide();

	e->ignore();
}

void SoftwareSnap::mousePressEvent(QMouseEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareSnap::mousePressEvent(QMouseEvent *e = %1)").arg((qulonglong)e));
#endif

	if ( e->button() != Qt::RightButton) {
		hide();
		resetSnapForced();
	} else
		ctxMenuRequested = true;
}

void SoftwareSnap::leaveEvent(QEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareSnap::leaveEvent(QEvent *e = %1)").arg((qulonglong)e));
#endif

	if ( !qmc2SoftwareList->snapForced && !ctxMenuRequested ) hide();
	QWidget::leaveEvent(e);
	ctxMenuRequested = false;
}

void SoftwareSnap::paintEvent(QPaintEvent *e)
{
	QPainter p(this);
	p.eraseRect(rect());
	p.end();
}

void SoftwareSnap::loadSnapshot()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareSnap::loadSnapshot()");
#endif

	ctxMenuRequested = false;

	if ( !qmc2SoftwareList || qmc2SoftwareSnapPosition == QMC2_SWSNAP_POS_DISABLE_SNAPS ) {
		myItem = NULL;
		resetSnapForced();
		return;
	}

#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareSnap::loadSnapshot(): snapForced = '%1'").arg(qmc2SoftwareList->snapForced ? "true" : "false"));
#endif

	// check if the mouse cursor is still on a software item
	QTreeWidgetItem *item = NULL;
	QRect rect;
	switch ( qmc2SoftwareList->toolBoxSoftwareList->currentIndex() ) {
		case QMC2_SWLIST_KNOWN_SW_PAGE:
			if ( qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->underMouse() ) {
				item = qmc2SoftwareList->treeWidgetKnownSoftware->itemAt(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->mapFromGlobal(QCursor::pos()));
				if ( item ) {
					rect = qmc2SoftwareList->treeWidgetKnownSoftware->visualItemRect(item);
					rect.setWidth(MIN(rect.width(), qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->width()));
					rect.translate(0, 4);
					position = qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->mapToGlobal(rect.bottomLeft());
				}
			}
			break;
		case QMC2_SWLIST_FAVORITES_PAGE:
			if ( qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->underMouse() ) {
				item = qmc2SoftwareList->treeWidgetFavoriteSoftware->itemAt(qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->mapFromGlobal(QCursor::pos()));
				if ( item ) {
					rect = qmc2SoftwareList->treeWidgetFavoriteSoftware->visualItemRect(item);
					rect.setWidth(MIN(rect.width(), qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->width()));
					rect.translate(0, 4);
					position = qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->mapToGlobal(rect.bottomLeft());
				}
			}
			break;
		case QMC2_SWLIST_SEARCH_PAGE:
			if ( qmc2SoftwareList->treeWidgetSearchResults->viewport()->underMouse() ) {
				item = qmc2SoftwareList->treeWidgetSearchResults->itemAt(qmc2SoftwareList->treeWidgetSearchResults->viewport()->mapFromGlobal(QCursor::pos()));
				if ( item ) {
					rect = qmc2SoftwareList->treeWidgetSearchResults->visualItemRect(item);
					rect.setWidth(MIN(rect.width(), qmc2SoftwareList->treeWidgetSearchResults->viewport()->width()));
					rect.translate(0, 4);
					position = qmc2SoftwareList->treeWidgetSearchResults->viewport()->mapToGlobal(rect.bottomLeft());
				}
			}
			break;
	}

	// try to fall back to 'selected item' if applicable (no mouse hover)
	if ( !item || qmc2SoftwareList->snapForced ) {
		if ( qmc2SoftwareList->snapForced && myItem != NULL ) {
			item = myItem;
			switch ( qmc2SoftwareList->toolBoxSoftwareList->currentIndex() ) {
				case QMC2_SWLIST_KNOWN_SW_PAGE:
					rect = qmc2SoftwareList->treeWidgetKnownSoftware->visualItemRect(item);
					rect.setWidth(MIN(rect.width(), qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->width()));
					rect.translate(0, 4);
					position = qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->mapToGlobal(rect.bottomLeft());
					break;
				case QMC2_SWLIST_FAVORITES_PAGE:
					rect = qmc2SoftwareList->treeWidgetFavoriteSoftware->visualItemRect(item);
					rect.setWidth(MIN(rect.width(), qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->width()));
					rect.translate(0, 4);
					position = qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->mapToGlobal(rect.bottomLeft());
					break;
				case QMC2_SWLIST_SEARCH_PAGE:
					rect = qmc2SoftwareList->treeWidgetSearchResults->visualItemRect(item);
					rect.setWidth(MIN(rect.width(), qmc2SoftwareList->treeWidgetSearchResults->viewport()->width()));
					rect.translate(0, 4);
					position = qmc2SoftwareList->treeWidgetSearchResults->viewport()->mapToGlobal(rect.bottomLeft());
					break;
			}
		}
	}

	// if we can't figure out which item we're on, let's escape from here...
	if ( !item ) {
		myItem = NULL;
		resetSnapForced();
		qmc2SoftwareList->cancelSoftwareSnap();
		return;
	}

	listName = item->text(QMC2_SWLIST_COLUMN_LIST);
	entryName = item->text(QMC2_SWLIST_COLUMN_NAME);
	myItem = (SoftwareItem *)item;

	QPixmap pm;
	bool pmLoaded = QPixmapCache::find("sws_" + listName + "_" + entryName, &pm);

	if ( !pmLoaded ) {
		if ( qmc2UseSoftwareSnapFile ) {
			// FIXME: add ZIP support here
		} else {
#if defined(QMC2_EMUTYPE_MAME)
			QDir snapDir(qmc2Config->value("MAME/FilesAndDirectories/SoftwareSnapDirectory").toString() + "/" + listName);
#elif defined(QMC2_EMUTYPE_MESS)
			QDir snapDir(qmc2Config->value("MESS/FilesAndDirectories/SoftwareSnapDirectory").toString() + "/" + listName);
#endif
#ifdef QMC2_DEBUG
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareSnap::loadSnapshot(): trying to load software snapshot from '%1'").arg(snapDir.absoluteFilePath(entryName + ".png")));
#endif
			if ( snapDir.exists(entryName + ".png") ) {
				QString filePath = snapDir.absoluteFilePath(entryName + ".png");
				if ( pm.load(filePath) ) {
					pmLoaded = true;
					QPixmapCache::insert("sws_" + listName + "_" + entryName, pm); 
#ifdef QMC2_DEBUG
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareSnap::loadSnapshot(): software snapshot loaded successfully from '%1'").arg(snapDir.absoluteFilePath(entryName + ".png")));
#endif
				}
#ifdef QMC2_DEBUG
				else
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareSnap::loadSnapshot(): software snapshot load failed, file = '%1'").arg(snapDir.absoluteFilePath(entryName + ".png")));
#endif
			}
#ifdef QMC2_DEBUG
			else
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareSnap::loadSnapshot(): software snapshot not found, file = '%1'").arg(snapDir.absoluteFilePath(entryName + ".png")));
#endif
		}
	}

	focusWidget = QApplication::focusWidget();

	if ( pmLoaded ) {
		resize(pm.size());
		switch ( qmc2SoftwareSnapPosition ) {
			case QMC2_SWSNAP_POS_ABOVE_CENTER:
				rect.translate(0, -4);
				switch ( qmc2SoftwareList->toolBoxSoftwareList->currentIndex() ) {
					case QMC2_SWLIST_KNOWN_SW_PAGE:
						position.setX(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->mapToGlobal(rect.center()).x() - width() / 2);
						position.setY(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->mapToGlobal(rect.topLeft()).y() - height() - 4);
						break;
					case QMC2_SWLIST_FAVORITES_PAGE:
						position.setX(qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->mapToGlobal(rect.center()).x() - width() / 2);
						position.setY(qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->mapToGlobal(rect.topLeft()).y() - height() - 4);
						break;
					case QMC2_SWLIST_SEARCH_PAGE:
						position.setX(qmc2SoftwareList->treeWidgetSearchResults->viewport()->mapToGlobal(rect.center()).x() - width() / 2);
						position.setY(qmc2SoftwareList->treeWidgetSearchResults->viewport()->mapToGlobal(rect.topLeft()).y() - height() - 4);
						break;
				}
				break;

			case QMC2_SWSNAP_POS_ABOVE_RIGHT:
				rect.translate(0, -4);
				switch ( qmc2SoftwareList->toolBoxSoftwareList->currentIndex() ) {
					case QMC2_SWLIST_KNOWN_SW_PAGE:
						position.setX(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->mapToGlobal(rect.bottomRight()).x() - width());
						position.setY(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->mapToGlobal(rect.topLeft()).y() - height() - 4);
						break;
					case QMC2_SWLIST_FAVORITES_PAGE:
						position.setX(qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->mapToGlobal(rect.bottomRight()).x() - width());
						position.setY(qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->mapToGlobal(rect.topLeft()).y() - height() - 4);
						break;
					case QMC2_SWLIST_SEARCH_PAGE:
						position.setX(qmc2SoftwareList->treeWidgetSearchResults->viewport()->mapToGlobal(rect.bottomRight()).x() - width());
						position.setY(qmc2SoftwareList->treeWidgetSearchResults->viewport()->mapToGlobal(rect.topLeft()).y() - height() - 4);
						break;
				}
				break;

			case QMC2_SWSNAP_POS_ABOVE_LEFT:
				rect.translate(0, -4);
				switch ( qmc2SoftwareList->toolBoxSoftwareList->currentIndex() ) {
					case QMC2_SWLIST_KNOWN_SW_PAGE:
						position.setY(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->mapToGlobal(rect.topLeft()).y() - height() - 4);
						break;
					case QMC2_SWLIST_FAVORITES_PAGE:
						position.setY(qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->mapToGlobal(rect.topLeft()).y() - height() - 4);
						break;
					case QMC2_SWLIST_SEARCH_PAGE:
						position.setY(qmc2SoftwareList->treeWidgetSearchResults->viewport()->mapToGlobal(rect.topLeft()).y() - height() - 4);
						break;
				}
				break;

			case QMC2_SWSNAP_POS_BELOW_RIGHT:
				switch ( qmc2SoftwareList->toolBoxSoftwareList->currentIndex() ) {
					case QMC2_SWLIST_KNOWN_SW_PAGE:
						position.setX(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->mapToGlobal(rect.bottomRight()).x() - width());
						break;
					case QMC2_SWLIST_FAVORITES_PAGE:
						position.setX(qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->mapToGlobal(rect.bottomRight()).x() - width());
						break;
					case QMC2_SWLIST_SEARCH_PAGE:
						position.setX(qmc2SoftwareList->treeWidgetSearchResults->viewport()->mapToGlobal(rect.bottomRight()).x() - width());
						break;
				}
				break;

			case QMC2_SWSNAP_POS_BELOW_CENTER:
				switch ( qmc2SoftwareList->toolBoxSoftwareList->currentIndex() ) {
					case QMC2_SWLIST_KNOWN_SW_PAGE:
						position.setX(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->mapToGlobal(rect.center()).x() - width() / 2);
						break;
					case QMC2_SWLIST_FAVORITES_PAGE:
						position.setX(qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->mapToGlobal(rect.center()).x() - width() / 2);
						break;
					case QMC2_SWLIST_SEARCH_PAGE:
						position.setX(qmc2SoftwareList->treeWidgetSearchResults->viewport()->mapToGlobal(rect.center()).x() - width() / 2);
						break;
				}
				break;

			case QMC2_SWSNAP_POS_BELOW_LEFT:
			default:
				// already prepared above...
				break;
		}
		move(position);
		QPalette pal = palette();
		QPainter p;
		p.begin(&pm);
		p.setPen(QPen(QColor(0, 0, 0, 64), 1));
		rect = pm.rect();
		rect.setWidth(rect.width() - 1);
		rect.setHeight(rect.height() - 1);
		p.drawRect(rect);
		p.end();
		pal.setBrush(QPalette::Window, pm);
		setPalette(pal);
		showNormal();
		update();
		raise();
		snapForcedResetTimer.start(QMC2_SWSNAP_UNFORCE_DELAY);
	} else {
		myItem = NULL;
		resetSnapForced();
		qmc2SoftwareList->cancelSoftwareSnap();
	}
}

void SoftwareSnap::resetSnapForced()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareSnap::resetSnapForced()");
#endif

	if ( qmc2SoftwareList ) {
		QTreeWidgetItem *item = NULL;
		if ( !qmc2SoftwareList->snapForced ) {
			switch ( qmc2SoftwareList->toolBoxSoftwareList->currentIndex() ) {
				case QMC2_SWLIST_KNOWN_SW_PAGE:
					if ( qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->underMouse() ) {
						item = qmc2SoftwareList->treeWidgetKnownSoftware->itemAt(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->mapFromGlobal(QCursor::pos()));
						if ( item )
							if ( item != myItem || item->text(QMC2_SWLIST_COLUMN_NAME) != entryName  ) {
								qmc2SoftwareList->cancelSoftwareSnap();
								qmc2SoftwareList->snapTimer.start(QMC2_SWSNAP_DELAY);
							}
					}
					break;
				case QMC2_SWLIST_FAVORITES_PAGE:
					if ( qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->underMouse() ) {
						item = qmc2SoftwareList->treeWidgetFavoriteSoftware->itemAt(qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->mapFromGlobal(QCursor::pos()));
						if ( item )
							if ( item != myItem || item->text(QMC2_SWLIST_COLUMN_NAME) != entryName  ) {
								qmc2SoftwareList->cancelSoftwareSnap();
								qmc2SoftwareList->snapTimer.start(QMC2_SWSNAP_DELAY);
							}
					}
					break;
				case QMC2_SWLIST_SEARCH_PAGE:
					if ( qmc2SoftwareList->treeWidgetSearchResults->viewport()->underMouse() ) {
						item = qmc2SoftwareList->treeWidgetSearchResults->itemAt(qmc2SoftwareList->treeWidgetSearchResults->viewport()->mapFromGlobal(QCursor::pos()));
						if ( item )
							if ( item != myItem || item->text(QMC2_SWLIST_COLUMN_NAME) != entryName  ) {
								qmc2SoftwareList->cancelSoftwareSnap();
								qmc2SoftwareList->snapTimer.start(QMC2_SWSNAP_DELAY);
							}
					}
					break;
			}
		}
	}
	qmc2SoftwareList->snapForced = false;
}

void SoftwareSnap::contextMenuEvent(QContextMenuEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareSnap::contextMenuEvent(QContextMenuEvent *e = %1)").arg((qulonglong)e));
#endif

	contextMenu->move(qmc2MainWindow->adjustedWidgetPosition(mapToGlobal(e->pos()), contextMenu));
	contextMenu->show();
}

void SoftwareSnap::copyToClipboard()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareSnap::copyToClipboard()");
#endif

	QPixmap pm(size());
	render(&pm);
	qApp->clipboard()->setPixmap(pm);
}

SoftwareEntryXmlHandler::SoftwareEntryXmlHandler(QTreeWidgetItem *item)
{
#ifdef QMC2_DEBUG
//	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareEntryXmlHandler::SoftwareListXmlHandler(QTreeWidgetItem *item = %1)").arg((qulonglong)item));
#endif

	parentTreeWidgetItem = (SoftwareItem *)item;
	softwareValid = false;
	partItem = dataareaItem = romItem = NULL;
}

SoftwareEntryXmlHandler::~SoftwareEntryXmlHandler()
{
#ifdef QMC2_DEBUG
//	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareEntryXmlHandler::~SoftwareListXmlHandler()");
#endif

}

bool SoftwareEntryXmlHandler::startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &attributes)
{
#ifdef QMC2_DEBUG
//	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareEntryXmlHandler::startElement(const QString &namespaceURI = ..., const QString &localName = %1, const QString &qName = %2, const QXmlAttributes &attributes = ...)").arg(localName).arg(qName));
#endif

	if ( qName == "software" ) {
		softwareValid = ( attributes.value("name") == parentTreeWidgetItem->text(QMC2_SWLIST_COLUMN_NAME) );
		if ( softwareValid )
			qmc2SoftwareList->successfulLookups.clear();
	}

	if ( !softwareValid )
		return true;

	if ( qName == "part" ) {
		if ( partItem == NULL ) {
			partItem = new SoftwareItem(parentTreeWidgetItem);
			partItem->setText(QMC2_SWLIST_COLUMN_TITLE, QObject::tr("Part:") + " " + attributes.value("name"));
			partItem->setText(QMC2_SWLIST_COLUMN_PART, attributes.value("name"));
			partItem->setText(QMC2_SWLIST_COLUMN_INTERFACE, attributes.value("interface"));
			partItem->setText(QMC2_SWLIST_COLUMN_LIST, parentTreeWidgetItem->text(QMC2_SWLIST_COLUMN_LIST));
			QStringList mountList;
			QString mountDev = qmc2SoftwareList->lookupMountDevice(partItem->text(QMC2_SWLIST_COLUMN_PART), partItem->text(QMC2_SWLIST_COLUMN_INTERFACE), &mountList);
			if ( mountList.count() > 0 ) {
				QComboBox *comboBoxMountDevices = new QComboBox(parentTreeWidgetItem->treeWidget());
				mountList.prepend(QObject::tr("Don't mount"));
				mountList.prepend(QObject::tr("Auto mount"));
				comboBoxMountDevices->insertItems(0, mountList);
				comboBoxMountDevices->insertSeparator(QMC2_SWLIST_MSEL_SEPARATOR);
				if ( !qmc2SoftwareList->autoMounted ) {
					comboBoxMountDevices->setCurrentIndex(QMC2_SWLIST_MSEL_DONT_MOUNT); // ==> don't mount
					partItem->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("Not mounted"));
				} else {
					comboBoxMountDevices->setCurrentIndex(QMC2_SWLIST_MSEL_AUTO_MOUNT); // ==> auto mount
					if ( mountDev.isEmpty() )
						partItem->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("Not mounted"));
					else
						partItem->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("Mounted on:") + " " + mountDev);
				}
				parentTreeWidgetItem->treeWidget()->setItemWidget(partItem, QMC2_SWLIST_COLUMN_PUBLISHER, comboBoxMountDevices);
				QObject::connect(comboBoxMountDevices, SIGNAL(currentIndexChanged(int)), qmc2SoftwareList, SLOT(checkMountDeviceSelection()));
			} else {
				partItem->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("No mount device"));
				partItem->setText(QMC2_SWLIST_COLUMN_PUBLISHER, QObject::tr("Unmanaged"));
			}
		}
	} else if ( qName == "dataarea" ) {
		if ( partItem != NULL ) {
			dataareaItem = new SoftwareItem(partItem);
			dataareaItem->setText(QMC2_SWLIST_COLUMN_TITLE, QObject::tr("Data area:") + " " + attributes.value("name"));
			QString s = attributes.value("size");
			if ( !s.isEmpty() )
				dataareaItem->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("Size:") + " " + s);
		}
	} else if ( qName == "diskarea" ) {
		if ( partItem != NULL ) {
			dataareaItem = new SoftwareItem(partItem);
			dataareaItem->setText(QMC2_SWLIST_COLUMN_TITLE, QObject::tr("Disk area:") + " " + attributes.value("name"));
			QString s = attributes.value("size");
			if ( !s.isEmpty() )
				dataareaItem->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("Size:") + " " + s);
		}
	} else if ( qName == "rom" ) {
		if ( dataareaItem != NULL ) {
			QString romName = attributes.value("name");
			if ( !romName.isEmpty() ) {
				romItem = new SoftwareItem(dataareaItem);
				romItem->setText(QMC2_SWLIST_COLUMN_TITLE, romName);
				QString s = attributes.value("size");
				if ( !s.isEmpty() )
					romItem->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("Size:") + " " + s);
				s = attributes.value("crc");
				if ( !s.isEmpty() )
					romItem->setText(QMC2_SWLIST_COLUMN_PUBLISHER, QObject::tr("CRC:") + " " + s);
			}
		}
	} else if ( qName == "disk" ) {
		if ( dataareaItem != NULL ) {
			QString diskName = attributes.value("name");
			if ( !diskName.isEmpty() ) {
				romItem = new SoftwareItem(dataareaItem);
				romItem->setText(QMC2_SWLIST_COLUMN_TITLE, diskName);
				QString s = attributes.value("sha1");
				if ( !s.isEmpty() )
					romItem->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("SHA1:") + " " + s);
			}
		}
	}

	return true;
}

bool SoftwareEntryXmlHandler::endElement(const QString &namespaceURI, const QString &localName, const QString &qName)
{
#ifdef QMC2_DEBUG
//	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareEntryXmlHandler::endElement(const QString &namespaceURI = ..., const QString &localName = %1, const QString &qName = %2)").arg(localName).arg(qName));
#endif

	if ( qName == "software" ) {
		softwareValid = false;
	} else if ( qName == "part" ) {
		partItem = NULL;
	} else if ( qName == "dataarea" || qName == "diskarea" ) {
		dataareaItem = NULL;
	} else if ( qName == "rom" ) {
		romItem = NULL;
	}

	return true;
}

bool SoftwareEntryXmlHandler::characters(const QString &str)
{
#ifdef QMC2_DEBUG
//	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareEntryXmlHandler::characters(const QString &str = ...)"));
#endif

	currentText += QString::fromUtf8(str.toAscii());
	return true;
}
