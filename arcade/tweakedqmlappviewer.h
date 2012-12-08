#ifndef TWEAKEDQMLAPPVIEWER_H
#define TWEAKEDQMLAPPVIEWER_H

#include <QTimer>
#include <QMap>

#include "qmlapplicationviewer.h"

class TweakedQmlApplicationViewer : public QmlApplicationViewer
{
    Q_OBJECT

public:
    int numFrames;
    QTimer frameCheckTimer;
    QByteArray savedGeometry;
    bool savedMaximized;
    QList<QObject *> gameList;

    explicit TweakedQmlApplicationViewer(QWidget *parent = 0);
    virtual ~TweakedQmlApplicationViewer();

public slots:
    void fpsReady();
    void loadSettings();
    void saveSettings();
    void switchToFullScreen(bool initially = false);
    void switchToWindowed(bool initially = false);
    QString romStateText(int status);
    int romStateCharToInt(char status);
    void loadGamelist();

protected:
    void paintEvent(QPaintEvent *);
    void closeEvent(QCloseEvent *);
};

#endif
