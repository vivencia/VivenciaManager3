#ifndef USERMANAGEMENT_H
#define USERMANAGEMENT_H

#include "userrecord.h"

#include <QCoreApplication>

class userRecord;
class userManagementUI;
class passwordManager;

static const QString ADMIN_PRIVILEDGES_STR ( QCoreApplication::tr ( "Administrator priviledges required!" ) );
static const QString ADMIN_PASSWORD_WRONG_STR ( QCoreApplication::tr ( "Wrong administrator's password!" ) );
static const QString ADMIN_PASSWORD_STR ( QCoreApplication::tr ( "Please enter administrator's password: " ) );

enum USR_MNGR_RETURN_CODE {
    LOGIN_OTHER_ERROR = -1, NO_ADMIN_PRIVILEDGES = 2<<0, USER_REGISTERED = 2<<1,
    USER_UNREGISTERED = 2<<2, PASSWORD_DOESNT_MATCH = 2<<3,
    PASSWORD_MATCHES = 2<<4, USERNAME_EMPTY = 2 << 5, USERNAME_TOO_SHORT = 2<<6,
    USER_PASSWORD_TOO_SHORT = 2<<7
};

class UserManagement
{

private:

    friend class VivenciaDB;
    friend class userManagementUI;

    explicit UserManagement ();
    userRecord* user_rec;

    struct privileges_st
    {
        uint readable_tables;
        uint writable_tables;
        bool can_export; // will only export tables with read access
        bool can_import; // will only import tables with read and write access
        bool is_admin;

        ulong bitTable;

        privileges_st () :
            readable_tables ( 0 ),
            writable_tables ( 0 ),
            can_export ( true ), can_import ( false ),is_admin ( false ), bitTable ( 0 ) {}
    };

    struct USER_INFO {
        QString name;
        QString passwd;
        QString location;
        privileges_st privileges;
    };

    void privilegesToDB ( USER_INFO* usr_info ) const;
    void DBtoPrivileges ( USER_INFO* usr_info ) const;
    QString findAdminUser () const;

    //friend bool updateUsersTable ();
    static UserManagement* s_instance;
    friend UserManagement* USRMNGR ();
    friend void deleteUserManagementInstance ();

    QString m_strErr;
    USR_MNGR_RETURN_CODE mRetCode;
    userManagementUI* mDlgWindow;
    passwordManager* mPasswdMngr;
    USER_INFO mUsrInfo;

public:
    ~UserManagement ();

    void showConfigWindow ( const QString& username = QString::null, const bool modal = false );

    inline USR_MNGR_RETURN_CODE returnCode () const {
        return mRetCode;
    }
    inline const QString& returnCodeString () const {
        return m_strErr;
    }

    void checkUserName ( const QString& user );
    void checkPassword ( const QString& username, const QString& password );

    bool loadUserInfo ( const QString& username, USER_INFO* usr_info, const bool check_only = false );

    // Use TABLE_ORDER enum for the following functions
    void setCanRead ( const bool read, const uint table_order, USER_INFO* usr_info ) const;
    bool canRead ( const uint table_order, USER_INFO* usr_info ) const;
    bool canRead ( const QString& user, const uint table_order );

    void setCanWrite ( const bool write, const uint table_order, USER_INFO* usr_info ) const;
    bool canWrite ( const uint table_order, USER_INFO* usr_info ) const;
    bool canWrite ( const QString& user, const uint table_order );

    void setCanImport ( const bool import, USER_INFO* usr_info ) const;
    bool canImport ( const USER_INFO* const usr_info ) const;
    bool canImport ( const QString& user );

    void setCanExport ( const bool b_export, USER_INFO* usr_info ) const;
    bool canExport ( const USER_INFO* const usr_info ) const;
    bool canExport ( const QString& user );

    bool isAdmin ( const USER_INFO* const usr_info ) const;
    bool isAdmin ( const QString& user );

    void setUserLocation ( USER_INFO* usr_info );

    bool askForAdminPassord () const;
    void doActualInsertOrEdit ( const USER_INFO* const usr_info );
    void insertUser ( const USER_INFO* const usr_info );
    void insertAdminRecord ( USER_INFO* const usr_info );
    void editUser ( USER_INFO* usr_info );
    void removeUser ( USER_INFO* usr_info );
};

inline UserManagement* USRMNGR ()
{
    if ( !UserManagement::s_instance )
        UserManagement::s_instance = new UserManagement;
    return UserManagement::s_instance;
}

#endif // USERMANAGEMENT_H
