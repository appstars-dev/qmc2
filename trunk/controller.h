#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#include <QMap>
#include <QMenu>
#include <QWidget>
#include <QPixmap>
#include <QPainter>
#include <QTreeWidgetItem>
#if QMC2_OPENGL == 1
#include <QGLWidget>
#endif

#include "unzip.h"

#if QMC2_OPENGL == 1
class Controller : public QGLWidget
#else
class Controller : public QWidget
#endif
{
  Q_OBJECT

  public:
    unzFile controllerFile;
#if QT_VERSION < 0x040600
    QPixmap *currentControllerPixmap;
#else
    QPixmap currentControllerPixmap;
#endif
    QMenu *contextMenu;

    Controller(QWidget *parent = 0);
    ~Controller();

  public slots:
    void drawCenteredImage(QPixmap *, QPainter *);
    void drawScaledImage(QPixmap *, QPainter *);
    bool loadController(QString, QString, bool checkOnly = FALSE, QString *fileName = NULL);
    void copyToClipboard();

  protected:
    void paintEvent(QPaintEvent *);
    void contextMenuEvent(QContextMenuEvent *);
};

#endif
