#include "htmldb.h"
#include "configops.h"
#include "fileops.h"
#include "cleanup.h"
#include "heapmanager.h"

htmlDB* htmlDB::s_instance ( nullptr );

void deletehtmlDB_instance ()
{
	heap_del ( htmlDB::s_instance );
}

htmlDB::htmlDB ()
{
	checkInit ();
	addPostRoutine ( deletehtmlDB_instance );
}

htmlDB::~htmlDB () {}

void htmlDB::checkInit ()
{
	return;
}
