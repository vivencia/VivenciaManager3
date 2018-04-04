#ifndef GLOBAL_H
#define GLOBAL_H

#include <vmlibs.h>

#include <QtCore/QLatin1String>
constexpr const QLatin1String PROGRAM_NAME ( "VivenciaManager4", 16 );

#define VERSION_MAJOR 3
#define VERSION_MINOR 9
#define VERSION_REVISION 0
constexpr const QLatin1String VERSION_APPEND ( "Getting back on track", 21 );
constexpr const QLatin1String VERSION_DATE ( "2018-02-15", 10 );

#define QUOTEME_(x) #x
#define QUOTEME(x) QUOTEME_(x)

static const QString PROGRAM_VERSION (
	QUOTEME(VERSION_MAJOR) + CHR_DOT +
	QUOTEME(VERSION_MINOR) + CHR_DOT +
	QUOTEME(VERSION_REVISION ) + QLatin1String ( " - " ) +
	VERSION_APPEND + CHR_SPACE + CHR_L_PARENTHESIS + VERSION_DATE + CHR_R_PARENTHESIS
);

#endif // GLOBAL_H
