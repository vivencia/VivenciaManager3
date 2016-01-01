#ifndef CLIENT_H
#define CLIENT_H

#include "dbrecord.h"

const uint CLIENT_FIELD_COUNT ( 12 );

class Client : public DBRecord
{

    friend class VivenciaDB;
    friend class Data;
    friend class tableView;

public:
    explicit Client ( const bool connect_helper_funcs = false );
    virtual ~Client ();

    int searchCategoryTranslate ( const SEARCH_CATEGORIES sc ) const;

    static inline const QString clientName ( const int id ) {
        return ( id > 0 ) ? clientName ( QString::number ( id ) ) : QString::null;
    }
    static QString clientName ( const QString& id );
    static int clientID ( const QString& name );
    static QString concatenateClientInfo ( const Client& client );

    static const TABLE_INFO t_info;

protected:
    friend bool updateClientTable ();

    RECORD_FIELD m_RECFIELDS[CLIENT_FIELD_COUNT];
    void ( *helperFunction[CLIENT_FIELD_COUNT] ) ( const DBRecord* );
};

#endif // CLIENT_H
