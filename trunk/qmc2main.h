#ifndef _QMC2_MAIN_H_
#define _QMC2_MAIN_H_

#include <QApplication>
#include <QCloseEvent>
#include <QSocketNotifier>
#include <QTimer>
#include <QTime>
#include <QNetworkReply>
#include <QWidgetAction>
#include "ui_qmc2main.h"
#include "ui_options.h"
#include "macros.h"
#if QMC2_USE_PHONON_API
#include "qmc2_phonon.h"
#endif

class KeyPressFilter : public QObject
{
  Q_OBJECT

  public:
    KeyPressFilter(QObject *parent = NULL) : QObject(parent) { ; }

  protected:
    bool eventFilter(QObject *, QEvent *);
};

#if defined(QMC2_EMUTYPE_MAME)
class AutoPopupToolButton : public QToolButton
{
  Q_OBJECT

  public:
    QTimer menuTimer;

    AutoPopupToolButton(QWidget *parent = 0) : QToolButton(parent)
    {
      connect(&menuTimer, SIGNAL(timeout()), this, SLOT(hideMenu()));
    }

  signals:
    void paintFinished();
    void menuHidden();

  public slots:
    void hideMenu()
    {
      if ( menu() )
        if ( menu()->activeAction() == NULL ) {
          QTimer::singleShot(0, menu(), SLOT(hide()));
          emit menuHidden();
        }
    }

  protected:
    void enterEvent(QEvent *e)
    {
      QToolButton::enterEvent(e);
      QTimer::singleShot(0, this, SLOT(showMenu()));
      menuTimer.stop();
    }
    void leaveEvent(QEvent *e)
    {
      QToolButton::leaveEvent(e);
      menuTimer.start(1000);
    }
    void paintEvent(QPaintEvent *e)
    {
      QToolButton::paintEvent(e);
      emit paintFinished();
    }
};
#endif

class MainWindow : public QMainWindow, public Ui::MainWindow
{
  Q_OBJECT

  public:
    bool isActiveState;
    QTimer searchTimer;
    QTimer updateTimer;
    QPixmap qmc2GhostImagePixmap;
    QPushButton *pushButtonGlobalEmulatorOptionsExportToFile;
    QMenu *selectMenuGlobalEmulatorOptionsExportToFile;
    QPushButton *pushButtonGlobalEmulatorOptionsImportFromFile;
    QMenu *selectMenuGlobalEmulatorOptionsImportFromFile;
    QPushButton *pushButtonCurrentEmulatorOptionsExportToFile;
    QMenu *selectMenuCurrentEmulatorOptionsExportToFile;
    QPushButton *pushButtonCurrentEmulatorOptionsImportFromFile;
    QMenu *selectMenuCurrentEmulatorOptionsImportFromFile;
    QLabel *labelEmuSelector;
    QComboBox *comboBoxEmuSelector;
    QMenu *menuRomStatusFilter;
    QMenu *menuTabWidgetGamelist;
    QMenu *menuTabWidgetGameDetail;
    QMenu *menuTabWidgetLogsAndEmulators;
    QMenu *menuTabWidgetSoftwareDetail;
    QMenu *menuHorizontalSplitter;
    QMenu *menuVerticalSplitter;
    QMenu *menuGamelistHeader;
    QMenu *menuHierarchyHeader;
    QMenu *menuCategoryHeader;
    QAction *actionMenuGamelistHeaderCategory;
    QAction *actionMenuHierarchyHeaderCategory;
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
    QMenu *menuVersionHeader;
    QAction *actionMenuGamelistHeaderVersion;
    QAction *actionMenuHierarchyHeaderVersion;
#endif
    QAction *actionRomStatusFilterC;
    QAction *actionRomStatusFilterM;
    QAction *actionRomStatusFilterI;
    QAction *actionRomStatusFilterN;
    QAction *actionRomStatusFilterU;
    QList<QAction *> contextMenuPlayActions;
#if QMC2_JOYSTICK == 1
    int joyIndex;
#endif
#if QMC2_USE_PHONON_API
    Phonon::MediaObject *phononAudioPlayer;
    Phonon::AudioOutput *phononAudioOutput;
    Phonon::Path phononAudioPath;
    bool audioFastForwarding;
    bool audioFastBackwarding;
    bool audioSkippingTracks;
    Phonon::State audioState;
#endif
#if defined(QMC2_EMUTYPE_MAME)
    AutoPopupToolButton *toolButtonMAWSQuickLinks;
    QMenu *menuMAWSQuickLinks;
    QMap<QString, QAction *> mawsQDLActions;
#endif
#if defined(QMC2_MEMORY_INFO_ENABLED)
    QTimer memoryUpdateTimer;
#endif
#if defined(Q_WS_X11) || defined(Q_WS_WIN)
    QWidget *widgetEmbeddedEmus;
    QWidget *embedderCornerWidget;
    QHBoxLayout *embedderCornerLayout;
    QToolButton *toolButtonEmbedderMaximizeToggle;
#endif
#if defined(Q_WS_X11)
    QToolButton *toolButtonEmbedderAutoPause;
    QMenu *menuAutoPause;
#endif
    QList<int> hSplitterSizes;
    QRect desktopGeometry;
    QWidget *hSplitterWidget0;
    QWidget *vSplitterWidget0;
    QTimer activityCheckTimer;
    bool activityState;
    QString urlSectionRegExp;
    int retry_tabWidgetGameDetail_currentIndex;

    QString foreignEmuName;
    QString foreignID;
    QString foreignDescription;
    bool launchForeignID;

    QWidgetAction *widgetActionToolbarSearch;
    QComboBox *comboBoxToolbarSearch;

    static QColor qmc2StatusColorGreen;
    static QColor qmc2StatusColorYellowGreen;
    static QColor qmc2StatusColorRed;
    static QColor qmc2StatusColorBlue;
    static QColor qmc2StatusColorGrey;

    QToolButton *floatToggleButtonSoftwareDetail;

    int sortCriteriaLogicalIndex();
    QPoint adjustedWidgetPosition(QPoint, QWidget *);
    QStringList &getXmlChoices(QString, QString, QString optionAttribute = QString());
    static bool qStringListLessThan(const QString &, const QString &);

    MainWindow(QWidget *parent = 0);
    ~MainWindow();

  public slots:
    // game menu
    void on_actionPlay_triggered(bool checked = false);
    void on_actionPlayEmbedded_triggered(bool checked = false);
    void on_actionPlayTagged_triggered(bool checked = false);
    void on_actionPlayEmbeddedTagged_triggered(bool checked = false);
    void on_actionToFavorites_triggered(bool checked = false);
    void on_actionToFavoritesTagged_triggered(bool checked = false);
    void on_actionReload_triggered(bool checked = false);
    void on_actionExitStop_triggered(bool checked = false);
    void on_actionCheckCurrentROM_triggered(bool checked = false);
    void on_actionCheckROMStateTagged_triggered(bool checked = false);
    void on_actionAnalyseCurrentROM_triggered(bool checked = false);
    void on_actionAnalyseROMTagged_triggered(bool checked = false);
    void on_actionRunRomTool_triggered(bool checked = false);
    void on_actionRunRomToolTagged_triggered(bool checked = false);
    void on_actionSetTag_triggered(bool checked = false);
    void on_actionUnsetTag_triggered(bool checked = false);
    void on_actionToggleTag_triggered(bool checked = false);
    void on_actionToggleTagCursorDown_triggered(bool checked = false);
    void on_actionToggleTagCursorUp_triggered(bool checked = false);
    void on_actionTagAll_triggered(bool checked = false);
    void on_actionUntagAll_triggered(bool checked = false);
    void on_actionInvertTags_triggered(bool checked = false);
    void on_actionTagVisible_triggered(bool checked = false);
    void on_actionUntagVisible_triggered(bool checked = false);
    void on_actionInvertVisibleTags_triggered(bool checked = false);

    // arcade menu
    void on_actionArcadeToggle_triggered(bool checked = false);
    void on_actionArcadeSetup_triggered(bool checked = false);

    // tools menu
    void on_actionCheckROMs_triggered(bool checked = false);
    void on_actionCheckSamples_triggered(bool checked = false);
    void on_actionCheckImagesAndIcons_triggered(bool checked = false);
    void on_actionROMAlyzer_triggered(bool checked = false);
    void on_actionExportROMStatus_triggered(bool checked = false);
    void on_actionDemoMode_triggered(bool checked = false);
    void on_actionNewBrowserWindow_triggered(bool checked = false);
    void on_actionClearImageCache_triggered(bool checked = false);
    void on_actionClearIconCache_triggered(bool checked = false);
    void on_actionClearMAWSCache_triggered(bool checked = false);
#if defined(QMC2_YOUTUBE_ENABLED)
    void on_actionClearYouTubeCache_triggered(bool checked = false);
#endif
    void on_actionRecreateTemplateMap_triggered(bool checked = false);
    void on_actionCheckTemplateMap_triggered(bool checked = false);
    void on_actionClearROMStateCache_triggered(bool checked = false);
    void on_actionClearGamelistCache_triggered(bool checked = false);
    void on_actionClearXMLCache_triggered(bool checked = false);
    void on_actionClearSoftwareListCache_triggered(bool checked = false);
    void on_actionClearAllEmulatorCaches_triggered(bool checked = false);
    void on_actionOptions_triggered(bool checked = false);

    // display menu
    void on_actionFullscreenToggle_triggered(bool checked = false);
    void on_actionLaunchQMC2MAME_triggered(bool checked = false);
    void on_actionLaunchQMC2MESS_triggered(bool checked = false);
    void on_actionLaunchQMC2UME_triggered(bool checked = false);

    // help menu
    void on_actionDocumentation_triggered(bool checked = false);
    void on_actionAbout_triggered(bool checked = false);
    void on_actionAboutQt_triggered(bool checked = false);

    // search widget
    void on_comboBoxSearch_editTextChanged(const QString &);
    void comboBoxSearch_editTextChanged_delayed();
    void on_comboBoxSearch_activated(const QString &);
    void on_listWidgetSearch_currentTextChanged(QString);
    void on_listWidgetSearch_itemActivated(QListWidgetItem *);
    void on_listWidgetSearch_currentItemChanged(QListWidgetItem *, QListWidgetItem *);
    void on_listWidgetSearch_itemPressed(QListWidgetItem *);
    void on_listWidgetSearch_itemSelectionChanged();

    // favorites & played widgets
    void on_listWidgetFavorites_currentTextChanged(QString);
    void on_listWidgetFavorites_itemSelectionChanged();
    void on_listWidgetFavorites_itemActivated(QListWidgetItem *);
    void on_listWidgetPlayed_currentTextChanged(QString);
    void on_listWidgetPlayed_itemSelectionChanged();
    void on_listWidgetPlayed_itemActivated(QListWidgetItem *);

    // context menus
    void on_treeWidgetGamelist_customContextMenuRequested(const QPoint &);
    void on_treeWidgetHierarchy_customContextMenuRequested(const QPoint &);
    void on_listWidgetSearch_customContextMenuRequested(const QPoint &);
    void on_listWidgetFavorites_customContextMenuRequested(const QPoint &);
    void on_listWidgetPlayed_customContextMenuRequested(const QPoint &);
    void on_treeWidgetEmulators_customContextMenuRequested(const QPoint &);
    void on_tabWidgetGamelist_customContextMenuRequested(const QPoint &);
    void on_tabWidgetGameDetail_customContextMenuRequested(const QPoint &);
    void on_tabWidgetLogsAndEmulators_customContextMenuRequested(const QPoint &);
    void on_tabWidgetSoftwareDetail_customContextMenuRequested(const QPoint &p);
    void on_hSplitter_customContextMenuRequested(const QPoint &);
    void on_vSplitter_customContextMenuRequested(const QPoint &);

    // splitter flip/swap callbacks
    void menuHorizontalSplitter_FlipOrientation_activated();
    void menuHorizontalSplitter_SwapLayouts_activated();
    void menuVerticalSplitter_FlipOrientation_activated();
    void menuVerticalSplitter_SwapWidgets_activated();

    // joystick functions
#if QMC2_JOYSTICK == 1
    void joystickAxisValueChanged(int, int);
    void joystickButtonValueChanged(int, bool);
    void joystickHatValueChanged(int, int);
    void joystickTrackballValueChanged(int, int, int);
    void mapJoystickFunction(QString);
#endif

    // audio player functions
    void on_actionAudioPreviousTrack_triggered(bool checked = false);
    void toolButtonAudioPreviousTrack_resetButton();
    void on_actionAudioNextTrack_triggered(bool checked = false);
    void toolButtonAudioNextTrack_resetButton();
    void on_actionAudioFastBackward_triggered(bool checked = false);
    void on_toolButtonAudioFastBackward_clicked(bool checked = false);
    void toolButtonAudioFastBackward_resetButton();
    void on_actionAudioFastForward_triggered(bool checked = false);
    void on_toolButtonAudioFastForward_clicked(bool checked = false);
    void toolButtonAudioFastForward_resetButton();
    void on_actionAudioStopTrack_triggered(bool checked = false);
    void on_actionAudioPauseTrack_triggered(bool checked = false);
    void on_actionAudioPlayTrack_triggered(bool checked = false);
    void on_toolButtonAudioAddTracks_clicked();
    void on_toolButtonAudioAddURL_clicked();
    void on_toolButtonAudioRemoveTracks_clicked();
    void on_toolButtonAudioSetupEffects_clicked();
    void on_listWidgetAudioPlaylist_itemSelectionChanged();
    void on_dialAudioVolume_valueChanged(int);
    void on_actionAudioRaiseVolume_triggered(bool checked = false);
    void on_actionAudioLowerVolume_triggered(bool checked = false);
    void audioFinished();
    void audioTick(qint64);
    void audioTotalTimeChanged(qint64);
    void audioFade(int);
    void audioMetaDataChanged();
    void audioBufferStatus(int);
    void audioScrollToCurrentItem();

    // download manager widget
    void on_checkBoxRemoveFinishedDownloads_stateChanged(int);

    // ROM state filter toggles
    void romStateFilterC_toggled(bool);
    void romStateFilterM_toggled(bool);
    void romStateFilterI_toggled(bool);
    void romStateFilterN_toggled(bool);
    void romStateFilterU_toggled(bool);

    // arcade mode actions and functions
    void on_actionArcadeShowFPS_toggled(bool);
    void on_actionArcadeTakeScreenshot_triggered();
    void destroyArcadeView();

    // tab widget position callbacks
    void menuTabWidgetGamelist_North_activated();
    void menuTabWidgetGamelist_South_activated();
    void menuTabWidgetGamelist_West_activated();
    void menuTabWidgetGamelist_East_activated();
    void menuTabWidgetGameDetail_North_activated();
    void menuTabWidgetGameDetail_South_activated();
    void menuTabWidgetGameDetail_West_activated();
    void menuTabWidgetGameDetail_East_activated();
    void menuTabWidgetGameDetail_Setup_activated();
    void menuTabWidgetLogsAndEmulators_North_activated();
    void menuTabWidgetLogsAndEmulators_South_activated();
    void menuTabWidgetLogsAndEmulators_West_activated();
    void menuTabWidgetLogsAndEmulators_East_activated();
    void menuTabWidgetSoftwareDetail_North_activated();
    void menuTabWidgetSoftwareDetail_South_activated();
    void menuTabWidgetSoftwareDetail_West_activated();
    void menuTabWidgetSoftwareDetail_East_activated();

    // memory indicator
    void memoryUpdateTimer_timeout();

    // other
    void on_tabWidgetGameDetail_currentChanged(int);
    void retry_tabWidgetGameDetail_currentChanged() { on_tabWidgetGameDetail_currentChanged(retry_tabWidgetGameDetail_currentIndex); };
    void tabWidgetGameDetail_tabMoved(int, int);
    void on_tabWidgetGamelist_currentChanged(int);
    void on_tabWidgetLogsAndEmulators_currentChanged(int);
    void tabWidgetLogsAndEmulators_updateCurrent() { on_tabWidgetLogsAndEmulators_currentChanged(tabWidgetLogsAndEmulators->currentIndex()); };
    void on_tabWidgetSoftwareDetail_currentChanged(int);
    void tabWidgetSoftwareDetail_updateCurrent() { on_tabWidgetSoftwareDetail_currentChanged(tabWidgetSoftwareDetail->currentIndex()); };
    void on_treeWidgetGamelist_itemActivated(QTreeWidgetItem *, int);
    void on_treeWidgetGamelist_itemDoubleClicked(QTreeWidgetItem *, int); 
    void on_treeWidgetGamelist_itemExpanded(QTreeWidgetItem *);
    void on_treeWidgetGamelist_currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *);
    void on_treeWidgetGamelist_itemSelectionChanged();
    void treeWidgetGamelist_itemSelectionChanged_delayed();
    void on_treeWidgetHierarchy_itemActivated(QTreeWidgetItem *, int);
    void on_treeWidgetHierarchy_itemDoubleClicked(QTreeWidgetItem *, int);
    void on_treeWidgetHierarchy_currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *);
    void on_treeWidgetHierarchy_itemSelectionChanged();
    void on_stackedWidgetView_currentChanged(int);
    void pushButtonGlobalEmulatorOptionsExportToFile_clicked(QString useFileName = QString());
    void pushButtonGlobalEmulatorOptionsSelectExportFile_clicked();
    void pushButtonGlobalEmulatorOptionsImportFromFile_clicked(QString useFileName = QString());
    void pushButtonGlobalEmulatorOptionsSelectImportFile_clicked();
    void pushButtonCurrentEmulatorOptionsExportToFile_clicked(QString useFileName = QString());
    void pushButtonCurrentEmulatorOptionsSelectExportFile_clicked();
    void pushButtonCurrentEmulatorOptionsImportFromFile_clicked(QString useFileName = QString());
    void pushButtonCurrentEmulatorOptionsSelectImportFile_clicked();
#if defined(Q_WS_X11) || defined(Q_WS_WIN)
    void action_embedEmulator_triggered();
    void on_tabWidgetEmbeddedEmulators_tabCloseRequested(int);
    void embedderOptions_toggled(bool);
    void closeEmbeddedEmuTab();
    void toolButtonEmbedderMaximizeToggle_toggled(bool);
    void embedderOptionsMenu_KillEmulator_activated();
    void embedderOptionsMenu_TerminateEmulator_activated();
    void embedderOptionsMenu_ToFavorites_activated();
    void embedderOptionsMenu_CopyCommand_activated();
#endif
#if defined(Q_WS_X11)
    void action_embedderScanPauseKey_triggered();
#endif
    void action_terminateEmulator_triggered();
    void action_killEmulator_triggered();
    void action_copyEmulatorCommand_triggered();
    void action_removeFromFavorites_triggered();
    void action_clearAllFavorites_triggered();
    void action_saveFavorites_triggered();
    void action_removeFromPlayed_triggered();
    void action_clearAllPlayed_triggered();
    void action_savePlayed_triggered();
    void scrollToCurrentItem();
    void checkCurrentSearchSelection();
    void checkCurrentFavoritesSelection();
    void checkCurrentPlayedSelection();
    void log(char, QString);
    void init();
    void setupStyle(QString);
    void setupStyleSheet(QString);
    void viewFullDetail();
    void viewParentClones();
    void loadGameInfoDB();
    void loadEmuInfoDB();
#if defined(QMC2_YOUTUBE_ENABLED)
    void loadYouTubeVideoInfoMap();
#endif
    void on_treeWidgetGamelist_itemEntered(QTreeWidgetItem *, int);
    void on_treeWidgetGamelist_itemPressed(QTreeWidgetItem *, int);
    void on_treeWidgetHierarchy_itemEntered(QTreeWidgetItem *, int);
    void on_treeWidgetHierarchy_itemPressed(QTreeWidgetItem *, int);
    void on_treeWidgetCategoryView_itemEntered(QTreeWidgetItem *, int);
    void on_treeWidgetCategoryView_itemPressed(QTreeWidgetItem *, int);
    void treeWidgetCategoryView_headerSectionClicked(int);
    void on_treeWidgetCategoryView_itemActivated(QTreeWidgetItem *, int);
    void on_treeWidgetCategoryView_itemDoubleClicked(QTreeWidgetItem *, int);
    void on_treeWidgetCategoryView_currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *);
    void on_treeWidgetCategoryView_itemSelectionChanged();
    void on_treeWidgetCategoryView_customContextMenuRequested(const QPoint &);
    void viewByCategory();
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
    void on_treeWidgetVersionView_itemEntered(QTreeWidgetItem *, int);
    void on_treeWidgetVersionView_itemPressed(QTreeWidgetItem *, int);
    void treeWidgetVersionView_headerSectionClicked(int);
    void on_treeWidgetVersionView_itemActivated(QTreeWidgetItem *, int);
    void on_treeWidgetVersionView_itemDoubleClicked(QTreeWidgetItem *, int);
    void on_treeWidgetVersionView_currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *);
    void on_treeWidgetVersionView_itemSelectionChanged();
    void on_treeWidgetVersionView_customContextMenuRequested(const QPoint &);
    void viewByVersion();
#endif
#if defined(QMC2_EMUTYPE_MAME)
    void mawsLoadStarted();
    void mawsLoadFinished(bool);
    void mawsQuickLinksSetVisible(bool);
    void mawsQuickLinksMenuHidden();
    void createMawsQuickLinksMenu();
    void setupMawsQuickLinks();
    void downloadMawsQuickLink();
    void storeMawsIcon();
    void startMawsAutoDownloads();
#endif
#if defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
    void projectMessSystemLoadStarted();
    void projectMessSystemLoadFinished(bool);
    void projectMessLoadStarted();
    void projectMessLoadFinished(bool);
#endif
    void createFifo(bool logFifoCreation = true);
    void recreateFifo();
    void processFifoData();
    void on_hSplitter_splitterMoved(int, int);
    void on_vSplitter_splitterMoved(int, int);
    void on_comboBoxViewSelect_currentIndexChanged(int);
    void processEvents() { qApp->processEvents(); }
    void treeWidgetGamelist_headerSectionClicked(int);
    void treeWidgetHierarchy_headerSectionClicked(int);
#if defined(QMC2_EMUTYPE_MESS)
    QString &messWikiToHtml(QString &);
#endif
    void startDownload(QWidget *, QNetworkReply *, QString saveAsName = QString(), QString savePath = QString());
    void on_pushButtonClearFinishedDownloads_clicked();
    void on_pushButtonReloadSelectedDownloads_clicked();
    void on_pushButtonStopSelectedDownloads_clicked();
    void emuSelector_currentIndexChanged(const QString &);
    void checkActivity();
    void enableContextMenuPlayActions(bool);
    void softwareLoadInterrupted();
    void action_foreignIDsMenuItem_triggered();
    void checkRomPath();

    // float toggle button callbacks for 'special' widgets
    void floatToggleButtonSoftwareDetail_toggled(bool);
    void stackedWidgetSpecial_setCurrentIndex(int);

    // callbacks for list view header context menu requests
    void treeWidgetGamelistHeader_customContextMenuRequested(const QPoint &);
    void actionGamelistHeader_triggered();
    void treeWidgetHierarchyHeader_customContextMenuRequested(const QPoint &);
    void actionHierarchyHeader_triggered();
    void treeWidgetCategoryViewHeader_customContextMenuRequested(const QPoint &);
    void actionCategoryHeader_triggered();
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
    void treeWidgetVersionViewHeader_customContextMenuRequested(const QPoint &);
    void actionVersionHeader_triggered();
#endif
    void comboBoxToolbarSearch_editTextChanged(const QString &);
    void comboBoxToolbarSearch_activated(const QString &);

  protected:
    void closeEvent(QCloseEvent *);
};

#endif
