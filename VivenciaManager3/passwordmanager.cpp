#include "passwordmanager.h"
#include "global.h"
#include "vmnotify.h"
#include "fileops.h"
#include "configops.h"
#include "cleanup.h"

#include <QFile>
#include <QWidget>
#include <QCoreApplication>
#include <QStringList>
#include <QDataStream>

//TODO test everything


static const QString testCmdDir ( QStringLiteral ( "/tmp/.testcmd" ) );
static const QString testCmd1 ( QStringLiteral ( "mkdir /tmp/.testcmd" ) );
static const QString testCmd2 ( QStringLiteral ( "rmdir /tmp/.testcmd" ) );
static const QString rootID ( QStringLiteral ( "toor" ) );
static const QString rootFile ( QStringLiteral ( "/tmp/.toorhash" ) );

void removeTempFiles ()
{
	PointersList<fileOps::st_fileInfo*> temp_files;
	fileOps::lsDir ( temp_files, CONFIG ()->appDataDir (), QStringList () << QLatin1String ( ".1pwd" ), fileOps::LS_HIDDEN_FILES );
	for ( uint i ( 0 ); i < temp_files.count (); ++i )
		fileOps::removeFile ( temp_files.at ( i )->fullpath );
	temp_files.clear ( true );
	fileOps::removeFile ( rootFile );
}

static bool testSudoPasswd ( const QString& passwd )
{
	( void )fileOps::sysExec ( sudoCommand.arg ( passwd, testCmd1 ), emptyString );
    if ( fileOps::exists ( testCmdDir ).isOn () ) {
		if ( fileOps::sysExec ( sudoCommand.arg ( passwd, testCmd2 ), emptyString ) == 0 )
			return true;
	}
	VM_NOTIFY ()->notifyMessage ( QCoreApplication::tr ( "Password test failed" ), QCoreApplication::tr ( "Wrong sudo password" ) );
	return false;
}

passwordManager::passwordManager ( const PassWdSaveState s_state )
	: ms_state ( s_state ), mFile ( nullptr ), mbUpToDate ( true )
{
	addPostRoutine ( removeTempFiles, true );
	if ( ms_state != PWSS_DO_NOT_SAVE ) {
		mFilename = CONFIG ()->appDataDir () + CHR_DOT +
				QLatin1String ( "vmpm." ) + QString::number ( ms_state ) + QLatin1String ( "pwd" );
		mFile = new QFile ( mFilename );
		if ( ms_state == PWSS_SAVE )
			load ();
	}
}

passwordManager::~passwordManager ()
{
	if ( !mbUpToDate )
		toFile ( mFile );
}

bool passwordManager::insert ( const QString& id, const QString& password )
{
	if ( mPairs.insert ( id, password ).key () == id ) {
		mbUpToDate = !( ms_state != PWSS_DO_NOT_SAVE ); //only signal that disk file is not up to date when we are asked to save the passwords
		return true;
	}
	return false;
}

bool passwordManager::remove ( const QString& id )
{
	if ( mPairs.remove ( id ) >= 1 ) {
		mbUpToDate = !( ms_state != PWSS_DO_NOT_SAVE ); //only signal that disk file is not up to date when we are asked to save the passwords
		return true;
	}
	return false;
}

bool passwordManager::change ( const QString& id, const QString& oldpasswd, const QString& newpasswd )
{
	if ( mPairs.value ( id ) == oldpasswd )
		return insert ( id, newpasswd );
	return false;
}

bool passwordManager::exists ( const QString& id ) const
{
	return mPairs.contains ( id );
}

QString passwordManager::getPassword ( const QString& id ) const
{
	return mPairs.value ( id );
}

QString passwordManager::getID ( const uint pos ) const
{
	QHash<QString,QString>::const_iterator ite_start ( mPairs.begin () );
	const QHash<QString,QString>::const_iterator ite_end ( mPairs.end () );
	uint x ( 0 );
	while ( x < pos && ite_start != ite_end ) {
		++ite_start;
		++x;
	}
	return ite_start.key ();
}

QString passwordManager::toString ( const QString& id ) const
{
	QDataStream out_string;
	bool ok ( false );
	if ( id.isEmpty () ) {
		out_string << mPairs;
		ok = true;
	}
	else {
		if ( exists ( id ) ) {
			QHash<QString,QString> temp;
			temp.insert ( id, mPairs.value ( id ) );
			out_string << temp;
			ok = true;
		}
	}
	if ( ok ) {
		QString ret;
		out_string >> ret;
		return ret;
	}
	else
		return QString::null;
}

bool passwordManager::fromString ( const QString& hash_str, const bool overwrite )
{
	bool ok ( false );
	if ( !hash_str.isEmpty () ) {
		QDataStream in_string;
		in_string << hash_str;

		if ( !overwrite ) {
			QHash<QString,QString> temp;
			in_string >> temp;
			const QHash<QString,QString>::const_iterator first ( temp.begin () );
			ok = insert ( first.key (), first.value () );
		}
		else {
			mPairs.clear ();
			in_string >> mPairs;
			mbUpToDate = !( ms_state != PWSS_DO_NOT_SAVE ); //only signal that disk file is not up to date when we are asked to save the passwords
			ok = true;
		}
	}
	return ok;
}

bool passwordManager::load ()
{
    if ( fileOps::exists ( mFilename ).isOn () ) {
		if ( !mFile )
			mFile = new QFile ( mFilename );
		return fromFile ( mFile, true );
	}
	return false;
}

bool passwordManager::toFile ( QFile* file, const QString& id ) const
{
	bool ok ( false );
	if ( file != nullptr ) {
		if ( file->open ( QIODevice::Truncate | QIODevice::WriteOnly ) ) {
			QHash<QString,QString> temp;
			if ( id.isEmpty () ) { // do not save root password into user's file
				QHash<QString,QString>::const_iterator ite_start ( mPairs.begin () );
				const QHash<QString,QString>::const_iterator ite_end ( mPairs.end () );
				while ( ite_start != ite_end ) {
					if ( ite_start.key () != rootID )
						temp.insert ( ite_start.key (), ite_start.value () );
					++ite_start;
				}
				file << temp;
				ok = true;
			}
			else {
				if ( exists ( id ) ) { //no need to check for rootID here. This branch is for saving the root password, among others, of course
					temp.insert ( id, mPairs.value ( id ) );
					file << temp;
					ok = true;
				}
			}
			if ( file->fileName () == mFilename )
				const_cast<passwordManager*>( this )->mbUpToDate = true;
			file->close ();
		}
	}
	return ok;
}

bool passwordManager::fromFile ( QFile* const file, const bool overwrite )
{
	bool ok ( false );
	if ( file->open ( QIODevice::ReadOnly ) ) {
		const QString content ( file->readAll () );
		QDataStream in_string;
		in_string << content;
		file->close ();
		if ( !overwrite ) {
			QHash<QString,QString> temp;
			in_string >> temp;
			const QHash<QString,QString>::const_iterator first ( temp.begin () );
			ok = insert ( first.key (), first.value () );
		}
		else {
			mPairs.clear ();
			in_string >> mPairs;
			mbUpToDate = !( ms_state != PWSS_DO_NOT_SAVE ); //only signal that disk file is not up to date when we are asked to save the passwords
			ok = true;
		}
	}
	return ok;
}

bool passwordManager::sudoPassword ( QString& r_passwd, const QString& message )
{
	bool ok ( false );
	if ( exists ( rootID ) )
		ok = testSudoPasswd ( getPassword ( rootID ) );
	else {
		QFile* file ( new QFile ( rootFile ) );
		if ( file->open ( QIODevice::ReadOnly ) ) {
			fromFile ( file );
			ok = testSudoPasswd ( getPassword ( rootID ) );
		}
		else {
			QString passwd;
			if ( vmNotify::inputBox ( passwd, nullptr, QCoreApplication::tr ( "Your account's password needed" ), message,
									 emptyString, emptyString, emptyString, vmCompleters::NONE, true ) ) {
				if ( testSudoPasswd ( passwd ) ) {
					insert ( rootID, passwd );
					toFile ( file, rootID );
					ok = true;
				}
			}
		}
		delete file;
	}
	if ( ok )
		r_passwd = getPassword ( rootID );
	return ok;
}

bool passwordManager::askPassword ( QString &passwd, const QString& id, const QString &message )
{
#ifdef TRANSITION_PERIOD
	passwd = "percival";
	return true;
#endif
	if ( !exists ( id ) ) {
		if ( vmNotify::inputBox ( passwd, nullptr, QCoreApplication::tr ( "Password required" ), message,
					emptyString, emptyString, emptyString, vmCompleters::NONE, true ) ) {
			insert ( id, passwd );
			if ( ms_state != PWSS_DO_NOT_SAVE )
				toFile ( mFile, id );
			return true;
		}
		return false;
	}
	passwd = mPairs.value ( id );
	return true;
}
