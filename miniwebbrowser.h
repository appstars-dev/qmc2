#ifndef _MINIWEBBROWSER_H_
#define _MINIWEBBROWSER_H_

#include <QWebView>
#include <QMouseEvent>
#include <QTimer>
#include <QCache>
#include "ui_miniwebbrowser.h"
#include "macros.h"

class MiniWebBrowser;

class BrowserWidget : public QWebView
{
  Q_OBJECT

  public:
    QPoint lastMouseClickPosition;
    bool mouseCurrentlyOnView;
    QTimer bwuDelayTimer;
    MiniWebBrowser *parentBrowser;

    BrowserWidget(QWidget *parent, MiniWebBrowser *browserParent) : QWebView(parent)
    {
      bwuDelayTimer.setSingleShot(true);
      lastMouseClickPosition = QPoint(-1, -1);
      mouseCurrentlyOnView = false;
      parentBrowser = browserParent;
    }

  public slots:
    void delayedUpdate()
    {
      if ( !bwuDelayTimer.isActive() ) {
        QTimer::singleShot(QMC2_MAWS_BWU_DELAY, this, SLOT(update()));
        bwuDelayTimer.start(QMC2_MAWS_BWU_DELAY);
      }
    }

  signals:
    void mouseOnView(bool);

  protected:
    void mousePressEvent(QMouseEvent *e)
    {
      lastMouseClickPosition = e->pos();
      QWebView::mousePressEvent(e);
    }
    void enterEvent(QEvent *e)
    {
      QWebView::enterEvent(e);
      mouseCurrentlyOnView = true;
      emit mouseOnView(true);
    }
    void leaveEvent(QEvent *e)
    {
      QWebView::leaveEvent(e);
      mouseCurrentlyOnView = false;
      emit mouseOnView(false);
    }
    QWebView *createWindow(QWebPage::WebWindowType);
};

class MiniWebBrowser : public QWidget, public Ui::MiniWebBrowser
{
  Q_OBJECT

  public:
    QUrl homeUrl;
    static QCache<QString, QIcon> iconCache;
    static QStringList supportedSchemes;
    bool firstTimeLoadStarted,
         firstTimeLoadProgress,
         firstTimeLoadFinished;
    QTimer statusTimer;
    BrowserWidget *webViewBrowser;

    MiniWebBrowser(QWidget *parent = 0);
    ~MiniWebBrowser();

  public slots:
    void on_comboBoxURL_activated();
    void on_toolButtonHome_clicked();
    void on_toolButtonLoad_clicked();
    void on_spinBoxZoom_valueChanged(int);

    // page actions
    void processPageActionDownloadRequested(const QNetworkRequest &);
    void processPageActionHandleUnsupportedContent(QNetworkReply *);

    // other
    void webViewBrowser_linkClicked(const QUrl);
    void webViewBrowser_urlChanged(const QUrl);
    void webViewBrowser_loadStarted();
    void webViewBrowser_loadFinished(bool);
    void webViewBrowser_loadProgress(int);
    void webViewBrowser_statusBarMessage(const QString &);
    void webViewBrowser_iconChanged();
    void webViewBrowser_linkHovered(const QString &, const QString &, const QString &);
    void webViewBrowser_statusBarVisibilityChangeRequested(bool);
    void webViewBrowser_frameCreated(QWebFrame *);
    void statusTimeout();

  signals:
    void titleChanged(QString &);
};

#endif
