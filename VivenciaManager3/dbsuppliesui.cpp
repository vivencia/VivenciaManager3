#include "dbsuppliesui.h"
#include "global.h"
#include "dbsupplies.h"
#include "vmtablewidget.h"
#include "vivenciadb.h"
#include "completers.h"
#include "global.h"
#include "mainwindow.h"
#include "vmlistitem.h"
#include "searchui.h"
#include "cleanup.h"
#include "heapmanager.h"

#include <QPushButton>
#include <QVBoxLayout>

dbSuppliesUI* dbSuppliesUI::s_instance ( nullptr );

void deleteDBSuppliesInstance ()
{
	heap_del ( dbSuppliesUI::s_instance );
}

dbSuppliesUI::dbSuppliesUI ()
	: QObject ( nullptr ), m_table ( nullptr ),
	  supplies_rec ( new dbSupplies ( true ) ), btnSuppliesEditTable ( nullptr )
{
	addPostRoutine ( deleteDBSuppliesInstance );
}

dbSuppliesUI::~dbSuppliesUI ()
{
	heap_del ( m_table );
	heap_del ( supplies_rec );
}

bool dbSuppliesUI::setupUI ()
{
	if ( btnSuppliesEditTable )
		return false;
	
	btnSuppliesEditTable = new QPushButton ( tr ( "&Edit" ) );
	btnSuppliesEditTable->setCheckable ( true );
	btnSuppliesEditTable->connect ( btnSuppliesEditTable, &QPushButton::clicked,
			this, [&] ( const bool checked ) { return btnSuppliesEditTable_clicked ( checked ); } );

	btnSuppliesCancelEdit = new QPushButton ( tr ( "Cancel changes" ) );
	btnSuppliesCancelEdit->setEnabled ( false );
	btnSuppliesCancelEdit->connect ( btnSuppliesCancelEdit, &QPushButton::clicked,
			this, [&] ( const bool ) { return btnSuppliesCancelEdit_clicked (); } );

	btnSuppliesInsertRowAbove = new QPushButton ( tr ( "Add new item above" ) );
	btnSuppliesInsertRowAbove->setEnabled ( false );
	btnSuppliesInsertRowAbove->connect ( btnSuppliesInsertRowAbove, &QPushButton::clicked,
			this, [&] ( const bool ) { return insertRow ( m_table->currentRow () ); } );

	btnSuppliesInsertRowBelow = new QPushButton ( tr ( "Add new item below" ) );
	btnSuppliesInsertRowBelow->setEnabled ( false );
	btnSuppliesInsertRowBelow->connect ( btnSuppliesInsertRowBelow, &QPushButton::clicked,
			this, [&] ( const bool ) { return insertRow ( m_table->currentRow () + 1 ); } );

	btnSuppliesRemoveRow = new QPushButton ( tr ( "Remove item" ) );
	btnSuppliesRemoveRow->setEnabled ( false );
	btnSuppliesRemoveRow->connect ( btnSuppliesRemoveRow, &QPushButton::clicked,
			this, [&] ( const bool ) { if ( m_table->currentRow () >= 1 ) return rowRemoved ( m_table->currentRow () ); } );

	QFrame* vLine ( new QFrame );
	vLine->setFrameStyle ( QFrame::VLine|QFrame::Sunken );
	
	QHBoxLayout* lButtons ( new QHBoxLayout );
	lButtons->setSpacing ( 2 );
	lButtons->setMargin ( 2 );
	lButtons->addWidget ( btnSuppliesEditTable );
	lButtons->addWidget ( btnSuppliesCancelEdit );
	lButtons->addWidget ( vLine );
	lButtons->addWidget ( btnSuppliesInsertRowAbove );
	lButtons->addWidget ( btnSuppliesInsertRowBelow );
	lButtons->addWidget ( btnSuppliesRemoveRow );

	createTable ();

	suppliesLayout = new QVBoxLayout;
	m_table->addToLayout ( suppliesLayout );
	suppliesLayout->addLayout ( lButtons, 1 );

	/*if ( DATA ()->reads[TABLE_SUPPLIES_ORDER] ) {
		const bool can_write ( DATA ()->writes[TABLE_SUPPLIES_ORDER] );
		btnSuppliesCancelEdit->setEnabled ( can_write );
		btnSuppliesInsertRowAbove->setEnabled ( can_write );
		btnSuppliesInsertRowBelow->setEnabled ( can_write );
		btnSuppliesRemoveRow->setEnabled ( can_write );
	}
	else {
		btnsFrame->setEnabled ( false );
		m_table->setEnabled ( false );
	}*/

	return true;
}

void dbSuppliesUI::createTable ()
{
	if ( m_table != nullptr ) return;

	m_table = new vmTableWidget;
	vmTableColumn *fields ( m_table->createColumns( SUPPLIES_FIELD_COUNT ) );

	for ( uint i ( 0 ); i < SUPPLIES_FIELD_COUNT; ++i ) {
		fields[i].label = VivenciaDB::getTableColumnLabel ( &supplies_rec->t_info, i );

		switch ( i ) {
			case FLD_SUPPLIES_ID:
				fields[FLD_SUPPLIES_ID].editable = false;
				fields[FLD_SUPPLIES_ID].width = 40;
			break;
			case FLD_SUPPLIES_ITEM:
				fields[FLD_SUPPLIES_ITEM].completer_type = vmCompleters::ITEM_NAMES;
				fields[FLD_SUPPLIES_ITEM].width = 250;
			break;
			case FLD_SUPPLIES_BRAND:
				fields[FLD_SUPPLIES_BRAND].completer_type = vmCompleters::BRAND;
				fields[FLD_SUPPLIES_BRAND].text_type = vmWidget::TT_UPPERCASE;
				fields[FLD_SUPPLIES_BRAND].width = 120;
			break;
			case FLD_SUPPLIES_TYPE:
				fields[FLD_SUPPLIES_TYPE].completer_type = vmCompleters::STOCK_TYPE;
				fields[FLD_SUPPLIES_TYPE].width = 120;
			break;
			case FLD_SUPPLIES_SUPPLIER:
				fields[FLD_SUPPLIES_SUPPLIER].completer_type = vmCompleters::SUPPLIER;
				fields[FLD_SUPPLIES_SUPPLIER].width = 150;
			break;
			case FLD_SUPPLIES_UNIT:
				fields[FLD_SUPPLIES_UNIT].width = 40;
			break;
			case FLD_SUPPLIES_PRICE:
				fields[FLD_SUPPLIES_PRICE].text_type = vmLineEdit::TT_PRICE;
			break;
			case FLD_SUPPLIES_DATE_IN:
				fields[FLD_SUPPLIES_DATE_IN].wtype = WT_DATEEDIT;
			default:
			break;
		}
	}
	m_table->setKeepModificationRecords ( false );
	m_table->initTable ( 500 );

	// ignore FLD_SUPPLIES_PLACE and FLD_SUPPLIES_QUANTITY in the UI, but keep them in the base class so that VivenciaDB will have no problems distinguishing between the two classes
	m_table->setColumnHidden ( FLD_SUPPLIES_QUANTITY, true );
	m_table->setColumnHidden ( FLD_SUPPLIES_PLACE, true );

	m_table->setSelectionMode ( QAbstractItemView::SingleSelection );
	VDB ()->populateTable ( supplies_rec, m_table );

	m_table->setCallbackForCellChanged ( [&] ( const vmTableItem* const item ) {
				return tableChanged ( item ); } );
	m_table->setCallbackForCellNavigation ( [&] ( const uint row, const uint col, const uint prev_row, const uint ) {
				return readRowData ( row, col, prev_row ); } );
	m_table->setCallbackForRowRemoved ( [&] ( const uint row ) {
				return rowRemoved ( row ); } );
}

void dbSuppliesUI::readRowData ( const uint row, const uint, const uint prev_row, const uint )
{
	if ( prev_row != row ) {
		supplies_rec->clearAll ();
		if ( !m_table->sheetItem ( row, 0 )->text ().isEmpty () ) {
			supplies_rec->setAction ( ACTION_READ );
			for ( uint i_col ( 0 ) ; i_col < unsigned ( m_table->columnCount () ); ++i_col )
				setRecValue ( supplies_rec, i_col, m_table->sheetItem ( row, i_col )->text () );
			supplies_rec->setAction ( ACTION_EDIT );
		}
		else
			supplies_rec->setAction ( ACTION_ADD );
		supplies_rec->setIntBackupValue ( FLD_SUPPLIES_ITEM, row );
	}
}

void dbSuppliesUI::tableChanged ( const vmTableItem* const item )
{
	const bool bNewRecord ( m_table->sheetItem ( item->row (), FLD_SUPPLIES_ID )->text ().isEmpty () );
	supplies_rec->setAction ( bNewRecord ? ACTION_ADD : ACTION_EDIT );
	setRecValue ( supplies_rec, item->column (), item->text () );
	supplies_rec->saveRecord ();
	if ( bNewRecord )
		m_table->sheetItem ( item->row (), FLD_SUPPLIES_ID )->setText (
			recStrValue ( supplies_rec, FLD_SUPPLIES_ID ), false, false );
	m_table->setTableUpdated ();
}

void dbSuppliesUI::rowRemoved ( const uint row )
{
	if ( !m_table->sheetItem ( row, 0 )->text ().isEmpty () ) {
		supplies_rec->setAction ( ACTION_DEL );
		supplies_rec->deleteRecord (); // deletes from database before removing the row
	}
	m_table->removeRow ( row );
}

void dbSuppliesUI::insertRow ( const uint i_row )
{
	m_table->insertRow ( i_row );
	m_table->setCurrentCell ( i_row, m_table->currentColumn () );
}

void dbSuppliesUI::showSearchResult ( vmListItem* item, const bool bshow )
{
	if ( bshow ) {
		for ( uint i ( 0 ); i < SUPPLIES_FIELD_COUNT; ++i )
			m_table->setColumnSearchStatus ( i, item->searchFieldStatus ( i ) == SS_SEARCH_FOUND );
		if ( m_table->searchStart ( SEARCH_UI ()->searchTerm () ) ) {
			MAINWINDOW ()->showTab ( MainWindow::TI_SUPPLIES );
			m_table->searchFirst ();
		}
	}
	else
		m_table->searchCancel ();
}

void dbSuppliesUI::btnSuppliesEditTable_clicked ( const bool checked )
{
	m_table->setEditable ( checked );
	if ( checked ) {
		btnSuppliesEditTable->setText ( tr ( "Save" ) );
		m_table->setFocus ();
	}
	else
		btnSuppliesEditTable->setText ( tr ( "&Edit" ) );
	btnSuppliesInsertRowAbove->setEnabled ( checked );
	btnSuppliesInsertRowBelow->setEnabled ( checked );
	btnSuppliesCancelEdit->setEnabled ( checked );
	btnSuppliesRemoveRow->setEnabled ( checked );
}

void dbSuppliesUI::btnSuppliesCancelEdit_clicked ()
{
	if ( m_table->isEditable () ) {
		btnSuppliesEditTable->setChecked ( false );
		btnSuppliesEditTable_clicked ( false );
	}
}
