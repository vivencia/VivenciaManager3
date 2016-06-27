#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "global.h"
#include "completers.h"
#include "simplecalculator.h"
#include "servicesofferedui.h"
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

static uint leftPanel_multiplier ( 0 );

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
	  ui ( new Ui::MainWindow ), calMainView ( nullptr ),
	  menuJobDoc ( nullptr ), menuJobXls ( nullptr ), menuJobPdf ( nullptr ), subMenuJobPdfView ( nullptr ),
	  subMenuJobPdfEMail ( nullptr ), todoListWidget ( nullptr ), timerUpdate ( nullptr ),
	  sepWin_JobPictures ( nullptr ), sepWin_JobReport ( nullptr ), jobsPicturesMenuAction ( nullptr ),
	  clientTaskPanel ( nullptr ), mainTaskPanel ( nullptr ), selJob_callback ( nullptr ), mb_jobPosActions ( false ),
	  mClientCurItem ( nullptr ), mJobCurItem ( nullptr ), mPayCurItem ( nullptr ), mBuyCurItem ( nullptr )
{
	ui->setupUi ( this );
	setWindowIcon ( ICON ( "vm-logo-22x22.png" ) );
	setWindowTitle ( PROGRAM_NAME + QLatin1String ( " - " ) + PROGRAM_VERSION );

	clientsList = new vmListWidget;
	jobsList = new vmListWidget;
	paysList = new vmListWidget;
	buysList = new vmListWidget;
	buysJobList = ui->buysJobListWidget;

	createTrayIcon (); // create the icon for VM_NOTIFY () use
}

void MainWindow::continueStartUp ()
{
	setStatusBar ( nullptr );
	findCalendarsHiddenWidgets ();
	createTrayIcon ( false ); // create the menu for the icon
	setupCustomControls ();
	restoreLastSession ();
	updateProgramDate ();

	int coords[4] = { 0 };
	CONFIG ()->geometryFromConfigFile ( coords );
	setGeometry ( coords[0], coords[1], coords[2], coords[3] );
	show ();
	leftToRightWorkflowRatio = ( static_cast<double>( ui->scrollLeftPanel->widget ()->height () ) / static_cast<double>( ui->scrollWorkFlow->widget ()->height () ) );
	leftPanel_multiplier = static_cast<uint> ( std::ceil ( 1.0 / leftToRightWorkflowRatio ) );
	// connect for last to avoid unnecessary signal emission when we are removing tabs
	connect ( ui->tabMain, &QTabWidget::currentChanged, this, &MainWindow::tabMain_currentTabChanged );
	connect ( qApp, &QCoreApplication::aboutToQuit, this, [&] () { return exitRequested (); } );
	this->tabPaysLists_currentChanged ( 0 ); // update pay totals line edit text
}

MainWindow::~MainWindow ()
{
	EXITING_PROGRAM = true;
	BackupDialog::doDayBackup ();

	timerUpdate->stop ();
	for ( int i ( clientsList->count () - 1 ); i >= 0; --i )
		delete clientsList->item ( i );
	
	/*clientsList->setIgnoreChanges ( true );
	jobsList->setIgnoreChanges ( true );
	paysList->setIgnoreChanges ( true );
	buysList->setIgnoreChanges ( true );
	buysJobList->setIgnoreChanges ( true );
	ui->lstBuySuppliers->setIgnoreChanges ( true );
	paysOverdueList->setIgnoreChanges ( true );
	paysOverdueClientList->setIgnoreChanges ( true );
	ui->lstCalJobsDay->setIgnoreChanges ( true );
	ui->lstCalJobsWeek->setIgnoreChanges ( true );
	ui->lstCalJobsMonth->setIgnoreChanges ( true );
	ui->lstCalPaysDay->setIgnoreChanges ( true );
	ui->lstCalPaysWeek->setIgnoreChanges ( true );
	ui->lstCalPaysMonth->setIgnoreChanges ( true );
	ui->lstCalBuysDay->setIgnoreChanges ( true );
	ui->lstCalBuysWeek->setIgnoreChanges ( true );
	ui->lstCalBuysMonth->setIgnoreChanges ( true );
	crash->eliminateRestoreInfo ();

	for ( int i ( clientsList->count () - 1 ); i >= 0; --i )
		delete clientsList->item ( i );*/

	heap_del ( sepWin_JobPictures );
	heap_del ( sepWin_JobReport );
	heap_del ( ui );
	heap_del ( crash );
	heap_del ( mCal );

	cleanUpApp ();
}

//--------------------------------------------CLIENT------------------------------------------------------------
//TODO
void MainWindow::saveClientWidget ( vmWidget* widget, const uint id )
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
	clientsList->setSortingEnabled ( true );
	clientsList->sortItems ( 0 );
	clientsList->setCallbackForCurrentItemChanged ( [&] ( vmListItem* item ) { 
		return clientsListWidget_currentItemChanged ( item ); } );

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
		return btnClientAdd_clicked (); } );
	connect ( ui->btnClientEdit, &QToolButton::clicked, this, [&] () {
		return btnClientEdit_clicked (); } );
	connect ( ui->btnClientDel, &QToolButton::clicked, this, [&] () {
		return btnClientDel_clicked (); } );
	connect ( ui->btnClientSave, &QToolButton::clicked, this, [&] () {
		return btnClientSave_clicked (); } );
	connect ( ui->btnClientCancel, &QToolButton::clicked, this, [&] () {
		return btnClientCancel_clicked (); } );

	setTabOrder ( ui->txtClientZipCode, ui->contactsClientPhones->combo () );
	setTabOrder ( ui->contactsClientPhones->combo (), ui->contactsClientEmails->combo () );
	ui->dteClientDateFrom->setTabOrder ( this, ui->contactsClientEmails->combo (), ui->dteClientDateTo );
	ui->dteClientDateTo->setTabOrder ( this, ui->dteClientDateFrom, ui->chkClientActive );
}

void MainWindow::clientKeyPressedSelector ( const QKeyEvent* ke )
{
	if ( ke->key () == Qt::Key_Escape )
		btnClientCancel_clicked ();
	else {
		clientsList->setFocus (); // trigger any pending editing_Finished or textAltered event
		btnClientSave_clicked ();
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

bool MainWindow::displayClient ( clientListItem* client_item, const bool b_select, jobListItem* job_item, buyListItem* buy_item )
{
	// This function is called by user interaction and programatically. When called programatically, item may be nullptr
	if ( client_item ) {
		ui->tabMain->setCurrentIndex ( 0 );
		if ( client_item->loadData () ) {
			controlClientForms ( client_item );
			loadClientInfo ( client_item->clientRecord () );
			if ( client_item != mClientCurItem ) {
				if ( b_select )
					clientsList->setCurrentItem ( client_item, false );
				mClientCurItem = client_item;
				fillAllLists ( client_item );
				if ( !job_item )
					job_item = client_item->jobs->last ();
				displayJob ( job_item, true, buy_item );
				if ( tabPaysLists->currentIndex () == 1 )
					loadClientOverduesList ();
			}
		}
	}
	saveView ();
	return ( client_item != nullptr );
}

void MainWindow::loadClientInfo ( const Client* const client )
{
	if ( client != nullptr ) {
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

clientListItem* MainWindow::getClientItem ( const int id ) const
{
	if ( id >= 1 ) {
		const int n ( globalMainWindow->clientsList->count () );
		clientListItem* client_item ( nullptr );
		for ( int i ( 0 ); i < n; ++i ) {
			client_item = static_cast<clientListItem*> ( globalMainWindow->clientsList->item ( i ) );
			if ( client_item->dbRecID () == id )
				return client_item;
		}
	}
	return nullptr;
}

void MainWindow::fillAllLists ( const clientListItem* client_item )
{
	jobsList->clear (); // clears the list but does delete the items
	paysList->clear (); // clears the list but does delete the items
	buysList->clear (); // clears the list but does delete the items

	uint i ( 0 );
	if ( client_item )
	{
		jobListItem* job_item ( nullptr );
		for ( i = 0; i < client_item->jobs->count (); ++i )
		{
			job_item = client_item->jobs->at ( i );
			(void) job_item->loadData ();
			job_item->addToList ( jobsList );
		}

		payListItem* pay_item ( nullptr );
		vmNumber totalPays;
		for ( i = 0; i < client_item->pays->count (); ++i )
		{
			pay_item = client_item->pays->at ( i );
			(void) pay_item->loadData ();
			pay_item->addToList ( paysList );
			totalPays += pay_item->payRecord ()->price ( FLD_PAY_PRICE );
		}
		paysList->setProperty ( PROPERTY_TOTAL_PAYS, totalPays.toPrice () );

		buyListItem* buy_item ( nullptr );
		for ( i = 0; i < client_item->buys->count (); ++i )
		{
			buy_item = client_item->buys->at ( i );
			(void) buy_item->loadData ();
			buy_item->addToList ( buysList );
		}
	}

	jobsList->setIgnoreChanges ( false );
	paysList->setIgnoreChanges ( false );
	buysList->setIgnoreChanges ( false );
}

void MainWindow::btnClientAdd_clicked ()
{
	clientListItem* client_item ( new clientListItem );
	client_item->setRelation ( RLI_CLIENTITEM );
	client_item->setRelatedItem ( RLI_CLIENTPARENT, client_item );
	(void) client_item->loadData ();
	client_item->setAction ( ACTION_ADD, true );
	client_item->addToList ( clientsList );
	ui->txtClientName->setFocus ();
}

void MainWindow::btnClientEdit_clicked ()
{
	if ( mClientCurItem )
	{
		if ( mClientCurItem->action () == ACTION_READ )
		{
			mClientCurItem->setAction ( ACTION_EDIT, true );
			VM_NOTIFY ()->notifyMessage ( TR_FUNC ( "Editing client record" ),
						recStrValue ( mClientCurItem->clientRecord (), FLD_CLIENT_NAME ) );
			controlClientForms ( mClientCurItem );
			ui->txtClientName->setFocus ();
		}
	}
}

void MainWindow::btnClientDel_clicked ()
{
	if ( mClientCurItem )
	{
		clientListItem* client_item ( mClientCurItem );
		RECORD_ACTION cur_action ( client_item->action () );
		client_item->setAction ( ACTION_DEL, true );
		const QString text ( TR_FUNC ( "Are you sure you want to remove %1's record?" ) );
		if ( VM_NOTIFY ()->questionBox ( TR_FUNC ( "Question" ), text.arg ( DATA ()->currentClientName () ) ) )
			removeListItem ( client_item );
		else
			client_item->setAction ( cur_action, true );
	}
}

void MainWindow::btnClientSave_clicked ()
{
	if ( mClientCurItem )
	{
		if ( mClientCurItem->clientRecord ()->saveRecord () )
		{
			mClientCurItem->setAction ( mClientCurItem->clientRecord ()->action () );
			if ( mClientCurItem->dbRecID () == -1 )
				mClientCurItem->setDBRecID ( recIntValue ( mClientCurItem->clientRecord (), FLD_CLIENT_ID ) );
			crash->eliminateRestoreInfo ( mClientCurItem->crashID () );
			VM_NOTIFY ()->notifyMessage ( TR_FUNC ( "Record saved!" ), TR_FUNC ( "Client data saved!" ) );
			controlClientForms ( mClientCurItem );
			controlJobForms ( nullptr );
			saveView ();
		}
	}
}

void MainWindow::btnClientCancel_clicked ()
{
	if ( mClientCurItem )
	{
		switch ( mClientCurItem->action () )
		{
			case ACTION_ADD:
			{
				clientListItem* client_item ( mClientCurItem );
				mClientCurItem = nullptr;
				removeListItem ( client_item );
			}
			break;
			case ACTION_EDIT:
				mClientCurItem->setAction ( ACTION_REVERT, true );
				displayClient ( mClientCurItem );
			break;
			default:
			break;
		}
	}
}
//--------------------------------------------CLIENT------------------------------------------------------------

//-------------------------------------EDITING-FINISHED-CLIENT--------------------------------------------------
void MainWindow::txtClientName_textAltered ( const QString& text )
{
	const bool input_ok ( text.isEmpty () ? false : ( Client::clientID ( text ) == -1 ) );
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
	setRecValue ( mClientCurItem->clientRecord (), sender->id (), sender->text () );
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
	setRecValue ( mClientCurItem->clientRecord (), sender->id (),
				  static_cast<const vmDateEdit* const>( sender )->date ().toString ( DATE_FORMAT_DB ) );
	postFieldAlterationActions ( mClientCurItem );
}

void MainWindow::contactsClientAdd ( const QString& info, const vmWidget* const sender )
{
	setRecValue ( mClientCurItem->clientRecord (), sender->id (),
				  addToStringRecord ( recStrValue ( mClientCurItem->clientRecord (), sender->id () ), info ) );
	postFieldAlterationActions ( mClientCurItem );
}

void MainWindow::contactsClientDel ( const int idx, const vmWidget* const sender )
{
	stringRecord info_rec ( recStrValue ( mClientCurItem->clientRecord (), sender->id () ) );
	info_rec.removeField ( idx );
	setRecValue ( mClientCurItem->clientRecord (), sender->id (), info_rec.toString () );
	postFieldAlterationActions ( mClientCurItem );
}
//-------------------------------------EDITING-FINISHED-CLIENT---------------------------------------------------

//--------------------------------------------JOB------------------------------------------------------------
void MainWindow::saveJobWidget ( vmWidget* widget, const uint id )
{
	widget->setID ( id );
	jobWidgetList[id] = widget;
}

void MainWindow::showJobSearchResult ( vmListItem* item, const bool bshow )
{
	if ( item != nullptr  ) {
		jobListItem* job_item ( static_cast<jobListItem*> ( item ) );

		if ( bshow )
			displayClient ( static_cast<clientListItem*> ( item->relatedItem ( RLI_CLIENTPARENT ) ), true, job_item );

		for ( uint i ( 0 ); i < JOB_FIELD_COUNT; ++i ) {
			if ( job_item->searchFieldStatus ( i ) == SS_SEARCH_FOUND ) {
				if ( i == FLD_JOB_REPORT ) {
					for ( uint day ( 0 ); day < job_item->searchSubFields ()->count (); ++day ) {
						if ( job_item->searchSubFields ()->at ( day ) != job_item->searchSubFields ()->defaultValue () ) {
							static_cast<vmListItem*> ( ui->lstJobDayReport->item ( day ) )->highlight ( bshow ? vmBlue : vmDefault_Color );
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
		return on_btnJobAdd_clicked (); } );
	connect ( ui->btnJobEdit, &QToolButton::clicked, this, [&] () {
		return on_btnJobEdit_clicked (); } );
	connect ( ui->btnJobSave, &QToolButton::clicked, this, [&] () {
		return on_btnJobSave_clicked (); } );
	connect ( ui->btnJobCancel, &QToolButton::clicked, this, [&] () {
		return on_btnJobCancel_clicked (); } );
	connect ( ui->btnJobDel, &QToolButton::clicked, this, [&] () {
		return on_btnJobDel_clicked (); } );
	connect ( ui->btnQuickProject, &QPushButton::clicked, this, [&] () {
		return on_btnQuickProject_clicked (); } );

	jobsList->setCallbackForCurrentItemChanged ( [&] ( vmListItem* item ) {
		return jobsListWidget_currentItemChanged ( item ); } );
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
	
	ui->dteJobAddDate->setTabOrder ( this, ui->btnJobNextDay, ui->btnJobAddDay );
	setTabOrder ( ui->btnJobAddDay, ui->btnJobEditDay );
	setTabOrder ( ui->btnJobSeparatePicture, ui->jobImageViewer );
	setTabOrder ( ui->jobImageViewer, ui->btnPayInfoEdit );
}

void MainWindow::setUpJobButtons ( const QString& path )
{
	ui->btnJobOpenFileFolder->setEnabled ( false );
	ui->btnJobOpenDoc->setEnabled ( false );
	ui->btnJobOpenXls->setEnabled ( false );
	ui->btnJobOpenPdf->setEnabled ( false );
	if ( fileOps::exists ( path ).isOn () ) {
		ui->btnJobOpenFileFolder->setEnabled ( true );
		QStringList filesfound;
		QDir dir ( fileOps::dirFromPath ( path ) );
		dir.setFilter ( QDir::Files|QDir::Readable|QDir::Writable );
		dir.setSorting ( QDir::Type|QDir::DirsLast );
		dir.setNameFilters ( QStringList () << QStringLiteral ( "*.doc" ) << QStringLiteral ( "*.docx" )
											<< QStringLiteral ( "*.pdf" ) << QStringLiteral ( "*.xls" )
											<< QStringLiteral ( "*.xlsx" ) );
		filesfound = dir.entryList ();

		if ( !filesfound.isEmpty () ) {
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
			uint i ( 0 );
			do {
				menutext = static_cast<QString> ( filesfound.at ( i ) );
				if ( menutext.contains ( configOps::projectDocumentFormerExtension () ) ) {
					ui->btnJobOpenDoc->setEnabled ( true );
					if ( menuJobDoc == nullptr ) {
						menuJobDoc = new QMenu ( this );
						ui->btnJobOpenDoc->setMenu ( menuJobDoc );
						connect ( menuJobDoc, &QMenu::triggered, this, [&] ( QAction* action ) { return jobOpenProjectFile ( action ); } );
						connect ( ui->btnJobOpenDoc, &QToolButton::clicked, ui->btnJobOpenDoc, [&] ( const bool ) { return ui->btnJobOpenDoc->showMenu (); } );
					}
					menuJobDoc->addAction ( new vmAction ( 0, menutext, this ) );
				}
				else if ( menutext.contains ( configOps::projectPDFExtension () ) ) {
					ui->btnJobOpenPdf->setEnabled ( true );
					if ( menuJobPdf == nullptr ) {
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
				else {
					ui->btnJobOpenXls->setEnabled ( true );
					if ( menuJobXls == nullptr ) {
						menuJobXls = new QMenu ( this );
						ui->btnJobOpenXls->setMenu ( menuJobXls );
						connect ( menuJobXls, &QMenu::triggered, this, [&] ( QAction* action ) { return jobOpenProjectFile ( action ); } );
						connect ( ui->btnJobOpenXls, &QToolButton::clicked, ui->btnJobOpenXls, [&] ( const bool ) { return ui->btnJobOpenXls->showMenu (); } );
					}
					menuJobXls->addAction ( new vmAction ( 2, menutext, this ) );
				}
			} while ( ++i < unsigned ( filesfound.count () ) );
			filesfound.clear ();
		}
	}
}

void MainWindow::setupJobPictureControl ()
{
	QMenu* menuJobClientsYearPictures ( new QMenu );
	vmAction* action ( nullptr );
	const uint year ( static_cast<uint> ( vmNumber::currentDate.year () ) );
	for ( uint i ( 2008 ); i <= year; ++i )
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
			displayClient ( client_item, true, static_cast<jobListItem*>( cal_item->relatedItem ( RLI_DATEITEM ) ) );
		else
			displayJob ( static_cast<jobListItem*>( cal_item->relatedItem ( RLI_DATEITEM ) ), true );
		
		ui->lstJobDayReport->setCurrentRow ( cal_item->data ( Qt::UserRole ).toInt () - 1, true ); // select pertinent day
	}
}

void MainWindow::updateCalendarWithJobInfo ( Job* const job, const RECORD_ACTION action )
{
	vmListItem* item ( nullptr );
	int i ( ui->lstJobDayReport->count () - 1 );
	const uint n_days ( stringTable::countRecords ( recStrValue ( job, FLD_JOB_REPORT ) ) );
	vmNumber date;
	const vmNumber pricePerDay ( job->price ( FLD_JOB_PRICE ) / n_days );

	ui->lstJobDayReport->setIgnoreChanges ( true );

	switch ( action )
	{
		default: return;
		case ACTION_ADD:
		{
			for ( ; i >= 0 ; --i )
			{
				item = static_cast<vmListItem*>( ui->lstJobDayReport->item ( i ) );
				if ( item->data ( JILUR_REMOVED ).toBool () == true )
					continue;
				else
					item->setData ( JILUR_ADDED, false );
				date.fromTrustedStrDate ( item->data ( JILUR_DATE ).toString (), vmNumber::VDF_DB_DATE );
				mCal->addCalendarExchangeRule ( job->ce_list, CEAO_ADD_DATE1, date, vmNumber::emptyNumber, i + 1 );
				if ( !pricePerDay.isNull () )
					mCal->addCalendarExchangeRule ( job->ce_list, CEAO_ADD_PRICE_DATE1, date, pricePerDay );
			}
		}
		break;
		case ACTION_DEL:
		{
			for ( ; i >= 0 ; --i )
			{
				item = static_cast<vmListItem*>( ui->lstJobDayReport->item ( i ) );
				date.fromTrustedStrDate ( item->data ( JILUR_DATE ).toString (), vmNumber::VDF_DB_DATE );
				mCal->addCalendarExchangeRule ( job->ce_list, CEAO_DEL_DATE1, date, vmNumber::emptyNumber );
				if ( !pricePerDay.isNull () )
					mCal->addCalendarExchangeRule ( job->ce_list, CEAO_DEL_PRICE_DATE1, date, pricePerDay );
			}
		}
		break;
		case ACTION_EDIT:
		{
			if ( job->wasModified ( FLD_JOB_REPORT ) || job->wasModified ( FLD_JOB_PRICE ) )
			{
				const stringTable& originalJobReport ( recStrValueAlternate ( job, FLD_JOB_REPORT ) );
				const vmNumber oldPricePerDay ( vmNumber ( recStrValueAlternate ( job, FLD_JOB_PRICE ), VMNT_PRICE, 1 ) / originalJobReport.countRecords () );
				const stringRecord* jobDay ( nullptr );

				for ( ; i >= 0 ; --i )
				{
					item = static_cast<vmListItem*>( ui->lstJobDayReport->item ( i ) );
					if ( item->data ( JILUR_REMOVED ).toBool () == true )
					{
						jobDay = &originalJobReport.readRecord ( originalJobReport.findRecordRowByFieldValue ( item->data ( JILUR_DATE ).toString (), Job::JRF_DATE ) );
						if ( jobDay->isOK () )
						{
							// Use the recorded table instead of data from the list item because  there is a possibility that the item
							// had the date changed before the user decided it was all wrong and the info had to be removed
							date.fromTrustedStrDate ( jobDay->fieldValue ( Job::JRF_DATE ), vmNumber::VDF_DB_DATE );
							mCal->addCalendarExchangeRule ( job->ce_list, CEAO_DEL_DATE1, date, vmNumber::emptyNumber );
							if ( !oldPricePerDay.isNull () )
								mCal->addCalendarExchangeRule ( job->ce_list, CEAO_DEL_PRICE_DATE1, date, oldPricePerDay );
						}
						continue;
					}
					if ( item->data ( JILUR_ADDED ).toBool () == true )
					{
						item->setData ( JILUR_ADDED, false );
						date.fromTrustedStrDate ( item->data ( JILUR_DATE ).toString (), vmNumber::VDF_DB_DATE );
						mCal->addCalendarExchangeRule ( job->ce_list, CEAO_ADD_DATE1, date, vmNumber::emptyNumber, i + 1 );
						if ( !pricePerDay.isNull () )
							mCal->addCalendarExchangeRule ( job->ce_list, CEAO_ADD_PRICE_DATE1, date, pricePerDay );
					}
					date.fromTrustedStrDate ( item->data ( JILUR_DATE ).toString (), vmNumber::VDF_DB_DATE );
				}

				if ( pricePerDay != oldPricePerDay ) // even if total price was not changed, the number of days might have been
				{
					jobDay = &originalJobReport.first ();
					while ( jobDay->isOK () )
					{
						date.fromTrustedStrDate ( jobDay->fieldValue ( Job::JRF_DATE ), vmNumber::VDF_DB_DATE );
						if ( !oldPricePerDay.isNull () )
						{
							mCal->addCalendarExchangeRule ( job->ce_list, CEAO_DEL_PRICE_DATE1, date, oldPricePerDay );
							mCal->addCalendarExchangeRule ( job->ce_list, CEAO_ADD_PRICE_DATE1, date, pricePerDay );
						}
						jobDay = &originalJobReport.next ();
					}
				}
			}
		}
		break;
	}
	if ( !job->ce_list.isEmpty () )
		job->updateCalendarJobInfo ();
	ui->lstJobDayReport->setIgnoreChanges ( false );
}

void MainWindow::jobKeyPressedSelector ( const QKeyEvent* ke )
{
	switch ( ke->key () )
	{
		case Qt::Key_Enter:
		case Qt::Key_Return:
			clientsList->setFocus (); // trigger any pending editing_Finished or textAltered event
			on_btnJobSave_clicked ();
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
			on_btnJobCancel_clicked ();
		break;
	}
}

void MainWindow::jobsListWidget_currentItemChanged ( vmListItem* item )
{
	/*if ( !item ) {
		if ( JOB_PREV_ITEM->row () >= 0 ) // make sure PREV_ITEM exists
			item = JOB_PREV_ITEM;
		else { // PREV_ITEM was the deleted item. Select the last entry in the list
			item = static_cast<vmListItem*>( jobsList->item ( jobsList->count () - 1 ) );
			JOB_PREV_ITEM = nullptr;
		}
	}*/
	insertNavItem ( item );
	displayJob ( static_cast<jobListItem*> ( item ) );
}

void MainWindow::jobDayReportListWidget_currentItemChanged ( vmListItem* item )
{
	if ( item )
	{
		if ( !item->data ( JILUR_REMOVED ).toBool () )
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
			ui->txtJobReport->textEditWithCompleter::setText ( TR_FUNC ( " - THIS DAY WAS REMOVED - " ) );
	}
	else
	{
		ui->timeJobStart->setTime ( vmNumber () );
		ui->timeJobEnd->setTime ( vmNumber () );
		ui->txtJobTotalDayTime->setText ( emptyString );
		ui->txtJobWheather->setText ( emptyString );
		ui->txtJobReport->textEditWithCompleter::setText ( emptyString );
		ui->btnJobDelDay->setEnabled ( false );
		ui->btnJobCancelDelDay->setEnabled ( false );
	}
	ui->btnJobNextDay->setEnabled ( ui->lstJobDayReport->currentRow () < signed( ui->lstJobDayReport->count () - 1 ) );
	ui->btnJobPrevDay->setEnabled ( ui->lstJobDayReport->currentRow () > 0 );
	controlJobDayForms ( mJobCurItem );
}

void MainWindow::controlJobForms ( const jobListItem* const job_item )
{
	const triStateType editing_action ( job_item ? static_cast<TRI_STATE> ( job_item->action () >= ACTION_ADD ) : TRI_UNDEF );

	ui->cboJobType->setEditable ( editing_action.isOn () );
	ui->txtJobAddress->setEditable ( editing_action.isOn () );
	ui->txtJobPrice->setEditable ( editing_action.isOn () );
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

	ui->cboJobType->highlight ( editing_action.isOn () ? ( ui->cboJobType->text ().isEmpty () ? vmRed : vmDefault_Color ) : vmDefault_Color );
	ui->dteJobStart->highlight ( editing_action.isOn () ? ( vmNumber ( ui->dteJobStart->date () ).isDateWithinRange ( vmNumber::currentDate, 0, 4 ) ? vmDefault_Color : vmRed ) : vmDefault_Color );

	if ( jobsPicturesMenuAction != nullptr ) {
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

	ui->btnJobAddDay->setEnabled ( false );
	ui->btnJobEditDay->setEnabled ( b_hasDays );
	ui->btnJobPrevDay->setEnabled ( ui->lstJobDayReport-> currentRow () >= 1 );
	ui->btnJobNextDay->setEnabled ( ui->lstJobDayReport->currentRow () < signed( ui->lstJobDayReport->count () - 1 ) );
	ui->btnJobDelDay->setEnabled ( b_hasDays );
	ui->btnJobCancelDelDay->setEnabled ( b_hasDays ? ui->lstJobDayReport->currentItem ()->data ( JILUR_REMOVED ).toBool () : false );
	//ui->btnJobMachines->setEnabled ( editing_action.isOn () );
	ui->btnJobSeparateReportWindow->setEnabled ( job_item != nullptr );
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

void MainWindow::displayJob ( jobListItem* job_item, const bool b_select, buyListItem* buy_item )
{
	if ( job_item ) {
		if ( job_item->loadData () ) {
			controlJobForms ( job_item );
			loadJobInfo ( job_item->jobRecord () );
			if ( job_item != mJobCurItem ) {
				if ( b_select )
					jobsList->setCurrentItem ( job_item, false );
				mJobCurItem = job_item;
				displayPay ( job_item->payItem (), true );
				fillJobBuyList ( job_item );
				if ( !buy_item ) {
					if ( !job_item->buys->isEmpty () )
						buy_item = static_cast<buyListItem*>( job_item->buys->last ()->relatedItem ( RLI_CLIENTITEM ) );
				}
				displayBuy ( buy_item, true );
			}
		}
	}
	else {
		controlJobForms ( nullptr );
		loadJobInfo ( nullptr );
	}
	if ( b_select )
		saveView ();
}

void MainWindow::loadJobInfo ( const Job* const job )
{
	decodeJobReportInfo ( job );
	if ( job ) {
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
	else {
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

jobListItem* MainWindow::getJobItem ( const clientListItem* const parent_client, const int id ) const
{
	if ( id >= 1 && parent_client )
	{
		int i ( 0 );
		while ( i < static_cast<int> ( parent_client->jobs->count () ) )
		{
			if ( parent_client->jobs->at ( i )->dbRecID () == static_cast<int> ( id ) )
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
	if ( ui->cboJobPictures->count () == 0 ) {
		const uint year ( static_cast<uint> ( vmNumber::currentDate.year () ) );
		const QString picturePath ( CONFIG ()->getProjectBasePath ( ui->txtClientName->text () ) +
									jobPicturesSubDir );
		fileOps::createDir ( picturePath );
		QMenu* yearsMenu ( ui->btnJobClientsYearPictures->menu () );
		for ( uint i ( 0 ), y ( 2008 ); y <= year; ++i, ++y ) {
			yearsMenu->actions ().at ( i )->setEnabled (
				fileOps::isDir ( picturePath + QString::number ( y ) ).isOn () );
		}
	}
	ui->cboJobPictures->setEditText ( ui->jobImageViewer->imageFileName () );
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

	if ( job != nullptr ) {
		if ( job->jobItem ()->daysList->isEmpty () ) {
			const stringTable& str_report ( recStrValue ( job, FLD_JOB_REPORT ) );
			const stringRecord* rec ( nullptr );

			rec = &str_report.first ();
			if ( rec->isOK () ) {
				QString str_date;
				vmListItem* day_entry ( nullptr );
				uint day ( 1 ), jilur_idx ( 0 ) ;
				vmNumber totalDayTime;
				do {
					if ( rec->first () ) {
						str_date = vmNumber ( rec->curValue (), VMNT_DATE, vmNumber::VDF_DB_DATE ).toDate ( vmNumber::VDF_HUMAN_DATE );
						day_entry = new vmListItem ( QString::number ( day++ ) + lstJobReportItemPrefix + str_date ); //any type id
						day_entry->setData ( JILUR_DATE, str_date );
						jilur_idx = JILUR_START_TIME;
	
						while ( rec->next () ) {
							switch ( jilur_idx ) {
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
		else {
			for ( uint i ( 0 ); i < job->jobItem ()->daysList->count (); ++i )
				ui->lstJobDayReport->addItem ( job->jobItem ()->daysList->at ( i ) );
		}
		ui->lstJobDayReport->setIgnoreChanges ( false );
		ui->lstJobDayReport->setCurrentRow ( ui->lstJobDayReport->count () - 1, true );
	}
}

void MainWindow::updateJobInfo ( const QString& text, const uint user_role )
{
	vmListItem* day_entry ( ui->lstJobDayReport->currentItem () );
	if ( day_entry ) {
		const int cur_row ( ui->lstJobDayReport->currentRow () );
		day_entry->setData ( user_role, text );
		stringTable job_report ( recStrValue ( mJobCurItem->jobRecord (), FLD_JOB_REPORT ) );
		stringRecord rec ( job_report.readRecord ( cur_row ) );
		rec.changeValue ( user_role - Qt::UserRole, text );
		job_report.changeRecord ( cur_row, rec );

		setRecValue ( mJobCurItem->jobRecord (), FLD_JOB_REPORT, job_report.toString () );
		postFieldAlterationActions ( mJobCurItem );
	}
}

void MainWindow::updateJobInfoByRemoval ( const uint day, const bool bUndo )
{
	stringTable job_report ( recStrValue ( mJobCurItem->jobRecord (), FLD_JOB_REPORT ) );
	if ( !bUndo )
		job_report.removeRecord ( day );
	else { // Restore removed day to list of valid days. Only works before commiting (saving) changes
		const stringRecord deletedDayRecord ( stringTable (
					mJobCurItem->jobRecord ()->recordStrAlternate ( FLD_JOB_REPORT ) ).readRecord ( day ) );
		job_report.insertRecord ( day, deletedDayRecord );
	}
	setRecValue ( mJobCurItem->jobRecord (), FLD_JOB_REPORT, job_report.toString () );
	postFieldAlterationActions ( mJobCurItem );
}

void MainWindow::addJobPayment ( jobListItem* const job_item )
{
	//paysList->setIgnoreChanges ( true );
	payListItem* pay_item ( new payListItem );
	pay_item->setRelation ( RLI_CLIENTITEM );
	pay_item->setRelatedItem ( RLI_CLIENTPARENT, mClientCurItem );
	pay_item->setRelatedItem ( RLI_JOBPARENT, job_item );
	pay_item->createDBRecord ();
	pay_item->setAction ( ACTION_ADD, true );
	mPayCurItem = pay_item;
	pay_item->addToList ( paysList );
	job_item->setPayItem ( pay_item );
	//paysList->setIgnoreChanges ( false );
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
	
	if ( pay->saveRecord () ) {
		job_item->payItem ()->setAction ( pay->action () );
		job_item->payItem ()->setDBRecID ( recIntValue ( pay, FLD_PAY_ID ) );
		mClientCurItem->pays->append ( job_item->payItem () );
		crash->eliminateRestoreInfo ( job_item->payItem ()->crashID () );
		addPaymentOverdueItems ( job_item->payItem () );
		payOverdueGUIActions ( pay, ACTION_ADD );
		paysList->setCurrentItem ( job_item->payItem (), true );
	}
}

void MainWindow::removeJobPayment ( payListItem* pay_item )
{
	if ( pay_item ) {
		pay_item->setAction ( ACTION_DEL, true );
		payOverdueGUIActions ( pay_item->payRecord (), ACTION_DEL );
		updateTotalPayValue ( vmNumber::zeroedPrice, pay_item->payRecord ()->price ( FLD_PAY_PRICE ) );
		updateCalendarWithPayInfo ( pay_item->payRecord (), ACTION_DEL ); // remove pay info from calendar
		removePaymentOverdueItems ( pay_item );
		mClientCurItem->pays->remove ( paysList->currentRow () );
		removeListItem ( pay_item, false );
	}
}

void MainWindow::alterJobPayPrice ( jobListItem* const job_item )
{
	Payment* pay ( job_item->payItem ()->payRecord () );
	pay->setAction ( ACTION_EDIT );
	ui->txtPayTotalPrice->setText ( recStrValue ( job_item->jobRecord (), FLD_JOB_PRICE ), true );
	on_btnPayInfoSave_clicked ();
}

void MainWindow::calcJobTime ()
{
	vmNumber time ( ui->timeJobEnd->time () );
	time -= ui->timeJobStart->time ();
	ui->txtJobTotalDayTime->setText ( time.toTime ( vmNumber::VTF_FANCY ) ); // Just a visual feedback

	vmNumber total_time;
	for ( int i ( 0 ); i < ui->lstJobDayReport->count (); ++i ) {
		if ( i != ui->lstJobDayReport->currentRow () ) {
			total_time += vmNumber ( ui->lstJobDayReport->item ( i )->data (
										 JILUR_DAY_TIME ).toString (), VMNT_TIME, vmNumber::VTF_DAYS );
		}
		else
			ui->lstJobDayReport->item ( i )->setData ( JILUR_DAY_TIME, time.toTime () );
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
	for ( ; i >= 0 ; --i ) {
		item = ui->lstJobDayReport->item ( i );
		if ( !item->data ( JILUR_REMOVED ).toBool () ) {
			if ( str_date == item->data ( JILUR_DATE ).toString () )
				return TRI_ON;
		}
	}
	return ui->lstJobDayReport->isEmpty () ? TRI_UNDEF : TRI_OFF;
}

void MainWindow::rescanJobDaysList ()
{
	vmListItem* day_entry ( nullptr );
	bool b_reorder ( false );
	QString itemText;

	for ( uint i ( 0 ); i < (unsigned) ui->lstJobDayReport->count (); ) {
		day_entry = ui->lstJobDayReport->item ( i );
		if ( day_entry->data ( JILUR_REMOVED ).toBool () == true ) {
			ui->lstJobDayReport-> removeRow ( i, 1, true );
			b_reorder = true;
		}
		else {
			if ( b_reorder ) {
				itemText = day_entry->data ( Qt::DisplayRole ).toString (); //QListWidgetItem::text () is an inline function that calls data ()
				itemText.remove ( 0, itemText.indexOf ( QLatin1Char ( 'o' ) ) ); // Removes all characters before 'o' from lstJobReportItemPrefix ( "o dia")
				itemText.prepend ( QString::number ( i + 1 ) ); //Starts the new label with item index minus number of all items removed. The +1 is because day counting starts at 1, not 0
				day_entry->setText ( itemText, false, false, false );
			}
			++i; //move to the next item only when we did not remove an item
		}
	}
}

void MainWindow::fillCalendarJobsList ( const stringTable& jobids, vmListWidget* list )
{
	if ( jobids.countRecords () > 0 ) {
		stringRecord str_rec ( jobids.first () );
		if ( str_rec.isOK () ) {
			int jobid ( -1 );
			const QLatin1String dayStr ( " (Day(s) " );
			jobListItem* job_item ( nullptr ), *job_parent ( nullptr );
			clientListItem* client_parent ( nullptr );
			Job job;
			QString day, curLabel;
			list->setIgnoreChanges ( true );
			do
			{
				jobid =  str_rec.fieldValue ( 0 ).toInt () ;
				if ( job.readRecord ( jobid ) )
				{
					client_parent = getClientItem ( recIntValue ( &job, FLD_JOB_CLIENTID ) );
					job_parent = getJobItem ( client_parent, recIntValue ( &job, FLD_JOB_ID ) );
					
					if ( job_parent )
					{
						day = str_rec.fieldValue ( 2 );
						job_item = static_cast<jobListItem*>( job_parent->relatedItem ( RLI_DATEITEM ) );
						if ( !job_item )
						{
							job_item = new jobListItem;
							job_item->setRelation ( RLI_DATEITEM );
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
				str_rec = jobids.next ();
			} while ( str_rec.isOK () );
			list->setIgnoreChanges ( false );
		}
	}
}

void MainWindow::controlFormsForJobSelection ( const bool bStartSelection )
{
	ui->btnJobSelectJob->setEnabled ( bStartSelection );
	agClientOps->setEnabled ( !bStartSelection );
	agClientPays->setEnabled ( !bStartSelection );
	agClientBuys->setEnabled ( !bStartSelection );
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
	//ui->lstJobDayReport->setIgnoreChanges ( true );
	jobDayReportListWidget_currentItemChanged ( nullptr ); // clear forms

	const uint n_days ( ui->lstJobDayReport->count () + 1 );
	vmListItem* new_item ( new vmListItem ( QString::number ( n_days ) + lstJobReportItemPrefix + vmdate.toDate ( vmNumber::VDF_HUMAN_DATE ) ) );
	new_item->setData ( JILUR_ADDED, true );
	new_item->setData ( JILUR_REMOVED, false );
	new_item->addToList ( ui->lstJobDayReport );
	updateJobInfo ( vmdate.toDate ( vmNumber::VDF_DB_DATE ), JILUR_DATE );
	//Force adding currently displayed times to list item's data for future calculations. Also,
	// there is a great chance times for different job days will be similar, and this step
	// ensures they will be recorded even if the user does not change them
	//ui->timeJobStart->setTime ( ui->timeJobStart->time (), true );
	//ui->timeJobEnd->setTime ( ui->timeJobEnd->time (), true );

	if ( n_days == 1 ) {
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
	updateJobInfo ( vmdate.toDate ( vmNumber::VDF_DB_DATE ), JILUR_DATE );

	if ( ui->lstJobDayReport->currentRow () == 0 ) // first day
		ui->dteJobStart->setDate ( vmdate, true );
	else if ( ui->lstJobDayReport->currentRow () == (ui->lstJobDayReport->count () - 1) ) // last day
		ui->dteJobEnd->setDate ( vmdate, true );
}

void MainWindow::btnJobDelDay_clicked ()
{
	vmListItem* item ( ui->lstJobDayReport->currentItem () );
	if ( item ) {
		QFont fntStriked ( ui->lstJobDayReport->font () );
		fntStriked.setStrikeOut ( true );
		item->setFont ( fntStriked );
		item->setBackground ( QBrush ( Qt::red ) );
		item->setData ( JILUR_REMOVED, true );
		ui->btnJobCancelDelDay->setEnabled ( true );
		updateJobInfoByRemoval ( ui->lstJobDayReport->currentRow () );
		ui->btnJobDelDay->setEnabled ( false );
		ui->btnJobCancelDelDay->setEnabled ( true );
	}
}

void MainWindow::btnJobCancelDelDay_clicked ()
{
	vmListItem* item ( ui->lstJobDayReport->currentItem () );
	if ( item && item->data ( JILUR_REMOVED ).toBool () == true ) {
		QFont fntStriked ( ui->lstJobDayReport->font () );
		fntStriked.setStrikeOut ( false );
		item->setFont ( fntStriked );
		item->setBackground ( QBrush ( Qt::white ) );
		item->setData ( JILUR_REMOVED, false );
		updateJobInfoByRemoval ( ui->lstJobDayReport->currentRow (), true );
	}
}

void MainWindow::btnJobPrevDay_clicked ()
{
	int current_row ( ui->lstJobDayReport->currentRow () );
	if ( current_row >= 1 ) {
		ui->lstJobDayReport->setCurrentRow ( --current_row, true );
		ui->btnJobNextDay->setEnabled ( true );
	}
	else
		ui->btnJobPrevDay->setEnabled ( false );
}

void MainWindow::btnJobNextDay_clicked ()
{
	int current_row ( ui->lstJobDayReport->currentRow () );
	if ( current_row < signed( ui->lstJobDayReport->count () - 1 ) ) {
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
	switch ( static_cast<vmAction*> ( action )->id () ) {
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
	if ( fileOps::exists ( ui->txtJobProjectPath->text () ).isOn () ) {
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
	// Do not use sender->id (). Could use sender->parentWidget ()->id () but this method
	// is not shared between objects, only cboJobType uses it
	const bool b_ok ( !sender->text ().isEmpty () );
	mJobCurItem->setFieldInputStatus ( FLD_JOB_TYPE, b_ok, sender->vmParent () );
	if ( b_ok ) {
		setRecValue ( mJobCurItem->jobRecord (), FLD_JOB_TYPE, sender->text () );
		mJobCurItem->update ();
		postFieldAlterationActions ( mJobCurItem );
	}
	ui->btnJobSave->setEnabled ( mJobCurItem->isGoodToSave () );
}

void MainWindow::txtJob_textAltered ( const vmWidget* const sender )
{
	setRecValue ( mJobCurItem->jobRecord (), sender->id (), sender->text () );
	postFieldAlterationActions ( mJobCurItem );
	if ( sender->id () == FLD_JOB_PROJECT_PATH )
		setUpJobButtons ( sender->text () );
}

void MainWindow::txtJobWheather_textAltered ( const vmWidget* const sender )
{
	updateJobInfo ( sender->text (), JILUR_WHEATHER );
}

void MainWindow::txtJobTotalDayTime_textAltered ( const vmWidget* const sender )
{
	const vmNumber time ( sender->text (), VMNT_TIME );
	if ( time.isTime () ) {
		if ( ui->lstJobDayReport->count () == 1 ) {
			ui->txtJobTotalAllDaysTime->setText ( sender->text () );
			setRecValue ( mJobCurItem->jobRecord (), sender->id (), sender->text () );
			postFieldAlterationActions ( mJobCurItem );
		}
		else {
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
	if ( input_ok ) {
		setRecValue ( mJobCurItem->jobRecord (), sender->id (), date.toDate ( vmNumber::VDF_DB_DATE ) );
		mJobCurItem->update ();
	}
	if ( sender->id () == FLD_JOB_STARTDATE ) {
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
void MainWindow::savePayWidget ( vmWidget* widget, const uint id )
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
	for ( uint i ( 0 ); i < PAY_FIELD_COUNT; ++i ) {
		if ( pay_item->searchFieldStatus ( i ) == SS_SEARCH_FOUND ) {
			if ( i != FLD_PAY_INFO )
				payWidgetList.at ( i )->highlight ( bshow ? vmBlue : vmDefault_Color, SEARCH_UI ()->searchTerm () );
			else {
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

	savePayWidget ( ui->txtPayTotalPayments, FLD_PAY_TOTALPAYS );
	ui->txtPayTotalPayments->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
		return txtPay_textAltered ( sender ); } );

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

	paysList->setCallbackForCurrentItemChanged ( [&] ( vmListItem* item ) {
		return paysListWidget_currentItemChanged ( item ); } );

	( void )vmTableWidget::createPayHistoryTable ( ui->tablePayments );
	savePayWidget ( ui->tablePayments, FLD_PAY_INFO );
	//ui->tablePayments->setParentLayout ( ui->gLayoutPayForms );
	ui->tablePayments->setUtilitiesPlaceLayout ( ui->layoutPayUtilities );
	ui->tablePayments->setCallbackForCellChanged ( [&] ( const vmTableItem* const item ) {
		return interceptPaymentCellChange ( item ); } );
	ui->tablePayments->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
		return payKeyPressedSelector ( ke ); } );

	paysOverdueList = new vmListWidget;
	paysOverdueList->setCallbackForCurrentItemChanged ( [&] ( vmListItem* item ) {
		return paysOverdueListWidget_currentItemChanged ( item ); } );
	paysOverdueClientList = new vmListWidget;
	paysOverdueClientList->setCallbackForCurrentItemChanged ( [&] ( vmListItem* item ) {
		return paysOverdueClientListWidget_currentItemChanged ( item ); } );

	tabPaysLists = new QTabWidget ( this );
	tabPaysLists->addTab ( paysList, TR_FUNC ( "Client pays" ) );
	tabPaysLists->addTab ( paysOverdueClientList, TR_FUNC ( "Client overdue pays" ) );
	tabPaysLists->addTab ( paysOverdueList, TR_FUNC ( "All overdue pays" ) );
	tabPaysLists->setCurrentIndex ( 0 );
	connect ( tabPaysLists, &QTabWidget::currentChanged, this, [&] ( const int index ) { return tabPaysLists_currentChanged ( index ); } );

	QLabel* lblPayTotal ( new QLabel ( TR_FUNC ( "Total: " ) ) );
	lblPayTotal->setToolTip ( TR_FUNC ( "Displays the total payment amount for the current selected tab category" ) );
	txtPayTotals = new vmLineEdit;
	frmPayTotals = new QFrame;
	QHBoxLayout* lTotals ( new QHBoxLayout );
	lTotals->setMargin ( 2 );
	lTotals->setSpacing ( 2 );
	lTotals->addWidget ( lblPayTotal );
	lTotals->addWidget ( txtPayTotals );
	frmPayTotals->setLayout ( lTotals );
	frmPayTotals->setMinimumSize ( 200, 30 );
	
	connect ( ui->btnPayPayReceipt, &QToolButton::clicked, this, [&] () { return
			EDITOR ()->startNewReport ()->createPaymentStub ( mPayCurItem->dbRecID () ); } );
	connect ( ui->btnPayPaymentsReport, &QToolButton::clicked, this, [&] () { return
			EDITOR ()->startNewReport ()->createJobReport ( mClientCurItem->dbRecID (), true ); } );
	connect ( ui->btnPayPaymentsReportOnlyUnPaid, &QToolButton::clicked, this, [&] () { return
			EDITOR ()->startNewReport ()->createJobReport ( mClientCurItem->dbRecID (), false ); } );
	connect ( ui->btnPayInfoDel, &QToolButton::clicked, this, [&] () { return on_btnPayInfoDel_clicked (); } );
	connect ( ui->btnPayInfoEdit, &QToolButton::clicked, this, [&] () { return on_btnPayInfoEdit_clicked (); } );
	connect ( ui->btnPayInfoSave, &QToolButton::clicked, this, [&] () { return on_btnPayInfoSave_clicked (); } );
	connect ( ui->btnPayInfoCancel, &QToolButton::clicked, this, [&] () { return on_btnPayInfoCancel_clicked (); } );
}

void MainWindow::displayPayFromCalendar ( vmListItem* cal_item )
{
	clientListItem* client_item ( getClientItem ( cal_item->data ( Qt::UserRole + 1 ).toInt () ) );
	if ( client_item ) {
		payListItem* pay_item ( getPayItem ( client_item, cal_item->data ( Qt::UserRole ).toInt () ) );
		if ( pay_item ) {
			jobListItem* job_item ( static_cast<jobListItem*> ( pay_item->relatedItem ( RLI_JOBPARENT ) ) );
			displayClient ( client_item, true, job_item );
		}
	}
}

void MainWindow::paysListWidget_currentItemChanged ( vmListItem* item )
{
	insertNavItem ( item );
	displayJob ( static_cast<jobListItem*> ( item->relatedItem ( RLI_JOBPARENT ) ), true );
}

void MainWindow::paysOverdueListWidget_currentItemChanged ( vmListItem* item )
{
	payListItem* pay_item ( static_cast<payListItem*> ( item ) );
	/* displayClient will ignore a chunk of calls if pay_item->client_parent == mClientCurItem, which it is and supposed to.
	 * Force reselection of current client
	 **/
	
	mClientCurItem = nullptr;
	displayClient ( static_cast<clientListItem*> ( pay_item->relatedItem ( RLI_CLIENTPARENT ) ), true,
		static_cast<jobListItem*> ( pay_item->relatedItem ( RLI_JOBPARENT ) ) );
}

void MainWindow::paysOverdueClientListWidget_currentItemChanged ( vmListItem* item )
{
	payListItem* pay_item ( static_cast<payListItem*> ( item ) );
	displayJob ( static_cast<jobListItem*> ( pay_item->relatedItem ( RLI_JOBPARENT ) ), true );
}


void MainWindow::updateCalendarWithPayInfo ( Payment* const pay, const RECORD_ACTION action )
{
	if ( action != ACTION_DEL ) {
		if ( !pay->wasModified ( FLD_PAY_INFO ) )
			return;
	}

	stringRecord rec;
	vmNumber date, price;

	if ( action != ACTION_ADD ) { // block to remove old info - only when deleting or editing
		// Remove old occurences. Easier than trying to compare what's the same, what's not, what's removed, what's addes
		const stringTable origTable ( recStrValueAlternate ( pay, FLD_PAY_INFO ) );
		if ( origTable.firstStr () ) {
			do {
				rec.fromString ( origTable.curRecord () );
				if ( rec.first () ) {
					do {
						date.fromStrDate ( rec.fieldValue ( PHR_DATE ) );
						mCal->addCalendarExchangeRule ( pay->ce_list, CEAO_DEL_DATE1, date, vmNumber::emptyNumber );
						price.fromStrPrice ( rec.fieldValue ( PHR_VALUE ) );
						mCal->addCalendarExchangeRule ( pay->ce_list, CEAO_DEL_PRICE_DATE1, date, price );
						if ( rec.fieldValue ( PHR_PAID ) == CHR_ONE ) {	// use payment
							date.fromStrDate ( rec.fieldValue ( PHR_USE_DATE ) );
							mCal->addCalendarExchangeRule ( pay->ce_list, CEAO_DEL_DATE2, date, vmNumber::emptyNumber );
							mCal->addCalendarExchangeRule ( pay->ce_list, CEAO_DEL_PRICE_DATE2, date, price );
						}
					} while ( rec.next () );
				}
			} while ( origTable.nextStr () );
		}
	}

	if ( action != ACTION_DEL ) { // Now, insert new occurences, only when editing or inserting
		const stringTable newTable ( recStrValue ( pay, FLD_PAY_INFO ) );
		if ( newTable.firstStr () ) {
			do {
				rec.fromString ( newTable.curRecord () );
				if ( rec.first () ) {
					do {
						date.fromStrDate ( rec.fieldValue ( PHR_DATE ) );
						mCal->addCalendarExchangeRule ( pay->ce_list, CEAO_ADD_DATE1, date, vmNumber::emptyNumber );
						price.fromStrPrice ( rec.fieldValue ( PHR_VALUE ) );
						mCal->addCalendarExchangeRule ( pay->ce_list, CEAO_ADD_PRICE_DATE1, date, price );
						if ( rec.fieldValue ( PHR_PAID ) == CHR_ONE ) {
							date.fromStrDate ( rec.fieldValue ( PHR_USE_DATE ) );
							mCal->addCalendarExchangeRule ( pay->ce_list, CEAO_ADD_DATE2, date, vmNumber::emptyNumber );
							mCal->addCalendarExchangeRule ( pay->ce_list, CEAO_ADD_PRICE_DATE2, date, price );
						}
						APP_COMPLETERS ()->updateCompleter ( rec.fieldValue ( PHR_METHOD ), vmCompleters::PAYMENT_METHOD );
						APP_COMPLETERS ()->updateCompleter ( rec.fieldValue ( PHR_ACCOUNT ), vmCompleters::ACCOUNT );
					} while ( rec.next () );
				}
			} while ( newTable.nextStr () );
		}
	}
	if ( !pay->ce_list.isEmpty () )
		pay->updateCalendarPayInfo ();
}

void MainWindow::addPaymentOverdueItems ( payListItem* pay_item )
{
	bool bOldState ( true );
	if ( tabPaysLists->currentIndex () == 1 ) {
		bOldState = paysOverdueClientList->isIgnoringChanges ();
		paysOverdueClientList->setIgnoreChanges ( true );
		payListItem* pay_item_overdue_client ( new payListItem );
		pay_item_overdue_client->setRelation ( RLI_DATEITEM );
		pay_item->syncSiblingWithThis ( pay_item_overdue_client );
		pay_item_overdue_client->update ();
		pay_item_overdue_client->addToList ( paysOverdueClientList );
		paysOverdueClientList->setIgnoreChanges ( bOldState );
	}
	
	if ( paysOverdueList->count () != 0 ) {
		bOldState = paysOverdueList->isIgnoringChanges ();
		paysOverdueList->setIgnoreChanges ( true );
		payListItem* pay_item_overdue ( new payListItem );
		pay_item_overdue->setRelation ( RLI_EXTRAITEM );
		pay_item->syncSiblingWithThis ( pay_item_overdue );
		pay_item_overdue->update ();
		pay_item_overdue->addToList ( paysOverdueList );
		paysOverdueList->setIgnoreChanges ( bOldState );
	}
}

void MainWindow::removePaymentOverdueItems ( payListItem* pay_item )
{
	bool bOldState ( true );
	if ( pay_item->relatedItem ( RLI_DATEITEM ) ) {
		bOldState = paysOverdueClientList->isIgnoringChanges ();
		paysOverdueClientList->setIgnoreChanges ( true );
		paysOverdueClientList->removeRow ( pay_item->relatedItem ( RLI_DATEITEM )->row (), 1, true );
		paysOverdueClientList->setIgnoreChanges ( bOldState );
	}
	
	if ( pay_item->relatedItem ( RLI_EXTRAITEM ) ) {
		bOldState = paysOverdueList->isIgnoringChanges ();
		paysOverdueList->setIgnoreChanges ( true );
		paysOverdueList->removeRow ( pay_item->relatedItem ( RLI_EXTRAITEM )->row (), 1, true );
		paysOverdueList->setIgnoreChanges ( bOldState );
	}
}

void MainWindow::updateTotalPayValue ( const vmNumber& nAdd, const vmNumber& nSub )
{
	vmNumber totalPrice;
	totalPrice.fromTrustedStrPrice ( paysList->property ( PROPERTY_TOTAL_PAYS ).toString (), false );
	totalPrice -= nSub;
	totalPrice += nAdd;
	paysList->setProperty ( PROPERTY_TOTAL_PAYS, totalPrice.toPrice () );
	if ( tabPaysLists->currentIndex () == 0 )
		txtPayTotals->setText ( totalPrice.toPrice () );	
}

void MainWindow::updatePayOverdueTotals ( const vmNumber& nAdd, const vmNumber& nSub )
{
	vmNumber totalPrice;
	if ( paysOverdueClientList->count () != 0 ) {
		totalPrice.fromTrustedStrPrice ( paysOverdueClientList->property ( PROPERTY_TOTAL_PAYS ).toString (), false );
		totalPrice -= nSub;
		totalPrice += nAdd;
		paysOverdueClientList->setProperty ( PROPERTY_TOTAL_PAYS, totalPrice.toPrice () );
		if ( tabPaysLists->currentIndex () == 1 )
			txtPayTotals->setText ( totalPrice.toPrice () );
	}
	if ( paysOverdueList->count () != 0 ) {
		totalPrice.fromTrustedStrPrice ( paysOverdueList->property ( PROPERTY_TOTAL_PAYS ).toString (), false );
		totalPrice -= nSub;
		totalPrice += nAdd;
		paysOverdueList->setProperty ( PROPERTY_TOTAL_PAYS, totalPrice.toPrice () );
		if ( tabPaysLists->currentIndex () == 2 )
			txtPayTotals->setText ( totalPrice.toPrice () );
	}
}

void MainWindow::payOverdueGUIActions ( Payment* const pay, const RECORD_ACTION new_action )
{
	bool bIsOverdue ( false );
	if ( pay ) {
		const int overdueIdx ( recStrValue ( pay, FLD_PAY_OVERDUE ).toInt () );
		if ( new_action != ACTION_NONE ) {
			vmNumber new_price, old_price;
			vmNumber new_paid, old_paid;
			if ( overdueIdx == 2 && new_action == ACTION_EDIT )
				*(const_cast<RECORD_ACTION*>(&new_action)) = ACTION_DEL;
			switch ( new_action ) {
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
				default: break; // avoid compiler warning/error
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
		else { //reading record. Only update the one control
			ui->chkPayOverdue->setChecked ( overdueIdx == 2 );
			bIsOverdue = ( overdueIdx == 1 );
		}
		ui->chkPayOverdue->setToolTip ( chkPayOverdueToolTip_overdue[overdueIdx] );
		pay->payItem ()->update ();
	}
	else {
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

	ui->btnPayPaymentsReport->setEnabled ( paysList->count () > 0 );
	ui->btnPayPaymentsReportOnlyUnPaid->setEnabled ( paysList->count () > 0 );
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

void MainWindow::displayPay ( payListItem* pay_item, const bool b_select )
{
	if ( pay_item ) {
		if ( pay_item->loadData () ) {
			controlPayForms ( pay_item );
			loadPayInfo ( pay_item->payRecord () );
			if ( pay_item != mPayCurItem ) {
				if ( b_select )
					paysList->setCurrentItem ( pay_item, false );
				mPayCurItem = pay_item;
			}
		}
	}
	else {
		loadPayInfo ( nullptr );
		controlPayForms ( nullptr );
	}
	if ( b_select )
		saveView ();
}

void MainWindow::loadPayInfo ( const Payment* const pay )
{
	if ( pay != nullptr ) {
		// See comments under loadBuyInfo for the rationale of a table's state change calls
		ui->tablePayments->setEditable ( false );
		ui->tablePayments->clear ( true );

		ui->txtPayID->setText ( recStrValue ( pay, FLD_PAY_ID ) );
		ui->txtPayTotalPrice->setText ( pay->action () != ACTION_ADD ? recStrValue ( pay, FLD_PAY_PRICE ) :
					( mJobCurItem ? recStrValue ( mJobCurItem->jobRecord (), FLD_JOB_PRICE ) :
						  vmNumber::zeroedPrice.toPrice () ) );
		ui->txtPayTotalPayments->setText ( recStrValue ( pay, FLD_PAY_TOTALPAYS ) );
		ui->txtPayTotalPaid->setText ( recStrValue ( pay, FLD_PAY_TOTALPAID ) );
		ui->txtPayObs->setText ( recStrValue ( pay, FLD_PAY_OBS ) );
		ui->tablePayments->loadFromStringTable ( recStrValue ( pay, FLD_PAY_INFO ) );

		payOverdueGUIActions ( const_cast<Payment*>( pay ), ACTION_NONE );
	}
	else {
		ui->tablePayments->setEditable ( false );
		ui->tablePayments->clear ( true );
		ui->txtPayID->setText ( QStringLiteral ( "-1" ) );
		ui->txtPayTotalPrice->setText ( vmNumber::zeroedPrice.toPrice () );
		ui->txtPayTotalPayments->setText ( CHR_ZERO );
		ui->txtPayObs->setText ( QStringLiteral ( "N/A" ) );
		ui->txtPayTotalPaid->setText ( vmNumber::zeroedPrice.toPrice () );
	}
}

payListItem* MainWindow::getPayItem ( const clientListItem* const parent_client, const int id ) const
{
	if ( id >= 1 && parent_client ) {
		int i ( 0 );
		while ( i < static_cast<int> ( parent_client->pays->count () ) )
		{
			if ( parent_client->pays->at ( i )->dbRecID () == static_cast<int> ( id ) )
				return parent_client->pays->at ( i );
			++i;
		}
	}
	return nullptr;
}

void MainWindow::fillCalendarPaysList ( const stringTable& payids, vmListWidget* list, const bool use_date )
{
	if ( payids.countRecords () > 0 ) {
		vmListItem* pay_item ( nullptr );
		Payment pay;
		QString label;
		bool ok ( true );
		int payid ( payids.first ().fieldValue ( 0 ).toInt ( &ok ) );
		if ( ok ) {
			do {
				if ( pay.readRecord ( payid ) ) {
					label = Client::clientName ( recStrValue ( &pay, FLD_PAY_CLIENTID ) ) %
							CHR_SPACE % CHR_HYPHEN % CHR_SPACE %
							recStrValue ( &pay, FLD_PAY_TOTALPAID ) %
							( use_date ? TR_FUNC ( " (use)" ) : TR_FUNC ( " (pay)" ) );
					pay_item = new vmListItem ( label );
					pay_item->setData ( Qt::UserRole, payid );
					pay_item->setData ( Qt::UserRole + 1, recStrValue ( &pay, FLD_PAY_CLIENTID ) );
					list->addItem ( pay_item );
				}
				payid = payids.next ().fieldValue ( 0 ).toInt ( &ok );
			} while ( ok );
		}
	}
}

void MainWindow::loadClientOverduesList ()
{
	if ( !mClientCurItem ) return;

	if ( paysOverdueClientList->count () != 0 ) {
		if ( static_cast<clientListItem*> ( static_cast<payListItem*>( paysOverdueClientList->item ( 0 ) )->relatedItem ( RLI_CLIENTPARENT ) ) == mClientCurItem )
			return;
		else
			paysOverdueClientList->clear ();
	}
	paysOverdueClientList->setIgnoreChanges ( true );
	payListItem* pay_item_overdue_client ( nullptr ), *pay_item ( nullptr );
	vmNumber totalOverdue;
	for ( uint x ( 0 ); x < mClientCurItem->pays->count (); ++x ) {
		pay_item = mClientCurItem->pays->at ( x );
		pay_item_overdue_client = static_cast<payListItem*>( pay_item->relatedItem ( RLI_DATEITEM ) );
		if ( !pay_item_overdue_client ) {
			if ( pay_item->loadData () ) {
				if ( recStrValue ( pay_item->payRecord (), FLD_PAY_OVERDUE ) == CHR_ONE ) {
					pay_item_overdue_client = new payListItem;
					pay_item_overdue_client->setRelation ( RLI_DATEITEM );
					pay_item->syncSiblingWithThis ( pay_item_overdue_client );
					pay_item_overdue_client->update ();
					pay_item_overdue_client->addToList ( paysOverdueClientList );
					totalOverdue += pay_item->payRecord ()->price ( FLD_PAY_OVERDUE_VALUE );
				}
				else
					continue;
			}
			else
				continue;
		}
		pay_item_overdue_client->update ();
		paysOverdueClientList->setProperty ( PROPERTY_TOTAL_PAYS, totalOverdue.toPrice () );
		txtPayTotals->setText ( totalOverdue.toPrice () );
	}
}

void MainWindow::loadAllOverduesList ()
{
	if ( paysOverdueList->count () == 0 ) {
		payListItem* pay_item_overdue ( nullptr ), *pay_item ( nullptr );
		clientListItem* client_item ( nullptr );
		const uint n ( clientsList->count () );
		vmNumber totalOverdue;
		for ( uint i ( 0 ); i < n; ++i ) {
			client_item = static_cast<clientListItem*> ( clientsList->item ( i ) );
			for ( uint x ( 0 ); x < client_item->pays->count (); ++x ) {
				pay_item = client_item->pays->at ( x );
				if ( pay_item->loadData () ) {
					if ( recStrValue ( pay_item->payRecord (), FLD_PAY_OVERDUE ) == CHR_ONE ) {
						pay_item_overdue = new payListItem;
						pay_item_overdue->setRelation ( RLI_EXTRAITEM );
						pay_item->syncSiblingWithThis ( pay_item_overdue );
						pay_item_overdue->update ();
						pay_item_overdue->addToList ( paysOverdueList );
						totalOverdue += pay_item->payRecord ()->price ( FLD_PAY_OVERDUE_VALUE );
						qDebug () << pay_item->payRecord ()->price ( FLD_PAY_OVERDUE_VALUE ).toPrice();
					}
				}
			}
		}
		paysOverdueList->setProperty ( PROPERTY_TOTAL_PAYS, totalOverdue.toPrice () );
		txtPayTotals->setText ( totalOverdue.toPrice () );
	}
}

void MainWindow::interceptPaymentCellChange ( const vmTableItem* const item )
{
	const uint row ( static_cast<uint>( item->row () ) );
	if ( row == ui->tablePayments->totalsRow () )
		return;

	const uint col ( static_cast<uint>( item->column () ) );
	const bool b_paid ( ui->tablePayments->sheetItem ( row, PHR_PAID )->text () == CHR_ONE );
	bool bSetUseDate ( false );

	triStateType input_ok;
	switch ( col ) {
		case PHR_DATE:
			bSetUseDate = true;
		case PHR_USE_DATE:
		{
			input_ok = ui->tablePayments->isRowEmpty ( row );
			if ( input_ok.isOff () ) {
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

			if ( col == PHR_PAID ) {
				input_ok = ( b_paid == !( ui->tablePayments->sheetItem ( row, PHR_METHOD )->text ().isEmpty () ) );
				mPayCurItem->setFieldInputStatus ( PHR_METHOD + INFO_TABLE_COLUMNS_OFFSET, input_ok.isOn (), ui->tablePayments->sheetItem ( row, PHR_METHOD ) );
				input_ok = ( b_paid == !( ui->tablePayments->sheetItem ( row, PHR_ACCOUNT )->text ().isEmpty () ) );
				mPayCurItem->setFieldInputStatus ( PHR_ACCOUNT + INFO_TABLE_COLUMNS_OFFSET, input_ok.isOn (), ui->tablePayments->sheetItem ( row, PHR_ACCOUNT ) );
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

	if ( bSetUseDate ) {
		const QString strMethod ( ui->tablePayments->sheetItem ( row, PHR_METHOD )->text () );
		if ( strMethod == QStringLiteral ( "TED" ) || strMethod == QStringLiteral ( "DOC" ) || 
			 strMethod.contains ( QStringLiteral ( "Transferência" ), Qt::CaseInsensitive ) ) {
			 ui->tablePayments->sheetItem ( row, PHR_USE_DATE )->setDate ( 
				 ui->tablePayments->sheetItem ( row, PHR_DATE )->date () );
		}
	}
	
	if ( input_ok.isOn () ) {
		if ( ( row + 1 ) > ui->txtPayTotalPayments->text ().toUInt () )
			ui->txtPayTotalPayments->setText ( QString::number ( row + 1 ), true );
		
		stringTable pay_data;
		ui->tablePayments->makeStringTable ( pay_data );
		setRecValue ( mPayCurItem->payRecord (), FLD_PAY_INFO, pay_data.toString () );
	}
	postFieldAlterationActions ( mPayCurItem );
}

void MainWindow::updatePayTotalPaidValue ()
{
	vmNumber total_paid;
	for ( int i ( 0 ); i <= ui->tablePayments->lastUsedRow (); ++i ) {
		if ( ui->tablePayments->sheetItem ( i, PHR_PAID )->text () == CHR_ONE )
			total_paid += vmNumber ( ui->tablePayments->sheetItem ( i, PHR_VALUE )->text (), VMNT_PRICE );
	}
	ui->txtPayTotalPaid->setText ( total_paid.toPrice () ); // do not notify text change so that payOverdueGUIActions can use the current stores paid value
	payOverdueGUIActions ( mPayCurItem->payRecord (), ACTION_EDIT );
	setRecValue ( mPayCurItem->payRecord (), FLD_PAY_TOTALPAID, total_paid.toPrice () ); // now we can update the record
}

void MainWindow::payKeyPressedSelector ( const QKeyEvent* ke )
{
	if ( ke->key () == Qt::Key_Escape )
		on_btnPayInfoCancel_clicked ();
	else {
		clientsList->setFocus (); // trigger any pending editing_Finished or textAltered event
		on_btnPayInfoSave_clicked ();
	}
}

// Delay loading lists until they are necessary
void MainWindow::tabPaysLists_currentChanged ( const int index )
{
	switch ( index ) {
		case 0: paysCurrentList = paysList; break;
		case 1: loadClientOverduesList (); paysCurrentList = paysOverdueClientList; break;
		case 2:	loadAllOverduesList ();	paysCurrentList = paysOverdueList; break;
	}
	paysOverdueClientList->setIgnoreChanges ( index != 1 );
	paysOverdueList->setIgnoreChanges ( index != 2 );
	txtPayTotals->setText ( paysCurrentList->property ( PROPERTY_TOTAL_PAYS ).toString () );
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
	else {
		ui->txtPayTotalPrice->setText ( new_price.toPrice () );
		setRecValue ( mPayCurItem->payRecord (), FLD_PAY_PRICE, new_price.toPrice () );
	}
	mPayCurItem->setFieldInputStatus ( FLD_PAY_PRICE, input_ok, ui->txtPayTotalPrice );
	postFieldAlterationActions ( mPayCurItem );
}

void MainWindow::txtPay_textAltered ( const vmWidget* const sender )
{
	setRecValue ( mPayCurItem->payRecord (), sender->id (), sender->text () );
	/* No need to call postFieldAlterationActions because the controls bound to the other fields
	 * are readonly and that function will be called from the table change callback */
	if ( sender->id () == FLD_PAY_OBS )
		postFieldAlterationActions ( mPayCurItem );
}

void MainWindow::chkPayOverdue_toggled ( const bool checked )
{
	const QString* chr ( &CHR_ZERO );
	if ( checked )
		chr = &CHR_TWO; //ignore price differences
	else {
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
void MainWindow::saveBuyWidget ( vmWidget* widget, const uint id )
{
	widget->setID ( id );
	buyWidgetList[id] = widget;
}

void MainWindow::showBuySearchResult ( vmListItem* item, const bool bshow )
{
	buyListItem* buy_item ( static_cast<buyListItem*> ( item ) );
	if ( bshow ) {
		displayClient ( static_cast<clientListItem*> ( buy_item->relatedItem ( RLI_CLIENTPARENT ) ), true,
						static_cast<jobListItem*> ( buy_item->relatedItem ( RLI_JOBPARENT ) ),
						buy_item );
	}

	for ( uint i ( 0 ); i < BUY_FIELD_COUNT; ++i ) {
		if ( buy_item->searchFieldStatus ( i ) == SS_SEARCH_FOUND ) {
			if ( i == FLD_BUY_PAYINFO ) {
				if ( bshow )
					ui->tableBuyPayments->searchStart ( SEARCH_UI ()->searchTerm () );
				else
					ui->tableBuyPayments->searchCancel ();
			}
			else if ( i == FLD_BUY_REPORT ) {
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
	buysList->setCallbackForCurrentItemChanged ( [&] ( vmListItem* item ) {
		return buysListWidget_currentItemChanged ( item ); } );
	buysJobList->setCallbackForCurrentItemChanged ( [&] ( vmListItem* item ) {
		return buysJobListWidget_currentItemChanged ( item ); } );
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
	//ui->tableBuyItems->setParentLayout ( ui->gLayoutBuyForms );
	ui->tableBuyItems->setUtilitiesPlaceLayout ( ui->layoutBuyItemsUtilities );
	ui->tableBuyItems->setCallbackForCellChanged ( [&] ( const vmTableItem* const item ) {
		return interceptBuyItemsCellChange ( item ); } );
	ui->tableBuyItems->setCallbackForMonitoredCellChanged ( [&] ( const vmTableItem* const item ) {
		return ui->txtBuyTotalPrice->setText ( item->text (), true ); } );
	ui->tableBuyItems->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
		return buyKeyPressedSelector ( ke ); } );

	( void ) vmTableWidget::createPayHistoryTable ( ui->tableBuyPayments, this, PHR_METHOD );
	ui->tableBuyPayments->setKeepModificationRecords ( false );
	saveBuyWidget ( ui->tableBuyPayments, FLD_BUY_PAYINFO );
	//ui->tableBuyPayments->setParentLayout ( ui->gLayoutBuyForms );
	ui->tableBuyPayments->setUtilitiesPlaceLayout ( ui->layoutBuyPaymentsUtilities );
	ui->tableBuyPayments->setCallbackForCellChanged ( [&] ( const vmTableItem* const item ) {
		return interceptBuyPaymentCellChange ( item ); } );
	ui->tableBuyPayments->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
		return buyKeyPressedSelector ( ke ); } );

	connect ( ui->btnShowSuppliersDlg, &QToolButton::clicked, this, [&] () { return SUPPLIERS ()->showDialog (); } );
	connect ( ui->btnBuyAdd, &QToolButton::clicked, this, [&] () { return on_btnBuyAdd_clicked (); } );
	connect ( ui->btnBuyEdit, &QToolButton::clicked, this, [&] () { return on_btnBuyEdit_clicked (); } );
	connect ( ui->btnBuyDel, &QToolButton::clicked, this, [&] () { return on_btnBuyDel_clicked (); } );
	connect ( ui->btnBuySave, &QToolButton::clicked, this, [&] () { return on_btnBuySave_clicked (); } );
	connect ( ui->btnBuyCancel, &QToolButton::clicked, this, [&] () { return on_btnBuyCancel_clicked (); } );
	connect ( ui->btnBuyCopyRows, &QToolButton::clicked, this, [&] () { return on_btnBuyCopyRows_clicked (); } );
	
	setTabOrder ( ui->btnShowSuppliersDlg, ui->dteBuyDate );
	setTabOrder ( ui->dteBuyDate, ui->dteBuyDeliveryDate );
	setTabOrder ( ui->dteBuyDeliveryDate, ui->txtBuyDeliveryMethod );
	setTabOrder ( ui->txtBuyDeliveryMethod, ui->tableBuyItems );
}

void MainWindow::displayBuyFromCalendar ( vmListItem* cal_item )
{
	clientListItem* client_item ( getClientItem ( cal_item->data ( Qt::UserRole + 1 ).toInt () ) );
	if ( client_item ) {
		buyListItem* buy_item ( getBuyItem ( client_item, cal_item->data ( Qt::UserRole ).toInt () ) );
		if ( buy_item ) {
			jobListItem* job_item ( static_cast<jobListItem*> ( buy_item->relatedItem ( RLI_JOBPARENT ) ) );
			displayClient ( client_item, true, job_item, buy_item );
		}
	}
}

void MainWindow::buysListWidget_currentItemChanged ( vmListItem* item )
{
	insertNavItem ( item );
	if ( item ) {
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
	//buyListItem* buy_jobitem ( static_cast<buyListItem*> ( item ) );
	//if ( !buy_jobitem )
	//	buy_jobitem = BUY_JOB_PREV_ITEM;
	displayBuy ( item ? static_cast<buyListItem*>( item->relatedItem ( RLI_CLIENTITEM ) ) : nullptr, true );
}

void MainWindow::buySuppliersListWidget_currentItemChanged ( vmListItem* item )
{
	if ( item ) {
		ui->lstBuySuppliers->setIgnoreChanges ( true );
		buyListItem* buy_item ( static_cast<buyListItem*> ( item ) );
		if ( static_cast<clientListItem*> ( buy_item->relatedItem ( RLI_CLIENTPARENT ) ) != mClientCurItem ) {
			displayClient ( static_cast<clientListItem*> ( buy_item->relatedItem ( RLI_CLIENTPARENT ) ), true,
							static_cast<jobListItem*> ( buy_item->relatedItem ( RLI_JOBPARENT ) ),
							static_cast<buyListItem*> ( buy_item->relatedItem ( RLI_CLIENTITEM ) ) );
		}
		else {
			if ( static_cast<jobListItem*> ( buy_item->relatedItem ( RLI_JOBPARENT ) ) != mJobCurItem ) {
				displayJob ( static_cast<jobListItem*> ( buy_item->relatedItem ( RLI_JOBPARENT ) ), true, 
							 static_cast<buyListItem*> ( buy_item->relatedItem ( RLI_CLIENTITEM ) ) ) ;
			}
			else {
				displayBuy ( static_cast<buyListItem*> ( buy_item->relatedItem ( RLI_CLIENTITEM ) ), true );
			}
		}
		ui->lstBuySuppliers->setIgnoreChanges ( false );
	}
}

void MainWindow::updateCalendarWithBuyInfo ( Buy* const buy, const RECORD_ACTION action )
{
	const vmNumber date ( buy->date ( FLD_BUY_DATE ) );
	const vmNumber price ( buy->price ( FLD_BUY_PRICE ) );

	switch ( action ) {
		case ACTION_ADD:
		{
			if ( !price.isNull () )
				mCal->addCalendarExchangeRule ( buy->ce_list, CEAO_ADD_PRICE_DATE1, date, price );
			mCal->addCalendarExchangeRule ( buy->ce_list, CEAO_ADD_DATE1, date, vmNumber::emptyNumber );
		}
		break;
		case ACTION_DEL:
		{
			mCal->addCalendarExchangeRule ( buy->ce_list, CEAO_DEL_DATE1, date, vmNumber::emptyNumber );
			mCal->addCalendarExchangeRule ( buy->ce_list, CEAO_DEL_PRICE_DATE1, date, price );
		}
		break;
		case ACTION_EDIT:
		{
			if ( buy->wasModified ( FLD_BUY_PRICE ) || buy->wasModified ( FLD_BUY_DATE ) ) {
				const vmNumber origDate ( recStrValueAlternate ( buy, FLD_BUY_DATE ), VMNT_DATE, vmNumber::VDF_DB_DATE );

				/* If either date or price were changed, we must exclude the
				 * the original price (if) stored in calendar, and (possibly) add the new price
				 */
				const vmNumber origPrice ( recStrValueAlternate ( buy, FLD_BUY_PRICE ), VMNT_PRICE, 1 );
				if ( !origPrice.isNull () ) {
					if ( origPrice != price && !price.isNull () ) {
						mCal->addCalendarExchangeRule ( buy->ce_list, CEAO_DEL_PRICE_DATE1, origDate, origPrice );
						mCal->addCalendarExchangeRule ( buy->ce_list, CEAO_ADD_PRICE_DATE1, date, price );
					}
				}
				// Now, we alter the date in calendar only if date was changed
				if ( origDate != date ) {
					mCal->addCalendarExchangeRule ( buy->ce_list, CEAO_DEL_DATE1, origDate, vmNumber::emptyNumber );
					mCal->addCalendarExchangeRule ( buy->ce_list, CEAO_ADD_DATE1, date, vmNumber::emptyNumber );
				}
			}
		}
		break;
		default:
			return;
	}
	updateCalendarWithBuyPayInfo ( buy, action );
	if ( !buy->ce_list.isEmpty () )
		buy->updateCalendarBuyInfo ();
}

void MainWindow::updateCalendarWithBuyPayInfo ( Buy* const buy, const RECORD_ACTION action )
{
	if ( action != ACTION_DEL )
	{
		if ( !buy->wasModified ( FLD_BUY_PAYINFO ) )
			return;
	}

	stringRecord rec;
	vmNumber date, price;

	if ( action != ACTION_ADD )
	{ // block to remove old info - only when deleting or editing
		// Remove old occurences. Easier than trying to compare what's the same, what's not, what's removed, what's addes
		const stringTable origTable ( recStrValueAlternate ( buy, FLD_BUY_PAYINFO ) );
		if ( origTable.firstStr () )
		{
			do
			{
				rec.fromString ( origTable.curRecord () );
				if ( rec.first () )
				{
					do
					{
						date.fromStrDate ( rec.fieldValue ( PHR_DATE ) );
						mCal->addCalendarExchangeRule ( buy->ce_list, CEAO_DEL_DATE2, date, vmNumber::emptyNumber );
						if ( rec.fieldValue ( PHR_PAID ) == CHR_ONE ) {
							price.fromStrPrice ( rec.fieldValue ( PHR_VALUE ) );
							mCal->addCalendarExchangeRule ( buy->ce_list, CEAO_DEL_PRICE_DATE2, date, price );
						}
					} while ( rec.next () );
				}
			} while ( origTable.nextStr () );
		}
	}

	if ( action != ACTION_DEL )
	{ // Now, insert new occurences, only when editing or inserting
		const stringTable newTable ( recStrValue ( buy, FLD_BUY_PAYINFO ) );
		if ( newTable.firstStr () )
		{
			do
			{
				rec.fromString ( newTable.curRecord () );
				if ( rec.first () )
				{
					do
					{
						date.fromStrDate ( rec.fieldValue ( PHR_DATE ) );
						mCal->addCalendarExchangeRule ( buy->ce_list, CEAO_ADD_DATE2, date, vmNumber::emptyNumber );
						if ( rec.fieldValue ( PHR_PAID ) == CHR_ONE )
						{
							price.fromStrPrice ( rec.fieldValue ( PHR_VALUE ) );
							mCal->addCalendarExchangeRule ( buy->ce_list, CEAO_ADD_PRICE_DATE2, date, price );
						}
						APP_COMPLETERS ()->updateCompleter ( rec.fieldValue ( PHR_METHOD ), vmCompleters::PAYMENT_METHOD );
					} while ( rec.next () );
				}
			} while ( newTable.nextStr () );
		}
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

void MainWindow::displayBuy ( buyListItem* buy_item, const bool b_select )
{
	if ( buy_item && buy_item->loadData () ) {
		controlBuyForms ( buy_item );
		loadBuyInfo ( buy_item->buyRecord () );
		if ( buy_item != mBuyCurItem ) {
			if ( b_select ) { // Sync both lists, but make sure no signals are emitted
				buysList->setCurrentItem ( buy_item, false );
				buysJobList->setCurrentItem ( buy_item->relatedItem ( RLI_JOBITEM ), false );
			}
			mBuyCurItem = buy_item;
			if ( buy_item->relatedItem ( RLI_EXTRAITEM ) != nullptr )
				ui->lstBuySuppliers->setCurrentItem ( static_cast<buyListItem*> ( buy_item->relatedItem ( RLI_EXTRAITEM ) ), false );
		}
	}
	else {
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
	
	if ( buy ) {
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
	else {
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

buyListItem* MainWindow::getBuyItem ( const clientListItem* const parent_client, const int id ) const
{
	if ( id >= 1 && parent_client ) {
		int i ( 0 );
		while ( i < static_cast<int> ( parent_client->buys->count () ) ) {
			if ( parent_client->buys->at ( i )->dbRecID () == static_cast<int> ( id ) )
				return parent_client->buys->at ( i );
			++i;
		}
	}
	return nullptr;
}

void MainWindow::fillJobBuyList ( const jobListItem* parent_job )
{
	buysJobList->clear ();
	if ( parent_job ) {
		vmNumber totalPrice, totalPaid;
		buyListItem* buy_item ( nullptr );
		for ( uint i ( 0 ); i < parent_job->buys->count (); ++i ) {
			buy_item = parent_job->buys->at ( i );
			buy_item->addToList ( buysJobList );
			totalPrice += buy_item->buyRecord ()->price ( FLD_BUY_PRICE );
			totalPaid += buy_item->buyRecord ()->price ( FLD_BUY_TOTALPAID );
		}
		ui->txtBuyTotalPurchasePriceForJob->setText ( totalPrice.toPrice () );
		ui->txtBuyTotalPaidForJob->setText ( totalPaid.toPrice () );
	}
	buysJobList->setIgnoreChanges ( false );
}

void MainWindow::fillCalendarBuysList ( const stringTable& buyids, vmListWidget* list, const bool pay_date )
{
	if ( buyids.countRecords () > 0 ) {
		vmListItem* buy_item ( nullptr );
		Buy buy;
		QString label;
		bool ok ( true );
		int buyid ( buyids.first ().fieldValue ( 0 ).toInt ( &ok ) );
		if ( ok ) {
			do {
				if ( buy.readRecord ( buyid ) ) {
					label = Client::clientName ( recStrValue ( &buy, FLD_BUY_CLIENTID ) ) +
							CHR_SPACE + CHR_HYPHEN % CHR_SPACE +
							recStrValue ( &buy, FLD_BUY_SUPPLIER ) +
							CHR_SPACE + CHR_HYPHEN % CHR_SPACE +
							recStrValue ( &buy, FLD_BUY_PRICE ) +
							( pay_date ? TR_FUNC ( " (payment)" ) : TR_FUNC ( " (purchase)" ) );
					buy_item = new vmListItem ( label );
					buy_item->setData ( Qt::UserRole, buyid );
					buy_item->setData ( Qt::UserRole + 1, recStrValue ( &buy, FLD_BUY_CLIENTID ) );
					list->addItem ( buy_item );
				}
				buyid = buyids.next ().fieldValue ( 0 ).toInt ( &ok );
			} while ( ok );
		}
	}
}

void MainWindow::interceptBuyItemsCellChange ( const vmTableItem* const item )
{
	const uint row ( item->row () );
	if ( row == ui->tableBuyItems->totalsRow () )
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
	const uint row ( item->row () );
	if ( row == ui->tableBuyPayments->totalsRow () )
		return;

	const uint col ( item->column () );
	switch ( col ) {
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
			if ( col != PHR_METHOD ) {
				updateBuyTotalPaidValue ();
				if ( bPaid && !ui->tableBuyPayments->sheetItem ( row, PHR_DATE )->cellIsAltered () )
					ui->tableBuyPayments->sheetItem ( row, PHR_DATE )->setDate ( vmNumber::currentDate );
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
	for ( int i ( 0 ); i <= ui->tableBuyPayments->lastUsedRow (); ++i ) {
		if ( ui->tableBuyPayments->sheetItem ( i, PHR_PAID )->text () == CHR_ONE )
			total += vmNumber ( ui->tableBuyPayments->sheetItem ( i, PHR_VALUE )->text (), VMNT_PRICE );
	}
	ui->txtBuyTotalPaid->setText ( total.toPrice () );
	setRecValue ( mBuyCurItem->buyRecord (), FLD_BUY_TOTALPAID, total.toPrice () );
}

void MainWindow::buyKeyPressedSelector ( const QKeyEvent* ke )
{
	if ( ke->key () == Qt::Key_Escape )
		on_btnBuyCancel_clicked ();
	else {
		clientsList->setFocus (); // trigger any pending editing_Finished or textAltered event
		on_btnBuySave_clicked ();
	}
}

void MainWindow::getPurchasesForSuppliers ( const QString& supplier )
{
	ui->lstBuySuppliers->clear ();
	int client_id ( VDB ()->firstDBRecord ( TABLE_CLIENT_ORDER ) );
	if ( client_id != -1 )
	{
		Buy buy;
		buyListItem* buy_client ( nullptr ), *new_buyitem ( nullptr );
		QString str_clientid;
		const int last_id ( VDB ()->lastDBRecord ( TABLE_CLIENT_ORDER ) );
		do
		{
			str_clientid = QString::number ( client_id );
			buy.stquery.str_query = QLatin1String ( "SELECT * FROM PURCHASES WHERE `SUPPLIER`='" ) +
									supplier + CHR_CHRMARK + QLatin1String ( " AND `CLIENTID`='" ) + str_clientid + CHR_CHRMARK;

			if ( buy.readFirstRecord ( FLD_BUY_SUPPLIER, supplier, false ) )
			{
				do
				{
					buy_client = getBuyItem ( getClientItem ( client_id ), recIntValue ( &buy, FLD_BUY_ID ) );
					if ( buy_client )
					{
						new_buyitem = static_cast<buyListItem*> ( buy_client->relatedItem ( RLI_EXTRAITEM ) );
						if ( !new_buyitem )
						{
							new_buyitem = new buyListItem;
							new_buyitem->setRelation ( RLI_EXTRAITEM );
							buy_client->syncSiblingWithThis ( new_buyitem );
						}
						(void) new_buyitem->loadData ();
						new_buyitem->addToList ( ui->lstBuySuppliers );
					}
				} while ( buy.readNextRecord ( true, false ) );
			}
		} while ( ++client_id <= last_id );
		++client_id;
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
		ui->tableBuyPayments->sheetItem ( row, PHR_VALUE )->setText ( price, false, true );
		ui->tableBuyPayments->sheetItem ( row, PHR_DATE )->setDate ( ui->dteBuyDate->date () );
	//}
}
//--------------------------------------------BUY------------------------------------------------------------

//--------------------------------------------EDITING-FINISHED-BUY-----------------------------------------------------------	
void MainWindow::txtBuy_textAltered ( const vmWidget* const sender )
{
	bool input_ok ( true );
	if ( sender->id () == FLD_BUY_PRICE ) {
		const vmNumber price ( sender->text (), VMNT_PRICE, 1 );
		input_ok = price > 0;
		mBuyCurItem->setFieldInputStatus ( FLD_BUY_PRICE, input_ok, ui->txtBuyTotalPrice );
		setBuyPayValueToBuyPrice ( sender->text () );
	}
	if ( input_ok )
		setRecValue ( mBuyCurItem->buyRecord (), sender->id (), sender->text () );
	postFieldAlterationActions ( mBuyCurItem );
}

void MainWindow::dteBuy_dateAltered ( const vmWidget* const sender )
{
	const vmNumber date ( static_cast<const vmDateEdit* const>( sender )->date () );
	const bool input_ok ( date.isDateWithinRange ( vmNumber::currentDate, 0, 4 ) );
	if ( input_ok ) {
		setRecValue ( mBuyCurItem->buyRecord (), sender->id (), date.toDate ( vmNumber::VDF_DB_DATE ) );
		mBuyCurItem->update ();
	}
	mBuyCurItem->setFieldInputStatus ( sender->id (), input_ok, sender );
	postFieldAlterationActions ( mBuyCurItem );
}

void MainWindow::cboBuySuppliers_textAltered ( const QString& text )
{
	const bool input_ok ( !text.isEmpty () );
	if ( input_ok ) {
		setRecValue ( mBuyCurItem->buyRecord (), FLD_BUY_SUPPLIER, ui->cboBuySuppliers->text () );
		mBuyCurItem->update ();
	}
	cboBuySuppliers_indexChanged ( input_ok ? ui->cboBuySuppliers->currentIndex () : -1 );
	mBuyCurItem->setFieldInputStatus ( FLD_BUY_SUPPLIER, input_ok, ui->cboBuySuppliers );
	postFieldAlterationActions ( mBuyCurItem );
}

void MainWindow::cboBuySuppliers_indexChanged ( const int idx )
{
	ui->lstBuySuppliers->setIgnoreChanges ( true );
	if ( idx != -1 ) {
		const QString supplier ( ui->cboBuySuppliers->itemText ( idx ) );
		getPurchasesForSuppliers ( supplier );
		ui->lblBuySupplierBuys->setText ( TR_FUNC ( "Purchases from " ) + supplier );
		SUPPLIERS ()->displaySupplier ( supplier );
	}
	else {
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
	servicesOfferedUI::init ();
	//ui->btnStandardPrices->setEnabled ( DATA ()->reads[TABLE_SERVICES_ORDER] );

	dbSuppliesUI::init ();
	//ui->tabSupplies->setEnabled ( DATA ()->reads[TABLE_SUPPLIES_ORDER] );

	InventoryUI::init ();
	//ui->tabInventory->setEnabled ( DATA ()->reads[TABLE_INVENTORY_ORDER] );
	//ui->btnImportExport->setEnabled ( DATA ()->reads[TABLES_IN_DB] || DATA ()->writes[TABLES_IN_DB] );

	crash = new crashRestore ( QStringLiteral ( "vmmain" ) );
	ui->txtSearch->setEditable ( true );
	connect ( ui->txtSearch, &QLineEdit::textChanged, this, [&] ( const QString& text ) { return on_txtSearch_textEdited ( text ); } );
	ui->txtSearch->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
		return searchCallbackSelector ( ke ); } );
	connect ( ui->btnSearchStart, &QToolButton::clicked, this, [&] () { return on_btnSearchStart_clicked (); } );
	connect ( ui->btnSearchCancel, &QToolButton::clicked, this, [&] () { return on_btnSearchCancel_clicked (); } );

	setupCalendarMethods ();
	setupClientPanel ();
	setupJobPanel ();
	setupPayPanel ();
	setupBuyPanel ();
	setupLeftPanel ();
	setupWorkFlow ();

	connect ( ui->btnReportGenerator, &QToolButton::clicked, this, [&] () { return btnReportGenerator_clicked (); } );
	connect ( ui->btnBackupRestore, &QToolButton::clicked, this, [&] () { return btnBackupRestore_clicked (); } );
	connect ( ui->btnCalculator, &QToolButton::clicked, this, [&] () { return btnCalculator_clicked (); } );
	connect ( ui->btnServicesPrices, &QToolButton::clicked, this, [&] () { return btnServicesPrices_clicked (); } );
	connect ( ui->btnEstimates, &QToolButton::clicked, this, [&] () { return btnEstimates_clicked (); } );
	connect ( ui->btnCompanyPurchases, &QToolButton::clicked, this, [&] () { return btnCompanyPurchases_clicked (); } );
	connect ( ui->btnConfiguration, &QToolButton::clicked, this, [&] () { return btnConfiguration_clicked (); } );
	connect ( ui->btnExitProgram, &QToolButton::clicked, this, [&] () { return btnExitProgram_clicked (); } );

	installEventFilter ( this );
}

bool MainWindow::restoreItem ( const stringRecord& restore_info )
{
	if ( restore_info.isOK () ) {
		bool ok ( true );

		const int type_id ( restore_info.fieldValue ( CF_SUBTYPE ).toInt ( &ok ) );
		if ( !ok || type_id < 0 )
			return false;

		clientListItem* client_item ( getClientItem ( restore_info.fieldValue ( CF_CLIENTID ).toInt ( &ok ) ) );
		if ( client_item == nullptr )
			return false;

		jobListItem* job_item ( getJobItem ( client_item, restore_info.fieldValue ( CF_JOBID ).toInt ( &ok ) ) );
		const int rec_id ( restore_info.fieldValue ( CF_ID ).toInt ( &ok ) );
		if ( !ok || rec_id == -1 )
			return false;

		const RECORD_ACTION action ( static_cast<RECORD_ACTION> ( restore_info.fieldValue ( CF_ACTION ).toInt ( &ok ) ) );

		clientsList->setCurrentItem ( client_item, true );
		if ( action >= ACTION_ADD ) {
			vmListItem* item ( nullptr );
			std::function<void ()> actionFunc;
			std::function<void ()> displayFunc;

			switch ( type_id ) {
				case CLIENT_TABLE:
					actionFunc = [&] () {
						return ( action == ACTION_EDIT ? btnClientEdit_clicked () : btnClientAdd_clicked () ); };
					displayFunc = [&] () { return  displayClient ( mClientCurItem, true ); };
					item = mClientCurItem;
				break;
				case JOB_TABLE:
					actionFunc = [&] () {
						return ( action == ACTION_EDIT ? on_btnJobEdit_clicked () : on_btnJobAdd_clicked () ); };
					displayFunc = [&] () { return displayJob ( mJobCurItem, true ); };
					item = mJobCurItem;
				break;
				case PAYMENT_TABLE:
					actionFunc = [&] () {
						return ( action == ACTION_EDIT ? on_btnPayInfoEdit_clicked () : addJobPayment ( job_item ) ); };
					displayFunc = [&] () { return  displayPay ( mPayCurItem, true ); };
					item = mPayCurItem;
				break;
				case PURCHASE_TABLE:
					actionFunc = [&] () {
						return ( action == ACTION_EDIT ? on_btnBuyEdit_clicked () : on_btnBuyAdd_clicked () ); };
					displayFunc = [&] () { return  displayBuy ( mBuyCurItem, true ); };
					item = mBuyCurItem;
				break;
				default:
				return false;
			}
			item->dbRec ()->fromStringRecord ( restore_info, CF_DBRECORD );
			displayFunc ();
			item->update ();
			actionFunc ();
		}
		return true;
	}
	return false;
}

void MainWindow::restoreLastSession ()
{
	clientsList->setIgnoreChanges ( false );
	ui->tabMain->setCurrentIndex ( 0 );
	if ( crash->needRestore () ) {
		VM_NOTIFY ()->notifyMessage ( TR_FUNC ( "Session Manager" ), TR_FUNC ( "Restoring last session due to unclean program termination" ) );
		if ( restoreItem ( crash->restoreFirstState () ) ) {
			do {
			} while ( restoreItem ( crash->restoreNextState () ) );
		}
		crash->eliminateRestoreInfo ();
		if ( crash->infoLoaded () )
			return;
	}

	clientListItem* client_item ( getClientItem ( CONFIG ()->lastViewedRecord ( CLIENT_TABLE ) ) );
	if ( !client_item )
		client_item = static_cast<clientListItem*> ( clientsList->item ( clientsList->count () - 1 ) );
	jobListItem* job_item ( getJobItem ( client_item, CONFIG ()->lastViewedRecord ( JOB_TABLE ) ) );
	displayClient ( client_item, true, job_item, getBuyItem ( client_item, CONFIG ()->lastViewedRecord ( PURCHASE_TABLE ) ) );
	insertNavItem ( jobsList->currentItem () );
	clientsList->setFocus ();
}

void MainWindow::searchCallbackSelector ( const QKeyEvent* ke )
{
	if ( ke->key () == Qt::Key_Escape )
		on_btnSearchCancel_clicked ();
	else
		on_btnSearchStart_clicked ();
}

void MainWindow::setupLeftPanel ()
{
	clientTaskPanel = new vmTaskPanel;

	agClientOps = clientTaskPanel->createGroup ( ICON ( "client.png" ), TR_FUNC ( "Clients" ), true, false );
	QFrame* frmClientOpsUtilitiesPanel ( new QFrame );
	QVBoxLayout* layoutClientOpsUtilitiesPanel ( new QVBoxLayout );
	layoutClientOpsUtilitiesPanel->setMargin ( 0 );
	layoutClientOpsUtilitiesPanel->setSpacing ( 0 );
	frmClientOpsUtilitiesPanel->setLayout ( layoutClientOpsUtilitiesPanel );
	frmClientOpsUtilitiesPanel->setFrameShape ( QFrame::NoFrame );
	clientsList->setUtilitiesPlaceLayout ( layoutClientOpsUtilitiesPanel );
	QGridLayout* layoutClientWidgets ( new QGridLayout );
	layoutClientWidgets->setMargin ( 2 );
	layoutClientWidgets->setSpacing ( 2 );
	layoutClientWidgets->addWidget ( frmClientOpsUtilitiesPanel, 0, 0, 1, 3 );
	layoutClientWidgets->addWidget ( clientsList, 1, 0, 2, 2 );
	layoutClientWidgets->addWidget ( ui->frmClientToolbar, 1, 2, 2, 3 );
	layoutClientWidgets->setColumnMinimumWidth ( 0, 200 );
	layoutClientWidgets->setRowMinimumHeight ( 1, 300 );
	clientsList->setParentLayout ( layoutClientWidgets );
	agClientOps->addLayout ( layoutClientWidgets );

	agClientJobs = clientTaskPanel->createGroup ( ICON ( "job_jobs->png" ), TR_FUNC ( "Jobs" ), true, false );
	QFrame* frmJobOpsUtilitiesPanel ( new QFrame );
	QVBoxLayout* layoutJobOpsUtilitiesPanel ( new QVBoxLayout );
	layoutJobOpsUtilitiesPanel->setMargin ( 0 );
	layoutJobOpsUtilitiesPanel->setSpacing ( 0 );
	frmJobOpsUtilitiesPanel->setLayout ( layoutJobOpsUtilitiesPanel );
	frmJobOpsUtilitiesPanel->setFrameShape ( QFrame::NoFrame );
	jobsList->setUtilitiesPlaceLayout ( layoutJobOpsUtilitiesPanel );
	QGridLayout* layoutJobWidgets ( new QGridLayout );
	layoutJobWidgets->setMargin ( 2 );
	layoutJobWidgets->setSpacing ( 2 );
	layoutJobWidgets->addWidget ( frmJobOpsUtilitiesPanel, 0, 0, 1, 3 );
	layoutJobWidgets->addWidget ( jobsList, 1, 0, 2, 2 );
	layoutJobWidgets->addWidget ( ui->frmJobToolbar, 1, 2, 2, 3 );
	layoutJobWidgets->setColumnMinimumWidth ( 0, 200 );
	layoutJobWidgets->setRowMinimumHeight ( 1, 200 );
	jobsList->setParentLayout ( layoutJobWidgets );
	agClientJobs->addLayout ( layoutJobWidgets );
	
	agClientPays = clientTaskPanel->createGroup ( ICON ( "client_pays->png" ), TR_FUNC ( "Payments" ), true, false );
	QFrame* frmPayOpsUtilitiesPanel ( new QFrame );
	QVBoxLayout* layoutPayOpsUtilitiesPanel ( new QVBoxLayout );
	layoutPayOpsUtilitiesPanel->setMargin ( 0 );
	layoutPayOpsUtilitiesPanel->setSpacing ( 0 );
	frmPayOpsUtilitiesPanel->setLayout ( layoutPayOpsUtilitiesPanel );
	frmPayOpsUtilitiesPanel->setFrameShape ( QFrame::NoFrame );
	paysList->setUtilitiesPlaceLayout ( layoutPayOpsUtilitiesPanel );
	QGridLayout* layoutPayWidgets ( new QGridLayout );
	layoutPayWidgets->setMargin ( 2 );
	layoutPayWidgets->setSpacing ( 2 );
	layoutPayWidgets->addWidget ( frmPayOpsUtilitiesPanel, 0, 0, 1, 3 );
	layoutPayWidgets->addWidget ( tabPaysLists, 1, 0, 2, 2 );
	layoutPayWidgets->addWidget ( ui->frmPayToolbar, 1, 2, 2, 3 );
	layoutPayWidgets->addWidget ( frmPayTotals, 3, 0, 3, 3 );
	layoutPayWidgets->setColumnMinimumWidth ( 0, 200 );
	layoutPayWidgets->setRowMinimumHeight ( 1, 200 );
	paysList->setParentLayout ( layoutPayWidgets );
	agClientPays->addLayout ( layoutPayWidgets );
	
	agClientBuys = clientTaskPanel->createGroup ( ICON ( "client_buys->png" ), TR_FUNC ( "Purchases" ), true, false );
	QFrame* frmBuyOpsUtilitiesPanel ( new QFrame );
	QVBoxLayout* layoutBuyOpsUtilitiesPanel ( new QVBoxLayout );
	layoutBuyOpsUtilitiesPanel->setMargin ( 0 );
	layoutBuyOpsUtilitiesPanel->setSpacing ( 0 );
	frmBuyOpsUtilitiesPanel->setLayout ( layoutBuyOpsUtilitiesPanel );
	frmBuyOpsUtilitiesPanel->setFrameShape ( QFrame::NoFrame );
	buysList->setUtilitiesPlaceLayout ( layoutBuyOpsUtilitiesPanel );
	QGridLayout* layoutBuyWidgets ( new QGridLayout );
	layoutBuyWidgets->setMargin ( 2 );
	layoutBuyWidgets->setSpacing ( 2 );
	layoutBuyWidgets->addWidget ( frmBuyOpsUtilitiesPanel, 0, 0, 1, 3 );
	layoutBuyWidgets->addWidget ( buysList, 1, 0, 2, 2 );
	layoutBuyWidgets->addWidget ( ui->frmBuyToolbar,  1, 2, 2, 3 );
	layoutBuyWidgets->setColumnMinimumWidth ( 0, 200 );
	layoutBuyWidgets->setRowMinimumHeight ( 1, 200 );
	buysList->setParentLayout ( layoutBuyWidgets );
	agClientBuys->addLayout ( layoutBuyWidgets );

	clientTaskPanel->addStretch ( 2 );
	ui->scrollLeftPanel->setWidget ( clientTaskPanel );
	syncWorkFlowWithLeftPanel ( 0 );
}

void MainWindow::setupWorkFlow ()
{
	mainTaskPanel = new vmTaskPanel;
	grpClients = mainTaskPanel->createGroup ( TR_FUNC ( "CLIENT INFORMATION" ), true, false );
	grpClients->setMinimumHeight ( ui->frmClientInfo->sizeHint ().height () );
	grpClients->addQEntry ( ui->frmClientInfo, new QHBoxLayout );

	grpJobs = mainTaskPanel->createGroup ( TR_FUNC ( "JOB REPORT AND IMAGES" ), true, false );
	grpJobs->setMinimumHeight ( ui->frmJobInfo->sizeHint ().height () + ui->frmJobInfo_2->sizeHint ().height () );
	grpJobs->addQEntry ( ui->frmJobInfo, new QHBoxLayout );
	grpJobs->addQEntry ( ui->frmJobInfo_2, new QHBoxLayout );

	grpPays = mainTaskPanel->createGroup ( TR_FUNC ( "PAYMENTS FOR JOB" ), true, false );
	grpPays->setMinimumHeight ( ui->frmPayInfo->sizeHint ().height () );
	grpPays->addQEntry ( ui->frmPayInfo, new QHBoxLayout );

	grpBuys = mainTaskPanel->createGroup ( TR_FUNC ( "PURCHASES FOR JOB" ), true, false );
	grpBuys->setMinimumHeight ( ui->splitterBuyInfo->sizeHint ().height () );
	grpBuys->addQEntry ( ui->splitterBuyInfo, new QHBoxLayout );

	ui->scrollWorkFlow->setWidget ( mainTaskPanel );

	setupTabNavigationButtons ();
	ui->tabMain->removeTab ( 7 );
	ui->tabMain->removeTab ( 6 );
	ui->tabMain->removeTab ( 5 );
	ui->tabMain->removeTab ( 4 );

	syncLeftPanelWithWorkFlow ( 0 );
}

void MainWindow::setupTabNavigationButtons ()
{
	mBtnNavPrev = new QToolButton;
	mBtnNavPrev->setIcon ( ICON ( "arrow-left" ) );
	mBtnNavPrev->setEnabled ( false );
	connect ( mBtnNavPrev, &QToolButton::clicked, this, [&] () { return navigatePrev (); } );
	mBtnNavNext = new QToolButton;
	mBtnNavNext->setIcon ( ICON ( "arrow-right" ) );
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
}

void MainWindow::syncLeftPanelWithWorkFlow ( const int value )
{
	disconnect ( ui->scrollLeftPanel->verticalScrollBar (), nullptr, this, nullptr );
	ui->scrollLeftPanel->verticalScrollBar ()->setValue ( leftToRightWorkflowRatio * value );
	connect ( ui->scrollLeftPanel->verticalScrollBar (), &QScrollBar::valueChanged, this, [&] ( const int value ) { return syncWorkFlowWithLeftPanel ( value ); } );
}

void MainWindow::syncWorkFlowWithLeftPanel ( const int value )
{
	disconnect ( ui->scrollWorkFlow->verticalScrollBar (), nullptr, this, nullptr );
	ui->scrollWorkFlow->verticalScrollBar ()->setValue ( leftPanel_multiplier * value );
	connect ( ui->scrollWorkFlow->verticalScrollBar (), &QScrollBar::valueChanged, this, [&] ( const int value ) { return syncLeftPanelWithWorkFlow ( value ); } );
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
				return updateCalendarView ( year, month ); } );
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
	date.setDate ( 1, month, year );
	const stringTable jobsPerDateList ( mCal->dateLog ( date, FLD_CALENDAR_MONTH,
				FLD_CALENDAR_JOBS, price, FLD_CALENDAR_TOTAL_JOBPRICE_SCHEDULED, true ) );

	if ( jobsPerDateList.countRecords () > 0 )
	{
		const stringRecord* str_rec ( &jobsPerDateList.first () );
		if ( str_rec->isOK () )
		{
			int jobid ( -1 );
			Job job;
			QString tooltip, day;
			QTextCharFormat dateChrFormat;
			ui->calMain->setDateTextFormat ( QDate ( 0, 0 ,0 ), QTextCharFormat () );

			do
			{
				jobid =  str_rec->fieldValue ( 0 ).toInt ();
				if ( job.readRecord ( jobid ) )
				{
					date.fromTrustedStrDate( str_rec->fieldValue ( 3 ), vmNumber::VDF_DB_DATE );
					day = str_rec->fieldValue( 2 );
					dateChrFormat = ui->calMain->dateTextFormat ( date.toQDate () );
					tooltip = dateChrFormat.toolTip ();
					tooltip +=  QLatin1String ( "<br>Job ID: " ) + recStrValue ( &job, FLD_JOB_ID ) +
								QLatin1String ( "<br>Client: " ) + Client::clientName ( recStrValue ( &job, FLD_JOB_CLIENTID ) ) +
								QLatin1String ( "<br>Job type: " ) + recStrValue ( &job, FLD_JOB_TYPE ) +
								QLatin1String ( " (Day " ) + str_rec->fieldValue( 2 ) + CHR_R_PARENTHESIS + QLatin1String ( "<br>" );

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
	}
	if ( bFillList )
	{
		QString price;
		const int search_field ( FLD_CALENDAR_DAY_DATE + index );
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
	}
	if ( bFillList )
	{
		QString price;
		const int search_field ( FLD_CALENDAR_DAY_DATE + index );
		fillCalendarBuysList (
			mCal->dateLog ( mCalendarDate, search_field,
							FLD_CALENDAR_BUYS, price, FLD_CALENDAR_TOTAL_BUY_BOUGHT ), list );
		line->setText ( price );
		fillCalendarBuysList (
			mCal->dateLog ( mCalendarDate, search_field,
							FLD_CALENDAR_BUYS_PAY, price, FLD_CALENDAR_TOTAL_BUY_PAID ), list, true );
		if ( !price.isEmpty () )
			line->setText ( line->text () + QLatin1String ( " (" ) + price + CHR_R_PARENTHESIS );
	}
}

void MainWindow::changeCalendarToolBoxesText ( const vmNumber& date )
{
	vmNumber firstDayOfWeek ( date );
	firstDayOfWeek.setDate ( 1 - date.dayOfWeek (), 0, 0, true );
	const QString str_firstDayOfWeek ( firstDayOfWeek.toString () );
	vmNumber lastDayOfWeek ( firstDayOfWeek );
	lastDayOfWeek.setDate ( 6, 0, 0, true );
	const QString str_lastDayOfWeek ( lastDayOfWeek.toString () );
	const QString date_str ( date.toString () );
	const QString monthName ( QDate::longMonthName ( date.month () ) );

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
void MainWindow::findCalendarsHiddenWidgets ()
{
	const QWidgetList wlist ( QApplication::allWidgets () );
	QList<QWidget*>::const_iterator i ( wlist.constBegin () );
	const QList<QWidget*>::const_iterator i_end ( wlist.constEnd () );

	menuMainCal = nullptr;

	for ( ; i != i_end; ++i )
	{
		if ( ( static_cast<QWidget*> ( *i ) )->objectName ().rightRef ( 6 ).contains ( QStringLiteral ( "port" ) ) )
		{
			if ( static_cast<QWidget*> ( *i )->parentWidget ()->objectName ().contains ( QStringLiteral ( "qt_cal" ) ) )
			{
				if ( static_cast<QWidget*> ( *i )->parentWidget ()->parentWidget ()->accessibleName () == QStringLiteral ( "calMain" ) )
				{
					calMainView = static_cast<QWidget*> ( *i );
					return;
				}
			}
		}
	}
}

void MainWindow::exitRequested ( const bool user_requested )
{
	static bool already_called ( false );

	// the user clicked a button to exit the program. This function gets called and do the cleanup
	// job. Now, it asks Qt to exit the program and will be called again because it is hooked up to the
	// signal aboutToQuit (); only, we do not need to do anything this time, so we return.
	if ( already_called )
		return;

	/*QString strActionType;
	guiSlotFunc action_func ( nullptr );
	bool ask ( true );

	switch ( mTabView ) {
		case EDIT_CLIENT:
			action_func = &MainWindow::on_btnEditClient_clicked;
			strActionType = TR_FUNC ( "user" );
		break;
		case ADD_CLIENT:
			action_func = &MainWindow::on_btnInsertClient_clicked;
			strActionType = TR_FUNC ( "user" );
		break;
		case EDIT_JOB:
			action_func = &MainWindow::on_btnEditJob_clicked;
			strActionType = TR_FUNC ( "job" );
		break;
		case ADD_JOB:
			action_func = &MainWindow::on_btnInsertJob_clicked;
			strActionType = TR_FUNC ( "job" );
		break;
		case EDIT_SCHED:
			action_func = &MainWindow::on_btnEditSched_clicked;
			strActionType = TR_FUNC ( "schedule" );
		break;
		case ADD_SCHED:
			action_func = &MainWindow::on_btnInsertSched_clicked;
			strActionType = TR_FUNC ( "schedule" );
		break;
		case EDIT_PAY:
			action_func = &MainWindow::on_btnEditPay_clicked;
			strActionType = TR_FUNC ( "payment" );
		break;
		case ADD_PAY:
			action_func = &MainWindow::on_btnInsertPay_clicked;
			strActionType = TR_FUNC ( "payment" );
		break;
		case EDIT_BUY:
			action_func = &MainWindow::on_btnBuyEdit_clicked;
			strActionType = TR_FUNC ( "purchase" );
		break;
		case ADD_BUY:
			action_func = &MainWindow::on_btnInsertBuy_clicked;
			strActionType = TR_FUNC ( "purchase" );
		default:
			ask = false;
		break;
	}

	if ( ask ) {
		if ( user_requested ) {
			const Message::MESSAGE_BTNCLICKED answer =
				vmNotify::customBox ( TR_FUNC ( QString ( "Do you wish to save the current %1 information?" ).arg ( strActionType ).toUtf8 () ),
				vmNotify::QUESTION, TR_FUNC ( "Yes" ), TR_FUNC ( "No" ), TR_FUNC ( "Do not exit" ), this );
			switch ( answer ) {
				case Message::BUTTON_1:
					( this->*action_func ) ();
				break;
				case Message::BUTTON_3:
					return;
				break;
				default:
				break;
			}
		}
		else {
			if ( vmNotify::questionBox (
				TR_FUNC ( QString ( "Do you wish to save the current %1 information?" ).arg ( strActionType ).toUtf8 () ) ) )
				( this->*action_func ) ();
		}
	}
	*/
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

	if ( user_requested ) {
		already_called = true;
		delete globalMainWindow;
		qApp->quit ();
	}
}

void MainWindow::closeEvent ( QCloseEvent* e )
{
	e->ignore ();
	trayActivated ( QSystemTrayIcon::Trigger );
}

void MainWindow::changeEvent ( QEvent* e )
{
	QMainWindow::changeEvent ( e );
	switch ( e->type () ) {
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
	if ( e->type () == QEvent::KeyPress ) {
		const QKeyEvent* k = static_cast<QKeyEvent*> ( e );
		if ( k->modifiers () & Qt::ControlModifier ) {
			switch ( k->key () ) {
				case Qt::Key_F:
					ui->txtSearch->setFocus ();
					b_accepted = true;
				break;
				default:
				break;
			}
		}
		else { // no control key pressed
			b_accepted = unsigned( k->key () - Qt::Key_F4 ) <= unsigned( Qt::Key_F12 - Qt::Key_F4 );
			switch ( k->key () ) {
				case Qt::Key_Escape:
					if ( selJob_callback ) {
						selJob_callback ( -1 );
						selJob_callback = nullptr;
						b_accepted = true;
					}
				break;

				case Qt::Key_F4: btnReportGenerator_clicked (); break;
				case Qt::Key_F5: btnBackupRestore_clicked (); break;
				case Qt::Key_F6: btnCalculator_clicked (); break;
				case Qt::Key_F7: btnServicesPrices_clicked (); break;
				case Qt::Key_F8: btnEstimates_clicked (); break;
				case Qt::Key_F9: btnCompanyPurchases_clicked (); break;
				case Qt::Key_F10: btnConfiguration_clicked (); break;
				case Qt::Key_F12: btnExitProgram_clicked (); break;
			}
		}
	}
	else if ( e->type () == QEvent::MouseButtonPress ) {
		if ( o == static_cast<QObject*> ( calMainView ) ) {
			if ( static_cast<QMouseEvent*> ( e )->button () == Qt::RightButton ) {
				QMouseEvent eventBtnPress ( QEvent::MouseButtonPress, static_cast<QMouseEvent*> ( e )->pos (), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
				QMouseEvent eventBtnRelease ( QEvent::MouseButtonRelease, static_cast<QMouseEvent*> ( e )->pos (), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier );
				QApplication::sendEvent ( o, &eventBtnPress );
				QApplication::sendEvent ( o, &eventBtnRelease );
				QApplication::sendPostedEvents ( o, 0 ); // force fake event to be processed before posting another one to que queue
				mCalPopupMenuPos = static_cast<QMouseEvent*> ( e )->globalPos ();
				QTimer::singleShot ( 500, this, [&] () { return execMenuMainCal (); } );
				return true;
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
			buysList->setFocus ();
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
	if ( item != nullptr ) {
		navItems.insert ( navItems.currentIndex () + 1, item );
		mBtnNavPrev->setEnabled ( navItems.currentIndex () > 0 );
	}
}

void MainWindow::displayNav ()
{
	clientListItem* parentClient ( static_cast<clientListItem*>( navItems.current ()->relatedItem ( RLI_CLIENTPARENT ) ) );
	jobListItem* parentJob ( static_cast<jobListItem*>( navItems.current ()->relatedItem ( RLI_JOBPARENT ) ) );
	const bool bSelectClient ( parentClient != mClientCurItem );
	
	switch ( navItems.current ()->subType () ) {
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
			else {
				if ( parentJob != mJobCurItem )
					displayJob ( parentJob, true );
				else
					displayPay ( static_cast<payListItem*>( navItems.current () ), true ); 
			}
		break;
 		case PURCHASE_TABLE:
			if ( bSelectClient )
				displayClient ( parentClient, true, parentJob, static_cast<buyListItem*>( navItems.current () ) );
			else {
				if ( parentJob != mJobCurItem )
					displayJob ( parentJob, true );
				else
					displayPay ( static_cast<payListItem*>( navItems.current () ), true ); 
			}
		break;
		default: break;
	}
}
//--------------------------------------------WINDOWS-KEYS-EVENTS-EXTRAS-------------------------------------------------

//--------------------------------------------SLOTS-----------------------------------------------------------
void MainWindow::showTab ( const TAB_INDEXES ti ) //new todo test improve
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
	if ( widget == ui->txtJobReport ) {
		ui->btnJobSeparateReportWindow->setChecked ( false );
		ui->gLayoutJobExtraInfo->addWidget ( ui->txtJobReport, 1, 5, 6, 3 ); // line copied from ui_mainwindow.h
		ui->txtJobReport->show ();
	}
	else if ( widget == ui->jobImageViewer ) {
		//ui->btnJobSeparatePicture->setChecked ( false );
		ui->gLayoutJobExtraInfo->addWidget ( ui->frmJobPicturesControls, 8, 0, 1, 8); // line copied from ui_mainwindow.h
		ui->frmJobPicturesControls->show ();
		ui->hLayoutImgViewer->insertWidget ( 1, ui->jobImageViewer ); // insert after the first horizontal spacer to centralize it
		ui->jobImageViewer->show ();
	}
}

void MainWindow::quickProjectClosed ()
{
	ui->btnQuickProject->setChecked ( false );
	if ( mJobCurItem->dbRecID () == QUICK_PROJECT ()->jobID ().toInt () ) {
		const bool bReading ( mJobCurItem->action () == ACTION_READ );
		if ( bReading )
			on_btnJobEdit_clicked ();
		ui->txtJobProjectID->setText ( QUICK_PROJECT ()->qpString (), true );
		ui->txtJobPrice->setText ( QUICK_PROJECT ()->totalPrice (), true );
		if ( bReading )
			on_btnJobSave_clicked ();
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

inline void MainWindow::btnServicesPrices_clicked () { SERVICES ()->isVisible ()
			? SERVICES ()->hide () : SERVICES ()->showWindow (); }

inline void MainWindow::btnEstimates_clicked () { if ( !mClientCurItem ) return; ESTIMATE ()->isVisible ()
			? ESTIMATE ()->hide () : ESTIMATE ()->showWindow ( recStrValue ( mClientCurItem->clientRecord (), FLD_CLIENT_NAME ) ); }

inline void MainWindow::btnCompanyPurchases_clicked () { COMPANY_PURCHASES ()->isVisible ()
			? COMPANY_PURCHASES ()->hide () : COMPANY_PURCHASES ()->show (); }

inline void MainWindow::btnConfiguration_clicked () { CONFIG ()->dialogWindow ()->isVisible ()
			? CONFIG ()->dialogWindow ()->hide () : CONFIG ()->dialogWindow ()->show () ; }

inline void MainWindow::btnExitProgram_clicked () { exitRequested ( true ); }
//--------------------------------------------SLOTS------------------------------------------------------------

/*void MainWindow::on_lstClients_customContextMenuRequested ( const QPoint &pos )
{
	QMenu* menu = new QMenu ( this );
	menu->addAction ( static_cast<QAction*> ( action ) );
	connect ( menu, SIGNAL ( triggered ( QAction* ) ), this, SLOT ( itemsListContextMenu ( QAction *action ) ) );
	menu->exec ( mapToGlobal ( pos ) );
	delete menu;
	menu= nullptr;
}*/
//--------------------------------------------LIST-LISTITEM-----------------------------------------------------------

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
	SEARCH_UI ()->searchCancel ();
	ui->txtSearch->clear ();
	ui->btnSearchCancel->setEnabled ( false );
	ui->btnSearchStart->setEnabled ( false );
}
//--------------------------------------------SEARCH------------------------------------------------------------

//--------------------------------------------TRAY-IMPORT-EXPORT-BACKUP--------------------------------
void MainWindow::trayMenuTriggered ( QAction* action )
{
	const uint idx ( static_cast<vmAction*> ( action )->id () );

	if ( idx == 100 ) {
		btnExitProgram_clicked ();
		return;
	}
	else if ( idx == 101 )
		trayActivated ( QSystemTrayIcon::Trigger );
}

void MainWindow::trayActivated ( QSystemTrayIcon::ActivationReason reason )
{
	switch ( reason ) {
		case QSystemTrayIcon::Trigger:
		case QSystemTrayIcon::DoubleClick:
			if ( isMinimized () || isHidden () ) {
				trayIconMenu->actions ().at ( 0 )->setText ( TR_FUNC ( "Hide" ) );
				showNormal ();
				activateWindow ();
				raise ();
				/*if ( reportWindow != nullptr ) {
					if ( reportWindow->isHidden () )
						reportWindow->showNormal ();
				}*/
			}
			else {
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
	if ( b_setup ) {
		trayIcon = new QSystemTrayIcon ();
		trayIcon->setIcon ( ICON ( "vm-logo-22x22.png" ) );
		trayIcon->setToolTip ( windowTitle () );
		connect ( trayIcon, &QSystemTrayIcon::activated, this,
				  [&] ( QSystemTrayIcon::ActivationReason actReason ) { return trayActivated ( actReason ); } );

		trayIcon->show ();
		vmNotify::initNotify ();
	}
	else {
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
	// do not propagate the status to DBRecord so that it might use the info from the previous 
	// state to perform some actions if needed
	item->setAction ( ACTION_DEL, true );
	crash->eliminateRestoreInfo ( item->crashID () );
	if ( item->dbRec () && item->relation () == RLI_CLIENTITEM )
		item->dbRec ()->deleteRecord ();
	if ( b_auto_select_another )
		list->setIgnoreChanges ( false );
	list->removeItem ( item, b_delete_item );
}

void MainWindow::postFieldAlterationActions ( vmListItem* item )
{
	item->saveCrashInfo ( crash );
	const bool bCanSave ( item->isGoodToSave () );
	switch ( item->subType () ) {
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
	if ( timerUpdate == nullptr ) {
		timerUpdate = new QTimer ( this );
		connect ( timerUpdate, &QTimer::timeout, this, [&] () { return updateProgramDate (); } );
		timerUpdate->start ( 1000 * 60 * 5 ); // check date every five minutes (hibernation or suspend will thwart	the timer
	}
}
//----------------------------------------------CURRENT-DATE-BTNS------------------------------------------------------

//--------------------------------------------JOB---------------------------------------------------------------
void MainWindow::on_btnJobAdd_clicked ()
{
	//jobsList->setIgnoreChanges ( true );
	jobListItem* job_item ( new jobListItem );
	job_item->setRelation ( RLI_CLIENTITEM );
	job_item->setRelatedItem ( RLI_CLIENTPARENT, mClientCurItem );
	job_item->setRelatedItem ( RLI_JOBPARENT, job_item );
	(void) job_item->loadData ();
	job_item->setAction ( ACTION_ADD, true );
	job_item->addToList ( jobsList );
	addJobPayment ( job_item );
	setRecValue ( job_item->jobRecord (), FLD_JOB_CLIENTID, recStrValue ( mClientCurItem->clientRecord (), FLD_CLIENT_ID ) );
	//displayJob ( job_item, true );
	ui->cboJobType->setFocus ();
	//jobsList->setIgnoreChanges ( false );
}

void MainWindow::on_btnJobEdit_clicked ()
{
	if ( mJobCurItem ) {
		VM_NOTIFY ()->notifyMessage ( TR_FUNC ( "Editing job record" ), recStrValue ( mJobCurItem->jobRecord (), FLD_JOB_TYPE ) );
		mJobCurItem->setAction ( ACTION_EDIT, true );
		controlJobForms ( mJobCurItem );
		ui->txtJobPrice->setFocus ();
	}
}

void MainWindow::on_btnJobDel_clicked ()
{
	if ( mJobCurItem ) {
		jobListItem* job_item ( mJobCurItem );
		const QString text ( TR_FUNC ( "Are you sure you want to remove job %1?" ) );

		if ( VM_NOTIFY ()->questionBox ( TR_FUNC ( "Question" ), text.arg ( recStrValue ( job_item->jobRecord (), FLD_JOB_TYPE ) ) ) ) {
			removeJobPayment ( mJobCurItem->payItem () );
			job_item->setAction ( ACTION_DEL, true );

			updateCalendarWithJobInfo ( job_item->jobRecord (), ACTION_DEL ); // remove job info from calendar and clear job days list
			mClientCurItem->jobs->remove ( jobsList->currentRow () );
			removeListItem ( job_item );
		}
	}
}

void MainWindow::on_btnJobSave_clicked ()
{
	if ( mJobCurItem ) {
		Job* job ( mJobCurItem->jobRecord () );
		if ( job->saveRecord () ) {
			mJobCurItem->setAction ( job->action () );
			if ( mJobCurItem->dbRecID () == -1 ) {
				mJobCurItem->setDBRecID ( recIntValue ( job, FLD_JOB_ID ) );
				mClientCurItem->jobs->append ( mJobCurItem );
				ui->jobImageViewer->setID ( job->actualRecordInt ( FLD_JOB_ID ) );
				saveJobPayment ( mJobCurItem );
			}
			else {
				if ( job->wasModified ( FLD_JOB_PRICE ) )
					alterJobPayPrice ( mJobCurItem );
			}
			updateCalendarWithJobInfo ( job, job->prevAction () );
			rescanJobDaysList ();
			crash->eliminateRestoreInfo ( mJobCurItem->crashID () );
			VM_NOTIFY ()->notifyMessage ( TR_FUNC ( "Record saved" ), TR_FUNC ( "Job data saved!" ) );
			controlJobForms ( mJobCurItem );
			//When the job list is empty, we cannot add a purchase. Now that we've saved a job, might be we added one too, so now we can add a buy
			ui->btnBuyAdd->setEnabled ( true );
			saveView ();
		}
	}
}

void MainWindow::on_btnJobCancel_clicked ()
{
	if ( mJobCurItem ) {
		switch ( mJobCurItem->action () ) {
			case ACTION_ADD:
			{
				removeListItem ( mPayCurItem, false, false );
				/* Must set it to null because we make the above call with argument b_select_another to
				 * false, which will cause it not to call paysListWidget_currentItemChanged, which will
				 * not set mPayCurItem, but mPayCurItem points to an object that is no more. To avoid an
				 * inadverted reference to it within the chain of calls issued by the following instruction
				 * we set it to nullptr
				 */
				jobListItem* job_item ( mJobCurItem );
				mJobCurItem = nullptr;
				removeListItem ( job_item );
			}
			break;
			case ACTION_EDIT:
				mJobCurItem->setAction ( ACTION_REVERT, true );
				displayJob ( mJobCurItem );
			break;
			default:
			break;
		}
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
	if ( lastDir == CHR_ZERO ) {
		picturePath = recStrValue ( mJobCurItem->jobRecord (), FLD_JOB_PICTURE_PATH );
		str_lbljobpicture = TR_FUNC ( "Job pictures path:" );
	}
	else {
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
	const uint npics ( ui->cboJobPictures->count () );
	if ( npics > 0 ) {
		vmNumber::VM_DATE_FORMAT format ( vmNumber::VDF_DROPBOX_DATE );
		QString strDate;
		do {
			strDate = date.toDate ( format );
			for ( uint i ( 0 ); i < npics; ++i ) {
				if ( ui->cboJobPictures->itemText ( i ).startsWith ( strDate, Qt::CaseInsensitive ) ) {
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
		for ( uint i ( 0 ); i < npics; ++i ) {
			filename = ui->txtJobPicturesPath->text () + ui->cboJobPictures->itemText ( i );
			if ( date == fileOps::modifiedDate ( filename ) ) {
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
	if ( sepWin_JobPictures && index != -1 ) {
		if ( !sepWin_JobPictures->isHidden () )
			sepWin_JobPictures->setWindowTitle ( ui->jobImageViewer->imageFileName () );
	}
}

void MainWindow::btnJobRenamePicture_clicked ( const bool checked )
{
	ui->cboJobPictures->setEditable ( checked );
	if ( checked ) {
		if ( !ui->cboJobPictures->text ().isEmpty () ) {
			ui->cboJobPictures->setEditable ( true );
			ui->cboJobPictures->lineEdit ()->selectAll ();
			ui->cboJobPictures->setFocus ();
		}
	}
	else {
		const int new_index ( ui->jobImageViewer->rename ( ui->cboJobPictures->text () ) );
		if ( new_index != -1 ) {
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
	if ( sepWin_JobPictures == nullptr ) {
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
	if ( sepWin_JobReport == nullptr ) {
		sepWin_JobReport = new separateWindow ( ui->txtJobReport );
		sepWin_JobReport->setCallbackForReturningToParent ( [&] ( QWidget* wdgt ) { return receiveWidgetBack ( wdgt ); } );
	}
	if ( checked ) {
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

//--------------------------------------------SCHED------------------------------------------------------------
void MainWindow::execMenuMainCal ()
{
	execSchedOrPayMenu ( mCalPopupMenuPos, true );
}

void MainWindow::execSchedOrPayMenu ( const QPoint& pos, const bool b_sched_menu )
{
	if ( b_sched_menu ) {
		if ( menuMainCal == nullptr ) {
			menuMainCal = new QMenu ( calMainView );
			menuMainCal->addAction ( TR_FUNC ( "New task to this date" ), this, SLOT ( newTaskFromCalendar () ) );
			menuMainCal->addAction ( TR_FUNC ( "Delete this day's tasks" ), this, SLOT ( deleteAllSchedsFromDate () ) );
			subMenuMainCal = new QMenu ( TR_FUNC ( "Go to task" ) );
			//connect ( subMenuMainCal, &QMenu::triggered, this, [&] ( QAction* act ) { return showJobFromMenuMainCal ( act ); } );
			menuMainCal->addMenu ( subMenuMainCal );
		}
		QStringList jobInfo;
		subMenuMainCal->clear ();
		vmNumber date ( ui->calMain->selectedDate () );

		int n_dates ( 1 ); // current month's task
		if ( date.month () == vmNumber::currentDate.month () && date.year () == vmNumber::currentDate.year () )
			n_dates = 2; // have the option to go to last month's task also
		subMenuMainCal->setEnabled ( false );
		vmAction* action = nullptr;
		uint x ( 0 );
		QString menutext;
		do {
			//DATA ()->jobInfoFromSchedDate ( date, jobInfo );
			if ( !jobInfo.isEmpty () ) {
				x = 0;
				do {
					menutext = jobInfo.at ( x ); // type name
					menutext += " - ";
					menutext += jobInfo.at ( x + 1 ); // parent client name
					action = new vmAction ( x, menutext, this );
					action->setData ( jobInfo.at ( x + 2 ) ); //encoded JobID String
					subMenuMainCal->addAction( static_cast<QAction*> ( action ) );
					x += 3;
				} while ( x < unsigned ( jobInfo.count () ) );
				jobInfo.clear ();
				subMenuMainCal->setEnabled ( true );
			}
			if ( n_dates == 2 )
				date.setDate ( 0, -1, 0, true ); // now look for last month's tasks
		} while ( --n_dates > 0 );
		menuMainCal->exec ( pos );
	}
}
//--------------------------------------------PAY------------------------------------------------------------

void MainWindow::on_btnPayInfoDel_clicked ()
{
	const uint row ( ui->tablePayments->currentRow () == -1 ? 0 : static_cast<uint> ( ui->tablePayments->currentRow () ) );
	ui->tableBuyPayments->removeRow ( row );
	//for ( uint i_col ( 0 ); i_col < ui->tablePayments->colCount (); ++i_col )
	//	interceptPaymentCellChange ( ui->tablePayments->sheetItem ( row, i_col ) );
}

void MainWindow::on_btnPayInfoEdit_clicked ()
{
	if ( mPayCurItem ) {
		if ( mPayCurItem->action () == ACTION_READ ) {
			mPayCurItem->setAction ( ACTION_EDIT, true );
			controlPayForms ( mPayCurItem );
			ui->tablePayments->setFocus ();
		}
	}
}

void MainWindow::on_btnPayInfoSave_clicked ()
{
	if ( mPayCurItem ) {
		Payment* pay ( mPayCurItem->payRecord () );
		const bool bWasOverdue ( pay->actualRecordStr ( FLD_PAY_OVERDUE ) == CHR_ONE );
		
		if ( pay->saveRecord () ) {
			mPayCurItem->setAction ( pay->action () );
			
			if ( pay->wasModified ( FLD_PAY_OVERDUE ) ) {
				const bool bIsOverdue ( recStrValue ( pay, FLD_PAY_OVERDUE ) == CHR_ONE );
				if ( bIsOverdue && !bWasOverdue )
					addPaymentOverdueItems ( mPayCurItem );
				else if ( !bIsOverdue && bWasOverdue )
					removePaymentOverdueItems ( mPayCurItem );
			}
			if ( ui->tablePayments->tableChanged () )
				updateCalendarWithPayInfo ( pay, pay->prevAction () );
			ui->tablePayments->setTableUpdated ();
			crash->eliminateRestoreInfo ( mPayCurItem->crashID () );
			VM_NOTIFY ()->notifyMessage ( TR_FUNC ( "Record saved" ), TR_FUNC ( "Payment data saved!" ) );
			controlPayForms ( mPayCurItem );
		}
	}
}

void MainWindow::on_btnPayInfoCancel_clicked ()
{
	if ( mPayCurItem && mPayCurItem->action () == ACTION_EDIT ) {
		payOverdueGUIActions ( mPayCurItem->payRecord (), ACTION_REVERT );
		mPayCurItem->setAction ( ACTION_REVERT, true );
		displayPay( mPayCurItem );
	}
}
//--------------------------------------------PAY------------------------------------------------------------

//--------------------------------------------BUY------------------------------------------------------------
void MainWindow::on_btnBuyCopyRows_clicked ()
{
	selectBuysItems ( PS_EXEC );
}

void MainWindow::on_btnBuyAdd_clicked ()
{
	buyListItem* buy_item ( new buyListItem );
	buy_item->setRelation ( RLI_CLIENTITEM );
	buy_item->setRelatedItem ( RLI_CLIENTPARENT, mClientCurItem );
	buy_item->setRelatedItem ( RLI_JOBPARENT, mJobCurItem );
	(void) buy_item->loadData ();
	buy_item->setAction ( ACTION_ADD, true );
	mBuyCurItem = buy_item;

	buyListItem* buy_item2 ( new buyListItem );
	buy_item2->setRelation ( RLI_JOBITEM );
	buy_item->syncSiblingWithThis ( buy_item2 );
	buy_item2->addToList ( buysJobList, false );
	
	buy_item->addToList ( buysList );

	setRecValue ( buy_item->buyRecord (), FLD_BUY_CLIENTID, recStrValue ( mClientCurItem->clientRecord (), FLD_CLIENT_ID ) );
	setRecValue ( buy_item->buyRecord (), FLD_BUY_JOBID, recStrValue ( mJobCurItem->jobRecord (), FLD_JOB_ID ) );
	ui->cboBuySuppliers->setFocus ();
}

void MainWindow::on_btnBuyEdit_clicked ()
{
	if ( mBuyCurItem ) {
		mBuyCurItem->setAction ( ACTION_EDIT, true );
		controlBuyForms ( mBuyCurItem );
		ui->txtBuyTotalPrice->setFocus ();
	}
}

void MainWindow::on_btnBuyDel_clicked ()
{
	if ( mBuyCurItem ) {
		const QString text ( TR_FUNC ( "Are you sure you want to remove buy %1?" ) );
		if ( VM_NOTIFY ()->questionBox ( TR_FUNC ( "Question" ), text.arg ( recStrValue ( mBuyCurItem->buyRecord (), FLD_BUY_PRICE ) ) ) ) {
			buysJobList->setIgnoreChanges ( true );
			mJobCurItem->buys->remove ( buysJobList->currentRow () );
			buyListItem* buyjob_item ( static_cast<buyListItem*>( mBuyCurItem->relatedItem ( RLI_JOBITEM ) ) );
			delete buyjob_item;
			buysJobList->setIgnoreChanges ( false );
			
			mBuyCurItem->setAction ( ACTION_DEL, true );
			updateCalendarWithBuyInfo ( mBuyCurItem->buyRecord (), ACTION_DEL );
			updateBuyTotalPriceWidgets ( mBuyCurItem );
			mClientCurItem->buys->remove ( buysList->currentRow () );
			removeListItem ( mBuyCurItem );
		}
	}
}

void MainWindow::on_btnBuySave_clicked ()
{
	if ( mBuyCurItem ) {
		Buy* buy ( mBuyCurItem->buyRecord () );
		if ( buy->saveRecord () ) {
			mBuyCurItem->setAction ( buy->action () );
			if ( mBuyCurItem->dbRecID () == -1 ) {
				mBuyCurItem->setDBRecID ( recIntValue ( buy, FLD_JOB_ID ) );
				mClientCurItem->buys->append ( mBuyCurItem );
				mJobCurItem->buys->append ( static_cast<buyListItem*> ( mBuyCurItem->relatedItem ( RLI_JOBITEM ) ) );
			}
			if ( ui->tableBuyPayments->tableChanged () ) {
				updateCalendarWithBuyInfo ( buy, buy->prevAction () );
				ui->tableBuyPayments->setTableUpdated ();
			}
			vmTableWidget::exchangePurchaseTablesInfo ( ui->tableBuyItems, SUPPLIES ()->table (), buy, SUPPLIES ()->supplies_rec );
			updateBuyTotalPriceWidgets ( mBuyCurItem );

			Data::insertComboItem ( ui->cboBuySuppliers, recStrValue ( buy, FLD_BUY_SUPPLIER ) );
			crash->eliminateRestoreInfo ( mBuyCurItem->crashID () );
			VM_NOTIFY ()->notifyMessage ( TR_FUNC ( "Record saved" ), recStrValue ( mBuyCurItem->buyRecord (), FLD_BUY_SUPPLIER ) + 
										  TR_FUNC ( " purchase info saved!" ) );
			controlBuyForms ( mBuyCurItem );
			saveView ();
		}
	}
}

void MainWindow::on_btnBuyCancel_clicked ()
{
	if ( mBuyCurItem ) {
		switch ( mBuyCurItem->action () ) {
			case ACTION_ADD:
			{
				removeListItem ( static_cast<buyListItem*> ( mBuyCurItem->relatedItem ( RLI_JOBITEM ) ), false, false );
				buyListItem* buy_item ( mBuyCurItem );
				mBuyCurItem = nullptr;
				removeListItem ( buy_item );
			}
			break;
			case ACTION_EDIT:
				mBuyCurItem->setAction ( ACTION_REVERT, true );
				displayBuy ( mBuyCurItem );
			break;
			default:
			break;
		}
	}
}
//-------------------------------------------BUY------------------------------------------------------------
