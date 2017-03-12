#include "cleanup.h"
#include "vmlist.h"

typedef PointersList<VMCleanUpFunction> VMVFuncList;
static VMVFuncList funcList ( 25 );

void addPostRoutine ( VMCleanUpFunction func , const bool bEarlyExec )
{
	if ( bEarlyExec )
		( func ) ();
    funcList.append ( func );
}

void cleanUpApp ()
{
    for ( int i ( static_cast<int>(funcList.count ()) - 1 ); i >= 0 ; --i )
        ( funcList.at ( i ) ) ();
    funcList.clear ();
}
