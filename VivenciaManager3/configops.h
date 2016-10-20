#ifndef CONFIGOPS_H
#define CONFIGOPS_H

#include "global.h"

#include <QString>

class configFile;
class configDialog;

enum CFG_FIELDS
{
	MAINWINDOW_GEOMETRY = 0, HOME_DIR = 1, LAST_VIEWED_RECORDS = 2, LAST_LOGGED_USER = 3,
	CONFIG_FILE = 4, DATA_DIR = 5, FILEMANAGER = 6, PICTURE_VIEWER = 7, PICTURE_EDITOR = 8,
	EMAIL_CLIENT = 9, DOC_VIEWER = 10, DOC_EDITOR = 11 , XLS_EDITOR = 12, BASE_PROJECT_DIR = 13,
	ESTIMATE_DIR = 14, REPORT_DIR = 15, BACKUP_DIR = 16, HTML_DIR = 17, DROPBOX_DIR = 18, EMAIL_ADDRESS = 19
};

static const QLatin1String STR_VIVENCIA_LOGO ( "vivencia.jpg" );
static const QLatin1String STR_VIVENCIA_REPORT_LOGO ( "vivencia_report_logo.jpg" );
static const QLatin1String STR_PROJECT_DOCUMENT_FILE ( "project.docx" );
static const QLatin1String STR_PROJECT_SPREAD_FILE ( "spreadsheet.xlsx" );
static const QLatin1String STR_PROJECT_PDF_FILE ( "projeto.pdf" );
static const QString XDG_OPEN ( QStringLiteral ( "xdg-open" ) );

class configOps
{

public:
	~configOps ();

	static void init () {
		if ( !s_instance )
			s_instance = new configOps;
	}

	bool setConfigFile ( const QString& filename );
	const QString& readConfigFile ( const int category = -1, QString category_name = QString::null ) const;
    void writeConfigFile ( const int category, const QString& fieldValue, QString category_name = QString::null );

	const QString& lastViewedRecords ( const bool use_default = false ) const;
	int lastViewedRecord ( const uint table ) const;
	void geometryFromConfigFile ( int *coords );
	void saveGeometry ( const int* coords );

	const QString& loggedUser () const;

	// These must be static because they are called by Data::commandMySQLServer and Data::isMySQLRunning before vdb is set
	static const QString kdesu ( const QString& message );
	static const QString gksu ( const QString& message, const QString& appname );
	static bool isSystem ( const QString& os_name );
    static bool initSystem ( const QString& initName );

	const QString& defaultConfigDir () const;

	const QString& appConfigFile ( const bool use_default = false ) const;
	const QString setAppConfigFile ( const QString& );

	inline const QString& homeDir () const
	{
		return getConfigValue ( HOME_DIR, true );
	}

	inline const QString& appDataDir ( const bool use_default = false ) const
	{
		return getConfigValue ( DATA_DIR, use_default );
	}

	inline const QString& setAppDataDir ( const QString& str )
	{
		return setDir ( DATA_DIR, str );
	}

	inline const QString& backupDir ( const bool use_default = false ) const
	{
		return getConfigValue ( BACKUP_DIR, use_default );
	}

	inline const QString& setBackupDir ( const QString& str )
	{
		return setDir ( BACKUP_DIR, str );
	}

	inline const QString& HTMLDir ( const bool use_default = false ) const
	{
		return getConfigValue ( HTML_DIR, use_default );
	}
	
	inline const QString& setHTMLDir ( const QString& str )
	{
		return setDir ( HTML_DIR, str );
	}
	
	inline const QString& dropboxDir ( const bool use_default = false ) const
	{
		return getConfigValue ( DROPBOX_DIR, use_default );
	}
	
	inline const QString& setDropboxDir ( const QString& str )
	{
		return setDir ( DROPBOX_DIR, str );
	}
	
	inline const QString& fileManager ( const bool use_default = false ) const
	{
		return getConfigValue ( FILEMANAGER, use_default );
	}

	inline const QString& setFileManager ( const QString& str )
	{
		return setApp ( FILEMANAGER, str );
	}

	inline const QString& pictureViewer ( const bool use_default = false ) const
	{
		return getConfigValue ( PICTURE_VIEWER, use_default );
	}

	inline const QString& setPictureViewer ( const QString& str )
	{
		return setApp ( PICTURE_VIEWER, str );
	}

	inline const QString& pictureEditor ( const bool use_default = false ) const
	{
		return getConfigValue ( PICTURE_EDITOR, use_default );
	}

	inline const QString& setPictureEditor ( const QString& str )
	{
		return setApp ( PICTURE_EDITOR, str );
	}

	inline const QString& emailClient ( const bool use_default = false ) const
	{
		return getConfigValue ( EMAIL_CLIENT, use_default );
	}

	inline const QString& setEmailClient ( const QString& str )
	{
		return setApp ( EMAIL_CLIENT, str );
	}

	inline const QString& universalViewer ( const bool use_default = false ) const
	{
		return getConfigValue ( DOC_VIEWER, use_default );
	}

	inline const QString& setUniversalViewer ( const QString& str )
	{
		return setApp ( DOC_VIEWER, str );
	}

	inline const QString& docEditor ( const bool use_default = false ) const
	{
		return getConfigValue ( DOC_EDITOR, use_default );
	}

	inline const QString& setDocEditor ( const QString& str )
	{
		return setApp ( DOC_EDITOR, str );
	}

	inline const QString& xlsEditor ( const bool use_default = false ) const
	{
		return getConfigValue ( XLS_EDITOR, use_default );
	}

	inline const QString& setXlsEditor ( const QString& str )
	{
		return setApp ( XLS_EDITOR, str );
	}

	inline const QString& projectsBaseDir ( const bool use_default = false ) const
	{
		return getConfigValue ( BASE_PROJECT_DIR, use_default );
	}

	inline const QString& setProjectsBaseDir ( const QString& str )
	{
		return setDir ( BASE_PROJECT_DIR, str );
	}

	const QString& getProjectBasePath ( const QString& client_name );

	inline static const QString estimatesDirSuffix () { return QStringLiteral ( "Orçamentos" ); }
	const QString& estimatesDir ( const QString& client_name, const bool use_default = false );
	const QString& setEstimatesDir ( const QString&, const bool full_path = false );

	inline static const QString reportsDirSuffix () { return QStringLiteral ( "Relatórios" ); }
	const QString& reportsDir ( const QString& username, const bool use_default = false );
	const QString& setReportsDir ( const QString&, const bool full_path = false );

	const QString& defaultEmailAddress () const;

	inline const QString vivienciaLogo () const {
		return appDataDir () + STR_VIVENCIA_LOGO;
	}
	inline const QString vivienciaReportLogo () const {
		return appDataDir () + STR_VIVENCIA_REPORT_LOGO;
	}
	inline const QString projectDocumentFile () const {
		return appDataDir () + STR_PROJECT_DOCUMENT_FILE;
	}
	inline const QString projectSpreadSheetFile () const {
		return appDataDir () + STR_PROJECT_SPREAD_FILE;
	}
	inline const QString projectPDFFile () const {
		return appDataDir () + STR_PROJECT_PDF_FILE;
	}

	static inline const QLatin1String projectDocumentExtension () {
		return QLatin1String ( ".docx" );
	}
	static inline const QLatin1String projectDocumentFormerExtension () {
		return QLatin1String ( ".doc" );
	}
	static inline const QLatin1String projectReportExtension () {
		return QLatin1String ( ".vmr" );
	}
	static inline const QLatin1String projectSpreadSheetExtension () {
		return QLatin1String ( ".xlsx" );
	}
	static inline const QLatin1String projectSpreadSheetFormerExtension () {
		return QLatin1String ( ".xls" );
	}
	static inline const QLatin1String projectPDFExtension () {
		return QLatin1String ( ".pdf" );
	}
	static inline const QLatin1String projectExtraFilesExtension () {
		return QLatin1String ( ".txt" );
	}

	static QString readConfig ( const QString& filename, const QString& field );
	static bool writeConfig ( const QString& filename, const QString& field, const QString& value );

	configDialog* dialogWindow ();

private:
	explicit configOps ();
	static configOps* s_instance;
	friend configOps* CONFIG ();
	friend void deleteConfigInstance ();

	const QString& getConfigValue ( const CFG_FIELDS field, const bool use_default ) const;
	const QString& setApp ( const CFG_FIELDS field, const QString& app );
	const QString& setDir ( const CFG_FIELDS field, const QString& app );

	QString m_filename, mRetString;
	configFile* m_cfgFile;
	configDialog* cfgDlg;
};

inline configOps *CONFIG ()
{
	return configOps::s_instance;
}

#endif // CONFIGOPS_H
