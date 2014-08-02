#include <QApplication>
#include <QDateTime>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QUuid>

#include "macros.h"
#include "settings.h"
#include "checksumdbmgr.h"

// external global variables
extern Settings *qmc2Config;

CheckSumDatabaseManager::CheckSumDatabaseManager(QObject *parent)
	: QObject(parent)
{
	QString userScopePath = QMC2_DYNAMIC_DOT_PATH;
	m_connectionName = QString("checksum-db-connection-%1").arg(QUuid::createUuid().toString());
	m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
	QString variantName = QMC2_VARIANT_NAME.toLower();
	m_db.setDatabaseName(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/CheckSumDbDatabasePath", QString(userScopePath + "/%1-checksum.db").arg(variantName)).toString());
	m_tableBasename = QString("%1_checksum").arg(variantName.replace("-", "_"));
	if ( m_db.open() ) {
		QStringList tables = m_db.driver()->tables(QSql::Tables);
		if ( tables.count() != 2 || !tables.contains(m_tableBasename) || !tables.contains(QString("%1_metadata").arg(m_tableBasename)) )
			recreateDatabase();
	} else
		emit log(tr("WARNING: failed to open check-sum database '%1': error = '%2'").arg(m_db.databaseName()).arg(m_db.lastError().text()));
}

CheckSumDatabaseManager::~CheckSumDatabaseManager()
{
	if ( m_db.isOpen() )
		m_db.close();
}

QString CheckSumDatabaseManager::qmc2Version()
{
	QString qmc2_version;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT qmc2_version FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( query.first() )
			qmc2_version = query.value(0).toString();
		query.finish();
	} else
		emit log(tr("WARNING: failed to fetch '%1' from check-sum database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
	return qmc2_version;
}

void CheckSumDatabaseManager::setQmc2Version(QString qmc2_version)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT qmc2_version FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( !query.next() ) {
			query.finish();
			query.prepare(QString("INSERT INTO %1_metadata (qmc2_version, row) VALUES (:qmc2_version, 0)").arg(m_tableBasename));
			query.bindValue(":qmc2_version", qmc2_version);
			if ( !query.exec() )
				emit log(tr("WARNING: failed to add '%1' to check-sum database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1_metadata SET qmc2_version=:qmc2_version WHERE row=0").arg(m_tableBasename));
			query.bindValue(":qmc2_version", qmc2_version);
			if ( !query.exec() )
				emit log(tr("WARNING: failed to update '%1' in check-sum database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
		}
		query.finish();
	} else
		emit log(tr("WARNING: failed to fetch '%1' from check-sum database: query = '%2', error = '%3'").arg("qmc2_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
}

int CheckSumDatabaseManager::checkSumDbVersion()
{
	int checksum_db_version = -1;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT checksum_db_version FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( query.first() )
			checksum_db_version = query.value(0).toInt();
		query.finish();
	} else
		emit log(tr("WARNING: failed to fetch '%1' from check-sum database: query = '%2', error = '%3'").arg("checksum_db_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
	return checksum_db_version;
}

void CheckSumDatabaseManager::setCheckSumDbVersion(int checksum_db_version)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT checksum_db_version FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( !query.next() ) {
			query.finish();
			query.prepare(QString("INSERT INTO %1_metadata (checksum_db_version, row) VALUES (:checksum_db_version, 0)").arg(m_tableBasename));
			query.bindValue(":checksum_db_version", checksum_db_version);
			if ( !query.exec() )
				emit log(tr("WARNING: failed to add '%1' to check-sum database: query = '%2', error = '%3'").arg("checksum_db_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1_metadata SET checksum_db_version=:checksum_db_version WHERE row=0").arg(m_tableBasename));
			query.bindValue(":checksum_db_version", checksum_db_version);
			if ( !query.exec() )
				emit log(tr("WARNING: failed to update '%1' in check-sum database: query = '%2', error = '%3'").arg("checksum_db_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
		}
		query.finish();
	} else
		emit log(tr("WARNING: failed to fetch '%1' from check-sum database: query = '%2', error = '%3'").arg("checksum_db_version").arg(query.lastQuery()).arg(m_db.lastError().text()));
}

uint CheckSumDatabaseManager::scanTime()
{
	uint scan_time = 0;
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT scan_time FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( query.first() )
			scan_time = query.value(0).toUInt();
		query.finish();
	} else
		emit log(tr("WARNING: failed to fetch '%1' from check-sum database: query = '%2', error = '%3'").arg("scan_time").arg(query.lastQuery()).arg(m_db.lastError().text()));
	return scan_time;
}

void CheckSumDatabaseManager::setScanTime(uint scan_time)
{
	QSqlQuery query(m_db);
	query.prepare(QString("SELECT scan_time FROM %1_metadata WHERE row=0").arg(m_tableBasename));
	if ( query.exec() ) {
		if ( !query.next() ) {
			query.finish();
			query.prepare(QString("INSERT INTO %1_metadata (scan_time, row) VALUES (:scan_time, 0)").arg(m_tableBasename));
			query.bindValue(":scan_time", scan_time);
			if ( !query.exec() )
				emit log(tr("WARNING: failed to add '%1' to check-sum database: query = '%2', error = '%3'").arg("scan_time").arg(query.lastQuery()).arg(m_db.lastError().text()));
		} else {
			query.finish();
			query.prepare(QString("UPDATE %1_metadata SET scan_time=:scan_time WHERE row=0").arg(m_tableBasename));
			query.bindValue(":scan_time", scan_time);
			if ( !query.exec() )
				emit log(tr("WARNING: failed to update '%1' in check-sum database: query = '%2', error = '%3'").arg("scan_time").arg(query.lastQuery()).arg(m_db.lastError().text()));
		}
		query.finish();
	} else
		emit log(tr("WARNING: failed to fetch '%1' from check-sum database: query = '%2', error = '%3'").arg("scan_time").arg(query.lastQuery()).arg(m_db.lastError().text()));
}

int CheckSumDatabaseManager::checkSumRowCount()
{
	QSqlQuery query(m_db);
	if ( query.exec(QString("SELECT COUNT(*) FROM %1").arg(m_tableBasename)) ) {
		if ( query.first() )
			return query.value(0).toInt();
		else
			return -1;
	} else {
		emit log(tr("WARNING: failed to fetch row count from check-sum database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(m_db.lastError().text()));
	}	return -1;
}

void CheckSumDatabaseManager::recreateDatabase()
{
	QSqlQuery query(m_db);
	if ( !query.exec(QString("DROP INDEX IF EXISTS %1_index").arg(m_tableBasename)) ) {
		emit log(tr("WARNING: failed to remove check-sum database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(m_db.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("DROP TABLE IF EXISTS %1").arg(m_tableBasename)) ) {
		emit log(tr("WARNING: failed to remove check-sum database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(m_db.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("DROP TABLE IF EXISTS %1_metadata").arg(m_tableBasename)) ) {
		emit log(tr("WARNING: failed to remove check-sum database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(m_db.lastError().text()));
		return;
	}
	query.finish();
	// vaccum'ing the database frees all disk-space previously used
	query.exec("VACUUM");
	query.finish();
	if ( !query.exec(QString("CREATE TABLE %1 (sha1 TEXT, crc TEXT, path TEXT, PRIMARY KEY (sha1, crc), CONSTRAINT %1_unique_checksums UNIQUE (sha1, crc))").arg(m_tableBasename)) ) {
		emit log(tr("WARNING: failed to create check-sum database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(m_db.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("CREATE INDEX %1_index ON %1 (sha1, crc)").arg(m_tableBasename)) ) {
		emit log(tr("WARNING: failed to create check-sum database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(m_db.lastError().text()));
		return;
	}
	query.finish();
	if ( !query.exec(QString("CREATE TABLE %1_metadata (row INTEGER PRIMARY KEY, scan_time UNSIGNED BIG INT, qmc2_version TEXT, checksum_db_version INTEGER)").arg(m_tableBasename)) ) {
		emit log(tr("WARNING: failed to create check-sum database: query = '%1', error = '%2'").arg(query.lastQuery()).arg(m_db.lastError().text()));
		return;
	}
	setScanTime(QDateTime::currentDateTime().toTime_t());
	emit log(tr("check-sum database '%1' initialized").arg(databasePath()));
}
