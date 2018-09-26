#include "mainwindow.h"
#include <QApplication>

#include <vmlist.h>
#include <vmUtils/textdb.h>
#include <vmStringRecord/stringrecord.h>

#include <QString>
#include <QDebug>

int main(int argc, char *argv[])
{
	dataFile* file1 ( new dataFile ( "/home/guilherme/Documents/VivenciaManager/Test vm components/TestDataFile/data1") );
	file1->load ();
	stringRecord rec1, rec2, rec3;
	for ( uint i ( 0 ); i < 10; ++i )
	{
		rec1.fastAppendValue ( QString::number ( i ) );
		rec2.fastAppendValue ( QString::number ( i ) + QString::number ( i ) );
	}
	if ( !file1->getRecord ( rec3, 0 ) )
		file1->appendRecord ( rec1 );
	else
		file1->changeRecord ( 0, rec2 );

	rec1.clear ();
	rec2.clear ();
	for ( uint i ( 10 ); i < 20; ++i )
	{
		rec1.fastAppendValue ( QString::number ( i ) );
		rec2.fastAppendValue ( QString::number ( i ) + QString::number ( i ) );
	}
	if ( !file1->getRecord ( rec3, 1 ) )
		file1->appendRecord ( rec1 );
	else
		file1->changeRecord ( 1, rec2 );

	rec1.clear ();
	rec1.insertField ( 0, "5" );
	rec1.insertField ( 1, "55" );
	rec1.insertField ( 2, "555" );
	rec2.clear ();
	rec2.insertField ( 0, "55" );
	rec2.insertField ( 1, "5555" );
	rec2.insertField ( 2, "555555" );

	if ( rec3.isNull () )
		file1->insertRecord ( 1, rec1 );
	else
		file1->insertRecord ( 1, rec2 );

	rec1.clear ();
	for ( uint x ( 0 ); x < file1->recCount (); ++x )
	{
		file1->getRecord ( rec1, static_cast<int>(x) );
		for ( uint y ( 0 ); y < rec1.nFields (); ++y )
			qDebug () << rec1.fieldValue ( y );
		qDebug () << endl << "-----------------------" << endl;
	}

	file1->deleteRecord ( 1 );
	rec1.clear ();
	for ( uint x ( 0 ); x < file1->recCount (); ++x )
	{
		file1->getRecord ( rec1, static_cast<int>(x) );
		for ( uint y ( 0 ); y < rec1.nFields (); ++y )
			qDebug () << rec1.fieldValue ( y );
		qDebug () << endl << "-----------------------" << endl;
	}

	file1->commit ();
	delete file1;
	return 0;
	QApplication a(argc, argv);
	MainWindow w;
	w.show();

	return a.exec();
}
