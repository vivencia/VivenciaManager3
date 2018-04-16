#include <textdb.h>

#include <QtWidgets/QApplication>
#include <QMutex>
#include <QWaitCondition>

static void msleep(unsigned long msecs)
	{
		QMutex mutex;
		mutex.lock();

		QWaitCondition waitCondition;
		waitCondition.wait(&mutex, msecs);

		mutex.unlock();
	}

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	configFile cfg1;
	cfg1.setFileName ( "/home/guilherme/Documents/VivenciaManager/Test vm components/TestConfigFile/cfg1.cfg" );
	cfg1.load ( false );
	if ( !cfg1.setWorkingSection ( "CFG1-SECTION1" ) )
	{
		cfg1.insertNewSection ( "CFG1-SECTION1" );
		cfg1.insertField ( "FIELD11", "VALUE1" );
		cfg1.insertField ( "FIELD12", "VALUE2" );
		cfg1.insertField ( "FIELD13", "VALUE3" );
	}
	else
	{
		cfg1.setFieldValue ( "FIELD13", "VALUE3333333" );
		cfg1.insertField ( "FIELD14", "VALUE4" );
	}

	if ( !cfg1.setWorkingSection ( "CFG1-SECTION2" ) )
	{
		cfg1.insertNewSection ( "CFG1-SECTION2" );
		cfg1.insertField ( "FIELD21", "VALUE1" );
		cfg1.insertField ( "FIELD22", "VALUE2" );
		cfg1.insertField ( "FIELD23", "VALUE3" );
	}
	else
	{
		cfg1.setFieldValue ( "FIELD21", "VALUE22222222" );
		cfg1.insertField ( "FIELD24", "VALUE4" );
	}

	cfg1.commit ();
	//msleep ( 2000 );

	configFile cfg1a;
	cfg1a.setIgnoreEvents ( true );
	cfg1a.setFileName ( "/home/guilherme/Documents/VivenciaManager/Test vm components/TestConfigFile/cfg1.cfg" );
	cfg1a.load ( true );
	if ( !cfg1a.setWorkingSection ( "CFG1-SECTION3" ) )
	{
		cfg1a.insertNewSection ( "CFG1-SECTION3" );
		cfg1a.insertField ( "FIELD31", "VALUE1" );
		cfg1a.insertField ( "FIELD32", "VALUE2" );
		cfg1a.insertField ( "FIELD33", "VALUE3" );
	}

	if ( cfg1a.setWorkingSection ( "CFG1-SECTION1" ) )
		cfg1a.setFieldValue ( "FIELD12", "NEW_VALUE2" );

	cfg1a.commit ();
	msleep ( 300 );

	if ( cfg1.setWorkingSection ( "CFG1-SECTION3" ) )
	{
		cfg1.setFieldValue ( "FIELD33", "NEW_VALUE333" );
		cfg1.setFieldValue ( "FIELD32", "NEW_VALUE323232" );
	}
	cfg1.commit ();

	//msleep ( 4000 );

	return 0;
}
