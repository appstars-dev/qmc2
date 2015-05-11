#ifndef _GAMELIST_H_
#define _GAMELIST_H_

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QIcon>
#include <QTreeWidgetItem>
#endif
#include <QHash>
#include <QMap>

#include "xmldbmgr.h"
#include "userdatadbmgr.h"
#include "datinfodbmgr.h"
#include "macros.h"

class Gamelist : public QObject
{
	Q_OBJECT

	public:
		int numTotalGames;
		int numGames;
		int numCorrectGames;
		int numMostlyCorrectGames;
		int numIncorrectGames;
		int numNotFoundGames;
		int numUnknownGames;
		int numSearchGames;
		int numDevices;
		int cachedGamesCounter;
		int numTaggedSets;
		int numVerifyRoms;
		int uncommittedXmlDbRows;
		bool verifyCurrentOnly;
		bool autoRomCheck;
		bool mergeCategories;
		bool dtdBufferReady;
		char oldRomState;
		QIcon qmc2UnknownImageIcon;
		QIcon qmc2UnknownBIOSImageIcon;
		QIcon qmc2UnknownDeviceImageIcon;
		QIcon qmc2CorrectImageIcon;
		QIcon qmc2CorrectBIOSImageIcon;
		QIcon qmc2CorrectDeviceImageIcon;
		QIcon qmc2MostlyCorrectImageIcon;
		QIcon qmc2MostlyCorrectBIOSImageIcon;
		QIcon qmc2MostlyCorrectDeviceImageIcon;
		QIcon qmc2IncorrectImageIcon;
		QIcon qmc2IncorrectBIOSImageIcon;
		QIcon qmc2IncorrectDeviceImageIcon;
		QIcon qmc2NotFoundImageIcon;
		QIcon qmc2NotFoundBIOSImageIcon;
		QIcon qmc2NotFoundDeviceImageIcon;
		QProcess *loadProc;
		QProcess *verifyProc;
		QTime loadTimer;
		QTime verifyTimer;
		QTime parseTimer;
		QTime miscTimer;
		QFile romCache;
		QFile gamelistCache;
		QTextStream tsRomCache;
		QTextStream tsGamelistCache;
		QString emulatorType;
		QString emulatorVersion;
		QString verifyLastLine;
		QStringList emulatorIdentifiers;
		QStringList verifiedList;
		QString xmlLineBuffer;
		QHash<QString, QString> driverNameHash;
		QHash<QString, char> gameStatusHash;
		QMap<QString, QString *> categoryNames;
		QMap<QString, QString *> categoryMap;
		QMap<QString, QString *> versionNames;
		QMap<QString, QString *> versionMap;
		QTreeWidgetItem *checkedItem;
		QStringList biosSets;
		QStringList deviceSets;

		static QStringList romTypeNames;
		static QStringList phraseTranslatorList;
		static QMap<QString, QString> reverseTranslation;

		QString lookupDriverName(QString);
		QString romStatus(QString, bool translated = false);
		char romState(QString);
		bool isBios(QString systemName) { return biosSets.contains(systemName); }
		bool isDevice(QString systemName) { return deviceSets.contains(systemName); }
		int rank(QString systemName) { return userDataDb()->rank(systemName); }
		QString comment(QString systemName) { return userDataDb()->comment(systemName); }

		void clearCategoryNames();
		void clearVersionNames();
		XmlDatabaseManager *xmlDb() { return m_xmlDb; }
		UserDataDatabaseManager *userDataDb() { return m_userDataDb; }
		DatInfoDatabaseManager *datInfoDb() { return m_datInfoDb; }

		Gamelist(QObject *parent = 0);
		~Gamelist();

	public slots:
		void load();
		void verify(bool currentOnly = false);
		void loadCatverIni();
		void createVersionView();
		void loadCategoryIni();
		void createCategoryView();
		void loadFavorites();
		void saveFavorites();
		void loadPlayHistory();
		void savePlayHistory();
		QString status();

		// process management
		void loadStarted();
		void loadFinished(int, QProcess::ExitStatus);
		void loadReadyReadStandardOutput();
		void loadError(QProcess::ProcessError);
		void loadStateChanged(QProcess::ProcessState);
		void verifyStarted();
		void verifyFinished(int, QProcess::ExitStatus);
		void verifyReadyReadStandardOutput();

		// internal methods
		QString value(QString, QString, bool translate = false);
		void parse();
		void parseGameDetail(QTreeWidgetItem *);
		void insertAttributeItems(QTreeWidgetItem *, QString, QStringList, QStringList, bool translate = false);
		void insertAttributeItems(QList<QTreeWidgetItem *> *itemList, QString element, QStringList attributes, QStringList descriptions, bool translate = false);
		void enableWidgets(bool enable = true);
		void filter(bool initial = false);
		bool loadIcon(QString, QTreeWidgetItem *, bool checkOnly = false, QString *fileName = NULL);

	private:
		XmlDatabaseManager *m_xmlDb;
		UserDataDatabaseManager *m_userDataDb;
		DatInfoDatabaseManager *m_datInfoDb;
};

class GamelistItem : public QTreeWidgetItem
{
	public:
		GamelistItem(QTreeWidget *parentTreeWidget = 0) : QTreeWidgetItem(parentTreeWidget, QTreeWidgetItem::UserType) {}
		GamelistItem(QTreeWidgetItem *parentItem) : QTreeWidgetItem(parentItem, QTreeWidgetItem::UserType) {}

		virtual bool operator<(const QTreeWidgetItem &) const;
};

#endif
