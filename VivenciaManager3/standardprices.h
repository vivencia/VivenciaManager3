#ifndef servicesOffered_H
#define servicesOffered_H

#include "dbrecord.h"

const uint SERVICES_FIELD_COUNT ( 8 );

class servicesOffered : public DBRecord
{

    friend class VivenciaDB;
    friend class Data;
    friend class servicesOfferedUI;

public:
    servicesOffered ();
    virtual ~servicesOffered ();

    static const TABLE_INFO t_info;

private:
    RECORD_FIELD m_RECFIELDS[SERVICES_FIELD_COUNT];
    void ( *helperFunction[SERVICES_FIELD_COUNT] ) ( const DBRecord* );
};

#endif // servicesOffered_H
