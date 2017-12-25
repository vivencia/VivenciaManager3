#include "global.h"
#include "system_init.h"

#include <QApplication>
#include <QtGui>

#ifdef TRANSLATION_ENABLED
#include <QTranslator>
#include <QLocale>
#endif

/*#include <time.h>
clock_t start, finish;
start = clock();

// code here
finish = clock();
qDebug() << ( (finish - start) );
*/

int main ( int argc, char *argv[] )
{
	QApplication app ( argc, argv );
	APP_START_CMD = QString ( argv[0] );
	
#ifdef TRANSLATION_ENABLED
	const QString locale ( QLocale::system ().name () );
	QTranslator translator;
	translator.load ( QLatin1String ( ":/i18n/VivenciaManager_" ) + locale );
	app.installTranslator ( &translator );
#endif

	Sys_Init::init ();
	return app.exec ();
}
