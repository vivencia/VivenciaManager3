#ifndef CLIENT_H
#define CLIENT_H

#include "dbrecord.h"

class clientListItem;

const uint CLIENT_FIELD_COUNT ( 12 );

static const uint CLIENTS_FIELDS_TYPE[CLIENT_FIELD_COUNT] = {
	DBTYPE_ID, DBTYPE_LIST, DBTYPE_LIST, DBTYPE_NUMBER, DBTYPE_LIST, DBTYPE_LIST,
	DBTYPE_NUMBER, DBTYPE_SUBRECORD, DBTYPE_SUBRECORD, DBTYPE_DATE, DBTYPE_DATE,
	DBTYPE_YESNO
};

class Client : public DBRecord
{

    friend class VivenciaDB;
    friend class Data;
    friend class tableView;

public:
    explicit Client ( const bool connect_helper_funcs = false );
    virtual ~Client ();

	inline DB_FIELD_TYPE fieldType ( const uint field ) const {
		return static_cast<DB_FIELD_TYPE> ( CLIENTS_FIELDS_TYPE[field] ); }
	
    int searchCategoryTranslate ( const SEARCH_CATEGORIES sc ) const;
	void copySubRecord ( const uint subrec_field, const stringRecord& subrec );

    static inline const QString clientName ( const int id ) {
        return ( id > 0 ) ? clientName ( QString::number ( id ) ) : QString::null;
    }
    static QString clientName ( const QString& id );
    static int clientID ( const QString& name );
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
