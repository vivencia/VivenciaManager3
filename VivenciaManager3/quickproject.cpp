#include "quickproject.h"
#include "vivenciadb.h"
#include "global.h"

const double TABLE_VERSION ( 2.2 );

static const uint QP_FIELDS_TYPE[QUICK_PROJECT_FIELD_COUNT] = {
	DBTYPE_ID, DBTYPE_SHORTTEXT, DBTYPE_LIST, DBTYPE_NUMBER, DBTYPE_PRICE,
	DBTYPE_PRICE, DBTYPE_NUMBER, DBTYPE_PRICE, DBTYPE_PRICE, DBTYPE_PRICE
};

bool updateQuickProjectTable ()
{
#ifdef TRANSITION_PERIOD
	static const QString oldColumnNames[7] { QStringLiteral ( "QUANTITY_SOLD" ),
				QStringLiteral ( "SELL_PRICE" ), QStringLiteral ( "SELL_TOTAL_PRICE" ),
				QStringLiteral ( "QUANTITY_BOUGHT" ), QStringLiteral ( "BOUGHT_PRICE" ),
				QStringLiteral ( "BOUGHT_TOTAL_PRICE" ), QStringLiteral ( "PROFIT" ) };
	(void) VDB ()->removeColumn ( QStringLiteral ( "N_ROWS" ), &quickProject::t_info );
	(void) VDB ()->renameColumn ( oldColumnNames[0], FLD_QP_SELL_QUANTITY, &quickProject::t_info );
	(void) VDB ()->renameColumn ( oldColumnNames[1], FLD_QP_SELL_UNIT_PRICE, &quickProject::t_info );
	(void) VDB ()->renameColumn ( oldColumnNames[2], FLD_QP_SELL_TOTALPRICE, &quickProject::t_info );
	(void) VDB ()->renameColumn ( oldColumnNames[3], FLD_QP_PURCHASE_QUANTITY, &quickProject::t_info );
	(void) VDB ()->renameColumn ( oldColumnNames[4], FLD_QP_PURCHASE_UNIT_PRICE, &quickProject::t_info );
	(void) VDB ()->renameColumn ( oldColumnNames[5], FLD_QP_PURCHASE_TOTAL_PRICE, &quickProject::t_info );
	(void) VDB ()->renameColumn ( oldColumnNames[6], FLD_QP_RESULT, &quickProject::t_info );
	return true;
	quickProject qp;
	if ( qp.readFirstRecord () ) {
		//Fix stringRecord format to use the new version used in VivenciaManager3
		QString field_value;
		do {
			qp.setAction ( ACTION_EDIT );
			for ( uint i ( FLD_QP_ITEM ); i <= FLD_QP_RESULT; ++i ) {
				field_value = recStrValue ( &qp, i );
				field_value.remove ( 0, 1 );
				setRecValue ( &qp, i, field_value );
			}
			qp.saveRecord ();
		} while ( qp.readNextRecord () );
	}
	return true;
#endif //TRANSITION_PERIOD
	return false;
}

const TABLE_INFO quickProject::t_info = {
	QUICK_PROJECT_TABLE,
	QStringLiteral ( "QUICK_PROJECT" ),
	QStringLiteral ( " ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci" ),
	QStringLiteral ( " PRIMARY KEY ( `ID` ), UNIQUE KEY `id` ( `ID` )" ),
	QStringLiteral ( "`ID`|`PROJECT_ID`|`ITEM`|`SELL_QUANTITY`|`SELL_UNIT_PRICE`|`SELL_TOTAL_PRICE`|"
	"`PURCHASE_QUANTITY`|`PURCHASE_UNIT_PRICE`|`PURCHASE_TOTAL_PRICE`|`RESULT`|" ),
	QStringLiteral ( " int ( 9 ) NOT NULL, | int ( 9 ) NOT NULL, | longtext COLLATE utf8_unicode_ci, | longtext COLLATE utf8_unicode_ci, |"
	" longtext COLLATE utf8_unicode_ci, | longtext COLLATE utf8_unicode_ci, | longtext COLLATE utf8_unicode_ci, | longtext COLLATE utf8_unicode_ci, |"
	" longtext COLLATE utf8_unicode_ci, | longtext COLLATE utf8_unicode_ci, |" ),
	QStringLiteral ( "ID|Project ID|Item|Quantity|Selling price (un)|Selling price (total)|Purchase quantity|Purchase price (un)|Purchase price (total)|Result|" ),
	QP_FIELDS_TYPE, TABLE_VERSION, QUICK_PROJECT_FIELD_COUNT, TABLE_QP_ORDER, &updateQuickProjectTable
	#ifdef TRANSITION_PERIOD
	, false
	#endif
};

quickProject::quickProject ()
	: DBRecord ( QUICK_PROJECT_FIELD_COUNT )
{
	::memset ( this->helperFunction, 0, sizeof ( this->helperFunction ) );
	DBRecord::t_info = & ( quickProject::t_info );
	DBRecord::m_RECFIELDS = this->m_RECFIELDS;
	DBRecord::helperFunction = this->helperFunction;
}

quickProject::~quickProject () {}
