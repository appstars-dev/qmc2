#include <QtGui>

#if defined(Q_WS_X11)

#include <QX11Info>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include "embedder.h"
#include "qmc2main.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;
extern QSettings *qmc2Config;

Embedder::Embedder(QString name, QString id, WId wid, bool currentlyPaused, QWidget *parent)
    : QWidget(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::Embedder(QString name = %1, QString id = %2, WId wid = %3, bool currentlyPaused = %4, QWidget *parent = %5)").arg(name).arg(id).arg((qulonglong)wid).arg(currentlyPaused).arg((qulonglong)parent));
#endif

  gameName = name;
  gameID = id;
  winId = wid;
  embedded = false;
  pauseKeyPressed = false;
  isPaused = currentlyPaused;

#if QT_VERSION >= 0x040700
  setAttribute(Qt::WA_NativeWindow);
  setAttribute(Qt::WA_DontCreateNativeAncestors);
  createWinId();
#endif

  embedContainer = new QX11EmbedContainer(this);
  embedContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  embedContainer->setObjectName("QMC2_EMBED_CONTAINER");
  embedContainer->setAutoFillBackground(true);
  QPalette pal = embedContainer->palette();
  pal.setColor(QPalette::Window, Qt::black);
  embedContainer->setPalette(pal);

  setFocusProxy(embedContainer);

  gridLayout = new QGridLayout(this);
  gridLayout->getContentsMargins(&cmLeft, &cmTop, &cmRight, &cmBottom);
  gridLayout->setContentsMargins(0, 0, 0, 0);
  gridLayout->setRowStretch(0, 0);
  gridLayout->setRowStretch(1, 4);
  gridLayout->setColumnStretch(0, 4);
  gridLayout->addWidget(embedContainer, 1, 0);
  setLayout(gridLayout);

  connect(embedContainer, SIGNAL(clientIsEmbedded()), SLOT(clientEmbedded()));
  connect(embedContainer, SIGNAL(clientClosed()), SLOT(clientClosed()));
  connect(embedContainer, SIGNAL(error(QX11EmbedContainer::Error)), SLOT(clientError(QX11EmbedContainer::Error)));

  embedderOptions = NULL;
  optionsShown = false;

  QTimer::singleShot(0, this, SLOT(embed()));
}

Embedder::~Embedder()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::~Embedder()"));
#endif

}

void Embedder::embed()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::embed()"));
#endif

  nativeResolution = QPixmap::grabWindow(winId).size();
  embedContainer->embedClient(winId);
}

void Embedder::release()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::release()"));
#endif

  embedded = false;
  embedContainer->clearFocus();
  embedContainer->discardClient();
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("emulator released, window ID = 0x%1").arg(QString::number(winId, 16)));
}

void Embedder::clientEmbedded()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::clientEmbedded()"));
#endif

  embedded = true;

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("emulator embedded, window ID = 0x%1").arg(QString::number(winId, 16)));

  // this works around a Qt bug when the tool bar is vertical and obscured by the emulator window before embedding
  QTimer::singleShot(0, qmc2MainWindow->toolbar, SLOT(update()));

  // gain focus
  QTimer::singleShot(QMC2_EMBED_FOCUS_DELAY, this, SLOT(forceFocus()));
}

void Embedder::clientClosed()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::clientClosed()"));
#endif

  if ( embedded ) 
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("emulator closed, window ID = 0x%1").arg(QString::number(winId, 16)));
  embedded = false;
  emit closing();
}

void Embedder::clientError(QX11EmbedContainer::Error error)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::clientError(QX11EmbedContainer::Error error = %1)").arg((int)error));
#endif

  switch ( error ) {
    case QX11EmbedContainer::InvalidWindowID:
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: embedder: invalid window ID = 0x%1").arg(QString::number(winId, 16)));
      break;

    case QX11EmbedContainer::Unknown:
    default:
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: embedder: unknown error, window ID = 0x%1").arg(QString::number(winId, 16)));
      break;
  }
  emit closing();
}

void Embedder::closeEvent(QCloseEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::closeEvent(QCloseEvent *e = %1)").arg((qulonglong)e));
#endif

  if ( embedded )
    release();

  if ( embedderOptions )
    delete embedderOptions;

  QWidget::closeEvent(e);
}

void Embedder::showEvent(QShowEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::showEvent(QShowEvent *e = %1)").arg((qulonglong)e));
#endif

  embedContainer->hide();

  QTimer::singleShot(QMC2_EMBED_MAXIMIZE_DELAY, embedContainer, SLOT(showMaximized()));

  if ( qmc2MainWindow->toolButtonEmbedderMaximizeToggle->isChecked() )
    QTimer::singleShot(0, qmc2MainWindow->menuBar(), SLOT(hide()));

  if ( embedded && qmc2MainWindow->toolButtonEmbedderAutoPause->isChecked() )
    QTimer::singleShot(QMC2_EMBED_PAUSERESUME_DELAY, this, SLOT(showEventDelayed()));

  // gain focus
  QTimer::singleShot(QMC2_EMBED_FOCUS_DELAY, this, SLOT(forceFocus()));
}

void Embedder::hideEvent(QHideEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::showEvent(QShowEvent *e = %1)").arg((qulonglong)e));
#endif

  if ( embedded && qmc2MainWindow->toolButtonEmbedderAutoPause->isChecked() )
    QTimer::singleShot(QMC2_EMBED_PAUSERESUME_DELAY, this, SLOT(hideEventDelayed()));
}

void Embedder::showEventDelayed()
{
	qApp->processEvents();

	if ( isVisible() )
		resume();
}

void Embedder::hideEventDelayed()
{
	qApp->processEvents();

	if ( !isVisible() )
		pause();
}

void Embedder::toggleOptions()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::toggleOptions()"));
#endif

  optionsShown = !optionsShown;
  if ( optionsShown ) {
    gridLayout->setRowStretch(0, 1);
    gridLayout->setRowStretch(1, 4);
    gridLayout->setContentsMargins(cmLeft, cmTop, cmRight, cmBottom);
    if ( !embedderOptions ) {
      embedderOptions = new EmbedderOptions(this);
      embedderOptions->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
      gridLayout->addWidget(embedderOptions, 0, 0);
    }
    embedderOptions->show();
  } else {
    gridLayout->setRowStretch(0, 0);
    gridLayout->setRowStretch(1, 4);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    if ( embedderOptions )
      embedderOptions->hide();
  }
}

void Embedder::adjustIconSizes()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::adjustIconSizes()"));
#endif

  // serious hack to access the tab bar without sub-classing from QTabWidget ;)
  QTabBar *tabBar = qmc2MainWindow->tabWidgetEmbeddedEmulators->findChild<QTabBar *>();
  int index = qmc2MainWindow->tabWidgetEmbeddedEmulators->indexOf(this);

  QFont f;
  f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
  QFontMetrics fm(f);
  QToolButton *optionsButton = (QToolButton *)tabBar->tabButton(index, QTabBar::LeftSide);
  QToolButton *releaseButton = (QToolButton *)tabBar->tabButton(index, QTabBar::RightSide);
  int baseSize = fm.height() + 2;
  QSize optionsButtonSize(2 * baseSize, baseSize);
  QSize releaseButtonSize(baseSize, baseSize);
  optionsButton->setFixedSize(optionsButtonSize);
  releaseButton->setFixedSize(releaseButtonSize);
}

void Embedder::forceFocus()
{
	if ( embedded ) {
		XSetInputFocus(QX11Info::display(), winId, RevertToParent, QDateTime::currentDateTime().toTime_t());
	} else {
		activateWindow();
		setFocus();
	}
}

void Embedder::pause()
{
	if ( !isPaused )
		simulatePauseKey();
}

void Embedder::resume()
{
	if ( isPaused )
		simulatePauseKey();
}

void Embedder::simulatePauseKey()
{
	XKeyEvent xev;
	xev.display = QX11Info::display();
	xev.window = winId;
	xev.root = qApp->desktop()->winId();
	xev.subwindow = 0;
	xev.time = QDateTime::currentDateTime().toTime_t();
	xev.x = xev.y = xev.x_root = xev.y_root = 1;
	xev.same_screen = true;
	xev.keycode = XKeysymToKeycode(xev.display, qmc2Config->value(QMC2_FRONTEND_PREFIX + "Embedder/PauseKey", Qt::Key_P).toInt());
	xev.state = 0;

	if ( pauseKeyPressed ) {
		xev.type = KeyRelease;
		XSendEvent(xev.display, xev.window, true, KeyPressMask, (XEvent *)&xev);
		pauseKeyPressed = false;
	} else {
		xev.type = KeyPress;
		XSendEvent(xev.display, xev.window, true, KeyPressMask, (XEvent *)&xev);
		pauseKeyPressed = true;
		QTimer::singleShot(QMC2_XKEYEVENT_TRANSITION_TIME, this, SLOT(simulatePauseKey()));
	}
}

#endif
