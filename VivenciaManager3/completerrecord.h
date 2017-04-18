#ifndef COMPLETERRECORD_H
#define COMPLETERRECORD_H

#include "dbrecord.h"
#include "completers.h"

#include <QStringList>
#include <QtSql/QSqlQuery>

const uint CR_FIELD_COUNT ( 13 );

class completerRecord : public DBRecord
{

friend class VivenciaDB;
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
	void updateCompleterInternal ( const uint field, const QStringList& str_list );
};

#endif // COMPLETERRECORD_H
