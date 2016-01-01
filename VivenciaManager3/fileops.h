#ifndef FILEOPS_H
#define FILEOPS_H

#include "vmnumberformats.h"
#include "vmlist.h"
#include "tristatetype.h"

#include <QString>

class fileOps
{

public:

	enum LS_FILTER {
		LS_DIRS			= 0x001,
		LS_FILES		= 0x002,
		LS_HIDDEN_FILES	= 0x004 | LS_FILES,
		LS_ALL			= LS_DIRS | LS_FILES | LS_HIDDEN_FILES
	};

	struct st_fileInfo {
		QString filename;
		QString fullpath;
		bool is_dir;
		bool is_file;

		st_fileInfo () : is_dir ( false ), is_file ( true ) {}
	};

	constexpr explicit inline fileOps () {}

	static const QString appPath ( const QString& appname );
    static triStateType exists ( const QString& file );

	static const QString currentUser ();

    static triStateType isDir ( const QString& param );
    static triStateType isLink ( const QString& param );
    static triStateType isFile ( const QString& param );

	static vmNumber modifiedDate ( const QString& path );

    static triStateType canRead ( const QString& path );
    static triStateType canWrite ( const QString& path );
    static triStateType canExecute ( const QString& path );

    static triStateType createDir ( const QString& path );
    static bool copyFile ( const QString& dst, const QString& src );

    static triStateType removeFile ( const QString& filename );
    static triStateType rename ( const QString& old_name, const QString& new_name );

	static const QString nthDirFromPath ( const QString& path, const int n = -1 );
	static const QString dirFromPath ( const QString& path );
	static const QString fileExtension ( const QString& filepath );
	static const QString fileNameWithoutPath ( const QString& filepath );
	static const QString filePathWithoutExtension ( const QString& filepath );
	static const QString replaceFileExtension ( const QString& filepath, const QString& new_ext );

	static void lsDir ( PointersList<st_fileInfo*>& result, const QString& baseDir, const QStringList& name_filters,
						const int filter = LS_FILES, const int follow_into = 0 );

    static bool rmDir ( const QString& baseDir, const QStringList& name_filters,
						const int filter = LS_FILES, const int follow_into = 0 );

	static int sysExec ( const QStringList &command_line, const QString& as_root_message = QString::null );
	static int sysExec ( const QString& command_line, const QString& as_root_message = QString::null );
    static bool executeWait ( const QString& arguments, const QString& program, int* exitCode = nullptr, const QString& as_root_message = QString::null );
    static const QString executeAndCaptureOutput ( const QString& arguments, const QString& program, int* exitCode = nullptr, const QString& as_root_message = QString::null );
    static bool executeWithFeedFile ( const QString& arguments, const QString& program, const QString& filename, int* exitCode = nullptr, const QString& as_root_message = QString::null );
	static bool execute ( const QString& arguments, const QString& program );
	static void openAddress ( const QString& address );

    static QString getExistingDir ( const QString& dir );
    static QString getOpenFileName ( const QString& dir, const QString& filter );
    static QString getSaveFileName ( const QString& dir, const QString& filter );
};

#endif // FILEOPS_H
