#ifndef ARCADESETTINGS_H
#define ARCADESETTINGS_H

#include <QSettings>
#include <QLocale>
#include <QSize>

class ArcadeSettings : public QSettings
{
    Q_OBJECT

public:
    QString arcadeTheme;
    QString frontEndPrefix;
    QString emulatorPrefix;
    QMap<QString, QLocale::Language> languageMap;

    explicit ArcadeSettings(QString);
    virtual ~ArcadeSettings();
    
    QString languageToString(QLocale::Language);
    QLocale::Language languageFromString(QString);

signals:
    
public slots:
    // global settings
    void setApplicationVersion(QString);
    QString applicationVersion();
    void setViewerGeometry(QByteArray);
    QByteArray viewerGeometry();
    void setViewerMaximized(bool);
    bool viewerMaximized();
    void setConsoleGeometry(QByteArray);
    QByteArray consoleGeometry();
    void setUseFilteredList(bool);
    bool useFilteredList();
    void setFilteredListFile(QString);
    QString filteredListFile();

    // default settings for command line arguments
    void setDefaultTheme(QString);
    QString defaultTheme();
    void setDefaultConsoleType(QString);
    QString defaultConsoleType();
    void setDefaultGraphicsSystem(QString);
    QString defaultGraphicsSystem();
    void setDefaultLanguage(QString);
    QString defaultLanguage();

    // theme-specific settings
    void setFpsVisible(bool);
    bool fpsVisible();
    void setFullScreen(bool);
    bool fullScreen();
    void setLastIndex(int);
    int lastIndex();
    // ToxicWaste
    void setShowBackgroundAnimation(bool);
    bool showBackgroundAnimation();
    void setAnimateInForeground(bool);
    bool animateInForeground();
    void setSecondaryImageType(QString);
    QString secondaryImageType();
    void setCabinetFlipped(bool);
    bool cabinetFlipped();
    void setMenuHidden(bool);
    bool menuHidden();
    void setShowShaderEffect(bool);
    bool showShaderEffect();
    void setConfirmQuit(bool);
    bool confirmQuit();
    void setGameCardPage(int);
    int gameCardPage();
    // darkone
    void setToolbarHidden(bool);
    bool toolbarHidden();
    void setListHidden(bool);
    bool listHidden();
    void setSortByName(bool);
    bool sortByName();
    void setBackLight(bool);
    bool backLight();
    void setToolbarAutoHide(bool);
    bool toolbarAutoHide();
    void setLaunchFlash(bool);
    bool launchFlash();
    void setLaunchZoom(bool);
    bool launchZoom();
    void setDataTypePrimary(QString);
    QString dataTypePrimary();
    void setDataTypeSecondary(QString);
    QString dataTypeSecondary();
    void setLightTimeout(double);
    double lightTimeout();
    void setOverlayScale(double);
    double overlayScale();
    void setColourScheme(QString);
    QString colourScheme();

    // main frontend / emulator settings (from QMC2, read-only)
    QString gameListCacheFile();
    QString romStateCacheFile();
    bool previewsZipped();
    QString previewZipFile();
    QString previewFolder();
    bool flyersZipped();
    QString flyerZipFile();
    QString flyerFolder();
    bool cabinetsZipped();
    QString cabinetZipFile();
    QString cabinetFolder();
    bool controllersZipped();
    QString controllerZipFile();
    QString controllerFolder();
    bool marqueesZipped();
    QString marqueeZipFile();
    QString marqueeFolder();
    bool titlesZipped();
    QString titleZipFile();
    QString titleFolder();
    bool pcbsZipped();
    QString pcbZipFile();
    QString pcbFolder();
    QString optionsTemplateFile();
    QString emulatorExecutablePath();
    QString emulatorWorkingDirectory();
    QString gameInfoDB();
    bool compressGameInfoDB();
    QString emuInfoDB();
    bool compressEmuInfoDB();
    int joystickAxisMinimum(int, int);
    int joystickAxisMaximum(int, int);
};

#endif
