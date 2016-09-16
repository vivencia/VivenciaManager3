#ifndef JOB_H
#define JOB_H

#include "global.h"
#include "dbrecord.h"

class Client;
class jobListItem;

class QListWidgetItem;

const uint JOB_FIELD_COUNT ( 12 );
const uint JOB_REPORT_FIELDS_COUNT ( 5 );

static const uint JOBS_FIELDS_TYPE[JOB_FIELD_COUNT] = {
	DBTYPE_ID, DBTYPE_ID, DBTYPE_SHORTTEXT, DBTYPE_DATE, DBTYPE_DATE, DBTYPE_TIME,
	DBTYPE_PRICE, DBTYPE_FILE, DBTYPE_SHORTTEXT, DBTYPE_FILE, DBTYPE_SHORTTEXT, DBTYPE_LONGTEXT
};

#ifdef TRANSITION_PERIOD
enum OLD_JOB_TABLE_FIELDS {
	FLD_OLDJOB_ID = 0, FLD_OLDJOB_CLIENTID = 1, FLD_OLDJOB_STARTDATE = 2, FLD_OLDJOB_ENDDATE = 3, FLD_OLDJOB_MOWER = 4, FLD_OLDJOB_TIME = 5,
	FLD_OLDJOB_TYPENAME = 6, FLD_OLDJOB_PRICE = 7, FLD_OLDJOB_FILEPATH = 8, FLD_OLDJOB_IMAGE = 9,
	FLD_OLDJOB_PROJECTID = 10, FLD_OLDJOB_REPORT = 11
};
#endif

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

	inline DB_FIELD_TYPE fieldType ( const uint field ) const {
		return static_cast<DB_FIELD_TYPE> ( JOBS_FIELDS_TYPE[field] ); }

	int searchCategoryTranslate ( const SEARCH_CATEGORIES sc ) const;

    inline const QString projectIDTemplate () const {
		return date ( FLD_JOB_STARTDATE ).toDate ( vmNumber::VDF_FILE_DATE ) +
            CHR_HYPHEN + recStrValue ( this, FLD_JOB_ID );
	}

	inline const QString jobSummary () const { return jobSummary ( recStrValue ( this, FLD_JOB_ID ) ); }
    static const QString jobAddress ( const Job* const job = nullptr, Client* client = nullptr );
	static QString jobSummary ( const QString& jobid );
	static QString concatenateJobInfo ( const Job& job );
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
