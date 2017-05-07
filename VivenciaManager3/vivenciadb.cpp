#include "vivenciadb.h"
#include "global.h"

#include "client.h"
#include "job.h"
#include "payment.h"
#include "purchases.h"
#include "quickproject.h"
#include "standardprices.h"
#include "supplierrecord.h"
#include "userrecord.h"
#include "generaltable.h"
#include "companypurchases.h"
#include "inventory.h"
#include "dbsupplies.h"
#include "completerrecord.h"
#include "machinesrecord.h"
#include "dbcalendar.h"
#include "completers.h"
#include "fileops.h"
#include "backupdialog.h"
#include "vmcompress.h"
#include "fixdatabaseui.h"
#include "cleanup.h"
#include "stringrecord.h"
#include "heapmanager.h"
#include "textdb.h"
#include "vmtablewidget.h"
#include "vmnotify.h"
#include "keychain/passwordsessionmanager.h"
#include "crashrestore.h"

#include <QtSql/QSqlError>
#include <QtSql/QSqlResult>
#include <QStringMatcher>
#include <QTextStream>
#include <QThread>

VivenciaDB* VivenciaDB::s_instance ( nullptr );

static const QLatin1String chrDelim2 ( "\'," );
static const QString VMDB_PASSWORD_SERVICE ( QStringLiteral ( "vmdb_pswd_service" ) );
static const QString VMDB_PASSWORD_SERVER_ADMIN_ID ( QStringLiteral ( "vmdb_pswd_server_admin_id" ) );

void deleteVivenciaDBInstance ()
{
	heap_del ( VivenciaDB::s_instance );
}

const TABLE_INFO* VivenciaDB::table_info[TABLES_IN_DB] = {
	&generalTable::t_info,
	&userRecord::t_info,
	&Client::t_info,
	&Job::t_info,
	&Payment::t_info,
	&Buy::t_info,
	&quickProject::t_info,
	&dbSupplies::t_info,
	&servicesOffered::t_info,
	&supplierRecord::t_info,
	&companyPurchases::t_info,
	&Inventory::t_info,
	&completerRecord::t_info,
	&dbCalendar::t_info,
	&machinesRecord::t_info
};

static const uint PORT ( 3306 );

static const QString HOST ( QStringLiteral ( "localhost" ) );
static const QString CONNECTION_NAME ( QStringLiteral ( "vivenciadb" ) );
static const QString OLD_CONN_NAME ( QStringLiteral ( "vivencia" ) );

static const QString OLD_DB_NAME ( QLatin1String ( "Vivencia" ) );
static const QString OLD_USER_NAME ( QLatin1String ( "root" ) );

static const QString DB_DRIVER_NAME ( QStringLiteral ( "QMYSQL3" ) );
static const QString ROOT_CONNECTION ( QStringLiteral ( "root_connection" ) );

//-----------------------------------------STATIC---------------------------------------------
QString VivenciaDB::getTableColumnName ( const TABLE_INFO* __restrict t_info, uint column )
{
	int sep_1 ( 0 ) , sep_2 ( -1 );

	QString col_name;
	while ( ( sep_2 = t_info->field_names.indexOf ( CHR_PIPE, sep_1 ) ) != -1 )
	{
		if ( column-- == 0 )
		{
			col_name = t_info->field_names.mid ( sep_1, sep_2-sep_1 );
			break;
		}
		sep_1 = ( sep_2 ) + 1;
	}
	return col_name;
}

QString VivenciaDB::getTableColumnFlags ( const TABLE_INFO* __restrict t_info, uint column )
{
	int sep_1 ( 0 ), sep_2 ( -1 );

	QString col_flags ( emptyString );
	while ( ( sep_2 = t_info->field_flags.indexOf ( CHR_PIPE, sep_1 ) ) != -1 )
	{
		if ( column-- == 0 )
		{
			col_flags = t_info->field_flags.mid ( sep_1, sep_2-sep_1 );
			col_flags.chop ( 2 );
			break;
		}
		sep_1 = ( sep_2 ) + 1;
	}
	return col_flags;
}

QString VivenciaDB::getTableColumnLabel ( const TABLE_INFO* __restrict t_info, uint column )
{
	int sep_1 ( 0 ) , sep_2 ( -1 );

	QString col_label ( emptyString );
	while ( ( sep_2 = t_info->field_labels.indexOf ( CHR_PIPE, sep_1 ) ) != -1 )
	{
		if ( column-- == 0 )
		{
			col_label = t_info->field_labels.mid ( sep_1, sep_2 - sep_1 );
			break;
		}
		sep_1 = ( sep_2 ) + 1;
	}
	return col_label;
}

QString VivenciaDB::tableName ( const TABLE_ORDER table )
{	
	QString ret;
	switch ( table )
	{
		case TABLE_GENERAL_ORDER:
			ret = generalTable::t_info.table_name;
		break;
		case TABLE_USERS_ORDER:
			ret = userRecord::t_info.table_name;
		break;
		case TABLE_CLIENT_ORDER:
			ret = Client::t_info.table_name;
		break;
		case TABLE_JOB_ORDER:
			ret = Job::t_info.table_name;
		break;
		case TABLE_PAY_ORDER:
			ret = Payment::t_info.table_name;
		break;
		case TABLE_BUY_ORDER:
			ret = Buy::t_info.table_name;
		break;
		case TABLE_QP_ORDER:
			ret = quickProject::t_info.table_name;
		break;
		case TABLE_SUPPLIES_ORDER:
			ret = dbSupplies::t_info.table_name;
		break;
		case TABLE_SERVICES_ORDER:
			ret = servicesOffered::t_info.table_name;
		break;
		case TABLE_SUPPLIER_ORDER:
			ret = supplierRecord::t_info.table_name;
		break;
		case TABLE_CP_ORDER:
			ret = companyPurchases::t_info.table_name;
		break;
		case TABLE_INVENTORY_ORDER:
			ret = Inventory::t_info.table_name;
		break;
		case TABLE_COMPLETER_RECORDS_ORDER:
			ret = completerRecord::t_info.table_name;
		break;
		case TABLE_CALENDAR_ORDER:
			ret = dbCalendar::t_info.table_name;
		break;
		case TABLE_MACHINES_ORDER:
			ret = machinesRecord::t_info.table_name;
		break;
	}
	return ret;
}
//-----------------------------------------STATIC---------------------------------------------

VivenciaDB::VivenciaDB ()
	: lowest_id ( 0, TABLES_IN_DB + 1 ), highest_id ( 0, TABLES_IN_DB + 1 ),
	  m_ok ( false ), mNewDB ( false ), mBackupSynced ( true )
{
	openDataBase ();
	addPostRoutine ( deleteVivenciaDBInstance );
}

VivenciaDB::~VivenciaDB ()
{
	database ()->close ();
}

//-----------------------------------------READ-OPEN-LOAD-------------------------------------------
bool VivenciaDB::openDataBase ()
{
	if ( !database ()->isOpen () )
	{
		//TEST: do not provide a connection name so that this database becomes the default for the application (see docs)
		// This is so that any QSqlQuery object will automatically use VivenciaDatabase;
		m_db = QSqlDatabase::addDatabase ( DB_DRIVER_NAME );
		database ()->setHostName ( HOST );
		database ()->setPort ( PORT );
		database ()->setUserName ( USER_NAME );
		database ()->setPassword ( PASSWORD );
		database ()->setDatabaseName ( DB_NAME );
		m_ok = database ()->open ();
	}
	return m_ok;
}

void VivenciaDB::doPreliminaryWork ()
{	
	/* This piece of code was created and discarded at 2016/02/14. At this date I discovered that General table
	 * was empty, for some reason, and I needed to use an update function (in Payment) which would not be called
	 * for lack of database table version to compare. Fixed it. Code discarded.
	 * 
	 * UPDATE: this happend again and was noted at 2016/09/06. Cause unkown
	 * Noticed again at 2017/04/25. Cause unknown. UPDATE: there is no more need to use this commented out code
	 */
	/*if ( this->recordCount ( &gen_rec.t_info ) < TABLES_IN_DB ) {
		for ( uint i ( 0 ); i < TABLES_IN_DB; ++i )
			gen_rec.insertOrUpdate ( table_info[i] );
	}
	::exit ( 1 );
	return;*/
	
	if ( !mNewDB )
	{
		generalTable gen_rec;
		for ( uint i ( 1 ); i <= TABLES_IN_DB; ++i )
		{
			if ( gen_rec.readRecord ( i ) )
			{
				const QString& version ( recStrValue ( &gen_rec, FLD_GENERAL_TABLEVERSION ) );
				if ( !version.isEmpty () )
				{
					if ( table_info[i-1]->version == static_cast<unsigned char>( version.at ( 0 ).toLatin1 () ) )
						continue;
				}
			}
			if ( ( *table_info[i-1]->update_func ) () )
				gen_rec.insertOrUpdate ( table_info[i-1] );
		}
	}
}
//-----------------------------------------READ-OPEN-LOAD-------------------------------------------

//-----------------------------------------CREATE-DESTROY-MODIFY-------------------------------------
bool VivenciaDB::createDatabase ()
{
	if ( !openDataBase () )
	{
		m_ok = false;
		QString passwd;
		if ( APP_PSWD_MANAGER ()->getPassword_UserInteraction ( passwd, VMDB_PASSWORD_SERVICE, VMDB_PASSWORD_SERVER_ADMIN_ID,
			APP_TR_FUNC ( "You need to provide MYSQL's root(admin) password to create the application's database" ) ) )
		{
			QSqlDatabase dbRoot ( QSqlDatabase::addDatabase ( DB_DRIVER_NAME, ROOT_CONNECTION ) );
			dbRoot.setDatabaseName ( STR_MYSQL );
			dbRoot.setHostName ( HOST );
			dbRoot.setUserName ( QStringLiteral ( "root" ) );
			dbRoot.setPassword ( passwd );

			if ( dbRoot.open () )
			{
				const QString str_query ( QLatin1String ( "CREATE DATABASE " ) + DB_NAME +
									  QLatin1String ( " DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci" ) );
				QSqlQuery rootQuery ( dbRoot );
				m_ok = rootQuery.exec ( str_query );
				rootQuery.finish ();
				dbRoot.commit ();
				dbRoot.close ();
				rootQuery.clear ();
				if ( m_ok )
				{
					static_cast<void>( APP_PSWD_MANAGER ()->savePassword ( passwordSessionManager::SavePermanment,
						passwd, VMDB_PASSWORD_SERVICE, VMDB_PASSWORD_SERVER_ADMIN_ID ) );
				}
			}
			QSqlDatabase::removeDatabase ( ROOT_CONNECTION );
		}
	}
	if ( !m_ok )
		vmNotify::messageBox ( nullptr, APP_TR_FUNC ( "Database error" ), APP_TR_FUNC ( "MYSQL could create the database." ) );
	return m_ok;
}

bool VivenciaDB::createUser ()
{
	m_ok = false;
	QString passwd;
	if ( APP_PSWD_MANAGER ()->getPassword_UserInteraction ( passwd, VMDB_PASSWORD_SERVICE, VMDB_PASSWORD_SERVER_ADMIN_ID,
		APP_TR_FUNC ( "You need to provide MYSQL's root(admin) password to create the application's database" ) ) )
	{
		QSqlDatabase dbRoot ( QSqlDatabase::addDatabase ( DB_DRIVER_NAME, ROOT_CONNECTION ) );
		dbRoot.setDatabaseName ( STR_MYSQL );
		dbRoot.setHostName ( HOST );
		dbRoot.setUserName ( QStringLiteral ( "root" ) );
		dbRoot.setPassword ( passwd );

		if ( dbRoot.open () )
		{
			QString str_query ( QString ( QStringLiteral ( "CREATE USER %1@%2 IDENTIFIED BY '%3'" ) ).arg ( USER_NAME, HOST, PASSWORD ) );
			QSqlQuery rootQuery ( dbRoot );
			rootQuery.exec ( str_query );

			str_query = QString ( QStringLiteral ( "GRANT ALL ON %1.* TO '%2'@'%3';" ) ).arg ( DB_NAME, USER_NAME, HOST );
			m_ok = rootQuery.exec ( str_query );
			rootQuery.finish ();
			dbRoot.commit ();
			dbRoot.close ();
			rootQuery.clear ();
			if ( m_ok )
			{
				static_cast<void>( APP_PSWD_MANAGER ()->savePassword ( passwordSessionManager::SavePermanment,
					passwd, VMDB_PASSWORD_SERVICE, VMDB_PASSWORD_SERVER_ADMIN_ID ) );
			}
		}
		QSqlDatabase::removeDatabase ( ROOT_CONNECTION );

	}
	if ( !m_ok )
		vmNotify::messageBox ( nullptr, APP_TR_FUNC ( "Database error" ),
								APP_TR_FUNC ( "MYSQL could create the user for the application's database." ) );
	return m_ok;
}

bool VivenciaDB::databaseIsEmpty () const
{
	if ( m_ok )
	{
		QSqlQuery query ( QLatin1String ( "SHOW TABLES FROM " ) + DB_NAME, *database () );
		if ( query.exec () )
			return ( !query.isActive () || !query.next () );
	}
	return true;
}

bool VivenciaDB::createAllTables ()
{
	database ()->close ();
	if ( openDataBase () )
	{
		uint i ( 0 );
		do
		{
			createTable ( tableInfo ( i ) );
		} while ( ++i < TABLES_IN_DB );
		mNewDB = true;
		return true;
	}
	return false;
}

bool VivenciaDB::createTable ( const TABLE_INFO* t_info )
{
	if ( tableExists ( t_info ) )
		return true;

	QString cmd ( QLatin1String ( "CREATE TABLE " ) + t_info->table_name + CHR_L_PARENTHESIS );
	int sep_1[2] = { 0 };
	int sep_2[2] = { 0 };

	QString values;
	while ( ( sep_2[0] = t_info->field_names.indexOf ( CHR_PIPE, sep_1[0] ) ) != -1 )
	{
		values.append ( t_info->field_names.midRef ( sep_1[0], sep_2[0] - sep_1[0] ) );
		sep_1[0] = sep_2[0] + 1;

		sep_2[1] = t_info->field_flags.indexOf ( CHR_PIPE, sep_1[1] );
		values.append ( t_info->field_flags.midRef ( sep_1[1], sep_2[1]-sep_1[1] ) );
		sep_1[1] = sep_2[1] + 1;
	}
	values.append ( t_info->primary_key );
	cmd += values + CHR_R_PARENTHESIS + CHR_SPACE + t_info->table_flags;

	database ()->exec ( cmd );
	return ( database ()->lastError ().type () == QSqlError::NoError );
}

bool VivenciaDB::deleteDB ( const QString& dbname )
{
	QSqlQuery query ( QLatin1String ( "DROP DATABASE " ) + ( dbname.isEmpty () ? DB_NAME : dbname ), *database () );
	return query.exec ();
}

bool VivenciaDB::optimizeTable ( const TABLE_INFO* __restrict t_info ) const
{
	QSqlQuery query ( QLatin1String ( "OPTIMIZE LOCAL TABLE " ) + t_info->table_name, *database () );
	return query.exec ();
}

bool VivenciaDB::lockTable ( const TABLE_INFO* __restrict t_info ) const
{
	QSqlQuery query ( QLatin1String ( "LOCK TABLE " ) + t_info->table_name + QLatin1String ( "READ" ), *database () );
	return query.exec ();
}

bool VivenciaDB::unlockTable ( const TABLE_INFO* __restrict t_info ) const
{
	QSqlQuery query ( QLatin1String ( "UNLOCK TABLE " ) + t_info->table_name, *database () );
	return query.exec ();
}

bool VivenciaDB::unlockAllTables () const
{
	QSqlQuery query ( QStringLiteral ( "UNLOCK TABLES" ), *database () );
	return query.exec ();
}

// After a call to this function, must needs call unlockAllTables
bool VivenciaDB::flushAllTables () const
{
	QSqlQuery query ( QStringLiteral ( "FLUSH TABLES WITH READ LOCK" ), *database () );
	return query.exec ();
}

bool VivenciaDB::deleteTable ( const QString& table_name )
{
	QSqlQuery query ( QLatin1String ( "DROP TABLE " ) + table_name, *database () );
	return query.exec ();
}

bool VivenciaDB::clearTable ( const QString& table_name )
{
	QSqlQuery query ( QLatin1String ( "TRUNCATE TABLE " ) + table_name, *database () );
	return query.exec ();
}

bool VivenciaDB::tableExists ( const TABLE_INFO* __restrict t_info )
{
	QSqlQuery query ( QLatin1String ( "SHOW TABLES LIKE '" ) + t_info->table_name + CHR_CHRMARK, *database () );
	if ( query.exec () && query.first () )
		return query.value ( 0 ).toString () == t_info->table_name;
	return false;
}

bool VivenciaDB::insertColumn ( const uint column, const TABLE_INFO* __restrict t_info )
{
	const QString cmd ( QLatin1String ( "ALTER TABLE " ) + t_info->table_name +
						QLatin1String ( " ADD COLUMN " ) + getTableColumnName ( t_info, column ) +
						getTableColumnFlags ( t_info, column ) + ( column > 0 ?
								QString ( QLatin1String ( " AFTER " ) + getTableColumnName ( t_info, column - 1 ) ) :
								QLatin1String ( " FIRST" ) )
					  );
	QSqlQuery query ( cmd, *database () );
	if ( query.exec () ) {
		mBackupSynced = false;
		return true;
	}
	return false;
}

bool VivenciaDB::removeColumn ( const QString& column_name, const TABLE_INFO* t_info )
{
	const QString cmd ( QLatin1String ( "ALTER TABLE " ) + t_info->table_name +
						QLatin1String ( " DROP COLUMN " ) + column_name );
	QSqlQuery query ( cmd, *database () );
	if ( query.exec () )
	{
		mBackupSynced = false;
		return true;
	}
	return false;
}

bool VivenciaDB::renameColumn ( const QString& old_column_name, const uint col_idx, const TABLE_INFO* __restrict t_info )
{
	const QString cmd ( QLatin1String ( "ALTER TABLE " ) + t_info->table_name +
						QLatin1String ( " CHANGE " ) + old_column_name +
						getTableColumnName ( t_info, col_idx ) + CHR_SPACE + getTableColumnFlags ( t_info, col_idx ) );
	QSqlQuery query ( cmd, *database () );
	if ( query.exec () )
	{
		mBackupSynced = false;
		return true;
	}
	return false;
}

bool VivenciaDB::changeColumnProperties ( const uint column, const TABLE_INFO* __restrict t_info )
{
	const QString cmd ( QLatin1String ( "ALTER TABLE " ) + t_info->table_name +
						QLatin1String ( " MODIFY COLUMN " ) + getTableColumnName ( t_info, column ) +
						getTableColumnFlags ( t_info, column ) );
	QSqlQuery query ( cmd, *database () );
	if ( query.exec () )
	{
		mBackupSynced = false;
		return true;
	}
	return false;
}

uint VivenciaDB::recordCount ( const TABLE_INFO* __restrict t_info ) const
{
	QSqlQuery query ( QLatin1String ( "SELECT COUNT(*) FROM " ) + t_info->table_name, *database () );
	if ( query.exec () && query.first () )
		return query.value ( 0 ).toUInt ();
	return 0;
}

void VivenciaDB::populateTable ( const DBRecord* db_rec, vmTableWidget* table ) const
{
	QThread* workerThread ( new QThread );
	threadedDBOps* worker ( new threadedDBOps );
	
	worker->setCallbackForFinished ( [&] () { workerThread->quit (); return worker->deleteLater (); } );
	worker->moveToThread ( workerThread );
	worker->connect ( workerThread, &QThread::started, worker, [&,db_rec,table] () { return worker->populateTable ( db_rec, table ); } );
	worker->connect ( workerThread, &QThread::finished, workerThread, [&] () { return workerThread->deleteLater(); } );
	
	workerThread->start ();
}
//-----------------------------------------CREATE-DESTROY-MODIFY-------------------------------------

//-----------------------------------------COMMOM-DBRECORD-------------------------------------------
bool VivenciaDB::insertRecord ( const DBRecord* db_rec ) const
{
	QString str_query ( QLatin1String ( "INSERT INTO " ) + db_rec->t_info->table_name + QLatin1String ( " VALUES ( " ) );
	QString values;

	const uint field_count ( db_rec->t_info->field_count );
	for ( uint i ( 0 ); i < field_count; ++i )
		values += CHR_CHRMARK + recStrValue ( db_rec, i ) + chrDelim2;
	values.chop ( 1 );
	
	str_query += values + QLatin1String ( " )" );
	database ()->exec ( str_query );
	if ( database ()->lastError ().type () == QSqlError::NoError )
	{
		mBackupSynced = false;
		return true;
	}
	return false;
}

bool VivenciaDB::updateRecord ( const DBRecord* db_rec ) const
{
	if ( !db_rec || !db_rec->isModified () )
		return true;

	QString str_query ( QLatin1String ( "UPDATE " ) +
						db_rec->t_info->table_name + QLatin1String ( " SET " ) );

	int sep_1 ( ( db_rec->t_info->field_names.indexOf ( CHR_PIPE, 1 ) ) + 1 ); // skip ID field
	int sep_2 ( 0 );

	const uint field_count ( db_rec->t_info->field_count );
	const QLatin1String chrDelim ( "=\'" );
	const QChar CHR_PIPE_PTR ( CHR_PIPE );
	const QStringMatcher sep_matcher ( &CHR_PIPE_PTR, 1 );

	for ( uint i ( 1 ); i < field_count; ++i )
	{
		sep_2 = sep_matcher.indexIn ( db_rec->t_info->field_names, sep_1 );
		if ( db_rec->isModified ( i ) )
		{
			str_query += db_rec->t_info->field_names.midRef ( sep_1, sep_2 - sep_1 ) +
						 chrDelim + recStrValue ( db_rec, i ) + chrDelim2;
		}
		sep_1 = sep_2 + 1;
	}
	str_query.chop ( 1 );
	str_query += ( QLatin1String ( " WHERE ID='" ) + db_rec->actualRecordStr ( 0 ) + CHR_CHRMARK ); // id ()

	database ()->exec ( str_query );

	if ( database ()->lastError ().type () == QSqlError::NoError )
	{
		mBackupSynced = false;
		return true;
	}
	return false;
}

bool VivenciaDB::removeRecord ( const DBRecord* db_rec ) const
{
	if ( db_rec->actualRecordInt ( 0 ) >= 0 )
	{
		const QString str_query ( QLatin1String ( "DELETE FROM " ) + db_rec->t_info->table_name +
								  QLatin1String ( " WHERE ID='" ) + db_rec->actualRecordStr ( 0 ) + CHR_CHRMARK );
		database ()->exec ( str_query );
		
		// The deletion of a temporary record must decrease the highest id for the table in orer to try to minimize
		// useless ids
		const uint id ( static_cast<uint>( db_rec->actualRecordInt ( 0 ) ) );		
		if ( id >= getHighestID ( db_rec->t_info->table_order ) )
			const_cast<VivenciaDB*> ( this )->setHighestID ( db_rec->t_info->table_order, id - 1 );

		if ( database ()->lastError ().type () == QSqlError::NoError )
		{
			mBackupSynced = false;
			return true;
		}
	}
	return false;
}

void VivenciaDB::loadDBRecord ( DBRecord* db_rec, const QSqlQuery* const query )
{
	for ( uint i ( 0 ); i < db_rec->t_info->field_count; ++i )
		setRecValue ( db_rec, i, query->value ( static_cast<int>(i) ).toString () );
}

bool VivenciaDB::insertDBRecord ( DBRecord* db_rec )
{
	const uint table_order ( db_rec->t_info->table_order );
	const bool bOutOfOrder ( db_rec->recordInt ( 0 ) >= 1 );
	const uint new_id ( bOutOfOrder ? static_cast<uint>(db_rec->recordInt ( 0 )) : getNextID ( table_order ) );

	db_rec->setIntValue ( 0, static_cast<int>(new_id) );
	db_rec->setValue ( 0, QString::number ( new_id ) );
	const bool ret ( insertRecord ( db_rec ) );
	if ( ret )
		highest_id[table_order] = new_id;
	return ret;
}

bool VivenciaDB::getDBRecord ( DBRecord* db_rec, const uint field, const bool load_data )
{
	/* I've met with a few instances when the program would crash without a seemingly good reason and when
	 * the mysql server was being updated or stopped for some reason. When those happend, the program's execution pointer
	 * was here, execing the query.
	 */
	try
	{
		const QString cmd ( QLatin1String ( "SELECT * FROM " ) +
						db_rec->t_info->table_name + QLatin1String ( " WHERE " ) +
						getTableColumnName ( db_rec->t_info, field ) + CHR_EQUAL +
						db_rec->backupRecordStr ( field )
					  );

		QSqlQuery query ( *database () );
		query.setForwardOnly ( true );
		query.exec ( cmd );
		if ( query.first () )
		{
			if ( load_data )
				loadDBRecord ( db_rec, &query );
			/* For the SELECT statement have some use, at least save the read id for later use.
			 * Save all the initial id fields. Fast table lookup and fast item lookup, that's
			 * the goal of this part of the code
			*/
			uint i ( 0 );
			while ( db_rec->fieldType ( i ) == DBTYPE_ID )
			{
				db_rec->setIntValue ( i, query.value ( static_cast<int>(i) ).toInt () );
				i++;
			}
			return true;
		}
	}
	catch ( const std::runtime_error& e )
	{
		qDebug () << "A runtime exception was caught with message: \"" << e.what () << "\"";
		APP_RESTORER ()->saveSession ();
	}
	catch (const std::exception& e)
	{
		qDebug () << "A standard exception was caught with message: \"" << e.what () << "\"";
		APP_RESTORER ()->saveSession ();
	}
	
	return false;
}

bool VivenciaDB::getDBRecord ( DBRecord* db_rec, DBRecord::st_Query& stquery, const bool load_data )
{
	/* I've met with a few instances when the program would crash without a seemingly good reason and when
	 * the mysql server was being updated or stopped for some reason. When those happend, the program's execution pointer
	 * was here, execing the query.
	 */
	try
	{
		
		bool ret ( false );

		if ( stquery.reset )
		{
			stquery.reset = false;
			if ( !stquery.query )
				stquery.query = new QSqlQuery ( *database () );
			else
				stquery.query->finish ();

			stquery.query->setForwardOnly ( stquery.forward );

			if ( stquery.str_query.isEmpty () )
			{
				const QString cmd ( QLatin1String ( "SELECT * FROM " ) + db_rec->t_info->table_name +
								( stquery.field >= 0 ? QLatin1String ( " WHERE " ) +
								  getTableColumnName ( db_rec->t_info, static_cast<uint>(stquery.field) ) + CHR_EQUAL +
								  CHR_QUOTES + stquery.search + CHR_QUOTES : emptyString )
							  );
				ret = stquery.query->exec ( cmd );
			}
			else
				ret = stquery.query->exec ( stquery.str_query );

			if ( ret )
				ret = stquery.forward ? stquery.query->first () : stquery.query->last ();
		}
		else
			ret = stquery.forward ? stquery.query->next () : stquery.query->previous ();

		if ( ret )
		{
			if ( load_data )
				loadDBRecord ( db_rec, stquery.query );
	
			uint i ( 0 ); // save first id fields. Useful for subsequent queries or finding related vmListItems
			do
			{
				if ( db_rec->fieldType ( i ) == DBTYPE_ID )
					db_rec->setIntValue ( i, stquery.query->value ( static_cast<int>(i) ).toInt () );
				else
					break;
			} while ( ++i < db_rec->fieldCount () );
			setRecValue ( db_rec, 0, stquery.query->value ( 0 ).toString () );
			return true;
		}
	}
	catch ( const std::runtime_error& e )
	{
		qDebug () << "A runtime exception was caught with message: \"" << e.what () << "\"";
		APP_RESTORER ()->saveSession ();
	}
	catch (const std::exception& e)
	{
		qDebug () << "A standard exception was caught with message: \"" << e.what () << "\"";
		APP_RESTORER ()->saveSession ();
	}
	
	return false;
}

uint VivenciaDB::getHighestID ( const uint table ) const
{
	if ( highest_id.at ( table ) == highest_id.defaultValue () )
	{
		QSqlQuery query;
		if ( runQuery ( QLatin1String ( "SELECT MAX(ID) FROM " ) + tableInfo ( table )->table_name, query ) )
		{
			if ( recordCount ( tableInfo ( table ) ) > 0 )
			{
				const_cast<VivenciaDB*>( this )->setHighestID ( table, query.value ( 0 ).toUInt () );
			}
		}
	}
	return highest_id.at ( table );
}

uint VivenciaDB::getLowestID ( const uint table ) const
{
	if ( lowest_id.at ( table ) == lowest_id.defaultValue () )
	{
		QSqlQuery query;
		if ( runQuery ( QLatin1String ( "SELECT MIN(ID) FROM " ) + tableInfo ( table )->table_name, query ) )
		{
			if ( recordCount ( tableInfo ( table ) ) > 0 )
			{
				const_cast<VivenciaDB*>( this )->setLowestID ( table, query.value ( 0 ).toUInt () );
			}
		}
	}
	return lowest_id.at ( table );
}

uint VivenciaDB::getNextID ( const uint table )
{
	const uint next_id ( getHighestID ( table ) + 1 );
	setHighestID ( table, next_id );
	return next_id;
}

bool VivenciaDB::recordExists ( const QString& table_name, const int id ) const
{
	if ( id >= 0 )
	{
		QSqlQuery query ( *database () );
		if ( query.exec ( QString ( QStringLiteral ( "SELECT `ID` FROM %1 WHERE `ID`=%2" ) ).arg (
							  table_name. QString::number ( id ) ) ) )
			return ( query.isActive () && query.next () );
	}
	return false;
}

bool VivenciaDB::runQuery ( const QString& query_cmd, QSqlQuery& queryRes ) const
{
	if ( m_ok && !query_cmd.isEmpty () )
	{
		queryRes.setForwardOnly ( true );
		if ( queryRes.exec ( query_cmd ) )
			return queryRes.first ();
	}
	return false;
}
//-----------------------------------------COMMOM-DBRECORD-------------------------------------------

//-----------------------------------------IMPORT-EXPORT--------------------------------------------
bool VivenciaDB::doBackup ( const QString& filepath, const QString& tables, BackupDialog* __restrict bDlg )
{
	QString passwd;
	if ( APP_PSWD_MANAGER ()->getPassword_UserInteraction ( passwd, VMDB_PASSWORD_SERVICE, VMDB_PASSWORD_SERVER_ADMIN_ID,
			APP_TR_FUNC ( "You need to provide MYSQL's root(admin) password password for backup operations" ) ) )
	{			
		flushAllTables ();
		BackupDialog::incrementProgress ( bDlg ); //2
		const QString mysqldump_args ( QLatin1String ( "--user=root --password=" ) +
			   passwd + CHR_SPACE + DB_NAME + CHR_SPACE + tables );

		const QString dump ( fileOps::executeAndCaptureOutput ( mysqldump_args, backupApp () ) );
		unlockAllTables ();
		BackupDialog::incrementProgress ( bDlg ); //3
		if ( !dump.isEmpty () )
		{
			QFile file ( filepath );
			if ( file.open ( QIODevice::WriteOnly | QIODevice::Text ) )
			{
				QTextStream out ( &file );
				out.setCodec ( "UTF-8" );
				out.setAutoDetectUnicode ( true );
				out.setLocale ( QLocale::system () );
				QString strHeader ( dump.left ( 500 ) );
				strHeader.replace ( QStringLiteral ( "SET NAMES latin1" ), QStringLiteral ( "SET NAMES utf8" ) );
				strHeader.append ( dump.midRef ( 500, dump.length () - 500 ) );
				out << strHeader.toLocal8Bit ();
				file.close ();
				BackupDialog::incrementProgress ( bDlg ); //4
				mBackupSynced = true;
				static_cast<void>( APP_PSWD_MANAGER ()->savePassword ( passwordSessionManager::SavePermanment, passwd, VMDB_PASSWORD_SERVICE, VMDB_PASSWORD_SERVER_ADMIN_ID ) );
				return true;
			}
		}
	}
	return false;
}

/* Returns true if dumpfile is of the same version the program supports
   Returns false if there is any mismatch
   In the future, I might do a per table analysis and use mysqlimport
 */
bool VivenciaDB::checkDumpFile ( const QString& dumpfile )
{
	bool ok ( false );
	if ( m_ok )
	{
		QFile file ( dumpfile );
		if ( file.open ( QIODevice::ReadOnly|QIODevice::Text ) )
		{
			char* __restrict line_buf ( new char [8*1024] );// OPT-6
			qint64 n_chars ( -1 );
			QString line, searchedExpr;
			const QString expr ( QStringLiteral ( "(%1,%2,%3," ) );

			do
			{
				n_chars = file.readLine ( line_buf, 8*1024 );
				if ( n_chars >= 500 )
				{ // The first time ths function was created, the searched line had 509 characters
					line = QString::fromUtf8 ( line_buf, static_cast<int>(strlen ( line_buf ) - 1) );
					if ( line.contains ( QStringLiteral ( "INSERT INTO `GENERAL`" ) ) )
					{
						ok = true; // so far, it is
						for ( uint i ( 0 ); i < TABLES_IN_DB; ++i )
						{
							searchedExpr = expr.arg ( table_info[i]->table_order ).arg ( table_info[i]->table_name ).arg ( table_info[i]->version );
							if ( !line.contains ( searchedExpr ) )
							{ // one single mismatch the dump file cannot be used by mysqlimport
								ok = false;
								break;
							}
						}
					}
				}
			} while ( n_chars != -1 );
			delete [] line_buf;
			file.close ();
		}
	}
	return ok;
}

bool VivenciaDB::doRestore (const QString& filepath, BackupDialog* __restrict bDlg )
{
	QString passwd;
	if ( APP_PSWD_MANAGER ()->getPassword_UserInteraction ( passwd, VMDB_PASSWORD_SERVICE, VMDB_PASSWORD_SERVER_ADMIN_ID,
			APP_TR_FUNC ( "You need to provide MYSQL's root(admin) password to restore from file" ) ) )
	{

		QString uncompressed_file ( QStringLiteral ( "/tmp/vmdb_tempfile" ) );

		const bool bSourceCompressed ( VMCompress::isCompressed ( filepath ) );
		if ( bSourceCompressed )
		{
			fileOps::removeFile ( uncompressed_file );
			VMCompress::decompress ( filepath, uncompressed_file );
		}
		else
			uncompressed_file = filepath;

		BackupDialog::incrementProgress ( bDlg ); //3
		QString restoreapp_args;
		bool ret ( true );

		//hopefully, by now, Data will have intermediated well the situation, having all the pieces of the program
		// aware of the changes that might occur.
		database ()->close ();

		// use a previous dump from mysqldump to create a new database.
		// Note that DB_NAME must refer to an unexisting database or this will fail
		// Also I think the dump must be of a complete database, with all tables, to avoid inconsistencies
		if ( !checkDumpFile ( uncompressed_file ) )
		{ // some table version mismatch
			restoreapp_args = QLatin1String ( "--user=root --default_character_set=utf8 --password=" ) + passwd + CHR_SPACE + DB_NAME;
			ret = fileOps::executeWithFeedFile ( restoreapp_args, importApp (), uncompressed_file );
		}
		// use a previous dump from mysqldump to update or replace one or more tables in an existing database
		// Tables will be overwritten or created. Note that DB_NAME must refer to an existing database
		else
		{
			restoreapp_args = QLatin1String ( "--user=root --password=" ) + passwd + CHR_SPACE + DB_NAME + CHR_SPACE + uncompressed_file;
			ret = fileOps::executeWait ( restoreapp_args, restoreApp () );
		}
		BackupDialog::incrementProgress ( bDlg ); //4

		if ( bSourceCompressed ) // do not leave trash behind
			fileOps::removeFile ( uncompressed_file );

		if ( ret )
		{
			mBackupSynced = true;
			static_cast<void>( APP_PSWD_MANAGER ()->savePassword ( passwordSessionManager::SavePermanment, passwd, VMDB_PASSWORD_SERVICE, VMDB_PASSWORD_SERVER_ADMIN_ID ) );
			return openDataBase ();
		}
	}
	return false;
}

bool VivenciaDB::importFromCSV ( const QString& filename, BackupDialog* __restrict bDlg )
{
	dataFile* tdb ( new dataFile ( filename ) );
	if ( !tdb->open () )
		return false;
	tdb->setRecordSeparationChar ( CHR_NEWLINE );
	if ( !tdb->load ().isOn () )
		return false;

	BackupDialog::incrementProgress ( bDlg ); //3

	stringRecord rec;
	rec.setFieldSeparationChar ( CHR_SEMICOLON );
	if ( !tdb->getRecord ( rec, 0 ) )
		return false;

	const int table_id  ( rec.fieldValue ( 1 ).toInt () );
	DBRecord* db_rec ( nullptr );
	switch ( table_id )
	{
		case GENERAL_TABLE:
			db_rec = new generalTable;
		break;
		case USERS_TABLE:
			db_rec = new userRecord;
		break;
		case CLIENT_TABLE:
			db_rec = new Client;
		break;
		case JOB_TABLE:
			db_rec = new Job;
		break;
		case PAYMENT_TABLE:
			db_rec = new Payment;
		break;
		case PURCHASE_TABLE:
			db_rec = new Buy;
		break;
		case QUICK_PROJECT_TABLE:
			db_rec = new quickProject;
		break;
		case INVENTORY_TABLE:
			db_rec = new Inventory;
		break;
		case SERVICES_TABLE:
			db_rec = new servicesOffered;
		break;
		case SUPPLIER_TABLE:
			db_rec = new supplierRecord;
		break;
		case COMPANY_PURCHASES_TABLE:
			db_rec = new companyPurchases;
		break;
		case SUPPLIES_TABLE:
			db_rec = new dbSupplies;
		break;
		case COMPLETER_RECORDS_TABLE:
			db_rec = new completerRecord;
		break;
	}

	BackupDialog::incrementProgress ( bDlg ); //4
	bool bSuccess ( false );
	//we don't import if there is a version mismatch
	if ( rec.fieldValue ( 0 ).isEmpty () )
		 return false;
	if ( rec.fieldValue ( 0 ).at ( 0 ) == db_rec->t_info->version )
	{
		uint fld ( 1 );
		int idx ( 1 );
		do
		{
			if ( !tdb->getRecord ( rec, idx ) )
				break;
			if ( rec.first () )
			{
				setRecValue ( db_rec, 0, rec.curValue () );
				do
				{
					if ( rec.next () )
						setRecValue ( db_rec, fld++, rec.curValue () );
					else
						break;
				} while ( true );
			}
			db_rec->saveRecord ();
			++idx;
		} while ( true );
		if ( idx >= 2 )
			bSuccess = true;
		delete db_rec;
	}

	BackupDialog::incrementProgress ( bDlg ); //5
	delete tdb;
	return bSuccess;
}

bool VivenciaDB::exportToCSV ( const uint table, const QString& filename, BackupDialog* __restrict bDlg )
{
	const TABLE_INFO* t_info ( nullptr );
	switch ( table )
	{
		case GENERAL_TABLE:
			t_info = &generalTable::t_info;
		break;
		case USERS_TABLE:
			t_info = &userRecord::t_info;
		break;
		case CLIENT_TABLE:
			t_info = &Client::t_info;
		break;
		case JOB_TABLE:
			t_info = &Job::t_info;
		break;
		case PAYMENT_TABLE:
			t_info = &Payment::t_info;
		break;
		case PURCHASE_TABLE:
			t_info = &Buy::t_info;
		break;
		case QUICK_PROJECT_TABLE:
			t_info = &quickProject::t_info;
		break;
		case INVENTORY_TABLE:
			t_info = &Inventory::t_info;
		break;
		case SERVICES_TABLE:
			t_info = &servicesOffered::t_info;
		break;
		case SUPPLIER_TABLE:
			t_info = &supplierRecord::t_info;
		break;
		case COMPANY_PURCHASES_TABLE:
			t_info = &companyPurchases::t_info;
		break;
		case SUPPLIES_TABLE:
			t_info = &dbSupplies::t_info;
		break;
		case COMPLETER_RECORDS_TABLE:
			t_info = &completerRecord::t_info;
		break;
	}
	static_cast<void>(fileOps::removeFile ( filename ));
	dataFile* __restrict tdb ( new dataFile ( filename ) );
	if ( !tdb->open () )
		return false;
	tdb->setRecordSeparationChar ( CHR_NEWLINE, CHR_PIPE );
	BackupDialog::incrementProgress ( bDlg ); //2

	stringRecord rec;
	rec.setFieldSeparationChar ( CHR_PIPE );
	rec.fastAppendValue ( QString::number ( t_info->table_id ) ); //Record table_id information
	rec.fastAppendValue ( QString::number ( t_info->version, 'f', 1 ) ); //Record version information
	tdb->insertRecord ( 0, rec );
	BackupDialog::incrementProgress ( bDlg ); //3

	const QString cmd ( QLatin1String ( "SELECT * FROM " ) + t_info->table_name );

	QSqlQuery query ( *database () );
	query.setForwardOnly ( true );
	query.exec ( cmd );

	if ( query.first () )
	{
		uint pos ( 1 ), i ( 0 );
		do
		{
			rec.clear ();
			for ( i = 0; i < t_info->field_count; ++i )
				rec.fastAppendValue ( query.value ( static_cast<int>(i) ).toString () );
			tdb->insertRecord ( static_cast<int>(pos), rec );
			++pos;
		} while ( query.next () );
	}

	BackupDialog::incrementProgress ( bDlg ); //4

	const uint n ( tdb->recCount () );
	tdb->commit ();
	delete tdb;

	BackupDialog::incrementProgress ( bDlg ); //5
	return ( n > 0 );
}
//-----------------------------------------IMPORT-EXPORT--------------------------------------------

threadedDBOps::threadedDBOps () : QObject ( nullptr ), m_finishedFunc ( nullptr ) {}
threadedDBOps::~threadedDBOps () {}

void threadedDBOps::populateTable ( const DBRecord* db_rec, vmTableWidget* table )
{
	const QString cmd ( QLatin1String ( "SELECT * FROM " ) + db_rec->t_info->table_name );

	QSqlQuery query ( *( VDB ()->database () ) );
	query.setForwardOnly ( true );
	query.exec ( cmd );
	if ( !query.first () ) return;

	uint row ( 0 );
	do
	{
		for ( uint col ( 0 ); col < db_rec->t_info->field_count; col++)
			table->sheetItem ( row, col )->setText ( query.value ( static_cast<int>( col ) ).toString (), false, false );
		if ( static_cast<int>( ++row ) >= table->totalsRow () )
				table->appendRow ();
		if ( ( row % 100 ) == 0 ) // process pending events every 100 rows
			qApp->processEvents ();
	} while ( query.next () );

	if ( m_finishedFunc )
		m_finishedFunc ();
}
