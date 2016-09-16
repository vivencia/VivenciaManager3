#include "payment.h"
#include "vmlistitem.h"
#include "global.h"
#include "vivenciadb.h"
#include "stringrecord.h"

#ifdef TRANSITION_PERIOD
#include "job.h"

#include <QtSql/QSqlQuery>

enum OLD_PAY_TABLE {
	FLD_OLDPAY_ID = 0, FLD_OLDPAY_CLIENTID = 1, FLD_OLDPAY_JOBID = 2, FLD_OLDPAY_PAID = 3, FLD_OLDPAY_USEDATE = 4, FLD_OLDPAY_DATE = 5,
	FLD_OLDPAY_PRICE = 6, FLD_OLDPAY_METHOD = 7, FLD_OLDPAY_VALUE = 8, FLD_OLDPAY_TOTALPAYS = 9, FLD_OLDPAY_PAYNUMBER = 10,
	FLD_OLDPAY_ACCOUNT = 11, FLD_OLDPAY_OBS = 12, FLD_OLDPAY_METHOD_EXTRA = 13
};

#endif

const double TABLE_VERSION ( 2.2 );

void updateOverdueInfo ( const DBRecord* db_rec );

bool updatePaymentTable ()
{
/*	if ( TABLE_VERSION == 2.2 ) {
		QSqlQuery query;
		if ( VDB ()->runQuery ( QStringLiteral ( "SELECT * FROM `PAYMENTS`" ), query ) ) {
			vmNumber pay_price, paid_price;
			Payment pay ( false );
			do {
				pay.readRecord ( query.value ( FLD_PAY_ID ).toInt () );
				pay.setAction ( ACTION_EDIT );
				pay_price.fromTrustedStrPrice ( query.value ( FLD_PAY_PRICE ).toString () );
				paid_price.fromTrustedStrPrice ( query.value ( FLD_PAY_TOTALPAID ).toString () );
				
				if ( query.value ( FLD_PAY_INFO ).toString ().isEmpty () && !pay_price.isNull () ) {
					if ( query.value ( FLD_PAY_ID ).toInt () <= 315 ) {
						setRecValue ( &pay, FLD_PAY_TOTALPAYS, CHR_ONE );
						setRecValue ( &pay, FLD_PAY_TOTALPAID, pay_price.toPrice () );
						paid_price = pay_price;
					}
				}
				if ( paid_price < pay_price ) {
					if ( query.value ( FLD_PAY_OVERDUE ).toString () != CHR_TWO ) {
						setRecValue ( &pay, FLD_PAY_OVERDUE, CHR_ONE );
						setRecValue ( &pay, FLD_PAY_OVERDUE_VALUE, ( pay_price - paid_price ).toPrice () );	
					}
				}
				else {
					setRecValue ( &pay, FLD_PAY_OVERDUE, CHR_ZERO );
					setRecValue ( &pay, FLD_PAY_OVERDUE_VALUE, vmNumber::zeroedPrice.toPrice () );
				}
				pay.saveRecord ();
			} while ( query.next () );
		}
	}
	VDB ()->optimizeTable ( &Payment::t_info );
	return true;
	*/
#ifdef TRANSITION_PERIOD

	QSqlQuery query;
	QString jobid;
	int n_pays ( 0 );
	bool ok ( true );
	Payment pay ( false );
	stringTable pay_info;
	stringRecord rec, old_pay_method;
	QString pay_obs, pay_obs_temp, str;
	vmNumber pay_paid, pay_price;
	bool b_paid ( false );

    /*Job job;
    if ( job.readFirstRecord () ) {
        do {
            jobid = recStrValue ( &job, FLD_JOB_ID );
            if ( !pay.readRecord ( FLD_PAY_JOBID, jobid, false ) ) {
                pay.clearAll ();
                pay.setAction ( ACTION_ADD );
                pay.setValue ( FLD_PAY_OVERDUE, CHR_ONE );
                pay.setValue ( FLD_PAY_TOTALPAYS, CHR_ZERO );
                pay.setValue ( FLD_PAY_CLIENTID, recStrValue ( &job, FLD_JOB_CLIENTID ) );
                pay.setValue ( FLD_PAY_JOBID, jobid );
                pay.setValue ( FLD_PAY_PRICE, recStrValue ( &job, FLD_JOB_PRICE ) );
                pay.saveRecord ();
            }
        } while ( job.readNextRecord () );
    }
    VDB ()->optimizeTable ( &Payment::t_info );
    return true;*/

	if ( VDB ()->runQuery ( QStringLiteral ( "SELECT * FROM `PAYMENTS`" ), query ) ) {
		const QChar CHR_SEP ( char ( 30 ) );
		do {
			str = query.value ( FLD_OLDPAY_METHOD_EXTRA ).toString ();
			if ( !str.isEmpty () ) {
				str.remove ( CHR_SEP );
				str.replace ( CHR_SEMICOLON, CHR_SEP );
				( void )VDB ()->database ()->exec ( QString ( QStringLiteral ( "UPDATE PAYMENTS SET FLD_PAY_METHOD_EXTRA='%1' WHERE ID='%2'" ) ).
								   arg ( str, query.value ( FLD_OLDPAY_ID ).toString () ) );
			}

			str = query.value ( FLD_OLDPAY_ACCOUNT ).toString ();
			ok = false;
			if ( !str.isEmpty () ) {
				if ( str.contains ( QStringLiteral ( "BB" ), Qt::CaseInsensitive ) ) {
					if ( str.contains ( QStringLiteral ( "CC" ), Qt::CaseInsensitive ) )
						str = QStringLiteral ( "BB - CC" );
					else
						str = QStringLiteral ( "BB - Poupança" );
					ok = true;
				}
				else if ( str.contains ( QStringLiteral ( "arteira" ), Qt::CaseInsensitive ) ) {
					ok = true;
					str = QStringLiteral ( "Carteira" );
				}
				if ( ok ) {
					( void )VDB ()->database ()->exec ( QString ( QStringLiteral ( "UPDATE PAYMENTS SET ACCOUNT='%1' WHERE ID='%2'" ) ).
									   arg ( str, query.value ( FLD_OLDPAY_ID ).toString () ) );
				}
			}

			str = query.value ( FLD_OLDPAY_METHOD ).toString ();
			ok = false;
			if ( !str.isEmpty () ) {
				if ( str.contains ( CHR_PLUS ) ) {
					ok = true;
					if ( query.value ( FLD_OLDPAY_PAYNUMBER ).toString () == CHR_ONE )
						str = QStringLiteral ( "Dinheiro" );
					else
						str = QStringLiteral ( "Cheque pré-datado" );
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
					( void )VDB ()->database ()->exec ( QString ( QStringLiteral ( "UPDATE PAYMENTS SET METHOD='%1' WHERE ID='%2'" ) ).
									   arg ( str, query.value ( FLD_OLDPAY_ID ).toString () ) );
				}
			}
		} while ( query.next () );
	}

	( void )VDB ()->database ()->exec ( QStringLiteral ( "RENAME TABLE `PAYMENTS` TO `OLD_PAYMENTS`" ) );
	( void )VDB ()->createTable ( &Payment::t_info );

	QSqlQuery job_query;
	query.clear ();
	if ( VDB ()->runQuery ( QStringLiteral ( "SELECT * FROM `JOBS`" ), job_query ) ) {
		pay.setAction ( ACTION_ADD );
		do {
			jobid = job_query.value ( FLD_OLDJOB_ID ).toString ();
			if ( VDB ()->runQuery ( QStringLiteral ( "SELECT * FROM OLD_PAYMENTS WHERE JOBID=" ) + jobid, query ) ) {
				n_pays = 0;
				b_paid = query.value ( FLD_OLDPAY_PAID ).toString () == CHR_ONE;

				do {
					rec.clear ();
					n_pays++;
					pay_obs_temp = query.value ( FLD_OLDPAY_OBS ).toString ();
					if ( !pay_obs_temp.isEmpty () ) {
						if ( pay_obs_temp != QStringLiteral ( "N/A") )
							pay_obs += pay_obs_temp + CHR_DOT + CHR_SPACE;
					}
					str = query.value ( FLD_OLDPAY_METHOD_EXTRA ).toString ();
					if ( str.isEmpty () ) {
						rec.insertField ( PHR_VALUE, query.value ( FLD_OLDPAY_VALUE ).toString () );
						rec.insertField ( PHR_METHOD, query.value ( FLD_OLDPAY_METHOD ).toString () );
						rec.insertField ( PHR_ACCOUNT, query.value ( FLD_OLDPAY_ACCOUNT ).toString () );
						rec.insertField ( PHR_DATE, query.value ( FLD_OLDPAY_DATE ).toString () );
						rec.insertField ( PHR_PAID, query.value ( FLD_OLDPAY_PAID ).toString () );
						rec.insertField ( PHR_USE_DATE, query.value ( FLD_OLDPAY_USEDATE ).toString () );
						pay_info.fastAppendRecord ( rec );
						if ( b_paid )
							pay_paid += vmNumber ( query.value ( FLD_OLDPAY_VALUE ).toString (), VMNT_PRICE );
					}
					else {
						old_pay_method.fromString ( str );
						if ( old_pay_method.first () ) {
							rec.insertField ( PHR_METHOD, old_pay_method.curValue () ); // retrieve method
							do {
								if ( old_pay_method.next () ) { // retrieve value
									if ( b_paid )
										pay_paid += vmNumber ( old_pay_method.curValue (), VMNT_PRICE );
									rec.insertField ( PHR_VALUE, old_pay_method.curValue () );
								}
								if ( old_pay_method.next () )
									rec.insertField ( PHR_ACCOUNT, old_pay_method.curValue () ); // retrieve account
								rec.insertField ( PHR_DATE, query.value ( FLD_OLDPAY_DATE ).toString () );
								rec.insertField ( PHR_PAID, query.value ( FLD_OLDPAY_PAID ).toString () );
								rec.insertField ( PHR_USE_DATE, query.value ( FLD_OLDPAY_USEDATE ).toString () );

								pay_info.fastAppendRecord ( rec );
								if ( old_pay_method.next () ) { // retrieve method again, if it exists
									rec.clear ();
									rec.insertField ( PHR_METHOD, str );
									n_pays++;
								}
								else
									break;
							} while ( true );
						}
					}
				} while ( query.next () );
				pay_price.fromTrustedStrPrice ( job_query.value ( FLD_OLDJOB_PRICE ).toString () );
				if ( pay_paid < pay_price ) {
					pay.setValue ( FLD_PAY_OVERDUE, CHR_ONE );
					pay.setValue ( FLD_PAY_OVERDUE_VALUE, vmNumber ( pay_price - pay_paid ).toPrice () );
				}
				else
					pay.setValue ( FLD_PAY_OVERDUE, CHR_ZERO );

				pay.setValue ( FLD_PAY_OBS, pay_obs );
				pay.setValue ( FLD_PAY_INFO, pay_info.toString () );
				pay.setValue ( FLD_PAY_TOTALPAID, pay_paid.toPrice () );
				pay.setValue ( FLD_PAY_TOTALPAYS, QString::number ( n_pays ) );
				pay.setValue ( FLD_PAY_CLIENTID, QString::number ( getNewClientID ( job_query.value ( FLD_OLDJOB_CLIENTID ).toInt () ) ) );
				pay.setValue ( FLD_PAY_JOBID, jobid );
				pay.setValue ( FLD_PAY_PRICE, pay_price.toPrice () );
				updateOverdueInfo ( static_cast<DBRecord*>( &pay ) );
                pay_obs.clear ();
                pay_paid.clear ( false );
                pay_info.clear ();
			}
            else { // job does not have a payment record. Create an empty one
                pay.setValue ( FLD_PAY_OVERDUE, CHR_ONE );
                pay.setValue ( FLD_PAY_TOTALPAYS, CHR_ZERO );
                pay.setValue ( FLD_PAY_CLIENTID, QString::number ( getNewClientID ( job_query.value ( FLD_OLDJOB_CLIENTID ).toInt () ) ) );
                pay.setValue ( FLD_PAY_JOBID, jobid );
                pay.setValue ( FLD_PAY_PRICE, job_query.value( FLD_OLDJOB_PRICE ).toString() );
            }
            pay.saveRecord ();
            pay.clearAll ();
            pay.setAction ( ACTION_ADD );
		} while ( job_query.next () );
	}
	VDB ()->setHighestID ( TABLE_JOB_ORDER ); // Job will update its table. Reset information within VivenciaDB
	VDB ()->database ()->exec ( QStringLiteral ( "DROP TABLE `OLD_PAYMENTS`" ) );
	VDB ()->optimizeTable ( &Payment::t_info );
	return true;
#endif
	return false;
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
//		case SC_REPORT_1: return FLD_PAY_METHOD_EXTRA;
		case SC_REPORT_2:
			return FLD_PAY_OBS;
		case SC_PRICE_1:
			return FLD_PAY_PRICE;
//		case SC_PRICE_2: return FLD_PAY_VALUE;
//		case SC_DATE_1: return FLD_PAY_DATE;
//		case SC_DATE_2: return FLD_PAY_USEDATE;
//		case SC_TYPE: return FLD_PAY_ACCOUNT;
		case SC_EXTRA_1:
			return FLD_PAY_CLIENTID;
		case SC_EXTRA_2:
			return FLD_PAY_JOBID;
		default:
			return -1;
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
