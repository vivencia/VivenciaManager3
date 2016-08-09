/******************************************************************************
 *   Copyright (C) 2011-2015 Frank Osterfeld <frank.osterfeld@gmail.com>	  *
 *																			*
 * This program is distributed in the hope that it will be useful, but		*
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY *
 * or FITNESS FOR A PARTICULAR PURPOSE. For licensing and distribution		*
 * details, check the accompanying file 'COPYING'.							*
 *****************************************************************************/
#ifndef KEYCHAIN_P_H
#define KEYCHAIN_P_H

#include "keychain.h"
#include "kwallet_interface.h"

#include <QCoreApplication>
#include <QObject>
#include <QSettings>
#include <QtDBus/QDBusPendingCallWatcher>

namespace vmPasswordManager
{

class passwdManagerPrivate : public QObject
{

friend class passwdManager;
friend class passwdManager_Read;
friend class passwdManager_Write;
	
public:
	enum Mode {
		Text,
		Binary
	};

	inline virtual ~passwdManagerPrivate () {}
	virtual void startPwdProcess() = 0;

	static QString modeToString ( Mode m );
	static Mode stringToMode ( const QString& s );

	Mode mode;

	org::kde::KWallet* iface;
	int walletHandle;

	static void gnomeKeyring_readCb ( int result, const char* string, passwdManagerPrivate* data );
	static void gnomeKeyring_writeCb ( int result, passwdManagerPrivate* self );

	virtual void fallbackOnError ( const QDBusError& err ) = 0;

	void kwalletWalletFound ( QDBusPendingCallWatcher* watcher );
	virtual void kwalletFinished ( QDBusPendingCallWatcher* watcher );
	virtual void kwalletOpenFinished ( QDBusPendingCallWatcher* watcher );

protected:
	passwdManagerPrivate ( const QString& service_, passwdManager* qq );

	vmPasswordManager::Error error;
	QString errorString;
	QString service;
	bool autoDelete;
	bool insecureFallback;
	QSettings* m_settings;
	QString key;
	passwdManager* const q;
	QByteArray data;
};

class passwdManagerPrivate_Read : public passwdManagerPrivate
{

friend class passwdManager_Read;

public:
	explicit passwdManagerPrivate_Read( const QString& service_, passwdManager_Read* qq );
	virtual ~passwdManagerPrivate_Read () {}

	void startPwdProcess ();
	void fallbackOnError ( const QDBusError& err );

private:
	void kwalletOpenFinished ( QDBusPendingCallWatcher* watcher );
	void kwalletEntryTypeFinished ( QDBusPendingCallWatcher* watcher );
	void kwalletFinished ( QDBusPendingCallWatcher* watcher );
};

class passwdManagerPrivate_Write : public passwdManagerPrivate
{

friend class passwdManager_Write;

public:
	explicit passwdManagerPrivate_Write( const QString &service_, passwdManager_Write* qq );
	virtual ~passwdManagerPrivate_Write () {}
	
	void startPwdProcess ();
	void fallbackOnError ( const QDBusError& err );
};

class passwdManagerPrivate_Delete : public passwdManagerPrivate
{

friend class passwdManager_Delete;

public:
	explicit passwdManagerPrivate_Delete( const QString& service_, passwdManager_Delete* qq );
	virtual ~passwdManagerPrivate_Delete () {}

	void startPwdProcess ();
	void fallbackOnError ( const QDBusError& err );

protected:
	void doStart ();
};

}

#endif // KEYCHAIN_P_H
