#ifndef ARCHIVEFILE_H
#define ARCHIVEFILE_H

#include <QObject>
#include <QString>
#include <QList>
#include <QHash>
#include <QStringList>
#include <QDateTime>
#include <QByteArray>

#include <archive.h>
#include "macros.h"

class ArchiveEntryMetaData
{
	public:
		explicit ArchiveEntryMetaData(QString name = QString(), quint64 size = 0, QDateTime date = QDateTime())
		{
			setName(name);
			setSize(size);
			setDate(date);
		}

		void setName(QString name) { m_name = name; }
		QString name() { return m_name; }
		void setSize(quint64 size) { m_size = size; }
		quint64 size() { return m_size; }
		void setDate(QDateTime date) { m_date = date; }
		QDateTime date() { return m_date; }

	private:
		QString m_name;
		quint64 m_size;
		QDateTime m_date;
};

class ArchiveFile : public QObject
{
	Q_OBJECT

	public:
		explicit ArchiveFile(QString fileName = QString(), bool sequential = false, QObject *parent = 0);
		~ArchiveFile();

		QString fileName() { return m_fileName; }
		QList<ArchiveEntryMetaData> &entryList() { return m_entryList; }
		bool isOpen() { return m_archive != 0; }
		bool open(QString fileName = QString());
		void close();
		bool seekNextEntry(ArchiveEntryMetaData *metaData, bool *reset = 0);
		bool seekEntry(QString name);
		bool seekEntry(uint index);
		bool hasError() { return errorCode() == ARCHIVE_FATAL; }
		bool hasWarning() { return errorCode() == ARCHIVE_WARN; }
		qint64 readBlock(QByteArray *buffer);
		QString errorString();
		int errorCode();
		void createEntryList();

	private:
		int indexOfName(QString name) { if ( m_nameToIndexCache.contains(name) ) return m_nameToIndexCache[name]; else return -1; }

		struct archive *m_archive;
		const void *m_buffer;
		QList<ArchiveEntryMetaData> m_entryList;
		QHash<QString, int> m_nameToIndexCache;
		QString m_fileName;
		bool m_sequential;
};

#endif
