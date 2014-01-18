#include <QtGui>
#include <QTest>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QCache>
#include <QInputDialog>
#include <QWidgetAction>
#include <QLocale>
#include <QPainterPath>

#include "softwarelist.h"
#include "gamelist.h"
#include "qmc2main.h"
#include "options.h"
#include "iconlineedit.h"
#include "macros.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern Settings *qmc2Config;
extern Options *qmc2Options;
extern Gamelist *qmc2Gamelist;
extern bool qmc2CleaningUp;
extern bool qmc2EarlyStartup;
extern bool qmc2UseSoftwareSnapFile;
extern SoftwareList *qmc2SoftwareList;
extern SoftwareSnap *qmc2SoftwareSnap;
extern SoftwareSnapshot *qmc2SoftwareSnapshot;
extern int qmc2SoftwareSnapPosition;
extern bool qmc2IgnoreItemActivation;
extern bool qmc2SmoothScaling;
extern bool qmc2RetryLoadingImages;
extern bool qmc2ShowGameName;
extern int qmc2UpdateDelay;
extern int qmc2DefaultLaunchMode;
extern bool qmc2StopParser;
extern bool qmc2CriticalSection;
extern bool qmc2UseDefaultEmulator;
extern QCache<QString, ImagePixmap> qmc2ImagePixmapCache;
extern QMap<QString, QPair<QString, QAction *> > qmc2ShortcutMap;
extern QMap<QString, QString> qmc2CustomShortcutMap;

QMap<QString, QStringList> systemSoftwareListMap;
QMap<QString, QStringList> systemSoftwareFilterMap;
QMap<QString, QString> softwareListXmlDataCache;
QMap<QString, QMap<QString, char> > softwareListStateMap;
QString swlBuffer;
QString swlSelectedMountDevice;
QString swStatesBuffer;
QString swStatesLastLine;
bool swlSupported = true;
bool SoftwareList::isInitialLoad = true;

SoftwareList::SoftwareList(QString sysName, QWidget *parent)
	: QWidget(parent)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::SoftwareList(QString sysName = %1, QWidget *parent = %2)").arg(sysName).arg((qulonglong)parent));
#endif

	setupUi(this);

	progressBar->setVisible(false);

	// hide snapname device selection initially
	comboBoxSnapnameDevice->hide();

	if ( !qmc2SoftwareSnap )
		qmc2SoftwareSnap = new SoftwareSnap(0);

	qmc2SoftwareSnap->hide();
	snapTimer.setSingleShot(true);
	connect(&snapTimer, SIGNAL(timeout()), qmc2SoftwareSnap, SLOT(loadSnapshot()));

	systemName = sysName;
	loadProc = verifyProc = NULL;
	exporter = NULL;
	currentItem = enteredItem = NULL;
	snapForced = autoSelectSearchItem = interruptLoad = isLoading = isReady = fullyLoaded = updatingMountDevices = negatedMatch = false;
	validData = autoMounted = true;
	cachedDeviceLookupPosition = 0;

#if defined(QMC2_EMUTYPE_MAME)
	comboBoxDeviceConfiguration->setVisible(false);
	QString altText = tr("Add the currently selected software to the favorites list");
	toolButtonAddToFavorites->setToolTip(altText); toolButtonAddToFavorites->setStatusTip(altText);
	treeWidgetFavoriteSoftware->setColumnCount(QMC2_SWLIST_COLUMN_DEVICECFG);
#elif defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
	horizontalLayout->removeItem(horizontalSpacer);
#endif

	oldMin = 0;
	oldMax = 1;
	oldFmt = qmc2MainWindow->progressBarGamelist->format();

	comboBoxSearch->setLineEdit(new IconLineEdit(QIcon(QString::fromUtf8(":/data/img/find.png")), QMC2_ALIGN_LEFT, comboBoxSearch));
	comboBoxSearch->lineEdit()->setPlaceholderText(tr("Enter search string"));

	QFontMetrics fm(QApplication::font());
	QSize iconSize(fm.height() - 2, fm.height() - 2);
	QSize iconSizeMiddle = iconSize + QSize(2, 2);
	treeWidgetKnownSoftware->setIconSize(iconSizeMiddle);
	treeWidgetFavoriteSoftware->setIconSize(iconSizeMiddle);
	treeWidgetSearchResults->setIconSize(iconSizeMiddle);
	toolButtonAddToFavorites->setIconSize(iconSize);
	toolButtonRemoveFromFavorites->setIconSize(iconSize);
	toolButtonFavoritesOptions->setIconSize(iconSize);
	toolButtonPlay->setIconSize(iconSize);
	toolButtonReload->setIconSize(iconSize);
	toolButtonExport->setIconSize(iconSize);
	toolButtonToggleSoftwareInfo->setIconSize(iconSize);
	toolButtonCompatFilterToggle->setIconSize(iconSize);
	toolButtonToggleSnapnameAdjustment->setIconSize(iconSize);
	toolButtonSoftwareStates->setIconSize(iconSize);
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
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
	toolButtonFavoritesOptions->setEnabled(false);
	toolButtonPlay->setEnabled(false);
	toolButtonPlayEmbedded->setEnabled(false);
	toolButtonExport->setEnabled(false);
	toolButtonToggleSoftwareInfo->setEnabled(false);
	toolButtonCompatFilterToggle->setEnabled(false);
	toolButtonToggleSnapnameAdjustment->setEnabled(false);
	toolButtonSoftwareStates->setEnabled(false);
	comboBoxDeviceConfiguration->setEnabled(false);

	// software list context menu
	softwareListMenu = new QMenu(this);
	QString s = tr("Play selected software");
	QAction *action = softwareListMenu->addAction(tr("&Play"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/launch.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(playActivated()));
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
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
	softwareListMenu->addSeparator();
	s = tr("Clear software selection");
	actionClearSelection = softwareListMenu->addAction(tr("&Clear selection"));
	actionClearSelection->setToolTip(s); actionClearSelection->setStatusTip(s);
	actionClearSelection->setIcon(QIcon(QString::fromUtf8(":/data/img/broom.png")));
	connect(actionClearSelection, SIGNAL(triggered()), this, SLOT(clearSoftwareSelection()));

	// favorites options menu
	favoritesOptionsMenu = new QMenu(this);
	s = tr("Load favorites from a file...");
	action = favoritesOptionsMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/fileopen.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(loadFavoritesFromFile()));
	s = tr("Save favorites to a file...");
	action = favoritesOptionsMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/filesaveas.png")));
	actionSaveFavoritesToFile = action;
	connect(action, SIGNAL(triggered()), this, SLOT(saveFavoritesToFile()));
	toolButtonFavoritesOptions->setMenu(favoritesOptionsMenu);

	// snapname adjustment menu
	menuSnapnameAdjustment = new QMenu(this);
	s = tr("Adjust pattern...");
	action = menuSnapnameAdjustment->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/configure.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(adjustSnapnamePattern()));
	toolButtonToggleSnapnameAdjustment->setMenu(menuSnapnameAdjustment);

	// software-states menu
	menuSoftwareStates = new QMenu(this);
	s = tr("Check software-states");
	action = menuSoftwareStates->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/update.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(checkSoftwareStates()));
	actionCheckSoftwareStates = action;
	actionCheckSoftwareStates->setShortcut(QKeySequence(qmc2CustomShortcutMap["F10"]));
	actionCheckSoftwareStates->setShortcutContext(Qt::ApplicationShortcut);
	qmc2ShortcutMap["F10"].second = actionCheckSoftwareStates;
	menuSoftwareStates->addSeparator();
	stateFilter = new SoftwareStateFilter(menuSoftwareStates);
	QWidgetAction *stateFilterAction = new QWidgetAction(menuSoftwareStates);
	stateFilterAction->setDefaultWidget(stateFilter);
	menuSoftwareStates->addAction(stateFilterAction);

	// search options menu
	menuSearchOptions = new QMenu(this);
	s = tr("Negate search");
	actionNegateSearch = menuSearchOptions->addAction(s);
	actionNegateSearch->setToolTip(s); actionNegateSearch->setStatusTip(s);
	actionNegateSearch->setIcon(QIcon(QString::fromUtf8(":/data/img/find_negate.png")));
	actionNegateSearch->setCheckable(true);
	bool negated = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/NegateSearch", false).toBool();
	actionNegateSearch->setChecked(negated);
	negateSearchTriggered(negated);
	connect(actionNegateSearch, SIGNAL(triggered(bool)), this, SLOT(negateSearchTriggered(bool)));
	IconLineEdit *ile = ((IconLineEdit *)comboBoxSearch->lineEdit());
	connect(ile, SIGNAL(returnPressed()), this, SLOT(comboBoxSearch_editTextChanged_delayed()));
	ile->button()->setPopupMode(QToolButton::InstantPopup);
	ile->button()->setMenu(menuSearchOptions);

	// restore widget states
	treeWidgetKnownSoftware->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/KnownSoftwareHeaderState").toByteArray());
	treeWidgetFavoriteSoftware->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/FavoriteSoftwareHeaderState").toByteArray());
	treeWidgetSearchResults->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SearchResultsHeaderState").toByteArray());
	toolBoxSoftwareList->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/PageIndex").toInt());
	toolButtonToggleSoftwareInfo->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/ShowSoftwareInfo", false).toBool());
	toolButtonCompatFilterToggle->blockSignals(true);
	toolButtonCompatFilterToggle->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/CompatFilter", true).toBool());
	toolButtonCompatFilterToggle->blockSignals(false);
	toolButtonToggleSnapnameAdjustment->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/AdjustSnapname", false).toBool());
	toolButtonSoftwareStates->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/ShowSoftwareStates", false).toBool());
	if ( toolButtonSoftwareStates->isChecked() )
		toolButtonSoftwareStates->setMenu(menuSoftwareStates);

	connect(treeWidgetKnownSoftware->header(), SIGNAL(sectionClicked(int)), this, SLOT(treeWidgetKnownSoftware_headerSectionClicked(int)));
	connect(treeWidgetFavoriteSoftware->header(), SIGNAL(sectionClicked(int)), this, SLOT(treeWidgetFavoriteSoftware_headerSectionClicked(int)));
	connect(treeWidgetSearchResults->header(), SIGNAL(sectionClicked(int)), this, SLOT(treeWidgetSearchResults_headerSectionClicked(int)));
	connect(&searchTimer, SIGNAL(timeout()), this, SLOT(comboBoxSearch_editTextChanged_delayed()));

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
	action = menuKnownSoftwareHeader->addAction(tr("Supported"), this, SLOT(actionKnownSoftwareHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_SUPPORTED);
	action->setChecked(!treeWidgetKnownSoftware->isColumnHidden(QMC2_SWLIST_COLUMN_SUPPORTED));
	menuKnownSoftwareHeader->addSeparator();
	action = menuKnownSoftwareHeader->addAction(QIcon(":data/img/reset.png"), tr("Reset"), this, SLOT(actionKnownSoftwareHeader_triggered())); action->setData(QMC2_SWLIST_RESET);
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
	action = menuFavoriteSoftwareHeader->addAction(tr("Supported"), this, SLOT(actionFavoriteSoftwareHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_SUPPORTED);
	action->setChecked(!treeWidgetFavoriteSoftware->isColumnHidden(QMC2_SWLIST_COLUMN_LIST));
#if defined(QMC2_EMUTYPE_MESS) | defined(QMC2_EMUTYPE_UME)
	action = menuFavoriteSoftwareHeader->addAction(tr("Device configuration"), this, SLOT(actionFavoriteSoftwareHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_DEVICECFG);
	action->setChecked(!treeWidgetFavoriteSoftware->isColumnHidden(QMC2_SWLIST_COLUMN_DEVICECFG));
#endif
	menuFavoriteSoftwareHeader->addSeparator();
	action = menuFavoriteSoftwareHeader->addAction(QIcon(":data/img/reset.png"), tr("Reset"), this, SLOT(actionFavoriteSoftwareHeader_triggered())); action->setData(QMC2_SWLIST_RESET);
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
	action = menuSearchResultsHeader->addAction(tr("Supported"), this, SLOT(actionSearchResultsHeader_triggered())); action->setCheckable(true); action->setData(QMC2_SWLIST_COLUMN_SUPPORTED);
	action->setChecked(!treeWidgetSearchResults->isColumnHidden(QMC2_SWLIST_COLUMN_LIST));
	menuSearchResultsHeader->addSeparator();
	action = menuSearchResultsHeader->addAction(QIcon(":data/img/reset.png"), tr("Reset"), this, SLOT(actionSearchResultsHeader_triggered())); action->setData(QMC2_SWLIST_RESET);
	header->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(header, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(treeWidgetSearchResultsHeader_customContextMenuRequested(const QPoint &)));

	// detail update timer
	connect(&detailUpdateTimer, SIGNAL(timeout()), this, SLOT(updateDetail()));

	isReady = true;
}

SoftwareList::~SoftwareList()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::~SoftwareList()");
#endif

	if ( exporter )
		exporter->close();

	// save widget states
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/KnownSoftwareHeaderState", treeWidgetKnownSoftware->header()->saveState());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/FavoriteSoftwareHeaderState", treeWidgetFavoriteSoftware->header()->saveState());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SearchResultsHeaderState", treeWidgetSearchResults->header()->saveState());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/PageIndex", toolBoxSoftwareList->currentIndex());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/ShowSoftwareInfo", toolButtonToggleSoftwareInfo->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/CompatFilter", toolButtonCompatFilterToggle->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/AdjustSnapname", toolButtonToggleSnapnameAdjustment->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/ShowSoftwareStates", toolButtonSoftwareStates->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/NegateSearch", actionNegateSearch->isChecked());
}

void SoftwareList::adjustSnapnamePattern()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::adjustSnapnamePattern()");
#endif

	bool ok;
	QStringList items;
	items << "soft-lists/$SOFTWARE_LIST$/$SOFTWARE_NAME$" << "soft-lists/$SOFTWARE_LIST$/$SOFTWARE_NAME$/%i";
	QString storedPattern = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SnapnamePattern", "soft-lists/$SOFTWARE_LIST$/$SOFTWARE_NAME$").toString();
	int index = items.indexOf(storedPattern);
	if ( index < 0 ) {
		items << storedPattern;
		index = 2;
	}
	QString pattern = QInputDialog::getItem(this,
						tr("Snapname adjustment pattern"),
						tr("Enter the pattern used for snapname adjustment:\n(Allowed macros: $SOFTWARE_LIST$, $SOFTWARE_NAME$)"),
						items, index, true, &ok);
	if ( ok )
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SnapnamePattern", pattern);
}

void SoftwareList::clearSoftwareSelection()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::clearSoftwareSelection()");
#endif

	switch ( toolBoxSoftwareList->currentIndex() ) {
		case QMC2_SWLIST_KNOWN_SW_PAGE:
			treeWidgetKnownSoftware->clearSelection();
			break;
		case QMC2_SWLIST_FAVORITES_PAGE:
			treeWidgetFavoriteSoftware->clearSelection();
			break;
		case QMC2_SWLIST_SEARCH_PAGE:
			treeWidgetSearchResults->clearSelection();
			break;
	}
}

void SoftwareList::negateSearchTriggered(bool negate)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::negateSearchTriggered(bool negate = %1)").arg(negate));
#endif

	IconLineEdit *ile = ((IconLineEdit *)comboBoxSearch->lineEdit());
	if ( negate )
		ile->button()->setIcon(QIcon(QString::fromUtf8(":/data/img/find_negate.png")));
	else
		ile->button()->setIcon(QIcon(QString::fromUtf8(":/data/img/find.png")));
	negatedMatch = negate;

	searchTimer.start(QMC2_SEARCH_DELAY);
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
		while ( !swlLines[i].startsWith(s) && i < swlLinesMax && !interruptLoad ) i++;
		softwareListXmlBuffer = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
		while ( !swlLines[i].startsWith("</softwarelist>") && i < swlLinesMax && !interruptLoad )
			softwareListXmlBuffer += swlLines[i++].simplified() + "\n";
		softwareListXmlBuffer += "</softwarelist>";
		if ( i < swlLinesMax )
			softwareListXmlDataCache[listName] = softwareListXmlBuffer;
		else {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: software list '%1' not found").arg(listName));
			softwareListXmlBuffer.clear();
		}
	}

	return softwareListXmlBuffer;
}

QString &SoftwareList::getXmlDataWithEnabledSlots(QStringList swlArgs)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::getXmlDataWithEnabledSlots(QStringList swlArgs = ..."));
#endif

	static QString xmlBuffer;

	xmlBuffer.clear();

	qmc2CriticalSection = true;

	QString userScopePath = QMC2_DYNAMIC_DOT_PATH;
	QProcess commandProc;
#if defined(QMC2_SDLMESS)
	commandProc.setStandardOutputFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmess.tmp").toString());
#elif defined(QMC2_MESS)
	commandProc.setStandardOutputFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mess.tmp").toString());
#elif defined(QMC2_SDLMAME)
	commandProc.setStandardOutputFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmame.tmp").toString());
#elif defined(QMC2_MAME)
	commandProc.setStandardOutputFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mame.tmp").toString());
#elif defined(QMC2_SDLUME)
	commandProc.setStandardOutputFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlume.tmp").toString());
#elif defined(QMC2_UME)
	commandProc.setStandardOutputFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-ume.tmp").toString());
#endif
#if !defined(QMC2_OS_WIN)
	commandProc.setStandardErrorFile("/dev/null");
#endif

	QStringList args;
	args << systemName << swlArgs << "-listxml";

#ifdef QMC2_DEBUG
	printf("SoftwareList::getXmlDataWithEnabledSlots(): args = %s\n", (const char *)args.join(" ").toLocal8Bit());
#endif

	bool commandProcStarted = false;
	int retries = 0;
	commandProc.start(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ExecutableFile").toString(), args);
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
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't start emulator executable within a reasonable time frame, giving up"));
		qmc2CriticalSection = false;
		return xmlBuffer;
	}

#if defined(QMC2_SDLMESS)
	QFile qmc2TempXml(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmess.tmp").toString());
#elif defined(QMC2_MESS)
	QFile qmc2TempXml(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mess.tmp").toString());
#elif defined(QMC2_SDLMAME)
	QFile qmc2TempXml(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmame.tmp").toString());
#elif defined(QMC2_MAME)
	QFile qmc2TempXml(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mame.tmp").toString());
#elif defined(QMC2_SDLUME)
	QFile qmc2TempXml(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlume.tmp").toString());
#elif defined(QMC2_UME)
	QFile qmc2TempXml(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-ume.tmp").toString());
#endif

	if ( commandProcStarted && qmc2TempXml.open(QFile::ReadOnly) ) {
		QTextStream ts(&qmc2TempXml);
		xmlBuffer = ts.readAll();
#if defined(QMC2_OS_WIN)
		xmlBuffer.replace("\r\n", "\n"); // convert WinDOS's "0x0D 0x0A" to just "0x0A" 
#endif
		qmc2TempXml.close();
		qmc2TempXml.remove();
		if ( !xmlBuffer.isEmpty() ) {
			QStringList xmlLines = xmlBuffer.split("\n");
			qApp->processEvents();
			xmlBuffer.clear();
			if ( !xmlLines.isEmpty() ) {
				int i = 0;
#if defined(QMC2_EMUTYPE_MESS)
				QString s = "<machine name=\"" + systemName + "\"";
#elif defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
				QString s = "<game name=\"" + systemName + "\"";
#endif
				while ( i < xmlLines.count() && !xmlLines[i].contains(s) ) i++;
				xmlBuffer = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
				if ( i < xmlLines.count() ) {
#if defined(QMC2_EMUTYPE_MESS)
					while ( i < xmlLines.count() && !xmlLines[i].contains("</machine>") )
						xmlBuffer += xmlLines[i++].simplified() + "\n";
					if ( i == xmlLines.count() && !xmlLines[i - 1].contains("</machine>") ) {
						qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: invalid XML data retrieved for '%1'").arg(systemName));
						xmlBuffer.clear();
					} else
						xmlBuffer += "</machine>\n";
#elif defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
					while ( i < xmlLines.count() && !xmlLines[i].contains("</game>") )
						xmlBuffer += xmlLines[i++].simplified() + "\n";
					if ( i == xmlLines.count() && !xmlLines[i - 1].contains("</game>") ) {
						qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: invalid XML data retrieved for '%1'").arg(systemName));
						xmlBuffer.clear();
					} else
						xmlBuffer += "</game>\n";
#endif
				} else {
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: invalid XML data retrieved for '%1'").arg(systemName));
					xmlBuffer.clear();
				}
			}
		}
	}

	qmc2CriticalSection = false;
	return xmlBuffer;
}

void SoftwareList::on_comboBoxDeviceConfiguration_currentIndexChanged(int index)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_comboBoxDeviceConfiguration_currentIndexChanged(int index = %1)").arg(index));
#endif

#if defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
	QTimer::singleShot(0, this, SLOT(updateMountDevices()));
#endif
}

QString &SoftwareList::lookupMountDevice(QString device, QString deviceInterface, QStringList *mountList)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::lookupMountDevice(QString device = %1, QString deviceInterface = %2, QStringList *mountList = %3)").arg(device).arg(deviceInterface).arg((qulonglong)mountList));
#endif

	static QString softwareListDeviceName;

	QMap<QString, QStringList> deviceInstanceMap;
	softwareListDeviceName.clear();

	QStringList *xmlData = &qmc2Gamelist->xmlLines;
	QStringList dynamicXmlData;
	if ( comboBoxDeviceConfiguration->currentIndex() > 0 ) {
		qmc2Config->beginGroup(QMC2_EMULATOR_PREFIX + QString("Configuration/Devices/%1/%2").arg(systemName).arg(comboBoxDeviceConfiguration->currentText()));
		QStringList instances = qmc2Config->value("Instances").toStringList();
		QStringList files = qmc2Config->value("Files").toStringList();
		QStringList slotNames = qmc2Config->value("Slots").toStringList();
		QStringList slotOptions = qmc2Config->value("SlotOptions").toStringList();
		QStringList slotBIOSs = qmc2Config->value("SlotBIOSs").toStringList();
		qmc2Config->endGroup();
		QStringList swlArgs;
		for (int j = 0; j < slotNames.count(); j++) {
			if ( !slotOptions[j].isEmpty() ) {
				QString slotOpt = slotOptions[j];
				if ( !slotBIOSs[j].isEmpty() )
					slotOpt += ",bios=" + slotBIOSs[j];
				swlArgs << QString("-%1").arg(slotNames[j]) << slotOpt;
			}
		}
		for (int j = 0; j < instances.count(); j++) {
#if defined(QMC2_OS_WIN)
			swlArgs << QString("-%1").arg(instances[j]) << files[j].replace('/', '\\');
#else
			swlArgs << QString("-%1").arg(instances[j]) << files[j].replace("~", "$HOME");
#endif
		}
		foreach (QString line, getXmlDataWithEnabledSlots(swlArgs).split("\n", QString::SkipEmptyParts))
			dynamicXmlData << line.trimmed();
		xmlData = &dynamicXmlData;
#ifdef QMC2_DEBUG
		printf("SoftwareList::getXmlDataWithEnabledSlots(): XML data start\n");
		foreach (QString line, dynamicXmlData)
			printf("%s\n", (const char *)line.toLocal8Bit());
		printf("SoftwareList::getXmlDataWithEnabledSlots(): XML data end\n");
#endif
	}

	int i = 0;
	if ( xmlData == &qmc2Gamelist->xmlLines )
		i = cachedDeviceLookupPosition;
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
	QString s = "<game name=\"" + systemName + "\"";
	while ( i < xmlData->count() && !(*xmlData)[i].contains(s) ) i++;
	if ( i < xmlData->count() && (*xmlData)[i].contains(s) && xmlData == &qmc2Gamelist->xmlLines ) cachedDeviceLookupPosition = i - 1;
	while ( i < xmlData->count() && !(*xmlData)[i].contains("</game>") ) {
		QString line = (*xmlData)[i++].simplified();
		if ( line.startsWith("<device type=\"") ) {
			int startIndex = line.indexOf("interface=\"");
			int endIndex;
			if ( startIndex >= 0 ) {
				startIndex += 11;
				int endIndex = line.indexOf("\"", startIndex);
				QStringList devInterfaces = line.mid(startIndex, endIndex - startIndex).split(",", QString::SkipEmptyParts);
				line = qmc2Gamelist->xmlLines[i++].simplified();
				startIndex = line.indexOf("briefname=\"") + 11;
				endIndex = line.indexOf("\"", startIndex);
				QString devName = line.mid(startIndex, endIndex - startIndex);
				if ( !devName.isEmpty() )
					foreach (QString devIf, devInterfaces)
						deviceInstanceMap[devIf] << devName;
			} else {
				line = (*xmlData)[i++].simplified();
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
	while ( i < xmlData->count() && !(*xmlData)[i].contains(s) ) i++;
	if ( i < xmlData->count() && (*xmlData)[i].contains(s) && xmlData == &qmc2Gamelist->xmlLines ) cachedDeviceLookupPosition = i - 1;
	while ( i < xmlData->count() && !(*xmlData)[i].contains("</machine>") ) {
		QString line = (*xmlData)[i++].simplified();
		if ( line.startsWith("<device type=\"") ) {
			int startIndex = line.indexOf("interface=\"");
			int endIndex;
			if ( startIndex >= 0 ) {
				startIndex += 11;
				int endIndex = line.indexOf("\"", startIndex);
				QStringList devInterfaces = line.mid(startIndex, endIndex - startIndex).split(",", QString::SkipEmptyParts);
				line = (*xmlData)[i++].simplified();
				startIndex = line.indexOf("briefname=\"") + 11;
				endIndex = line.indexOf("\"", startIndex);
				QString devName = line.mid(startIndex, endIndex - startIndex);
				if ( !devName.isEmpty() )
					foreach (QString devIf, devInterfaces)
						deviceInstanceMap[devIf] << devName;
			} else {
				line = (*xmlData)[i++].simplified();
				startIndex = line.indexOf("briefname=\"") + 11;
				endIndex = line.indexOf("\"", startIndex);
				QString devName = line.mid(startIndex, endIndex - startIndex);
				if ( !devName.isEmpty() )
					deviceInstanceMap[devName] << devName;
			}
		}
	}
#endif

	QStringList briefNames = deviceInstanceMap[deviceInterface];
	briefNames.sort();

	if ( briefNames.contains(device) )
		softwareListDeviceName = device;
	else for (int i = 0; i < briefNames.count() && softwareListDeviceName.isEmpty(); i++) {
			softwareListDeviceName = briefNames[i];
			if ( successfulLookups.contains(softwareListDeviceName) )
				softwareListDeviceName.clear();
	}

	if ( successfulLookups.contains(softwareListDeviceName) )
		softwareListDeviceName.clear();

	if ( mountList != NULL )
		*mountList = briefNames;

	if ( !softwareListDeviceName.isEmpty() )
		successfulLookups << softwareListDeviceName;

	return softwareListDeviceName;
}

void SoftwareList::getXmlData()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::getXmlData()");
#endif

	QStringList softwareList = systemSoftwareListMap[systemName];
	if ( softwareList.isEmpty() || softwareList.contains("NO_SOFTWARE_LIST") ) {
		softwareList.clear();
		int i = 0;
		QString filter;
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
		QString s = "<game name=\"" + systemName + "\"";
		while ( !qmc2Gamelist->xmlLines[i].contains(s) && !interruptLoad ) i++;
		while ( !qmc2Gamelist->xmlLines[i].contains("</game>") && !interruptLoad ) {
			QString line = qmc2Gamelist->xmlLines[i++].simplified();
			if ( line.startsWith("<softwarelist name=\"") ) {
				int startIndex = line.indexOf("\"") + 1;
				int endIndex = line.indexOf("\"", startIndex);
				softwareList << line.mid(startIndex, endIndex - startIndex); 
				startIndex = line.indexOf(" filter=\"");
				if ( startIndex >= 0 ) {
					startIndex += 9;
					endIndex = line.indexOf("\"", startIndex);
					filter = line.mid(startIndex, endIndex - startIndex);
				}
			}
		}
#elif defined(QMC2_EMUTYPE_MESS)
		QString s = "<machine name=\"" + systemName + "\"";
		while ( !qmc2Gamelist->xmlLines[i].contains(s) && !interruptLoad ) i++;
		while ( !qmc2Gamelist->xmlLines[i].contains("</machine>") && !interruptLoad ) {
			QString line = qmc2Gamelist->xmlLines[i++].simplified();
			if ( line.startsWith("<softwarelist name=\"") ) {
				int startIndex = line.indexOf("\"") + 1;
				int endIndex = line.indexOf("\"", startIndex);
				softwareList << line.mid(startIndex, endIndex - startIndex); 
				startIndex = line.indexOf(" filter=\"");
				if ( startIndex >= 0 ) {
					startIndex += 9;
					endIndex = line.indexOf("\"", startIndex);
					filter = line.mid(startIndex, endIndex - startIndex);
				}
			}
		}
#endif
		if ( softwareList.isEmpty() )
			softwareList << "NO_SOFTWARE_LIST";
		else
			softwareList.sort();
		systemSoftwareListMap[systemName] = softwareList;
#ifdef QMC2_DEBUG
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: systemSoftwareListMap[%1] = %2").arg(systemName).arg(softwareList.join(", ")));
#endif

		if ( !filter.isEmpty() )
			systemSoftwareFilterMap[systemName] = filter.split(",", QString::SkipEmptyParts);
	}
#ifdef QMC2_DEBUG
	else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: systemSoftwareListMap[%1] = %2 (cached)").arg(systemName).arg(systemSoftwareListMap[systemName].join(", ")));
#endif

	if ( !softwareList.isEmpty() && !softwareList.contains("NO_SOFTWARE_LIST") ) {
		QString swlString = softwareList.join(", ");
		if ( toolButtonSoftwareStates->isChecked() && stateFilter->checkBoxStateFilter->isChecked() )
			toolBoxSoftwareList->setItemText(QMC2_SWLIST_KNOWN_SW_PAGE, tr("Known software (%1)").arg(swlString) + " - " + tr("filtered"));
		else
			toolBoxSoftwareList->setItemText(QMC2_SWLIST_KNOWN_SW_PAGE, tr("Known software (%1)").arg(swlString));
		toolBoxSoftwareList->setItemText(QMC2_SWLIST_FAVORITES_PAGE, tr("Favorites (%1)").arg(swlString));
		toolBoxSoftwareList->setItemText(QMC2_SWLIST_SEARCH_PAGE, tr("Search (%1)").arg(swlString));
		toolBoxSoftwareList->setEnabled(true);

#if defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
		// load stored device configurations, if any...
		qmc2Config->beginGroup(QString(QMC2_EMULATOR_PREFIX + "Configuration/Devices/%1").arg(systemName));
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
}

void SoftwareList::updateMountDevices()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::updateMountDevices()");
#endif

	if ( updatingMountDevices )
		return;

	updatingMountDevices = true;
	autoMounted = true;

	QTreeWidget *treeWidget = NULL;
	switch ( toolBoxSoftwareList->currentIndex() ) {
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

	QTreeWidgetItemIterator it(treeWidget);
	while ( *it ) {
		QComboBox *comboBox = (QComboBox *)treeWidget->itemWidget(*it, QMC2_SWLIST_COLUMN_PUBLISHER);
		if ( comboBox ) {
			comboBox->blockSignals(true);
			comboBox->setUpdatesEnabled(false);
			comboBox->clear();
			QStringList mountList;
			successfulLookups.clear();
			QString mountDev = lookupMountDevice((*it)->text(QMC2_SWLIST_COLUMN_PART), (*it)->text(QMC2_SWLIST_COLUMN_INTERFACE), &mountList);
			if ( mountList.count() > 0 ) {
				mountList.prepend(QObject::tr("Don't mount"));
				mountList.prepend(QObject::tr("Auto mount"));
				comboBox->insertItems(0, mountList);
				comboBox->insertSeparator(QMC2_SWLIST_MSEL_SEPARATOR);
				comboBox->setCurrentIndex(QMC2_SWLIST_MSEL_AUTO_MOUNT);
				if ( mountDev.isEmpty() )
					(*it)->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("Not mounted"));
				else
					(*it)->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("Mounted on:") + " " + mountDev);
			} else {
				(*it)->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("No mount device"));
				(*it)->setText(QMC2_SWLIST_COLUMN_PUBLISHER, QObject::tr("Unmanaged"));
			}
			comboBox->setUpdatesEnabled(true);
			comboBox->blockSignals(false);
		}
		it++;
	}

	updatingMountDevices = false;
}

bool SoftwareList::load()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::load()");
#endif

	setEnabled(qmc2UseDefaultEmulator);

	bool swlCacheOkay = true;
	autoMounted = true;
	interruptLoad = false;
	isLoading = true;
	fullyLoaded = false;
	validData = swlSupported;
	QString swlCachePath = qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareListCache").toString();
	numSoftwareTotal = numSoftwareCorrect = numSoftwareIncorrect = numSoftwareMostlyCorrect = numSoftwareNotFound = numSoftwareUnknown = 0;
	updateStats();

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

	cachedDeviceLookupPosition = 0;

	if ( swlBuffer.isEmpty() && swlSupported ) {
		oldMin = qmc2MainWindow->progressBarGamelist->minimum();
		oldMax = qmc2MainWindow->progressBarGamelist->maximum();
		oldFmt = qmc2MainWindow->progressBarGamelist->format();

          	qmc2MainWindow->tabSoftwareList->setUpdatesEnabled(true);
		labelLoadingSoftwareLists->setText(tr("Loading software-lists, please wait..."));
		labelLoadingSoftwareLists->setVisible(true);
		toolBoxSoftwareList->setVisible(false);
		show();
		qApp->processEvents();

		swlLines.clear();
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
#elif defined(QMC2_EMUTYPE_UME)
				if ( line.startsWith("UME_VERSION") ) {
					QStringList words = line.split("\t");
					if ( words.count() > 1 ) {
						if ( qmc2Gamelist->emulatorVersion == words[1] )
							swlCacheOkay = true;
					}
				}
#endif
				if ( swlCacheOkay ) {
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading XML software list data from cache"));
					QTime elapsedTime(0, 0, 0, 0);
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
					qmc2MainWindow->progressBarGamelist->setRange(oldMin, oldMax);
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
#elif defined(QMC2_EMUTYPE_UME)
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ERROR: the file name for the UME software list cache is empty -- please correct this and reload the game list afterwards"));
#endif
			labelLoadingSoftwareLists->setVisible(false);
			toolBoxSoftwareList->setVisible(true);

			isLoading = false;
			isInitialLoad = false;
			return false;
		}
        }

	if ( !swlCacheOkay ) {
		isInitialLoad = true;
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
#elif defined(QMC2_EMUTYPE_UME)
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ERROR: can't open the UME software list cache for writing, path = %1 -- please check/correct access permissions and reload the game list afterwards").arg(swlCachePath));
#endif
			isLoading = false;
			isInitialLoad = false;
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
#elif defined(QMC2_EMUTYPE_UME)
		tsSWLCache << "UME_VERSION\t" + qmc2Gamelist->emulatorVersion + "\n";
#endif
		
		loadProc = new QProcess(this);

		connect(loadProc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(loadError(QProcess::ProcessError)));
		connect(loadProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(loadFinished(int, QProcess::ExitStatus)));
		connect(loadProc, SIGNAL(readyReadStandardOutput()), this, SLOT(loadReadyReadStandardOutput()));
		connect(loadProc, SIGNAL(readyReadStandardError()), this, SLOT(loadReadyReadStandardError()));
		connect(loadProc, SIGNAL(started()), this, SLOT(loadStarted()));
		connect(loadProc, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(loadStateChanged(QProcess::ProcessState)));

		QString command = qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ExecutableFile").toString();
		QStringList args;
		args << "-listsoftware";
		QString hashPath = qmc2Config->value(QMC2_EMULATOR_PREFIX + "Configuration/Global/hashpath").toString().replace("~", "$HOME");
		if ( !hashPath.isEmpty() )
			args << "-hashpath" << hashPath;

		if ( !qmc2StopParser ) {
			validData = true;
			loadFinishedFlag = false;
			loadProc->start(command, args);
			// FIXME: this is blocking the GUI shortly
			if ( loadProc->waitForStarted() && !qmc2StopParser ) {
				while ( !loadFinishedFlag && !qmc2StopParser ) {
					qApp->processEvents();
#if defined(QMC2_OS_MAC)
					QTest::qWait(10);
#else
					loadProc->waitForFinished(10);
#endif
				}
			} else
				validData = false;
		} 

		if ( qmc2StopParser ) {
			if ( loadProc->state() == QProcess::Running ) {
				loadProc->kill();
				validData = false;
			}
		}

		if ( !validData ) {
			toolBoxSoftwareList->setItemText(QMC2_SWLIST_KNOWN_SW_PAGE, tr("Known software (no data available)"));
			toolBoxSoftwareList->setItemText(QMC2_SWLIST_FAVORITES_PAGE, tr("Favorites (no data available)"));
			toolBoxSoftwareList->setItemText(QMC2_SWLIST_SEARCH_PAGE, tr("Search (no data available)"));

			labelLoadingSoftwareLists->setVisible(false);
			toolBoxSoftwareList->setVisible(true);

			isLoading = false;
			isInitialLoad = false;
			return false;
		}
	}

	labelLoadingSoftwareLists->setVisible(false);
	toolBoxSoftwareList->setVisible(true);

#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::load(): validData = %1").arg(validData));
#endif

	getXmlData();

	QStringList softwareList = systemSoftwareListMap[systemName];
	if ( !softwareList.contains("NO_SOFTWARE_LIST") && !interruptLoad ) {
		swlLines = swlBuffer.split("\n");
		foreach (QString swList, softwareList) {
			if ( interruptLoad ) break;
			QString softwareListXml = getSoftwareListXmlData(swList);
			if ( interruptLoad ) break;
			if ( softwareListXml.size() > QMC2_SWLIST_SIZE_THRESHOLD ) {
				toolBoxSoftwareList->setVisible(false);
				labelLoadingSoftwareLists->setText(tr("Loading software-list '%1', please wait...").arg(swList));
				labelLoadingSoftwareLists->setVisible(true);
				qmc2MainWindow->tabSoftwareList->update();
				qApp->processEvents();
			}
			if ( !softwareListXml.isEmpty() ) {
#ifdef QMC2_DEBUG
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::load(): XML data for software list '%1' follows:\n%2").arg(swList).arg(softwareListXml));
#endif
				QXmlInputSource xmlInputSource;
				xmlInputSource.setData(softwareListXml);
				SoftwareListXmlHandler xmlHandler(treeWidgetKnownSoftware);
				QXmlSimpleReader xmlReader;
				xmlReader.setContentHandler(&xmlHandler);
				if ( !xmlReader.parse(xmlInputSource) && !interruptLoad )
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: error while parsing XML data for software list '%1'").arg(swList));
				else if ( xmlHandler.newSoftwareStates )
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, QObject::tr("state info for software-list '%1': L:%2 C:%3 M:%4 I:%5 N:%6 U:%7").arg(swList).arg(xmlHandler.numTotal).arg(xmlHandler.numCorrect).arg(xmlHandler.numMostlyCorrect).arg(xmlHandler.numIncorrect).arg(xmlHandler.numNotFound).arg(xmlHandler.numUnknown));
				numSoftwareTotal += xmlHandler.numTotal;
				numSoftwareCorrect += xmlHandler.numCorrect;
				numSoftwareIncorrect += xmlHandler.numIncorrect;
				numSoftwareMostlyCorrect += xmlHandler.numMostlyCorrect;
				numSoftwareNotFound += xmlHandler.numNotFound;
				numSoftwareUnknown += xmlHandler.numUnknown;
			}
			updateStats();
		}

		QTimer::singleShot(0, labelLoadingSoftwareLists, SLOT(hide()));
		QTimer::singleShot(0, toolBoxSoftwareList, SLOT(show()));

		// load favorites
#if defined(QMC2_EMUTYPE_MAME)
		QStringList softwareNames = qmc2Config->value(QString(QMC2_EMULATOR_PREFIX + "Favorites/%1/SoftwareNames").arg(systemName)).toStringList();
#elif defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
		QStringList softwareNames = qmc2Config->value(QString(QMC2_EMULATOR_PREFIX + "Favorites/%1/SoftwareNames").arg(systemName)).toStringList();
		QStringList configNames = qmc2Config->value(QString(QMC2_EMULATOR_PREFIX + "Favorites/%1/DeviceConfigs").arg(systemName)).toStringList();
#endif

		QStringList compatFilters = systemSoftwareFilterMap[systemName];
		for (int i = 0; i < softwareNames.count() && !interruptLoad; i++) {
			if ( interruptLoad )
				break;
			QString software = softwareNames[i];
			QList<QTreeWidgetItem *> matchedSoftware = treeWidgetKnownSoftware->findItems(software, Qt::MatchExactly | Qt::MatchCaseSensitive, QMC2_SWLIST_COLUMN_NAME);
			QTreeWidgetItem *swItem = NULL;
			if ( matchedSoftware.count() > 0 ) swItem = matchedSoftware.at(0);
			if ( swItem ) {
				SoftwareItem *item = new SoftwareItem(treeWidgetFavoriteSoftware);
				item->setWhatsThis(QMC2_SWLIST_COLUMN_TITLE, swItem->whatsThis(QMC2_SWLIST_COLUMN_TITLE));
				item->setWhatsThis(QMC2_SWLIST_COLUMN_NAME, swItem->whatsThis(QMC2_SWLIST_COLUMN_NAME));
				bool showItem = true;
				if ( toolButtonCompatFilterToggle->isChecked() ) {
					QStringList compatList = item->whatsThis(QMC2_SWLIST_COLUMN_TITLE).split(",", QString::SkipEmptyParts);
					showItem = compatList.isEmpty() || compatFilters.isEmpty();
					for (int i = 0; i < compatList.count() && !showItem; i++)
						for (int j = 0; j < compatFilters.count() && !showItem; j++)
							showItem = (compatList[i] == compatFilters[j]);
				}
				item->setHidden(!showItem);
				item->setText(QMC2_SWLIST_COLUMN_TITLE, swItem->text(QMC2_SWLIST_COLUMN_TITLE));
				item->setIcon(QMC2_SWLIST_COLUMN_TITLE, swItem->icon(QMC2_SWLIST_COLUMN_TITLE));
				item->setText(QMC2_SWLIST_COLUMN_NAME, swItem->text(QMC2_SWLIST_COLUMN_NAME));
				item->setText(QMC2_SWLIST_COLUMN_PUBLISHER, swItem->text(QMC2_SWLIST_COLUMN_PUBLISHER));
				item->setText(QMC2_SWLIST_COLUMN_YEAR, swItem->text(QMC2_SWLIST_COLUMN_YEAR));
				item->setText(QMC2_SWLIST_COLUMN_PART, swItem->text(QMC2_SWLIST_COLUMN_PART));
				item->setText(QMC2_SWLIST_COLUMN_INTERFACE, swItem->text(QMC2_SWLIST_COLUMN_INTERFACE));
				item->setText(QMC2_SWLIST_COLUMN_LIST, swItem->text(QMC2_SWLIST_COLUMN_LIST));
				item->setText(QMC2_SWLIST_COLUMN_SUPPORTED, swItem->text(QMC2_SWLIST_COLUMN_SUPPORTED));
				SoftwareItem *subItem = new SoftwareItem(item);
				subItem->setText(QMC2_SWLIST_COLUMN_TITLE, tr("Waiting for data..."));
#if defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
				if ( configNames.count() > i )
					item->setText(QMC2_SWLIST_COLUMN_DEVICECFG, configNames[i]);
#endif
			}
		}
		actionSaveFavoritesToFile->setEnabled(softwareNames.count() > 0);
		toolButtonFavoritesOptions->setEnabled(true);
		toolButtonExport->setEnabled(true);
	}

	treeWidgetKnownSoftware->setSortingEnabled(true);
	treeWidgetKnownSoftware->header()->setSortIndicatorShown(true);
	treeWidgetFavoriteSoftware->setSortingEnabled(true);
	treeWidgetFavoriteSoftware->header()->setSortIndicatorShown(true);
	treeWidgetSearchResults->setSortingEnabled(true);
	treeWidgetSearchResults->header()->setSortIndicatorShown(true);

	toolButtonReload->setEnabled(true);
	toolButtonToggleSoftwareInfo->setEnabled(true);
	toolButtonCompatFilterToggle->setEnabled(true);
	toolButtonToggleSnapnameAdjustment->setEnabled(true);
	toolButtonSoftwareStates->setEnabled(true);

	isLoading = false;
	fullyLoaded = !interruptLoad;
	isInitialLoad = false;
	return true;
}

bool SoftwareList::save()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::save()");
#endif

	if ( !fullyLoaded )
		return false;

	qmc2Config->remove(QString(QMC2_EMULATOR_PREFIX + "Favorites/%1").arg(systemName));

	QStringList softwareNames;
#if defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
	QStringList configNames;
	bool onlyEmptyConfigNames = true;
#endif

	for (int i = 0; i < treeWidgetFavoriteSoftware->topLevelItemCount(); i++) {
		QTreeWidgetItem *item = treeWidgetFavoriteSoftware->topLevelItem(i);
		softwareNames << item->text(QMC2_SWLIST_COLUMN_NAME);
#if defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
		QString s = item->text(QMC2_SWLIST_COLUMN_DEVICECFG);
		if ( !s.isEmpty() )
			onlyEmptyConfigNames = false;
		configNames << s;
#endif
	}

	if ( !softwareNames.isEmpty() ) {
#if defined(QMC2_EMUTYPE_MAME)
		qmc2Config->setValue(QString(QMC2_EMULATOR_PREFIX + "Favorites/%1/SoftwareNames").arg(systemName), softwareNames);
#elif defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
		qmc2Config->setValue(QString(QMC2_EMULATOR_PREFIX + "Favorites/%1/SoftwareNames").arg(systemName), softwareNames);
		if ( onlyEmptyConfigNames )
			qmc2Config->remove(QString(QMC2_EMULATOR_PREFIX + "Favorites/%1/DeviceConfigs").arg(systemName));
		else
			qmc2Config->setValue(QString(QMC2_EMULATOR_PREFIX + "Favorites/%1/DeviceConfigs").arg(systemName), configNames);
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

	snapForced = false;

	if ( qmc2SoftwareSnap )
		if ( qmc2SoftwareSnap->geometry().contains(QCursor::pos()) ) {
			snapForced = true;
			snapTimer.start(QMC2_SWSNAP_DELAY);
		}

	if ( !snapForced )
		cancelSoftwareSnap();
	else
		QTimer::singleShot(QMC2_SWSNAP_UNFORCE_DELAY, this, SLOT(checkSoftwareSnap()));

	QWidget::leaveEvent(e);
}

void SoftwareList::checkSoftwareSnap()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::checkSoftwareSnap()");
#endif

	if ( qmc2SoftwareSnap && qmc2SoftwareSnap->isVisible() ) {
		if ( !qmc2SoftwareSnap->geometry().contains(QCursor::pos()) && !qmc2SoftwareSnap->ctxMenuRequested )
			cancelSoftwareSnap();
		else {
			qmc2SoftwareSnap->ctxMenuRequested = qmc2SoftwareSnap->contextMenu->isVisible();
			QTimer::singleShot(QMC2_SWSNAP_UNFORCE_DELAY, this, SLOT(checkSoftwareSnap()));
		}
	}
}

void SoftwareList::updateDetail()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::updateDetail()");
#endif

	qmc2MainWindow->tabWidgetSoftwareDetail_updateCurrent();
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
	loadFinishedFlag = false;
	qmc2MainWindow->progressBarGamelist->setRange(0, 0);
	qmc2MainWindow->progressBarGamelist->reset();
}

void SoftwareList::loadFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::loadFinished(int exitCode = %1, QProcess::ExitStatus exitStatus = %2)").arg(exitCode).arg(exitStatus));
#endif

	if ( exitStatus == QProcess::NormalExit && exitCode == 0 ) {
		validData = true;
	} else {
#if defined(QMC2_EMUTYPE_MAME)
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the external process called to load the MAME software lists didn't exit cleanly -- exitCode = %1, exitStatus = %2").arg(exitCode).arg(exitStatus == QProcess::NormalExit ? tr("normal") : tr("crashed")));
#elif defined(QMC2_EMUTYPE_MESS)
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the external process called to load the MESS software lists didn't exit cleanly -- exitCode = %1, exitStatus = %2").arg(exitCode).arg(exitStatus == QProcess::NormalExit ? tr("normal") : tr("crashed")));
#elif defined(QMC2_EMUTYPE_UME)
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the external process called to load the UME software lists didn't exit cleanly -- exitCode = %1, exitStatus = %2").arg(exitCode).arg(exitStatus == QProcess::NormalExit ? tr("normal") : tr("crashed")));
#endif
		validData = false;
	}
	QTime elapsedTime(0, 0, 0, 0);
	elapsedTime = elapsedTime.addMSecs(loadTimer.elapsed());
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading XML software list data and (re)creating cache, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));

	if ( fileSWLCache.isOpen() )
		fileSWLCache.close();

	loadFinishedFlag = true;

	qmc2MainWindow->progressBarGamelist->setRange(oldMin, oldMax);
	qmc2MainWindow->progressBarGamelist->setFormat(oldFmt);
	qmc2MainWindow->progressBarGamelist->reset();
}

void SoftwareList::loadReadyReadStandardOutput()
{
	QProcess *proc = (QProcess *)sender();

#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::loadReadyReadStandardOutput()"));
#endif

#if defined(QMC2_OS_WIN)
	QString s = swlLastLine + QString::fromLatin1(proc->readAllStandardOutput());
	s.replace("\r\n", "\n"); // convert WinDOS's "0x0D 0x0A" to just "0x0A" 
#else
	QString s = swlLastLine + proc->readAllStandardOutput();
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
#elif defined(QMC2_EMUTYPE_UME)
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the currently selected UME emulator doesn't support software lists"));
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
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::loadError(QProcess::ProcessError processError = %1)").arg(processError));
#endif

#if defined(QMC2_EMUTYPE_MAME)
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the external process called to load the MAME software lists caused an error -- processError = %1").arg(processError));
#elif defined(QMC2_EMUTYPE_MESS)
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the external process called to load the MESS software lists caused an error -- processError = %1").arg(processError));
#elif defined(QMC2_EMUTYPE_MESS)
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the external process called to load the UME software lists caused an error -- processError = %1").arg(processError));
#endif
	validData = false;
	loadFinishedFlag = true;

	if ( fileSWLCache.isOpen() )
		fileSWLCache.close();

	qmc2MainWindow->progressBarGamelist->setRange(0, 1);
	qmc2MainWindow->progressBarGamelist->reset();
}

void SoftwareList::loadStateChanged(QProcess::ProcessState processState)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::loadStateChanged(QProcess::ProcessState processState = %1)").arg(processState));
#endif

}

void SoftwareList::checkSoftwareStates()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::checkSoftwareStates()"));
#endif

	QStringList softwareLists = systemSoftwareListMap[systemName];
	progressBar->setFormat(tr("Checking software-states - %p%"));
	progressBar->setRange(0, treeWidgetKnownSoftware->topLevelItemCount());
	progressBar->setValue(0);

	QWidget *focusWidget = qApp->focusWidget();
	qmc2MainWindow->tabWidgetGamelist->setEnabled(false);
	qmc2MainWindow->menuBar()->setEnabled(false);
	qmc2MainWindow->toolbar->setEnabled(false);
	actionCheckSoftwareStates->setEnabled(false);

	if ( treeWidgetKnownSoftware->topLevelItemCount() > QMC2_SWLIST_COUNT_THRESHOLD ) {
		labelLoadingSoftwareLists->setText(tr("Checking software-states, please wait..."));
		labelLoadingSoftwareLists->setVisible(true);
		toolBoxSoftwareList->setVisible(false);
	}

	numSoftwareCorrect = numSoftwareIncorrect = numSoftwareMostlyCorrect = numSoftwareNotFound = numSoftwareUnknown = 0;
	updateStats();

	foreach (QString softwareList, softwareLists) {
		if ( softwareList == "NO_SOFTWARE_LIST" )
			break;

		if ( verifyProc )
			delete verifyProc;

		verifyProc = new QProcess(this);

		connect(verifyProc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(verifyError(QProcess::ProcessError)));
		connect(verifyProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(verifyFinished(int, QProcess::ExitStatus)));
		connect(verifyProc, SIGNAL(readyReadStandardOutput()), this, SLOT(verifyReadyReadStandardOutput()));
		connect(verifyProc, SIGNAL(readyReadStandardError()), this, SLOT(verifyReadyReadStandardError()));
		connect(verifyProc, SIGNAL(started()), this, SLOT(verifyStarted()));
		connect(verifyProc, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(verifyStateChanged(QProcess::ProcessState)));

		softwareListName = softwareList;
		swStatesLastLine.clear();
		softwareListStateMap[softwareListName].clear();
		softwareListItems = treeWidgetKnownSoftware->findItems(softwareList, Qt::MatchExactly, QMC2_SWLIST_COLUMN_LIST);
		favoritesListItems = treeWidgetFavoriteSoftware->findItems(softwareList, Qt::MatchExactly, QMC2_SWLIST_COLUMN_LIST);
		searchListItems = treeWidgetSearchResults->findItems(softwareList, Qt::MatchExactly, QMC2_SWLIST_COLUMN_LIST);

		QString softwareStateCachePath = QDir::toNativeSeparators(QDir::cleanPath(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareStateCache").toString() + "/" + softwareList + ".ssc"));
		softwareStateFile.setFileName(softwareStateCachePath);
		if ( !softwareStateFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text) )
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: can't open software state cache file '%1' for writing, please check access permissions").arg(softwareStateCachePath));
		else {
			softwareStateStream.setDevice(&softwareStateFile);
			softwareStateStream << "# THIS FILE IS AUTO-GENERATED - PLEASE DO NOT EDIT!\n";
		}

		QString command = qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ExecutableFile").toString();
		QStringList args;
		args << "-verifysoftlist" << softwareList;
		QString romPath = qmc2Config->value(QMC2_EMULATOR_PREFIX + "Configuration/Global/rompath").toString().replace("~", "$HOME");
		if ( !romPath.isEmpty() )
			args << "-rompath" << romPath;
		QString hashPath = qmc2Config->value(QMC2_EMULATOR_PREFIX + "Configuration/Global/hashpath").toString().replace("~", "$HOME");
		if ( !hashPath.isEmpty() )
			args << "-hashpath" << hashPath;

		if ( !qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/WorkingDirectory", QString()).toString().isEmpty() )
			verifyProc->setWorkingDirectory(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/WorkingDirectory").toString());

		verifyProc->start(command, args);

		verifyReadingStdout = false;
		int retries = 0;
		bool started = verifyProc->waitForStarted(QMC2_PROCESS_POLL_TIME);
		while ( !started && retries++ < QMC2_PROCESS_POLL_RETRIES )
			started = verifyProc->waitForStarted(QMC2_PROCESS_POLL_TIME_LONG);

		if ( started ) {
			bool verifyProcRunning = (verifyProc->state() == QProcess::Running);
			while ( !verifyProc->waitForFinished(QMC2_PROCESS_POLL_TIME) && verifyProcRunning ) {
				qApp->processEvents();
				verifyProcRunning = (verifyProc->state() == QProcess::Running);
			}
			verifyProc->waitForFinished();
		} else {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't start emulator executable within a reasonable time frame, giving up"));
			break;
		}
	}

	labelLoadingSoftwareLists->setVisible(false);
	toolBoxSoftwareList->setVisible(true);

	qmc2MainWindow->tabWidgetGamelist->setEnabled(true);
	qmc2MainWindow->menuBar()->setEnabled(true);
	qmc2MainWindow->toolbar->setEnabled(true);
	actionCheckSoftwareStates->setEnabled(true);
	if ( focusWidget )
		focusWidget->setFocus();

	QTimer::singleShot(0, progressBar, SLOT(hide()));
}

void SoftwareList::verifyStarted()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::verifyStarted()"));
#endif

	progressBar->setVisible(true);
}

void SoftwareList::verifyFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::verifyFinished(int exitCode = %1, QProcess::ExitStatus exitStatus = %2)").arg(exitCode).arg(exitStatus));
#endif

	while ( verifyReadingStdout ) {
		QTest::qWait(10);
		qApp->processEvents();
	}

	bool notFoundState = true;

	if ( (exitStatus != QProcess::NormalExit || exitCode != 0) && exitCode != 2 ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the external process called to verify the states for software-list '%1' didn't exit cleanly -- exitCode = %2, exitStatus = %3").arg(softwareListName).arg(exitCode).arg(exitStatus == QProcess::NormalExit ? tr("normal") : tr("crashed")));
		notFoundState = false;
	}

	for (int i = 0; i < softwareListItems.count(); i++) {
		QTreeWidgetItem *softwareItem = softwareListItems[i];
		QString softwareName = softwareItem->text(QMC2_SWLIST_COLUMN_NAME);

		QTreeWidgetItem *favoriteItem = NULL;
		foreach (QTreeWidgetItem *item, favoritesListItems) {
			if ( item->text(QMC2_SWLIST_COLUMN_NAME) == softwareName ) {
				favoriteItem = item;
				break;
			}
		}

		QTreeWidgetItem *searchItem = NULL;
		foreach (QTreeWidgetItem *item, searchListItems) {
			if ( item->text(QMC2_SWLIST_COLUMN_NAME) == softwareName ) {
				searchItem = item;
				break;
			}
		}

		QString listName = softwareItem->text(QMC2_SWLIST_COLUMN_LIST);
		if ( !softwareListStateMap[listName].contains(softwareName) ) {
			progressBar->setValue(progressBar->value() + 1);
			if ( notFoundState ) {
				softwareItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_notfound.png")));
				softwareItem->setWhatsThis(QMC2_SWLIST_COLUMN_NAME, "N");
				if ( stateFilter->checkBoxStateFilter->isChecked() )
					softwareItem->setHidden(!stateFilter->toolButtonNotFound->isChecked());
				else
					softwareItem->setHidden(false);
				if ( favoriteItem )
					favoriteItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_notfound.png")));
				if ( searchItem )
					searchItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_notfound.png")));
				if ( softwareStateFile.isOpen() )
					softwareStateStream << softwareName << " N\n";
				softwareListStateMap[listName][softwareName] = 'N';
				numSoftwareNotFound++;
			} else {
				softwareItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_unknown.png")));
				softwareItem->setWhatsThis(QMC2_SWLIST_COLUMN_NAME, "U");
				if ( stateFilter->checkBoxStateFilter->isChecked() )
					softwareItem->setHidden(!stateFilter->toolButtonUnknown->isChecked());
				else
					softwareItem->setHidden(false);
				if ( favoriteItem )
					favoriteItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_unknown.png")));
				if ( searchItem )
					searchItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_unknown.png")));
				if ( softwareStateFile.isOpen() )
					softwareStateStream << softwareName << " U\n";
				softwareListStateMap[listName][softwareName] = 'U';
				numSoftwareUnknown++;
			}
		}

		if ( i % QMC2_SWLIST_CHECK_RESPONSE == 0 ) {
			updateStats();
			qApp->processEvents();
		}
	}

	updateStats();

	if ( softwareStateFile.isOpen() )
		softwareStateFile.close();

	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("state info for software-list '%1': L:%2 C:%3 M:%4 I:%5 N:%6 U:%7").arg(softwareListName).arg(softwareListItems.count()).arg(numSoftwareCorrect).arg(numSoftwareMostlyCorrect).arg(numSoftwareIncorrect).arg(numSoftwareNotFound).arg(numSoftwareUnknown));

	if ( toolButtonCompatFilterToggle->isChecked() )
		on_toolButtonCompatFilterToggle_clicked(true);

	softwareListItems.clear();
}

void SoftwareList::verifyReadyReadStandardOutput()
{
	verifyReadingStdout = true;

#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::verifyReadyReadStandardOutput()"));
#endif

#if defined(QMC2_OS_WIN)
	QString s = swStatesLastLine + QString::fromLatin1(verifyProc->readAllStandardOutput());
	s.replace("\r\n", "\n"); // convert WinDOS's "0x0D 0x0A" to just "0x0A" 
#else
	QString s = swStatesLastLine + verifyProc->readAllStandardOutput();
#endif
	QStringList lines = s.split("\n");

	if ( s.endsWith("\n") ) {
		swStatesLastLine.clear();
	} else {
		swStatesLastLine = lines.last();
		lines.removeLast();
	}
 
	foreach (QString line, lines) {
		line = line.simplified();
		if ( !line.isEmpty() ) {
			QStringList words = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
			if ( line.startsWith("romset") ) {
				progressBar->setValue(progressBar->value() + 1);
				QStringList romsetWords = words[1].split(":", QString::SkipEmptyParts);
				QString listName = romsetWords[0];
				QString softwareName = romsetWords[1];
				QString status = words.last();

				QTreeWidgetItem *softwareItem = NULL;
				foreach (QTreeWidgetItem *item, softwareListItems) {
					if ( item->text(QMC2_SWLIST_COLUMN_NAME) == softwareName ) {
						softwareItem = item;
						break;
					}
				}

				if ( !softwareItem )
					continue;

				QTreeWidgetItem *favoriteItem = NULL;
				foreach (QTreeWidgetItem *item, favoritesListItems) {
					if ( item->text(QMC2_SWLIST_COLUMN_NAME) == softwareName ) {
						favoriteItem = item;
						break;
					}
				}

				QTreeWidgetItem *searchItem = NULL;
				foreach (QTreeWidgetItem *item, searchListItems) {
					if ( item->text(QMC2_SWLIST_COLUMN_NAME) == softwareName ) {
						searchItem = item;
						break;
					}
				}

				char charStatus = 'U';
				if ( status == "good" )
					charStatus = 'C';
				else if ( status == "bad" )
					charStatus = 'I';
				else if ( status == "available" )
					charStatus = 'M';

				softwareListStateMap[listName][softwareName] = charStatus;

				switch ( charStatus ) {
					case 'C':
						softwareItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_correct.png")));
						softwareItem->setWhatsThis(QMC2_SWLIST_COLUMN_NAME, "C");
						if ( stateFilter->checkBoxStateFilter->isChecked() )
							softwareItem->setHidden(!stateFilter->toolButtonCorrect->isChecked());
						else
							softwareItem->setHidden(false);
						if ( favoriteItem )
							favoriteItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_correct.png")));
						if ( searchItem )
							searchItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_correct.png")));
						if ( softwareStateFile.isOpen() )
							softwareStateStream << softwareName << " C\n";
						numSoftwareCorrect++;
						break;
					case 'M':
						softwareItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_mostlycorrect.png")));
						softwareItem->setWhatsThis(QMC2_SWLIST_COLUMN_NAME, "M");
						if ( stateFilter->checkBoxStateFilter->isChecked() )
							softwareItem->setHidden(!stateFilter->toolButtonMostlyCorrect->isChecked());
						else
							softwareItem->setHidden(false);
						if ( favoriteItem )
							favoriteItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_mostlycorrect.png")));
						if ( searchItem )
							searchItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_mostlycorrect.png")));
						if ( softwareStateFile.isOpen() )
							softwareStateStream << softwareName << " M\n";
						numSoftwareMostlyCorrect++;
						break;
					case 'I':
						softwareItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_incorrect.png")));
						softwareItem->setWhatsThis(QMC2_SWLIST_COLUMN_NAME, "I");
						if ( stateFilter->checkBoxStateFilter->isChecked() )
							softwareItem->setHidden(!stateFilter->toolButtonIncorrect->isChecked());
						else
							softwareItem->setHidden(false);
						if ( favoriteItem )
							favoriteItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_incorrect.png")));
						if ( searchItem )
							searchItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_incorrect.png")));
						if ( softwareStateFile.isOpen() )
							softwareStateStream << softwareName << " I\n";
						numSoftwareIncorrect++;
						break;
					case 'U':
						softwareItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_unknown.png")));
						softwareItem->setWhatsThis(QMC2_SWLIST_COLUMN_NAME, "U");
						if ( stateFilter->checkBoxStateFilter->isChecked() )
							softwareItem->setHidden(!stateFilter->toolButtonUnknown->isChecked());
						else
							softwareItem->setHidden(false);
						if ( favoriteItem )
							favoriteItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_unknown.png")));
						if ( searchItem )
							searchItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_unknown.png")));
						if ( softwareStateFile.isOpen() )
							softwareStateStream << softwareName << " U\n";
						numSoftwareUnknown++;
						break;
				}
			}
		}
	}

	updateStats();

	verifyReadingStdout = false;
}

void SoftwareList::verifyReadyReadStandardError()
{
#ifdef QMC2_DEBUG
	QString data = verifyProc->readAllStandardError();
	data = data.trimmed();
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::verifyReadyReadStandardError(): data = '%1'").arg(data));
#endif
}

void SoftwareList::verifyError(QProcess::ProcessError processError)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::verifyError(QProcess::ProcessError processError = %1)").arg(processError));
#endif

	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the external process called to verify software-states caused an error -- processError = %1").arg(processError));

	progressBar->setVisible(false);
}

void SoftwareList::verifyStateChanged(QProcess::ProcessState processState)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::verifyStateChanged(QProcess::ProcessState processState = %1)").arg(processState));
#endif
}

void SoftwareList::on_toolButtonToggleSnapnameAdjustment_clicked(bool checked)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_toolButtonToggleSnapnameAdjustment_clicked(bool checked = %1)").arg(checked));
#endif

	if ( checked && mountedSoftware.count() > 1 )
		comboBoxSnapnameDevice->show();
	else
		comboBoxSnapnameDevice->hide();
}

void SoftwareList::on_toolButtonSoftwareStates_toggled(bool checked)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_toolButtonSoftwareStates_toggled(bool checked = %1)").arg(checked));
#endif

	QString itemText = toolBoxSoftwareList->itemText(QMC2_SWLIST_KNOWN_SW_PAGE);
	itemText.remove(QRegExp(" - " + tr("filtered") + "$"));

	if ( checked ) {
		toolButtonSoftwareStates->setMenu(menuSoftwareStates);
		if ( isReady )
			qmc2SoftwareList->toolBoxSoftwareList->setItemText(QMC2_SWLIST_KNOWN_SW_PAGE, itemText + " - " + tr("filtered"));
	} else {
		toolButtonSoftwareStates->setMenu(NULL);
		if ( isReady )
			qmc2SoftwareList->toolBoxSoftwareList->setItemText(QMC2_SWLIST_KNOWN_SW_PAGE, itemText);
	}
	updateStats();

	if ( isReady )
		QTimer::singleShot(0, toolButtonReload, SLOT(animateClick()));

	qApp->processEvents();
}

void SoftwareList::on_toolButtonToggleSoftwareInfo_clicked(bool checked)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_toolButtonToggleSoftwareInfo_clicked(bool checked = %1)").arg(checked));
#endif

	QTreeWidget *treeWidget = NULL;

	switch ( toolBoxSoftwareList->currentIndex() ) {
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

	if ( !treeWidget ) {
		qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_DEFAULT_PAGE);
		qmc2MainWindow->vSplitter->setSizes(qmc2MainWindow->vSplitterSizes);
		qmc2MainWindow->on_tabWidgetLogsAndEmulators_currentChanged(qmc2MainWindow->tabWidgetLogsAndEmulators->currentIndex());
		return;
	}

	checked &= (treeWidget->selectedItems().count() > 0);

	if ( checked ) {
		qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_SOFTWARE_PAGE);
		qmc2MainWindow->tabWidgetSoftwareDetail_updateCurrent();
		if ( qmc2MainWindow->floatToggleButtonSoftwareDetail->isChecked() )
			qmc2MainWindow->vSplitter->setSizes(qmc2MainWindow->vSplitterSizesSoftwareDetail);
	} else {
		qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_DEFAULT_PAGE);
		qmc2MainWindow->on_tabWidgetLogsAndEmulators_currentChanged(qmc2MainWindow->tabWidgetLogsAndEmulators->currentIndex());
		qmc2MainWindow->vSplitter->setSizes(qmc2MainWindow->vSplitterSizes);
	}
}

void SoftwareList::on_toolButtonCompatFilterToggle_clicked(bool checked)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_toolButtonCompatFilterToggle_clicked(bool checked = %1)").arg(checked));
#endif

	QStringList compatFilters = systemSoftwareFilterMap[qmc2SoftwareList->systemName];
	for (int count = 0; count < treeWidgetKnownSoftware->topLevelItemCount(); count++) {
		QTreeWidgetItem *item = treeWidgetKnownSoftware->topLevelItem(count);
		QStringList compatList = item->whatsThis(QMC2_SWLIST_COLUMN_TITLE).split(",", QString::SkipEmptyParts);
		bool showItem = true;
		if ( checked ) {
			showItem = compatList.isEmpty() || compatFilters.isEmpty();
			for (int i = 0; i < compatList.count() && !showItem; i++)
				for (int j = 0; j < compatFilters.count() && !showItem; j++)
					showItem = (compatList[i] == compatFilters[j]);
		}
		if ( toolButtonSoftwareStates->isChecked() && stateFilter->checkBoxStateFilter->isChecked() ) {
			switch ( item->whatsThis(QMC2_SWLIST_COLUMN_NAME).at(0).toLatin1() ) {
				case 'C':
					item->setHidden(!stateFilter->toolButtonCorrect->isChecked() || !showItem);
					break;
				case 'M':
					item->setHidden(!stateFilter->toolButtonMostlyCorrect->isChecked() || !showItem);
					break;
				case 'I':
					item->setHidden(!stateFilter->toolButtonIncorrect->isChecked() || !showItem);
					break;
				case 'N':
					item->setHidden(!stateFilter->toolButtonNotFound->isChecked() || !showItem);
					break;
				case 'U':
				default:
					item->setHidden(!stateFilter->toolButtonUnknown->isChecked() || !showItem);
					break;
			}
			if ( item->isHidden() && item->isSelected() )
				item->setSelected(false);
		} else {
			if ( !showItem ) {
				item->setHidden(true);
				if ( item->isSelected() )
					item->setSelected(false);
			} else
				item->setHidden(false);
		}
	}
	for (int count = 0; count < treeWidgetFavoriteSoftware->topLevelItemCount(); count++) {
		QTreeWidgetItem *item = treeWidgetFavoriteSoftware->topLevelItem(count);
		QStringList compatList = item->whatsThis(QMC2_SWLIST_COLUMN_TITLE).split(",", QString::SkipEmptyParts);
		bool showItem = true;
		if ( checked ) {
			showItem = compatList.isEmpty() || compatFilters.isEmpty();
			for (int i = 0; i < compatList.count() && !showItem; i++)
				for (int j = 0; j < compatFilters.count() && !showItem; j++)
					showItem = (compatList[i] == compatFilters[j]);
		}
		if ( !showItem ) {
			item->setHidden(true);
			if ( item->isSelected() )
				item->setSelected(false);
		} else
			item->setHidden(false);
	}
	for (int count = 0; count < treeWidgetSearchResults->topLevelItemCount(); count++) {
		QTreeWidgetItem *item = treeWidgetSearchResults->topLevelItem(count);
		QStringList compatList = item->whatsThis(QMC2_SWLIST_COLUMN_TITLE).split(",", QString::SkipEmptyParts);
		bool showItem = true;
		if ( checked ) {
			showItem = compatList.isEmpty() || compatFilters.isEmpty();
			for (int i = 0; i < compatList.count() && !showItem; i++)
				for (int j = 0; j < compatFilters.count() && !showItem; j++)
					showItem = (compatList[i] == compatFilters[j]);
		}
		if ( !showItem ) {
			item->setHidden(true);
			if ( item->isSelected() )
				item->setSelected(false);
		} else
			item->setHidden(false);
	}
}

void SoftwareList::on_toolButtonReload_clicked(bool checked)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_toolButtonReload_clicked(bool checked = %1)").arg(checked));
#endif

	save();

	numSoftwareTotal = numSoftwareCorrect = numSoftwareIncorrect = numSoftwareMostlyCorrect = numSoftwareNotFound = numSoftwareUnknown = 0;
	updateStats();

	treeWidgetKnownSoftware->clear();
	treeWidgetFavoriteSoftware->clear();
	treeWidgetSearchResults->clear();
	toolBoxSoftwareList->setEnabled(false);
	toolButtonAddToFavorites->setEnabled(false);
	toolButtonRemoveFromFavorites->setEnabled(false);
	toolButtonFavoritesOptions->setEnabled(false);
	toolButtonPlay->setEnabled(false);
	toolButtonPlayEmbedded->setEnabled(false);
	toolButtonExport->setEnabled(false);
	toolButtonToggleSoftwareInfo->setEnabled(false);
	toolButtonCompatFilterToggle->setEnabled(false);
	toolButtonToggleSnapnameAdjustment->setEnabled(false);
	toolButtonSoftwareStates->setEnabled(false);
	comboBoxDeviceConfiguration->setEnabled(false);
	comboBoxDeviceConfiguration->clear();
	comboBoxDeviceConfiguration->insertItem(0, tr("Default configuration"));
	qApp->processEvents();

	QTimer::singleShot(0, this, SLOT(load()));
}

void SoftwareList::on_toolButtonExport_clicked(bool checked)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_toolButtonExport_clicked(bool checked = %1)").arg(checked));
#endif

	if ( !exporter )
		exporter = new SoftwareListExporter(this);

	exporter->show();
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

	QStringList compatFilters = systemSoftwareFilterMap[qmc2SoftwareList->systemName];
	if ( si ) {
		while ( si->parent() ) si = si->parent();
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
			item->setIcon(QMC2_SWLIST_COLUMN_TITLE, si->icon(QMC2_SWLIST_COLUMN_TITLE));
			item->setWhatsThis(QMC2_SWLIST_COLUMN_TITLE, si->whatsThis(QMC2_SWLIST_COLUMN_TITLE));
			item->setWhatsThis(QMC2_SWLIST_COLUMN_NAME, si->whatsThis(QMC2_SWLIST_COLUMN_NAME));
			QStringList compatList = item->whatsThis(QMC2_SWLIST_COLUMN_TITLE).split(",", QString::SkipEmptyParts);
			bool showItem = true;
			if ( toolButtonCompatFilterToggle->isChecked() ) {
				showItem = compatList.isEmpty() || compatFilters.isEmpty();
				for (int i = 0; i < compatList.count() && !showItem; i++)
					for (int j = 0; j < compatFilters.count() && !showItem; j++)
						showItem = (compatList[i] == compatFilters[j]);
			}
			item->setHidden(!showItem);
			item->setText(QMC2_SWLIST_COLUMN_NAME, si->text(QMC2_SWLIST_COLUMN_NAME));
			item->setText(QMC2_SWLIST_COLUMN_PUBLISHER, si->text(QMC2_SWLIST_COLUMN_PUBLISHER));
			item->setText(QMC2_SWLIST_COLUMN_YEAR, si->text(QMC2_SWLIST_COLUMN_YEAR));
			item->setText(QMC2_SWLIST_COLUMN_PART, si->text(QMC2_SWLIST_COLUMN_PART));
			item->setText(QMC2_SWLIST_COLUMN_INTERFACE, si->text(QMC2_SWLIST_COLUMN_INTERFACE));
			item->setText(QMC2_SWLIST_COLUMN_LIST, si->text(QMC2_SWLIST_COLUMN_LIST));
			item->setText(QMC2_SWLIST_COLUMN_SUPPORTED, si->text(QMC2_SWLIST_COLUMN_SUPPORTED));
#if defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
			if ( comboBoxDeviceConfiguration->currentIndex() > 0 )
				item->setText(QMC2_SWLIST_COLUMN_DEVICECFG, comboBoxDeviceConfiguration->currentText());
			else
				item->setText(QMC2_SWLIST_COLUMN_DEVICECFG, QString());
#endif
		}
	}

	actionSaveFavoritesToFile->setEnabled(treeWidgetFavoriteSoftware->topLevelItemCount() > 0);
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

	actionSaveFavoritesToFile->setEnabled(treeWidgetFavoriteSoftware->topLevelItemCount() > 0);
}

void SoftwareList::on_toolButtonPlay_clicked(bool checked)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_toolButtonPlay_clicked(bool checked = %1)").arg(checked));
#endif

	QTimer::singleShot(0, qmc2MainWindow, SLOT(on_actionPlay_triggered()));
}

void SoftwareList::on_toolButtonPlayEmbedded_clicked(bool checked)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_toolButtonPlayEmbedded_clicked(bool checked = %1)").arg(checked));
#endif

	QTimer::singleShot(0, qmc2MainWindow, SLOT(on_actionPlayEmbedded_triggered()));
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

	if ( item->childCount() < 1 )
		return;

	if ( item->child(0)->text(QMC2_SWLIST_COLUMN_TITLE) == tr("Waiting for data...") ) {
		QString softwareListXml = getSoftwareListXmlData(item->text(QMC2_SWLIST_COLUMN_LIST));
		if ( !softwareListXml.isEmpty() ) {
			QXmlInputSource xmlInputSource;
			xmlInputSource.setData(softwareListXml);
			successfulLookups.clear();
			SoftwareEntryXmlHandler xmlHandler(item);
			QXmlSimpleReader xmlReader;
			xmlReader.setContentHandler(&xmlHandler);
			xmlReader.setFeature("http://xml.org/sax/features/namespaces", false);
			xmlReader.setFeature("http://trolltech.com/xml/features/report-whitespace-only-CharData", false);
			treeWidgetKnownSoftware->setSortingEnabled(false);
			item->child(0)->setText(QMC2_SWLIST_COLUMN_TITLE, tr("Searching"));
			treeWidgetKnownSoftware->viewport()->update();
			qApp->processEvents();
			if ( !xmlReader.parse(xmlInputSource) )
				if ( !xmlHandler.success )
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: error while parsing XML data for software list entry '%1:%2'").arg(item->text(QMC2_SWLIST_COLUMN_LIST)).arg(item->text(QMC2_SWLIST_COLUMN_NAME)));
			treeWidgetKnownSoftware->setSortingEnabled(true);
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

	if ( item->childCount() < 1 )
		return;

	if ( item->child(0)->text(QMC2_SWLIST_COLUMN_TITLE) == tr("Waiting for data...") ) {
		QString softwareListXml = getSoftwareListXmlData(item->text(QMC2_SWLIST_COLUMN_LIST));
		if ( !softwareListXml.isEmpty() ) {
			QXmlInputSource xmlInputSource;
			xmlInputSource.setData(softwareListXml);
			successfulLookups.clear();
			SoftwareEntryXmlHandler xmlHandler(item);
			QXmlSimpleReader xmlReader;
			xmlReader.setContentHandler(&xmlHandler);
			xmlReader.setFeature("http://xml.org/sax/features/namespaces", false);
			xmlReader.setFeature("http://trolltech.com/xml/features/report-whitespace-only-CharData", false);
			treeWidgetFavoriteSoftware->setSortingEnabled(false);
			item->child(0)->setText(QMC2_SWLIST_COLUMN_TITLE, tr("Searching"));
			treeWidgetFavoriteSoftware->viewport()->update();
			qApp->processEvents();
			if ( !xmlReader.parse(xmlInputSource) )
				if ( !xmlHandler.success )
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: error while parsing XML data for software list entry '%1:%2'").arg(item->text(QMC2_SWLIST_COLUMN_LIST)).arg(item->text(QMC2_SWLIST_COLUMN_NAME)));
			treeWidgetFavoriteSoftware->setSortingEnabled(true);
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

	if ( item->childCount() < 1 )
		return;

	if ( item->child(0)->text(QMC2_SWLIST_COLUMN_TITLE) == tr("Waiting for data...") ) {
		QString softwareListXml = getSoftwareListXmlData(item->text(QMC2_SWLIST_COLUMN_LIST));
		if ( !softwareListXml.isEmpty() ) {
			QXmlInputSource xmlInputSource;
			xmlInputSource.setData(softwareListXml);
			successfulLookups.clear();
			SoftwareEntryXmlHandler xmlHandler(item);
			QXmlSimpleReader xmlReader;
			xmlReader.setContentHandler(&xmlHandler);
			xmlReader.setFeature("http://xml.org/sax/features/namespaces", false);
			xmlReader.setFeature("http://trolltech.com/xml/features/report-whitespace-only-CharData", false);
			treeWidgetSearchResults->setSortingEnabled(false);
			item->child(0)->setText(QMC2_SWLIST_COLUMN_TITLE, tr("Searching"));
			treeWidgetSearchResults->viewport()->update();
			qApp->processEvents();
			if ( !xmlReader.parse(xmlInputSource) )
				if ( !xmlHandler.success )
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: error while parsing XML data for software list entry '%1:%2'").arg(item->text(QMC2_SWLIST_COLUMN_LIST)).arg(item->text(QMC2_SWLIST_COLUMN_NAME)));
			treeWidgetSearchResults->setSortingEnabled(true);
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
			updateMountDevices();
			on_treeWidgetKnownSoftware_itemSelectionChanged();
			break;
		case QMC2_SWLIST_FAVORITES_PAGE:
			on_treeWidgetFavoriteSoftware_itemSelectionChanged();
			break;
		case QMC2_SWLIST_SEARCH_PAGE:
			updateMountDevices();
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
		qmc2SoftwareSnap->snapForcedResetTimer.stop();
		snapForced = true;
		qmc2SoftwareSnap->myItem = item;
		snapTimer.start(QMC2_SWSNAP_DELAY);
		if ( toolButtonToggleSoftwareInfo->isChecked() ) {
			qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_SOFTWARE_PAGE);
			if ( qmc2MainWindow->floatToggleButtonSoftwareDetail->isChecked() )
				qmc2MainWindow->vSplitter->setSizes(qmc2MainWindow->vSplitterSizesSoftwareDetail);
		} else {
			qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_DEFAULT_PAGE);
			qmc2MainWindow->on_tabWidgetLogsAndEmulators_currentChanged(qmc2MainWindow->tabWidgetLogsAndEmulators->currentIndex());
			qmc2MainWindow->vSplitter->setSizes(qmc2MainWindow->vSplitterSizes);
		}
		currentItem = item;
		while ( currentItem->parent() ) currentItem = currentItem->parent();
		if ( qmc2MainWindow->stackedWidgetSpecial->currentIndex() == QMC2_SPECIAL_SOFTWARE_PAGE )
			detailUpdateTimer.start(qmc2UpdateDelay);
	} else {
		qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_DEFAULT_PAGE);
		qmc2MainWindow->on_tabWidgetLogsAndEmulators_currentChanged(qmc2MainWindow->tabWidgetLogsAndEmulators->currentIndex());
		qmc2MainWindow->vSplitter->setSizes(qmc2MainWindow->vSplitterSizes);
		cancelSoftwareSnap();
		currentItem = NULL;
	}
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
		qmc2SoftwareSnap->snapForcedResetTimer.stop();
		snapForced = true;
		qmc2SoftwareSnap->myItem = item;
		snapTimer.start(QMC2_SWSNAP_DELAY);
		if ( toolButtonToggleSoftwareInfo->isChecked() ) {
			qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_SOFTWARE_PAGE);
			if ( qmc2MainWindow->floatToggleButtonSoftwareDetail->isChecked() )
				qmc2MainWindow->vSplitter->setSizes(qmc2MainWindow->vSplitterSizesSoftwareDetail);
		} else {
			qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_DEFAULT_PAGE);
			qmc2MainWindow->on_tabWidgetLogsAndEmulators_currentChanged(qmc2MainWindow->tabWidgetLogsAndEmulators->currentIndex());
			qmc2MainWindow->vSplitter->setSizes(qmc2MainWindow->vSplitterSizes);
		}
		currentItem = item;
		while ( currentItem->parent() ) currentItem = currentItem->parent();
		if ( qmc2MainWindow->stackedWidgetSpecial->currentIndex() == QMC2_SPECIAL_SOFTWARE_PAGE )
			detailUpdateTimer.start(qmc2UpdateDelay);
	} else {
		qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_DEFAULT_PAGE);
		qmc2MainWindow->on_tabWidgetLogsAndEmulators_currentChanged(qmc2MainWindow->tabWidgetLogsAndEmulators->currentIndex());
		qmc2MainWindow->vSplitter->setSizes(qmc2MainWindow->vSplitterSizes);
		cancelSoftwareSnap();
		currentItem = NULL;
	}
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
		qmc2SoftwareSnap->snapForcedResetTimer.stop();
		snapForced = true;
		qmc2SoftwareSnap->myItem = item;
		snapTimer.start(QMC2_SWSNAP_DELAY);
		if ( toolButtonToggleSoftwareInfo->isChecked() ) {
			qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_SOFTWARE_PAGE);
			if ( qmc2MainWindow->floatToggleButtonSoftwareDetail->isChecked() )
				qmc2MainWindow->vSplitter->setSizes(qmc2MainWindow->vSplitterSizesSoftwareDetail);
		} else {
			qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_DEFAULT_PAGE);
			qmc2MainWindow->on_tabWidgetLogsAndEmulators_currentChanged(qmc2MainWindow->tabWidgetLogsAndEmulators->currentIndex());
			qmc2MainWindow->vSplitter->setSizes(qmc2MainWindow->vSplitterSizes);
		}
		currentItem = item;
		while ( currentItem->parent() ) currentItem = currentItem->parent();
		if ( qmc2MainWindow->stackedWidgetSpecial->currentIndex() == QMC2_SPECIAL_SOFTWARE_PAGE )
			detailUpdateTimer.start(qmc2UpdateDelay);
	} else {
		qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_DEFAULT_PAGE);
		qmc2MainWindow->on_tabWidgetLogsAndEmulators_currentChanged(qmc2MainWindow->tabWidgetLogsAndEmulators->currentIndex());
		qmc2MainWindow->vSplitter->setSizes(qmc2MainWindow->vSplitterSizes);
		cancelSoftwareSnap();
		currentItem = NULL;
	}
}

void SoftwareList::on_treeWidgetKnownSoftware_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::on_treeWidgetKnownSoftware_customContextMenuRequested(const QPoint &p = ...)");
#endif

	cancelSoftwareSnap();

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

	cancelSoftwareSnap();

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

	cancelSoftwareSnap();

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

	if ( item == enteredItem )
		return;

	enteredItem = item;

	if ( !qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SoftwareSnapOnMouseHover", false).toBool() )
		return;

	if ( !snapForced ) {
		if ( qmc2SoftwareSnap ) {
			if ( qmc2SoftwareSnap->myItem != item ) {
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

	if ( item == enteredItem )
		return;

	enteredItem = item;

	if ( !qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SoftwareSnapOnMouseHover", false).toBool() )
		return;

	if ( !snapForced ) {
		if ( qmc2SoftwareSnap ) {
			if ( qmc2SoftwareSnap->myItem != item ) {
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

	if ( item == enteredItem )
		return;

	enteredItem = item;

	if ( !qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SoftwareSnapOnMouseHover", false).toBool() )
		return;

	if ( !snapForced ) {
		if ( qmc2SoftwareSnap ) {
			if ( qmc2SoftwareSnap->myItem != item ) {
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

	if ( !qmc2IgnoreItemActivation ) {
		cancelSoftwareSnap();
		switch ( qmc2DefaultLaunchMode ) {
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
			case QMC2_LAUNCH_MODE_EMBEDDED:
				QTimer::singleShot(0, this, SLOT(playEmbeddedActivated()));
				break;
#endif
			case QMC2_LAUNCH_MODE_INDEPENDENT:
			default:
				QTimer::singleShot(0, this, SLOT(playActivated()));
				break;
		}
	}
	qmc2IgnoreItemActivation = false;
}

void SoftwareList::on_treeWidgetKnownSoftware_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
	if ( !qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/DoubleClickActivation").toBool() ) {
		qmc2IgnoreItemActivation = true;
		item->setExpanded(!item->isExpanded());
	}
}

void SoftwareList::on_treeWidgetFavoriteSoftware_itemActivated(QTreeWidgetItem *item, int column)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_treeWidgetFavoriteSoftware_itemActivated(QTreeWidgetItem *item = %1, int column = %2)").arg((qulonglong)item).arg(column));
#endif

	if ( !qmc2IgnoreItemActivation ) {
		cancelSoftwareSnap();
		switch ( qmc2DefaultLaunchMode ) {
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
			case QMC2_LAUNCH_MODE_EMBEDDED:
				QTimer::singleShot(0, this, SLOT(playEmbeddedActivated()));
				break;
#endif
			case QMC2_LAUNCH_MODE_INDEPENDENT:
			default:
				QTimer::singleShot(0, this, SLOT(playActivated()));
				break;
		}
	}
	qmc2IgnoreItemActivation = false;
}

void SoftwareList::on_treeWidgetFavoriteSoftware_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
	if ( !qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/DoubleClickActivation").toBool() ) {
		qmc2IgnoreItemActivation = true;
		item->setExpanded(!item->isExpanded());
	}
}

void SoftwareList::on_treeWidgetSearchResults_itemActivated(QTreeWidgetItem *item, int column)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_treeWidgetSearchResults_itemActivated(QTreeWidgetItem *item = %1, int column = %2)").arg((qulonglong)item).arg(column));
#endif

	if ( !qmc2IgnoreItemActivation ) {
		cancelSoftwareSnap();
		switch ( qmc2DefaultLaunchMode ) {
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
			case QMC2_LAUNCH_MODE_EMBEDDED:
				QTimer::singleShot(0, this, SLOT(playEmbeddedActivated()));
				break;
#endif
			case QMC2_LAUNCH_MODE_INDEPENDENT:
			default:
				QTimer::singleShot(0, this, SLOT(playActivated()));
				break;
		}
	}
	qmc2IgnoreItemActivation = false;
}

void SoftwareList::on_treeWidgetSearchResults_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
	if ( !qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/DoubleClickActivation").toBool() ) {
		qmc2IgnoreItemActivation = true;
		item->setExpanded(!item->isExpanded());
	}
}

void SoftwareList::on_comboBoxSearch_editTextChanged(const QString &)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::on_comboBoxSearch_editTextChanged(const QString &)");
#endif

	searchTimer.start(QMC2_SEARCH_DELAY);
}

void SoftwareList::comboBoxSearch_editTextChanged_delayed()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::comboBoxSearch_editTextChanged_delayed()");
#endif

	static bool searchActive = false;

	if ( searchActive || isLoading )
		return;

	searchActive = true;

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

	if ( negatedMatch ) {
		QList<QTreeWidgetItem *> positiveMatches = matches;
		QList<QTreeWidgetItem *> positiveMatchesByShortName = matchesByShortName;
		matches.clear();
		matchesByShortName.clear();
		for (int i = 0; i < treeWidgetKnownSoftware->topLevelItemCount(); i++) {
			QTreeWidgetItem *item = treeWidgetKnownSoftware->topLevelItem(i);
			if ( !positiveMatches.contains(item) && !positiveMatchesByShortName.contains(item))
				matches << item;
		}
		for (int i = 0; i < treeWidgetKnownSoftware->topLevelItemCount(); i++) {
			QTreeWidgetItem *item = treeWidgetKnownSoftware->topLevelItem(i);
			if ( !positiveMatches.contains(item) && !positiveMatchesByShortName.contains(item))
				matchesByShortName << item;
		}
	}

	for (int i = 0; i < matchesByShortName.count(); i++) {
		QTreeWidgetItem *item = matchesByShortName[i];
		if ( !matches.contains(item) )
			matches.append(item);
	}

	QStringList compatFilters = systemSoftwareFilterMap[qmc2SoftwareList->systemName];
	for (int i = 0; i < matches.count(); i++) {
		SoftwareItem *item = new SoftwareItem(treeWidgetSearchResults);
		SoftwareItem *subItem = new SoftwareItem(item);
		subItem->setText(QMC2_SWLIST_COLUMN_TITLE, tr("Waiting for data..."));
		QTreeWidgetItem *matchItem = matches.at(i);
		item->setText(QMC2_SWLIST_COLUMN_TITLE, matchItem->text(QMC2_SWLIST_COLUMN_TITLE));
		item->setIcon(QMC2_SWLIST_COLUMN_TITLE, matchItem->icon(QMC2_SWLIST_COLUMN_TITLE));
		item->setWhatsThis(QMC2_SWLIST_COLUMN_TITLE, matchItem->whatsThis(QMC2_SWLIST_COLUMN_TITLE));
		item->setWhatsThis(QMC2_SWLIST_COLUMN_NAME, matchItem->whatsThis(QMC2_SWLIST_COLUMN_NAME));
		bool showItem = true;
		if ( qmc2SoftwareList->toolButtonCompatFilterToggle->isChecked() ) {
			QStringList compatList = item->whatsThis(QMC2_SWLIST_COLUMN_TITLE).split(",", QString::SkipEmptyParts);
			showItem = compatList.isEmpty() || compatFilters.isEmpty();
			for (int j = 0; j < compatList.count() && !showItem; j++)
				for (int k = 0; k < compatFilters.count() && !showItem; k++)
					showItem = (compatList[j] == compatFilters[k]);
		}
		item->setHidden(!showItem);
		item->setText(QMC2_SWLIST_COLUMN_NAME, matchItem->text(QMC2_SWLIST_COLUMN_NAME));
		item->setText(QMC2_SWLIST_COLUMN_PUBLISHER, matchItem->text(QMC2_SWLIST_COLUMN_PUBLISHER));
		item->setText(QMC2_SWLIST_COLUMN_YEAR, matchItem->text(QMC2_SWLIST_COLUMN_YEAR));
		item->setText(QMC2_SWLIST_COLUMN_PART, matchItem->text(QMC2_SWLIST_COLUMN_PART));
		item->setText(QMC2_SWLIST_COLUMN_INTERFACE, matchItem->text(QMC2_SWLIST_COLUMN_INTERFACE));
		item->setText(QMC2_SWLIST_COLUMN_LIST, matchItem->text(QMC2_SWLIST_COLUMN_LIST));
		item->setText(QMC2_SWLIST_COLUMN_SUPPORTED, matchItem->text(QMC2_SWLIST_COLUMN_SUPPORTED));
	}

	if ( autoSelectSearchItem ) {
  		treeWidgetSearchResults->setFocus();
		if ( treeWidgetSearchResults->currentItem() )
			treeWidgetSearchResults->currentItem()->setSelected(true);
	}

	autoSelectSearchItem = false;

	searchActive = false;
}

void SoftwareList::on_comboBoxSearch_activated(const QString &pattern)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareList::on_comboBoxSearch_activated(const QString &pattern = %1)").arg(pattern));
#endif

	autoSelectSearchItem = true;
	comboBoxSearch_editTextChanged_delayed();
}

QStringList &SoftwareList::arguments(QStringList *softwareLists, QStringList *softwareNames)
{
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

#if defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
	// optionally add arguments for the selected device configuration
	QString devConfigName = comboBoxDeviceConfiguration->currentText();
	if ( devConfigName != tr("Default configuration") ) {
		qmc2Config->beginGroup(QMC2_EMULATOR_PREFIX + QString("Configuration/Devices/%1/%2").arg(systemName).arg(devConfigName));
		QStringList instances = qmc2Config->value("Instances").toStringList();
		QStringList files = qmc2Config->value("Files").toStringList();
		QStringList slotNames = qmc2Config->value("Slots").toStringList();
		QStringList slotOptions = qmc2Config->value("SlotOptions").toStringList();
		QStringList slotBIOSs = qmc2Config->value("SlotBIOSs").toStringList();
		qmc2Config->endGroup();
		for (int i = 0; i < slotNames.count(); i++) {
			if ( !slotOptions[i].isEmpty() ) {
				QString slotOpt = slotOptions[i];
				if ( !slotBIOSs[i].isEmpty() )
					slotOpt += ",bios=" + slotBIOSs[i];
				swlArgs << QString("-%1").arg(slotNames[i]) << slotOpt;
			}
		}
		for (int i = 0; i < instances.count(); i++) {
#if defined(QMC2_OS_WIN)
			swlArgs << QString("-%1").arg(instances[i]) << files[i].replace('/', '\\');
#else
			swlArgs << QString("-%1").arg(instances[i]) << files[i].replace("~", "$HOME");
#endif
		}
	}
#endif

	QList<QTreeWidgetItem *> selectedItems = treeWidget->selectedItems();

	QString snapnameList, snapnameSoftware;
	if ( selectedItems.count() > 0 ) {
		QTreeWidgetItemIterator it(treeWidget);
		QStringList manualMounts;
		if ( !autoMounted ) {
			// manually mounted
			while ( *it ) {
				QComboBox *comboBox = (QComboBox *)treeWidget->itemWidget(*it, QMC2_SWLIST_COLUMN_PUBLISHER);
				if ( comboBox ) {
					if ( comboBox->currentIndex() > QMC2_SWLIST_MSEL_DONT_MOUNT ) {
						if ( snapnameList.isEmpty() ) {
							QTreeWidgetItem *item = *it;
							while ( item->parent() ) item = item->parent();
							snapnameList = item->text(QMC2_SWLIST_COLUMN_LIST);
							snapnameSoftware = item->text(QMC2_SWLIST_COLUMN_NAME);
							if ( comboBoxSnapnameDevice->isVisible() ) {
								if ( snapnameList + ":" + snapnameSoftware != comboBoxSnapnameDevice->currentText() ) {
									snapnameList.clear();
									snapnameSoftware.clear();
								}
							}
						}
						swlArgs << QString("-%1").arg(comboBox->currentText());
						QTreeWidgetItem *item = *it;
						while ( item->parent() ) item = item->parent();
						swlArgs << QString("%1:%2:%3").arg(item->text(QMC2_SWLIST_COLUMN_LIST)).arg(item->text(QMC2_SWLIST_COLUMN_NAME)).arg(item->text(QMC2_SWLIST_COLUMN_PART));
						if ( softwareLists )
							*softwareLists << item->text(QMC2_SWLIST_COLUMN_LIST);
						if ( softwareNames )
							*softwareNames << item->text(QMC2_SWLIST_COLUMN_NAME);
					}
				}
				it++;
			}
		} else {
			// automatically mounted
			QTreeWidgetItem *item = selectedItems[0];
			while ( item->parent() ) item = item->parent();
			snapnameList = item->text(QMC2_SWLIST_COLUMN_LIST);
			snapnameSoftware = item->text(QMC2_SWLIST_COLUMN_NAME);
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
			if ( softwareLists )
				*softwareLists << item->text(QMC2_SWLIST_COLUMN_LIST);
			if ( softwareNames )
				*softwareNames << item->text(QMC2_SWLIST_COLUMN_NAME);
		}
	}

	if ( toolButtonToggleSnapnameAdjustment->isChecked() && !snapnameList.isEmpty() ) {
		QString snapnamePattern = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SnapnamePattern", "soft-lists/$SOFTWARE_LIST$/$SOFTWARE_NAME$").toString();
		snapnamePattern.replace("$SOFTWARE_LIST$", snapnameList).replace("$SOFTWARE_NAME$", snapnameSoftware);
		swlArgs.prepend(snapnamePattern);
		swlArgs.prepend("-snapname");
	}

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
		if ( action->data().toInt() == QMC2_SWLIST_RESET ) {
			for (int i = 0; i < treeWidgetKnownSoftware->columnCount(); i++) treeWidgetKnownSoftware->setColumnHidden(i, false);
			treeWidgetKnownSoftware->header()->resizeSections(QHeaderView::Stretch);
			foreach (QAction *a, menuKnownSoftwareHeader->actions())
				if ( a->isCheckable() ) {
					a->blockSignals(true);
					a->setChecked(true);
					a->blockSignals(false);
				}
			return;
		}
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
		if ( action->data().toInt() == QMC2_SWLIST_RESET ) {
			for (int i = 0; i < treeWidgetFavoriteSoftware->columnCount(); i++) treeWidgetFavoriteSoftware->setColumnHidden(i, false);
			treeWidgetFavoriteSoftware->header()->resizeSections(QHeaderView::Stretch);
			foreach (QAction *a, menuFavoriteSoftwareHeader->actions())
				if ( a->isCheckable() ) {
					a->blockSignals(true);
					a->setChecked(true);
					a->blockSignals(false);
				}
			return;
		}
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
		if ( action->data().toInt() == QMC2_SWLIST_RESET ) {
			for (int i = 0; i < treeWidgetSearchResults->columnCount(); i++) treeWidgetSearchResults->setColumnHidden(i, false);
			treeWidgetSearchResults->header()->resizeSections(QHeaderView::Stretch);
			foreach (QAction *a, menuSearchResultsHeader->actions())
				if ( a->isCheckable() ) {
					a->blockSignals(true);
					a->setChecked(true);
					a->blockSignals(false);
				}
			return;
		}
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
		mountedSoftware.clear();
	} else if ( mountDevice == QObject::tr("Don't mount") ) {
		while ( *it ) {
			QComboBox *comboBox = (QComboBox *)treeWidget->itemWidget(*it, QMC2_SWLIST_COLUMN_PUBLISHER);
			if ( comboBox ) {
				QTreeWidgetItem *pItem = *it;
				while ( pItem->parent() ) pItem = pItem->parent();
				if ( comboBox == comboBoxSender ) {
					(*it)->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("Not mounted"));
					mountedSoftware.removeAll(pItem->text(QMC2_SWLIST_COLUMN_LIST) + ":" + pItem->text(QMC2_SWLIST_COLUMN_NAME));
				} else if ( comboBox->currentIndex() == QMC2_SWLIST_MSEL_AUTO_MOUNT ) {
					comboBox->blockSignals(true);
					comboBox->setCurrentIndex(QMC2_SWLIST_MSEL_DONT_MOUNT); // => don't mount
					comboBox->blockSignals(false);
					(*it)->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("Not mounted"));
					mountedSoftware.removeAll(pItem->text(QMC2_SWLIST_COLUMN_LIST) + ":" + pItem->text(QMC2_SWLIST_COLUMN_NAME));
				}
			}
			it++;
		}
		autoMounted = false;
	} else {
		while ( *it ) {
			QComboBox *comboBox = (QComboBox *)treeWidget->itemWidget(*it, QMC2_SWLIST_COLUMN_PUBLISHER);
			if ( comboBox ) {
				QTreeWidgetItem *pItem = *it;
				while ( pItem->parent() ) pItem = pItem->parent();
				if ( comboBox != comboBoxSender ) {
					if ( comboBox->currentText() == mountDevice || comboBox->currentText() == QObject::tr("Auto mount") ) {
						comboBox->blockSignals(true);
						comboBox->setCurrentIndex(QMC2_SWLIST_MSEL_DONT_MOUNT); // => don't mount
						comboBox->blockSignals(false);
						(*it)->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("Not mounted"));
						mountedSoftware.removeAll(pItem->text(QMC2_SWLIST_COLUMN_LIST) + ":" + pItem->text(QMC2_SWLIST_COLUMN_NAME));
					}
				} else {
					(*it)->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("Mounted on:") + " " + mountDevice);
					mountedSoftware << pItem->text(QMC2_SWLIST_COLUMN_LIST) + ":" + pItem->text(QMC2_SWLIST_COLUMN_NAME);
				}
			}
			it++;
		}
		autoMounted = false;
	}

	if ( toolButtonToggleSnapnameAdjustment->isChecked() ) {
		if ( !autoMounted && mountedSoftware.count() > 1 ) {
			comboBoxSnapnameDevice->setUpdatesEnabled(false);
			comboBoxSnapnameDevice->clear();
			qSort(mountedSoftware);
			comboBoxSnapnameDevice->addItems(mountedSoftware);
			comboBoxSnapnameDevice->setUpdatesEnabled(true);
			comboBoxSnapnameDevice->show();
		} else
			comboBoxSnapnameDevice->hide();
	} else
		comboBoxSnapnameDevice->hide();
}

void SoftwareList::loadFavoritesFromFile()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::loadFavoritesFromFile()");
#endif

	QString proposedName = systemName + ".fav";

	if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "SoftwareList/LastFavoritesStoragePath") )
		proposedName.prepend(qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareList/LastFavoritesStoragePath").toString());

	QString filePath = QFileDialog::getOpenFileName(this, tr("Choose file to merge favorites from"), proposedName, tr("All files (*)"), 0, qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);

	if ( !filePath.isEmpty() ) {
		QFileInfo fiFilePath(filePath);
		QString storagePath = fiFilePath.absolutePath();
		if ( !storagePath.endsWith("/") ) storagePath.append("/");
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "SoftwareList/LastFavoritesStoragePath", storagePath);

		// import software-list favorites
		QFile favoritesFile(filePath);
		QStringList compatFilters = systemSoftwareFilterMap[systemName];
		if ( favoritesFile.open(QIODevice::ReadOnly | QIODevice::Text) ) {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading software-favorites for '%1' from '%2'").arg(systemName).arg(filePath));
			QTextStream ts(&favoritesFile);
			int lineCounter = 0;
			while ( !ts.atEnd() ){
				QString line = ts.readLine().trimmed();
				lineCounter++;
				if ( !line.startsWith("#") && !line.isEmpty()) {
					QStringList words = line.split("\t");
					if ( words.count() > 1 ) {
						QString listName = words[0];
						QString entryName = words[1];
						QList<QTreeWidgetItem *> matchedItems = treeWidgetFavoriteSoftware->findItems(entryName, Qt::MatchExactly | Qt::MatchCaseSensitive, QMC2_SWLIST_COLUMN_NAME);
						if ( matchedItems.count() <= 0 ) {
							matchedItems = treeWidgetKnownSoftware->findItems(entryName, Qt::MatchExactly | Qt::MatchCaseSensitive, QMC2_SWLIST_COLUMN_NAME);
							if ( matchedItems.count() > 0 ) {
								SoftwareItem *knowSoftwareItem = (SoftwareItem *)matchedItems.at(0);
								SoftwareItem *item = new SoftwareItem(treeWidgetFavoriteSoftware);
								SoftwareItem *subItem = new SoftwareItem(item);
								subItem->setText(QMC2_SWLIST_COLUMN_TITLE, tr("Waiting for data..."));
								item->setText(QMC2_SWLIST_COLUMN_TITLE, knowSoftwareItem->text(QMC2_SWLIST_COLUMN_TITLE));
								item->setWhatsThis(QMC2_SWLIST_COLUMN_TITLE, knowSoftwareItem->whatsThis(QMC2_SWLIST_COLUMN_TITLE));
								item->setWhatsThis(QMC2_SWLIST_COLUMN_NAME, knowSoftwareItem->whatsThis(QMC2_SWLIST_COLUMN_NAME));
								bool showItem = true;
								if ( toolButtonCompatFilterToggle->isChecked() ) {
									QStringList compatList = item->whatsThis(QMC2_SWLIST_COLUMN_TITLE).split(",", QString::SkipEmptyParts);
									showItem = compatList.isEmpty() || compatFilters.isEmpty();
									for (int i = 0; i < compatList.count() && !showItem; i++)
										for (int j = 0; j < compatFilters.count() && !showItem; j++)
											showItem = (compatList[i] == compatFilters[j]);
								}
								item->setHidden(!showItem);
								item->setText(QMC2_SWLIST_COLUMN_NAME, knowSoftwareItem->text(QMC2_SWLIST_COLUMN_NAME));
								item->setText(QMC2_SWLIST_COLUMN_PUBLISHER, knowSoftwareItem->text(QMC2_SWLIST_COLUMN_PUBLISHER));
								item->setText(QMC2_SWLIST_COLUMN_YEAR, knowSoftwareItem->text(QMC2_SWLIST_COLUMN_YEAR));
								item->setText(QMC2_SWLIST_COLUMN_PART, knowSoftwareItem->text(QMC2_SWLIST_COLUMN_PART));
								item->setText(QMC2_SWLIST_COLUMN_INTERFACE, knowSoftwareItem->text(QMC2_SWLIST_COLUMN_INTERFACE));
								item->setText(QMC2_SWLIST_COLUMN_LIST, knowSoftwareItem->text(QMC2_SWLIST_COLUMN_LIST));
								item->setText(QMC2_SWLIST_COLUMN_SUPPORTED, knowSoftwareItem->text(QMC2_SWLIST_COLUMN_SUPPORTED));
#if defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
								if ( words.count() > 2 )
									item->setText(QMC2_SWLIST_COLUMN_DEVICECFG, words[2]);
#endif
								qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("entry '%1:%2' successfully imported").arg(listName).arg(entryName));
							} else
								qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: entry '%1:%2' cannot be associated with any known software for this system (line %3 ignored)").arg(listName).arg(entryName).arg(lineCounter));
						} else
							qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: a favorite entry for '%1:%2' already exists (line %3 ignored)").arg(listName).arg(entryName).arg(lineCounter));
					} else
						qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: syntax error on line %1 (ignored)").arg(lineCounter));
				}
			}
			favoritesFile.close();
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading software-favorites for '%1' from '%2')").arg(systemName).arg(filePath));
		} else
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open '%1' for reading, please check permissions").arg(filePath));
	}
}

void SoftwareList::saveFavoritesToFile()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareList::saveFavoritesToFile()");
#endif

	QString proposedName = systemName + ".fav";

	if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "SoftwareList/LastFavoritesStoragePath") )
		proposedName.prepend(qmc2Config->value(QMC2_FRONTEND_PREFIX + "SoftwareList/LastFavoritesStoragePath").toString());

	QString filePath = QFileDialog::getSaveFileName(this, tr("Choose file to store favorites to"), proposedName, tr("All files (*)"), 0, qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);

	if ( !filePath.isEmpty() ) {
		QFileInfo fiFilePath(filePath);
		QString storagePath = fiFilePath.absolutePath();
		if ( !storagePath.endsWith("/") ) storagePath.append("/");
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "SoftwareList/LastFavoritesStoragePath", storagePath);

		// export software-list favorites
		QFile favoritesFile(filePath);
		if ( favoritesFile.open(QIODevice::WriteOnly | QIODevice::Text) ) {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("saving software-favorites for '%1' to '%2'").arg(systemName).arg(filePath));
			QTextStream ts(&favoritesFile);
#if defined(QMC2_EMUTYPE_MESS)
			ts << QString("# MESS software-list favorites export for driver '%1'\n").arg(systemName);
			ts << QString("# Format: <list-name><TAB><entry-name>[<TAB><additional-device-configuration>]\n");
#elif defined(QMC2_EMUTYPE_MAME)
			ts << QString("# MAME software-list favorites export for driver '%1'\n").arg(systemName);
			ts << QString("# Format: <list-name><TAB><entry-name>\n");
#elif defined(QMC2_EMUTYPE_UME)
			ts << QString("# UME software-list favorites export for driver '%1'\n").arg(systemName);
			ts << QString("# Format: <list-name><TAB><entry-name>[<TAB><additional-device-configuration>]\n");
#endif
			for (int i = 0; i < treeWidgetFavoriteSoftware->topLevelItemCount(); i++) {
				QTreeWidgetItem *item = treeWidgetFavoriteSoftware->topLevelItem(i);
				if ( item ) {
					ts << item->text(QMC2_SWLIST_COLUMN_LIST) << "\t" << item->text(QMC2_SWLIST_COLUMN_NAME);
#if defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
					if ( !item->text(QMC2_SWLIST_COLUMN_DEVICECFG).isEmpty() )
						ts << "\t" << item->text(QMC2_SWLIST_COLUMN_DEVICECFG);
#endif
					ts << "\n";
				}

			}
			favoritesFile.close();
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (saving software-favorites for '%1' to '%2')").arg(systemName).arg(filePath));
		} else
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open '%1' for writing, please check permissions").arg(filePath));
	}
}

QString SoftwareList::softwareStatus(QString listName, QString softwareName, bool translated)
{
	if ( softwareListStateMap.contains(listName) ) {
		if ( softwareListStateMap[listName].contains(softwareName) ) {
			switch ( softwareListStateMap[listName][softwareName] ) {
				case 'C':
					if ( translated )
						return tr("correct");
					else
						return "correct";
					break;
				case 'M':
					if ( translated )
						return tr("mostly correct");
					else
						return "mostly correct";
					break;
				case 'I':
					if ( translated )
						return tr("incorrect");
					else
						return "incorrect";
					break;
				case 'N':
					if ( translated )
						return tr("not found");
					else
						return "not found";
					break;
				case 'U':
				default:
					if ( translated )
						return tr("unknown");
					else
						return "unknown";
					break;
			}
		} else {
			if ( translated )
				return tr("unknown");
			else
				return "unknown";
		}
	} else {
		if ( translated )
			return tr("unknown");
		else
			return "unknown";
	}
}

QString SoftwareList::status(SoftwareListXmlHandler *handler)
{
	QLocale locale;
	QString statusString = "<b>";
	if ( handler ) {
		statusString += "<font color=black>" + tr("L:") + locale.toString(numSoftwareTotal + handler->numTotal) + "</font> ";
		if ( toolButtonSoftwareStates->isChecked() ) {
			statusString += "<font color=#00cc00>" + tr("C:") + locale.toString(numSoftwareCorrect + handler->numCorrect) + "</font> ";
			statusString += "<font color=#a2c743>" + tr("M:") + locale.toString(numSoftwareMostlyCorrect + handler->numMostlyCorrect) + "</font> ";
			statusString += "<font color=#f90000>" + tr("I:") + locale.toString(numSoftwareIncorrect + handler->numIncorrect) + "</font> ";
			statusString += "<font color=#7f7f7f>" + tr("N:") + locale.toString(numSoftwareNotFound + handler->numNotFound) + "</font> ";
			statusString += "<font color=#0000f9>" + tr("U:") + locale.toString(numSoftwareUnknown + handler->numUnknown) + "</font> ";
		}
	} else {
		statusString += "<font color=black>" + tr("L:") + locale.toString(numSoftwareTotal) + "</font> ";
		if ( toolButtonSoftwareStates->isChecked() ) {
			statusString += "<font color=#00cc00>" + tr("C:") + locale.toString(numSoftwareCorrect) + "</font> ";
			statusString += "<font color=#a2c743>" + tr("M:") + locale.toString(numSoftwareMostlyCorrect) + "</font> ";
			statusString += "<font color=#f90000>" + tr("I:") + locale.toString(numSoftwareIncorrect) + "</font> ";
			statusString += "<font color=#7f7f7f>" + tr("N:") + locale.toString(numSoftwareNotFound) + "</font> ";
			statusString += "<font color=#0000f9>" + tr("U:") + locale.toString(numSoftwareUnknown) + "</font> ";
		}
	}
	statusString += "</b>";
	return statusString;
}

void SoftwareList::updateStats(SoftwareListXmlHandler *handler)
{
	labelSoftwareListStats->setText(status(handler));
}

SoftwareListXmlHandler::SoftwareListXmlHandler(QTreeWidget *parent)
{
#ifdef QMC2_DEBUG
//	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareListXmlHandler::SoftwareListXmlHandler(QTreeWidget *parent = %1)").arg((qulonglong)parent));
#endif

	parentTreeWidget = parent;
	numTotal = numCorrect = numMostlyCorrect = numIncorrect = numNotFound = numUnknown = elementCounter = 0;
	newSoftwareStates = false;
}

SoftwareListXmlHandler::~SoftwareListXmlHandler()
{
#ifdef QMC2_DEBUG
//	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareListXmlHandler::~SoftwareListXmlHandler()");
#endif

}

void SoftwareListXmlHandler::loadSoftwareStates(QString listName)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareListXmlHandler::loadSoftwareStates(QString listName = %1)").arg(listName));
#endif

	QString softwareStateCachePath = QDir::toNativeSeparators(QDir::cleanPath(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareStateCache").toString() + "/" + listName + ".ssc"));
	QFile stateCacheFile(softwareStateCachePath);
	if ( stateCacheFile.open(QIODevice::ReadOnly | QIODevice::Text) ) {
		numTotal = numCorrect = numMostlyCorrect = numIncorrect = numNotFound = numUnknown = 0;
		QTextStream ts(&stateCacheFile);
		ts.readLine(); // comment line
		while ( !ts.atEnd() ) {
			QStringList words = ts.readLine().trimmed().split(" ", QString::SkipEmptyParts);
			if ( words.count() > 1 ) {
				switch ( words[1][0].toLatin1() ) {
					case 'C':
						softwareListStateMap[listName][words[0]] = 'C';
						break;
					case 'M':
						softwareListStateMap[listName][words[0]] = 'M';
						break;
					case 'I':
						softwareListStateMap[listName][words[0]] = 'I';
						break;
					case 'N':
						softwareListStateMap[listName][words[0]] = 'N';
						break;
					case 'U':
					default:
						softwareListStateMap[listName][words[0]] = 'U';
						break;
				}
			}
		}
		stateCacheFile.close();
	}
	newSoftwareStates = true;
}

bool SoftwareListXmlHandler::startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &attributes)
{
#ifdef QMC2_DEBUG
//	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareListXmlHandler::startElement(const QString &namespaceURI = ..., const QString &localName = %1, const QString &qName = %2, const QXmlAttributes &attributes = ...)").arg(localName).arg(qName));
#endif

	if ( qmc2SoftwareList->interruptLoad )
		return false;

	if ( ++elementCounter % QMC2_SWLIST_LOAD_RESPONSE_LONG == 0 ) {
		qmc2SoftwareList->updateStats(this);
		qApp->processEvents();
	}

	if ( qName == "softwarelist" ) {
		softwareListName = attributes.value("name");
		compatFilters = systemSoftwareFilterMap[qmc2SoftwareList->systemName];
		if ( qmc2SoftwareList->toolButtonSoftwareStates->isChecked() )
			if ( !softwareListStateMap.contains(softwareListName) )
				loadSoftwareStates(softwareListName);
	} else if ( qName == "software" ) {
		softwareName = attributes.value("name");
		softwareSupported = attributes.value("supported");
		if ( softwareSupported.isEmpty() || softwareSupported == "yes" )
			softwareSupported = QObject::tr("yes");
		else if ( softwareSupported == "no" )
			softwareSupported = QObject::tr("no");
		else
			softwareSupported = QObject::tr("partially");
		softwareItem = new SoftwareItem(parentTreeWidget);
		SoftwareItem *subItem = new SoftwareItem(softwareItem);
		subItem->setText(QMC2_SWLIST_COLUMN_TITLE, QObject::tr("Waiting for data..."));
		softwareItem->setText(QMC2_SWLIST_COLUMN_NAME, softwareName);
		softwareItem->setText(QMC2_SWLIST_COLUMN_LIST, softwareListName);
		softwareItem->setText(QMC2_SWLIST_COLUMN_SUPPORTED, softwareSupported);
		numTotal++;
		if ( qmc2SoftwareList->toolButtonSoftwareStates->isChecked() ) {
			if ( softwareListStateMap[softwareListName].contains(softwareName) ) {
				switch ( softwareListStateMap[softwareListName][softwareName] ) {
					case 'C':
						numCorrect++;
						softwareItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_correct.png")));
						softwareItem->setWhatsThis(QMC2_SWLIST_COLUMN_NAME, "C");
						if ( qmc2SoftwareList->stateFilter->checkBoxStateFilter->isChecked() )
							softwareItem->setHidden(!qmc2SoftwareList->stateFilter->toolButtonCorrect->isChecked());
						else
							softwareItem->setHidden(false);
						break;
					case 'M':
						numMostlyCorrect++;
						softwareItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_mostlycorrect.png")));
						softwareItem->setWhatsThis(QMC2_SWLIST_COLUMN_NAME, "M");
						if ( qmc2SoftwareList->stateFilter->checkBoxStateFilter->isChecked() )
							softwareItem->setHidden(!qmc2SoftwareList->stateFilter->toolButtonMostlyCorrect->isChecked());
						else
							softwareItem->setHidden(false);
						break;
					case 'I':
						numIncorrect++;
						softwareItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_incorrect.png")));
						softwareItem->setWhatsThis(QMC2_SWLIST_COLUMN_NAME, "I");
						if ( qmc2SoftwareList->stateFilter->checkBoxStateFilter->isChecked() )
							softwareItem->setHidden(!qmc2SoftwareList->stateFilter->toolButtonIncorrect->isChecked());
						else
							softwareItem->setHidden(false);
						break;
					case 'N':
						numNotFound++;
						softwareItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_notfound.png")));
						softwareItem->setWhatsThis(QMC2_SWLIST_COLUMN_NAME, "N");
						if ( qmc2SoftwareList->stateFilter->checkBoxStateFilter->isChecked() )
							softwareItem->setHidden(!qmc2SoftwareList->stateFilter->toolButtonNotFound->isChecked());
						else
							softwareItem->setHidden(false);
						break;
					case 'U':
					default:
						numUnknown++;
						softwareItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_unknown.png")));
						softwareItem->setWhatsThis(QMC2_SWLIST_COLUMN_NAME, "U");
						if ( qmc2SoftwareList->stateFilter->checkBoxStateFilter->isChecked() )
							softwareItem->setHidden(!qmc2SoftwareList->stateFilter->toolButtonUnknown->isChecked());
						else
							softwareItem->setHidden(false);
						break;
				}
			} else {
				numUnknown++;
				softwareItem->setIcon(QMC2_SWLIST_COLUMN_TITLE, QIcon(QString::fromUtf8(":/data/img/software_unknown.png")));
				softwareItem->setWhatsThis(QMC2_SWLIST_COLUMN_NAME, "U");
				if ( qmc2SoftwareList->stateFilter->checkBoxStateFilter->isChecked() )
					softwareItem->setHidden(!qmc2SoftwareList->stateFilter->toolButtonUnknown->isChecked());
				else
					softwareItem->setHidden(false);
			}
		}
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
	} else if ( qName == "feature" ) {
		if ( attributes.value("name") == "compatibility" ) {
			// we use the invisible whatsThis data of the title column to store the software-compatibility list
			QString partCompat = attributes.value("value");
			if ( !partCompat.isEmpty() ) {
				softwareItem->setWhatsThis(QMC2_SWLIST_COLUMN_TITLE, partCompat);
				if ( qmc2SoftwareList->toolButtonCompatFilterToggle->isChecked() ) {
					QStringList compatList = partCompat.split(",", QString::SkipEmptyParts);
					bool showItem = compatList.isEmpty() || compatFilters.isEmpty();
					for (int i = 0; i < compatList.count() && !showItem; i++)
						for (int j = 0; j < compatFilters.count() && !showItem; j++)
							showItem = (compatList[i] == compatFilters[j]);
					if ( !softwareItem->isHidden() )
						softwareItem->setHidden(!showItem);
				}
			}
		}
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

	if ( qmc2SoftwareList->interruptLoad )
		return false;

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

	currentText += QString::fromUtf8(str.toLocal8Bit());
	return true;
}

SoftwareSnap::SoftwareSnap(QWidget *parent)
	: QWidget(parent, Qt::ToolTip)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareSnap::SoftwareSnap(QWidget *parent = %1)").arg((qulonglong)parent));
#endif

	setFocusPolicy(Qt::NoFocus);
	snapForcedResetTimer.setSingleShot(true);
	connect(&snapForcedResetTimer, SIGNAL(timeout()), this, SLOT(resetSnapForced()));

	ctxMenuRequested = false;

	contextMenu = new QMenu(this);
	contextMenu->hide();
	
	QString s;
	QAction *action;

	s = tr("Copy image to clipboard");
	action = contextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/editcopy.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(copyToClipboard()));

	s = tr("Copy file path to clipboard");
	action = contextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/editcopy.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(copyPathToClipboard()));
	actionCopyPathToClipboard = action;

	contextMenu->addSeparator();

	s = tr("Zoom in (+10%)");
	action = contextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/zoom-in.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(zoomIn()));

	s = tr("Zoom out (-10%)");
	action = contextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/zoom-out.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(zoomOut()));

	s = tr("Reset zoom (100%)");
	action = contextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/zoom-none.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(resetZoom()));

	contextMenu->addSeparator();

	s = tr("Refresh cache slot");
	action = contextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/reload.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(refresh()));

	zoom = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SoftwareSnapZoom", 100).toInt();

	if ( useZip() ) {
		foreach (QString filePath, qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareSnapFile").toString().split(";", QString::SkipEmptyParts)) {
			snapFileMap[filePath] = unzOpen(filePath.toLocal8Bit());
			if ( snapFileMap[filePath] == NULL )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open software snap-shot file, please check access permissions for %1").arg(filePath));
		}
	} else if ( useSevenZip() ) {
		foreach (QString filePath, qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareSnapFile").toString().split(";", QString::SkipEmptyParts)) {
			SevenZipFile *snapFile = new SevenZipFile(filePath);
			if ( !snapFile->open() ) {
				  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open software snap-shot file %1").arg(filePath) + " - " + tr("7z error") + ": " + snapFile->lastError());
				  delete snapFile;
			} else {
				snapFileMap7z[filePath] = snapFile;
				connect(snapFile, SIGNAL(dataReady()), this, SLOT(sevenZipDataReady()));
			}
		}
	}

	reloadActiveFormats();
}

SoftwareSnap::~SoftwareSnap()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareSnap::~SoftwareSnap()");
#endif

	if ( qmc2UseSoftwareSnapFile ) {
		foreach (unzFile snapFile, snapFileMap)
			unzClose(snapFile);
		foreach (SevenZipFile *snapFile, snapFileMap7z) {
			snapFile->close();
			delete snapFile;
		}
		snapFileMap.clear();
		snapFileMap7z.clear();
	}
}

QString SoftwareSnap::primaryPathFor(QString list, QString name)
{
	if ( !qmc2UseSoftwareSnapFile ) {
		QStringList fl = qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareSnapDirectory").toString().split(";", QString::SkipEmptyParts);
		QString baseDirectory;
		if ( fl.count() > 0 )
			baseDirectory = fl[0];
		return QDir::toNativeSeparators(QDir::cleanPath(baseDirectory + "/" + list + "/" + name + ".png"));
	} else // we don't support on-the-fly image replacement for zipped images yet!
		return QString();
}

bool SoftwareSnap::replaceImage(QString list, QString name, QPixmap &pixmap)
{
	if ( !qmc2UseSoftwareSnapFile ) {
		QString savePath = primaryPathFor(list, name);
		if ( !savePath.isEmpty() ) {
			bool goOn = true;
			if ( QFile::exists(savePath) ) {
				QString backupPath = savePath + ".bak";
				if ( QFile::exists(backupPath) )
					QFile::remove(backupPath);
				if ( !QFile::copy(savePath, backupPath) ) {
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't create backup of existing image file '%1' as '%2'").arg(savePath).arg(backupPath));
					goOn = false;
				}
			}
			if ( goOn ) {
				QString primaryPath = QFileInfo(savePath).absoluteDir().absolutePath();
				QDir ppDir(primaryPath);
				if ( !ppDir.exists() )
					ppDir.mkpath(primaryPath);
				if ( pixmap.save(savePath, "PNG") ) {
					refresh();
					if ( qmc2SoftwareSnapshot )
						qmc2SoftwareSnapshot->refresh();
					return true;
				} else {
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't create image file '%1'").arg(savePath));
					return false;
				}
			} else
				return false;
		} else {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't determine primary path for image-type '%1'").arg(tr("software snapshot")));
			return false;
		}
	} else // we don't support on-the-fly image replacement for zipped images yet!
		return false;
}

void SoftwareSnap::zoomIn()
{
	zoom += 10;
	if ( zoom > 400 )
		zoom = 400;
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SoftwareSnapZoom", zoom);
	refresh();
}

void SoftwareSnap::zoomOut()
{
	zoom -= 10;
	if ( zoom < 10 )
		zoom = 10;
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SoftwareSnapZoom", zoom);
	refresh();
}

void SoftwareSnap::resetZoom()
{
	zoom = 100;
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SoftwareSnapZoom", zoom);
	refresh();
}

void SoftwareSnap::mousePressEvent(QMouseEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareSnap::mousePressEvent(QMouseEvent *e = %1)").arg((qulonglong)e));
#endif

	if ( e->button() != Qt::RightButton)
		hide();
	else
		ctxMenuRequested = true;
}

void SoftwareSnap::enterEvent(QEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareSnap::enterEvent(QEvent *e = %1)").arg((qulonglong)e));
#endif

	if ( contextMenu->isVisible() )
		QTimer::singleShot(0, contextMenu, SLOT(hide()));
	ctxMenuRequested = false;

	QWidget::enterEvent(e);
}

void SoftwareSnap::leaveEvent(QEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareSnap::leaveEvent(QEvent *e = %1)").arg((qulonglong)e));
#endif

	if ( !qmc2SoftwareList->snapForced && !ctxMenuRequested ) {
		myItem = NULL;
		hide();
	}  else if ( !qmc2SoftwareList->snapForced )
		QTimer::singleShot(QMC2_SWSNAP_UNFORCE_DELAY, qmc2SoftwareList, SLOT(checkSoftwareSnap()));

	ctxMenuRequested = contextMenu->isVisible();

	QWidget::leaveEvent(e);
}

void SoftwareSnap::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	loadSnapshot();
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
		myCacheKey.clear();
		return;
	}

	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/AutoDisableSoftwareSnap", true).toBool() ) {
		if ( qmc2MainWindow->stackedWidgetSpecial->currentIndex() == QMC2_SPECIAL_SOFTWARE_PAGE || (qmc2MainWindow->tabWidgetSoftwareDetail->parent() == qmc2MainWindow && qmc2MainWindow->tabWidgetSoftwareDetail->isVisible()) ) {
			myItem = NULL;
			resetSnapForced();
			myCacheKey.clear();
			return;
		}
	}

#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareSnap::loadSnapshot(): snapForced = '%1'").arg(qmc2SoftwareList->snapForced ? "true" : "false"));
#endif

	// check if the mouse cursor is still on a software item
	QTreeWidgetItem *item = NULL;
	QTreeWidget *treeWidget = NULL;
	QRect rect;

	switch ( qmc2SoftwareList->toolBoxSoftwareList->currentIndex() ) {
		case QMC2_SWLIST_KNOWN_SW_PAGE:
			if ( qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->underMouse() ) {
				item = qmc2SoftwareList->treeWidgetKnownSoftware->itemAt(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->mapFromGlobal(QCursor::pos()));
				if ( item ) {
					rect = qmc2SoftwareList->treeWidgetKnownSoftware->visualItemRect(item);
					rect.setWidth(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->width());
					rect.setX(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->x());
					rect.translate(0, 4);
					position = qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->mapToGlobal(rect.bottomLeft());
				}
			}
			treeWidget = qmc2SoftwareList->treeWidgetKnownSoftware;
			break;
		case QMC2_SWLIST_FAVORITES_PAGE:
			if ( qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->underMouse() ) {
				item = qmc2SoftwareList->treeWidgetFavoriteSoftware->itemAt(qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->mapFromGlobal(QCursor::pos()));
				if ( item ) {
					rect = qmc2SoftwareList->treeWidgetFavoriteSoftware->visualItemRect(item);
					rect.setWidth(qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->width());
					rect.setX(qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->x());
					rect.translate(0, 4);
					position = qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->mapToGlobal(rect.bottomLeft());
				}
			}
			treeWidget = qmc2SoftwareList->treeWidgetFavoriteSoftware;
			break;
		case QMC2_SWLIST_SEARCH_PAGE:
			if ( qmc2SoftwareList->treeWidgetSearchResults->viewport()->underMouse() ) {
				item = qmc2SoftwareList->treeWidgetSearchResults->itemAt(qmc2SoftwareList->treeWidgetSearchResults->viewport()->mapFromGlobal(QCursor::pos()));
				if ( item ) {
					rect = qmc2SoftwareList->treeWidgetSearchResults->visualItemRect(item);
					rect.setWidth(qmc2SoftwareList->treeWidgetSearchResults->viewport()->width());
					rect.setX(qmc2SoftwareList->treeWidgetSearchResults->viewport()->x());
					rect.translate(0, 4);
					position = qmc2SoftwareList->treeWidgetSearchResults->viewport()->mapToGlobal(rect.bottomLeft());
				}
			}
			treeWidget = qmc2SoftwareList->treeWidgetSearchResults;
			break;
	}

	// try to fall back to 'selected item' if applicable (no mouse hover)
	if ( !item || qmc2SoftwareList->snapForced ) {
		if ( qmc2SoftwareList->snapForced && myItem != NULL ) {
			item = myItem;
			switch ( qmc2SoftwareList->toolBoxSoftwareList->currentIndex() ) {
				case QMC2_SWLIST_KNOWN_SW_PAGE:
					rect = qmc2SoftwareList->treeWidgetKnownSoftware->visualItemRect(item);
					rect.setWidth(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->width());
					rect.setX(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->x());
					rect.translate(0, 4);
					position = qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->mapToGlobal(rect.bottomLeft());
					treeWidget = qmc2SoftwareList->treeWidgetKnownSoftware;
					break;
				case QMC2_SWLIST_FAVORITES_PAGE:
					rect = qmc2SoftwareList->treeWidgetFavoriteSoftware->visualItemRect(item);
					rect.setWidth(qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->width());
					rect.setX(qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->x());
					rect.translate(0, 4);
					position = qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->mapToGlobal(rect.bottomLeft());
					treeWidget = qmc2SoftwareList->treeWidgetFavoriteSoftware;
					break;
				case QMC2_SWLIST_SEARCH_PAGE:
					rect = qmc2SoftwareList->treeWidgetSearchResults->visualItemRect(item);
					rect.setWidth(qmc2SoftwareList->treeWidgetSearchResults->viewport()->width());
					rect.setX(qmc2SoftwareList->treeWidgetSearchResults->viewport()->x());
					rect.translate(0, 4);
					position = qmc2SoftwareList->treeWidgetSearchResults->viewport()->mapToGlobal(rect.bottomLeft());
					treeWidget = qmc2SoftwareList->treeWidgetSearchResults;
					break;
			}
		}
	}

	// if we can't figure out which item we're on, let's escape from here...
	if ( !item || !treeWidget ) {
		myItem = NULL;
		resetSnapForced();
		qmc2SoftwareList->cancelSoftwareSnap();
		myCacheKey.clear();
		return;
	}

	if ( !qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SoftwareSnapOnMouseHover", false).toBool() ) {
		// paranoia check :)
		QList<QTreeWidgetItem *> itemList = treeWidget->selectedItems();
		if ( !itemList.isEmpty() )
			if ( itemList[0] != item ) {
				myItem = NULL;
				resetSnapForced();
				qmc2SoftwareList->cancelSoftwareSnap();
				myCacheKey.clear();
				return;
			}
	}

	listName = item->text(QMC2_SWLIST_COLUMN_LIST);
	entryName = item->text(QMC2_SWLIST_COLUMN_NAME);
	myItem = (SoftwareItem *)item;
	myCacheKey = "sws_" + listName + "_" + entryName;

	ImagePixmap pm;
	bool pmLoaded = false;
	ImagePixmap *cpm = qmc2ImagePixmapCache.object(myCacheKey);
	if ( cpm ) {
		pmLoaded = true;
		pm = *cpm;
	}

	if ( !pmLoaded ) {
		if ( qmc2UseSoftwareSnapFile ) {
			if ( useZip() ) {
				// try loading image from (semicolon-separated) ZIP archive(s)
				if ( snapFileMap.isEmpty() ) {
					foreach (QString filePath, qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareSnapFile").toString().split(";", QString::SkipEmptyParts)) {
						snapFileMap[filePath] = unzOpen(filePath.toLocal8Bit());
						if ( snapFileMap[filePath] == NULL )
							qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open software snap-shot file, please check access permissions for %1").arg(filePath));
					}
				}
				foreach (unzFile snapFile, snapFileMap) {
					if ( snapFile ) {
						bool fileOk = true;
						QByteArray imageData;
						foreach (int format, activeFormats) {
							QString formatName = ImageWidget::formatNames[format];
							foreach (QString extension, ImageWidget::formatExtensions[format].split(", ", QString::SkipEmptyParts)) {
								QString pathInZip = listName + "/" + entryName + "." + extension;
								if ( unzLocateFile(snapFile, pathInZip.toLocal8Bit().constData(), 0) == UNZ_OK ) {
									if ( unzOpenCurrentFile(snapFile) == UNZ_OK ) {
										char imageBuffer[QMC2_ZIP_BUFFER_SIZE];
										int len;
										while ( (len = unzReadCurrentFile(snapFile, &imageBuffer, QMC2_ZIP_BUFFER_SIZE)) > 0 ) {
											for (int i = 0; i < len; i++)
												imageData += imageBuffer[i];
										}
										unzCloseCurrentFile(snapFile);
										fileOk = true;
									} else
										fileOk = false;
								} else
									fileOk = false;

								if ( fileOk ) {
									if ( pm.loadFromData(imageData, formatName.toLocal8Bit().constData()) ) {
										pmLoaded = true;
										qmc2ImagePixmapCache.insert(myCacheKey, new ImagePixmap(pm), pm.toImage().byteCount());
										break;
									}
								}
							}

							if ( pmLoaded )
								break;
						}
					}

					if ( pmLoaded )
						break;
				}
			} else if ( useSevenZip() ) {
				// try loading image from (semicolon-separated) 7z archive(s)

				// FIXME
			}
		} else {
			// try loading image from (semicolon-separated) software-snapshot folder(s)
			pmLoaded = false;
			foreach (QString baseDirectory, qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareSnapDirectory").toString().split(";", QString::SkipEmptyParts)) {
				QDir snapDir(baseDirectory + "/" + listName);
				foreach (int format, activeFormats) {
					foreach (QString extension, ImageWidget::formatExtensions[format].split(", ", QString::SkipEmptyParts)) {
						QString fullEntryName = entryName + "." + extension;
						if ( snapDir.exists(fullEntryName) ) {
							QString filePath = snapDir.absoluteFilePath(fullEntryName);
							if ( pm.load(filePath) ) {
								pmLoaded = true;
								pm.imagePath = filePath;
								qmc2ImagePixmapCache.insert(myCacheKey, new ImagePixmap(pm), pm.toImage().byteCount()); 
							}
						}
						if ( pmLoaded )
							break;
					}
					if ( pmLoaded )
						break;
				}
				if ( pmLoaded )
					break;
			}
		}
	}

	if ( pmLoaded && !pm.isGhost ) {
		qreal factor = (qreal)zoom / 100.0;
		QSize zoomSize(factor * pm.size().width(), factor * pm.size().height());
		pm = pm.scaled(zoomSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		resize(pm.size());
		switch ( qmc2SoftwareSnapPosition ) {
			case QMC2_SWSNAP_POS_ABOVE_CENTER:
				rect.translate(0, -4);
				switch ( qmc2SoftwareList->toolBoxSoftwareList->currentIndex() ) {
					case QMC2_SWLIST_KNOWN_SW_PAGE:
						position.setX(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->mapToGlobal(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->rect().center()).x() - width() / 2);
						position.setY(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->mapToGlobal(rect.topLeft()).y() - height() - 4);
						break;
					case QMC2_SWLIST_FAVORITES_PAGE:
						position.setX(qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->mapToGlobal(qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->rect().center()).x() - width() / 2);
						position.setY(qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->mapToGlobal(rect.topLeft()).y() - height() - 4);
						break;
					case QMC2_SWLIST_SEARCH_PAGE:
						position.setX(qmc2SoftwareList->treeWidgetSearchResults->viewport()->mapToGlobal(qmc2SoftwareList->treeWidgetSearchResults->viewport()->rect().center()).x() - width() / 2);
						position.setY(qmc2SoftwareList->treeWidgetSearchResults->viewport()->mapToGlobal(rect.topLeft()).y() - height() - 4);
						break;
				}
				break;

			case QMC2_SWSNAP_POS_ABOVE_RIGHT:
				rect.translate(0, -4);
				switch ( qmc2SoftwareList->toolBoxSoftwareList->currentIndex() ) {
					case QMC2_SWLIST_KNOWN_SW_PAGE:
						position.setX(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->mapToGlobal(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->rect().bottomRight()).x() - width());
						position.setY(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->mapToGlobal(rect.topLeft()).y() - height() - 4);
						break;
					case QMC2_SWLIST_FAVORITES_PAGE:
						position.setX(qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->mapToGlobal(qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->rect().bottomRight()).x() - width());
						position.setY(qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->mapToGlobal(rect.topLeft()).y() - height() - 4);
						break;
					case QMC2_SWLIST_SEARCH_PAGE:
						position.setX(qmc2SoftwareList->treeWidgetSearchResults->viewport()->mapToGlobal(qmc2SoftwareList->treeWidgetSearchResults->viewport()->rect().bottomRight()).x() - width());
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
						position.setX(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->mapToGlobal(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->rect().bottomRight()).x() - width());
						break;
					case QMC2_SWLIST_FAVORITES_PAGE:
						position.setX(qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->mapToGlobal(qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->rect().bottomRight()).x() - width());
						break;
					case QMC2_SWLIST_SEARCH_PAGE:
						position.setX(qmc2SoftwareList->treeWidgetSearchResults->viewport()->mapToGlobal(qmc2SoftwareList->treeWidgetSearchResults->viewport()->rect().bottomRight()).x() - width());
						break;
				}
				break;

			case QMC2_SWSNAP_POS_BELOW_CENTER:
				switch ( qmc2SoftwareList->toolBoxSoftwareList->currentIndex() ) {
					case QMC2_SWLIST_KNOWN_SW_PAGE:
						position.setX(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->mapToGlobal(qmc2SoftwareList->treeWidgetKnownSoftware->viewport()->rect().center()).x() - width() / 2);
						break;
					case QMC2_SWLIST_FAVORITES_PAGE:
						position.setX(qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->mapToGlobal(qmc2SoftwareList->treeWidgetFavoriteSoftware->viewport()->rect().center()).x() - width() / 2);
						break;
					case QMC2_SWLIST_SEARCH_PAGE:
						position.setX(qmc2SoftwareList->treeWidgetSearchResults->viewport()->mapToGlobal(qmc2SoftwareList->treeWidgetSearchResults->viewport()->rect().center()).x() - width() / 2);
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
		snapForcedResetTimer.start(QMC2_SWSNAP_UNFORCE_DELAY);
	} else {
		myItem = NULL;
		resetSnapForced();
		qmc2SoftwareList->cancelSoftwareSnap();
	}
}

void SoftwareSnap::refresh()
{
	if ( !myCacheKey.isEmpty() ) {
		qmc2ImagePixmapCache.remove(myCacheKey);
		update();
	}
}

void SoftwareSnap::sevenZipDataReady()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareSnap::sevenZipDataReady()");
#endif

	update();
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

	ctxMenuRequested = true;
	if ( !myCacheKey.isEmpty() ) {
		ImagePixmap *cpm = qmc2ImagePixmapCache.object(myCacheKey);
		if ( cpm )
			actionCopyPathToClipboard->setVisible(!cpm->imagePath.isEmpty());
		else
			actionCopyPathToClipboard->setVisible(false);
	} else
		actionCopyPathToClipboard->setVisible(false);
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

void SoftwareSnap::copyPathToClipboard()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareSnap::copyPathToClipboard()");
#endif

	if ( !myCacheKey.isEmpty() ) {
		ImagePixmap *cpm = qmc2ImagePixmapCache.object(myCacheKey);
		if ( cpm )
			qApp->clipboard()->setText(cpm->imagePath);
	}
}

void SoftwareSnap::reloadActiveFormats()
{
	activeFormats.clear();
	QStringList imgFmts = qmc2Config->value(QMC2_FRONTEND_PREFIX + "ActiveImageFormats/sws", QStringList()).toStringList();
	if ( imgFmts.isEmpty() )
		activeFormats << QMC2_IMAGE_FORMAT_INDEX_PNG;
	else for (int i = 0; i < imgFmts.count(); i++)
		activeFormats << imgFmts[i].toInt();
}

bool SoftwareSnap::useZip()
{
	return qmc2UseSoftwareSnapFile && qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareSnapFileType").toInt() == QMC2_IMG_FILETYPE_ZIP;
}

bool SoftwareSnap::useSevenZip()
{
	return qmc2UseSoftwareSnapFile && qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareSnapFileType").toInt() == QMC2_IMG_FILETYPE_7Z;
}

SoftwareEntryXmlHandler::SoftwareEntryXmlHandler(QTreeWidgetItem *item)
{
#ifdef QMC2_DEBUG
//	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareEntryXmlHandler::SoftwareEntryXmlHandler(QTreeWidgetItem *item = %1)").arg((qulonglong)item));
#endif

	parentTreeWidgetItem = (SoftwareItem *)item;
	softwareName = parentTreeWidgetItem->text(QMC2_SWLIST_COLUMN_NAME);
	softwareValid = success = false;
	partItem = dataareaItem = romItem = NULL;
	elementCounter = animSequenceCounter = 0;
}

SoftwareEntryXmlHandler::~SoftwareEntryXmlHandler()
{
#ifdef QMC2_DEBUG
//	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareEntryXmlHandler::~SoftwareEntryXmlHandler()");
#endif

}

bool SoftwareEntryXmlHandler::startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &attributes)
{
#ifdef QMC2_DEBUG
//	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareEntryXmlHandler::startElement(const QString &namespaceURI = ..., const QString &localName = %1, const QString &qName = %2, const QXmlAttributes &attributes = ...)").arg(localName).arg(qName));
#endif

	if ( ++elementCounter % QMC2_SWLIST_LOAD_RESPONSE_LONG == 0 ) {
		QTreeWidgetItem *item = parentTreeWidgetItem->child(0);
		if ( elementCounter % QMC2_SWLIST_LOAD_ANIM_DELAY == 0 ) {
			if ( item->text(QMC2_SWLIST_COLUMN_TITLE).startsWith(QObject::tr("Searching")) ) {
				QString dot(".");
				item->setText(QMC2_SWLIST_COLUMN_TITLE, QObject::tr("Searching") + dot.repeated(++animSequenceCounter));
			}
		}
		parentTreeWidgetItem->treeWidget()->viewport()->update();
		qApp->processEvents();
	}

	if ( !softwareValid ) {
		if ( qName == "software" ) {
			softwareValid = ( attributes.value("name") == softwareName );
			if ( softwareValid ) {
				qmc2SoftwareList->successfulLookups.clear();
				parentTreeWidgetItem->child(0)->setText(QMC2_SWLIST_COLUMN_TITLE, QObject::tr("Updating"));
				parentTreeWidgetItem->treeWidget()->viewport()->update();
				qApp->processEvents();
			}
		}

		return true;
	}

	if ( qName == "part" ) {
		if ( partItem == NULL ) {
			partItem = new SoftwareItem((QTreeWidget *)NULL);
			partItem->setText(QMC2_SWLIST_COLUMN_TITLE, QObject::tr("Part:") + " " + attributes.value("name"));
			partItem->setText(QMC2_SWLIST_COLUMN_PART, attributes.value("name"));
			partItem->setText(QMC2_SWLIST_COLUMN_INTERFACE, attributes.value("interface"));
			partItem->setText(QMC2_SWLIST_COLUMN_LIST, parentTreeWidgetItem->text(QMC2_SWLIST_COLUMN_LIST));
			QStringList mountList;
			QString mountDev = qmc2SoftwareList->lookupMountDevice(partItem->text(QMC2_SWLIST_COLUMN_PART), partItem->text(QMC2_SWLIST_COLUMN_INTERFACE), &mountList);
			QComboBox *comboBoxMountDevices = NULL;
			if ( mountList.count() > 0 ) {
				comboBoxMountDevices = new QComboBox;
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
			partItems << partItem;
			comboBoxes[partItem] = comboBoxMountDevices;
		}

		return true;
	}

	if ( qName == "feature" ) {
		if ( partItem != NULL ) {
			QString featureName = attributes.value("name");
			if ( featureName == "part id" || featureName == "part_id" ) {
				QString partTitle = attributes.value("value");
				if ( !partTitle.isEmpty() )
					partItem->setText(QMC2_SWLIST_COLUMN_TITLE, partItem->text(QMC2_SWLIST_COLUMN_TITLE) + " (" + partTitle + ")");
			}
		}

		return true;
	}

	if ( qName == "dataarea" ) {
		if ( partItem != NULL ) {
			dataareaItem = new SoftwareItem(partItem);
			dataareaItem->setText(QMC2_SWLIST_COLUMN_TITLE, QObject::tr("Data area:") + " " + attributes.value("name"));
			QString s = attributes.value("size");
			if ( !s.isEmpty() )
				dataareaItem->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("Size:") + " " + s);
		}

		return true;
	}

	if ( qName == "diskarea" ) {
		if ( partItem != NULL ) {
			dataareaItem = new SoftwareItem(partItem);
			dataareaItem->setText(QMC2_SWLIST_COLUMN_TITLE, QObject::tr("Disk area:") + " " + attributes.value("name"));
			QString s = attributes.value("size");
			if ( !s.isEmpty() )
				dataareaItem->setText(QMC2_SWLIST_COLUMN_NAME, QObject::tr("Size:") + " " + s);
		}

		return true;
	}

	if ( qName == "rom" ) {
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

		return true;
	}

	if ( qName == "disk" ) {
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

		return true;
	}

	if ( qName == "info" ) {
		infoItem = new SoftwareItem((QTreeWidget *)NULL);
		infoItem->setText(QMC2_SWLIST_COLUMN_TITLE, QObject::tr("Info:") + " " + attributes.value("name"));
#if defined(QMC2_OS_WIN)
		infoItem->setText(QMC2_SWLIST_COLUMN_NAME, QString::fromUtf8(attributes.value("value").toLocal8Bit()));
#else
		infoItem->setText(QMC2_SWLIST_COLUMN_NAME, attributes.value("value"));
#endif
		infoItems << infoItem;
	}

	return true;
}

bool SoftwareEntryXmlHandler::endElement(const QString &namespaceURI, const QString &localName, const QString &qName)
{
#ifdef QMC2_DEBUG
//	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareEntryXmlHandler::endElement(const QString &namespaceURI = ..., const QString &localName = %1, const QString &qName = %2)").arg(localName).arg(qName));
#endif

	if ( !softwareValid )
		return true;

	if ( qName == "software" ) {
		// stop here...
		parentTreeWidgetItem->treeWidget()->setUpdatesEnabled(false);
		QTreeWidgetItem *childItem = parentTreeWidgetItem->takeChild(0);
		delete childItem;
		parentTreeWidgetItem->addChildren(partItems);
		for (int i = 0; i < partItems.count(); i++) {
			QTreeWidgetItem *item = partItems[i];
			QComboBox *cb = comboBoxes[item];
			if ( cb )
				parentTreeWidgetItem->treeWidget()->setItemWidget(item, QMC2_SWLIST_COLUMN_PUBLISHER, cb);
		}
		if ( !infoItems.isEmpty() )
			parentTreeWidgetItem->addChildren(infoItems);
		parentTreeWidgetItem->treeWidget()->setUpdatesEnabled(true);
		parentTreeWidgetItem->treeWidget()->viewport()->update();
		qApp->processEvents();
		success = true;
		return false;
	}

	if ( qName == "part" ) {
		partItem = NULL;
		return true;
	}

	if ( qName == "dataarea" || qName == "diskarea" ) {
		dataareaItem = NULL;
		return true;
	}

	if ( qName == "rom" ) {
		romItem = NULL;
		return true;
	}

	return true;
}

SoftwareSnapshot::SoftwareSnapshot(QWidget *parent)
#if QMC2_OPENGL == 1
	: QGLWidget(parent)
#else
	: QWidget(parent)
#endif
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareSnapshot::SoftwareSnapshot(QWidget *parent = %1)").arg((qulonglong)parent));
#endif

	contextMenu = new QMenu(this);
	contextMenu->hide();

	QString s;
	QAction *action;

	s = tr("Copy image to clipboard");
	action = contextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/editcopy.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(copyToClipboard()));

	s = tr("Copy file path to clipboard");
	action = contextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/editcopy.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(copyPathToClipboard()));
	actionCopyPathToClipboard = action;

	contextMenu->addSeparator();

	s = tr("Refresh cache slot");
	action = contextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/reload.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(refresh()));

	reloadActiveFormats();
}

SoftwareSnapshot::~SoftwareSnapshot()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SoftwareSnapshot::~SoftwareSnapshot()");
#endif

	if ( qmc2UseSoftwareSnapFile ) {
		foreach (unzFile snapFile, qmc2SoftwareSnap->snapFileMap)
			unzClose(snapFile);
		foreach (SevenZipFile *snapFile, qmc2SoftwareSnap->snapFileMap7z) {
			snapFile->close();
			delete snapFile;
		}
		qmc2SoftwareSnap->snapFileMap.clear();
		qmc2SoftwareSnap->snapFileMap7z.clear();
	}
}

void SoftwareSnapshot::paintEvent(QPaintEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareSnapshot::paintEvent(QPaintEvent *e = %1)").arg((qulonglong)e));
#endif

	QPainter p(this);

	if ( !qmc2SoftwareList->currentItem ) {
		drawCenteredImage(0, &p); // clear snapshot widget
		myCacheKey.clear();
		return;
	}

	QString listName = qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_LIST);
	QString entryName = qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_NAME);
	myCacheKey = "sws_" + listName + "_" + entryName;

	ImagePixmap *cpm = qmc2ImagePixmapCache.object(myCacheKey);
	if ( !cpm )
		loadSnapshot(listName, entryName);
	else {
		currentSnapshotPixmap = *cpm;
		currentSnapshotPixmap.imagePath = cpm->imagePath;
	}

	drawScaledImage(&currentSnapshotPixmap, &p);
}

QString SoftwareSnapshot::toBase64()
{
	ImagePixmap pm;
	if ( !currentSnapshotPixmap.isNull() )
		pm = currentSnapshotPixmap;
	else
		pm = qmc2MainWindow->qmc2GhostImagePixmap;
	QByteArray imageData;
	QBuffer buffer(&imageData);
	pm.save(&buffer, "PNG");
	return QString(imageData.toBase64());
}

void SoftwareSnapshot::refresh()
{
	if ( !myCacheKey.isEmpty() ) {
		qmc2ImagePixmapCache.remove(myCacheKey);
		update();
	}
}

bool SoftwareSnapshot::loadSnapshot(QString listName, QString entryName)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SoftwareSnapshot::loadSnapshot(QString listName = %1, QString entryName = %2)").arg(listName).arg(entryName));
#endif

	ImagePixmap pm;
	bool fileOk = true;

	myCacheKey = "sws_" + listName + "_" + entryName;
	currentSnapshotPixmap.imagePath.clear();

	if ( qmc2UseSoftwareSnapFile ) {
		if ( qmc2SoftwareSnap->useZip() ) {
			// try loading image from (semicolon-separated) ZIP archive(s)
			if ( qmc2SoftwareSnap->snapFileMap.isEmpty() ) {
				foreach (QString filePath, qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareSnapFile").toString().split(";", QString::SkipEmptyParts)) {
					qmc2SoftwareSnap->snapFileMap[filePath] = unzOpen(filePath.toLocal8Bit());
					if ( qmc2SoftwareSnap->snapFileMap[filePath] == NULL )
						qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open software snap-shot file, please check access permissions for %1").arg(filePath));
				}
			}
			foreach (unzFile snapFile, qmc2SoftwareSnap->snapFileMap) {
				if ( snapFile ) {
					foreach (int format, activeFormats) {
						QString formatName = ImageWidget::formatNames[format];
						foreach (QString extension, ImageWidget::formatExtensions[format].split(", ", QString::SkipEmptyParts)) {
							QByteArray imageData;
							QString pathInZip = listName + "/" + entryName + "." + extension;
							if ( unzLocateFile(snapFile, pathInZip.toLocal8Bit().constData(), 0) == UNZ_OK ) {
								if ( unzOpenCurrentFile(snapFile) == UNZ_OK ) {
									char imageBuffer[QMC2_ZIP_BUFFER_SIZE];
									int len;
									while ( (len = unzReadCurrentFile(snapFile, &imageBuffer, QMC2_ZIP_BUFFER_SIZE)) > 0 ) {
										for (int i = 0; i < len; i++)
											imageData += imageBuffer[i];
									}
									unzCloseCurrentFile(snapFile);
									fileOk = true;
								} else
									fileOk = false;
							} else
								fileOk = false;

							if ( fileOk ) {
								if ( pm.loadFromData(imageData, formatName.toLocal8Bit().constData()) ) {
									qmc2ImagePixmapCache.insert(myCacheKey, new ImagePixmap(pm), pm.toImage().byteCount());
									break;
								} else
									fileOk = false;
							}

							if ( fileOk )
								break;
						}

						if ( fileOk )
							break;
					}
				}

				if ( fileOk )
					break;
			}
		} else  if ( qmc2SoftwareSnap->useSevenZip() ) {
			// try loading image from (semicolon-separated) 7z archive(s)

			// FIXME
		}
	} else {
		// try loading image from (semicolon-separated) software-snapshot folder(s)
		fileOk = false;
		foreach (QString baseDirectory, qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareSnapDirectory").toString().split(";", QString::SkipEmptyParts)) {
			QDir snapDir(baseDirectory + "/" + listName);
			foreach (int format, activeFormats) {
				foreach (QString extension, ImageWidget::formatExtensions[format].split(", ", QString::SkipEmptyParts)) {
					QString fullEntryName = entryName + "." + extension;
					if ( snapDir.exists(fullEntryName) ) {
						QString filePath = snapDir.absoluteFilePath(fullEntryName);
						if ( pm.load(filePath) ) {
							fileOk = true;
							currentSnapshotPixmap = pm;
							currentSnapshotPixmap.imagePath = filePath;
							qmc2ImagePixmapCache.insert(myCacheKey, new ImagePixmap(currentSnapshotPixmap), currentSnapshotPixmap.toImage().byteCount()); 
						} else
							fileOk = false;
					}

					if ( fileOk )
						break;
				}

				if ( fileOk )
					break;
			}

			if ( fileOk )
				break;
		}
	}

	if ( !fileOk ) {
		currentSnapshotPixmap = qmc2MainWindow->qmc2GhostImagePixmap;
		if ( !qmc2RetryLoadingImages )
			qmc2ImagePixmapCache.insert(myCacheKey, new ImagePixmap(currentSnapshotPixmap), currentSnapshotPixmap.toImage().byteCount());
        }

	return fileOk;
}

void SoftwareSnapshot::drawCenteredImage(QPixmap *pm, QPainter *p)
{
	p->eraseRect(rect());

	if ( pm == NULL ) {
		p->end();
		return;
	}

	// last resort if pm->load() retrieved a null pixmap...
	if ( pm->isNull() )
		pm = &qmc2MainWindow->qmc2GhostImagePixmap;

	int posx = (rect().width() - pm->width()) / 2;
	int posy = (rect().height() - pm->height()) / 2;

	p->drawPixmap(posx, posy, *pm);

	if ( qmc2ShowGameName ) {
		// draw entry title
		p->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform);
		QString title = qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_TITLE);
		QFont f(qApp->font());
		f.setWeight(QFont::Bold);
		p->setFont(f);
		QFontMetrics fm(f);
		QRect r = rect();
		int adjustment = fm.height() / 2;
		r = r.adjusted(+adjustment, +adjustment, -adjustment, -adjustment);
		QRect outerRect = p->boundingRect(r, Qt::AlignCenter | Qt::TextWordWrap, title);
		r.setTop(r.bottom() - outerRect.height());
		r = p->boundingRect(r, Qt::AlignCenter | Qt::TextWordWrap, title);
		r = r.adjusted(-adjustment, -adjustment, +adjustment, +adjustment);
		r.setBottom(rect().bottom());
		QPainterPath pp;
		pp.addRoundedRect(r, 5, 5);
		p->fillPath(pp, QBrush(QColor(0, 0, 0, 128), Qt::SolidPattern));
		p->setPen(QPen(QColor(255, 255, 255, 255)));
		p->drawText(r, Qt::AlignCenter | Qt::TextWordWrap, title);
	}

	p->end();
}

void SoftwareSnapshot::drawScaledImage(QPixmap *pm, QPainter *p)
{
	if ( pm == NULL ) {
		p->eraseRect(rect());
		p->end();
		return;
	}

	// last resort if pm->load() retrieved a null pixmap...
	if ( pm->isNull() )
		pm = &qmc2MainWindow->qmc2GhostImagePixmap;

	double desired_width;
	double desired_height;

	if ( pm->width() > pm->height() ) {
		desired_width  = contentsRect().width();
		desired_height = (double)pm->height() * (desired_width / (double)pm->width());
		if ( desired_height > contentsRect().height() ) {
			desired_height = contentsRect().height();
			desired_width  = (double)pm->width() * (desired_height / (double)pm->height());
		}
	} else {
		desired_height = contentsRect().height();
		desired_width  = (double)pm->width() * (desired_height / (double)pm->height());
		if ( desired_width > contentsRect().width() ) {
			desired_width = contentsRect().width();
			desired_height = (double)pm->height() * (desired_width / (double)pm->width());
		}
	}

	QPixmap pmScaled;

	if ( qmc2SmoothScaling )
		pmScaled = pm->scaled((int)desired_width, (int)desired_height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	else
		pmScaled = pm->scaled((int)desired_width, (int)desired_height, Qt::KeepAspectRatio, Qt::FastTransformation);

	drawCenteredImage(&pmScaled, p);
}

void SoftwareSnapshot::copyToClipboard()
{
	qApp->clipboard()->setPixmap(currentSnapshotPixmap);
}

void SoftwareSnapshot::copyPathToClipboard()
{
	if ( !currentSnapshotPixmap.imagePath.isEmpty() )
		qApp->clipboard()->setText(currentSnapshotPixmap.imagePath);
}

void SoftwareSnapshot::contextMenuEvent(QContextMenuEvent *e)
{
	actionCopyPathToClipboard->setVisible(!currentSnapshotPixmap.imagePath.isEmpty());
	contextMenu->move(qmc2MainWindow->adjustedWidgetPosition(mapToGlobal(e->pos()), contextMenu));
	contextMenu->show();
}

void SoftwareSnapshot::reloadActiveFormats()
{
	activeFormats.clear();
	QStringList imgFmts = qmc2Config->value(QMC2_FRONTEND_PREFIX + "ActiveImageFormats/sws", QStringList()).toStringList();
	if ( imgFmts.isEmpty() )
		activeFormats << QMC2_IMAGE_FORMAT_INDEX_PNG;
	else for (int i = 0; i < imgFmts.count(); i++)
		activeFormats << imgFmts[i].toInt();
}
