#include <QtGui>
#if defined(Q_WS_X11)
#include "embedder.h"
#include "qmc2main.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;

Embedder::Embedder(quint64 wid, QWidget *parent)
    : QWidget(parent)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::Embedder(quint64 wid = %1, QWidget *parent = %2)").arg((qulonglong)wid).arg((qulonglong)parent));
#endif

  gridLayout = new QGridLayout(this);
  setLayout(gridLayout);

  winId = wid;
  embedded = false;

  embedContainer = new QX11EmbedContainer(this);
  gridLayout->addWidget(embedContainer, 1, 0);

  connect(embedContainer, SIGNAL(clientIsEmbedded()), SLOT(clientEmbedded()));
  connect(embedContainer, SIGNAL(clientClosed()), SLOT(clientClosed()));
  connect(embedContainer, SIGNAL(error(QX11EmbedContainer::Error)), SLOT(clientError(QX11EmbedContainer::Error)));

  embedderOptions = NULL;
  optionsShown = FALSE;

  QTimer::singleShot(0, this, SLOT(embed()));
}

void Embedder::embed()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::embed()"));
#endif

  embedContainer->embedClient(winId);
}

void Embedder::release()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::release()"));
#endif

  embedContainer->clearFocus();
  embedContainer->discardClient();
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("emulator released, window ID = 0x%1").arg(QString::number(winId, 16)));
  embedded = false;
}

void Embedder::clientEmbedded()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::clientEmbedded()"));
#endif

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("emulator embedded, window ID = 0x%1").arg(QString::number(winId, 16)));
  // effectively transfer focus to the emulator
  embedContainer->setFocus();
  embedded = true;
}

void Embedder::clientClosed()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::clientClosed()"));
#endif

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("emulator closed, window ID = 0x%1").arg(QString::number(winId, 16)));
  embedded = false;

  emit closing();
}

void Embedder::clientError(QX11EmbedContainer::Error error)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::clientError(QX11EmbedContainer::Error error = %1)").arg((int)error));
#endif

  switch ( error ) {
    case QX11EmbedContainer::Unknown:
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: embedder: unknown error, window ID = 0x%1").arg(QString::number(winId, 16)));
      break;

    case QX11EmbedContainer::InvalidWindowID:
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: embedder: invalid window ID = 0x%1").arg(QString::number(winId, 16)));
      break;

    default:
      break;
  }
}

void Embedder::closeEvent(QCloseEvent *e)
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::closeEvent(QCloseEvent *e = %1)").arg((qulonglong)e));
#endif

  release();

  if ( embedderOptions )
    delete embedderOptions;

  QWidget::closeEvent(e);
}

void Embedder::toggleOptions()
{
#ifdef QMC2_DEBUG
  log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::toggleOptions()"));
#endif

  optionsShown = !optionsShown;
  if ( optionsShown ) {
    if ( !embedderOptions ) {
      embedderOptions = new EmbedderOptions(this);
      gridLayout->addWidget(embedderOptions, 0, 0);
    }
    embedderOptions->show();
  } else {
    if ( embedderOptions )
      embedderOptions->hide();
  }
}

#endif
