#ifndef GLOBAL_H
#define GLOBAL_H

#ifdef DEBUG
#include <QDebug>
#undef USE_THREADS
#define DBG_OUT(str,desc,separate) \
		if ( separate ) \
			qDebug () << QLatin1String ( "-----------------------------------" ); \
		if ( desc ) \
			qDebug () << \
			QLatin1String ( "In ") << \
			QLatin1String ( __FILE__ ) << \
			QLatin1String ( "  - at line  " ) << \
			QString::number ( __LINE__  ) << endl << \
			QLatin1String ( #str ) << \
			QLatin1String ( ": " ) << \
			str << endl; \
		else \
			qDebug () << \
			QLatin1String ( "In " ) << \
			QLatin1String ( __FILE__ ) << \
			QLatin1String ( "  - at line  " ) << \
			QString::number ( __LINE__  ) << endl << \
			str << endl;
#else
#define DBG_OUT(str,desc,separate)
#define USE_THREADS
#endif

#ifndef TRANSITION_PERIOD
//#define TRANSITION_PERIOD
#endif

#ifdef TRANSLATION_ENABLED
#define TR_FUNC tr
#define APP_TR_FUNC QApplication::tr
#else
#define TR_FUNC QStringLiteral
#define APP_TR_FUNC QStringLiteral
#endif

#define QUOTEME_(x) #x
#define QUOTEME(x) QUOTEME_(x)

/* Don't know what happend here. This is the fix a found for the pŕoblem of Qt misinterpreting all the strings
 * that have non-english characters in them. They display fine on the window, but not on the console. The system-wide 
 * Qt libraries themselves do not read those strings, no matter if write them on the editor and hardcode them into the
 * program or if I read from the system using C libraries or if I read from the Database using Qt's own mechanism for doing
 * it so. 
 */
#if QT_VERSION >= QT_VERSION_CHECK(5, 9, 1)
#define QSTRING_ENCODING_FIX(str) QString::fromLocal8Bit ( str.toUtf8 () )
#else
#define QSTRING_ENCODING_FIX(str) str
#endif

constexpr const QLatin1Char CHR_DOT ( '.' );
constexpr const QLatin1Char CHR_COMMA ( ',' );
constexpr const QLatin1Char CHR_SPACE ( ' ' );
constexpr const QLatin1Char CHR_F_SLASH ( '/' );
constexpr const QLatin1Char CHR_B_SLASH ( '\\' );
constexpr const QLatin1Char CHR_L_PARENTHESIS ( '(' );
constexpr const QLatin1Char CHR_R_PARENTHESIS ( ')' );
constexpr const QLatin1Char CHR_NEWLINE ( '\n' );
constexpr const QLatin1Char CHR_PERCENT ( '%' );
constexpr const QLatin1Char CHR_PIPE ( '|' );
constexpr const QLatin1Char CHR_HYPHEN ( '-' );
constexpr const QLatin1Char CHR_EQUAL ( '=' );
constexpr const QLatin1Char CHR_QUESTION_MARK ( '?' );
constexpr const QLatin1Char CHR_CHRMARK ( '\'' );
constexpr const QLatin1Char CHR_COLON ( ':' );
constexpr const QLatin1Char CHR_SEMICOLON ( ';' );
constexpr const QLatin1Char CHR_QUOTES ( '\"' );
constexpr const QLatin1Char CHR_L_BRACKET ( '[' );
constexpr const QLatin1Char CHR_R_BRACKET ( ']' );
constexpr const QLatin1Char CHR_TILDE ( '~' );

const char* const PROPERTY_PRINT_PREVIEW ( "ppp" );
const char* const PROPERTY_DOC_MODIFIED ( "pdm" );

constexpr const QLatin1String colorsStr[6] = {
	QLatin1String ( "220, 220, 220", 13 ),		// gray		- 0
	QLatin1String ( "255, 0, 0", 9 ),			// red		- 1
	QLatin1String ( "255, 255, 0", 11 ),		// yellow	- 2
	QLatin1String ( "0, 255, 0", 9 ),			// green	- 3
	QLatin1String ( "0, 0, 255", 9 ),			// blue		- 4
	QLatin1String ( "255, 255, 255", 13 )		// white	- 5
};

const QString CHR_ZERO ( QStringLiteral ( "0" ) );
const QString CHR_ONE ( QStringLiteral ( "1" ) );
const QString CHR_TWO ( QStringLiteral ( "2" ) );
const QString DATE_FORMAT_DB ( QStringLiteral ( "yyyy/MM/dd" ) );
const QString DATE_FORMAT_HUMAN ( QStringLiteral ( "dd/MM/yyyy" ) );
const QString sudoCommand ( QStringLiteral ( "echo \"%1\" | sudo -S sh -c \"%2\"" ) );

constexpr const QLatin1String TIME_FORMAT_DEFAULT ( "HH:mm", 5 );
//constexpr const QLatin1String DATE_FORMAT_LONG ( " dddd, dd 'de' MMMM 'de' yyyy" );
static const QString emptyString;

constexpr const QLatin1String PROGRAM_NAME ( "VivenciaManager3", 16 );

#define VERSION_MAJOR 3
#define VERSION_MINOR 4
#define VERSION_REVISION 0
constexpr const QLatin1String VERSION_APPEND ( "Starting slowly", 15 );
constexpr const QLatin1String VERSION_DATE ( "2018-01-14", 10 );

static const QString PROGRAM_VERSION (
	QUOTEME(VERSION_MAJOR) + CHR_DOT +
	QUOTEME(VERSION_MINOR) + CHR_DOT +
	QUOTEME(VERSION_REVISION ) + QLatin1String ( " - " ) +
	VERSION_APPEND + CHR_SPACE + CHR_L_PARENTHESIS + VERSION_DATE + CHR_R_PARENTHESIS
);

#define ICON(name) QIcon::fromTheme ( QStringLiteral ( name ), QIcon ( QStringLiteral ( ":resources/" name"" ) ) )
#define PIXMAP(name) QPixmap ( QStringLiteral ( ":resources/" name ) )

extern QString APP_START_CMD;

static const QString STR_MYSQL ( QStringLiteral ( "mysql" ) );
static const QString SYSTEMD ( QStringLiteral ( "systemd" ) );
static const QString SYSTEM_ROOT_SERVICE ( QStringLiteral ( "SYSTEM_ROOT_SERVICE" ) );
static const QString SYSTEM_ROOT_PASSWORD_ID ( QStringLiteral ( "SYSTEM_ROOT_PASSWORD_ID" ) );

static const QString DB_NAME ( QStringLiteral ( "VivenciaDatabase3" ) );
static const QString USER_NAME ( QStringLiteral ( "vivenciamngr3" ) );
static const QString PASSWORD ( QStringLiteral ( "fenixfenix3" ) ) ;

#include <QWidget>
#define TITLE_BAR_HEIGHT qApp->style ()->pixelMetric ( QStyle::PM_TitleBarHeight )

#endif // GLOBAL_H
