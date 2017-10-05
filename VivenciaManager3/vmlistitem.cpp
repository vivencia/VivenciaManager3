#include "vmlistitem.h"
#include "vmlistwidget.h"
#include "client.h"
#include "job.h"
#include "payment.h"
#include "purchases.h"
#include "global_enums.h"
#include "data.h"
#include "crashrestore.h"
#include "heapmanager.h"
#include "stringrecord.h"

#include <QBrush>
#include <QApplication>

const QString vmListItem::actionSuffix[4] = {
	emptyString, QStringLiteral (  " - DEL" ),
	QStringLiteral ( " - NEW" ), QStringLiteral ( " - EDIT" )
};

#define CLIENT_REC static_cast<Client*>( vmListItem::m_dbrec )
#define JOB_REC static_cast<Job*>( vmListItem::m_dbrec )
#define PAY_REC static_cast<Payment*>( vmListItem::m_dbrec )
#define BUY_REC static_cast<Buy*>( vmListItem::m_dbrec )

const Qt::GlobalColor COLORS[4] = { Qt::white, Qt::red, Qt::green, Qt::blue };

void item_swap ( vmListItem& item1, vmListItem& item2 )
{
	table_item_swap ( static_cast<vmTableItem&>( item1 ), static_cast<vmTableItem&>( item2 ) );
	
	using std::swap;
	swap ( item1.m_crashid, item2.m_crashid );
	swap ( item1.mRelation, item2.mRelation );
	swap ( item1.mLastRelation, item2.mLastRelation );
	swap ( item1.item_related, item2.item_related );
	swap ( item1.m_list, item2.m_list );
	swap ( item1.n_badInputs, item2.n_badInputs );
	swap ( item1.mTotal_badInputs, item2.mTotal_badInputs );
	swap ( item1.badInputs_ptr, item2.badInputs_ptr );
	swap ( item1.mbSearchCreated, item2.mbSearchCreated );
	swap ( item1.mbInit, item2.mbInit );
	swap ( item1.mbSorted, item2.mbSorted );
	swap ( item1.searchFields, item2.searchFields );
	swap ( item1.m_dbrec, item2.m_dbrec );
	swap ( item1.m_action, item2.m_action );
}

vmListItem::vmListItem ()
	: vmTableItem (), m_crashid ( -1 ), m_dbrec ( nullptr ), mRelation ( RLI_CLIENTITEM ), mLastRelation ( RLI_CLIENTITEM ), 
	  searchFields ( nullptr ), item_related { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr },
	  m_action ( ACTION_NONE ), m_list ( nullptr ), badInputs_ptr ( nullptr ),
	  n_badInputs ( 0 ), mTotal_badInputs ( 0 ), mbSearchCreated ( false ), mbInit ( true ), mbSorted ( false )
{
	setAction ( ACTION_READ );
	setFlags ( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren ); //TEST
	mbInit = false;
}

vmListItem::vmListItem ( const uint type_id, const uint nbadInputs, bool* const badinputs_ptr )
	: vmListItem ()
{
	mTotal_badInputs = nbadInputs;
	badInputs_ptr = badinputs_ptr;
	setSubType ( static_cast<int>( type_id ) );
}

void vmListItem::copy ( const vmListItem& src_item )
{
	this->vmTableItem::copy ( src_item );
	setCrashID ( src_item.crashID () );
	setRelation ( src_item.relation () );
	setLastRelation ( src_item.lastRelation () );
	std::copy ( src_item.item_related, src_item.item_related + 20, item_related );
	n_badInputs = src_item.n_badInputs;
	mTotal_badInputs = src_item.mTotal_badInputs;
	std::copy ( src_item.badInputs_ptr, src_item.badInputs_ptr + src_item.n_badInputs, badInputs_ptr );
	mbSearchCreated = src_item.mbSearchCreated;
	mbInit = src_item.mbInit;
	mbSorted = src_item.mbSorted;
	std::copy ( src_item.searchFields, src_item.searchFields + src_item.m_dbrec->fieldCount (), searchFields );
	setDBRec ( src_item.dbRec (), true );
	setDBRecID ( src_item.dbRecID () );
	setAction ( src_item.action () );
	setText ( src_item.text (), false, false, false );
	addToList ( src_item.listWidget (), false );
}

vmListItem::vmListItem ( const QString& label )
	: vmListItem ()
{
	setText ( label, false, false, false );
}

vmListItem::~vmListItem ()
{
	if ( !Data::EXITING_PROGRAM )
		disconnectRelation ( static_cast<int>( RLI_CLIENTITEM ), this );
	if ( mbSearchCreated )
		delete[] searchFields;
}

void vmListItem::setRelation ( const uint relation )
{
	mRelation = relation;
	setRelatedItem ( relation, this );
	relationActions ();
}

void vmListItem::disconnectRelation ( const uint start_relation, vmListItem* item )
{
	for ( uint i ( start_relation ); i <= lastRelation (); ++i )
	{
		if ( relatedItem ( static_cast<RELATED_LIST_ITEMS>( i ) ) == item )
			setRelatedItem ( static_cast<RELATED_LIST_ITEMS>( i ), nullptr );
		else
		{
			if ( relatedItem ( static_cast<RELATED_LIST_ITEMS>( i ) ) != nullptr )
				relatedItem ( static_cast<RELATED_LIST_ITEMS>( i ) )->disconnectRelation ( i+1, item );
		}
	}
}

void vmListItem::syncSiblingWithThis ( vmListItem* sibling )
{
	if ( relation () != sibling->relation () )
	{
		if ( sibling->relation () > lastRelation () )
			setLastRelation ( sibling->relation () );
		sibling->setDBRecID ( dbRecID () );
		sibling->setDBRec ( dbRec (), true );
		sibling->setLastRelation ( lastRelation () );
		this->setRelatedItem ( sibling->relation (), sibling );
		relationActions ( sibling );
		for ( uint i ( RLI_CLIENTPARENT ); i <= lastRelation (); ++i )
			sibling->setRelatedItem ( static_cast<RELATED_LIST_ITEMS>( i ), this->relatedItem ( static_cast<RELATED_LIST_ITEMS>( i ) ) );
		sibling->setAction ( action (), false, true );
	}
}

void vmListItem::addToList ( vmListWidget* const w_list, const bool b_makecall )
{
	if ( m_list != w_list )
	{
		if ( m_list != nullptr )
			m_list->removeItem ( this );
		m_list = w_list;
	}
	w_list->addItem ( this, b_makecall );
}

void vmListItem::setAction ( const RECORD_ACTION action, const bool bSetDBRec, const bool bSelfOnly )
{
	if ( action != m_action )
	{
		/* When this item was added to listWidget () for the first time, sorting was disabled if it were previously enabled.
		 * This item was in adding mode so it went right to the top of the list. Now this is being saved, in other words,
		 * it's accepted into the list, so we remove the item (do not delete it), enable sorting back on and re-add it to the
		 * list. It will go to the right row
		 */
		
		if ( itemIsSorted () && action == ACTION_READ )
		{
			if ( m_action == ACTION_ADD )
			{
				vmListWidget* parentList ( listWidget () );
				parentList->setIgnoreChanges ( true );
				parentList->removeItem ( this );
				parentList->setSortingEnabled ( true );
				parentList->setIgnoreChanges ( false );
				addToList ( parentList );
			}
		}
		
		m_action = action != ACTION_REVERT ? action : ACTION_READ;
		// Since m_dbrec is a shared pointer among all related items, only the first -external- call
		// to setAction must change. All subsequent calls -internal- can skip these steps
		if ( m_dbrec && bSetDBRec )
			m_dbrec->setAction ( action );

		if ( !mbInit )
			changeAppearance ();
		
		n_badInputs = action == ACTION_ADD ? static_cast<int>(mTotal_badInputs) : 0;
		for ( uint i ( 0 ); i < mTotal_badInputs; ++i )
			badInputs_ptr[i] = action == ACTION_ADD ? false : true;
		
		// update all related items, except self. Call setAction with self_only to true so that we don't enter infinite recurssion.
		if ( !bSelfOnly && relation () == RLI_CLIENTITEM )
		{
			for ( uint i ( RLI_CLIENTITEM + 1 ); i <= lastRelation (); ++i )
			{
				if ( relatedItem ( static_cast<RELATED_LIST_ITEMS>( i ) ) != nullptr )
					relatedItem ( static_cast<RELATED_LIST_ITEMS>( i ) )->setAction ( m_action, false, true );
			}
		}
	}
}

void vmListItem::setDBRec ( DBRecord* dbrec, const bool self_only )
{
	// update all related items, except self. Call setAction with self_only to true so that we don't enter infinite recurssion.
	if ( !self_only )
	{
		for ( uint i ( RLI_CLIENTITEM ); i <= lastRelation (); ++i )
		{
			if ( relatedItem ( static_cast<RELATED_LIST_ITEMS>( i ) ) != nullptr )
				relatedItem ( static_cast<RELATED_LIST_ITEMS>( i ) )->m_dbrec = dbrec;
		}
	}
	else
		m_dbrec = dbrec;
}

void vmListItem::createDBRecord ()
{
	/* Pointers shared among several objects present a serious problem, specially when
	 * deleting them. Any derived class will have m_dbrec shared among the related items,
	 * but only RLI_CLIENTITEM will delete the pointer. This basic class will only delete it
	 * when it creates it, and will (must) not attempt to destroy a pointer created in a
	 * derived class
	 */
	setDBRec ( static_cast<DBRecord*>( new DBRecord ( static_cast<uint>( 0 ) ) ) );
}

bool vmListItem::loadData ()
{
	/* A generic list item cannot create a generic dbrecord. It could, if we wanted. But the
	 * purpose of a generic list item is to carry a specific database pointer across the application. Therefore,
	 * a DBREcord must be created either directly from from a derived class or genericly and then copy from a specifc record,
	 * when used in a generic list item.
	 */
	if ( dbRec () )
	{
		if ( action () == ACTION_READ )
			return ( m_dbrec->readRecord ( static_cast<uint>( id () ) ) );
		return true; // when adding or editing, do not read from the database, but use current user input
	}
	return false;
}

void vmListItem::setRelatedItem ( const uint rel_idx, vmListItem* const item )
{
	item_related[rel_idx] = item;
}

vmListItem* vmListItem::relatedItem ( const uint rel_idx ) const
{
	return item_related[rel_idx];
}

void vmListItem::changeAppearance ()
{
	if ( listWidget () )
		listWidget ()->setUpdatesEnabled ( false );
	setBackground ( QBrush ( COLORS[static_cast<uint>( action () )] ) );
	QFont fnt ( font () );
	fnt.setItalic ( action () > ACTION_READ );
	setFont ( fnt );
	
	int startItem ( static_cast<int>( relation () >= RLI_CLIENTITEM ? lastRelation () : relation () ) );
	
	for ( int i ( startItem ); i >= RLI_CLIENTPARENT; --i )
	{
		if ( relatedItem ( static_cast<RELATED_LIST_ITEMS>( i ) ) != nullptr )
		{
			relatedItem ( static_cast<RELATED_LIST_ITEMS>( i ) )->setIcon ( *Data::listIndicatorIcons[static_cast<uint>( action () )] );
		}
	}
	
	update ();
	if ( listWidget () )
	{
		listWidget ()->setUpdatesEnabled ( true );
		listWidget ()->repaint ();
		listWidget ()->parentWidget ()->repaint ();
	}
	qApp->sendPostedEvents();
}

void vmListItem::deleteRelatedItem ( const uint rel_idx )
{
	heap_del ( item_related[rel_idx] );
}

uint vmListItem::translatedInputFieldIntoBadInputField ( const uint field ) const
{
	return field;
}

void vmListItem::setFieldInputStatus ( const uint field, const bool ok, const vmWidget* widget )
{
	const uint bad_field ( translatedInputFieldIntoBadInputField ( field ) );
	if ( badInputs_ptr[bad_field] != ok )
		ok ? --n_badInputs : ++n_badInputs;
	badInputs_ptr[bad_field] = ok;
	if ( widget )
		const_cast<vmWidget*>( widget )->highlight ( ok ? vmDefault_Color : vmRed );
}

void vmListItem::saveCrashInfo ( crashRestore* crash )
{
	stringRecord state_info ( crash->fieldSeparatorForRecord () );
	state_info.fastAppendValue ( QString::number ( subType () ) ); // CF_SUBTYPE - client, job, pay or buy
	state_info.fastAppendValue ( QString::number ( static_cast<int>( action () ) ) ); //CF_ACTION
	state_info.fastAppendValue ( QString::number ( crashID () ) ); //CF_CRASHID
	state_info.appendStrRecord ( m_dbrec->toStringRecord ( crash->fieldSeparatorForRecord () ) ); //CF_DBRECORD
	setCrashID ( crash->commitState ( crashID (), state_info ) );
}

void vmListItem::setSearchArray ()
{
	if ( m_dbrec != nullptr && !mbSearchCreated )
	{
		searchFields = new triStateType[m_dbrec->fieldCount ()];
		mbSearchCreated = true;
	}
}

clientListItem::~clientListItem ()
{
	if ( relation () == RLI_CLIENTITEM )
	{
		heap_del ( m_dbrec );
		heap_del ( jobs );
		heap_del ( pays );
		heap_del ( buys );
	}
	vmListItem::m_dbrec = nullptr;
}

void clientListItem::update ()
{
	if ( relation () == RLI_CLIENTITEM ) // updates only come through via the client parent
	{
		if ( loadData () )
		{
			setLabel ( recStrValue ( CLIENT_REC, FLD_CLIENT_NAME ) );
			if ( listWidget () )
				listWidget ()->update ();
		}
	}
}

void clientListItem::createDBRecord ()
{
	setDBRec ( static_cast<DBRecord*>( new Client ( true ) ) );
	CLIENT_REC->setListItem ( this );
}

bool clientListItem::loadData ()
{
	if ( !m_dbrec )
		createDBRecord ();
	if ( action () == ACTION_READ )
		return ( CLIENT_REC->readRecord ( static_cast<uint>( id () ) ) );
	return true; // when adding or editing, do not read from the database, but use current user input
}

void clientListItem::relationActions ( vmListItem* subordinateItem )
{
	if ( relation () == RLI_CLIENTITEM )
	{
		if ( subordinateItem == nullptr )
		{
			jobs = new PointersList<jobListItem*> ( 100 );
			pays = new PointersList<payListItem*> ( 100 );
			buys = new PointersList<buyListItem*> ( 50 );
			// No need to explicitly call clear ( true ) in the dctor
			jobs->setAutoDeleteItem ( true );
			pays->setAutoDeleteItem ( true );
			buys->setAutoDeleteItem ( true );
		}
		else
		{
			static_cast<clientListItem*>( subordinateItem )->jobs = this->jobs;
			static_cast<clientListItem*>( subordinateItem )->pays = this->pays;
			static_cast<clientListItem*>( subordinateItem )->buys = this->buys;
		}
	}
}

jobListItem::~jobListItem ()
{
	if ( relation () == RLI_CLIENTITEM )
	{
		heap_del ( m_dbrec );
		heap_del ( buys );
		heap_del ( daysList );
		if ( mSearchSubFields != nullptr )
		{
			mSearchSubFields->clear ();
			delete mSearchSubFields;
		}
		vmListItem::m_dbrec = nullptr;
	}
}

uint jobListItem::translatedInputFieldIntoBadInputField ( const uint field ) const
{
	uint input_field ( 0 );
	switch ( field )
	{
		case FLD_JOB_TYPE:
			input_field = 0;
		break;
		case FLD_JOB_STARTDATE:
			input_field = 1;
		break;
	}
	return input_field;
}

void jobListItem::createDBRecord ()
{
	setDBRec ( static_cast<DBRecord*>( new Job ( true ) ) );
	JOB_REC->setListItem ( this );
}

bool jobListItem::loadData ()
{
	if ( !m_dbrec )
		createDBRecord ();
	if ( action () == ACTION_READ )
		return ( JOB_REC->readRecord ( static_cast<uint>( id () ) ) );
	return true; // when adding or editing, do not read from the database, but use current user input
}

void jobListItem::update ()
{
	if ( loadData () )
	{
		if ( relation () == RLI_CLIENTITEM ) // updates only come through via the client parent
		{
			setLabel ( recStrValue ( JOB_REC, FLD_JOB_TYPE ) + QLatin1String ( " - " ) + recStrValue ( JOB_REC, FLD_JOB_STARTDATE ) );
			if ( listWidget () )
				listWidget ()->update ();
		}
	}
}

void jobListItem::relationActions ( vmListItem* subordinateItem )
{
	if ( relation () == RLI_CLIENTITEM )
	{
		if ( subordinateItem == nullptr )
		{
			buys = new PointersList<buyListItem*> ( 50 );
			buys->setAutoDeleteItem ( true );
			daysList = new PointersList<vmListItem*> ( 10 );
			daysList->setAutoDeleteItem ( true );
		}
		else
		{
			static_cast<jobListItem*>( subordinateItem )->buys = this->buys;
			static_cast<jobListItem*>( subordinateItem )->daysList = this->daysList;
		}
	}
}

podList<uint>* jobListItem::searchSubFields () const
{
	if ( mSearchSubFields == nullptr )
		const_cast<jobListItem*>( this )->mSearchSubFields = new podList<uint> ( 0, 5 );
	return mSearchSubFields;
}

void jobListItem::setReportSearchFieldFound ( const uint report_field, const uint day )
{
	mSearchSubFields->operator []( day ) = report_field;
}

payListItem::~payListItem ()
{
	if ( relation () == RLI_CLIENTITEM )
	{
		for ( uint i ( RLI_JOBITEM ); i <= lastRelation (); ++i )
			deleteRelatedItem ( i );
		heap_del ( m_dbrec );
		vmListItem::m_dbrec = nullptr;
	}
	else
		relatedItem ( RLI_CLIENTITEM )->setRelatedItem ( relation (), nullptr );
}

void payListItem::update ()
{
	if ( loadData () )
	{
		if ( relation () == RLI_CLIENTITEM ) // updates only come through via the client parent
		{
			if ( action () == ACTION_ADD )
				setText ( APP_TR_FUNC ( "Automatically generated payment info - edit it after saving job" ), false, false, false );
			else
			{
				if ( !recStrValue ( PAY_REC, FLD_PAY_PRICE ).isEmpty () )
				{
					setLabel ( PAY_REC->price ( FLD_PAY_PRICE ).toPrice () + CHR_SPACE + CHR_L_PARENTHESIS +
						PAY_REC->price ( FLD_PAY_TOTALPAID ).toPrice () + CHR_R_PARENTHESIS );
				}
				else
					setLabel ( APP_TR_FUNC ( "No payment yet for job" ) );
			}
			if ( listWidget () )
				listWidget ()->update ();

			if ( relatedItem ( PAY_ITEM_OVERDUE_ALL ) != nullptr )
				static_cast<payListItem*>( relatedItem ( PAY_ITEM_OVERDUE_ALL ) )->updatePayExtraItems ( PAY_ITEM_OVERDUE_ALL );
			
			if ( relatedItem ( PAY_ITEM_OVERDUE_CLIENT ) != nullptr )
				static_cast<payListItem*>( relatedItem ( PAY_ITEM_OVERDUE_CLIENT ) )->updatePayExtraItems ( PAY_ITEM_OVERDUE_CLIENT );
			if ( relatedItem ( RLI_CALENDARITEM ) != nullptr )
				static_cast<payListItem*>( relatedItem ( RLI_CALENDARITEM ) )->updatePayCalendarItem ();
		}
		else if ( relation () == RLI_CALENDARITEM )
			updatePayCalendarItem ();
		else
			updatePayExtraItems ( relation () );
	}
}

void payListItem::updatePayCalendarItem ()
{	
	QString label ( recStrValue ( static_cast<clientListItem*>( relatedItem ( RLI_CLIENTPARENT ) )->clientRecord (), FLD_CLIENT_NAME ) +
			  CHR_L_PARENTHESIS + recStrValue ( payRecord (), FLD_PAY_TOTALPAID ) + CHR_R_PARENTHESIS + CHR_SPACE + CHR_HYPHEN + CHR_SPACE );
	
	if ( !data ( Qt::UserRole ).isNull () )
		label += QLatin1String ( "Pay day #: " ) + data ( Qt::UserRole ).toString () + CHR_SPACE;
	if ( !data ( Qt::UserRole + 1 ).isNull () )
		label += QLatin1String ( "Pay use #: " ) + data ( Qt::UserRole + 1 ).toString ();
	setLabel ( label );
	setToolTip ( label );
	if ( listWidget () )
		listWidget ()->update ();
}

void payListItem::updatePayExtraItems ( uint relation )
{
	if ( relation == PAY_ITEM_OVERDUE_ALL )
	{
		setLabel ( recStrValue ( static_cast<clientListItem*>(
			relatedItem ( RLI_CLIENTPARENT ) )->clientRecord (), FLD_CLIENT_NAME ) +
			CHR_SPACE + CHR_HYPHEN + CHR_SPACE + PAY_REC->price ( FLD_PAY_PRICE ).toPrice () + 
			CHR_SPACE + CHR_L_PARENTHESIS + PAY_REC->price ( FLD_PAY_OVERDUE_VALUE ).toPrice () + CHR_R_PARENTHESIS );
	}
	else
	{
		setLabel ( recStrValue ( static_cast<jobListItem*>( 
			relatedItem ( RLI_JOBPARENT ) )->jobRecord (), FLD_JOB_TYPE ) +
			CHR_SPACE + CHR_HYPHEN + CHR_SPACE + PAY_REC->price ( FLD_PAY_PRICE ).toPrice () + 
			CHR_SPACE + CHR_L_PARENTHESIS + PAY_REC->price ( FLD_PAY_OVERDUE_VALUE ).toPrice () + CHR_R_PARENTHESIS );
	}
	if ( listWidget () )
		listWidget ()->update ();
}

void payListItem::relationActions ( vmListItem* ) {}

void payListItem::createDBRecord ()
{
	setDBRec ( static_cast<DBRecord*>( new Payment ( true ) ) );
	PAY_REC->setListItem ( this );
}

bool payListItem::loadData ()
{
	if ( !m_dbrec )
		createDBRecord ();
	if ( action () == ACTION_READ )
		return ( PAY_REC->readRecord ( static_cast<uint>( id () ) ) );
	return true; // when adding or editing, do not read from the database, but use current user input
}

uint payListItem::translatedInputFieldIntoBadInputField ( const uint field ) const
{
	uint input_field ( 0 );
	switch ( field )
	{
		case FLD_PAY_PRICE:
			input_field = 0;
		break;
		case PHR_VALUE + INFO_TABLE_COLUMNS_OFFSET:
			input_field = 1;
		break;
		case PHR_DATE + INFO_TABLE_COLUMNS_OFFSET:
			input_field = 2;
		break;
		case PHR_USE_DATE + INFO_TABLE_COLUMNS_OFFSET:
			input_field = 3;
		break;
		case PHR_ACCOUNT + INFO_TABLE_COLUMNS_OFFSET:
			input_field = 4;
		break;
		case PHR_METHOD + INFO_TABLE_COLUMNS_OFFSET:
			input_field = 5;
		break;
	}
	return input_field;
}

buyListItem::~buyListItem ()
{
	if ( relation () == RLI_CLIENTITEM )
	{
		for ( uint i ( RLI_JOBITEM ); i <= lastRelation (); ++i )
			deleteRelatedItem ( i );
		
		heap_del ( m_dbrec );
		vmListItem::m_dbrec = nullptr;
	}
	else
		relatedItem ( RLI_CLIENTITEM )->setRelatedItem ( relation (), nullptr );
}

void buyListItem::update ()
{
	if ( loadData () )
	{
		const QString strBodyText ( recStrValue ( BUY_REC, FLD_BUY_DATE ) + QLatin1String ( " - " ) + 
								recStrValue ( BUY_REC, FLD_BUY_PRICE ) + QLatin1String ( " (" ) );
	
		if ( relation () == RLI_CLIENTITEM ) // updates only come through via the client parent
		{
			setLabel ( strBodyText + recStrValue ( BUY_REC, FLD_BUY_SUPPLIER ) + CHR_R_PARENTHESIS );
			if ( relatedItem ( RLI_JOBITEM ) != nullptr )
			{
				relatedItem ( RLI_JOBITEM )->setText ( text () , false, false, false );
				if ( relatedItem ( RLI_JOBITEM )->listWidget () )
					relatedItem ( RLI_JOBITEM )->listWidget ()->update ();
			}
			if ( relatedItem ( RLI_EXTRAITEMS ) != nullptr )
				static_cast<buyListItem*>( relatedItem ( RLI_EXTRAITEMS ) )->updateBuyExtraItem ( strBodyText );
			if ( relatedItem ( RLI_CALENDARITEM ) != nullptr )
				static_cast<buyListItem*>( relatedItem ( RLI_CALENDARITEM ) )->updateBuyCalendarItem ();
			
			if ( listWidget () )
				listWidget ()->update ();
		}
		else
		{
			if ( relation () == RLI_EXTRAITEMS )
				updateBuyExtraItem ( strBodyText );
			else if ( relation () == RLI_CALENDARITEM )
				updateBuyCalendarItem ();
		}
	}
}

void buyListItem::updateBuyExtraItem ( const QString& label )
{
	setLabel ( label +
			  recStrValue ( static_cast<clientListItem*>( relatedItem ( RLI_CLIENTPARENT ) )->clientRecord (), FLD_CLIENT_NAME ) + 
			  CHR_R_PARENTHESIS );
	if ( listWidget () )
		listWidget ()->update ();
}

void buyListItem::updateBuyCalendarItem ()
{
	static const QString purchaseDateStr ( TR_FUNC ( " (%1 at %2 - purchase date)" ) );
	static const QString payDateStr ( TR_FUNC ( " (%1 at %2 - payment #%3)" ) );
	
	QString label ( recStrValue ( static_cast<clientListItem*>( relatedItem ( RLI_CLIENTPARENT ) )->clientRecord (), FLD_CLIENT_NAME ) +
					   QLatin1String ( " - " ) + recStrValue ( buyRecord (), FLD_BUY_SUPPLIER ) );
	
	if ( data ( Qt::UserRole + 1 ).toBool () == true )
	{
		label += purchaseDateStr.arg ( recStrValue ( buyRecord (), FLD_BUY_PRICE ), recStrValue ( buyRecord (), FLD_BUY_DATE ) );
	}
	
	if ( data ( Qt::UserRole + 2 ).toBool () == true )
	{
		const uint paynumber ( data ( Qt::UserRole ).toUInt () );
		const stringRecord payRecord ( stringTable ( recStrValue ( buyRecord (), FLD_BUY_PAYINFO ) ).readRecord ( paynumber - 1 ) );		
		label += payDateStr.arg ( payRecord.fieldValue ( PHR_VALUE ), payRecord.fieldValue ( PHR_DATE ), QString::number ( paynumber ) );
	}
	setLabel ( label );
	setToolTip ( label );
	if ( listWidget () )
		listWidget ()->update ();
}

void buyListItem::relationActions ( vmListItem* ) {}

void buyListItem::createDBRecord ()
{
	setDBRec ( static_cast<DBRecord*>( new Buy ( true ) ) );
	BUY_REC->setListItem ( this );
}

bool buyListItem::loadData ()
{
	if ( !m_dbrec )
		createDBRecord ();
	if ( action () == ACTION_READ )
		return ( BUY_REC->readRecord ( static_cast<uint>( id () ) ) );
	return true; // when adding or editing, do not read from the database, but use current user input
}

uint buyListItem::translatedInputFieldIntoBadInputField ( const uint field ) const
{
	uint input_field ( 0 );
	switch ( field )
	{
		case FLD_BUY_PRICE:
			input_field = 0;
		break;
		case FLD_BUY_DATE:
			input_field = 1;
		break;
		case FLD_BUY_DELIVERDATE:
			input_field = 2;
		break;
		case FLD_BUY_SUPPLIER:
			input_field = 3;
		break;
	}
	return input_field;
}
