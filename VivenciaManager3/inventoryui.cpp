#include "inventoryui.h"
#include "global.h"
#include "inventory.h"
#include "vmtablewidget.h"
#include "vivenciadb.h"
#include "completers.h"
#include "data.h"
#include "mainwindow.h"
#include "cleanup.h"
#include "heapmanager.h"

#include <QApplication>
#include <QPushButton>
#include <QVBoxLayout>

InventoryUI* InventoryUI::s_instance ( nullptr );

void deleteInventoryInstance ()
{
	heap_del ( InventoryUI::s_instance );
}

InventoryUI::InventoryUI ()
    : QObject ( nullptr ), inventory_rec ( new Inventory ( true ) ),
	  m_table ( nullptr ), btnInventoryEditTable ( nullptr )
{
	addPostRoutine ( deleteInventoryInstance );
}

InventoryUI::~InventoryUI ()
{
	heap_del ( m_table );
	heap_del ( inventory_rec );
}

bool InventoryUI::setupUI ()
{
	if ( btnInventoryEditTable != nullptr )
		return false;

	btnInventoryEditTable = new QPushButton ( tr ( "&Edit" ) );
	btnInventoryEditTable->setCheckable ( true );
	btnInventoryEditTable->connect ( btnInventoryEditTable, &QPushButton::clicked,
	this, [&] ( const bool checked ) {
		return btnInventoryEditTable_clicked ( checked );
	} );

	btnInventoryCancelEdit = new QPushButton ( tr ( "Cancel changes" ) );;
	btnInventoryCancelEdit->connect ( btnInventoryCancelEdit, &QPushButton::clicked,
	this, [&] ( const bool ) {
		return btnInventoryCancelEdit_clicked (); } );

	btnInventoryInsertRowAbove = new QPushButton ( tr ( "Add new item above" ) );
	btnInventoryInsertRowAbove->connect ( btnInventoryInsertRowAbove, &QPushButton::clicked,
	this, [&] ( const bool ) {
		return btnInventoryInsertRowAbove_clicked (); } );

	btnInventoryInsertRowBelow = new QPushButton ( tr ( "Add new item below" ) );
	btnInventoryInsertRowBelow->connect ( btnInventoryInsertRowBelow, &QPushButton::clicked,
	this, [&] ( const bool ) {
		return btnInventoryInsertRowBelow_clicked (); } );

	btnInventoryRemoveRow = new QPushButton ( tr ( "Remove item" ) );
	btnInventoryRemoveRow->connect ( btnInventoryRemoveRow, &QPushButton::clicked,
	this, [&] ( const bool ) {
		return btnInventoryRemoveRow_clicked (); } );

	QGridLayout* gLayoutInventoryButtons ( new QGridLayout );
	gLayoutInventoryButtons->addWidget ( btnInventoryEditTable, 0, 0 );
	gLayoutInventoryButtons->addWidget ( btnInventoryCancelEdit, 0, 1 );
	gLayoutInventoryButtons->addWidget ( new QWidget (), 0, 3, 1, 1 );
	gLayoutInventoryButtons->addWidget ( btnInventoryInsertRowAbove, 1, 0 );
	gLayoutInventoryButtons->addWidget ( btnInventoryInsertRowBelow, 1, 1 );
	gLayoutInventoryButtons->addWidget ( btnInventoryRemoveRow, 1, 2 );

	createTable ();
	m_table->setCallbackForCellChanged ( [&] ( const vmTableItem* const item ) {
		return tableChanged ( item ); } );
	m_table->setCallbackForCellNavigation ( [&] ( const uint row, const uint col, const uint prev_row, const uint ) {
		return readRowData ( row, col, prev_row ); } );
	m_table->setCallbackForRowRemoved ( [&] ( const uint row ) {
		return rowRemoved ( row ); } );

	inventoryLayout = new QVBoxLayout;
	m_table->addToLayout ( inventoryLayout );
	inventoryLayout->addLayout ( gLayoutInventoryButtons, 1 );

	/*if ( DATA ()->reads[TABLE_INVENTORY_ORDER] ) {
		const bool can_write ( DATA ()->writes[TABLE_INVENTORY_ORDER] );
		btnInventoryCancelEdit->setEnabled ( can_write );
		btnInventoryInsertRowAbove->setEnabled ( can_write );
		btnInventoryInsertRowBelow->setEnabled ( can_write );
		btnInventoryRemoveRow->setEnabled ( can_write );
	}
	else {
		btnsFrame->setEnabled ( false );
		m_table->setEnabled ( false );
	}*/
	return true;
}

void InventoryUI::readRowData ( const uint row, const uint, const uint prev_row, const uint )
{
	if ( prev_row != row ) {
		inventory_rec->clearAll ();
		if ( !m_table->sheetItem ( row, 0 )->text ().isEmpty () ) {
			inventory_rec->setAction ( ACTION_READ );
			for ( uint i_col ( 0 ) ; i_col < unsigned ( m_table->columnCount () ); ++i_col )
				setRecValue ( inventory_rec, i_col, m_table->sheetItem ( row, i_col )->text () );
			inventory_rec->setAction ( ACTION_EDIT );
		}
		else
			inventory_rec->setAction ( ACTION_ADD );
		inventory_rec->setIntBackupValue ( FLD_INVENTORY_ITEM, row );
	}
}

void InventoryUI::tableChanged ( const vmTableItem* const item )
{
	setRecValue ( inventory_rec, item->column (), item->text () );
	inventory_rec->saveRecord ();
	m_table->setTableUpdated ();
}

void InventoryUI::rowRemoved ( const uint row )
{
	if ( !m_table->sheetItem ( row, 0 )->text ().isEmpty () ) {
		inventory_rec->setAction ( ACTION_DEL );
		inventory_rec->deleteRecord (); // deletes from database before removing the row
	}
	m_table->removeRow ( row );
}

void InventoryUI::insertRow ( const uint i_row )
{
	m_table->insertRow ( i_row );
	m_table->setCurrentCell ( i_row, m_table->currentColumn () );
}

void InventoryUI::createTable ()
{
	if ( m_table != nullptr ) return;
	vmTableColumn *fields[INVENTORY_FIELD_COUNT+1] = { nullptr };

	for ( uint i ( 0 ); i < INVENTORY_FIELD_COUNT; ++i ) {
		fields[i] = new vmTableColumn;
		fields[i]->label = VivenciaDB::getTableColumnLabel ( &inventory_rec->t_info, i );

		switch ( i ) {
			case FLD_INVENTORY_ID:
				fields[FLD_INVENTORY_ID]->editable = false;
				fields[FLD_INVENTORY_ID]->width = 40;
			break;
			case FLD_INVENTORY_ITEM:
				fields[FLD_INVENTORY_ITEM]->completer_type = vmCompleters::ITEM_NAMES;
				fields[FLD_INVENTORY_ITEM]->width = 250;
			break;
			case FLD_INVENTORY_BRAND:
				fields[FLD_INVENTORY_BRAND]->completer_type = vmCompleters::BRAND;
				fields[FLD_INVENTORY_BRAND]->text_type = vmWidget::TT_UPPERCASE;
				fields[FLD_INVENTORY_BRAND]->width = 120;
			break;
			case FLD_INVENTORY_TYPE:
				fields[FLD_INVENTORY_TYPE]->completer_type = vmCompleters::STOCK_TYPE;
				fields[FLD_INVENTORY_TYPE]->width = 120;
			break;
			case FLD_INVENTORY_SUPPLIER:
				fields[FLD_INVENTORY_SUPPLIER]->completer_type = vmCompleters::SUPPLIER;
				fields[FLD_INVENTORY_SUPPLIER]->width = 150;
			break;
			case FLD_INVENTORY_PLACE:
				fields[FLD_INVENTORY_PLACE]->completer_type = vmCompleters::STOCK_PLACE;
				fields[FLD_INVENTORY_PLACE]->width = 120;
			break;
			case FLD_INVENTORY_UNIT:
				fields[FLD_INVENTORY_UNIT]->width = 40;
			break;
			case FLD_INVENTORY_PRICE:
				fields[FLD_INVENTORY_PRICE]->text_type = vmLineEdit::TT_PRICE;
			break;
			case FLD_INVENTORY_DATE_IN:
				fields[FLD_INVENTORY_DATE_IN]->wtype = WT_DATEEDIT;
			break;
			case FLD_INVENTORY_QUANTITY:
				fields[FLD_INVENTORY_QUANTITY]->text_type = vmLineEdit::TT_DOUBLE;
				fields[FLD_INVENTORY_QUANTITY]->width = 60;
			break;
		}
	}

	m_table = new vmTableWidget ( 30, fields, nullptr );
	m_table->setKeepModificationRecords ( false );
	VDB ()->populateTable ( inventory_rec, m_table );
}

void InventoryUI::btnInventoryEditTable_clicked ( const bool checked )
{
	m_table->setEditable ( checked );
	if ( checked ) {
		btnInventoryEditTable->setText ( tr ( "Save" ) );
		m_table->setFocus ();
	}
	else
		btnInventoryEditTable->setText ( tr ( "&Edit" ) );
	btnInventoryInsertRowAbove->setEnabled ( checked );
	btnInventoryInsertRowBelow->setEnabled ( checked );
	btnInventoryCancelEdit->setEnabled ( checked );
	btnInventoryRemoveRow->setEnabled ( checked );
}

void InventoryUI::btnInventoryCancelEdit_clicked ()
{
	if ( m_table->isEditable () ) {
		btnInventoryEditTable->setChecked ( false );
		btnInventoryEditTable_clicked ( false );
	}
}

void InventoryUI::btnInventoryRemoveRow_clicked ()
{
	if ( m_table->currentRow () >= 1 )
		rowRemoved ( m_table->currentRow () );
}

void InventoryUI::btnInventoryInsertRowBelow_clicked ()
{
	insertRow ( m_table->currentRow () + 1 );
}

void InventoryUI::btnInventoryInsertRowAbove_clicked ()
{
	insertRow ( m_table->currentRow () );
}
