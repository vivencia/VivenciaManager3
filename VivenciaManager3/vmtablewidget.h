#ifndef VMTABLEWIDGET_H
#define VMTABLEWIDGET_H

#include "global.h"
#include "vmlist.h"
#include "vmtableitem.h"
#include "completers.h"
#include "vmnumberformats.h"
#include "vmwidget.h"
#include "stringrecord.h"

#include <QTableWidget>

class vmTableSearchPanel;
class vmTableFilterPanel;
class QToolButton;
class QVBoxLayout;
class QGridLayout;

struct vmTableColumn {
	PREDEFINED_WIDGET_TYPES wtype;
	vmWidget::TEXT_TYPE text_type;
	vmLineEditWithButton::LINE_EDIT_BUTTON_TYPE button_type;
	vmCompleters::COMPLETER_CATEGORIES completer_type;
	uint width;
	bool editable;
	QString label;
	QString formula;
	QString default_value;

	vmTableColumn () : wtype ( WT_LINEEDIT ), text_type ( vmLineEdit::TT_TEXT ),
		button_type ( vmLineEditWithButton::LEBT_NO_BUTTON ),
		completer_type ( vmCompleters::NONE ), width ( 0 ), editable ( true ),
		default_value () {}
};

enum contextMenuDefaultActions {
	UNDO = -1, COPY_CELL = -2, COPY_ROW = -3, ADD_ROW = -4, APPEND_ROW = -5, DEL_ROW = -6,
	CLEAR_ROW = -7,	CLEAR_TABLE = -8, DATE_TODAY = -9, DATE_YESTERDAY = -10, DATE_TOMORROW = -11,
	SET_FORMULA = -12, SET_FORMULA_OVERRIDE = -13, FORMULA_TITLE = -14
};

struct spreadRow {
	VMList<QString> field_value;
	podList<uint> column;
	int row;

	inline spreadRow ()
		: field_value ( emptyString, 12 ), column ( 0, 12 ), row ( -1 )
	{}
};

constexpr uint PURCHASES_TABLE_REG_COL ( 6 );
static const char* const PROPERTY_TABLE_HAS_ITEM_TO_REGISTER ( "pthitr" );

class vmTableWidget : public QTableWidget, public vmWidget
{

friend class vmTableSearchPanel;
friend class vmTableFilterPanel;
friend class vmTableItem;
friend class vmLineEdit;

public:
	explicit vmTableWidget ( QWidget* parent = nullptr ); // constructor for promoted widgets used by QtDesigner
	vmTableWidget ( const uint rows, QWidget* parent = nullptr );
	virtual ~vmTableWidget ();

	vmTableColumn* createColumns ( const uint nCols );
	void initTable ( const uint rows );

	bool selectFound ( const vmTableItem* item );
	bool searchStart ( const QString& searchTerm );
	void searchCancel ();
	bool searchFirst ();
	bool searchPrev ();
	bool searchNext ();
	bool searchLast ();
	inline const QString& searchTerm () const { return mSearchTerm; }

	// Convenient functions that are called by more than one module in the program. Put here to reduce code and binary size
	static vmTableWidget* createPurchasesTable ( vmTableWidget* table = nullptr, QWidget* parent = nullptr );
	static void exchangePurchaseTablesInfo (
		const vmTableWidget* const src_table, vmTableWidget* const dst_table,
        const DBRecord* const src_dbrec, DBRecord* const dst_dbrec );

	static vmTableWidget* createPayHistoryTable ( vmTableWidget* table = nullptr, QWidget* parent = nullptr,
			const PAY_HISTORY_RECORD last_column = PHR_USE_DATE );

	static void processPayHistoryForCalendar ( stringTable& pay_data, const vmTableItem* const item, vmTableWidget* table  );
	
	virtual void insertRow ( const uint row, const uint n = 1 );
	virtual void removeRow ( const uint row, const uint n = 1 );
	void setRowVisible ( const uint row, const bool bVisible );
	inline uint visibleRows () const { return m_nVisibleRows; }

	/* m_clearRowNotify is by default true because clearRow is to be called when the table is editable (in fact, if it's
     * called when not editable, the function will return and do nothing. Since it is editable, it should permit to
	 * send modification signals. However, when captured by outside functions that monitor changes and those functions
	 * should check for an empty row, before concluding that those changes came from a full row clearing, isRowEmpty will return
	 * false untill the last column, which is an undesired effect. In such cases, call clearRow with b_notify set to false and afterwards
	 * iterate through all items in the row, calling the monitor function for each item.
	 * The separate function to set the flag m_clearRowNotify is there because clearRow can be called dicrectly or via clearRow_slot
	 */
    //inline void setClearRowOrdersNotify ( const bool order_notify ) {
    //	m_clearRowNotify = order_notify;
    //}
	void clearRow ( const uint row, const uint n = 1 );
	void clear ( const bool force = false );
	inline void appendRow () {
		insertRow ( mbPlainTable ? rowCount () : mTotalsRow ); }

	void rowActivatedConnection ( const bool b_activate );
	void setCellValue ( const QString& value, const uint row, const uint col );
	void setRowData ( const spreadRow* s_row, const bool b_notify = false );
	void rowData ( const uint row, spreadRow* ) const;
	void cellContentChanged ( const vmTableItem* const item );
	void cellModified ( const vmTableItem* const item );

	void renameAction ( const contextMenuDefaultActions action, const QString& new_text );
	void addContextMenuAction ( vmAction* );
	void removeContextMenuAction ( vmAction* );

	void loadFromStringTable ( const stringTable& data );
	void makeStringTable ( stringTable& data );
	void setCellWidget ( vmTableItem* const sheet_item );
    void addToLayout ( QGridLayout* glayout, const uint row, const uint column );
    void addToLayout ( QVBoxLayout* vblayout, const uint strecth = 0 );
    // Call these when the table is laid out in the Designer
    bool setParentLayout ( QGridLayout* glayout );
    bool setParentLayout ( QVBoxLayout* vblayout );
	inline void setUtilitiesPlaceLayout ( QVBoxLayout* layoutUtilities ) { mLayoutUtilities = layoutUtilities; }

	void setIsList ();
	void setIsPlainTable ();
	inline bool isPlainTable () const { return mbPlainTable; }
	inline void setKeepModificationRecords ( const bool bkeeprec ) { mbKeepModRec = bkeeprec; }

	void setEditable ( const bool editable );

	inline bool isEmpty () const {
		return static_cast<bool> ( m_lastUsedRow == 0 ); }

	inline vmTableItem* sheetItem ( const int row, const int col ) const {
		return static_cast<vmTableItem*> ( this->item ( row, col ) ); }

	inline uint colCount () const { return m_ncols; }
	inline uint totalsRow () const { return mTotalsRow; }
	void setLastUsedRow ( const int row );
	inline int lastUsedRow () const { return m_lastUsedRow; }

	uint selectedRowsCount () const;
	bool isRowEmpty ( const uint row ) const;
	bool isRowSelected ( const uint row ) const;
	bool isRowSelectedAndNotEmpty ( const uint row ) const;

	inline bool tableChanged () const { return mTableChanged; }
	void setTableUpdated ();

	void setHiddenText ( const uint row, const uint col, const QString& , const bool notify_change = true );
	QString hiddenText ( const uint row, const uint col ) const;

	inline uint totalChangedRows () const { return modifiedRows.count (); }
	inline uint modifiedRow ( const uint idx ) const { return modifiedRows.at ( idx ); }

	void setColumnReadOnly ( const uint i_col, const bool readonly );
	int isCellMonitored ( const vmTableItem* const item );
	void insertMonitoredCell ( const uint row, const uint col );
	void removeMonitoredCell ( const uint row, const uint col );
	void setCellColor ( const uint row, const uint col, const Qt::GlobalColor color );

	void highlight ( const VMColors color, const QString& text );
	void highlightCell ( const int row, const int col, const VMColors color );
	void unHighlightCell ( const int row, const int col );

	bool isColumnSelectedForSearch ( const uint column ) const;
	void setColumnSearchStatus ( const uint column, const bool bsearch );
	void reHilightItems ( vmTableItem* next, vmTableItem* prev );

	inline static QString defaultBackgroundColor () { return vmTableWidget::defaultBGColor; }
	inline QModelIndex indexFromItem ( const vmTableItem* const item ) const { return QTableWidget::indexFromItem ( static_cast<QTableWidgetItem*>( const_cast<vmTableItem*>( item ) ) ); }

	// This first callback is mandatory. Because it is called in a loop, and quite frequently,
	// In order to avoid checking if cellChanged_func is not null in every iteration, we will assume it is not null
	inline void setCallbackForCellChanged ( std::function<void ( const vmTableItem* const item )> func ) {
		cellChanged_func = func; }
	inline void setCallbackForCellNavigation ( std::function<void ( const uint row, const uint col, const uint prev_row, const uint prev_col )> func ) {
		cellNavigation_func = func; }
	inline void setCallbackForMonitoredCellChanged ( std::function<void ( const vmTableItem* const item )> func ) {
		monitoredCellChanged_func = func; }
	inline void setCallbackForRowRemoved ( std::function<void ( const uint row )> func ) {
		rowRemoved_func = func; }
	inline void setCallbackForRowActivated ( std::function<void ( const int row )> func ) {
		rowActivated_func = func; }

	inline void callRowActivated_func ( const int row ) const { if ( rowActivated_func ) rowActivated_func ( row ); }
	
protected:
	void keyPressEvent ( QKeyEvent* k );
	inline void setVisibleRows ( const uint n ) { m_nVisibleRows = n; }

private:
	void enableOrDisableActionsForCell ( const vmTableItem* sheetItem );
	void sharedContructorsCode ();
	void fixTotalsRow ();
	void reScanItem ( vmTableItem* const sheet_item );
    void interceptCompleterActivated ( const QModelIndex& index, const vmTableWidget* const table = nullptr );
	void undoChange ();
	void copyCellContents ();
	void copyRowContents ();
	void insertRow_slot ();
	void appendRow_slot ();
	void removeRow_slot ();
	void clearRow_slot ();
	void clearTable_slot ();
	void setFormulaOverrideForCell ( vmTableItem* item, const bool boverride );
	void setFormulaForCell ( vmTableItem* item );

	void headerItemToggled ( const uint col, const bool checked );
	void textWidgetChanged ( const vmWidget* const sender );
	void dateWidgetChanged ( const vmWidget* const sender );
	void timeWidgetChanged ( const vmWidget* const sender );
	void checkBoxToggled ( const vmWidget* const sender );
	void displayContextMenuForCell ( const QPoint& pos, const vmWidget* cellWidget );
	void cellWidgetRelevantKeyPressed ( const QKeyEvent* const ke, const vmWidget* const widget );

	vmTableColumn* mCols;
	int mTotalsRow;
	int m_lastUsedRow;
	uint m_nVisibleRows;
	uint m_ncols;
	uint mAddedItems;
	bool mTableChanged;
    //bool m_clearRowNotify;

    /* TRUE: changes a recorded in a list so that at a later point this list can iterated
     * through and with the changes update some database table or something else.
     * FALSE: the changes are not kept and when one is signaled, the receiver must use that
     * value immediately otherwise it will no be possible to know, later, what was and was not changed
     */
    bool mbKeepModRec;
	bool mbPlainTable;
	bool mbTableIsList;
    bool mIsPurchaseTable;
	bool mbDoNotUpdateCompleters;

	// Because of the mask, the maximum number of columns is limited to 32
	uint readOnlyColumnsMask; // columns set here do not receive user input

	podList<uint> modifiedRows;
	PointersList<const vmTableItem*> mMonitoredCells;
	PointersList<vmTableItem*> m_highlightedCells;
	PointersList<vmTableItem*> m_itemsToReScan;
	PointersList<vmTableItem*> mSearchList;

	QMenu* mContextMenu, *mSubMenuFormula;
	vmAction* mCopyCellAction, *mCopyRowContents, *mInsertRowAction, *mAppendRowAction,
			  *mDeleteRowAction, *mClearRowAction, *mClearTableAction,
			  *mUndoAction, *mOverrideFormulaAction, *mSetFormulaAction, *mFormulaTitleAction;

	QString mSearchTerm;
    QLayout* mParentLayout;
	QAction* mSeparator, *mDatesSeparator;
	vmTableSearchPanel* m_searchPanel;
	vmTableFilterPanel* m_filterPanel;
	QVBoxLayout* mLayoutUtilities;
	vmTableItem* mContextMenuCell;

	static QString defaultBGColor;

	std::function<void ( const vmTableItem* const item )> cellChanged_func;
	std::function<void ( const uint row, const uint col, const uint prev_row, const uint prev_col )> cellNavigation_func;
	std::function<void ( const vmTableItem* const item )> monitoredCellChanged_func;
	std::function<void ( const uint row )> rowRemoved_func;
	std::function<void ( const int row )> rowActivated_func;
};

#endif // VMTABLEWIDGET_H
