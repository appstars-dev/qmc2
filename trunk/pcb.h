#ifndef _PCB_H_
#define _PCB_H_

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
class PCB : public QGLWidget
#else
class PCB : public QWidget
#endif
{
  Q_OBJECT

  public:
    unzFile pcbFile;
#if QT_VERSION < 0x040600
    QPixmap *currentPCBPixmap;
#else
    QPixmap currentPCBPixmap;
#endif
    QMenu *contextMenu;

    PCB(QWidget *parent = 0);
    ~PCB();

  public slots:
    void drawCenteredImage(QPixmap *, QPainter *);
    void drawScaledImage(QPixmap *, QPainter *);
    bool loadPCB(QString, QString, bool checkOnly = FALSE, QString *fileName = NULL);
    void copyToClipboard();

  protected:
    void paintEvent(QPaintEvent *);
    void contextMenuEvent(QContextMenuEvent *);
};

#endif
