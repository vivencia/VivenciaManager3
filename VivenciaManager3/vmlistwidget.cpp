#include "vmlistwidget.h"
#include "vmlistitem.h"

vmListWidget::vmListWidget ( QWidget* parent, const uint nRows )
	: vmTableWidget ( parent ), mbIgnore ( true ), mPrevRow ( -2 ), mCurrentItem ( nullptr ), mCurrentItemChangedFunc ( nullptr )
{
	setIsList ();
	vmTableColumn* cols ( createColumns ( 1 ) );
	cols[0].width = 1000;
	initTable ( nRows );
	connect ( this, &QTableWidget::currentCellChanged, this, [&] ( const int row, const int, const int prev_row, const int )
			  { return rowSelected ( row, prev_row ); } );
	setIgnoreChanges ( false );
}

vmListWidget::~vmListWidget ()
{
	this->vmListWidget::clear ( true, true );
}

void vmListWidget::setCurrentRow ( int row, const bool b_makecall )
{
	if ( row != mPrevRow )
	{
		if ( row == -1 )
			row = mPrevRow;
		else
		{
			if ( mCurrentItem != nullptr )
				mPrevRow = mCurrentItem->row ();
		}
		mCurrentItem = item ( row );
		if ( !isIgnoringChanges () && mCurrentItem )
		{
			scrollToItem ( mCurrentItem );
			setCurrentCell ( row, 0, QItemSelectionModel::ClearAndSelect );
			if ( b_makecall && mCurrentItemChangedFunc )
				mCurrentItemChangedFunc ( mCurrentItem );
		}
	}
}

void vmListWidget::addItem ( vmListItem* item )
{
	if ( item != nullptr )
	{
		const uint row ( rowCount () );
		appendRow ();
		item->update ();
		setItem ( row, 0, item );
		if ( !isIgnoringChanges () )
		{
			mCurrentItem = item;
			scrollToItem ( mCurrentItem );
			setCurrentCell ( row, 0, QItemSelectionModel::ClearAndSelect );
			if ( mCurrentItemChangedFunc )
				mCurrentItemChangedFunc ( mCurrentItem );
		}
	}
}

void vmListWidget::clear (const bool b_ignorechanges , const bool b_del )
{
	setIgnoreChanges ( b_ignorechanges ); //once called, the callee must set/unset this property
	removeRow ( 0, rowCount (), b_del );
	mPrevRow = -2;
	mCurrentItem = nullptr;
}

void vmListWidget::insertRow ( const uint row, const uint n )
{
	if ( row <= (unsigned) rowCount () )
	{
		setVisibleRows ( visibleRows () +  n );
		for ( uint i_row ( 0 ); i_row < n; ++i_row )
			QTableWidget::insertRow ( row + i_row );
		setLastUsedRow ( rowCount () - 1 );
	}
}

void vmListWidget::removeRow ( const uint row, const uint n, const bool bDel )
{
	if ( row < (unsigned) rowCount () )
	{
		int i_row ( row + n - 1 );
		const bool bResetCurrentRow ( !isIgnoringChanges () && ( mCurrentItem ? (mCurrentItem->row () >= (signed) row) : true ) );
		
		setVisibleRows ( visibleRows () - n );
		if ( mPrevRow >= (signed) row )
			mPrevRow -= n;
		
		vmListItem* item ( nullptr );
		if ( bDel )
		{
			for ( ; i_row >= (signed) row; --i_row  )
			{
				for ( int i_col ( colCount () - 1 ); i_col >= 0; --i_col )
				{
					item = static_cast<vmListItem*>( sheetItem ( i_row, i_col ) );
					if ( item != nullptr )
					{
						item->m_list = nullptr;
						delete item;
					}
				}
				QTableWidget::removeRow ( i_row );
			}
		}
		else
		{
			for ( ; i_row >= (signed) row; --i_row  )
			{
				for ( int i_col ( colCount () - 1 ); i_col >= 0; --i_col )
				{
					static_cast<vmListItem*>( sheetItem ( i_row, i_col ) )->m_list = nullptr;
					(void) QTableWidget::takeItem ( i_row, i_col );
				}
				QTableWidget::removeRow ( i_row );
			}
		}
		if ( bResetCurrentRow )
			setCurrentRow ( -1, true );
		setLastUsedRow ( rowCount () - 1 );
	}
}

void vmListWidget::rowSelected ( const int row, const int prev_row )
{
	if ( mCurrentItem )
	{
		if ( mCurrentItem->row () == row )
			return;
		mPrevRow = prev_row;
	}
	else
		mPrevRow = -2;	
	
	mCurrentItem = item ( row );
	if ( !isIgnoringChanges () )
	{
		if ( mCurrentItemChangedFunc )
			mCurrentItemChangedFunc ( mCurrentItem );
	}
}