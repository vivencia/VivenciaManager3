#ifndef COMPANYPURCHASES_H
#define COMPANYPURCHASES_H

#include "dbrecord.h"

const uint COMPANY_PURCHASES_FIELD_COUNT ( 10 );

class companyPurchases : public DBRecord
{

friend class VivenciaDB;
friend class Data;
friend class companyPurchasesUI;

public:
	explicit companyPurchases ( const bool connect_helper_funcs = false );
	virtual ~companyPurchases ();

private:
	static const TABLE_INFO t_info;

	friend bool updateCPTable ();

	RECORD_FIELD m_RECFIELDS[COMPANY_PURCHASES_FIELD_COUNT];
	void ( *helperFunction[COMPANY_PURCHASES_FIELD_COUNT] ) ( const DBRecord* );
};

#endif // COMPANYPURCHASES_H
