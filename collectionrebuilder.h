#ifndef _FILEEDITWIDGET_H_
#define _FILEEDITWIDGET_H_

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QFile>

#include "checksumdbmgr.h"
#include "xmldbmgr.h"
#include "ui_collectionrebuilder.h"

class CollectionRebuilder;

class CollectionRebuilderThread : public QThread
{
	Q_OBJECT

	public:
		bool exitThread;
		bool isActive;
		bool isWaiting;
		bool isPaused;
		bool pauseRequested;
		bool stopRebuilding;
		QMutex mutex;
		QWaitCondition waitCondition;

		CollectionRebuilderThread(QObject *parent = 0);
		~CollectionRebuilderThread();

		CheckSumDatabaseManager *checkSumDb() { return m_checkSumDb; }
		CollectionRebuilder *rebuilderDialog() { return m_rebuilderDialog; }
		XmlDatabaseManager *xmlDb() { return m_xmlDb; }
		void reopenDatabase();
		bool parseXml(QString, QString *, QStringList *, QStringList *, QStringList *, QStringList *, QStringList *);
		bool nextId(QString *, QStringList *, QStringList *, QStringList *, QStringList *, QStringList *);
		bool rewriteSet(QString *, QStringList *, QStringList *, QStringList *, QStringList *, QStringList *);
		bool writeAllFileData(QString, QString, QStringList *, QStringList *, QStringList *, QStringList *, QStringList *);
		bool writeAllZipData(QString, QString, QStringList *, QStringList *, QStringList *, QStringList *, QStringList *);
		bool readFileData(QString, QByteArray *);
		bool readSevenZipFileData(QString, QString, QByteArray *);
		bool readZipFileData(QString, QString, QByteArray *);
		bool createBackup(QString filePath);

	public slots:
		void pause();
		void resume();

	signals:
		void log(const QString &);
		void rebuildStarted();
		void rebuildFinished();
		void rebuildPaused();
		void rebuildResumed();
		void progressTextChanged(const QString &);
		void progressRangeChanged(int, int);
		void progressChanged(int);
		void statusUpdated(int, int, int);

	protected:
		void run();

	private:
		CheckSumDatabaseManager *m_checkSumDb;
		XmlDatabaseManager *m_xmlDb;
		CollectionRebuilder *m_rebuilderDialog;
		qint64 m_xmlIndex, m_xmlIndexCount;
		QFile m_xmlFile;
};

class CollectionRebuilder : public QDialog, public Ui::CollectionRebuilder
{
	Q_OBJECT

       	public:
		CollectionRebuilder(QWidget *parent = 0);
		~CollectionRebuilder();

		CollectionRebuilderThread *rebuilderThread() { return m_rebuilderThread; }

	public slots:
		void on_spinBoxMaxLogSize_valueChanged(int);
		void log(const QString &);
		void clear() { plainTextEditLog->clear(); }
		void scrollToEnd();
		void adjustIconSizes();
		void on_pushButtonStartStop_clicked();
		void on_pushButtonPauseResume_clicked();
		void on_comboBoxXmlSource_currentIndexChanged(int);
		void on_toolButtonRemoveXmlSource_clicked();
		void rebuilderThread_rebuildStarted();
		void rebuilderThread_rebuildFinished();
		void rebuilderThread_rebuildPaused();
		void rebuilderThread_rebuildResumed();
		void rebuilderThread_progressTextChanged(const QString &);
		void rebuilderThread_progressRangeChanged(int, int);
		void rebuilderThread_progressChanged(int);
		void rebuilderThread_statusUpdated(int, int, int);

	protected:
		void showEvent(QShowEvent *);
		void hideEvent(QHideEvent *);
		void closeEvent(QCloseEvent *);
		void keyPressEvent(QKeyEvent *);

	private:
		CollectionRebuilderThread *m_rebuilderThread;
		QString m_defaultSetEntity, m_defaultRomEntity, m_defaultDiskEntity;
};

#endif
