#if defined(QMC2_YOUTUBE_ENABLED)

#include <QtTest>
#include <QSettings>

#include "youtubevideoplayer.h"
#include "macros.h"
#include "qmc2main.h"

#define QMC2_DEBUG

extern MainWindow *qmc2MainWindow;
extern QNetworkAccessManager *qmc2NetworkAccessManager;
extern QSettings *qmc2Config;

YouTubeVideoPlayer::YouTubeVideoPlayer(QWidget *parent)
	: QWidget(parent)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::YouTubeVideoPlayer(QWidget *parent = %1)").arg((qulonglong) parent));
#endif

	setupUi(this);

	videoFinished();
	comboBoxPreferredFormat->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "YouTubeWidget/PreferredFormat", YOUTUBE_FORMAT_MP4_1080P_INDEX).toInt());
	videoPlayer->audioOutput()->setVolume(qmc2Config->value(QMC2_FRONTEND_PREFIX + "YouTubeWidget/AudioVolume", 0.5).toDouble());

	youTubeFormats << YOUTUBE_FORMAT_FLV_240P
		<< YOUTUBE_FORMAT_FLV_360P
		<< YOUTUBE_FORMAT_MP4_360P
		<< YOUTUBE_FORMAT_FLV_480P
		<< YOUTUBE_FORMAT_MP4_720P
		<< YOUTUBE_FORMAT_MP4_1080P
		<< YOUTUBE_FORMAT_MP4_3072P;

	videoInfoReply = NULL;
	videoInfoManager = NULL;
	videoPlayer->mediaObject()->setTickInterval(1000);
	volumeSlider->setAudioOutput(videoPlayer->audioOutput());
	connect(videoPlayer->mediaObject(), SIGNAL(tick(qint64)), this, SLOT(videoTick(qint64)));
	connect(videoPlayer->mediaObject(), SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(videoStateChanged(Phonon::State, Phonon::State)));
	connect(videoPlayer, SIGNAL(finished()), this, SLOT(videoFinished()));

	adjustIconSizes();

	QTimer::singleShot(100, this, SLOT(init()));
}

YouTubeVideoPlayer::~YouTubeVideoPlayer()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::~YouTubeVideoPlayer()");
#endif

  	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "YouTubeWidget/PreferredFormat", comboBoxPreferredFormat->currentIndex());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "YouTubeWidget/AudioVolume", videoPlayer->audioOutput()->volume());

	if ( videoInfoReply ) {
		disconnect(videoInfoReply);
		delete videoInfoReply;
	}
	if ( videoInfoManager ) {
		disconnect(videoInfoManager);
		delete videoInfoManager;
	}
}

void YouTubeVideoPlayer::init()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::init()");
#endif

	//QString video = "bcwBowBFFzc";
	//QString video = "vK9rfCpjOQc";
	QString video = "gO-OwcBCa8Y";
	playVideo(video);
}

void YouTubeVideoPlayer::adjustIconSizes()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::adjustIconSizes()"));
#endif

	QFont f;
	f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	QFontMetrics fm(f);
	QSize iconSize = QSize(fm.height() - 2, fm.height() - 2);
	comboBoxPreferredFormat->setIconSize(iconSize);
	toolButtonPlayPause->setIconSize(iconSize);
	volumeSlider->setIconSize(iconSize);
	toolBox->setItemIcon(YOUTUBE_ATTACHED_VIDEOS_PAGE, QIcon(QPixmap(QString::fromUtf8(":/data/img/movie.png")).scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
	toolBox->setItemIcon(YOUTUBE_VIDEO_PLAYER_PAGE, QIcon(QPixmap(QString::fromUtf8(":/data/img/youtube.png")).scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
	toolBox->setItemIcon(YOUTUBE_SEARCH_VIDEO_PAGE, QIcon(QPixmap(QString::fromUtf8(":/data/img/pacman.png")).scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
}

void YouTubeVideoPlayer::videoFinished()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::videoFinished()");
#endif

	labelPlayingTime->setText("--:--:--");
	toolButtonPlayPause->setIcon(QIcon(QString::fromUtf8(":/data/img/media_stop.png")));
}

void YouTubeVideoPlayer::videoTick(qint64 time)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::videoTick(quint64 time = %1)").arg(time));
#endif

	QTime hrTime;
	hrTime = hrTime.addMSecs(videoPlayer->mediaObject()->remainingTime());
	labelPlayingTime->setText(hrTime.toString("hh:mm:ss"));
}

void YouTubeVideoPlayer::videoStateChanged(Phonon::State newState, Phonon::State oldState)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::videoStateChanged(Phonon::State newState = %1, Phonon::State oldState = %2)").arg(newState).arg(oldState));
#endif

	switch ( newState ) {
		case Phonon::LoadingState:
		case Phonon::PlayingState:
		case Phonon::BufferingState:
			toolButtonPlayPause->setIcon(QIcon(QString::fromUtf8(":/data/img/media_play.png")));
			break;
		case Phonon::PausedState:
			toolButtonPlayPause->setIcon(QIcon(QString::fromUtf8(":/data/img/media_pause.png")));
			break;
		case Phonon::ErrorState:
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("video player: playback error: %1").arg(videoPlayer->mediaObject()->errorString()));
		case Phonon::StoppedState:
		default:
			toolButtonPlayPause->setIcon(QIcon(QString::fromUtf8(":/data/img/media_stop.png")));
			labelPlayingTime->setText("--:--:--");
			break;
	}
}

void YouTubeVideoPlayer::playVideo(QString &videoID)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::playVideo(QString videoID = %1)").arg(videoID));
#endif

	currentVideoID = videoID;
	QUrl url = getVideoStreamUrl(videoID);
	printf("\nUsing this URL:\n%s\n", (const char *)url.toString().toLatin1());
	if ( url.isValid() ) {
		videoPlayer->play(Phonon::MediaSource(QUrl::fromEncoded((const char *)url.toString().toLatin1())));
	}
}

QUrl YouTubeVideoPlayer::getVideoStreamUrl(QString videoID)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::getVideoStreamUrl(QString videoID = %1)").arg(videoID));
#endif

	static QUrl videoUrl;
	static QString localIpRange;

	if ( videoInfoReply ) {
		disconnect(videoInfoReply);
		delete videoInfoReply;
		videoInfoReply = NULL;
	}
	videoInfoBuffer.clear();
	viError = viFinished = false;
	videoInfoRequest.setUrl(QString("http://www.youtube.com/get_video_info?&video_id=%1").arg(videoID));
	videoInfoRequest.setRawHeader("User-Agent", "QMC2's YouTube Player");
	if ( videoInfoManager ) {
		disconnect(videoInfoManager);
		delete videoInfoManager;
		videoInfoManager = NULL;
	}
	videoInfoManager = new QNetworkAccessManager(this);
	videoInfoReply = videoInfoManager->get(videoInfoRequest);
	connect(videoInfoReply, SIGNAL(readyRead()), this, SLOT(videoInfoReadyRead()));
	connect(videoInfoReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(videoInfoError(QNetworkReply::NetworkError)));
	connect(videoInfoReply, SIGNAL(finished()), this, SLOT(videoInfoFinished()));

	while ( !viFinished && !viError ) QTest::qWait(10);

	if ( viFinished && !viError ) {
		QStringList videoInfoList = videoInfoBuffer.split("&");

		// FIXME: debug
		printf("\n");

		QMap <QString, QUrl> formatToUrlMap;
		foreach (QString videoInfo, videoInfoList) {
			if ( videoInfo.startsWith("fmt_url_map=") ) {
				QStringList fmtUrlMap = videoInfo.replace("fmt_url_map=", "").split("%2C");
				foreach (QString fmtUrl, fmtUrlMap) {
					QStringList formatAndUrl = fmtUrl.split("%7C");
					if ( formatAndUrl.count() > 1 ) {
						QUrl url = QUrl::fromEncoded(formatAndUrl[1].toAscii());
						QString urlStr = url.toString();
						url.setEncodedUrl(urlStr.toAscii());
						/*
						if ( url.queryItemValue("ip") != "0.0.0.0" )
							localIpRange = url.queryItemValue("ip");
						else if ( !localIpRange.isEmpty() ) {
							QList<QPair<QString, QString> > qItems = url.queryItems();
							for (int i = 0; i < qItems.count(); i++) {
								if ( qItems[i].first == "ip" ) {
									qItems[i].second = localIpRange;
									break;
								}
							}
							url.setQueryItems(qItems);
						}
						*/

						// FIXME: debug
						printf("%s\t%s\n", (const char *)formatAndUrl[0].toLatin1(), (const char *)url.toString().toLatin1());

						formatToUrlMap[formatAndUrl[0]] = url;
					}
				}
			}
		}
		for (int i = 0; i < comboBoxPreferredFormat->count(); i++) {
			if ( formatToUrlMap.contains(indexToFormat(i)) )
				comboBoxPreferredFormat->setItemIcon(i, QIcon(QString::fromUtf8(":/data/img/trafficlight_green.png")));
			else
				comboBoxPreferredFormat->setItemIcon(i, QIcon(QString::fromUtf8(":/data/img/trafficlight_off.png")));
		}
		for (int i = comboBoxPreferredFormat->currentIndex(); i >= 0; i--) {
			if ( formatToUrlMap.contains(indexToFormat(i)) ) {
				videoUrl = formatToUrlMap[indexToFormat(i)];
				break;
			}
		}
		return videoUrl;
	} else if ( viError ) {
		return QString();
	}
}

QString YouTubeVideoPlayer::indexToFormat(int index)
{
	switch ( index ) {
		case YOUTUBE_FORMAT_MP4_3072P_INDEX: return YOUTUBE_FORMAT_MP4_3072P;
		case YOUTUBE_FORMAT_MP4_1080P_INDEX: return YOUTUBE_FORMAT_MP4_1080P;
		case YOUTUBE_FORMAT_MP4_720P_INDEX: return YOUTUBE_FORMAT_MP4_720P;
		case YOUTUBE_FORMAT_MP4_360P_INDEX: return YOUTUBE_FORMAT_MP4_360P;
		case YOUTUBE_FORMAT_FLV_480P_INDEX: return YOUTUBE_FORMAT_FLV_480P;
		case YOUTUBE_FORMAT_FLV_360P_INDEX: return YOUTUBE_FORMAT_FLV_360P;
		case YOUTUBE_FORMAT_FLV_240P_INDEX: default: return YOUTUBE_FORMAT_FLV_240P;
	}
}

void YouTubeVideoPlayer::videoInfoReadyRead()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::videoInfoReadyRead()");
#endif

	videoInfoBuffer += videoInfoReply->readAll();
}

void YouTubeVideoPlayer::videoInfoError(QNetworkReply::NetworkError error)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::videoInfoError(QNetworkReply::NetworkError error = %1)").arg(error));
#endif

	viError = true;
}

void YouTubeVideoPlayer::videoInfoFinished()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::videoInfoFinished()");
#endif

	viFinished = true;
}

void YouTubeVideoPlayer::on_toolButtonPlayPause_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::on_toolButtonPlayPause_clicked()");
#endif

	if ( videoPlayer->isPlaying() )
		videoPlayer->pause();
	else if ( videoPlayer->isPaused() )
		videoPlayer->play();
	else if ( !currentVideoID.isEmpty() )
		playVideo(currentVideoID);
}

void YouTubeVideoPlayer::on_comboBoxPreferredFormat_currentIndexChanged(int index)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::on_comboBoxPreferredFormat_currentIndexChanged(int index = %1)").arg(index));
#endif

	if ( !currentVideoID.isEmpty() )
		playVideo(currentVideoID);
}

void YouTubeVideoPlayer::showEvent(QShowEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::showEvent(QShowEvent *e = %1)").arg((qulonglong)e));
#endif

	if ( videoPlayer->isPaused() )
		videoPlayer->play();
}

void YouTubeVideoPlayer::hideEvent(QHideEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::hideEvent(QHideEvent *e = %1)").arg((qulonglong)e));
#endif

	if ( videoPlayer->isPlaying() )
		videoPlayer->pause();
}
#endif
