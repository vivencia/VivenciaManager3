#include "generaltable.h"
#include "vivenciadb.h"

#ifdef TRANSITION_PERIOD
#include "global.h"
#include "vmnotify.h"
#include "client.h"
#include "job.h"
#include "payment.h"
#include "purchases.h"

#include <QApplication>

static const QLatin1String whereStatement ( " WHERE ID=" );
static const QLatin1String setStatement ( " SET ID=" );
static const QLatin1String updateStatement ( "UPDATE " );
static const QLatin1String selectStatement ( "SELECT ID FROM " );

/*
 * The old database has its indexes all mixed up and we need to fix that in order to improve performance
 * and also to put order into the mess. The reason to start each and every table from index 1 is because
 * a negative search result will return the record with index 0, for some reason I yet to not know. It's
 * a Mysql issue (or feature ?) but I need to circumvent this for getting useful search results
 */
void updateIDs ( const TABLE_ORDER table, podList<int> *new_ids )
{
	const QString tableName ( VDB ()->tableName ( table ) );
	int id ( 0 );
	const uint maxID ( VDB ()->getHighLowID ( table ) );
	QString idStr;
	QSqlQuery unusedQuery;
	if ( new_ids != nullptr ) {
		new_ids->setDefaultValue ( -1 );
		new_ids->reserve ( maxID + 1 );
	}
	for ( ; ( unsigned )id <= maxID; ++id ) {
		idStr = QString::number ( id );
		if ( VDB ()->runQuery ( selectStatement + tableName + whereStatement + idStr, unusedQuery ) ) {
			( void )VDB ()->runQuery ( updateStatement + tableName + setStatement
							   + QString::number ( id + 10000 ) + whereStatement + idStr, unusedQuery );
		}
	}
	int new_id ( 1 );
	for ( id = 0; ( unsigned )id <= maxID; ++id ) {
		idStr = QString::number ( id + 10000 );
		if ( VDB ()->runQuery ( selectStatement + tableName + whereStatement + idStr, unusedQuery ) ) {
			( void )VDB ()->runQuery ( updateStatement + tableName + setStatement
							   + QString::number ( new_id ) + whereStatement + idStr, unusedQuery );
			if ( new_ids != nullptr )
				new_ids->operator [] ( id ) = new_id;
			new_id++;
		}
	}
}

#endif

const float GENERAL_FIELD_TABLE_VERSION ( 2.5 );

static const uint GENERAL_FIELDS_TYPE[GENERAL_FIELD_COUNT] = {
	DBTYPE_ID, DBTYPE_SHORTTEXT, DBTYPE_SHORTTEXT, DBTYPE_NUMBER,
	DBTYPE_SHORTTEXT, DBTYPE_FILE, DBTYPE_SHORTTEXT
};

bool updateGeneralTable ()
{
#ifdef TRANSITION_PERIOD
	VDB ()->database ()->exec ( QStringLiteral ( "DROP TABLE SCHEDS" ) );
	VDB ()->database ()->exec ( QStringLiteral ( "DROP TABLE IMPROVEMENTS" ) );

	vmNotify* pBox ( nullptr );
	pBox = vmNotify::progressBox ( nullptr, nullptr, TABLES_IN_DB - 1, 0,
			QStringLiteral ( "Updating old database. This might take a while..." ),
			QStringLiteral ( "Updating table GENERAL" ) );

	generalTable gt;
	for ( uint i ( 0 ); i < TABLES_IN_DB; ++i ) {
		switch ( i ) {
			case TABLE_GENERAL_ORDER:
				gt.insertOrUpdate ( VivenciaDB::tableInfo ( TABLE_GENERAL_ORDER ) );
			break;
			case TABLE_JOB_ORDER:
				// This order is important because the update function in Job will cause a revolution
				// in ids

				if ( pBox ) {
					pBox = vmNotify::progressBox ( pBox, nullptr, TABLES_IN_DB - 1, 3, QString::null,
							QLatin1String ( "Updating table " ) + VivenciaDB::tableInfo ( TABLE_PAY_ORDER )->table_name );
				}
				( *VivenciaDB::tableInfo ( TABLE_PAY_ORDER )->update_func ) ();
				gt.insertOrUpdate ( VivenciaDB::tableInfo ( TABLE_PAY_ORDER ) );
				gt.clearAll ();
				if ( pBox ) {
					pBox = vmNotify::progressBox ( pBox, nullptr, TABLES_IN_DB - 1, 4, QString::null,
							QLatin1String ( "Updating table " ) + VivenciaDB::tableInfo ( TABLE_BUY_ORDER )->table_name );
				}
				( *VivenciaDB::tableInfo ( TABLE_BUY_ORDER )->update_func ) ();
				gt.insertOrUpdate ( VivenciaDB::tableInfo ( TABLE_BUY_ORDER ) );
				gt.clearAll ();
				if ( pBox ) {
					pBox = vmNotify::progressBox ( pBox, nullptr, TABLES_IN_DB - 1, 5, QString::null,
							QLatin1String ( "Updating table " ) + VivenciaDB::tableInfo ( TABLE_JOB_ORDER )->table_name );
				}
				( *VivenciaDB::tableInfo ( TABLE_JOB_ORDER )->update_func ) ();
				gt.insertOrUpdate ( VivenciaDB::tableInfo ( TABLE_JOB_ORDER ) );
				i = TABLE_BUY_ORDER;
			break;
			case TABLE_PAY_ORDER:
			case TABLE_BUY_ORDER:
			break;
			default:
				if ( pBox ) {
					pBox = vmNotify::progressBox ( pBox, nullptr, TABLES_IN_DB - 1, i, QString::null,
							QLatin1String ( "Updating table " ) + VivenciaDB::tableInfo ( i )->table_name );
				}
				if ( VivenciaDB::tableInfo ( i )->update_func != nullptr )
					( *VivenciaDB::tableInfo ( i )->update_func ) ();

				if ( !VivenciaDB::tableInfo ( i )->new_table )
					updateIDs ( static_cast<TABLE_ORDER>( i ) );

				VDB ()->optimizeTable ( VivenciaDB::tableInfo ( i ) );
				gt.insertOrUpdate ( VivenciaDB::tableInfo ( i ) );
			break;
		}
		gt.clearAll ();
	}
	VDB ()->setHighestID ( TABLE_GENERAL_ORDER, TABLES_IN_DB );
	VDB ()->optimizeTable ( &generalTable::t_info );
#endif

	// must return false everytime an update is performed in GENERAL table to avoid VivenciaDB
	// inserting a record or overwriting any change performed here. Therefore, every update function
	// must do its job, plus what VivenciaDB would for GENERAL table
	return false;
}

static const TABLE_INFO general_tinfo = {
	GENERAL_TABLE,
	QStringLiteral ( "GENERAL" ),
	QStringLiteral ( " ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci" ),
	QStringLiteral ( " PRIMARY KEY ( `ID` ) , UNIQUE KEY `id` ( `ID` ) " ),
	QStringLiteral ( "`ID`|`TABLE_NAME`|`TABLE_VERSION`|`TABLE_ID`|`MOWER_OIL_CHANGE`|`CONFIG_FILE`|`EXTRA`|" ),
	QStringLiteral ( " int ( 4 ) NOT NULL, | varchar ( 20 ) NOT NULL, | varchar ( 10 ) NOT NULL, | varchar ( 20 ) NOT NULL, |"
	" varchar ( 20 ) DEFAULT NULL, | varchar ( 255 ) COLLATE utf8_unicode_ci DEFAULT NULL, | varchar ( 50 ) DEFAULT NULL, |"),
	QStringLiteral ( "ID|Table Name|Table Version|Table ID|Oil Change|Config File|Extra|" ),
	GENERAL_FIELDS_TYPE, GENERAL_FIELD_TABLE_VERSION,
	GENERAL_FIELD_COUNT, TABLE_GENERAL_ORDER, &updateGeneralTable
	#ifdef TRANSITION_PERIOD
	, false
	#endif
};

const TABLE_INFO generalTable::t_info = general_tinfo;

generalTable::generalTable ()
	: DBRecord ( GENERAL_FIELD_COUNT )
{
	::memset ( this->helperFunction, 0, sizeof ( this->helperFunction ) );
	DBRecord::t_info = &( generalTable::t_info );
	DBRecord::m_RECFIELDS = this->m_RECFIELDS;
	DBRecord::helperFunction = this->helperFunction;
}

generalTable::~generalTable () {}

void generalTable::insertOrUpdate ( const TABLE_INFO* const t_info )
{
	const QString genIDStr ( QString::number ( t_info->table_order + 1 ) );

	if ( readRecord ( FLD_GENERAL_ID, genIDStr ) ) {
		setAction ( ACTION_EDIT );
		setRecValue ( this, FLD_GENERAL_TABLENAME, t_info->table_name );
		setRecValue ( this, FLD_GENERAL_TABLEVERSION, QString::number ( t_info->version ) );
	}
	else {
		setAction ( ACTION_ADD );
		setRecIntValue ( this, FLD_GENERAL_ID, t_info->table_order + 1 );
		setRecValue ( this, FLD_GENERAL_ID, genIDStr );
		setRecValue ( this, FLD_GENERAL_TABLENAME, t_info->table_name );
		setRecValue ( this, FLD_GENERAL_TABLEVERSION, QString::number ( t_info->version ) );
		setRecValue ( this, FLD_GENERAL_TABLEID, QString::number ( t_info->table_id ) );
		setRecValue ( this, FLD_GENERAL_MOWER_OIL_TIME, emptyString );
		setRecValue ( this, FLD_GENERAL_CONFIG_FILE, emptyString );
		setRecValue ( this, FLD_GENERAL_EXTRA, emptyString );
	}
	saveRecord ();
}
