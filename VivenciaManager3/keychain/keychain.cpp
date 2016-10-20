/******************************************************************************
 *   Copyright (C) 2011-2015 Frank Osterfeld <frank.osterfeld@gmail.com>	  *
 *																			*
 * This program is distributed in the hope that it will be useful, but		*
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY *
 * or FITNESS FOR A PARTICULAR PURPOSE. For licensing and distribution		*
 * details, check the accompanying file 'COPYING'.							*
 *****************************************************************************/

#include "global.h"
#include "keychain.h"
#include "keychain_p.h"
#include "vmnotify.h"

#include <QEventLoop>

#include <iostream>

using namespace vmPasswordManager;

passwdManager::passwdManager ( passwdManagerPrivate* q )
	: d ( q ), finishedFunc ( nullptr ), destroyedFunc ( nullptr )
{}

passwdManager::~passwdManager ()
{
	delete d;
}

QString passwdManager::service () const
{
	return d->service;
}

QSettings* passwdManager::settings () const 
{
	return d->m_settings;
}

void passwdManager::setSettings ( QSettings* settings ) 
{
	d->m_settings = settings;
}

void passwdManager::start ()
{
	QEventLoop loop;
	setCallbackForFinished ( [&loop] ( vmPasswordManager::passwdManager* ) { return loop.quit (); } );
	d->startPwdProcess ();
	loop.exec ();
}

bool passwdManager::autoDelete () const
{
	return d->autoDelete;
}

void passwdManager::setAutoDelete ( bool autoDelete )
{
	d->autoDelete = autoDelete;
}

bool passwdManager::insecureFallback () const
{
	return d->insecureFallback;
}

void passwdManager::setInsecureFallback ( bool insecureFallback )
{
	d->insecureFallback = insecureFallback;
}

void passwdManager::emitFinished ()
{
	if ( finishedFunc )
		finishedFunc ( this );
	if ( d->autoDelete )
		d->deleteLater ();
}

void passwdManager::emitFinishedWithError ( Error error, const QString& errorString )
{
	d->error = error;
	d->errorString = errorString;
	std::cerr << "Password/keychain error (" << qPrintable ( QString::number ( static_cast<int>( error ) ) )
			  << "): " << qPrintable ( errorString ) << std::endl;
	emitFinished ();
}

Error passwdManager::error () const
{
	return d->error;
}

QString passwdManager::errorString () const
{
	return d->errorString;
}

void passwdManager::setError( Error error )
{
	d->error = error;
}

void passwdManager::setErrorString ( const QString& errorString )
{
	d->errorString = errorString;
}

passwdManager_Read::passwdManager_Read ( const QString& service )
	: passwdManager ( new passwdManagerPrivate_Read ( service, this ) )
{}

passwdManager_Read::~passwdManager_Read ()
{}

QString passwdManager_Read::textData () const
{
	return QString::fromUtf8 ( d->data );
}

QByteArray passwdManager_Read::binaryData () const
{
	return d->data;
}

QString passwdManager::key () const
{
	return d->key;
}

void passwdManager::setKey ( const QString& key_ )
{
	d->key = key_;
}

passwdManager_Write::passwdManager_Write( const QString& service )
	: passwdManager ( new passwdManagerPrivate_Write ( service, this ) )
{}

passwdManager_Write::~passwdManager_Write ()
{}

void passwdManager_Write::setBinaryData ( const QByteArray& data ) {
	d->data = data;
	d->mode = passwdManagerPrivate::Binary;
}

void passwdManager_Write::setTextData ( const QString& data )
{
	d->data = data.toUtf8 ();
	d->mode = passwdManagerPrivate::Text;
}

passwdManager_Delete::passwdManager_Delete ( const QString& service )
	: passwdManager ( new passwdManagerPrivate_Delete ( service, this ) )
{}

passwdManager_Delete::~passwdManager_Delete ()
{}

passwdManagerPrivate_Delete::passwdManagerPrivate_Delete ( const QString& service_, passwdManager_Delete* qq )
	: passwdManagerPrivate ( service_, qq )
{}

passwdManagerPrivate_Read::passwdManagerPrivate_Read (const QString& service_, passwdManager_Read* qq )
	: passwdManagerPrivate ( service_, qq )
{}

passwdManagerPrivate::passwdManagerPrivate ( const QString& service_, passwdManager* qq )
	: error ( NoError )
	, service ( service_ )
	, autoDelete ( true )
	, insecureFallback ( false )
	, m_settings ( nullptr )
	, q ( qq )
{}

QString passwdManagerPrivate::modeToString ( Mode m )
{
	return m == Text ? QStringLiteral ( "Text" ) : QStringLiteral ( "Binary" );
}

passwdManagerPrivate::Mode passwdManagerPrivate::stringToMode ( const QString& s )
{
	if ( s == QStringLiteral ( "Binary" ) || s == CHR_TWO )
		return Binary;
	else
		return Text;
}

passwdManagerPrivate_Write::passwdManagerPrivate_Write( const QString& service_, passwdManager_Write* qq )
	: passwdManagerPrivate ( service_, qq )
{}
