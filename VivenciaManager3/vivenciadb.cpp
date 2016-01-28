#include "vivenciadb.h"
#include "global.h"
#include "client.h"
#include "job.h"
#include "data.h"
#include "payment.h"
#include "purchases.h"
#include "usermanagement.h"
#include "quickproject.h"
#include "standardprices.h"
#include "supplierrecord.h"
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
#include "passwordmanager.h"
#include "vmnotify.h"

#include <QStringMatcher>
#include <QtSql/QSqlError>
#include <QtSql/QSqlResult>
#include <QTextStream>

// TODO at some point
// MyISAM-table 'SCHEDS.MYI' is usable but should be fixed
// above is part of the output of myisamchk -e /var/lib/mysql/VivenciaManager/*.MYI, run as root
// This should be the line looked for when trying to fix the database. One day there will be a module for database options like change password, fix, optimize, etc

VivenciaDB* VivenciaDB::s_instance ( nullptr );

static const QLatin1String chrDelim2 ( "\'," );

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

static const QString DB_DRIVER_NAME ( QStringLiteral ( "QMYSQL" ) );
static const QString ROOT_CONNECTION ( QStringLiteral ( "root_connection" ) );

//-----------------------------------------STATIC---------------------------------------------
QString VivenciaDB::getTableColumnName ( const TABLE_INFO* t_info, uint column )
{
	int sep_1 ( 0 ) , sep_2 ( -1 );

	QString col_name;
	while ( ( sep_2 = t_info->field_names.indexOf ( CHR_PIPE, sep_1 ) ) != -1 ) {
		if ( column-- == 0 ) {
			col_name = t_info->field_names.mid ( sep_1, sep_2-sep_1 );
			break;
		}
		sep_1 = unsigned ( sep_2 ) + 1;
	}
	return col_name;
}

QString VivenciaDB::getTableColumnFlags ( const TABLE_INFO* t_info, uint column )
{
	int sep_1 ( 0 ), sep_2 ( -1 );

	QString col_flags ( emptyString );
	while ( ( sep_2 = t_info->field_flags.indexOf ( CHR_PIPE, sep_1 ) ) != -1 ) {
		if ( column-- == 0 ) {
			col_flags = t_info->field_flags.mid ( sep_1, sep_2-sep_1 );
            //col_flags.truncate ( col_flags.count () - 2 );
            col_flags.chop ( 2 );
			break;
		}
		sep_1 = unsigned ( sep_2 ) + 1;
	}
	return col_flags;
}

QString VivenciaDB::getTableColumnLabel ( const TABLE_INFO* t_info, uint column )
{
	int sep_1 ( 0 ) , sep_2 ( -1 );

	QString col_label ( emptyString );
	while ( ( sep_2 = t_info->field_labels.indexOf ( CHR_PIPE, sep_1 ) ) != -1 ) {
		if ( column-- == 0 ) {
			col_label = t_info->field_labels.mid ( sep_1, sep_2 - sep_1 );
			break;
		}
		sep_1 = unsigned ( sep_2 ) + 1;
	}
	return col_label;
}

QString VivenciaDB::tableName ( const TABLE_ORDER table )
{	
	QString ret;
	switch ( table ) {
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
      m_ok ( false ), mNewDB ( false ), mBackupSynced ( true ), rootPasswdMngr ( nullptr )
{
	openDataBase ();
	addPostRoutine ( deleteVivenciaDBInstance );
}

VivenciaDB::~VivenciaDB ()
{
	heap_del ( rootPasswdMngr );
	m_db.close ();
}

//-----------------------------------------READ-OPEN-LOAD-------------------------------------------
bool VivenciaDB::openDataBase ()
{
	if ( !m_db.isOpen () ) {
		//m_db = QSqlDatabase::addDatabase ( DB_DRIVER_NAME, CONNECTION_NAME );
		//TEST: do not provide a connection name so that this database becomes the default for the application (see docs)
		// This is so that any QSqlQuery object will automatically use VivenciaDatabase;
		//m_db = QSqlDatabase::addDatabase ( DB_DRIVER_NAME, CONNECTION_NAME );
		m_db = QSqlDatabase::addDatabase ( DB_DRIVER_NAME );
		m_db.setHostName ( HOST );
		m_db.setPort ( PORT );
		m_db.setUserName ( USER_NAME );
		m_db.setPassword ( PASSWORD );
		m_db.setDatabaseName ( DB_NAME );
		m_ok = m_db.open ();
	}
	return m_ok;
}

void VivenciaDB::doPreliminaryWork ()
{
	generalTable gen_rec;
	if ( gen_rec.readFirstRecord () ) {
		double t_version ( 0.0 );
		uint i ( 0 );
		do {
			t_version = recStrValue ( &gen_rec, FLD_GENERAL_TABLEVERSION ).toDouble ();
			if ( table_info[i]->version != t_version ) {
				if ( ( table_info[i]->update_func != nullptr ) && ( *table_info[i]->update_func ) () )
					gen_rec.insertOrUpdate ( table_info[i] );
			}
			++i;
		} while ( gen_rec.readNextRecord () );
		if ( i == ( TABLES_IN_DB - 1 ) ) {
			if ( ( *table_info[i]->update_func ) () )
				gen_rec.insertOrUpdate ( table_info[i] );
		}
	}
	else { // either GENERAL table is empty or inexistent
		if ( !mNewDB ) {
			//SPLASH ()->displayMessage ( QCoreApplication::tr ( "Converting database to the new format. This may take a while..." ) );
			( *table_info[TABLE_GENERAL_ORDER]->update_func ) ();
		}
	}
}
//-----------------------------------------READ-OPEN-LOAD-------------------------------------------

//-----------------------------------------CREATE-DESTROY-MODIFY-------------------------------------
bool VivenciaDB::createDatabase ()
{
	rootPasswdMngr = new passwordManager ( passwordManager::PWSS_SAVE_TEMP );
	if ( !openDataBase () ) {
		m_ok = false;
		QString rootpswd;
		/* vmNotify::inputBox returns false when empty strings are entered. However, if mysql's root password was never set
		 * it is null by default and we should consider that possibility. So far, if that is the case, this app will return an
		 * error and the database will not be created.
		 */
		if ( rootPasswdMngr->askPassword ( rootpswd, DB_DRIVER_NAME, QCoreApplication::tr ( "You need to provide MYSQL's root(admin) password "
										"to create the application's database" ) ) ) {

			QSqlDatabase dbRoot ( QSqlDatabase::addDatabase ( DB_DRIVER_NAME, ROOT_CONNECTION ) );
			dbRoot.setDatabaseName ( STR_MYSQL );
			dbRoot.setHostName ( HOST );
			dbRoot.setUserName ( QStringLiteral ( "root" ) );
			dbRoot.setPassword ( rootpswd );

			if ( dbRoot.open () ) {
				const QString str_query ( QLatin1String ( "CREATE DATABASE " ) + DB_NAME +
									  QLatin1String ( " DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci" ) );
				QSqlQuery rootQuery ( dbRoot );
				m_ok = rootQuery.exec ( str_query );
				rootQuery.finish ();
				dbRoot.commit ();
				dbRoot.close ();
				rootQuery.clear ();
			}
			QSqlDatabase::removeDatabase ( ROOT_CONNECTION );
		}
	}
	if ( !m_ok )
		vmNotify::messageBox ( nullptr, QCoreApplication::tr ( "Database error" ),
									  QCoreApplication::tr ( "MYSQL could create the database." ) );
	return m_ok;
}

bool VivenciaDB::createUser ()
{
	m_ok = false;
	QString rootpswd;
	if ( rootPasswdMngr->askPassword ( rootpswd, DB_DRIVER_NAME, QCoreApplication::tr ( "You need to provide MYSQL's root(admin) password "
									"to create the application's database" ) ) ) {

		QSqlDatabase dbRoot ( QSqlDatabase::addDatabase ( DB_DRIVER_NAME, ROOT_CONNECTION ) );
		dbRoot.setDatabaseName ( STR_MYSQL );
		dbRoot.setHostName ( HOST );
		dbRoot.setUserName ( QStringLiteral ( "root" ) );
		dbRoot.setPassword ( rootpswd );

		if ( dbRoot.open () ) {
			QString str_query ( QString ( QStringLiteral ( "CREATE USER %1@%2 IDENTIFIED BY '%3'" ) ).arg ( USER_NAME, HOST, PASSWORD ) );
			QSqlQuery rootQuery ( dbRoot );
			rootQuery.exec ( str_query );

			str_query = QString ( QStringLiteral ( "GRANT ALL ON %1.* TO '%2'@'%3';" ) ).arg ( DB_NAME, USER_NAME, HOST );
			m_ok = rootQuery.exec ( str_query );

			rootQuery.finish ();
			dbRoot.commit ();
			dbRoot.close ();
			rootQuery.clear ();
		}
		QSqlDatabase::removeDatabase ( ROOT_CONNECTION );

	}
	if ( !m_ok )
		vmNotify::messageBox ( nullptr, QCoreApplication::tr ( "Database error" ),
								QCoreApplication::tr ( "MYSQL could create the user for the application's database." ) );
	return m_ok;
}

bool VivenciaDB::databaseIsEmpty () const
{
	if ( m_ok ) {
		QSqlQuery query ( QLatin1String ( "SHOW TABLES FROM " ) + DB_NAME, m_db );
		if ( query.exec () )
			return ( !query.isActive () || !query.next () );
	}
	return true;
}

bool VivenciaDB::createAllTables ()
{
	m_db.close ();
	if ( openDataBase () ) {
		uint i ( 0 );
		do {
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
	uint sep_1[2] = { 0 };
	int sep_2[2] = { 0 };

	QString values;
	while ( ( sep_2[0] = t_info->field_names.indexOf ( CHR_PIPE, sep_1[0] ) ) != -1 ) {
		values.append ( t_info->field_names.mid ( sep_1[0], sep_2[0] - sep_1[0] ) );
		sep_1[0] = unsigned ( sep_2[0] ) + 1;

		sep_2[1] = t_info->field_flags.indexOf ( CHR_PIPE, sep_1[1] );
		values.append ( t_info->field_flags.mid ( sep_1[1], sep_2[1]-sep_1[1] ) );
		sep_1[1] = unsigned ( sep_2[1] ) +1;
	}
	values.append ( t_info->primary_key );
	cmd += values + CHR_R_PARENTHESIS + CHR_SPACE + t_info->table_flags;

	m_db.exec ( cmd );
	return ( m_db.lastError ().type () == QSqlError::NoError );
}

bool VivenciaDB::deleteDB ( const QString& dbname )
{
	QSqlQuery query ( QLatin1String ( "DROP DATABASE " ) +
			( dbname.isEmpty () ? DB_NAME : dbname ), m_db );
	return query.exec ();
}

bool VivenciaDB::optimizeTable ( const TABLE_INFO* t_info ) const
{
	QSqlQuery query ( QLatin1String ( "OPTIMIZE LOCAL TABLE " ) + t_info->table_name, m_db );
	return query.exec ();
}

bool VivenciaDB::lockTable ( const TABLE_INFO* t_info ) const
{
	QSqlQuery query ( QLatin1String ( "LOCK TABLE " ) + t_info->table_name + QLatin1String ( "READ" ), m_db );
	return query.exec ();
}

bool VivenciaDB::unlockTable ( const TABLE_INFO* t_info ) const
{
	QSqlQuery query ( QLatin1String ( "UNLOCK TABLE " ) + t_info->table_name, m_db );
	return query.exec ();
}

bool VivenciaDB::unlockAllTables () const
{
	QSqlQuery query ( QStringLiteral ( "UNLOCK TABLES" ), m_db );
	return query.exec ();
}

// After a call to this function, must needs call unlockAllTables
bool VivenciaDB::flushAllTables () const
{
	QSqlQuery query ( QStringLiteral ( "FLUSH TABLES WITH READ LOCK" ), m_db );
	return query.exec ();
}

bool VivenciaDB::deleteTable ( const TABLE_INFO* t_info )
{
	QSqlQuery query ( QLatin1String ( "DROP TABLE " ) + t_info->table_name, m_db );
	return query.exec ();
}

bool VivenciaDB::clearTable ( const TABLE_INFO* t_info )
{
	QSqlQuery query ( QLatin1String ( "TRUNCATE TABLE " ) + t_info->table_name, m_db );
	return query.exec ();
}

bool VivenciaDB::tableExists ( const TABLE_INFO* t_info )
{
	QSqlQuery query ( QLatin1String ( "SHOW TABLES LIKE '" ) + t_info->table_name + CHR_CHRMARK, m_db );
	if ( query.exec () && query.first () )
		return query.value ( 0 ).toString () == t_info->table_name;
	return false;
}

bool VivenciaDB::insertColumn ( const uint column, const TABLE_INFO* t_info )
{
	const QString cmd ( QLatin1String ( "ALTER TABLE " ) + t_info->table_name +
						QLatin1String ( " ADD COLUMN " ) + getTableColumnName ( t_info, column ) +
						getTableColumnFlags ( t_info, column ) + ( column > 0 ?
								QString ( QLatin1String ( " AFTER " ) + getTableColumnName ( t_info, column - 1 ) ) :
                                QLatin1String ( " FIRST" ) )
					  );
	QSqlQuery query ( cmd, m_db );
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
	QSqlQuery query ( cmd, m_db );
    if ( query.exec () ) {
        mBackupSynced = false;
        return true;
    }
    return false;
}

bool VivenciaDB::renameColumn ( const QString& old_column_name, const uint col_idx, const TABLE_INFO* t_info )
{
	const QString cmd ( QLatin1String ( "ALTER TABLE " ) + t_info->table_name +
						QLatin1String ( " CHANGE " ) + old_column_name +
						getTableColumnName ( t_info, col_idx ) + CHR_SPACE + getTableColumnFlags ( t_info, col_idx ) );
	QSqlQuery query ( cmd, m_db );
    if ( query.exec () ) {
        mBackupSynced = false;
        return true;
    }
    return false;
}

bool VivenciaDB::changeColumnProperties ( const uint column, const TABLE_INFO* t_info )
{
	const QString cmd ( QLatin1String ( "ALTER TABLE " ) + t_info->table_name +
						QLatin1String ( " MODIFY COLUMN " ) + getTableColumnName ( t_info, column ) +
						getTableColumnFlags ( t_info, column ) );
	QSqlQuery query ( cmd, m_db );
    if ( query.exec () ) {
        mBackupSynced = false;
        return true;
    }
    return false;
}

uint VivenciaDB::recordCount ( const TABLE_INFO* t_info ) const
{
	QSqlQuery query ( QLatin1String ( "SELECT COUNT(*) FROM " ) + t_info->table_name, m_db );
	if ( query.exec () && query.first () )
		return query.value ( 0 ).toUInt ();
	return 0;
}

void VivenciaDB::populateTable ( const DBRecord* db_rec, vmTableWidget* table ) const
{
	const QString cmd ( QLatin1String ( "SELECT * FROM " ) + db_rec->t_info->table_name );

	QSqlQuery query ( m_db );
	query.setForwardOnly ( true );
	query.exec ( cmd );
	if ( !query.first () ) return;

	spreadRow* s_row ( new spreadRow );
	uint col ( 0 );

	s_row->field_value.reserve ( db_rec->t_info->field_count );
	s_row->column.reserve ( db_rec->t_info->field_count );

	for ( col = 0; col < db_rec->t_info->field_count; ++col )
		s_row->column.append ( col );

	do {
		for ( col = 0; col < db_rec->t_info->field_count; ++col )
			s_row->field_value[col] = query.value ( col ).toString ();
		++( s_row->row );
		table->setRowData ( s_row );
	} while ( query.next () );

	delete s_row;
}
//-----------------------------------------CREATE-DESTROY-MODIFY-------------------------------------

//-----------------------------------------COMMOM-DBRECORD-------------------------------------------
bool VivenciaDB::insertRecord ( const DBRecord* db_rec ) const
{
	QString str_query ( QLatin1String ( "INSERT INTO " ) +
						db_rec->t_info->table_name + QLatin1String ( " VALUES ( " ) );
	QString values;

	const uint field_count ( db_rec->t_info->field_count );
	for ( uint i ( 0 ); i < field_count; ++i )
		values += CHR_CHRMARK + recStrValue ( db_rec, i ) + chrDelim2;

    //values.truncate ( values.count () - 1 ); // removes the last comma character
    values.chop ( 1 );
	str_query += values + QLatin1String ( " )" );

	m_db.exec ( str_query );
    if ( m_db.lastError ().type () == QSqlError::NoError ) {
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

	uint sep_1 ( ( db_rec->t_info->field_names.indexOf ( CHR_PIPE, 1 ) ) + 1 ); // skip ID field
	int sep_2 ( 0 );

	const uint field_count ( db_rec->t_info->field_count );
	const QLatin1String chrDelim ( "=\'" );
	const QChar CHR_PIPE_PTR ( CHR_PIPE );
	const QStringMatcher sep_matcher ( &CHR_PIPE_PTR, 1 );

	for ( uint i ( 1 ); i < field_count; ++i ) {
		sep_2 = sep_matcher.indexIn ( db_rec->t_info->field_names, sep_1 );
		if ( db_rec->isModified ( i ) ) {
			str_query += db_rec->t_info->field_names.midRef ( sep_1, sep_2 - sep_1 ) +
						 chrDelim + recStrValue ( db_rec, i ) + chrDelim2;
		}
		sep_1 = unsigned ( sep_2 ) + 1;
	}
    //str_query.truncate ( str_query.count () - 1 ); // removes the last comma character
    str_query.chop ( 1 );
	str_query += ( QLatin1String ( " WHERE ID='" ) + db_rec->actualRecordStr ( 0 ) + CHR_CHRMARK ); // id ()

	m_db.exec ( str_query );

    if ( m_db.lastError ().type () == QSqlError::NoError ) {
        mBackupSynced = false;
        return true;
    }
    return false;
}

bool VivenciaDB::removeRecord ( const DBRecord* db_rec ) const
{
	if ( db_rec->actualRecordInt ( 0 ) >= 0 ) {
		const QString str_query ( QLatin1String ( "DELETE FROM " ) + db_rec->t_info->table_name +
								  QLatin1String ( " WHERE ID='" ) + db_rec->actualRecordStr ( 0 ) + CHR_CHRMARK );
		m_db.exec ( str_query );
		if ( (unsigned) db_rec->actualRecordInt ( 0 ) == highest_id[db_rec->t_info->table_order] )
			const_cast<VivenciaDB*> ( this )->setHighestID ( db_rec->t_info->table_order,
					const_cast<VivenciaDB*> ( this )->getHighLowID ( db_rec->t_info->table_order, true ) );
        if ( m_db.lastError ().type () == QSqlError::NoError ) {
            mBackupSynced = false;
            return true;
        }
	}
	return false;
}

void VivenciaDB::loadDBRecord ( DBRecord* db_rec, const QSqlQuery* const query )
{
	db_rec->setIntValue ( 0, query->value ( 0 ).toInt () );
	for ( uint i ( 0 ); i < db_rec->t_info->field_count; ++i )
		setRecValue ( db_rec, i, query->value ( i ).toString () );
}

int VivenciaDB::lastDBRecord ( const uint table )
{
	if ( highest_id[table] == 0 )
		(void) getHighLowID ( table );
	return ( highest_id.at ( table ) );
}

int VivenciaDB::firstDBRecord ( const uint table )
{
	//if ( lowest_id[table] == -1 )
	(void) getHighLowID ( table, false );
	return ( lowest_id.at ( table ) );
}

bool VivenciaDB::insertDBRecord ( DBRecord* db_rec )
{
	const uint table_order ( db_rec->t_info->table_order );
	const bool bOutOfOrder ( db_rec->recordInt ( 0 ) >= 1 );
	const int new_id ( bOutOfOrder ? db_rec->recordInt ( 0 ) : getHighLowID ( table_order ) + 1 );

	db_rec->setIntValue ( 0, new_id );
	db_rec->setValue ( 0, QString::number ( new_id ) );
	const bool ret ( insertRecord ( db_rec ) );
	if ( ret )
		highest_id[table_order] = new_id;
	return ret;
}

bool VivenciaDB::getDBRecord ( DBRecord* db_rec, const uint field, const bool load_data )
{
	const QString cmd ( QLatin1String ( "SELECT * FROM " ) +
						db_rec->t_info->table_name + QLatin1String ( " WHERE " ) +
						getTableColumnName ( db_rec->t_info, field ) + CHR_EQUAL +
						db_rec->backupRecordStr ( field )
					  );

	QSqlQuery query ( m_db );
	query.setForwardOnly ( true );
	query.exec ( cmd );
	if ( query.first () ) {
		if ( load_data ) {
			loadDBRecord ( db_rec, &query );
			return true;
		}
		else {
			/* For the SELECT statement have some use, at least save the read id for later use.
			 * Save all the initial id fields. Fast table lookup and fast item lookup, that's
			 * the goal of this part of the code
			 */
			uint i ( 0 );
			while ( db_rec->fieldType ( i ) == DBTYPE_ID ) {
				db_rec->setIntValue ( i, query.value ( i ).toInt () );
				i++;
			}
			return true;
		}
	}
	return false;
}

bool VivenciaDB::getDBRecord ( DBRecord* db_rec, DBRecord::st_Query& stquery, const bool load_data )
{
	bool ret ( false );

	if ( stquery.reset ) {
		stquery.reset = false;
		if ( !stquery.query )
			stquery.query = new QSqlQuery ( m_db );
		else
			stquery.query->finish ();

		stquery.query->setForwardOnly ( stquery.forward );

		if ( stquery.str_query.isEmpty () ) {
			const QString cmd ( QLatin1String ( "SELECT * FROM " ) + db_rec->t_info->table_name +
								( stquery.field != -1 ? QLatin1String ( " WHERE " ) +
								  getTableColumnName ( db_rec->t_info, stquery.field ) + CHR_EQUAL +
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

	if ( ret ) {
		if ( load_data )
			loadDBRecord ( db_rec, stquery.query );
		else {
			uint i ( 0 ); // save first id fields. Useful for subsequent queries or finding related vmListItems
			do {
				if ( db_rec->fieldType ( i ) == DBTYPE_ID )
					db_rec->setIntValue ( i, stquery.query->value ( i ).toInt () );
				else
					break;
			} while ( ++i < db_rec->fieldCount () );
            setRecValue ( db_rec, 0, stquery.query->value ( 0 ).toString () );
            //db_rec->setValue ( 0, stquery.query->value ( 0 ).toString () );
		}
	}
	return ret;
}

uint VivenciaDB::getHighLowID ( const uint table, const bool high )
{
	QSqlQuery query ( QLatin1String ( "SELECT " ) +
					  QLatin1String ( high ? "MAX(ID) FROM " : "MIN(ID) FROM " ) +
					  tableInfo ( table )->table_name, m_db );
	if ( query.exec () ) {
		if ( query.first () ) {
			if ( recordCount ( tableInfo ( table ) ) > 0 ) {
				const int id ( query.value ( 0 ).toInt () );
				if ( high )
					highest_id[table] = id;
				else
					lowest_id[table] = id;
				return id;
			}
		}
	}
	return high ? 0 : 1;
}

bool VivenciaDB::recordExists ( const QString& table_name, const int id ) const
{
	if ( id >= 0 ) {
		QSqlQuery query ( m_db );
		if ( query.exec ( QString ( QStringLiteral ( "SELECT `ID` FROM %1 WHERE `ID`=%2" ) ).arg (
							  table_name. QString::number ( id ) ) ) )
			return ( query.isActive () && query.next () );
	}
	return false;
}

bool VivenciaDB::runQuery ( const QString& query_cmd, QSqlQuery& queryRes ) const
{
	if ( m_ok && !query_cmd.isEmpty () ) {
		queryRes.setForwardOnly ( true );
		if ( queryRes.exec ( query_cmd ) )
			return queryRes.first ();
	}
	return false;
}
//-----------------------------------------COMMOM-DBRECORD-------------------------------------------

//-----------------------------------------IMPORT-EXPORT--------------------------------------------
bool VivenciaDB::doBackup ( const QString& filepath, const QString& tables )
{
	QString passwd;
	if ( rootPasswdMngr->askPassword ( passwd, DB_DRIVER_NAME,
			QCoreApplication::tr ( "You need to provide MYSQL's root(admin) password password for backup operations" ) ) ) {
		flushAllTables ();
		BACKUP ()->incrementProgress (); //2
        const QString mysqldump_args ( QLatin1String ( "--user=root --password=" ) +
                   passwd + CHR_SPACE + DB_NAME + CHR_SPACE + tables );

        const QString dump ( fileOps::executeAndCaptureOutput ( mysqldump_args, backupApp () ) );
		unlockAllTables ();
		BACKUP ()->incrementProgress (); //3
		if ( !dump.isEmpty () ) {
			QFile file ( filepath );
			if ( file.open ( QIODevice::WriteOnly | QIODevice::Text ) ) {
				QTextStream out ( &file );
                out.setCodec ( "UTF-8" );
				out.setAutoDetectUnicode ( true );
				out.setLocale ( QLocale::system () );
				QString strHeader ( dump.left ( 500 ) );
				strHeader.replace ( QStringLiteral ( "SET NAMES latin1" ), QStringLiteral ( "SET NAMES utf8" ) );
				strHeader.append ( dump.mid ( 500, dump.length () - 500 ) );
				out << strHeader.toLocal8Bit ();
				file.close ();
				BACKUP ()->incrementProgress (); //4
                mBackupSynced = true;
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
	if ( m_ok ) {
		QFile file ( dumpfile );
		if ( file.open ( QIODevice::ReadOnly|QIODevice::Text ) ) {
			char* line_buf = new char [8*1024];// OPT-6
			int n_chars ( -1 );
			QString line, searchedExpr;
			const QString expr ( QLatin1String ( "(%1,%2,%3," ) );

			do {
				n_chars = file.readLine ( line_buf, 8*1024 );
				if ( n_chars >= 500 ) { // The first time ths function was created, the searched line had 509 characters
					line = QString::fromUtf8 ( line_buf, strlen ( line_buf ) -1 );
					if ( line.contains ( QLatin1String ( "INSERT INTO `GENERAL`" ) ) ) {
						ok = true; // so far, it is
						for ( uint i ( 0 ); i < TABLES_IN_DB; ++i ) {
							searchedExpr = expr.arg ( table_info[i]->table_order ).arg ( table_info[i]->table_name ).arg ( table_info[i]->version );
							if ( !line.contains ( searchedExpr ) ) { // one single mismatch the dump file cannot be used by mysqlimport
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

bool VivenciaDB::doRestore ( const QString& filepath )
{
	QString rootpswd;
	if ( !rootPasswdMngr->askPassword ( rootpswd, DB_DRIVER_NAME,
				QCoreApplication::tr ( "You need to provide MYSQL's root(admin) password to restore from file" ) ) )
		return false;

	QString uncompressed_file ( QStringLiteral ( "/tmp/vmdb_tempfile" ) );

	const bool bSourceCompressed ( VMCompress::isCompressed ( filepath ) );
	if ( bSourceCompressed ) {
		fileOps::removeFile ( uncompressed_file );
		VMCompress::decompress ( filepath, uncompressed_file );
	}
	else
		uncompressed_file = filepath;

	BACKUP ()->incrementProgress (); //3
	QString restoreapp_args;
	bool ret ( true );

	//hopefully, by now, Data will have intermediated well the situation, having all the pieces of the program
	// aware of the changes that might occur.
	m_db.close ();

	// use a previous dump from mysqldump to create a new database.
	// Note that DB_NAME must refer to an unexisting database or this will fail
	// Also I think the dump must be of a complete database, with all tables, to avoid inconsistencies
	if ( !checkDumpFile ( uncompressed_file ) ) { // some table version mismatch
        restoreapp_args = QLatin1String ( "--user=root --default_character_set=utf8 --password=" ) + rootpswd + CHR_SPACE + DB_NAME;
        ret = fileOps::executeWithFeedFile ( restoreapp_args, importApp (), uncompressed_file );
	}
	// use a previous dump from mysqldump to update or replace one or more tables in an existing database
	// Tables will be overwritten or created. Note that DB_NAME must refer to an existing database
	else {
        restoreapp_args = QLatin1String ( "--user=root --password=" ) + rootpswd + CHR_SPACE + DB_NAME + CHR_SPACE + uncompressed_file;
        ret = fileOps::executeWait ( restoreapp_args, restoreApp () );
	}
	BACKUP ()->incrementProgress (); //4

	if ( bSourceCompressed ) // do not leave trash behind
        fileOps::removeFile ( uncompressed_file );

    if ( ret ) {
        mBackupSynced = true;
		return openDataBase ();
    }

	return false;
}

bool VivenciaDB::importFromCSV ( const QString& filename )
{
	dataFile *tdb = new dataFile ( filename );
	if ( !tdb->open () )
		return false;
	if ( !tdb->load () )
		return false;

	BACKUP ()->incrementProgress (); //3

	stringRecord rec;
	if ( !tdb->getRecord ( rec, 0 ) )
		return false;

	const int table_id  ( rec.fieldValue ( 1 ).toInt () );
	DBRecord* db_rec ( nullptr );
	switch ( table_id ) {
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

	BACKUP ()->incrementProgress (); //4
    bool bSuccess ( false );
	if ( rec.fieldValue ( 0 ).toFloat () == db_rec->t_info->version ) { //we don't import if there is a version mismatch
		uint fld ( 1 );
		uint idx ( 1 );
		do {
			if ( !tdb->getRecord ( rec, idx ) )
				break;
			if ( rec.first () ) {
				setRecValue ( db_rec, 0, rec.curValue () );
				//db_rec->setValue ( 0, rec.curValue () );
				do {
					if ( rec.next () )
						setRecValue ( db_rec, fld++, rec.curValue () );
					//db_rec->setValue ( fld++, rec.curValue () );
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

	BACKUP ()->incrementProgress (); //5
	delete tdb;
    return bSuccess;
}

bool VivenciaDB::exportToCSV ( const uint table, const QString& filename )
{
	const TABLE_INFO* t_info ( nullptr );
	switch ( table ) {
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
    (void) fileOps::removeFile ( filename );
	dataFile* tdb ( new dataFile ( filename ) );
	if ( !tdb->open () )
		return false;
	BACKUP ()->incrementProgress (); //2

	stringRecord rec;
	rec.fastAppendValue ( QString::number ( t_info->table_id ) ); //Record table_id information
	rec.fastAppendValue ( QString::number ( t_info->version, 'f', 1 ) ); //Record version information
	tdb->insertRecord ( 0, rec );
	BACKUP ()->incrementProgress (); //3

	const QString cmd ( QLatin1String ( "SELECT * FROM " ) + t_info->table_name );

	QSqlQuery query ( m_db );
	query.setForwardOnly ( true );
	query.exec ( cmd );

	if ( query.first () ) {
		uint pos ( 1 ), i ( 0 );

		do {
			for ( i = 0; i < t_info->field_count; ++i )
				rec.fastAppendValue ( query.value ( i ).toString () );
			tdb->insertRecord ( pos, rec );
			rec.clear ();
			++pos;
		} while ( query.next () );
	}

	BACKUP ()->incrementProgress (); //4

	const uint n ( tdb->recCount () );
	tdb->commit ();
	delete tdb;

	BACKUP ()->incrementProgress (); //5
	return ( n > 0 );
}

//-----------------------------------------IMPORT-EXPORT--------------------------------------------
