#ifndef STRINGRECORD_H
#define STRINGRECORD_H

#include "tristatetype.h"
#include "vmlist.h"

#include <QString>
#include <QChar>

static const QLatin1Char public_sep ( 31 );
static const QChar record_sep ( 30 );
static const QChar table_sep  ( 29 );

//------------------------------------------------------RECORD-----------------------------------------------
class stringRecord
{

public:
	inline explicit stringRecord () : mFields ( 0 ), mFastIdx ( -1 ), mState ( TRI_OFF ) {}
	inline stringRecord ( const stringRecord &other )
		: mData ( other.mData ), mFields ( other.mFields ), mFastIdx ( other.mFastIdx ), mState ( other.mState ) {}
	inline stringRecord ( const QString& str ) {
		fromString ( str );
	}

	inline static uint countFields ( const QString& str ) {
		return str.count ( record_sep );
	}

	bool isOK () const;
	void fromString ( const QString& str );
	inline uint nFields () const {
		return mFields;
	}
	void clear ();
	void insertField ( const int field, const QString& value = QString::null );
	void changeValue ( const int field, const QString& value );
	void removeField ( const int field );
	bool removeFieldByValue ( const QString& value, const bool b_allmatches = false );
	void insertStrRecord ( const int field, const QString& inserting_rec );
	inline void insertStrRecord ( const int field, const stringRecord& inserting_rec ) {
		insertStrRecord ( field, inserting_rec.toString () );
	}
	void appendStrRecord ( const QString& inserting_rec );
	inline void appendStrRecord ( const stringRecord& inserting_rec ) {
		appendStrRecord ( inserting_rec.toString () );
	}

	const QString section ( const uint start_field, int end_field = -1 ) const;
	const QString fieldValue ( int field ) const;
	void fastAppendValue ( const QString& value );
	int field ( const QString& value, const int init_idx = -1, const bool bprecise = true ) const;
	inline const QString& toString () const {
		return mData;
	}

	static const QString fieldValue ( const QString& str_record, const uint field );

	/* Do not rely on curValue () alone after calling any of these functions, rather rely on
	 * their return value. In order to speed things up I will not clear mCurValue if a result is
	 * not found so it will remain with its last valid value. Also, do not call next () and then prev ()
	 * or vice-versa, for the index pointer will be swapped when changing directions. Actually, only after the
	 * second call will it be at the right position, i.e. next () -> prev () -> prev () will yield the expected result
	 */
	bool first () const;
	bool next () const;
	bool prev () const;
	bool last () const;
	bool moveToRec ( const uint rec ) const;

	inline const QString& curValue () const {
		return mCurValue;
	}

#define addToStringRecord(strRec,value) strRec + value + record_sep

	// a Null stringRecord is not to be used, unless when returning an invalid result. This is so the caller
	// knows that it should not use that returned object, and discard it
	struct Null {};
	static constexpr Null null = {};
	inline stringRecord ( const Null& ) : mFields ( -1 ), mFastIdx ( -1 ), mState ( TRI_UNDEF ) {}
	inline void setNull () {
		mState.setUndefined ();
	}
	inline bool isNull () const {
		return mState.isUndefined ();
	}

private:
	QString mData;
	mutable QString mCurValue;
	uint mFields;
	int mFastIdx;
	triStateType mState;
};

static const stringRecord emptyStrRecord;
//------------------------------------------------------RECORD-----------------------------------------------

//------------------------------------------------------TABLE------------------------------------------------
class stringTable
{

public:
	inline explicit stringTable () : nRecords ( 0 ), mFastIdx ( -1 ) {}
	inline stringTable ( const stringTable& other )
		: mRecords ( other.mRecords ), nRecords ( other.nRecords ), mFastIdx ( other.mFastIdx ) {}
	inline stringTable ( const QString& str ) {
		fromString ( str );
	}

	bool isOK () const;
	void fromString ( const QString& str );
	inline uint countRecords () const {
		return nRecords;
	}
	static inline uint countRecords ( const QString& str_table ) {
		return str_table.count ( table_sep );
	}

	void clear ();

	void insertRecord ( const uint row, const QString& record );
	inline void insertRecord ( const uint row, const stringRecord& record ) {
		insertRecord ( row, record.toString () );
	}
	void changeRecord ( const uint row, const QString& record );
	inline void changeRecord ( const uint row, const stringRecord& record ) {
		changeRecord ( row, record.toString () );
	}
	void changeRecord ( const uint row, const uint field, const QString& new_value );
	bool removeRecordByValue ( const QString& record, const bool b_allmatches = false );
	inline bool removeRecordByValue ( const stringRecord& record, const bool b_allmatches = false ) {
		return removeRecordByValue ( record.toString (), b_allmatches );
	}
	void removeRecord ( const uint row );

	const stringRecord& readRecord ( int row ) const;
	int findRecordRowByFieldValue ( const QString& value, const uint field, const uint nth_occurrence = 0 ) const;
	int findRecordRowThatContainsWord ( const QString& word, podList<uint>* dayAndFieldList = nullptr, const int field = -1, const uint nth_occurrence = 0 ) const;
	void fastAppendRecord ( const QString& record );
	inline void fastAppendRecord ( const stringRecord& record ) {
		fastAppendRecord ( record.toString () );
	}
	void appendTable ( const QString& inserting_table );

	inline const QString& toString () const {
		return mRecords;
	}

	const stringRecord& first () const;
	const stringRecord& next () const;
	const stringRecord& prev () const;
	const stringRecord& last () const;

	bool firstStr () const;
	bool nextStr () const;
	bool prevStr () const;
	bool lastStr () const;

	inline const QString& curRecord () const {
		return mCurRecord;
	}

private:
	QString mRecords;
	mutable stringRecord mRecord;
	mutable QString mCurRecord;
	uint nRecords;
	int mFastIdx;
};
//------------------------------------------------------TABLE------------------------------------------------

#endif // STRINGRECORD_H
