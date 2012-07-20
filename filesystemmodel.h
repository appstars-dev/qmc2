#ifndef _FILESYSTEMMODEL_H_
#define _FILESYSTEMMODEL_H_

#include <QDir>
#include <QDirIterator>
#include <QDateTime>
#include <QFileIconProvider>
#include <QAbstractItemModel>
#include <QFileSystemModel>
#include <QApplication>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QTimer>
#include <QLocale>
#include <QTest>

#include "macros.h"
#include "unzip.h"
#include <time.h>

#define QMC2_DIRENTRY_THRESHOLD		250

class DirectoryModel : public QFileSystemModel
{
	Q_OBJECT

	public:
		DirectoryModel(QObject *parent = 0) : QFileSystemModel(parent) {}

		virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const
		{
			if ( orientation == Qt::Horizontal ) {
				switch ( role ) {
					case Qt::DisplayRole:
#if defined(Q_OS_WIN)
						return tr("Drives / Folders");
#else
						return tr("Folders");
#endif
					case Qt::TextAlignmentRole:
						return Qt::AlignLeft;
				}
			}

			return QVariant();
		}
};

class DirectoryScannerThread : public QThread
{
	Q_OBJECT

	public:
		QMutex waitMutex;
		QWaitCondition waitCondition;
		bool isScanning;
		bool stopScanning;
		bool quitFlag;
		bool isReady;
		QString dirPath;
		QStringList dirEntries;
		QStringList nameFilters;

		DirectoryScannerThread(QString path, QObject *parent = 0) : QThread(parent)
		{
			isReady = isScanning = stopScanning = quitFlag = false;
			dirPath = path;
			start();
		}

		~DirectoryScannerThread()
		{
			stopScanning = quitFlag = true;
			quit();
		}

	protected:
		void run()
		{
			while ( !quitFlag ) {
#if defined(QMC2_DEBUG)
				printf("DirectoryScannerThread: waiting\n");
#endif
				waitMutex.lock();
				isReady = true;
				waitCondition.wait(&waitMutex);
				waitMutex.unlock();
#if defined(QMC2_DEBUG)
				printf("DirectoryScannerThread: starting scan of %s\n", (const char *)dirPath.toAscii());
#endif
				if ( !stopScanning && !quitFlag ) {
					waitMutex.lock();
					isReady = false;
					isScanning = true;
					stopScanning = false;
					dirEntries.clear();
					QDirIterator dirIterator(dirPath, nameFilters, QDir::Files);
					while ( dirIterator.hasNext() && !stopScanning && !quitFlag ) {
						dirIterator.next();
						dirEntries << dirIterator.fileName();
						if ( dirEntries.count() >= QMC2_DIRENTRY_THRESHOLD ) {
							emit entriesAvailable(dirEntries);
#if defined(QMC2_DEBUG)
							foreach (QString entry, dirEntries)
								printf("DirectoryScannerThread: %s\n", (const char *)entry.toAscii());
#endif
							dirEntries.clear();
							//QThread::yieldCurrentThread();
							//QTest::qSleep(1);
						}
					}
					if ( !stopScanning && !quitFlag ) {
						if ( dirEntries.count() > 0 ) {
							emit entriesAvailable(dirEntries);
#if defined(QMC2_DEBUG)
							foreach (QString entry, dirEntries)
								printf("DirectoryScannerThread: %s\n", (const char *)entry.toAscii());
#endif
						}
						emit finished();
					}
					isScanning = false;
					waitMutex.unlock();
				}
#if defined(QMC2_DEBUG)
				printf("DirectoryScannerThread: finished scan of %s\n", (const char *)dirPath.toAscii());
#endif
			}
#if defined(QMC2_DEBUG)
			printf("DirectoryScannerThread: ended\n");
#endif
		}

	signals:
		void entriesAvailable(const QStringList &);
		void finished();
};

class FileSystemItem : public QObject
{
	Q_OBJECT

	public:
		enum Column {NAME, SIZE, DATE, LASTCOLUMN};

		FileSystemItem(QString path, FileSystemItem *parent = 0, bool archiveMember = false, quint64 uncompressedSize = 0, QDateTime entryDate = QDateTime()) : QObject(parent)
		{
			mParent = parent;
			if ( parent ) {
				parent->addFile(this);
				mFileInfo = QFileInfo(path);
				mFileName = mFileInfo.fileName();
				mAbsDirPath = parent->absoluteDirPath();
				mAbsFilePath = mAbsDirPath + QString("/") + path;
				mFileInfo = QFileInfo(mAbsFilePath);
				mIsArchiveMember = archiveMember;
				mUncompressedSize = uncompressedSize;
				mEntryDate = entryDate;
				if ( mIsArchiveMember ) {
					mIsArchive = false;
					mFileName = path;
					mAbsFilePath = parent->absoluteFilePath() + "\\" + path;
					mFileInfo = QFileInfo(path);
				} else
					mIsArchive = mFileName.toLower().endsWith(".zip");
			} else {
				mAbsDirPath = path;
				mFileInfo = QFileInfo();
				mIsArchiveMember = mIsArchive = false;
			}
		}

		~FileSystemItem()
		{
			qDeleteAll(mFiles);
		}

		FileSystemItem *fileAt(int position)
		{
			return mFiles.value(position, 0);
		}

		int fileCount() const
		{
			return mFiles.count();
		}

		int fileNumber() const
		{
			if ( mParent )
				return mParent->mFiles.indexOf(const_cast<FileSystemItem *>(this));
			else
				return 0;
		}

		FileSystemItem *itemParent()
		{
			return mParent;
		}

		QString absoluteFilePath() const
		{
			return mAbsFilePath;
		}

		QString absoluteDirPath() const
		{
			return mAbsDirPath;
		}

		QString fileName() const
		{
			return mFileName;
		}

		QDateTime fileDate() const
		{
			if ( mIsArchiveMember )
				return mEntryDate;
			else
				return mFileInfo.lastModified();
		}

		quint64 fileSize() const
		{
			if ( mIsArchiveMember )
				return mUncompressedSize;
			else
				return mFileInfo.size();
		}

		QFileInfo fileInfo() const
		{
			return mFileInfo;
		}

		bool isArchive() const
		{
			return mIsArchive;
		}

		void addFile(FileSystemItem *file)
		{
			mFiles.append(file);
		}

		void sort(Qt::SortOrder sortOrder = Qt::AscendingOrder, int column = NAME)
		{
			switch ( column ) {
				case SIZE: {
						QMultiMap<quint64, FileSystemItem *> map;
						foreach (FileSystemItem *item, mFiles) map.insert(item->fileSize(), item);
						mFiles = map.values();
					}
					break;
				case DATE: {
						QMultiMap<QDateTime, FileSystemItem *> map;
						foreach (FileSystemItem *item, mFiles) map.insert(item->fileDate(), item);
						mFiles = map.values();
					}
					break;
				case NAME:
				default: {
						QMultiMap<QString, FileSystemItem *> map;
						foreach (FileSystemItem *item, mFiles) map.insert(item->fileName().toLower(), item);
						mFiles = map.values();
					}
					break;
			}

			if ( sortOrder == Qt::DescendingOrder )
				for (int k = 0; k < mFiles.size() / 2; k++) mFiles.swap(k, mFiles.size() - (1 + k));
		}

	private:
		FileSystemItem* mParent;
		QList<FileSystemItem*> mFiles;
		QFileInfo mFileInfo;
		QString mAbsFilePath;
		QString mAbsDirPath;
		QString mFileName;
		bool mIsArchive;
		bool mIsArchiveMember;
		quint64 mUncompressedSize;
		QDateTime mEntryDate;
};

class FileSystemModel : public QAbstractItemModel
{
	Q_OBJECT

	public:
		enum Column {NAME, SIZE, DATE, LASTCOLUMN};
		DirectoryScannerThread *dirScanner;

		FileSystemModel(QObject *parent) : QAbstractItemModel(parent), mIconFactory(new QFileIconProvider())
		{
			mHeaders << tr("Name") << tr("Size") /*<< tr("Type")*/ << tr("Date modified");
			mRootItem = new FileSystemItem("");
			mCurrentPath = "";
			mFileCount = mStaleCount = 0;
			mBreakZipScan = false;
			dirScanner = new DirectoryScannerThread(mRootItem->absoluteDirPath());
			connect(dirScanner, SIGNAL(entriesAvailable(const QStringList &)), this, SLOT(scannerEntriesAvailable(const QStringList &)));
			connect(dirScanner, SIGNAL(finished()), this, SLOT(scannerFinished()));
		}

		~FileSystemModel()
		{
			if ( dirScanner ) {
				dirScanner->stopScanning = true;
				dirScanner->quitFlag = true;
				dirScanner->waitCondition.wakeAll();
				dirScanner->deleteLater();
				dirScanner = NULL;
			}
			delete mRootItem;
			delete mIconFactory;
		}

		virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const
		{
			if ( orientation == Qt::Horizontal ) {
				switch ( role ) {
					case Qt::DisplayRole:
						return mHeaders.at(section);
					case Qt::TextAlignmentRole:
						return int(SIZE) == section || int(DATE) == section ? Qt::AlignRight : Qt::AlignLeft;
				}
			}

			return QVariant();
		}

		virtual bool canFetchMore(const QModelIndex &parent) const
		{
			FileSystemItem *parentItem = getItem(parent);
			if ( parentItem == mRootItem )
				return (mStaleCount > 0);
			else 
				return false;
		}

		virtual void fetchMore(const QModelIndex &parent)
		{
			FileSystemItem *parentItem = getItem(parent);

			if ( parentItem == mRootItem ) {
				emit layoutAboutToBeChanged();
				int itemsToFetch = qMin(QMC2_DIRENTRY_THRESHOLD, mStaleCount);
				beginInsertRows(QModelIndex(), mFileCount, mFileCount + itemsToFetch - 1);
				mFileCount += itemsToFetch;
				mStaleCount -= itemsToFetch;
				endInsertRows();
				emit layoutChanged();
#if defined(QMC2_DEBUG)
				printf("mFileCount = %d, mStaleCount = %d\n", mFileCount, mStaleCount);
#endif
			}
		}

		virtual Qt::ItemFlags flags(const QModelIndex &index) const
		{
			return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
		}

		virtual int columnCount(const QModelIndex &parent = QModelIndex()) const
		{
			return LASTCOLUMN;
		}

		virtual int rowCount(const QModelIndex &parent = QModelIndex()) const
		{
			FileSystemItem *parentItem = getItem(parent);
			if ( parentItem == mRootItem )
				return mFileCount;
			else if ( parentItem->itemParent() == mRootItem && parent.column() == int(NAME) )
				return parentItem->fileCount();
			else
				return 0;
		}

		virtual QModelIndex index(int row, int column, const QModelIndex &parent) const
		{
#if defined(QMC2_DEBUG)
			printf("index requested for row = %d, column = %d\n", row, column);
#endif
			if ( parent.isValid() && parent.column() != int(NAME) )
				return QModelIndex();

			FileSystemItem *childItem = getItem(parent)->fileAt(row);

			if ( childItem )
				return createIndex(row, column, childItem);
			else
				return QModelIndex();
		}

		virtual QModelIndex parent(const QModelIndex &index) const
		{
			if ( !index.isValid() )
				return QModelIndex();

			FileSystemItem *parentItem = getItem(index)->itemParent();

			if ( !parentItem || parentItem == mRootItem )
				return QModelIndex();

			return createIndex(parentItem->fileNumber(), NAME, parentItem);
		}

		virtual QVariant data(const QModelIndex &index, int role) const
		{
			if ( !index.isValid() )
				return QVariant();

			if ( role == Qt::TextAlignmentRole ) {
				if ( int(SIZE) == index.column() || int(DATE) == index.column() )
					return Qt::AlignRight;
				else
					return Qt::AlignLeft;
			}

			if ( role != Qt::DisplayRole && role != Qt::DecorationRole )
				return QVariant();

			FileSystemItem *item = getItem(index);

			if ( !item )
				return QVariant();

			if ( role == Qt::DecorationRole ) {
				if ( index.column() == int(NAME) ) {
					QIcon icon = mIconFactory->icon(item->fileInfo());
					if ( icon.isNull() ) { // icon fall-back
						if ( item->fileName().endsWith("/") )
							icon = mIconFactory->icon(QFileIconProvider::Folder);
						else
							icon = mIconFactory->icon(QFileIconProvider::File);
					}
					return icon;
				} else
					return QIcon();
			}

			QVariant data;
			Column col = Column(index.column());

			switch ( col ) {
				case NAME:
					data = item->fileName();
					break;
				case SIZE:
					data = humanReadable(item->fileSize());
					break;
				case DATE:
					data = item->fileDate().toString(Qt::LocalDate);
					break;
				default:
					break;
			}

			return data;
		}

		virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder)
		{
			emit layoutAboutToBeChanged();
			reset();
			mRootItem->sort(order, column);
			emit layoutChanged();
		}

		QString humanReadable(quint64 value) const
		{
			static QString hrString;
			static qreal hrValue;
			static QLocale locale;
#if __WORDSIZE == 64
			if ( (qreal)value / (qreal)QMC2_ONE_KILOBYTE < (qreal)QMC2_ONE_KILOBYTE ) {
				hrValue = (qreal)value / (qreal)QMC2_ONE_KILOBYTE;
				hrString = locale.toString(hrValue, 'f', 2) + QString(tr(" KB"));
			} else if ( (qreal)value / (qreal)QMC2_ONE_MEGABYTE < (qreal)QMC2_ONE_KILOBYTE ) {
				hrValue = (qreal)value / (qreal)QMC2_ONE_MEGABYTE;
				hrString = locale.toString(hrValue, 'f', 2) + QString(tr(" MB"));
			} else if ( (qreal)value / (qreal)QMC2_ONE_GIGABYTE < (qreal)QMC2_ONE_KILOBYTE ) {
				hrValue = (qreal)value / (qreal)QMC2_ONE_GIGABYTE;
				hrString = locale.toString(hrValue, 'f', 2) + QString(tr(" GB"));
			} else {
				hrValue = (qreal)value / (qreal)QMC2_ONE_TERABYTE;
				hrString = locale.toString(hrValue, 'f', 2) + QString(tr(" TB"));
			}
#else
			if ( (qreal)value / (qreal)QMC2_ONE_KILOBYTE < (qreal)QMC2_ONE_KILOBYTE ) {
				hrValue = (qreal)value / (qreal)QMC2_ONE_KILOBYTE;
				hrString = locale.toString(hrValue, 'f', 2) + QString(tr(" KB"));
			} else if ( (qreal)value / (qreal)QMC2_ONE_MEGABYTE < (qreal)QMC2_ONE_KILOBYTE ) {
				hrValue = (qreal)value / (qreal)QMC2_ONE_MEGABYTE;
				hrString = locale.toString(hrValue, 'f', 2) + QString(tr(" MB"));
			} else {
				hrValue = (qreal)value / (qreal)QMC2_ONE_GIGABYTE;
				hrString = locale.toString(hrValue, 'f', 2) + QString(tr(" GB"));
			}
#endif
			return hrString;
		}

		QString absolutePath(const QModelIndex &index)
		{
			FileSystemItem *item = static_cast<FileSystemItem*>(index.internalPointer());

			if ( item )
				return item->absoluteFilePath();
			else
				return QString();
		}

		QModelIndex firstIndex() const
		{
			return createIndex(0, NAME, mRootItem->fileAt(0));
		}

		QString currentPath() const
		{
			return mCurrentPath;
		}

		QModelIndex setCurrentPath(const QString &path, bool scan = true)
		{
			mCurrentPath = path;
			mBreakZipScan = true;

			if ( dirScanner )
				if ( dirScanner->isScanning ) {
					dirScanner->stopScanning = true;
					while ( dirScanner->isScanning ) QTest::qWait(1);
				}

			mFileCount = mStaleCount = 0;

			beginRemoveRows(QModelIndex(), 0, mRootItem->fileCount() - 1);
			delete mRootItem;
			endRemoveRows();

			mRootItem = new FileSystemItem(path, 0);

			if ( scan )
				populateItems();

			return firstIndex();
		}

		void setNameFilters(const QStringList &filters)
		{
			mNameFilters = filters;
		}

		virtual bool insertRows(int row, int /*count*/, const QModelIndex &parent = QModelIndex())
		{
			FileSystemItem *parentItem = getItem(parent);
			if ( parentItem->itemParent() == mRootItem && parent.column() == int(NAME) ) {
				if ( mZipEntryList.count() > 0 ) {
					emit layoutAboutToBeChanged();
					beginInsertRows(parent, row, row + mZipEntryList.count() - 1);
					for (int i = 0; i < mZipEntryList.count(); i++)
						new FileSystemItem(mZipEntryList[i], parentItem, true, mZipEntrySizes[i], mZipEntryDates[i]);
					endInsertRows();
					emit layoutChanged();
				}
				return true;
			} else
				return false;
		}

		void openZip(const QModelIndex &index)
		{
			if ( !index.isValid() )
				return;

			FileSystemItem *fileItem = getItem(index);

			if ( !fileItem || fileItem == mRootItem || fileItem->fileCount() > 0 )
				return;

			unzFile zipFile = unzOpen((const char *)fileItem->absoluteFilePath().toAscii());

			if ( zipFile ) {
		  		char zipFileName[QMC2_ZIP_BUFFER_SIZE];
				unz_file_info zipInfo;
				int row = 0;
				mZipEntryList.clear();
				mZipEntrySizes.clear();
				mZipEntryDates.clear();
				mBreakZipScan = false;
				// the zip-entry lists currently carry only one entry at time for better GUI response
				do {
					if ( unzGetCurrentFileInfo(zipFile, &zipInfo, zipFileName, QMC2_ZIP_BUFFER_SIZE, 0, 0, 0, 0) == UNZ_OK ) {
						mZipEntryList << zipFileName;
						mZipEntrySizes << zipInfo.uncompressed_size;
						struct tm *t;
						time_t clock = time(NULL);
						t = localtime(&clock);
						t->tm_isdst = -1;
						t->tm_sec  = (((int)zipInfo.dosDate) <<  1) & 0x3e;
						t->tm_min  = (((int)zipInfo.dosDate) >>  5) & 0x3f;
						t->tm_hour = (((int)zipInfo.dosDate) >> 11) & 0x1f;
						t->tm_mday = (int)(zipInfo.dosDate >> 16) & 0x1f;
						t->tm_mon  = ((int)(zipInfo.dosDate >> 21) & 0x0f) - 1;
						t->tm_year = ((int)(zipInfo.dosDate >> 25) & 0x7f) + 80;
						mZipEntryDates << QDateTime::fromTime_t(mktime(t));
					}
					insertRows(row, 1, index);
					mZipEntryList.clear();
					mZipEntrySizes.clear();
					mZipEntryDates.clear();
					row++;
				} while ( unzGoToNextFile(zipFile) == UNZ_OK && !mBreakZipScan );
				unzClose(zipFile);
				mBreakZipScan = false;
			}
		}

		bool isZip(const QModelIndex &index)
		{
			FileSystemItem *item = getItem(index);
			if ( item ) {
				if ( item->itemParent() != mRootItem )
					return false;
				else
					return item->isArchive();
			} else
				return false;
		}

		void breakZipScan()
		{
			mBreakZipScan = true;
		}

	public slots:
		QModelIndex refresh()
		{
			return setCurrentPath(mCurrentPath);
		}

		void setSearchPattern(QString str)
		{
			mSearchPattern = str;
		}

	private slots:
		void scannerFinished()
		{
			emit finished();
		}

		void scannerEntriesAvailable(const QStringList &entryList)
		{
			QString filterPattern;
			int filteredCount = 0;
			if ( !mSearchPattern.isEmpty() ) {
				filterPattern = mSearchPattern;
				filterPattern = "*" + filterPattern.replace(' ', "* *") + "*";
			}
			if ( filterPattern.isEmpty() ) {
				filteredCount = entryList.count();
				foreach (QString entry, entryList)
					new FileSystemItem(entry, mRootItem);
			} else {
				foreach (QString entry, entryList)
					if ( entry.indexOf(QRegExp(filterPattern, Qt::CaseInsensitive, QRegExp::Wildcard)) >= 0 ) {
						new FileSystemItem(entry, mRootItem);
						filteredCount++;
					}
			}
			mStaleCount += filteredCount;
			fetchMore(QModelIndex());
#if defined(QMC2_DEBUG)
			printf("mFileCount = %d, mStaleCount = %d\n", mFileCount, mStaleCount);
#endif
		}

		void populateItems()
		{
			if ( dirScanner ) {
				if ( !dirScanner->isReady ) QTimer::singleShot(10, this, SLOT(populateItems()));
				dirScanner->dirPath = mRootItem->absoluteDirPath();
				dirScanner->nameFilters = mNameFilters;
				dirScanner->stopScanning = false;
				dirScanner->waitCondition.wakeAll();
			}
		}

		FileSystemItem *getItem(const QModelIndex &index) const
		{
			if ( index.isValid() ) {
				FileSystemItem *item = static_cast<FileSystemItem*>(index.internalPointer());
				if ( item ) return item;
			}
			return mRootItem;
		}

	private:
		FileSystemItem *mRootItem;
		QString mCurrentPath;
		QStringList mHeaders;
		QStringList mNameFilters;
		QStringList mZipEntryList;
		QList<quint64> mZipEntrySizes;
		QList<QDateTime> mZipEntryDates;
		QFileIconProvider *mIconFactory;
		QString mSearchPattern;
		int mFileCount;
		int mStaleCount;
		bool mBreakZipScan;

	signals:
		void finished();
};

#endif
