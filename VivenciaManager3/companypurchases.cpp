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

bool updateCPTable ()
{
#ifdef TABLE_UPDATE_AVAILABLE
	return true;
#else
	return false;
#endif //TABLE_UPDATE_AVAILABLE
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

	if ( connect_helper_funcs )
	{
		DBRecord::helperFunction = this->helperFunction;
		setHelperFunction ( FLD_CP_PAY_REPORT, &updateCPPayReport );
		setHelperFunction ( FLD_CP_SUPPLIER, &updateSupplierTable );
		setHelperFunction ( FLD_CP_DELIVERY_METHOD, &updateCPDeliverMethodCompleter );
	}
}

companyPurchases::~companyPurchases () {}
