#include "mainwindow.h"
#include <QApplication>

#include <vmlist.h>
#include <vmUtils/fileops.h>
#include <vmUtils/fast_library_functions.h>

#include <QString>
#include <QDebug>

int main(int , char **)
{
	/*QApplication a(argc, argv);
	MainWindow w;
	w.show();
	return a.exec();*/

	/*pointersList<QString*> array;
	array.setAutoDeleteItem( true );
	for ( uint i ( 0 ); i < 1000; ++i )
		array.insert ( i, new QString ( QString::number ( i ) ) );
	for ( uint i ( 0 ); i < 1000; ++i )
		qDebug () << *array[i];

	return 0;*/
	pointersList<fileOps::st_fileInfo*> files ( fileOps::fileCount ( "/home/guilherme/Downloads/kde-services-3.0.0/applications/" ) + 1 );

	fileOps::lsDir ( files, "/home/guilherme/Downloads/kde-services-3.0.0/applications/" );
	for ( uint i ( 0 ); i < files.count (); ++i )
	{
		qDebug () << i << ":  " << files.at ( i )->filename;
	}
	//insertStringIntoContainer ( result, filename,
	//		[&] ( const int idx ) -> QString { return result.at ( idx ).filename; },
	//		[&,fi] ( const
	return 0;
}
