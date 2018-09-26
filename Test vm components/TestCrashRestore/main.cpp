#include "mainwindow.h"
#include <QApplication>

#include <vmlist.h>
#include <vmUtils/configops.h>
#include <vmUtils/crashrestore.h>
#include <vmStringRecord/stringrecord.h>

#include <QString>
#include <QDebug>

int main(int argc, char *argv[])
{
	configOps::setAppName ( "CrashRestoreTest" );
	crashRestore* cr ( new crashRestore ( "cr1 ") );

	if ( cr->needRestore () )
	{
		stringRecord* restoredInfo ( nullptr );
		restoredInfo = const_cast<stringRecord*>( &cr->restoreFirstState () );
		do
		{
			for ( uint i ( 0 ); i < restoredInfo->nFields (); ++i )
				qDebug () << restoredInfo->fieldValue ( i );
			qDebug () << "-------------------------------";
			restoredInfo = const_cast<stringRecord*>( &cr->restoreNextState () );
			if ( cr->infoLoaded () )
				cr->eliminateRestoreInfo ( -1 );
		} while ( restoredInfo->isOK () );
		delete cr;
		return 0;
	}

	stringRecord info;
	for ( uint x ( 0 ); x < 10; ++x )
		info.insertField ( x, "Value " + QString::number ( x ) );
	cr->commitState ( 0, info );
	info.clear ();
	for ( uint x ( 0 ); x < 10; ++x )
		info.insertField ( x, "More Values " + QString::number ( x ) + QString::number ( x ) );
	cr->commitState ( 1, info );
	info.clear ();

	info.insertField ( 0, "This info should be lost" );
	cr->commitState ( 2, info );
	info.clear ();
	info.insertField ( 0, "Info that is from a single string" );
	cr->commitState ( 2, info );
	info.clear ();
	info.insertField ( 0, "Another single string" );
	cr->commitState ( 3, info );
	info.clear ();
	delete cr;
	return 0;

	QApplication a(argc, argv);
	MainWindow w;
	w.show();

	return a.exec();
}
