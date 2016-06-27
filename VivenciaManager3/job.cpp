#include "job.h"
#include "global.h"
#include "data.h"
#include "mainwindow.h"
#include "vmlistitem.h"
#include "vivenciadb.h"
#include "vmnumberformats.h"
#include "vmwidgets.h"
#include "stringrecord.h"
#include "dbcalendar.h"

const double TABLE_VERSION ( 2.0 );

#ifdef TRANSITION_PERIOD
#include "quickproject.h"
#include "generaltable.h"
#include "vmlist.h"

#include <QtSql/QSqlQuery>

void getJobTimes ( const QString& strInput, vmNumber* in_hour, vmNumber* out_hour )
{
	int hs_idx ( -1 );
	uint i ( 0 );
	bool b_foundHour ( false );
	uint hour ( 0 ) , min ( 0 );
	const QString str ( strInput.simplified () );
	static const QString HS ( QStringLiteral ( "hs" ) );
	static const QString AS ( QString::fromUtf8 ( "às" ) );

	do {
		hs_idx = str.indexOf ( HS, hs_idx + 1 );
		if ( hs_idx != -1 ) {
			if ( str.at ( hs_idx - 3 ) != CHR_COLON ) continue;
			i = hs_idx - 5;
			if ( !str.at ( hs_idx - 5 ).isDigit () )
				++i;
			if ( !b_foundHour ) {
				if ( str.mid( hs_idx + 2, 60 ).indexOf ( AS ) != -1 ) {
					hour = str.mid ( i, hs_idx - i - 3 ).toInt ();
					min = str.mid ( hs_idx - 2, 2 ).toInt ();
					b_foundHour = true;
					in_hour->setTime ( hour, min );
				}
			}
			else {
				hour = str.mid ( i, hs_idx - i - 3 ).toInt ();
				min = str.mid ( hs_idx - 2, 2 ).toInt ();
				out_hour->setTime ( hour, min );
				b_foundHour = false;
				break;
			}
		}
		else
			break;
	} while ( true );
}

void getJobDaysReports ( const QString& job_report,	stringTable& job_info, const QString& one_day_date )
{
	const QString strCue ( QStringLiteral ( "o dia," ) );
	int day_idx1 ( -1 ), day_idx2 ( -1 );
	int pos_date ( -1 ), i ( 0 );
	int datetime_portion_length ( 0 );
	QString str_date;
	QString str_day;
	QString str_day_report;
	int n_day ( -1 );
	vmNumber in_hour, out_hour, date;
	stringRecord day_rec;

	do {
		if ( ( day_idx1 = job_report.indexOf ( strCue, day_idx1 + 1 ) ) > 0 ) {
			str_day = job_report.at ( day_idx1 - 1 );
			if ( str_day.at ( 0 ).isDigit () ) {
				if ( day_idx1 - 2 >= 0 ) {
					if ( job_report.at ( day_idx1 - 2 ).isDigit () )
						str_day.prepend ( job_report.at ( day_idx1 - 2 ) );
				}
				n_day = str_day.toInt ();
				if ( n_day <= 0 )
					break;
				//qDebug () << "This is day " << str_day;

				if ( ( pos_date = job_report.indexOf ( CHR_COLON, ( day_idx1 + 7 ) ) ) != -1 ) {
					str_date = job_report.mid ( ( day_idx1 + 7 ), pos_date - ( day_idx1 + 7 ) );
					i = 1;
					do {
						if ( str_date.at ( i ).isDigit () ) continue;
						else if ( str_date.at ( i ) == CHR_F_SLASH ) continue;
						else break;
					} while ( ++i < str_date.count () );

					str_date.truncate ( i );
					date.fromStrDate ( str_date );
					str_date = date.toDate ( vmNumber::VDF_DB_DATE );
					//qDebug () << "The date is " << str_date;
				}
				if ( ( day_idx2 = job_report.indexOf ( strCue, day_idx1 + 8 ) ) < 0 )
					day_idx2 = job_report.toUtf8 ().size () + 1;

				datetime_portion_length = job_report.indexOf ( CHR_DOT, day_idx1 + 1 ) + 2 - day_idx1;

				//qDebug () << QLatin1String ( "The time is: " ) << Data::getJobTime ( job_report.mid ( day_idx1, day_idx2 - day_idx1 ), &in_hour, &out_hour ).toString ();
				//qDebug () << "Start time: " << in_hour.toTime () << "  - End time: " << out_hour.toTime ();
				getJobTimes ( job_report.mid ( day_idx1, day_idx2 - day_idx1 ), &in_hour, &out_hour );
				str_day_report = job_report.mid ( day_idx1 + datetime_portion_length, day_idx2 - day_idx1 - datetime_portion_length - str_day.length () - 1 );

				day_rec.fastAppendValue ( str_date ); //Job::JRF_DATE
				day_rec.fastAppendValue ( in_hour.toTime () ); //Job::JRF_START_TIME
				day_rec.fastAppendValue ( out_hour.toTime () ); //Job::JRF_END_TIME
				day_rec.fastAppendValue ( emptyString ); //Job::JRF_WHEATHER
				day_rec.fastAppendValue ( str_day_report ); //Job::JRF_REPORT
				job_info.insertRecord ( n_day - 1, day_rec );
				day_rec.clear ();
				in_hour.clear ( false );
				out_hour.clear ( false );
			}
		}
	} while ( day_idx1 != -1 );
	if ( n_day == -1 ) {
		getJobTimes ( job_report, &in_hour, &out_hour );
		day_idx2 = job_report.toUtf8 ().size () + 1;
		datetime_portion_length = job_report.indexOf ( CHR_DOT, 0 ) + 2;
		str_day_report = job_report.mid ( datetime_portion_length, day_idx2 - datetime_portion_length );

		day_rec.fastAppendValue ( one_day_date ); //Job::JRF_DATE
		day_rec.fastAppendValue ( in_hour.toTime () ); //Job::JRF_START_TIME
		day_rec.fastAppendValue ( out_hour.toTime () ); //Job::JRF_END_TIME
		day_rec.fastAppendValue ( emptyString ); //Job::JRF_WHEATHER
		day_rec.fastAppendValue ( str_day_report ); //Job::JRF_REPORT
		job_info.insertRecord ( 0, day_rec );
	}
}

void changePaysJobIDs ( const QString& str_newid, const QString& str_oldid )
{
	Payment pay ( false );
	if ( pay.readRecord ( FLD_PAY_JOBID, str_oldid, false ) ) {
		pay.setAction ( ACTION_EDIT );
		setRecValue ( &pay, FLD_PAY_JOBID, str_newid );
		pay.saveRecord ();
	}
}

void changeBuysJobIDs ( const QString& str_newid, const QString& str_oldid )
{
	Buy buy ( false );
	if ( buy.readFirstRecord ( FLD_BUY_JOBID, str_oldid, false ) ) {
		do {
            buy.setAction ( ACTION_EDIT );
			setRecValue ( &buy, FLD_BUY_JOBID, str_newid );
			buy.saveRecord ();
			buy.clearAll ();
		} while ( buy.readNextRecord ( true, false ) );
	}
}

void changeQPJobIDs ( const QString& str_newid, const QString& str_oldid )
{
	quickProject qp;
	if ( qp.readRecord ( FLD_QP_JOB_ID, str_oldid ) ) {
		qp.setAction ( ACTION_EDIT );
		setRecValue ( &qp, FLD_QP_JOB_ID, str_newid );
		qp.saveRecord ();
	}
}
#endif //TRANSITION_PERIOD

bool updateJobTable ()
{
#ifdef TRANSITION_PERIOD
	VDB ()->database ()->exec ( QStringLiteral ( "RENAME TABLE `JOBS` TO `OLD_JOBS`" ) );
	VDB ()->createTable ( &Job::t_info );

	Job job ( false );
	QSqlQuery query;
	stringTable job_report_info;
	vmNumber price;
	QString strNewId, strOldId;

	if ( VDB ()->runQuery ( QStringLiteral ( "SELECT * FROM `OLD_JOBS`" ), query ) ) {
		job.setAction ( ACTION_ADD );
		do {
			job.setValue ( FLD_JOB_CLIENTID, QString::number ( getNewClientID ( query.value ( FLD_OLDJOB_CLIENTID ).toInt () ) ) );
			job.setValue ( FLD_JOB_TYPE, query.value ( FLD_OLDJOB_TYPENAME ).toString () );
			job.setValue ( FLD_JOB_STARTDATE, query.value ( FLD_OLDJOB_STARTDATE ).toString () );
			job.setValue ( FLD_JOB_ENDDATE, query.value ( FLD_OLDJOB_ENDDATE ).toString () );
			job.setValue ( FLD_JOB_TIME, query.value ( FLD_OLDJOB_TIME ).toString () );
			job.setValue ( FLD_JOB_PROJECT_PATH, query.value ( FLD_OLDJOB_FILEPATH ).toString () );
			job.setValue ( FLD_JOB_PROJECT_ID, query.value ( FLD_OLDJOB_PROJECTID ).toString () );
			job.setValue ( FLD_JOB_PICTURE_PATH, query.value ( FLD_OLDJOB_IMAGE ).toString () );
			job.setValue ( FLD_JOB_PRICE, query.value ( FLD_OLDJOB_PRICE ).toString () );
			if ( recStrValue ( &job, FLD_JOB_PRICE ).isEmpty () )
				job.setValue ( FLD_JOB_PRICE, price.toPrice () );

			getJobDaysReports ( query.value ( FLD_OLDJOB_REPORT ).toString (), job_report_info, query.value ( FLD_OLDJOB_STARTDATE ).toString () );

			if ( !job_report_info.isOK () )
				continue;
			job.setValue ( FLD_JOB_REPORT, job_report_info.toString () );
			job.saveRecord ();

			strOldId = query.value ( FLD_OLDJOB_ID ).toString ();
			strNewId = recStrValue ( &job, FLD_JOB_ID );
			if ( strNewId != strOldId ) {
				changePaysJobIDs ( strNewId, strOldId );
				changeBuysJobIDs ( strNewId, strOldId );
				changeQPJobIDs ( strNewId, strOldId );
			}
			job.clearAll ();
			job_report_info.clear ();
			job.setAction ( ACTION_ADD );
		} while ( query.next () );
	}
	VDB ()->database ()->exec ( QStringLiteral ( "DROP TABLE `OLD_JOBS`" ) );
	// Index report column for fast search
	VDB ()->database ()->exec ( QStringLiteral ( "ALTER TABLE JOBS ADD FULLTEXT REP_IDX(REPORT)" ) );
	VDB ()->database ()->exec ( QStringLiteral ( "REPAIR TABLE JOBS QUICK" ) );
	VDB ()->optimizeTable ( &Job::t_info );
	return true;

#endif //TRANSITION_PERIOD
	return false;
}

const TABLE_INFO Job::t_info = {
	JOB_TABLE,
	QStringLiteral ( "JOBS" ),
	QStringLiteral ( " ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci" ),
	QStringLiteral ( " PRIMARY KEY ( `ID` ) , UNIQUE KEY `id` ( `ID` ) " ),
	QStringLiteral ( "`ID`|`CLIENTID`|`TYPE`|`STARTDATE`|`ENDDATE`|`TIME`|`PRICE`|`PROJECTPATH`|`PROJECTID`|`IMAGEPATH`|`JOB_ADDRESS`|`REPORT`|" ),
	QStringLiteral ( " int ( 9 ) NOT NULL, | int ( 9 ) NOT NULL, | varchar ( 100 ) COLLATE utf8_unicode_ci DEFAULT NULL, | varchar ( 20 ) DEFAULT NULL, |"
	" varchar ( 20 ) DEFAULT NULL, | varchar ( 20 ) DEFAULT NULL, | varchar ( 20 ) COLLATE utf8_unicode_ci DEFAULT NULL, |"
	" varchar ( 255 ) COLLATE utf8_unicode_ci DEFAULT NULL, | varchar ( 30 ) DEFAULT NULL, | varchar ( 255 ) COLLATE utf8_unicode_ci DEFAULT NULL, |"
	" varchar ( 255 ) COLLATE utf8_unicode_ci DEFAULT NULL, | longtext COLLATE utf8_unicode_ci, |" ),
	QStringLiteral ( "ID|Client ID|Type|Start date|Finish date|Worked hours|Price|Project path|Project ID|Image path|Job Address|Report|" ),
	JOBS_FIELDS_TYPE,
	TABLE_VERSION, JOB_FIELD_COUNT, TABLE_JOB_ORDER, &updateJobTable
	#ifdef TRANSITION_PERIOD
	, true
	#endif
};

static void updateJobCompleters ( const DBRecord* db_rec )
{
	APP_COMPLETERS ()->updateCompleter ( recStrValue ( db_rec, FLD_JOB_TYPE ) , vmCompleters::JOB_TYPE );
}

Job::Job ( const bool connect_helper_funcs )
	: DBRecord ( JOB_FIELD_COUNT ), ce_list ( 10 )
{
	::memset ( this->helperFunction, 0, sizeof ( this->helperFunction ) );
	DBRecord::t_info = & ( this->t_info );
	DBRecord::m_RECFIELDS = this->m_RECFIELDS;

	if ( connect_helper_funcs ) {
		DBRecord::helperFunction = this->helperFunction;
		setHelperFunction ( FLD_JOB_TYPE, &updateJobCompleters );
	}
}

Job::~Job ()
{
	ce_list.clear ( true );
}

int Job::searchCategoryTranslate ( const SEARCH_CATEGORIES sc ) const
{
	switch ( sc ) {
		case SC_ID: return FLD_JOB_ID;
		case SC_REPORT_1: return FLD_JOB_REPORT;
		case SC_ADDRESS_1: return FLD_JOB_PROJECT_PATH;
		case SC_ADDRESS_2: return FLD_JOB_ADDRESS;
		case SC_PRICE_1: return FLD_JOB_PRICE;
		case SC_DATE_1: return FLD_JOB_STARTDATE;
		case SC_DATE_2: return FLD_JOB_ENDDATE;
		case SC_TIME_1: return FLD_JOB_TIME;
		case SC_TYPE: return FLD_JOB_TYPE;
		case SC_EXTRA_1: return FLD_JOB_CLIENTID;
		case SC_EXTRA_2: return FLD_JOB_PROJECT_ID;
		default: return -1;
	}
}

void Job::updateCalendarJobInfo ()
{
	if ( ce_list.isEmpty () )
		return;

	dbCalendar* cal ( new dbCalendar );
	stringRecord calendarIdTrio;
	calendarIdTrio.fastAppendValue ( actualRecordStr ( FLD_JOB_ID ) );
	calendarIdTrio.fastAppendValue ( actualRecordStr ( FLD_JOB_CLIENTID ) );

	CALENDAR_EXCHANGE* ce ( nullptr );
	for ( uint i ( 0 ); i < ce_list.count (); ++i ) {
		ce = ce_list.at ( i );
		calendarIdTrio.changeValue ( 2, QString::number ( ce->extra_info ) );
		switch ( ce->action ) {
			case CEAO_NOTHING:
				continue;
			case CEAO_ADD_DATE1:
				cal->addDate ( ce->date, FLD_CALENDAR_JOBS, calendarIdTrio );
			break;
			case CEAO_DEL_DATE1:
				cal->delDate ( ce->date, FLD_CALENDAR_JOBS, calendarIdTrio );
			break;
			case CEAO_ADD_PRICE_DATE1:
				cal->addPrice ( ce->date, ce->price, FLD_CALENDAR_TOTAL_JOBPRICE_SCHEDULED );
			break;
			case CEAO_DEL_PRICE_DATE1:
				cal->delPrice ( ce->date, ce->price, FLD_CALENDAR_TOTAL_JOBPRICE_SCHEDULED );
			break;
			default:
			break;
		}
	}
	ce_list.clearButKeepMemory ( true );
	delete cal;
}

const QString Job::jobAddress ( const Job* const job, Client* client )
{
    if ( job ) {
        if ( !recStrValue ( job, FLD_JOB_ADDRESS ).isEmpty () )
            return recStrValue ( job, FLD_JOB_ADDRESS );
    }
    if ( client == nullptr ) {
        Client localClient ( false );
        client = &localClient;
    }

    if ( job != nullptr ) {
        if ( !client->readRecord ( recStrValue ( job, FLD_JOB_CLIENTID ).toInt () ) )
            return emptyString;
    }
    return	recStrValue ( client, FLD_CLIENT_STREET ) + CHR_COMMA + CHR_SPACE +
            recStrValue ( client, FLD_CLIENT_NUMBER ) + CHR_SPACE + CHR_HYPHEN + CHR_SPACE +
            recStrValue ( client, FLD_CLIENT_DISTRICT ) + CHR_SPACE + CHR_HYPHEN + CHR_SPACE +
            recStrValue ( client, FLD_CLIENT_CITY );
}

QString Job::jobSummary ( const QString& jobid )
{
	if ( !jobid.isEmpty () ) {
		Job job;
		if ( job.readRecord ( jobid.toInt () ) ) {
			const QLatin1String str_sep ( " - " );
			return ( recStrValue ( &job, FLD_JOB_TYPE ) + str_sep +
				 vmNumber ( recStrValue ( &job, FLD_JOB_STARTDATE ), VMNT_DATE, vmNumber::VDF_HUMAN_DATE ).toString () +
				 str_sep + Client::clientName ( recStrValue ( &job, FLD_JOB_CLIENTID ) ) +
				 CHR_L_PARENTHESIS + recStrValue ( &job, FLD_JOB_CLIENTID ) + CHR_R_PARENTHESIS );
		}
	}
	return QStringLiteral ( "No job" );
}

QString Job::concatenateJobInfo ( const Job& job )
{
	QString info;

	info = QLatin1String ( "Serviço " ) + recStrValue ( &job, FLD_JOB_PROJECT_ID ) + CHR_NEWLINE;
	info += QLatin1String ( "Cliente: " ) + recStrValue ( &job, FLD_JOB_CLIENTID ) + CHR_NEWLINE;
	info += QLatin1String ( "Catetoria: " ) + recStrValue ( &job, FLD_JOB_TYPE ) + CHR_NEWLINE;

	info += QLatin1String ( "Período de execução: " ) +
			job.date ( FLD_JOB_STARTDATE ).toDate ( vmNumber::VDF_LONG_DATE ) + QLatin1String ( " a " ) +
			job.date ( FLD_JOB_ENDDATE ).toDate ( vmNumber::VDF_LONG_DATE ) + CHR_NEWLINE;

	info += QLatin1String ( "Tempo de serviço: " ) + job.time ( FLD_JOB_TIME ).toTime ( vmNumber::VTF_FANCY ) + CHR_NEWLINE;

	if ( !recStrValue ( &job, FLD_JOB_PROJECT_ID ).isEmpty () )
		info += QLatin1String ( "Projeto #: " ) + recStrValue ( &job, FLD_JOB_PROJECT_ID ) + CHR_NEWLINE;

	if ( !recStrValue ( &job, FLD_JOB_REPORT ).isEmpty () ) {
		info += QLatin1String ( "Relatório de execução:" ) + CHR_NEWLINE;
		const stringTable jobReport ( recStrValue ( &job, FLD_JOB_REPORT ) );
		if ( jobReport.firstStr () ) {
			stringRecord jobDay;
			do {
				jobDay.fromString ( jobReport.curRecord () );
				info += CHR_NEWLINE + jobDay.fieldValue ( JRF_DATE ) + QLatin1String ( " - " ) +
						jobDay.fieldValue ( JRF_WHEATHER ) + QLatin1String ( ": " ) +
						jobDay.fieldValue ( JRF_REPORT ) + CHR_NEWLINE;
			} while ( jobReport.nextStr () );
		}
	}
	return info;
}

QString Job::jobTypeWithDate ( const QString& id )
{
    QSqlQuery query;
    if ( VDB ()->runQuery ( QLatin1String ( "SELECT TYPE,STARTDATE FROM JOBS WHERE ID='" ) + id + CHR_CHRMARK, query ) )
        return  CHR_L_PARENTHESIS + id + CHR_R_PARENTHESIS + CHR_SPACE +
                query.value ( 0 ).toString () + QLatin1String ( " - " ) + query.value ( 1 ).toString ();
    return emptyString;
}

const QString Job::jobTypeWithDate () const
{
    return  CHR_L_PARENTHESIS + recStrValue ( this, FLD_JOB_ID ) + CHR_R_PARENTHESIS + CHR_SPACE +
            recStrValue ( this, FLD_JOB_TYPE ) + QLatin1String ( " - " ) + date ( FLD_JOB_STARTDATE ).toDate ( vmNumber::VDF_HUMAN_DATE );
}

void Job::setListItem ( jobListItem* job_item )
{
    DBRecord::mListItem = static_cast<vmListItem*>( job_item );
}

jobListItem* Job::jobItem () const
{
    return static_cast<jobListItem*>( DBRecord::mListItem );
}
