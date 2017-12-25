#include "htmldb.h"
#include "configops.h"
#include "fileops.h"
#include "system_init.h"
#include "heapmanager.h"

htmlDB* htmlDB::s_instance ( nullptr );

void deletehtmlDB_instance ()
{
	heap_del ( htmlDB::s_instance );
}

htmlDB::htmlDB ()
{
	checkInit ();
	Sys_Init::addPostRoutine ( deletehtmlDB_instance );
}

htmlDB::~htmlDB () {}

void htmlDB::checkInit ()
{
	return;
}
