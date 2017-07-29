#ifndef CLIENT_H
#define CLIENT_H

#include "dbrecord.h"

class clientListItem;

const uint CLIENT_FIELD_COUNT ( 12 );

class Client : public DBRecord
{

friend class VivenciaDB;
friend class tableView;

public:
	explicit Client ( const bool connect_helper_funcs = false );
	virtual ~Client ();

	int searchCategoryTranslate ( const SEARCH_CATEGORIES sc ) const;
	void copySubRecord ( const uint subrec_field, const stringRecord& subrec );

	static inline const QString clientName ( const uint id ) {
		return ( id >= 1 ) ? clientName ( QString::number ( id ) ) : QString::null;
	}
	static QString clientName ( const QString& id );
	static uint clientID ( const QString& name );
	static QString concatenateClientInfo ( const Client& client );

	void setListItem ( clientListItem* client_item );
	clientListItem* clientItem () const;

	static const TABLE_INFO t_info;

protected:
	friend bool updateClientTable ();

	RECORD_FIELD m_RECFIELDS[CLIENT_FIELD_COUNT];
	void ( *helperFunction[CLIENT_FIELD_COUNT] ) ( const DBRecord* );
};

#endif // CLIENT_H
