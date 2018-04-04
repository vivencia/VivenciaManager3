#include "system_init.h"
#include "global.h"
#include "mainwindow.h"
#include "backupdialog.h"

#include <heapmanager.h>
#include <dbRecords/vivenciadb.h>
#include <dbRecords/completers.h>
#include <vmUtils/configops.h>
#include <vmUtils/fileops.h>
#include <vmPasswordManager/passwordsessionmanager.h>
#include <vmNotify/vmnotify.h>
#include <vmDocumentEditor/documenteditor.h>

#include <QtWidgets/QFileDialog>

extern "C"
{
	#include <unistd.h>
}

bool Sys_Init::EXITING_PROGRAM ( false );
QString Sys_Init::APP_START_CMD;

MainWindow* Sys_Init::mainwindow_instance ( nullptr );
configOps* Sys_Init::config_instance ( nullptr );
VivenciaDB* Sys_Init::vdb_instance ( nullptr );
vmCompleters* Sys_Init::completers_instance ( nullptr );
vmNotify* Sys_Init::notify_instance ( nullptr );
passwordSessionManager* Sys_Init::passwd_instance ( nullptr );
documentEditor* Sys_Init::editor_instance ( nullptr );
companyPurchasesUI* Sys_Init::cp_instance ( nullptr );
suppliersDlg* Sys_Init::sup_instance ( nullptr );
estimateDlg* Sys_Init::estimates_instance ( nullptr );
simpleCalculator* Sys_Init::calc_instance ( nullptr );
configDialog* Sys_Init::configdlg_instance ( nullptr );
spreadSheetEditor* Sys_Init::qp_instance ( nullptr );
BackupDialog* Sys_Init::backup_instance ( nullptr );
restoreManager* Sys_Init::restore_instance ( nullptr );

void deleteInstances ()
{
	heap_del ( Sys_Init::completers_instance );
	heap_del ( Sys_Init::vdb_instance );
	heap_del ( Sys_Init::config_instance );
	heap_del ( Sys_Init::notify_instance );
	heap_del ( Sys_Init::passwd_instance );
	heap_del ( Sys_Init::editor_instance );
	heap_del ( Sys_Init::mainwindow_instance );
	heap_del ( Sys_Init::cp_instance );
	heap_del ( Sys_Init::sup_instance );
	heap_del ( Sys_Init::estimates_instance );
	heap_del ( Sys_Init::calc_instance );
	heap_del ( Sys_Init::configdlg_instance );
	heap_del ( Sys_Init::qp_instance );
	heap_del ( Sys_Init::backup_instance );
	heap_del ( Sys_Init::restore_instance );
}
//--------------------------------------STATIC-HELPER-FUNCTIONS---------------------------------------------

void Sys_Init::restartProgram ()
{
	char* args[2] = { 0, 0 };
	args[0] = static_cast<char*> ( ::malloc ( static_cast<size_t>(APP_START_CMD.toLocal8Bit ().count ()) * sizeof (char) ) );
	::strcpy ( args[0], APP_START_CMD.toLocal8Bit ().constData () );
	::execv ( args[0], args );
	::free ( args[0] );
	qApp->exit ( 0 );
}

DB_ERROR_CODES Sys_Init::checkSystem ( const bool bFirstPass )
{
	if ( !fileOps::exists ( MYSQL_INIT_SCRIPT ).isOn () )
	{
		NOTIFY ()->criticalBox ( APP_TR_FUNC ( "MYSQL is not installed - Exiting" ),
			QApplication::tr ( "Could not find mysql init script at " ) + MYSQL_INIT_SCRIPT +
			QApplication::tr ( ". Please check if mysql-client and mysql-server are installed." ) );
		return ERR_MYSQL_NOT_FOUND;
	}

	const QString groups ( fileOps::executeAndCaptureOutput ( fileOps::currentUser (), QStringLiteral ( "groups" ) ) );
	bool ret ( groups.contains ( QRegExp ( QStringLiteral ( "mysql|root" ) ) ) );
	if ( !ret && bFirstPass )
	{
		QString passwd;
		if ( PASSWORD_MANAGER ()->getPassword_UserInteraction ( passwd, SYSTEM_ROOT_SERVICE, SYSTEM_ROOT_PASSWORD_ID,
			APP_TR_FUNC ( "To access mysql databases you need to belong the the mysql group. This program will add you to that group, "
						  "but it needs administrator privileges." ) ) )
		{
			static_cast<void>( fileOps::sysExec ( sudoCommand.arg ( passwd, QLatin1String ( "adduser " ) + fileOps::currentUser () + QLatin1String ( " mysql" ) ) ) );
			ret = checkSystem ( false );
			if ( !ret )
				return ERR_USER_NOT_ADDED_TO_MYSQL_GROUP;
		}
	}
	return NO_ERR;
}

DB_ERROR_CODES Sys_Init::checkLocalSetup ()
{
	if ( !fileOps::exists ( CONFIG ()->projectDocumentFile () ).isOn () )
	{
		const QString installedDir ( QFileDialog::getExistingDirectory ( nullptr,
									 APP_TR_FUNC ( "VivenciaManager needs to be setup. Choose the directory into which the download file was extracted." ),
									 QStringLiteral ( "~" ) )
								   );
		if ( installedDir.isEmpty () )
		{
			if ( NOTIFY ()->criticalBox ( APP_TR_FUNC ( "Setup files are missing" ),
										 QApplication::tr ( "Choose, again, the directory where the extracted setup files are" ),
										 false ) )
				checkSetup ();
			else
				return ERR_SETUP_FILES_MISSING;
		}
		else
		{
			const QString dataDir ( CONFIG ()->appDataDir ( true ) ); // the first time, use the default data dir. Later, the user might change it
			fileOps::createDir ( dataDir );
			fileOps::copyFile ( dataDir, installedDir + CHR_F_SLASH + STR_PROJECT_DOCUMENT_FILE );
			fileOps::copyFile ( dataDir, installedDir + CHR_F_SLASH + STR_PROJECT_SPREAD_FILE );
			fileOps::copyFile ( dataDir, installedDir + CHR_F_SLASH + STR_VIVENCIA_LOGO );
		}
	}
	return NO_ERR;
}

void Sys_Init::checkSetup ()
{
	DB_ERROR_CODES err_code ( NO_ERR );

	err_code = checkSystem ();
	if ( err_code != NO_ERR )
		deInit ( err_code );

	err_code = checkLocalSetup ();
	if ( err_code != NO_ERR )
		deInit ( err_code );

	err_code = VDB ()->checkDatabase ();

	switch ( err_code )
	{
		case NO_ERR: break;
		case ERR_NO_DB:
		case ERR_DB_EMPTY:
			err_code = static_cast<DB_ERROR_CODES>( BACKUP ()->showNoDatabaseOptionsWindow () );
			if ( err_code != NO_ERR )
				deInit ( err_code );
		break;
		case ERR_DATABASE_PROBLEM:
		case ERR_MYSQL_NOT_FOUND:
		case ERR_COMMAND_MYSQL:
		case ERR_USER_NOT_ADDED_TO_MYSQL_GROUP:
		case ERR_SETUP_FILES_MISSING:
			deInit ( err_code );
	}
}

//--------------------------------------STATIC-HELPER-FUNCTIONS---------------------------------------------
void Sys_Init::init ( const QString& cmd )
{
	APP_START_CMD = cmd;
	vmNumber::updateCurrentDate ();

	config_instance = new configOps;
	vdb_instance = new VivenciaDB;
	completers_instance = new vmCompleters;
	mainwindow_instance = new MainWindow;
	notify_instance = new vmNotify ( "BR", mainwindow_instance );

	//DBRecord::setDatabaseManager ( vdb_instance );
	DBRecord::setCompleterManager ( completers_instance );
	documentEditor::setCompleterManager ( completers_instance );
	dbListItem::appStartingProcedures ();

	checkSetup ();
	VDB ()->doPreliminaryWork ();

	loadDataIntoMemory ();
	MAINWINDOW ()->continueStartUp ();
}

void Sys_Init::deInit ( int err_code )
{
	static bool already_called ( false );

	// the user clicked a button to exit the program. This function gets called and do the cleanup
	// job. Now, it asks Qt to exit the program and will be called again because it is hooked up to the
	// signal aboutToQuit (); only, we do not need to do anything this time, so we return.
	if ( already_called )
		return;
	already_called = true;
	
	EXITING_PROGRAM = true;
	dbListItem::appExitingProcedures ();
	BackupDialog::doDayBackup ();
	deleteInstances ();
	qApp->exit ( err_code );
}

void Sys_Init::loadDataIntoMemory ()
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