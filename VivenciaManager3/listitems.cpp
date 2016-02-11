#include "listitems.h"
#include "client.h"
#include "job.h"
#include "payment.h"
#include "purchases.h"
#include "global_enums.h"
#include "data.h"
#include "crashrestore.h"
#include "vmwidgets.h"
#include "heapmanager.h"
#include "stringrecord.h"

#include <QBrush>
#include <QApplication>

static const QString actionSuffix[4] = {
	emptyString, QStringLiteral (  " - DEL" ),
	QStringLiteral ( " - NEW" ), QStringLiteral ( " - EDIT" )
};

#define CLIENT_REC static_cast<Client*>( vmListItem::m_dbrec )
#define JOB_REC static_cast<Job*>( vmListItem::m_dbrec )
#define PAY_REC static_cast<Payment*>( vmListItem::m_dbrec )
#define BUY_REC static_cast<Buy*>( vmListItem::m_dbrec )

const Qt::GlobalColor COLORS[4] = { Qt::white, Qt::red, Qt::green, Qt::blue };

vmListItem::vmListItem ( const uint type_id, const uint nbadInputs, bool* const badinputs_ptr )
	: QListWidgetItem (), vmWidget ( WT_LISTITEM, type_id ),
	  item_related { nullptr, nullptr, nullptr, nullptr },
			   m_crashid ( -1 ), m_dbrec ( nullptr ), mRelation ( RLI_CLIENTITEM ), searchFields ( nullptr ),
			   m_action ( ACTION_NONE ), m_list ( nullptr ), badInputs_ptr ( badinputs_ptr ),
			   n_badInputs ( 0 ), mTotal_badInputs ( nbadInputs ), mbSearchCreated ( false )
{
	setAction ( ACTION_READ, true );
}

vmListItem::~vmListItem ()
{
	if ( !EXITING_PROGRAM )
		disconnectRelation ( RLI_CLIENTITEM, this );
	if ( mbSearchCreated )
		delete[] searchFields;
}

QString vmListItem::defaultStyleSheet () const
{
	QString colorstr;
	if ( !listWidget () )
		colorstr = QStringLiteral ( " ( 255, 255, 255 ) }" );
	else {
		vmListItem* item ( new vmListItem ( 1000 ) );
		listWidget ()->addItem ( item );
		colorstr = item->backgroundColor ().name ();
		listWidget ()->removeItemWidget ( item );
		delete item;
	}
	return ( colorstr );
}

void vmListItem::highlight ( const VMColors vm_color, const QString& )
{
	setBackgroundColor ( QColor ( vm_color == vmDefault_Color ?
		vmListItem::defaultStyleSheet () : colorsStr[Data::vmColorToRGB ( vm_color )] ) );
}

void vmListItem::setRelation ( const RELATED_LIST_ITEMS relation )
{
	mRelation = relation;
	item_related[static_cast<int> ( relation )] = this;
	relationActions ();
}

void vmListItem::disconnectRelation ( const uint start_relation, vmListItem* item )
{
	for ( uint i ( start_relation ); i <= RLI_EXTRAITEM; ++i ) {
		if ( item_related[i] == item )
			item_related[i] = nullptr;
		else {
			if ( item_related[i] != nullptr )
				item_related[i]->disconnectRelation ( i+1, item );
		}
	}
}

void vmListItem::syncSiblingWithThis ( vmListItem* sibling )
{
	if ( mRelation != sibling->mRelation ) {
		sibling->setDBRecID ( dbRecID () );
		sibling->setDBRec ( dbRec (), true );
		this->item_related[sibling->mRelation] = sibling;
		relationActions ( sibling );
		for ( uint i ( 0 ); i <= RLI_EXTRAITEM; ++i )
			sibling->item_related[i] = this->item_related[i];
		sibling->setAction ( action (), false, true );
	}
}

void vmListItem::addToList ( vmListWidget* const w_list )
{
	//if ( m_list != w_list ) {
		w_list->addItem ( this );
		m_list = w_list;
	//}
	//w_list->setCurrentItem ( this );
}

void vmListItem::setAction ( const RECORD_ACTION action, const bool bSetDBRec, const bool bSelfOnly )
{
	if ( action != m_action ) {
		m_action = action != ACTION_REVERT ? action : ACTION_READ;
		setBackground ( QBrush ( COLORS[static_cast<int>( action )] ) );
		QFont fnt ( font () );
		fnt.setItalic ( action > ACTION_READ );
		setFont ( fnt );

		// Since m_dbrec is a shared pointer among all related items, only the first -external- call
		// to setAction must change. All subsequent calls -internal- can skip these steps
		if ( m_dbrec && bSetDBRec )
			m_dbrec->setAction ( action );

		// update all related items, except self. Call setAction with self_only to true so that we don't enter infinite recurssion.
		if ( !bSelfOnly ) {
			for ( uint i ( RLI_CLIENTITEM ); i <= RLI_EXTRAITEM; ++i ) {
				if ( ( static_cast<uint> ( mRelation ) != i ) && item_related[i] != nullptr )
					item_related[i]->setAction ( m_action, false, true );
			}
			update ( false );
		}

		n_badInputs = action == ACTION_ADD ? mTotal_badInputs : 0;
		for ( uint i ( 0 ); i < mTotal_badInputs; ++i )
			badInputs_ptr[i] = action == ACTION_ADD ? false : true;
	}
}

void vmListItem::setDBRec ( DBRecord* dbrec, const bool self_only )
{
	// update all related items, except self. Call setAction with self_only to true so that we don't enter infinite recurssion.
	if ( !self_only ) {
		for ( uint i ( RLI_CLIENTITEM ); i <= RLI_EXTRAITEM; ++i ) {
			//if ( ( static_cast<uint> ( mRelation ) != i ) && item_related[i] != nullptr )
			if ( item_related[i] != nullptr )
				item_related[i]->m_dbrec = dbrec;
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
	setDBRec ( static_cast<DBRecord*> ( new DBRecord ( (uint) 0 ) ) );
}

bool vmListItem::loadData ()
{
	/* A generic list item cannot create a generic dbrecord. It could, if we wanted. But the
	 * purpose of a generic list item is to carry a specific database pointer across the application. Therefore,
	 * a DBREcord must be created either directly from from a derived class or genericly and then copy from a specifc record,
	 * when used in a generic list item.
	 */
	if ( dbRec () ) {
		if ( action () == ACTION_READ )
			return ( m_dbrec->readRecord ( id () ) );
		return true; // when adding or editing, do not read from the database, but use current user input
	}
	return false;
}

void vmListItem::update ( const bool bQtCall )
{
	/* bQtCall: Qt makes frequent calls to update. It is not necessary. By default, we ignore
	 * their calls. When needed, we will use the update method, but it shall be at our discretion.
	 * This is the main reason to name a method exactly like the parent's instead
	 */
	if ( !bQtCall ) {
		setData ( Qt::ToolTipRole, QString ( m_strBodyText + actionSuffix[action()] ) );
		QListWidgetItem::setText ( m_strBodyText + actionSuffix[action()] );
	}
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
		const_cast<vmWidget*> ( widget )->highlight ( ok ? vmDefault_Color : vmRed );
}

void vmListItem::saveCrashInfo ( crashRestore* crash )
{
	stringRecord state_info;
	state_info.fastAppendValue ( QString::number ( subType () ) ); // CF_SUBTYPE - client, job, pay or buy
	state_info.fastAppendValue ( QString::number ( item_related[static_cast<int>( RLI_CLIENTPARENT )]->dbRecID () ) ); //CF_CLIENTID
	state_info.fastAppendValue ( item_related[static_cast<int>( RLI_JOBPARENT )] != nullptr ?
		QString::number ( item_related[static_cast<int>( RLI_JOBPARENT )]->dbRecID () ) : QStringLiteral ( "-1" ) ); //CF_JOBID - a client does not have a JOBPARENT
	state_info.fastAppendValue ( QString::number ( dbRecID () ) ); //CF_ID
	state_info.fastAppendValue ( QString::number ( m_action ) ); //CF_ACTION
	state_info.appendStrRecord ( m_dbrec->toStringRecord () ); //CF_DBRECORD
	m_crashid = crash->commitState ( m_crashid, state_info.toString () );
}

void vmListItem::setSearchArray ()
{
	if ( m_dbrec != nullptr ) {
		searchFields = new triStateType[m_dbrec->fieldCount ()];
		mbSearchCreated = true;
	}
}

clientListItem::~clientListItem ()
{
	if ( mRelation == RLI_CLIENTITEM ) {
		heap_del ( CLIENT_REC );
		heap_del ( jobs );
		heap_del ( pays );
		heap_del ( buys );
	}
	vmListItem::m_dbrec = nullptr;
}

void clientListItem::update ( const bool bQtCall )
{
	if ( !bQtCall && m_dbrec ) {
		m_strBodyText = recStrValue ( CLIENT_REC, FLD_CLIENT_NAME );
		vmListItem::update ( false );
	}
}

void clientListItem::createDBRecord ()
{
	setDBRec ( static_cast<DBRecord*> ( new Client ( true ) ) );
}

bool clientListItem::loadData ()
{
	if ( !m_dbrec ) {
		createDBRecord ();
		CLIENT_REC->setListItem ( this );
	}
	if ( action () == ACTION_READ )
		return ( CLIENT_REC->readRecord ( id () ) );
	return true; // when adding or editing, do not read from the database, but use current user input
}

void clientListItem::relationActions ( vmListItem* subordinateItem )
{
	if ( mRelation == RLI_CLIENTITEM ) {
		if ( subordinateItem == nullptr ) {
			jobs = new PointersList<jobListItem*> ( 50 );
			pays = new PointersList<payListItem*> ( 50 );
			buys = new PointersList<buyListItem*> ( 20 );
			// No need to explicitly call clear ( true ) in the dctor
			jobs->setAutoDeleteItem ( true );
			pays->setAutoDeleteItem ( true );
			buys->setAutoDeleteItem ( true );
		}
		else {
			static_cast<clientListItem*>( subordinateItem )->jobs = this->jobs;
			static_cast<clientListItem*>( subordinateItem )->pays = this->pays;
			static_cast<clientListItem*>( subordinateItem )->buys = this->buys;
		}
	}
}

jobListItem::~jobListItem ()
{
	if ( mRelation == RLI_CLIENTITEM ) {
		heap_del ( JOB_REC );
		buys->clear ();
		heap_del ( buys );
		daysList->clear ();
		heap_del ( daysList );
		if ( mSearchSubFields != nullptr ) {
			mSearchSubFields->clear ();
			delete mSearchSubFields;
		}
	}
	vmListItem::m_dbrec = nullptr;
}

uint jobListItem::translatedInputFieldIntoBadInputField ( const uint field ) const
{
	uint input_field ( 0 );
	switch ( field ) {
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
	setDBRec ( static_cast<DBRecord*> ( new Job ( true ) ) );
}

bool jobListItem::loadData ()
{
	if ( !m_dbrec ) {
		createDBRecord ();
		JOB_REC->setListItem ( this );
	}
	if ( action () == ACTION_READ )
		return ( JOB_REC->readRecord ( id () ) );
	return true; // when adding or editing, do not read from the database, but use current user input
}

void jobListItem::update ( const bool bQtCall )
{
	if ( bQtCall ) return;

	if ( m_dbrec ) {
		if ( mRelation == RLI_CLIENTITEM ) {
			m_strBodyText = recStrValue ( JOB_REC, FLD_JOB_TYPE );
			if ( !m_strBodyText.isEmpty () )
				m_strBodyText += QLatin1String ( " - " ) + recStrValue ( JOB_REC, FLD_JOB_STARTDATE );
		}
	}
	else
		m_strBodyText = CHR_QUESTION_MARK;
	vmListItem::update ( false );
}

void jobListItem::relationActions ( vmListItem* subordinateItem )
{
	if ( mRelation == RLI_CLIENTITEM ) {
		if ( subordinateItem == nullptr ) {
			buys = new PointersList<buyListItem*> ( 50 );
			daysList = new PointersList<vmListItem*> ( 5 );
		}
		else {
			static_cast<jobListItem*>( subordinateItem )->buys = this->buys;
			static_cast<jobListItem*>( subordinateItem )->daysList = this->daysList;
		}
	}
}

podList<uint>* jobListItem::searchSubFields () const
{
	if ( mSearchSubFields == nullptr )
		const_cast<jobListItem*>( this )->mSearchSubFields = new podList<uint> ( 5 );
	return mSearchSubFields;
}

void jobListItem::setReportSearchFieldFound ( const uint report_field, const uint day )
{
	mSearchSubFields->operator []( day ) = report_field;
}

payListItem::~payListItem ()
{
	if ( mRelation == RLI_CLIENTITEM ) {
		heap_del ( item_related[RLI_JOBITEM] );
		heap_del ( item_related[RLI_EXTRAITEM] );
		heap_del ( PAY_REC );
		vmListItem::m_dbrec = nullptr;
	}
	else
		item_related[RLI_CLIENTITEM]->item_related[mRelation] = nullptr;
}

void payListItem::update ( const bool bQtCall )
{
	if ( bQtCall ) return;

	if ( m_dbrec ) {
		if ( item_related[RLI_CLIENTITEM] != nullptr ) {
			if ( action () == ACTION_ADD )
				item_related[RLI_CLIENTITEM]->m_strBodyText = APP_TR_FUNC ( "Automatically generated payment info - edit it after saving job" );
			else {
				if ( !recStrValue ( PAY_REC, FLD_PAY_PRICE ).isEmpty () ) {
					item_related[RLI_CLIENTITEM]->m_strBodyText = PAY_REC->price ( FLD_PAY_PRICE ).toPrice () + CHR_SPACE + CHR_L_PARENTHESIS +
						PAY_REC->price ( FLD_PAY_TOTALPAID ).toPrice () + CHR_R_PARENTHESIS;
				}
				else
					item_related[RLI_CLIENTITEM]->m_strBodyText = APP_TR_FUNC ( "No payment yet for job" );
			}
			item_related[RLI_CLIENTITEM]->vmListItem::update ( false );

			if ( item_related[RLI_EXTRAITEM] != nullptr ) {
				item_related[RLI_EXTRAITEM]->m_strBodyText = recStrValue ( static_cast<clientListItem*> (
					item_related[RLI_CLIENTPARENT] )->clientRecord (), FLD_CLIENT_NAME ) +
					CHR_SPACE + CHR_HYPHEN + CHR_SPACE + PAY_REC->price ( FLD_PAY_PRICE ).toPrice () + 
					CHR_SPACE + CHR_L_PARENTHESIS + PAY_REC->price ( FLD_PAY_OVERDUE_VALUE ).toPrice () + CHR_R_PARENTHESIS;
				item_related[RLI_EXTRAITEM]->vmListItem::update ( false );
			}
		}
	}
	else {
		m_strBodyText = CHR_QUESTION_MARK;
		vmListItem::update ( false );
	}
}

void payListItem::relationActions ( vmListItem* ) {}

void payListItem::createDBRecord ()
{
	setDBRec ( static_cast<DBRecord*> ( new Payment ( true ) ) );
}

bool payListItem::loadData ()
{
	if ( !m_dbrec ) {
		createDBRecord ();
		PAY_REC->setListItem ( this );
	}
	if ( action () == ACTION_READ )
		return ( PAY_REC->readRecord ( id () ) );
	return true; // when adding or editing, do not read from the database, but use current user input
}

uint payListItem::translatedInputFieldIntoBadInputField ( const uint field ) const
{
	uint input_field ( 0 );
	switch ( field ) {
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
	/* CLIENTITEM will delete all the dependants that have not been deleted
	 * When cleaning up an item that has dependants, begin by removing those depedants
	 * and for last delete the parent. This will ensure that code bound to those dependants
	 * gets called without crash for trying to refer to a pointer that is no more. If you are
	 * certain no code depends on the dependants, cleaning the master is the fastest way to go
	 */
	if ( mRelation == RLI_CLIENTITEM ) {
		heap_del ( item_related[RLI_EXTRAITEM] );
		heap_del ( item_related[RLI_DATEITEM] );
		heap_del ( item_related[RLI_JOBITEM] );
		heap_del ( BUY_REC );
		vmListItem::m_dbrec = nullptr;
	}
	else
		item_related[RLI_CLIENTITEM]->item_related[mRelation] = nullptr;
}

void buyListItem::update ( const bool bQtCall )
{
	if ( bQtCall ) return;

	if ( m_dbrec ) {
		const QString strBodyText ( recStrValue ( BUY_REC, FLD_BUY_DATE ) + QLatin1String ( " - " ) +
						recStrValue ( BUY_REC, FLD_BUY_PRICE ) + QLatin1String ( " (" ) );

		if ( mRelation != RLI_EXTRAITEM ) { //EXTRAITEM does not produce changes on other items, so we do not need to waste time updating the others when updating it
			item_related[RLI_CLIENTITEM]->m_strBodyText = strBodyText + recStrValue ( BUY_REC, FLD_BUY_SUPPLIER ) + CHR_R_PARENTHESIS;
			item_related[RLI_CLIENTITEM]->vmListItem::update ( false );

			if ( item_related[RLI_JOBITEM] != nullptr ) {
				item_related[RLI_JOBITEM]->m_strBodyText = item_related[RLI_CLIENTITEM]->m_strBodyText;
				item_related[RLI_JOBITEM]->vmListItem::update ( false );
			}
		}
		if ( item_related[RLI_EXTRAITEM] != nullptr ) {
			item_related[RLI_EXTRAITEM]->m_strBodyText = strBodyText + Client::clientName ( recStrValue ( BUY_REC, FLD_BUY_CLIENTID ) ) + CHR_R_PARENTHESIS;
			item_related[RLI_EXTRAITEM]->vmListItem::update ( false );
		}
	}
	else {
		m_strBodyText = CHR_QUESTION_MARK;
		vmListItem::update ( false );
	}
}

void buyListItem::relationActions ( vmListItem* ) {}

void buyListItem::createDBRecord ()
{
	setDBRec ( static_cast<DBRecord*> ( new Buy ( true ) ) );
}

bool buyListItem::loadData ()
{
	if ( !m_dbrec ) {
		createDBRecord ();
		BUY_REC->setListItem ( this );
	}
	if ( action () == ACTION_READ )
		return ( BUY_REC->readRecord ( id () ) );
	return true; // when adding or editing, do not read from the database, but use current user input
}

uint buyListItem::translatedInputFieldIntoBadInputField ( const uint field ) const
{
	uint input_field ( 0 );
	switch ( field ) {
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
