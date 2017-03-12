#include "vmcheckedtableitem.h"
#include "fast_library_functions.h"

#include <QPainter>
#include <QMouseEvent>

vmCheckedTableItem::vmCheckedTableItem ( const Qt::Orientation orientation, QWidget* parent )
	: QHeaderView ( orientation, parent ), checkChange_func ( nullptr), mColumnState ( 0 ), mbIsCheckable ( false )
{
	// set clickable by default
	setSectionsClickable ( true );
}

void vmCheckedTableItem::paintSection ( QPainter* painter, const QRect& rect, const int logicalIndex ) const
{
	if ( !mbIsCheckable )
		QHeaderView::paintSection ( painter, rect, logicalIndex );
	else
	{
		painter->save ();
		QHeaderView::paintSection ( painter, rect, logicalIndex );
		painter->restore ();
		QStyleOptionButton option;
		if ( isEnabled () )
			option.state |= QStyle::State_Enabled;
		option.rect = checkBoxRect ( rect );
		if ( isBitSet ( mColumnState, logicalIndex ) )
			option.state |= QStyle::State_On;
		else
			option.state |= QStyle::State_Off;
		style ()->drawControl ( QStyle::CE_CheckBox, &option, painter );
	}
}

void vmCheckedTableItem::mousePressEvent ( QMouseEvent* event )
{
	if ( mbIsCheckable )
	{
		const int column ( logicalIndexAt ( event->pos () ) );
		if ( column >= 0 )
			setChecked ( column, !isBitSet ( mColumnState, column ), true );
	}
	else
		QHeaderView::mousePressEvent ( event );
}

void vmCheckedTableItem::setCheckable ( const bool checkable )
{
	mbIsCheckable = checkable;
	// repaint (), update (), qApp->flush (), qApp->processEvents () .. nothing works but this
	parentWidget ()->resize ( parentWidget ()->width () + 1 , parentWidget ()->height () );
}

bool vmCheckedTableItem::isChecked ( const uint column ) const
{
	// when not checkable, all columns are considered checked
	return mbIsCheckable ? isBitSet ( mColumnState, column ) : true;
}

void vmCheckedTableItem::setChecked ( const uint column, const bool checked, const bool b_notify )
{
	if ( isEnabled () && ( isBitSet ( mColumnState, column ) != checked ) ) {
		checked ? setBit ( mColumnState, column ) : unSetBit ( mColumnState, column );
		updateSection ( column );
		if ( b_notify && checkChange_func )
			checkChange_func ( column, checked );
	}
}

QRect vmCheckedTableItem::checkBoxRect ( const QRect& sourceRect ) const
{
	QStyleOptionButton checkBoxStyleOption;
	const QRect checkBoxRect ( style ()->subElementRect ( QStyle::SE_CheckBoxIndicator, &checkBoxStyleOption ) );
	const QPoint checkBoxPoint ( sourceRect.x () + 2,
								 sourceRect.y () + sourceRect.height () / 2 -
								 checkBoxRect.height () / 2 );
	return QRect ( checkBoxPoint, checkBoxRect.size () );
}
