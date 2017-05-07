#include "data.h"
#include "global.h"
#include "usermanagement.h"
#include "mainwindow.h"
#include "quickproject.h"
#include "backupdialog.h"
#include "configops.h"
#include "fileops.h"
#include "vmlistitem.h"
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
#include "keychain/passwordsessionmanager.h"

#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QComboBox>
#include <QMessageBox>
#include <QListWidget>
#include <QMenu>
#include <QPoint>
#include <QIcon>
#include <QDesktopWidget>

extern "C"
{
	#include <unistd.h>
}

QIcon* Data::listIndicatorIcons[4] = { nullptr };
bool Data::EXITING_PROGRAM ( false );

static const QString MYSQL_INIT_SCRIPT ( QStringLiteral ( "/etc/init.d/mysql" ) );
QString APP_START_CMD ( emptyString );

#define ERR_DATABASE_PROBLEM 1
#define ERR_MYSQL_NOT_FOUND 2
#define ERR_USER_NOT_ADDED_TO_MYSQL_GROUP 3
#define ERR_SETUP_FILES_MISSING 4
#define ERR_COMMAND_MYSQL 5

//--------------------------------------STATIC-HELPER-FUNCTIONS---------------------------------------------

void Data::restartProgram ()
{
	char* args[2] = { 0, 0 };
	args[0] = static_cast<char*> ( ::malloc ( static_cast<size_t>(APP_START_CMD.toLocal8Bit ().count ()) * sizeof (char) ) );
	::strcpy ( args[0], APP_START_CMD.toLocal8Bit ().constData () );
	::execv ( args[0], args );
	::free ( args[0] );
	qApp->exit ( 0 );
}

bool Data::isMySQLRunning ()
{
	QString outStr ( fileOps::executeAndCaptureOutput ( QStringLiteral ( "status" ), MYSQL_INIT_SCRIPT ) );

	if ( fileOps::exists ( QStringLiteral ( "/etc/lsb-release" ) ).isOn () ) // *buntu derivatives
	{
		if ( configOps::initSystem ( SYSTEMD ) )
			return ( static_cast<bool> ( outStr.indexOf ( QStringLiteral ( "active (running)" ) ) != -1 ) );
		else
			return ( static_cast<bool> ( outStr.indexOf ( QStringLiteral ( "start/running, process" ) ) != -1 ) );
	}
	else
		return ( static_cast<bool> ( outStr.indexOf ( QStringLiteral ( "OK" ) ) != -1 ) );
}

QString Data::commandMySQLServer ( const QString& command, const QString& message, const bool only_return_cmd_line )
{
	if ( !only_return_cmd_line )
	{
		if ( !fileOps::executeWait ( command, MYSQL_INIT_SCRIPT, nullptr, message ) )
			::exit ( ERR_COMMAND_MYSQL );
	}
	return ( MYSQL_INIT_SCRIPT + CHR_SPACE + command );
}

bool Data::checkSystem ( const bool bFirstPass )
{
	if ( !fileOps::exists ( MYSQL_INIT_SCRIPT).isOn () )
	{
		QMessageBox::critical ( nullptr, APP_TR_FUNC ( "MYSQL is not installed - Exiting" ),
			QApplication::tr ( "Could not find mysql init script at " ) + MYSQL_INIT_SCRIPT +
			QApplication::tr ( ". Please check if mysql-client and mysql-server are installed." ), QMessageBox::Ok );
		::exit ( ERR_MYSQL_NOT_FOUND );
	}

	const QString groups ( fileOps::executeAndCaptureOutput ( fileOps::currentUser (), QStringLiteral ( "groups" ) ) );
	bool ret ( groups.contains ( QRegExp ( QStringLiteral ( "mysql|root" ) ) ) );
	if ( !ret && bFirstPass )
	{
		QString passwd;
		if ( APP_PSWD_MANAGER ()->getPassword_UserInteraction ( passwd, SYSTEM_ROOT_SERVICE, SYSTEM_ROOT_PASSWORD_ID,
			APP_TR_FUNC ( "To access mysql databases you need to belong the the mysql group. This program will add you to that group, "
						  "but it needs administrator privileges." ) ) )
		{
			static_cast<void>( fileOps::sysExec ( sudoCommand.arg ( passwd, QLatin1String ( "adduser " ) + fileOps::currentUser () + QLatin1String ( " mysql" ) ) ) );
			ret = checkSystem ( false );
			if ( !ret )
				::exit ( ERR_USER_NOT_ADDED_TO_MYSQL_GROUP );
		}
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
	static_cast<void>( checkSystem () );
	checkDatabase ();

	if ( !fileOps::exists ( CONFIG ()->projectDocumentFile () ).isOn () )
	{
		const QString installedDir ( QFileDialog::getExistingDirectory ( nullptr,
									 APP_TR_FUNC ( "VivenciaManager needs to be setup. Choose the directory into which the download file was extracted." ),
									 QStringLiteral ( "~" ) )
								   );
		if ( installedDir.isEmpty () )
		{
			if ( QMessageBox::critical ( nullptr,
										 APP_TR_FUNC ( "Setup files are missing" ),
										 QApplication::tr ( "Choose, again, the directory where the extracted setup files are" ),
										 QMessageBox::Ok, QMessageBox::Cancel ) == QMessageBox::Ok )
				checkSetup ();
			else
				::exit ( ERR_SETUP_FILES_MISSING );
		}
		else
		{
			const QString dataDir ( CONFIG ()->appDataDir () );
			fileOps::createDir ( dataDir );
			fileOps::copyFile ( dataDir, installedDir + CHR_F_SLASH + STR_PROJECT_DOCUMENT_FILE );
			fileOps::copyFile ( dataDir, installedDir + CHR_F_SLASH + STR_PROJECT_SPREAD_FILE );
			fileOps::copyFile ( dataDir, installedDir + CHR_F_SLASH + STR_VIVENCIA_LOGO );
		}
	}
}

void Data::checkDatabase ()
{
	if ( !isMySQLRunning () )
	{
		static_cast<void>(commandMySQLServer ( QStringLiteral ( "start" ),
							 APP_TR_FUNC (  "The mysql server is not running. "
									 "It needs to be started in order to run this program. "
									 "Please, type below the administrator's password." ) ));
	}

	if ( isMySQLRunning () )
	{
		if ( !VDB ()->openDataBase () || VDB ()->databaseIsEmpty () ) {
			BACKUP ()->showNoDatabaseOptionsWindow ();
			if ( !BACKUP ()->actionSuccess () )
				::exit ( ERR_DATABASE_PROBLEM );
		}
	}
	else {
		static_cast<void>(QMessageBox::critical ( nullptr,
										 APP_TR_FUNC ( "Could not connect to mysql server" ),
										 APP_TR_FUNC (	"Make sure the packages mysql-client/server are installed. "
														"The application will now exit. Try to manually start the mysql daemon"
														" or troubleshoot for the problem if it persists." ), QMessageBox::Ok ));
		::exit ( ERR_DATABASE_PROBLEM );
	}
}
//--------------------------------------STATIC-HELPER-FUNCTIONS---------------------------------------------
void Data::init ()
{
	VivenciaDB::init ();
	vmNumber::updateCurrentDate ();
	configOps::init ();
	checkSetup ();

	Calculator::init ();
	spellCheck::init ();
	vmCompleters::init ();
	vmDateEdit::createDateButtonsMenu ();
	VDB ()->doPreliminaryWork ();

	listIndicatorIcons[static_cast<int>( ACTION_DEL)] = new QIcon ( ICON ( "listitem-delete" ) );
	listIndicatorIcons[static_cast<int>( ACTION_ADD)] = new QIcon ( ICON ( "listitem-add" ) );
	listIndicatorIcons[static_cast<int>( ACTION_EDIT)] = new QIcon ( ICON ( "listitem-edit" ) );
	
	MainWindow::init ();
	loadDataIntoMemory ();
	MAINWINDOW ()->continueStartUp ();
}

void Data::de_init ()
{
	static bool already_called ( false );

	// the user clicked a button to exit the program. This function gets called and do the cleanup
	// job. Now, it asks Qt to exit the program and will be called again because it is hooked up to the
	// signal aboutToQuit (); only, we do not need to do anything this time, so we return.
	if ( already_called )
		return;
	already_called = true;
	
	EXITING_PROGRAM = true;
	BackupDialog::doDayBackup ();
	
	delete listIndicatorIcons[1];
	delete listIndicatorIcons[2];
	delete listIndicatorIcons[3];
	
	cleanUpApp ();
	qApp->quit ();
}

void Data::loadDataIntoMemory ()
{	
	// To debug the GUI, it is possible to introduce a return here and skip all the code below
	clientListItem* client_item ( nullptr );
	jobListItem* job_item ( nullptr );
	payListItem* pay_item ( nullptr );
	buyListItem* buy_item ( nullptr ), *buy_item2 ( nullptr );
	QString jobid;

	uint id ( VDB ()->getLowestID ( TABLE_CLIENT_ORDER ) );
	const uint lastRec ( VDB ()->getHighestID ( TABLE_CLIENT_ORDER ) );

	Client client;
	Job job;
	Payment pay;
	Buy buy;
	do
	{
		if ( client.readRecord ( id, false ) )
		{
			client_item = new clientListItem;
			client_item->setDBRecID ( id );
			client_item->setRelation ( RLI_CLIENTITEM );
			client_item->setRelatedItem ( RLI_CLIENTPARENT, client_item );
			static_cast<void>( client_item->loadData () );
			client_item->addToList ( MAINWINDOW ()->UserInterface ()->clientsList );

			if ( job.readFirstRecord ( FLD_JOB_CLIENTID, QString::number ( id ), false ) )
			{
				do
				{
					job_item = new jobListItem;
					job_item->setDBRecID ( static_cast<uint>(job.actualRecordInt ( FLD_JOB_ID )) );
					job_item->setRelation ( RLI_CLIENTITEM );
					job_item->setRelatedItem ( RLI_CLIENTPARENT, client_item );
					job_item->setRelatedItem ( RLI_JOBPARENT, job_item );
					job_item->setRelatedItem ( RLI_JOBITEM, job_item );
					client_item->jobs->append ( job_item );
					jobid = QString::number ( job.actualRecordInt ( FLD_JOB_ID ) );

					if ( pay.readFirstRecord ( FLD_PAY_JOBID, jobid, false ) )
					{
						do
						{
							pay_item = new payListItem;
							pay_item->setDBRecID ( static_cast<uint>(pay.actualRecordInt ( FLD_PAY_ID )) );
							pay_item->setRelation ( RLI_CLIENTITEM );
							pay_item->setRelatedItem ( RLI_CLIENTPARENT, client_item );
							pay_item->setRelatedItem ( RLI_JOBPARENT, job_item );
							client_item->pays->append ( pay_item );
							job_item->setPayItem ( pay_item );
						} while ( pay.readNextRecord ( true, false ) );
					}

					if ( buy.readFirstRecord ( FLD_BUY_JOBID, jobid, false ) )
					{
						do
						{
							buy_item = new buyListItem;
							buy_item->setDBRecID ( static_cast<uint>(buy.actualRecordInt ( FLD_BUY_ID )) );
							buy_item->setRelation ( RLI_CLIENTITEM );
							buy_item->setRelatedItem ( RLI_CLIENTPARENT, client_item );
							buy_item->setRelatedItem ( RLI_JOBPARENT, job_item );
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

void Data::copyToClipboard ( const QString& str )
{
	QApplication::clipboard ()->setText ( str, QClipboard::Clipboard );
}

int Data::insertComboItem ( vmComboBox* __restrict cbo, const QString& text )
{
	if ( text.isEmpty () )
		return -1;

	QString str;
	int x ( 0 );
	int i ( 0 );
	const int n_items ( cbo->count () );

	for ( ; i < n_items; ++i )
	{
		str = cbo->itemText ( i );
		if ( text.compare ( str, Qt::CaseInsensitive ) == 0 )
			return i; // item already in combo, do nothing
		// Insert item alphabetically
		for ( x = 0; x < text.count (); ++x )
		{
			if ( x >= str.count () )
				break;
			if ( text.at ( x ) > str.at ( x ) )
				break;
			else if ( text.at ( x ) == str.at ( x ) )
				continue;
			else
			{
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

	for ( ; i < list.count (); ++i )
	{
		str = list.at ( i );
		if ( text.compare ( str, Qt::CaseInsensitive ) == 0 )
			return i; // item already in list, do nothing
		// Insert item alphabetically
		for ( x = 0; x < text.count (); ++x )
		{
			if ( x >= str.count () )
				break;
			if ( text.at ( x ) > str.at ( x ) )
				break;
			else if ( text.at ( x ) == str.at ( x ) )
				continue;
			else
			{
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
	if ( MAINWINDOW ()->isAncestorOf ( widget ) )
		refWidget = MAINWINDOW ();
	else
	{
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

void Data::fillJobTypeList ( QStringList& jobList, const QString& clientid )
{
	Job job;
	if ( job.readFirstRecord ( FLD_JOB_CLIENTID, clientid ) )
	{
		do
		{
			jobList.append ( job.jobTypeWithDate () );
		} while ( job.readNextRecord ( true ) );
	}
}

int Data::vmColorIndex ( const VMColors vmcolor )
{
	int idx ( -1 );
	switch ( vmcolor )
	{
		case vmGray: idx = 0; break;
		case vmRed: idx = 1; break;
		case vmYellow: idx = 2; break;
		case vmGreen: idx = 3; break;
		case vmBlue: idx = 4; break;
		case vmWhite: idx = 5; break;
		case vmDefault_Color: break;
	}
	return idx;
}

