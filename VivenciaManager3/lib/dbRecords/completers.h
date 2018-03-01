#ifndef COMPLETERS_H
#define COMPLETERS_H

#include "common/vmlist.h"

#include <QObject>
#include <QCompleter>
#include <QStringList>

const uint COMPLETERS_COUNT ( 15 );

class DBRecord;
class completerRecord;
class vmLineEdit;
class vmComboBox;

class vmCompleters
{

public:
	~vmCompleters ();

	static void init () {
		if ( !s_instance )
			s_instance = new vmCompleters;
	}

	/* To use a completer call setComplete */
	void setCompleter ( vmLineEdit* line , const COMPLETER_CATEGORIES type ) const;
	void setCompleter ( vmComboBox* cbo, const COMPLETER_CATEGORIES type ) const;

	inline QCompleter* getCompleter ( const COMPLETER_CATEGORIES type ) const
	{
		return completersList.at ( static_cast<int> ( type ) );
	}

	/* To update the completer with newly entered values, put updateCompleter some where in VivenciaDB */
	void updateCompleter ( const QString& str, const COMPLETER_CATEGORIES type );

	void fillList ( const vmCompleters::COMPLETER_CATEGORIES type, QStringList &list ) const;
	int inList ( const QString& str, const vmCompleters::COMPLETER_CATEGORIES type ) const;

	COMPLETER_CATEGORIES completerType ( QCompleter* completer, const QString& completion = QString::null ) const;
	void encodeCompleterISRForSpreadSheet ( const DBRecord* dbrec );

private:
	explicit vmCompleters ();
	static vmCompleters* s_instance;
	friend vmCompleters *APP_COMPLETERS ();
	friend void deleteCompletersInstance ();
	void loadCompleters ();

	PointersList<QCompleter*> completersList;
};

inline vmCompleters *APP_COMPLETERS ()
{
	return vmCompleters::s_instance;
}

#endif // COMPLETERS_H
