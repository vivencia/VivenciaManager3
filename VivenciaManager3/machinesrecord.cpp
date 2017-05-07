#include "machinesrecord.h"
#include "global.h"
#include "vivenciadb.h"

static const unsigned int TABLE_VERSION ( 'A' );

static const uint MACHINES_FIELDS_TYPE[MACHINES_FIELD_COUNT] =
{
	DBTYPE_ID, DBTYPE_ID, DBTYPE_SHORTTEXT, DBTYPE_SHORTTEXT, DBTYPE_SHORTTEXT,
	DBTYPE_SUBRECORD, DBTYPE_SUBRECORD, DBTYPE_SUBRECORD, DBTYPE_SUBRECORD
};

bool updateMachinesTable ()
{
#ifdef TABLE_UPDATE_AVAILABLE
	VDB ()->optimizeTable ( &machinesRecord::t_info );
	return true;
#else
	VDB ()->optimizeTable ( &machinesRecord::t_info );
	return false;
#endif //TABLE_UPDATE_AVAILABLE
}

const TABLE_INFO machinesRecord::t_info =
{
	MACHINES_TABLE,
	QStringLiteral ( "MACHINES" ),
	QStringLiteral ( " ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci" ),
	QStringLiteral ( " PRIMARY KEY ( `ID` ) , UNIQUE KEY `id` ( `ID` ) " ),
	QStringLiteral ( "`ID`|`SUPPLIES_ID`|`NAME`|`BRAND`|`TYPE`|`EVENTS`|`EVENT_DATES`|`EVENT_TIMES`|`EVENT_JOBS`|" ),
	QStringLiteral ( " int ( 9 ) NOT NULL, | int ( 9 ) DEFAULT NULL, | varchar ( 200 ) COLLATE utf8_unicode_ci DEFAULT NULL, |"
	" varchar ( 100 ) COLLATE utf8_unicode_ci DEFAULT NULL, | varchar ( 80 ) DEFAULT NULL, |"
	" longtext COLLATE utf8_unicode_ci, | longtext COLLATE utf8_unicode_ci, | longtext COLLATE utf8_unicode_ci, |"
	" longtext COLLATE utf8_unicode_ci, |" ),
	QStringLiteral ( "ID|Supplies ID|Name|Brand|Type|Events|Events dates|Events times|Events jobs|" ),
	MACHINES_FIELDS_TYPE, TABLE_VERSION, MACHINES_FIELD_COUNT, TABLE_MACHINES_ORDER, &updateMachinesTable
	#ifdef TRANSITION_PERIOD
	, true
	#endif
};

machinesRecord::machinesRecord ()
	: DBRecord ( MACHINES_FIELD_COUNT )
{
	::memset ( this->helperFunction, 0, sizeof ( this->helperFunction ) );
	DBRecord::t_info = & ( this->t_info );
	DBRecord::m_RECFIELDS = this->m_RECFIELDS;
	DBRecord::helperFunction = this->helperFunction;
}

machinesRecord::~machinesRecord () {}
