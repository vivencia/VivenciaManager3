#include "standardprices.h"

static const double TABLE_VERSION ( 1.0 );

static const uint SERVICES_FIELDS_TYPE[SERVICES_FIELD_COUNT] = {
	DBTYPE_ID, DBTYPE_SHORTTEXT, DBTYPE_SHORTTEXT, DBTYPE_PRICE, DBTYPE_DATE, DBTYPE_TIME,
	DBTYPE_SHORTTEXT, DBTYPE_SHORTTEXT
};

const TABLE_INFO servicesOffered::t_info =
{
	SERVICES_TABLE,
	QStringLiteral ( "SERVICES" ),
	QStringLiteral ( " ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci" ),
	QStringLiteral ( " PRIMARY KEY (`ID`), UNIQUE KEY `id` (`ID`)" ),
	QStringLiteral ( "`ID`|`TYPE`|`BASIC_UNIT`|`PRICE_PER_UNIT`|`LAST_UPDATE`|`AVERAGE_TIME`|`DISCOUNT_FROM`|`DISCOUNT_FACTOR`|" ),
	QStringLiteral ( " int(9) NOT NULL, | varchar(150) COLLATE utf8_unicode_ci DEFAULT NULL, | varchar(10) DEFAULT NULL, | varchar(20) DEFAULT NULL, |"
	" varchar(20) DEFAULT NULL, | varchar(20) DEFAULT NULL, | varchar(20) DEFAULT NULL, | varchar(20) DEFAULT NULL, |" ),
	QStringLiteral ( "ID|Service type|Basic unit|Price per unit|Last update|Execution average time|Discount from (units)|Discount factor|" ),
	SERVICES_FIELDS_TYPE, TABLE_VERSION, SERVICES_FIELD_COUNT, TABLE_SERVICES_ORDER, nullptr
	#ifdef TRANSITION_PERIOD
	, false
	#endif
};

servicesOffered::servicesOffered ()
	: DBRecord ( SERVICES_FIELD_COUNT )
{
	::memset ( this->helperFunction, 0, sizeof ( this->helperFunction ) );
	DBRecord::t_info = & ( servicesOffered::t_info );
	DBRecord::m_RECFIELDS = this->m_RECFIELDS;
	DBRecord::helperFunction = this->helperFunction;
}

servicesOffered::~servicesOffered () {}
