#include "supplierrecord.h"
#include "global.h"
#include "completers.h"

#ifdef TRANSITION_PERIOD
#include "purchases.h"
#include "vivenciadb.h"
#endif

static const double TABLE_VERSION ( 1.1 );

static const uint SUPPLIER_FIELDS_TYPE[SUPPLIER_FIELD_COUNT] = {
	DBTYPE_ID, DBTYPE_SHORTTEXT, DBTYPE_SHORTTEXT, DBTYPE_NUMBER, DBTYPE_SHORTTEXT,
	DBTYPE_SHORTTEXT, DBTYPE_PHONE, DBTYPE_SHORTTEXT
};

bool updateSupplierTable ()
{
#ifdef TRANSITION_PERIOD
	( void ) VDB ()->database ()->exec ( QStringLiteral (
			"ALTER TABLE SUPPLIERS CHANGE PHONE_1 PHONES varchar ( 100 ) COLLATE utf8_unicode_ci DEFAULT NULL" ) );

	QSqlQuery query ( *( VDB ()->database () ) );
	query.setForwardOnly ( true );
	query.exec ( QStringLiteral ( "SELECT * FROM SUPPLIERS" ) );
	if ( !query.first () ) return false;
	const uint FLD_OLDSUPPLIER_TEL_1 ( 6 );
	const uint FLD_OLDSUPPLIER_TEL_2 ( 7 );
	const uint FLD_OLDSUPPLIER_EMAIL ( 8 );
	stringRecord rec;
	int idx ( 0 );
	vmNumber phone1, phone2;
	const QString update_cmd ( QStringLiteral ( "UPDATE SUPPLIERS SET %1='%2' WHERE ID='%3'" ) );
	QSqlQuery update_query ( *( VDB ()->database () ) );

	do {
		phone1.fromStrPhone ( query.value ( FLD_OLDSUPPLIER_TEL_1 ).toString () );
		if ( phone1.isPhone () ) {
			rec.clear ();
			rec.fastAppendValue ( phone1.toPhone () );
			phone2.fromStrPhone ( query.value ( FLD_OLDSUPPLIER_TEL_2 ).toString () );
			if ( phone2.isPhone () && phone2 != phone1 )
				rec.fastAppendValue ( phone2.toPhone () );
			update_query.exec ( update_cmd.arg ( QStringLiteral ( "PHONES" ), rec.toString (), query.value ( FLD_SUPPLIER_ID ).toString () ) );
		}

		idx = query.value ( FLD_OLDSUPPLIER_EMAIL ).toString ().indexOf ( CHR_SEMICOLON );
		if ( idx != -1 ) {
			rec.clear ();
			rec.fastAppendValue ( query.value ( FLD_OLDSUPPLIER_EMAIL ).toString ().left ( idx ) );
			rec.fastAppendValue ( query.value ( FLD_OLDSUPPLIER_EMAIL ).toString ().right ( query.value ( FLD_OLDSUPPLIER_EMAIL ).toString ().length () - 1 - idx ) );
			update_query.exec ( update_cmd.arg ( QStringLiteral ( "EMAIL" ), rec.toString (), query.value ( FLD_SUPPLIER_ID ).toString () ) );
		}
	} while ( query.next () );

	( void ) VDB ()->database ()->exec ( QStringLiteral ( "ALTER TABLE SUPPLIERS DROP PHONE_2" ) );

	Buy buy ( false );
	if ( buy.readFirstRecord () ) {
		do {
			supplierRecord::insertIfSupplierInexistent ( recStrValue ( &buy, FLD_BUY_SUPPLIER ) );
		} while ( buy.readNextRecord () );
	}
	return true;

#endif //TRANSITION_PERIOD
	return false;
}

const TABLE_INFO supplierRecord::t_info = {
	SUPPLIER_TABLE,
	QStringLiteral ( "SUPPLIERS" ),
	QStringLiteral ( " ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci" ),
	QStringLiteral ( " PRIMARY KEY ( `ID` ) , UNIQUE KEY `id` ( `ID` ) " ),
	QStringLiteral ( "`ID`|`NAME`|`STREET`|`NUMBER`|`DISTRICT`|`CITY`|`PHONES`|`EMAIL`|" ),
	QStringLiteral ( " int ( 9 ) NOT NULL, | varchar ( 100 ) COLLATE utf8_unicode_ci DEFAULT NULL, |"
	" varchar ( 100 ) COLLATE utf8_unicode_ci DEFAULT NULL, | varchar ( 10 ) DEFAULT NULL, |"
	" varchar ( 50 ) COLLATE utf8_unicode_ci DEFAULT NULL, | varchar ( 50 ) COLLATE utf8_unicode_ci DEFAULT NULL, |"
	" varchar ( 100 ) COLLATE utf8_unicode_ci DEFAULT NULL, |varchar ( 200 ) COLLATE utf8_unicode_ci DEFAULT NULL, |" ),
	QStringLiteral ( "ID|Name|Street|Number|District|City|Phones|E-mail|" ),
	SUPPLIER_FIELDS_TYPE, TABLE_VERSION, SUPPLIER_FIELD_COUNT, TABLE_SUPPLIER_ORDER, &updateSupplierTable
	#ifdef TRANSITION_PERIOD
	, false
	#endif
};

static void updateSupplierNameCompleter ( const DBRecord* db_rec )
{
	APP_COMPLETERS ()->updateCompleter ( recStrValue ( db_rec, FLD_SUPPLIER_NAME ), vmCompleters::SUPPLIER );
}

static void updateSupplierStreetCompleter ( const DBRecord* db_rec )
{
	APP_COMPLETERS ()->updateCompleter ( recStrValue ( db_rec, FLD_SUPPLIER_STREET ), vmCompleters::ADDRESS );
}

static void updateSupplierDistrictCompleter ( const DBRecord* db_rec )
{
	APP_COMPLETERS ()->updateCompleter ( recStrValue ( db_rec, FLD_SUPPLIER_DISTRICT ), vmCompleters::ADDRESS );
}

static void updateSupplierCityCompleter ( const DBRecord* db_rec )
{
	APP_COMPLETERS ()->updateCompleter ( recStrValue ( db_rec, FLD_SUPPLIER_CITY ), vmCompleters::ADDRESS );
}

supplierRecord::supplierRecord ( const bool connect_helper_funcs )
	: DBRecord ( SUPPLIER_FIELD_COUNT )
{
	::memset ( this->helperFunction, 0, sizeof ( this->helperFunction ) );
	DBRecord::t_info = & ( this->t_info );
	DBRecord::m_RECFIELDS = this->m_RECFIELDS;

	if ( connect_helper_funcs ) {
		DBRecord::helperFunction = this->helperFunction;
		setHelperFunction ( FLD_SUPPLIER_NAME, &updateSupplierNameCompleter );
		setHelperFunction ( FLD_SUPPLIER_STREET, &updateSupplierStreetCompleter );
		setHelperFunction ( FLD_SUPPLIER_DISTRICT, &updateSupplierDistrictCompleter );
		setHelperFunction ( FLD_SUPPLIER_CITY, &updateSupplierCityCompleter );
	}
}

supplierRecord::~supplierRecord () {}

void supplierRecord::insertIfSupplierInexistent ( const QString& supplier )
{
	QSqlQuery query ( *( VDB ()->database () ) );
	if ( query.exec ( QLatin1String ( "SELECT ID FROM SUPPLIERS WHERE NAME=\'" ) + supplier + QLatin1Char ( '\'' ) ) ) {
		if ( query.isActive () && query.next () )
			return;
		supplierRecord sup_rec;
		sup_rec.setAction ( ACTION_ADD );
		sup_rec.setValue ( FLD_SUPPLIER_NAME, supplier );
		sup_rec.saveRecord ();
	}
}
