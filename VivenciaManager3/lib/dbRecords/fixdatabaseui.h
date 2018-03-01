#ifndef FIXDATABASEUI_H
#define FIXDATABASEUI_H

#include "vmlist.h"
#include "fixdatabase.h"

#include <QDialog>

class vmTableWidget;

class QPushButton;

class fixDatabaseUI : public QDialog
{

public:
	virtual ~fixDatabaseUI ();
	void showWindow ();
	void doCheck ();
	void doFix ();

private:
	explicit fixDatabaseUI ();

	void setupUI ();
	void populateTable ();
	friend void deleteFixDBInstance ();
	static fixDatabaseUI *s_instance;
	friend fixDatabaseUI *FIX_DB ();

	vmTableWidget* tablesView;

	fixDatabase *m_fdb;
	QPushButton* btnCheck;
	QPushButton* btnFix;
	QPushButton* btnClose;

	PointersList<fixDatabase::badTable*> m_tables;
};

inline fixDatabaseUI *FIX_DB ()
{
	if ( !fixDatabaseUI::s_instance )
		fixDatabaseUI::s_instance = new fixDatabaseUI;
	return fixDatabaseUI::s_instance;
}

#endif // FIXDATABASEUI_H
