#ifndef INVENTORY_H
#define INVENTORY_H

#include "dbrecord.h"

const uint INVENTORY_FIELD_COUNT ( 10 );

class Inventory : public DBRecord
{

friend class VivenciaDB;

public:
	explicit Inventory ( const bool connect_helper_funcs = false );
	virtual ~Inventory ();

	uint isrRecordField ( const ITEMS_AND_SERVICE_RECORD isr_field ) const;
	inline QString isrValue ( const ITEMS_AND_SERVICE_RECORD isr_field, const int = -1 ) const
	{
		return recStrValue ( this, isrRecordField ( isr_field ) );
	}

	static const TABLE_INFO t_info;

protected:
	friend bool updateInvetory ();
	RECORD_FIELD m_RECFIELDS[INVENTORY_FIELD_COUNT];
	void ( *helperFunction[INVENTORY_FIELD_COUNT] ) ( const DBRecord* );
};

#endif // INVENTORY_H
