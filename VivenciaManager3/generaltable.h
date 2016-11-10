#ifndef GENERALTABLE_H
#define GENERALTABLE_H

#include "dbrecord.h"

const uint GENERAL_FIELD_COUNT ( 7 );

extern void updateIDs ( const TABLE_ORDER table , podList<int>* new_ids = nullptr );

class generalTable : public DBRecord
{

friend class VivenciaDB;
friend class Data;

public:
	generalTable ();
	virtual ~generalTable ();

	void insertOrUpdate ( const TABLE_INFO* const t_info );
	
protected:
	friend bool updateGeneralTable ();
	static const TABLE_INFO t_info;

	RECORD_FIELD m_RECFIELDS[GENERAL_FIELD_COUNT];
	void ( *helperFunction[GENERAL_FIELD_COUNT] ) ( const DBRecord* );
};

#endif // GENERALTABLE_H
