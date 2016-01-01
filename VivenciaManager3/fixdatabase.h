#ifndef FIXDATABASE_H
#define FIXDATABASE_H

#include "vmlist.h"

#include <QString>

class passwordManager;

class QFile;

class fixDatabase
{

public:
	explicit fixDatabase ();
	~fixDatabase ();

	enum CheckResult {
		CR_UNDEFINED = -1, CR_OK = 0, CR_TABLE_CORRUPTED = 1, CR_TABLE_CRASHED = 2
	};

	struct badTable {
		QString table;
		QString err;
		CheckResult result;
	};

	inline bool needsFixing () const {
		return b_needsfixing;
	}

	bool checkDatabase ();
	bool fixTables ( const QString& table = QLatin1String ( "*" ) );

	inline void badTables ( PointersList<fixDatabase::badTable*>& tables ) const {
		tables = m_badtables;
	}

private:
	bool readOutputFile ( const QString& r_passwd );

	passwordManager* mPasswdMngr;
	PointersList<fixDatabase::badTable*> m_badtables;
	bool b_needsfixing;
	QString mStrOutput;
};

#endif // FIXDATABASE_H
