#include "payment.h"
#include "vmlistitem.h"
#include "global.h"
#include "vivenciadb.h"
#include "stringrecord.h"

const double TABLE_VERSION ( 2.3 );

void updateOverdueInfo ( const DBRecord* db_rec );

bool updatePaymentTable ()
{
#ifdef TABLE_UPDATE_AVAILABLE
	Payment pay;
	if ( pay.readFirstRecord () )
	{
		Job job;
		do 
		{
			if ( recStrValue ( &pay, FLD_PAY_OVERDUE ) == CHR_ONE )
			{
				if ( job.readRecord ( static_cast<uint>(recIntValue ( &pay, FLD_PAY_JOBID )) ) )
				{
					if ( job.date ( FLD_JOB_ENDDATE ).year () < 2014 )
					{
						pay.setAction ( ACTION_EDIT );
						setRecValue ( &pay, FLD_PAY_OVERDUE, CHR_ZERO );
						pay.saveRecord ( true );
						pay.clearAll ();
					}
				}
			}
		} while ( pay.readNextRecord () );
	}
	return true;
#else
	return false;
#endif
}

const TABLE_INFO Payment::t_info = {
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

void updateOverdueInfo ( const DBRecord* db_rec )
{
	Payment* pay ( static_cast<Payment*> ( const_cast<DBRecord*> ( db_rec ) ) );

	if ( recStrValue ( pay, FLD_PAY_OVERDUE ) != CHR_TWO )
	{
		// CHR_TWO == ignore price differences.
		const vmNumber price ( recStrValue ( pay, FLD_PAY_PRICE ), VMNT_PRICE, 1 );
		const vmNumber total_paid ( recStrValue ( pay, FLD_PAY_TOTALPAID ), VMNT_PRICE, 1 );

		if ( total_paid < price)
		{
			setRecValue ( pay, FLD_PAY_OVERDUE, CHR_ONE );
			setRecValue ( pay, FLD_PAY_OVERDUE_VALUE, vmNumber ( price - total_paid ).toPrice () );
		}
		else
		{
			setRecValue ( pay, FLD_PAY_OVERDUE, CHR_ZERO );
			if ( pay->action () == ACTION_EDIT ) // clear field (if it contains a previous value) for neatness
				setRecValue ( pay, FLD_PAY_OVERDUE_VALUE, vmNumber::zeroedPrice.toPrice () );
		}
	}
}

Payment::Payment ( const bool )
	: DBRecord ( PAY_FIELD_COUNT )
{
	::memset ( this->helperFunction, 0, sizeof ( this->helperFunction ) );
	DBRecord::t_info = &( this->t_info );
	DBRecord::m_RECFIELDS = this->m_RECFIELDS;

/*	if ( connect_helper_funcs ) {
		DBRecord::helperFunction = this->helperFunction;
		setHelperFunction ( FLD_PAY_OVERDUE, &updateOverdueInfo );
	}*/

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
