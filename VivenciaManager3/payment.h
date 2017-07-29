#ifndef PAYMENT_H
#define PAYMENT_H

#include "dbrecord.h"

class payListItem;

const uint PAY_FIELD_COUNT ( 10 );
const uint PAY_INFO_FIELD_COUNT ( 6 );

class Payment : public DBRecord
{

friend class MainWindow;
friend class VivenciaDB;

friend void updateCalendarPayInfo ( const DBRecord* db_rec );
friend void updateOverdueInfo ( const DBRecord* db_rec );

public:

	explicit Payment ( const bool connect_helper_funcs = false );
	virtual ~Payment ();
	
	int searchCategoryTranslate ( const SEARCH_CATEGORIES sc ) const;
	void copySubRecord ( const uint subrec_field, const stringRecord& subrec );

	void setListItem ( payListItem* pay_item );
	payListItem* payItem () const;

	static const TABLE_INFO t_info;

protected:
	friend bool updatePaymentTable ();
	void ( *helperFunction[PAY_FIELD_COUNT] ) ( const DBRecord* );

	RECORD_FIELD m_RECFIELDS[PAY_FIELD_COUNT];
};

#endif // PAYMENT_H
