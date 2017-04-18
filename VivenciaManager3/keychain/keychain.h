/******************************************************************************
 *   Copyright (C) 2011-2015 Frank Osterfeld <frank.osterfeld@gmail.com>	  *
 *																			*
 * This program is distributed in the hope that it will be useful, but		*
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY *
 * or FITNESS FOR A PARTICULAR PURPOSE. For licensing and distribution		*
 * details, check the accompanying file 'COPYING'.							*
 *****************************************************************************/
#ifndef KEYCHAIN_H
#define KEYCHAIN_H

#include <QObject>
#include <QtCore/QString>

#include <functional>

class QSettings;

namespace vmPasswordManager
{

/**
 * Error codes
 */
enum Error {
	NoError=0, /**< No error occurred, operation was successful */
	EntryNotFound, /**< For the given key no data was found */
	CouldNotDeleteEntry, /**< Could not delete existing secret data */
	AccessDeniedByUser, /**< User denied access to keychain */
	AccessDenied, /**< Access denied for other reasons */
	NoBackendAvailable, /**< No platform-specific keychain service available */
	NotImplemented, /**< Not implemented on platform */
	OtherError /**< Something else went wrong (errorString() might provide details) */
};

class passwdManagerPrivate;

class passwdManager : public QObject
{

friend class passwdManagerPrivate;
friend class passwdManagerPrivate_Read;
friend class passwdManagerPrivate_Write;
friend class passwdManagerPrivate_Delete;
	
public:	
	~passwdManager ();

	QSettings* settings () const;
	void setSettings ( QSettings* settings );

	void start (); 

	QString service () const;

	Error error () const;
	QString errorString () const;

	bool autoDelete () const;
	void setAutoDelete ( bool autoDelete );

	bool insecureFallback () const;
	void setInsecureFallback ( bool insecureFallback );

	QString key () const;
	void setKey ( const QString& key );
	
	inline void setCallbackForFinished ( const std::function<void ( vmPasswordManager::passwdManager* )>& func )
		{ finishedFunc = func; }
	inline void setCallbackForDestroyed ( const std::function<void ( vmPasswordManager::passwdManager* )>& func )
		{ destroyedFunc = func; }

protected:
	explicit passwdManager ( passwdManagerPrivate* q );

private:
	void setError ( Error error );
	void setErrorString ( const QString& errorString );
	void emitFinished ();
	void emitFinishedWithError ( Error, const QString& errorString );

protected:
	passwdManagerPrivate* const d;
	std::function<void ( vmPasswordManager::passwdManager* )> finishedFunc;
	std::function<void ( vmPasswordManager::passwdManager* )> destroyedFunc;
};

class passwdManagerPrivate_Read;

class passwdManager_Read : public passwdManager
{

public:
	explicit passwdManager_Read ( const QString& service );
	~passwdManager_Read ();

	QByteArray binaryData () const;
	QString textData () const;

private:
	friend class vmPasswordManager::passwdManagerPrivate_Read;
};

class passwdManagerPrivate_Write;

class passwdManager_Write : public passwdManager
{
	
public:
	explicit passwdManager_Write ( const QString& service );
	~passwdManager_Write ();

	void setBinaryData ( const QByteArray& data );
	void setTextData ( const QString& data );

private:

	friend class vmPasswordManager::passwdManagerPrivate_Write;
};

class passwdManagerPrivate_Delete;

class passwdManager_Delete : public passwdManager
{
	
public:
	explicit passwdManager_Delete ( const QString& service );
	~passwdManager_Delete ();

private:
	friend class vmPasswordManager::passwdManagerPrivate_Delete;
};

} // namespace vmPasswordManager

#endif
