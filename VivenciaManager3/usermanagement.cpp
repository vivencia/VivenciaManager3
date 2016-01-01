#include "usermanagement.h"
#include "usermanagementui.h"
#include "global.h"
#include "passwordmanager.h"
#include "data.h"
#include "global_enums.h"
#include "vivenciadb.h"
#include "logindialog.h"
#include "cleanup.h"
#include "heapmanager.h"
#include "vmnotify.h"
#include "fast_library_functions.h"

#include <QCoreApplication>

#include <cmath>
/*	End bit			 Start bit	 End bit	     Start bit 	            Bit number
 *    63                36          35              8           7 6 5 4 3      2     1   0
 *     1 ..............  1   	    1...............1           1 1 1 1 1      1     1   1
 *     28 Writable Tables		    28 Readable Tables		    5 Reserved   Admin  Exp Imp
*/

static const uchar MAX_TABLES ( 28 );
static const uchar READABLE_TABLES_SHIFT ( 8 );
static const uchar WRITABLE_TABLES_SHIFT ( READABLE_TABLES_SHIFT + MAX_TABLES );
static const uchar IMPORT_BIT	( 0 );
static const uchar EXPORT_BIT	( 1 );
static const uchar ADMIN_BIT	( 2 );

static const QString PASSWORD_DOESNT_MATCH_STR ( QCoreApplication::tr ( "Password does not match!" ) );
static const QString USER_UNREGISTERED_STR ( QCoreApplication::tr ( "The selected user name is not registered!" ) );
static const QString USER_REGISTERED_ERR_STR ( QCoreApplication::tr ( "The selected user name is already registered!" ) );
static const QString USER_REGISTERED_OK_STR ( QCoreApplication::tr ( "User registered! Please enter the password." ) );
static const QString PASSWORD_LENGTH_STR ( QCoreApplication::tr ( "The password length must be at least 4 characters long." ) );
static const QString NAME_LENGTH_STR ( QCoreApplication::tr ( "The name length must be at least 4 characters long." ) );
static const QString USERNAME_REQUIRED_STR ( QCoreApplication::tr ( "Please enter the user name." ) );
static const QString PASSWORD_OK_STR ( QCoreApplication::tr ( "Password is correct." ) );
static const QString OTHER_ERROR_STR ( QCoreApplication::tr ( "An unknown error occurred while writing to the database. Please try again." ) );

UserManagement* UserManagement::s_instance ( nullptr );

void deleteUserManagementInstance ()
{
    heap_del ( UserManagement::s_instance );
}

/*static inline uint tableOrderToTableID ( const uint order )
{
	return ( 2 << ( order - 1 ) );
}

static inline uint tableIDToTableOrder ( const uint id )
{
	return ( log2 ( id ) );
}*/

UserManagement::UserManagement ()
    : user_rec ( nullptr ), mDlgWindow ( nullptr ), mPasswdMngr ( nullptr )
{
    user_rec = new userRecord;
	mPasswdMngr = new passwordManager ( passwordManager::PWSS_SAVE );
    addPostRoutine ( deleteUserManagementInstance );
}

UserManagement::~UserManagement ()
{
    heap_del ( mDlgWindow );
    heap_del ( user_rec );
    heap_del ( mPasswdMngr );
}

void UserManagement::showConfigWindow ( const QString& username, const bool modal )
{
    if ( !mDlgWindow )
        mDlgWindow = new userManagementUI;
    mDlgWindow->showWindow ( username, modal );
}

void UserManagement::privilegesToDB ( USER_INFO* usr_info ) const
{
    privileges_st* usr_priv ( &usr_info->privileges );
    ulong& bittable ( usr_priv->bitTable );
    bittable = 0;

    if ( usr_priv->can_import )
        setBit ( bittable, IMPORT_BIT );
    if ( usr_priv->can_export )
        setBit ( bittable, EXPORT_BIT );
    if ( usr_priv->is_admin )
        setBit ( bittable, ADMIN_BIT );
    for ( uchar i ( 0 ); i < MAX_TABLES; ++i ) {
        if ( isBitSet ( usr_priv->readable_tables, i ) )
            setBit ( bittable, READABLE_TABLES_SHIFT + i );
        if ( isBitSet ( usr_priv->writable_tables, i ) )
            setBit ( bittable, WRITABLE_TABLES_SHIFT + i );
    }
}

void UserManagement::DBtoPrivileges ( USER_INFO* usr_info ) const
{
    privileges_st* usr_priv ( &usr_info->privileges );
    ulong& bittable ( usr_priv->bitTable );

    usr_priv->can_import =	isBitSet ( bittable, IMPORT_BIT );
    usr_priv->can_export =	isBitSet ( bittable, EXPORT_BIT );
    usr_priv->is_admin =	isBitSet ( bittable, ADMIN_BIT );

    for ( uchar i ( 0 ); i < MAX_TABLES; ++i ) {
        if ( isBitSet ( bittable, i + READABLE_TABLES_SHIFT ) )
            setBit ( usr_priv->readable_tables, i );
        if ( isBitSet ( bittable, i + WRITABLE_TABLES_SHIFT ) )
            setBit ( usr_priv->writable_tables, i );
    }
}

QString UserManagement::findAdminUser () const
{
    if ( user_rec->readFirstRecord () ) {
        ulong privileges ( 0 );
        do {
            privileges = recStrValue ( user_rec, FLD_USER_PRIVILEDGES ).toULong ();
            if ( isBitSet ( privileges, ADMIN_BIT ) ) {
                mPasswdMngr->fromString ( recStrValue ( user_rec, FLD_USER_INFO ) );
                return mPasswdMngr->getID ();
            }
        } while ( user_rec->readNextRecord () );
    }
    return emptyString;
}

bool UserManagement::loadUserInfo ( const QString& user, USER_INFO* usr_info, const bool check_only )
{
    if ( usr_info != nullptr ) {
        // For only a few users this method is as good as any. Since I do not want the user name appearing
        // in the database, going through every record is the only way I could conceive.
        if ( user_rec->readFirstRecord () ) {
            do {
                mPasswdMngr->fromString ( recStrValue ( user_rec, FLD_USER_INFO ) );
                if ( mPasswdMngr->exists ( user ) ) {
                    if ( !check_only ) {
                        usr_info->name = user;
                        usr_info->passwd = mPasswdMngr->getPassword ( user );
                        usr_info->privileges.bitTable = recStrValue ( user_rec, FLD_USER_PRIVILEDGES ).toULong ();
                        usr_info->location = recStrValue ( user_rec, FLD_USER_LOCATION );
                    }
                    return true;
                }
            } while ( user_rec->readNextRecord () );
        }
    }
    return false;
}

void UserManagement::checkUserName ( const QString& user )
{
    if ( user.length () < 4 ) {
        if ( user.isEmpty () ) {
            mRetCode = USERNAME_EMPTY;
            m_strErr = USERNAME_REQUIRED_STR;
        }
        else {
            mRetCode = USERNAME_TOO_SHORT;
            m_strErr = NAME_LENGTH_STR;
        }
    }
    if ( !loadUserInfo ( user, &mUsrInfo, true ) ) {
        mRetCode = USER_UNREGISTERED;
        m_strErr = USER_UNREGISTERED_STR;
    }
    else {
        mRetCode = USER_REGISTERED;
        m_strErr = USER_REGISTERED_OK_STR;
    }
}

void UserManagement::checkPassword ( const QString& user, const QString& password )
{
    checkUserName ( user );
    if ( mRetCode == USER_REGISTERED ) {
        if ( password.length () < 4 ) {
            mRetCode = USER_PASSWORD_TOO_SHORT;
            m_strErr = PASSWORD_LENGTH_STR;
        }
        else {
            if ( mPasswdMngr->getPassword ( user ) != password ) {
                mRetCode = PASSWORD_DOESNT_MATCH;
                m_strErr = PASSWORD_DOESNT_MATCH_STR;
            }
            else {
                mRetCode = PASSWORD_MATCHES;
                m_strErr = PASSWORD_OK_STR;
            }
        }
    }
}

void UserManagement::setCanRead ( const bool read, const uint table_order, USER_INFO* usr_info ) const
{
    if ( read )
        setBit ( usr_info->privileges.readable_tables, READABLE_TABLES_SHIFT + table_order );
    else
        unSetBit ( usr_info->privileges.readable_tables, READABLE_TABLES_SHIFT + table_order );
}

bool UserManagement::canRead ( const uint table_order , USER_INFO* usr_info ) const
{
    return isBitSet ( usr_info->privileges.bitTable, READABLE_TABLES_SHIFT + table_order );
}

bool UserManagement::canRead ( const QString& user, const uint table_order )
{
    if ( loadUserInfo ( user, &mUsrInfo ) )
        return canRead ( table_order, &mUsrInfo );
    return false;
}

void UserManagement::setCanWrite ( const bool write, const uint table_order, USER_INFO* usr_info ) const
{
    if ( write )
        setBit ( usr_info->privileges.writable_tables, WRITABLE_TABLES_SHIFT + table_order );
    else
        unSetBit ( usr_info->privileges.writable_tables, WRITABLE_TABLES_SHIFT + table_order );
}

bool UserManagement::canWrite ( const uint table_order, USER_INFO* usr_info ) const
{
    return isBitSet ( usr_info->privileges.bitTable, WRITABLE_TABLES_SHIFT + table_order );
}

bool UserManagement::canWrite ( const QString& user, const uint table_order )
{
    if ( loadUserInfo ( user, &mUsrInfo ) )
        return canWrite ( table_order, &mUsrInfo );
    return false;
}

void UserManagement::setCanImport ( const bool import, USER_INFO* usr_info ) const
{
    usr_info->privileges.can_import = import;
}

bool UserManagement::canImport ( const USER_INFO* const usr_info ) const
{
    return isBitSet ( usr_info->privileges.bitTable, IMPORT_BIT );
}

bool UserManagement::canImport ( const QString& user )
{
    if ( loadUserInfo ( user, &mUsrInfo ) )
        return canImport ( &mUsrInfo );
    return false;
}

void UserManagement::setCanExport ( const bool b_export, USER_INFO* usr_info ) const
{
    if ( canRead ( TABLE_GENERAL_ORDER, usr_info ) )
        usr_info->privileges.can_export = b_export;
}

bool UserManagement::canExport ( const USER_INFO* const usr_info ) const
{
    return isBitSet ( usr_info->privileges.bitTable, EXPORT_BIT );
}

bool UserManagement::canExport ( const QString& user )
{
    if ( loadUserInfo ( user, &mUsrInfo ) )
        return canExport ( &mUsrInfo );
    return false;
}

bool UserManagement::isAdmin ( const USER_INFO* const usr_info ) const
{
    return isBitSet ( usr_info->privileges.bitTable, ADMIN_BIT );
}

bool UserManagement::isAdmin ( const QString& user )
{
    if ( loadUserInfo ( user, &mUsrInfo ) )
        return isAdmin ( &mUsrInfo );
    return false;
}

//Will be improved in the future
void UserManagement::setUserLocation ( USER_INFO* usr_info )
{
    usr_info->location = QStringLiteral ( "localhost" );
}

bool UserManagement::askForAdminPassord () const
{
	QString password;
	if ( vmNotify::inputBox ( password, nullptr, QCoreApplication::tr ( "Admnistrator password required" ),
							 QCoreApplication::tr ( "Type admin's password to execute this task" ) ) ) {
	   const QString admin_user ( findAdminUser () );
	   const_cast<UserManagement*>( this )->checkPassword ( admin_user, password );
		if ( mRetCode != PASSWORD_MATCHES )
			VM_NOTIFY ()->messageBox ( QCoreApplication::tr ( "Incorrect password" ), QCoreApplication::tr ( "Won't proceed." ) );
		else
			return true;
	}
	return false;
}

void UserManagement::doActualInsertOrEdit ( const USER_INFO* const usr_info )
{
    setRecValue ( user_rec, FLD_USER_LOCATION, usr_info->location );
    setRecValue ( user_rec, FLD_USER_PRIVILEDGES, QString::number ( usr_info->privileges.bitTable ) );
    mPasswdMngr->clear ();
	//mPasswdMngr->insert ( usr_info->name, usr_info->passwd );
    setRecValue ( user_rec, FLD_USER_INFO, mPasswdMngr->toString ( usr_info->name ) );
    user_rec->saveRecord ();
}

void UserManagement::insertUser ( const USER_INFO* const usr_info )
{
    if ( VDB ()->recordCount ( &( user_rec->t_info ) ) == 0 )
        insertAdminRecord ( const_cast<USER_INFO*> ( usr_info ) );
    else {
        if ( !loadUserInfo ( usr_info->name, const_cast<USER_INFO*> ( usr_info ), true ) ) {
            if ( askForAdminPassord () ) {
                user_rec->clearAll ();
                user_rec->setAction ( ACTION_ADD );
                doActualInsertOrEdit ( usr_info );
            }
        }
    }
}

void UserManagement::insertAdminRecord ( USER_INFO* const usr_info )
{
    static const QString firstUserStr ( QCoreApplication::tr (
                                            "The first registerd user for this application will be known as the Administrator."
                                            " This account is responsible for adding and/or removing other users, perform database "
                                            " maintenance tasks among other things. Proceed if you are intent on being this kind of user."
                                            " If you don't, the program will exit." )
                                      );
	if ( VM_NOTIFY ()->questionBox ( QCoreApplication::tr ( "Create ADMIN user" ), firstUserStr ) ) {
        // The login dialog will reload the information afterwards. Make sure that information reflects the status of admin's permissions
        usr_info->privileges.is_admin = true;
        usr_info->privileges.can_export = true;
        usr_info->privileges.can_import = true;
        setAllBits ( usr_info->privileges.readable_tables );
        setAllBits ( usr_info->privileges.writable_tables );
        setAllBits ( usr_info->privileges.bitTable );
        user_rec->clearAll ();
        user_rec->setAction ( ACTION_ADD );
        doActualInsertOrEdit ( usr_info );
    }
}

void UserManagement::editUser ( USER_INFO* usr_info )
{
    if ( loadUserInfo ( usr_info->name, usr_info ) ) {
        if ( askForAdminPassord () ) {
            user_rec->setAction ( ACTION_EDIT );
            doActualInsertOrEdit ( usr_info );
        }
    }
}

void UserManagement::removeUser ( USER_INFO* usr_info )
{
    if ( loadUserInfo ( usr_info->name, usr_info ) ) {
		VM_NOTIFY ()->questionBox ( QCoreApplication::tr ( "Remove user" ), QCoreApplication::tr ( "Really remove user " )
									+ usr_info->name + CHR_QUESTION_MARK );
        if ( askForAdminPassord () ) {
            user_rec->setAction ( ACTION_DEL );
            user_rec->deleteRecord ();

        }
    }
}
