#include "dbtableview.h"
#include "global.h"
#include "heapmanager.h"
#include "cleanup.h"
#include "vmlistwidget.h"
#include "vmlistitem.h"
#include "vivenciadb.h"
#include "vmtablewidget.h"
#include "vmnotify.h"

#include <QSplitter>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QLabel>
#include <QSqlQuery>

static const QString information_schema_tables[] = {
	"TABLE_CATALOG", "TABLE_SCHEMA",
	"TABLE_NAME", "TABLE_TYPE", "ENGINE", "VERSION", "ROW_FORMAT", "TABLE_ROWS", "AVG_ROW_LENGTH",
	"DATA_LENGTH", "MAX_DATA_LENGTH", "INDEX_LENGTH", "DATA_FREE", "AUTO_INCREMENT", "CREATE_TIME",
	"UPDATE_TIME", "CHECK_TIME", "TABLE_COLLATION", "CHECKSUM", "CREATE_OPTIONS", "TABLE_COMMENT"
};

dbTableView* dbTableView::s_instance ( nullptr );

void deleteDBTableViewInstance ()
{
	heap_del ( dbTableView::s_instance );
}

dbTableView::dbTableView ()
	: QObject ( nullptr )
{
	mTablesList = new vmListWidget;
	mTablesList->setMaximumWidth ( 300 );
	mTablesList->setCallbackForRowActivated ( [&] ( const int row ) { return showTable ( mTablesList->item ( row )->text () ); } );
			
	QVBoxLayout* vLayout ( new QVBoxLayout );
	vLayout->addWidget ( new QLabel ( TR_FUNC ( "Available tables in the database:" ) ) );
	vLayout->addWidget ( mTablesList, 3 );
	
	mLeftFrame = new QFrame;
	mLeftFrame->setLayout ( vLayout );
	
	mTabView = new QTabWidget;
	mTabView->setTabsClosable ( true );
	mTabView->connect ( mTabView, &QTabWidget::tabCloseRequested, mTabView, [&] ( const int idx ) { mTabView->removeTab ( idx ); } );
	
	mMainLayoutSplitter = new QSplitter;
	mMainLayoutSplitter->insertWidget ( 0, mLeftFrame );
	mMainLayoutSplitter->insertWidget ( 1, mTabView );
	mMainLayoutSplitter->setStretchFactor ( 0, 1 );
	mMainLayoutSplitter->setStretchFactor ( 1, 4 );

	mMainLayout = new QVBoxLayout;
	mMainLayout->addWidget ( mMainLayoutSplitter, 2 );
	addPostRoutine ( deleteDBTableViewInstance );
}

dbTableView::~dbTableView ()
{
	return;
}

void dbTableView::reload ()
{
	mTablesList->setIgnoreChanges ( true );
	loadTablesIntoList ();
	mTablesList->setIgnoreChanges ( false );
}

void dbTableView::loadTablesIntoList ()
{
	mTablesList->clear ( true, true );
	QSqlQuery queryRes;
	if ( VDB ()->runQuery ( QStringLiteral ( "SHOW TABLES" ), queryRes ) )
	{
		do
		{
			mTablesList->addItem ( new vmListItem ( queryRes.value ( 0 ).toString () ) );
		} while ( queryRes.next () );
	}
	for ( int i ( 0 ); i < mTabView->count (); ++i )
	{
		tableViewWidget* widget ( static_cast<tableViewWidget*>( mTabView->widget ( i ) ) );
		if ( widget && widget->isVisible () )
			widget->reload ();
	}
}

void dbTableView::showTable ( const QString& tablename )
{
	bool b_widgetpresent ( false );
	for ( int i ( 0 ); i < mTabView->count (); ++i )
	{
		tableViewWidget* widget ( static_cast<tableViewWidget*>( mTabView->widget ( i ) ) );
		if ( widget )
		{
			if ( widget->tableName () == tablename )
			{
				if ( widget->isVisible () )
				{
					widget->reload ();
					mTabView->setCurrentWidget ( widget );
					widget->show ();
				}
				else
				{
					mTabView->setCurrentIndex ( mTabView->indexOf ( widget ) );
				}
				b_widgetpresent = true;
			}
		}
	}
	if ( !b_widgetpresent )
	{
		tableViewWidget* widget ( new tableViewWidget ( tablename ) );
		mTabView->setCurrentIndex ( mTabView->addTab ( widget, tablename ) );
		widget->setFocus ();
	}
}

tableViewWidget::tableViewWidget ( const QString& tablename )
	: QFrame ( nullptr ), m_table ( nullptr ), m_cols ( QString::null, 10 ), m_tablename ( tablename ), m_nrows ( 0 ), mb_loaded ( false )
{
	mLayout = new QVBoxLayout;
	setLayout ( mLayout );
	createTable ();
	showTable ();
}

tableViewWidget::~tableViewWidget () {}

void tableViewWidget::showTable ()
{
	if ( !mb_loaded )
		load ();
	show ();
}

void tableViewWidget::load ()
{
	QString str_tables;
	for ( uint i ( 0 ); i < m_cols.count (); ++i )
		str_tables += m_cols.at ( i ) + CHR_COMMA;
	str_tables.chop ( 1 );

	QSqlQuery query;	
	/* From the Qt's doc: Using SELECT * is not recommended because the order of the fields in the query is undefined.*/
	if ( VDB ()->runQuery ( QLatin1String ( "SELECT " ) + str_tables + QLatin1String ( " FROM " ) + tableName (), query ) )
	{
		uint row ( 0 );
		uint col ( 0 );
		
		//m_table->enableQtListenerToSimpleTableItemEdition ( false );
		do {
			do {
				m_table->sheetItem ( row, col )->setText ( query.value ( static_cast<int>( col ) ).toString (), false );
			} while ( ++col < m_cols.count () );
			col = 0;
			++row;
		} while ( query.next () );
		m_table->enableQtListenerToSimpleTableItemEdition ( true );
	}
	getTableLastUpdate ( m_updatedate, m_updatetime ); // save the last update time to avoid unnecessary reloads
	mb_loaded = true;
}

void tableViewWidget::reload ()
{
	bool b_doreload ( false );
	
	vmNumber update_date, update_time;
	getTableLastUpdate ( update_date, update_time );
	if ( update_date <= m_updatedate )
	{
		if ( update_time > m_updatetime )
			b_doreload = true;
	}
	else
		b_doreload = true;
	
	if ( b_doreload )
	{
		//Reload column information
		VMList<QString> old_cols ( m_cols );
		getTableInfo ();
		if ( m_cols.count () != old_cols.count () )
		{
			mLayout->removeWidget ( m_table );
			delete m_table;
			createTable ();
		}
		for ( uint i ( 0 ); i < m_cols.count (); ++i )
		{
			if ( m_cols.at ( i ) != old_cols.at ( i ) )
				m_table->horizontalHeaderItem ( static_cast<int>( i ) )->setText ( m_cols.at ( i ) );
		}
		
		getTableLastUpdate ( m_updatedate, m_updatetime ); // save the last update time to avoid unnecessary reloads
		m_table->clear ( true );
		
		if ( m_nrows >= static_cast<uint>( m_table->rowCount () ) )
			m_table->insertRow ( static_cast<uint>( m_table->rowCount () ), m_nrows - static_cast<uint>( m_table->rowCount () ) + 1 );
		load ();
	}
}

void tableViewWidget::focusInEvent ( QFocusEvent* e )
{
	QFrame::focusInEvent ( e );
	m_table->setFocus ();
}

void tableViewWidget::createTable ()
{
	getTableInfo ();
	
	m_table = new vmTableWidget;
	vmTableColumn* columns ( m_table->createColumns ( m_cols.count () ) );
	for ( uint i ( 0 ); i < m_cols.count (); ++i )
	{
		columns[i].label = m_cols.at ( i );
	}
	m_table->setIsPlainTable ( false );
	m_table->setCallbackForCellChanged ( [&] ( const vmTableItem* const item ) { return updateTable ( item ); } );
	m_table->initTable ( m_nrows );
	m_table->setMinimumWidth ( 800 );
	m_table->setColumnsAutoResize ( true );
	m_table->addToLayout ( mLayout, 3 );
	
	vmAction* actionReload ( new vmAction ( 0 ) );
	actionReload->setShortcut ( QKeySequence ( Qt::Key_F5 ) );
	static_cast<void>( connect ( actionReload, &QAction::triggered, this, [&] ( const bool ) { return reload (); } ) );	
	addAction ( actionReload );
}

void tableViewWidget::getTableInfo ()
{
	QSqlQuery query;
	if ( VDB ()->runQuery ( QLatin1String ( "DESCRIBE " ) + tableName (), query ) )
	{
		do {
			m_cols.append ( query.value ( 0 ).toString () );
		} while ( query.next () );
	}
	query.clear ();
	if ( VDB ()->runQuery ( QLatin1String ( "SELECT " ) + information_schema_tables[7] + QLatin1String ( " FROM INFORMATION_SCHEMA.TABLES WHERE TABLE_NAME LIKE '" ) + tableName () + QLatin1Char ( '\'' ), query ) )
		m_nrows = query.value ( 0 ).toUInt ();
}

void tableViewWidget::getTableLastUpdate ( vmNumber& date, vmNumber& time )
{
	QSqlQuery query;
	if ( VDB ()->runQuery ( QLatin1String ( "SELECT " ) + information_schema_tables[15] + QLatin1String ( " FROM INFORMATION_SCHEMA.TABLES WHERE TABLE_NAME LIKE '" ) + tableName () + QLatin1Char ( '\'' ), query ) )
	{
		const QString query_res ( query.value ( 0 ).toString () );
		date.dateFromMySQLDate ( query_res.left ( 10 ) );
		time.fromTrustedStrTime ( query_res.right ( 8 ), vmNumber::VTF_24_HOUR, true );
	}
}

void tableViewWidget::updateTable ( const vmTableItem* const item )
		
{
	const QString record_id ( m_table->sheetItem ( static_cast<uint>( item->row () ), 0 )->text () );
	const uint field ( static_cast<uint>( item->column () ) );
	QSqlQuery query;
	if ( VDB ()->runQuery ( QLatin1String ( "UPDATE " ) + tableName () + QLatin1String ( " SET " ) +
							m_cols.at ( field ) + QLatin1String ( "=\'" ) + item->text () + QLatin1String ( "\' WHERE ID=" )
							+ record_id, query ) )
	{
		getTableLastUpdate ( m_updatedate, m_updatetime ); // save the last update time to avoid unnecessary reloads
		
		VM_NOTIFY ()->notifyMessage ( TR_FUNC ( "Warning!! Table %1 updated" ).arg ( tableName () ), 
									  TR_FUNC ( "The record id (%1) was updated from the old value (%2) to the new (%3)" ).arg ( 
										  record_id, item->prevText (), item->text () ) );
	}
}
