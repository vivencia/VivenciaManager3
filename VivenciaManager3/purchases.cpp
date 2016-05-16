#include "purchases.h"
#include "vmlistitem.h"
#include "global.h"
#include "stringrecord.h"
#include "completers.h"
#include "dbcalendar.h"
#include "supplierrecord.h"

const double TABLE_VERSION ( 2.0 );

#ifdef TRANSITION_PERIOD
#include "global.h"
#include "vivenciadb.h"
#include "job.h"
#include "client.h"

#include <QtSql/QSqlQuery>

enum OLD_BUY_TABLE {
	FLD_OLDBUY_ID = 0, FLD_OLDBUY_CLIENTID = 1, FLD_OLDBUY_JOBID = 2, FLD_OLDBUY_PAID = 3, FLD_OLDBUY_DATE = 4, FLD_OLDBUY_PAYDATE = 5,
	FLD_OLDBUY_DELIVERDATE = 6, FLD_OLDBUY_PRICE = 7, FLD_OLDBUY_TOTALPAYS = 8, FLD_OLDBUY_PAYNUMBER = 9, FLD_OLDBUY_PAYVALUE = 10,
	FLD_OLDBUY_PAYMETHOD = 11, FLD_OLDBUY_DELIVERMETHOD = 12, FLD_OLDBUY_SUPPLIER = 13, FLD_OLDBUY_REPORT = 14, FLD_OLDBUY_NOTES = 15
};

QString findUnity ( const QString& str )
{
	QString ret ( QStringLiteral ( "un" ) );
	if ( !str.isEmpty () ) {
		const QChar firstChar ( str.at ( 0 ) );
		int idx ( -1 );
		if ( ( firstChar >=  CHR_ONE && firstChar <= QLatin1Char ( '9' ) ) ||
				( firstChar >= QLatin1Char ( 'a' ) && firstChar <= QLatin1Char ( 'z' ) ) ) {
			idx = str.indexOf ( CHR_SPACE );
			if ( idx != -1 )
				ret = str.left ( idx );
		}
		if ( idx == -1 ) {
			idx = str.indexOf ( CHR_L_PARENTHESIS );
			if ( idx != -1 )
				ret = str.mid ( idx + 1, str.indexOf ( CHR_R_PARENTHESIS, idx + 1 ) - idx - 1 );
		}
	}
	return ret;
}

QString findMaker ( const QString& str )
{
	QString ret;
	const int length ( str.length () );
	int i = 0;
	int idx ( -1 );
	do {
		if ( str.at ( i ) >= QLatin1Char ( 'A' ) &&
				str.at ( i ) <= QLatin1Char ( 'Z' ) ) {
			if ( i == ( length - 1 ) )
				break; // the captioned letter is the last in the string. Nothing else to be done
			if ( str.at ( i + 1 ) >= QLatin1Char ( 'A' ) &&
					str.at ( i + 1 ) <= QLatin1Char ( 'Z' ) ) {
				idx = str.indexOf ( CHR_SPACE, i + 1 );
				if ( idx == -1 )
					idx = length;
				ret = str.mid ( i, idx - i );
				if ( idx == length )
					break;
				i = idx + 1; // found a space, maybe vendor has more than one name, so continue
			}
		}
		++i;
	} while ( i < length );
	return ret;
}

bool updateStrReport ( const QString& str_report, QString& new_report )
{
	QString line;
	uint row ( 0 );
	stringRecord record;
	stringTable table;
	QString item, quantity, price, total_price, unit, maker;
	do {
		line = str_report.section ( CHR_NEWLINE, row, row );
		if ( !line.isEmpty () ) {
			item = line.section ( record_sep, 0, 0 );
			if ( !item.isEmpty () ) {
				quantity = line.section ( record_sep, 1, 1 );
				price = line.section ( record_sep, 2, 2 );
				total_price = line.section ( record_sep, 3, 3 );
				unit = findUnity ( item );
				if ( !unit.isEmpty () ) {
					if ( item.contains ( CHR_L_PARENTHESIS + unit ) ) {
						item.remove ( unit );
						item.remove ( CHR_L_PARENTHESIS );
						item.remove ( CHR_R_PARENTHESIS );
					}
				}
				maker = findMaker ( item );
				if ( !maker.isEmpty () )
					item.remove ( maker );
				item = item.simplified ();
				record.fastAppendValue ( item );
				record.fastAppendValue ( unit );
				record.fastAppendValue ( maker );
				record.fastAppendValue ( quantity );
				record.fastAppendValue ( price );
				record.fastAppendValue ( total_price );
				table.fastAppendRecord ( record );
				record.clear ();
			}
			++row;
		}
	} while ( !line.isEmpty () );
	if ( table.isOK () ) {
		new_report = table.toString ();
		return true;
	}
	return false;
}

#endif //TRANSITION_PERIOD

bool updatePurchaseTable ()
{
    /*const int newCId[101] = {
        1,
        2, 3, 4, 5, 6, 7, 8, 0, 0, 9,
        0, 10, 0, 0, 11, 12, 13, 14, 0, 15,
        16, 17, 18, 19, 20, 21, 22, 23, 24, 0,
        25, 26, 27, 28, 29, 0, 0, 30, 31, 0,
        32, 33, 34, 0, 35, 36, 37, 38, 39, 0,
        40, 41, 0, 0, 42, 0, 43, 44, 0,
        45, 0, 46, 47, 48, 49, 50, 51,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61,
        62, 63, 64, 65, 66, 67, 68, 69, 70, 71,
        72, 73, 74, 75, 76, 77, 78, 79, 80, 81,
        82, 83, 84
    };
    Buy buyy;
    if ( buyy.readFirstRecord () ) {
        int old_cid ( 0 );
        do {
            old_cid = recStrValue ( &buyy, FLD_BUY_CLIENTID ).toInt();
            buyy.setAction( ACTION_EDIT );
            setRecIntValue ( &buyy, FLD_BUY_CLIENTID, newCId[old_cid] );
            setRecValue ( &buyy, FLD_BUY_CLIENTID, QString::number( newCId[old_cid] ) );
            buyy.saveRecord ();
        } while ( buyy.readNextRecord() );
    }
    VDB ()->optimizeTable ( &Buy::t_info );
    return true;*/

#ifdef TRANSITION_PERIOD
	bool ok ( true );
	QSqlQuery query;

	if ( VDB ()->runQuery ( QStringLiteral ( "SELECT * FROM `PURCHASES`" ), query ) ) {
		QString str;
		do {
			if ( updateStrReport ( query.value ( FLD_OLDBUY_REPORT ).toString (), str ) ) {
				( void )VDB ()->database ()->exec ( QString ( QStringLiteral ( "UPDATE `PURCHASES` SET REPORT='%1' WHERE ID='%2'" ) ).
								   arg ( str, query.value ( FLD_OLDBUY_ID ).toString () ) );
			}

			str = query.value ( FLD_OLDBUY_DELIVERMETHOD ).toString ();
			ok = false;
			if ( !str.isEmpty () ) {
				if ( str.startsWith ( 'r' ) ) {
					str.replace ( 0, 1, QLatin1Char ( 'R' ) );
					ok = true;
				}
				else if ( str.contains ( QStringLiteral ( "Obra" ) ) ) {
					str.replace ( QStringLiteral ( "Obra" ) , QStringLiteral ( "obra" ) );
					ok = true;
				}
				if ( ok ) {
					( void )VDB ()->database ()->exec ( QString ( QStringLiteral ( "UPDATE PURCHASES SET DELIVERMETHOD='%1' WHERE ID='%2'" ) ).
									   arg ( str, query.value ( FLD_OLDBUY_ID ).toString () ) );
				}
			}

			str = query.value ( FLD_OLDBUY_PAYMETHOD ).toString ();
			ok = false;
			if ( !str.isEmpty () ) {
				if ( str.contains ( CHR_PLUS ) ) {
					ok = true;
					if ( query.value ( FLD_OLDBUY_PAYNUMBER ).toString () == CHR_ONE )
						str = QStringLiteral ( "Dinheiro" );
					else
						str = QString::fromUtf8 ( "Cheque pré-datado" );
				}
				else if ( str.contains ( QStringLiteral ( "vistab" ) ) ) {
					ok = true;
					str.truncate ( str.length () - 2 );
				}
				else if ( str.contains ( QStringLiteral ( "ercadoria" ) ) ) {
					ok = true;
					str = QStringLiteral ( "Mercadoria" );
				}
				if ( ok ) {
					( void )VDB ()->database ()->exec ( QString ( QStringLiteral ( "UPDATE PURCHASES SET PAYMETHOD='%1' WHERE ID='%2'" ) ).
									   arg ( str, query.value ( FLD_OLDBUY_ID ).toString () ) );
				}
			}
		} while ( query.next () );
	}

	QString jobid;
	Buy buy;
	stringTable pay_info;
	stringRecord rec;
	vmNumber price_paid;
	int n_days ( 0 );

	VDB ()->database ()->exec ( QStringLiteral ( "RENAME TABLE `PURCHASES` TO `OLD_PURCHASES`" ) );
	VDB ()->createTable ( &Buy::t_info );

	buy.setAction ( ACTION_ADD );
	query.clear ();
	if ( VDB ()->runQuery ( QStringLiteral ( "SELECT * FROM `OLD_PURCHASES` WHERE TOTALPAYS != '1' "), query ) ) {
		do {
			if ( query.value ( FLD_OLDBUY_JOBID ).toString () != jobid ) {
				if ( !jobid.isEmpty () ) {
					buy.setValue ( FLD_BUY_TOTALPAID, price_paid.toPrice () );
					buy.setValue ( FLD_BUY_TOTALPAYS, QString::number ( n_days ) );
					buy.setValue ( FLD_BUY_PAYINFO, pay_info.toString () );
					buy.saveRecord ();
					pay_info.clear ();
					n_days = 0;
					price_paid.clear ( false );
				}
			}

			if ( n_days == 0 ) {
				buy.setValue ( FLD_BUY_CLIENTID, QString::number ( getNewClientID ( query.value ( FLD_OLDBUY_CLIENTID ).toInt () ) ) );
				buy.setValue ( FLD_BUY_JOBID, query.value ( FLD_OLDBUY_JOBID ).toString () );
				buy.setValue ( FLD_BUY_DATE, query.value ( FLD_OLDBUY_DATE ).toString () );
				buy.setValue ( FLD_BUY_DELIVERDATE, query.value ( FLD_OLDBUY_DELIVERDATE ).toString () );
				buy.setValue ( FLD_BUY_NOTES, query.value ( FLD_OLDBUY_NOTES ).toString () );
				buy.setValue ( FLD_BUY_DELIVERMETHOD, query.value ( FLD_OLDBUY_DELIVERMETHOD ).toString () );
				buy.setValue ( FLD_BUY_SUPPLIER, query.value ( FLD_OLDBUY_SUPPLIER ).toString () );
				buy.setValue ( FLD_BUY_PRICE, query.value ( FLD_OLDBUY_PRICE ).toString () );
				buy.setValue ( FLD_BUY_REPORT, query.value ( FLD_OLDBUY_REPORT ).toString () );
			}
			++n_days;

			if ( query.value ( FLD_OLDBUY_PAID ).toString () == CHR_ONE )
				price_paid += vmNumber ( query.value ( FLD_OLDBUY_PAYVALUE ).toString (), VMNT_PRICE, 0 );
			rec.fastAppendValue ( query.value ( FLD_OLDBUY_PAYDATE ).toString () ); //PHR_DATE
			rec.fastAppendValue ( query.value ( FLD_OLDBUY_PAYVALUE ).toString () ); //PHR_VALUE
			rec.fastAppendValue ( query.value ( FLD_OLDBUY_PAID ).toString () ); //PHR_PAID
			rec.fastAppendValue ( query.value ( FLD_OLDBUY_PAYMETHOD ).toString () ); //PHR_METHOD
			pay_info.fastAppendRecord ( rec );
			rec.clear ();
			jobid = query.value ( FLD_OLDBUY_JOBID ).toString ();
		} while ( query.next () );
	}

	if ( !jobid.isEmpty () ) {
		buy.setValue ( FLD_BUY_TOTALPAID, price_paid.toPrice () );
		buy.setValue ( FLD_BUY_TOTALPAYS, QString::number ( n_days ) );
		buy.setValue ( FLD_BUY_PAYINFO, pay_info.toString () );
		buy.saveRecord ();
		buy.setAction ( ACTION_ADD );
	}

	QSqlQuery job_query;
	query.clear ();
    if ( VDB ()->runQuery ( QStringLiteral ( "SELECT * FROM `JOBS`" ), job_query ) ) {
		buy.setAction ( ACTION_ADD );
		do {
			jobid = job_query.value ( FLD_JOB_ID ).toString ();
			if ( VDB ()->runQuery ( QStringLiteral ( "SELECT * FROM `OLD_PURCHASES` WHERE JOBID=" ) + jobid, query ) ) {
				do {
					if ( query.value ( FLD_OLDBUY_TOTALPAYS ).toString () != CHR_ONE )
						continue; // These recored have already been processed

					// Calls to query must be placed here, before query is pointing to an invalid record because of query.next ()
                    buy.setValue ( FLD_BUY_CLIENTID, QString::number ( getNewClientID ( job_query.value ( FLD_JOB_CLIENTID ).toString ().toInt () ) ) );
					buy.setValue ( FLD_BUY_JOBID, jobid );
					buy.setValue ( FLD_BUY_DATE, query.value ( FLD_OLDBUY_DATE ).toString () );
					buy.setValue ( FLD_BUY_DELIVERDATE, query.value ( FLD_OLDBUY_DELIVERDATE ).toString () );
					buy.setValue ( FLD_BUY_NOTES, query.value ( FLD_OLDBUY_NOTES ).toString () );
					buy.setValue ( FLD_BUY_DELIVERMETHOD, query.value ( FLD_OLDBUY_DELIVERMETHOD ).toString () );
					buy.setValue ( FLD_BUY_SUPPLIER, query.value ( FLD_OLDBUY_SUPPLIER ).toString () );
					buy.setValue ( FLD_BUY_PRICE, query.value ( FLD_OLDBUY_PRICE ).toString () );
					buy.setValue ( FLD_BUY_REPORT, query.value ( FLD_OLDBUY_REPORT ).toString () );

					if ( query.value ( FLD_OLDBUY_PAID ).toString () == CHR_ONE )
						price_paid = vmNumber ( query.value ( FLD_OLDBUY_PAYVALUE ).toString (), VMNT_PRICE, 0 );
					rec.fastAppendValue ( query.value ( FLD_OLDBUY_PAYDATE ).toString () ); //PHR_DATE
					rec.fastAppendValue ( query.value ( FLD_OLDBUY_PAYVALUE ).toString () ); //PHR_VALUE
					rec.fastAppendValue ( query.value ( FLD_OLDBUY_PAID ).toString () ); //PHR_PAID
					rec.fastAppendValue ( query.value ( FLD_OLDBUY_PAYMETHOD ).toString () ); //PHR_METHOD
					pay_info.fastAppendRecord ( rec );

					buy.setValue ( FLD_BUY_TOTALPAID, price_paid.toPrice () );
					buy.setValue ( FLD_BUY_TOTALPAYS, query.value ( FLD_OLDBUY_TOTALPAYS ).toString () );
					buy.setValue ( FLD_BUY_PAYINFO, pay_info.toString () );
					buy.saveRecord ();
					buy.clearAll ();
					rec.clear ();
					pay_info.clear ();
					buy.setAction ( ACTION_ADD );
				} while ( query.next () );
			}
		} while ( job_query.next () );
	}
	( void )VDB ()->database ()->exec ( QStringLiteral ( "DROP TABLE `OLD_PURCHASES`" ) );
	VDB ()->optimizeTable ( &Buy::t_info );
	return true;
#endif
	return false;
}

const TABLE_INFO Buy::t_info = {
	PURCHASE_TABLE,
	QStringLiteral ( "PURCHASES" ),
	QStringLiteral ( " ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci" ),
	QStringLiteral ( " PRIMARY KEY ( `ID` ), UNIQUE KEY `id` ( `ID` ) " ),
	QStringLiteral ( "`ID`|`CLIENTID`|`JOBID`|`DATE`|`DELIVERDATE`|`PRICE`|`TOTAL_PAID`|`TOTALPAYS`|`DELIVERMETHOD`|`SUPPLIER`|`NOTES`|`REPORT`|`PAYINFO`|" ),
	QStringLiteral ( " int ( 9 ) NOT NULL, | int ( 9 ) NOT NULL, | int ( 9 ) DEFAULT NULL, | varchar ( 30 ) DEFAULT NULL, |"
	" varchar ( 30 ) DEFAULT NULL, | varchar ( 30 ) DEFAULT NULL, | varchar ( 30 ) DEFAULT NULL, | varchar ( 10 ) DEFAULT NULL, |"
	" varchar ( 40 ) DEFAULT NULL, | varchar ( 100 ) DEFAULT NULL, | varchar ( 200 ) DEFAULT NULL, |"
	" longtext COLLATE utf8_unicode_ci, | longtext COLLATE utf8_unicode_ci, |" ),
	QStringLiteral ( "ID|Client ID|Job ID|Purchase date|Delivery date|Price|Price paid|Number of payments|Delivery method|Supplier|Notes|Report|Pay info" ),
	BUYS_FIELDS_TYPE,
	TABLE_VERSION, BUY_FIELD_COUNT, TABLE_BUY_ORDER, &updatePurchaseTable
	#ifdef TRANSITION_PERIOD
	, true
	#endif
};

void updateSupplierTable_purchases ( const DBRecord* db_rec )
{
	supplierRecord::insertIfSupplierInexistent ( recStrValue ( db_rec, FLD_BUY_SUPPLIER ) );
}

void updateBuyDeliverCompleter ( const DBRecord* db_rec )
{
	APP_COMPLETERS ()->updateCompleter ( recStrValue ( db_rec, FLD_BUY_DELIVERMETHOD ), vmCompleters::DELIVERY_METHOD );
}

Buy::Buy ( const bool connect_helper_funcs )
	: DBRecord ( BUY_FIELD_COUNT ), ce_list ( 10 )
{
	::memset ( this->helperFunction, 0, sizeof ( this->helperFunction ) );
	DBRecord::t_info = & ( this->t_info );
	DBRecord::m_RECFIELDS = this->m_RECFIELDS;

	if ( connect_helper_funcs ) {
		DBRecord::helperFunction = this->helperFunction;
		setHelperFunction ( FLD_BUY_SUPPLIER, &updateSupplierTable_purchases );
		setHelperFunction ( FLD_BUY_DELIVERMETHOD, &updateBuyDeliverCompleter );
	}
}

Buy::~Buy ()
{
	ce_list.clear ( true );
}

uint Buy::isrRecordField ( const ITEMS_AND_SERVICE_RECORD isr_field ) const
{
	uint rec_field ( 0 );
	switch ( isr_field ) {
		case ISR_SUPPLIER:
			rec_field = FLD_BUY_SUPPLIER;
		break;
		case ISR_ID:
			rec_field = FLD_BUY_ID;
		break;
		case ISR_OWNER:
			rec_field = FLD_BUY_JOBID;
		break;
		case ISR_DATE:
			rec_field = FLD_BUY_DATE;
		break;
		default:
			rec_field = FLD_BUY_REPORT;
		break;
	}
	return rec_field;
}

QString Buy::isrValue ( const ITEMS_AND_SERVICE_RECORD isr_field, const int sub_record ) const
{
	const uint rec_field ( isrRecordField ( isr_field ) );
	if ( rec_field == FLD_BUY_REPORT ) {
		if ( sub_record >= 0 ) {
			const stringTable table ( recStrValue ( this, FLD_BUY_REPORT ) );
			const stringRecord rec ( table.readRecord ( sub_record ) );
			return rec.fieldValue ( isr_field );
		}
		return emptyString;
	}
	else
		return recStrValue ( this, rec_field );
}

int Buy::searchCategoryTranslate ( const SEARCH_CATEGORIES sc ) const
{
	switch ( sc ) {
		case SC_ID: return FLD_BUY_ID;
		case SC_REPORT_1: return FLD_BUY_REPORT;
		case SC_REPORT_2: return FLD_BUY_NOTES;
		case SC_ADDRESS_1: return FLD_BUY_SUPPLIER;
		case SC_PRICE_1: return FLD_BUY_PRICE;
		case SC_DATE_1: return FLD_BUY_DATE;
		case SC_DATE_3: return FLD_BUY_DELIVERDATE;
		case SC_EXTRA_1: return FLD_BUY_CLIENTID;
		case SC_EXTRA_2: return FLD_BUY_JOBID;
		default: return -1;
	}
}

void Buy::updateCalendarBuyInfo ()
{
	if ( ce_list.isEmpty () )
		return;

	dbCalendar* cal ( new dbCalendar );
	stringRecord calendarIdPair;
	calendarIdPair.fastAppendValue ( actualRecordStr ( FLD_BUY_ID ) );
	calendarIdPair.fastAppendValue ( actualRecordStr ( FLD_BUY_CLIENTID ) );

	CALENDAR_EXCHANGE* ce ( nullptr );
	for ( uint i ( 0 ); i < ce_list.count (); ++i ) {
		ce = ce_list.at ( i );
		switch ( ce->action ) {
			case CEAO_NOTHING:
				continue;
			case CEAO_ADD_DATE1:
				cal->addDate ( ce->date, FLD_CALENDAR_BUYS, calendarIdPair );
			break;
			case CEAO_DEL_DATE1:
				cal->delDate ( ce->date, FLD_CALENDAR_BUYS, calendarIdPair );
			break;
			case CEAO_ADD_DATE2:
				cal->addDate ( ce->date, FLD_CALENDAR_BUYS_PAY, calendarIdPair );
			break;
			case CEAO_DEL_DATE2:
				cal->delDate ( ce->date, FLD_CALENDAR_BUYS_PAY, calendarIdPair );
			break;
			case CEAO_ADD_PRICE_DATE1:
				cal->addPrice ( ce->date, ce->price, FLD_CALENDAR_TOTAL_BUY_BOUGHT );
			break;
			case CEAO_DEL_PRICE_DATE1:
				cal->delPrice ( ce->date, ce->price, FLD_CALENDAR_TOTAL_BUY_BOUGHT );
			break;
			case CEAO_ADD_PRICE_DATE2:
				cal->addPrice ( ce->date, ce->price, FLD_CALENDAR_TOTAL_BUY_PAID );
			break;
			case CEAO_DEL_PRICE_DATE2:
				cal->delPrice ( ce->date, ce->price, FLD_CALENDAR_TOTAL_BUY_PAID );
			break;
		}
	}
	ce_list.clearButKeepMemory ( true );
	delete cal;
}

void Buy::setListItem ( buyListItem* buy_item )
{
    DBRecord::mListItem = static_cast<vmListItem*>( buy_item );
}

buyListItem* Buy::buyItem () const
{
    return static_cast<buyListItem*>( DBRecord::mListItem );
}
