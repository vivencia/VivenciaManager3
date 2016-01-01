#ifndef USERRECORD_H
#define USERRECORD_H

#include "dbrecord.h"

const uint USERS_FIELD_COUNT ( 5 );

class userRecord : public DBRecord
{

    friend class VivenciaDB;
    friend class Data;
    friend class userManagement;

public:
    userRecord ();
    virtual ~userRecord ();

    static const TABLE_INFO t_info;

    RECORD_FIELD m_RECFIELDS[USERS_FIELD_COUNT];
    void ( *helperFunction[USERS_FIELD_COUNT] ) ( const DBRecord* );
};

#endif // USERRECORD_H
