#include "vmtablewidget.h"
#include "global.h"
#include "vmwidgets.h"
#include "textdb.h"
#include "data.h"
#include "stringrecord.h"
#include "vmwidgets.h"
#include "heapmanager.h"
#include "vmcheckedtableitem.h"
#include "vmtableutils.h"
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
//---------------------------------------------STATIC---------------------------------------------------------

vmTableWidget::vmTableWidget ( QWidget* parent )
	: QTableWidget ( parent ), vmWidget ( WT_TABLE ),
	  mCols ( nullptr ), mTotalsRow ( -1 ), m_lastUsedRow ( -1 ), m_nVisibleRows ( 0 ), m_ncols ( 0 ), mAddedItems ( 0 ),
	  mTableChanged ( false ), mbKeepModRec ( true ), mbPlainTable ( false ), mbTableIsList ( false ), 
	  mIsPurchaseTable ( false ), mbDoNotUpdateCompleters ( false ), readOnlyColumnsMask ( 0 ), modifiedRows ( 0, 10 ),
	  mMonitoredCells ( 2 ), m_highlightedCells ( 5 ), m_itemsToReScan ( 10 ), mSearchList ( 10 ),
	  mContextMenu ( nullptr ), mOverrideFormulaAction ( nullptr ), mSetFormulaAction ( nullptr ),
	  mFormulaTitleAction ( nullptr ), mParentLayout ( nullptr ), m_searchPanel ( nullptr ), m_filterPanel ( nullptr ),
	  mContextMenuCell ( nullptr ), cellChanged_func ( nullptr ), cellNavigation_func ( nullptr ),
	  monitoredCellChanged_func ( nullptr ), rowRemoved_func ( nullptr ), rowActivated_func ( nullptr )
{
	sharedContructorsCode ();
}

vmTableWidget::vmTableWidget ( const uint rows, QWidget* parent )
	: QTableWidget ( static_cast<int>(rows) + 1, 0, parent ), vmWidget ( WT_TABLE, static_cast<int>(rows) ),
	  mCols ( nullptr ), mTotalsRow ( -1 ), m_lastUsedRow ( -1 ), m_nVisibleRows ( 0 ), m_ncols ( 0 ), mAddedItems ( 0 ),
	  mTableChanged ( false ), mbKeepModRec ( true ), mbPlainTable ( false ), mbTableIsList ( false ), 
	  mIsPurchaseTable ( false ), mbDoNotUpdateCompleters ( false ), readOnlyColumnsMask ( 0 ), modifiedRows ( 0, 10 ),
	  mMonitoredCells ( 2 ), m_highlightedCells ( 5 ), m_itemsToReScan ( rows + 1 ), mSearchList ( 10 ),
	  mContextMenu ( nullptr ), mOverrideFormulaAction ( nullptr ), mSetFormulaAction ( nullptr ),
	  mFormulaTitleAction ( nullptr ), mParentLayout ( nullptr ), m_searchPanel ( nullptr ), m_filterPanel ( nullptr ),
	  mContextMenuCell ( nullptr ), cellChanged_func ( nullptr ), cellNavigation_func ( nullptr ),
	  monitoredCellChanged_func ( nullptr ), rowRemoved_func ( nullptr ), rowActivated_func ( nullptr )
{
	sharedContructorsCode ();
	initTable ( rows );
}

vmTableWidget::~vmTableWidget ()
{
	disconnect ( this, nullptr, nullptr, nullptr );
	heap_dellarr ( mCols );
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
	heap_del ( m_searchPanel );
	heap_del ( m_filterPanel );
}

vmTableColumn* vmTableWidget::createColumns ( const uint nCols )
{
	mCols = new vmTableColumn[nCols];
	m_ncols = nCols;
	return mCols;
}

void vmTableWidget::initTable ( const uint rows )
{
	setUpdatesEnabled ( false );
	uint i_col ( 0 );
	uint i_row ( 0 );

	vmTableColumn* column ( nullptr );
	vmTableItem* sheet_item ( nullptr );
	QString col_header;

	mTotalsRow = static_cast<int>( rows );
	m_nVisibleRows = rows + static_cast<uint>( !mbPlainTable );
	setRowCount ( static_cast<int>( m_nVisibleRows ) );

	if ( !isList () )
	{
		QFont titleFont ( font () );
		titleFont.setBold ( true );
		vmCheckedTableItem* headerItem ( new vmCheckedTableItem ( Qt::Horizontal, this ) );
		headerItem->setCallbackForCheckStateChange ( [&] ( const uint col, const bool checked ) {
			return headerItemToggled ( col, checked ); } );
		setHorizontalHeader ( headerItem );
	}
	else
		horizontalHeader ()->setVisible ( false );

	do
	{
		column = &mCols[i_col];
		insertColumn ( static_cast<int>( i_col ) );

		if ( !column->editable )
			setBit ( readOnlyColumnsMask, static_cast<uchar>( i_col ) ); // bit is set? read only cell : cell can be cleared or other actions

		if ( !isList () )
		{
			for ( i_row = 0; i_row < m_nVisibleRows; ++i_row )
			{
				if ( i_row != static_cast<uint>( totalsRow () ) )
					sheet_item = new vmTableItem ( column->wtype, column->text_type, column->default_value, this );
				else
					sheet_item = new vmTableItem ( WT_LINEEDIT, i_col != 0 ? column->text_type : vmWidget::TT_TEXT, emptyString, this );
			
				setItem ( static_cast<int>( i_row ), static_cast<int>( i_col ), sheet_item );
				sheet_item->setButtonType ( column->button_type );
				sheet_item->setCompleterType ( column->completer_type );
				setCellWidget ( sheet_item );
				sheet_item->setEditable ( false );
				if ( !mbPlainTable )
				{
					if ( i_row == static_cast<uint>( totalsRow () ) )
					{
						if ( i_col == 0 )
							sheet_item->setText ( TR_FUNC ( "Total:" ), false, false );
						else
						{
							if ( column->text_type >= vmLineEdit::TT_PRICE )
							{
								col_header = QChar ( 'A' + i_col );
								sheet_item->setFormula ( QLatin1String ( "sum " ) + col_header + CHR_ZERO + CHR_SPACE + col_header + 
														 QLatin1String ( "%1" ), QString::number ( mTotalsRow - 1 ) );
							}
						}
					}
					else
					{
						if ( !column->formula.isEmpty () )
							sheet_item->setFormula ( column->formula, QString::number ( i_row ) );
					}
				}
			}
			setHorizontalHeaderItem ( static_cast<int>( i_col ), new vmTableItem ( WT_TABLE_ITEM, vmLineEdit::TT_TEXT, column->label, this ) );
		}
		
		uint colWidth ( column->width );
		if ( colWidth == 0 )
		{
			switch ( column->wtype )
			{
				case WT_DATEEDIT:
					colWidth = 180;
				break;
				case WT_LINEEDIT:
					colWidth = ( column->text_type >= vmWidget::TT_PHONE ? 130 : 100 );
				break;
				case WT_LINEEDIT_WITH_BUTTON:
					colWidth = 150;
				break;
				default:
					colWidth = 100;
				break;
			}
		}
		setColumnWidth ( static_cast<int>( i_col ), static_cast<int>( colWidth ) );
		++i_col;
	}
	while ( i_col < m_ncols );
	
	//All the items that could not have their formula calculated at the time of creation will now
	for ( i_row = 0; i_row < m_itemsToReScan.count (); ++i_row )
		m_itemsToReScan.at ( i_row )->targetsFromFormula ();
	
	setUpdatesEnabled ( true );
}

bool vmTableWidget::selectFound ( const vmTableItem* item )
{
	if ( item != nullptr )
	{
		scrollToItem ( item, QAbstractItemView::PositionAtCenter );
		//setCurrentItem ( const_cast<vmTableItem*>( item ), QItemSelectionModel::ClearAndSelect );
		//selectRow ( item->row () );
		return true;
	}
	return false;
}

// Assumes searchTerm is valid. It's a private function for that reason: only friend
bool vmTableWidget::searchStart ( const QString& searchTerm )
{
	if ( searchTerm.count () >= 2 && searchTerm != mSearchTerm )
	{
		searchCancel ();

		uint i_row ( 0 ), i_col ( 0 );
		uint max_row ( lastUsedRow () >= 0 ? static_cast<uint>(lastUsedRow ()) + 1 : 0 );
		uint max_col ( isList () ? 1 : colCount () );
		vmTableItem* item ( nullptr );
		mSearchTerm = searchTerm;

		for ( ; i_row < max_row; ++i_row )
		{
			for ( i_col = 0 ; i_col < max_col; ++i_col )
			{
				if ( isColumnSelectedForSearch ( i_col ) )
				{
					item = sheetItem ( i_row, i_col );
					if ( item->text ().contains ( searchTerm, Qt::CaseInsensitive ) )
					{
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
	if ( horizontalHeader () )
		static_cast<vmCheckedTableItem*>( horizontalHeader () )->setCheckable ( false );
	mSearchList.clearButKeepMemory ( false );
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

vmTableWidget* vmTableWidget::createPurchasesTable ( vmTableWidget* table, QWidget* parent )
{
	vmTableColumn* cols ( table->createColumns ( PURCHASES_TABLE_COLS ) );
	uint i ( 0 );
	for ( ; i < PURCHASES_TABLE_COLS; ++i )
	{
		switch ( i )
		{
			case ISR_NAME:
				cols[ISR_NAME].completer_type = vmCompleters::PRODUCT_OR_SERVICE;
				cols[ISR_NAME].width = 280;
				cols[ISR_NAME].label = TR_FUNC ( "Item" );
			break;
			case ISR_UNIT:
				cols[ISR_UNIT].label = TR_FUNC ( "Unit" );
			break;
			case ISR_BRAND:
				cols[ISR_BRAND].completer_type = vmCompleters::BRAND;
				cols[ISR_BRAND].text_type = vmWidget::TT_UPPERCASE;
				cols[ISR_BRAND].label = TR_FUNC ( "Maker" );
			break;
			case ISR_QUANTITY:
				cols[ISR_QUANTITY].text_type = vmLineEdit::TT_DOUBLE;
				cols[ISR_QUANTITY].label = TR_FUNC ( "Quantity" );
			break;
			case ISR_UNIT_PRICE:
				cols[ISR_UNIT_PRICE].text_type = vmLineEdit::TT_PRICE;
				cols[ISR_UNIT_PRICE].button_type = vmLineEditWithButton::LEBT_CALC_BUTTON;
				cols[ISR_UNIT_PRICE].label = TR_FUNC ( "Unit price" );
			break;
			case ISR_TOTAL_PRICE:
				cols[ISR_TOTAL_PRICE].text_type = vmLineEdit::TT_PRICE;
				cols[ISR_TOTAL_PRICE].formula = QStringLiteral ( "* D%1 E%1" );
				cols[ISR_TOTAL_PRICE].editable = false;
				cols[ISR_TOTAL_PRICE].label = TR_FUNC ( "Total price" );
			break;
			case PURCHASES_TABLE_REG_COL:
				cols[PURCHASES_TABLE_REG_COL].wtype = WT_CHECKBOX;
				cols[PURCHASES_TABLE_REG_COL].width = 50;
				//cols[PURCHASES_TABLE_REG_COL].default_value = CHR_ZERO;
				cols[PURCHASES_TABLE_REG_COL].label = TR_FUNC ( "Register" );
			break;
		}
	}

	if ( table )
		table->initTable ( 10 );
	else
		table = new vmTableWidget ( 10, parent );

	table->setKeepModificationRecords ( false );
	table->mIsPurchaseTable = true;
	table->insertMonitoredCell ( static_cast<uint>(table->totalsRow ()), ISR_TOTAL_PRICE );
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

	if ( dst_table != nullptr )
	{
		bEditable = dst_table->isEditable ();
		dst_table->setEditable ( true );
	}

	s_row->field_value[s_row->column[4]] = recStrValue ( src_dbrec, src_dbrec->isrRecordField ( ISR_SUPPLIER ) );
	s_row->field_value[s_row->column[5]] = recStrValue ( src_dbrec, src_dbrec->isrRecordField ( ISR_DATE ) );

	for ( uint i_row ( 0 ); i_row <= static_cast<uint>(src_table->lastUsedRow ()); ++i_row )
	{
		if ( src_table->sheetItem ( i_row, PURCHASES_TABLE_REG_COL )->text () == CHR_ZERO ) // item is not marked to be exported
			continue;

		s_row->field_value[s_row->column[0]] = src_table->sheetItem ( i_row, ISR_NAME )->text ();
		s_row->field_value[s_row->column[1]] = src_table->sheetItem ( i_row, ISR_UNIT )->text ();
		s_row->field_value[s_row->column[2]] = src_table->sheetItem ( i_row, ISR_BRAND )->text ();
		s_row->field_value[s_row->column[3]] = src_table->sheetItem ( i_row, ISR_UNIT_PRICE )->text ();

		if ( dst_dbrec->readRecord ( s_row->column[0], s_row->field_value[s_row->column[0]] ) ) //item description
		{
			if ( recStrValue ( dst_dbrec, s_row->column[1] ) == s_row->field_value[s_row->column[1]] ) // item unit
			{
				if ( recStrValue ( dst_dbrec, s_row->column[2] ) == s_row->field_value[s_row->column[2]] ) // item brand
				{
					if ( recStrValue ( dst_dbrec, s_row->column[4] ) == s_row->field_value[s_row->column[4]] ) // item supplier
					{
						if ( recStrValue ( dst_dbrec, s_row->column[3] ) == s_row->field_value[s_row->column[3]] ) // item price
						{
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
		if ( dst_table != nullptr )
		{
			if ( dst_dbrec->action () == ACTION_ADD )
				s_row->row = dst_table->lastUsedRow () + 1;
			else
				s_row->row = recIntValue ( dst_dbrec, dst_dbrec->isrRecordField ( ISR_ID ) );
			s_row->field_value[dst_dbrec->isrRecordField ( ISR_ID )] = QString::number ( s_row->row );
			dst_table->setRowData ( s_row, true );
		}
		else
		{
			if ( dst_dbrec->action () == ACTION_ADD )
			{
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
	vmTableColumn* cols ( table->createColumns ( last_column + 1 ) );

	for ( uint i ( 0 ); i <= last_column; ++i )
	{
		switch ( i )
		{
			case PHR_DATE:
				cols[PHR_DATE].label = TR_FUNC ( "Date" );
				cols[PHR_DATE].wtype = WT_DATEEDIT;
			break;
			case PHR_VALUE:
				cols[PHR_VALUE].label = TR_FUNC ( "Value" );
				cols[PHR_VALUE].button_type = vmLineEditWithButton::LEBT_CALC_BUTTON;
				cols[PHR_VALUE].text_type = vmLineEdit::TT_PRICE;
				cols[PHR_VALUE].wtype = WT_LINEEDIT_WITH_BUTTON;
			break;
			case PHR_PAID:
				cols[PHR_PAID].label = TR_FUNC ( "Paid?" );
				cols[PHR_PAID].wtype = WT_CHECKBOX;
				cols[PHR_PAID].width = 50;
				//cols[PHR_PAID].default_value = CHR_ZERO;
			break;
			case PHR_METHOD:
				cols[PHR_METHOD].label = TR_FUNC ( "Method" );
				cols[PHR_METHOD].completer_type = vmCompleters::PAYMENT_METHOD;
				cols[PHR_METHOD].text_type = vmWidget::TT_UPPERCASE;
				cols[PHR_METHOD].width = 200;
			break;
			case PHR_ACCOUNT:
				cols[PHR_ACCOUNT].label = TR_FUNC ( "Account" );
				cols[PHR_ACCOUNT].completer_type = vmCompleters::ACCOUNT;
				cols[PHR_ACCOUNT].width = 120;
			break;
			case PHR_USE_DATE:
				cols[PHR_USE_DATE].label = TR_FUNC ( "Use Date" );
				cols[PHR_USE_DATE].wtype = WT_DATEEDIT;
			break;
		}
	}

	if ( table )
		table->initTable ( 5 );
	else
		table = new vmTableWidget ( 5, parent );

	table->setKeepModificationRecords ( false );
	/* insertMonitoredCell () assumes the totals row and columns are monitored. Se we setup the callback to nullptr
	 * to avoid the c++ lib to throw a std::bad_function_call. Any pay history object that wishes to monitor changes to
	 * those cells must simply call setCallbackForMonitoredCellChanged () when setting up the table right after the call
	 * to this function.
	 */
	table->setCallbackForMonitoredCellChanged ( [&] ( const vmTableItem* const ) { return nullptr; } );
	table->insertMonitoredCell ( static_cast<uint>(table->totalsRow ()), PHR_VALUE );
	return table;
}

void vmTableWidget::insertRow ( const uint row, const uint n )
{
	if ( row <= static_cast<uint> ( rowCount () ) )
	{
		uint i_row ( 0 );
		m_nVisibleRows += n;
		uint i_col ( 0 );
		QString new_formula;
		vmTableItem* sheet_item ( nullptr ), *new_SheetItem ( nullptr );
		
		mTotalsRow += n;
		for ( ; i_row < n; ++i_row )
		{ // Insert new rows accordingly, i.e., compute their formulas, if they have
			QTableWidget::insertRow ( static_cast<int>( row + i_row ) );
			for ( i_col = 0; i_col < colCount (); ++i_col )
			{
				new_SheetItem = new vmTableItem ( mCols[i_col].wtype, mCols[i_col].text_type, mCols[i_col].default_value, this );
				new_SheetItem->setButtonType ( mCols[i_col].button_type );
				new_SheetItem->setCompleterType ( mCols[i_col].completer_type );
				setItem ( static_cast<int>( row + i_row ), static_cast<int>( i_col ), new_SheetItem );
				setCellWidget ( new_SheetItem );

				/* The formula must be set for last because setFormula calls vmTableItem::targetsFromFormula ();
				 * when inserting cells, that function might attempt to solve the formula using data from the previous
				 * cell (i.e. before inserting) or, worse, from the totalsRow (), which will cause unpredicted (but fatal) results
				 */
				if ( !mCols[i_col].formula.isEmpty () )
					new_SheetItem->setFormula ( mCols[i_col].formula, QString::number ( row + i_row ) );
				
				new_SheetItem->setEditable ( isEditable () );
			}
		}

		if ( !mbPlainTable )
		{
			// update all rows after the inserted ones, so that their formula reflects the row changes
			for ( i_row = row + n; static_cast<int>( i_row ) < mTotalsRow; ++i_row )
			{
				for ( i_col = 0; i_col < colCount (); ++i_col )
				{
					sheet_item = sheetItem ( i_row, i_col );
					if ( sheet_item->hasFormula () )
					{
						new_formula = sheet_item->formulaTemplate ();
						sheet_item->setFormula ( new_formula, QString::number ( i_row ) );
					}
				}
			}
			fixTotalsRow ();
		
			const uint modified_rows ( modifiedRows.count () );
			if ( modified_rows > 0 )
			{
				uint i ( 0 ), list_value ( 0 ), start_modification ( 0 );
				for ( i = 0; i < modified_rows; ++i )
				{
					if ( row < modifiedRows.at ( i ) ) break;
					start_modification++; // modified above insertion start point are not affected
				}
				for ( i = start_modification; i < modified_rows; ++i )
				{
					list_value = modifiedRows.at ( i );
					modifiedRows.replace ( i, list_value + n ); // push down modified rows
				}
			}
		}
	}
}

void vmTableWidget::removeRow ( const uint row, const uint n )
{
	if ( static_cast<int>( row ) < totalsRow () )
	{
		uint i_row ( 0 );
		m_nVisibleRows -= n;
		for ( ; i_row < n; ++i_row )
			QTableWidget::removeRow ( static_cast<int>( row ) );

		if ( static_cast<int>( row + n ) >= lastUsedRow () )
			setLastUsedRow ( static_cast<int>( row - 1 ) );
			
		if ( !mbPlainTable )
		{
			mTotalsRow -= n;
			vmTableItem* sheet_item ( nullptr );
			uint i_col ( 0 );
			QString new_formula;
			for ( i_row = row; static_cast<int>( i_row ) < totalsRow (); ++i_row )
			{
				for ( i_col = 0; i_col < colCount (); ++i_col )
				{
					sheet_item = sheetItem ( i_row, i_col );
					if ( sheet_item->hasFormula () )
					{
						new_formula = sheet_item->formulaTemplate ();
						sheet_item->setFormula ( new_formula, QString::number ( i_row ) );
					}
				}
			}
			fixTotalsRow ();
			
			const uint modified_rows ( modifiedRows.count () );
			if ( modified_rows > 0 )
			{
				uint i ( 0 ), list_value ( 0 ), start_modification ( 0 );
				for ( i = 0 ; i < modifiedRows.count (); ++i )
				{
					if ( row < modifiedRows.at ( i ) ) 
						break;
					start_modification++; // modified rows above deletion start point are not affected
				}
				for ( i = start_modification; i < modifiedRows.count (); ++i )
				{
					list_value = modifiedRows.at ( i );
					modifiedRows.replace ( i, list_value - n ); // push up modified rows
				}
				modifiedRows.removeOne ( row, 0 );
			}
		}
	}
}

void vmTableWidget::setRowVisible ( const uint row, const bool bVisible )
{
	const bool b_isVisible ( !isRowHidden ( static_cast<int>( row ) ) );
	if ( !b_isVisible && bVisible )
	{
		++m_nVisibleRows;
		QTableWidget::setRowHidden ( static_cast<int>( row ), false );
	}
	else if ( b_isVisible && !bVisible )
	{
		--m_nVisibleRows;
		QTableWidget::setRowHidden ( static_cast<int>( row ), true );
	}
}

void vmTableWidget::clearRow ( const uint row, const uint n )
{
	if ( isEditable () )
	{
		if ( static_cast<int>(row) <= lastUsedRow () )
		{
			vmTableItem* item ( nullptr );
			for ( uint i_row ( row ); i_row < row + n; ++i_row )
			{
				// Callback before removing so that its values might be used for something before they cannot be used anymore
				if ( rowRemoved_func )
					rowRemoved_func ( i_row );
				
				for ( uint i_col ( 0 ); i_col <= ( colCount () - 1 ); ++i_col )
				{
					if ( !isBitSet ( readOnlyColumnsMask, static_cast<uchar>( i_col ) ) )
					{
						item = sheetItem ( i_row, i_col );
						/* TODO test: In previous versions the argument to setTextToDefault was set to true.
						 * Now, we issue rowRemoved_func only once
						*/
						item->setTextToDefault ();
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
	const int max_row ( force ? totalsRow () - 1 : lastUsedRow () );
	if ( max_row > 0 )
	{
		setUpdatesEnabled ( false );
		uint i_col ( 0 );
		vmTableItem* item ( nullptr );
		for ( uint i_row ( 0 ); i_row <= static_cast<uint>( max_row ); ++i_row )
		{
			for ( i_col = 0; i_col <= ( colCount () - 1 ); ++i_col )
			{
				if ( !isBitSet ( readOnlyColumnsMask, static_cast<uchar>( i_col ) ) )
				{
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
		setLastUsedRow ( - 1 );
		setUpdatesEnabled ( true );
	}
}

void vmTableWidget::rowActivatedConnection ( const bool b_activate )
{
	if ( rowActivated_func )
	{
		if ( b_activate )
			static_cast<void>( connect ( this, &QTableWidget::itemClicked, this, [&] ( QTableWidgetItem* current ) {
				return rowActivated_func ( current->row () ); } ) );
		else
			static_cast<void>( disconnect ( this, &QTableWidget::itemActivated, nullptr, nullptr ) );
	}
}

void vmTableWidget::setCellValue ( const QString& value, const uint row, const uint col )
{
	if ( col < colCount () )
	{
		if ( static_cast<int>(row) >= totalsRow () )
			appendRow ();
		sheetItem ( row, col )->setText ( value, false, false );
	}
}

void vmTableWidget::setRowData ( const spreadRow* s_row, const bool b_notify )
{
	if ( s_row != nullptr )
	{
		if ( s_row->row == -1 )
			const_cast<spreadRow*>( s_row )->row = lastUsedRow () + 1;
		if ( s_row->row >= totalsRow () )
			appendRow ();
		for ( uint i_col ( 0 ), used_col ( s_row->column.at ( 0 ) ); i_col < s_row->column.count (); used_col = s_row->column.at ( ++i_col ) )
			sheetItem ( static_cast<uint>(s_row->row), used_col )->setText ( s_row->field_value.at ( used_col ), false, b_notify );
	}
}

void vmTableWidget::rowData ( const uint row, spreadRow* s_row ) const
{
	if ( static_cast<int>( row ) < totalsRow () )
	{
		uint i_col ( 0 );
		s_row->row = static_cast<int>( row );
		s_row->field_value.clearButKeepMemory ();

		for ( ; i_col < colCount (); ++i_col )
			s_row->field_value.append ( sheetItem ( row, i_col )->text () );
	}
}

void vmTableWidget::cellContentChanged ( const vmTableItem* const item )
{
	mTableChanged = true;
	if ( mbKeepModRec )
	{
		const uint row ( static_cast<uint>( item->row () ) );
		uint insert_pos ( modifiedRows.count () ); // insert rows in crescent order
		for ( int i ( static_cast<int>( modifiedRows.count () - 1 ) ); i >= 0; --i )
		{
			if ( row == modifiedRows.at ( i ) )
			{
				if ( cellChanged_func )
					cellChanged_func ( item );
				return;
			}
			else if ( row > modifiedRows.at ( i ) )
			{
				break;
			}
			else
				--insert_pos;
		}
		modifiedRows.insert ( insert_pos, row );
	}
	if ( mIsPurchaseTable )
	{
		if ( item->column () == PURCHASES_TABLE_REG_COL )
			setProperty ( PROPERTY_TABLE_HAS_ITEM_TO_REGISTER, true );
		else
		{
			if ( sheetItem ( static_cast<uint>( item->row () ), PURCHASES_TABLE_REG_COL )->text () == CHR_ONE )
				setProperty ( PROPERTY_TABLE_HAS_ITEM_TO_REGISTER, true );
		}
	}
	if ( cellChanged_func )
		cellChanged_func ( item );
}

void vmTableWidget::cellModified ( const vmTableItem* const item )
{
	if ( item && cellChanged_func != nullptr )
		cellChanged_func ( item );
}

// Customize context menu
void vmTableWidget::renameAction ( const contextMenuDefaultActions action, const QString& new_text )
{
	if ( new_text.isEmpty () ) return;
	switch ( action )
	{
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
		break;
		default:
		break;
	}
}

void vmTableWidget::addContextMenuAction ( vmAction* action )
{
	if ( mAddedItems == 0 )
		mSeparator = mContextMenu->addSeparator ();
	++mAddedItems;
	mContextMenu->addAction ( static_cast<QAction*>( action ) );
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
	if ( data.isOK () )
	{
		setUpdatesEnabled ( false );
		spreadRow* s_row ( new spreadRow );
		uint i_col ( 0 );

		for ( ; i_col < colCount (); ++i_col )
			s_row->column[i_col] = i_col;

		const stringRecord* rec ( &data.first () );
		if ( rec->isOK () )
		{
			s_row->row = 0;
			do
			{
				if ( rec->first () )
				{
					for ( i_col = 0; i_col < colCount (); ++i_col )
					{
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
		setUpdatesEnabled ( true );
		scrollToTop ();
	}
}

void vmTableWidget::makeStringTable ( stringTable& data )
{
	if ( lastUsedRow () >= 0 )
	{
		stringRecord rec;
		uint i_col ( 0 );
		for ( uint i_row ( 0 ); i_row <= static_cast<uint>( lastUsedRow () ); ++i_row )
		{
			for ( i_col = 0; i_col < colCount (); ++i_col )
				rec.fastAppendValue ( sheetItem ( i_row, i_col )->text () );
			data.fastAppendRecord ( rec );
			rec.clear ();
		}
	}
}

void vmTableWidget::setCellWidget ( vmTableItem* const sheet_item )
{
	vmWidget* widget ( nullptr );
	// read only columns do not connect signals, do not need completers
	const bool bCellReadOnly ( isBitSet ( readOnlyColumnsMask, static_cast<uchar>( sheet_item->column () ) ) || sheet_item->row () == totalsRow () );

	switch ( sheet_item->widgetType () )
	{
		case WT_LINEEDIT:
			widget = new vmLineEdit;
			static_cast<vmLineEdit*>( widget )->setFrame ( false );
			static_cast<vmLineEdit*>( widget )->setTextType ( sheet_item->textType () );
			if ( !bCellReadOnly )
			{
				APP_COMPLETERS ()->setCompleter ( static_cast<vmLineEdit*>( widget ), sheet_item->completerType () );
				widget->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) { return textWidgetChanged ( sender ); } );
			}
		break;
		case WT_LINEEDIT_WITH_BUTTON:
			widget = new vmLineEditWithButton;
			static_cast<vmLineEditWithButton*>( widget )->setButtonType ( sheet_item->buttonType () );
			static_cast<vmLineEditWithButton*>( widget )->lineControl ()->setTextType ( sheet_item->textType () );
			if ( !bCellReadOnly )
			{
				APP_COMPLETERS ()->setCompleter ( static_cast<vmLineEditWithButton*>( widget )->lineControl (), sheet_item->completerType () );
					static_cast<vmLineEditWithButton*>( widget )->lineControl ()->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
						return textWidgetChanged ( sender ); } );
			}
		break;
		case WT_CHECKBOX: // so far, only check box has a different default value, depending on the table to which they belong
			widget = new vmCheckBox;
			static_cast<vmCheckBox*>( widget )->setChecked ( sheet_item->defaultValue () == CHR_ONE );
			if ( !bCellReadOnly )
				widget->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) { return checkBoxToggled ( sender ); } );
		break;
		case WT_DATEEDIT:
			widget = new vmDateEdit;
			if ( !bCellReadOnly )
				widget->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) { return dateWidgetChanged ( sender ); } );
		break;
		case WT_TIMEEDIT:
			widget = new vmTimeEdit;
			if ( !bCellReadOnly )
				widget->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) { return timeWidgetChanged ( sender ); } );
		break;
		case WT_COMBO:
			widget = new vmComboBox;
			if ( !bCellReadOnly )
				widget->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) { return textWidgetChanged ( sender ); } );
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

void vmTableWidget::addToLayout ( QGridLayout* glayout, const uint row, const uint column )
{
	if ( setParentLayout ( glayout ) )
		glayout->addWidget ( this, static_cast<int>(row), static_cast<int>(column) );
}

void vmTableWidget::addToLayout ( QVBoxLayout* vblayout, const uint stretch )
{
	if ( setParentLayout ( vblayout ) )
	{
		vblayout->addWidget ( this, static_cast<int>(stretch) );
		setUtilitiesPlaceLayout ( vblayout );
	}
}

bool vmTableWidget::setParentLayout ( QGridLayout* glayout )
{
	if ( mParentLayout == static_cast<QLayout*>( glayout ) )
		return false;
	if ( mParentLayout != nullptr )
		mParentLayout->removeWidget ( this );
	mParentLayout = static_cast<QLayout*>( glayout );
	return true;
}

bool vmTableWidget::setParentLayout ( QVBoxLayout* vblayout )
{
	if ( mParentLayout == static_cast<QLayout*>( vblayout ) )
		return false;
	if ( mParentLayout != nullptr )
		mParentLayout->removeWidget ( this );
	mParentLayout = static_cast<QLayout*>( vblayout );
	return true;
}

void vmTableWidget::setIsList ()
{
	mbTableIsList = true;
	setIsPlainTable ();
}

void vmTableWidget::setIsPlainTable () 
{ 
	mbPlainTable = true;
	setKeepModificationRecords ( false ); 
	setSelectionMode ( QAbstractItemView::SingleSelection );
	setSelectionBehavior ( QAbstractItemView::SelectRows );
}

/* For optimal results, when populating a table, set it readonly.
 * When clearing a table, set it readonly. This way, there won't be a plethora of
 * signals emitted.
 */
void vmTableWidget::setEditable ( const bool editable )
{
	if ( isEditable () == editable )
		return;

	uint i_row ( 0 );
	uint i_col ( 0 );
	for ( ; static_cast<int>( i_row ) < totalsRow (); ++i_row )
	{
		for ( i_col = 0; i_col < colCount (); ++i_col )
		{
			if ( !isBitSet ( readOnlyColumnsMask, static_cast<uchar>(i_col) ) )
				sheetItem ( i_row, i_col )->setEditable ( editable );
		}
	}

	if ( editable )
	{
		if ( isPlainTable () )
		{
			static_cast<void>( connect ( this, &QTableWidget::itemChanged, this, [&] ( QTableWidgetItem *item ) { 
					return ( cellModified ( static_cast<vmTableItem*>( item ) ) ); } ) );
		}
		if ( cellNavigation_func != nullptr )
		{
			static_cast<void>( connect ( this, &QTableWidget::currentCellChanged, this, [&] ( const int row, const int col, const int prev_row, const int prev_col ) {
				return cellNavigation_func ( static_cast<uint> ( row ), static_cast<uint> ( col ), static_cast<uint> ( prev_row ), static_cast<uint> ( prev_col ) ); } ) );
		}
		if ( mIsPurchaseTable )
		{
			static_cast<void>( connect ( APP_COMPLETERS ()->getCompleter ( vmCompleters::PRODUCT_OR_SERVICE ),
					  static_cast<void (QCompleter::*)( const QModelIndex& )>( &QCompleter::activated ),
					this, ( [&, this] ( const QModelIndex& index ) { return interceptCompleterActivated ( index, this );
			} ) ) );
		}
	}
	else
	{
		static_cast<void>( this->disconnect ( nullptr, nullptr, nullptr ) );
	}
	vmWidget::setEditable ( editable );
}

void vmTableWidget::setLastUsedRow ( const int row )
{
	if ( row < 0 )
		m_lastUsedRow = -1;
	else
	{
		if ( row > m_lastUsedRow &&	row < ( isPlainTable () ? rowCount () : static_cast<int>( totalsRow () ) ) ) 
			m_lastUsedRow = row;
	}
}

uint vmTableWidget::selectedRowsCount () const
{
	uint n ( 0 );
	for ( uint i_row ( 0 ); static_cast<int>(i_row) <= lastUsedRow (); ++i_row )
	{
		for ( uint i_col ( 0 ); i_col < ( colCount () - 1 ); ++i_col )
		{
			if ( sheetItem ( i_row, i_col )->isSelected () )
			{
				++n;
				break;
			}
		}
	}
	return n;
}

bool vmTableWidget::isRowEmpty ( const uint row ) const
{
	for ( uint i_col ( 0 ); i_col < colCount (); ++i_col )
	{
		if ( sheetItem ( row, i_col )->text () != sheetItem ( row, i_col )->defaultValue () )
		{ // cell is not empty
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
	if ( selectionBehavior () == QAbstractItemView::SelectRows )
	{
		if ( !selectedItems ().isEmpty () )
			return selectedItems ().contains ( sheetItem ( row, 0 ) );
	}
	else
	{
		for ( uint i_col ( 0 ); i_col < colCount (); ++i_col )
		{
			if ( sheetItem ( row, i_col )->isSelected () )
				return true;
		}
	}
	return false;
}

bool vmTableWidget::isRowSelectedAndNotEmpty ( const uint row ) const
{
	if ( isRowSelected ( row ) )
	{
		vmTableItem* item ( nullptr );
		for ( uint i_col = 0; i_col < colCount (); ++i_col )
		{
			item = sheetItem ( row, i_col );
			if ( !item->text ().isEmpty () )
				return ( !isBitSet ( readOnlyColumnsMask, static_cast<uchar>(i_col) ) );
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
	for ( uint i_row ( 0 ); static_cast<int>(i_row) <= lastUsedRow (); ++i_row )
	{
		for ( uint i_col = 0; i_col <= ( colCount () - 1 ); ++i_col )
			sheetItem ( i_row, i_col )->syncOriginalTextWithCurrent ();
	}
}

void vmTableWidget::setHiddenText ( const uint row, const uint col, const QString& str, const bool notify_change )
{
	vmTableItem* item ( sheetItem ( row, col ) );
	if ( item != nullptr )
	{
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
	if ( i_col < colCount () )
	{
		readonly ? setBit ( readOnlyColumnsMask, static_cast<uchar>(i_col) ) :
		unSetBit ( readOnlyColumnsMask, static_cast<uchar>(i_col) );
	}
}

int vmTableWidget::isCellMonitored ( const vmTableItem* const item )
{
	if ( item == nullptr )
		return -2; // a null item is to be ignored, obviously. -2 will not be monitored
	for ( uint i ( 0 ); i < mMonitoredCells.count (); ++i )
	{
		if ( mMonitoredCells.at ( i ) == item )
			return static_cast<int>(i);
	}
	return -1; //-1 not monitored
}

//todo: monitoredCellChanged_func gets called before the item's text is updated, so we get the old text instead of the new one
void vmTableWidget::insertMonitoredCell ( const uint row, const uint col )
{
	const vmTableItem* item ( sheetItem ( row, col ) );
	if ( isCellMonitored ( item ) == -1 )
	{
		mMonitoredCells.append ( item );
		/* Most likely, a monitored cell will be read only, and display the result of some formula dependent on changes happening
		 * on other cells. setCellWidget is the method responsible for setting the callbacks for altered contents, but will not
		 * do so on read only cells. Therefore, we do this here only for cells that matter. We do not need to change the text of the
		 * widget because vmTableItem already does it, and, in fact, monitoredCellChanged_func will be called by the setText method
		 * of widget which, in turn, is called by vmTableItem::setText
		*/
		if ( isBitSet ( readOnlyColumnsMask, static_cast<uchar>(col) ) || row == static_cast<uint>(totalsRow ()) )
		{
			if ( item->widget () ) 
			{ // call setCallbackForMonitoredCellChanged with nullptr as argument when you wish to handle the monitoring outside this class
				item->widget ()->setCallbackForContentsAltered ( [&, item] ( const vmWidget* const ) {
						return monitoredCellChanged_func ( item ); } );
			}
			else //callback for simple text cells (plain table)
				cellChanged_func = monitoredCellChanged_func;
		}
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
	if ( color != vmDefault_Color )
	{
		vmTableItem* item ( nullptr );
		uint i_col ( 0 );
		for ( uint i_row ( 0 ); static_cast<int>(i_row) <= lastUsedRow (); ++i_row )
		{
			for ( i_col = 0; i_col < colCount (); ++i_col )
			{
				if ( sheetItem ( i_row, i_col )->text ().contains ( text ) )
				{
					item = sheetItem ( i_row, i_col );
					item->highlight ( color );
					m_highlightedCells.append ( item );
				}
			}
		}
	}
	else
	{
		for ( uint i ( 0 ); i < m_highlightedCells.count (); ++i )
			m_highlightedCells.at ( i )->highlight ( vmDefault_Color );
		m_highlightedCells.clearButKeepMemory ();
	}
}

inline bool vmTableWidget::isColumnSelectedForSearch ( const uint column ) const
{
	return column < colCount () ? static_cast<vmCheckedTableItem*>(horizontalHeader ())->isChecked ( column ) : false;
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

void vmTableWidget::keyPressEvent ( QKeyEvent* k )
{
	const bool ctrlKey ( k->modifiers () & Qt::ControlModifier );
	if ( ctrlKey )
	{
		bool b_accept ( false );
		if ( k->key () == Qt::Key_F )
		{
			if ( !m_searchPanel )
			{
				m_searchPanel = new vmTableSearchPanel ( this );
				m_searchPanel->setUtilityIndex ( addUtilityPanel ( m_searchPanel ) );
			}
			b_accept = toggleUtilityPanel ( m_searchPanel->utilityIndex () );
		}
		
		else if ( k->key () == Qt::Key_L )
		{
			if ( !m_filterPanel )
			{
				m_filterPanel = new vmTableFilterPanel ( this );
				m_filterPanel->setUtilityIndex ( addUtilityPanel ( m_filterPanel ) );
			}
			b_accept = toggleUtilityPanel ( m_filterPanel->utilityIndex () );
		}

		else if ( k->key () == Qt::Key_Escape )
		{
			if ( m_searchPanel && m_searchPanel->isVisible () )
			{
				utilitiesLayout ()->removeWidget ( m_searchPanel );
				m_searchPanel->setVisible ( false );
			}
			if ( m_filterPanel && m_filterPanel->isVisible () )
			{
				utilitiesLayout ()->removeWidget ( m_filterPanel );
				m_filterPanel->setVisible ( false );
			}
			b_accept = true;
		}
		if ( b_accept )
		{
			k->setAccepted ( true );
			return;
		}
	}
	
	if ( isEditable () )
	{
		if ( ctrlKey )
		{
			switch ( k->key () )
			{
				case Qt::Key_Enter:
				case Qt::Key_Return:
					if ( keypressed_func )
						keypressed_func ( k, this );
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
		else
		{ // no CTRL modifier
			switch ( k->key () )
			{
				case Qt::Key_Enter:
				case Qt::Key_Return:
					if ( keypressed_func )
						keypressed_func ( k, this );
				break;
				case Qt::Key_F2:
				case Qt::Key_F3:
				case Qt::Key_F4:
					if ( m_searchPanel && m_searchPanel->isVisible () )
					{
						QApplication::sendEvent ( m_searchPanel, k ); // m_searchPanel will propagate the event its children
						QApplication::sendPostedEvents ( m_searchPanel, 0 ); // force event to be processed before posting another one to que queue
					}
				break;
				default:
				break;
			}
		}
	}
	QTableWidget::keyPressEvent ( k );
}

void vmTableWidget::enableOrDisableActionsForCell ( const vmTableItem* sheetItem )
{
	mUndoAction->setEnabled ( sheetItem->cellIsAltered () );
	mCopyCellAction->setEnabled ( !isRowEmpty ( static_cast<uint>(sheetItem->row ()) ) );
	mCopyRowContents->setEnabled ( !isRowEmpty ( static_cast<uint>(sheetItem->row ()) ) );
	mAppendRowAction->setEnabled ( sheetItem->isEditable () );
	mInsertRowAction->setEnabled ( sheetItem->isEditable () );
	mDeleteRowAction->setEnabled ( sheetItem->isEditable () );
	mClearRowAction->setEnabled ( sheetItem->isEditable () );
	mClearTableAction->setEnabled ( sheetItem->isEditable () );
	mSubMenuFormula->setEnabled ( sheetItem->isEditable () );
	if ( this->isEditable () ) //even if the item is not editable, the formula might be changed
	{
		if ( mOverrideFormulaAction == nullptr )
		{
			mFormulaTitleAction = new vmAction ( FORMULA_TITLE );
			mOverrideFormulaAction = new vmAction ( SET_FORMULA_OVERRIDE, TR_FUNC ( "Allow formula override" ) );
			mOverrideFormulaAction->setCheckable ( true );
			connect ( mOverrideFormulaAction, &vmAction::triggered, this, [&, sheetItem ] ( const bool checked ) {
						return setFormulaOverrideForCell ( const_cast<vmTableItem*>( sheetItem ), checked ); } );
			mSetFormulaAction = new vmAction ( SET_FORMULA, TR_FUNC ( "Set formula" ) );
			connect ( mSetFormulaAction, &vmAction::triggered, this, [&, sheetItem ] () {
						return setFormulaForCell ( const_cast<vmTableItem*>( sheetItem ) ); } );
			mSubMenuFormula->addAction ( mFormulaTitleAction );
			mSubMenuFormula->addSeparator ();
			mSubMenuFormula->addAction ( mOverrideFormulaAction );
			mSubMenuFormula->addAction ( mSetFormulaAction );
		}
		mFormulaTitleAction->QAction::setText ( sheetItem->hasFormula () ? sheetItem->formula () : TR_FUNC ( "No formula" ) );
		mOverrideFormulaAction->setEnabled ( sheetItem->hasFormula () );
		mOverrideFormulaAction->setChecked ( sheetItem->formulaOverride () );
	}
}

void vmTableWidget::sharedContructorsCode ()
{
	setWidgetPtr ( static_cast<QWidget*>( this ) );
	setSelectionMode ( QAbstractItemView::ExtendedSelection );
	setSelectionBehavior ( QAbstractItemView::SelectItems );
	setAlternatingRowColors ( true );
	setAttribute ( Qt::WA_KeyCompression );
	setFrameShape ( QFrame::NoFrame );
	setFrameShadow ( QFrame::Plain );

	setEditable ( false );

	if ( vmTableWidget::defaultBGColor.isEmpty () )
		vmTableWidget::defaultBGColor = palette ().color ( backgroundRole () ).name ();

	mUndoAction = new vmAction ( UNDO, TR_FUNC ( "Undo change (CTRL+Z)" ), this );
	static_cast<void>( connect ( mUndoAction, &vmAction::triggered, this, [&] () { return undoChange (); } ) );

	mCopyCellAction = new vmAction ( COPY_CELL, TR_FUNC ( "Copy cell contents to clipboard (CTRL+C)" ), this );
	static_cast<void>( connect ( mCopyCellAction, &vmAction::triggered, this, [&] () { return copyCellContents (); } ) );

	mCopyRowContents = new vmAction ( COPY_ROW, TR_FUNC ( "Copy row contents to clipboard (CTRL+B)" ), this );
	static_cast<void>( connect ( mCopyRowContents, &vmAction::triggered, this, [&] () { return copyRowContents (); } ) );

	mInsertRowAction = new vmAction ( ADD_ROW, TR_FUNC ( "Insert line here (CTRL+I)" ), this );
	static_cast<void>( connect ( mInsertRowAction, &vmAction::triggered, this, [&] () { return insertRow_slot (); } ) );

	mAppendRowAction = new vmAction ( APPEND_ROW, TR_FUNC ( "Append line (CTRL+O)" ), this );
	static_cast<void>( connect ( mAppendRowAction, &vmAction::triggered, this, [&] () { return appendRow_slot (); } ) );

	mDeleteRowAction = new vmAction ( DEL_ROW, TR_FUNC ( "Remove this line (CTRL+R)" ), this );
	static_cast<void>( connect ( mDeleteRowAction, &vmAction::triggered, this, [&] () { return removeRow_slot (); } ) );

	mClearRowAction = new vmAction ( CLEAR_ROW, TR_FUNC ( "Clear line (CTRL+D)" ), this );
	static_cast<void>( connect ( mClearRowAction, &vmAction::triggered, this, [&] () { return clearRow_slot (); } ) );

	mClearTableAction = new vmAction ( CLEAR_TABLE, TR_FUNC ( "Clear table (CTRL+T)" ), this );
	static_cast<void>( connect ( mClearTableAction, &vmAction::triggered, this, [&] () { return clearTable_slot (); } ) );

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
	for ( uint i_col ( 0 ); i_col < colCount (); ++i_col )
	{
		sheet_item = sheetItem ( static_cast<uint>(mTotalsRow), i_col );
		if ( sheet_item->textType () >= vmLineEdit::TT_PRICE )
			sheet_item->setFormula ( sheet_item->formulaTemplate (), totalsRowStr );
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


	if ( record.isOK () )
	{
		if ( currentRow () == -1 )
			setCurrentCell ( 0, 0, QItemSelectionModel::ClearAndSelect );
		if ( currentRow () >= 0 )
		{
			const uint current_row ( static_cast<uint>(currentRow ()) );
			if ( record.first () )
			{
				uint i_col ( 0 );
				do
				{
					sheetItem ( current_row, i_col++ )->setText ( record.curValue (), false, true );
				} while ( i_col < ISR_TOTAL_PRICE && record.next () );
				setCurrentCell ( static_cast<int>(current_row), ISR_QUANTITY, QItemSelectionModel::ClearAndSelect );
			}
		}
	}
	mbDoNotUpdateCompleters = false;
}

void vmTableWidget::undoChange ()
{
	if ( isEditable () )
	{
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
		if ( selIndexes.count () > 0 )
		{
			QModelIndexList::const_iterator itr ( selIndexes.constBegin () );
			const QModelIndexList::const_iterator itr_end ( selIndexes.constEnd () );
			for ( ; itr != itr_end; ++itr )
			{
				cell_text.append ( sheetItem ( static_cast<uint>(static_cast<QModelIndex>(*itr).row ()),
											   static_cast<uint>(static_cast<QModelIndex>(*itr).column ()) )->text () );
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
	if ( mContextMenuCell != nullptr )
	{
		for ( ; i_col < colCount (); ++i_col )
			rowText += sheetItem ( static_cast<uint>(mContextMenuCell->row ()), i_col )->text () + QLatin1Char ( '\t' );
		rowText.chop ( 1 );
	}
	else
	{
		const QModelIndexList selIndexes ( selectedIndexes () );
		if ( selIndexes.count () > 0 )
		{
			QModelIndexList::const_iterator itr ( selIndexes.constBegin () );
			const QModelIndexList::const_iterator itr_end ( selIndexes.constEnd () );
			for ( ; itr != itr_end; ++itr )
			{
				for ( i_col = 0; i_col < colCount (); ++i_col )
					rowText += sheetItem ( static_cast<uint>(static_cast<QModelIndex>(*itr).row ()), i_col )->text () + QLatin1Char ( '\t' );
				rowText.chop ( 1 );
				rowText += CHR_NEWLINE;
			}
			rowText.chop ( 1 );
		}
	}

	Data::copyToClipboard ( rowText );
}

// Inserts one or more lines ( depending on how many are selected - i.e. two selected, in whichever order, inserts two lines )
void vmTableWidget::insertRow_slot ()
{
	if ( mContextMenuCell != nullptr )
		insertRow ( static_cast<uint>(mContextMenuCell->row ()) );
	else
	{
		const QModelIndexList selIndexes ( selectedIndexes () );
		if ( selIndexes.count () > 0 )
		{
			QModelIndexList::const_iterator itr ( selIndexes.constBegin () );
			const QModelIndexList::const_iterator itr_end ( selIndexes.constEnd () );
			for ( ; itr != itr_end; ++itr )
				insertRow ( static_cast<uint>(static_cast<QModelIndex>(*itr).row ()) + 1 );
		}
	}
}

void vmTableWidget::appendRow_slot ()
{
	appendRow ();
}

void vmTableWidget::removeRow_slot ()
{
	if ( mContextMenuCell && rowRemoved_func )
	{
		rowRemoved_func ( static_cast<uint>(mContextMenuCell->row ()) );
		removeRow ( static_cast<uint>(mContextMenuCell->row ()) );
	}
	else
	{
		const QModelIndexList selIndexes ( selectedIndexes () );
		if ( selIndexes.count () > 0 )
		{
			// Move from last selected toward the first to avoid conflicts and errors
			QModelIndexList::const_iterator itr ( selIndexes.constEnd () );
			const QModelIndexList::const_iterator itr_begin ( selIndexes.constBegin () );
			for ( --itr; ; --itr )
			{
				// Emit the signal before removing the row so that the slot(s) that catch the signal can retrieve information from the row
				// (i.e. an ID, or name or anything to be used by the database ) before it disappears
				if ( rowRemoved_func )
					rowRemoved_func ( static_cast<uint>(static_cast<QModelIndex>(*itr).row ()) );
				removeRow ( static_cast<uint>(static_cast<QModelIndex>(*itr).row ()) );
				if ( itr == itr_begin )
					break;
			}
		}
	}
}

void vmTableWidget::clearRow_slot ()
{
	if ( isEditable () )
	{
		if ( mContextMenuCell != nullptr )
			clearRow ( static_cast<uint>(mContextMenuCell->row ()) );
		else
		{
			const QModelIndexList selIndexes ( selectedIndexes () );
			if ( selIndexes.count () > 0 )
			{
				QModelIndexList::const_iterator itr ( selIndexes.constBegin () );
				const QModelIndexList::const_iterator itr_end ( selIndexes.constEnd () );
				for ( ; itr != itr_end; ++itr )
					clearRow ( static_cast<uint>(static_cast<QModelIndex>(*itr).row ()) );
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
	if ( vmNotify::inputBox ( strNewFormula, this, TR_FUNC ( "New Formula" ), TR_FUNC ( "Set new cell formula" ),
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
	if ( cellWidget->ownerItem () )
	{
		mContextMenuCell = const_cast<vmTableItem*> ( cellWidget->ownerItem () );
		setCurrentItem ( mContextMenuCell, QItemSelectionModel::ClearAndSelect );
		enableOrDisableActionsForCell ( mContextMenuCell );
		Data::execMenuWithinWidget ( mContextMenu, cellWidget->toQWidget (), pos );
		mContextMenuCell = nullptr;
	}
}

void vmTableWidget::cellWidgetRelevantKeyPressed ( const QKeyEvent* const ke, const vmWidget* const widget )
{
	if ( ke->key () == Qt::Key_Escape )
	{
		/* The text is different from the original, canceling (pressing the ESC key) should return
		 * the widget's text to the original
		 */
		vmTableItem* item ( const_cast<vmTableItem*> ( widget->ownerItem () ) );
		if ( item->text () != item->originalText () )
		{
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
