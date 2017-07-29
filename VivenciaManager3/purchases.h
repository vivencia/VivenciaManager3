#ifndef PURCHASES_H
#define PURCHASES_H

#include "dbrecord.h"

class buyListItem;

const uint BUY_FIELD_COUNT ( 13 );

class Buy : public DBRecord
{

friend class VivenciaDB;
friend class MainWindow;

friend void updateCalendarBuyInfo ( const DBRecord* db_rec );

public:

	explicit Buy ( const bool connect_helper_funcs = false );
	virtual ~Buy ();

	uint isrRecordField ( const ITEMS_AND_SERVICE_RECORD ) const;
	QString isrValue ( const ITEMS_AND_SERVICE_RECORD isr_field, const int sub_record = -1 ) const;
	int searchCategoryTranslate ( const SEARCH_CATEGORIES sc ) const;
	void copySubRecord ( const uint subrec_field, const stringRecord& subrec );

	void setListItem ( buyListItem* buy_item );
	buyListItem* buyItem () const;

	static const TABLE_INFO t_info;

protected:
	friend bool updatePurchaseTable ();
	void ( *helperFunction[BUY_FIELD_COUNT] ) ( const DBRecord* );

	RECORD_FIELD m_RECFIELDS[BUY_FIELD_COUNT];
};

#endif // PURCHASES_H
