#ifndef JOB_H
#define JOB_H

#include "global.h"
#include "dbrecord.h"

class Client;
class jobListItem;

class QListWidgetItem;

const uint JOB_FIELD_COUNT ( 13 );
const uint JOB_REPORT_FIELDS_COUNT ( 5 );

class Job : public DBRecord
{

friend class VivenciaDB;
friend class MainWindow;

friend void updateCalendarJobInfo ( const DBRecord* db_rec );

public:

	enum JOB_REPORT_FIELDS
	{
		JRF_DATE = 0, JRF_START_TIME = 1, JRF_END_TIME = 2, JRF_WHEATHER = 3, JRF_REPORT = 4, JRF_EXTRA = 5
		/*Possible JRF_EXTRA values: CHR_ONE: information added / CHR_ZERO: information removed*/
	};

	explicit Job ( const bool connect_helper_funcs = false );
	virtual ~Job ();

	int searchCategoryTranslate ( const SEARCH_CATEGORIES sc ) const;
	void copySubRecord ( const uint subrec_field, const stringRecord& subrec );

	inline const QString projectIDTemplate () const
	{
		return date ( FLD_JOB_STARTDATE ).toDate ( vmNumber::VDF_FILE_DATE ) +
			CHR_HYPHEN + recStrValue ( this, FLD_JOB_ID );
	}

	inline const QString jobSummary () const { return jobSummary ( recStrValue ( this, FLD_JOB_ID ) ); }
	static const QString jobAddress ( const Job* const job = nullptr, Client* client = nullptr );
	static QString jobSummary ( const QString& jobid );
	static QString concatenateJobInfo ( const Job& job, const bool b_skip_report );
	static inline const QString jobTypeWithDate ( const int id ) {
		return ( id > 0 ) ? jobTypeWithDate ( QString::number ( id ) ) : emptyString;
	}
	static QString jobTypeWithDate ( const QString& id );
	const QString jobTypeWithDate () const;

	void setListItem ( jobListItem* job_item );
	jobListItem* jobItem () const;

	static const TABLE_INFO t_info;

protected:
	friend bool updateJobTable ();

	RECORD_FIELD m_RECFIELDS[JOB_FIELD_COUNT];
	void ( *helperFunction[JOB_FIELD_COUNT] ) ( const DBRecord* );
};

#endif // JOB_H
