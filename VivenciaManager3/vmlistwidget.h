#ifndef VMLISTWIDGET_H
#define VMLISTWIDGET_H

#include "vmtablewidget.h"
#include "vmlistitem.h"

class QResizeEvent;

class vmListWidget : public vmTableWidget
{

public:
	vmListWidget ( QWidget* parent = nullptr, const uint nRows = 0 );
	virtual ~vmListWidget ();

	void setIgnoreChanges ( const bool b_ignore );
	inline bool isIgnoringChanges () const { return mbIgnore; }
	inline void setCurrentItem ( const vmListItem* const item, const bool b_makecall ) { setCurrentRow ( item ? item->row () : -1, b_makecall ); }
	void setCurrentRow ( int row, const bool b_makecall );
	inline vmListItem* currentItem () const { return mCurrentItem; }
	void addItem ( vmListItem* item, const bool b_makecall = true );
	inline void removeItem ( vmListItem* item, const bool bDel = false ) { removeRow ( item->row (), 1, bDel ); }
	void insertRow ( const uint row, const uint n = 1 );
	void removeRow ( const uint row, const uint n = 1, const bool bDel = false );
	void clear ( const bool b_ignorechanges = true, const bool b_del = false );
	inline int count () const { return rowCount (); }
	inline vmListItem* item ( const int row ) const { return static_cast<vmListItem*>( sheetItem ( row >= 0 ? row : mPrevRow, 0 ) ); }
	
	/* Default: false. Many items are managed elsewhere because they are used to more than just displaying rows of
	 * text. This property is used to allow for the proper cleaning of garbage in addition to avoid the inadvertent
	 * attempt to delete something that may be deleted elsewhere another time.
	 */
	inline void setDeleteItemsWhenDestroyed ( const bool b_destroydelete ) { mbDestroyDelete = b_destroydelete; }
	inline bool isDeleteItemsWhenDestroyed () const { return mbDestroyDelete; }
	
	inline void setCallbackForCurrentItemChanged ( std::function<void( vmListItem* current )> func ) {
		mCurrentItemChangedFunc = func; }

protected:
	void resizeEvent ( QResizeEvent* e );
	
private:
	void rowSelected ( const int row, const int prev_row );
	
	bool mbIgnore, mbDestroyDelete;
	int mPrevRow;
	vmListItem* mCurrentItem;
	std::function<void( vmListItem* current )> mCurrentItemChangedFunc;
};

#endif // VMLISTWIDGET_H
