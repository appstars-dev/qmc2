#include <QApplication>
#include <QNetworkAccessManager>

#include "qmc2main.h"
#include "macros.h"
#include "downloaditem.h"

extern MainWindow *qmc2MainWindow;
extern QNetworkAccessManager *qmc2NetworkAccessManager;

DownloadItem::DownloadItem(QNetworkReply *reply, QString file, QTreeWidget *parent)
  : QTreeWidgetItem(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: DownloadItem::DownloadItem(QNetworkReply *reply = %1, QString file = %2, QTreeWidget *parent = %3)")
                      .arg((qulonglong)reply).arg(file).arg((qulonglong)parent));
#endif

  progressWidget = NULL;
  itemDownloader = NULL;
  treeWidget = parent;

  progressWidget = new QProgressBar(0);
  QFont f(qApp->font());
  f.setPointSize(f.pointSize() - 2);
  progressWidget->setFont(f);
  progressWidget->setFormat(reply->url().toString());
  treeWidget->setItemWidget(this, QMC2_DOWNLOAD_COLUMN_PROGRESS, progressWidget);
  setIcon(QMC2_DOWNLOAD_COLUMN_STATUS, QIcon(QString::fromUtf8(":/data/img/download.png")));
  setWhatsThis(0, "initializing");
  treeWidget->resizeColumnToContents(QMC2_DOWNLOAD_COLUMN_STATUS);
  progressWidget->reset();
  progressWidget->setRange(-1, -1);
  progressWidget->setValue(-1);
  progressWidget->show();

  itemDownloader = new ItemDownloader(reply, file, progressWidget, this);
}

DownloadItem::~DownloadItem()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DownloadItem::~DownloadItem()");
#endif

  if ( progressWidget )
    delete progressWidget;

  if ( itemDownloader )
    delete itemDownloader;
}

ItemDownloader::ItemDownloader(QNetworkReply *reply, QString file, QProgressBar *progress, DownloadItem *parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ItemDownloader::ItemDownloader(QNetworkReply *reply = %1, QString file = %2, QProgressBar *progress = %3, DownloadItem *parent = %4)")
                      .arg((qulonglong)reply).arg(file).arg((qulonglong)progress).arg((qulonglong)parent));
#endif

  networkReply = reply;
  localPath = file;
  progressWidget = progress;
  downloadItem = parent;
  retryCount = 0;

  init();
}

ItemDownloader::~ItemDownloader()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ItemDownloader::~ItemDownloader()");
#endif

}

void ItemDownloader::init()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ItemDownloader::init(): networkReply = %1").arg((qulonglong)networkReply));
#endif

  if ( !networkReply )
    return;

  if ( localFile.isOpen() )
    localFile.close();

  localFile.setFileName(localPath);

  if ( localFile.exists() )
    localFile.remove();

  if ( !localFile.open(QIODevice::WriteOnly) ) {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open '%1' for writing").arg(localPath));
    return;
  }

  dataReceived = 0;
  retryCount++;
  progressWidget->reset();
  progressWidget->setRange(-1, -1);
  progressWidget->setValue(-1);
  progressWidget->setFormat(networkReply->url().toString());
  downloadItem->setIcon(QMC2_DOWNLOAD_COLUMN_STATUS, QIcon(QString::fromUtf8(":/data/img/download.png")));
  downloadItem->progressWidget->setEnabled(TRUE);
  downloadItem->setWhatsThis(0, "downloading");
  downloadItem->setToolTip(QMC2_DOWNLOAD_COLUMN_PROGRESS,
                           tr("Source URL: %1").arg(networkReply->url().toString()) + "\n" +
                           tr("Local path: %2").arg(localPath) + "\n" +
                           tr("Status: %1").arg(tr("initializing download")) + "\n" +
                           tr("Total size: %1").arg(tr("unknown")) + "\n" +
                           tr("Downloaded: %1 (%2%)").arg(0).arg(0));
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("download started: URL = %1, local path = %2, reply ID = %3")
                      .arg(networkReply->url().toString()).arg(localPath).arg((qulonglong)networkReply));

  connect(networkReply, SIGNAL(readyRead()), this, SLOT(readyRead()));
  connect(networkReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(error(QNetworkReply::NetworkError)));
  connect(networkReply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgress(qint64, qint64)));
  connect(networkReply, SIGNAL(metaDataChanged()), this, SLOT(metaDataChanged()));
  connect(networkReply, SIGNAL(finished()), this, SLOT(finished()));
  connect(qmc2NetworkAccessManager, SIGNAL(finished(QNetworkReply *)), this, SLOT(managerFinished(QNetworkReply *)));

  connect(&errorCheckTimer, SIGNAL(timeout()), this, SLOT(checkError()));
  errorCheckTimer.start(QMC2_DOWNLOAD_CHECK_TIMEOUT);
}

void ItemDownloader::checkError()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ItemDownloader::checkError(): networkReply = %1").arg((qulonglong)networkReply));
#endif

  if ( networkReply->error() != QNetworkReply::NoError ) {
    error(networkReply->error());
    finished();
  }

  if ( dataReceived == 0 ) {
    if ( retryCount < QMC2_DOWNLOAD_OPCANCEL_RETRY ) {
      QTimer::singleShot((qrand() % 6 + 5) * QMC2_DOWNLOAD_RETRY_DELAY, this, SLOT(reload()));
    } else {
      error(QNetworkReply::TimeoutError);
      finished();
    }
  }
}

void ItemDownloader::readyRead()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ItemDownloader::readyRead(): networkReply = %1").arg((qulonglong)networkReply));
#endif

  QByteArray buffer = networkReply->readAll();
  dataReceived += buffer.length();
  retryCount = 0;

  if ( localFile.isOpen() )
    localFile.write(buffer);
}

void ItemDownloader::error(QNetworkReply::NetworkError code)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ItemDownloader::error(QNetworkReply::NetworkError code = %1): networkReply = %2").arg(code).arg((qulonglong)networkReply));
#endif

  if ( code == QNetworkReply::OperationCanceledError ) {
    downloadItem->setIcon(QMC2_DOWNLOAD_COLUMN_STATUS, QIcon(QString::fromUtf8(":/data/img/stop_browser.png")));
  } else {
    progressWidget->setFormat(tr("Error #%1: ").arg(code) + networkReply->url().toString());
    downloadItem->setIcon(QMC2_DOWNLOAD_COLUMN_STATUS, QIcon(QString::fromUtf8(":/data/img/warning.png")));
  }
  downloadItem->treeWidget->resizeColumnToContents(QMC2_DOWNLOAD_COLUMN_STATUS);
  progressWidget->setEnabled(FALSE);

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("download aborted: reason = %1, URL = %2, local path = %3, reply ID = %4")
                      .arg(networkReply->errorString()).arg(networkReply->url().toString()).arg(localPath).arg((qulonglong)networkReply));
  downloadItem->setWhatsThis(0, "aborted");
  errorCheckTimer.stop();

  if ( localFile.isOpen() )
    localFile.close();

  downloadItem->setToolTip(QMC2_DOWNLOAD_COLUMN_PROGRESS,
                           tr("Source URL: %1").arg(networkReply->url().toString()) + "\n" +
                           tr("Local path: %2").arg(localPath) + "\n" +
                           tr("Status: %1").arg(tr("download aborted")) + "\n" +
                           tr("Total size: %1").arg(progressWidget->maximum()) + "\n" +
                           tr("Downloaded: %1 (%2%)").arg(progressWidget->value()).arg(((double)progressWidget->value()/(double)progressWidget->maximum()) * 100));
}

void ItemDownloader::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ItemDownloader::downloadProgress(qint64 bytesReceived = %1, qint64 bytesTotal = %2): networkReply = %3").arg(bytesReceived).arg(bytesTotal).arg((qulonglong)networkReply));
#endif

  progressWidget->setRange(0, bytesTotal); // FIXME: may overflow?
  progressWidget->setValue(bytesReceived);

  downloadItem->setToolTip(QMC2_DOWNLOAD_COLUMN_PROGRESS,
                           tr("Source URL: %1").arg(networkReply->url().toString()) + "\n" +
                           tr("Local path: %2").arg(localPath) + "\n" +
                           tr("Status: %1").arg(tr("downloading")) + "\n" +
                           tr("Total size: %1").arg(progressWidget->maximum()) + "\n" +
                           tr("Downloaded: %1 (%2%)").arg(progressWidget->value()).arg(((double)progressWidget->value()/(double)progressWidget->maximum()) * 100));
}

void ItemDownloader::metaDataChanged()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ItemDownloader::metaDataChanged(): networkReply = %1").arg((qulonglong)networkReply));
#endif

}

void ItemDownloader::finished()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ItemDownloader::finished(): networkReply = %1").arg((qulonglong)networkReply));
#endif

  errorCheckTimer.stop();

  if ( downloadItem->whatsThis(0) != "aborted" && downloadItem->whatsThis(0) != "finished" ) {
    downloadItem->setWhatsThis(0, "finished");
    progressWidget->setValue(progressWidget->maximum());
    downloadItem->setIcon(QMC2_DOWNLOAD_COLUMN_STATUS, QIcon(QString::fromUtf8(":/data/img/ok.png")));
    downloadItem->treeWidget->resizeColumnToContents(QMC2_DOWNLOAD_COLUMN_STATUS);
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("download finished: URL = %1, local path = %2, reply ID = %3")
                        .arg(networkReply->url().toString()).arg(localPath).arg((qulonglong)networkReply));
    downloadItem->setToolTip(QMC2_DOWNLOAD_COLUMN_PROGRESS,
                             tr("Source URL: %1").arg(networkReply->url().toString()) + "\n" +
                             tr("Local path: %2").arg(localPath) + "\n" +
                             tr("Status: %1").arg(tr("download finished")) + "\n" +
                             tr("Total size: %1").arg(progressWidget->maximum()) + "\n" +
                             tr("Downloaded: %1 (%2%)").arg(progressWidget->value()).arg(((double)progressWidget->value()/(double)progressWidget->maximum()) * 100));
  }
  progressWidget->setEnabled(FALSE);

  // close connection and file
  networkReply->close();
  if ( localFile.isOpen() )
    localFile.close();
}

void ItemDownloader::managerFinished(QNetworkReply *reply)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ItemDownloader::managerFinished(QNetworkReply *reply = %1): networkReply = %2").arg((qulonglong)reply).arg((qulonglong)networkReply)));
#endif

  if ( reply == networkReply )
    finished();
}

void ItemDownloader::reload()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ItemDownloader::reload(): networkReply = %1").arg((qulonglong)networkReply));
#endif

  networkReply->close();
  errorCheckTimer.stop();
  networkReply = qmc2NetworkAccessManager->get(QNetworkRequest(networkReply->url()));
  init();
}

