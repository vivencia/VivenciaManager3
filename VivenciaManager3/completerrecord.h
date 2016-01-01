#ifndef COMPLETERRECORD_H
#define COMPLETERRECORD_H

#include "global.h" // after TRANSITION_PERIOD is over, remove this line

#include "dbrecord.h"
#include "completers.h"

#include <QStringList>
#include <QtSql/QSqlQuery>

const uint CR_FIELD_COUNT ( 13 );

class completerRecord : public DBRecord
{

friend class VivenciaDB;
friend class Data;
friend class vmCompleters;

public:
	completerRecord ();
	virtual ~completerRecord ();

	void loadCompleterStrings ( QStringList& completer_strings, const vmCompleters::COMPLETER_CATEGORIES );
	void loadCompleterStringsForProductsAndServices ( QStringList& completer_strings, QStringList& completer_strings_2 );
	void updateTable ( const vmCompleters::COMPLETER_CATEGORIES category, const QString& str );

protected:
	friend bool updateCompleterRecordTable ();
	static const TABLE_INFO t_info;

	RECORD_FIELD m_RECFIELDS[CR_FIELD_COUNT];
	void ( *helperFunction[CR_FIELD_COUNT] ) ( const DBRecord* );

private:
	QSqlQuery query;
	void runQuery ( QStringList& results, const TABLE_INFO* t_info, const uint field, const bool b_check_duplicates = false );

#ifdef TRANSITION_PERIOD
	// These are not needed after creation of table
	void updateCompleterInternal ( const uint field, const QStringList& str_list );
#endif
};

#endif // COMPLETERRECORD_H
