#include "dbrecord.h"
#include "global.h"
#include "vivenciadb.h"

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>

static const int podBytes ( sizeof ( RECORD_FIELD::i_field ) + sizeof ( RECORD_FIELD::modified ) + sizeof ( RECORD_FIELD::was_modified ) );

static const TABLE_INFO generic_tinfo = {
	0,
	emptyString, emptyString, emptyString,
	emptyString, emptyString, emptyString,
	nullptr, 0.0, 0, 0, nullptr
	#ifdef TRANSITION_PERIOD
	, false
	#endif
};

static inline int recordFieldsCompare_pod ( const RECORD_FIELD& rec1, const RECORD_FIELD& rec2 )
{
	return ::memcmp ( &rec1, &rec2, podBytes );
}

DBRecord::DBRecord ( const uint field_count )
    : t_info ( nullptr ), m_RECFIELDS ( nullptr ) , mListItem ( nullptr ), helperFunction ( nullptr ),
	  fld_count ( field_count ), mb_modified ( false ), mb_synced ( true ),
	  mb_completerUpdated ( false ), m_action ( ACTION_NONE )
{
	setAction ( ACTION_READ );
}

DBRecord::DBRecord ( const DBRecord* other )
    : t_info ( nullptr ), m_RECFIELDS ( nullptr ), mListItem ( nullptr ),helperFunction ( nullptr ),
	  fld_count ( 0 ), mb_modified ( false ), mb_synced ( true ),
	  mb_completerUpdated ( false ), m_action ( ACTION_NONE )
{
	setAction ( ACTION_READ );
	if ( other != nullptr )
		copy ( other );
}

DBRecord::~DBRecord () {}

bool DBRecord::operator== ( const DBRecord& other ) const
{
	if ( other.t_info == this->t_info )
	{
		if ( other.fld_count == this->fld_count )
		{
			for ( uint i ( 0 ); i < fld_count; ++i )
			{
				if ( recordFieldsCompare_pod ( other.m_RECFIELDS[i], this->m_RECFIELDS[i] ) != 0 )
					return false;
				if ( QString::compare ( other.m_RECFIELDS[i].str_field[0], this->m_RECFIELDS[i].str_field[0] ) != 0 )
					return false;
				if ( QString::compare ( other.m_RECFIELDS[i].str_field[1], this->m_RECFIELDS[i].str_field[1] ) != 0 )
					return false;
			}
		}
	}
	return false;
}

const DBRecord& DBRecord::operator= ( const DBRecord& other )
{
	if ( *this != &other )
	{
		if ( t_info != other.t_info)
			helperFunction = nullptr;
		copy ( other );
	}
	return *this;
}

void DBRecord::copy ( const DBRecord& dbrec )
{
	t_info = dbrec.t_info;
	fld_count = dbrec.fld_count;
	mb_modified = dbrec.mb_modified;
	mb_synced = dbrec.mb_synced;
	mb_completerUpdated = dbrec.mb_completerUpdated;
	m_action = dbrec.m_action;

	for ( uint i ( 0 ); i < fld_count; ++i )
	{
		m_RECFIELDS[i].str_field[RECORD_FIELD::IDX_ACTUAL] = dbrec.m_RECFIELDS[i].str_field[RECORD_FIELD::IDX_ACTUAL];
		m_RECFIELDS[i].str_field[RECORD_FIELD::IDX_TEMP] = dbrec.m_RECFIELDS[i].str_field[RECORD_FIELD::IDX_TEMP];
		m_RECFIELDS[i].i_field[RECORD_FIELD::IDX_ACTUAL] = dbrec.m_RECFIELDS[i].i_field[RECORD_FIELD::IDX_ACTUAL];
		m_RECFIELDS[i].i_field[RECORD_FIELD::IDX_TEMP] = dbrec.m_RECFIELDS[i].i_field[RECORD_FIELD::IDX_TEMP];
		helperFunction[i] = dbrec.helperFunction[i];
		m_RECFIELDS[i].modified = dbrec.m_RECFIELDS[i].modified;
	}
}

void DBRecord::setHelperFunction ( const uint field, void ( *helperFunc ) ( const DBRecord* ) )
{
	if ( field >= 1 && field < fld_count )
		this->helperFunction[field] = helperFunc;
}

void DBRecord::callHelperFunctions ()
{
	if ( helperFunction != nullptr )
	{
		uint i ( 1 ); // any field but ID can have a helper function
		do
		{
			if ( isModified ( i ) || action () == ACTION_DEL )
			{
				if ( helperFunction[i] )
					( *helperFunction[i] ) ( this );
			}
		} while ( ++i < fld_count );
	}
}

/*const vmNumber& DBRecord::setPrice ( const uint field, const QString& price )
{
	rec_number.fromStrPrice ( price );
	setRecValue ( this, field, rec_number.toPrice () );
	return rec_number;
}*/

bool DBRecord::readRecord ( const int id, const bool load_data )
{
	if ( id >= 1 && id != actualRecordInt ( 0 ) )
	{
		setIntValue ( 0, id );
		setBackupValue ( 0, QString::number ( id ) );
		return VDB ()->getDBRecord ( this, 0, load_data );
	}
	return ( id >= 1 );
}

bool DBRecord::readRecord ( const uint field, const QString& search, const bool load_data )
{
	if ( search.isEmpty () )
		return false;

	if ( signed ( field ) != stquery.field || search != stquery.search )
	{
		stquery.reset = true;
		stquery.field = field;
		stquery.search = search;
	}
	else
		return true;

	return VDB ()->getDBRecord ( this, stquery, load_data );
}

void DBRecord::resetQuery ()
{
	stquery.reset = true;
	stquery.field = -1;
	if ( stquery.query )
		stquery.query->finish ();
}

bool DBRecord::readFirstRecord ( const bool load_data )
{
	return readRecord ( VDB ()->getLowestID ( t_info->table_order ), load_data );
}

bool DBRecord::readFirstRecord ( const int field, const QString& search, const bool load_data )
{
	stquery.forward = true;
	stquery.reset = true;
	stquery.field = field;
	stquery.search = search;
	return VDB ()->getDBRecord ( this, stquery, load_data );
}

bool DBRecord::readLastRecord ( const bool load_data )
{
	return readRecord ( VDB ()->getHighestID ( t_info->table_order ), load_data );
}

bool DBRecord::readLastRecord ( const int field, const QString& search, const bool load_data )
{
	stquery.forward = false;
	stquery.reset = true;
	stquery.field = field;
	stquery.search = search;
	return VDB ()->getDBRecord ( this, stquery, load_data );
}

// We browse by position, but the indexes might not be the same as the positions due to records removal
bool DBRecord::readNextRecord ( const bool follow_search, const bool load_data )
{
	if ( !follow_search )
	{
		const int last_id ( VDB ()->getHighestID ( t_info->table_order ) );
		if ( last_id >= 1 && actualRecordInt ( 0 ) < last_id )
		{
			do
			{
				if ( readRecord ( actualRecordInt ( 0 ) + 1, load_data ) )
					return true;
			} while ( actualRecordInt ( 0 ) <= last_id );
		}
	}
	else
	{
		stquery.forward = true;
		return VDB ()->getDBRecord ( this, stquery, load_data );
	}
	return false;
}

bool DBRecord::readPrevRecord ( const bool follow_search, const bool load_data )
{
	if ( !follow_search )
	{
		const int first_id ( VDB ()->getLowestID ( t_info->table_order ) );
		if ( first_id >= 0 && actualRecordInt ( 0 ) > 0 )
		{
			do
			{
				if ( readRecord ( actualRecordInt ( 0 ) - 1, load_data ) )
					return true;
			} while ( actualRecordInt ( 0 ) >= 0 );
		}
	}
	else
	{
		stquery.forward = false;
		return VDB ()->getDBRecord ( this, stquery, load_data );
	}
	return false;
}

/* MySQL removes record with ID=0 when the delete query contains the line WHERE ID=''.
 * When the record is new (ACTION_ADD) there is no actualRecordInt(0), but actualRecordStr(0)
 * is an empty string which will delete ID=0. So we must make sure we do not
 * arrive at VDB ()->removeRecord (), or there will be trouble
 */
bool DBRecord::deleteRecord ()
{
	if ( actualRecordInt ( 0 ) >= 1 )
	{
		setAction ( ACTION_DEL );
		callHelperFunctions ();
		if ( VDB ()->removeRecord ( this ) )
		{
			clearAll ();
			setAction ( ACTION_READ );
			return true;
		}
	}
	return false;
}

bool DBRecord::saveRecord ()
{
	bool ret ( false );
	if ( m_action == ACTION_ADD )
		ret = VDB ()->insertDBRecord ( this );
	else
		ret = VDB ()->updateRecord ( this );
	// yes, the completer is updated by now, but the flag indicates that, upon next change to the record,
	// the completer must be updated again. It is just faster and cleaner to do it here, just once, than
	// anywhere else. The mb_completerUpdated flag is just used to speedup execution by not doing the same thing over for
	// all the fields in the respective record that comprise the product´s completer for it
	if ( ret )
	{
		callHelperFunctions ();
		setAction ( ACTION_READ );
		setAllModified ( false );
		setCompleterUpdated ( false );
	}
	return ret;
}

int DBRecord::searchCategoryTranslate ( const SEARCH_CATEGORIES ) const
{
	return -1;
}

void DBRecord::setModified ( const uint field, const bool modified )
{
	( m_RECFIELDS+field )->modified = modified;
	if ( modified )
		mb_modified = true;
}

void DBRecord::setAllModified ( const bool modified )
{
	uint i ( 1 );
	do
	{
		m_RECFIELDS[i].was_modified = m_RECFIELDS[i].modified;
		m_RECFIELDS[i].modified = modified;
	} while ( ++i < fld_count );
	mb_modified = modified;
}

void DBRecord::clearAll ()
{
	uint i ( 0 );
	do
	{
		m_RECFIELDS[i].str_field[RECORD_FIELD::IDX_ACTUAL].clear ();
		m_RECFIELDS[i].i_field[RECORD_FIELD::IDX_ACTUAL] = 0;
		m_RECFIELDS[i].modified = false;
		m_RECFIELDS[i].was_modified = false;
	} while  ( ++i < fld_count );
	mb_modified = false;
	mb_synced = true;
}

void DBRecord::setAction ( const RECORD_ACTION action )
{
	if ( action != m_action )
	{
		m_prevaction = m_action;
		m_action = action;
		if ( m_action != ACTION_DEL )
		{
			switch ( action )
			{
				case ACTION_READ:
					// copy temp values into actual after a save operation. Canceled edits must not be synced
					if ( !inSync () )
						sync ( RECORD_FIELD::IDX_TEMP, false );
				case ACTION_REVERT:
                    *const_cast<RECORD_ACTION*>( &m_action ) = ACTION_READ;
                    mb_synced = true;
					fptr_change = &DBRecord::setValue;
					fptr_changeInt = &DBRecord::setIntValue;
					fptr_recordStr = &DBRecord::actualRecordStr;
					fptr_recordInt = &DBRecord::actualRecordInt;
					fptr_recordStrAlternate = &DBRecord::backupRecordStr;
				break;
				case ACTION_ADD:
					fptr_change = &DBRecord::addValue;
					fptr_changeInt = &DBRecord::addIntValue;
					fptr_recordStr = &DBRecord::actualRecordStr;
					fptr_recordInt = &DBRecord::actualRecordInt;
					fptr_recordStrAlternate = &DBRecord::backupRecordStr;
					DBRecord::createTemporaryRecord ( this );
				break;
				case ACTION_EDIT:
					sync ( RECORD_FIELD::IDX_ACTUAL, true );
					fptr_change = &DBRecord::editValue;
					fptr_changeInt = &DBRecord::editIntValue;
					fptr_recordStr = &DBRecord::backupRecordStr;
					fptr_recordInt = &DBRecord::backupRecordInt;
					fptr_recordStrAlternate = &DBRecord::actualRecordStr;
				break;
				default:
				break;
			}
		}
	}
}


void DBRecord::createTemporaryRecord ( DBRecord* dbrec )
{
	const uint table ( dbrec->t_info->table_order );
	const uint new_id ( VDB ()->getNextID ( table ) );
	dbrec->setIntValue ( 0, new_id ); // this is set so that VivenciaDB::insertDBREcord can use the already evaluated value
	dbrec->setIntBackupValue ( 0, new_id ); // this is set so that calls using recIntValue in a ACTION_ADD record will retrieve the correct value
	const QString str_id ( QString::number ( new_id ) );
	dbrec->setValue ( 0, str_id );
	dbrec->setBackupValue ( 0, str_id );
}

void DBRecord::sync ( const int src_index, const bool b_force )
{
	for ( uint i ( 1 ); i < fld_count; ++i )
	{
		if ( isModified ( i ) || b_force )
		{
            if ( m_RECFIELDS[i].str_field[!src_index] != m_RECFIELDS[i].str_field[src_index] )
			{
				m_RECFIELDS[i].str_field[!src_index] = m_RECFIELDS[i].str_field[src_index];
                m_RECFIELDS[i].i_field[!src_index] = m_RECFIELDS[i].i_field[src_index];
            }
		}
	}
	mb_synced = true;
}

void DBRecord::fastCopy ( const DBRecord* dbrec )
{
	if ( dbrec->typeID () == typeID () )
		*m_RECFIELDS = *( dbrec->m_RECFIELDS );
}

stringRecord DBRecord::toStringRecord () const
{
	stringRecord str_rec;
	for ( uint i ( 0 ); i < fld_count; ++i )
		str_rec.fastAppendValue ( recStrValue ( this, i ) );
	return str_rec;
}

void DBRecord::fromStringRecord ( const stringRecord& str_rec, const uint fromField )
{
	bool ok ( false );
	if ( fromField > 0 )
		ok = str_rec.moveToRec ( fromField );
	else
		ok = str_rec.first ();

	if ( ok )
	{
		setRecValue ( this, 0, str_rec.curValue () );
		setRecIntValue ( this, 0, recStrValue ( this, 0 ).toInt () );
		uint i ( 1 );
		do
		{
			if ( str_rec.next () )
				setRecValue ( this, i++, str_rec.curValue () );
			else
				break;
		} while ( true );
	}
}

void DBRecord::contains ( const QString& value, podList<uint>& fields ) const
{
	fields.clearButKeepMemory ();
	for ( uint i ( 0 ); i < t_info->field_count; ++i )
	{
		if ( recStrValue ( this, i ).contains ( value, Qt::CaseInsensitive ) )
			fields.append ( i );
	}
}
