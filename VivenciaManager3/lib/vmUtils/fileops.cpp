#include "fileops.h"
#include "global.h"
#include "vmnotify.h"
#include "configops.h"
#include "emailconfigdialog.h"

#include <cstdio>
#include <cstdlib>
#include <ctime>

#include <QFile>
#include <QFileDialog>
#include <QCoreApplication>
#include <QProcess>

extern "C"
{
#include <sys/stat.h>
#include <pwd.h>
#include <unistd.h>
#include <dirent.h>
}

const QString fileOps::appPath ( const QString& appname )
{
	const QString strPATH ( QLatin1String ( ::getenv ( "PATH" ) ) );

	if ( !strPATH.isEmpty () )
	{
		QString path;
		int idx1 = 0;
		int idx2 ( strPATH.indexOf ( CHR_COLON ) );
		while ( idx2 != -1 )
		{
			path = strPATH.mid ( idx1, idx2 - idx1 );
			path += CHR_F_SLASH + appname;
			if ( exists ( path ).isOn () )
				return path;
			else
			{
				idx1 = idx2 + 1;
				idx2 = strPATH.indexOf ( CHR_COLON, idx2 + 1 );
			}
		}
		path = strPATH.mid ( idx1, strPATH.length () - 1 );
		path += CHR_F_SLASH + appname;
		if ( exists ( path ).isOn () )
			return path;
	}
	return emptyString;
}

triStateType fileOps::exists ( const QString& file )
{
	if ( !file.isEmpty () )
	{
		struct stat stFileInfo;
		return ( ::stat ( file.toUtf8 ().constData (), &stFileInfo ) == 0 ) ? TRI_ON : TRI_OFF;
	}
	return TRI_UNDEF;
}

const QString fileOps::currentUser ()
{
	return QLatin1String ( ::getenv ( "USER" ) );
}

triStateType fileOps::isDir ( const QString& param )
{
	if ( !param.isEmpty () )
	{
		struct stat stFileInfo;
		if ( ::stat ( param.toUtf8 (), &stFileInfo ) == 0 )
			return ( static_cast<bool> ( S_ISDIR ( stFileInfo.st_mode ) ) ) ? TRI_ON : TRI_OFF;
	}
	return TRI_UNDEF;
}

triStateType fileOps::isLink ( const QString& param )
{
	if ( !param.isEmpty () )
	{
		struct stat stFileInfo;
		if ( ::stat ( param.toUtf8 (), &stFileInfo ) == 0 )
			return ( static_cast<bool> ( S_ISLNK ( stFileInfo.st_mode ) ) ) ? TRI_ON : TRI_OFF;
	}
	return TRI_UNDEF;
}

triStateType fileOps::isFile ( const QString& param )
{
	if ( !param.isEmpty () )
	{
		struct stat stFileInfo;
		if ( ::stat ( param.toUtf8 (), &stFileInfo ) == 0 )
			return ( static_cast<bool> ( S_ISREG ( stFileInfo.st_mode ) ) ) ? TRI_ON : TRI_OFF;
	}
	return TRI_UNDEF;
}

long int fileOps::fileSize ( const QString& filepath )
{
	if ( isFile ( filepath ).isOn () )
	{
		struct stat stFileInfo;
		if ( ::stat ( filepath.toUtf8 (), &stFileInfo ) == 0 )
			return stFileInfo.st_size;
	}
	return -1;
}

vmNumber fileOps::modifiedDate ( const QString& path )
{
	vmNumber ret;
	if ( !path.isEmpty () )
	{
		struct stat stFileInfo;
		if ( ::stat ( path.toUtf8 (), &stFileInfo ) == 0 )
		{
			time_t filetime ( stFileInfo.st_mtime );
			struct tm* __restrict date ( ::localtime ( &filetime ) );
			ret.setDate ( static_cast<int> ( date->tm_mday ),
						  static_cast<int> ( date->tm_mon ) + 1,
						  static_cast<int> ( date->tm_year ) + 1900
						);
		}
	}
	return ret;
}

triStateType fileOps::canRead ( const QString& path )
{
	struct stat stFileInfo;
	if ( ::stat ( path.toUtf8 (), &stFileInfo ) == 0 )
	{
		bool r_ok ( false );
		const QString username ( QLatin1String ( ::getenv ( "USER" ) ) );
		struct passwd* pwd ( new struct passwd );
		const size_t buffer_len ( static_cast<size_t>(::sysconf ( _SC_GETPW_R_SIZE_MAX )) * sizeof ( char ) );
		char* __restrict buffer ( new char[buffer_len] );
		::getpwnam_r ( username.toUtf8 (), pwd, buffer, buffer_len, &pwd );
		if ( ( stFileInfo.st_mode & S_IRUSR ) == S_IRUSR ) /* Read by owner.*/
			r_ok = ( stFileInfo.st_uid == pwd->pw_uid ); // file can be read by username

		if ( !r_ok )
		{ // not read by owner or uids don't match
			if ( ( stFileInfo.st_mode & S_IRGRP ) == S_IRGRP ) /* Read by group.*/
				r_ok = ( stFileInfo.st_gid == pwd->pw_gid ); // file cannot be read by username directly. But can indirectly because of group permissions;
			if ( !r_ok )
			{ // cannot be read by group or gids don't match
				if ( ( stFileInfo.st_mode & S_IROTH ) == S_IROTH )
				{ /* Read by others.*/
					// file cannot be read by username directly. But can indirectly because of other permissions
					r_ok = true;
				}
			}
		}
		delete pwd;
		delete [] buffer;
		return triStateType ( r_ok );
	}
	return TRI_UNDEF;
}

triStateType fileOps::canWrite ( const QString& path )
{
	struct stat stFileInfo;
	if ( ::stat ( path.toUtf8 (), &stFileInfo ) == 0 )
	{
		bool w_ok ( false );
		const QString username ( QLatin1String ( ::getenv ( "USER" ) ) );
		struct passwd* pwd ( new struct passwd );
		const size_t buffer_len ( static_cast<size_t>(::sysconf ( _SC_GETPW_R_SIZE_MAX )) * sizeof ( char ) );
		char* __restrict buffer ( new char[buffer_len] );
		::getpwnam_r ( username.toUtf8 (), pwd, buffer, buffer_len, &pwd );
		if ( ( stFileInfo.st_mode & S_IWUSR ) == S_IWUSR ) /* Write by owner.*/
			w_ok = ( stFileInfo.st_uid == pwd->pw_uid ); // file can be written by username

		if ( !w_ok )
		{ // not written by owner or uids don't match
			if ( ( stFileInfo.st_mode & S_IWGRP ) == S_IWGRP ) /* Read by group.*/
				w_ok = ( stFileInfo.st_gid == pwd->pw_gid ); // file cannot be written by username directly. But can indirectly because of group permissions;
			if ( !w_ok )
			{ // cannot be written by group or gids don't match
				if ( ( stFileInfo.st_mode & S_IWOTH ) == S_IWOTH )
				{ /* Read by others.*/
					// file cannot be written by username directly. But can indirectly because of other permissions
					w_ok = true;
				}
			}
		}
		delete pwd;
		delete [] buffer;
		return triStateType ( w_ok );
	}
	return TRI_UNDEF;
}

triStateType fileOps::canExecute ( const QString& path )
{
	struct stat stFileInfo;
	if ( ::stat ( path.toUtf8 (), &stFileInfo ) == 0 )
	{
		bool x_ok ( false );
		const QString username ( QLatin1String ( ::getenv ( "USER" ) ) );
		struct passwd* pwd ( new struct passwd );
		const size_t buffer_len ( static_cast<size_t>(::sysconf ( _SC_GETPW_R_SIZE_MAX )) * sizeof (char) );
		char* __restrict buffer ( new char[buffer_len] );
		::getpwnam_r ( username.toUtf8 (), pwd, buffer, buffer_len, &pwd );
		if ( ( stFileInfo.st_mode & S_IXUSR ) == S_IXUSR ) /* Execute by owner.*/
			x_ok = ( stFileInfo.st_uid == pwd->pw_uid ); // file can be executed by username
		if ( !x_ok )
		{
			// not executed by owner or uids don't match
			if ( ( stFileInfo.st_mode & S_IXGRP ) == S_IXGRP ) /* Execute by group.*/
				x_ok = ( stFileInfo.st_gid == pwd->pw_gid ); // file cannot be executed by username directly. But can indirectly because of group permissions;
			if ( !x_ok )
			{
				// cannot be executed by group or gids don't match
				if ( ( stFileInfo.st_mode & S_IXOTH ) == S_IXOTH ) /* Executed by others. */
				{
					// file cannot be executed by username directly. But can indirectly because of other permissions
					x_ok = true;
				}
			}
		}
		delete pwd;
		delete [] buffer;
		return triStateType ( x_ok );
	}
	return TRI_UNDEF;
}

triStateType fileOps::createDir ( const QString& path )
{
	triStateType ret ( isDir ( path ) );
	if ( ret == TRI_UNDEF )
	{
		// Either path is an empty string, and so the next function will yield -1 and we will not
		// proceed, returning TRI_UNDEF as the default result for empty parameters, or path does not
		// exists because the ::stat func inside isDir could not find it
		const int idx ( path.lastIndexOf ( CHR_F_SLASH, ( path.at ( path.length () - 1 ) == CHR_F_SLASH ) ? -2 : -1 ) );
		if ( idx != -1 )
		{
			ret = createDir ( path.left ( idx ) );
			if ( ret == TRI_ON )
			{
				QString new_path ( path );
				if ( new_path.at ( new_path.length () - 1 ) != CHR_F_SLASH )
					new_path.append ( CHR_F_SLASH );
				struct stat stFileInfo;
				::stat ( new_path.left ( idx ).toUtf8 (), &stFileInfo );
				return ( ::mkdir ( new_path.toUtf8 (), stFileInfo.st_mode ) == 0 ) ? TRI_ON : TRI_OFF;
			}
		}
	}
	return ret;
}

bool fileOps::copyFile ( const QString& dst, const QString& src )
{
	QString mutable_dst ( dst );

	bool ret ( false );
	if ( exists ( src ).isOn () )
	{
		if ( isFile ( dst ).isOn () )
		{
			if ( canWrite ( dst ).isOn () )
			{
				if ( VM_NOTIFY ()->questionBox ( APP_TR_FUNC ( "Same filename" ), QString ( APP_TR_FUNC (
													 "Destination file %1 already exists. Overwrite it?" ) ).arg ( dst ) ) )
					ret = ::remove ( dst.toLocal8Bit () ); // if remove succeeds there is not need to check if dst's path is writable
			}
		}
		else if ( isDir ( dst ).isOn () )
		{
			if ( canWrite ( dst ).isOn () )
			{
				ret = true;
				if ( mutable_dst.at ( mutable_dst.length () - 1 ) != CHR_F_SLASH )
					mutable_dst.append ( CHR_F_SLASH );
				mutable_dst.append ( fileNameWithoutPath ( src ) );
			}
		}
		else //dst does not exist
			ret = !createDir ( mutable_dst.left ( mutable_dst.lastIndexOf ( CHR_F_SLASH ) ) ).isOff ();

		if ( ret )
			ret = QFile::copy ( src, mutable_dst );
	}
	return ret;
}

triStateType fileOps::rename ( const QString& old_name, const QString& new_name )
{
	return ( ::rename ( old_name.toLocal8Bit (), new_name.toLocal8Bit () ) == 0 ) ? TRI_ON : TRI_OFF;
}

triStateType fileOps::removeFile ( const QString& filename )
{
	if ( isFile ( filename ) == TRI_ON )
		return ( ::remove ( filename.toUtf8 () ) == 0 ) ? TRI_ON : TRI_OFF;
	return TRI_UNDEF;
}

const QString fileOps::nthDirFromPath ( const QString& c_path, const int n )
{
	QString dir;
	if ( !c_path.isEmpty () )
	{
		QString path ( c_path );
		if ( isDir ( path ).isOn () )
		{
			if ( path.at ( path.length () -1 ) != CHR_F_SLASH )
				path.append ( CHR_F_SLASH );
		}
		int idx ( -1 ), idx2 ( 0 );
		if ( n == -1 ) // last dir
		{
			idx = path.lastIndexOf ( CHR_F_SLASH, -2 );
			if ( idx != -1 )
			{
				dir = path.mid ( idx + 1, path.length () - idx - 1 );
				if ( !isDir ( c_path.left ( idx + 1 ) + dir ).isOn () )
				{
					idx2 = path.lastIndexOf ( CHR_F_SLASH, 0 - ( path.length () - idx ) - 1 );
					dir = path.mid ( idx2 + 1, idx - idx2 );
				}
			}
		}
		else
		{
			int i = 0;
			while ( ( idx = path.indexOf ( CHR_F_SLASH, idx2 + 1 ) ) != -1 )
			{
				if ( i == 0 ) //include leading '/' only for the top level dir
					dir = path.mid ( idx2, idx - idx2 + 1 );
				else
					dir = path.mid ( idx2 + 1, idx - idx2 );
				if ( i == n )
					return dir;
				idx2 = idx;
				++i;
			}
			dir.clear (); // n is bigger than the number of dirs and subdir
		}
	}
	return dir;
}

// Returns dir name from path. The filename does not need to exists. If path is null/empty or a dir, returns itself
const QString fileOps::dirFromPath ( const QString& path )
{
	if ( !isDir ( path ).isOn () )
	{
		const int idx ( path.lastIndexOf ( CHR_F_SLASH ) );
		if ( idx != -1 )
			return path.left ( idx + 1 );
	}
	return path;
}

const QString fileOps::fileExtension ( const QString& filepath )
{
	const int idx ( filepath.indexOf ( CHR_DOT ) );
	if ( idx != -1 )
		return filepath.right ( filepath.length () - idx - 1 );
	return emptyString;
}

const QString fileOps::fileNameWithoutPath ( const QString& filepath )
{
	const int idx ( filepath.lastIndexOf ( CHR_F_SLASH ) );
	if ( idx != -1 )
		return filepath.right ( filepath.length () - idx - 1 );
	return filepath;
}

const QString fileOps::filePathWithoutExtension ( const QString& filepath )
{
	const int idx ( filepath.indexOf ( CHR_DOT ) );
	if ( idx != -1 )
		return filepath.left ( idx );
	return filepath;
}

const QString fileOps::replaceFileExtension ( const QString& filepath, const QString& new_ext )
{
	return QString ( filePathWithoutExtension ( filepath ) + CHR_DOT + new_ext );
}

void fileOps::lsDir ( PointersList<st_fileInfo*>& result, const QString& baseDir,
					  const QStringList& name_filters, const QStringList& exclude_filter, const int filter, const int follow_into )
{
	DIR* __restrict dir ( nullptr );
	if ( ( dir = ::opendir ( baseDir.toUtf8 ().constData () ) ) != nullptr )
	{
		struct dirent* __restrict dir_ent ( nullptr );
		QString filename, pathname;
		static const QString dot ( CHR_DOT );
		static const QString doubleDot ( QStringLiteral ( ".." ) );
		bool ok ( false );

		while ( ( dir_ent = ::readdir ( dir ) ) != nullptr )
		{
			filename = QString::fromUtf8 ( dir_ent->d_name );
			if ( filename == dot || filename == doubleDot )
				continue;
			if ( !( filter & LS_HIDDEN_FILES ) )
			{
				if ( filename.startsWith ( CHR_DOT ) || filename.startsWith ( CHR_TILDE ) )
					continue;
			}
			
			ok = true;
			for ( int i ( 0 ); i < exclude_filter.count (); ++i )
			{
				if ( filename == exclude_filter.at ( i ) )
				{
					ok = false;
					break;
				}
			}
			if ( !ok )
				continue;
			
			pathname = baseDir + CHR_F_SLASH + filename;
			
			ok = ( filter & LS_FILES ) && ( dir_ent->d_type == DT_REG );
			if ( !ok )
				ok = ( filter & LS_DIRS ) && ( dir_ent->d_type == DT_DIR );
			else
			{
				const int filter_n ( name_filters.count () );
				if ( filter_n > 0 )
				{
					int i ( 0 );
					for ( ; i < filter_n; ++i )
					{
						if ( filename.contains ( name_filters.at ( i ), Qt::CaseInsensitive ) )
							break;
					}
					if ( i >= filter_n )
						ok = false;
				}
			}

			if ( ok )
			{
				st_fileInfo* fi ( new st_fileInfo );
				result.append ( fi );
				fi->filename = filename;
				fi->fullpath = pathname;
				if ( dir_ent->d_type == DT_REG )
					fi->is_file = true;
				else
				{
					fi->is_dir = true;
					if ( follow_into != 0 )
						lsDir ( result, pathname, name_filters, exclude_filter, filter,
								follow_into == -1 ? -1 : follow_into - 1 );
				}
			}
		}
		::closedir ( dir );
	}
}

bool fileOps::rmDir ( const QString& baseDir,
					  const QStringList& name_filters, const int filter, const int follow_into )
{
	bool ok ( false );
	static int level ( 0 );
	++level;
	DIR* __restrict dir ( nullptr );
	if ( ( dir = ::opendir ( baseDir.toUtf8 () ) ) != nullptr )
	{	
		struct dirent* __restrict dir_ent ( nullptr );
		QString filename, pathname;

		while ( ( dir_ent = ::readdir ( dir ) ) != nullptr )
		{
			filename = QString::fromUtf8 ( dir_ent->d_name );
			if ( filename == QStringLiteral ( "." ) || filename == QStringLiteral ( ".." ) )
				continue;

			pathname = baseDir + CHR_F_SLASH + filename;
			ok = ( filter & LS_FILES ) && ( dir_ent->d_type == DT_REG );
			if ( !ok )
				ok = ( filter & LS_DIRS ) && ( dir_ent->d_type == DT_DIR );
			else
			{
				const int filter_n ( name_filters.count () );
				if ( filter_n > 0 )
				{
					int i ( 0 );
					for ( ; i < filter_n; ++i )
					{
						if ( filename.contains ( name_filters.at ( i ), Qt::CaseInsensitive ) )
							break;
					}
					if ( i >= filter_n )
						ok = false;
				}
			}

			if ( ok )
			{
				if ( dir_ent->d_type == DT_REG )
					::remove ( pathname.toUtf8 () );
				else
				{
					if ( follow_into != 0 )
						(void) rmDir ( pathname, name_filters, filter, follow_into == -1 ? -1 : follow_into - 1 );
					ok = ( ::rmdir ( pathname.toUtf8 () ) == 0 );
				}
			}
		}
		if ( --level == 0 )
			ok = ( ::rmdir ( baseDir.toUtf8 () ) == 0 );
		::closedir ( dir );
	}
	return ok;
}

static QString suProgram ( const QString& message, const QString& command )
{
	QString cmdLine;
	switch ( fileOps::detectDesktopEnvironment () )
	{
		case fileOps::DesktopEnv_Plasma5:
		case fileOps::DesktopEnv_Kde4:
			cmdLine = configOps::kdesu ( message );
		break;
		case fileOps::DesktopEnv_Gnome:
		case fileOps::DesktopEnv_Unity:
		case fileOps::DesktopEnv_Xfce:
		case fileOps::DesktopEnv_Other:
			cmdLine = configOps::gksu ( message, emptyString );
	}
	cmdLine += CHR_SPACE + CHR_QUOTES + command + CHR_QUOTES;
	return cmdLine;
}

int fileOps::sysExec ( const QStringList &command_line, const QString& as_root_message )
{
	return sysExec ( command_line.join ( emptyString ), as_root_message );
}

int fileOps::sysExec ( const QString& command_line, const QString& as_root_message )
{
	QString cmdLine ( QSTRING_ENCODING_FIX(command_line) );
	if ( !as_root_message.isEmpty () )
		cmdLine = suProgram ( as_root_message, cmdLine );
	return ::system ( cmdLine.toLocal8Bit () );
}

bool fileOps::executeWait ( const QString& arguments, const QString& program,
							int* __restrict exitCode, const QString& as_root_message )
{
	QProcess* __restrict proc ( new QProcess () );
	QString prog ( program );
	if ( !arguments.isEmpty () )
	{
		prog += CHR_SPACE;
		prog += CHR_QUOTES + QSTRING_ENCODING_FIX ( arguments ) + CHR_QUOTES;
	}
	if ( !as_root_message.isEmpty () )
		prog = suProgram ( as_root_message, prog );

	proc->start ( prog );
	const bool ret ( proc->waitForFinished ( -1 ) );
	if ( exitCode != nullptr )
	{
		*exitCode = proc->exitCode ();
	}
	delete proc;
	return ret;
}

const QString fileOps::executeAndCaptureOutput ( const QString& arguments, const QString& program,
												 int* __restrict exitCode, const QString& as_root_message )
{
	QProcess* __restrict proc ( new QProcess () );
	QString prog ( program );
	if ( !arguments.isEmpty () )
	{
		prog += CHR_SPACE;
		prog += QSTRING_ENCODING_FIX( arguments );
	}
	if ( !as_root_message.isEmpty () )
		prog = suProgram ( as_root_message, prog );

	proc->start ( prog );
	proc->waitForFinished ();
	const QString output ( QString::fromUtf8 ( proc->readAllStandardOutput ().constData () ) );
	if ( exitCode != nullptr ) {
		*exitCode = proc->exitCode ();
	}
	delete proc;
	return output;
}

bool fileOps::executeWithFeedFile ( const QString& arguments, const QString& program,
									const QString& filename, int* __restrict exitCode,
									const QString& as_root_message )
{
	QProcess* __restrict proc ( new QProcess () );
	QString prog ( program );
	if ( !arguments.isEmpty () )
	{
		prog += CHR_SPACE;
		prog += QSTRING_ENCODING_FIX( arguments );
	}
	if ( !as_root_message.isEmpty () )
		prog = suProgram ( as_root_message, prog );

	proc->setStandardInputFile ( filename );
	proc->start ( prog );
	const bool ret ( proc->waitForFinished () );
	if ( exitCode != nullptr )
	{
		*exitCode = proc->exitCode ();
	}
	delete proc;
	return ret;
}

bool fileOps::execute ( const QString& arguments, const QString& program )
{
	bool ret ( false );
	QProcess* __restrict proc ( new QProcess () );
	const QString prog ( program + CHR_SPACE + CHR_QUOTES + QSTRING_ENCODING_FIX( arguments ) + CHR_QUOTES );
	ret = proc->startDetached ( prog );
	delete proc;
	return ret;
}

void fileOps::openAddress ( const QString& address )
{
	if ( address.contains ( QLatin1Char ( '@' ) ) )
		EMAIL_CONFIG ()->sendEMail ( address );
	else
	{
		if ( address.contains ( QStringLiteral ( "www" ) ) )
		{
			if ( !address.startsWith ( QStringLiteral ( "http://" ) ) )
			{
				execute ( QLatin1String ( "http://" ) + address, XDG_OPEN );
				return;
			}
		}
		execute ( address, XDG_OPEN );
	}
}

QString fileOps::getExistingDir ( const QString& dir )
{
	return QFileDialog::getExistingDirectory ( nullptr, APP_TR_FUNC ( "Choose dir " ), QSTRING_ENCODING_FIX( dir ) );
}

QString fileOps::getOpenFileName ( const QString& dir, const QString& filter )
{
	return QFileDialog::getOpenFileName ( nullptr, APP_TR_FUNC ( "Choose file" ), QSTRING_ENCODING_FIX( dir ), filter );
}

QString fileOps::getSaveFileName ( const QString& default_name, const QString& filter )
{
	return QFileDialog::getSaveFileName ( nullptr, APP_TR_FUNC ( "Save as ..." ), default_name, filter );
}

fileOps::DesktopEnvironment fileOps::detectDesktopEnvironment ()
{
	// session/desktop env variables: XDG_SESSION_DESKTOP or XDG_CURRENT_DESKTOP
	const QString xdgCurrentDesktop ( QLatin1String ( ::getenv ( "XDG_CURRENT_DESKTOP" ) ) );
	if ( xdgCurrentDesktop == QStringLiteral ( "KDE" ) )
		return getKDEVersion ();
	else if ( xdgCurrentDesktop == QStringLiteral ( "GNOME" ) )
		return DesktopEnv_Gnome;
	else if ( xdgCurrentDesktop == QStringLiteral ( "Unity" ) )
		return DesktopEnv_Unity;
	else if ( xdgCurrentDesktop.contains ( QStringLiteral ( "xfce" ), Qt::CaseInsensitive )
			|| xdgCurrentDesktop.contains ( QStringLiteral ( "xubuntu" ), Qt::CaseInsensitive ) )
		return DesktopEnv_Xfce;
	else
		return DesktopEnv_Other;
}

// the following detection algorithm is derived from chromium,
// licensed under BSD, see base/nix/xdg_util.cc
fileOps::DesktopEnvironment fileOps::getKDEVersion ()
{
	const QString value ( QLatin1String ( ::getenv ( "KDE_SESSION_VERSION" ) ) );
	if ( value == QStringLiteral ( "5" ) )
		return DesktopEnv_Plasma5;
	else if ( value == QStringLiteral ( "4" ) )
		return DesktopEnv_Kde4;
	else // most likely KDE3
		return DesktopEnv_Other;
}
