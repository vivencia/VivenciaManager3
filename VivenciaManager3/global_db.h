#ifndef GLOBAL_DB_H
#define GLOBAL_DB_H

#include "vmlist.h"
#include "global_enums.h"

#include <QString>

struct RECORD_FIELD
{

public:

	enum { IDX_ACTUAL = 0, IDX_TEMP = 1 };

	int i_field[2];
	bool modified;
	bool was_modified;
	QString str_field[2];

	RECORD_FIELD () : modified ( false ), was_modified ( false )
	{
		i_field[0] = i_field[1] = -1;
	}
};

enum DB_FIELD_TYPE
{
	DBTYPE_ID = 0, DBTYPE_YESNO = 1, DBTYPE_LIST, DBTYPE_FILE,
	DBTYPE_SHORTTEXT, DBTYPE_LONGTEXT, DBTYPE_SUBRECORD,
	DBTYPE_DATE, DBTYPE_TIME, DBTYPE_PHONE, DBTYPE_PRICE, DBTYPE_NUMBER
};

enum RECORD_ACTION
{
	ACTION_NONE = -2, ACTION_REVERT = -1, ACTION_READ = 0, ACTION_DEL = 1, ACTION_ADD = 2, ACTION_EDIT = 3
};

struct TABLE_INFO //compile-time set
{
	const unsigned int table_id;
	const QString table_name;
	const QString table_flags;
	const QString primary_key;
	const QString field_names;
	const QString field_flags;
	const QString field_labels;
	const DB_FIELD_TYPE* field_types;
	const unsigned char version;
	uint field_count;
	const unsigned int table_order;
	bool ( *update_func ) ();// New field: From version 2.0, upates are conducted within the originating module
#ifdef TRANSITION_PERIOD
	/* certain routines in the transition period from the 2 to the 3 series (re)create tables. These
	 * tables are created orderly and do not need, for example, to have their IDs rearranged. Other update
	 * routines might check this flag, although none does so far
	 */

	bool new_table;
#endif
};

#endif // GLOBAL_DB_H
