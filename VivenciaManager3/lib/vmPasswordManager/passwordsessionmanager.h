#ifndef PASSWORDSESSIONMANAGER_H
#define PASSWORDSESSIONMANAGER_H

#include "global.h"
#include "keychain/keychain.h"

#include <QHash>

class passwordSessionManager
{

public:
	
	enum PSM_SAVE_OPTIONS {
		SaveSession,
		SavePermanment,
		DoNotSave
	};

	~passwordSessionManager ();
	bool getPassword ( QString& passwd, const QString& service, const QString& id );
	bool getPassword_UserInteraction ( QString& passwd, const QString& service, const QString& id, const QString& message );

	bool savePassword ( PSM_SAVE_OPTIONS save_option, const QString& passwd, const QString& service, const QString& id );
	
private:
	explicit passwordSessionManager ();
	static passwordSessionManager* s_instance;
	friend void deletepasswordSessionManagerInstance ();
	friend passwordSessionManager* APP_PSWD_MANAGER ();
	
	vmPasswordManager::Error err_code;
	QHash<QString,QString> tempPasswords;
};

inline passwordSessionManager* APP_PSWD_MANAGER ()
{
	if ( !passwordSessionManager::s_instance )
		passwordSessionManager::s_instance = new passwordSessionManager ();
	return passwordSessionManager::s_instance;
}

#endif // PASSWORDSESSIONMANAGER_H