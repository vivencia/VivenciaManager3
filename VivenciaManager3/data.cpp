#include "data.h"
#include "global.h"
#include "usermanagement.h"
#include "mainwindow.h"
#include "quickproject.h"
#include "backupdialog.h"
#include "configops.h"
#include "fileops.h"
#include "listitems.h"
#include "completers.h"
#include "spellcheck.h"
#include "vmnotify.h"
#include "calculator.h"
#include "cleanup.h"
#include "heapmanager.h"
#include "vmwidgets.h"
#include "companypurchasesui.h"
#include "dbsuppliesui.h"
#include "suppliersdlg.h"
#include "fast_library_functions.h"

#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QComboBox>
#include <QMessageBox>
#include <QListWidget>
#include <QMenu>
#include <QPoint>
#include <QDesktopWidget>

extern "C"
{
    #include <unistd.h>
}

static const QString MYSQL_INIT_SCRIPT ( QStringLiteral ( "/etc/init.d/mysql" ) );
QString APP_START_CMD ( emptyString );

#define ERR_DATABASE_PROBLEM 1
#define ERR_MYSQL_NOT_FOUND 2
#define ERR_USER_NOT_ADDED_TO_MYSQL_GROUP 3
#define ERR_SETUP_FILES_MISSING 4
#define ERR_COMMAND_MYSQL 5

Data* Data::s_instance ( nullptr );

void deleteDataInstance ()
{
	heap_del ( Data::s_instance );
}

//--------------------------------------STATIC-HELPER-FUNCTIONS---------------------------------------------

void Data::restartProgram ()
{
	char *args[2] = { 0, 0 };
	args[0] = static_cast<char*> ( ::malloc ( APP_START_CMD.toLocal8Bit ().count () * sizeof ( char ) ) );
	::strcpy ( args[0], APP_START_CMD.toLocal8Bit ().constData () );
	::execv ( args[0], args );
	::free ( args[0] );
	qApp->exit ( 0 );
}

bool Data::isMySQLRunning ()
{
    QString outStr ( fileOps::executeAndCaptureOutput ( QStringLiteral ( "status" ), MYSQL_INIT_SCRIPT ) );

    if ( configOps::isSystem ( UBUNTU ) ) {
        if ( configOps::initSystem ( SYSTEMD ) )
            return ( static_cast<bool> ( outStr.indexOf ( QStringLiteral ( "active (running)" ) ) != -1 ) );
        else
            return ( static_cast<bool> ( outStr.indexOf ( QStringLiteral ( "start/running, process" ) ) != -1 ) );
    }
	else if ( configOps::isSystem ( OPENSUSE ) )
		return ( static_cast<bool> ( outStr.indexOf ( QStringLiteral ( "running" ) ) != -1 ) );
	else
		return ( static_cast<bool> ( outStr.indexOf ( QStringLiteral ( "OK" ) ) != -1 ) );
}

QString Data::commandMySQLServer ( const QString& command, const QString& message, const bool only_return_cmd_line )
{
	if ( !only_return_cmd_line ) {
        if ( !fileOps::executeWait ( command, MYSQL_INIT_SCRIPT, nullptr, message ) )
            ::exit ( ERR_COMMAND_MYSQL );
	}
    return ( MYSQL_INIT_SCRIPT + CHR_SPACE + command );
}

bool Data::checkSystem ( const bool bFirstPass )
{
    if ( !fileOps::exists ( MYSQL_INIT_SCRIPT).isOn () ) {
        QMessageBox::critical ( nullptr, APP_TR_FUNC ( "MYSQL is not installed - Exiting" ),
            QApplication::tr ( "Could not find mysql init script at " ) + MYSQL_INIT_SCRIPT +
            QApplication::tr ( "Please check if mysql-client and mysql-server are installed." ), QMessageBox::Ok );
        ::exit ( ERR_MYSQL_NOT_FOUND );
    }

    const QString groups ( fileOps::executeAndCaptureOutput ( fileOps::currentUser (), QStringLiteral ( "groups" ) ) );
    bool ret ( groups.contains ( QRegExp ( QStringLiteral ( "mysql|root" ) ) ) );
    if ( !ret && bFirstPass ) {
        const QString args ( fileOps::currentUser () + QLatin1String ( " mysql" ) );
        if ( fileOps::executeWait ( args, QStringLiteral ( "adduser" ), nullptr,
							   QApplication::tr ( "To access mysql databases you need to belong the the mysql group."
									   "This program will add you to that group, but it needs the administrator password." )
                             ) )
            ret = checkSystem ( false );
        if ( !ret )
            ::exit ( ERR_USER_NOT_ADDED_TO_MYSQL_GROUP );
	}
    return ret;
}

/*
 * If there is something wrong, Data::checkDatabase will exit the app. Any other options that might be presented to the user,
 * if they do not encounter any error, will lead to a sane state to start the program and, therefore, all the calls subsequent to that one
 * will find a sane environment
 */
void Data::checkSetup ()
{
    (void) checkSystem ();
	checkDatabase ();

	const QString dataDir ( CONFIG ()->appDataDir () );
    if ( !fileOps::exists ( CONFIG ()->projectDocumentFile () ).isOn () ) {
		const QString installedDir ( QFileDialog::getExistingDirectory ( nullptr,
                                     APP_TR_FUNC ( "VivenciaManager needs to be setup. Choose the directory into which the download file was extracted." ),
                                     QStringLiteral ( "~" ) )
								   );
		if ( installedDir.isEmpty () ) {
			if ( QMessageBox::critical ( nullptr,
                                         APP_TR_FUNC ( "Setup files are missing" ),
										 QApplication::tr ( "Choose, again, the directory where the extracted setup files are" ),
										 QMessageBox::Ok, QMessageBox::Cancel ) == QMessageBox::Ok )
				checkSetup ();
			else
                ::exit ( ERR_SETUP_FILES_MISSING );
		}
		else {
			fileOps::createDir ( dataDir );
			fileOps::copyFile ( dataDir, installedDir + CHR_F_SLASH + STR_PROJECT_DOCUMENT_FILE );
			fileOps::copyFile ( dataDir, installedDir + CHR_F_SLASH + STR_PROJECT_SPREAD_FILE );
			fileOps::copyFile ( dataDir, installedDir + CHR_F_SLASH + STR_VIVENCIA_LOGO );
		}
	}
}

void Data::checkDatabase ()
{
	if ( !isMySQLRunning () ) {
        (void) commandMySQLServer ( QStringLiteral ( "start" ),
                             APP_TR_FUNC (  "The mysql server is not running. "
									 "It needs to be started in order to run this program. "
									 "Please, type below the administrator's password." )
						   );
	}

	if ( isMySQLRunning () ) {
		if ( !VDB ()->openDataBase () || VDB ()->databaseIsEmpty () ) {
			BACKUP ()->showNoDatabaseOptionsWindow ();
			if ( !BACKUP ()->actionSuccess () )
                ::exit ( ERR_DATABASE_PROBLEM );
		}
	}
	else {
		( void ) QMessageBox::critical ( nullptr,
                                         APP_TR_FUNC ( "Could not connect to mysql server" ),
                                         APP_TR_FUNC ( "The application will now exit. Try to manually start the mysql daemon"
                                                 " or troubleshoot for the problem if it persists." ), QMessageBox::Ok );
        ::exit ( ERR_DATABASE_PROBLEM );
	}
}
//--------------------------------------STATIC-HELPER-FUNCTIONS---------------------------------------------

Data::Data ()
{
	vmNumber::updateCurrentDate ();
	configOps::init ();
	checkSetup ();

	Calculator::init ();
	spellCheck::init ();
	vmCompleters::init ();
	vmDateEdit::createDateButtonsMenu ();
	VDB ()->doPreliminaryWork ();
	addPostRoutine ( deleteDataInstance );
}

Data::~Data ()
{
	//globalMainWindow = nullptr;
}

void Data::startUserInteface ()
{
	globalMainWindow = new MainWindow;
	loadDataIntoMemory ();
	globalMainWindow->continueStartUp ();
}

void Data::loadDataIntoMemory ()
{
    // To debug the GUI, it is possible to introduce a return here and skip all the code below
	clientListItem* client_item ( nullptr );
	jobListItem* job_item ( nullptr );
	payListItem* pay_item ( nullptr );
	buyListItem* buy_item ( nullptr ), *buy_item2 ( nullptr );
	QString jobid;

	int id ( VDB ()->firstDBRecord ( TABLE_CLIENT_ORDER ) );
	const int lastRec ( VDB ()->lastDBRecord ( TABLE_CLIENT_ORDER ) );

	Client client;
	Job job;
	Payment pay;
	Buy buy;
	do {
        if ( client.readRecord ( id, false ) ) {
			client_item = new clientListItem;
			client_item->setDBRecID ( id );
			client_item->setRelation ( RLI_CLIENTITEM );
			client_item->item_related[RLI_CLIENTPARENT] = client_item;
			(void) client_item->loadData ();
			client_item->update ( false );
            client_item->addToList ( globalMainWindow->clientsList );

			if ( job.readFirstRecord ( FLD_JOB_CLIENTID, QString::number ( id ), false ) ) {
				do {
					job_item = new jobListItem;
					job_item->setDBRecID ( job.actualRecordInt ( FLD_JOB_ID ) );
					job_item->setRelation ( RLI_CLIENTITEM );
					job_item->item_related[RLI_CLIENTPARENT] = client_item;
					job_item->item_related[RLI_JOBPARENT] = job_item;
					job_item->item_related[RLI_JOBITEM] = job_item;
                    client_item->jobs->append ( job_item );
					jobid = QString::number ( job.actualRecordInt ( FLD_JOB_ID ) );

					if ( pay.readFirstRecord ( FLD_PAY_JOBID, jobid, false ) ) {
						do {
							pay_item = new payListItem;
							pay_item->setDBRecID ( pay.actualRecordInt ( FLD_PAY_ID ) );
							pay_item->setRelation ( RLI_CLIENTITEM );
							pay_item->item_related[RLI_CLIENTPARENT] = client_item;
							pay_item->item_related[RLI_JOBPARENT] = job_item;
                            client_item->pays->append ( pay_item );
							job_item->setPayItem ( pay_item );
						} while ( pay.readNextRecord ( true, false ) );
					}

					if ( buy.readFirstRecord ( FLD_BUY_JOBID, jobid, false ) ) {
						do {
							buy_item = new buyListItem;
							buy_item->setDBRecID ( buy.actualRecordInt ( FLD_BUY_ID ) );
							buy_item->setRelation ( RLI_CLIENTITEM );
							buy_item->item_related[RLI_CLIENTPARENT] = client_item;
							buy_item->item_related[RLI_JOBPARENT] = job_item;
                            client_item->buys->append ( buy_item );

							buy_item2 = new buyListItem;
							buy_item2->setRelation ( RLI_JOBITEM );
                            buy_item->syncSiblingWithThis ( buy_item2 );
                            job_item->buys->append ( buy_item2 );
						} while ( buy.readNextRecord ( true, false ) );
					}

				} while ( job.readNextRecord ( true, false ) );
			}
		}
	} while ( ++id <= lastRec );
}

const QString Data::currentClientID () const
{
	if ( globalMainWindow->clientsList->currentItem () ) {
		Client* client ( static_cast<clientListItem*> ( globalMainWindow->clientsList->currentItem () )->clientRecord () );
		if ( client ) {
			if ( client->action () != ACTION_ADD )
				return recStrValue ( client, FLD_CLIENT_ID );
		}
	}
	return emptyString;
}

const QString Data::currentClientName () const
{
	Client* client ( currentClient () );
	if ( client )
		return recStrValue ( client, FLD_CLIENT_NAME );
	return emptyString;
}

Client* Data::currentClient () const
{
	if ( globalMainWindow->clientsList->currentItem () )
		return static_cast<clientListItem*> ( globalMainWindow->clientsList->currentItem () )->clientRecord ();
	return nullptr;
}

Job* Data::currentJob () const
{
	if ( globalMainWindow->jobsList->currentItem () )
		return static_cast<jobListItem*> ( globalMainWindow->jobsList->currentItem () )->jobRecord ();
	return nullptr;
}

Payment* Data::currentPay () const
{
	if ( globalMainWindow->paysList->currentItem () )
		return static_cast<payListItem*> ( globalMainWindow->paysList->currentItem () )->payRecord ();
	return nullptr;
}

Buy* Data::currentBuy () const
{
	if ( globalMainWindow->buysList->currentItem () )
		return static_cast<buyListItem*> ( globalMainWindow->buysList->currentItem () )->buyRecord ();
	return nullptr;
}

QString Data::mostOften ( const st_mostOften &st )
{
	QString strResult;
	DBRecord* dbrec ( nullptr );
	switch ( st.table ) {
		case CLIENT_TABLE:
			dbrec = new Client;
		break;
		case JOB_TABLE:
			dbrec = new Job;
		break;
		case PAYMENT_TABLE:
			dbrec = new Payment;
		break;
		case PURCHASE_TABLE:
			dbrec = new Buy;
		break;
		default:
			return strResult;
	}

	if ( dbrec->readFirstRecord ( st.id_field, QString::number ( st.client_id ) ) ) {
		VMList<QString> str_list;
		podList<int> str_count ( 0, 30 );
		int i ( 0 );
		do {
			if ( !st.search_term.isEmpty () ) {
				if ( st.search_term != dbrec->actualRecordStr ( st.search_field ) )
					continue;
			}
			strResult = dbrec->actualRecordStr ( st.result_field );
			i = str_list.contains ( strResult );
			if ( i == -1 ) {
				i = str_list.count ();
				str_list.append ( strResult );
			}
			++str_count[i];
		} while ( dbrec->readNextRecord ( true ) );
		strResult = str_list.at ( 0 );
		uint max ( 0 );
		for ( unsigned ( i ) = 1; i < str_count.count (); ++i ) {
			// If we have two or more strResult categories with the same count, priorize the latest
			if ( str_count.at ( i ) >= str_count.at ( max ) ) {
				strResult = str_list.at ( i );
				max = i;
			}
		}
		str_list.clear ();
		str_count.clear ();
	}
	return strResult;
}

void Data::copyToClipboard ( const QString& str )
{
	QApplication::clipboard ()->setText ( str, QClipboard::Clipboard );
}

int Data::insertComboItem ( vmComboBox* cbo, const QString& text )
{
	if ( text.isEmpty () )
		return -1;

	QString str;
	int x ( 0 );
	int i ( 0 );
	const int n_items ( cbo->count () );

	for ( ; i < n_items; ++i ) {
		str = cbo->itemText ( i );
		if ( text.compare ( str, Qt::CaseInsensitive ) == 0 )
			return i; // item already in combo, do nothing
		// Insert item alphabetically
		for ( x = 0; x < text.count (); ++x ) {
			if ( x >= str.count () )
				break;
			if ( text.at ( x ) > str.at ( x ) )
				break;
			else if ( text.at ( x ) == str.at ( x ) )
				continue;
			else {
				cbo->insertItem ( text, i, false );
				return i;
			}
		}
	}
	cbo->insertItem ( text, i, false );
	return i;
}

int Data::insertStringListItem ( QStringList& list, const QString& text )
{
	if ( text.isEmpty () )
		return -1;

	QString str;
	int x ( 0 );
	int i ( 0 );

	for ( ; i < list.count (); ++i ) {
		str = list.at ( i );
		if ( text.compare ( str, Qt::CaseInsensitive ) == 0 )
			return i; // item already in list, do nothing
		// Insert item alphabetically
		for ( x = 0; x < text.count (); ++x ) {
			if ( x >= str.count () )
				break;
			if ( text.at ( x ) > str.at ( x ) )
				break;
			else if ( text.at ( x ) == str.at ( x ) )
				continue;
			else {
				list.insert ( i, text );
				return i;
			}
		}
	}
	list.append ( text );
	return i;
}

QPoint Data::getGlobalWidgetPosition ( const QWidget* widget )
{
	QWidget* refWidget ( nullptr );
	if ( globalMainWindow->isAncestorOf ( widget ) )
		refWidget = globalMainWindow;
	else {
		refWidget = widget->parentWidget ();
		if ( refWidget == nullptr )
			refWidget = qApp->desktop ();
	}
	QPoint wpos;
	const QPoint posInRefWidget ( widget->mapTo ( refWidget, widget->pos () ) );
	wpos.setX ( refWidget->pos ().x () + posInRefWidget.x () - widget->pos ().x () );
	wpos.setY ( refWidget->pos ().y () + posInRefWidget.y () - widget->pos ().y () + widget->height () + TITLE_BAR_HEIGHT );
	return wpos;
}

void Data::execMenuWithinWidget ( QMenu* menu, const QWidget* widget, const QPoint& mouse_pos )
{
	QPoint menuPos ( getGlobalWidgetPosition ( widget ) );
	menuPos.setX ( menuPos.x () + mouse_pos.x () + TITLE_BAR_HEIGHT );
	menuPos.setY ( menuPos.y () + mouse_pos.y () );
	menu->exec ( menuPos );
}

void Data::fillJobTypeList ( QStringList &jobList, const QString& clientid )
{
    Job job;
    if ( job.readFirstRecord ( FLD_JOB_CLIENTID, clientid ) ) {
        do {
            jobList.append ( job.jobTypeWithDate () );
            //insertStringListItem ( jobList, job.jobTypeWithDate () );
        } while ( job.readNextRecord ( true ) );
    }
}
//--------------------------------------EXTRAS-----------------------------------------------
