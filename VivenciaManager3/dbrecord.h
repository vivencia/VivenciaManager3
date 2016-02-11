#ifndef DBRECORD_H
#define DBRECORD_H

#include "vmlist.h"
#include "vmnumberformats.h"
#include "stringrecord.h"
#include "global_db.h"

#include <QStringList>
#include <QtSql/QSqlQuery>

#define setRecValue(dbrecord,field,value) (dbrecord)->change ( field, value )
#define setRecIntValue(dbrecord,field,value) (dbrecord)->changeInt ( field, value )
#define recStrValue(dbrecord,field) (dbrecord)->recordStr ( field )
#define recStrValueAlternate(dbrecord,field) (dbrecord)->recordStrAlternate ( field )
#define recIntValue(dbrecord,field) (dbrecord)->recordInt ( field )

class vmListItem;

class DBRecord
{

	typedef void ( DBRecord::*alterRecord ) ( const uint, const QString& );
	typedef void ( DBRecord::*alterRecordInt ) ( const uint, const int );
	typedef const QString& ( DBRecord::*recordValueStr ) ( const uint ) const;
	typedef int ( DBRecord::*recordValueInt ) ( const uint ) const;

	friend class VivenciaDB;
	friend class Data;
	friend class searchUI;
	friend class vmListItem;

public:

	struct st_Query {
		QString search;
		QString str_query;
		int field;
		QSqlQuery* query;
		bool reset;
		bool forward;

		st_Query ()
			: field ( -1 ), query ( nullptr ), reset ( true ), forward ( true ) {}

		~st_Query () {
			delete query;
		}
	};


	DBRecord ( const uint field_count );
	DBRecord ( const DBRecord* other );

	virtual ~DBRecord (); // if a class has virtual functions, if should have a virtual destructor

	const DBRecord& operator= ( const DBRecord& other );
	bool operator== ( const DBRecord& other ) const;
	inline bool operator!= ( const DBRecord& other ) const {
		return !( this->operator == ( other ) ); }

	inline void callHelperFunction ( const uint field )
	{
		if ( helperFunction[field] )
			( *helperFunction[field] ) ( this );
	}

	// Convenience functions that assume the recorded value in db is formatted ( for the sake of speed )
	// but handles graciously if it is empty
	inline vmNumber phone ( const uint field ) const {
		return vmNumber ( recStrValue ( this, field ), VMNT_PHONE, 1 ); }
	inline vmNumber price ( const uint field ) const {
		return vmNumber ( recStrValue ( this, field ), VMNT_PRICE, 1 ); }
	inline vmNumber date ( const uint field ) const {
		return vmNumber ( recStrValue ( this, field ), VMNT_DATE, vmNumber::VDF_DB_DATE ); }
	inline vmNumber time ( const uint field ) const {
		return vmNumber ( recStrValue ( this, field ), VMNT_TIME, vmNumber::VTF_DAYS ); }
	inline vmNumber number ( const uint field ) const {
		return vmNumber ( recStrValue ( this, field ), VMNT_INT, 1 ); }
	inline bool opt ( const uint field ) const {
		return recStrValue ( this, field ) == QStringLiteral ( "1" ); }

	/* These functions take unformatted text from an untrusted source (i.e. the user),
	 * format it to satisfy the number condition and return a reference of the formatted string
	 * or the number itself to be used by the GUI. To avoid problems, the returned reference must be used immediately
	 * or passed on to another object or function that - then - copies the it (i.e. a line edit
	 * receives the text via reference but copies it over to an internal buffer. This avoids too
	 * many copies of the object and speeds up execution
	 *
	 * CHR_ZERO and CHR_ONE are note used so that I don't have to include "global.h"
	 */

	//const vmNumber& setPrice ( const uint field, const QString& price );
	inline void setDate ( const uint field, const vmNumber& date ) {
		setRecValue ( this, field, date.toDate ( vmNumber::VDF_DB_DATE ) ); }
	inline void setTime ( const uint field, const vmNumber& time ) {
		setRecValue ( this, field, time.toTime ( vmNumber::VTF_DAYS ) ); }
	inline void setBool ( const uint field, const bool arg ) {
		setRecValue ( this, field, ( arg ? QStringLiteral ( "1" ) : QStringLiteral ( "2" ) ) ); }

	void setHelperFunction ( const uint field, void ( *helperFunc ) ( const DBRecord* ) );
	bool readRecord ( const int id, const bool load_data = true );
	bool readRecord ( const uint field, const QString& search, const bool load_data = true );

	inline bool searchQueryIsOn () const { return !stquery.reset; }
	void resetQuery ();
	bool readFirstRecord ( const bool load_data = true );
	bool readFirstRecord ( const int field, const QString& search = QString::null, const bool load_data = true );
	bool readLastRecord ( const bool load_data = true );
	bool readLastRecord ( const int field, const QString& search = QString::null, const bool load_data = true );
	bool readNextRecord ( const bool follow_search = false, const bool load_data = true );
	bool readPrevRecord ( const bool follow_search = false, const bool load_data = true );
	bool deleteRecord ();
	bool saveRecord ();
	void contains ( const QString& value, podList<uint>& fields ) const;

	inline bool isModified () const { return mb_modified; }
	inline bool isModified ( const uint field ) const { return m_RECFIELDS[field].modified; }
	inline bool wasModified ( const uint field ) const { return m_RECFIELDS[field].was_modified; }

	inline uint fieldCount () const { return fld_count; }
	inline uint typeID () const { return t_info->table_id; }

	inline virtual QString isrValue ( const ITEMS_AND_SERVICE_RECORD, const int = -1 ) const { return QString (); }
	inline virtual uint isrRecordField ( const ITEMS_AND_SERVICE_RECORD ) const { return 0; }
	virtual int searchCategoryTranslate ( const SEARCH_CATEGORIES sc ) const;
	inline virtual DB_FIELD_TYPE fieldType ( const uint /*field*/ ) const { return DBTYPE_SHORTTEXT; }

	inline const QString& recordStr ( const uint field ) const {
			return ( this->*DBRecord::fptr_recordStr ) ( field ); }
	inline int recordInt ( const uint field ) const {
			return ( this->*DBRecord::fptr_recordInt ) ( field ); }
	inline const QString& recordStrAlternate ( const uint field ) const {
			return ( this->*DBRecord::fptr_recordStrAlternate ) ( field ); }
	inline void change ( const uint field, const QString& value ) {
			( this->*DBRecord::fptr_change ) ( field, value ); }
	inline void changeInt ( const uint field, const int value ) {
			( this->*DBRecord::fptr_changeInt ) ( field, value ); }

	void setModified ( const uint field, const bool modified );
	void setAllModified ( const bool modified );
	void changeWasModifiedFlag ( const bool modified, const int field = -1 );
	void clearAll ();

	inline RECORD_ACTION action () const { return m_action; }
	inline RECORD_ACTION prevAction () const { return m_prevaction; }
	void setAction ( const RECORD_ACTION action );
	static void addToTemporaryRecords ( DBRecord* dbrec );
	static void removeFromTemporaryRecords ( DBRecord* dbrec );

	inline bool inSync () const { return mb_synced; }
	void sync ( const int src_index, const bool b_force );

	void fastCopy ( const DBRecord* dbrec );

	inline void setCompleterUpdated ( const bool b_updated ) { mb_completerUpdated = b_updated; }
	inline bool completerUpdated () const { return mb_completerUpdated; }

	stringRecord toStringRecord () const;
	void fromStringRecord ( const stringRecord& str_rec, const uint fromField = 0 );

	inline const QString& actualRecordStr ( const uint field ) const {
		return m_RECFIELDS[field].str_field[RECORD_FIELD::IDX_ACTUAL]; }

	inline int actualRecordInt ( const uint field ) const {
		return m_RECFIELDS[field].i_field[RECORD_FIELD::IDX_ACTUAL]; }

	inline const QString& backupRecordStr ( const uint field ) const {
		return m_RECFIELDS[field].str_field[RECORD_FIELD::IDX_TEMP]; }

	inline int backupRecordInt ( const uint field ) const {
		return m_RECFIELDS[field].i_field[RECORD_FIELD::IDX_TEMP]; }

    inline void setListItem ( vmListItem* listitem ) { mListItem = listitem; }

protected:
	friend void updateSuppliesItemCompleter ( const DBRecord* db_rec );
	friend void updateInventoryItemCompleter ( const DBRecord* db_rec );

	void callHelperFunctions ();
	void copy ( const DBRecord& dbrec );

	/* Load with value = ACTION_READ */
	inline void setValue ( const uint field, const QString& value ) {
		m_RECFIELDS[field].str_field[RECORD_FIELD::IDX_ACTUAL] = value; }
	inline void setBackupValue ( const uint field, const QString& value ) {
		m_RECFIELDS[field].str_field[RECORD_FIELD::IDX_TEMP] = value; }
	inline void setIntValue ( const uint field, const int value ) {
		m_RECFIELDS[field].i_field[RECORD_FIELD::IDX_ACTUAL] = value; }
	inline void setIntBackupValue ( const uint field, const int value ) {
		m_RECFIELDS[field].i_field[RECORD_FIELD::IDX_TEMP] = value; }
	/* Load with value = ACTION_READ */

	/* Add value into new record = ACTION_ADD */
	inline void addValue ( const uint field, const QString& value )
	{
		m_RECFIELDS[field].str_field[RECORD_FIELD::IDX_ACTUAL] = value;
		m_RECFIELDS[field].modified = mb_modified = true;
	}

	inline void addIntValue ( const uint field, const int value )
	{
		m_RECFIELDS[field].i_field[RECORD_FIELD::IDX_ACTUAL] = value;
		m_RECFIELDS[field].modified = mb_modified = true;
	}
	/* Add value into new record = ACTION_ADD */

	/* Alter value if different = ACTION_EDIT */
	inline void editValue ( const uint field, const QString& value )
	{
		if ( value != actualRecordStr ( field ) ) {
			m_RECFIELDS[field].str_field[RECORD_FIELD::IDX_TEMP] = value;
			m_RECFIELDS[field].modified = mb_modified = true;
			mb_synced = false;
		}
	}

	inline void editIntValue ( const uint field, const int value )
	{
		if ( value != actualRecordInt ( field ) ) {
			m_RECFIELDS[field].i_field[RECORD_FIELD::IDX_TEMP] = value;
			m_RECFIELDS[field].modified = mb_modified = true;
			mb_synced = false;
		}
	}
	/* Alter value if different = ACTION_EDIT */

	const TABLE_INFO* t_info;
	RECORD_FIELD* m_RECFIELDS;
    vmListItem* mListItem;

	typedef void ( **HelperFunction ) ( const DBRecord* );
	HelperFunction helperFunction;

	uint fld_count;
	bool mb_modified;
	bool mb_synced;
	bool mb_completerUpdated;
	RECORD_ACTION m_action, m_prevaction;

	st_Query stquery;
	static PointersList<DBRecord*> tempNewRecs[TABLES_IN_DB];

	alterRecord fptr_change;
	alterRecordInt fptr_changeInt;
	recordValueStr fptr_recordStr;
	recordValueInt fptr_recordInt;
	recordValueStr fptr_recordStrAlternate;
};

#endif // DBRECORD_H
