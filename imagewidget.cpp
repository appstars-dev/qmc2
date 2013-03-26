#include <QPixmap>
#include <QImage>
#include <QImageReader>
#include <QDir>
#include <QByteArray>
#include <QBuffer>
#include <QMap>
#include <QClipboard>
#include <QCache>
#include <QTreeWidgetItem>

#include "imagewidget.h"
#include "qmc2main.h"
#include "macros.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern bool qmc2SmoothScaling;
extern bool qmc2RetryLoadingImages;
extern bool qmc2ParentImageFallback;
extern bool qmc2ShowGameName;
extern bool qmc2ShowGameNameOnlyWhenRequired;
extern QTreeWidgetItem *qmc2CurrentItem;
extern QMap<QString, QString> qmc2ParentMap;
extern QMap<QString, QString> qmc2GamelistDescriptionMap;
extern QCache<QString, ImagePixmap> qmc2ImagePixmapCache;

ImageWidget::ImageWidget(QWidget *parent)
#if QMC2_OPENGL == 1
	: QGLWidget(parent)
#else
	: QWidget(parent)
#endif
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ImageWidget::ImageWidget(QWidget *parent = %1)").arg((qulonglong)parent));
#endif

	contextMenu = new QMenu(this);
	contextMenu->hide();

	QString s;
	QAction *action;

	s = tr("Copy image to clipboard");
	action = contextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/editcopy.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(copyToClipboard()));

	s = tr("Copy file path to clipboard");
	action = contextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/editcopy.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(copyPathToClipboard()));
	actionCopyPathToClipboard = action;

	contextMenu->addSeparator();

	s = tr("Refresh cache slot");
	action = contextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/reload.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(refresh()));

	if ( useZip() ) {
		foreach (QString filePath, imageZip().split(";", QString::SkipEmptyParts)) {
			unzFile imageFile = unzOpen((const char *)filePath.toLocal8Bit());
			if ( imageFile == NULL )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open %1 file, please check access permissions for %2").arg(imageType()).arg(imageZip()));
			else
				imageFileMap[filePath] = imageFile;
		}
	}
}

ImageWidget::~ImageWidget()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageWidget::~ImageWidget()");
#endif

	if ( useZip() ) {
		foreach (unzFile imageFile, imageFileMap)
			unzClose(imageFile);
	}
}

QString ImageWidget::cleanDir(QString dirs)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ImageWidget::cleanDir(QString dirs = %1)").arg(dirs));
#endif

	QStringList dirList;
	foreach (QString dir, dirs.split(";", QString::SkipEmptyParts)) {
		if ( !dir.endsWith("/") )
			dir += "/";
		dirList << dir;
	}
	return dirList.join(";");
}

void ImageWidget::paintEvent(QPaintEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ImageWidget::paintEvent(QPaintEvent *e = %1)").arg((qulonglong)e));
#endif

	QPainter p(this);

	if ( !qmc2CurrentItem ) {
		drawCenteredImage(0, &p); // clear image widget
		return;
	}

	if ( qmc2CurrentItem->text(QMC2_GAMELIST_COLUMN_GAME) == tr("Waiting for data...") ) {
		drawCenteredImage(0, &p); // clear image widget
		return;
	}

	QTreeWidgetItem *topLevelItem = qmc2CurrentItem;
	while ( topLevelItem->parent() )
		topLevelItem = topLevelItem->parent();

	QString gameName = topLevelItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON);
	cacheKey = cachePrefix() + gameName;
	ImagePixmap *cpm = qmc2ImagePixmapCache.object(cacheKey);
	if ( !cpm ) {
		qmc2CurrentItem = topLevelItem;
		loadImage(gameName, gameName);
	}

	if ( scaledImage() )
		drawScaledImage(&currentPixmap, &p);
	else
		drawCenteredImage(&currentPixmap, &p);
}

void ImageWidget::refresh()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageWidget::refresh()");
#endif

	if ( !cacheKey.isEmpty() ) {
		qmc2ImagePixmapCache.remove(cacheKey);
		update();
	}
}

bool ImageWidget::loadImage(QString gameName, QString onBehalfOf, bool checkOnly, QString *fileName, bool loadImages)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ImageWidget::loadImage(QString gameName = %1, QString onBehalfOf = %2, bool checkOnly = %3, QString *fileName = %4, bool loadImages = %5)").arg(gameName).arg(onBehalfOf).arg(checkOnly).arg((qulonglong)fileName).arg(loadImages));
#endif

	ImagePixmap pm;
	char imageBuffer[QMC2_ZIP_BUFFER_SIZE];

	if ( fileName )
		*fileName = "";

	bool fileOk = true;

	if ( useZip() ) {
		// try loading image from ZIP(s)
		QByteArray imageData;
		int len, i;
		QString gameFile = gameName + ".png";

		if ( fileName )
			*fileName = gameFile;

		foreach (unzFile imageFile, imageFileMap) {
			if ( unzLocateFile(imageFile, (const char *)gameFile.toLocal8Bit(), 0) == UNZ_OK ) {
				if ( unzOpenCurrentFile(imageFile) == UNZ_OK ) {
					while ( (len = unzReadCurrentFile(imageFile, &imageBuffer, QMC2_ZIP_BUFFER_SIZE)) > 0 ) {
						for (i = 0; i < len; i++)
							imageData += imageBuffer[i];
					}
					fileOk = true;
					unzCloseCurrentFile(imageFile);
				} else
					fileOk = false;
			} else
				fileOk = false;

			if ( fileOk )
				break;
			else
				imageData.clear();
		}

		if ( fileOk )
			fileOk = pm.loadFromData(imageData, "PNG");

		if ( !checkOnly ) {
			if ( fileOk ) {
				qmc2ImagePixmapCache.insert(onBehalfOf, new ImagePixmap(pm), pm.toImage().byteCount());
				currentPixmap = pm;
			} else {
				QString parentName = qmc2ParentMap[gameName];
				if ( qmc2ParentImageFallback && !parentName.isEmpty() ) {
					fileOk = loadImage(parentName, onBehalfOf);
				} else {
					currentPixmap = qmc2MainWindow->qmc2GhostImagePixmap;
					if ( !qmc2RetryLoadingImages )
						qmc2ImagePixmapCache.insert(onBehalfOf, new ImagePixmap(currentPixmap), currentPixmap.toImage().byteCount()); 
				}
			}
		}
	} else {
		// try loading image from (semicolon-separated) folder(s)
		foreach (QString baseDirectory, imageDir().split(";", QString::SkipEmptyParts)) {
			QString imgDir = baseDirectory + gameName;
			QString imagePath = imgDir + ".png";

			if ( fileName )
				*fileName = imagePath;

			QFile f(imagePath);
			if ( !f.exists() ) {
				QDir dir(imgDir);
				if ( dir.exists() ) {
					QStringList nameFilter;
					nameFilter << "*.png";
					QStringList dirEntries = dir.entryList(nameFilter, QDir::Files | QDir::NoDotAndDotDot | QDir::Readable | QDir::CaseSensitive, QDir::Name | QDir::Reversed);
					if ( dirEntries.count() > 0 ) {
						imagePath = imgDir + "/" + dirEntries[0];
						if ( fileName )
							*fileName = imagePath;
					}
				}
			}

			if ( checkOnly ) {
				if ( loadImages )
					fileOk = pm.load(imagePath, "PNG");
				else {
					QFile f(imagePath);
					fileOk = f.exists();
					if ( !fileOk ) {
						QString parentName = qmc2ParentMap[gameName];
						if ( qmc2ParentImageFallback && !parentName.isEmpty() )
							fileOk = loadImage(parentName, onBehalfOf, checkOnly, fileName, false);
					}
				}
			} else {
				if ( pm.load(imagePath, "PNG") ) {
					pm.imagePath = imagePath;
					qmc2ImagePixmapCache.insert(onBehalfOf, new ImagePixmap(pm), pm.toImage().byteCount());
					currentPixmap = pm;
					fileOk = true;
				} else {
					QString parentName = qmc2ParentMap[gameName];
					if ( qmc2ParentImageFallback && !parentName.isEmpty() ) {
						fileOk = loadImage(parentName, onBehalfOf);
					} else {
						currentPixmap = qmc2MainWindow->qmc2GhostImagePixmap;
						if ( !qmc2RetryLoadingImages )
							qmc2ImagePixmapCache.insert(onBehalfOf, new ImagePixmap(currentPixmap), currentPixmap.toImage().byteCount()); 
						fileOk = false;
					}
				}
			}

			if ( fileOk )
				break;
		}
	}

	return fileOk;
}

QString ImageWidget::primaryPathFor(QString gameName)
{
	if ( !useZip() ) {
		QStringList fl = imageDir().split(";", QString::SkipEmptyParts);
		QString baseDirectory;
		if ( fl.count() > 0 )
			baseDirectory = fl[0];
		return QDir::toNativeSeparators(QDir::cleanPath(baseDirectory + "/" + gameName + ".png"));
	} else // we don't support on-the-fly image replacement for zipped images yet!
		return QString();
}

bool ImageWidget::replaceImage(QString gameName, QPixmap &pixmap)
{
	if ( !useZip() ) {
		QString savePath = primaryPathFor(gameName);
		if ( !savePath.isEmpty() ) {
			bool goOn = true;
			if ( QFile::exists(savePath) ) {
				QString backupPath = savePath + ".bak";
				if ( QFile::exists(backupPath) )
					QFile::remove(backupPath);
				if ( !QFile::copy(savePath, backupPath) ) {
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't create backup of existing image file '%1' as '%2'").arg(savePath).arg(backupPath));
					goOn = false;
				}
			}
			if ( goOn ) {
				if ( pixmap.save(savePath, "PNG") ) {
					currentPixmap = pixmap;
					currentPixmap.imagePath = savePath;
					update();
					return true;
				} else {
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't create image file '%1'").arg(savePath));
					return false;
				}
			} else
				return false;
		} else {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't determine primary path for image-type '%1'").arg(imageType()));
			return false;
		}
	} else // we don't support on-the-fly image replacement for zipped images yet!
		return false;
}

bool ImageWidget::checkImage(QString gameName, unzFile zip, QSize *sizeReturn, int *bytesUsed, QString *fileName, QString *readerError)
{
	QImage image;
	char imageBuffer[QMC2_ZIP_BUFFER_SIZE];

	if ( fileName )
		*fileName = "";

	bool fileOk = true;

	if ( useZip() ) {
		// try loading image from ZIP(s)
		QByteArray imageData;
		int len, i;
		QString gameFile = gameName + ".png";

		if ( fileName )
			*fileName = gameFile;

		if ( zip == NULL ) {
			foreach (unzFile imageFile, imageFileMap) {
				if ( unzLocateFile(imageFile, (const char *)gameFile.toLocal8Bit(), 0) == UNZ_OK ) {
					if ( unzOpenCurrentFile(imageFile) == UNZ_OK ) {
						while ( (len = unzReadCurrentFile(imageFile, &imageBuffer, QMC2_ZIP_BUFFER_SIZE)) > 0 ) {
							for (i = 0; i < len; i++)
								imageData += imageBuffer[i];
						}
						fileOk = true;
						unzCloseCurrentFile(imageFile);
					} else
						fileOk = false;
				} else
					fileOk = false;

				if ( fileOk )
					break;
				else
					imageData.clear();
			}
		} else {
			if ( unzLocateFile(zip, (const char *)gameFile.toLocal8Bit(), 0) == UNZ_OK ) {
				if ( unzOpenCurrentFile(zip) == UNZ_OK ) {
					while ( (len = unzReadCurrentFile(zip, &imageBuffer, QMC2_ZIP_BUFFER_SIZE)) > 0 ) {
						for (i = 0; i < len; i++)
							imageData += imageBuffer[i];
					}
					unzCloseCurrentFile(zip);
				} else
					fileOk = false;
			} else
				fileOk = false;
		}

		if ( fileOk ) {
			QBuffer buffer(&imageData);
			QImageReader imageReader(&buffer, "PNG");
			fileOk = imageReader.read(&image);
			if ( fileOk ) {
				if ( sizeReturn )
					*sizeReturn = image.size();
				if ( bytesUsed )
					*bytesUsed = image.byteCount();
			} else if ( readerError != NULL && imageReader.error() != QImageReader::FileNotFoundError )
				*readerError = imageReader.errorString();
		}
	} else {
		// try loading image from (semicolon-separated) folder(s)
		foreach (QString baseDirectory, imageDir().split(";", QString::SkipEmptyParts)) {
			QString imgDir = baseDirectory + gameName;
			QString localImagePath = imgDir + ".png";

			if ( fileName )
				*fileName = QDir::toNativeSeparators(localImagePath);

			QImageReader imageReader(localImagePath, "PNG");
			fileOk = imageReader.read(&image);

			if ( fileOk ) {
				if ( sizeReturn )
					*sizeReturn = image.size();
				if ( bytesUsed )
					*bytesUsed = image.byteCount();
				break;
			} else if ( readerError != NULL && imageReader.error() != QImageReader::FileNotFoundError )
				*readerError = imageReader.errorString();
		}
	}

	return fileOk;
}

void ImageWidget::drawCenteredImage(QPixmap *pm, QPainter *p)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ImageWidget::drawCenteredImage(QPixmap *pm = %1, QPainter *p = %2)").arg((qulonglong)pm).arg((qulonglong)p));
#endif

	p->eraseRect(rect());

	if ( pm == NULL ) {
		p->end();
		return;
	}

	// last resort if pm->load() retrieved a null pixmap...
	if ( pm->isNull() )
		pm = &qmc2MainWindow->qmc2GhostImagePixmap;

	int posx = (rect().width() - pm->width()) / 2;
	int posy = (rect().height() - pm->height()) / 2;

	p->drawPixmap(posx, posy, *pm);

	bool drawGameName = false;
	if ( qmc2ShowGameName ) {
		if ( qmc2ShowGameNameOnlyWhenRequired ) {
			if ( qmc2MainWindow->hSplitter->sizes()[0] == 0 || qmc2MainWindow->tabWidgetGamelist->currentIndex() != QMC2_GAMELIST_INDEX ) {
				drawGameName = true;
			} else {
				drawGameName = false;
			}
		} else
			drawGameName = true;
	} else
		drawGameName = false;

	if ( drawGameName ) {
		// draw game/machine title
		QString title = qmc2GamelistDescriptionMap[qmc2CurrentItem->text(QMC2_GAMELIST_COLUMN_NAME)];
		QFont f(qApp->font());
		f.setWeight(QFont::Bold);
		p->setFont(f);
		QFontMetrics fm(f);
		QRect r = rect();
		int adjustment = fm.height() / 2;
		r = r.adjusted(+adjustment, +adjustment, -adjustment, -adjustment);
		QRect outerRect = p->boundingRect(r, Qt::AlignCenter | Qt::TextWordWrap, title);
		r.setTop(r.bottom() - outerRect.height());
		r = p->boundingRect(r, Qt::AlignCenter | Qt::TextWordWrap, title);
		r = r.adjusted(-adjustment, -adjustment, +adjustment, +adjustment);
		r.setBottom(rect().bottom());
		p->setPen(QColor(255, 255, 255, 0));
		p->fillRect(r, QBrush(QColor(0, 0, 0, 128), Qt::SolidPattern));
		p->setPen(QPen(QColor(255, 255, 255, 255)));
		p->drawText(r, Qt::AlignCenter | Qt::TextWordWrap, title);
	}

	p->end();
}

void ImageWidget::drawScaledImage(QPixmap *pm, QPainter *p)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ImageWidget::drawScaledImage(QPixmap *pm = %1, QPainter *p = %2)").arg((qulonglong)pm).arg((qulonglong)p));
#endif

	if ( pm == NULL ) {
		p->eraseRect(rect());
		p->end();
		return;
	}

	// last resort if pm->load() retrieved a null pixmap...
	if ( pm->isNull() )
		pm = &qmc2MainWindow->qmc2GhostImagePixmap;

	double desired_width;
	double desired_height;

	if ( pm->width() > pm->height() ) {
		desired_width  = contentsRect().width();
		desired_height = (double)pm->height() * (desired_width / (double)pm->width());
		if ( desired_height > contentsRect().height() ) {
			desired_height = contentsRect().height();
			desired_width  = (double)pm->width() * (desired_height / (double)pm->height());
		}
	} else {
		desired_height = contentsRect().height();
		desired_width  = (double)pm->width() * (desired_height / (double)pm->height());
		if ( desired_width > contentsRect().width() ) {
			desired_width = contentsRect().width();
			desired_height = (double)pm->height() * (desired_width / (double)pm->width());
		}
	}

	QPixmap pmScaled;

	if ( qmc2SmoothScaling )
		pmScaled = pm->scaled((int)desired_width, (int)desired_height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	else
		pmScaled = pm->scaled((int)desired_width, (int)desired_height, Qt::KeepAspectRatio, Qt::FastTransformation);

	drawCenteredImage(&pmScaled, p);
}

void ImageWidget::copyToClipboard()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageWidget::copyToClipboard()");
#endif

	if ( !currentPixmap.isNull() )
		qApp->clipboard()->setPixmap(currentPixmap);
}

void ImageWidget::copyPathToClipboard()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageWidget::copyPathToClipboard()");
#endif

	if ( !currentPixmap.imagePath.isEmpty() )
		qApp->clipboard()->setText(currentPixmap.imagePath);
}

void ImageWidget::contextMenuEvent(QContextMenuEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ImageWidget::contextMenuEvent(QContextMenuEvent *e = %1)").arg((qulonglong)e));
#endif

	actionCopyPathToClipboard->setVisible(!currentPixmap.imagePath.isEmpty());
	contextMenu->move(qmc2MainWindow->adjustedWidgetPosition(mapToGlobal(e->pos()), contextMenu));
	contextMenu->show();
}

QString ImageWidget::toBase64()
{
	ImagePixmap pm;
	if ( !currentPixmap.isNull() )
		pm = currentPixmap;
	else
		pm = qmc2MainWindow->qmc2GhostImagePixmap;
	QByteArray imageData;
	QBuffer buffer(&imageData);
	pm.save(&buffer, "PNG");
	return QString(imageData.toBase64());
}
