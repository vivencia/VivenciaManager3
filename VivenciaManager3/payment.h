#ifndef PAYMENT_H
#define PAYMENT_H

#include "dbrecord.h"

struct CALENDAR_EXCHANGE;

const uint PAY_FIELD_COUNT ( 10 );
const uint PAY_INFO_FIELD_COUNT ( 6 );

const uint PAYS_FIELDS_TYPE[PAY_FIELD_COUNT] = {
	DBTYPE_ID, DBTYPE_ID, DBTYPE_ID, DBTYPE_PRICE, DBTYPE_NUMBER, DBTYPE_PRICE,
	DBTYPE_SHORTTEXT, DBTYPE_SUBRECORD, DBTYPE_YESNO, DBTYPE_PRICE
};

class Payment : public DBRecord
{

friend class MainWindow;
friend class VivenciaDB;

friend void updateCalendarPayInfo ( const DBRecord* db_rec );
friend void updateOverdueInfo ( const DBRecord* db_rec );

public:

	explicit Payment ( const bool connect_helper_funcs = false );
	virtual ~Payment ();
	inline DB_FIELD_TYPE fieldType ( const uint field ) const {
		return static_cast<DB_FIELD_TYPE> ( PAYS_FIELDS_TYPE[field] );
	}
	int searchCategoryTranslate ( const SEARCH_CATEGORIES sc ) const;
	void updateCalendarPayInfo ();

	static const TABLE_INFO t_info;

protected:
	friend bool updatePaymentTable ();
	void ( *helperFunction[PAY_FIELD_COUNT] ) ( const DBRecord* );

	RECORD_FIELD m_RECFIELDS[PAY_FIELD_COUNT];
	PointersList<CALENDAR_EXCHANGE*> ce_list;
};

#endif // PAYMENT_H
