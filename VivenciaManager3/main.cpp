#include "global.h"
#include "data.h"

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

stringRecord result ()
{
	return stringRecord ( "datenfeld1" + record_separator + "datenfeld2" + record_separator + "datenfeld3" + record_separator + "datenfeld4" + record_separator );
}

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

	Data::init ();
	return app.exec ();
}
