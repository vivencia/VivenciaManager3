#include "vmtablewidget.h"
#include "global.h"
#include "vmwidgets.h"
#include "textdb.h"
#include "data.h"
#include "stringrecord.h"
#include "vmwidgets.h"
#include "heapmanager.h"
#include "vmcheckedtableitem.h"
#include "vmnotify.h"
#include "fast_library_functions.h"

#include <QLabel>
#include <QMouseEvent>
#include <QToolButton>
#include <QVBoxLayout>
#include <QMenu>
#include <QApplication>
#include <QGridLayout>

QString vmTableWidget::defaultBGColor ( emptyString );

constexpr uint PURCHASES_TABLE_COLS ( 7 );

//---------------------------------------------STATIC---------------------------------------------------------
/*static QString encode_pos(int row, int col)
{
	return QString(col + 'A') + QString::number(row + 1);
}*/

/*int rowByContent ( const vmTableWidget* table, const uint column, const QString& content )
{
	for ( int i_row ( 0 ); i_row <= table->lastUsedRow (); ++i_row ) {
		if ( table->sheetItem ( i_row, column )->text () == content )
			return i_row;
	}
	return -1;
}*/
//---------------------------------------------STATIC---------------------------------------------------------

//---------------------------------------------TABLE WIDGET---------------------------------------------------------
vmTableWidget::vmTableWidget ( QWidget* parent )
	: QTableWidget ( parent ), vmWidget ( WT_TABLE ),
	  mTotalsRow ( -1 ), m_lastUsedRow ( -1 ), m_ncols ( 0 ), mAddedItems ( 0 ),
	  mTableChanged ( false ), mbKeepModRec ( true ),
	  mPlainTable ( false ), mIsPurchaseTable ( false ), mbDoNotUpdateCompleters ( false ), readOnlyColumnsMask ( 0 ), modifiedRows ( 0, 10 ),
	  mMonitoredCells ( 2 ), m_highlightedCells ( 5 ), m_itemsToReScan ( 10 ), mSearchList ( 10 ),
	  mContextMenu ( nullptr ), mOverrideFormulaAction ( nullptr ), mSetFormulaAction ( nullptr ),
	  mFormulaTitleAction ( nullptr ), mParentLayout ( nullptr ), m_searchPanel ( nullptr ),
	  mContextMenuCell ( nullptr ), cellChanged_func ( nullptr ), cellNavigation_func ( nullptr ),
	  monitoredCellChanged_func ( nullptr ), rowRemoved_func ( nullptr ), rowActivated_func ( nullptr ),
	  searchPanelPlacement_func ( nullptr )
{
	sharedContructorsCode ();
}

vmTableWidget::vmTableWidget ( const uint rows, vmTableColumn** cols, QWidget* parent )
	: QTableWidget ( rows + 1, 0, parent ), vmWidget ( WT_TABLE, rows ),
	  mTotalsRow ( -1 ), m_lastUsedRow ( -1 ), m_ncols ( 0 ), mAddedItems ( 0 ),
	  mTableChanged ( false ), mbKeepModRec ( true ),
	  mPlainTable ( false ), mIsPurchaseTable ( false ), mbDoNotUpdateCompleters ( false ), readOnlyColumnsMask ( 0 ), modifiedRows ( 0, 10 ),
	  mMonitoredCells ( 2 ), m_highlightedCells ( 5 ), m_itemsToReScan ( rows + 1 ), mSearchList ( 10 ),
	  mContextMenu ( nullptr ), mOverrideFormulaAction ( nullptr ), mSetFormulaAction ( nullptr ),
	  mFormulaTitleAction ( nullptr ), mParentLayout ( nullptr ), m_searchPanel ( nullptr ),
	  mContextMenuCell ( nullptr ), cellChanged_func ( nullptr ), cellNavigation_func ( nullptr ),
	  monitoredCellChanged_func ( nullptr ), rowRemoved_func ( nullptr ), rowActivated_func ( nullptr ),
	  searchPanelPlacement_func ( nullptr )
{
	sharedContructorsCode ();
	initTable ( rows, cols );
}

vmTableWidget::~vmTableWidget ()
{
	disconnect ( this, nullptr, nullptr, nullptr );
	mSearchList.clear ( true );
	modifiedRows.clear ();
	heap_del ( m_searchPanel );
	heap_del ( mCopyCellAction );
	heap_del ( mCopyRowContents );
	heap_del ( mInsertRowAction );
	heap_del ( mDeleteRowAction );
	heap_del ( mClearRowAction );
	heap_del ( mClearTableAction );
	heap_del ( mUndoAction );
	heap_del ( mContextMenu );
	heap_del ( mOverrideFormulaAction );
	heap_del ( mSetFormulaAction );
	heap_del ( mFormulaTitleAction );
}

void vmTableWidget::initTable ( const uint rows, vmTableColumn** cols )
{
	uint i_col ( 0 );
	int i_row ( 0 );

	uint colWidth ( 0 );
	vmTableColumn* column ( nullptr );
	vmTableItem* sheet_item ( nullptr );

	QString col_header;
	QFont titleFont ( font () );
	titleFont.setBold ( true );
	mTotalsRow = rows;
	const int row_count ( rows + static_cast<uint>( !mPlainTable ) );
	setRowCount ( row_count );

	vmCheckedTableItem* headerItem ( new vmCheckedTableItem ( Qt::Horizontal, this ) );
	headerItem->setCallbackForCheckStateChange ( [&] ( const uint col, const bool checked ) {
			return headerItemToggled ( col, checked ); } );
	setHorizontalHeader ( headerItem );

	do
	{
		column = cols[i_col];
		if ( column != nullptr ) {
			insertColumn ( i_col );

			if ( !column->editable )
				setBit ( readOnlyColumnsMask, i_col ); // bit is set? read only cell : cell can be cleared or other actions

			for ( i_row = 0; i_row < row_count; ++i_row ) {
				if ( i_row != mTotalsRow || mPlainTable ) // last row, read-only, displays formulas like "sum" for the previous rows
					sheet_item = new vmTableItem ( column->wtype, column->text_type, column->default_value, this );
				else
					sheet_item = new vmTableItem ( WT_LINEEDIT, i_col != 0 ? column->text_type : vmWidget::TT_TEXT, emptyString, this );

				setItem ( i_row, i_col, sheet_item );
				sheet_item->setButtonType ( column->button_type );
				sheet_item->setCompleterType ( column->completer_type );
				setCellWidget ( sheet_item );
				sheet_item->setEditable ( false );
				if ( i_row == mTotalsRow && !mPlainTable ) {
					if ( i_col == 0 )
						sheet_item->setText ( tr ( "Total:" ), false, false );
					else {
						if ( column->text_type >= vmLineEdit::TT_PRICE ) {
							col_header = QChar ( 'A' + i_col );
							sheet_item->setFormula ( QLatin1String ( "sum " ) + col_header +
													 CHR_ZERO + CHR_SPACE + col_header + QLatin1String ( "%1" ), QString::number ( mTotalsRow - 1 ) );
						}
					}
				}
				else { //i_row != mTotalsRow)
					if ( !column->formula.isEmpty () )
						sheet_item->setFormula ( column->formula, QString::number ( i_row ) );
				}
			}

			setHorizontalHeaderItem ( i_col, new vmTableItem ( WT_TABLE_ITEM, vmLineEdit::TT_TEXT, column->label, this ) );
			colWidth = column->width;
			if ( colWidth == 0 ) {
				switch ( column->wtype ) {
					case WT_DATEEDIT:
						colWidth = 180;
					break;
					case WT_LINEEDIT:
						colWidth = ( column->text_type >= vmWidget::TT_PHONE ? 130 : 100 );
					break;
					case WT_LINEEDIT_WITH_BUTTON:
						colWidth = 150;
					default:
						colWidth = 100;
					break;
				}
			}
			setColumnWidth ( i_col, colWidth );
			++i_col;
		}
		else
			break;
	}
	while ( true );

	m_ncols = unsigned ( columnCount () );
	for ( --i_col; ; --i_col ) {
		delete cols[i_col];
		cols[i_col] = nullptr;
		if ( i_col == 0 )
			break;
	}

	//All the items that could not have their formula calculated at the time of creation will now
	for ( i_row = 0; i_row < (signed) m_itemsToReScan.count (); ++i_row )
		m_itemsToReScan.at ( i_row )->targetsFromFormula ();
}

bool vmTableWidget::selectFound ( const vmTableItem* item )
{
	if ( item != nullptr ) {
		scrollToItem ( item, QAbstractItemView::PositionAtCenter );
		setCurrentItem ( const_cast<vmTableItem*>( item ), QItemSelectionModel::ClearAndSelect );
		selectRow ( item->row () );
		return true;
	}
	return false;
}

// Assumes searchTerm is valid. It's a private function for that reason: only friend
bool vmTableWidget::searchStart ( const QString& searchTerm )
{
	if ( searchTerm.count () >= 2 && searchTerm != mSearchTerm ) {
		searchCancel ();

		uint i_row ( 0 ), i_col ( 0 );
		uint max_row ( lastUsedRow () ), max_col ( colCount () );
		vmTableItem* item ( nullptr );
		mSearchTerm = searchTerm;

		for ( ; i_row <= max_row; ++i_row ) {
			for ( i_col = 0 ; i_col < max_col; ++i_col ) {
				if ( isColumnSelectedForSearch ( i_col ) ) {
					item = sheetItem ( i_row, i_col );
					if ( item->text ().contains ( searchTerm, Qt::CaseInsensitive ) ) {
						item->highlight ( vmBlue );
						mSearchList.append ( item );
					}
				}
			}
		}
		return !mSearchList.isEmpty ();
	}
	return false;
}

void vmTableWidget::searchCancel ()
{
	for ( uint i ( 0 ); i < mSearchList.count (); ++i )
		mSearchList.at ( i )->highlight ( vmDefault_Color );
	static_cast<vmCheckedTableItem*> ( horizontalHeader () )->setCheckable ( false );
	mSearchList.clearButKeepMemory ( true );
	mSearchTerm.clear ();
}

bool vmTableWidget::searchFirst ()
{
	return selectFound ( mSearchList.first () );
}

bool vmTableWidget::searchPrev ()
{
	return selectFound ( mSearchList.prev () );
}

bool vmTableWidget::searchNext ()
{
	return selectFound ( mSearchList.next () );
}

bool vmTableWidget::searchLast ()
{
	return selectFound ( mSearchList.last () );
}

void vmTableWidget::showSearchPanel ()
{
	searchPanelPlacement_func ( true );
	m_searchPanel->showPanel ();
}

void vmTableWidget::hideSearchPanel ()
{
	searchPanelPlacement_func ( false );
	m_searchPanel->hide ();
	setFocus ();
}

vmTableWidget* vmTableWidget::createPurchasesTable ( vmTableWidget* table, QWidget* parent )
{
	vmTableColumn* cols[PURCHASES_TABLE_COLS+1] = { nullptr };
	uint i ( 0 );
	for ( ; i < PURCHASES_TABLE_COLS; ++i ) {
		cols[i] = new vmTableColumn;
		switch ( i ) {
			case ISR_NAME:
				cols[ISR_NAME]->completer_type = vmCompleters::PRODUCT_OR_SERVICE;
				cols[ISR_NAME]->width = 280;
				cols[ISR_NAME]->label = tr ( "Item" );
			break;
			case ISR_UNIT:
				cols[ISR_UNIT]->label = tr ( "Unit" );
			break;
			case ISR_BRAND:
				cols[ISR_BRAND]->completer_type = vmCompleters::BRAND;
				cols[ISR_BRAND]->text_type = vmWidget::TT_UPPERCASE;
				cols[ISR_BRAND]->label = tr ( "Maker" );
			break;
			case ISR_QUANTITY:
				cols[ISR_QUANTITY]->text_type = vmLineEdit::TT_DOUBLE;
				cols[ISR_QUANTITY]->label = tr ( "Quantity" );
			break;
			case ISR_UNIT_PRICE:
				cols[ISR_UNIT_PRICE]->text_type = vmLineEdit::TT_PRICE;
				cols[ISR_UNIT_PRICE]->button_type = vmLineEditWithButton::LEBT_CALC_BUTTON;
				cols[ISR_UNIT_PRICE]->label = tr ( "Unit price" );
			break;
			case ISR_TOTAL_PRICE:
				cols[ISR_TOTAL_PRICE]->text_type = vmLineEdit::TT_PRICE;
				cols[ISR_TOTAL_PRICE]->formula = QStringLiteral ( "* D%1 E%1" );
				cols[ISR_TOTAL_PRICE]->editable = false;
				cols[ISR_TOTAL_PRICE]->label = tr ( "Total price" );
			break;
			case PURCHASES_TABLE_REG_COL:
				cols[PURCHASES_TABLE_REG_COL]->wtype = WT_CHECKBOX;
				cols[PURCHASES_TABLE_REG_COL]->width = 50;
				cols[PURCHASES_TABLE_REG_COL]->default_value = CHR_ZERO;
				cols[PURCHASES_TABLE_REG_COL]->label = tr ( "Register" );
			break;
		}
	}

	if ( table )
		table->initTable ( 10, cols );
	else
		table = new vmTableWidget ( 10, cols, parent );

	table->mIsPurchaseTable = true;
	table->insertMonitoredCell ( table->totalsRow (), ISR_TOTAL_PRICE );
	return table;
}

void vmTableWidget::exchangePurchaseTablesInfo (
	const vmTableWidget* const src_table, vmTableWidget* const dst_table,
	const DBRecord* const src_dbrec, DBRecord* const dst_dbrec )
{
	if ( !src_table->property ( PROPERTY_TABLE_HAS_ITEM_TO_REGISTER ).toBool () )
		return;

	spreadRow* s_row ( new spreadRow );
	s_row->column[0] = dst_dbrec->isrRecordField ( ISR_NAME );
	s_row->column[1] = dst_dbrec->isrRecordField ( ISR_UNIT );
	s_row->column[2] = dst_dbrec->isrRecordField ( ISR_BRAND );
	s_row->column[3] = dst_dbrec->isrRecordField ( ISR_UNIT_PRICE );
	s_row->column[4] = dst_dbrec->isrRecordField ( ISR_SUPPLIER );
	s_row->column[5] = dst_dbrec->isrRecordField ( ISR_DATE );

	bool bEditable ( false );

	if ( dst_table != nullptr ) {
		bEditable = dst_table->isEditable ();
		dst_table->setEditable ( true );
	}

	s_row->field_value[s_row->column[4]] = recStrValue ( src_dbrec, src_dbrec->isrRecordField ( ISR_SUPPLIER ) );
	s_row->field_value[s_row->column[5]] = recStrValue ( src_dbrec, src_dbrec->isrRecordField ( ISR_DATE ) );

	for ( int i_row ( 0 ); i_row <= src_table->lastUsedRow (); ++i_row ) {
		if ( src_table->sheetItem ( i_row, PURCHASES_TABLE_REG_COL )->text () == CHR_ZERO ) // item is not marked to be exported
			continue;

		s_row->field_value[s_row->column[0]] = src_table->sheetItem ( i_row, ISR_NAME )->text ();
		s_row->field_value[s_row->column[1]] = src_table->sheetItem ( i_row, ISR_UNIT )->text ();
		s_row->field_value[s_row->column[2]] = src_table->sheetItem ( i_row, ISR_BRAND )->text ();
		s_row->field_value[s_row->column[3]] = src_table->sheetItem ( i_row, ISR_UNIT_PRICE )->text ();

		if ( dst_dbrec->readRecord ( s_row->column[0], s_row->field_value[s_row->column[0]] ) ) { //item description
			if ( recStrValue ( dst_dbrec, s_row->column[1] ) == s_row->field_value[s_row->column[1]] ) { // item unit
				if ( recStrValue ( dst_dbrec, s_row->column[2] ) == s_row->field_value[s_row->column[2]] ) { // item brand
					if ( recStrValue ( dst_dbrec, s_row->column[4] ) == s_row->field_value[s_row->column[4]] ) { // item supplier
						if ( recStrValue ( dst_dbrec, s_row->column[3] ) == s_row->field_value[s_row->column[3]] ) { // item price
							if ( recStrValue ( dst_dbrec, s_row->column[5] ) == s_row->field_value[s_row->column[5]] ) // item date
								continue; // everything is the same, do nothing
						}
						dst_dbrec->setAction ( ACTION_EDIT ); // either price or date are different: edit to update the info
					}
				}
			}
		}
		if ( dst_dbrec->action () == ACTION_READ )
			dst_dbrec->setAction ( ACTION_ADD ); //one single piece of different information (price and date excluded ) and we add the new info

		/* dst_table might be nullptr. For optimization purpose, a table might be uninitialized by the time this function is called
		 * but the dbrecord must be existent. At a later point, when the table might be created, VivenciaDB::populateTable must be called
		 */
		if ( dst_table != nullptr ) {
			if ( dst_dbrec->action () == ACTION_ADD )
				s_row->row = dst_table->lastUsedRow () + 1;
			else
				s_row->row = recIntValue ( dst_dbrec, dst_dbrec->isrRecordField ( ISR_ID ) );
			s_row->field_value[dst_dbrec->isrRecordField ( ISR_ID )] = QString::number ( s_row->row );
			dst_table->setRowData ( s_row, true );
		}
		else {
			if ( dst_dbrec->action () == ACTION_ADD ) {
				setRecValue ( dst_dbrec, s_row->column[0], s_row->field_value[s_row->column[0]] );
				setRecValue ( dst_dbrec, s_row->column[1], s_row->field_value[s_row->column[1]] );
				setRecValue ( dst_dbrec, s_row->column[2], s_row->field_value[s_row->column[2]] );
				setRecValue ( dst_dbrec, s_row->column[4], s_row->field_value[s_row->column[4]] );
			}
			// only update is on either PRICE or DATE when editing.
			setRecValue ( dst_dbrec, s_row->column[3], s_row->field_value[s_row->column[3]] );
			setRecValue ( dst_dbrec, s_row->column[5], s_row->field_value[s_row->column[5]] );
			dst_dbrec->saveRecord ();
			dst_dbrec->clearAll ();
		}
	}
	delete s_row;
	if ( dst_table != nullptr )
		dst_table->setEditable ( bEditable );
}

vmTableWidget* vmTableWidget::createPayHistoryTable ( vmTableWidget* table, QWidget* parent,
		const PAY_HISTORY_RECORD last_column )
{
	vmTableColumn** cols ( new vmTableColumn*[last_column + 2] );
	cols[static_cast<uint> ( last_column ) + 1] = nullptr;

	for ( uint i ( 0 ); i <= last_column; ++i ) {
		switch ( i ) {
			case PHR_DATE:
				cols[PHR_DATE] = new vmTableColumn;
				cols[PHR_DATE]->label = tr ( "Date" );
				cols[PHR_DATE]->wtype = WT_DATEEDIT;
			break;
			case PHR_VALUE:
				cols[PHR_VALUE] = new vmTableColumn;
				cols[PHR_VALUE]->label = tr ( "Value" );
				cols[PHR_VALUE]->button_type = vmLineEditWithButton::LEBT_CALC_BUTTON;
				cols[PHR_VALUE]->text_type = vmLineEdit::TT_PRICE;
			break;
			case PHR_PAID:
				cols[PHR_PAID] = new vmTableColumn;
				cols[PHR_PAID]->label = tr ( "Paid?" );
				cols[PHR_PAID]->wtype = WT_CHECKBOX;
				cols[PHR_PAID]->width = 50;
			break;
			case PHR_METHOD:
				cols[PHR_METHOD] = new vmTableColumn;
				cols[PHR_METHOD]->label = tr ( "Method" );
				cols[PHR_METHOD]->completer_type = vmCompleters::PAYMENT_METHOD;
				cols[PHR_METHOD]->text_type = vmWidget::TT_UPPERCASE;
				cols[PHR_METHOD]->width = 200;
			break;
			case PHR_ACCOUNT:
				cols[PHR_ACCOUNT] = new vmTableColumn;
				cols[PHR_ACCOUNT]->label = tr ( "Account" );
				cols[PHR_ACCOUNT]->completer_type = vmCompleters::ACCOUNT;
				cols[PHR_ACCOUNT]->width = 120;
			break;
			case PHR_USE_DATE:
				cols[PHR_USE_DATE] = new vmTableColumn;
				cols[PHR_USE_DATE]->label = tr ( "Use Date" );
				cols[PHR_USE_DATE]->wtype = WT_DATEEDIT;
			break;
		}
	}

	if ( table )
		table->initTable ( 5, cols );
	else
		table = new vmTableWidget ( 5, cols, parent );

	table->insertMonitoredCell ( table->totalsRow (), PHR_VALUE );
	heap_del ( cols );
	return table;
}

void vmTableWidget::insertRow ( const uint row, const uint n )
{
	if ( row <= (unsigned) rowCount () ) {
		QString new_formula;
		int i_row ( 0 );
		uint i_col ( 0 );
		vmTableItem* sheet_item ( nullptr ), *new_SheetItem ( nullptr );
		const uint reference_row ( row == totalsRow () ? row - 1 : row );

		for ( ; i_row < (signed) n; ++i_row ) { // Insert new rows accordingly, i.e., compute their formulas, if they have
			QTableWidget::insertRow ( row + i_row );
			for ( i_col = 0; i_col < colCount (); ++i_col ) {
				sheet_item = sheetItem ( reference_row + i_row, i_col ); // use row above as template
				new_SheetItem = new vmTableItem ( sheet_item->widgetType (), sheet_item->textType (), sheet_item->defaultValue (), this );
				new_SheetItem->setButtonType ( sheet_item->buttonType () );
				new_SheetItem->setCompleterType ( sheet_item->completerType () );
				setItem ( row + i_row, i_col, new_SheetItem );
				if ( sheet_item->widget () )
					setCellWidget ( new_SheetItem );

				/* The formula must be set for last because setFormula calls vmTableItem::targetsFromFormula ();
				 * when inserting cells, that function might attempt to solve the formula using data from the previous
				 * cell (i.e. before inserting) or, worse, from the totalsRow (), which will cause unpredicted (but fatal) results
				 */
				if ( sheet_item->hasFormula () ) {
					new_formula = sheet_item->formulaTemplate ();
					new_SheetItem->setFormula ( new_formula, QString::number ( row + i_row ) );
				}
				new_SheetItem->setEditable ( sheet_item->isEditable () );
			}
		}
		mTotalsRow += n;

		// update all rows after the inserted ones, so that their formula reflects the row changes
		if ( !mPlainTable ) {
			for ( i_row = row + n; i_row < mTotalsRow; ++i_row ) {
				for ( i_col = 0; i_col < colCount (); ++i_col ) {
					sheet_item = sheetItem ( i_row, i_col );
					if ( sheet_item->hasFormula () ) {
						new_formula = sheet_item->formulaTemplate ();
						sheet_item->setFormula ( new_formula, QString::number ( i_row ) );
					}
				}
			}
			fixTotalsRow ();
		}

		if ( m_lastUsedRow <= ( signed )( row + n ) )
			m_lastUsedRow += n;

		uint i ( 0 ), list_value ( 0 ), start_modification ( 0 );

		const uint modified_rows ( modifiedRows.count () );
		if ( modified_rows > 0 ) {
			for ( i = 0; i < modified_rows; ++i ) {
				if ( row < modifiedRows.at ( i ) ) break;
				start_modification++; // modified above insertion start point are not affected
			}
			for ( i = start_modification; i < modified_rows; ++i ) {
				list_value = modifiedRows.at ( i );
				modifiedRows.replace ( i, list_value + n ); // push down modified rows
			}
		}
	}
}

void vmTableWidget::removeRow ( const uint row, const uint n )
{
	if ( row < (unsigned) rowCount () ) {
		QString new_formula;

		int i_row ( 0 );
		uint i_col ( 0 );

		for ( ; i_row < (signed) n; ++i_row )
			QTableWidget::removeRow ( row );
		mTotalsRow -= n;

		if ( !mPlainTable ) {
			vmTableItem* sheet_item ( nullptr );
			for ( i_row = row; i_row < (signed) mTotalsRow; ++i_row ) {
				for ( i_col = 0; i_col < colCount (); ++i_col ) {
					sheet_item = sheetItem ( i_row, i_col );
					if ( sheet_item->hasFormula () ) {
						new_formula = sheet_item->formulaTemplate ();
						sheet_item->setFormula ( new_formula, QString::number ( i_row ) );
					}
				}
			}
			fixTotalsRow ();
		}

		if ( ( signed )( row + n ) >= m_lastUsedRow )
			m_lastUsedRow -= n;

		uint i ( 0 ), list_value ( 0 ), start_modification ( 0 );

		const uint modified_rows ( modifiedRows.count () );
		if ( modified_rows > 0 ) {
			for ( i = 0 ; i < modifiedRows.count (); ++i ) {
				if ( row < modifiedRows.at ( i ) ) break;
				start_modification++; // modified rows above deletion start point are not affected
			}
			for ( i = start_modification; i < modifiedRows.count (); ++i ) {
				list_value = modifiedRows.at ( i );
				modifiedRows.replace ( i, list_value - n ); // push up modified rows
			}
		}
		modifiedRows.removeOne ( row, 0 );
	}
}

void vmTableWidget::clearRow ( const uint row, const uint n )
{
	if ( isEditable () ) {
		if ( ( signed )row <= m_lastUsedRow ) {
			vmTableItem* item ( nullptr );
			for ( uint i_row ( row ); i_row < row + n; ++i_row ) {
				for ( uint i_col = 0; i_col <= ( colCount () - 1 ); ++i_col ) {
					if ( !isBitSet ( readOnlyColumnsMask, i_col ) ) {
						item = sheetItem ( i_row, i_col );
						item->setTextToDefault ( true );
						item->highlight ( vmDefault_Color );
					}
				}
			}
		}
	}
}

// Clear items for fresh view
void vmTableWidget::clear ( const bool force )
{
	const int max_row ( force ? mTotalsRow - 1 : m_lastUsedRow );
	uint i_col ( 0 );
	vmTableItem* item ( nullptr );
	for ( int i_row ( 0 ); i_row <= max_row; ++i_row ) {
		for ( i_col = 0; i_col <= ( colCount () - 1 ); ++i_col ) {
			if ( !isBitSet ( readOnlyColumnsMask, i_col ) ) {
				item = sheetItem ( i_row, i_col );
				item->setTextToDefault ( isEditable () );
				item->highlight ( vmDefault_Color );
			}
		}
	}
	scrollToItem ( sheetItem ( 0, 0 ), QAbstractItemView::PositionAtTop );
	setCurrentCell ( 0, 0, QItemSelectionModel::ClearAndSelect );
	mTableChanged = false;
	setProperty ( PROPERTY_TABLE_HAS_ITEM_TO_REGISTER, false );
	m_lastUsedRow = -1;
}

void vmTableWidget::rowActivatedConnection ( const bool b_activate )
{
	if ( rowActivated_func ) {
		if ( b_activate )
			connect ( this, &QTableWidget::itemActivated, this, [&] ( QTableWidgetItem* current ) {
				return rowActivated_func ( static_cast<vmTableItem*>( current ) ); } );
		else
			disconnect ( this, &QTableWidget::currentItemChanged, nullptr, nullptr );
	}
}

void vmTableWidget::setCellValue ( const QString& value, const uint row, const uint col )
{
	if ( col < colCount () ) {
		if ( row >= (unsigned) rowCount () )
			appendRow ();
		sheetItem ( row, col )->setText ( value, false, false );
	}
}

void vmTableWidget::setRowData ( const spreadRow* s_row, const bool b_notify )
{
	if ( s_row != nullptr ) {
		if ( s_row->row == -1 )
			const_cast<spreadRow*>( s_row )->row = lastUsedRow () + 1;
		if ( s_row->row >= signed ( totalsRow () ) )
			appendRow ();
		for ( uint i_col ( 0 ), used_col ( s_row->column.at ( 0 ) );
				i_col < unsigned ( s_row->column.count () );
				used_col = s_row->column.at ( ++i_col ) )
			sheetItem ( s_row->row, used_col )->setText ( s_row->field_value.at ( used_col ), false, b_notify );
	}
}

void vmTableWidget::rowData ( const uint row, spreadRow* s_row ) const
{
	if ( row < (unsigned)mTotalsRow ) {
		uint i_col ( 0 );
		s_row->row = row;
		s_row->field_value.clearButKeepMemory ();

		for ( ; i_col < colCount (); ++i_col )
			s_row->field_value.append ( sheetItem ( row, i_col )->text () );
	}
}

void vmTableWidget::cellContentChanged ( const vmTableItem* const item )
{
	const uint row ( item->row () );
	mTableChanged = true;
	//if ( row < mTotalsRow ) {
		if ( mbKeepModRec ) {
			uint insert_pos ( modifiedRows.count () ); // insert rows in crescent order

			for ( int i ( signed ( modifiedRows.count () - 1 ) ); i >= 0; --i ) {
				if ( unsigned ( row ) == modifiedRows.at ( i ) ) {
					if ( cellChanged_func )
						cellChanged_func ( item );
					return;
				}
				else if ( unsigned ( row ) > modifiedRows.at ( i ) ) {
					break;
				}
				else
					--insert_pos;
			}
			modifiedRows.insert ( insert_pos, unsigned ( row ) );
		}
		if ( mIsPurchaseTable ) {
			if ( item->column () == PURCHASES_TABLE_REG_COL )
				setProperty ( PROPERTY_TABLE_HAS_ITEM_TO_REGISTER, true );
		}
		if ( cellChanged_func )
			cellChanged_func ( item );
	//}
	//else {
	//	if ( isCellMonitored ( item ) >= 0 && monitoredCellChanged_func )
	//		monitoredCellChanged_func ( item );
	//}
}

// Customize context menu
void vmTableWidget::renameAction ( const contextMenuDefaultActions action, const QString& new_text )
{
	if ( new_text.isEmpty () ) return;
	switch ( action ) {
		case UNDO:
			mUndoAction->setLabel ( new_text );
		break;
		case COPY_CELL:
			mCopyCellAction->setLabel ( new_text );
		break;
		case COPY_ROW:
			mCopyRowContents->setLabel ( new_text );
		break;
		case ADD_ROW:
			mInsertRowAction->setLabel ( new_text );
		break;
		case DEL_ROW:
			mDeleteRowAction->setLabel ( new_text );
		break;
		case CLEAR_ROW:
			mClearRowAction->setLabel ( new_text );
		break;
		case CLEAR_TABLE:
			mClearTableAction->setLabel ( new_text );
		default:
		break;
	}
}

void vmTableWidget::addContextMenuAction ( vmAction* action )
{
	if ( mAddedItems == 0 )
		mSeparator = mContextMenu->addSeparator ();
	++mAddedItems;
	mContextMenu->addAction ( static_cast<QAction*> ( action ) );
}

void vmTableWidget::removeContextMenuAction ( vmAction* action )
{
	if ( mAddedItems == 0 )
		return;
	if ( mAddedItems == 1 )
		mContextMenu->removeAction ( mSeparator );
	--mAddedItems;
	mContextMenu->removeAction ( action );
}

void vmTableWidget::loadFromStringTable ( const stringTable& data )
{
	clear ( true );
	if ( data.isOK () ) {
		spreadRow* s_row ( new spreadRow );
		uint i_col ( 0 );

		for ( ; i_col < colCount (); ++i_col )
			s_row->column[i_col] = i_col;

		const stringRecord* rec ( &data.first () );
		if ( rec->isOK () ) {
			s_row->row = 0;
			do {
				if ( rec->first () ) {
					for ( i_col = 0; i_col < colCount (); ++i_col ) {
						s_row->field_value[i_col] = rec->curValue ();
						if ( !rec->next () )
							break;
					}
					setRowData ( s_row );
					++( s_row->row );
					rec = &data.next ();
				}
			} while ( rec->isOK () );
		}
		delete s_row;
		scrollToTop ();
	}
}

void vmTableWidget::makeStringTable ( stringTable& data )
{
	stringRecord rec;
	uint i_col ( 0 );
	for ( uint i_row ( 0 ); ( signed )i_row <= lastUsedRow (); ++i_row ) {
		for ( i_col = 0; i_col < colCount (); ++i_col )
			rec.fastAppendValue ( sheetItem ( i_row, i_col )->text () );
		data.fastAppendRecord ( rec );
		rec.clear ();
	}
}

void vmTableWidget::setCellWidget ( vmTableItem* const sheet_item )
{
	vmWidget* widget ( nullptr );
	// read only columns do not connect signals, do not need completers
	const bool is_read_only ( isBitSet ( readOnlyColumnsMask, sheet_item->column () ) );

	switch ( sheet_item->widgetType () ) {
		case WT_LINEEDIT:
			widget = new vmLineEdit;
			static_cast<vmLineEdit*> ( widget )->setFrame ( false );
			static_cast<vmLineEdit*> ( widget )->setTextType ( sheet_item->textType () );
			if ( !is_read_only ) {
				APP_COMPLETERS ()->setCompleter ( static_cast<vmLineEdit*>( widget ), sheet_item->completerType () );
				widget->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
					return textWidgetChanged ( sender ); } );
			}
		break;
		case WT_LINEEDIT_WITH_BUTTON:
			widget = new vmLineEditWithButton;
			static_cast<vmLineEditWithButton*> ( widget )->setButtonType ( sheet_item->buttonType () );
			static_cast<vmLineEditWithButton*> ( widget )->lineControl ()->setTextType ( sheet_item->textType () );
			if ( !is_read_only ) {
				APP_COMPLETERS ()->setCompleter ( static_cast<vmLineEditWithButton*> ( widget )->lineControl (), sheet_item->completerType () );
					static_cast<vmLineEditWithButton*> ( widget )->lineControl ()->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
						return textWidgetChanged ( sender ); } );
			}
		break;
		case WT_CHECKBOX: // so far, only check box has a different default value, depending on the table to which they belong
			widget = new vmCheckBox;
			static_cast<vmCheckBox*> ( widget )->setChecked ( sheet_item->defaultValue () == CHR_ONE );
			if ( !is_read_only )
				widget->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
					return checkBoxToggled ( sender ); } );
		break;
		case WT_DATEEDIT:
			widget = new vmDateEdit;
			if ( !is_read_only )
				widget->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
					return dateWidgetChanged ( sender ); } );
		break;
		case WT_TIMEEDIT:
			widget = new vmTimeEdit;
			if ( !is_read_only )
				widget->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
					return timeWidgetChanged ( sender ); } );
		break;
		case WT_COMBO:
			widget = new vmComboBox;
			if ( !is_read_only )
				widget->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
					return textWidgetChanged ( sender ); } );
		break;
		default:
		return;
	}

	sheet_item->setWidget ( widget );
	QTableWidget::setCellWidget ( sheet_item->row (), sheet_item->column (), widget->toQWidget () );
	widget->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const widget ) {
		return cellWidgetRelevantKeyPressed ( ke, widget ); } );
	widget->setOwnerItem ( sheet_item );
	widget->setCallbackForContextMenu ( [&] ( const QPoint& pos, const vmWidget* const vm_widget ) {
		return displayContextMenuForCell ( pos, vm_widget ); } );
}

void vmTableWidget::searchPanelPlacement_GridLayout ( const bool bShow )
{
	// A QGridLayout is not dynamic as a QVBoxLayout. A dummy widget must be placed
	// before the table so that the search panel occupies its place
	QGridLayout* glayout ( static_cast<QGridLayout*>( mParentLayout ) );
	if ( bShow ) {
		int row ( 0 ), col ( 0 ), rowSpan ( 0 ), colSpan ( 0 );
		glayout->getItemPosition ( glayout->indexOf ( this ), &row, &col, &rowSpan, &colSpan );
		glayout->addWidget ( m_searchPanel, row - 1, col, rowSpan, colSpan );
	}
	else
		glayout->removeWidget( m_searchPanel );
}

void vmTableWidget::searchPanelPlacement_BoxLayout ( const bool bShow )
{
	QVBoxLayout* vblayout ( static_cast<QVBoxLayout*>( mParentLayout ) );
	if ( bShow ) {
		const int index ( vblayout->indexOf ( this ) );
		vblayout->insertWidget( index, m_searchPanel, vblayout->stretch ( index ) );
	}
	else
		vblayout->removeWidget( m_searchPanel );
}

void vmTableWidget::addToLayout ( QGridLayout* glayout, const uint row, const uint column )
{
	if ( setParentLayout ( glayout ) )
		glayout->addWidget ( this, row, column );
}

void vmTableWidget::addToLayout ( QVBoxLayout* vblayout, const uint stretch )
{
	if ( setParentLayout ( vblayout ) )
		vblayout->addWidget ( this, stretch );
}

bool vmTableWidget::setParentLayout ( QGridLayout* glayout )
{
	if ( mParentLayout == static_cast<QLayout*>( glayout ) )
		return false;
	if ( mParentLayout != nullptr )
		mParentLayout->removeWidget ( this );
	mParentLayout = static_cast<QLayout*>( glayout );
	searchPanelPlacement_func = [&] ( const bool bShow ) { return searchPanelPlacement_GridLayout ( bShow ); };
	return true;
}

bool vmTableWidget::setParentLayout ( QVBoxLayout* vblayout )
{
	if ( mParentLayout == static_cast<QLayout*>( vblayout ) )
		return false;
	if ( mParentLayout != nullptr )
		mParentLayout->removeWidget ( this );
	mParentLayout = static_cast<QLayout*>( vblayout );
	searchPanelPlacement_func = [&] ( const bool bShow ) { return searchPanelPlacement_BoxLayout ( bShow ); };
	return true;
}

/* For optimal results, when populating a table, set it readonly.
 * When clearing a table, set it readonly. This way, there won't be a plethora of
 * signals emitted.
 */
void vmTableWidget::setEditable ( const bool editable )
{
	if ( isEditable () == editable )
		return;

	int i_row ( 0 );
	uint i_col ( 0 );
	for ( ; i_row < mTotalsRow; ++i_row ) {
		for ( i_col = 0; i_col < colCount (); ++i_col ) {
			if ( !isBitSet ( readOnlyColumnsMask, i_col ) )
				sheetItem ( i_row, i_col )->setEditable ( editable );
		}
	}

	if ( editable ) {
		if ( cellNavigation_func != nullptr ) {
			connect ( this, &QTableWidget::currentCellChanged, this, [&] ( const int row, const int col, const int prev_row, const int prev_col ) {
				return cellNavigation_func ( static_cast<uint> ( row ), static_cast<uint> ( col ), static_cast<uint> ( prev_row ), static_cast<uint> ( prev_col ) ); } );
		}
		if ( mIsPurchaseTable ) {
			connect ( APP_COMPLETERS ()->getCompleter ( vmCompleters::PRODUCT_OR_SERVICE ),
					  static_cast<void (QCompleter::*)( const QModelIndex& )>( &QCompleter::activated ),
					this, ( [&, this] ( const QModelIndex& index ) { return interceptCompleterActivated ( index, this );
			} ) );
		}
	}
	else {
		if ( cellNavigation_func != nullptr )
			disconnect ( this, &QTableWidget::currentCellChanged, nullptr, nullptr );
		if ( mIsPurchaseTable )
			disconnect ( APP_COMPLETERS ()->getCompleter ( vmCompleters::PRODUCT_OR_SERVICE ), nullptr, this, nullptr );
	}
	rowActivatedConnection ( editable );
	vmWidget::setEditable ( editable );
}

uint vmTableWidget::selectedRowsCount () const
{
	uint n ( 0 );
	for ( uint i_row ( 0 ); ( signed )i_row <= m_lastUsedRow; ++i_row ) {
		for ( uint i_col = 0; i_col < ( colCount () - 1 ); ++i_col ) {
			if ( item ( i_row, i_col )->isSelected () ) {
				++n;
				break;
			}
		}
	}
	return n;
}

bool vmTableWidget::isRowEmpty ( const uint row ) const
{
	for ( uint i_col ( 0 ); i_col < colCount (); ++i_col ) {
		if ( sheetItem ( row, i_col )->text () != sheetItem ( row, i_col )->defaultValue () ) { // cell is not empty
			//if ( !isBitSet ( readOnlyColumnsMask, i_col ) ) { // and it is a cell that can be emptied
			return false; // ergo, row is not clear
			//}
		}
	}
	return true;
}

/* The table defaults to selection by item instead of by row so that one can deal with the cells individually,
but more than often we need the data from the row and we use a single selected item to account for the whole row
*/
bool vmTableWidget::isRowSelected ( const uint row ) const
{
	for ( uint i_col = 0; i_col < colCount (); ++i_col ) {
		if ( sheetItem ( row, i_col )->isSelected () )
			return true;
	}
	return false;
}

// This does the same both above do but in one single loop
bool vmTableWidget::isRowSelectedAndNotEmpty ( const uint row ) const
{
	vmTableItem* item ( nullptr );
	for ( uint i_col = 0; i_col < colCount (); ++i_col ) {
		item = sheetItem ( row, i_col );
		if ( item->isSelected () ) {
			if ( !item->text ().isEmpty () )
				return ( !isBitSet ( readOnlyColumnsMask, i_col ) );
		}
	}
	return false;
}

void vmTableWidget::setTableUpdated ()
{
	modifiedRows.clearButKeepMemory ();
	mTableChanged = false;
	setProperty ( PROPERTY_TABLE_HAS_ITEM_TO_REGISTER, false );
	// Accept user input and save data for newer usage
	for ( uint i_row ( 0 ); ( signed )i_row <= m_lastUsedRow; ++i_row ) {
		for ( uint i_col = 0; i_col <= ( colCount () - 1 ); ++i_col )
			sheetItem ( i_row, i_col )->syncOriginalTextWithCurrent ();
	}
}

void vmTableWidget::setHiddenText ( const uint row, const uint col, const QString& str, const bool notify_change )
{
	vmTableItem* item ( sheetItem ( row, col ) );
	if ( item != nullptr ) {
		item->setData ( Qt::WhatsThisRole, str );
		if ( notify_change )
			cellContentChanged ( item );
	}
}

QString vmTableWidget::hiddenText ( const uint row, const uint col ) const
{
	const vmTableItem* item ( sheetItem ( row, col ) );
	return ( ( item != nullptr ) ? item->data ( Qt::WhatsThisRole ).toString () : emptyString );
}

void vmTableWidget::setColumnReadOnly ( const uint i_col, const bool readonly )
{
	if ( i_col < colCount () ) {
		readonly ? setBit ( readOnlyColumnsMask, i_col ) :
		unSetBit ( readOnlyColumnsMask, i_col );
	}
}

int vmTableWidget::isCellMonitored ( const vmTableItem* const item )
{
	if ( item == nullptr )
		return -2; // a null item is to be ignored, obviously. -2 will not be monitored
	for ( uint i = 0; i < mMonitoredCells.count (); ++i ) {
		if ( mMonitoredCells.at ( i ) == item )
			return i;
	}
	return -1; //-1 not monitored
}

//todo: monitoredCellChanged_func gets called before the item's text is updated, so we get the old text instead of the new one
void vmTableWidget::insertMonitoredCell ( const uint row, const uint col )
{
	const vmTableItem* item ( this->sheetItem ( row, col ) );
	if ( isCellMonitored ( item ) == -1 ) {
		mMonitoredCells.append ( item );
		/* Most likely, a monitored cell will be read only, and display the result of some formula dependant on changes happening
		 * on other cells. setCellWidget is the method responsible for setting the callbacks for altered contents, but will not
		 * do so on read only cells. Therefore, we do this here only for cells that matter. We do not need to change the text of
		 * widget because vmTableItem already does it, and, in fact, monitoredCellChanged_func will be called by the setText method
		 * of widget which, in turn, is called by vmTableItem::setText
		*/
		if ( isBitSet ( readOnlyColumnsMask, col ) )
			item->widget ()->setCallbackForContentsAltered ( [&] ( const vmWidget* const widget ) {
				return monitoredCellChanged_func ( widget->ownerItem () );
		} );
	}
}

void vmTableWidget::removeMonitoredCell ( const uint row, const uint col )
{
	const int pos ( isCellMonitored ( sheetItem ( row, col ) ) );
	if ( pos >= 0 )
		mMonitoredCells.remove ( pos );
}

void vmTableWidget::setCellColor ( const uint row, const uint col, const Qt::GlobalColor color )
{
	if ( col < colCount () )
		sheetItem ( row, col )->setData ( Qt::BackgroundRole, QBrush ( color ) );
}

void vmTableWidget::highlight ( const VMColors color, const QString& text )
{
	if ( color != vmDefault_Color ) {
		vmTableItem* item ( nullptr );
		uint i_col ( 0 );
		for ( uint i_row ( 0 ); ( signed )i_row <= m_lastUsedRow; ++i_row ) {
			for ( i_col = 0; i_col < colCount (); ++i_col ) {
				if ( sheetItem ( i_row, i_col )->text ().contains ( text ) ) {
					item = sheetItem ( i_row, i_col );
					item->highlight ( color );
					m_highlightedCells.append ( item );
				}
			}
		}
	}
	else {
		for ( uint i ( 0 ); i < m_highlightedCells.count (); ++i )
			m_highlightedCells.at ( i )->highlight ( vmDefault_Color );
		m_highlightedCells.clearButKeepMemory ();
	}
}

void vmTableWidget::highlightCell ( const int row, const int col, const VMColors color )
{
	sheetItem ( row, col )->highlight ( color );
}

void vmTableWidget::unHighlightCell ( const int row, const int col )
{
	sheetItem ( row, col )->highlight ( vmDefault_Color );
}

inline bool vmTableWidget::isColumnSelectedForSearch ( const uint column ) const
{
	return column < colCount () ? static_cast<vmCheckedTableItem*> ( horizontalHeader () )->isChecked ( column ) : false;
}

void vmTableWidget::setColumnSearchStatus ( const uint column, const bool bsearch )
{
	if ( column < colCount () )
		static_cast<vmCheckedTableItem*> ( horizontalHeader () )->setChecked ( column, bsearch );
}

void vmTableWidget::reHilightItems ( vmTableItem* next, vmTableItem* prev )
{
	if ( prev )
		prev->setBackground ( QBrush () );
	if ( next ) {
		next->setBackground ( QBrush ( Qt::yellow ) );
		scrollToItem ( next, QAbstractItemView::PositionAtTop );
	}
}

void vmTableWidget::mousePressEvent ( QMouseEvent* e )
{
	if ( e->button () & Qt::RightButton ) {
		mContextMenuCell = static_cast<vmTableItem*> ( itemAt ( e->pos () ) );
		if ( mContextMenuCell ) {
			enableOrDisableActionsForCell ( mContextMenuCell );
			e->accept();
			mContextMenu->exec ( e->globalPos () );
			mContextMenuCell = nullptr;
			return;
		}
	}
	e->ignore ();
	QAbstractItemView::mousePressEvent ( e );
}

void vmTableWidget::keyPressEvent ( QKeyEvent* k )
{
	const bool ctrlKey ( k->modifiers () & Qt::ControlModifier );
	if ( isEditable () ) {
		if ( ctrlKey ) {
			switch ( k->key () ) {
				case Qt::Key_Enter:
				case Qt::Key_Return:
					if ( keypressed_func )
						keypressed_func ( k, this );
				break;
				case Qt::Key_F:
					if ( !m_searchPanel )
						m_searchPanel = new vmTableSearchPanel ( this );
					showSearchPanel ();
				break;
				case Qt::Key_I:
					insertRow_slot ();
				break;
				case Qt::Key_O:
					appendRow_slot ();
				break;
				case Qt::Key_R:
					removeRow_slot ();
				break;
				case Qt::Key_D:
					clearRow_slot ();
				break;
				case Qt::Key_C:
					copyCellContents ();
				break;
				case Qt::Key_B:
					copyRowContents ();
				break;
				case Qt::Key_Z:
					undoChange ();
				break;
				default:
					k->setAccepted ( false );
				return;
			}
		}
		else { // no CTRL modifier
			switch ( k->key () ) {
				case Qt::Key_Escape:
					if ( m_searchPanel && m_searchPanel->isVisible () ) {
						hideSearchPanel ();
						return;
					}
				case Qt::Key_Enter:
				case Qt::Key_Return:
					if ( keypressed_func )
						keypressed_func ( k, this );
				break;
				case Qt::Key_F2:
				case Qt::Key_F3:
				case Qt::Key_F4:
					if ( m_searchPanel && m_searchPanel->isVisible () ) {
						QApplication::sendEvent ( m_searchPanel, k ); // m_searchPanel will propagate the event its children
						QApplication::sendPostedEvents ( m_searchPanel, 0 ); // force event to be processed before posting another one to que queue
					}
				break;
				default:
				break;
			}
			QTableWidget::keyPressEvent ( k );
		}
	}
	else {
		if ( ctrlKey && k->key () == Qt::Key_F ) {
			if ( !m_searchPanel )
				m_searchPanel = new vmTableSearchPanel ( this );
			showSearchPanel ();
			return;
		}
		else if ( k->key () == Qt::Key_Escape ) {
			if ( m_searchPanel && m_searchPanel->isVisible () ) {
				hideSearchPanel ();
				return;
			}
		}
		QTableWidget::keyPressEvent ( k );
	}
}

void vmTableWidget::enableOrDisableActionsForCell ( const vmTableItem* sheetItem )
{
	mUndoAction->setEnabled ( sheetItem->cellIsAltered () );
	mCopyCellAction->setEnabled ( !isRowEmpty ( sheetItem->row () ) );
	mCopyRowContents->setEnabled ( !isRowEmpty ( sheetItem->row () ) );
	mAppendRowAction->setEnabled ( sheetItem->isEditable () );
	mInsertRowAction->setEnabled ( sheetItem->isEditable () );
	mDeleteRowAction->setEnabled ( sheetItem->isEditable () );
	mClearRowAction->setEnabled ( sheetItem->isEditable () );
	mClearTableAction->setEnabled ( sheetItem->isEditable () );
	mSubMenuFormula->setEnabled ( sheetItem->isEditable () );
	if ( this->isEditable () ) { //even if the item is not editable, the formula might be changed
		if ( mOverrideFormulaAction == nullptr ) {
			mFormulaTitleAction = new vmAction ( FORMULA_TITLE );
			mOverrideFormulaAction = new vmAction ( SET_FORMULA_OVERRIDE, tr ( "Allow formula override" ) );
			mOverrideFormulaAction->setCheckable ( true );
			connect ( mOverrideFormulaAction, &vmAction::triggered, this, [&, sheetItem ] ( const bool checked ) {
						return setFormulaOverrideForCell ( const_cast<vmTableItem*>( sheetItem ), checked ); } );
			mSetFormulaAction = new vmAction ( SET_FORMULA, tr ( "Set formula" ) );
			connect ( mSetFormulaAction, &vmAction::triggered, this, [&, sheetItem ] () {
						return setFormulaForCell ( const_cast<vmTableItem*>( sheetItem ) ); } );
			mSubMenuFormula->addAction ( mFormulaTitleAction );
			mSubMenuFormula->addSeparator ();
			mSubMenuFormula->addAction ( mOverrideFormulaAction );
			mSubMenuFormula->addAction ( mSetFormulaAction );
		}
		mFormulaTitleAction->QAction::setText ( sheetItem->hasFormula () ? sheetItem->formula () : tr ( "No formula" ) );
		mOverrideFormulaAction->setEnabled ( sheetItem->hasFormula () );
		mOverrideFormulaAction->setChecked ( sheetItem->formulaOverride () );
	}
}

void vmTableWidget::sharedContructorsCode ()
{
	setWidgetPtr ( static_cast<QWidget*> ( this ) );
	setSelectionMode ( QAbstractItemView::ExtendedSelection );
	setSelectionBehavior ( QAbstractItemView::SelectItems );
	setAlternatingRowColors ( true );
	setAttribute ( Qt::WA_KeyCompression );
	setFrameShape ( QFrame::NoFrame );
	setFrameShadow ( QFrame::Plain );

	setEditable ( false );

	if ( vmTableWidget::defaultBGColor.isEmpty () )
		vmTableWidget::defaultBGColor = palette ().color ( backgroundRole () ).name ();

	mUndoAction = new vmAction ( UNDO, tr ( "Undo change (CTRL+Z)" ), this );
	connect ( mUndoAction, &vmAction::triggered, this, [&] () {
				return undoChange (); } );

	mCopyCellAction = new vmAction ( COPY_CELL, tr ( "Copy cell contents to clipboard (CTRL+C)" ), this );
	connect ( mCopyCellAction, &vmAction::triggered, this, [&] () {
				return copyCellContents (); } );

	mCopyRowContents = new vmAction ( COPY_ROW, tr ( "Copy row contents to clipboard (CTRL+B)" ), this );
	connect ( mCopyRowContents, &vmAction::triggered, this, [&] () {
				return copyRowContents (); } );

	mInsertRowAction = new vmAction ( ADD_ROW, tr ( "Insert line here (CTRL+I)" ), this );
	connect ( mInsertRowAction, &vmAction::triggered, this, [&] () {
				return insertRow_slot (); } );

	mAppendRowAction = new vmAction ( APPEND_ROW, tr ( "Append line (CTRL+O)" ), this );
	connect ( mAppendRowAction, &vmAction::triggered, this, [&] () {
				return appendRow_slot (); } );

	mDeleteRowAction = new vmAction ( DEL_ROW, tr ( "Remove this line (CTRL+R)" ), this );
	connect ( mDeleteRowAction, &vmAction::triggered, this, [&] () {
				return removeRow_slot (); } );

	mClearRowAction = new vmAction ( CLEAR_ROW, tr ( "Clear line (CTRL+D)" ), this );
	connect ( mClearRowAction, &vmAction::triggered, this, [&] () {
				return clearRow_slot (); } );

	mClearTableAction = new vmAction ( CLEAR_TABLE, tr ( "Clear table (CTRL+T)" ), this );
	connect ( mClearTableAction, &vmAction::triggered, this, [&] () {
				return clearTable_slot (); } );

	mSubMenuFormula = new QMenu ( QStringLiteral ( "Formula" ) );

	mContextMenu = new QMenu ( this );
	mContextMenu->addAction ( mUndoAction );
	mContextMenu->addSeparator ();
	mContextMenu->addAction ( mCopyCellAction );
	mContextMenu->addAction ( mCopyRowContents );
	mContextMenu->addSeparator ();
	mContextMenu->addAction ( mAppendRowAction );
	mContextMenu->addAction ( mInsertRowAction );
	mContextMenu->addAction ( mDeleteRowAction );
	mContextMenu->addAction ( mClearRowAction );
	mContextMenu->addAction ( mClearTableAction );
	mContextMenu->addSeparator ();
	mContextMenu->addMenu ( mSubMenuFormula );
}

void vmTableWidget::fixTotalsRow ()
{
	const QString totalsRowStr ( QString::number ( mTotalsRow - 1 ) );
	vmTableItem* sheet_item ( nullptr );
	for ( uint i_col ( 0 ); i_col < colCount (); ++i_col ) {
		if ( sheetItem ( mTotalsRow, i_col )->textType () >= vmLineEdit::TT_PRICE ) {
			sheet_item = sheetItem ( mTotalsRow, i_col );
			sheet_item->setFormula ( sheet_item->formulaTemplate (), totalsRowStr );
		}
	}
}

void vmTableWidget::reScanItem ( vmTableItem* const sheet_item )
{
	if ( m_itemsToReScan.contains ( sheet_item ) == -1 )
		m_itemsToReScan.append ( sheet_item );
}

void vmTableWidget::interceptCompleterActivated ( const QModelIndex& index, const vmTableWidget* const table )
{
	if ( table != this )
		return;

	mbDoNotUpdateCompleters = true; //avoid a call to update the item's completer because we are retrieving the values from the completer itself
	const stringRecord record ( APP_COMPLETERS ()->getCompleter (
									vmCompleters::PRODUCT_OR_SERVICE )->completionModel ()->data (
									index.sibling ( index.row (), 1 ) ).toString () );


	if ( record.isOK () ) {
		if ( currentRow () == -1 )
			setCurrentCell ( 1, 0, QItemSelectionModel::Select );
		const int current_row ( currentRow () );

		if ( record.first () ) {
			sheetItem ( current_row, 0 )->setText ( record.curValue (), false, true );
			uint i_col ( 1 );
			do {
				if ( record.next () )
					sheetItem ( current_row, i_col++ )->setText ( record.curValue (), false, true );
				else
					break;
			} while ( i_col < ISR_TOTAL_PRICE );
		}
		setCurrentCell ( current_row, ISR_QUANTITY, QItemSelectionModel::ClearAndSelect );
	}
	mbDoNotUpdateCompleters = false;
}

void vmTableWidget::undoChange ()
{
	if ( isEditable () ) {
		vmTableItem* item ( nullptr );
		if ( mContextMenuCell )
			item = mContextMenuCell;
		else
			item = static_cast<vmTableItem*> ( currentItem () );
		if ( item && item->cellIsAltered () )
			item->setText ( item->prevText (), false, true );
	}
}

void vmTableWidget::copyCellContents ()
{
	QString cell_text;
	if ( mContextMenuCell != nullptr )
		cell_text = mContextMenuCell->text ();
	else {
		const QModelIndexList selIndexes ( selectedIndexes () );
		if ( selIndexes.count () > 0 ) {
			QModelIndexList::const_iterator itr = selIndexes.constBegin ();
			const QModelIndexList::const_iterator itr_end = selIndexes.constEnd ();
			for ( ; itr != itr_end; ++itr ) {
				cell_text.append ( sheetItem ( static_cast<QModelIndex> ( *itr ).row (),
											   static_cast<QModelIndex> ( *itr ).column () )->text () );
				cell_text.append ( CHR_NEWLINE );
			}
			//cell_text.truncate ( cell_text.length () - 1 ); // remove last newline character
			cell_text.chop ( 1 );
		}
	}
	Data::copyToClipboard ( cell_text );
}

void vmTableWidget::copyRowContents ()
{
	QString rowText;
	uint i_col ( 0 );
	if ( mContextMenuCell != nullptr ) {
		for ( ; i_col < colCount (); ++i_col )
			rowText += sheetItem ( mContextMenuCell->row (), i_col )->text () + QLatin1Char ( '\t' );
		//rowText.truncate ( rowText.length () - 1 ); // remove last tab character
		rowText.chop ( 1 );
	}
	else {
		const QModelIndexList selIndexes ( selectedIndexes () );
		if ( selIndexes.count () > 0 ) {
			QModelIndexList::const_iterator itr = selIndexes.constBegin ();
			const QModelIndexList::const_iterator itr_end = selIndexes.constEnd ();
			for ( ; itr != itr_end; ++itr ) {
				for ( i_col = 0; i_col < colCount (); ++i_col )
					rowText += sheetItem ( static_cast<QModelIndex> ( *itr ).row (), i_col )->text () + QLatin1Char ( '\t' );
				//rowText.truncate ( rowText.length () - 1 ); // remove last tab character
				rowText.chop ( 1 );
				rowText += CHR_NEWLINE;
			}
			//rowText.truncate ( rowText.length () - 1 ); // remove last new line character
			rowText.chop ( 1 );
		}
	}

	Data::copyToClipboard ( rowText );
}

// Inserts one or more lines ( depending on how many are selected - i.e. two selected, in whichever order, inserts two lines )
void vmTableWidget::insertRow_slot ()
{
	if ( mContextMenuCell != nullptr )
		insertRow ( mContextMenuCell->row () );
	else {
		const QModelIndexList selIndexes ( selectedIndexes () );
		if ( selIndexes.count () > 0 ) {
			QModelIndexList::const_iterator itr = selIndexes.constBegin ();
			const QModelIndexList::const_iterator itr_end = selIndexes.constEnd ();
			for ( ; itr != itr_end; ++itr )
				insertRow ( static_cast<QModelIndex> ( *itr ).row () + 1 );
		}
	}
}

void vmTableWidget::appendRow_slot ()
{
	appendRow ();
}

void vmTableWidget::removeRow_slot ()
{
	if ( mContextMenuCell && rowRemoved_func ) {
		rowRemoved_func ( mContextMenuCell->row () );
		removeRow ( mContextMenuCell->row () );
	}
	else {
		const QModelIndexList selIndexes ( selectedIndexes () );
		if ( selIndexes.count () > 0 ) {
			// Move from last selected toward the first to avoid conflicts and errors
			QModelIndexList::const_iterator itr = selIndexes.constEnd ();
			const QModelIndexList::const_iterator itr_begin = selIndexes.constBegin ();
			for ( --itr; ; --itr ) {
				// Emit the signal before removing the row so that the slot(s) that catch the signal can retrieve information from the row
				// (i.e. an ID, or name or anything to be used by the database ) before it disappears
				if ( rowRemoved_func )
					rowRemoved_func ( static_cast<QModelIndex> ( *itr ).row () );
				removeRow ( static_cast<QModelIndex> ( *itr ).row () );
				if ( itr == itr_begin )
					break;
			}
		}
	}
}

void vmTableWidget::clearRow_slot ()
{
	if ( isEditable () ) {
		if ( mContextMenuCell != nullptr )
			clearRow ( mContextMenuCell->row () );
		else {
			const QModelIndexList selIndexes ( selectedIndexes () );
			if ( selIndexes.count () > 0 ) {
				QModelIndexList::const_iterator itr = selIndexes.constBegin ();
				const QModelIndexList::const_iterator itr_end = selIndexes.constEnd ();
				for ( ; itr != itr_end; ++itr )
					clearRow ( static_cast<QModelIndex> ( *itr ).row () );
			}
		}
	}
}

void vmTableWidget::clearTable_slot ()
{
	if ( isEditable () )
		clear ( true );
}

void vmTableWidget::setFormulaOverrideForCell ( vmTableItem* item, const bool boverride )
{
	const bool bWasOverride ( item->formulaOverride () );
	item->setFormulaOverride ( boverride );
	if ( bWasOverride )
		item->computeFormula (); // change cell value to that of the formula
	item->setEditable ( true ); //temporarily editable
}

void vmTableWidget::setFormulaForCell ( vmTableItem* item )
{
	QString strNewFormula;
	if ( vmNotify::inputBox ( strNewFormula, this, tr ( "New Formula" ), tr ( "Set new cell formula" ),
							  item->formula () ) ) {
		if ( item->setCustomFormula ( strNewFormula ) )
			mFormulaTitleAction->QAction::setText ( item->formula () );
	}
}

void vmTableWidget::headerItemToggled ( const uint col, const bool checked )
{
	qDebug () << "Column " << col << " is now " << ( checked ? "checked." : "unchecked." );
}

void vmTableWidget::textWidgetChanged ( const vmWidget* const sender )
{
	vmTableItem* const item ( const_cast<vmTableItem*> ( sender->ownerItem () ) );
	item->setText ( sender->text (), true, false );
	if ( !mbDoNotUpdateCompleters ) {
		// Update the runtime completers to make the entered text available for the current session, if not already in the model
		// The runtime completer will, in turn, update the database. TODO: not only for the completers table, but all tables: a
		// way to alter the data manually
		APP_COMPLETERS ()->updateCompleter ( sender->text (), item->completerType () );
	}
	cellContentChanged ( item );
}

void vmTableWidget::dateWidgetChanged ( const vmWidget* const sender )
{
	const_cast<vmTableItem*> ( sender->ownerItem () )->setText (
		static_cast<const vmDateEdit* const>( sender )->date ().toString ( DATE_FORMAT_DB ), true, false );
	cellContentChanged ( sender->ownerItem () );
}

void vmTableWidget::timeWidgetChanged ( const vmWidget* const sender )
{
	const_cast<vmTableItem*> ( sender->ownerItem () )->setText (
		static_cast<const vmTimeEdit* const>( sender )->time ().toString ( TIME_FORMAT_DEFAULT ), true, false );
	cellContentChanged ( sender->ownerItem () );
}

void vmTableWidget::checkBoxToggled ( const vmWidget* const sender )
{
	const_cast<vmTableItem*> ( sender->ownerItem () )->setText ( sender->text (), true, false );
	cellContentChanged ( sender->ownerItem () );
}

void vmTableWidget::displayContextMenuForCell ( const QPoint& pos, const vmWidget* cellWidget )
{
	if ( cellWidget->ownerItem () ) {
		mContextMenuCell = const_cast<vmTableItem*> ( cellWidget->ownerItem () );
		enableOrDisableActionsForCell ( mContextMenuCell );
		Data::execMenuWithinWidget ( mContextMenu, cellWidget->toQWidget (), pos );
		mContextMenuCell = nullptr;
	}
}

void vmTableWidget::cellWidgetRelevantKeyPressed ( const QKeyEvent* const ke, const vmWidget* const widget )
{
	if ( ke->key () == Qt::Key_Escape ) {
		/* The text is different from the original, canceling (pressing the ESC key) should return
		 * the widget's text to its original
		 */
		vmTableItem* item ( const_cast<vmTableItem*> ( widget->ownerItem () ) );
		if ( item->text () != item->originalText () ) {
			item->setText ( item->originalText (), false, false );
			item->setCellIsAltered ( false );
		}
		/* If text is the same the item had when it went from non-editable to editable,
		 * propagate the signal to the table so that it can do the appropriate cancel editing routine
		 */
		else
			keyPressEvent ( const_cast<QKeyEvent*> ( ke ) );
	}
}
//---------------------------------------------TABLE WIDGET---------------------------------------------------

//---------------------------------------------SEARCH-PANEL---------------------------------------------------
vmTableSearchPanel::vmTableSearchPanel ( vmTableWidget* table )
	: QFrame ( nullptr ), m_SearchedWord ( emptyString ), m_table ( table ),
	  btnSearchNext ( nullptr ), btnSearchPrev ( nullptr ), btnSearchCancel ( nullptr )
{
	txtSearchTerm = new vmLineEdit;
	txtSearchTerm->setEditable ( true );
	txtSearchTerm->setCallbackForRelevantKeyPressed ( ( [&]( const QKeyEvent* ke, const vmWidget* const ) {
		return txtSearchTerm_keyPressed ( ke ); } ) );
	txtSearchTerm->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
		return searchTextChanged ( sender->text () ); } );

	btnSearchStart = new QToolButton;
	btnSearchStart->setIcon ( ICON ( "search.png" ) );
	btnSearchStart->setEnabled ( false );
	connect ( btnSearchStart, &QToolButton::clicked, this, [&] () { return searchStart (); } );

	btnSearchPrev = new QToolButton;
	btnSearchPrev->setIcon ( ICON ( "arrow-left.png" ) );
	btnSearchPrev->setEnabled ( false );
	connect ( btnSearchPrev, &QToolButton::clicked, this, [&] () { return searchPrev (); } );

	btnSearchNext = new QToolButton;
	btnSearchNext->setIcon ( ICON ( "arrow-right.png" ) );
	btnSearchNext->setEnabled ( false );
	connect ( btnSearchNext, &QToolButton::clicked, this, [&] () { return searchNext (); } );

	btnSearchCancel = new QToolButton;
	btnSearchCancel->setIcon ( ICON ( "cancel.png" ) );
	connect ( btnSearchCancel, &QToolButton::clicked, this, [&] () { return searchCancel (); } );

	chkSearchAllTable = new vmCheckBox ( tr ( "Search all fields" ) );
	chkSearchAllTable->setChecked ( true );
	chkSearchAllTable->setEditable ( true );
	chkSearchAllTable->setCallbackForContentsAltered ( [&] ( const vmWidget* const ) { return searchFieldsChanged (); } );

	QHBoxLayout* mLayout ( new QHBoxLayout );
	mLayout->setSpacing( 2 );
	mLayout->setMargin ( 2 );
	mLayout->addWidget ( txtSearchTerm, 3 );
	mLayout->addWidget ( btnSearchStart );
	mLayout->addWidget ( btnSearchPrev );
	mLayout->addWidget ( btnSearchNext );
	mLayout->addWidget ( btnSearchCancel );
	mLayout->addStretch ( 2 );
	mLayout->addWidget ( chkSearchAllTable, 0, Qt::AlignRight );
	setLayout ( mLayout );
}

void vmTableSearchPanel::showPanel ()
{
	show ();
	txtSearchTerm->setFocus ();
}

void vmTableSearchPanel::searchFieldsChanged ( const vmCheckBox* const )
{
	if ( chkSearchAllTable->isChecked () )
		static_cast<vmCheckedTableItem*> ( m_table->horizontalHeader () )->setCheckable ( false );
	else {
		static_cast<vmCheckedTableItem*> ( m_table->horizontalHeader () )->setCheckable ( true );
		for ( uint col ( 0 ); col < m_table->colCount (); ++col )
			static_cast<vmCheckedTableItem*> ( m_table->horizontalHeader () )->setChecked ( col, false );
	}
}

bool vmTableSearchPanel::searchStart ()
{
	m_table->searchStart ( txtSearchTerm->text () );
	const bool b_result ( m_table->searchFirst () );
	btnSearchStart->setEnabled ( !b_result );
	btnSearchNext->setEnabled ( b_result );
	btnSearchPrev->setEnabled ( !b_result );
	return b_result;
}

bool vmTableSearchPanel::searchNext ()
{
	const bool b_result ( m_table->searchNext () );
	btnSearchNext->setEnabled ( b_result );
	btnSearchPrev->setEnabled ( btnSearchPrev->isEnabled () | b_result );
	return b_result;
}

bool vmTableSearchPanel::searchPrev ()
{
	const bool b_result ( m_table->searchPrev () );
	btnSearchNext->setEnabled ( btnSearchNext->isEnabled () | b_result );
	btnSearchPrev->setEnabled ( b_result );
	return b_result;
}

void vmTableSearchPanel::searchCancel ()
{
	m_table->searchCancel ();
	btnSearchNext->setEnabled ( false );
	btnSearchPrev->setEnabled ( false );
	txtSearchTerm->clear ();
	m_table->hideSearchPanel ();
}

void vmTableSearchPanel::searchTextChanged ( const QString& text )
{
	if ( !text.isEmpty () ) {
		const bool b_newsearch ( text != m_table->searchTerm () );
		btnSearchStart->setEnabled ( b_newsearch );
		btnSearchPrev->setEnabled ( false );
		btnSearchNext->setEnabled ( false );
	}
}

void vmTableSearchPanel::txtSearchTerm_keyPressed ( const QKeyEvent* ke )
{
	switch ( ke->key () ) {
		case Qt::Key_Enter:
		case Qt::Key_Return:
			searchStart ();
		break;
		case Qt::Key_Escape:
			if ( txtSearchTerm->text () != m_table->searchTerm () ) {
				txtSearchTerm->setText ( m_table->searchTerm () );
				btnSearchStart->setEnabled ( !txtSearchTerm->text ().isEmpty () );
			}
			else {
				hide ();
				m_table->setFocus ();
			}
		break;
		case Qt::Key_F2:
			searchPrev ();
		break;
		case Qt::Key_F3:
			searchNext ();
		break;
		default:
		break;
	}
}
//---------------------------------------------SEARCH-PANEL---------------------------------------------------
