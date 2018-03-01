#ifndef DBSUPPLIES_H
#define DBSUPPLIES_H

#include "dbrecord.h"

const uint SUPPLIES_FIELD_COUNT ( 10 );

class dbSupplies : public DBRecord
{

friend class VivenciaDB;
friend class dbSuppliesUI;

public:
	explicit dbSupplies ( const bool connect_helper_funcs = false );
	virtual ~dbSupplies ();

	uint isrRecordField ( const ITEMS_AND_SERVICE_RECORD ) const;
	inline QString isrValue ( const ITEMS_AND_SERVICE_RECORD isr_field, const int = -1 ) const
	{
		return recStrValue ( this, isrRecordField ( isr_field ) );
	}

	static const TABLE_INFO t_info;

	static void notifyDBChange ( const uint id );
	
protected:
	friend bool updateSuppliesTable ();

	RECORD_FIELD m_RECFIELDS[SUPPLIES_FIELD_COUNT];
	void ( *helperFunction[SUPPLIES_FIELD_COUNT] ) ( const DBRecord* );
};

#endif // DBSUPPLIES_H
