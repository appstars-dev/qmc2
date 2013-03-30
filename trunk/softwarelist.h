#ifndef _SOFTWARELIST_H_
#define _SOFTWARELIST_H_

#include <QProcess>
#include <QTime>
#include <QTimer>
#include <QFile>
#include <QAction>
#include <QTextStream>
#include <QXmlDefaultHandler>
#include <QMenu>
#if QMC2_OPENGL == 1
#include <QGLWidget>
#endif
#include "ui_softwarelist.h"
#include "unzip.h"
#include "swlistexport.h"
#include "imagewidget.h"

class SoftwareListExporter;

class SoftwareItem : public QTreeWidgetItem
{
	public:
		SoftwareItem(QTreeWidget *parent) : QTreeWidgetItem(parent) { ; }
		SoftwareItem(QTreeWidgetItem *parent) : QTreeWidgetItem(parent) { ; }
		SoftwareItem(SoftwareItem *parent) : QTreeWidgetItem((QTreeWidgetItem *)parent) { ; }

	protected:
		bool operator<(const QTreeWidgetItem &other) const
		{
			if ( parent() != NULL )
				return false;
			if ( other.parent() != NULL )
				return false;
			int col = treeWidget()->sortColumn();
			return ( text(col) < other.text(col) );
		}
};

class SoftwareListXmlHandler : public QXmlDefaultHandler
{
	public:
		QTreeWidget *parentTreeWidget;
		SoftwareItem *softwareItem;
		QString softwareListName;
		QString softwareName;
		QString softwareSupported;
		QString softwareTitle;
		QString softwarePublisher;
		QString softwareYear;
		QString softwarePart;
		QString softwareInterface;
		QString currentText;
		QStringList compatFilters;
		int elementCounter;

		SoftwareListXmlHandler(QTreeWidget *);
		~SoftwareListXmlHandler();
		
		bool startElement(const QString &, const QString &, const QString &, const QXmlAttributes &);
		bool endElement(const QString &, const QString &, const QString &);
		bool characters(const QString &);
};

class SoftwareEntryXmlHandler : public QXmlDefaultHandler
{
	public:
		SoftwareItem *parentTreeWidgetItem;
		SoftwareItem *partItem;
		SoftwareItem *infoItem;
		SoftwareItem *dataareaItem;
		SoftwareItem *romItem;
		bool softwareValid;
		bool success;
		QString softwareName;
		int elementCounter;
		int animSequenceCounter;
		QList<QTreeWidgetItem *> partItems;
		QList<QTreeWidgetItem *> infoItems;
		QMap<QTreeWidgetItem *, QComboBox *> comboBoxes;

		SoftwareEntryXmlHandler(QTreeWidgetItem *);
		~SoftwareEntryXmlHandler();
		
		bool startElement(const QString &, const QString &, const QString &, const QXmlAttributes &);
		bool endElement(const QString &, const QString &, const QString &);
};

// 'floating' snapshot-viewer
class SoftwareSnap : public QWidget
{
	Q_OBJECT

	public:
		QString listName;
		QString entryName;
		QPoint position;
		QMap<QString, unzFile> snapFileMap;
		SoftwareItem *myItem;
		QTimer snapForcedResetTimer;
		QMenu *contextMenu;
		bool ctxMenuRequested;
		QString myCacheKey;
		QAction *actionCopyPathToClipboard;
		int zoom;

		SoftwareSnap(QWidget *parent = 0);
		~SoftwareSnap();

		QString primaryPathFor(QString, QString);

	public slots:
		void loadSnapshot();
		bool replaceImage(QString, QString, QPixmap &);
		void resetSnapForced();
		void copyToClipboard();
		void copyPathToClipboard();
		void refresh();
		void zoomIn();
		void zoomOut();
		void resetZoom();

	protected:
		void paintEvent(QPaintEvent *);
		void mousePressEvent(QMouseEvent *);
		void enterEvent(QEvent *);
		void leaveEvent(QEvent *);
		void contextMenuEvent(QContextMenuEvent *);
};

// 'embedded' snapshot-viewer
#if QMC2_OPENGL == 1
class SoftwareSnapshot : public QGLWidget
#else
class SoftwareSnapshot : public QWidget
#endif
{
	Q_OBJECT

	public:
		ImagePixmap currentSnapshotPixmap;
		QMenu *contextMenu;
		QString myCacheKey;
		QAction *actionCopyPathToClipboard;

		SoftwareSnapshot(QWidget *parent = 0);
		~SoftwareSnapshot();

		QString toBase64();

	public slots:
		void drawCenteredImage(QPixmap *, QPainter *);
		void drawScaledImage(QPixmap *, QPainter *);
		bool loadSnapshot(QString, QString);
		void copyToClipboard();
		void copyPathToClipboard();
		void refresh();

	protected:
		void paintEvent(QPaintEvent *);
		void contextMenuEvent(QContextMenuEvent *);
};

class SoftwareList : public QWidget, public Ui::SoftwareList
{
	Q_OBJECT
	
	public:
		QProcess *loadProc;
		QProcess *verifyProc;
		QTime loadTimer;
		QFile fileSWLCache;
		QString systemName;
		QTextStream tsSWLCache;
		QStringList swlLines;
		QMenu *softwareListMenu;
		QMenu *favoritesOptionsMenu;
		QMenu *menuKnownSoftwareHeader;
		QMenu *menuFavoriteSoftwareHeader;
		QMenu *menuSearchResultsHeader;
		QMenu *menuSnapnameAdjustment;
		QMenu *menuSoftwareStates;
		QMenu *menuSearchOptions;
		QAction *actionAddToFavorites;
		QAction *actionRemoveFromFavorites;
		QAction *actionSaveFavoritesToFile;
		QAction *actionClearSelection;
		QAction *actionNegateSearch;
		QTimer snapTimer;
		QTimer searchTimer;
		QTimer detailUpdateTimer;
		bool snapForced;
		bool validData;
		bool autoSelectSearchItem;
		bool autoMounted;
		bool interruptLoad;
		bool isLoading;
		bool fullyLoaded;
		bool loadFinishedFlag;
		bool updatingMountDevices;
		bool negatedMatch;
		QStringList successfulLookups;
		QStringList mountedSoftware;
		int cachedDeviceLookupPosition;
		SoftwareListExporter *exporter;
		int oldMax, oldMin;
		QString oldFmt;
		QTreeWidgetItem *currentItem;
		QMap<QString, int> softwareStateMap;
		QString softwareListName;
		QList<QTreeWidgetItem *> softwareListItems;
		quint64 numSoftwareCorrect, numSoftwareIncorrect, numSoftwareMostlyCorrect, numSoftwareNotFound, numSoftwareUnknown;
		bool verifyReadingStdout;

		SoftwareList(QString, QWidget *);
		~SoftwareList();

		QString &getSoftwareListXmlData(QString);
		QString &getXmlData();
		QString &getXmlDataWithEnabledSlots(QStringList);
		QString &lookupMountDevice(QString, QString, QStringList *mountList = NULL);
		QStringList &arguments(QStringList *softwareLists = NULL, QStringList *softwareNames = NULL);

	public slots:
		bool load();
		bool save();
		void updateMountDevices();
		void checkSoftwareStates();

		// auto-connected callback functions
		void on_toolButtonReload_clicked(bool);
		void on_toolButtonExport_clicked(bool);
		void on_toolButtonAddToFavorites_clicked(bool);
		void on_toolButtonToggleSoftwareInfo_clicked(bool);
		void on_toolButtonCompatFilterToggle_clicked(bool);
		void on_toolButtonToggleSnapnameAdjustment_clicked(bool);
		void on_toolButtonSoftwareStates_clicked(bool);
		void on_toolButtonRemoveFromFavorites_clicked(bool);
		void on_toolButtonPlay_clicked(bool);
		void on_toolButtonPlayEmbedded_clicked(bool);
		void on_treeWidgetKnownSoftware_itemSelectionChanged();
		void on_treeWidgetKnownSoftware_customContextMenuRequested(const QPoint &);
		void on_treeWidgetKnownSoftware_itemEntered(QTreeWidgetItem *, int);
		void on_treeWidgetKnownSoftware_itemActivated(QTreeWidgetItem *, int);
		void on_treeWidgetKnownSoftware_itemDoubleClicked(QTreeWidgetItem *item, int column);
		void on_treeWidgetKnownSoftware_itemExpanded(QTreeWidgetItem *);
		void on_treeWidgetKnownSoftware_itemClicked(QTreeWidgetItem *, int) { on_treeWidgetKnownSoftware_itemSelectionChanged(); }
		void on_treeWidgetFavoriteSoftware_itemSelectionChanged();
		void on_treeWidgetFavoriteSoftware_customContextMenuRequested(const QPoint &);
		void on_treeWidgetFavoriteSoftware_itemEntered(QTreeWidgetItem *, int);
		void on_treeWidgetFavoriteSoftware_itemActivated(QTreeWidgetItem *, int);
		void on_treeWidgetFavoriteSoftware_itemDoubleClicked(QTreeWidgetItem *item, int column);
		void on_treeWidgetFavoriteSoftware_itemExpanded(QTreeWidgetItem *);
		void on_treeWidgetFavoriteSoftware_itemClicked(QTreeWidgetItem *, int) { on_treeWidgetFavoriteSoftware_itemSelectionChanged(); }
		void on_treeWidgetSearchResults_itemSelectionChanged();
		void on_treeWidgetSearchResults_customContextMenuRequested(const QPoint &);
		void on_treeWidgetSearchResults_itemEntered(QTreeWidgetItem *, int);
		void on_treeWidgetSearchResults_itemActivated(QTreeWidgetItem *, int);
		void on_treeWidgetSearchResults_itemDoubleClicked(QTreeWidgetItem *item, int column);
		void on_treeWidgetSearchResults_itemExpanded(QTreeWidgetItem *);
		void on_treeWidgetSearchResults_itemClicked(QTreeWidgetItem *, int) { on_treeWidgetSearchResults_itemSelectionChanged(); }
		void on_comboBoxSearch_editTextChanged(const QString &);
		void on_comboBoxSearch_activated(const QString &);
		void on_comboBoxDeviceConfiguration_currentIndexChanged(int);
		void on_toolBoxSoftwareList_currentChanged(int);

		// process management
		void loadStarted();
		void loadFinished(int, QProcess::ExitStatus);
		void loadReadyReadStandardOutput();
		void loadReadyReadStandardError();
		void loadError(QProcess::ProcessError);
		void loadStateChanged(QProcess::ProcessState);
		void verifyStarted();
		void verifyFinished(int, QProcess::ExitStatus);
		void verifyReadyReadStandardOutput();
		void verifyReadyReadStandardError();
		void verifyError(QProcess::ProcessError);
		void verifyStateChanged(QProcess::ProcessState);
 
		// other
		void treeWidgetKnownSoftware_headerSectionClicked(int);
		void treeWidgetFavoriteSoftware_headerSectionClicked(int);
		void treeWidgetSearchResults_headerSectionClicked(int);
		void addToFavorites() { on_toolButtonAddToFavorites_clicked(false); }
		void removeFromFavorites() { on_toolButtonRemoveFromFavorites_clicked(false); }
		void playActivated() { on_toolButtonPlay_clicked(false); }
		void playEmbeddedActivated() { on_toolButtonPlayEmbedded_clicked(false); }
		void cancelSoftwareSnap();
		void comboBoxSearch_editTextChanged_delayed();
		void checkMountDeviceSelection();
		void loadFavoritesFromFile();
		void saveFavoritesToFile();
		void checkSoftwareSnap();
		void updateDetail();
		void adjustSnapnamePattern();
		void clearSoftwareSelection();
		void negateSearchTriggered(bool);

		// callbacks for software-list header context menu requests
		void treeWidgetKnownSoftwareHeader_customContextMenuRequested(const QPoint &);
		void actionKnownSoftwareHeader_triggered();
		void treeWidgetFavoriteSoftwareHeader_customContextMenuRequested(const QPoint &);
		void actionFavoriteSoftwareHeader_triggered();
		void treeWidgetSearchResultsHeader_customContextMenuRequested(const QPoint &);
		void actionSearchResultsHeader_triggered();

	protected:
		void closeEvent(QCloseEvent *);
		void showEvent(QShowEvent *);
		void hideEvent(QHideEvent *);
		void mouseMoveEvent(QMouseEvent *);
		void leaveEvent(QEvent *);
		void resizeEvent(QResizeEvent *);
};

#endif
