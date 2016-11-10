#include "dbcalendar.h"
#include "global.h"
#include "global_enums.h"
#include "stringrecord.h"

#include <QCoreApplication>

const double TABLE_VERSION ( 1.7 );

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
    date.setDate ( 1, DBMonthToMonth( dbmonth ), year + 2009 );
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

#include "vivenciadb.h"
#include "calculator.h"
#include "stringrecord.h"
#include "vmnotify.h"

bool updateCalendarTable ()
{
	//( void )VDB ()->database ()->exec ( QStringLiteral ( "RENAME TABLE `CALENDAR` TO `OLD_CALENDAR`" ) );
    //VDB ()->createTable ( &dbCalendar::t_info );

	const uint max_pbar_value ( VDB ()->getHighestID ( TABLE_JOB_ORDER ) + VDB ()->getHighestID ( TABLE_PAY_ORDER ) +
								VDB ()->getHighestID ( TABLE_BUY_ORDER ) - 10 );
	vmNotify* pBox ( nullptr );
	pBox = vmNotify::progressBox ( nullptr, nullptr, max_pbar_value, 0,
			QStringLiteral ( "Updating the Calendar database. This might take a while..." ),
			QStringLiteral ( "Starting...." ) );

	
	VDB ()->clearTable ( &dbCalendar::t_info );
    Calculator::init ();
	dbCalendar calendar;

	Job job;
	uint i ( 1 );
	if ( job.readFirstRecord () ) {
		do {
			pBox = vmNotify::progressBox ( pBox, nullptr, max_pbar_value, i++, QString::null, QStringLiteral( "Scanning the Jobs table" ) );
			calendar.updateCalendarWithJobInfo ( &job );
		} while ( job.readNextRecord () );
    }

    Payment pay;
    if ( pay.readFirstRecord () ) {
        do {
			pBox = vmNotify::progressBox ( pBox, nullptr, max_pbar_value, i++, QString::null, QStringLiteral( "Scanning the Payments table" ) );
            calendar.updateCalendarWithPayInfo ( &pay );
        } while ( pay.readNextRecord () );
    }

    Buy buy;
    if ( buy.readFirstRecord () ) {
        do {
			pBox = vmNotify::progressBox ( pBox, nullptr, max_pbar_value, i++, QString::null, QStringLiteral( "Scanning the Purchases table" ) );
            calendar.updateCalendarWithBuyInfo ( &buy );
        } while ( buy.readNextRecord () );
	}
	pBox = vmNotify::progressBox ( pBox, nullptr, max_pbar_value, i++, QString::null, QStringLiteral( "Done. Calendar table updated" ) );
    VDB ()->optimizeTable( &dbCalendar::t_info );
	return true;
	//return false;
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

void dbCalendar::updateCalendarWithJobInfo ( const Job* const job )
{
	const stringTable jobReport ( recStrValue ( job, FLD_JOB_REPORT ) );
	const int n_days ( static_cast<int>( jobReport.countRecords () ) );
	vmNumber date;
	const vmNumber pricePerDay ( job->price ( FLD_JOB_PRICE ) / n_days );
	stringRecord* dayRecord ( nullptr );
	PointersList<CALENDAR_EXCHANGE*> ce_list ( 5 );
	
	switch ( job->action () )
	{
		default: return;
		case ACTION_ADD:
		case ACTION_READ: // this is when updating the table. All read jobs will contain new info as though there new
		{					// The prevAction for those DBRecords that have just been created/read is ACTION_NONE
			for ( int i ( 0 ); i < n_days ; ++i )
			{
				dayRecord = const_cast<stringRecord*>(&jobReport.readRecord ( i ));
				date.fromTrustedStrDate ( dayRecord->fieldValue ( Job::JRF_DATE ), vmNumber::VDF_DB_DATE );
				addCalendarExchangeRule ( ce_list, CEAO_ADD_DATE1, date, vmNumber::emptyNumber, i + 1 );
				if ( !pricePerDay.isNull () )
					addCalendarExchangeRule ( ce_list, CEAO_ADD_PRICE_DATE1, date, pricePerDay );
			}
		}
		break;
		case ACTION_DEL:
		{
			for ( int i ( 0 ); i < n_days ; ++i )
			{
				dayRecord = const_cast<stringRecord*>(&jobReport.readRecord ( i ));
				date.fromTrustedStrDate ( dayRecord->fieldValue ( Job::JRF_DATE ), vmNumber::VDF_DB_DATE );
				addCalendarExchangeRule ( ce_list, CEAO_DEL_DATE1, date, vmNumber::emptyNumber, i + 1 );
				if ( !pricePerDay.isNull () )
					addCalendarExchangeRule ( ce_list, CEAO_DEL_PRICE_DATE1, date, pricePerDay );
			}
		}
		break;
		case ACTION_EDIT:
		{
			const stringTable& oldJobReport ( recStrValueAlternate ( job, FLD_JOB_REPORT ) );
			const int old_n_days ( static_cast<int>( oldJobReport.countRecords () ) );
			const vmNumber oldPricePerDay ( vmNumber ( recStrValueAlternate ( job, FLD_JOB_PRICE ), VMNT_PRICE, 1 ) / old_n_days );
			
			if ( job->wasModified ( FLD_JOB_REPORT ) )
			{	
				for ( int i ( 0 ); i < old_n_days ; ++i )
				{
					dayRecord = const_cast<stringRecord*>(&oldJobReport.readRecord ( i ));
					date.fromTrustedStrDate ( dayRecord->fieldValue ( Job::JRF_DATE ), vmNumber::VDF_DB_DATE );
					addCalendarExchangeRule ( ce_list, CEAO_DEL_DATE1, date, vmNumber::emptyNumber, i + 1 );
					if ( !oldPricePerDay.isNull () )
						addCalendarExchangeRule ( ce_list, CEAO_DEL_PRICE_DATE1, date, pricePerDay );
				}
				
				for ( int i ( 0 ); i < n_days ; ++i )
				{
					dayRecord = const_cast<stringRecord*>(&oldJobReport.readRecord ( i ));
					date.fromTrustedStrDate ( dayRecord->fieldValue ( Job::JRF_DATE ), vmNumber::VDF_DB_DATE );
					addCalendarExchangeRule ( ce_list, CEAO_ADD_DATE1, date, vmNumber::emptyNumber, i + 1 );
					if ( !pricePerDay.isNull () )
						addCalendarExchangeRule ( ce_list, CEAO_ADD_PRICE_DATE1, date, pricePerDay );
				}
			} //if ( job->wasModified ( FLD_JOB_REPORT ) )
			else
			{
				if ( job->wasModified ( FLD_JOB_PRICE ) )
				{
					for ( int i ( 0 ); i < n_days ; ++i )
					{
						dayRecord = const_cast<stringRecord*>(&jobReport.readRecord ( i ));
						date.fromTrustedStrDate ( dayRecord->fieldValue ( Job::JRF_DATE ), vmNumber::VDF_DB_DATE );
						addCalendarExchangeRule ( ce_list, CEAO_EDIT_PRICE_DATE1, date, pricePerDay );
						ce_list[ce_list.currentIndex()]->price2 = oldPricePerDay;
					}
				}
			}
		}
		break;
	}
	updateCalendarDB ( job, ce_list );
}

void dbCalendar::updateCalendarWithPayInfo ( const Payment* const pay )
{	
	switch ( pay->action () )
	{
		case ACTION_ADD: // An adding pay is an empty pay. 
		case ACTION_REVERT:
		break;
		default:
		{
			vmNumber date, price;
			const stringTable payInfo ( recStrValue ( pay, FLD_PAY_INFO ) );
			const uint n_pays ( payInfo.countRecords () );
			stringRecord* payRecord ( nullptr );
			PointersList<CALENDAR_EXCHANGE*> ce_list ( 5 );
			
			switch ( pay->action () )
			{
				case ACTION_READ: // see updateCalendarWithJobInfo
					for ( uint i ( 0 ); i < n_pays ; ++i )
					{
						payRecord = const_cast<stringRecord* >( &payInfo.readRecord ( i ) );
						price.fromTrustedStrPrice ( payRecord->fieldValue ( PHR_VALUE ), 1 );
						date.fromTrustedStrDate ( payRecord->fieldValue ( PHR_DATE ), vmNumber::VDF_DB_DATE );
						addCalendarExchangeRule ( ce_list, CEAO_ADD_DATE1, date, vmNumber::emptyNumber, i + 1 );
						addCalendarExchangeRule ( ce_list, CEAO_ADD_PRICE_DATE1, date, price );
						if ( payRecord->fieldValue ( PHR_PAID ) == CHR_ONE )
						{
							date.fromTrustedStrDate ( payRecord->fieldValue ( PHR_USE_DATE ), vmNumber::VDF_DB_DATE );
							addCalendarExchangeRule ( ce_list, CEAO_ADD_DATE2, date, vmNumber::emptyNumber, i + 1 );
							addCalendarExchangeRule ( ce_list, CEAO_ADD_PRICE_DATE2, date, price );
						}
					}
				break;
				case ACTION_DEL:
					for ( uint i ( 0 ); i < n_pays ; ++i )
					{
						payRecord = const_cast<stringRecord* >( &payInfo.readRecord ( i ) );
						price.fromTrustedStrPrice ( payRecord->fieldValue ( PHR_VALUE ), 1 );
						date.fromTrustedStrDate ( payRecord->fieldValue ( PHR_DATE ), vmNumber::VDF_DB_DATE );
						addCalendarExchangeRule ( ce_list, CEAO_DEL_DATE1, date, vmNumber::emptyNumber );
						addCalendarExchangeRule ( ce_list, CEAO_DEL_PRICE_DATE1, date, price );
						if ( payRecord->fieldValue ( PHR_PAID ) == CHR_ONE )
						{
							date.fromTrustedStrDate ( payRecord->fieldValue ( PHR_USE_DATE ), vmNumber::VDF_DB_DATE );
							addCalendarExchangeRule ( ce_list, CEAO_DEL_DATE2, date, vmNumber::emptyNumber );
							addCalendarExchangeRule ( ce_list, CEAO_DEL_PRICE_DATE2, date, price );
						}
					}
				break;
				case ACTION_EDIT:
				{
					if ( pay->isModified ( FLD_PAY_INFO ) )
					{
						stringTable oldPayInfo ( recStrValueAlternate ( pay, FLD_PAY_INFO ) );
						const uint old_n_pays ( oldPayInfo.countRecords () );
						for ( uint i ( 0 ); i < old_n_pays ; ++i )
						{
							payRecord = const_cast<stringRecord* >( &oldPayInfo.readRecord ( i ) );
							price.fromTrustedStrPrice ( payRecord->fieldValue ( PHR_VALUE ), 1 );
							date.fromTrustedStrDate ( payRecord->fieldValue ( PHR_DATE ), vmNumber::VDF_DB_DATE );
							addCalendarExchangeRule ( ce_list, CEAO_DEL_DATE1, date, vmNumber::emptyNumber );
							addCalendarExchangeRule ( ce_list, CEAO_DEL_PRICE_DATE1, date, price );
							if ( payRecord->fieldValue ( PHR_PAID ) == CHR_ONE )
							{
								date.fromTrustedStrDate ( payRecord->fieldValue ( PHR_USE_DATE ), vmNumber::VDF_DB_DATE );
								addCalendarExchangeRule ( ce_list, CEAO_DEL_DATE2, date, vmNumber::emptyNumber );
								addCalendarExchangeRule ( ce_list, CEAO_DEL_PRICE_DATE2, date, price );
							}
						}
						
						for ( uint i ( 0 ); i < n_pays ; ++i )
						{
							payRecord = const_cast<stringRecord* >( &payInfo.readRecord ( i ) );
							price.fromTrustedStrPrice ( payRecord->fieldValue ( PHR_VALUE ), 1 );
							date.fromTrustedStrDate ( payRecord->fieldValue ( PHR_DATE ), vmNumber::VDF_DB_DATE );
							addCalendarExchangeRule ( ce_list, CEAO_ADD_DATE1, date, vmNumber::emptyNumber, i + 1 );
							addCalendarExchangeRule ( ce_list, CEAO_ADD_PRICE_DATE1, date, price );
							if ( payRecord->fieldValue ( PHR_PAID ) == CHR_ONE )
							{
								date.fromTrustedStrDate ( payRecord->fieldValue ( PHR_USE_DATE ), vmNumber::VDF_DB_DATE );
								addCalendarExchangeRule ( ce_list, CEAO_ADD_DATE2, date, vmNumber::emptyNumber, i + 1 );
								addCalendarExchangeRule ( ce_list, CEAO_ADD_PRICE_DATE2, date, price );
							}
						}
					}
				}
				break; // action == ACTION_EDIT
				default: break;
			}
			updateCalendarDB ( pay, ce_list );
		}
		break;  // default action
	}
}

void dbCalendar::updateCalendarWithBuyInfo ( const Buy* const buy )
{
	const vmNumber date ( buy->date ( FLD_BUY_DATE ) );
	const vmNumber price ( buy->price ( FLD_BUY_PRICE ) );
	PointersList<CALENDAR_EXCHANGE*> ce_list ( 5 );

	switch ( buy->action () )
	{
		case ACTION_ADD:
		case ACTION_READ: // see updateCalendarWithJobInfo
		{
			if ( !price.isNull () )
				addCalendarExchangeRule ( ce_list, CEAO_ADD_PRICE_DATE1, date, price );
			addCalendarExchangeRule ( ce_list, CEAO_ADD_DATE1, date );
		}
		break;
		case ACTION_DEL:
		{
			addCalendarExchangeRule ( ce_list, CEAO_DEL_DATE1, date );
			addCalendarExchangeRule ( ce_list, CEAO_DEL_PRICE_DATE1, date, price );
		}
		break;
		case ACTION_EDIT:
		{
			if ( buy->isModified ( FLD_BUY_PRICE ) || buy->isModified ( FLD_BUY_DATE ) )
			{
				const vmNumber origDate ( recStrValueAlternate ( buy, FLD_BUY_DATE ), VMNT_DATE, vmNumber::VDF_DB_DATE );

				/* If either date or price were changed, we must exclude the
				 * the original price (if) stored in calendar, and (possibly) add the new price
				 */
				const vmNumber origPrice ( recStrValueAlternate ( buy, FLD_BUY_PRICE ), VMNT_PRICE, 1 );
				if ( !origPrice.isNull () )
				{
					if ( origPrice != price && !price.isNull () )
					{
						addCalendarExchangeRule ( ce_list, CEAO_DEL_PRICE_DATE1, origDate, origPrice );
						addCalendarExchangeRule ( ce_list, CEAO_ADD_PRICE_DATE1, date, price );
					}
				}
				// Now, we alter the date in calendar only if date was changed
				if ( origDate != date )
				{
					addCalendarExchangeRule ( ce_list, CEAO_DEL_DATE1, origDate );
					addCalendarExchangeRule ( ce_list, CEAO_ADD_DATE1, date );
				}
			}
		}
		break;
		default:
			return;
	}
	updateCalendarWithBuyPayInfo ( buy, ce_list );
	updateCalendarDB ( buy, ce_list );
}

void dbCalendar::updateCalendarWithBuyPayInfo ( const Buy* const buy, PointersList<CALENDAR_EXCHANGE*>& ce_list )
{
	if ( buy->action () >= ACTION_ADD )
	{
		if ( !buy->isModified ( FLD_BUY_PAYINFO ) )
			return;
	}

	stringRecord* rec ( nullptr );
	vmNumber date, price;
	const stringTable newTable ( recStrValue ( buy, FLD_BUY_PAYINFO ) );

	switch ( buy->action () )
	{
		case ACTION_ADD:
		case ACTION_READ:
		{
			rec = const_cast<stringRecord*>(&newTable.first ());
			while ( rec->isOK () )
			{
				date.fromTrustedStrDate ( rec->fieldValue ( PHR_DATE ), vmNumber::VDF_DB_DATE );
				addCalendarExchangeRule ( ce_list, CEAO_ADD_DATE2, date, vmNumber::emptyNumber, newTable.currentIndex () + 1 );
				if ( rec->fieldValue ( PHR_PAID ) == CHR_ONE )
				{
					price.fromTrustedStrPrice ( rec->fieldValue ( PHR_VALUE ) );
					addCalendarExchangeRule ( ce_list, CEAO_ADD_PRICE_DATE2, date, price );
				}
				rec = const_cast<stringRecord*>(&newTable.next ());
			}
		}
		break;
		
		case ACTION_DEL:
		{
			rec = const_cast<stringRecord*>(&newTable.first ());
			while ( rec->isOK () )
			{
				date.fromTrustedStrDate ( rec->fieldValue ( PHR_DATE ), vmNumber::VDF_DB_DATE );
				addCalendarExchangeRule ( ce_list, CEAO_DEL_DATE2, date, vmNumber::emptyNumber, newTable.currentIndex () + 1 );
				if ( rec->fieldValue ( PHR_PAID ) == CHR_ONE )
				{
					price.fromTrustedStrPrice ( rec->fieldValue ( PHR_VALUE ) );
					addCalendarExchangeRule ( ce_list, CEAO_DEL_PRICE_DATE2, date, price );
				}
				rec = const_cast<stringRecord*>(&newTable.next ());
			}
		}
		break;
		
		case ACTION_EDIT:
		{
			const stringTable oldTable ( recStrValueAlternate ( buy, FLD_BUY_PAYINFO ) );
			rec = const_cast<stringRecord*>(&oldTable.first ());
			while ( rec->isOK () )
			{
				date.fromTrustedStrDate ( rec->fieldValue ( PHR_DATE ), vmNumber::VDF_DB_DATE );
				addCalendarExchangeRule ( ce_list, CEAO_DEL_DATE2, date, vmNumber::emptyNumber, newTable.currentIndex () + 1 );
				if ( rec->fieldValue ( PHR_PAID ) == CHR_ONE )
				{
					price.fromTrustedStrPrice ( rec->fieldValue ( PHR_VALUE ) );
					addCalendarExchangeRule ( ce_list, CEAO_DEL_PRICE_DATE2, date, price );
				}
				rec = const_cast<stringRecord*>(&oldTable.next ());
			}
			
			rec = const_cast<stringRecord*>(&newTable.first ());
			while ( rec->isOK () )
			{
				date.fromTrustedStrDate ( rec->fieldValue ( PHR_DATE ), vmNumber::VDF_DB_DATE );
				addCalendarExchangeRule ( ce_list, CEAO_ADD_DATE2, date, vmNumber::emptyNumber, newTable.currentIndex () + 1 );
				if ( rec->fieldValue ( PHR_PAID ) == CHR_ONE )
				{
					price.fromTrustedStrPrice ( rec->fieldValue ( PHR_VALUE ) );
					addCalendarExchangeRule ( ce_list, CEAO_ADD_PRICE_DATE2, date, price );
				}
				rec = const_cast<stringRecord*>(&newTable.next ());
			}
		} //case ACTION_EDIT
		break;
		default:
		break;
	}
}

void dbCalendar::updateCalendarDB ( const DBRecord* dbrec, PointersList<CALENDAR_EXCHANGE*>& ce_list )
{
	if ( ce_list.isEmpty () )
		return;

	stringRecord calendarIdTrio;
	calendarIdTrio.fastAppendValue ( dbrec->actualRecordStr ( 0 ) );
	calendarIdTrio.fastAppendValue ( dbrec->actualRecordStr ( 1 ) );

	CALENDAR_EXCHANGE* ce ( nullptr );
	
	uint calendarField[4];
	switch ( dbrec->typeID () )
	{
		case JOB_TABLE:
			calendarField[0] = FLD_CALENDAR_JOBS;
			calendarField[2] = FLD_CALENDAR_TOTAL_JOBPRICE_SCHEDULED;
		break;
		case PAYMENT_TABLE:
			calendarField[0] = FLD_CALENDAR_PAYS;
			calendarField[1] = FLD_CALENDAR_PAYS_USE;
			calendarField[2] = FLD_CALENDAR_TOTAL_PAY_RECEIVED;
			calendarField[3] = FLD_CALENDAR_TOTAL_PAY_RECEIVED;
		break;
		case PURCHASE_TABLE:
			calendarField[0] = FLD_CALENDAR_BUYS;
			calendarField[1] = FLD_CALENDAR_BUYS_PAY;
			calendarField[2] = FLD_CALENDAR_TOTAL_BUY_BOUGHT;
			calendarField[3] = FLD_CALENDAR_TOTAL_BUY_PAID;
		break;
	}

	for ( uint i ( 0 ); i < ce_list.count (); ++i )
	{
		ce = ce_list.at ( i );
		if ( ce->extra_info > 0 )
			calendarIdTrio.changeValue ( 2, QString::number ( ce->extra_info ) );
		switch ( ce->action )
		{
			case CEAO_NOTHING:
				continue;
			case CEAO_ADD_DATE1:
				addDate ( ce->date, calendarField[0], calendarIdTrio );
			break;
			case CEAO_DEL_DATE1:
				delDate ( ce->date, calendarField[0], calendarIdTrio );
			break;
			case CEAO_ADD_DATE2:
				addDate ( ce->date, calendarField[1], calendarIdTrio );
			break;
			case CEAO_DEL_DATE2:
				delDate ( ce->date, calendarField[1], calendarIdTrio );
			break;
			case CEAO_ADD_PRICE_DATE1:
				addPrice ( ce->date, ce->price, calendarField[2] );
			break;
			case CEAO_DEL_PRICE_DATE1:
				delPrice ( ce->date, ce->price, calendarField[2] );
			break;
			case CEAO_EDIT_PRICE_DATE1:
				editPrice ( ce->date, ce->price, ce->price2, calendarField[2] );
			break;
			case CEAO_ADD_PRICE_DATE2:
				addPrice ( ce->date, ce->price, calendarField[3] );
			break;
			case CEAO_DEL_PRICE_DATE2:
				delPrice ( ce->date, ce->price, calendarField[3] );
			break;
			case CEAO_EDIT_PRICE_DATE2:
				editPrice ( ce->date, ce->price, ce->price2, calendarField[3] );
			break;
		}
	}
	ce_list.clear ( true );
}

const stringTable& dbCalendar::dateLog ( const vmNumber& date, const uint search_field, const uint requested_field,
        QString& price, const uint requested_field_price, const bool bIncludeDates )
{
	mStrTable.clear ();
	if ( requested_field < FLD_CALENDAR_JOBS || requested_field > FLD_CALENDAR_BUYS_PAY )
		return mStrTable;

	vmNumber total_price;

	switch ( search_field )
	{
		case FLD_CALENDAR_DAY_DATE:
			if ( readRecord ( FLD_CALENDAR_DAY_DATE, date.toDate ( vmNumber::VDF_DB_DATE ) ) )
			{
				price = recStrValue ( this, requested_field_price );
				mStrTable.fromString ( recStrValue ( this, requested_field ) );
			}
		break;
		case FLD_CALENDAR_WEEK_NUMBER:
		case FLD_CALENDAR_MONTH:
			if ( readFirstRecord ( search_field, QString::number ( search_field == FLD_CALENDAR_MONTH ?
                                    monthToDBMonth ( date ) : weekNumberToDBWeekNumber ( date ) ) ) )
			{
                uint i_row ( 0 );
				do
				{
					mStrTable.appendTable ( recStrValue ( this, requested_field ) );
                    if ( bIncludeDates )
					{
                        for ( ; i_row < mStrTable.countRecords (); ++i_row )
                            mStrTable.changeRecord ( i_row, 3, recStrValue ( this, FLD_CALENDAR_DAY_DATE ) );
                    }
					total_price += vmNumber ( recStrValue ( this, requested_field_price ), VMNT_PRICE, 1 );
				} while ( readNextRecord ( true ) );
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
	stringTable ids;
	if ( readRecord ( FLD_CALENDAR_DAY_DATE, date.toDate ( vmNumber::VDF_DB_DATE ) ) )
	{
		ids.fromString ( recStrValue ( this, field ) );
        if ( ids.toString () == id_trio.toString () )
            return;
        setAction ( ACTION_EDIT );
	}
	else
	{
		setAction ( ACTION_ADD );
		setRecValue ( this, FLD_CALENDAR_DAY_DATE, date.toDate ( vmNumber::VDF_DB_DATE ) );
		setRecValue ( this, FLD_CALENDAR_WEEK_NUMBER, QString::number ( weekNumberToDBWeekNumber ( date ) ) );
		setRecValue ( this, FLD_CALENDAR_MONTH, QString::number ( monthToDBMonth ( date ) ) );
		setRecValue ( this, FLD_CALENDAR_TOTAL_JOBPRICE_SCHEDULED, vmNumber::zeroedPrice.toPrice () );
		setRecValue ( this, FLD_CALENDAR_TOTAL_PAY_RECEIVED, vmNumber::zeroedPrice.toPrice () );
		setRecValue ( this, FLD_CALENDAR_TOTAL_BUY_PAID, vmNumber::zeroedPrice.toPrice () );
		setRecValue ( this, FLD_CALENDAR_TOTAL_BUY_BOUGHT, vmNumber::zeroedPrice.toPrice () );
	}
	ids.fastAppendRecord ( id_trio );
	setRecValue ( this, field, ids.toString () );
	saveRecord ();
}

void dbCalendar::delDate ( const vmNumber& date, const uint field, const stringRecord& id_trio )
{
	if ( readRecord ( FLD_CALENDAR_DAY_DATE, date.toDate ( vmNumber::VDF_DB_DATE ) ) )
	{		
		setAction ( ACTION_EDIT );
		stringTable ids;
		ids.fromString ( recStrValue ( this, field ) );
		if ( ids.removeRecordByValue ( id_trio ) )
		{
			setRecValue ( this, field, ids.toString () );
			saveRecord ();
		}
	}
}

void dbCalendar::addPrice ( const vmNumber& date, const vmNumber& price, const uint field )
{
	vmNumber new_price ( price );
	if ( readRecord ( FLD_CALENDAR_DAY_DATE, date.toDate ( vmNumber::VDF_DB_DATE ) ) )
	{
		const vmNumber old_price ( recStrValue ( this, field ), VMNT_PRICE, 1 );
		new_price += old_price;
        setAction ( ACTION_EDIT );
	}
	else
	{
		setAction ( ACTION_ADD );
		setRecValue ( this, FLD_CALENDAR_DAY_DATE, date.toDate ( vmNumber::VDF_DB_DATE ) );
		setRecValue ( this, FLD_CALENDAR_WEEK_NUMBER, QString::number ( weekNumberToDBWeekNumber ( date ) ) );
		setRecValue ( this, FLD_CALENDAR_MONTH, QString::number ( monthToDBMonth ( date ) ) );
	}
	setRecValue ( this, field, new_price.toPrice () );
	saveRecord ();
}

void dbCalendar::editPrice ( const vmNumber& date, const vmNumber& new_price, const vmNumber& old_price, const uint field )
{
	vmNumber price;
	if ( readRecord ( FLD_CALENDAR_DAY_DATE, date.toDate ( vmNumber::VDF_DB_DATE ) ) )
	{
		setAction ( ACTION_EDIT );
		price.fromTrustedStrPrice ( recStrValue ( this, field ) );
		price -= old_price;
		price += new_price;
	}
	else
	{
		setAction ( ACTION_ADD );
		setRecValue ( this, FLD_CALENDAR_DAY_DATE, date.toDate ( vmNumber::VDF_DB_DATE ) );
		setRecValue ( this, FLD_CALENDAR_WEEK_NUMBER, QString::number ( weekNumberToDBWeekNumber ( date ) ) );
		setRecValue ( this, FLD_CALENDAR_MONTH, QString::number ( monthToDBMonth ( date ) ) );
		price = new_price;
	}
	setRecValue ( this, field, price.toPrice () );
	saveRecord ();
}

void dbCalendar::delPrice ( const vmNumber& date, const vmNumber& price, const uint field )
{
	if ( !price.isNull () )
	{
		if ( readRecord ( FLD_CALENDAR_DAY_DATE, date.toDate ( vmNumber::VDF_DB_DATE ) ) )
		{
            setAction ( ACTION_EDIT );
			const vmNumber old_price ( recStrValue ( this, field ), VMNT_PRICE, 1 );
            const vmNumber new_price ( old_price - price );
			setRecValue ( this, field, new_price.toPrice () );
			saveRecord ();
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
