#ifndef _MESSDEVCFG_H_
#define _MESSDEVCFG_H_

#include <QItemDelegate>
#include <QModelIndex>
#include <QXmlDefaultHandler>
#include "filesystemmodel.h"

#include "ui_messdevcfg.h"

class MESSDeviceFileDelegate : public QItemDelegate
{
	Q_OBJECT

	public:
		MESSDeviceFileDelegate(QObject *parent = 0);

		QWidget *createEditor(QWidget *, const QStyleOptionViewItem &, const QModelIndex &) const;
		virtual void setEditorData(QWidget *, const QModelIndex &) const;
		virtual void setModelData(QWidget *, QAbstractItemModel *, const QModelIndex &) const;
		virtual void updateEditorGeometry(QWidget *, const QStyleOptionViewItem &, const QModelIndex &) const;

	public slots:
		void dataChanged(QWidget *);

	signals:
		void editorDataChanged(const QString &);
};

class MESSDeviceConfiguratorXmlHandler : public QXmlDefaultHandler
{
	public:
		QTreeWidget *parentTreeWidget;
		QString deviceType;
		QString deviceTag;
		QString deviceBriefName;
		QStringList deviceInstances;
		QStringList deviceExtensions;
		QString slotName;
		QStringList newSlots;
		QStringList allSlots;
		QMap<QString, QStringList> newSlotOptions;
		QMap<QString, QString> newSlotDevices;
		QMap<QString, QString> slotDeviceNames;
		QMap<QString, QString> defaultSlotOptions;

		MESSDeviceConfiguratorXmlHandler(QTreeWidget *);
		~MESSDeviceConfiguratorXmlHandler();

		bool startElement(const QString &, const QString &, const QString &, const QXmlAttributes &);
		bool endElement(const QString &, const QString &, const QString &);
		bool characters(const QString &);
};

class MESSDeviceConfigurator : public QWidget, public Ui::MESSDeviceConfigurator
{
	Q_OBJECT

	public:
		bool dontIgnoreNameChange;
		bool refreshRunning;
		bool updateSlots;
		bool isLoading;
		bool fileChooserSetup;
		bool isManualSlotOptionChange;
		bool includeFolders;
		bool foldersFirst;
		bool fullyLoaded;
		MESSDeviceFileDelegate fileEditDelegate;
		QString messMachineName;
		QString currentConfigName;
		QMap<QString, QPair<QStringList, QStringList> > configurationMap;
		QMap<QString, QPair<QStringList, QStringList> > slotMap;
		QMap<QString, QPair<QStringList, QStringList> > slotBiosMap;
		QMap<QString, QString> extensionInstanceMap;
		QMenu *deviceConfigurationListMenu;
		QMenu *configurationMenu;
		QMenu *deviceContextMenu;
		QMenu *slotContextMenu;
		QMenu *dirChooserContextMenu;
		QMenu *fileChooserContextMenu;
		QMenu *folderModeMenu;
		QAction *actionRenameConfiguration;
		QAction *actionRemoveConfiguration;
		QByteArray fileChooserHeaderState;
		QByteArray dirChooserHeaderState;
		QAction *actionChooserToggleArchive;
		QTimer searchTimer;
		bool comboBoxChooserFilterPatternHadFocus;
		FileSystemModel *fileModel;
		int fileModelRowInsertionCounter;
		QAction *actionChooserOpenExternally;
		QAction *actionChooserOpenFolder;
		QAction *actionChooserPlay;
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
		QAction *actionChooserPlayEmbedded;
#endif
		DirectoryModel *dirModel;
		QModelIndex modelIndexFileModel;
		QModelIndex modelIndexDirModel;
		QString normalXmlBuffer;
		QString slotXmlBuffer;
		QMap<QComboBox *, int> slotPreselectionMap;
		QMap<QComboBox *, int> nestedSlotPreselectionMap;
		QMap<QComboBox *, int> newNestedSlotPreselectionMap;
		QStringList nestedSlots;
		QMap<QString, QMap<QString, QString> > nestedSlotOptionMap;
		QStringList allSlots;
		QMap<QString, QString> slotDeviceNames;
		QListWidgetItem *configurationRenameItem;
		QString oldConfigurationName;

		MESSDeviceConfigurator(QString, QWidget *);
		~MESSDeviceConfigurator();

		QString &getXmlData(QString);
		QString &getXmlDataWithEnabledSlots(QString);
		QComboBox *comboBoxByName(QString, QTreeWidgetItem **returnItem = NULL);
		void addNestedSlot(QString, QStringList, QStringList, QString);
		void insertChildItems(QTreeWidgetItem *, QList<QTreeWidgetItem *> &);
		void checkRemovedSlots(QTreeWidgetItem *parentItem = NULL);
		bool checkParentSlot(QTreeWidgetItem *, QString &);
		void updateSlotBiosSelections();

	public slots:
		void preselectSlots();
		bool readSystemSlots();
		bool refreshDeviceMap();
		void preselectNestedSlots();
		bool load();
		bool save();
		void saveSetup();

		// auto-connected callback functions
		void on_lineEditConfigurationName_textChanged(const QString &);
		void on_toolButtonNewConfiguration_clicked();
		void on_toolButtonCloneConfiguration_clicked();
		void on_toolButtonSaveConfiguration_clicked();
		void on_toolButtonRemoveConfiguration_clicked();
		void on_listWidgetDeviceConfigurations_itemActivated(QListWidgetItem *);
		void on_listWidgetDeviceConfigurations_currentTextChanged(const QString &);
		void on_listWidgetDeviceConfigurations_itemClicked(QListWidgetItem *);
		void on_listWidgetDeviceConfigurations_customContextMenuRequested(const QPoint &);
		void on_treeWidgetDeviceSetup_customContextMenuRequested(const QPoint &);
		void on_treeWidgetSlotOptions_customContextMenuRequested(const QPoint &);
		void on_tabWidgetDeviceSetup_currentChanged(int);
		void on_toolButtonChooserFilter_toggled(bool);
		void on_comboBoxDeviceInstanceChooser_activated(const QString &);
		void on_treeViewDirChooser_customContextMenuRequested(const QPoint &);
		void on_treeViewFileChooser_customContextMenuRequested(const QPoint &);
		void on_treeViewFileChooser_activated(const QModelIndex &);
		void on_toolButtonChooserSaveConfiguration_clicked();
		void on_toolButtonChooserReload_clicked();
		void on_comboBoxChooserFilterPattern_editTextChanged(const QString &);
		void on_splitterFileChooser_splitterMoved(int, int);

		// other callbacks
		void actionSelectDefaultDeviceDirectory_triggered();
		void actionSelectFile_triggered();
		void actionRenameConfiguration_activated();
		void actionRemoveConfiguration_activated();
		void treeViewDirChooser_selectionChanged(const QItemSelection &, const QItemSelection &);
		void treeViewFileChooser_selectionChanged(const QItemSelection &, const QItemSelection &);
		void dirChooserUseCurrentAsDefaultDirectory();
		void dirChooserDelayedInit();
		void fileModel_rowsInserted(const QModelIndex &, int, int);
		void fileModel_finished();
		void treeViewDirChooser_headerClicked(int);
		void treeViewFileChooser_headerClicked(int);
		void treeViewFileChooser_sectionMoved(int, int, int);
		void treeViewFileChooser_sectionResized(int, int, int);
		void comboBoxChooserFilterPattern_editTextChanged_delayed();
		void treeViewFileChooser_toggleArchive();
		void treeViewFileChooser_expandRequested();
		void treeViewFileChooser_openFolder();
		void treeViewFileChooser_openFileExternally();
		void slotOptionChanged(int);
		void folderModeMenu_foldersOff();
		void folderModeMenu_foldersOn();
		void folderModeMenu_foldersFirst();

		// misc
		void editorDataChanged(const QString &);
		void setupFileChooser();
		void configurationItemChanged(QListWidgetItem *);

	protected:
		void closeEvent(QCloseEvent *);
		void showEvent(QShowEvent *);
		void hideEvent(QHideEvent *);
};

class FileChooserKeyEventFilter : public QObject
{
	Q_OBJECT

	public:
		FileChooserKeyEventFilter(QObject *parent = 0) : QObject(parent) { ; }

	protected:
		bool eventFilter(QObject *obj, QEvent *event);

	signals:
		void expandRequested();
};

#endif
