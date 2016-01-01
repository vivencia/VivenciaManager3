#include "companypurchases.h"
#include "global.h"
#include "global_enums.h"
#include "completers.h"
#include "supplierrecord.h"

#include <QCoreApplication>

const double TABLE_VERSION ( 2.0 );

constexpr uint CP_FIELDS_TYPE[COMPANY_PURCHASES_FIELD_COUNT] = {
	DBTYPE_ID, DBTYPE_DATE, DBTYPE_DATE, DBTYPE_SHORTTEXT, DBTYPE_SHORTTEXT,
	DBTYPE_SHORTTEXT, DBTYPE_PRICE, DBTYPE_PRICE, DBTYPE_LIST, DBTYPE_LIST
};

#ifdef TRANSITION_PERIOD

#include "vivenciadb.h"
#include "purchases.h"

#include <QSqlQuery>
#include <QSqlRecord>

enum {
	OLD_FLD_CP_ID = 0, OLD_FLD_CP_PAID = 1, OLD_FLD_CP_DATE = 2, OLD_FLD_CP_PAYDATE = 3, OLD_FLD_CP_DELIVERDATE = 4, OLD_FLD_CP_PRICE = 5,
	OLD_FLD_CP_TOTALPAYS = 6, OLD_FLD_CP_PAYNUMBER = 7, OLD_FLD_CP_PAYVALUE = 8, OLD_FLD_CP_PAYMETHOD = 9, OLD_FLD_CP_DELIVERMETHOD = 10,
	OLD_FLD_CP_SUPPLIER = 11, OLD_FLD_CP_REPORT = 12, OLD_FLD_CP_NOTES = 13, OLD_FLD_CP_TYPE = 14,
};
#endif

bool updateCPTable ()
{
#ifdef TRANSITION_PERIOD
	VDB ()->database ()->exec ( QStringLiteral ( "RENAME TABLE `COMPANY_PURCHASES` TO `OLD_COMPANY_PURCHASES`" ) );
	VDB ()->createTable ( &companyPurchases::t_info );
	companyPurchases cp_rec ( false );

	QSqlQuery cp_query;
	if ( VDB ()->runQuery ( QStringLiteral ( "SELECT * FROM `OLD_COMPANY_PURCHASES`" ), cp_query ) ) {
		cp_rec.setAction ( ACTION_ADD );
		QString old_report, new_report;
		stringTable cp_pay_history;
		stringRecord rec;
		do {
			old_report = cp_query.record ().value ( OLD_FLD_CP_REPORT ).toString ();
			updateStrReport ( old_report, new_report );

			rec.insertField ( PHR_VALUE, cp_query.value ( OLD_FLD_CP_PAYVALUE ).toString () );
			rec.insertField ( PHR_METHOD, cp_query.value ( OLD_FLD_CP_PAYMETHOD ).toString () );
			rec.insertField ( PHR_DATE, cp_query.value ( OLD_FLD_CP_DATE ).toString () );
			rec.insertField ( PHR_PAID, cp_query.value ( OLD_FLD_CP_PAID ).toString () );
			cp_pay_history.fastAppendRecord ( rec );

			setRecValue ( &cp_rec, FLD_CP_DATE, cp_query.value ( OLD_FLD_CP_DATE ).toString () );
			setRecValue ( &cp_rec, FLD_CP_DELIVERY_DATE, cp_query.value ( OLD_FLD_CP_DELIVERDATE ).toString () );
			setRecValue ( &cp_rec, FLD_CP_DELIVERY_METHOD, cp_query.value ( OLD_FLD_CP_DELIVERMETHOD ).toString () );
			setRecValue ( &cp_rec, FLD_CP_NOTES, cp_query.value ( OLD_FLD_CP_NOTES ).toString () );
			setRecValue ( &cp_rec, FLD_CP_PAY_VALUE, cp_query.value ( OLD_FLD_CP_PAYVALUE ).toString () );
			setRecValue ( &cp_rec, FLD_CP_TOTAL_PRICE, cp_query.value ( OLD_FLD_CP_PRICE ).toString () );
			setRecValue ( &cp_rec, FLD_CP_ITEMS_REPORT, new_report );
			setRecValue ( &cp_rec, FLD_CP_PAY_REPORT, cp_pay_history.toString () );

			cp_rec.saveRecord ();
			rec.clear ();
			cp_pay_history.clear ();
			cp_rec.clearAll ();
			cp_rec.setAction ( ACTION_ADD );
		} while ( cp_query.next () );
		( void )VDB ()->database ()->exec ( QStringLiteral ( "DROP TABLE `OLD_COMPANY_PURCHASES`" ) );
		return true;
	}
#endif
	return false;
}

const TABLE_INFO companyPurchases::t_info = {
	COMPANY_PURCHASES_TABLE,
	QStringLiteral ( "COMPANY_PURCHASES" ),
	QStringLiteral ( " ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci" ),
	QStringLiteral ( " PRIMARY KEY ( `ID` ), UNIQUE KEY `id` ( `ID` )" ),
	QStringLiteral ( "`ID`|`DATE`|`DELIVERY_DATE`|`DELIVERY_METHOD`|`NOTES`|`SUPPLIER`|"
	"`TOTAL_PRICE`|`PAY_VALUE`|`ITEM_REPORT`|`PAY_REPORT`|" ),
	QStringLiteral ( " int ( 9 ) NOT NULL, | varchar ( 20 ) DEFAULT NULL, | varchar ( 20 ) DEFAULT NULL, |"
	" varchar ( 30 ) DEFAULT NULL, | varchar ( 200 ) DEFAULT NULL, | varchar ( 100 ) DEFAULT NULL, |"
	" varchar ( 30 ) DEFAULT NULL, | varchar ( 30 ) DEFAULT NULL, | longtext COLLATE utf8_unicode_ci DEFAULT NULL, |"
	" longtext COLLATE utf8_unicode_ci DEFAULT NULL, |" ),
	QCoreApplication::tr ( "ID|Purchase date|Delivery date|Delivery method|Notes|Supplier|Total price|Payment value|Items bought|Payment history|" ),
	CP_FIELDS_TYPE, TABLE_VERSION, COMPANY_PURCHASES_FIELD_COUNT, TABLE_CP_ORDER, &updateCPTable
	#ifdef TRANSITION_PERIOD
	, true
	#endif
};

static void updateCPPayReport ( const DBRecord* db_rec )
{
	//TODO
	APP_COMPLETERS ()->updateCompleter ( recStrValue ( db_rec, FLD_CP_PAY_REPORT ), vmCompleters::STOCK_TYPE );
}

static void updateSupplierTable ( const DBRecord* db_rec )
{
	supplierRecord::insertIfSupplierInexistent ( recStrValue ( db_rec, FLD_CP_SUPPLIER ) );
}

static void updateCPDeliverMethodCompleter ( const DBRecord* db_rec )
{
	APP_COMPLETERS ()->updateCompleter ( recStrValue ( db_rec, FLD_CP_DELIVERY_METHOD ), vmCompleters::DELIVERY_METHOD );
}

companyPurchases::companyPurchases ( const bool connect_helper_funcs )
	: DBRecord ( COMPANY_PURCHASES_FIELD_COUNT )
{
	::memset ( this->helperFunction, 0, sizeof ( this->helperFunction ) );
	DBRecord::t_info = &( this->t_info );
	DBRecord::m_RECFIELDS = this->m_RECFIELDS;

	if ( connect_helper_funcs ) {
		DBRecord::helperFunction = this->helperFunction;
		setHelperFunction ( FLD_CP_PAY_REPORT, &updateCPPayReport );
		setHelperFunction ( FLD_CP_SUPPLIER, &updateSupplierTable );
		setHelperFunction ( FLD_CP_DELIVERY_METHOD, &updateCPDeliverMethodCompleter );
	}
}

companyPurchases::~companyPurchases () {}
