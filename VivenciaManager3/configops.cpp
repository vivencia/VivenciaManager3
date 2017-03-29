#include "configops.h"
#include "global.h"
#include "vivenciadb.h"
#include "fileops.h"
#include "textdb.h"
#include "cleanup.h"
#include "configdialog.h"
#include "mainwindow.h"
#include "generaltable.h"
#include "heapmanager.h"

#include <QTextStream>
#include <QStandardPaths>

constexpr const int CFG_CATEGORIES ( 20 );
static const QString CFG_FILE_FIELDS_STR[CFG_CATEGORIES] =
{
	QStringLiteral ( "MAINWINDOW_GEOMETRY" ), QStringLiteral ( "HOME_DIR" ), QStringLiteral ( "LAST_VIEWED_RECORDS" ),
	QStringLiteral ( "LAST_LOGGED_USER" ), QStringLiteral ( "CONFIG_FILE" ), QStringLiteral ( "DATA_DIR" ),
	QStringLiteral ( "FILEMANAGER" ), QStringLiteral ( "PICTURE_VIEWER" ), QStringLiteral ( "PICTURE_EDITOR" ),
	QStringLiteral ( "EMAIL_CLIENT" ), QStringLiteral ( "DOC_VIEWER" ), QStringLiteral ( "DOC_EDITOR" ),
	QStringLiteral ( "XLS_EDITOR" ), QStringLiteral ( "BASE_PROJECT_DIR" ), QStringLiteral ( "ESTIMATE_DIR" ),
	QStringLiteral ( "REPORT_DIR" ), QStringLiteral ( "BACKUP_DIR" ), QStringLiteral ( "HTML_DIR" ), 
	QStringLiteral ( "DROPBOX_DIR" ), QStringLiteral ( "EMAIL_ADDRESS" )
};

static const QString HOME_DIR_DIR ( QLatin1String ( ::getenv ( "HOME" ) ) + CHR_F_SLASH );
static const QString DEFAULT_CONFIG_DIR ( HOME_DIR_DIR + QLatin1String ( "/.config/" ) + PROGRAM_NAME + CHR_F_SLASH );

static const QString DEFAULT_OPTS[CFG_CATEGORIES] = {
	( QStringLiteral ( "max" ) ), // DEFAULT_MAINWINDOW_GEOMETRY
	( HOME_DIR_DIR ),
	( QStringLiteral ( "0:0:0:0:0:" ) ), // DEFAULT_LAST_VIEWED_RECORDS
	( emptyString ), // DEFAULT_LAST_LOGGED_USER
	( DEFAULT_CONFIG_DIR + QLatin1String ( "vmngr3.conf" ) ), // DEFAULT_OPTS[CONFIG_FILE]
	/* I am assuming the QStringList has at least one item. I will play along with this because I think it will have at least one item
	 * under all circurmstances. If I encounter an error at some point I will create a static function which will check for emptiness in the list
	 */
	( QStandardPaths::standardLocations ( QStandardPaths::DataLocation ).at ( 0 ) + QLatin1String ( "/data/" ) + PROGRAM_NAME + CHR_F_SLASH ), // DEFAULT_DATA_DIR
	( XDG_OPEN ), // DEFAULT_FILE_MANAGER
	( XDG_OPEN ), // DEFAULT_PICTURE_VIEWER
	( QStringLiteral ( "gimp" ) ), // DEFAULT_PICTURE_EDITOR
	( QStringLiteral ( "thunderbird" ) ), // DEFAULT_EMAIL_CLIENT
	( XDG_OPEN ), // DEFAULT_PDF_VIEWER
	( XDG_OPEN ), // DEFAULT_DOC_EDITOR
	( XDG_OPEN ), // DEFAULT_XLS_EDITOR
	// the same (regarding QStringList)
	( QStandardPaths::standardLocations ( QStandardPaths::DocumentsLocation ).at ( 0 ) + QLatin1String ( "/Vivencia/" ) ), // DEFAULT_PROJECTS_DIR
	( QStandardPaths::standardLocations ( QStandardPaths::DocumentsLocation ).at ( 0 ) + QString::fromUtf8 ( "/Vivencia/%1/" ) + configOps::estimatesDirSuffix () + CHR_F_SLASH ), // DEFAULT_ESTIMATE_DIR
	( QStandardPaths::standardLocations ( QStandardPaths::DocumentsLocation ).at ( 0 ) + QString::fromUtf8 ( "/Vivencia/%1/" ) + configOps::reportsDirSuffix () + CHR_F_SLASH ), // DEFAULT_REPORTS_DIR
	( QStandardPaths::standardLocations ( QStandardPaths::DocumentsLocation ).at ( 0 ) + QLatin1String ( "/Vivencia/VMDB/" ) ), // BACKUP_DIR
	( QStandardPaths::standardLocations ( QStandardPaths::DocumentsLocation ).at ( 0 ) + QLatin1String ( "/Vivencia/HTML/" ) ), // HTML_DIR
	( HOME_DIR_DIR ) + QLatin1String ( "Dropbox/" ), // DROPBOX_DIR
	( QStringLiteral ( "vivencia@gmail.com" ) ) // DEFAULT_EMAIL
};

configOps* configOps::s_instance ( nullptr );

void deleteConfigInstance ()
{
	heap_del ( configOps::s_instance );
}

configOps::configOps ()
	: m_cfgFile ( nullptr ), cfgDlg ( nullptr )
{
	VivenciaDB::init ();
	generalTable gen_rec;
	if ( gen_rec.readFirstRecord () )
		m_filename = recStrValue ( &gen_rec, FLD_GENERAL_CONFIG_FILE );

	if ( m_filename.isEmpty () )
		m_filename = DEFAULT_OPTS[CONFIG_FILE];

    fileOps::createDir ( fileOps::dirFromPath ( m_filename ) );
	m_cfgFile = new configFile ( m_filename );
	addPostRoutine ( deleteConfigInstance );
}

configOps::~configOps ()
{
	heap_del ( m_cfgFile );
}

const QString& configOps::getConfigValue ( const CFG_FIELDS field, const bool use_default ) const
{
	if ( use_default )
		return DEFAULT_OPTS[field];
	else
		return readConfigFile ( field );
}

const QString& configOps::setApp ( const CFG_FIELDS field, const QString& app )
{
	mRetString = app.contains ( CHR_F_SLASH ) ? app : fileOps::appPath ( app );
    if ( fileOps::exists ( mRetString ).isOn () )
		writeConfigFile ( field, mRetString );
	else
		mRetString = getConfigValue ( field, false );
	return mRetString;
}

const QString& configOps::setDir ( const CFG_FIELDS field, const QString& dir )
{
    if ( fileOps::isDir ( dir ).isOn () )
	{
		writeConfigFile ( field, dir );
		return dir;
	}
	return getConfigValue ( field, false );
}

bool configOps::setConfigFile ( const QString& filename )
{
    if ( fileOps::canWrite ( fileOps::dirFromPath ( filename ) ).isOn () )
	{
        m_cfgFile->remove ();
        heap_del ( m_cfgFile );
        m_cfgFile = new configFile ( filename );
        m_cfgFile->open ();
        m_filename = filename;

        generalTable gen_rec;
        if ( gen_rec.readFirstRecord () )
		{
            gen_rec.setAction ( ACTION_EDIT );
            setRecValue ( &gen_rec, FLD_GENERAL_CONFIG_FILE, m_filename );
            if ( gen_rec.saveRecord () )
                return true;
        }
	}
	return false;
}

const QString& configOps::readConfigFile ( const int category, QString category_name ) const
{
    if ( fileOps::exists ( m_filename ).isOn () )
	{
		if ( m_cfgFile->sectionCount () == 0 )// First time read. Load data
			m_cfgFile->load ();

		if ( m_cfgFile->setWorkingSection ( 0 ) )
		{
			if ( category >= 0 )
				category_name = CFG_FILE_FIELDS_STR[category];
			const QString value ( m_cfgFile->fieldValue ( category_name ) );
			if ( !value.isEmpty () )
				return m_cfgFile->fieldValue ( category_name );
		}
	}
	else
		m_cfgFile->open (); //config file does not exist. Create it.

	if ( category >= 0 )
		return DEFAULT_OPTS[category];
	else
		return emptyString;
}

void configOps::writeConfigFile ( const int category, const QString& fieldValue, QString category_name )
{
	if ( !fieldValue.isEmpty () )
	{
		if ( !m_cfgFile->setWorkingSection ( 0 ) ) // config file empty
			m_cfgFile->insertNewSection ( QStringLiteral ( "CONFIG" ), true );
		if ( category >= 0 )
			category_name = CFG_FILE_FIELDS_STR[category];
		const int idx ( m_cfgFile->fieldIndex ( category_name ) );

		if ( idx != -1 )
			m_cfgFile->setFieldValue ( category_name, fieldValue );
		else
			m_cfgFile->insertField ( category_name, fieldValue );

		m_cfgFile->commit ();
	}
}

void configOps::saveGeometry ( const int* coords )
{
	stringRecord str_geometry;
	str_geometry.fastAppendValue ( QString::number ( coords[0] ) );
	str_geometry.fastAppendValue ( QString::number ( coords[1] ) );
	str_geometry.fastAppendValue ( QString::number ( coords[2] ) );
	str_geometry.fastAppendValue ( QString::number ( coords[3] ) );
	writeConfigFile ( MAINWINDOW_GEOMETRY, str_geometry.toString () );
}

void configOps::geometryFromConfigFile ( int* coords )
{
	stringRecord str_geometry ( readConfigFile ( MAINWINDOW_GEOMETRY ) );
	if ( str_geometry.isOK () )
	{
		str_geometry.first ();
		coords[0] = str_geometry.curValue ().toInt ();
		str_geometry.next ();
		coords[1] = str_geometry.curValue ().toInt ();
		str_geometry.next ();
		coords[2] = str_geometry.curValue ().toInt ();
		str_geometry.next ();
		coords[3] = str_geometry.curValue ().toInt ();
	}
}

const QString& configOps::lastViewedRecords ( const bool use_default ) const
{
	if ( use_default )
		return DEFAULT_OPTS[LAST_VIEWED_RECORDS];
	else
		return readConfigFile ( LAST_VIEWED_RECORDS );
}

// Returns index of last viewed item within tables. This index is the QListWidget index
// and the index within each category List of pointers
uint configOps::lastViewedRecord ( const uint table ) const
{
	stringRecord lvr ( lastViewedRecords () );

	if ( lvr.isOK () )
	{
		uint field ( 0 );
		switch ( table )
		{
			case CLIENT_TABLE:		field = 0;		break;
			case JOB_TABLE:			field = 1;		break;
			case PURCHASE_TABLE:	field = 2;		break;
		}
		return lvr.fieldValue ( field ).toUInt ();
	}
	return 0;
}

const QString& configOps::defaultConfigDir () const
{
	return DEFAULT_CONFIG_DIR;
}

const QString configOps::kdesu ( const QString& message )
{
    QString ret ( fileOps::appPath ( QStringLiteral ( "kdesudo" ) ) );
	if ( ret.isEmpty () )
	{
		ret = QStringLiteral ( "/etc/alternatives/kdesu" );
        if ( !fileOps::fileOps::exists ( ret ).isOn () )
			return gksu ( message, emptyString );
	}
	ret += QLatin1String ( " --comment \"" ) + message + QLatin1String ( "\" -n -d -c " );
	return ret;
}

const QString configOps::gksu ( const QString& message, const QString& appname )
{
	QString ret ( fileOps::appPath ( QStringLiteral ( "gksu" ) ) );
	if ( !ret.isEmpty () )
		ret += QLatin1String ( " -m \"" ) + message + QLatin1String ( "\" -D " ) + appname + QLatin1String ( " -g -k " );
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

const QString& configOps::loggedUser () const
{
	return readConfigFile ( LAST_LOGGED_USER );
}

const QString& configOps::appConfigFile ( const bool use_default ) const
{
	return getConfigValue ( CONFIG_FILE, use_default );
}

const QString configOps::setAppConfigFile ( const QString& str )
{
    if ( fileOps::isDir ( fileOps::dirFromPath ( str ) ).isOn () )
	{
		writeConfigFile ( CONFIG_FILE, str );
		return str;
	}
	return appConfigFile ();
}

const QString& configOps::getProjectBasePath ( const QString& client_name )
{
	mRetString = projectsBaseDir ();
	if ( !client_name.isEmpty () )
		mRetString += client_name + CHR_F_SLASH;
	return mRetString;
}

const QString& configOps::estimatesDir ( const QString& client_name, const bool use_default )
{
	mRetString = getConfigValue ( ESTIMATE_DIR, use_default );
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

const QString& configOps::reportsDir ( const QString& client_name, const bool use_default )
{
	mRetString = getConfigValue ( REPORT_DIR, use_default );
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

const QString& configOps::defaultEmailAddress () const
{
	return DEFAULT_OPTS[EMAIL_ADDRESS];
}

QString configOps::readConfig ( const QString& filename, const QString& field )
{
	QString value ( emptyString );
	configFile* cfgFile ( new configFile ( filename ) );

	if ( cfgFile->open () )
	{
		if ( cfgFile->load ().isOn () )
			value = cfgFile->fieldValue ( field );
	}
	delete cfgFile;
	return value;
}

bool configOps::writeConfig ( const QString& filename, const QString& field, const QString& value )
{
	bool ret ( false );
	configFile* cfgFile ( new configFile ( filename ) );

	if ( cfgFile->open () )
	{
		if ( !cfgFile->load ().isOff () )
		{
			if ( !cfgFile->setWorkingSection ( 0 ) ) // config file empty
				cfgFile->insertNewSection ( QStringLiteral ( "CONFIG" ), true );
			const int idx ( cfgFile->fieldIndex ( field ) );
			if ( idx == -1 )
				cfgFile->insertField ( field, value );
			else
				cfgFile->setFieldValue ( static_cast<uint>(idx), value );
			cfgFile->commit ();
		}
	}
	delete cfgFile;
	return ret;
}

configDialog* configOps::dialogWindow ()
{
	if ( !cfgDlg )
		cfgDlg = new configDialog ( globalMainWindow );
	return cfgDlg;
}
