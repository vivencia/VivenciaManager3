#ifndef QUICKPROJECT_H
#define QUICKPROJECT_H

#include "dbrecord.h"

const uint QUICK_PROJECT_FIELD_COUNT ( 10 );

class quickProject : public DBRecord
{

friend class quickProjectUI;
friend class VivenciaDB;

public:
	quickProject ();
	virtual ~quickProject ();

private:
	static const TABLE_INFO t_info;

	RECORD_FIELD m_RECFIELDS[QUICK_PROJECT_FIELD_COUNT];
	void ( *helperFunction[QUICK_PROJECT_FIELD_COUNT] ) ( const DBRecord* );
	friend bool updateQuickProjectTable ();
};

#endif // QUICKPROJECT_H
