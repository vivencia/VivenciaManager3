#include "dbcalendar.h"
#include "global.h"
#include "global_enums.h"
#include "stringrecord.h"

#include <QCoreApplication>

const double TABLE_VERSION ( 1.5 );

const uint CALENDAR_FIELDS_TYPE[CALENDAR_FIELD_COUNT] = {
	DBTYPE_ID, DBTYPE_DATE, DBTYPE_NUMBER, DBTYPE_NUMBER, DBTYPE_SHORTTEXT,
	DBTYPE_SHORTTEXT, DBTYPE_SHORTTEXT, DBTYPE_SHORTTEXT, DBTYPE_SHORTTEXT
};

inline uint monthToDBMonth ( const vmNumber& date )
{
	return ( date.year () - 2009 ) * 12 + date.month ();
}

inline uint DBMonthToMonth ( const uint dbmonth )
{
    const int year ( dbmonth / 12 );
    return ( dbmonth - ( year * 12 ) );
}

inline void DBMonthToDate ( const uint dbmonth, vmNumber& date )
{
    const int year ( dbmonth / 12 );
    date.setDate( 1, DBMonthToMonth( dbmonth ), year + 2009 );
}

inline uint weekNumberToDBWeekNumber ( const vmNumber& date )
{
	return ( date.year () - 2009 ) * 100 + date.weekNumber ();
}

inline uint DBWeekNumberToWeekNumber ( const uint dbweek )
{
    const int year ( dbweek / 100 );
    return ( dbweek - ( year * 100 ) );
}

inline void DBWeekNumberToDate ( const uint dbweek, vmNumber& date )
{
	const int year ( dbweek / 100 );
	/* This wiil retrieve the week number back, but the day will not be the same (unless by coincidence)
	 * and the month might not be the same, as a week may have tow months (by comprising the end of one month
	 * and the beginning of the other)
	 */
    date.setDate ( ( DBWeekNumberToWeekNumber ( dbweek ) * 7 ) - 2, 1, year + 2009 );
}

#ifdef TRANSITION_PERIOD
#include "vivenciadb.h"
#include "calculator.h"

bool jobDate ( const stringTable& days, const uint day, vmNumber& date )
{
	if ( day < days.countRecords () ) {
		stringRecord strRecord ( days.readRecord ( day ) );
		if ( strRecord.isOK () ) {
			( void ) date.fromTrustedStrDate ( strRecord.fieldValue ( Job::JRF_DATE ), vmNumber::VDF_DB_DATE );
			return true;
		}
	}
	return false;
}

bool payDate ( const stringTable& pay_info, const uint day, vmNumber& date,
			   vmNumber& price, const bool use_date )
{
	if ( day < pay_info.countRecords () ) {
		stringRecord strRecord ( pay_info.readRecord ( day ) );
		if ( strRecord.isOK () ) {
			const vmNumber test_date ( strRecord.fieldValue (
										   use_date ? PHR_USE_DATE : PHR_DATE ), VMNT_DATE, vmNumber::VDF_DB_DATE );
			if ( date != test_date ) {
				if ( strRecord.fieldValue ( PHR_PAID ) == CHR_ONE )
					( void ) price.fromStrPrice ( strRecord.fieldValue ( PHR_VALUE ) );
				else
					price.clear ( false );
				date = test_date;
				return true;
			}
			date = test_date;
		}
	}
	return false;
}

void updateCalendarWithJobInfo ( const Job* const job )
{
	uint day ( 0 );
	vmNumber price ( 0.0 ), priceJobs ( 0.0 ) ;
	dbCalendar cal_rec;
	vmNumber date;
	stringRecord jobids;
	stringTable ids_table;

	const stringTable jobinfo ( recStrValue ( job, FLD_JOB_REPORT ) );
	if ( !jobinfo.isOK () ) return;

	( void ) price.fromTrustedStrPrice ( recStrValue ( job, FLD_JOB_PRICE ) );
	price /= jobinfo.countRecords ();

	while ( jobDate ( jobinfo, day, date ) ) {
		if ( date.year () >= 2009 ) {
			if ( cal_rec.readRecord ( FLD_CALENDAR_DAY_DATE, date.toDate ( vmNumber::VDF_DB_DATE ) ) ) {
				cal_rec.setAction ( ACTION_EDIT );
				ids_table = recStrValue ( &cal_rec, FLD_CALENDAR_JOBS );
				priceJobs.fromTrustedStrPrice ( recStrValue ( &cal_rec, FLD_CALENDAR_TOTAL_JOBPRICE_SCHEDULED ) );
			}
			else {
				cal_rec.setAction ( ACTION_ADD );
				cal_rec.clearAll ();
				ids_table.clear ();
				priceJobs.clear ( false );
				setRecValue ( &cal_rec, FLD_CALENDAR_DAY_DATE, date.toDate ( vmNumber::VDF_DB_DATE ) );
				setRecValue ( &cal_rec, FLD_CALENDAR_WEEK_NUMBER, QString::number ( weekNumberToDBWeekNumber ( date ) ) );
				setRecValue ( &cal_rec, FLD_CALENDAR_MONTH, QString::number ( monthToDBMonth ( date ) ) );
				setRecValue ( &cal_rec, FLD_CALENDAR_TOTAL_PAY_RECEIVED, vmNumber::zeroedPrice.toPrice () );
				setRecValue ( &cal_rec, FLD_CALENDAR_TOTAL_BUY_PAID, vmNumber::zeroedPrice.toPrice () );
				setRecValue ( &cal_rec, FLD_CALENDAR_TOTAL_BUY_BOUGHT, vmNumber::zeroedPrice.toPrice () );
			}
			jobids.clear ();
			jobids.fastAppendValue ( recStrValue ( job, FLD_JOB_ID ) );
			jobids.fastAppendValue ( recStrValue ( job, FLD_JOB_CLIENTID ) );
            jobids.fastAppendValue ( QString::number ( day + 1 ) );
			ids_table.fastAppendRecord ( jobids );
			setRecValue ( &cal_rec, FLD_CALENDAR_JOBS, ids_table.toString () );
			if ( !price.isNull () ) {
				priceJobs += price;
				setRecValue ( &cal_rec, FLD_CALENDAR_TOTAL_JOBPRICE_SCHEDULED, priceJobs.toPrice () );
			}
			cal_rec.saveRecord ();
		}
		day++;
	}
}

void updateCalendarWithPayInfo ( const Payment* const pay )
{
	dbCalendar cal_rec;
	uint day ( 0 );
	vmNumber price ( 0.0 ), pricePays ( 0.0 ) ;
	vmNumber date;
	stringRecord payids;
	stringTable ids_table;

	const stringTable pay_info ( recStrValue ( pay, FLD_PAY_INFO ) );
	if ( !pay_info.isOK () ) return;

	while ( payDate ( pay_info, day, date, price, false ) ) {
		if ( date.year () >= 2009 ) {
			if ( cal_rec.readRecord ( FLD_CALENDAR_DAY_DATE, date.toDate ( vmNumber::VDF_DB_DATE ) ) ) {
				cal_rec.setAction ( ACTION_EDIT );
				ids_table.fromString ( recStrValue ( &cal_rec, FLD_CALENDAR_PAYS ) );
				pricePays.fromStrPrice ( recStrValue ( &cal_rec, FLD_CALENDAR_TOTAL_PAY_RECEIVED ) );
			}
			else {
				cal_rec.setAction ( ACTION_ADD );
				cal_rec.clearAll ();
				ids_table.clear ();
				pricePays.clear ( false );
				setRecValue ( &cal_rec, FLD_CALENDAR_DAY_DATE, date.toDate ( vmNumber::VDF_DB_DATE ) );
				setRecValue ( &cal_rec, FLD_CALENDAR_WEEK_NUMBER, QString::number ( weekNumberToDBWeekNumber ( date ) ) );
				setRecValue ( &cal_rec, FLD_CALENDAR_MONTH, QString::number ( date.month () ) );
				setRecValue ( &cal_rec, FLD_CALENDAR_TOTAL_JOBPRICE_SCHEDULED, vmNumber::zeroedPrice.toPrice () );
				setRecValue ( &cal_rec, FLD_CALENDAR_TOTAL_BUY_PAID, vmNumber::zeroedPrice.toPrice () );
				setRecValue ( &cal_rec, FLD_CALENDAR_TOTAL_BUY_BOUGHT, vmNumber::zeroedPrice.toPrice () );

			}
			payids.clear ();
			payids.fastAppendValue ( recStrValue ( pay, FLD_PAY_ID ) );
			payids.fastAppendValue ( recStrValue ( pay, FLD_PAY_CLIENTID ) );
			ids_table.fastAppendRecord ( payids );
			setRecValue ( &cal_rec, FLD_CALENDAR_PAYS, ids_table.toString () );

			if ( !price.isNull () ) {
				pricePays += price;
				setRecValue ( &cal_rec, FLD_CALENDAR_TOTAL_PAY_RECEIVED, pricePays.toPrice () );
			}
			cal_rec.saveRecord ();
		}
		day++;
	}

	cal_rec.clearAll ();
	ids_table.clear ();
	day = 0;
	date.clear ( false );

	while ( payDate ( pay_info, day, date, price, true ) ) {
		if ( cal_rec.readRecord ( FLD_CALENDAR_DAY_DATE, date.toDate ( vmNumber::VDF_DB_DATE ) ) ) {
			cal_rec.setAction ( ACTION_EDIT );
			ids_table.fromString ( recStrValue ( &cal_rec, FLD_CALENDAR_PAYS_USE ) );
		}
		else {
			cal_rec.setAction ( ACTION_ADD );
			cal_rec.clearAll ();
			ids_table.clear ();
			setRecValue ( &cal_rec, FLD_CALENDAR_DAY_DATE, date.toDate ( vmNumber::VDF_DB_DATE ) );
			setRecValue ( &cal_rec, FLD_CALENDAR_WEEK_NUMBER, QString::number ( weekNumberToDBWeekNumber ( date ) ) );
			setRecValue ( &cal_rec, FLD_CALENDAR_MONTH, QString::number ( date.month () ) );
			setRecValue ( &cal_rec, FLD_CALENDAR_TOTAL_JOBPRICE_SCHEDULED, vmNumber::zeroedPrice.toPrice () );
			setRecValue ( &cal_rec, FLD_CALENDAR_TOTAL_BUY_PAID, vmNumber::zeroedPrice.toPrice () );
			setRecValue ( &cal_rec, FLD_CALENDAR_TOTAL_BUY_BOUGHT, vmNumber::zeroedPrice.toPrice () );
		}

		payids.clear ();
		payids.fastAppendValue ( recStrValue ( pay, FLD_PAY_ID ) );
		payids.fastAppendValue ( recStrValue ( pay, FLD_PAY_CLIENTID ) );
		ids_table.fastAppendRecord ( payids );
		setRecValue ( &cal_rec, FLD_CALENDAR_PAYS_USE, ids_table.toString () );
		cal_rec.saveRecord ();
		day++;
	}
}

void updateCalendarWithBuyInfo ( const Buy* const buy )
{
	dbCalendar cal_rec;
	vmNumber price ( 0.0 ), priceBuys ( 0.0 ), priceBuysPaid ( 0.0 ) ;
	vmNumber date ( recStrValue ( buy, FLD_BUY_DATE ), VMNT_DATE, vmNumber::VDF_DB_DATE );
	stringRecord buyids;
	stringTable ids_table;

	if ( date.year () >= 2009 ) {
		if ( cal_rec.readRecord ( FLD_CALENDAR_DAY_DATE, date.toDate ( vmNumber::VDF_DB_DATE ) ) ) {
			cal_rec.setAction ( ACTION_EDIT );
			ids_table.fromString ( recStrValue ( &cal_rec, FLD_CALENDAR_BUYS ) );
			priceBuys.fromStrPrice ( recStrValue ( &cal_rec, FLD_CALENDAR_TOTAL_BUY_BOUGHT ) );
		}
		else {
			cal_rec.setAction ( ACTION_ADD );
			cal_rec.clearAll ();
			ids_table.clear ();
			priceBuys.clear ( false );
			setRecValue ( &cal_rec, FLD_CALENDAR_DAY_DATE, date.toDate ( vmNumber::VDF_DB_DATE ) );
			setRecValue ( &cal_rec, FLD_CALENDAR_WEEK_NUMBER, QString::number ( weekNumberToDBWeekNumber ( date ) ) );
			setRecValue ( &cal_rec, FLD_CALENDAR_MONTH, QString::number ( date.month () ) );
			setRecValue ( &cal_rec, FLD_CALENDAR_TOTAL_JOBPRICE_SCHEDULED, vmNumber::zeroedPrice.toPrice () );
			setRecValue ( &cal_rec, FLD_CALENDAR_TOTAL_PAY_RECEIVED, vmNumber::zeroedPrice.toPrice () );
			setRecValue ( &cal_rec, FLD_CALENDAR_TOTAL_BUY_PAID, vmNumber::zeroedPrice.toPrice () );
		}

		buyids.clear ();
		buyids.fastAppendValue ( recStrValue ( buy, FLD_BUY_ID ) );
		buyids.fastAppendValue ( recStrValue ( buy, FLD_BUY_CLIENTID ) );
		ids_table.fastAppendRecord ( buyids );
		setRecValue ( &cal_rec, FLD_CALENDAR_BUYS, ids_table.toString () );

		price.fromStrPrice ( recStrValue ( buy, FLD_BUY_PRICE ) );
		if ( !price.isNull () ) {
			priceBuys += price;
			setRecValue ( &cal_rec, FLD_CALENDAR_TOTAL_BUY_BOUGHT, priceBuys.toPrice () );
		}
		cal_rec.saveRecord ();
	}

	cal_rec.clearAll ();
	ids_table.clear ();
	const stringTable pay_info ( recStrValue ( buy, FLD_BUY_PAYINFO ) );
	bool b_paid ( false );
	const stringRecord* rec ( nullptr );
	rec = &( pay_info.first () );
	while ( rec->isOK () ) {
		date.fromTrustedStrDate ( rec->fieldValue ( PHR_DATE ), vmNumber::VDF_DB_DATE );
		if ( date.year () >= 2009 ) {
			b_paid = rec->fieldValue ( PHR_PAID ) == CHR_ONE;
			if ( b_paid )
				price.fromStrPrice ( rec->fieldValue ( PHR_VALUE ) );
			else
				price.clear ( false );

			if ( cal_rec.readRecord ( FLD_CALENDAR_DAY_DATE, date.toDate ( vmNumber::VDF_DB_DATE ) ) ) {
				cal_rec.setAction ( ACTION_EDIT );
				ids_table.fromString ( recStrValue ( &cal_rec, FLD_CALENDAR_BUYS_PAY ) );
				priceBuysPaid.fromStrPrice ( recStrValue ( &cal_rec, FLD_CALENDAR_TOTAL_BUY_PAID ) );
			}
			else {
				cal_rec.setAction ( ACTION_ADD );
				cal_rec.clearAll ();
				setRecValue ( &cal_rec, FLD_CALENDAR_DAY_DATE, date.toDate ( vmNumber::VDF_DB_DATE ) );
				setRecValue ( &cal_rec, FLD_CALENDAR_WEEK_NUMBER, QString::number ( weekNumberToDBWeekNumber ( date ) ) );
				setRecValue ( &cal_rec, FLD_CALENDAR_MONTH, QString::number ( date.month () ) );
				setRecValue ( &cal_rec, FLD_CALENDAR_TOTAL_JOBPRICE_SCHEDULED, vmNumber::zeroedPrice.toPrice () );
				setRecValue ( &cal_rec, FLD_CALENDAR_TOTAL_PAY_RECEIVED, vmNumber::zeroedPrice.toPrice () );
				setRecValue ( &cal_rec, FLD_CALENDAR_TOTAL_BUY_BOUGHT, vmNumber::zeroedPrice.toPrice () );
				ids_table.clear ();
				priceBuysPaid.clear ( false );
			}

			buyids.clear ();
			buyids.fastAppendValue ( recStrValue ( buy, FLD_BUY_ID ) );
			buyids.fastAppendValue ( recStrValue ( buy, FLD_BUY_CLIENTID ) );
			ids_table.fastAppendRecord ( buyids );
			setRecValue ( &cal_rec, FLD_CALENDAR_BUYS_PAY, ids_table.toString () );

			if ( !price.isNull () )
				priceBuysPaid += price;
			setRecValue ( &cal_rec, FLD_CALENDAR_TOTAL_BUY_PAID, priceBuysPaid.toPrice () );
			cal_rec.saveRecord ();
		}
		rec = &( pay_info.next () );
	}
}
#endif

bool updateCalendarTable ()
{
#ifdef TRANSITION_PERIOD

    VDB ()->createTable ( &dbCalendar::t_info );
    Calculator::init ();

	Job job;
	if ( job.readFirstRecord () ) {
		do {
			updateCalendarWithJobInfo ( &job );
		} while ( job.readNextRecord () );
    }

    Payment pay;
    if ( pay.readFirstRecord () ) {
        do {
            updateCalendarWithPayInfo ( &pay );
        } while ( pay.readNextRecord () );
    }

    Buy buy;
    if ( buy.readFirstRecord () ) {
        do {
            updateCalendarWithBuyInfo ( &buy );
        } while ( buy.readNextRecord () );
	}
    VDB ()->optimizeTable( &dbCalendar::t_info );
	return true;
#endif
	return false;
}

const TABLE_INFO dbCalendar::t_info = {
	CALENDAR_TABLE,
	QStringLiteral ( "CALENDAR" ),
	QStringLiteral ( " ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci" ),
	QStringLiteral ( " PRIMARY KEY ( `ID` ) , UNIQUE KEY `id` ( `ID` ) " ),
	QStringLiteral ( "`ID`|`DAY_DATE`|`WEEK_NUMBER`|`MONTH`|`JOBS`|`PAYS`|`PAYS_USE`|"
	"`BUYS`|`BUYS_PAY`|`TOTAL_JOBPRICE_SCHEDULED`|`TOTAL_PAY_RECEIVED`|"
	"`TOTAL_BUY_BOUGHT`|`TOTAL_BUY_PAID`|" ),
	QStringLiteral ( " int ( 9 ) NOT NULL, | varchar ( 30 ) NOT NULL, | int ( 4 ) DEFAULT NULL, |"
	" int ( 4 ) DEFAULT NULL, | varchar ( 100 ) DEFAULT NULL, | varchar ( 100 ) DEFAULT NULL, |"
	" varchar ( 100 ) DEFAULT NULL, | varchar ( 100 ) DEFAULT NULL, | varchar ( 100 ) DEFAULT NULL, |"
	" varchar ( 50 ) DEFAULT NULL, | varchar ( 50 ) DEFAULT NULL, | varchar ( 50 ) DEFAULT NULL, |"
	" varchar ( 50 ) DEFAULT NULL, |" ),
	QCoreApplication::tr ( "ID|Date|Week Number|Month|Jobs|Payments|Deposit date|Purchases|"
	"Purchase pay date|Scheduled amount to receive|Amount actually receive|"
	"Purchases made|Purchases paid|" ),
	CALENDAR_FIELDS_TYPE, TABLE_VERSION, CALENDAR_FIELD_COUNT, TABLE_CALENDAR_ORDER, &updateCalendarTable
	#ifdef TRANSITION_PERIOD
	, true
	#endif
};

dbCalendar::dbCalendar ()
	: DBRecord ( CALENDAR_FIELD_COUNT )
{
	::memset ( this->helperFunction, 0, sizeof ( this->helperFunction ) );
	DBRecord::t_info = &( dbCalendar::t_info );
	DBRecord::m_RECFIELDS = this->m_RECFIELDS;
	DBRecord::helperFunction = this->helperFunction;
}

dbCalendar::~dbCalendar () {}

const stringTable& dbCalendar::dateLog ( const vmNumber& date, const uint search_field, const uint requested_field,
        QString& price, const uint requested_field_price, const bool bIncludeDates ) const
{
	mStrTable.clear ();
	if ( requested_field < FLD_CALENDAR_JOBS || requested_field > FLD_CALENDAR_BUYS_PAY )
		return mStrTable;

	dbCalendar cal_rec;
	vmNumber total_price;

	switch ( search_field ) {
		case FLD_CALENDAR_DAY_DATE:
			if ( cal_rec.readRecord ( FLD_CALENDAR_DAY_DATE, date.toDate ( vmNumber::VDF_DB_DATE ) ) ) {
				price = recStrValue ( &cal_rec, requested_field_price );
				mStrTable.fromString ( recStrValue ( &cal_rec, requested_field ) );
			}
		break;
		case FLD_CALENDAR_WEEK_NUMBER:
		case FLD_CALENDAR_MONTH:
			if ( cal_rec.readFirstRecord ( search_field, QString::number ( search_field == FLD_CALENDAR_MONTH ?
                                    monthToDBMonth ( date ) : weekNumberToDBWeekNumber ( date ) ) ) ) {
                uint i_row ( 0 );
				do {
					mStrTable.appendTable ( recStrValue ( &cal_rec, requested_field ) );
                    if ( bIncludeDates ) {
                        for ( ; i_row < mStrTable.countRecords (); ++i_row )
                            mStrTable.changeRecord ( i_row, 3, recStrValue ( &cal_rec, FLD_CALENDAR_DAY_DATE ) );
                    }
					total_price += vmNumber ( recStrValue ( &cal_rec, requested_field_price ), VMNT_PRICE, 1 );
				} while ( cal_rec.readNextRecord ( true ) );
				price = total_price.toPrice ();
			}
		break;
		default:
		break;
	}
	return mStrTable;
}

void dbCalendar::addDate ( const vmNumber& date, const uint field, const stringRecord& id_trio )
{
	dbCalendar cal_rec;
	stringTable ids;
	if ( cal_rec.readRecord ( FLD_CALENDAR_DAY_DATE, date.toDate ( vmNumber::VDF_DB_DATE ) ) ) {
		ids.fromString ( recStrValue ( &cal_rec, field ) );
        if ( ids.toString () == id_trio.toString () )
            return;
        cal_rec.setAction ( ACTION_EDIT );
	}
	else {
		cal_rec.setAction ( ACTION_ADD );
		setRecValue ( &cal_rec, FLD_CALENDAR_DAY_DATE, date.toDate ( vmNumber::VDF_DB_DATE ) );
		setRecValue ( &cal_rec, FLD_CALENDAR_WEEK_NUMBER, QString::number ( weekNumberToDBWeekNumber ( date ) ) );
		setRecValue ( &cal_rec, FLD_CALENDAR_MONTH, QString::number ( monthToDBMonth ( date ) ) );
		setRecValue ( &cal_rec, FLD_CALENDAR_TOTAL_JOBPRICE_SCHEDULED, vmNumber::zeroedPrice.toPrice () );
		setRecValue ( &cal_rec, FLD_CALENDAR_TOTAL_PAY_RECEIVED, vmNumber::zeroedPrice.toPrice () );
		setRecValue ( &cal_rec, FLD_CALENDAR_TOTAL_BUY_PAID, vmNumber::zeroedPrice.toPrice () );
		setRecValue ( &cal_rec, FLD_CALENDAR_TOTAL_BUY_BOUGHT, vmNumber::zeroedPrice.toPrice () );
	}
	ids.fastAppendRecord ( id_trio );
	setRecValue ( &cal_rec, field, ids.toString () );
	cal_rec.saveRecord ();
}

void dbCalendar::delDate ( const vmNumber& date, const uint field, const stringRecord& id_trio )
{
	dbCalendar cal_rec;
	if ( cal_rec.readRecord ( FLD_CALENDAR_DAY_DATE, date.toDate ( vmNumber::VDF_DB_DATE ) ) ) {
        cal_rec.setAction ( ACTION_EDIT );
		stringTable ids;
		ids.fromString ( recStrValue ( &cal_rec, field ) );
		if ( ids.removeRecordByValue ( id_trio ) ) {
			setRecValue ( &cal_rec, field, ids.toString () );
			cal_rec.saveRecord ();
		}
	}
}

void dbCalendar::addPrice ( const vmNumber& date, const vmNumber& price, const uint field )
{
	dbCalendar cal_rec;
	vmNumber new_price ( price );
	if ( cal_rec.readRecord ( FLD_CALENDAR_DAY_DATE, date.toDate ( vmNumber::VDF_DB_DATE ) ) ) {
		const vmNumber old_price ( recStrValue ( &cal_rec, field ), VMNT_PRICE, 1 );
		new_price += old_price;
        cal_rec.setAction ( ACTION_EDIT );
	}
	else {
		cal_rec.setAction ( ACTION_ADD );
		setRecValue ( &cal_rec, FLD_CALENDAR_DAY_DATE, date.toDate ( vmNumber::VDF_DB_DATE ) );
		setRecValue ( &cal_rec, FLD_CALENDAR_WEEK_NUMBER, QString::number ( weekNumberToDBWeekNumber ( date ) ) );
		setRecValue ( &cal_rec, FLD_CALENDAR_MONTH, QString::number ( monthToDBMonth ( date ) ) );
	}
	setRecValue ( &cal_rec, field, new_price.toPrice () );
	cal_rec.saveRecord ();
}

void dbCalendar::delPrice ( const vmNumber& date, const vmNumber& price, const uint field )
{
	dbCalendar cal_rec;
	if ( !price.isNull () ) {
		if ( cal_rec.readRecord ( FLD_CALENDAR_DAY_DATE, date.toDate ( vmNumber::VDF_DB_DATE ) ) ) {
            cal_rec.setAction ( ACTION_EDIT );
			const vmNumber old_price ( recStrValue ( &cal_rec, field ), VMNT_PRICE, 1 );
            const vmNumber new_price ( old_price - price );
			setRecValue ( &cal_rec, field, new_price.toPrice () );
			cal_rec.saveRecord ();
		}
	}
}

void dbCalendar::addCalendarExchangeRule ( PointersList<CALENDAR_EXCHANGE*>& ce_list,
		const CALENDAR_EXCHANGE_ACTION_ORDER action,
		const vmNumber& date, const vmNumber& price, const int extra_info )
{
	if ( action == CEAO_NOTHING )
		return;
	CALENDAR_EXCHANGE* ce ( new CALENDAR_EXCHANGE );
	ce->action = action;
	ce->date = date;
	ce->price = price;
	ce->extra_info = extra_info;
	ce_list.append ( ce );
}
