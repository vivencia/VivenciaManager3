#include "configops.h"
#include "heapmanager.h"
#include "fileops.h"
#include "textdb.h"

#include <vmStringRecord/stringrecord.h>

#include <QtCore/QTextStream>
#include <QtCore/QStandardPaths>
#include <QtWidgets/QWidget>

QString configOps::_appName;
QString configOps::m_defaultSectionName ( QStringLiteral ( "MAIN_SECTION" ) );

const QString configOps::configDefaultFieldsNames[CFG_CATEGORIES] =
{
	QStringLiteral ( "HOME_DIR" ), QStringLiteral ( "LAST_LOGGED_USER" ),
	QStringLiteral ( "FILEMANAGER" ), QStringLiteral ( "PICTURE_VIEWER" ), QStringLiteral ( "PICTURE_EDITOR" ),
	QStringLiteral ( "DOC_VIEWER" ), QStringLiteral ( "DOC_EDITOR" ), QStringLiteral ( "XLS_EDITOR" ),
	QStringLiteral ( "BASE_PROJECT_DIR" ), QStringLiteral ( "ESTIMATE_DIR" ), QStringLiteral ( "REPORT_DIR" ),
	QStringLiteral ( "BACKUP_DIR" ), QStringLiteral ( "DROPBOX_DIR" ), QStringLiteral ( "EMAIL_ADDRESS" )
};

auto _homeDir = [] () ->QString { return QLatin1String ( ::getenv ( "HOME" ) ) + CHR_F_SLASH; };
auto _configDir = [] () ->QString { return QStandardPaths::standardLocations ( QStandardPaths::ConfigLocation ).at ( 0 ) +
		CHR_F_SLASH + configOps::appName () + CHR_F_SLASH; };

static const QString DEFAULT_OPTS[CFG_CATEGORIES] =
{
	( _homeDir () ),
	( QString () ), // DEFAULT_LAST_LOGGED_USER
	( XDG_OPEN ), // DEFAULT_FILE_MANAGER
	( XDG_OPEN ), // DEFAULT_PICTURE_VIEWER
	( QStringLiteral ( "krita" ) ), // DEFAULT_PICTURE_EDITOR
	( XDG_OPEN ), // DEFAULT_PDF_VIEWER
	( XDG_OPEN ), // DEFAULT_DOC_EDITOR
	( XDG_OPEN ), // DEFAULT_XLS_EDITOR
	/* I am assuming the QStringList has at least one item. I will play along with this because I think it will have at least one item
	 * under all circurmstances. If I encounter an error at some point I will create a static function which will check for emptiness in the list
	 */
	( QStandardPaths::standardLocations ( QStandardPaths::DocumentsLocation ).at ( 0 ) + QLatin1String ( "/Vivencia/" ) ), // DEFAULT_PROJECTS_DIR
	( QStandardPaths::standardLocations ( QStandardPaths::DocumentsLocation ).at ( 0 ) + QString::fromUtf8 ( "/Vivencia/%1/" ) + configOps::estimatesDirSuffix () + CHR_F_SLASH ), // DEFAULT_ESTIMATE_DIR
	( QStandardPaths::standardLocations ( QStandardPaths::DocumentsLocation ).at ( 0 ) + QString::fromUtf8 ( "/Vivencia/%1/" ) + configOps::reportsDirSuffix () + CHR_F_SLASH ), // DEFAULT_REPORTS_DIR
	( QStandardPaths::standardLocations ( QStandardPaths::DocumentsLocation ).at ( 0 ) + QLatin1String ( "/Vivencia/VMDB/" ) ), // BACKUP_DIR
	( _homeDir () ) + QLatin1String ( "Dropbox/" ), // DROPBOX_DIR
	( QStringLiteral ( "vivencia@gmail.com" ) ) // DEFAULT_EMAIL
};

const QString& getDefaultFieldValuesByCategoryName ( const QString& category_name )
{
	for ( int i ( 0 ); i < CFG_CATEGORIES; ++i )
	{
		if ( category_name == configOps::configDefaultFieldsNames[i] )
			return DEFAULT_OPTS[i];
	}
	return emptyString;
}

configOps::configOps ( const QString& filename, const QString& object_name )
	: m_cfgFile ( nullptr )
{
	m_filename = filename.isEmpty () ? appConfigFile () : filename;
	fileOps::createDir ( fileOps::dirFromPath ( m_filename ) );
	m_cfgFile = new configFile ( m_filename, object_name );
	m_cfgFile->load ();
}

configOps::~configOps ()
{
	heap_del ( m_cfgFile );
}

const QString& configOps::setApp ( const CFG_FIELDS field, const QString& app )
{
	mRetString = app.contains ( CHR_F_SLASH ) ? app : fileOps::appPath ( app );
	const QString fieldName ( configDefaultFieldsNames[field ] );
	if ( fileOps::exists ( mRetString ).isOn () )
		setValue ( defaultSectionName (), fieldName, mRetString );
	else
		mRetString = getValue ( defaultSectionName (), fieldName );
	return mRetString;
}

const QString& configOps::setDir ( const CFG_FIELDS field, const QString& dir )
{
	const QString fieldName ( configDefaultFieldsNames[field ] );
	if ( fileOps::isDir ( dir ).isOn () )
	{
		setValue ( defaultSectionName (), fieldName, dir );
		return dir;
	}
	return getValue ( defaultSectionName (), fieldName );
}

const QString& configOps::getValue ( const QString& section_name, const QString& category_name )
{
	if ( m_cfgFile->setWorkingSection ( section_name ) )
		mRetString = m_cfgFile->fieldValue ( category_name );
	if ( mRetString.isEmpty () )
	{
		mRetString = getDefaultFieldValuesByCategoryName ( category_name );
		const_cast<configOps*>( this )->setValue ( section_name, category_name, mRetString ); //insert the default values into the config file
	}
	return mRetString;
}

void configOps::setValue ( const QString& section_name, const QString& category_name, const QString& value )
{
	if ( !m_cfgFile->setWorkingSection ( section_name ) )
		m_cfgFile->insertNewSection ( section_name );
	if ( m_cfgFile->fieldIndex ( category_name ) < 0 )
		m_cfgFile->insertField ( category_name, value );
	else
		m_cfgFile->setFieldValue ( category_name, value );
	m_cfgFile->commit ();
}

const QString configOps::homeDir ()
{
	return _homeDir ();
}

const QString configOps::defaultConfigDir ()
{
	if ( !appName ().isEmpty () )
		return _configDir ();
	else
	{
		qDebug () << "Error: configOps::appName () not set. This must one of the first actions of any program linked against vmUtils.so";
		return QString ();
	}
}

const QString configOps::appConfigFile ()
{
	if ( !appName ().isEmpty () )
		return _configDir () + QLatin1String ( "vmngr.conf" );
	else
	{
		qDebug () << "Error: configOps::appName () not set. This must one of the first actions of any program linked against vmUtils.so";
		return QString ();
	}
}

const QString configOps::appDataDir ()
{
	if ( !appName ().isEmpty () )
		return QStandardPaths::standardLocations ( QStandardPaths::DataLocation ).at ( 0 ) + CHR_F_SLASH;
	else
	{
		qDebug () << "Error: configOps::appName () not set. This must one of the first actions of any program linked against vmUtils.so";
		return QString ();
	}
}

const QString configOps::kdesu ( const QString& message )
{
	QString ret ( fileOps::appPath ( QStringLiteral ( "kdesudo" ) ) );
	if ( ret.isEmpty () )
	{
		ret = QStringLiteral ( "/etc/alternatives/kdesu" );
		if ( !fileOps::fileOps::exists ( ret ).isOn () ) // no kdesudo/kdesu? try gksu. The important thing is to use a sudo program
			return gksu ( message, QString () );
	}
	ret += QLatin1String ( " --comment \"" ) + message + QLatin1String ( "\" -n -d -c " );
	return ret;
}

const QString configOps::gksu ( const QString& message, const QString& appname )
{
	QString ret ( fileOps::appPath ( QStringLiteral ( "gksu" ) ) );
	if ( !ret.isEmpty () )
		ret += QLatin1String ( " -m \"" ) + message + QLatin1String ( "\" -D " ) + appname + QLatin1String ( " -g -k " );
	else
		return kdesu ( message ); // no gksu? try kdesudo/kdesu. The important thing is to use a sudo program
	return ret;
}

bool configOps::isSystem ( const QString& os_name )
{
	bool ret ( false );
	QFile file ( QStringLiteral ( "/etc/issue" ) );
	if ( file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
	{
		QTextStream in ( &file );
		if ( in.readLine ().contains ( os_name, Qt::CaseInsensitive ) )
			ret = true;
		file.close ();
	}
	return ret;
}

bool configOps::initSystem ( const QString& initName )
{
	int exitCode ( -1 );
	fileOps::executeWait ( initName, QStringLiteral ( "pgrep" ), &exitCode );
	return exitCode == 0;
}

const QString& configOps::getProjectBasePath ( const QString& client_name )
{
	mRetString = projectsBaseDir ();
	if ( !client_name.isEmpty () )
		mRetString += client_name + CHR_F_SLASH;
	return mRetString;
}

const QString& configOps::estimatesDir ( const QString& client_name )
{
	mRetString = getValue ( defaultSectionName (), configDefaultFieldsNames[ESTIMATE_DIR] );
	mRetString.replace ( QStringLiteral ( "%1" ), client_name );
	return mRetString;
}

const QString& configOps::setEstimatesDir ( const QString& str, const bool full_path )
{
	return setDir ( ESTIMATE_DIR, full_path ? str :
					DEFAULT_OPTS[ESTIMATE_DIR].left ( DEFAULT_OPTS[ESTIMATE_DIR].indexOf (
								estimatesDirSuffix () + str + CHR_F_SLASH ) )
				  );
}

const QString& configOps::reportsDir ( const QString& client_name )
{
	mRetString = getValue ( defaultSectionName (), configDefaultFieldsNames[REPORT_DIR] );
	mRetString.replace ( QStringLiteral ( "%1" ), client_name );
	return mRetString;
}

const QString& configOps::setReportsDir ( const QString& str, const bool full_path )
{
	return setDir ( REPORT_DIR, full_path ? str :
					DEFAULT_OPTS[REPORT_DIR].left ( DEFAULT_OPTS[REPORT_DIR].indexOf (
								reportsDirSuffix () + str + CHR_F_SLASH ) )
				  );
}

const QString& configOps::defaultEmailAddress ()
{
	return DEFAULT_OPTS[EMAIL_ADDRESS];
}

void configOps::getWindowGeometry ( QWidget* window, const QString& section_name, const QString& category_name )
{
	const stringRecord& str_geometry ( getValue ( section_name, category_name ) );

	int x ( 0 ), y ( 0 ), width ( 0 ), height ( 0 );
	if ( str_geometry.first () )
	{
		x = str_geometry.curValue ().toInt ();
		if ( str_geometry.next () )
		{
			y = str_geometry.curValue ().toInt ();
			if ( str_geometry.next () )
			{
				width = str_geometry.curValue ().toInt ();
				if ( str_geometry.next () )
				{
					height = str_geometry.curValue ().toInt ();
				}
			}
		}
		window->setGeometry ( x, y, width, height );
	}
}

void configOps::saveWindowGeometry ( QWidget* window, const QString& section_name, const QString& category_name )
{
	stringRecord str_geometry;
	str_geometry.fastAppendValue ( QString::number ( window->pos ().x () ) );
	str_geometry.fastAppendValue ( QString::number ( window->pos ().y () ) );
	str_geometry.fastAppendValue ( QString::number ( window->width () ) );
	str_geometry.fastAppendValue ( QString::number ( window->height () ) );
	setValue ( section_name, category_name, str_geometry.toString () );
}
