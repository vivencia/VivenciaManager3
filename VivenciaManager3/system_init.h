#ifndef DATA_H
#define DATA_H

#include <QString>

class QIcon;
typedef void ( *VMCleanUpFunction ) ();

namespace Sys_Init
{
	extern void init ();
	extern void deInit ();
	extern void loadDataIntoMemory ();

	extern void addPostRoutine ( VMCleanUpFunction func, const bool bEarlyExec = false );
	extern void cleanUpApp ();
	
	extern bool checkSystem ( const bool bFirstPass = true );
	extern void checkSetup ();
	extern void checkDatabase ();
	extern void restartProgram ();
	extern bool isMySQLRunning();
	extern QString commandMySQLServer ( const QString& command, const QString& message, const bool only_return_cmd_line = false );

	extern QIcon* listIndicatorIcons[4];
	extern bool EXITING_PROGRAM;
}
#endif // DATA_H
