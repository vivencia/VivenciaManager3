#ifndef SUPPLIERRECORD_H
#define SUPPLIERRECORD_H

#include "dbrecord.h"

const uint SUPPLIER_FIELD_COUNT ( 8 );

class supplierRecord : public DBRecord
{

friend class VivenciaDB;
friend class suppliersDlg;

public:
	explicit supplierRecord ( const bool connect_helper_funcs = false );
	virtual ~supplierRecord ();

	static void insertIfSupplierInexistent ( const QString& supplier );

	static const TABLE_INFO t_info;

protected:
	friend bool updateSupplierTable ();

	RECORD_FIELD m_RECFIELDS[SUPPLIER_FIELD_COUNT];
	void ( *helperFunction[SUPPLIER_FIELD_COUNT] ) ( const DBRecord*);
};

#endif // SUPPLIERRECORD_H
