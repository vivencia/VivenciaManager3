#include "inventory.h"
#include "completers.h"
#include "supplierrecord.h"
#include "vivenciadb.h"

#include <QCoreApplication>

static const unsigned int TABLE_VERSION ( 'A' );

const uint INVENTORY_FIELDS_TYPE[INVENTORY_FIELD_COUNT] =
{
	DBTYPE_ID, DBTYPE_LIST, DBTYPE_LIST, DBTYPE_NUMBER, DBTYPE_SHORTTEXT, DBTYPE_LIST, DBTYPE_DATE, DBTYPE_PRICE, 
	DBTYPE_LIST, DBTYPE_SHORTTEXT
};

bool updateInvetory ()
{
#ifdef TABLE_UPDATE_AVAILABLE
	VDB ()->optimizeTable ( &Inventory::t_info );
	return true;
#else
	VDB ()->optimizeTable ( &Inventory::t_info );
	return false;
#endif //TABLE_UPDATE_AVAILABLE
}

const TABLE_INFO Inventory::t_info =
{
	INVENTORY_TABLE,
	QStringLiteral ( "INVENTORY" ),
	QStringLiteral ( " ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci" ),
	QStringLiteral ( " PRIMARY KEY ( `ID` ) , UNIQUE KEY `id` ( `ID` ) " ),
	QStringLiteral ( "`ID`|`ITEM`|`BRAND`|`QUANTITY`|`UNIT`|`TYPE`|`DATE_IN`|`PRICE`|`SUPPLIER`|`PLACE`|" ),
	QStringLiteral ( " int ( 9 ) NOT NULL, | varchar ( 100 ) DEFAULT NULL, | varchar ( 60 ) DEFAULT NULL, | varchar ( 8 ) DEFAULT NULL, |"
	" varchar ( 8 ) DEFAULT NULL, | varchar ( 80 ) DEFAULT NULL, | varchar ( 20 ) DEFAULT NULL, |"
	" varchar ( 20 ) DEFAULT NULL, | varchar ( 50 ) DEFAULT NULL, | varchar ( 30 ) DEFAULT NULL, |" ),
	QCoreApplication::tr ( "ID|Item name|Brand|Quantity|Unity|Type/Category|Income date|Purchase price|Supplier|Place|" ),
	INVENTORY_FIELDS_TYPE, TABLE_VERSION, INVENTORY_FIELD_COUNT, TABLE_INVENTORY_ORDER, &updateInvetory
	#ifdef TRANSITION_PERIOD
	, false
	#endif
};

void updateInventoryISRUnitCompleter ( const DBRecord* db_rec )
{
	APP_COMPLETERS ()->encodeCompleterISRForSpreadSheet ( db_rec );
}

void updateInventoryISRPriceCompleter ( const DBRecord* db_rec )
{
	APP_COMPLETERS ()->encodeCompleterISRForSpreadSheet ( db_rec );
}

void updateInventoryItemCompleter ( const DBRecord* db_rec )
{
	APP_COMPLETERS ()->updateCompleter ( recStrValue ( db_rec, FLD_INVENTORY_ITEM ), vmCompleters::ITEM_NAMES );
	APP_COMPLETERS ()->encodeCompleterISRForSpreadSheet ( db_rec );
}

void updateInventoryBrandCompleter ( const DBRecord* db_rec )
{
	APP_COMPLETERS ()->updateCompleter ( recStrValue ( db_rec, FLD_INVENTORY_BRAND ), vmCompleters::BRAND );
	APP_COMPLETERS ()->encodeCompleterISRForSpreadSheet ( db_rec );
}

void updateInventoryTypeCompleter ( const DBRecord* db_rec )
{
	APP_COMPLETERS ()->updateCompleter ( recStrValue ( db_rec, FLD_INVENTORY_TYPE ), vmCompleters::STOCK_TYPE );
}

void updateSupplierTable_inventory ( const DBRecord* db_rec )
{
	supplierRecord::insertIfSupplierInexistent ( recStrValue ( db_rec, FLD_INVENTORY_SUPPLIER ) );
	APP_COMPLETERS ()->encodeCompleterISRForSpreadSheet ( db_rec );
}

void updateInventoryPlaceCompleter ( const DBRecord* db_rec )
{
	APP_COMPLETERS ()->updateCompleter ( recStrValue ( db_rec, FLD_INVENTORY_PLACE ), vmCompleters::STOCK_PLACE );
}

Inventory::Inventory ( const bool connect_helper_funcs )
	: DBRecord ( INVENTORY_FIELD_COUNT )
{
	::memset ( this->helperFunction, 0, sizeof ( this->helperFunction ) );
	DBRecord::t_info = & ( Inventory::t_info );
	DBRecord::m_RECFIELDS = this->m_RECFIELDS;

	if ( connect_helper_funcs )
	{
		DBRecord::helperFunction = this->helperFunction;
		setHelperFunction ( FLD_INVENTORY_ITEM, &updateInventoryItemCompleter );
		setHelperFunction ( FLD_INVENTORY_BRAND, &updateInventoryBrandCompleter );
		setHelperFunction ( FLD_INVENTORY_TYPE, &updateInventoryTypeCompleter );
		setHelperFunction ( FLD_INVENTORY_SUPPLIER, &updateSupplierTable_inventory );
		setHelperFunction ( FLD_INVENTORY_PLACE, &updateInventoryPlaceCompleter );
		setHelperFunction ( FLD_INVENTORY_UNIT, &updateInventoryISRUnitCompleter );
		setHelperFunction ( FLD_INVENTORY_PRICE, &updateInventoryISRPriceCompleter );
	}
}

Inventory::~Inventory () {}

uint Inventory::isrRecordField ( const ITEMS_AND_SERVICE_RECORD isr_field ) const
{
	uint rec_field ( 0 );
	switch ( isr_field )
	{
		case ISR_NAME:			rec_field = FLD_INVENTORY_ITEM;		break;
		case ISR_UNIT:			rec_field = FLD_INVENTORY_UNIT;		break;
		case ISR_BRAND:			rec_field = FLD_INVENTORY_BRAND;	break;
		case ISR_UNIT_PRICE:
		case ISR_TOTAL_PRICE:	rec_field = FLD_INVENTORY_PRICE;	break;
		case ISR_QUANTITY:		rec_field = FLD_INVENTORY_QUANTITY;	break;
		case ISR_SUPPLIER:		rec_field = FLD_INVENTORY_SUPPLIER;	break;
		case ISR_ID:			rec_field = FLD_INVENTORY_ID;		break;
		case ISR_OWNER:			rec_field = FLD_INVENTORY_TYPE;		break; // not any other field appropriate; but will not be used anyway
		case ISR_DATE:			rec_field = FLD_INVENTORY_DATE_IN;	break;
	}
	return rec_field;
}
