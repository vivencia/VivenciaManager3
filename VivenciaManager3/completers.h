#ifndef COMPLETERS_H
#define COMPLETERS_H

#include "vmlist.h"

#include <QObject>
#include <QCompleter>
#include <QStringList>

const uint COMPLETERS_COUNT ( 15 );

class DBRecord;
class completerRecord;

class QLineEdit;

class vmCompleters
{

public:
	~vmCompleters ();

	static void init () {
		if ( !s_instance )
			s_instance = new vmCompleters;
	}

	enum COMPLETER_CATEGORIES {
		NONE = -1, ALL_CATEGORIES = 0, PRODUCT_OR_SERVICE = 1,
		SUPPLIER = 2, BRAND = 3, STOCK_TYPE = 4, STOCK_PLACE = 5,
		PAYMENT_METHOD = 6, ADDRESS = 7, ITEM_NAMES = 8, CLIENT_NAME = 9,
		DELIVERY_METHOD = 10, ACCOUNT = 11, JOB_TYPE = 12, MACHINE_NAME = 13,
		MACHINE_EVENT = 14
	};

	/* To use a completer call setComplete */
	void setCompleter ( QLineEdit* , const COMPLETER_CATEGORIES type ) const;

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

	completerRecord* cr_rec;

	PointersList<QCompleter*> completersList;
};

inline vmCompleters *APP_COMPLETERS ()
{
	return vmCompleters::s_instance;
}

#endif // COMPLETERS_H
