#include "job.h"
#include "global.h"
#include "system_init.h"
#include "mainwindow.h"
#include "vmlistitem.h"
#include "vivenciadb.h"
#include "vmnumberformats.h"
#include "vmwidgets.h"
#include "stringrecord.h"

static const unsigned int TABLE_VERSION ( 'B' );

constexpr DB_FIELD_TYPE JOBS_FIELDS_TYPE[JOB_FIELD_COUNT] = {
	DBTYPE_ID, DBTYPE_ID, DBTYPE_SHORTTEXT, DBTYPE_DATE, DBTYPE_DATE, DBTYPE_TIME,
	DBTYPE_PRICE, DBTYPE_FILE, DBTYPE_SHORTTEXT, DBTYPE_FILE, DBTYPE_SHORTTEXT, DBTYPE_SUBRECORD, DBTYPE_SUBRECORD
};

#ifdef JOB_TABLE_UPDATE_AVAILABLE
bool updateJobTable ()
{
	VDB ()->insertColumn ( FLD_JOB_KEYWORDS, &Job::t_info );
	VDB ()->optimizeTable ( &Job::t_info );
	return true;
}
#endif //JOB_TABLE_UPDATE_AVAILABLE

const TABLE_INFO Job::t_info =
{
	JOB_TABLE,
	QStringLiteral ( "JOBS" ),
	QStringLiteral ( " ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci" ),
	QStringLiteral ( " PRIMARY KEY ( `ID` ) , UNIQUE KEY `id` ( `ID` ) " ),
	QStringLiteral ( "`ID`|`CLIENTID`|`TYPE`|`STARTDATE`|`ENDDATE`|`TIME`|`PRICE`|`PROJECTPATH`|`PROJECTID`|`IMAGEPATH`|`JOB_ADDRESS`|`KEYWORDS`|`REPORT`|" ),
	QStringLiteral ( " int ( 9 ) NOT NULL, | int ( 9 ) NOT NULL, | varchar ( 100 ) COLLATE utf8_unicode_ci DEFAULT NULL, | varchar ( 20 ) DEFAULT NULL, |"
	" varchar ( 20 ) DEFAULT NULL, | varchar ( 20 ) DEFAULT NULL, | varchar ( 20 ) COLLATE utf8_unicode_ci DEFAULT NULL, |"
	" varchar ( 255 ) COLLATE utf8_unicode_ci DEFAULT NULL, | varchar ( 30 ) DEFAULT NULL, | varchar ( 255 ) COLLATE utf8_unicode_ci DEFAULT NULL, |"
	" varchar ( 255 ) COLLATE utf8_unicode_ci DEFAULT NULL, | longtext COLLATE utf8_unicode_ci, |longtext COLLATE utf8_unicode_ci, |" ),
	QStringLiteral ( "ID|Client ID|Type|Start date|Finish date|Worked hours|Price|Project path|Project ID|Image path|Job Address|Job Keywords|Report|" ),
	JOBS_FIELDS_TYPE, TABLE_VERSION, JOB_FIELD_COUNT, TABLE_JOB_ORDER,
	#ifdef JOB_TABLE_UPDATE_AVAILABLE
	&updateJobTable
	#else
	nullptr
	#endif //JOB_TABLE_UPDATE_AVAILABLE
	#ifdef TRANSITION_PERIOD
	, true
	#endif
};

static void updateJobCompleters ( const DBRecord* db_rec )
{
	APP_COMPLETERS ()->updateCompleter ( recStrValue ( db_rec, FLD_JOB_TYPE ) , vmCompleters::JOB_TYPE );
}

Job::Job ( const bool connect_helper_funcs )
	: DBRecord ( JOB_FIELD_COUNT )
{
	::memset ( this->helperFunction, 0, sizeof ( this->helperFunction ) );
	DBRecord::t_info = & ( this->t_info );
	DBRecord::m_RECFIELDS = this->m_RECFIELDS;
	DBRecord::mFieldsTypes = JOBS_FIELDS_TYPE;
	
	if ( connect_helper_funcs )
	{
		DBRecord::helperFunction = this->helperFunction;
		setHelperFunction ( FLD_JOB_TYPE, &updateJobCompleters );
	}
}

Job::~Job () {}

int Job::searchCategoryTranslate ( const SEARCH_CATEGORIES sc ) const
{
	switch ( sc )
	{
		case SC_ID: return FLD_JOB_ID;
		case SC_REPORT_1: return FLD_JOB_REPORT;
		case SC_REPORT_2: return FLD_JOB_KEYWORDS;
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

void Job::copySubRecord ( const uint subrec_field, const stringRecord& subrec )
{
	if ( subrec_field == FLD_JOB_REPORT )
	{
		if ( subrec.curIndex () == -1 )
			subrec.first ();
		stringRecord job_report;
		for ( uint i ( 0 ); i < JOB_REPORT_FIELDS_COUNT; ++i )
		{
			job_report.fastAppendValue ( subrec.curValue () );
			if ( !subrec.next () ) break;
		}
		setRecValue ( this, FLD_JOB_REPORT, job_report.toString () );
	}
}

const QString Job::jobAddress ( const Job* const job, Client* client )
{
	if ( job )
	{
		if ( !recStrValue ( job, FLD_JOB_ADDRESS ).isEmpty () )
			return recStrValue ( job, FLD_JOB_ADDRESS );
	}
	if ( client == nullptr )
	{
		Client localClient ( false );
		client = &localClient;
	}

	if ( job != nullptr )
	{
		if ( !client->readRecord ( recStrValue ( job, FLD_JOB_CLIENTID ).toUInt () ) )
			return emptyString;
	}
	return	recStrValue ( client, FLD_CLIENT_STREET ) + CHR_COMMA + CHR_SPACE +
			recStrValue ( client, FLD_CLIENT_NUMBER ) + CHR_SPACE + CHR_HYPHEN + CHR_SPACE +
			recStrValue ( client, FLD_CLIENT_DISTRICT ) + CHR_SPACE + CHR_HYPHEN + CHR_SPACE +
			recStrValue ( client, FLD_CLIENT_CITY );
}

QString Job::jobSummary ( const QString& jobid )
{
	if ( !jobid.isEmpty () )
	{
		Job job;
		if ( job.readRecord ( jobid.toUInt () ) )
		{
			const QLatin1String str_sep ( " - " );
			return ( recStrValue ( &job, FLD_JOB_TYPE ) + str_sep +
				 vmNumber ( recStrValue ( &job, FLD_JOB_STARTDATE ), VMNT_DATE, vmNumber::VDF_HUMAN_DATE ).toString () +
				 str_sep + Client::clientName ( recStrValue ( &job, FLD_JOB_CLIENTID ) ) +
				 CHR_L_PARENTHESIS + recStrValue ( &job, FLD_JOB_CLIENTID ) + CHR_R_PARENTHESIS );
		}
	}
	return QStringLiteral ( "No job" );
}

QString Job::concatenateJobInfo ( const Job& job , const bool b_skip_report )
{
	QString info;

	info = APP_TR_FUNC ( "Serviço " ) + recStrValue ( &job, FLD_JOB_PROJECT_ID ) + CHR_NEWLINE;
	info += QLatin1String ( "Cliente: " ) + Client::clientName ( recStrValue ( &job, FLD_JOB_CLIENTID ).toUInt () ) + CHR_NEWLINE;
	info += QLatin1String ( "Catetoria: " ) + recStrValue ( &job, FLD_JOB_TYPE ) + CHR_NEWLINE;

	info += APP_TR_FUNC ( "Preço do serviço: " ) + recStrValue ( &job, FLD_JOB_PRICE ) + CHR_NEWLINE;
			
	info += APP_TR_FUNC ( "Período de execução: " ) +
			job.date ( FLD_JOB_STARTDATE ).toDate ( vmNumber::VDF_LONG_DATE ) + QLatin1String ( " a " ) +
			job.date ( FLD_JOB_ENDDATE ).toDate ( vmNumber::VDF_LONG_DATE ) + CHR_NEWLINE;

	info += APP_TR_FUNC ( "Duração do serviço: " ) + job.time ( FLD_JOB_TIME ).toTime ( vmNumber::VTF_FANCY ) + CHR_NEWLINE;

	if ( !recStrValue ( &job, FLD_JOB_PROJECT_ID ).isEmpty () )
		info += QLatin1String ( "Projeto #: " ) + recStrValue ( &job, FLD_JOB_PROJECT_ID ) + CHR_NEWLINE;

	if ( !b_skip_report && !recStrValue ( &job, FLD_JOB_REPORT ).isEmpty () )
	{
		info += APP_TR_FUNC ( "Relatório de execução:" ) + CHR_NEWLINE;
		const stringTable jobReport ( recStrValue ( &job, FLD_JOB_REPORT ) );
		if ( jobReport.firstStr () ) {
			stringRecord jobDay;
			do
			{
				jobDay.fromString ( jobReport.curRecordStr () );
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
	if ( VDB ()->runSelectLikeQuery ( QLatin1String ( "SELECT TYPE,STARTDATE FROM JOBS WHERE ID='" ) + id + CHR_CHRMARK, query ) )
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
