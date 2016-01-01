#ifndef GLOBAL_H
#define GLOBAL_H

#include <qglobal.h>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#define QT5
#else
#define QT4
#endif

#ifdef DEBUG
#include <QDebug>
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

constexpr const QLatin1String colorsStr[6] = {
	QLatin1String ( "220, 220, 220", 13 ),		// gray
	QLatin1String ( "255, 0, 0", 9 ),			// red
	QLatin1String ( "255, 255, 0", 11 ),		// yellow
	QLatin1String ( "0, 255, 0", 9 ),			// green
	QLatin1String ( "0, 0, 255", 9 ),			// blue
	QLatin1String ( "255, 255, 255", 13 )		// white
};

#ifdef TRANSITION_PERIOD
constexpr const QLatin1Char CHR_PLUS ( '+' );
#endif

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
#define VERSION_MINOR 0
#define VERSION_REVISION 0
constexpr const QLatin1String VERSION_APPEND ( "Finally here - 3 years later", 30 );
constexpr const QLatin1String VERSION_DATE ( "2015-11-20", 10 );

static const QString PROGRAM_VERSION (
    QUOTEME(VERSION_MAJOR) + CHR_DOT +
    QUOTEME(VERSION_MINOR) + CHR_DOT +
    QUOTEME(VERSION_REVISION ) + QLatin1String ( " - " ) +
    VERSION_APPEND + CHR_SPACE + CHR_L_PARENTHESIS + VERSION_DATE + CHR_R_PARENTHESIS
);

#define ICON(name) QIcon ( QStringLiteral ( ":resources/" name ) )
#define PIXMAP(name) QPixmap ( QStringLiteral ( ":resources/" name ) )

extern bool EXITING_PROGRAM;
extern QString APP_START_CMD;

static const QString OPENSUSE ( QStringLiteral ( "openSUSE" ) );
static const QString UBUNTU ( QStringLiteral ( "Ubuntu" ) );
static const QString STR_MYSQL ( QStringLiteral ( "mysql" ) );
static const QString SYSTEMD ( QStringLiteral ( "systemd" ) );
static const QString UPSTART ( QStringLiteral ( "upstart" ) );

#ifdef TRANSITION_PERIOD
static const QString DB_NAME ( QStringLiteral ( "VivenciaDatabase3" ) );
static const QString USER_NAME ( QStringLiteral ( "vivenciamngr3" ) );
static const QString PASSWORD ( QStringLiteral ( "fenixfenix3" ) ) ;

extern int getNewClientID ( const int old_id );
extern int getOldClientID ( const int new_id );
#else
static const QString DB_NAME ( QStringLiteral ( "VivenciaDatabase" ) );
static const QString USER_NAME ( QStringLiteral ( "vivenciamngr" ) );
static const QString PASSWORD ( QStringLiteral ( "fenixfenix" ) );
#endif

class MainWindow;
extern MainWindow* globalMainWindow;

#include <QWidget>
#define TITLE_BAR_HEIGHT qApp->style ()->pixelMetric ( QStyle::PM_TitleBarHeight )

#endif // GLOBAL_H
