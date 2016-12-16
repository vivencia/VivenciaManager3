#include "vmtableutils.h"
#include "vmtablewidget.h"
#include "vmcheckedtableitem.h"
#include "vmlinefilter.h"

#include <QHBoxLayout>
#include <QShowEvent>
#include <QHideEvent>

vmTableSearchPanel::vmTableSearchPanel ( const vmTableWidget* const table )
	: QFrame ( nullptr ), m_SearchedWord ( emptyString ), m_table ( const_cast<vmTableWidget*>( table ) ),
	  btnSearchNext ( nullptr ), btnSearchPrev ( nullptr ), btnSearchCancel ( nullptr )
{
	txtSearchTerm = new vmLineEdit;
	txtSearchTerm->setEditable ( true );
	txtSearchTerm->setCallbackForRelevantKeyPressed ( ( [&]( const QKeyEvent* ke, const vmWidget* const ) {
		return txtSearchTerm_keyPressed ( ke ); } ) );

	btnSearchStart = new QToolButton;
	btnSearchStart->setIcon ( ICON ( "search" ) );
	btnSearchStart->setEnabled ( false );
	connect ( btnSearchStart, &QToolButton::clicked, this, [&] () { return searchStart (); } );

	btnSearchPrev = new QToolButton;
	btnSearchPrev->setIcon ( ICON ( "arrow-left" ) );
	btnSearchPrev->setEnabled ( false );
	connect ( btnSearchPrev, &QToolButton::clicked, this, [&] () { return searchPrev (); } );

	btnSearchNext = new QToolButton;
	btnSearchNext->setIcon ( ICON ( "arrow-right" ) );
	btnSearchNext->setEnabled ( false );
	connect ( btnSearchNext, &QToolButton::clicked, this, [&] () { return searchNext (); } );

	btnSearchCancel = new QToolButton;
	btnSearchCancel->setIcon ( ICON ( "cancel" ) );
	connect ( btnSearchCancel, &QToolButton::clicked, this, [&] () { return hide (); } );

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
	
	setMaximumHeight ( 30 );
}

void vmTableSearchPanel::showEvent ( QShowEvent* se )
{
	se->setAccepted ( false );
	QFrame::showEvent ( se );
	txtSearchTerm->setFocus ();
}

void vmTableSearchPanel::hideEvent ( QHideEvent* he )
{
	he->setAccepted ( false );
	QFrame::hideEvent ( he );
	m_table->searchCancel ();
	btnSearchNext->setEnabled ( false );
	btnSearchPrev->setEnabled ( false );
	txtSearchTerm->clear ();
	m_table->setFocus ();
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

vmTableFilterPanel::vmTableFilterPanel ( const vmTableWidget * const table )
	: QFrame ( nullptr ), m_table ( const_cast<vmTableWidget*>( table ) ), searchLevels ( 10 )
{
	searchLevels.setAutoDeleteItem ( true );
	
	m_btnClose = new QToolButton;
	m_btnClose->setIcon ( ICON ( "stylepanel/panelCloseOver" ) );
	connect ( m_btnClose, &QToolButton::clicked, this, [&] () { return hide (); } );
	
	QLabel* lblFilter ( new QLabel ( TR_FUNC ( "Filter:" ) ) );
	m_txtFilter = new vmLineFilter;
	m_txtFilter->setEditable ( true );
	m_txtFilter->setCallbackForValidKeyEntered ( [&] ( const triStateType level, const uint startlevel ) { return doFilter ( level, startlevel ); } );
	
	QHBoxLayout* mainLayout ( new QHBoxLayout );
	mainLayout->setMargin ( 2 );
	mainLayout->setSpacing ( 2 );
	mainLayout->addWidget ( lblFilter );
	mainLayout->addWidget ( m_txtFilter, 2 );
	mainLayout->addWidget ( m_btnClose );
	setLayout ( mainLayout );
	
	setMaximumHeight ( 30 );
}

void vmTableFilterPanel::showEvent ( QShowEvent* se )
{
	se->setAccepted ( true );
	QFrame::showEvent ( se );
	m_txtFilter->setFocus ();
}

void vmTableFilterPanel::hideEvent ( QHideEvent* he )
{
	he->setAccepted ( true );
	QFrame::hideEvent ( he );
	m_txtFilter->clear ();
	m_table->setFocus ();
}

void vmTableFilterPanel::doFilter ( const triStateType level, const uint startlevel )
{
	qDebug () << endl << "------------------------------" << endl;
	qDebug () << "vmTableFilterPanel::doFilter ( level = " << level.toInt() << ", startlevel = " << startlevel << ")"  << endl;
	
	if ( level == CLEAR_LEVEL ) {
		for ( int i_row ( 0 ); i_row <= m_table->lastUsedRow (); ++i_row )
			m_table->setRowVisible ( i_row, true );
		searchLevels.clearButKeepMemory ();
	}
	else {
		if ( startlevel < searchLevels.count () ) {
			qDebug () << "startlevel < searchLevels.count () " << searchLevels.count();
			for ( uint i ( startlevel ); i < searchLevels.count (); ++i )
				searchLevels.remove ( i, true );
			podList<int>* rowLevel ( searchLevels.at ( startlevel - 1 ) );
			if ( rowLevel ) {
				for ( uint i ( 0 ); i < rowLevel->count (); ++i )
					m_table->setRowVisible ( rowLevel->at ( i ), true );	
			}
		}
		if ( level == ADD_LEVEL ) {
			bool b_keeprow ( false );
			int i_row ( 0 );
			podList<int>* newRowLevel ( new podList<int> );
			newRowLevel->setPreAllocNumber ( m_table->visibleRows () );
			qDebug () << "Visible rows: " << m_table->visibleRows ();
			do {
				if ( !m_table->isRowHidden ( i_row ) ) {
					b_keeprow = false;
					for ( uint i_col ( 0 ); i_col < m_table->colCount (); ++i_col ) {
						if ( m_txtFilter->matches ( m_table->sheetItem ( i_row, i_col )->text () ) ) {
							b_keeprow = true;
							qDebug () << m_table->sheetItem ( i_row, i_col )->text () << "  at row  " << i_row << "  matches search string  " << m_txtFilter->buffer ();
							break;
						}
					}
					m_table->setRowVisible ( i_row, b_keeprow );
					if ( b_keeprow )
						newRowLevel->append ( i_row );
				}
			} while ( ++i_row <= m_table->lastUsedRow () );
			searchLevels.append ( newRowLevel );
		}
	}
	m_table->scrollToItem ( m_table->currentItem (), QAbstractItemView::PositionAtCenter );
}
