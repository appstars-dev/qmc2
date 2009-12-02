#include <QWebHistory>
#include <QWebFrame>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QDir>

#include "miniwebbrowser.h"
#include "macros.h"
#include "qmc2main.h"

extern MainWindow *qmc2MainWindow;
extern QNetworkAccessManager *qmc2NetworkAccessManager;

QCache<QString, QIcon> MiniWebBrowser::iconCache;
QStringList MiniWebBrowser::supportedSchemes;

MiniWebBrowser::MiniWebBrowser(QWidget *parent)
  : QWidget(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MiniWebBrowser::MiniWebBrowser(QWidget *parent = %1)").arg((qulonglong) parent));
#endif

  setObjectName("MiniWebBrowser");

  if ( MiniWebBrowser::supportedSchemes.isEmpty() )
    MiniWebBrowser::supportedSchemes << "http" << "file";

  setupUi(this);

  webViewBrowser = new BrowserWidget(frameBrowser);
  webViewBrowser->setObjectName("webViewBrowser");
  gridLayoutBrowser->addWidget(webViewBrowser);

  labelStatus->hide();
  progressBar->hide();

  QFontMetrics fm(qApp->font());
  QSize iconSize(fm.height() - 2, fm.height() - 2);

  toolButtonBack->setIconSize(iconSize);
  toolButtonForward->setIconSize(iconSize);
  toolButtonReload->setIconSize(iconSize);
  toolButtonStop->setIconSize(iconSize);
  toolButtonHome->setIconSize(iconSize);
  toolButtonLoad->setIconSize(iconSize);

  firstTimeLoadStarted = TRUE;
  firstTimeLoadProgress = TRUE;
  firstTimeLoadFinished = TRUE;
 
  iconCache.setMaxCost(QMC2_BROWSER_ICONCACHE_SIZE);

  // we want the same global network access manager for all browsers
  webViewBrowser->page()->setNetworkAccessManager(qmc2NetworkAccessManager);

  // we want to manipulate the link activation
  webViewBrowser->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

  // connect page actions we provide
  connect(webViewBrowser->page(), SIGNAL(downloadRequested(const QNetworkRequest &)), this, SLOT(processPageActionDownloadRequested(const QNetworkRequest &)));
  connect(webViewBrowser->page(), SIGNAL(unsupportedContent(QNetworkReply *)), this, SLOT(processPageActionHandleUnsupportedContent(QNetworkReply *)));
  connect(webViewBrowser->page(), SIGNAL(linkHovered(const QString &, const QString &, const QString &)), this, SLOT(webViewBrowser_linkHovered(const QString &, const QString &, const QString &)));
  connect(webViewBrowser->page(), SIGNAL(statusBarVisibilityChangeRequested(bool)), this, SLOT(webViewBrowser_statusBarVisibilityChangeRequested(bool)));
  connect(webViewBrowser->page(), SIGNAL(frameCreated(QWebFrame *)), this, SLOT(webViewBrowser_frameCreated(QWebFrame *)));

  connect(webViewBrowser, SIGNAL(linkClicked(const QUrl)), this, SLOT(webViewBrowser_linkClicked(const QUrl)));
  connect(webViewBrowser, SIGNAL(urlChanged(const QUrl)), this, SLOT(webViewBrowser_urlChanged(const QUrl)));
  connect(webViewBrowser, SIGNAL(loadStarted()), this, SLOT(webViewBrowser_loadStarted()));
  connect(webViewBrowser, SIGNAL(loadFinished(bool)), this, SLOT(webViewBrowser_loadFinished(bool)));
  connect(webViewBrowser, SIGNAL(loadProgress(int)), this, SLOT(webViewBrowser_loadProgress(int)));
  connect(webViewBrowser, SIGNAL(statusBarMessage(const QString &)), this, SLOT(webViewBrowser_statusBarMessage(const QString &)));
  connect(webViewBrowser, SIGNAL(iconChanged()), this, SLOT(webViewBrowser_iconChanged()));
  connect(toolButtonBack, SIGNAL(clicked()), webViewBrowser, SLOT(back()));
  connect(toolButtonForward, SIGNAL(clicked()), webViewBrowser, SLOT(forward()));
  connect(toolButtonReload, SIGNAL(clicked()), webViewBrowser, SLOT(reload()));
  connect(toolButtonStop, SIGNAL(clicked()), webViewBrowser, SLOT(stop()));

  // hide page actions we don't provide
  webViewBrowser->pageAction(QWebPage::OpenImageInNewWindow)->setVisible(FALSE);
  webViewBrowser->pageAction(QWebPage::OpenFrameInNewWindow)->setVisible(FALSE);
  webViewBrowser->pageAction(QWebPage::OpenLinkInNewWindow)->setVisible(FALSE);

  // change provided page actions to better fit our usage / integrate into QMC2's look
  webViewBrowser->pageAction(QWebPage::OpenLink)->setText(tr("Open link"));
  webViewBrowser->pageAction(QWebPage::OpenLink)->setIcon(QIcon(QString::fromUtf8(":/data/img/fileopen.png")));
  webViewBrowser->pageAction(QWebPage::DownloadLinkToDisk)->setText(tr("Save link as..."));
  webViewBrowser->pageAction(QWebPage::DownloadLinkToDisk)->setIcon(QIcon(QString::fromUtf8(":/data/img/filesaveas.png")));
  webViewBrowser->pageAction(QWebPage::CopyLinkToClipboard)->setText(tr("Copy link"));
  webViewBrowser->pageAction(QWebPage::CopyLinkToClipboard)->setIcon(QIcon(QString::fromUtf8(":/data/img/editcopy.png")));
  webViewBrowser->pageAction(QWebPage::DownloadImageToDisk)->setText(tr("Save image as..."));
  webViewBrowser->pageAction(QWebPage::DownloadImageToDisk)->setIcon(QIcon(QString::fromUtf8(":/data/img/filesaveas.png")));
  webViewBrowser->pageAction(QWebPage::CopyImageToClipboard)->setText(tr("Copy image"));
  webViewBrowser->pageAction(QWebPage::CopyImageToClipboard)->setIcon(QIcon(QString::fromUtf8(":/data/img/editcopy.png")));
  webViewBrowser->pageAction(QWebPage::Back)->setText(tr("Go back"));
  webViewBrowser->pageAction(QWebPage::Back)->setIcon(QIcon(QString::fromUtf8(":/data/img/back.png")));
  webViewBrowser->pageAction(QWebPage::Forward)->setText(tr("Go forward"));
  webViewBrowser->pageAction(QWebPage::Forward)->setIcon(QIcon(QString::fromUtf8(":/data/img/forward.png")));
  webViewBrowser->pageAction(QWebPage::Reload)->setText(tr("Reload"));
  webViewBrowser->pageAction(QWebPage::Reload)->setIcon(QIcon(QString::fromUtf8(":/data/img/reload.png")));
  webViewBrowser->pageAction(QWebPage::Stop)->setText(tr("Stop"));
  webViewBrowser->pageAction(QWebPage::Stop)->setIcon(QIcon(QString::fromUtf8(":/data/img/stop_browser.png")));
  webViewBrowser->pageAction(QWebPage::Copy)->setText(tr("Copy"));
  webViewBrowser->pageAction(QWebPage::Copy)->setIcon(QIcon(QString::fromUtf8(":/data/img/editcopy.png")));
#if defined(QMC2_BROWSER_EXTRAS_ENABLED)
  webViewBrowser->pageAction(QWebPage::InspectElement)->setText(tr("Inspect"));
  webViewBrowser->pageAction(QWebPage::InspectElement)->setIcon(QIcon(QString::fromUtf8(":/data/img/inspect.png")));
#endif

  // setup browser settings
  webViewBrowser->page()->settings()->setIconDatabasePath(QMC2_DOT_PATH);
  webViewBrowser->page()->settings()->setAttribute(QWebSettings::AutoLoadImages, TRUE);
  webViewBrowser->page()->settings()->setAttribute(QWebSettings::JavascriptEnabled, TRUE);
  webViewBrowser->page()->settings()->setAttribute(QWebSettings::JavaEnabled, TRUE);
#if defined(QMC2_BROWSER_PLUGINS_ENABLED)
  webViewBrowser->page()->settings()->setAttribute(QWebSettings::PluginsEnabled, TRUE);
#else
  webViewBrowser->page()->settings()->setAttribute(QWebSettings::PluginsEnabled, FALSE);
#endif
  webViewBrowser->page()->settings()->setAttribute(QWebSettings::PrivateBrowsingEnabled, FALSE);
  webViewBrowser->page()->settings()->setAttribute(QWebSettings::JavascriptCanOpenWindows, FALSE);
  webViewBrowser->page()->settings()->setAttribute(QWebSettings::JavascriptCanAccessClipboard, FALSE);
#if defined(QMC2_BROWSER_EXTRAS_ENABLED)
  webViewBrowser->page()->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, TRUE);
#else
  webViewBrowser->page()->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, FALSE);
#endif
  webViewBrowser->page()->settings()->setAttribute(QWebSettings::LinksIncludedInFocusChain, FALSE);
  webViewBrowser->page()->settings()->setAttribute(QWebSettings::ZoomTextOnly, FALSE);
  webViewBrowser->page()->settings()->setAttribute(QWebSettings::PrintElementBackgrounds, FALSE);
  webViewBrowser->page()->settings()->setAttribute(QWebSettings::OfflineStorageDatabaseEnabled, FALSE);
  webViewBrowser->page()->settings()->setAttribute(QWebSettings::OfflineWebApplicationCacheEnabled, FALSE);
  webViewBrowser->page()->settings()->setAttribute(QWebSettings::LocalStorageDatabaseEnabled, FALSE);

  // we want to detect/handle unsupported content
  webViewBrowser->page()->setForwardUnsupportedContent(TRUE);

  // status bar timeout connection
  connect(&statusTimer, SIGNAL(timeout()), this, SLOT(statusTimeout()));

  // "activate" the combo box on pressing return
  connect(comboBoxURL->lineEdit(), SIGNAL(returnPressed()), this, SLOT(on_comboBoxURL_activated()));
}

MiniWebBrowser::~MiniWebBrowser()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MiniWebBrowser::~MiniWebBrowser()");
#endif

}

void MiniWebBrowser::on_comboBoxURL_activated()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MiniWebBrowser::on_comboBoxURL_activated()");
#endif

  if ( !comboBoxURL->lineEdit()->text().isEmpty() ) {
    QString text = comboBoxURL->lineEdit()->text().toLower();
    QUrl url(text, QUrl::TolerantMode);
    if ( url.scheme().isEmpty() )
      if ( !text.startsWith("http://") )
        text.prepend("http://");
    comboBoxURL->setEditText(text);
    url = QUrl(text, QUrl::TolerantMode);
    webViewBrowser->load(url);
    int i = comboBoxURL->findText(text);
    if ( i >= 0 )
      comboBoxURL->setCurrentIndex(i);
  }

  QTimer::singleShot(0, toolButtonLoad, SLOT(animateClick()));
}

void MiniWebBrowser::on_toolButtonHome_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MiniWebBrowser::on_toolButtonHome_clicked()");
#endif

  if ( homeUrl.isValid() )
    webViewBrowser->load(homeUrl);
}

void MiniWebBrowser::on_toolButtonLoad_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MiniWebBrowser::on_toolButtonLoad_clicked()");
#endif

  if ( !comboBoxURL->lineEdit()->text().isEmpty() ) {
    QString text = comboBoxURL->lineEdit()->text().toLower();
    QUrl url(text, QUrl::TolerantMode);
    if ( url.scheme().isEmpty() )
      if ( !text.startsWith("http://") )
        text.prepend("http://");
    comboBoxURL->setEditText(text);
    webViewBrowser->load(QUrl(text, QUrl::TolerantMode));
  }
}

void MiniWebBrowser::webViewBrowser_linkClicked(const QUrl url)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MiniWebBrowser::webViewBrowser_linkClicked(const QUrl url = %1)").arg(url.toString()));
#endif

  if ( url.isValid() ) {
    QWebHitTestResult hitTest = webViewBrowser->page()->mainFrame()->hitTestContent(webViewBrowser->lastMouseClickPosition);
    if ( hitTest.linkTargetFrame() ) {
      hitTest.linkTargetFrame()->load(url);
    } else {
      webViewBrowser->load(url);
      webViewBrowser_urlChanged(url);
    }
  }
}

void MiniWebBrowser::webViewBrowser_urlChanged(const QUrl url)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MiniWebBrowser::webViewBrowser_urlChanged(const QUrl url = %1)").arg(url.toString()));
#endif

  comboBoxURL->setEditText(QString::fromUtf8(webViewBrowser->url().toEncoded()));

  QString newTitle = webViewBrowser->page()->mainFrame()->title();
  if ( newTitle.isEmpty() ) newTitle = tr("No title");
  emit titleChanged(newTitle);

  int i = comboBoxURL->findText(comboBoxURL->lineEdit()->text());
  if ( i < 0 ) {
    comboBoxURL->insertItem(0, comboBoxURL->lineEdit()->text());
  } else {
    QString itemText = comboBoxURL->itemText(i);
    QIcon itemIcon = comboBoxURL->itemIcon(i);
    comboBoxURL->removeItem(i);
    comboBoxURL->insertItem(0, itemIcon, itemText);
  }
  comboBoxURL->setCurrentIndex(0);

  QTimer::singleShot(0, this, SLOT(webViewBrowser_iconChanged()));
}

void MiniWebBrowser::webViewBrowser_loadStarted()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MiniWebBrowser::webViewBrowser_loadStarted()");
#endif

  progressBar->reset();
  progressBar->setRange(0, 100);
  progressBar->setValue(0);
  QFont f(font());
  f.setPointSize(f.pointSize() - 2);
  progressBar->setFont(f);
  progressBar->setMaximumHeight(f.pointSize() + 4);
  progressBar->show();

  if ( firstTimeLoadStarted ) {
    firstTimeLoadStarted = FALSE;
    homeUrl = webViewBrowser->url();
    webViewBrowser->history()->clear();
    toolButtonStop->setEnabled(TRUE);
    toolButtonReload->setEnabled(FALSE);
    toolButtonBack->setEnabled(FALSE);
    toolButtonForward->setEnabled(FALSE);
    toolButtonHome->setEnabled(TRUE);
  } else {
    toolButtonStop->setEnabled(TRUE);
    toolButtonReload->setEnabled(FALSE);
    toolButtonBack->setEnabled(webViewBrowser->history()->canGoBack());
    toolButtonForward->setEnabled(webViewBrowser->history()->canGoForward());
    toolButtonHome->setEnabled(TRUE);
  }

  QTimer::singleShot(0, this, SLOT(webViewBrowser_iconChanged()));
}

void MiniWebBrowser::webViewBrowser_loadProgress(int progress)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MiniWebBrowser::webViewBrowser_loadProgress(int progress = %1)").arg(progress));
#endif

  progressBar->setValue(progress);

  if ( firstTimeLoadProgress ) {
    firstTimeLoadProgress = FALSE;
    homeUrl = webViewBrowser->url();
    webViewBrowser->history()->clear();
    toolButtonBack->setEnabled(FALSE);
    toolButtonForward->setEnabled(FALSE);
    toolButtonHome->setEnabled(TRUE);
  } else {
    QTimer::singleShot(0, this, SLOT(webViewBrowser_iconChanged()));
    toolButtonBack->setEnabled(webViewBrowser->history()->canGoBack());
    toolButtonForward->setEnabled(webViewBrowser->history()->canGoForward());
  }
}

void MiniWebBrowser::webViewBrowser_loadFinished(bool ok)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MiniWebBrowser::webViewBrowser_loadFinished(bool ok = %1)").arg(ok));
#endif

  progressBar->reset();
  progressBar->hide();

  if ( firstTimeLoadFinished ) {
    firstTimeLoadFinished = FALSE;
    homeUrl = webViewBrowser->url();
    webViewBrowser->history()->clear();
    toolButtonBack->setEnabled(FALSE);
    toolButtonForward->setEnabled(FALSE);
  } else {
    toolButtonBack->setEnabled(webViewBrowser->history()->canGoBack());
    toolButtonForward->setEnabled(webViewBrowser->history()->canGoForward());
  }
  toolButtonStop->setEnabled(FALSE);
  toolButtonReload->setEnabled(TRUE);
  toolButtonHome->setEnabled(TRUE);

  QTimer::singleShot(250, this, SLOT(webViewBrowser_iconChanged()));
}

void MiniWebBrowser::webViewBrowser_statusBarMessage(const QString &message)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MiniWebBrowser::webViewBrowser_statusBarMessage(const QString &message = %1)").arg(message));
#endif

  if ( !message.isEmpty() ) {
    statusTimer.stop();
    labelStatus->setVisible(TRUE);
    QFont f(font());
    f.setPointSize(f.pointSize() - 2);
    labelStatus->setFont(f);
    labelStatus->setMaximumHeight(f.pointSize() + 4);
    labelStatus->setText(message + " ");
  } else {
    labelStatus->clear();
    statusTimer.start(QMC2_BROWSER_STATUS_TIMEOUT);
  }
}

void MiniWebBrowser::webViewBrowser_iconChanged()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MiniWebBrowser::webViewBrowser_iconChanged()");
#endif

  int i = comboBoxURL->findText(comboBoxURL->lineEdit()->text());

  if ( i >= 0 ) {
    QFontMetrics fm(qApp->font());
    QSize iconSize(fm.height() - 2, fm.height() - 2);
    comboBoxURL->setIconSize(iconSize);
    QIcon pageIcon;
    QString urlStr = webViewBrowser->url().toString();
    if ( iconCache.contains(urlStr) )
      pageIcon = *iconCache[urlStr];
    if ( pageIcon.isNull() ) {
      pageIcon = QWebSettings::iconForUrl(webViewBrowser->url());
      if ( pageIcon.isNull() )
        pageIcon = QIcon(QString::fromUtf8(":/data/img/browser.png"));
      else
        // for the "cache cost" we simply assume that icons take up 64x64 = 4096 bytes
        iconCache.insert(urlStr, new QIcon(pageIcon), 4096);
    }
    comboBoxURL->setItemIcon(i, pageIcon);
    comboBoxURL->setCurrentIndex(i);
  }
}

void MiniWebBrowser::webViewBrowser_linkHovered(const QString &link, const QString &title, const QString &textContent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MiniWebBrowser::webViewBrowser_linkHovered(const QString &link = %1, const QString &title = %2, const QString &textContent = %3)").arg(link).arg(title).arg(textContent));
#endif
  
  if ( !link.isEmpty() ) {
    statusTimer.stop();
    labelStatus->setVisible(TRUE);
    QFont f(font());
    f.setPointSize(f.pointSize() - 2);
    labelStatus->setFont(f);
    labelStatus->setMaximumHeight(f.pointSize() + 4);
    labelStatus->setText(link + " ");
  } else {
    labelStatus->clear();
    statusTimer.start(QMC2_BROWSER_STATUS_TIMEOUT);
  }
}

void MiniWebBrowser::webViewBrowser_statusBarVisibilityChangeRequested(bool visible)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MiniWebBrowser::webViewBrowser_statusBarVisibilityChangeRequested(bool visible = %1)").arg(visible));
#endif
  
  labelStatus->setVisible(visible);
}

void MiniWebBrowser::webViewBrowser_frameCreated(QWebFrame *frame)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MiniWebBrowser::webViewBrowser_frameCreated(QWebFrame *frame = %1)").arg((qulonglong)frame));
#endif

}

void MiniWebBrowser::statusTimeout()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MiniWebBrowser::statusTimeout()");
#endif

  statusTimer.stop();
  labelStatus->setVisible(FALSE);
}

void MiniWebBrowser::processPageActionDownloadRequested(const QNetworkRequest &request)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MiniWebBrowser::processPageActionDownloadRequested(const QNetworkRequest &request = ...)");
#endif

  qmc2MainWindow->startDownload(qmc2NetworkAccessManager->get(request));
}

void MiniWebBrowser::processPageActionHandleUnsupportedContent(QNetworkReply *reply)
{
#ifdef QMC2_DEBUG
  QMap <QNetworkAccessManager::Operation, QString> ops;
  ops[QNetworkAccessManager::HeadOperation] = "QNetworkAccessManager::HeadOperation";
  ops[QNetworkAccessManager::GetOperation] = "QNetworkAccessManager::GetOperation";
  ops[QNetworkAccessManager::PutOperation] = "QNetworkAccessManager::PutOperation";
  ops[QNetworkAccessManager::PostOperation] = "QNetworkAccessManager::PostOperation";
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MiniWebBrowser::processPageActionHandleUnsupportedContent(QNetworkReply *reply = %1 [operation() = %2, url() = %3, ...])")
                      .arg((qulonglong)reply).arg(ops[reply->operation()]).arg(reply->url().toString()));
#endif
  QMap <QNetworkAccessManager::Operation, QString> opsShort;
  opsShort[QNetworkAccessManager::HeadOperation] = "HEAD";
  opsShort[QNetworkAccessManager::GetOperation] = "GET";
  opsShort[QNetworkAccessManager::PutOperation] = "PUT";
  opsShort[QNetworkAccessManager::PostOperation] = "POST";

  if ( !reply || reply->url().isEmpty() )
    return;

  QVariant header = reply->header(QNetworkRequest::ContentLengthHeader);
  bool ok;
  int size = header.toInt(&ok);
  if ( ok && size == 0 )
    return;

  if ( MiniWebBrowser::supportedSchemes.contains(reply->url().scheme().toLower()) ) {
    switch ( reply->operation() ) {
      case QNetworkAccessManager::GetOperation:
        qmc2MainWindow->startDownload(reply);
        break;

      default:
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("FIXME: MiniWebBrowser::processPageActionHandleUnsupportedContent(): OP = %1, URL = %2")
                            .arg(opsShort[reply->operation()]).arg(reply->url().toString()));
        break;
    }
  } else {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("FIXME: MiniWebBrowser::processPageActionHandleUnsupportedContent(): OP = %1, URL = %2")
                        .arg(opsShort[reply->operation()]).arg(reply->url().toString()));
  }
}

