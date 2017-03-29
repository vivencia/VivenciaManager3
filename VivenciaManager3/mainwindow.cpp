#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "global.h"
#include "completers.h"
#include "simplecalculator.h"
#include "backupdialog.h"
#include "data.h"
#include "configops.h"
#include "estimates.h"
#include "documenteditor.h"
#include "quickproject.h"
#include "quickprojectui.h"
#include "companypurchasesui.h"
#include "inventoryui.h"
#include "dbsuppliesui.h"
#include "dbsupplies.h"
#include "vmlistitem.h"
#include "vmwidgets.h"
#include "suppliersdlg.h"
#include "separatewindow.h"
#include "todolist.h"
#include "db_image.h"
#include "vmlist.h"
#include "vmnotify.h"
#include "crashrestore.h"
#include "vivenciadb.h"
#include "vmtaskpanel.h"
#include "vmactiongroup.h"
#include "heapmanager.h"
#include "dbcalendar.h"
#include "searchui.h"
#include "emailconfigdialog.h"
#include "reportgenerator.h"
#include "supplierrecord.h"
#include "machinesdlg.h"
#include "cleanup.h"
#include "configdialog.h"
#include "fast_library_functions.h"

#include <QEvent>
#include <QKeyEvent>
#include <QComboBox>
#include <QMenu>
#include <QTimer>
#include <QSystemTrayIcon>
#include <QTabWidget>
#include <QToolButton>
#include <QDir>
#include <QDesktopWidget>
#include <QScrollBar>

bool EXITING_PROGRAM ( false );
MainWindow* globalMainWindow ( nullptr );

QLatin1String MainWindow::lstJobReportItemPrefix ( "o dia - " );

const char* const PROPERTY_TOTAL_PAYS ( "ptp" );

static const QString chkPayOverdueToolTip_overdue[3] = {
	APP_TR_FUNC ( "The total price paid for the job is met, nothing needs to be done." ),
	APP_TR_FUNC ( "The total paid is less then the total price for the job.\n"
	"Check the box to ignore that discrepancy and mark the payment concluded as is." ),
	APP_TR_FUNC ( "Ignoring whether payment is overdue or not." )
};

static const QString jobPicturesSubDir ( QStringLiteral ( "Pictures/" ) );

MainWindow::MainWindow ()
	: QMainWindow ( nullptr ),
	  ui ( new Ui::MainWindow ),  menuJobDoc ( nullptr ), menuJobXls ( nullptr ), menuJobPdf ( nullptr ), 
	  subMenuJobPdfView ( nullptr ), subMenuJobPdfEMail ( nullptr ), todoListWidget ( nullptr ), timerUpdate ( nullptr ),
	  sepWin_JobPictures ( nullptr ), sepWin_JobReport ( nullptr ), jobsPicturesMenuAction ( nullptr ),
	  mainTaskPanel ( nullptr ), selJob_callback ( nullptr ), mb_jobPosActions ( false ),
	  mClientCurItem ( nullptr ), mJobCurItem ( nullptr ), mPayCurItem ( nullptr ), mBuyCurItem ( nullptr )
{
	ui->setupUi ( this );
	setWindowIcon ( ICON ( "vm-logo-22x22" ) );
	setWindowTitle ( PROGRAM_NAME + QLatin1String ( " - " ) + PROGRAM_VERSION );
	createTrayIcon (); // create the icon for VM_NOTIFY () use
}

MainWindow::~MainWindow ()
{
	EXITING_PROGRAM = true;
	BackupDialog::doDayBackup ();
	saveEditItems ();

	timerUpdate->stop ();
	
	ui->clientsList->setIgnoreChanges ( true );
	ui->jobsList->setIgnoreChanges ( true );
	ui->paysList->setIgnoreChanges ( true );
	ui->buysList->setIgnoreChanges ( true );
	ui->buysJobList->setIgnoreChanges ( true );
	ui->lstJobDayReport->setIgnoreChanges ( true );
	ui->lstBuySuppliers->setIgnoreChanges ( true );
	ui->paysOverdueList->setIgnoreChanges ( true );
	ui->paysOverdueClientList->setIgnoreChanges ( true );
	ui->lstCalJobsDay->setIgnoreChanges ( true );
	ui->lstCalJobsWeek->setIgnoreChanges ( true );
	ui->lstCalJobsMonth->setIgnoreChanges ( true );
	ui->lstCalPaysDay->setIgnoreChanges ( true );
	ui->lstCalPaysWeek->setIgnoreChanges ( true );
	ui->lstCalPaysMonth->setIgnoreChanges ( true );
	ui->lstCalBuysDay->setIgnoreChanges ( true );
	ui->lstCalBuysWeek->setIgnoreChanges ( true );
	ui->lstCalBuysMonth->setIgnoreChanges ( true );

	for ( int i ( ui->clientsList->count () - 1 ); i >= 0; --i )
		delete ui->clientsList->item ( i );
	
	heap_del ( sepWin_JobPictures );
	heap_del ( sepWin_JobReport );
	heap_del ( ui );
	heap_del ( mCal );

	cleanUpApp ();
}

void MainWindow::continueStartUp ()
{
	setStatusBar ( nullptr ); // we do not want a statusbar
	createTrayIcon ( false );
	setupCustomControls ();
	restoreLastSession ();
	updateProgramDate ();

	int coords[4] = { 0 };
	CONFIG ()->geometryFromConfigFile ( coords );
	setGeometry ( coords[0], coords[1], coords[2], coords[3] );
	show ();
	
	// connect for last to avoid unnecessary signal emission when we are removing tabs
	connect ( ui->tabMain, &QTabWidget::currentChanged, this, &MainWindow::tabMain_currentTabChanged );
	connect ( qApp, &QCoreApplication::aboutToQuit, this, [&] () { return exitRequested (); } );
	this->tabPaysLists_currentChanged ( 0 ); // update pay totals line edit text
}

void MainWindow::exitRequested ( const bool user_requested )
{
	static bool already_called ( false );

	// the user clicked a button to exit the program. This function gets called and do the cleanup
	// job. Now, it asks Qt to exit the program and will be called again because it is hooked up to the
	// signal aboutToQuit (); only, we do not need to do anything this time, so we return.
	if ( already_called )
		return;

	EXITING_PROGRAM = true;

	int coords[4] = { 0 };
	coords[0] = pos ().x ();
	coords[1] = pos ().y ();
	coords[2] = width ();
	coords[3] = height ();
	CONFIG ()->saveGeometry ( coords );

	// It seems pointless to start EDITOR () when we are leaving the program. So we check before calling closeAllTabs.
	// EDITOR will check for EXITING_PROGRAM as well to cope with this situation
	if ( EDITOR () != nullptr )
		EDITOR ()->closeAllTabs ();

	if ( user_requested )
	{
		already_called = true;
		delete globalMainWindow;
		qApp->quit ();
	}
}
//--------------------------------------------CLIENT------------------------------------------------------------
//TODO
void MainWindow::saveClientWidget ( vmWidget* widget, const int id )
{
	widget->setID ( id );
	clientWidgetList[id] = widget;
}

void MainWindow::showClientSearchResult ( vmListItem* item, const bool bshow )
{
	if ( bshow )
		displayClient ( static_cast<clientListItem*> ( const_cast<vmListItem*> ( item ) ), true );

	for ( uint i ( 0 ); i < CLIENT_FIELD_COUNT; ++i )
	{
		if ( item->searchFieldStatus ( i ) == SS_SEARCH_FOUND )
			clientWidgetList.at ( i )->highlight ( bshow ? vmBlue : vmDefault_Color, SEARCH_UI ()->searchTerm () );
	}
}

void MainWindow::setupClientPanel ()
{
	ui->clientsList->setParentLayout ( ui->gLayoutClientListPanel );
	ui->clientsList->setUtilitiesPlaceLayout ( ui->layoutClientsList );
	ui->clientsList->setSortingEnabled ( true );
	ui->clientsList->sortItems ( 0 );
	ui->clientsList->setCallbackForCurrentItemChanged ( [&] ( vmListItem* item ) { 
		return clientsListWidget_currentItemChanged ( item ); } );
	ui->clientsList->toggleUtilitiesPanel ();

	saveClientWidget ( ui->txtClientID, FLD_CLIENT_ID );
	saveClientWidget ( ui->txtClientName, FLD_CLIENT_NAME );
	ui->txtClientName->setCompleter ( APP_COMPLETERS ()->getCompleter ( vmCompleters::CLIENT_NAME ) );
	ui->txtClientName->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
		return txtClientName_textAltered ( sender->text () ); } );
	ui->txtClientName->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
		return clientKeyPressedSelector ( ke ); } );

	saveClientWidget ( ui->txtClientCity, FLD_CLIENT_CITY );
	ui->txtClientCity->setCompleter ( APP_COMPLETERS ()->getCompleter ( vmCompleters::ADDRESS ) );
	ui->txtClientCity->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
		return txtClient_textAltered ( sender ); } );
	ui->txtClientCity->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
		return clientKeyPressedSelector ( ke ); } );

	saveClientWidget ( ui->txtClientDistrict, FLD_CLIENT_DISTRICT );
	ui->txtClientDistrict->setCompleter ( APP_COMPLETERS ()->getCompleter ( vmCompleters::ADDRESS ) );
	ui->txtClientDistrict->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
		return txtClient_textAltered ( sender ); } );
	ui->txtClientDistrict->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
		return clientKeyPressedSelector ( ke ); } );

	saveClientWidget ( ui->txtClientStreetAddress, FLD_CLIENT_STREET );
	ui->txtClientStreetAddress->setCompleter ( APP_COMPLETERS ()->getCompleter ( vmCompleters::ADDRESS ) );
	ui->txtClientStreetAddress->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
		return txtClient_textAltered ( sender ); } );
	ui->txtClientStreetAddress->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
		return clientKeyPressedSelector ( ke ); } );

	saveClientWidget ( ui->txtClientNumberAddress, FLD_CLIENT_NUMBER );
	ui->txtClientNumberAddress->setTextType ( vmWidget::TT_INTEGER );
	ui->txtClientNumberAddress->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
		return txtClient_textAltered ( sender ); } );
	ui->txtClientNumberAddress->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
		return clientKeyPressedSelector ( ke ); } );

	saveClientWidget ( ui->txtClientZipCode, FLD_CLIENT_ZIP );
	ui->txtClientZipCode->setTextType ( vmWidget::TT_NUMBER_PLUS_SYMBOL );
	ui->txtClientZipCode->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
		return txtClientZipCode_textAltered ( sender->text () ); } );
	ui->txtClientZipCode->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
		return clientKeyPressedSelector ( ke ); } );

	saveClientWidget ( ui->contactsClientPhones, FLD_CLIENT_PHONES );
	ui->contactsClientPhones->setContactType ( contactsManagerWidget::CMW_PHONES );
	ui->contactsClientPhones->initInterface ();
	ui->contactsClientPhones->setCallbackForInsertion ( [&] ( const QString& phone, const vmWidget* const sender ) {
		return contactsClientAdd ( phone, sender ); } );
	ui->contactsClientPhones->setCallbackForRemoval ( [&] ( const int idx, const vmWidget* const sender ) {
		return contactsClientDel ( idx, sender ); } );
	ui->contactsClientEmails->setID ( FLD_CLIENT_EMAIL );
	ui->contactsClientEmails->setContactType ( contactsManagerWidget::CMW_EMAIL );
	ui->contactsClientEmails->initInterface ();
	ui->contactsClientEmails->setCallbackForInsertion ( [&] ( const QString& addrs, const vmWidget* const sender ) {
		return contactsClientAdd ( addrs, sender ); } );
	ui->contactsClientEmails->setCallbackForRemoval ( [&] ( const int idx, const vmWidget* const sender ) {
		return contactsClientDel ( idx, sender ); } );

	saveClientWidget ( ui->dteClientDateFrom, FLD_CLIENT_STARTDATE );
	ui->dteClientDateFrom->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
		return dteClient_dateAltered ( sender ); } );
	ui->dteClientDateFrom->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
		return clientKeyPressedSelector ( ke ); } );
	ui->dteClientDateTo->setID ( FLD_CLIENT_ENDDATE );
	ui->dteClientDateTo->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
		return dteClient_dateAltered ( sender ); } );
	ui->dteClientDateTo->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
		return clientKeyPressedSelector ( ke ); } );

	saveClientWidget ( ui->chkClientActive, FLD_CLIENT_STATUS );
	ui->chkClientActive->setLabel ( TR_FUNC ( "Currently active" ) );
	ui->chkClientActive->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
		return txtClient_textAltered ( sender ); } );

	//const bool b_enabled ( DATA ()->writes[TABLE_CLIENT_ORDER] && mClientCurItem != nullptr );
	//const bool b_enabled ( mClientCurItem != nullptr );

	connect ( ui->btnClientAdd, &QToolButton::clicked, this, [&] () {
		return (mClientCurItem = addClient ()); } );
	connect ( ui->btnClientEdit, &QToolButton::clicked, this, [&] () {
		return static_cast<void>(editClient ( mClientCurItem )); } );
	connect ( ui->btnClientDel, &QToolButton::clicked, this, [&] () {
		return static_cast<void>(delClient ( mClientCurItem )); } );
	connect ( ui->btnClientSave, &QToolButton::clicked, this, [&] () {
		return static_cast<void>(saveClient ( mClientCurItem )); } );
	connect ( ui->btnClientCancel, &QToolButton::clicked, this, [&] () {
		return static_cast<void>( cancelClient ( mClientCurItem )); } );
}

void MainWindow::clientKeyPressedSelector ( const QKeyEvent* ke )
{
	if ( ke->key () == Qt::Key_Escape )
		static_cast<void>(cancelClient ( mClientCurItem ));
	else {
		ui->clientsList->setFocus (); // trigger any pending editing_Finished or textAltered event
		static_cast<void>(saveClient ( mClientCurItem ));
	}
}

void MainWindow::clientsListWidget_currentItemChanged ( vmListItem* item )
{
	insertNavItem ( item );
	(void) displayClient ( static_cast<clientListItem*> ( item ) );
}

void MainWindow::controlClientForms ( const clientListItem* const client_item )
{
	const triStateType editing_action ( client_item ? static_cast<TRI_STATE> ( client_item->action () >= ACTION_ADD ) : TRI_UNDEF );

	ui->txtClientName->setEditable ( editing_action.isOn () );
	ui->txtClientCity->setEditable ( editing_action.isOn () );
	ui->txtClientDistrict->setEditable ( editing_action.isOn () );
	ui->txtClientStreetAddress->setEditable ( editing_action.isOn () );
	ui->txtClientNumberAddress->setEditable ( editing_action.isOn () );
	ui->txtClientZipCode->setEditable ( editing_action.isOn () );
	ui->dteClientDateFrom->setEditable ( editing_action.isOn () );
	ui->dteClientDateTo->setEditable ( editing_action.isOn () );
	ui->chkClientActive->setEditable ( editing_action.isOn () );

	ui->btnClientEdit->setEnabled ( editing_action.isOff () );
	ui->btnClientDel->setEnabled ( editing_action.isOff () );
	ui->btnClientSave->setEnabled ( editing_action.isOn () ? client_item->isGoodToSave () : false );
	ui->btnClientCancel->setEnabled ( editing_action.isOn () );

	ui->contactsClientPhones->setEditable ( editing_action.isOn () );
	ui->contactsClientEmails->setEditable ( editing_action.isOn () );

	ui->txtClientName->highlight ( editing_action.isOn () ? ( client_item->isGoodToSave () ?
			vmDefault_Color : vmRed ) : ( ui->chkClientActive->isChecked () ? vmGreen : vmDefault_Color ) );
}

bool MainWindow::saveClient ( clientListItem* client_item )
{
	if ( client_item != nullptr )
	{
		if ( client_item->clientRecord ()->saveRecord () )
		{
			client_item->setAction ( client_item->clientRecord ()->action () );
			VM_NOTIFY ()->notifyMessage ( TR_FUNC ( "Record saved!" ), TR_FUNC ( "Client data saved!" ) );
			controlClientForms ( client_item );
			controlJobForms ( nullptr );
			saveView ();
			removeEditItem ( static_cast<vmListItem*>(client_item) );
			return true;
		}
	}
	return false;
}

clientListItem* MainWindow::addClient ()
{
	clientListItem* client_item ( new clientListItem );
	client_item->setRelation ( RLI_CLIENTITEM );
	client_item->setRelatedItem ( RLI_CLIENTPARENT, client_item );
	static_cast<void>(client_item->loadData ());
	client_item->setAction ( ACTION_ADD, true );
	client_item->addToList ( ui->clientsList );
	insertEditItem ( static_cast<vmListItem*>(client_item) );
	client_item->setDBRecID ( static_cast<uint>(recIntValue ( client_item->clientRecord (), FLD_CLIENT_ID )) );
	ui->txtClientName->setFocus ();
	return client_item;
}

bool MainWindow::editClient ( clientListItem* client_item, const bool b_dogui )
{
	if ( client_item )
	{
		if ( client_item->action () == ACTION_READ )
		{
			client_item->setAction ( ACTION_EDIT, true );
			insertEditItem ( static_cast<vmListItem*>(client_item) );
			if ( b_dogui )
			{
				VM_NOTIFY ()->notifyMessage ( TR_FUNC ( "Editing client record" ),
						recStrValue ( client_item->clientRecord (), FLD_CLIENT_NAME ) );
				controlClientForms ( client_item );
				ui->txtClientName->setFocus ();
			}
			return true;
		}
	}
	return false;
}

bool MainWindow::delClient ( clientListItem* client_item )
{
	if ( client_item )
	{
		const QString text ( TR_FUNC ( "Are you sure you want to remove %1's record?" ) );
		if ( VM_NOTIFY ()->questionBox ( TR_FUNC ( "Question" ), text.arg ( recStrValue ( client_item->clientRecord (), FLD_CLIENT_NAME ) ) ) )
		{
			client_item->setAction ( ACTION_DEL, true );
			return cancelClient ( client_item );
		}
	}
	return false;
}

bool MainWindow::cancelClient ( clientListItem* client_item )
{
	if ( client_item )
	{
		switch ( client_item->action () )
		{
			case ACTION_ADD:
			case ACTION_DEL:
				removeListItem ( client_item );
			break;
			case ACTION_EDIT:
				removeEditItem ( static_cast<vmListItem*>(client_item) );
				client_item->setAction ( ACTION_REVERT, true );
				displayClient ( client_item );
			break;
			case ACTION_NONE:
			case ACTION_READ:
			case ACTION_REVERT:
				return false;
		}
		return true;
	}
	return false;
}

bool MainWindow::displayClient ( clientListItem* client_item, const bool b_select, jobListItem* job_item, buyListItem* buy_item )
{
	// This function is called by user interaction and programatically. When called programatically, item may be nullptr
	if ( client_item )
	{
		ui->tabMain->setCurrentIndex ( 0 );
		if ( client_item->loadData () )
		{
			controlClientForms ( client_item );
			loadClientInfo ( client_item->clientRecord () );
			if ( client_item != mClientCurItem )
			{
				if ( b_select )
					ui->clientsList->setCurrentItem ( client_item, false );
				mClientCurItem = client_item;
				fillAllLists ( client_item );
				if ( !job_item )
					job_item = client_item->jobs->last ();
				displayJob ( job_item, true, buy_item );
				if ( ui->tabPaysLists->currentIndex () == 1 )
					loadClientOverduesList ();
			}
		}
	}
	saveView ();
	return ( client_item != nullptr );
}

void MainWindow::loadClientInfo ( const Client* const client )
{
	if ( client != nullptr )
	{
		ui->txtClientID->setText ( recStrValue ( client, FLD_CLIENT_ID ) );
		ui->txtClientName->setText ( recStrValue ( client, FLD_CLIENT_NAME ) );
		ui->txtClientCity->setText ( client->action () != ACTION_ADD ? recStrValue ( client, FLD_CLIENT_CITY ) : QStringLiteral ( "São Pedro" ) );
		ui->txtClientDistrict->setText ( recStrValue ( client, FLD_CLIENT_DISTRICT ) );
		ui->txtClientNumberAddress->setText ( recStrValue ( client, FLD_CLIENT_NUMBER ) );
		ui->txtClientStreetAddress->setText ( recStrValue ( client, FLD_CLIENT_STREET ) );
		ui->txtClientZipCode->setText ( client->action () != ACTION_ADD ? recStrValue ( client, FLD_CLIENT_ZIP ) : QStringLiteral ( "13520-000" ) );
		ui->dteClientDateFrom->setDate ( client->date ( FLD_CLIENT_STARTDATE ).toQDate () );
		ui->dteClientDateTo->setDate ( client->date ( FLD_CLIENT_ENDDATE ).toQDate () );
		ui->chkClientActive->setChecked ( client->opt ( FLD_CLIENT_STATUS ) );
		ui->contactsClientPhones->decodePhones ( client->recordStr ( FLD_CLIENT_PHONES ) );
		ui->contactsClientEmails->decodeEmails ( client->recordStr ( FLD_CLIENT_EMAIL ) );
	}
}

clientListItem* MainWindow::getClientItem ( const uint id ) const
{
	if ( id >= 1 )
	{
		const int n ( globalMainWindow->ui->clientsList->count () );
		clientListItem* client_item ( nullptr );
		for ( int i ( 0 ); i < n; ++i )
		{
			client_item = static_cast<clientListItem*> ( globalMainWindow->ui->clientsList->item ( i ) );
			if ( client_item->dbRecID () == id )
				return client_item;
		}
	}
	return nullptr;
}

void MainWindow::fillAllLists ( const clientListItem* client_item )
{
	// clears the list but does not delete the items
	ui->jobsList->clear (); 
	ui->paysList->clear ();
	ui->buysList->clear ();

	uint i ( 0 );
	if ( client_item )
	{
		jobListItem* job_item ( nullptr );
		for ( i = 0; i < client_item->jobs->count (); ++i )
		{
			job_item = client_item->jobs->at ( i );
			(void) job_item->loadData ();
			job_item->addToList ( ui->jobsList );
		}

		payListItem* pay_item ( nullptr );
		vmNumber totalPays;
		for ( i = 0; i < client_item->pays->count (); ++i )
		{
			pay_item = client_item->pays->at ( i );
			(void) pay_item->loadData ();
			pay_item->addToList ( ui->paysList );
			totalPays += pay_item->payRecord ()->price ( FLD_PAY_PRICE );
		}
		ui->paysList->setProperty ( PROPERTY_TOTAL_PAYS, totalPays.toPrice () );
		ui->txtClientPayTotals->setText ( totalPays.toPrice () );

		buyListItem* buy_item ( nullptr );
		for ( i = 0; i < client_item->buys->count (); ++i )
		{
			buy_item = client_item->buys->at ( i );
			(void) buy_item->loadData ();
			buy_item->addToList ( ui->buysList );
		}
	}

	ui->jobsList->setIgnoreChanges ( false );
	ui->paysList->setIgnoreChanges ( false );
	ui->buysList->setIgnoreChanges ( false );
}
//--------------------------------------------CLIENT------------------------------------------------------------

//-------------------------------------EDITING-FINISHED-CLIENT--------------------------------------------------
void MainWindow::txtClientName_textAltered ( const QString& text )
{
	const bool input_ok ( text.isEmpty () ? false : ( Client::clientID ( text ) == 0 ) );
	mClientCurItem->setFieldInputStatus ( FLD_CLIENT_NAME, input_ok, ui->txtClientName );

	if ( input_ok )
	{
		setRecValue ( mClientCurItem->clientRecord (), FLD_CLIENT_NAME, text );
		mClientCurItem->update ();
	}
	else
	{
		if ( !text.isEmpty () )
			VM_NOTIFY ()->notifyMessage ( TR_FUNC ( "Warning" ), TR_FUNC ( "Client name already in use" ) );
	}
	postFieldAlterationActions ( mClientCurItem );
}

void MainWindow::txtClient_textAltered ( const vmWidget* const sender )
{
	setRecValue ( mClientCurItem->clientRecord (), static_cast<uint>(sender->id ()), sender->text () );
	postFieldAlterationActions ( mClientCurItem );
}

void MainWindow::txtClientZipCode_textAltered ( const QString& text )
{
	QString zipcode ( text );
	while ( zipcode.count () < 8 )
		zipcode.append ( CHR_ZERO );
	if ( !text.contains ( CHR_HYPHEN ) )
		zipcode.insert ( zipcode.length () - 3, CHR_HYPHEN );
	ui->txtClientZipCode->setText ( zipcode );

	setRecValue ( mClientCurItem->clientRecord (), FLD_CLIENT_ZIP, zipcode );
	postFieldAlterationActions ( mClientCurItem );
}

void MainWindow::dteClient_dateAltered ( const vmWidget* const sender )
{
	setRecValue ( mClientCurItem->clientRecord (), static_cast<uint>(sender->id ()),
				  static_cast<const vmDateEdit* const>( sender )->date ().toString ( DATE_FORMAT_DB ) );
	postFieldAlterationActions ( mClientCurItem );
}

void MainWindow::contactsClientAdd ( const QString& info, const vmWidget* const sender )
{
	setRecValue ( mClientCurItem->clientRecord (), static_cast<uint>(sender->id ()),
				  stringRecord::joinStringRecords ( recStrValue ( mClientCurItem->clientRecord (), static_cast<uint>(sender->id ()) ), info ) );
	postFieldAlterationActions ( mClientCurItem );
}

void MainWindow::contactsClientDel ( const int idx, const vmWidget* const sender )
{
	if ( idx >= 0 )
	{
		stringRecord info_rec ( recStrValue ( mClientCurItem->clientRecord (), static_cast<uint>(sender->id ()) ) );
		info_rec.removeField ( static_cast<uint>(idx) );
		setRecValue ( mClientCurItem->clientRecord (), static_cast<uint>(sender->id ()), info_rec.toString () );
		postFieldAlterationActions ( mClientCurItem );
	}
}
//-------------------------------------EDITING-FINISHED-CLIENT---------------------------------------------------

//--------------------------------------------JOB------------------------------------------------------------
void MainWindow::saveJobWidget ( vmWidget* widget, const int id )
{
	widget->setID ( id );
	jobWidgetList[id] = widget;
}

void MainWindow::showJobSearchResult ( vmListItem* item, const bool bshow )
{
	if ( item != nullptr  )
	{
		jobListItem* job_item ( static_cast<jobListItem*> ( item ) );

		if ( bshow )
			displayClient ( static_cast<clientListItem*> ( item->relatedItem ( RLI_CLIENTPARENT ) ), true, job_item );

		for ( uint i ( 0 ); i < JOB_FIELD_COUNT; ++i )
		{
			if ( job_item->searchFieldStatus ( i ) == SS_SEARCH_FOUND )
			{
				if ( i == FLD_JOB_REPORT )
				{
					for ( uint day ( 0 ); day < job_item->searchSubFields ()->count (); ++day )
					{
						if ( job_item->searchSubFields ()->at ( day ) != job_item->searchSubFields ()->defaultValue () )
						{
							static_cast<vmListItem*> ( ui->lstJobDayReport->item ( static_cast<int>(day) ) )->highlight ( bshow ? vmBlue : vmDefault_Color );
							jobWidgetList.at ( FLD_JOB_REPORT + job_item->searchSubFields ()->at ( day ) )->highlight ( bshow ? vmBlue : vmDefault_Color, SEARCH_UI ()->searchTerm () );
						}
					}
				}
				else
					jobWidgetList.at ( i )->highlight ( bshow ? vmBlue : vmDefault_Color, SEARCH_UI ()->searchTerm () );
			}
		}
	}
}

void MainWindow::setupJobPanel ()
{
	connect ( ui->btnJobAdd, &QToolButton::clicked, this, [&] () {
		return (mJobCurItem = addJob ( mClientCurItem )); } );
	connect ( ui->btnJobEdit, &QToolButton::clicked, this, [&] () {
		return static_cast<void>(editJob ( mJobCurItem )); } );
	connect ( ui->btnJobSave, &QToolButton::clicked, this, [&] () {
		return static_cast<void>(saveJob ( mJobCurItem )); } );
	connect ( ui->btnJobCancel, &QToolButton::clicked, this, [&] () {
		return static_cast<void>(cancelJob ( mJobCurItem )); } );
	connect ( ui->btnJobDel, &QToolButton::clicked, this, [&] () {
		return static_cast<void>(delJob ( mJobCurItem )); } );
	connect ( ui->btnQuickProject, &QPushButton::clicked, this, [&] () {
		return on_btnQuickProject_clicked (); } );
	connect ( ui->btnJobEditProjectPath, static_cast<void (QToolButton::*)( const bool )>(&QToolButton::clicked),
			  this, [&] ( const bool checked ) { ui->txtJobProjectPath->setEditable ( checked ); if ( checked ) ui->txtJobProjectPath->setFocus (); } );

	ui->jobsList->setUtilitiesPlaceLayout ( ui->layoutJobsListUtility );
	ui->jobsList->setCallbackForCurrentItemChanged ( [&] ( vmListItem* item ) {
		return jobsListWidget_currentItemChanged ( item ); } );
	ui->lstJobDayReport->setParentLayout( ui->gLayoutJobExtraInfo );
	ui->lstJobDayReport->setCallbackForCurrentItemChanged ( [&] ( vmListItem* item ) {
		return jobDayReportListWidget_currentItemChanged ( item ); } );

	saveJobWidget ( ui->txtJobID, FLD_JOB_ID );
	saveJobWidget ( ui->cboJobType, FLD_JOB_TYPE );
	ui->cboJobType->setCompleter ( vmCompleters::JOB_TYPE );
	ui->cboJobType->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
		return cboJobType_textAltered ( sender ); } );
	ui->cboJobType->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
		return jobKeyPressedSelector ( ke ); } );

	saveJobWidget ( ui->txtJobAddress, FLD_JOB_ADDRESS );
	ui->txtJobAddress->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
		return txtJob_textAltered ( sender ); } );
	ui->txtJobAddress->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
		return jobKeyPressedSelector ( ke ); } );

	saveJobWidget ( ui->txtJobProjectID, FLD_JOB_PROJECT_ID );
	ui->txtJobProjectID->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
		return txtJob_textAltered ( sender ); } );
	ui->txtJobProjectID->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
		return jobKeyPressedSelector ( ke ); } );

	saveJobWidget ( ui->txtJobPrice, FLD_JOB_PRICE );
	ui->txtJobPrice->setTextType ( vmLineEdit::TT_PRICE );
	ui->txtJobPrice->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
		return txtJob_textAltered ( sender ); } );
	ui->txtJobPrice->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
		return jobKeyPressedSelector ( ke ); } );

	saveJobWidget ( ui->txtJobProjectPath, FLD_JOB_PROJECT_PATH );
	ui->txtJobProjectPath->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
		return txtJob_textAltered ( sender ); } );
	ui->txtJobProjectPath->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
		return jobKeyPressedSelector ( ke ); } );

	saveJobWidget ( ui->txtJobPicturesPath, FLD_JOB_PICTURE_PATH );
	ui->txtJobPicturesPath->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
		return txtJob_textAltered ( sender ); } );
	ui->txtJobPicturesPath->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
		return jobKeyPressedSelector ( ke ); } );

	saveJobWidget ( ui->txtJobWheather, FLD_JOB_REPORT + Job::JRF_WHEATHER );
	ui->txtJobWheather->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
		return txtJobWheather_textAltered ( sender ); } );
	ui->txtJobWheather->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
		return jobKeyPressedSelector ( ke ); } );

	saveJobWidget ( ui->txtJobTotalDayTime, FLD_JOB_REPORT + Job::JRF_REPORT + 1 );
	ui->txtJobTotalDayTime->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
		return txtJobTotalDayTime_textAltered ( sender ); } );
	ui->txtJobTotalDayTime->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
		return jobKeyPressedSelector ( ke ); } );

	saveJobWidget ( ui->txtJobReport, FLD_JOB_REPORT + Job::JRF_REPORT );
	ui->txtJobReport->setCallbackForContentsAltered ( [&] ( const vmWidget* const widget ) {
		return updateJobInfo ( static_cast<textEditWithCompleter*>( const_cast<vmWidget*> ( widget ) )->currentText (), JILUR_REPORT ); } );
	ui->txtJobReport->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
		return jobKeyPressedSelector ( ke ); } );

	saveJobWidget ( ui->timeJobStart, FLD_JOB_REPORT + Job::JRF_START_TIME );
	ui->timeJobStart->setCallbackForContentsAltered ( [&] ( const vmWidget* const ) {
		return timeJobStart_timeAltered (); } );
	ui->timeJobStart->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
		return jobKeyPressedSelector ( ke ); } );

	saveJobWidget ( ui->timeJobEnd, FLD_JOB_REPORT + Job::JRF_END_TIME );
	ui->timeJobEnd->setCallbackForContentsAltered ( [&] ( const vmWidget* const ) {
		return timeJobEnd_timeAltered (); } );
	ui->timeJobEnd->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
		return jobKeyPressedSelector ( ke ); } );

	saveJobWidget ( ui->dteJobStart, FLD_JOB_STARTDATE );
	ui->dteJobStart->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
		return dteJob_dateAltered ( sender ); } );
	ui->dteJobStart->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
		return jobKeyPressedSelector ( ke ); } );
	saveJobWidget ( ui->dteJobEnd, FLD_JOB_ENDDATE );
	ui->dteJobEnd->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
		return dteJob_dateAltered ( sender ); } );
	ui->dteJobEnd->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
		return jobKeyPressedSelector ( ke ); } );

	setupJobPictureControl ();

	connect ( ui->btnJobSelectJob, &QToolButton::clicked, this, [&] () {
		return btnJobSelect_clicked (); } );
	connect ( ui->btnJobAddDay, &QToolButton::clicked, this, [&] () {
		return btnJobAddDay_clicked (); } );
	connect ( ui->btnJobEditDay, &QToolButton::clicked, this, [&] () {
		return btnJobEditDay_clicked (); } );
	connect ( ui->btnJobDelDay, &QToolButton::clicked, this, [&] () {
		return btnJobDelDay_clicked (); } );
	connect ( ui->btnJobCancelDelDay, &QToolButton::clicked, this, [&] () {
		return btnJobCancelDelDay_clicked (); } );
	connect ( ui->btnJobPrevDay, &QToolButton::clicked, this, [&] () {
		return btnJobPrevDay_clicked (); } );
	connect ( ui->btnJobNextDay, &QToolButton::clicked, this, [&] () {
		return btnJobNextDay_clicked (); } );
	ui->dteJobAddDate->setCallbackForContentsAltered ( [&] ( const vmWidget* const ) {
		return dteJobAddDate_dateAltered (); } );
	connect ( ui->btnJobMachines, &QToolButton::clicked, this, [&] () {
		return btnJobMachines_clicked (); } );
}

void MainWindow::setUpJobButtons ( const QString& path )
{
	ui->btnJobOpenFileFolder->setEnabled ( false );
	ui->btnJobOpenDoc->setEnabled ( false );
	ui->btnJobOpenXls->setEnabled ( false );
	ui->btnJobOpenPdf->setEnabled ( false );
	if ( fileOps::exists ( path ).isOn () )
	{
		ui->btnJobOpenFileFolder->setEnabled ( true );
		QStringList filesfound;
		QDir dir ( fileOps::dirFromPath ( path ) );
		dir.setFilter ( QDir::Files|QDir::Readable|QDir::Writable );
		dir.setSorting ( QDir::Type|QDir::DirsLast );
		dir.setNameFilters ( QStringList () << QStringLiteral ( "*.doc" ) << QStringLiteral ( "*.docx" )
											<< QStringLiteral ( "*.pdf" ) << QStringLiteral ( "*.xls" )
											<< QStringLiteral ( "*.xlsx" ) );
		filesfound = dir.entryList ();

		if ( !filesfound.isEmpty () )
		{
			if ( menuJobDoc != nullptr )
				menuJobDoc->clear ();
			if ( menuJobPdf != nullptr )
				menuJobPdf->clear ();
			if ( menuJobXls != nullptr )
				menuJobXls->clear ();
			if ( subMenuJobPdfView != nullptr ) {
				subMenuJobPdfView->clear ();
				subMenuJobPdfEMail->clear ();
			}

			QString menutext;
			int i ( 0 );
			do
			{
				menutext = static_cast<QString> ( filesfound.at ( i ) );
				if ( menutext.contains ( configOps::projectDocumentFormerExtension () ) )
				{
					ui->btnJobOpenDoc->setEnabled ( true );
					if ( menuJobDoc == nullptr )
					{
						menuJobDoc = new QMenu ( this );
						ui->btnJobOpenDoc->setMenu ( menuJobDoc );
						connect ( menuJobDoc, &QMenu::triggered, this, [&] ( QAction* action ) { return jobOpenProjectFile ( action ); } );
						connect ( ui->btnJobOpenDoc, &QToolButton::clicked, ui->btnJobOpenDoc, [&] ( const bool ) { return ui->btnJobOpenDoc->showMenu (); } );
					}
					menuJobDoc->addAction ( new vmAction ( 0, menutext, this ) );
				}
				else if ( menutext.contains ( configOps::projectPDFExtension () ) )
				{
					ui->btnJobOpenPdf->setEnabled ( true );
					if ( menuJobPdf == nullptr )
					{
						menuJobPdf = new QMenu ( this );
						ui->btnJobOpenPdf->setMenu ( menuJobPdf );
						subMenuJobPdfView = new QMenu ( TR_FUNC ( "View" ), this );
						menuJobPdf->addMenu ( subMenuJobPdfView );
						subMenuJobPdfEMail = new QMenu ( TR_FUNC ("E-mail" ), this );
						menuJobPdf->addSeparator ();
						menuJobPdf->addMenu ( subMenuJobPdfEMail );
						connect ( subMenuJobPdfView, &QMenu::triggered, this, [&] ( QAction* action ) { return jobOpenProjectFile ( action ); } );
						connect ( subMenuJobPdfEMail, &QMenu::triggered, this, [&] ( QAction* action ) { return jobEMailProjectFile ( action ); } );
						connect ( ui->btnJobOpenPdf, &QToolButton::clicked, ui->btnJobOpenPdf, [&] ( const bool ) { return ui->btnJobOpenPdf->showMenu (); } );
					}
					subMenuJobPdfView->addAction ( new vmAction ( 1, menutext, this ) );
					subMenuJobPdfEMail->addAction ( new vmAction ( 1, menutext, this ) );
				}
				else
				{
					ui->btnJobOpenXls->setEnabled ( true );
					if ( menuJobXls == nullptr )
					{
						menuJobXls = new QMenu ( this );
						ui->btnJobOpenXls->setMenu ( menuJobXls );
						connect ( menuJobXls, &QMenu::triggered, this, [&] ( QAction* action ) { return jobOpenProjectFile ( action ); } );
						connect ( ui->btnJobOpenXls, &QToolButton::clicked, ui->btnJobOpenXls, [&] ( const bool ) { return ui->btnJobOpenXls->showMenu (); } );
					}
					menuJobXls->addAction ( new vmAction ( 2, menutext, this ) );
				}
			} while ( ++i < filesfound.count () );
			filesfound.clear ();
		}
	}
}

void MainWindow::setupJobPictureControl ()
{
	QMenu* menuJobClientsYearPictures ( new QMenu );
	vmAction* action ( nullptr );
	for ( int i ( 2008 ); static_cast<uint>(i) <= vmNumber::currentDate.year (); ++i )
	{
		action = new vmAction ( i, QString::number ( i ), this );
		action->setCheckable ( true );
		menuJobClientsYearPictures->addAction ( action );
	}
	menuJobClientsYearPictures->addSeparator ();
	action = new vmAction ( 0, TR_FUNC ( "Return to seeing current job's pictures" ), this );
	action->setCheckable ( true );
	action->setChecked ( true );
	menuJobClientsYearPictures->addAction ( action );
	jobsPicturesMenuAction = action;

	connect ( menuJobClientsYearPictures, &QMenu::triggered, this, [&] ( QAction* act ) {
		return showClientsYearPictures ( act ); } );
	
	connect ( ui->btnJobAddPictures, &QToolButton::clicked, this, [&] () { return addJobPictures (); } );
	ui->btnJobClientsYearPictures->setMenu ( menuJobClientsYearPictures );
	connect ( ui->btnJobClientsYearPictures, &QToolButton::clicked, ui->btnJobClientsYearPictures, [&] () { return
				ui->btnJobClientsYearPictures->showMenu (); } );
	connect ( ui->btnJobPrevPicture, &QToolButton::clicked, this, [&] () { return
			showJobImageRequested ( ui->jobImageViewer->showPrevImage () ); } );
	connect ( ui->btnJobNextPicture, &QToolButton::clicked, this, [&] () { return
			showJobImageRequested ( ui->jobImageViewer->showNextImage () ); } );
	connect ( ui->btnJobOpenPictureFolder, &QToolButton::clicked, this, [&] () { return
			fileOps::execute ( ui->txtJobPicturesPath->text (), CONFIG ()->fileManager () ); } );
	connect ( ui->btnJobOpenPictureViewer, &QToolButton::clicked, this, [&] () { return
			fileOps::execute ( ui->jobImageViewer->imageCompletePath (), CONFIG ()->pictureViewer () ); } );
	connect ( ui->btnJobOpenPictureEditor, &QToolButton::clicked, this, [&] () { return
			fileOps::execute ( ui->jobImageViewer->imageCompletePath (), CONFIG ()->pictureEditor () ); } );
	connect ( ui->btnJobOpenFileFolder, &QToolButton::clicked, this, [&] () -> void { if ( fileOps::exists ( ui->txtJobProjectPath->text () ).isOn () )
				(void) fileOps::execute ( fileOps::dirFromPath ( ui->txtJobProjectPath->text () ), CONFIG ()->fileManager () ); } );
	connect ( ui->btnJobReloadPictures, &QToolButton::clicked, this, [&] () { return btnJobReloadPictures_clicked (); } );
	connect ( ui->btnJobRenamePicture, static_cast<void (QToolButton::*)(bool)>( &QToolButton::clicked ), this, [&] ( bool checked ) { return
				btnJobRenamePicture_clicked ( checked ); } );
	connect ( ui->btnJobSeparatePicture, static_cast<void (QToolButton::*)(bool)>( &QToolButton::clicked ), this, [&] ( bool checked ) { return
			checked ? showJobImageInWindow ( false ) : sepWin_JobPictures->returnToParent (); } );
	connect ( ui->btnJobSeparateReportWindow, static_cast<void (QToolButton::*)(bool)>( &QToolButton::clicked ), this, [&] ( bool checked ) { return
		btnJobSeparateReportWindow_clicked ( checked ); } );

	ui->cboJobPictures->setIgnoreChanges ( false );
	ui->cboJobPictures->setCallbackForIndexChanged ( [&] ( const int idx ) { return showJobImage ( idx ); } );
	ui->jobImageViewer->setCallbackForshowImageRequested ( [&] ( const int idx ) { return showJobImageRequested ( idx ); } );
	ui->jobImageViewer->setCallbackForshowMaximized ( [&] ( const bool maximized ) { return showJobImageInWindow ( maximized ); } );

	// Start off with an empty image. This will prevent the image viewer from crashing
	ui->jobImageViewer->showImage ( -1, emptyString );
}

void MainWindow::displayJobFromCalendar ( vmListItem* cal_item )
{
	clientListItem* client_item ( static_cast<clientListItem*>( cal_item->relatedItem ( RLI_CLIENTPARENT ) ) );
	if ( client_item )
	{
		if ( client_item != mClientCurItem )
			displayClient ( client_item, true, static_cast<jobListItem*>( cal_item->relatedItem ( RLI_CALENDARITEM ) ) );
		else
			displayJob ( static_cast<jobListItem*>( cal_item->relatedItem ( RLI_CALENDARITEM ) ), true );
		
		ui->lstJobDayReport->setCurrentRow ( cal_item->data ( Qt::UserRole ).toInt () - 1, true ); // select pertinent day
	}
}

void MainWindow::jobKeyPressedSelector ( const QKeyEvent* ke )
{
	switch ( ke->key () )
	{
		case Qt::Key_Enter:
		case Qt::Key_Return:
			ui->clientsList->setFocus (); // trigger any pending editing_Finished or textAltered event
			saveJob ( mJobCurItem );
		break;
		case Qt::Key_S:
			if ( mJobCurItem )
			{
				updateJobInfo ( ui->txtJobReport->currentText (), JILUR_REPORT );
				if ( mJobCurItem->jobRecord ()->saveRecord () )
					VM_NOTIFY ()->notifyMessage ( TR_FUNC ( "Job report committed" ), TR_FUNC ( "You still need to click on Save to commit other changes" ) );
			}
		break;
		case Qt::Key_Escape:
			static_cast<void>(cancelJob ( mJobCurItem ));
		break;
	}
}

void MainWindow::jobsListWidget_currentItemChanged ( vmListItem* item )
{
	insertNavItem ( item );
	displayJob ( static_cast<jobListItem*> ( item ) );
}

void MainWindow::jobDayReportListWidget_currentItemChanged ( vmListItem* item )
{
	if ( item && !item->data ( JILUR_REMOVED ).toBool () )
	{
		const vmNumber time1 ( item->data ( JILUR_START_TIME ).toString (), VMNT_TIME, vmNumber::VTF_DAYS );
		ui->timeJobStart->setTime ( time1.toQTime () );
		vmNumber timeAndDate ( item->data ( JILUR_END_TIME ).toString (), VMNT_TIME, vmNumber::VTF_DAYS );
		ui->timeJobEnd->setTime ( timeAndDate.toQTime () );
		timeAndDate -= time1;
		ui->txtJobTotalDayTime->setText ( timeAndDate.toTime ( vmNumber::VTF_FANCY ) );
		timeAndDate.fromTrustedStrDate ( item->data ( JILUR_DATE ).toString (), vmNumber::VDF_HUMAN_DATE );
		ui->dteJobAddDate->setDate ( timeAndDate );
		showDayPictures ( timeAndDate );
		ui->txtJobWheather->setText ( item->data ( JILUR_WHEATHER ).toString () );
		ui->txtJobReport->textEditWithCompleter::setText ( item->data ( JILUR_REPORT ).toString () );
	}
	else
	{
		ui->timeJobStart->setTime ( vmNumber () );
		ui->timeJobEnd->setTime ( vmNumber () );
		ui->txtJobTotalDayTime->setText ( emptyString );
		ui->txtJobWheather->setText ( emptyString );
		ui->txtJobReport->textEditWithCompleter::setText ( item ? TR_FUNC ( " - THIS DAY WAS REMOVED - " ) : emptyString );
		ui->btnJobDelDay->setEnabled ( false );
		ui->btnJobCancelDelDay->setEnabled ( false );
	}
	ui->btnJobNextDay->setEnabled ( ui->lstJobDayReport->currentRow () < (ui->lstJobDayReport->count () - 1) );
	ui->btnJobPrevDay->setEnabled ( ui->lstJobDayReport->currentRow () >= 1 );
}

void MainWindow::controlJobForms ( const jobListItem* const job_item )
{
	const triStateType editing_action ( job_item ? static_cast<TRI_STATE> ( job_item->action () >= ACTION_ADD ) : TRI_UNDEF );

	ui->cboJobType->setEditable ( editing_action.isOn () );
	ui->txtJobAddress->setEditable ( editing_action.isOn () );
	ui->txtJobPrice->setEditable ( editing_action.isOn () );
	ui->txtJobProjectPath->setEditable ( editing_action.isOn () );
	ui->txtJobProjectID->setEditable ( editing_action.isOn () );
	ui->txtJobPicturesPath->setEditable ( editing_action.isOn () );
	ui->dteJobStart->setEditable ( editing_action.isOn () );
	ui->dteJobEnd->setEditable ( editing_action.isOn () );
	ui->dteJobAddDate->setEditable ( editing_action.isOn () );

	ui->btnJobSave->setEnabled ( editing_action.isOn () ? job_item->isGoodToSave () : false );
	ui->btnJobEdit->setEnabled ( editing_action.isOff () );
	ui->btnJobCancel->setEnabled ( editing_action.isOn () );
	ui->btnJobDel->setEnabled ( editing_action.isOff () );
	ui->btnJobAdd->setEnabled ( mClientCurItem != nullptr ? mClientCurItem->action() != ACTION_ADD : false );
	ui->btnJobEditProjectPath->setEnabled ( editing_action.isOn () );

	ui->cboJobType->highlight ( editing_action.isOn () ? ( ui->cboJobType->text ().isEmpty () ? vmRed : vmDefault_Color ) : vmDefault_Color );
	ui->dteJobStart->highlight ( editing_action.isOn () ? ( vmNumber ( ui->dteJobStart->date () ).isDateWithinRange ( vmNumber::currentDate, 0, 4 ) ? vmDefault_Color : vmRed ) : vmDefault_Color );

	if ( jobsPicturesMenuAction != nullptr )
	{
		jobsPicturesMenuAction->setChecked ( false );
		jobsPicturesMenuAction = ui->btnJobClientsYearPictures->menu ()->actions ().last ();
		jobsPicturesMenuAction->setChecked ( true );
	}
	ui->btnQuickProject->setEnabled ( editing_action.isOff () ? true : ( editing_action.isOn () ? job_item->fieldInputStatus ( FLD_JOB_STARTDATE ) : false ) );
	controlJobDayForms ( job_item );
	controlJobPictureControls ();
}

void MainWindow::controlJobDayForms ( const jobListItem* const job_item )
{
	const triStateType editing_action ( job_item ? static_cast<TRI_STATE> ( job_item->action () != ACTION_READ ) : TRI_UNDEF );
	const bool b_hasDays ( ( ui->lstJobDayReport->count () > 0 ) && editing_action.isOn () );
	ui->timeJobStart->setEditable ( b_hasDays );
	ui->timeJobEnd->setEditable ( b_hasDays );
	ui->txtJobWheather->setEditable ( b_hasDays );
	ui->txtJobTotalDayTime->setEditable ( b_hasDays );
	ui->txtJobReport->setEditable ( b_hasDays );

	if ( editing_action.isOn () )
	{
		dteJobAddDate_dateAltered ();
	}
	else
	{
		ui->btnJobAddDay->setEnabled ( false );
		ui->btnJobEditDay->setEnabled ( b_hasDays );	
	}
	ui->btnJobDelDay->setEnabled ( b_hasDays );
	ui->btnJobCancelDelDay->setEnabled ( b_hasDays ? ui->lstJobDayReport->currentItem ()->data ( JILUR_REMOVED ).toBool () : false );
	ui->btnJobSeparateReportWindow->setEnabled ( job_item != nullptr );
	
	ui->btnJobPrevDay->setEnabled ( ui->lstJobDayReport-> currentRow () >= 1 );
	ui->btnJobNextDay->setEnabled ( ui->lstJobDayReport->currentRow () < ( ui->lstJobDayReport->count () - 1 ) );
}

void MainWindow::controlJobPictureControls ()
{
	const bool b_hasPictures ( ui->cboJobPictures->count () > 0 );
	ui->btnJobNextPicture->setEnabled ( b_hasPictures );
	ui->btnJobPrevPicture->setEnabled ( false );
	ui->btnJobOpenPictureEditor->setEnabled ( b_hasPictures );
	ui->btnJobOpenPictureFolder->setEnabled ( b_hasPictures );
	ui->btnJobOpenPictureViewer->setEnabled ( b_hasPictures );
	ui->btnJobReloadPictures->setEnabled ( !ui->txtJobPicturesPath->text ().isEmpty () );
	ui->btnJobRenamePicture->setEnabled ( b_hasPictures );
	ui->btnJobClientsYearPictures->setEnabled ( mJobCurItem != nullptr );
	ui->btnJobSeparatePicture->setEnabled ( b_hasPictures );
	const QString n_pics ( QString::number ( ui->cboJobPictures->count () ) + TR_FUNC ( " pictures" ) );
	ui->lblJobPicturesCount->setText ( ui->cboJobPictures->count () > 1 ? n_pics : n_pics.left ( n_pics.count () - 1 ) );
}

bool MainWindow::saveJob ( jobListItem* job_item )
{
	if ( job_item )
	{
		ui->txtJobReport->saveContents ( true ); // force committing the newest text to the buffers. Avoid depending on Qt's signals, which might occur later than when reaching here
		Job* job ( job_item->jobRecord () );
		if ( job->saveRecord ( false ) ) // do not change indexes just now. Wait for dbCalendar actions
		{
			if ( job_item->action () == ACTION_ADD )
			{
				ui->jobImageViewer->setID ( recIntValue ( job_item->jobRecord (), FLD_JOB_ID ) );
				saveJobPayment ( job_item );
			}
			else
			{
				if ( job->wasModified ( FLD_JOB_PRICE ) )
					alterJobPayPrice ( job_item );
			}
			mCal->updateCalendarWithJobInfo ( job );
			job_item->setAction ( ACTION_READ, true ); // now we can change indexes
			
			rescanJobDaysList ( job_item );
			VM_NOTIFY ()->notifyMessage ( TR_FUNC ( "Record saved" ), TR_FUNC ( "Job data saved!" ) );
			controlJobForms ( job_item );
			//When the job list is empty, we cannot add a purchase. Now that we've saved a job, might be we added one too, so now we can add a buy
			ui->btnBuyAdd->setEnabled ( true );
			saveView ();
			removeEditItem ( static_cast<vmListItem*>(job_item) );
			return true;
		}
	}
	return false;
}

jobListItem* MainWindow::addJob ( clientListItem* parent_client )
{
	jobListItem* job_item ( new jobListItem );
	job_item->setRelation ( RLI_CLIENTITEM );
	job_item->setRelatedItem ( RLI_CLIENTPARENT, static_cast<vmListItem*>(parent_client) );
	job_item->setRelatedItem ( RLI_JOBPARENT, job_item );
	static_cast<void>(job_item->loadData ());
	job_item->setAction ( ACTION_ADD, true );
	job_item->addToList ( ui->jobsList );
	parent_client->jobs->append ( job_item );
	insertEditItem ( static_cast<vmListItem*>(job_item) );
	setRecValue ( job_item->jobRecord (), FLD_JOB_CLIENTID, recStrValue ( parent_client->clientRecord (), FLD_CLIENT_ID ) );
	job_item->setDBRecID ( static_cast<uint>(recIntValue ( job_item->jobRecord (), FLD_JOB_ID )) );
	addJobPayment ( job_item );
	ui->cboJobType->setFocus ();
	return job_item;
}

bool MainWindow::editJob ( jobListItem* job_item, const bool b_dogui )
{
	if ( job_item )
	{
		if ( job_item->action () == ACTION_READ )
		{
			job_item->setAction ( ACTION_EDIT, true );
			insertEditItem ( static_cast<vmListItem*>(job_item) );
			if ( b_dogui )
			{
				VM_NOTIFY ()->notifyMessage ( TR_FUNC ( "Editing job record" ), recStrValue ( job_item->jobRecord (), FLD_JOB_TYPE ) );
				controlJobForms ( job_item );
				ui->txtJobPrice->setFocus ();
			}
			return true;
		}
	}
	return false;
}

bool MainWindow::delJob ( jobListItem* job_item )
{
	if ( job_item )
	{
		const QString text ( TR_FUNC ( "Are you sure you want to remove job %1?" ) );
		if ( VM_NOTIFY ()->questionBox ( TR_FUNC ( "Question" ), text.arg ( recStrValue ( job_item->jobRecord (), FLD_JOB_TYPE ) ) ) )
		{
			job_item->setAction ( ACTION_DEL, true );
			mCal->updateCalendarWithJobInfo ( job_item->jobRecord () ); // remove job info from calendar and clear job days list
			return cancelJob ( job_item );
		}
	}
	return false;
}

bool MainWindow::cancelJob ( jobListItem* job_item )
{
	if ( job_item )
	{
		switch ( job_item->action () )
		{
			case ACTION_ADD:
			case ACTION_DEL:
			{
				removeJobPayment ( job_item->payItem () );
				static_cast<clientListItem*>(job_item->relatedItem ( RLI_CLIENTPARENT ))->jobs->remove ( job_item->row () );
				removeListItem ( job_item );
			}
			break;
			case ACTION_EDIT:
				removeEditItem ( static_cast<vmListItem*>(job_item) );
				job_item->setAction ( ACTION_REVERT, true );
				fixJobDaysList ( job_item );
				displayJob ( job_item );
			break;
			case ACTION_NONE:
			case ACTION_READ:
			case ACTION_REVERT:
				return false;
		}
		return true;
	}
	return false;
}

void MainWindow::displayJob ( jobListItem* job_item, const bool b_select, buyListItem* buy_item )
{
	if ( job_item )
	{
		if ( job_item->loadData () )
		{
			ui->tabMain->setCurrentIndex ( 0 );
			loadJobInfo ( job_item->jobRecord () );
			controlJobForms ( job_item );
			if ( job_item != mJobCurItem )
			{
				if ( b_select )
					ui->jobsList->setCurrentItem ( job_item, false );
				mJobCurItem = job_item;
				displayPay ( job_item->payItem (), true );
				fillJobBuyList ( job_item );
				if ( !buy_item )
				{
					if ( !job_item->buys->isEmpty () )
						buy_item = static_cast<buyListItem*>( job_item->buys->last ()->relatedItem ( RLI_CLIENTITEM ) );
				}
				displayBuy ( buy_item, true );
			}
		}
	}
	else
	{
		mJobCurItem = nullptr;
		controlJobForms ( nullptr );
		loadJobInfo ( nullptr );
	}
	if ( b_select )
		saveView ();
}

void MainWindow::loadJobInfo ( const Job* const job )
{
	decodeJobReportInfo ( job );
	if ( job )
	{
		ui->txtJobID->setText ( recStrValue ( job, FLD_JOB_ID ) );
		ui->cboJobType->setText ( recStrValue ( job, FLD_JOB_TYPE ) );
		ui->txtJobAddress->setText ( Job::jobAddress ( job, mClientCurItem->clientRecord () ) );
		ui->txtJobPrice->setText ( recStrValue ( job, FLD_JOB_PRICE ) );
		ui->txtJobProjectPath->setText ( recStrValue ( job, FLD_JOB_PROJECT_PATH ) );
		ui->txtJobPicturesPath->setText ( recStrValue ( job, FLD_JOB_PICTURE_PATH ) );
		ui->txtJobProjectID->setText ( recStrValue ( job, FLD_JOB_PROJECT_ID ) );
		ui->jobImageViewer->showImage ( recIntValue ( job, FLD_JOB_ID ), recStrValue ( job, FLD_JOB_PICTURE_PATH ) );
		ui->dteJobStart->setDate ( job->date ( FLD_JOB_STARTDATE ).toQDate () );
		ui->dteJobEnd->setDate ( job->date ( FLD_JOB_ENDDATE ).toQDate () );
		ui->txtJobTotalAllDaysTime->setText ( job->time ( FLD_JOB_TIME ).toTime ( vmNumber::VTF_FANCY ) );
	}
	else
	{
		ui->txtJobID->setText ( QStringLiteral ( "-1" ) );
		ui->cboJobType->clearEditText ();
		ui->txtJobAddress->setText ( Job::jobAddress ( nullptr, mClientCurItem->clientRecord () ) );
		ui->txtJobPrice->setText ( vmNumber::zeroedPrice.toPrice () );
		ui->txtJobProjectPath->clear ();
		ui->txtJobPicturesPath->clear ();
		ui->txtJobProjectID->clear ();
		ui->jobImageViewer->showImage ( -1, emptyString );
		ui->cboJobPictures->clearEditText ();
		ui->cboJobPictures->clear ();
		ui->dteJobStart->setDate ( vmNumber () );
		ui->dteJobEnd->setDate ( vmNumber () );
		ui->txtJobTotalAllDaysTime->clear ();
	}
	scanJobImages ();
	setUpJobButtons ( ui->txtJobProjectPath->text () );
}

jobListItem* MainWindow::getJobItem ( const clientListItem* const parent_client, const uint id ) const
{
	if ( id >= 1 && parent_client )
	{
		int i ( 0 );
		while ( i < static_cast<int> ( parent_client->jobs->count () ) )
		{
			if ( parent_client->jobs->at ( i )->dbRecID () == id )
				return parent_client->jobs->at ( i );
			++i;
		}
	}
	return nullptr;
}

void MainWindow::scanJobImages ()
{
	ui->cboJobPictures->clear ();
	ui->cboJobPictures->insertItems ( 0, ui->jobImageViewer->imagesList () );
	if ( ui->cboJobPictures->count () == 0 )
	{
		const uint year ( static_cast<uint> ( vmNumber::currentDate.year () ) );
		const QString picturePath ( CONFIG ()->getProjectBasePath ( ui->txtClientName->text () ) +
									jobPicturesSubDir );
		fileOps::createDir ( picturePath );
		QMenu* yearsMenu ( ui->btnJobClientsYearPictures->menu () );
		for ( uint i ( 0 ), y ( 2008 ); y <= year; ++i, ++y )
		{
			yearsMenu->actions ().at ( static_cast<int>(i) )->setEnabled (
				fileOps::isDir ( picturePath + QString::number ( y ) ).isOn () );
		}
	}
	ui->cboJobPictures->setEditText ( ui->jobImageViewer->imageFileName () );
	controlJobPictureControls ();
}

void MainWindow::decodeJobReportInfo ( const Job* const job )
{
	ui->lstJobDayReport->clear ();
	ui->dteJobAddDate->setDate ( vmNumber () );
	ui->txtJobReport->clear ();
	ui->txtJobTotalAllDaysTime->clear ();
	ui->txtJobTotalDayTime->clear ();
	ui->txtJobWheather->clear ();
	ui->timeJobStart->setTime ( vmNumber () );
	ui->timeJobEnd->setTime ( vmNumber () );

	if ( job != nullptr )
	{
		if ( job->jobItem ()->daysList->isEmpty () )
		{
			const stringTable& str_report ( recStrValue ( job, FLD_JOB_REPORT ) );
			const stringRecord* rec ( nullptr );

			rec = &str_report.first ();
			if ( rec->isOK () )
			{
				QString str_date;
				vmListItem* day_entry ( nullptr );
				uint day ( 1 ), jilur_idx ( 0 );
				vmNumber totalDayTime;
				do
				{
					if ( rec->first () )
					{
						str_date = vmNumber ( rec->curValue (), VMNT_DATE, vmNumber::VDF_DB_DATE ).toDate ( vmNumber::VDF_HUMAN_DATE );
						day_entry = new vmListItem ( QString::number ( day++ ) + lstJobReportItemPrefix + str_date ); //any type id
						day_entry->setData ( JILUR_DATE, str_date );
						jilur_idx = JILUR_START_TIME;
	
						while ( rec->next () )
						{
							switch ( jilur_idx )
							{
								case JILUR_START_TIME:
									totalDayTime.fromTrustedStrTime ( rec->curValue (), vmNumber::VTF_DAYS );
									totalDayTime.makeOpposite ();
									day_entry->setData ( JILUR_START_TIME, rec->curValue () );
								break;
								case JILUR_END_TIME:
									day_entry->setData ( JILUR_END_TIME, rec->curValue () );
									totalDayTime += vmNumber ( rec->curValue (), VMNT_TIME, vmNumber::VTF_DAYS );
									day_entry->setData ( JILUR_DAY_TIME, totalDayTime.toTime () );
								break;
								case JILUR_WHEATHER:
									day_entry->setData ( JILUR_WHEATHER, rec->curValue () );
								break;
								case JILUR_REPORT:
									day_entry->setData ( JILUR_REPORT, rec->curValue () );
								break;
							}
							if ( ++jilur_idx > JILUR_REPORT )
								break; // it should never reach this statement, but I include it as a failsafe measure
						}
						day_entry->setData ( JILUR_ADDED, false );
						day_entry->setData ( JILUR_REMOVED, false );
						ui->lstJobDayReport->addItem ( day_entry );
						job->jobItem ()->daysList->append ( day_entry );
					}
					rec = &str_report.next ();
				} while ( rec->isOK () );
			}
		}
		else
		{
			for ( uint i ( 0 ); i < job->jobItem ()->daysList->count (); ++i )
				ui->lstJobDayReport->addItem ( job->jobItem ()->daysList->at ( i ) );
		}
		ui->lstJobDayReport->setIgnoreChanges ( false );
		ui->lstJobDayReport->setCurrentRow ( ui->lstJobDayReport->count () - 1, true );
	}
}

void MainWindow::fixJobDaysList ( jobListItem* const job_item )
{
	PointersList<vmListItem*>* daysList ( job_item->daysList );
	vmListItem* day_item ( daysList->last () );
	if ( day_item != nullptr )
	{
		int day (day_item->row ());
		do
		{
			if ( day_item->data ( JILUR_ADDED ).toBool () )
				daysList->remove ( day, true );
			else
			{
				if ( day_item->data ( JILUR_REMOVED ).toBool () )
					revertDayItem ( day_item );
			}
			--day;
		} while ( (day_item = daysList->prev ()) != nullptr );
	}
}

void MainWindow::revertDayItem ( vmListItem* day_item )
{
	QFont fntStriked ( ui->lstJobDayReport->font () );
	fntStriked.setStrikeOut ( false );
	day_item->setFont ( fntStriked );
	day_item->setBackground ( QBrush ( Qt::white ) );
	day_item->setData ( JILUR_REMOVED, false );
}

void MainWindow::updateJobInfo ( const QString& text, const uint user_role, vmListItem* const item )
{
	vmListItem* const day_entry ( item ? item : ui->lstJobDayReport->currentItem () );
	if ( day_entry )
	{
		const uint cur_row ( static_cast<uint>(ui->lstJobDayReport->currentRow ()) );
		day_entry->setData ( static_cast<int>(user_role), text );
		stringTable job_report ( recStrValue ( mJobCurItem->jobRecord (), FLD_JOB_REPORT ) );
		stringRecord rec ( job_report.readRecord ( cur_row ) );
		rec.changeValue ( user_role - Qt::UserRole, text );
		job_report.changeRecord ( cur_row, rec );
		setRecValue ( mJobCurItem->jobRecord (), FLD_JOB_REPORT, job_report.toString () );
		postFieldAlterationActions ( mJobCurItem );
	}
}

void MainWindow::addJobPayment ( jobListItem* const job_item )
{
	payListItem* pay_item ( new payListItem );
	pay_item->setRelation ( RLI_CLIENTITEM );
	pay_item->setRelatedItem ( RLI_CLIENTPARENT, job_item->relatedItem ( RLI_CLIENTPARENT ) );
	pay_item->setRelatedItem ( RLI_JOBPARENT, job_item );
	pay_item->createDBRecord ();
	pay_item->setAction ( ACTION_ADD, true );
	pay_item->addToList ( ui->paysList );
	pay_item->setDBRecID ( static_cast<uint>(recIntValue ( pay_item->payRecord (), FLD_PAY_ID )) );
	static_cast<clientListItem*>(job_item->relatedItem ( RLI_CLIENTPARENT ))->pays->append ( pay_item );
	job_item->setPayItem ( pay_item );
	insertEditItem ( static_cast<vmListItem*>(mPayCurItem) );
	mPayCurItem = pay_item;
}

void MainWindow::saveJobPayment ( jobListItem* const job_item )
{
	Payment* pay ( job_item->payItem ()->payRecord () );
	
	ui->txtPayTotalPrice->setText ( recStrValue ( job_item->jobRecord (), FLD_JOB_PRICE ), true );
	ui->txtPayID->setText ( recStrValue ( job_item->jobRecord (), FLD_JOB_ID ) );
	
	setRecValue ( pay, FLD_PAY_CLIENTID, recStrValue ( job_item->jobRecord (), FLD_JOB_CLIENTID ) );
	setRecValue ( pay, FLD_PAY_JOBID, recStrValue ( job_item->jobRecord (), FLD_JOB_ID ) );
	
	const vmNumber price ( pay->price ( FLD_PAY_PRICE ) );
	setRecValue ( pay, FLD_PAY_OVERDUE_VALUE, price.toPrice () );
	setRecValue ( pay, FLD_PAY_OVERDUE, price.isNull () ? CHR_ZERO : CHR_ONE );
	
	if ( pay->saveRecord () )
	{
		job_item->payItem ()->setAction ( pay->action () );
		job_item->payItem ()->setDBRecID ( static_cast<uint>(recIntValue ( pay, FLD_PAY_ID )) );
		addPaymentOverdueItems ( job_item->payItem () );
		payOverdueGUIActions ( pay, ACTION_ADD );
		ui->paysList->setCurrentItem ( job_item->payItem (), true );
		controlPayForms ( job_item->payItem () ); // enable/disable payment controls for immediate editing if the user wants to
		removeEditItem ( static_cast<vmListItem*>(job_item->payItem ()) );
	}
}

void MainWindow::removeJobPayment ( payListItem* pay_item )
{
	if ( pay_item )
	{
		pay_item->setAction ( ACTION_DEL, true );
		payOverdueGUIActions ( pay_item->payRecord (), ACTION_DEL );
		updateTotalPayValue ( vmNumber::zeroedPrice, pay_item->payRecord ()->price ( FLD_PAY_PRICE ) );
		mCal->updateCalendarWithPayInfo ( pay_item->payRecord () );
		removePaymentOverdueItems ( pay_item );
		mClientCurItem->pays->remove ( ui->paysList->currentRow () );
		removeListItem ( pay_item, false, false );
	}
}

void MainWindow::alterJobPayPrice ( jobListItem* const job_item )
{
	Payment* pay ( job_item->payItem ()->payRecord () );
	pay->setAction ( ACTION_EDIT );
	ui->txtPayTotalPrice->setText ( recStrValue ( job_item->jobRecord (), FLD_JOB_PRICE ), true );
	savePay ( job_item->payItem () );
}

void MainWindow::calcJobTime ()
{
	vmNumber time ( ui->timeJobEnd->time () );
	time -= ui->timeJobStart->time ();
	ui->txtJobTotalDayTime->setText ( time.toTime ( vmNumber::VTF_FANCY ) ); // Just a visual feedback

	vmNumber total_time;
	for ( int i ( 0 ); i < ui->lstJobDayReport->count (); ++i )
	{
		if ( ui->lstJobDayReport->item ( i )->data ( JILUR_REMOVED ).toBool () == false )
		{
			if ( i != ui->lstJobDayReport->currentRow () )
			{
				total_time += vmNumber ( ui->lstJobDayReport->item ( i )->data (
										 JILUR_DAY_TIME ).toString (), VMNT_TIME, vmNumber::VTF_DAYS );
			}
			else
				ui->lstJobDayReport->item ( i )->setData ( JILUR_DAY_TIME, time.toTime () );
		}
	}

	total_time += time;
	mJobCurItem->jobRecord ()->setTime ( FLD_JOB_TIME, total_time );
	// I could ask setText to emit the signal and have saving the time from within the textAltered callback function
	// but then the vmNumber would have to parse the fancy format and it is not worth the trouble nor the wasted cpu cycles
	ui->txtJobTotalAllDaysTime->setText ( total_time.toTime ( vmNumber::VTF_FANCY ) );
	postFieldAlterationActions ( mJobCurItem );
}

triStateType MainWindow::dateIsInDaysList ( const QString& str_date )
{
	vmListItem* item ( nullptr );
	int i ( ui->lstJobDayReport->count () - 1 );
	for ( ; i >= 0 ; --i )
	{
		item = ui->lstJobDayReport->item ( i );
		if ( !item->data ( JILUR_REMOVED ).toBool () )
		{
			if ( str_date == item->data ( JILUR_DATE ).toString () )
				return TRI_ON;
		}
	}
	return ui->lstJobDayReport->isEmpty () ? TRI_UNDEF : TRI_OFF;
}

void MainWindow::rescanJobDaysList ( jobListItem *const job_item )
{
	vmListItem* day_entry ( nullptr );
	bool b_reorder ( false );
	int removedRow ( -1 );
	QString itemText;

	stringTable jobReport ( recStrValue ( job_item->jobRecord (), FLD_JOB_REPORT ) );
	for ( int i ( 0 ); i < ui->lstJobDayReport->count (); )
	{
		day_entry = ui->lstJobDayReport->item ( i );
		if ( day_entry->data ( JILUR_REMOVED ).toBool () == true )
		{
			ui->lstJobDayReport->removeRow ( static_cast<uint>(i) ); // Let the jobListItem handle the item deletion
			job_item->daysList->remove ( i, true );
			b_reorder = true;
			removedRow = i;
			jobReport.removeRecord ( static_cast<uint>(i) );
		}
		else
		{
			if ( day_entry->data ( JILUR_ADDED ).toBool () == true )
				day_entry->setData ( JILUR_ADDED, false );
			if ( b_reorder )
			{
				itemText = day_entry->data ( Qt::DisplayRole ).toString (); //QListWidgetItem::text () is an inline function that calls data ()
				itemText.remove ( 0, itemText.indexOf ( QLatin1Char ( 'o' ) ) ); // Removes all characters before 'o' from lstJobReportItemPrefix ( "o dia")
				itemText.prepend ( QString::number ( i + 1 ) ); //Starts the new label with item index minus number of all items removed. The +1 is because day counting starts at 1, not 0
				day_entry->setText ( itemText, false, false, false );
			}
			++i; //move to the next item only when we did not remove an item
		}
	}
	if ( removedRow >= 0 )
	{
		if ( removedRow >= ui->lstJobDayReport->count () )
			removedRow = ui->lstJobDayReport->count () - 1;
		else if ( removedRow > 0 )
			removedRow--;
		else
			removedRow = 0;
		ui->lstJobDayReport->setCurrentRow ( removedRow, true );
		job_item->jobRecord ()->setAction ( ACTION_EDIT );
		setRecValue ( job_item->jobRecord (), FLD_JOB_REPORT, jobReport.toString () );
		job_item->jobRecord ()->saveRecord ();
	}
}

void MainWindow::fillCalendarJobsList ( const stringTable& jobids, vmListWidget* list )
{
	if ( jobids.countRecords () > 0 )
	{
		const stringRecord* str_rec ( &jobids.first () );
		if ( str_rec->isOK () )
		{
			uint jobid ( 0 );
			const QLatin1String dayStr ( " (Day(s) " );
			jobListItem* job_item ( nullptr ), *job_parent ( nullptr );
			clientListItem* client_parent ( nullptr );
			Job job;
			QString day, curLabel;
			list->setIgnoreChanges ( true );
			do
			{
				jobid =  str_rec->fieldValue ( 0 ).toUInt () ;
				if ( job.readRecord ( jobid ) )
				{
					client_parent = getClientItem ( static_cast<uint>(recIntValue ( &job, FLD_JOB_CLIENTID )) );
					job_parent = getJobItem ( client_parent, static_cast<uint>(recIntValue ( &job, FLD_JOB_ID )) );
					
					if ( job_parent )
					{
						day = str_rec->fieldValue ( 2 );
						job_item = static_cast<jobListItem*>( job_parent->relatedItem ( RLI_CALENDARITEM ) );
						if ( !job_item )
						{
							job_item = new jobListItem;
							job_item->setRelation ( RLI_CALENDARITEM );
							job_parent->syncSiblingWithThis ( job_item );
						}
						
						job_item->setData ( Qt::UserRole, day.toInt () );
						if ( job_item->listWidget () != list )
						{
							job_item->setText ( recStrValue ( client_parent->clientRecord (), FLD_CLIENT_NAME ) +
								CHR_SPACE + CHR_HYPHEN + CHR_SPACE + recStrValue ( &job, FLD_JOB_TYPE ) +
								dayStr + day + CHR_R_PARENTHESIS, false, false, false );
							job_item->addToList ( list );
						}
						else
						{
							curLabel = job_item->text ();
							curLabel.insert ( curLabel.count () - 1, CHR_COMMA + day );
							job_item->setText ( curLabel, false, false, false );
						}
					}
				}
				str_rec = &jobids.next ();
			} while ( str_rec->isOK () );
			list->setIgnoreChanges ( false );
		}
	}
}

void MainWindow::controlFormsForJobSelection ( const bool bStartSelection )
{
	ui->btnJobSelectJob->setEnabled ( bStartSelection );
	ui->tabMain->widget ( TI_CALENDAR )->setEnabled ( !bStartSelection );
	ui->tabMain->widget ( TI_INVENTORY )->setEnabled ( !bStartSelection );
	ui->tabMain->widget ( TI_SUPPLIES )->setEnabled ( !bStartSelection );
}

void MainWindow::selectJob ()
{
	controlFormsForJobSelection ( true );
	qApp->setActiveWindow ( this );
	this->setFocus ();
	VM_NOTIFY ()->notifyMessage ( TR_FUNC ( "Select the job you want" ), TR_FUNC ( "Then click on the button at the botton of jobs toolbar, SELECT THIS JOB. Press the ESC key to cancel." ) );
}

void MainWindow::btnJobSelect_clicked ()
{
	if ( selJob_callback && mJobCurItem ) {
		selJob_callback ( mJobCurItem->dbRecID () );
		selJob_callback = nullptr;
		controlFormsForJobSelection ( false );
	}
}

void MainWindow::btnJobAddDay_clicked ()
{
	const vmNumber vmdate ( ui->dteJobAddDate->date () );
	jobDayReportListWidget_currentItemChanged ( nullptr ); // clear forms

	const int n_days ( ui->lstJobDayReport->count () + 1 );
	vmListItem* new_item ( new vmListItem ( QString::number ( n_days ) + lstJobReportItemPrefix + vmdate.toDate ( vmNumber::VDF_HUMAN_DATE ) ) );
	new_item->setData ( JILUR_ADDED, true );
	new_item->setData ( JILUR_REMOVED, false );
	new_item->addToList ( ui->lstJobDayReport );
	updateJobInfo ( vmdate.toDate ( vmNumber::VDF_DB_DATE ), JILUR_DATE, new_item );
	
	if ( n_days == 1 )
	{
		ui->dteJobStart->setDate ( vmdate, true );
		controlJobDayForms ( mJobCurItem );
	}
	ui->dteJobEnd->setDate ( vmdate, true );
	ui->btnJobPrevDay->setEnabled ( n_days > 1 );
	ui->btnJobNextDay->setEnabled ( false );
	ui->btnJobDelDay->setEnabled ( true );
	ui->btnJobCancelDelDay->setEnabled ( false );
	ui->btnJobEditDay->setEnabled ( true );
	ui->btnJobAddDay->setEnabled( false );
	mJobCurItem->daysList->append ( new_item );
	//ui->lstJobDayReport->setIgnoreChanges ( false );
}

void MainWindow::btnJobEditDay_clicked ()
{
	const vmNumber vmdate ( ui->dteJobAddDate->date () );
	ui->lstJobDayReport->setIgnoreChanges ( true );
	vmListItem* day_entry ( static_cast<vmListItem*> ( ui->lstJobDayReport->currentItem () ) );
	day_entry->setText ( QString::number ( ui->lstJobDayReport->currentRow () + 1 ) + lstJobReportItemPrefix + vmdate.toDate ( vmNumber::VDF_HUMAN_DATE ), false, false, false );
	updateJobInfo ( vmdate.toDate ( vmNumber::VDF_DB_DATE ), JILUR_DATE, day_entry );

	if ( ui->lstJobDayReport->currentRow () == 0 ) // first day
		ui->dteJobStart->setDate ( vmdate, true );
	else if ( ui->lstJobDayReport->currentRow () == (ui->lstJobDayReport->count () - 1) ) // last day
		ui->dteJobEnd->setDate ( vmdate, true );
}

void MainWindow::btnJobDelDay_clicked ()
{
	vmListItem* item ( ui->lstJobDayReport->currentItem () );
	if ( item )
	{
		QFont fntStriked ( ui->lstJobDayReport->font () );
		fntStriked.setStrikeOut ( true );
		item->setFont ( fntStriked );
		item->setBackground ( QBrush ( Qt::red ) );
		item->setData ( JILUR_REMOVED, true );
		ui->btnJobCancelDelDay->setEnabled ( true );
		jobDayReportListWidget_currentItemChanged ( item ); // visual feedback
		if ( (item->row () == ui->lstJobDayReport->rowCount () - 1) && (item->row () > 0) )
		{
			ui->dteJobEnd->setDate ( vmNumber ( ui->lstJobDayReport->item ( item->row () - 1 )->data ( JILUR_DATE ).toString (), VMNT_DATE, vmNumber::VDF_DB_DATE ), true );
		}
		// Remove this day's time from total time
		ui->timeJobStart->setTime ( vmNumber::emptyNumber, false );
		ui->timeJobEnd->setTime ( vmNumber::emptyNumber, false );
		calcJobTime ();
		ui->btnJobDelDay->setEnabled ( false );
		ui->btnJobCancelDelDay->setEnabled ( true );
	}
}

void MainWindow::btnJobCancelDelDay_clicked ()
{
	vmListItem* item ( ui->lstJobDayReport->currentItem () );
	if ( item && item->data ( JILUR_REMOVED ).toBool () == true )
	{
		revertDayItem ( item );
		jobDayReportListWidget_currentItemChanged ( item ); // visual feedback
		calcJobTime (); // uptate job total time
	}
}

void MainWindow::btnJobPrevDay_clicked ()
{
	int current_row ( ui->lstJobDayReport->currentRow () );
	if ( current_row >= 1 )
	{
		ui->lstJobDayReport->setCurrentRow ( --current_row, true );
		ui->btnJobNextDay->setEnabled ( true );
	}
	else
		ui->btnJobPrevDay->setEnabled ( false );
}

void MainWindow::btnJobNextDay_clicked ()
{
	int current_row ( ui->lstJobDayReport->currentRow () );
	if ( current_row < signed( ui->lstJobDayReport->count () - 1 ) )
	{
		ui->lstJobDayReport->setCurrentRow ( ++current_row, true );
		ui->btnJobPrevDay->setEnabled ( true );
	}
	else
		ui->btnJobNextDay->setEnabled ( false );
}

void MainWindow::btnJobMachines_clicked ()
{
	MACHINES ()->showJobMachineUse ( recStrValue ( mJobCurItem->jobRecord (), FLD_JOB_ID ) );
}

void MainWindow::jobOpenProjectFile ( QAction* action )
{
	QString program;
	switch ( static_cast<vmAction*> ( action )->id () )
	{
		case 0: // doc(x)
			program = CONFIG ()->docEditor ();
		break;
		case 1: // pdf
			program = CONFIG ()->universalViewer ();
		break;
		case 2: // xls(x)
			program = CONFIG ()->xlsEditor ();
		break;
	}
	// There is a bug(feature) in Qt that inserts an & to automatically create a mnemonic to the menu entry
	// The bug is that text () returns include it
	const QString filename ( action->text ().remove ( QLatin1Char ( '&' ) ) );
	fileOps::execute ( ui->txtJobProjectPath->text () + filename, program );
}

void MainWindow::jobEMailProjectFile ( QAction* action )
{
	if ( fileOps::exists ( ui->txtJobProjectPath->text () ).isOn () )
	{
		EMAIL_CONFIG ()->sendEMail (
			recStrValue ( mClientCurItem->clientRecord (), FLD_CLIENT_EMAIL ),
			TR_FUNC ( "Project " ),
			fileOps::dirFromPath ( ui->txtJobProjectPath->text () ) + action->text ()
		);
	}
}
//--------------------------------------------JOB------------------------------------------------------------

//-----------------------------------EDITING-FINISHED-JOB----------------------------------------------------
void MainWindow::cboJobType_textAltered ( const vmWidget* const sender )
{
	// Do not use static_cast<uint>(sender->id ()). Could use sender->parentWidget ()->id () but this method
	// is not shared between objects, only cboJobType uses it
	const bool b_ok ( !sender->text ().isEmpty () );
	mJobCurItem->setFieldInputStatus ( FLD_JOB_TYPE, b_ok, sender->vmParent () );
	if ( b_ok )
	{
		setRecValue ( mJobCurItem->jobRecord (), FLD_JOB_TYPE, sender->text () );
		mJobCurItem->update ();
		postFieldAlterationActions ( mJobCurItem );
	}
	ui->btnJobSave->setEnabled ( mJobCurItem->isGoodToSave () );
}

void MainWindow::txtJob_textAltered ( const vmWidget* const sender )
{
	QString new_value ( sender->text () );
	switch ( sender->id () )
	{
		case FLD_JOB_PROJECT_PATH:
			if ( !new_value.endsWith ( CHR_F_SLASH ) )
				 new_value.append ( CHR_F_SLASH );
			setUpJobButtons ( new_value );
			ui->txtJobPicturesPath->setText ( new_value + QLatin1String ( "Pictures" ), true );
		case FLD_JOB_PICTURE_PATH:
			btnJobReloadPictures_clicked ();
		break;
	}
	setRecValue ( mJobCurItem->jobRecord (), static_cast<uint>(static_cast<uint>(sender->id ())), new_value );
	postFieldAlterationActions ( mJobCurItem );
}

void MainWindow::txtJobWheather_textAltered ( const vmWidget* const sender )
{
	updateJobInfo ( sender->text (), JILUR_WHEATHER );
}

void MainWindow::txtJobTotalDayTime_textAltered ( const vmWidget* const sender )
{
	const vmNumber time ( sender->text (), VMNT_TIME );
	if ( time.isTime () ) {
		if ( ui->lstJobDayReport->count () == 1 )
		{
			ui->txtJobTotalAllDaysTime->setText ( sender->text () );
			setRecValue ( mJobCurItem->jobRecord (), static_cast<uint>(sender->id ()), sender->text () );
			postFieldAlterationActions ( mJobCurItem );
		}
		else
		{
			ui->timeJobStart->setTime ( QTime () );
			ui->timeJobEnd->setTime ( time.toQTime (), true );
		}
	}
}

void MainWindow::timeJobStart_timeAltered ()
{
	calcJobTime ();
	updateJobInfo ( vmNumber ( ui->timeJobStart->time () ).toTime (), JILUR_START_TIME ); // save time for the day
}

void MainWindow::timeJobEnd_timeAltered ()
{
	calcJobTime ();
	updateJobInfo ( vmNumber ( ui->timeJobEnd->time () ).toTime (), JILUR_END_TIME );
}

void MainWindow::dteJob_dateAltered ( const vmWidget* const sender )
{
	const vmNumber date ( static_cast<const vmDateEdit* const>( sender )->date () );
	const bool input_ok ( date.isDateWithinRange ( vmNumber::currentDate, 0, 4 ) );
	//if ( input_ok )
		setRecValue ( mJobCurItem->jobRecord (), static_cast<uint>(sender->id ()), date.toDate ( vmNumber::VDF_DB_DATE ) );
	
	if ( static_cast<uint>(sender->id ()) == FLD_JOB_STARTDATE )
	{
		mJobCurItem->setFieldInputStatus ( FLD_JOB_STARTDATE, input_ok, sender );
		ui->btnQuickProject->setEnabled ( input_ok );
	}

	postFieldAlterationActions ( mJobCurItem );
}

void MainWindow::dteJobAddDate_dateAltered ()
{
	const vmNumber vmdate ( ui->dteJobAddDate->date () );
	const triStateType res ( dateIsInDaysList ( vmdate.toDate ( vmNumber::VDF_DB_DATE ) ) );
	ui->btnJobAddDay->setEnabled ( !res.isTrue () );
	ui->btnJobEditDay->setEnabled ( res.isOff () );
}
//-----------------------------------EDITING-FINISHED-JOB----------------------------------------------------

//--------------------------------------------PAY------------------------------------------------------------
void MainWindow::savePayWidget ( vmWidget* widget, const int id )
{
	widget->setID ( id );
	payWidgetList[id] = widget;
}

void MainWindow::showPaySearchResult ( vmListItem* item, const bool bshow )
{
	payListItem* pay_item ( static_cast<payListItem*> ( item ) );
	if ( bshow ) {
		displayClient ( static_cast<clientListItem*> ( item->relatedItem ( RLI_CLIENTPARENT ) ), true,
						static_cast<jobListItem*> ( item->relatedItem ( RLI_JOBPARENT ) ) );
		displayPay ( pay_item, true );
	}
	for ( uint i ( 0 ); i < PAY_FIELD_COUNT; ++i )
	{
		if ( pay_item->searchFieldStatus ( i ) == SS_SEARCH_FOUND )
		{
			if ( i != FLD_PAY_INFO )
				payWidgetList.at ( i )->highlight ( bshow ? vmBlue : vmDefault_Color, SEARCH_UI ()->searchTerm () );
			else
			{
				if ( bshow )
					ui->tablePayments->searchStart ( SEARCH_UI ()->searchTerm () );
				else
					ui->tablePayments->searchCancel ();
			}
		}
	}
}

void MainWindow::setupPayPanel ()
{
	savePayWidget ( ui->txtPayID, FLD_PAY_ID );
	savePayWidget ( ui->txtPayTotalPrice, FLD_PAY_PRICE );
	ui->txtPayTotalPrice->setTextType ( vmLineEdit::TT_PRICE );
	ui->txtPayTotalPrice->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
		return txtPayTotalPrice_textAltered ( sender->text () ); } );
	ui->txtPayTotalPrice->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
		return payKeyPressedSelector ( ke ); } );

	savePayWidget ( ui->txtPayTotalPaid, FLD_PAY_TOTALPAID );
	ui->txtPayTotalPaid->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
		return txtPay_textAltered ( sender ); } );
	ui->txtPayTotalPaid->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
		return payKeyPressedSelector ( ke ); } );

	savePayWidget ( ui->txtPayObs, FLD_PAY_OBS );
	ui->txtPayObs->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
		return txtPay_textAltered ( sender ); } );
	ui->txtPayObs->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
		return payKeyPressedSelector ( ke ); } );

	ui->chkPayOverdue->setLabel ( TR_FUNC ( "Ignore if overdue" ) );
	ui->chkPayOverdue->setCallbackForContentsAltered ( [&] ( const bool checked ) {
		return chkPayOverdue_toggled ( checked ); } );

	ui->paysList->setUtilitiesPlaceLayout ( ui->layoutClientPaysUtility );
	ui->paysList->setAlwaysEmitCurrentItemChanged ( true );
	ui->paysList->setCallbackForCurrentItemChanged ( [&] ( vmListItem* item ) {
		return paysListWidget_currentItemChanged ( item ); } );

	ui->paysOverdueList->setAlwaysEmitCurrentItemChanged ( true );
	ui->paysOverdueList->setCallbackForCurrentItemChanged ( [&] ( vmListItem* item ) {
		return paysOverdueListWidget_currentItemChanged ( item ); } );
	ui->paysOverdueList->setUtilitiesPlaceLayout ( ui->layoutOverduePaysUtility );
	
	ui->paysOverdueClientList->setAlwaysEmitCurrentItemChanged ( true );
	ui->paysOverdueClientList->setCallbackForCurrentItemChanged ( [&] ( vmListItem* item ) {
		return paysOverdueClientListWidget_currentItemChanged ( item ); } );
	ui->paysOverdueClientList->setUtilitiesPlaceLayout ( ui->layoutClientOverduePaysUtility );
	
	static_cast<void>(vmTableWidget::createPayHistoryTable ( ui->tablePayments ));
	savePayWidget ( ui->tablePayments, FLD_PAY_INFO );
	ui->tablePayments->setUtilitiesPlaceLayout ( ui->layoutPayTableUtility );
	ui->tablePayments->setCallbackForCellChanged ( [&] ( const vmTableItem* const item ) {
		return interceptPaymentCellChange ( item ); } );
	ui->tablePayments->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
		return payKeyPressedSelector ( ke ); } );
	
	connect ( ui->tabPaysLists, &QTabWidget::currentChanged, this, [&] ( const int index ) { return tabPaysLists_currentChanged ( index ); } );
	
	connect ( ui->btnPayPayReceipt, &QToolButton::clicked, this, [&] () { return
			EDITOR ()->startNewReport ()->createPaymentStub ( mPayCurItem->dbRecID () ); } );
	connect ( ui->btnPayPaymentsReport, &QToolButton::clicked, this, [&] () { return
			EDITOR ()->startNewReport ()->createJobReport ( mClientCurItem->dbRecID (), true ); } );
	connect ( ui->btnPayPaymentsReportOnlyUnPaid, &QToolButton::clicked, this, [&] () { return
			EDITOR ()->startNewReport ()->createJobReport ( mClientCurItem->dbRecID (), false ); } );
	connect ( ui->btnPayInfoDel, &QToolButton::clicked, this, [&] () { return static_cast<void>(delPay ( mPayCurItem )); } );
	connect ( ui->btnPayInfoEdit, &QToolButton::clicked, this, [&] () { return static_cast<void>(editPay ( mPayCurItem)); } );
	connect ( ui->btnPayInfoSave, &QToolButton::clicked, this, [&] () { return static_cast<void>(savePay ( mPayCurItem )); } );
	connect ( ui->btnPayInfoCancel, &QToolButton::clicked, this, [&] () { return static_cast<void>(cancelPay ( mPayCurItem)); } );
}

void MainWindow::displayPayFromCalendar ( vmListItem* cal_item )
{
	clientListItem* client_item ( static_cast<clientListItem*>( cal_item->relatedItem ( RLI_CLIENTPARENT ) ) );
	if ( client_item )
	{
		if ( client_item != mClientCurItem )
			displayClient ( client_item, true, static_cast<jobListItem*>( cal_item->relatedItem ( RLI_JOBPARENT ) ) );
		else
			displayJob ( static_cast<jobListItem*>( cal_item->relatedItem ( RLI_JOBPARENT ) ), true );
		
		ui->tablePayments->setCurrentCell ( cal_item->data ( Qt::UserRole ).toInt () - 1, 0 );
	}
}

void MainWindow::paysListWidget_currentItemChanged ( vmListItem* item )
{
	if ( item != mPayCurItem )
	{
		insertNavItem ( item );
		if ( static_cast<jobListItem*>( item->relatedItem ( RLI_JOBPARENT )) != mJobCurItem )
			displayJob ( static_cast<jobListItem*>( item->relatedItem ( RLI_JOBPARENT )), true );
	}
}

void MainWindow::paysOverdueListWidget_currentItemChanged ( vmListItem* item )
{
	if ( item->relatedItem ( RLI_CLIENTITEM ) != mPayCurItem )
	{
		payListItem* pay_item ( static_cast<payListItem*> ( item ) );
		/* displayClient will ignore a chunk of calls if pay_item->client_parent == mClientCurItem, which it is and supposed to.
		* Force reselection of current client
		*/
		mClientCurItem = nullptr;
		insertNavItem ( item->relatedItem ( RLI_CLIENTITEM ) );
		displayClient ( static_cast<clientListItem*> ( pay_item->relatedItem ( RLI_CLIENTPARENT ) ), true,
			static_cast<jobListItem*> ( pay_item->relatedItem ( RLI_JOBPARENT ) ) );
	}
}

void MainWindow::paysOverdueClientListWidget_currentItemChanged ( vmListItem* item )
{
	if ( item->relatedItem ( RLI_CLIENTITEM ) != mPayCurItem )
	{
		insertNavItem ( item->relatedItem ( RLI_CLIENTITEM ) );
		displayJob ( static_cast<jobListItem*>(item->relatedItem ( RLI_JOBPARENT )), true );
	}
}

void MainWindow::addPaymentOverdueItems ( payListItem* pay_item )
{
	bool bOldState ( true );
	if ( ui->tabPaysLists->currentIndex () == 1 )
	{
		bOldState = ui->paysOverdueClientList->isIgnoringChanges ();
		ui->paysOverdueClientList->setIgnoreChanges ( true );
		payListItem* pay_item_overdue_client ( new payListItem );
		pay_item_overdue_client->setRelation ( PAY_ITEM_OVERDUE_CLIENT );
		pay_item->syncSiblingWithThis ( pay_item_overdue_client );
		pay_item_overdue_client->update ();
		pay_item_overdue_client->addToList ( ui->paysOverdueClientList );
		ui->paysOverdueClientList->setIgnoreChanges ( bOldState );
	}
	
	if ( ui->paysOverdueList->count () != 0 )
	{
		bOldState = ui->paysOverdueList->isIgnoringChanges ();
		ui->paysOverdueList->setIgnoreChanges ( true );
		payListItem* pay_item_overdue ( new payListItem );
		pay_item_overdue->setRelation ( PAY_ITEM_OVERDUE_ALL );
		pay_item->syncSiblingWithThis ( pay_item_overdue );
		pay_item_overdue->update ();
		pay_item_overdue->addToList ( ui->paysOverdueList );
		ui->paysOverdueList->setIgnoreChanges ( bOldState );
	}
}

void MainWindow::removePaymentOverdueItems ( payListItem* pay_item )
{
	if ( pay_item->relatedItem ( PAY_ITEM_OVERDUE_CLIENT ) )
	{
		ui->paysOverdueClientList->removeRow ( static_cast<uint>( pay_item->relatedItem ( PAY_ITEM_OVERDUE_CLIENT )->row () ), 1, true );
	}
	
	if ( pay_item->relatedItem ( PAY_ITEM_OVERDUE_ALL ) )
	{
		ui->paysOverdueList->removeRow ( static_cast<uint>( pay_item->relatedItem ( PAY_ITEM_OVERDUE_ALL )->row () ), 1, true );
	}
}

void MainWindow::updateTotalPayValue ( const vmNumber& nAdd, const vmNumber& nSub )
{
	vmNumber totalPrice;
	totalPrice.fromTrustedStrPrice ( ui->paysList->property ( PROPERTY_TOTAL_PAYS ).toString (), false );
	totalPrice -= nSub;
	totalPrice += nAdd;
	ui->paysList->setProperty ( PROPERTY_TOTAL_PAYS, totalPrice.toPrice () );
	if ( ui->tabPaysLists->currentIndex () == 0 )
		ui->txtClientPayTotals->setText ( totalPrice.toPrice () );	
}

void MainWindow::updatePayOverdueTotals ( const vmNumber& nAdd, const vmNumber& nSub )
{
	vmNumber totalPrice;
	if ( ui->paysOverdueClientList->count () != 0 )
	{
		totalPrice.fromTrustedStrPrice ( ui->paysOverdueClientList->property ( PROPERTY_TOTAL_PAYS ).toString (), false );
		totalPrice -= nSub;
		totalPrice += nAdd;
		ui->paysOverdueClientList->setProperty ( PROPERTY_TOTAL_PAYS, totalPrice.toPrice () );
		ui->txtClientPayOverdueTotals->setText ( totalPrice.toPrice () );
	}
	if ( ui->paysOverdueList->count () != 0 )
	{
		totalPrice.fromTrustedStrPrice ( ui->paysOverdueList->property ( PROPERTY_TOTAL_PAYS ).toString (), false );
		totalPrice -= nSub;
		totalPrice += nAdd;
		ui->paysOverdueList->setProperty ( PROPERTY_TOTAL_PAYS, totalPrice.toPrice () );
		ui->txtAllOverdueTotals->setText ( totalPrice.toPrice () );
	}
}

void MainWindow::payOverdueGUIActions ( Payment* const pay, const RECORD_ACTION new_action )
{
	bool bIsOverdue ( false );
	if ( pay )
	{
		const int overdueIdx ( recStrValue ( pay, FLD_PAY_OVERDUE ).toInt () );
		if ( new_action != ACTION_NONE )
		{
			vmNumber new_price, old_price;
			vmNumber new_paid, old_paid;
			if ( overdueIdx == 2 && new_action == ACTION_EDIT )
				*(const_cast<RECORD_ACTION*>(&new_action)) = ACTION_DEL;
			switch ( new_action )
			{
				case ACTION_EDIT:
					new_price.fromTrustedStrPrice ( ui->txtPayTotalPrice->text (), false );
					old_price.fromTrustedStrPrice ( pay->backupRecordStr ( FLD_PAY_PRICE ), false );
					new_paid.fromTrustedStrPrice ( ui->txtPayTotalPaid->text (), false );
					old_paid.fromTrustedStrPrice ( pay->backupRecordStr ( FLD_PAY_TOTALPAID ), false );
				break;
				case ACTION_REVERT:
				case ACTION_READ:
					new_price.fromTrustedStrPrice ( pay->actualRecordStr ( FLD_PAY_PRICE ), false );
					old_price.fromTrustedStrPrice ( pay->backupRecordStr ( FLD_PAY_PRICE ), false );
					new_paid.fromTrustedStrPrice ( pay->actualRecordStr ( FLD_PAY_TOTALPAID ), false );
					old_paid.fromTrustedStrPrice ( pay->backupRecordStr ( FLD_PAY_TOTALPAID ), false );
				break;
				case ACTION_ADD:
					new_price.fromTrustedStrPrice ( pay->actualRecordStr( FLD_PAY_PRICE ), false );
					old_price = vmNumber::zeroedPrice;
					new_paid.fromTrustedStrPrice ( pay->actualRecordStr( FLD_PAY_TOTALPAID ), false );
					old_paid = vmNumber::zeroedPrice;
				break;
				case ACTION_DEL:
					new_price = vmNumber::zeroedPrice;
					old_price.fromTrustedStrPrice ( recStrValue ( pay, FLD_PAY_PRICE ) );
					new_paid = vmNumber::zeroedPrice;
					old_paid.fromTrustedStrPrice ( recStrValue ( pay, FLD_PAY_TOTALPAID ) );
				break;
				case ACTION_NONE: break;
			}
			
			bIsOverdue = new_paid < new_price;
			if ( bIsOverdue && overdueIdx == 0 )
				setRecValue ( pay, FLD_PAY_OVERDUE, CHR_ONE );
			else if ( !bIsOverdue && overdueIdx == 1 )
				setRecValue ( pay, FLD_PAY_OVERDUE, CHR_ZERO );
			
			setRecValue ( pay, FLD_PAY_OVERDUE_VALUE, ( new_price - new_paid ).toPrice () );
			updatePayOverdueTotals ( new_price - new_paid, old_price - old_paid );
			bIsOverdue &= overdueIdx != 2;
		}
		else //reading record. Only update the one control
		{
			ui->chkPayOverdue->setChecked ( overdueIdx == 2 );
			bIsOverdue = ( overdueIdx == 1 );
		}
		ui->chkPayOverdue->setToolTip ( chkPayOverdueToolTip_overdue[overdueIdx] );
		pay->payItem ()->update ();
	}
	else
	{
		ui->chkPayOverdue->setToolTip (	TR_FUNC ( "Check this box to signal that payment overdue is to be ignored." ) );
		ui->chkPayOverdue->setChecked ( false );
	}
	ui->chkPayOverdue->highlight ( bIsOverdue ? vmYellow : vmDefault_Color );
	ui->txtPayTotalPrice->highlight ( bIsOverdue ? vmYellow : vmDefault_Color );
}

void MainWindow::controlPayForms ( const payListItem* const pay_item )
{
	const triStateType editing_action ( pay_item ? static_cast<TRI_STATE> ( pay_item->action () == ACTION_EDIT ) : TRI_UNDEF );

	//Make tables editable only when editing a record, because in such situations there is no clearing and loading involved, but a mere display change
	ui->tablePayments->setEditable ( editing_action.isOn () );

	ui->txtPayObs->setEditable ( editing_action.isOn () );
	ui->txtPayTotalPrice->setEditable ( editing_action.isOn () );
	ui->chkPayOverdue->setEditable ( editing_action.isOn () );

	ui->btnPayPaymentsReport->setEnabled ( ui->paysList->count () > 0 );
	ui->btnPayPaymentsReportOnlyUnPaid->setEnabled ( ui->paysList->count () > 0 );
	ui->btnPayPayReceipt->setEnabled ( pay_item != nullptr );

	ui->btnPayInfoSave->setEnabled ( editing_action.isOn () ? pay_item->isGoodToSave () : false );
	ui->btnPayInfoEdit->setEnabled ( editing_action.isOff () );
	ui->btnPayInfoCancel->setEnabled ( editing_action.isOn () );
	ui->btnPayInfoDel->setEnabled ( editing_action.isOn () );

	const vmNumber paid_price ( ui->txtPayTotalPaid->text (), VMNT_PRICE, 1 );
	const vmNumber total_price ( ui->txtPayTotalPrice->text (), VMNT_PRICE, 1 );
	ui->txtPayTotalPaid->highlight ( paid_price < total_price ? vmYellow : vmDefault_Color );

	ui->tablePayments->scrollTo ( ui->tablePayments->indexAt ( QPoint ( 0, 0 ) ) );
}

bool MainWindow::savePay ( payListItem* pay_item )
{
	if ( pay_item != nullptr )
	{
		Payment* pay ( pay_item->payRecord () );
		const bool bWasOverdue ( pay->actualRecordStr ( FLD_PAY_OVERDUE ) == CHR_ONE );
		
		if ( pay->saveRecord ( false ) ) // do not change indexes just now. Wait for dbCalendar actions
		{	
			if ( pay->wasModified ( FLD_PAY_OVERDUE ) )
			{
				const bool bIsOverdue ( recStrValue ( pay, FLD_PAY_OVERDUE ) == CHR_ONE );
				if ( bIsOverdue && !bWasOverdue )
					addPaymentOverdueItems ( pay_item );
				else if ( !bIsOverdue && bWasOverdue )
					removePaymentOverdueItems ( pay_item );
			}
			if ( ui->tablePayments->tableChanged () )
			{
				mCal->updateCalendarWithPayInfo ( pay );
				ui->tablePayments->setTableUpdated ();
			}
			pay_item->setAction ( ACTION_READ, true );
			VM_NOTIFY ()->notifyMessage ( TR_FUNC ( "Record saved" ), TR_FUNC ( "Payment data saved!" ) );
			controlPayForms ( pay_item );
			removeEditItem ( static_cast<vmListItem*>(pay_item) );
			return true;
		}
	}
	return false;
}

bool MainWindow::editPay ( payListItem* pay_item, const bool b_dogui )
{
	if ( pay_item )
	{
		if ( pay_item->action () == ACTION_READ )
		{
			pay_item->setAction ( ACTION_EDIT, true );
			insertEditItem ( static_cast<vmListItem*>(pay_item) );
			if ( b_dogui )
			{
				controlPayForms ( pay_item );
				ui->tablePayments->setFocus ();
			}
			return true;
		}
	}
	return false;
}

bool MainWindow::delPay ( payListItem* pay_item )
{
	if ( pay_item )
	{
		const uint row ( ui->tablePayments->currentRow () == -1 ? 0 : static_cast<uint> ( ui->tablePayments->currentRow () ) );
		ui->tableBuyPayments->removeRow ( row );
		mCal->updateCalendarWithPayInfo ( pay_item->payRecord () );
		return true;
	}
	return false;
}

bool MainWindow::cancelPay ( payListItem* pay_item )
{
	if ( pay_item )
	{
		if ( pay_item->action () == ACTION_EDIT )
		{
			removeEditItem ( static_cast<vmListItem*>(pay_item) );
			payOverdueGUIActions ( pay_item->payRecord (), ACTION_REVERT );
			pay_item->setAction ( ACTION_REVERT, true );
			displayPay ( pay_item );
			return true;
		}
	}
	return false;
}

void MainWindow::displayPay ( payListItem* pay_item, const bool b_select )
{
	if ( pay_item )
	{
		if ( pay_item->loadData () )
		{
			ui->tabMain->setCurrentIndex ( 0 );
			controlPayForms ( pay_item );
			loadPayInfo ( pay_item->payRecord () );
			if ( pay_item != mPayCurItem )
			{
				if ( b_select )
					ui->paysList->setCurrentItem ( pay_item, false );
				mPayCurItem = pay_item;
				ui->paysOverdueClientList->setCurrentItem ( mPayCurItem->relatedItem ( PAY_ITEM_OVERDUE_CLIENT ), false );
				ui->paysOverdueList->setCurrentItem ( mPayCurItem->relatedItem ( PAY_ITEM_OVERDUE_ALL ), false );
			}
		}
	}
	else
	{
		mPayCurItem = nullptr;
		loadPayInfo ( nullptr );
		controlPayForms ( nullptr );
	}
	if ( b_select )
		saveView ();
}

void MainWindow::loadPayInfo ( const Payment* const pay )
{
	if ( pay != nullptr )
	{
		// See comments under loadBuyInfo for the rationale of a table's state change calls
		ui->tablePayments->setEditable ( false );
		ui->tablePayments->clear ( true );

		ui->txtPayID->setText ( recStrValue ( pay, FLD_PAY_ID ) );
		ui->txtPayTotalPrice->setText ( pay->action () != ACTION_ADD ? recStrValue ( pay, FLD_PAY_PRICE ) :
					( mJobCurItem ? recStrValue ( mJobCurItem->jobRecord (), FLD_JOB_PRICE ) :
						  vmNumber::zeroedPrice.toPrice () ) );
		ui->txtPayTotalPaid->setText ( recStrValue ( pay, FLD_PAY_TOTALPAID ) );
		ui->txtPayObs->setText ( recStrValue ( pay, FLD_PAY_OBS ) );
		ui->tablePayments->loadFromStringTable ( recStrValue ( pay, FLD_PAY_INFO ) );

		payOverdueGUIActions ( const_cast<Payment*>( pay ), ACTION_NONE );
	}
	else
	{
		ui->tablePayments->setEditable ( false );
		ui->tablePayments->clear ( true );
		ui->txtPayID->setText ( QStringLiteral ( "-1" ) );
		ui->txtPayTotalPrice->setText ( vmNumber::zeroedPrice.toPrice () );
		ui->txtPayObs->setText ( QStringLiteral ( "N/A" ) );
		ui->txtPayTotalPaid->setText ( vmNumber::zeroedPrice.toPrice () );
	}
}

payListItem* MainWindow::getPayItem ( const clientListItem* const parent_client, const uint id ) const
{
	if ( id >= 1 && parent_client )
	{
		int i ( 0 );
		while ( i < static_cast<int> ( parent_client->pays->count () ) )
		{
			if ( parent_client->pays->at ( i )->dbRecID () == id )
				return parent_client->pays->at ( i );
			++i;
		}
	}
	return nullptr;
}

void MainWindow::fillCalendarPaysList ( const stringTable& payids, vmListWidget* list, const bool use_date )
{
	if ( payids.countRecords () > 0 )
	{
		const stringRecord* str_rec ( &payids.first () );
		if ( str_rec->isOK () )
		{
			uint payid ( 0 );
			const QLatin1String payStr[2] = { QLatin1String( " Payment # " ), QLatin1String ( "Pay use # " ) };
			payListItem* pay_item ( nullptr ), *pay_parent ( nullptr );
			clientListItem* client_parent ( nullptr );
			Payment pay;
			QString paynumber, curLabel;
			list->setIgnoreChanges ( true );
			do
			{
				payid =  str_rec->fieldValue ( 0 ).toUInt () ;
				if ( pay.readRecord ( payid ) )
				{
					client_parent = getClientItem ( static_cast<uint>(recIntValue ( &pay, FLD_PAY_CLIENTID )) );
					pay_parent = getPayItem ( client_parent, static_cast<uint>(recIntValue ( &pay, FLD_PAY_ID )) );
					
					if ( pay_parent )
					{
						paynumber = str_rec->fieldValue ( 2 );
						if ( paynumber.isEmpty () )
							paynumber = CHR_ONE;
						pay_item = static_cast<payListItem*>( pay_parent->relatedItem ( RLI_CALENDARITEM ) );
						if ( !pay_item )
						{
							pay_item = new payListItem;
							pay_item->setRelation ( RLI_CALENDARITEM );
							pay_parent->syncSiblingWithThis ( pay_item );
						}
						
						pay_item->setData ( Qt::UserRole, paynumber.toInt () );
						if ( pay_item->listWidget () != list )
						{
							pay_item->setText ( recStrValue ( client_parent->clientRecord (), FLD_CLIENT_NAME ) +
								CHR_L_PARENTHESIS +  recStrValue ( &pay, FLD_PAY_TOTALPAID ) + CHR_R_PARENTHESIS +
								CHR_SPACE + CHR_HYPHEN + CHR_SPACE + payStr[static_cast<int>( !use_date )] + paynumber, false, false, false );
							pay_item->addToList ( list );
						}
						else
						{
							curLabel = pay_item->text ();
							curLabel.insert ( curLabel.count () - 1, paynumber );
							pay_item->setText ( curLabel, false, false, false );
						}
					}
				}
				str_rec = &payids.next ();
			} while ( str_rec->isOK () );
			list->setIgnoreChanges ( false );
		}
	}
}

void MainWindow::loadClientOverduesList ()
{
	if ( !mClientCurItem ) return;

	if ( ui->paysOverdueClientList->count () != 0 )
	{
		if ( static_cast<clientListItem*> ( static_cast<payListItem*>( ui->paysOverdueClientList->item ( 0 ) )->relatedItem ( RLI_CLIENTPARENT ) ) == mClientCurItem )
			return;
		else
			ui->paysOverdueClientList->clear ();
	}
	ui->paysOverdueClientList->setIgnoreChanges ( true );
	payListItem* pay_item_overdue_client ( nullptr ), *pay_item ( nullptr );
	vmNumber totalOverdue;
	for ( uint x ( 0 ); x < mClientCurItem->pays->count (); ++x )
	{
		pay_item = mClientCurItem->pays->at ( x );
		pay_item_overdue_client = static_cast<payListItem*>( pay_item->relatedItem ( PAY_ITEM_OVERDUE_CLIENT ) );
		if ( !pay_item_overdue_client )
		{
			if ( pay_item->loadData () )
			{
				if ( recStrValue ( pay_item->payRecord (), FLD_PAY_OVERDUE ) == CHR_ONE )
				{
					pay_item_overdue_client = new payListItem;
					pay_item_overdue_client->setRelation ( PAY_ITEM_OVERDUE_CLIENT );
					pay_item->syncSiblingWithThis ( pay_item_overdue_client );
					pay_item_overdue_client->update ();
					pay_item_overdue_client->addToList ( ui->paysOverdueClientList );
					totalOverdue += pay_item->payRecord ()->price ( FLD_PAY_OVERDUE_VALUE );
				}
				else
					continue;
			}
			else
				continue;
		}
		pay_item_overdue_client->update ();
		ui->paysOverdueClientList->setProperty ( PROPERTY_TOTAL_PAYS, totalOverdue.toPrice () );
		ui->txtClientPayOverdueTotals->setText ( totalOverdue.toPrice () );
	}
}

void MainWindow::loadAllOverduesList ()
{
	if ( ui->paysOverdueList->count () == 0 )
	{
		payListItem* pay_item_overdue ( nullptr ), *pay_item ( nullptr );
		clientListItem* client_item ( nullptr );
		const int n ( ui->clientsList->count () );
		vmNumber totalOverdue;
		for ( int i ( 0 ); i < n; ++i )
		{
			client_item = static_cast<clientListItem*> ( ui->clientsList->item ( i ) );
			for ( uint x ( 0 ); x < client_item->pays->count (); ++x )
			{
				pay_item = client_item->pays->at ( x );
				if ( pay_item->loadData () )
				{
					if ( recStrValue ( pay_item->payRecord (), FLD_PAY_OVERDUE ) == CHR_ONE ) 
					{
						pay_item_overdue = new payListItem;
						pay_item_overdue->setRelation ( PAY_ITEM_OVERDUE_ALL );
						pay_item->syncSiblingWithThis ( pay_item_overdue );
						pay_item_overdue->update ();
						pay_item_overdue->addToList ( ui->paysOverdueList );
						totalOverdue += pay_item->payRecord ()->price ( FLD_PAY_OVERDUE_VALUE );
					}
				}
			}
		}
		ui->paysOverdueList->setProperty ( PROPERTY_TOTAL_PAYS, totalOverdue.toPrice () );
		ui->txtAllOverdueTotals->setText ( totalOverdue.toPrice () );
	}
}

void MainWindow::interceptPaymentCellChange ( const vmTableItem* const item )
{
	const uint row ( static_cast<uint>( item->row () ) );
	if ( static_cast<int>(row) == ui->tablePayments->totalsRow () )
		return;

	const uint col ( static_cast<uint>( item->column () ) );
	const bool b_paid ( ui->tablePayments->sheetItem ( row, PHR_PAID )->text () == CHR_ONE );
	bool bSetUseDate ( false );

	triStateType input_ok;
	switch ( col )
	{
		case PHR_DATE:
			bSetUseDate = true;
		case PHR_USE_DATE:
		{
			input_ok = ui->tablePayments->isRowEmpty ( row );
			if ( input_ok.isOff () )
			{
				const vmNumber cell_date ( item->date () );
				input_ok = cell_date.isDateWithinRange ( vmNumber::currentDate, 0, 4 );
			}
			mPayCurItem->setFieldInputStatus ( col + INFO_TABLE_COLUMNS_OFFSET, input_ok.isOn (), item );
		}
		break;
		case PHR_PAID:
		case PHR_VALUE:
			updatePayTotalPaidValue ();
			ui->btnPayPayReceipt->setEnabled ( b_paid );
			if ( b_paid && !ui->tablePayments->sheetItem ( row, PHR_DATE )->cellIsAltered () )
				ui->tablePayments->sheetItem ( row, PHR_DATE )->setDate ( vmNumber::currentDate );

			input_ok = ( !b_paid == ui->tablePayments->sheetItem ( row, PHR_VALUE )->number ().isNull () );
			mPayCurItem->setFieldInputStatus ( PHR_VALUE + INFO_TABLE_COLUMNS_OFFSET, input_ok.isOn (), ui->tablePayments->sheetItem ( row, PHR_VALUE ) );

			if ( col == PHR_PAID )
			{
				// dates in the future are ok. Probably the payment is scheduled to happen at some future date
				if ( ui->tablePayments->sheetItem ( row, PHR_DATE )->date () <= vmNumber::currentDate )
				{
					input_ok = ( b_paid == !( ui->tablePayments->sheetItem ( row, PHR_METHOD )->text ().isEmpty () ) );
					mPayCurItem->setFieldInputStatus ( PHR_METHOD + INFO_TABLE_COLUMNS_OFFSET, input_ok.isOn (), ui->tablePayments->sheetItem ( row, PHR_METHOD ) );
					input_ok = ( b_paid == !( ui->tablePayments->sheetItem ( row, PHR_ACCOUNT )->text ().isEmpty () ) );
					mPayCurItem->setFieldInputStatus ( PHR_ACCOUNT + INFO_TABLE_COLUMNS_OFFSET, input_ok.isOn (), ui->tablePayments->sheetItem ( row, PHR_ACCOUNT ) );
				}
			}
		break;
		case PHR_METHOD:
			bSetUseDate = true;
		case PHR_ACCOUNT:
			input_ok = ( b_paid == !( ui->tablePayments->sheetItem ( row, col )->text ().isEmpty () ) );
			mPayCurItem->setFieldInputStatus ( col + INFO_TABLE_COLUMNS_OFFSET, input_ok.isOn (), item );
		break;
		default:
		break;
	}

	if ( bSetUseDate )
	{
		const QString strMethod ( ui->tablePayments->sheetItem ( row, PHR_METHOD )->text () );
		if ( strMethod == QStringLiteral ( "TED" ) || strMethod == QStringLiteral ( "DOC" ) || 
			 strMethod.contains ( QStringLiteral ( "Transferência" ), Qt::CaseInsensitive ) )
		{
			 ui->tablePayments->sheetItem ( row, PHR_USE_DATE )->setDate ( 
								ui->tablePayments->sheetItem ( row, PHR_DATE )->date () );
		}
	}
	
	//if ( input_ok.isOn () )
	//{
		if ( ( row + 1 ) > recStrValue ( mPayCurItem->payRecord (), FLD_PAY_TOTALPAYS ).toUInt () )
			 setRecValue ( mPayCurItem->payRecord (), FLD_PAY_TOTALPAYS, QString::number ( row + 1 ) );
		
		stringTable pay_data;
		ui->tablePayments->makeStringTable ( pay_data );

		switch ( col )
		{
			case PHR_METHOD:
				APP_COMPLETERS ()->updateCompleter ( item->text (), vmCompleters::PAYMENT_METHOD );
			break;
			case PHR_ACCOUNT:
				APP_COMPLETERS ()->updateCompleter ( item->text (), vmCompleters::ACCOUNT );
			break;
			default:
			break;
		}
		
		setRecValue ( mPayCurItem->payRecord (), FLD_PAY_INFO, pay_data.toString () );
	//}
	postFieldAlterationActions ( mPayCurItem );
}

void MainWindow::updatePayTotalPaidValue ()
{
	vmNumber total_paid;
	for ( int i ( 0 ); i <= ui->tablePayments->lastUsedRow (); ++i )
	{
		if ( ui->tablePayments->sheetItem ( static_cast<uint>(i), PHR_PAID )->text () == CHR_ONE )
			total_paid += vmNumber ( ui->tablePayments->sheetItem ( static_cast<uint>(i), PHR_VALUE )->text (), VMNT_PRICE );
	}
	ui->txtPayTotalPaid->setText ( total_paid.toPrice () ); // do not notify text change so that payOverdueGUIActions can use the current stores paid value
	payOverdueGUIActions ( mPayCurItem->payRecord (), ACTION_EDIT );
	setRecValue ( mPayCurItem->payRecord (), FLD_PAY_TOTALPAID, total_paid.toPrice () ); // now we can update the record
}

void MainWindow::payKeyPressedSelector ( const QKeyEvent* ke )
{
	if ( ke->key () == Qt::Key_Escape )
		static_cast<void>(cancelPay ( mPayCurItem ));
	else
	{
		ui->clientsList->setFocus (); // trigger any pending editing_Finished or textAltered event
		static_cast<void>(savePay ( mPayCurItem ));
	}
}

// Delay loading lists until they are necessary
void MainWindow::tabPaysLists_currentChanged ( const int index )
{
	switch ( index )
	{
		case 0: break;
		case 1: loadClientOverduesList (); break;
		case 2:	loadAllOverduesList ();	break;
	}
	ui->paysOverdueClientList->setIgnoreChanges ( index != 1 );
	ui->paysOverdueList->setIgnoreChanges ( index != 2 );
}
//--------------------------------------------PAY------------------------------------------------------------

//-------------------------------------EDITING-FINISHED-PAY--------------------------------------------------
void MainWindow::txtPayTotalPrice_textAltered ( const QString& text )
{
	const vmNumber new_price ( text, VMNT_PRICE, 1 );
	payOverdueGUIActions ( mPayCurItem->payRecord (), mPayCurItem->payRecord ()->action () );
	updateTotalPayValue ( new_price, vmNumber ( recStrValue ( mPayCurItem->payRecord (), FLD_PAY_PRICE ), VMNT_PRICE, 1 ) );

	bool input_ok ( true );
	if ( static_cast<jobListItem*> ( mPayCurItem->relatedItem ( RLI_JOBPARENT ) )->jobRecord ()->price ( FLD_JOB_PRICE ).isNull () )
		input_ok = !new_price.isNull ();
	else
	{
		ui->txtPayTotalPrice->setText ( new_price.toPrice () );
		setRecValue ( mPayCurItem->payRecord (), FLD_PAY_PRICE, new_price.toPrice () );
	}
	mPayCurItem->setFieldInputStatus ( FLD_PAY_PRICE, input_ok, ui->txtPayTotalPrice );
	postFieldAlterationActions ( mPayCurItem );
}

void MainWindow::txtPay_textAltered ( const vmWidget* const sender )
{
	setRecValue ( mPayCurItem->payRecord (), static_cast<uint>(sender->id ()), sender->text () );
	/* No need to call postFieldAlterationActions because the controls bound to the other fields
	 * are readonly and that function will be called from the table change callback */
	if ( static_cast<uint>(sender->id ()) == FLD_PAY_OBS )
		postFieldAlterationActions ( mPayCurItem );
}

void MainWindow::chkPayOverdue_toggled ( const bool checked )
{
	const QString* chr ( &CHR_ZERO );
	if ( checked )
		chr = &CHR_TWO; //ignore price differences
	else
	{
		const vmNumber paid_price ( mPayCurItem->payRecord ()->price ( FLD_PAY_TOTALPAID ) );
		if ( paid_price < mPayCurItem->payRecord ()->price ( FLD_PAY_PRICE ) )
			chr = &CHR_ONE;
	}
	setRecValue ( mPayCurItem->payRecord (), FLD_PAY_OVERDUE, *chr );
	payOverdueGUIActions ( mPayCurItem->payRecord (), mPayCurItem->payRecord ()->action () );
	postFieldAlterationActions ( mPayCurItem );
}
//--------------------------------------------EDITING-FINISHED-PAY-----------------------------------------------------------

//--------------------------------------------BUY------------------------------------------------------------
void MainWindow::saveBuyWidget ( vmWidget* widget, const int id )
{
	widget->setID ( id );
	buyWidgetList[id] = widget;
}

void MainWindow::showBuySearchResult ( vmListItem* item, const bool bshow )
{
	buyListItem* buy_item ( static_cast<buyListItem*> ( item ) );
	if ( bshow )
	{
		displayClient ( static_cast<clientListItem*> ( buy_item->relatedItem ( RLI_CLIENTPARENT ) ), true,
						static_cast<jobListItem*> ( buy_item->relatedItem ( RLI_JOBPARENT ) ),
						buy_item );
	}

	for ( uint i ( 0 ); i < BUY_FIELD_COUNT; ++i )
	{
		if ( buy_item->searchFieldStatus ( i ) == SS_SEARCH_FOUND )
		{
			if ( i == FLD_BUY_PAYINFO )
			{
				if ( bshow )
					ui->tableBuyPayments->searchStart ( SEARCH_UI ()->searchTerm () );
				else
					ui->tableBuyPayments->searchCancel ();
			}
			else if ( i == FLD_BUY_REPORT )
			{
				if ( bshow)
					ui->tableBuyItems->searchStart ( SEARCH_UI ()->searchTerm () );
				else
					ui->tableBuyItems->searchCancel ();
			}
			else
				buyWidgetList.at ( i )->highlight ( bshow ? vmBlue : vmDefault_Color, SEARCH_UI ()->searchTerm () );
		}
	}
}

void MainWindow::setupBuyPanel ()
{
	ui->buysList->setUtilitiesPlaceLayout ( ui->layoutBuysListUtility );
	ui->buysList->setCallbackForCurrentItemChanged ( [&] ( vmListItem* item ) {
		return buysListWidget_currentItemChanged ( item ); } );
	
	ui->buysJobList->setUtilitiesPlaceLayout ( ui->layoutJobBuysListUtility );
	ui->buysJobList->setCallbackForCurrentItemChanged ( [&] ( vmListItem* item ) {
		return buysJobListWidget_currentItemChanged ( item ); } );
	
	ui->lstBuySuppliers->setUtilitiesPlaceLayout ( ui->layoutSuppliersUtility );
	ui->lstBuySuppliers->setCallbackForCurrentItemChanged ( [&] ( vmListItem* item ) {
		return buySuppliersListWidget_currentItemChanged ( item ); } );

	saveBuyWidget ( ui->txtBuyID, FLD_BUY_ID );
	saveBuyWidget ( ui->txtBuyTotalPrice, FLD_BUY_PRICE );
	ui->txtBuyTotalPrice->setTextType ( vmLineEdit::TT_PRICE );
	ui->txtBuyTotalPrice->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
		return txtBuy_textAltered ( sender ); } );
	ui->txtBuyTotalPrice->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
		return buyKeyPressedSelector ( ke ); } );

	saveBuyWidget ( ui->txtBuyTotalPaid->lineControl (), FLD_BUY_TOTALPAID );
	ui->txtBuyTotalPaid->lineControl ()->setTextType ( vmLineEdit::TT_PRICE );
	ui->txtBuyTotalPaid->setButtonType ( vmLineEditWithButton::LEBT_CALC_BUTTON );
	ui->txtBuyTotalPaid->lineControl ()->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
		return txtBuy_textAltered ( sender ); } );
	ui->txtBuyTotalPaid->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
		return buyKeyPressedSelector ( ke ); } );

	saveBuyWidget ( ui->txtBuyNotes, FLD_BUY_NOTES );
	ui->txtBuyNotes->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
		return txtBuy_textAltered ( sender ); } );
	ui->txtBuyNotes->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
		return buyKeyPressedSelector ( ke ); } );

	saveBuyWidget ( ui->txtBuyDeliveryMethod, FLD_BUY_DELIVERMETHOD );
	ui->txtBuyDeliveryMethod->setCompleter ( APP_COMPLETERS ()->getCompleter ( vmCompleters::DELIVERY_METHOD ) );
	ui->txtBuyDeliveryMethod->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
		return txtBuy_textAltered ( sender ); } );
	ui->txtBuyDeliveryMethod->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
		return buyKeyPressedSelector ( ke ); } );

	saveBuyWidget ( ui->dteBuyDate, FLD_BUY_DATE );
	ui->dteBuyDate->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
		return dteBuy_dateAltered ( sender ); } );
	ui->dteBuyDate->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
		return buyKeyPressedSelector ( ke ); } );
	saveBuyWidget ( ui->dteBuyDeliveryDate, FLD_BUY_DELIVERDATE );
	ui->dteBuyDeliveryDate->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
		return dteBuy_dateAltered ( sender ); } );
	ui->dteBuyDeliveryDate->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
		return buyKeyPressedSelector ( ke ); } );

	saveBuyWidget ( ui->cboBuySuppliers, FLD_BUY_SUPPLIER );
	ui->cboBuySuppliers->setCompleter ( vmCompleters::SUPPLIER );
	ui->cboBuySuppliers->setIgnoreChanges ( false );
	ui->cboBuySuppliers->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
		return cboBuySuppliers_textAltered ( sender->text () ); } );
	ui->cboBuySuppliers->setCallbackForIndexChanged ( [&] ( const int index ) {
		return cboBuySuppliers_indexChanged ( index ); } );
	ui->cboBuySuppliers->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
		return buyKeyPressedSelector ( ke ); } );

	( void ) vmTableWidget::createPurchasesTable ( ui->tableBuyItems );
	ui->tableBuyItems->setKeepModificationRecords ( false );
	saveBuyWidget ( ui->tableBuyItems, FLD_BUY_REPORT );
	ui->tableBuyItems->setUtilitiesPlaceLayout ( ui->layoutTableBuyUtility );
	ui->tableBuyItems->setCallbackForCellChanged ( [&] ( const vmTableItem* const item ) {
		return interceptBuyItemsCellChange ( item ); } );
	ui->tableBuyItems->setCallbackForMonitoredCellChanged ( [&] ( const vmTableItem* const item ) {
		return ui->txtBuyTotalPrice->setText ( item->text (), true ); } );
	ui->tableBuyItems->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
		return buyKeyPressedSelector ( ke ); } );

	( void ) vmTableWidget::createPayHistoryTable ( ui->tableBuyPayments, this, PHR_METHOD );
	ui->tableBuyPayments->setKeepModificationRecords ( false );
	saveBuyWidget ( ui->tableBuyPayments, FLD_BUY_PAYINFO );
	ui->tableBuyPayments->setUtilitiesPlaceLayout ( ui->layoutTableBuyPaysUtility );
	ui->tableBuyPayments->setCallbackForCellChanged ( [&] ( const vmTableItem* const item ) {
		return interceptBuyPaymentCellChange ( item ); } );
	ui->tableBuyPayments->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
		return buyKeyPressedSelector ( ke ); } );

	connect ( ui->btnShowSuppliersDlg, &QToolButton::clicked, this, [&] () { return SUPPLIERS ()->showDialog (); } );
	connect ( ui->btnBuyAdd, &QToolButton::clicked, this, [&] () { return (mBuyCurItem = addBuy ( mClientCurItem, mJobCurItem )); } );
	connect ( ui->btnBuyEdit, &QToolButton::clicked, this, [&] () { return static_cast<void>( editBuy ( mBuyCurItem )); } );
	connect ( ui->btnBuyDel, &QToolButton::clicked, this, [&] () { return static_cast<void>( delBuy ( mBuyCurItem )); } );
	connect ( ui->btnBuySave, &QToolButton::clicked, this, [&] () { return static_cast<void>( saveBuy ( mBuyCurItem )); } );
	connect ( ui->btnBuyCancel, &QToolButton::clicked, this, [&] () { return static_cast<void>( cancelBuy ( mBuyCurItem )); } );
	connect ( ui->btnBuyCopyRows, &QToolButton::clicked, this, [&] () { return on_btnBuyCopyRows_clicked (); } );
}

void MainWindow::displayBuyFromCalendar ( vmListItem* cal_item )
{
	clientListItem* client_item ( static_cast<clientListItem*>( cal_item->relatedItem ( RLI_CLIENTPARENT ) ) );
	if ( client_item )
	{
		if ( client_item != mClientCurItem )
			displayClient ( client_item, true, static_cast<jobListItem*>( cal_item->relatedItem ( RLI_JOBPARENT ) ), static_cast<buyListItem*>( cal_item ) );
		else
			displayJob ( static_cast<jobListItem*>( cal_item->relatedItem ( RLI_JOBPARENT ) ), true, static_cast<buyListItem*>( cal_item ) );
		
		ui->tableBuyPayments->setCurrentCell ( cal_item->data ( Qt::UserRole ).toInt () - 1, 0 );
	}
}

void MainWindow::buysListWidget_currentItemChanged ( vmListItem* item )
{
	if ( item ) {
		insertNavItem ( item );
		if ( static_cast<jobListItem*> ( item->relatedItem ( RLI_JOBPARENT ) ) != mJobCurItem )
			displayJob ( static_cast<jobListItem*> ( item->relatedItem ( RLI_JOBPARENT ) ), true, static_cast<buyListItem*>( item ) );
		else
			displayBuy ( static_cast<buyListItem*> ( item ) );
	}
	else
		displayBuy ( nullptr );
}

void MainWindow::buysJobListWidget_currentItemChanged ( vmListItem* item )
{
	displayBuy ( item ? static_cast<buyListItem*>( item->relatedItem ( RLI_CLIENTITEM ) ) : nullptr, true );
}

void MainWindow::buySuppliersListWidget_currentItemChanged ( vmListItem* item )
{
	if ( item )
	{
		insertNavItem ( item );
		ui->lstBuySuppliers->setIgnoreChanges ( true );
		buyListItem* buy_item ( static_cast<buyListItem*> ( item ) );
		if ( static_cast<clientListItem*> ( buy_item->relatedItem ( RLI_CLIENTPARENT ) ) != mClientCurItem )
		{
			displayClient ( static_cast<clientListItem*> ( buy_item->relatedItem ( RLI_CLIENTPARENT ) ), true,
							static_cast<jobListItem*> ( buy_item->relatedItem ( RLI_JOBPARENT ) ),
							static_cast<buyListItem*> ( buy_item->relatedItem ( RLI_CLIENTITEM ) ) );
		}
		else
		{
			if ( static_cast<jobListItem*> ( buy_item->relatedItem ( RLI_JOBPARENT ) ) != mJobCurItem )
			{
				displayJob ( static_cast<jobListItem*> ( buy_item->relatedItem ( RLI_JOBPARENT ) ), true, 
							 static_cast<buyListItem*> ( buy_item->relatedItem ( RLI_CLIENTITEM ) ) ) ;
			}
			else
			{
				displayBuy ( static_cast<buyListItem*> ( buy_item->relatedItem ( RLI_CLIENTITEM ) ), true );
			}
		}
		ui->lstBuySuppliers->setIgnoreChanges ( false );
	}
}

void MainWindow::updateBuyTotalPriceWidgets ( const buyListItem* const buy_item )
{
	const Buy* buy ( buy_item->buyRecord () );
	if ( buy->wasModified ( FLD_BUY_PRICE ) )
	{
		vmNumber totalPrice ( ui->txtBuyTotalPurchasePriceForJob->text (), VMNT_PRICE, 1 );
		if ( buy->prevAction () == ACTION_EDIT )
			totalPrice -=  vmNumber ( buy->backupRecordStr ( FLD_BUY_PRICE ), VMNT_PRICE, 1 );
		totalPrice += buy->price ( FLD_BUY_PRICE );
		ui->txtBuyTotalPurchasePriceForJob->setText ( totalPrice.toPrice () );
	}
	
	if ( buy->wasModified ( FLD_BUY_TOTALPAID ) )
	{
		vmNumber totalPaid ( ui->txtBuyTotalPaidForJob->text (), VMNT_PRICE, 1 );
		if ( buy->prevAction () == ACTION_EDIT )
			totalPaid -=  vmNumber ( buy->backupRecordStr ( FLD_BUY_PRICE ), VMNT_PRICE, 1 );
		totalPaid += buy->price ( FLD_BUY_TOTALPAID );
		ui->txtBuyTotalPaidForJob->setText ( totalPaid.toPrice () );
	}
}

void MainWindow::controlBuyForms ( const buyListItem* const buy_item )
{
	const triStateType editing_action ( buy_item ? static_cast<TRI_STATE> ( buy_item->action () >= ACTION_ADD ) : TRI_UNDEF );

	//Make tables editable only when editing a record, because in such situations there is no clearing and loading involved, but a mere display change
	ui->tableBuyItems->setEditable ( editing_action.isOn () );
	ui->tableBuyPayments->setEditable ( editing_action.isOn () );
	ui->txtBuyTotalPrice->setEditable ( editing_action.isOn () );
	ui->txtBuyDeliveryMethod->setEditable ( editing_action.isOn () );
	ui->cboBuySuppliers->setEditable ( editing_action.isOn () );
	ui->dteBuyDate->setEditable ( editing_action.isOn () );
	ui->dteBuyDeliveryDate->setEditable ( editing_action.isOn () );
	ui->txtBuyNotes->setEditable ( editing_action.isOn () );

	ui->btnBuySave->setEnabled ( editing_action.isOn () ? buy_item->isGoodToSave () : false );
	ui->btnBuyEdit->setEnabled ( editing_action.isOff () );
	ui->btnBuyCancel->setEnabled ( editing_action.isOn () );
	ui->btnBuyDel->setEnabled ( editing_action.isOff () );
	ui->btnBuyAdd->setEnabled ( mJobCurItem != nullptr );

	static const Buy static_buy;
	const Buy* const buy ( buy_item ? buy_item->buyRecord () : &static_buy );
	const vmNumber buyTotalPrice ( recStrValue ( buy, FLD_BUY_PRICE ), VMNT_PRICE, 1 );
	ui->cboBuySuppliers->highlight ( editing_action.isOn () ? ( ui->cboBuySuppliers->text ().isEmpty () ? vmRed : vmDefault_Color ) : vmDefault_Color );
	ui->txtBuyTotalPrice->highlight ( editing_action.isOn () ? ( buyTotalPrice.isNull () ? vmRed : vmDefault_Color ) : vmDefault_Color );
	ui->dteBuyDate->highlight ( editing_action.isOn () ? ( vmNumber ( ui->dteBuyDate->date () ).isDateWithinRange ( vmNumber::currentDate, 0, 4 ) ? vmDefault_Color : vmRed ) : vmDefault_Color );
	ui->dteBuyDeliveryDate->highlight ( editing_action.isOn () ? ( vmNumber ( ui->dteBuyDeliveryDate->date () ).isDateWithinRange ( vmNumber::currentDate, 0, 4 ) ? vmDefault_Color : vmRed ) : vmDefault_Color );
	ui->txtBuyTotalPaid->highlight ( buyTotalPrice > buy->price ( FLD_BUY_TOTALPAID ) ? vmYellow : vmDefault_Color );

	ui->tableBuyItems->scrollTo ( ui->tableBuyItems->indexAt ( QPoint ( 0, 0 ) ) );
	ui->tableBuyPayments->scrollTo ( ui->tableBuyPayments->indexAt ( QPoint ( 0, 0 ) ) );
}

bool MainWindow::saveBuy ( buyListItem* buy_item )
{
	if ( buy_item )
	{
		Buy* buy ( buy_item->buyRecord () );
		if ( buy->saveRecord ( false ) ) // do not change indexes just now. Wait for dbCalendar actions
		{
			mCal->updateCalendarWithBuyInfo ( buy );
			buy_item->setAction ( ACTION_READ, true ); // now we can change indexes
			if ( ui->tableBuyPayments->tableChanged () )
			{
				vmTableWidget::exchangePurchaseTablesInfo ( ui->tableBuyItems, SUPPLIES ()->table (), buy, SUPPLIES ()->supplies_rec );
				ui->tableBuyPayments->setTableUpdated ();
			}
			updateBuyTotalPriceWidgets ( buy_item );

			static_cast<void>(Data::insertComboItem ( ui->cboBuySuppliers, recStrValue ( buy, FLD_BUY_SUPPLIER ) ));
			cboBuySuppliers_indexChanged ( ui->cboBuySuppliers->findText ( ui->cboBuySuppliers->text() ) ); // update suppliers' list with - possibly - a new entry
			
			VM_NOTIFY ()->notifyMessage ( TR_FUNC ( "Record saved" ), recStrValue ( buy_item->buyRecord (), FLD_BUY_SUPPLIER ) + 
										  TR_FUNC ( " purchase info saved!" ) );
			controlBuyForms ( buy_item );
			saveView ();
			removeEditItem ( static_cast<vmListItem*>(buy_item) );
			return true;
		}
	}
	return false;
}

buyListItem* MainWindow::addBuy ( clientListItem* client_parent, jobListItem* job_parent )
{
	buyListItem* buy_item ( new buyListItem );
	buy_item->setRelation ( RLI_CLIENTITEM );
	buy_item->setRelatedItem ( RLI_CLIENTPARENT, client_parent );
	buy_item->setRelatedItem ( RLI_JOBPARENT, job_parent );
	static_cast<void>(buy_item->loadData ());
	buy_item->setAction ( ACTION_ADD, true );
	client_parent->buys->append ( buy_item );

	buyListItem* buy_item2 ( new buyListItem );
	buy_item2->setRelation ( RLI_JOBITEM );
	buy_item->syncSiblingWithThis ( buy_item2 );
	buy_item2->addToList ( ui->buysJobList, false );
	job_parent->buys->append ( buy_item2 );
	
	buy_item->addToList ( ui->buysList );
	setRecValue ( buy_item->buyRecord (), FLD_BUY_CLIENTID, recStrValue ( client_parent->clientRecord (), FLD_CLIENT_ID ) );
	setRecValue ( buy_item->buyRecord (), FLD_BUY_JOBID, recStrValue ( job_parent->jobRecord (), FLD_JOB_ID ) );
	buy_item->setDBRecID ( static_cast<uint>(recIntValue ( buy_item->buyRecord (), FLD_BUY_ID )) );
	buy_item2->setDBRecID ( static_cast<uint>(recIntValue ( buy_item->buyRecord (), FLD_BUY_ID )) );
	insertEditItem ( static_cast<vmListItem*>(buy_item) );
	
	ui->cboBuySuppliers->setFocus ();
	return buy_item;
}

bool MainWindow::editBuy ( buyListItem* buy_item, const bool b_dogui )
{
	if ( buy_item )
	{
		if ( buy_item->action () == ACTION_READ )
		{
			buy_item->setAction ( ACTION_EDIT, true );
			insertEditItem ( static_cast<vmListItem*>(buy_item) );
			if ( b_dogui )
			{
				controlBuyForms ( buy_item );
				ui->txtBuyTotalPrice->setFocus ();
			}
			return true;
		}
	}
	return false;
}

bool MainWindow::delBuy ( buyListItem* buy_item )

{
	if ( buy_item )
	{
		const QString text ( TR_FUNC ( "Are you sure you want to remove buy %1?" ) );
		if ( VM_NOTIFY ()->questionBox ( TR_FUNC ( "Question" ), text.arg ( recStrValue ( buy_item->buyRecord (), FLD_BUY_PRICE ) ) ) )
		{
			buy_item->setAction ( ACTION_DEL, true );
			mCal->updateCalendarWithBuyInfo ( buy_item->buyRecord () );
			return cancelBuy ( buy_item );
		}
	}
	return false;
}

bool MainWindow::cancelBuy ( buyListItem* buy_item )
{
	if ( buy_item )
	{
		switch ( buy_item->action () )
		{
			case ACTION_ADD:
			case ACTION_DEL:
			{
				buyListItem* buy_jobitem ( static_cast<buyListItem*>(buy_item->relatedItem ( RLI_JOBITEM )) );
				ui->buysJobList->setIgnoreChanges ( true );
				static_cast<jobListItem*>(buy_item->relatedItem ( RLI_JOBPARENT ))->buys->remove ( buy_jobitem->row () );
				removeListItem ( buy_jobitem, false, false );
				ui->buysJobList->setIgnoreChanges ( false );
			
				buy_item->setAction ( ACTION_DEL, true );
				updateBuyTotalPriceWidgets ( buy_item );
				static_cast<clientListItem*>(buy_item->relatedItem ( RLI_CLIENTPARENT ))->buys->remove ( buy_item->row () );
				removeListItem ( buy_item );
			}
			break;
			case ACTION_EDIT:
				removeEditItem ( static_cast<vmListItem*>(buy_item) );
				buy_item->setAction ( ACTION_REVERT, true );
				displayBuy ( buy_item );
			break;
			case ACTION_NONE:
			case ACTION_READ:
			case ACTION_REVERT:
				return false;
		}
		return true;
	}
	return false;
}

void MainWindow::displayBuy ( buyListItem* buy_item, const bool b_select )
{
	if ( buy_item && buy_item->loadData () )
	{
		ui->tabMain->setCurrentIndex ( 0 );
		controlBuyForms ( buy_item );
		loadBuyInfo ( buy_item->buyRecord () );
		if ( buy_item != mBuyCurItem )
		{
			if ( b_select ) // Sync both lists, but make sure no signals are emitted
			{
				ui->buysList->setCurrentItem ( buy_item, false );
				ui->buysJobList->setCurrentItem ( buy_item->relatedItem ( RLI_JOBITEM ), false );
			}
			mBuyCurItem = buy_item;
			if ( buy_item->relatedItem ( RLI_EXTRAITEMS ) != nullptr )
				ui->lstBuySuppliers->setCurrentItem ( static_cast<buyListItem*> ( buy_item->relatedItem ( RLI_EXTRAITEMS ) ), false );
		}
	}
	else
	{
		mBuyCurItem = nullptr;
		loadBuyInfo ( nullptr );
		controlBuyForms ( nullptr );
	}
	if ( b_select )
		saveView ();
}

void MainWindow::loadBuyInfo ( const Buy* const buy )
{
	/* Tables are complex widgets, that string together several of the application's classes.
	 * To avoid a chain of calls when clearing a table, calls to all widgets (of several types)
	 * within all cells must be blocked by setting the table read-only. Otherwise we would see
	 * a plethora of unnecessary and potentially fatal callback calls
	 */
	const bool bIsEditable ( ui->tableBuyItems->isEditable () );
	ui->tableBuyItems->setEditable ( false );
	ui->tableBuyPayments->setEditable ( false );
	
	if ( buy )
	{
		ui->txtBuyID->setText ( recStrValue ( buy, FLD_BUY_ID ) );
		ui->txtBuyTotalPrice->setText ( recStrValue ( buy, FLD_BUY_PRICE ) );
		ui->txtBuyTotalPaid->setText ( recStrValue ( buy, FLD_BUY_TOTALPAID ) );
		ui->txtBuyNotes->setText ( recStrValue ( buy, FLD_BUY_NOTES ) );
		ui->txtBuyDeliveryMethod->setText ( recStrValue ( buy, FLD_BUY_DELIVERMETHOD ) );
		ui->dteBuyDate->setDate ( buy->date ( FLD_BUY_DATE ).toQDate () );
		ui->dteBuyDeliveryDate->setDate ( buy->date ( FLD_BUY_DELIVERDATE ).toQDate () );
		ui->tableBuyItems->loadFromStringTable ( recStrValue ( buy, FLD_BUY_REPORT ) );
		ui->tableBuyPayments->loadFromStringTable ( recStrValue ( buy, FLD_BUY_PAYINFO ) );

		const int cur_idx ( ui->cboBuySuppliers->findText ( recStrValue ( buy, FLD_BUY_SUPPLIER ) ) );		
		ui->cboBuySuppliers->setCurrentIndex ( cur_idx );
		//cboBuySuppliers_indexChanged ( cur_idx );
	}
	else
	{
		ui->tableBuyItems->clear ( true );
		ui->txtBuyID->setText ( QStringLiteral ( "-1" ) );
		ui->txtBuyTotalPrice->setText ( vmNumber::zeroedPrice.toPrice () );
		ui->txtBuyTotalPaid->setText ( vmNumber::zeroedPrice.toPrice () );
		ui->txtBuyNotes->setText ( emptyString );
		ui->txtBuyDeliveryMethod->setText ( emptyString );
		ui->dteBuyDate->setText ( emptyString );
		ui->dteBuyDeliveryDate->setText ( emptyString );
		ui->cboBuySuppliers->clearEditText ();
	}
	
	ui->tableBuyItems->setEditable ( bIsEditable );
	ui->tableBuyPayments->setEditable ( bIsEditable );
}

buyListItem* MainWindow::getBuyItem ( const clientListItem* const parent_client, const uint id ) const
{
	if ( id >= 1 && parent_client )
	{
		uint i ( 0 );
		while ( i < parent_client->buys->count () )
		{
			if ( parent_client->buys->at ( i )->dbRecID () == id )
				return parent_client->buys->at ( i );
			++i;
		}
	}
	return nullptr;
}

void MainWindow::fillJobBuyList ( const jobListItem* parent_job )
{
	ui->buysJobList->clear ();
	if ( parent_job )
	{
		vmNumber totalPrice, totalPaid;
		buyListItem* buy_item ( nullptr );
		for ( uint i ( 0 ); i < parent_job->buys->count (); ++i )
		{
			buy_item = parent_job->buys->at ( i );
			buy_item->addToList ( ui->buysJobList );
			totalPrice += buy_item->buyRecord ()->price ( FLD_BUY_PRICE );
			totalPaid += buy_item->buyRecord ()->price ( FLD_BUY_TOTALPAID );
		}
		ui->txtBuyTotalPurchasePriceForJob->setText ( totalPrice.toPrice () );
		ui->txtBuyTotalPaidForJob->setText ( totalPaid.toPrice () );
	}
	ui->buysJobList->setIgnoreChanges ( false );
}

void MainWindow::fillCalendarBuysList ( const stringTable& buyids, vmListWidget* list, const bool pay_date )
{
	if ( buyids.countRecords () > 0 )
	{
		const stringRecord* str_rec ( &buyids.first () );
		if ( str_rec->isOK () )
		{
			buyListItem* buy_item ( nullptr ), *buy_parent ( nullptr );
			clientListItem* client_parent ( nullptr );
			list->setIgnoreChanges ( true );
			do
			{	
				client_parent = getClientItem ( str_rec->fieldValue ( 1 ).toUInt () );
				buy_parent = getBuyItem ( client_parent, str_rec->fieldValue ( 0 ).toUInt () );
				if ( buy_parent )
				{
					buy_item = static_cast<buyListItem*>( buy_parent->relatedItem ( RLI_CALENDARITEM ) );
					if ( !buy_item )
					{
						buy_item = new buyListItem;
						buy_item->setRelation ( RLI_CALENDARITEM );
						buy_parent->syncSiblingWithThis ( buy_item );
						buy_item->setData ( Qt::UserRole, 0 ); // payment number
						buy_item->setData ( Qt::UserRole + 1, false ); // purchase date
						buy_item->setData ( Qt::UserRole + 2, false ); // payment date
					}
					
					if ( pay_date )
					{
						buy_item->setData ( Qt::UserRole + 2, true );
						buy_item->setData ( Qt::UserRole, str_rec->fieldValue ( 2 ).toInt () );
					}
					else
					{
						buy_item->setData ( Qt::UserRole + 1, true );
					}
					if ( buy_item->listWidget () != list )
						buy_item->addToList ( list );
					buy_item->update ();
				}
				str_rec = &buyids.next ();
			} while ( str_rec->isOK () );
			list->setIgnoreChanges ( false );
		}
	}
}

void MainWindow::interceptBuyItemsCellChange ( const vmTableItem* const item )
{
	const uint row ( static_cast<uint>(item->row ()) );
	if ( static_cast<int>(row) == ui->tableBuyItems->totalsRow () )
		return;

	stringTable buy_items;
	ui->tableBuyItems->makeStringTable ( buy_items );
	setRecValue ( mBuyCurItem->buyRecord (), FLD_BUY_REPORT, buy_items.toString () );
	postFieldAlterationActions ( mBuyCurItem );
}

/* The highlight calls here are only meant as a visual aid, and do not prevent the information being saved.
 * Perhaps in the future I will add a VM_NOTIFY ()->notifyMessage ()
*/
void MainWindow::interceptBuyPaymentCellChange ( const vmTableItem* const item )
{
	const uint row ( static_cast<uint>(item->row ()) );
	if ( static_cast<int>(row) == ui->tableBuyPayments->totalsRow () )
		return;

	const uint col ( static_cast<uint>(item->column ()) );
	switch ( col )
	{
		case PHR_ACCOUNT: return; // never reached
		case PHR_DATE:
		case PHR_USE_DATE:
		{
			const vmNumber cell_date ( item->date () );
			const bool bDateOk ( cell_date.isDateWithinRange ( vmNumber::currentDate, 0, 4 ) );
			ui->tableBuyPayments->sheetItem ( row, col )->highlight ( bDateOk ? vmDefault_Color : vmRed );
		}
		break;
		default:
		{
			const bool bPaid ( ui->tableBuyPayments->sheetItem ( row, PHR_PAID )->text () == CHR_ONE );
			const bool bValueOk ( bPaid == !ui->tableBuyPayments->sheetItem ( row, PHR_VALUE )->text ().isEmpty () );
			const bool bMethodOk ( bValueOk == !ui->tableBuyPayments->sheetItem ( row, PHR_METHOD )->text ().isEmpty () );
			ui->tableBuyPayments->sheetItem ( row, PHR_VALUE )->highlight ( bValueOk ? vmDefault_Color : vmRed );
			ui->tableBuyPayments->sheetItem ( row, PHR_METHOD )->highlight ( bMethodOk ? vmDefault_Color : vmRed );
			if ( col != PHR_METHOD )
			{
				updateBuyTotalPaidValue ();
				if ( bPaid && !ui->tableBuyPayments->sheetItem ( row, PHR_DATE )->cellIsAltered () )
					ui->tableBuyPayments->sheetItem ( row, PHR_DATE )->setDate ( vmNumber::currentDate );
			}
			else
			{
				if ( bMethodOk )
					APP_COMPLETERS ()->updateCompleter ( ui->tableBuyPayments->sheetItem ( row, PHR_METHOD )->text (), vmCompleters::PAYMENT_METHOD );
			}
		}
		break;
	}

	if ( signed( row + 1 ) >= ui->tableBuyPayments->lastUsedRow () )
		setRecValue ( mBuyCurItem->buyRecord (), FLD_BUY_TOTALPAYS, QString::number ( row + 1 ) );

	stringTable pay_data;
	ui->tableBuyPayments->makeStringTable ( pay_data );
	setRecValue ( mBuyCurItem->buyRecord (), FLD_BUY_PAYINFO, pay_data.toString () );
	postFieldAlterationActions ( mBuyCurItem );
}

void MainWindow::updateBuyTotalPaidValue ()
{
	vmNumber total;
	for ( int i ( 0 ); i <= ui->tableBuyPayments->lastUsedRow (); ++i )
	{
		if ( ui->tableBuyPayments->sheetItem ( static_cast<uint>(i), PHR_PAID )->text () == CHR_ONE )
			total += vmNumber ( ui->tableBuyPayments->sheetItem ( static_cast<uint>(i), PHR_VALUE )->text (), VMNT_PRICE );
	}
	ui->txtBuyTotalPaid->setText ( total.toPrice () );
	setRecValue ( mBuyCurItem->buyRecord (), FLD_BUY_TOTALPAID, total.toPrice () );
}

void MainWindow::buyKeyPressedSelector ( const QKeyEvent* ke )
{
	if ( ke->key () == Qt::Key_Escape )
		static_cast<void>(cancelBuy ( mBuyCurItem ));
	else
	{
		ui->clientsList->setFocus (); // trigger any pending editing_Finished or textAltered event
		saveBuy ( mBuyCurItem );
	}
}

void MainWindow::getPurchasesForSuppliers ( const QString& supplier )
{
	ui->lstBuySuppliers->clear ();
	Buy buy;
	buyListItem* buy_client ( nullptr ), *new_buyitem ( nullptr );
	buy.stquery.str_query = QLatin1String ( "SELECT * FROM PURCHASES WHERE `SUPPLIER`='" ) + supplier + CHR_CHRMARK;
	if ( buy.readFirstRecord ( FLD_BUY_SUPPLIER, supplier, false ) )
	{
		do
		{
			buy_client = getBuyItem ( getClientItem ( static_cast<uint>(recIntValue ( &buy, FLD_BUY_CLIENTID )) ), static_cast<uint>(recIntValue ( &buy, FLD_BUY_ID )) );
			if ( buy_client )
			{
				new_buyitem = static_cast<buyListItem*> ( buy_client->relatedItem ( RLI_EXTRAITEMS ) );
				if ( !new_buyitem )
				{
					new_buyitem = new buyListItem;
					new_buyitem->setRelation ( RLI_EXTRAITEMS );
					buy_client->syncSiblingWithThis ( new_buyitem );
				}
				(void) new_buyitem->loadData ();
				new_buyitem->addToList ( ui->lstBuySuppliers );
			}
		} while ( buy.readNextRecord ( true, false ) );
	}
}

void MainWindow::setBuyPayValueToBuyPrice ( const QString& price )
{
	int row ( ui->tableBuyPayments->lastUsedRow () );
	if ( row < 0 )
	{
		row = 0;
		ui->tableBuyPayments->setCurrentCell ( 0, 0, QItemSelectionModel::ClearAndSelect );
	}
	//if ( row == 0 )
	//{
		ui->tableBuyPayments->sheetItem ( static_cast<uint>(row), PHR_VALUE )->setText ( price, false, true );
		ui->tableBuyPayments->sheetItem ( static_cast<uint>(row), PHR_DATE )->setDate ( ui->dteBuyDate->date () );
	//}
}
//--------------------------------------------BUY------------------------------------------------------------

//--------------------------------------------EDITING-FINISHED-BUY-----------------------------------------------------------	
void MainWindow::txtBuy_textAltered ( const vmWidget* const sender )
{
	bool input_ok ( true );
	if ( static_cast<uint>(sender->id ()) == FLD_BUY_PRICE )
	{
		const vmNumber price ( sender->text (), VMNT_PRICE, 1 );
		input_ok = price > 0;
		mBuyCurItem->setFieldInputStatus ( FLD_BUY_PRICE, input_ok, ui->txtBuyTotalPrice );
		setBuyPayValueToBuyPrice ( sender->text () );
	}
	//if ( input_ok )
		setRecValue ( mBuyCurItem->buyRecord (), static_cast<uint>(sender->id ()), sender->text () );
	postFieldAlterationActions ( mBuyCurItem );
}

void MainWindow::dteBuy_dateAltered ( const vmWidget* const sender )
{
	const vmNumber date ( static_cast<const vmDateEdit* const>( sender )->date () );
	const bool input_ok ( date.isDateWithinRange ( vmNumber::currentDate, 0, 4 ) );
	//if ( input_ok )
	{
		setRecValue ( mBuyCurItem->buyRecord (), static_cast<uint>(sender->id ()), date.toDate ( vmNumber::VDF_DB_DATE ) );
		mBuyCurItem->update ();
	}
	mBuyCurItem->setFieldInputStatus ( static_cast<uint>(sender->id ()), input_ok, sender );
	postFieldAlterationActions ( mBuyCurItem );
}

void MainWindow::cboBuySuppliers_textAltered ( const QString& text )
{
	int idx ( -2 );
	if ( !text.isEmpty () )
	{
		setRecValue ( mBuyCurItem->buyRecord (), FLD_BUY_SUPPLIER, ui->cboBuySuppliers->text () );
		mBuyCurItem->update ();
		idx = ui->cboBuySuppliers->findText ( text );
	}
	cboBuySuppliers_indexChanged ( idx );
	mBuyCurItem->setFieldInputStatus ( FLD_BUY_SUPPLIER, idx != -2, ui->cboBuySuppliers );
	postFieldAlterationActions ( mBuyCurItem );
}

void MainWindow::cboBuySuppliers_indexChanged ( const int idx )
{
	ui->lstBuySuppliers->setIgnoreChanges ( true );
	if ( idx != -1 )
	{
		const QString supplier ( ui->cboBuySuppliers->itemText ( idx ) );
		getPurchasesForSuppliers ( supplier );
		ui->lblBuySupplierBuys->setText ( TR_FUNC ( "Purchases from " ) + supplier );
		SUPPLIERS ()->displaySupplier ( supplier );
	}
	else
	{
		ui->lblBuySupplierBuys->setText ( TR_FUNC ( "Supplier list empty" ) );
		ui->lstBuySuppliers->clear ();
	}
	ui->lstBuySuppliers->setIgnoreChanges ( false );
}
//--------------------------------------------EDITING-FINISHED-BUY-----------------------------------------------------------

//----------------------------------SETUP-CUSTOM-CONTROLS-NAVIGATION--------------------------------------
void MainWindow::setupCustomControls ()
{
	// They are needed to fill vmCompleters::ALL_CATEGORIES, which is needed by textEditWithCompleter
	dbSuppliesUI::init ();
	//ui->tabSupplies->setEnabled ( DATA ()->reads[TABLE_SUPPLIES_ORDER] );

	InventoryUI::init ();
	//ui->tabInventory->setEnabled ( DATA ()->reads[TABLE_INVENTORY_ORDER] );
	//ui->btnImportExport->setEnabled ( DATA ()->reads[TABLES_IN_DB] || DATA ()->writes[TABLES_IN_DB] );

	APP_RESTORER ()->addRestoreInfo ( m_crash = new crashRestore ( QStringLiteral ( "vmmain" ) ) );
	ui->txtSearch->setEditable ( true );
	connect ( ui->txtSearch, &QLineEdit::textChanged, this, [&] ( const QString& text ) { return on_txtSearch_textEdited ( text ); } );
	ui->txtSearch->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
		return searchCallbackSelector ( ke ); } );
	connect ( ui->btnSearchStart, &QToolButton::clicked, this, [&] () { return on_btnSearchStart_clicked (); } );
	connect ( ui->btnSearchCancel, &QToolButton::clicked, this, [&] () { return on_btnSearchCancel_clicked (); } );
	
	ui->lblCurInfoClient->setCallbackForLabelActivated ( [&] () { return ui->scrollWorkFlow->ensureWidgetVisible ( grpClients ); } );
	ui->lblCurInfoJob->setCallbackForLabelActivated ( [&] () { return ui->scrollWorkFlow->ensureWidgetVisible ( ui->btnJobAdd ); } );
	ui->lblCurInfoPay->setCallbackForLabelActivated ( [&] () { return ui->scrollWorkFlow->ensureWidgetVisible ( ui->txtPayTotalPrice ); } );
	ui->lblCurInfoBuy->setCallbackForLabelActivated ( [&] () { return ui->scrollWorkFlow->ensureWidgetVisible ( ui->tableBuyItems ); } );

	setupCalendarMethods ();
	setupClientPanel ();
	setupJobPanel ();
	setupPayPanel ();
	setupBuyPanel ();
	reOrderTabSequence ();
	setupWorkFlow ();

	connect ( ui->btnReportGenerator, &QToolButton::clicked, this, [&] () { return btnReportGenerator_clicked (); } );
	connect ( ui->btnBackupRestore, &QToolButton::clicked, this, [&] () { return btnBackupRestore_clicked (); } );
	connect ( ui->btnCalculator, &QToolButton::clicked, this, [&] () { return btnCalculator_clicked (); } );
	connect ( ui->btnEstimates, &QToolButton::clicked, this, [&] () { return btnEstimates_clicked (); } );
	connect ( ui->btnCompanyPurchases, &QToolButton::clicked, this, [&] () { return btnCompanyPurchases_clicked (); } );
	connect ( ui->btnConfiguration, &QToolButton::clicked, this, [&] () { return btnConfiguration_clicked (); } );
	connect ( ui->btnExitProgram, &QToolButton::clicked, this, [&] () { return btnExitProgram_clicked (); } );

	installEventFilter ( this );
}

void MainWindow::restoreCrashedItems ( const crashRestore* crash )
{
	stringRecord* __restrict savedRec ( nullptr );
	if ( (savedRec = const_cast<stringRecord*>(&crash->restoreFirstState ())) )
	{
		bool ok ( true );
		int type_id ( -1 );
		RECORD_ACTION action ( ACTION_NONE );
		vmListItem* item ( nullptr );
		clientListItem* client_parent ( nullptr );
		jobListItem* job_parent ( nullptr );
		
		type_id = savedRec->fieldValue ( CF_SUBTYPE ).toInt ( &ok );
		if ( !ok || type_id < 0 )
			return;
		action = static_cast<RECORD_ACTION> ( savedRec->fieldValue ( CF_ACTION ).toInt ( &ok ) );
		
		do
		{
			switch ( type_id )
			{
				case CLIENT_TABLE:
					if ( action == ACTION_EDIT )
					{
						client_parent = getClientItem ( savedRec->fieldValue ( CF_DBRECORD + FLD_CLIENT_ID ).toUInt ( &ok ) );
						if ( !client_parent )
							break;
						client_parent->loadData ();
						static_cast<void>(editClient ( client_parent, false ));
					}
					else
						client_parent = addClient ();
					client_parent->clientRecord ()->fromStringRecord ( *savedRec, CF_DBRECORD );
				break;
				case JOB_TABLE:
					client_parent = getClientItem ( savedRec->fieldValue ( CF_DBRECORD + FLD_JOB_CLIENTID ).toUInt ( &ok ) );
					if ( !client_parent )
						break;
					if ( action == ACTION_EDIT )
					{
						job_parent = getJobItem ( client_parent, savedRec->fieldValue ( CF_DBRECORD + FLD_JOB_ID ).toUInt ( &ok ) );
						if ( !job_parent )
							break;
						job_parent->loadData ();
						static_cast<void>(editJob ( job_parent, false ));
					}
					else
						job_parent = addJob ( client_parent );
					job_parent->jobRecord ()->fromStringRecord ( *savedRec, CF_DBRECORD );
				break;
				case PAYMENT_TABLE:
					client_parent = getClientItem ( savedRec->fieldValue ( CF_DBRECORD + FLD_PAY_CLIENTID ).toUInt ( &ok ) );
					if ( !client_parent )
						break;
					job_parent = getJobItem ( client_parent, savedRec->fieldValue ( CF_DBRECORD + FLD_PAY_JOBID ).toUInt ( &ok ) );
					if ( !job_parent )
						break;
					if ( action == ACTION_EDIT )
					{
						item = getPayItem ( client_parent, savedRec->fieldValue ( CF_SUBTYPE + FLD_PAY_ID ).toUInt ( &ok ) );
						if ( !item )
							break;
						static_cast<payListItem*>(item)->loadData ();
						static_cast<void>(editPay ( static_cast<payListItem*>(item), false ));
					}
					else
						item = job_parent->payItem ();
					item->dbRec ()->fromStringRecord ( *savedRec, CF_DBRECORD );
				break;
				case PURCHASE_TABLE:
					client_parent = getClientItem ( savedRec->fieldValue ( CF_DBRECORD + FLD_BUY_CLIENTID ).toUInt ( &ok ) );
					if ( !client_parent )
						break;
					job_parent = getJobItem ( client_parent, savedRec->fieldValue ( CF_DBRECORD + FLD_BUY_JOBID ).toUInt ( &ok ) );
					if ( !job_parent )
						break;
					if ( action == ACTION_EDIT )
					{
						item = getBuyItem ( client_parent, savedRec->fieldValue ( CF_DBRECORD + FLD_BUY_ID ).toUInt ( &ok ) );
						if ( !item )
							break;
						static_cast<buyListItem*>(item)->loadData ();
					}
					else
						item = addBuy ( client_parent, job_parent );
					item->dbRec ()->fromStringRecord ( *savedRec, CF_DBRECORD );
				break;
				default:
				break;
			}
		}
		while ( (savedRec = const_cast<stringRecord*>(&crash->restoreNextState ())) );
		
	}
}

void MainWindow::restoreLastSession ()
{
	ui->clientsList->setIgnoreChanges ( false );
	ui->tabMain->setCurrentIndex ( 0 );
	if ( m_crash->needRestore () )
	{
		VM_NOTIFY ()->notifyMessage ( TR_FUNC ( "Session Manager" ), TR_FUNC ( "Restoring last session due to unclean program termination" ) );
		restoreCrashedItems ( m_crash );
	}

	clientListItem* client_item ( getClientItem ( CONFIG ()->lastViewedRecord ( CLIENT_TABLE ) ) );
	if ( !client_item )
		client_item = static_cast<clientListItem*> ( ui->clientsList->item ( ui->clientsList->count () - 1 ) );
	jobListItem* job_item ( getJobItem ( client_item, CONFIG ()->lastViewedRecord ( JOB_TABLE ) ) );
	displayClient ( client_item, true, job_item, getBuyItem ( client_item, CONFIG ()->lastViewedRecord ( PURCHASE_TABLE ) ) );
	insertNavItem ( ui->jobsList->currentItem () );
	ui->clientsList->setFocus ();
}

void MainWindow::searchCallbackSelector ( const QKeyEvent* ke )
{
	if ( ke->key () == Qt::Key_Escape )
		on_btnSearchCancel_clicked ();
	else
		on_btnSearchStart_clicked ();
}

void MainWindow::reOrderTabSequence ()
{
	setTabOrder ( ui->chkClientActive, ui->contactsClientPhones->combo () );
	setTabOrder ( ui->contactsClientPhones->combo (), ui->contactsClientEmails->combo () );
	setTabOrder ( ui->contactsClientEmails->combo (), ui->dteClientDateFrom );
	setTabOrder ( ui->dteClientDateFrom, ui->dteClientDateTo );
	setTabOrder ( ui->dteClientDateTo, ui->btnJobAdd );
	
	setTabOrder ( ui->btnJobNextDay, ui->dteJobAddDate );
	setTabOrder ( ui->dteJobAddDate, ui->btnJobAddDay );
	setTabOrder ( ui->btnJobSeparatePicture, ui->jobImageViewer );
	setTabOrder ( ui->jobImageViewer, ui->btnPayInfoEdit );
	
	setTabOrder ( ui->buysJobList, ui->dteBuyDate );
	setTabOrder ( ui->dteBuyDate, ui->dteBuyDeliveryDate );
	setTabOrder ( ui->dteBuyDeliveryDate, ui->txtBuyDeliveryMethod );
}

void MainWindow::setupWorkFlow ()
{
	mainTaskPanel = new vmTaskPanel;
	grpClients = mainTaskPanel->createGroup ( TR_FUNC ( "CLIENT INFORMATION" ), true, false );
	grpClients->setMinimumHeight ( ui->frmClientInfo->sizeHint ().height () + 1 );
	grpClients->addQEntry ( ui->frmClientInfo, new QHBoxLayout );

	grpJobs = mainTaskPanel->createGroup ( TR_FUNC ( "JOB REPORT AND IMAGES" ), true, false );
	grpJobs->setMinimumHeight ( ui->frmJobInfo->sizeHint ().height () + ui->frmJobInfo_2->sizeHint ().height () );
	grpJobs->addQEntry ( ui->frmJobInfo, new QHBoxLayout );
	grpJobs->addQEntry ( ui->frmJobInfo_2, new QHBoxLayout );

	grpPays = mainTaskPanel->createGroup ( TR_FUNC ( "PAYMENTS FOR JOB" ), true, false );
	grpPays->setMinimumHeight ( ui->frmPayInfo->sizeHint ().height () + 1 );
	grpPays->addQEntry ( ui->frmPayInfo, new QHBoxLayout );

	grpBuys = mainTaskPanel->createGroup ( TR_FUNC ( "PURCHASES FOR JOB" ), true, false );
	grpBuys->setMinimumHeight ( ui->splitterBuyInfo->sizeHint ().height () + 1 );
	grpBuys->addQEntry ( ui->splitterBuyInfo, new QHBoxLayout );

	ui->scrollWorkFlow->setWidget ( mainTaskPanel );

	setupTabNavigationButtons ();
	ui->tabMain->removeTab ( 7 );
	ui->tabMain->removeTab ( 6 );
	ui->tabMain->removeTab ( 5 );
	ui->tabMain->removeTab ( 4 );
}

void MainWindow::setupTabNavigationButtons ()
{
	mBtnNavPrev = new QToolButton;
	mBtnNavPrev->setIcon ( ICON ( "go-previous" ) );
	mBtnNavPrev->setEnabled ( false );
	connect ( mBtnNavPrev, &QToolButton::clicked, this, [&] () { return navigatePrev (); } );
	mBtnNavNext = new QToolButton;
	mBtnNavNext->setIcon ( ICON ( "go-next" ) );
	mBtnNavNext->setEnabled ( false );
	connect ( mBtnNavNext, &QToolButton::clicked, this, [&] () { return navigateNext (); } );
	
	QHBoxLayout* lButtons ( new QHBoxLayout );
	lButtons->setMargin ( 1 );
	lButtons->setSpacing ( 1 );
	lButtons->addWidget ( mBtnNavPrev );
	lButtons->addWidget ( mBtnNavNext );
	QFrame* frmContainer ( new QFrame );
	frmContainer->setLayout ( lButtons );
	ui->tabMain->tabBar ()->setTabButton ( 0, QTabBar::LeftSide, frmContainer );
	navItems.setPreAllocNumber ( 50 );
	editItems.setPreAllocNumber ( 10 );
}
//----------------------------------SETUP-CUSTOM-CONTROLS-NAVIGATION--------------------------------------

//--------------------------------------------CALENDAR-----------------------------------------------------------
void MainWindow::setupCalendarMethods ()
{
	mCal = new dbCalendar;
	connect ( ui->calMain, &QCalendarWidget::activated, this, [&] ( const QDate& date ) {
				return calMain_activated ( date ); } );
	connect ( ui->calMain, &QCalendarWidget::clicked, this, [&] ( const QDate& date ) {
				return calMain_activated ( date ); } );
	connect ( ui->calMain, &QCalendarWidget::currentPageChanged, this, [&] ( const int year, const int month ) {
				return updateCalendarView ( static_cast<uint>(year), static_cast<uint>(month) ); } );
	connect ( ui->tboxCalJobs, &QToolBox::currentChanged, this, [&] ( const int index ) {
				return tboxCalJobs_currentChanged ( index ); } );
	connect ( ui->tboxCalPays, &QToolBox::currentChanged, this, [&] ( const int index ) {
				return tboxCalPays_currentChanged ( index ); } );
	connect ( ui->tboxCalBuys, &QToolBox::currentChanged, this, [&] ( const int index ) {
				return tboxCalBuys_currentChanged ( index ); } );
	
	ui->lstCalJobsDay->setCallbackForCurrentItemChanged ( [&] ( vmListItem* item ) {
				return displayJobFromCalendar ( item ); } );
	ui->lstCalJobsWeek->setCallbackForCurrentItemChanged ( [&] ( vmListItem* item ) {
				return displayJobFromCalendar ( item ); } );
	ui->lstCalJobsMonth->setCallbackForCurrentItemChanged ( [&] ( vmListItem* item ) {
				return displayJobFromCalendar ( item ); } );
	ui->lstCalPaysDay->setCallbackForCurrentItemChanged( [&] ( vmListItem* item ) {
				return displayPayFromCalendar ( item ); } );
	ui->lstCalPaysWeek->setCallbackForCurrentItemChanged( [&] ( vmListItem* item ) {
				return displayPayFromCalendar ( item ); } );
	ui->lstCalPaysMonth->setCallbackForCurrentItemChanged ( [&] ( vmListItem* item ) {
				return displayPayFromCalendar ( item ); } );
	ui->lstCalBuysDay->setCallbackForCurrentItemChanged ( [&] ( vmListItem* item ) {
				return displayBuyFromCalendar ( item ); } );
	ui->lstCalBuysWeek->setCallbackForCurrentItemChanged ( [&] ( vmListItem* item ) {
				return displayBuyFromCalendar ( item ); } );
	ui->lstCalBuysMonth->setCallbackForCurrentItemChanged ( [&] ( vmListItem* item ) {
				return displayBuyFromCalendar ( item ); } );
}

void MainWindow::calMain_activated ( const QDate& date, const bool bUserAction )
{
	if ( date != mCalendarDate.toQDate () || !bUserAction )
	{
		changeCalendarToolBoxesText ( vmNumber ( date ) );

		switch ( ui->tboxCalJobs->currentIndex () )
		{ 
			case 0:	 ui->lstCalJobsDay->clear ();	break;
			case 1:	 ui->lstCalJobsWeek->clear ();   break;
			case 2:	 ui->lstCalJobsMonth->clear ();  break;
		}
		switch ( ui->tboxCalPays->currentIndex () )
		{
			case 0:	 ui->lstCalPaysDay->clear ();	break;
			case 1:	 ui->lstCalPaysWeek->clear ();   break;
			case 2:	 ui->lstCalPaysMonth->clear ();  break;
		}
		switch ( ui->tboxCalBuys->currentIndex () )
		{
			case 0:	 ui->lstCalBuysDay->clear ();	break;
			case 1:	 ui->lstCalBuysWeek->clear ();   break;
			case 2:	 ui->lstCalBuysMonth->clear ();  break;
		}

		mCalendarDate = date;
		tboxCalJobs_currentChanged ( ui->tboxCalJobs->currentIndex () );
		tboxCalPays_currentChanged ( ui->tboxCalPays->currentIndex () );
		tboxCalBuys_currentChanged ( ui->tboxCalBuys->currentIndex () );
	}
}

void MainWindow::updateCalendarView ( const uint year, const uint month )
{
	QString price;
	vmNumber date;
	date.setDate ( 1, static_cast<int>(month), static_cast<int>(year) );
	const stringTable jobsPerDateList ( mCal->dateLog ( date, FLD_CALENDAR_MONTH,
				FLD_CALENDAR_JOBS, price, FLD_CALENDAR_TOTAL_JOBPRICE_SCHEDULED, true ) );

	if ( jobsPerDateList.countRecords () > 0 )
	{
		const stringRecord* str_rec ( &jobsPerDateList.first () );
		if ( str_rec->isOK () )
		{
			uint jobid ( 0 );
			Job job;
			QString tooltip, day;
			QTextCharFormat dateChrFormat;
			ui->calMain->setDateTextFormat ( QDate ( 0, 0 ,0 ), QTextCharFormat () );

			do
			{
				jobid =  str_rec->fieldValue ( 0 ).toUInt ();
				if ( job.readRecord ( jobid ) )
				{
					date.fromTrustedStrDate( str_rec->fieldValue ( 3 ), vmNumber::VDF_DB_DATE );
					day = str_rec->fieldValue( 2 );
					dateChrFormat = ui->calMain->dateTextFormat ( date.toQDate () );
					tooltip = dateChrFormat.toolTip ();
					tooltip +=  QLatin1String ( "<br>Job ID: " ) + recStrValue ( &job, FLD_JOB_ID ) +
								QLatin1String ( "<br>Client: " ) + Client::clientName ( recStrValue ( &job, FLD_JOB_CLIENTID ) ) +
								QLatin1String ( "<br>Job type: " ) + recStrValue ( &job, FLD_JOB_TYPE ) +
								QLatin1String ( " (Day " ) + str_rec->fieldValue ( 2 ) + CHR_R_PARENTHESIS + QLatin1String ( "<br>" );

					dateChrFormat.setForeground ( Qt::white );
					dateChrFormat.setBackground ( QBrush ( Qt::darkBlue ) );
					dateChrFormat.setToolTip ( tooltip );
					ui->calMain->setDateTextFormat ( date.toQDate (), dateChrFormat );
				}
				str_rec = &jobsPerDateList.next ();
			} while ( str_rec->isOK () );
		}
	}
}

void MainWindow::tboxCalJobs_currentChanged ( const int index )
{
	QString price;
	switch ( index )
	{
		case 0:
			if ( ui->lstCalJobsDay->count () == 0 )
			{
				fillCalendarJobsList (
					mCal->dateLog ( mCalendarDate, FLD_CALENDAR_DAY_DATE,
								FLD_CALENDAR_JOBS, price, FLD_CALENDAR_TOTAL_JOBPRICE_SCHEDULED ), ui->lstCalJobsDay );
				ui->txtCalPriceJobDay->setText ( price );
			}
		break;
		case 1:
			if ( ui->lstCalJobsWeek->count () == 0 )
			{
				fillCalendarJobsList (
					mCal->dateLog ( mCalendarDate, FLD_CALENDAR_WEEK_NUMBER,
								FLD_CALENDAR_JOBS, price, FLD_CALENDAR_TOTAL_JOBPRICE_SCHEDULED ), ui->lstCalJobsWeek );
				ui->txtCalPriceJobWeek->setText ( price );
			}
		break;
		case 2:
			if ( ui->lstCalJobsMonth->count () == 0 )
			{
				fillCalendarJobsList (
					mCal->dateLog ( mCalendarDate, FLD_CALENDAR_MONTH,
								FLD_CALENDAR_JOBS, price, FLD_CALENDAR_TOTAL_JOBPRICE_SCHEDULED ), ui->lstCalJobsMonth );
				ui->txtCalPriceJobMonth->setText ( price );
			}
		break;
		default: return;
	}
}

void MainWindow::tboxCalPays_currentChanged ( const int index )
{
	bool bFillList ( false );
	vmListWidget* list ( nullptr );
	vmLineEdit* line ( nullptr );

	switch ( index )
	{
		case 0:
			bFillList = ( ui->lstCalPaysDay->count () == 0 );
			list = ui->lstCalPaysDay;
			line = ui->txtCalPricePayDay;
		break;
		case 1:
			bFillList = ( ui->lstCalPaysWeek->count () == 0 );
			list = ui->lstCalPaysWeek;
			line = ui->txtCalPricePayWeek;
		break;
		case 2:
			bFillList = ( ui->lstCalPaysMonth->count () == 0 );
			list = ui->lstCalPaysMonth;
			line = ui->txtCalPricePayMonth;
		break;
		default: return;
	}
	if ( bFillList )
	{
		QString price;
		const uint search_field ( FLD_CALENDAR_DAY_DATE + static_cast<uint>(index) );
		fillCalendarPaysList ( mCal->dateLog ( mCalendarDate, search_field,
				FLD_CALENDAR_PAYS, price, FLD_CALENDAR_TOTAL_PAY_RECEIVED ), list );
		line->setText ( price );
		fillCalendarPaysList ( mCal->dateLog ( mCalendarDate, search_field,
				FLD_CALENDAR_PAYS_USE, price, FLD_CALENDAR_TOTAL_PAY_RECEIVED ), list, true );
	}
}

void MainWindow::tboxCalBuys_currentChanged ( const int index )
{
	bool bFillList ( false );
	vmListWidget* list ( nullptr );
	vmLineEdit* line ( nullptr );

	switch ( index )
	{
		case 0:
			bFillList = ( ui->lstCalBuysDay->count () == 0 );
			list = ui->lstCalBuysDay;
			line = ui->txtCalPriceBuyDay;
		break;
		case 1:
			bFillList = ( ui->lstCalBuysWeek->count () == 0 );
			list = ui->lstCalBuysWeek;
			line = ui->txtCalPriceBuyWeek;
		break;
		case 2:
			bFillList = ( ui->lstCalBuysMonth->count () == 0 );
			list = ui->lstCalBuysMonth;
			line = ui->txtCalPriceBuyMonth;
		break;
		default: return;
	}
	if ( bFillList )
	{
		QString price;
		const uint search_field ( FLD_CALENDAR_DAY_DATE + static_cast<uint>(index) );
		fillCalendarBuysList ( mCal->dateLog ( mCalendarDate, search_field,
							FLD_CALENDAR_BUYS, price, FLD_CALENDAR_TOTAL_BUY_BOUGHT ), list );
		line->setText ( price );
		fillCalendarBuysList ( mCal->dateLog ( mCalendarDate, search_field,
							FLD_CALENDAR_BUYS_PAY, price, FLD_CALENDAR_TOTAL_BUY_PAID ), list, true );
		if ( !price.isEmpty () )
			line->setText ( line->text () + QLatin1String ( " (" ) + price + CHR_R_PARENTHESIS );
	}
}

void MainWindow::changeCalendarToolBoxesText ( const vmNumber& date )
{
	vmNumber firstDayOfWeek ( date );
	firstDayOfWeek.setDate ( static_cast<int>(1 - date.dayOfWeek ()), 0, 0, true );
	const QString str_firstDayOfWeek ( firstDayOfWeek.toString () );
	vmNumber lastDayOfWeek ( firstDayOfWeek );
	lastDayOfWeek.setDate ( 6, 0, 0, true );
	const QString str_lastDayOfWeek ( lastDayOfWeek.toString () );
	const QString date_str ( date.toString () );
	const QString monthName ( QDate::longMonthName ( static_cast<int>(date.month ()) ) );

	ui->tboxCalJobs->setItemText ( 0, TR_FUNC ( "Jobs at " ) + date_str );
	ui->tboxCalJobs->setItemText ( 1, TR_FUNC ( "Jobs from " ) + str_firstDayOfWeek + TR_FUNC ( " through " ) + str_lastDayOfWeek );
	ui->tboxCalJobs->setItemText ( 2, TR_FUNC ( "Jobs in " ) + monthName );
	ui->tboxCalPays->setItemText ( 0, TR_FUNC ( "Payments at " ) + date_str );
	ui->tboxCalPays->setItemText ( 1, TR_FUNC ( "Payments from " ) + str_firstDayOfWeek + TR_FUNC ( " through " ) + str_lastDayOfWeek );
	ui->tboxCalPays->setItemText ( 2, TR_FUNC ( "Payments in " ) + monthName );
	ui->tboxCalBuys->setItemText ( 0, TR_FUNC ( "Purchases at " ) + date_str );
	ui->tboxCalBuys->setItemText ( 1, TR_FUNC ( "Purchases from " ) + str_firstDayOfWeek + TR_FUNC ( " through " ) + str_lastDayOfWeek );
	ui->tboxCalBuys->setItemText ( 2, TR_FUNC ( "Purchases in " ) + monthName );
}
//--------------------------------------------CALENDAR-----------------------------------------------------------

//--------------------------------------------WINDOWS-KEYS-EVENTS-EXTRAS-------------------------------------------------
void MainWindow::closeEvent ( QCloseEvent* e )
{
	e->ignore ();
	trayActivated ( QSystemTrayIcon::Trigger );
}

void MainWindow::changeEvent ( QEvent* e )
{
	QMainWindow::changeEvent ( e );
	switch ( e->type () )
	{
		default:
		break;
		case QEvent::LanguageChange:
			ui->retranslateUi ( this );
		break;
		case QEvent::StyleChange:
			setStyle ( QApplication::style () );
			setPalette ( QApplication::style ()->standardPalette () );
		break;
	}
}

void MainWindow::saveView ()
{
	ui->lblCurInfoClient->setText ( mClientCurItem ? mClientCurItem->text () : TR_FUNC ( "No client selected" ) );
	ui->lblCurInfoJob->setText ( mJobCurItem ? mJobCurItem->text () : TR_FUNC ( "No job selected" ) );
	ui->lblCurInfoPay->setText ( mPayCurItem ? mPayCurItem->text () : TR_FUNC ( "No payment selected" ) );
	ui->lblCurInfoBuy->setText ( mBuyCurItem ? mBuyCurItem->text () : TR_FUNC ( "No purchase selected" ) );
	ui->lblBuyAllPurchases_client->setText ( mClientCurItem ? mClientCurItem->text () : TR_FUNC ( "NONE" ) );
	
	stringRecord ids;
	ids.fastAppendValue ( mClientCurItem ?
						  QString::number ( mClientCurItem->dbRecID () ) : QStringLiteral ( "-1" ) );
	ids.fastAppendValue ( mJobCurItem ?
						  QString::number ( mJobCurItem->dbRecID () ) : QStringLiteral ( "-1" ) );
	ids.fastAppendValue ( mBuyCurItem ?
						  QString::number ( mBuyCurItem->dbRecID () ) : QStringLiteral ( "-1" ) );

	CONFIG ()->writeConfigFile ( LAST_VIEWED_RECORDS, ids.toString () );
}

// return false: standard event processing
// return true: we process the event
bool MainWindow::eventFilter ( QObject* o, QEvent* e )
{
	bool b_accepted ( false );
	if ( e->type () == QEvent::KeyPress )
	{
		const QKeyEvent* k = static_cast<QKeyEvent*> ( e );
		if ( k->modifiers () & Qt::ControlModifier )
		{
			switch ( k->key () )
			{
				case Qt::Key_F:
					ui->txtSearch->setFocus ();
					b_accepted = true;
				break;
				default:
				break;
			}
		}
		else
		{ // no control key pressed
			b_accepted = (k->key () - Qt::Key_F4) <= (Qt::Key_F12 - Qt::Key_F4);
			switch ( k->key () )
			{
				case Qt::Key_Escape:
					if ( selJob_callback )
					{
						selJob_callback ( 0 );
						selJob_callback = nullptr;
						b_accepted = true;
					}
				break;

				case Qt::Key_F4: btnReportGenerator_clicked (); break;
				case Qt::Key_F5: btnBackupRestore_clicked (); break;
				case Qt::Key_F6: btnCalculator_clicked (); break;
				case Qt::Key_F7: ; break;
				case Qt::Key_F8: btnEstimates_clicked (); break;
				case Qt::Key_F9: btnCompanyPurchases_clicked (); break;
				case Qt::Key_F10: btnConfiguration_clicked (); break;
				case Qt::Key_F12: btnExitProgram_clicked (); break;
			}
		}
	}
	e->setAccepted ( b_accepted );
	return b_accepted ? true : QMainWindow::eventFilter ( o, e );
}

void MainWindow::selectBuysItems ( const PROCESS_STEPS step )
{
	ui->btnBuyCopyRows->setEnabled ( step == PS_PREPARE );
	ui->tabWorkFlow->setEnabled ( true );

	switch ( step )
	{
		case PS_PREPARE:
			//ui->tabMain->setCurrentIndex ( BUYS_TAB );
			ui->buysList->setFocus ();
		break;
		case PS_EXEC:
		{
			const uint n_copied_items ( QUICK_PROJECT ()->copyItemsFromTable ( ui->tableBuyItems ) );
			ui->statusbar->showMessage ( QString ( TR_FUNC ( "%1 rows were copied to the project table!" ).arg ( n_copied_items ) ), 5000 );
		}
		case PS_FINISH:
		case PS_CANCEL:
			QUICK_PROJECT ()->selectDone ();
		break;
	}
}

void MainWindow::navigatePrev ()
{
	(void) navItems.prev ();
	mBtnNavNext->setEnabled ( true );
	displayNav ();
	mBtnNavPrev->setEnabled ( navItems.peekPrev () != nullptr );
}

void MainWindow::navigateNext ()
{
	(void) navItems.next ();
	mBtnNavPrev->setEnabled ( true );
	displayNav ();
	mBtnNavNext->setEnabled ( navItems.peekNext () != nullptr );
}

void MainWindow::insertNavItem ( vmListItem* item )
{
	if ( item != nullptr )
	{
		navItems.insert ( static_cast<uint>(navItems.currentIndex () + 1), item );
		mBtnNavPrev->setEnabled ( navItems.currentIndex () > 0 );
	}
}

void MainWindow::displayNav ()
{
	clientListItem* parentClient ( static_cast<clientListItem*>( navItems.current ()->relatedItem ( RLI_CLIENTPARENT ) ) );
	jobListItem* parentJob ( static_cast<jobListItem*>( navItems.current ()->relatedItem ( RLI_JOBPARENT ) ) );
	const bool bSelectClient ( parentClient != mClientCurItem );
	
	switch ( navItems.current ()->subType () )
	{
		case CLIENT_TABLE:
			displayClient ( parentClient, true );
		break;
		case JOB_TABLE:
			if ( bSelectClient )
				displayClient ( parentClient, true, static_cast<jobListItem*>( navItems.current () ) );
			else
				displayJob ( static_cast<jobListItem*>( navItems.current () ), true );
		break;
		case PAYMENT_TABLE:
			if ( bSelectClient )
				displayClient ( parentClient, true, parentJob );
			else
			{
				if ( parentJob != mJobCurItem )
					displayJob ( parentJob, true );
				else
					displayPay ( static_cast<payListItem*>( navItems.current () ), true ); 
			}
		break;
 		case PURCHASE_TABLE:
			if ( bSelectClient )
				displayClient ( parentClient, true, parentJob, static_cast<buyListItem*>( navItems.current () ) );
			else
			{
				if ( parentJob != mJobCurItem )
					displayJob ( parentJob, true );
				else
					displayPay ( static_cast<payListItem*>( navItems.current () ), true ); 
			}
		break;
		default: break;
	}
}

void MainWindow::insertEditItem ( vmListItem* item )
{
	if ( editItems.contains ( item ) == -1 )
		editItems.append ( item );
}

void MainWindow::removeEditItem ( vmListItem* item )
{
	m_crash->eliminateRestoreInfo ( item->crashID () );
	editItems.removeOne ( item );
}

void MainWindow::saveEditItems ()
{
	if ( !EXITING_PROGRAM || editItems.isEmpty () )
		return;
	
	QDialog* dlgSaveEditItems ( new QDialog ( this, Qt::Tool ) );
	dlgSaveEditItems->setWindowTitle ( TR_FUNC ( "Save these alterations?" ) );
	
	vmTableWidget* editItemsTable ( new vmTableWidget ( dlgSaveEditItems ) );
	editItemsTable->setIsPlainTable ();
	vmTableColumn* cols ( editItemsTable->createColumns ( 3 ) );
	cols[0].label = TR_FUNC ( "Pending operation" );
	cols[0].editable = false;
	cols[1].label = TR_FUNC ( "Status" );
	cols[1].editable = false;
	cols[2].label = TR_FUNC ( "Save?" );
	cols[2].wtype = WT_CHECKBOX;
	
	editItemsTable->initTable ( editItems.count () );
	spreadRow* s_row ( new spreadRow );
	s_row->column[0] = 0;
	s_row->column[1] = 1;
	s_row->column[2] = 2;
	vmListItem* edititem ( nullptr );
	
	for ( uint i ( 0 ); i < editItems.count (); ++i )
	{
		edititem = editItems.at ( i );
		s_row->row = static_cast<int>(i);
		s_row->field_value[0] = edititem->text ();
		s_row->field_value[1] = edititem->action () == ACTION_EDIT ? TR_FUNC ( "Editing" ) : TR_FUNC ( "New entry" );
		s_row->field_value[2] = CHR_ONE;
		editItemsTable->setRowData ( s_row );
	}
	delete s_row;
	editItemsTable->setEditable ( true );
	
	QPushButton* btnOK ( new QPushButton ( TR_FUNC ( "Continue" ) ) );
	connect ( btnOK, &QPushButton::clicked, this, [&] () { return dlgSaveEditItems->done ( 0 ); } );
	QVBoxLayout* vLayout ( new QVBoxLayout );
	vLayout->setMargin ( 2 );
	vLayout->setSpacing ( 2 );
	vLayout->addWidget ( editItemsTable, 2 );
	vLayout->addWidget ( btnOK, 0 );
	dlgSaveEditItems->setLayout ( vLayout );
	dlgSaveEditItems->exec ();
	
	for ( int i ( 0 ); i < editItemsTable->rowCount (); ++i )
	{
		if ( editItemsTable->sheetItem ( static_cast<uint>(i), 2 )->text () == CHR_ONE )
		{
			edititem = editItems.at ( i );
			switch ( edititem->subType () )
			{
				case CLIENT_TABLE:
					static_cast<void>(saveClient ( static_cast<clientListItem*>(edititem) ));
				break;
				case JOB_TABLE:
					static_cast<void>(saveJob ( static_cast<jobListItem*>(edititem) ));
				break;
				case PAYMENT_TABLE:
					static_cast<void>(savePay ( static_cast<payListItem*>(edititem) ));
				break;
				case PURCHASE_TABLE:
					static_cast<void>(saveBuy ( static_cast<buyListItem*>(edititem) ));
				break;
				default:
				break;
			}
		}
	}
	delete dlgSaveEditItems;
}
//--------------------------------------------WINDOWS-KEYS-EVENTS-EXTRAS-------------------------------------------------

//--------------------------------------------SLOTS-----------------------------------------------------------
void MainWindow::showTab ( const TAB_INDEXES ti )
{
	showNormal ();
	ui->tabMain->setCurrentIndex ( static_cast<int> ( ti ) );
}

void MainWindow::tabMain_currentTabChanged ( const int tab_idx )
{
	switch ( tab_idx )
	{
		case TI_CALENDAR:
			calMain_activated ( ui->calMain->selectedDate () );
			updateCalendarView ( mCalendarDate.year (), mCalendarDate.month () );
		break;
		case TI_INVENTORY:
			if ( INVENTORY ()->setupUI () )
				ui->tabInventory->setLayout ( INVENTORY ()->layout () );
			INVENTORY ()->table ()->setFocus ();
		break;
		case TI_SUPPLIES:
			if ( SUPPLIES ()->setupUI () )
				ui->tabSupplies->setLayout ( SUPPLIES ()->layout () );
			SUPPLIES ()->table ()->setFocus ();
		break;
		default:
		break;
	}
}

void MainWindow::on_btnQuickProject_clicked ()
{
	QUICK_PROJECT ()->setCallbackForDialogClosed ( [&] () { return quickProjectClosed (); } );
	QUICK_PROJECT ()->prepareToShow ( mJobCurItem->jobRecord () );
	QUICK_PROJECT ()->show ();
}

void MainWindow::receiveWidgetBack ( QWidget* widget )
{
	// if the UI changes, the widget positions might change too, se we always need to check back against ui_mainwindow.h to see if those numbers match
	// Still easier than having a function to query the layouts for the rows, columns and spans for the widgets: the UI does not change that often
	if ( widget == ui->txtJobReport )
	{
		ui->btnJobSeparateReportWindow->setChecked ( false );
		ui->gLayoutJobExtraInfo->addWidget ( ui->txtJobReport, 1, 6, 6, 3 ); // line copied from ui_mainwindow.h
		ui->txtJobReport->show ();
	}
	else if ( widget == ui->jobImageViewer )
	{
		ui->gLayoutJobExtraInfo->addWidget ( ui->frmJobPicturesControls, 8, 1, 1, 8 ); // line copied from ui_mainwindow.h
		ui->frmJobPicturesControls->show ();
		ui->hLayoutImgViewer->insertWidget ( 1, ui->jobImageViewer, 2 ); // insert after the first horizontal spacer to centralize it
		ui->jobImageViewer->show ();
	}
}

void MainWindow::quickProjectClosed ()
{
	ui->btnQuickProject->setChecked ( false );
	if ( mJobCurItem->dbRecID () == QUICK_PROJECT ()->jobID ().toUInt () )
	{
		const bool bReading ( mJobCurItem->action () == ACTION_READ );
		if ( bReading )
			static_cast<void>(editJob ( mJobCurItem ));
		ui->txtJobProjectID->setText ( QUICK_PROJECT ()->qpString (), true );
		ui->txtJobPrice->setText ( QUICK_PROJECT ()->totalPrice (), true );
		if ( bReading )
			static_cast<void>(saveJob ( mJobCurItem ));
	}
}

inline void MainWindow::btnReportGenerator_clicked ()
{ 
	if ( !EDITOR ()->isVisible () )
	{
		EDITOR ()->show ();
	} 
	EDITOR ()->activateWindow ();
	EDITOR ()->raise ();
}

inline void MainWindow::btnBackupRestore_clicked () { BACKUP ()->isVisible ()
			? BACKUP ()->hide () : BACKUP ()->showWindow (); }

inline void MainWindow::btnCalculator_clicked () { CALCULATOR ()->isVisible ()
			? CALCULATOR ()->hide () : CALCULATOR ()->showCalc ( emptyString, mapToGlobal ( ui->btnCalculator->pos () ) ); }

inline void MainWindow::btnEstimates_clicked () { if ( !mClientCurItem ) return; ESTIMATE ()->isVisible ()
			? ESTIMATE ()->hide () : ESTIMATE ()->showWindow ( recStrValue ( mClientCurItem->clientRecord (), FLD_CLIENT_NAME ) ); }

inline void MainWindow::btnCompanyPurchases_clicked () { COMPANY_PURCHASES ()->isVisible ()
			? COMPANY_PURCHASES ()->hide () : COMPANY_PURCHASES ()->show (); }

inline void MainWindow::btnConfiguration_clicked () { CONFIG ()->dialogWindow ()->isVisible ()
			? CONFIG ()->dialogWindow ()->hide () : CONFIG ()->dialogWindow ()->show () ; }

inline void MainWindow::btnExitProgram_clicked () { exitRequested ( true ); }
//--------------------------------------------SLOTS------------------------------------------------------------

//--------------------------------------------SEARCH------------------------------------------------------------
void MainWindow::on_txtSearch_textEdited ( const QString& text )
{
	ui->btnSearchStart->setEnabled ( text.length () >= 2 );
}

void MainWindow::on_btnSearchStart_clicked ()
{
	searchUI::init ();
	SEARCH_UI ()->prepareSearch ( ui->txtSearch->text (), ui->txtSearch );
	if ( SEARCH_UI ()->canSearch () )
		SEARCH_UI ()->search ();
	ui->btnSearchStart->setEnabled ( SEARCH_UI ()->canSearch () );
	ui->btnSearchCancel->setEnabled ( SEARCH_UI ()->isSearching () );
}

void MainWindow::on_btnSearchCancel_clicked ()
{
	if ( SEARCH_UI () )
	{
		SEARCH_UI ()->searchCancel ();
		ui->txtSearch->clear ();
		ui->btnSearchCancel->setEnabled ( false );
		ui->btnSearchStart->setEnabled ( false );
	}
}
//--------------------------------------------SEARCH------------------------------------------------------------

//--------------------------------------------TRAY-IMPORT-EXPORT-BACKUP--------------------------------
void MainWindow::trayMenuTriggered ( QAction* action )
{
	const int idx ( static_cast<vmAction*> ( action )->id () );

	if ( idx == 100 )
	{
		btnExitProgram_clicked ();
		return;
	}
	else if ( idx == 101 )
		trayActivated ( QSystemTrayIcon::Trigger );
}

void MainWindow::trayActivated ( QSystemTrayIcon::ActivationReason reason )
{
	switch ( reason )
	{
		case QSystemTrayIcon::Trigger:
		case QSystemTrayIcon::DoubleClick:
			if ( isMinimized () || isHidden () )
			{
				trayIconMenu->actions ().at ( 0 )->setText ( TR_FUNC ( "Hide" ) );
				showNormal ();
				activateWindow ();
				raise ();
				/*if ( reportWindow != nullptr ) {
					if ( reportWindow->isHidden () )
						reportWindow->showNormal ();
				}*/
			}
			else
			{
				trayIconMenu->actions ().at ( 0 )->setText ( TR_FUNC ( "Restore" ) );
				hide ();
				/*if ( reportWindow != nullptr )
					reportWindow->hide ();*/
			}
		break;
		default:
		break;
	}
}

void MainWindow::createTrayIcon ( const bool b_setup )
{
	if ( b_setup )
	{
		trayIcon = new QSystemTrayIcon ();
		trayIcon->setIcon ( ICON ( "vm-logo-22x22" ) );
		trayIcon->setToolTip ( windowTitle () );
		connect ( trayIcon, &QSystemTrayIcon::activated, this,
				  [&] ( QSystemTrayIcon::ActivationReason actReason ) { return trayActivated ( actReason ); } );

		trayIcon->show ();
		vmNotify::initNotify ();
	}
	else
	{
		trayIconMenu = new QMenu ( this );
		trayIconMenu->addAction ( new vmAction ( 101, TR_FUNC ( "Hide" ), this ) );
		trayIconMenu->addAction ( new vmAction ( 100, TR_FUNC ( "Exit" ), this ) );
		connect ( trayIconMenu, &QMenu::triggered, this, [&] ( QAction* act ) { return trayMenuTriggered ( act ); } );
		trayIcon->setContextMenu ( trayIconMenu );
	}
}
//--------------------------------------------TRAY-IMPORT-EXPORT-BACKUP--------------------------------

//--------------------------------------------SHARED-----------------------------------------------------------
void MainWindow::removeListItem ( vmListItem* item, const bool b_delete_item, const bool b_auto_select_another )
{
	vmListWidget* list ( item->listWidget () );
	list->setIgnoreChanges ( true );
	removeEditItem ( item ); // maybe item is in EDIT_ or ADD_ ACTION
	
	// do not propagate the status to DBRecord so that it might use the info from the previous 
	// state to perform some actions if needed
	item->setAction ( ACTION_DEL, true );
	if ( item->dbRec () && item->relation () == RLI_CLIENTITEM )
		item->dbRec ()->deleteRecord ();
	if ( b_auto_select_another )
		list->setIgnoreChanges ( false );
	list->removeItem ( item, b_delete_item );
}

void MainWindow::postFieldAlterationActions ( vmListItem* item )
{
	item->saveCrashInfo ( m_crash );
	const bool bCanSave ( true );//item->isGoodToSave () );
	switch ( item->subType () )
	{
		case CLIENT_TABLE:
			ui->btnClientSave->setEnabled ( bCanSave );
		break;
		case JOB_TABLE:
			ui->btnJobSave->setEnabled ( bCanSave );
		break;
		case PAYMENT_TABLE:
			ui->btnPayInfoSave->setEnabled ( bCanSave );
		break;
		case PURCHASE_TABLE:
			ui->btnBuySave->setEnabled ( bCanSave );
		break;
		default:
			break;
	}
	item->update ();
}
//--------------------------------------------SHARED-----------------------------------------------------------

//----------------------------------------------DATE-BTNS------------------------------------------------------
void MainWindow::updateProgramDate ()
{
	vmNumber::updateCurrentDate ();
	vmDateEdit::updateDateButtonsMenu ();
	ui->dteJobAddDate->setDate ( vmNumber::currentDate );
	if ( timerUpdate == nullptr )
	{
		timerUpdate = new QTimer ( this );
		connect ( timerUpdate, &QTimer::timeout, this, [&] () { return updateProgramDate (); } );
		timerUpdate->start ( 1000 * 60 * 5 ); // check date every five minutes (hibernation or suspend will thwart	the timer
	}
}
//----------------------------------------------CURRENT-DATE-BTNS------------------------------------------------------

//--------------------------------------------JOB---------------------------------------------------------------
void MainWindow::addJobPictures ()
{
	if ( mJobCurItem )
	{
		QString strPicturePath ( recStrValue ( mJobCurItem->jobRecord (), FLD_JOB_PICTURE_PATH ) );
		if ( strPicturePath.isEmpty () )
		{
			strPicturePath = recStrValue ( mJobCurItem->jobRecord (), FLD_JOB_PROJECT_ID );
			if ( !strPicturePath.isEmpty () )
				strPicturePath.prepend ( recStrValue ( mJobCurItem->jobRecord (), FLD_JOB_TYPE ) + QLatin1String ( " - " ) );
			else
			{
				strPicturePath = recStrValue ( mJobCurItem->jobRecord (), FLD_JOB_TYPE ) 
						+ QLatin1String ( " - " ) + vmNumber ( recStrValue ( mJobCurItem->jobRecord (), FLD_JOB_STARTDATE ), 
							VMNT_DATE, vmNumber::VDF_DB_DATE ).toDate ( vmNumber::VDF_FILE_DATE ) + CHR_F_SLASH;
			}
			
			const QString simpleJobBaseDir ( CONFIG ()->getProjectBasePath ( recStrValue ( mClientCurItem->clientRecord (), FLD_CLIENT_NAME ) )
											 + QStringLiteral ( "Serviços simples/" ) );
			if ( fileOps::exists ( simpleJobBaseDir ).isOff () )
				fileOps::createDir ( simpleJobBaseDir );
			strPicturePath.prepend ( simpleJobBaseDir );
			fileOps::createDir ( strPicturePath );
			if ( static_cast<TRI_STATE> ( mJobCurItem->action () >= ACTION_READ ) )
			{
				mJobCurItem->setAction ( ACTION_EDIT, true );
				ui->txtJobPicturesPath->setText ( strPicturePath, true );
				mJobCurItem->jobRecord ()->saveRecord ();
				mJobCurItem->setAction ( ACTION_READ );
			}
			else
				ui->txtJobPicturesPath->setText ( strPicturePath );
		}
		VM_NOTIFY ()->notifyMessage ( TR_FUNC ( "Waiting for " ) + CONFIG ()->fileManager () + TR_FUNC ( " to finish" ), 
									  TR_FUNC ( "Add some pictures to this job info data" ) );
		fileOps::executeWait ( strPicturePath, CONFIG ()->fileManager () );
		btnJobReloadPictures_clicked ();
	}
}

void MainWindow::btnJobReloadPictures_clicked ()
{
	ui->jobImageViewer->showImage ( -1, ui->txtJobPicturesPath->text () );
	ui->jobImageViewer->reload ();
	ui->cboJobPictures->clear ();
	ui->cboJobPictures->insertItems ( 0, ui->jobImageViewer->imagesList () );
	controlJobPictureControls ();
}

void MainWindow::showClientsYearPictures ( QAction* action )
{
	if ( action == jobsPicturesMenuAction )
		return;

	QString picturePath, str_lbljobpicture;
	const QString lastDir ( QString::number ( static_cast<vmAction*> ( action )->id () ) );
	ui->cboJobPictures->clear ();
	ui->cboJobPictures->clearEditText ();
	if ( lastDir == CHR_ZERO )
	{
		picturePath = recStrValue ( mJobCurItem->jobRecord (), FLD_JOB_PICTURE_PATH );
		str_lbljobpicture = TR_FUNC ( "Job pictures path:" );
	}
	else
	{
		picturePath = CONFIG ()->getProjectBasePath ( ui->txtClientName->text () );
		picturePath += jobPicturesSubDir + lastDir + CHR_F_SLASH;
		fileOps::createDir ( picturePath );
		str_lbljobpicture = QString ( TR_FUNC ( "Viewing clients pictures for year %1" ) ).arg ( lastDir );
	}
	ui->btnJobNextPicture->setEnabled ( true );
	ui->btnJobPrevPicture->setEnabled ( true );
	ui->jobImageViewer->showImage ( -1, picturePath );
	ui->cboJobPictures->insertItems ( 0, ui->jobImageViewer->imagesList () );
	ui->cboJobPictures->setEditText ( ui->jobImageViewer->imageFileName () );
	ui->lblJobPictures->setText ( str_lbljobpicture );
	ui->txtJobPicturesPath->setText ( picturePath );
	jobsPicturesMenuAction->setChecked ( false );
	jobsPicturesMenuAction = action;
	action->setChecked ( true );
}

void MainWindow::showDayPictures ( const vmNumber& date )
{
	const int npics ( ui->cboJobPictures->count () );
	if ( npics > 0 )
	{
		vmNumber::VM_DATE_FORMAT format ( vmNumber::VDF_DROPBOX_DATE );
		QString strDate;
		do
		{
			strDate = date.toDate ( format );
			for ( int i ( 0 ); i < npics; ++i )
			{
				if ( ui->cboJobPictures->itemText ( i ).startsWith ( strDate, Qt::CaseInsensitive ) )
				{
					ui->cboJobPictures->setCurrentIndex ( i );
					return;
				}
			}
			// Dropbox filename format did not yield any result. Try file format
			format = static_cast<vmNumber::VM_DATE_FORMAT>( static_cast<int>( format ) - 1 );
		} while ( format >= vmNumber::VDF_FILE_DATE );
		
		// Date by filename failed. Now we try the slower method. Look into the file properties and query its modification date.
		// Note: dropbox file format is preferred and is the default method since late 2013, save a few periods of exception
		// This last method will probably fail.
		QString filename;
		filename.reserve ( ui->txtJobPicturesPath->text ().count () + 20 );
		for ( int i ( 0 ); i < npics; ++i )
		{
			filename = ui->txtJobPicturesPath->text () + ui->cboJobPictures->itemText ( i );
			if ( date == fileOps::modifiedDate ( filename ) )
			{
				ui->cboJobPictures->setCurrentIndex ( i );
				return;
			}
		}
	}
}

void MainWindow::showJobImage ( const int index )
{
	ui->jobImageViewer->showSpecificImage ( index );
	showJobImageRequested ( index );
}

void MainWindow::showJobImageRequested ( const int index )
{
	ui->cboJobPictures->setCurrentIndex ( index );
	ui->btnJobPrevPicture->setEnabled ( index > 0 );
	ui->btnJobNextPicture->setEnabled ( index < ( ui->cboJobPictures->count () - 1 ) );
	if ( sepWin_JobPictures && index != -1 )
	{
		if ( !sepWin_JobPictures->isHidden () )
			sepWin_JobPictures->setWindowTitle ( ui->jobImageViewer->imageFileName () );
	}
}

void MainWindow::btnJobRenamePicture_clicked ( const bool checked )
{
	ui->cboJobPictures->setEditable ( checked );
	if ( checked )
	{
		if ( !ui->cboJobPictures->text ().isEmpty () )
		{
			ui->cboJobPictures->setEditable ( true );
			ui->cboJobPictures->lineEdit ()->selectAll ();
			ui->cboJobPictures->setFocus ();
		}
	}
	else
	{
		const int new_index ( ui->jobImageViewer->rename ( ui->cboJobPictures->text () ) );
		if ( new_index != -1 )
		{
			ui->cboJobPictures->removeItem ( ui->cboJobPictures->currentIndex () );
			ui->cboJobPictures->QComboBox::insertItem ( new_index, ui->cboJobPictures->text () );
			ui->cboJobPictures->setCurrentIndex ( new_index );
			ui->btnJobNextPicture->setEnabled ( new_index < ( ui->cboJobPictures->count () - 1 ) );
			ui->btnJobPrevPicture->setEnabled ( new_index >= 1 );
		}
	}
}

void MainWindow::showJobImageInWindow ( const bool maximized )
{
	if ( sepWin_JobPictures == nullptr )
	{
		sepWin_JobPictures = new separateWindow ( ui->jobImageViewer );
		sepWin_JobPictures->addToolBar ( ui->frmJobPicturesControls );
		sepWin_JobPictures->setCallbackForReturningToParent ( [&] ( QWidget* wdgt ) { return receiveWidgetBack ( wdgt ); } );
	}
	sepWin_JobPictures->showSeparate ( ui->jobImageViewer->imageFileName (), false, maximized ? Qt::WindowMaximized : Qt::WindowNoState );
	if ( !ui->btnJobSeparatePicture->isChecked () )
		ui->btnJobSeparatePicture->setChecked ( true );
}

void MainWindow::btnJobSeparateReportWindow_clicked ( const bool checked )
{
	if ( sepWin_JobReport == nullptr )
	{
		sepWin_JobReport = new separateWindow ( ui->txtJobReport );
		sepWin_JobReport->setCallbackForReturningToParent ( [&] ( QWidget* wdgt ) { return receiveWidgetBack ( wdgt ); } );
	}
	if ( checked )
	{
		QString title ( TR_FUNC ( "Job report " ) );
		if ( !ui->txtJobProjectID->text ().isEmpty () )
			title += ui->txtJobProjectID->text ();
		else
			title += TR_FUNC ( "for the client " ) + ui->txtClientName->text ();
		sepWin_JobReport->showSeparate ( title );
	}
	else
		sepWin_JobReport->returnToParent ();
}
//--------------------------------------------JOB------------------------------------------------------------

//--------------------------------------------BUY------------------------------------------------------------
void MainWindow::on_btnBuyCopyRows_clicked ()
{
	selectBuysItems ( PS_EXEC );
}
//-------------------------------------------BUY------------------------------------------------------------
