#ifndef VMMACHINES_H
#define VMMACHINES_H

#include "dbrecord.h"

const uint MACHINES_FIELD_COUNT ( 9 );

class machinesRecord : public DBRecord
{

	friend class VivenciaDB;
	friend class machinesDlg;

public:
	explicit machinesRecord ();
	virtual ~machinesRecord ();

	static const TABLE_INFO t_info;

protected:
	friend bool updateMachinesTable ();

	RECORD_FIELD m_RECFIELDS[MACHINES_FIELD_COUNT];
	void ( *helperFunction[MACHINES_FIELD_COUNT] ) ( const DBRecord* );
};

#endif // VMMACHINES_H
