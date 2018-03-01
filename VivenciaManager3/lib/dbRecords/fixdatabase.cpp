#include "fixdatabase.h"
#include "global.h"
#include "fileops.h"
#include "system_init.h"
#include "vmnotify.h"
#include "fixdatabaseui.h"
#include "heapmanager.h"
#include "keychain/passwordsessionmanager.h"

#include <QFile>
#include <QTextStream>
#include <QApplication>

static const QString strOutError ( QStringLiteral ( "myisamchk: error:" ) );
static const QString strOutTableUpperLimiter ( QStringLiteral ( "Checking MyISAM file" ) );
static const QLatin1String strOutTableLowerLimiter ( "------" );
static const QLatin1String fixApp ( "myisamchk" );
static const QLatin1String fixAppVerify ( " -e " );
static const QLatin1String fixAppSafeRecover ( " --safe-recover --correct-checksum --backup " );
static const QString mysqlLibDir ( QStringLiteral ( "/var/lib/mysql/%1/%2.MYI > %3 2>&1" ) );
static const QLatin1String tempFile ( "/tmp/.vm-fixdb" );

static void commandQueue ( QString& main_command )
{
	main_command += QLatin1String ( "; " ) + Sys_Init::commandMySQLServer ( QStringLiteral ( "stop" ), emptyString, true ) +
					QLatin1String ( "; " ) + Sys_Init::commandMySQLServer ( QStringLiteral ( "start" ), emptyString, true );
}

fixDatabase::fixDatabase ()
	: m_badtables ( 5 ), b_needsfixing ( false )
{}

fixDatabase::~fixDatabase ()
{
	m_badtables.clear ( true );
}

bool fixDatabase::readOutputFile ( const QString& r_passwd )
{
	bool ok ( false );
	if ( fileOps::exists ( tempFile ).isOn () )
	{
		QFile file ( tempFile );
		if ( file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
		{
			mStrOutput = file.readAll ();
			file.close ();
			ok = true;
		}
		static_cast<void>( fileOps::sysExec ( sudoCommand.arg ( r_passwd, QStringLiteral ( "rm -f " ) + tempFile ) ) );
	}
	return ok;
}

bool fixDatabase::checkDatabase ()
{
	// The server should be stopped for this operation to avoid making more errors when trying to fix them
	QString r_passwd;
	if ( APP_PSWD_MANAGER ()->getPassword_UserInteraction ( r_passwd, SYSTEM_ROOT_SERVICE, SYSTEM_ROOT_PASSWORD_ID,
														APP_TR_FUNC ( "In order to perform the check you need to have root privileges. Please type root's password." ) ) )
	{
		QString command ( fixApp + fixAppVerify + mysqlLibDir.arg ( DB_NAME, QStringLiteral ( "*" ), tempFile ) );
		commandQueue ( command );
		static_cast<void>( fileOps::sysExec ( sudoCommand.arg ( r_passwd, QLatin1String ( "rm -f " ) + tempFile ) ) );

		if ( readOutputFile ( r_passwd ) )
		{
			b_needsfixing = false;
			m_badtables.clear ( true );
			int idx ( mStrOutput.indexOf ( strOutTableUpperLimiter ) );
			int idx2 ( 0 );
			int idx3 ( 0 );
			const int len1 ( strOutTableUpperLimiter.length () + 1 );
			const int len2 ( DB_NAME.count () + 1 );
			const int len3 ( strOutError.length () + 1 );
			badTable* bt ( nullptr );

			while ( idx != -1 )
			{
				idx = mStrOutput.indexOf ( QStringLiteral ( ".MYI" ), idx2 + len1 );
				if ( idx == -1 )
					break;
				idx2 = mStrOutput.indexOf ( DB_NAME, idx2 + len1 ) + len2;
				if ( idx2 == -1 )
					break;
				bt = new badTable;
				bt->table = mStrOutput.mid ( idx2, idx - idx2 );
				idx = mStrOutput.indexOf ( strOutError, idx2 + 1 );
				if ( idx != -1 ) {
					idx3 = mStrOutput.indexOf ( strOutTableLowerLimiter, idx2 );
					if ( idx < idx3 )
					{
						idx2 = mStrOutput.indexOf ( CHR_NEWLINE, idx + len3 );
						idx += len3;
						bt->err = mStrOutput.mid ( idx , idx2 - idx );
						bt->result = CR_TABLE_CORRUPTED;
						b_needsfixing = true;
					}
					else
					{
						bt->err = QStringLiteral ( "OK" );
						bt->result = CR_OK;
					}
				}
				else
				{
					bt->err = QStringLiteral ( "OK" );
					bt->result = CR_OK;
				}
				m_badtables.append ( bt );
				idx = idx2 = mStrOutput.indexOf ( strOutTableUpperLimiter, idx2 + 1 );
			}
			// If we reach here, the password worked. Save it for the session
			APP_PSWD_MANAGER ()->savePassword ( passwordSessionManager::SaveSession, r_passwd, SYSTEM_ROOT_SERVICE, SYSTEM_ROOT_PASSWORD_ID );
			return true;
		}
	}
	return false;
}

bool fixDatabase::fixTables ( const QString& tables )
{
	QString r_passwd;
	if ( APP_PSWD_MANAGER ()->getPassword_UserInteraction ( r_passwd, SYSTEM_ROOT_SERVICE, SYSTEM_ROOT_PASSWORD_ID,
														APP_TR_FUNC ( "In order to fix the database, you need to have root privileges." ) ) )
	{

		QString command ( fixApp + fixAppSafeRecover + mysqlLibDir.arg ( DB_NAME, tables, tempFile ) );
		commandQueue ( command );
		static_cast<void>( fileOps::sysExec ( sudoCommand.arg ( r_passwd, command ), emptyString ) );
		if ( readOutputFile ( r_passwd ) )
			return ( !mStrOutput.contains ( QStringLiteral ( "err" ) ) && mStrOutput.contains ( QStringLiteral ( "recovering" ) ) );
	}
	return false;
}
