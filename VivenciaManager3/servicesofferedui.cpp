#include "servicesofferedui.h"
#include "vivenciadb.h"
#include "vmtablewidget.h"
#include "standardprices.h"
#include "cleanup.h"
#include "heapmanager.h"

#include <QHBoxLayout>

servicesOfferedUI* servicesOfferedUI::s_instance ( nullptr );

void deleteServicesInstance ()
{
	heap_del ( servicesOfferedUI::s_instance );
}

servicesOfferedUI::servicesOfferedUI ()
	: QDialog ( nullptr ), so_rec ( new servicesOffered ), m_table ( nullptr )
{
	addPostRoutine ( deleteServicesInstance );
}

servicesOfferedUI::~servicesOfferedUI ()
{
	heap_del ( m_table );
	heap_del ( so_rec );
}

void servicesOfferedUI::showWindow ()
{
	if ( !m_table )
		setupUI ();
	show ();
}

void servicesOfferedUI::setupUI ()
{
	vmTableColumn *fields ( m_table->createColumns( SERVICES_FIELD_COUNT ) );
	uint i ( 0 );

	for ( ; i < SERVICES_FIELD_COUNT; ++i ) {
		fields[i].label = VivenciaDB::getTableColumnLabel ( &so_rec->t_info, i );
		switch ( i ) {
		case FLD_SERVICES_ID:
			fields[FLD_SERVICES_ID].editable = false;
			break;
		case FLD_SERVICES_TYPE:
		case FLD_SERVICES_BASIC_UNIT:
			break;
		case FLD_SERVICES_DISCOUNT_FROM:
			fields[FLD_SERVICES_DISCOUNT_FROM].text_type = vmLineEdit::TT_INTEGER;
			break;
		case FLD_SERVICES_LAST_UPDATE:
		case FLD_SERVICES_AVERAGE_TIME:
			fields[i].wtype = WT_DATEEDIT;
			break;
		case FLD_SERVICES_PRICE_PER_UNIT:
			fields[FLD_SERVICES_PRICE_PER_UNIT].text_type = vmLineEdit::TT_PRICE;
			break;
		case FLD_SERVICES_DISCOUNT_FACTOR:
			fields[FLD_SERVICES_DISCOUNT_FACTOR].text_type = vmLineEdit::TT_DOUBLE;
			break;
		}
	}

	m_table = new vmTableWidget ( 20 );
	connect ( m_table, SIGNAL ( spreadCellChanged ( const int, const int ) ) , this, SLOT ( tableChanged ( const int, const int ) ) );
	connect ( m_table, SIGNAL ( rowRemoved ( const uint ) ), this, SLOT ( rowRemoved ( const uint ) ) );
	connect ( m_table, SIGNAL ( currentCellChanged ( const int, const int, const int, const int ) ), this, SLOT ( readRowData ( const int, const int, const int, const int ) ) );

	VDB ()->populateTable ( so_rec, m_table );

	QHBoxLayout* layout = new QHBoxLayout;
	layout->addWidget ( m_table, 1 );
	setLayout ( layout );
	setMinimumSize ( 800, 500 );
}

void servicesOfferedUI::readRowData ( const int row, const int, const int prev_row, const int )
{
	if ( m_table->isEditable () ) {
		if ( prev_row != row ) {
			so_rec->setAction ( ACTION_READ );
			for ( uint i_col ( 0 ) ; i_col < unsigned ( m_table->columnCount () ); ++i_col )
				so_rec->setValue ( i_col, m_table->sheetItem ( row, i_col )->text () );
		}
	}
}

void servicesOfferedUI::tableChanged ( const int row, const int col )
{
	if ( m_table->sheetItem ( row, 0 )->text ().isEmpty () )
		so_rec->setAction ( ACTION_ADD );
	else
		so_rec->setAction ( ACTION_EDIT );

	setRecValue ( so_rec, col, m_table->sheetItem ( row, col )->text () );
	so_rec->saveRecord ();
	m_table->setTableUpdated ();
}

void servicesOfferedUI::rowRemoved ( const uint )
{
	so_rec->deleteRecord (); // deletes from database before removing the row
}
