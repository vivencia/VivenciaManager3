#include "standardprices.h"
#include "vivenciadb.h"

static const unsigned int TABLE_VERSION ( 'A' );

constexpr DB_FIELD_TYPE SERVICES_FIELDS_TYPES[SERVICES_FIELD_COUNT] =
{
	DBTYPE_ID, DBTYPE_SHORTTEXT, DBTYPE_SHORTTEXT, DBTYPE_PRICE, DBTYPE_DATE, DBTYPE_TIME,
	DBTYPE_SHORTTEXT, DBTYPE_SHORTTEXT
};

#ifdef SP_TABLE_UPDATE_AVAILABLE
bool updateServicesOffered ()
{
	VDB ()->optimizeTable ( &servicesOffered::t_info );
	return true;
}
#endif //SP_TABLE_UPDATE_AVAILABLE

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
	SERVICES_FIELDS_TYPES, TABLE_VERSION, SERVICES_FIELD_COUNT, TABLE_SERVICES_ORDER,
	#ifdef SP_TABLE_UPDATE_AVAILABLE
	&updateServicesOffered
	#else
	nullptr
	#endif //SP_TABLE_UPDATE_AVAILABLE
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
	DBRecord::mFieldsTypes = SERVICES_FIELDS_TYPES;
	DBRecord::helperFunction = this->helperFunction;
}

servicesOffered::~servicesOffered () {}
