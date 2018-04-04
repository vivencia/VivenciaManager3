#ifndef CONFIGOPS_H
#define CONFIGOPS_H

#include "vmlibs.h"

#include <QtCore/QString>

class configFile;
class configDialog;

enum CFG_FIELDS
{
	MAINWINDOW_GEOMETRY = 0, HOME_DIR, LAST_LOGGED_USER,
	FILEMANAGER, PICTURE_VIEWER, PICTURE_EDITOR, DOC_VIEWER,
	DOC_EDITOR, XLS_EDITOR, BASE_PROJECT_DIR, ESTIMATE_DIR, REPORT_DIR,
	BACKUP_DIR, DROPBOX_DIR, APP_SCHEME, EMAIL_ADDRESS
};

static const QLatin1String STR_VIVENCIA_LOGO ( "vivencia.jpg" );
static const QLatin1String STR_VIVENCIA_REPORT_LOGO ( "vivencia_report_logo_2.jpg" );
static const QLatin1String STR_PROJECT_DOCUMENT_FILE ( "project.docx" );
static const QLatin1String STR_PROJECT_SPREAD_FILE ( "spreadsheet.xlsx" );
static const QLatin1String STR_PROJECT_PDF_FILE ( "projeto.pdf" );
static const QString XDG_OPEN ( QStringLiteral ( "xdg-open" ) );

constexpr const int CFG_CATEGORIES ( 16 );

class configOps
{

public:
	explicit configOps ();
	~configOps ();

	/* IMPORTANT: setAppName () must be called early on any program startup routine. appName () must not
	 * be used in any static variable because it will be empty
	 */
	inline static void setAppName ( const QString& app_name ) { configOps::_appName = app_name; }
	inline static const QString& appName () { return configOps::_appName; }

	const QString& getValue ( const QString& section_name, const QString& category_name ) const;
	void setValue ( const QString& section_name, const QString& category_name, const QString& value );

	void geometryFromConfigFile ( int* coords, const QString& windowName = QString (), const QString& sessionName = QString () );
	void saveGeometry ( const int* coords, const QString& windowName = QString (), const QString& sessionName = QString () );

	static const QString kdesu ( const QString& message );
	static const QString gksu ( const QString& message, const QString& appname );
	static bool isSystem ( const QString& os_name );
	static bool initSystem ( const QString& initName );

	static const QString homeDir ();
	static const QString defaultConfigDir ();
	static const QString appConfigFile ();
	static const QString appDataDir();

	inline const QString& loggedUser () const
	{
		return getValue ( defaultSecsionName, configDefaultFieldsNames[LAST_LOGGED_USER] );
	}

	inline const QString& backupDir () const
	{
		return getValue ( defaultSecsionName, configDefaultFieldsNames[BACKUP_DIR] );
	}

	inline const QString& setBackupDir ( const QString& str )
	{
		return setDir ( BACKUP_DIR, str );
	}

	inline const QString& dropboxDir () const
	{
		return getValue ( defaultSecsionName, configDefaultFieldsNames[DROPBOX_DIR] );
	}

	inline const QString& setDropboxDir ( const QString& str )
	{
		return setDir ( DROPBOX_DIR, str );
	}

	inline const QString& appScheme () const
	{
		return getValue ( defaultSecsionName, configDefaultFieldsNames[APP_SCHEME] );
	}

	inline const QString& setAppScheme ( const QString& str )
	{
		setValue ( defaultSecsionName, configDefaultFieldsNames[APP_SCHEME], str );
		return str;
	}

	inline const QString& fileManager () const
	{//TODO
		return getValue ( defaultSecsionName, configDefaultFieldsNames[FILEMANAGER] );
	}

	inline const QString& setFileManager ( const QString& str )
	{
		return setApp ( FILEMANAGER, str );
	}

	inline const QString& pictureViewer () const
	{
		return getValue ( defaultSecsionName, configDefaultFieldsNames[PICTURE_VIEWER] );
	}

	inline const QString& setPictureViewer ( const QString& str )
	{
		return setApp ( PICTURE_VIEWER, str );
	}

	inline const QString& pictureEditor () const
	{
		return getValue ( defaultSecsionName, configDefaultFieldsNames[PICTURE_EDITOR] );
	}

	inline const QString& setPictureEditor ( const QString& str )
	{
		return setApp ( PICTURE_EDITOR, str );
	}

	inline const QString& universalViewer () const
	{
		return getValue ( defaultSecsionName, configDefaultFieldsNames[DOC_VIEWER] );
	}

	inline const QString& setUniversalViewer ( const QString& str )
	{
		return setApp ( DOC_VIEWER, str );
	}

	inline const QString& docEditor () const
	{
		return getValue ( defaultSecsionName, configDefaultFieldsNames[DOC_EDITOR] );
	}

	inline const QString& setDocEditor ( const QString& str )
	{
		return setApp ( DOC_EDITOR, str );
	}

	inline const QString& xlsEditor () const
	{
		return getValue ( defaultSecsionName, configDefaultFieldsNames[XLS_EDITOR] );
	}

	inline const QString& setXlsEditor ( const QString& str )
	{
		return setApp ( XLS_EDITOR, str );
	}

	inline const QString& projectsBaseDir () const
	{
		return getValue ( defaultSecsionName, configDefaultFieldsNames[BASE_PROJECT_DIR] );
	}

	inline const QString& setProjectsBaseDir ( const QString& str )
	{
		return setDir ( BASE_PROJECT_DIR, str );
	}

	const QString& getProjectBasePath ( const QString& client_name );

	inline static const QString estimatesDirSuffix () { return QStringLiteral ( "Orçamentos" ); }
	const QString& estimatesDir ( const QString& client_name );
	const QString& setEstimatesDir ( const QString&, const bool full_path = false );

	inline static const QString reportsDirSuffix () { return QStringLiteral ( "Relatórios" ); }
	const QString& reportsDir ( const QString& client_name );
	const QString& setReportsDir ( const QString& str, const bool full_path = false );

	static const QString& defaultEmailAddress ();

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

private:
	friend const QString& getDefaultFieldValuesByCategoryName ( const QString& category_name );

	const QString& setApp ( const CFG_FIELDS field, const QString& app );
	const QString& setDir ( const CFG_FIELDS field, const QString& dir );

	QString m_filename, mRetString;
	configFile* m_cfgFile;

	static QString _appName;
	static const QString defaultSecsionName;
	static const QString configDefaultFieldsNames[CFG_CATEGORIES];
};

#endif // CONFIGOPS_H
