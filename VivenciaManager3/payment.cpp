#include "payment.h"
#include "vmlistitem.h"
#include "global.h"
#include "vivenciadb.h"
#include "stringrecord.h"

static const unsigned int TABLE_VERSION ( 'A' );

bool updatePaymentTable ()
{
#ifdef TABLE_UPDATE_AVAILABLE
	VDB ()->optimizeTable ( &Payment::t_info );
	return true;
#else
	VDB ()->optimizeTable ( &Payment::t_info );
	return false;
#endif
}

const TABLE_INFO Payment::t_info =
{
	PAYMENT_TABLE,
	QStringLiteral ( "PAYMENTS" ),
	QStringLiteral ( " ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci" ),
	QStringLiteral ( " PRIMARY KEY ( `ID` ) , UNIQUE KEY `id` ( `ID` ) " ),
	QStringLiteral ( "`ID`|`CLIENTID`|`JOBID`|`PRICE`|`TOTALPAYS`|`TOTALPAID`|`OBS`|`INFO`|`OVERDUE`|`OVERDUE_VALUE`|" ),
	QStringLiteral ( " int ( 9 ) NOT NULL, | int ( 9 ) NOT NULL, | int ( 9 ) DEFAULT NULL, | varchar ( 30 ) DEFAULT NULL, |"
	" varchar ( 30 ) DEFAULT NULL, | varchar ( 30 ) DEFAULT NULL, | varchar ( 100 ) COLLATE utf8_unicode_ci DEFAULT NULL, |"
	" longtext COLLATE utf8_unicode_ci DEFAULT NULL, | varchar ( 4 ) DEFAULT NULL, | varchar ( 30 ) DEFAULT NULL, |" ),
	QStringLiteral ( "ID|Client ID|Job ID|Price|Number of payments|Total paid|Observations|Info|Overdue|Overdue value|" ),
	PAYS_FIELDS_TYPE, TABLE_VERSION, PAY_FIELD_COUNT, TABLE_PAY_ORDER, &updatePaymentTable
	#ifdef TRANSITION_PERIOD
	, true
	#endif
};

Payment::Payment ( const bool )
	: DBRecord ( PAY_FIELD_COUNT )
{
	::memset ( this->helperFunction, 0, sizeof ( this->helperFunction ) );
	DBRecord::t_info = &( this->t_info );
	DBRecord::m_RECFIELDS = this->m_RECFIELDS;
}

Payment::~Payment () {}

int Payment::searchCategoryTranslate ( const SEARCH_CATEGORIES sc ) const
{
	switch ( sc )
	{
		case SC_ID:
			return FLD_PAY_ID;
		case SC_REPORT_2:
			return FLD_PAY_OBS;
		case SC_PRICE_1:
			return FLD_PAY_PRICE;
		case SC_EXTRA_1:
			return FLD_PAY_CLIENTID;
		case SC_EXTRA_2:
			return FLD_PAY_JOBID;
		default:
			return -1;
	}
}

void Payment::copySubRecord ( const uint subrec_field, const stringRecord& subrec )
{
	if ( subrec_field == FLD_PAY_INFO )
	{
		if ( subrec.curIndex () == -1 )
			subrec.first ();
		stringRecord pay_info;
		for ( uint i ( 0 ); i < PAY_INFO_FIELD_COUNT; ++i )
		{
			pay_info.fastAppendValue ( subrec.curValue () );
			if ( !subrec.next () ) break;
		}
		setRecValue ( this, FLD_PAY_INFO, pay_info.toString () );
	}
}

void Payment::setListItem ( payListItem* pay_item )
{
	DBRecord::mListItem = static_cast<vmListItem*>( pay_item );
}

payListItem* Payment::payItem () const
{
	return static_cast<payListItem*>( DBRecord::mListItem );
}
