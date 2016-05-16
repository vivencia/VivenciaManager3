#include "vmtableitem.h"
#include "vmtablewidget.h"
#include "vmnumberformats.h"
#include "vmwidgets.h"
#include "heapmanager.h"

static const QString SUM ( QStringLiteral ( "sum" ) );

static void decode_pos ( const QString& pos, int* const row, int* const col )
{
	if ( pos.isEmpty () ) {
		*col = -1;
		*row = -1;
	}
	else {
		*col = pos.at ( 0 ).toLatin1 () - 'A';
		*row = pos.right ( pos.size () - 1 ).toInt ();
	}
}

vmTableItem::vmTableItem ( const PREDEFINED_WIDGET_TYPES wtype,
						   const vmLineEdit::TEXT_TYPE ttype, const QString& text, const vmTableWidget* table )
	: QTableWidgetItem ( text ), vmWidget ( WT_TABLE_ITEM ),
	  m_wtype ( wtype ), m_texttype ( ttype ), m_btype ( vmLineEditWithButton::LEBT_NO_BUTTON ), mb_hasFormula ( false ), mb_formulaOverride ( false ), mb_customFormula ( false ),
	  mb_CellAltered ( false ), mDefaultValue ( text ), mCache ( text ), m_table ( const_cast<vmTableWidget*> ( table ) ),
	  m_targets ( 4 ), m_widget ( nullptr )
{}

vmTableItem::vmTableItem ( const QString& text )
	: QTableWidgetItem ( text ), vmWidget ( WT_LISTITEM ),
	  m_wtype ( WT_WIDGET_UNKNOWN ), m_texttype ( vmLineEdit::TT_TEXT ), m_btype ( vmLineEditWithButton::LEBT_NO_BUTTON ), mb_hasFormula ( false ), mb_formulaOverride ( false ), mb_customFormula ( false ),
	  mb_CellAltered ( false ), mDefaultValue ( text ), mCache ( text ), m_table ( nullptr ),
	  m_targets ( 0 ), m_widget ( nullptr )
{}

vmTableItem::~vmTableItem () {}

void vmTableItem::setEditable ( const bool editable )
{
	if ( m_widget )
		m_widget->setEditable ( editable );
	vmWidget::setEditable ( editable );
}

void vmTableItem::setText ( const QString& text, const bool b_from_cell_itself,
							const bool force_notify, const bool b_formulaResult )
{
	if ( b_formulaResult ) {
		 if ( formulaOverride () )
			return;
	}
	else {
        if ( hasFormula () && b_from_cell_itself )
			setFormulaOverride ( true );
	}

	if ( !isEditable () ) {
		mBackupData_cache = text;
		mprev_datacache = text;
	}
	else {
		mb_CellAltered = true;
		mprev_datacache = mCache;
	}

	mCache = text;
	if ( m_table )
		m_table->setLastUsedRow ( row () );

	for ( uint i ( 0 ); i < m_targets.count (); ++i )
		m_targets.at ( i )->computeFormula ();

	// The call to change the widget's text must be the last operation so that callbackers can
	// take the item's text (if so they wish) when there is a signal call by the widget, and get
	// the updated value, instead of the old one
	if ( !b_from_cell_itself && m_widget )
		m_widget->setText ( text, force_notify );
}

void vmTableItem::setDate ( const vmNumber& date )
{
	if ( m_widget->type () == WT_DATEEDIT )
		static_cast<vmDateEdit*> ( m_widget )->setDate ( date.toQDate (), isEditable () );
}

vmNumber vmTableItem::date ( const bool bCurText ) const
{
	if ( m_widget->type () == WT_DATEEDIT ) {
		return bCurText ? static_cast<vmDateEdit*> ( m_widget )->date () :
					vmNumber ( originalText (), VMNT_DATE, vmNumber::VDF_HUMAN_DATE );
	}
	return vmNumber::emptyNumber;
}

static VM_NUMBER_TYPE textTypeToNbrType ( const vmLineEdit::TEXT_TYPE tt )
{
	switch ( tt ) {
		case vmWidget::TT_PRICE:	return VMNT_PRICE;
		case vmWidget::TT_DOUBLE:	return VMNT_DOUBLE;
		case vmWidget::TT_PHONE:	return VMNT_PHONE;
		case vmWidget::TT_INTEGER:	return VMNT_INT;
		default:					return VMNT_UNSET;
	}
}

vmNumber vmTableItem::number ( const bool bCurText ) const
{
	if ( m_texttype != vmLineEdit::TT_TEXT )
        return vmNumber ( bCurText ? text () : originalText (), textTypeToNbrType ( textType () ), 1 );
	return vmNumber::emptyNumber;
}

void vmTableItem::highlight ( const VMColors color, const QString& )
{
	m_widget->highlight ( color );
}

QVariant vmTableItem::data ( const int role ) const
{
	//if ( row () >= 0 )
	//	return QVariant (); // TEST

	switch ( role ) {
		case Qt::DisplayRole:
		case Qt::StatusTipRole:
		case Qt::EditRole:
			return mCache;
		break;
		//case Qt::TextAlignmentRole:
		//	return static_cast<int> ( Qt::AlignCenter );
		//break;
		default:
			return QTableWidgetItem::data ( role );
		break;
	}
}

void vmTableItem::targetsFromFormula ()
{
	const QStringList list ( mStr_Formula.split ( CHR_SPACE ) );
	if ( list.isEmpty () )
		return;

	if ( list.count () > 1 ) {
		vmTableItem* sheet_item ( nullptr );
		int firstRow ( 0 );
		int firstCol ( 0 );
		int secondRow ( 0 );
		int secondCol ( 0 );
		decode_pos ( list.value ( 1 ), &firstRow, &firstCol );
		decode_pos ( list.value ( 2 ), &secondRow, &secondCol );

		mStrOp = list.value ( 0 ).toLower ();

		if ( mStrOp == SUM ) {
			for ( int r ( firstRow ); r <= secondRow; ++r ) {
				for ( int c ( firstCol ); c <= secondCol; ++c ) {
					sheet_item = m_table->sheetItem ( r, c );
					if ( sheet_item ) {
						if ( textType () >= vmLineEdit::TT_PRICE ) {
							if ( sheet_item->m_targets.contains ( this ) == -1 )
								sheet_item->m_targets.append ( this );
						}
					}
					else {
						/* This cell depends on value of some cell that is not created yet
						 * We can break the calculation now, because it is pointless, and resume
						 * at a later point, but never during the actual use of a table. All this
						 * means a little longer to show the table, but smoother operation
						 */
						m_table->reScanItem ( this );
						return;
					}
				}
			}
		}
		else {
			sheet_item = m_table->sheetItem ( firstRow, firstCol );
			if ( sheet_item ) {
				if ( sheet_item->m_targets.contains ( this ) == -1 )
					sheet_item->m_targets.append ( this );
				if ( secondRow != -1 ) {
					sheet_item = m_table->sheetItem ( secondRow, secondCol );
					if ( sheet_item ) {
						if ( sheet_item->m_targets.contains ( this ) == -1 )
							sheet_item->m_targets.append ( this );
					}
				}
			}
			else {
				m_table->reScanItem ( this );
				return;
			}
		}
	}
	computeFormula ();
}

void vmTableItem::setFormula ( const QString& formula_template,
							   const QString& firstValue, const QString& secondValue )
{
	mStr_FormulaTemplate = formula_template;
	mStr_Formula = secondValue.isEmpty () ? formula_template.arg ( firstValue ) : formula_template.arg ( firstValue, secondValue );
	mb_hasFormula = true;
	setData ( Qt::ToolTipRole, mStr_Formula );
	targetsFromFormula ();
}

bool vmTableItem::setCustomFormula ( const QString& strFormula )
{
	//TODO TODO TODO TODO TODO
	mb_customFormula = true;
	mStr_Formula = strFormula;
	mb_hasFormula = true;
	setData ( Qt::ToolTipRole, mStr_Formula );
	targetsFromFormula ();
	return true;
}

void vmTableItem::computeFormula ()
{
	const QStringList list ( mStr_Formula.split ( CHR_SPACE ) );
	int firstRow ( 0 );
	int firstCol ( 0 );
	int secondRow ( 0 );
	int secondCol ( 0 );
	decode_pos ( list.value ( 1 ), &firstRow, &firstCol );
	decode_pos ( list.value ( 2 ), &secondRow, &secondCol );

	vmNumber res;
	if ( mStrOp == SUM ) {
		vmTableItem* tableItem ( nullptr );
		for ( int r ( firstRow ); r <= secondRow; ++r ) {
			for ( int c ( firstCol ); c <= secondCol; ++c ) {
				tableItem = m_table->sheetItem ( r, c );
				if ( tableItem ) {
					if ( textType () >= vmLineEdit::TT_PRICE )
						res += tableItem->number ();
				}
			}
		}
	}
	else {
		vmNumber firstVal ( m_table->sheetItem ( firstRow, firstCol )->number () );
		vmNumber secondVal;
		if ( secondRow != -1 )
			secondVal = m_table->sheetItem ( secondRow, secondCol )->number ();

		switch ( mStrOp.constData ()->toLatin1 () ) {
			case '+':
				res = ( firstVal + secondVal ); break;
			case '-':
				res = ( firstVal - secondVal ); break;
			case '*':
				res = ( firstVal * secondVal ); break;
			case '/':
				res = ( firstVal / secondVal ); break;
			case '=':
				res = firstVal;	break;
			default: break;
		}
	}


	/*
	 * Force the widget to change its text and emit a signal to notify the callback,
	 * but only do that when editing the table. Changes when the table is read-only must not
	 * be captured. Since most formula cells are read-only, we need to check the table's
	 * state for an accurate position on the editing status.
	 */
	setText ( res.toString (), false, m_table->isEditable (), true );
}

QString vmTableItem::spreadItemToString () const
{
	stringRecord strItem;
	strItem.fastAppendValue ( QStringLiteral ( "##&&" ) );
	strItem.fastAppendValue ( QString::number ( widgetType () ) );
	strItem.fastAppendValue ( formula () );
	strItem.fastAppendValue ( mCache.toString () );
	return strItem.toString ();
}
