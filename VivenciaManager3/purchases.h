#ifndef PURCHASES_H
#define PURCHASES_H

#include "dbrecord.h"

class buyListItem;

const uint BUY_FIELD_COUNT ( 13 );

const uint BUYS_FIELDS_TYPE[BUY_FIELD_COUNT] = {
	DBTYPE_ID, DBTYPE_ID, DBTYPE_ID, DBTYPE_DATE, DBTYPE_DATE, DBTYPE_PRICE,
	DBTYPE_PRICE, DBTYPE_NUMBER, DBTYPE_LIST, DBTYPE_LIST, DBTYPE_SHORTTEXT,
	DBTYPE_SUBRECORD, DBTYPE_SUBRECORD
};

class Buy : public DBRecord
{

friend class VivenciaDB;
friend class MainWindow;

friend void updateCalendarBuyInfo ( const DBRecord* db_rec );

public:

	explicit Buy ( const bool connect_helper_funcs = false );
	virtual ~Buy ();

	inline DB_FIELD_TYPE fieldType ( const uint field ) const {
		return static_cast<DB_FIELD_TYPE> ( BUYS_FIELDS_TYPE[field] );
	}
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
