#include "global.h"
#include "singleapp.h"
#include "logindialog.h"
#include "data.h"

#include <QApplication>
#include <QtGui>
#include <QTranslator>
#include <QLocale>
#include <QStyleFactory>
#include <QStyle>

/*#include <time.h>
clock_t start, finish;
start = clock();

// your code here
finish = clock();
qDebug() << ( (finish - start) );
*/

int main ( int argc, char *argv[] )
{
	/*auto algorithm = [&]( double x, double m, double b ) -> double
	   {
	   return m*x+b;
	   };

	int a=algorithm(1,2,3), b=algorithm(4,5,6);*/
	//int z ( 4 ), y ( 4 ), x ( 4 );
	//qDebug () << [z,y,x]( int a, int b, int c ) -> int { return (a*b)+c; };
	/*const int y = 10;
	int x = [&y]( int a, int b, int c ) -> int { return (a*b)+c + y; }(4,4,4);
	qDebug () << x;
	return 0;*/
	
	/*SingleApp sapp ( PROGRAM_NAME );
	if ( sapp.IsAlreadyRunning () ) {
		qDebug () << PROGRAM_NAME << QLatin1String ( " is already running. Only one instance at a time allowed. Exiting..." );
		return 1;
	}*/
	
	QApplication app ( argc, argv );
	app.setStyle ( QStyleFactory::create ( QLatin1String ( "GTK+ Style" ) ) );
	app.setPalette ( app.style ()->standardPalette () );

#ifdef TRANSLATION_ENABLED
	const QString locale ( QLocale::system ().name () );
	QTranslator translator;
	translator.load ( QLatin1String ( ":/i18n/VivenciaManager_" ) + locale );
	app.installTranslator ( &translator );
#endif

	Data::init ();

	APP_START_CMD = QString ( argv[0] );

	//loginDialog loginDlg;
	//loginDlg.showDialog ();
	//if ( loginDlg.result () == QDialog::Accepted ) {
	DATA ()->startUserInteface ();
	return app.exec ();
	//}
	//return false;
}
