#include "dbsupplies.h"
#include "global_enums.h"
#include "completers.h"
#include "supplierrecord.h"

#include <QCoreApplication>

const double TABLE_VERSION ( 2.5 );

const uint SUPPLIES_FIELDS_TYPE[SUPPLIES_FIELD_COUNT] = {
	DBTYPE_ID, DBTYPE_LIST, DBTYPE_LIST, DBTYPE_NUMBER, DBTYPE_SHORTTEXT,
	DBTYPE_LIST, DBTYPE_DATE, DBTYPE_PRICE, DBTYPE_LIST,
	DBTYPE_SHORTTEXT
};

#ifdef TRANSITION_PERIOD
#include "vivenciadb.h"
#include "vmnumberformats.h"
#endif

bool updateSuppliesTable ()
{
#ifdef TRANSITION_PERIOD
	QString cmd;
	vmNumber t_version ( dbSupplies::t_info.version );
	if ( t_version == vmNumber ( 2.5 ) ) {
		cmd = QStringLiteral ( "ALTER TABLE STOCK DROP DATE_OUT" );
		VDB ()->database ()->exec ( cmd );

		cmd = QStringLiteral ( "ALTER TABLE STOCK DROP DATE_BUY" );
		VDB ()->database ()->exec ( cmd );

		cmd = QStringLiteral ( "RENAME TABLE STOCK TO SUPPLIES" );
		VDB ()->database ()->exec ( cmd );
	}
	return true;
#else
	return false;
#endif
}

void updateSuppliesISRUnitCompleter ( const DBRecord* db_rec )
{
	APP_COMPLETERS ()->encodeCompleterISRForSpreadSheet ( db_rec );
}

void updateSuppliesISRPriceCompleter ( const DBRecord* db_rec )
{
	APP_COMPLETERS ()->encodeCompleterISRForSpreadSheet ( db_rec );
}

void updateSuppliesItemCompleter ( const DBRecord* db_rec )
{
	APP_COMPLETERS ()->updateCompleter ( recStrValue ( db_rec, FLD_SUPPLIES_ITEM ), vmCompleters::ITEM_NAMES );
	APP_COMPLETERS ()->encodeCompleterISRForSpreadSheet ( db_rec );
}

void updateSuppliesTypeCompleter ( const DBRecord* db_rec )
{
	APP_COMPLETERS ()->updateCompleter ( recStrValue ( db_rec, FLD_SUPPLIES_TYPE ), vmCompleters::STOCK_TYPE );
}

void updateSuppliesSupplierCompleter ( const DBRecord* db_rec )
{
	supplierRecord::insertIfSupplierInexistent ( recStrValue ( db_rec, FLD_SUPPLIES_SUPPLIER ) );
	APP_COMPLETERS ()->encodeCompleterISRForSpreadSheet ( db_rec );
}

void updateSuppliesPlaceCompleter ( const DBRecord* db_rec )
{
	APP_COMPLETERS ()->updateCompleter ( recStrValue ( db_rec, FLD_SUPPLIES_PLACE ), vmCompleters::STOCK_PLACE );
}

void updateSuppliesBrandCompleter ( const DBRecord* db_rec )
{
	APP_COMPLETERS ()->updateCompleter ( recStrValue ( db_rec, FLD_SUPPLIES_BRAND ), vmCompleters::BRAND );
	APP_COMPLETERS ()->encodeCompleterISRForSpreadSheet ( db_rec );
}

const TABLE_INFO dbSupplies::t_info = {
	SUPPLIES_TABLE,
	QStringLiteral ( "SUPPLIES" ),
	QStringLiteral ( " ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci" ),
	QStringLiteral ( " PRIMARY KEY ( `ID` ) , UNIQUE KEY `id` ( `ID` ) " ),
	QStringLiteral ( "`ID`|`ITEM`|`BRAND`|`QUANTITY`|`UNIT`|`TYPE`|`DATE_IN`|`PRICE`|`SUPPLIER`|`PLACE`|" ),
	QStringLiteral ( " int ( 9 ) NOT NULL, | varchar ( 100 ) DEFAULT NULL, | varchar ( 60 ) DEFAULT NULL, | varchar ( 8 ) DEFAULT NULL, |"
	" varchar ( 8 ) DEFAULT NULL, | varchar ( 80 ) DEFAULT NULL, | varchar ( 20 ) DEFAULT NULL, |"
	" varchar ( 20 ) DEFAULT NULL, | varchar ( 50 ) DEFAULT NULL, | varchar ( 30 ) DEFAULT NULL, |" ),
	QCoreApplication::tr ( "ID|Item name|Brand|Stock quantity|Unity|Type/Category|Income date|Purchase price|Supplier|Place|" ),
	SUPPLIES_FIELDS_TYPE, TABLE_VERSION, SUPPLIES_FIELD_COUNT, TABLE_SUPPLIES_ORDER, &updateSuppliesTable
	#ifdef TRANSITION_PERIOD
	, false
	#endif
};

dbSupplies::dbSupplies ( const bool connect_helper_funcs )
	: DBRecord ( SUPPLIES_FIELD_COUNT )
{
	::memset ( this->helperFunction, 0, sizeof ( this->helperFunction ) );
	DBRecord::t_info = &( dbSupplies::t_info );
	DBRecord::m_RECFIELDS = this->m_RECFIELDS;

	if ( connect_helper_funcs )
	{
		DBRecord::helperFunction = this->helperFunction;
		setHelperFunction ( FLD_SUPPLIES_ITEM, &updateSuppliesItemCompleter );
		setHelperFunction ( FLD_SUPPLIES_TYPE, &updateSuppliesTypeCompleter );
		setHelperFunction ( FLD_SUPPLIES_SUPPLIER, &updateSuppliesSupplierCompleter );
		setHelperFunction ( FLD_SUPPLIES_BRAND, &updateSuppliesBrandCompleter );
		setHelperFunction ( FLD_SUPPLIES_PLACE, &updateSuppliesPlaceCompleter );
		setHelperFunction ( FLD_SUPPLIES_UNIT, &updateSuppliesISRUnitCompleter );
		setHelperFunction ( FLD_SUPPLIES_PRICE, &updateSuppliesISRPriceCompleter );
	}
}

dbSupplies::~dbSupplies () {}

uint dbSupplies::isrRecordField ( const ITEMS_AND_SERVICE_RECORD isr_field ) const
{
	uint rec_field ( 0 );
	switch ( isr_field )
	{
		case ISR_NAME:
			rec_field = FLD_SUPPLIES_ITEM;
		break;
		case ISR_UNIT:
			rec_field = FLD_SUPPLIES_UNIT;
		break;
		case ISR_BRAND:
			rec_field = FLD_SUPPLIES_BRAND;
		break;
		case ISR_UNIT_PRICE:
		case ISR_TOTAL_PRICE:
			rec_field = FLD_SUPPLIES_PRICE;
		break;
		case ISR_QUANTITY:
			rec_field = FLD_SUPPLIES_QUANTITY;
		break;
		case ISR_SUPPLIER:
			rec_field = FLD_SUPPLIES_SUPPLIER;
		break;
		case ISR_ID:
			rec_field = FLD_SUPPLIES_ID;
		break;
		case ISR_OWNER:
			rec_field = FLD_SUPPLIES_TYPE;
		break; // not any other field appropriate; but will not be used anyway
		case ISR_DATE:
			rec_field = FLD_SUPPLIES_DATE_IN;
		break;
	}
	return rec_field;
}
