#include "global.h"
#include "crashrestore.h"
#include "configops.h"
#include "textdb.h"
#include "fileops.h"
#include "heapmanager.h"
#include "cleanup.h"
#include "mainwindow.h"

restoreManager* restoreManager::s_instance ( nullptr );

void deleteRestoreManagerInstance ()
{
	PointersList<fileOps::st_fileInfo*> temp_files;
	fileOps::lsDir ( temp_files, CONFIG ()->appDataDir (), QStringList () << QLatin1String ( ".crash" ) );
	for ( uint i ( 0 ); i < temp_files.count (); ++i )
	{
		if ( fileOps::fileSize ( temp_files.at ( i )->fullpath ) <= 0 )
			fileOps::removeFile ( temp_files.at ( i )->fullpath );
	}
	temp_files.clear ( true );
	heap_del ( restoreManager::s_instance );
}

restoreManager::restoreManager ()
	: crashInfoList ( 5 ), mb_newDBSession ( false )
{
	addPostRoutine ( deleteRestoreManagerInstance, true );
}

restoreManager::~restoreManager ()
{
	crashInfoList.clear ( true );
}

void restoreManager::saveSession ()
{
	qDebug () << "Saving session ... ";
	/* We do not know the nature of the exeption that led here. So we attempt to save the current state information and then
	 * resume with program exiting process which, if successfull, will undo the saving state but better be safe than sorry
	 */
	crashRestore* crash ( crashInfoList.first () );
	while ( crash )
	{
		crash->done ();
		crash = crashInfoList.next ();
	}
	globalMainWindow->exitRequested ( true );
	deleteRestoreManagerInstance ();
}

crashRestore::crashRestore ( const QString& str_id )
	: m_statepos ( -1 ), mbInfoLoaded ( false )
{
	m_filename = CONFIG ()->appDataDir () + str_id + QLatin1String ( ".crash" );
	fileCrash = new dataFile ( m_filename );
}

crashRestore::~crashRestore ()
{
	heap_del ( fileCrash );
}

void crashRestore::done ()
{
	fileCrash->clear ();
	crashInfoLoaded.setOff ();
	fileOps::removeFile ( m_filename );
}

bool crashRestore::needRestore ()
{
	if ( APP_RESTORER ()->newDBSession () )
		return false;
	
	bool ret ( false );
	if ( crashInfoLoaded.isUndefined () )
	{
		if ( fileOps::exists ( m_filename ).isOn () )
		{
			if ( fileCrash->open () )
			{
				if ( fileCrash->load ().isOn () )
					ret = ( fileCrash->recCount () > 0 );
			}
		}
		if ( ret )
			crashInfoLoaded.setOn ();
		else {
			crashInfoLoaded.setOff ();
			done ();
		}
	}
	else
		ret = crashInfoLoaded.isOn ();
	return ret;
}

int crashRestore::commitState ( const int id, const stringRecord& value )
{
	int ret ( id );
	if ( id == -1 )
	{
		ret = static_cast<int>(fileCrash->recCount ());
		fileCrash->appendRecord ( value );
	}
	else
		fileCrash->changeRecord ( id, value );
	fileCrash->commit ();
	if ( fileCrash->recCount () > 0 )
		crashInfoLoaded.setOn ();
	return ret;
}

void crashRestore::eliminateRestoreInfo ( const int id )
{
	if ( id != -1 )
		fileCrash->deleteRecord ( id );

	if ( ( id == -1 ) || ( fileCrash->recCount () == 0 ) )
		done ();
}

const stringRecord& crashRestore::restoreState () const
{
	if ( crashInfoLoaded.isOn () )
	{
		fileCrash->getRecord ( const_cast<crashRestore*> ( this )->m_stateinfo, m_statepos );
		return m_stateinfo;
	}
	else
		return emptyStrRecord;
}

const stringRecord& crashRestore::restoreFirstState () const
{
	m_statepos = 0;
	return restoreState ();
}

const stringRecord& crashRestore::restoreNextState () const
{
	if ( m_statepos < static_cast<int>(fileCrash->recCount () - 1 ))
	{
		++m_statepos;
		restoreState ();
		if ( m_stateinfo.isOK () )
		{
			const_cast<crashRestore*> ( this )->mbInfoLoaded = ( unsigned ( m_statepos ) == fileCrash->recCount () - 1 );
			return m_stateinfo;
		}
	}
	return emptyStrRecord;
}

const stringRecord& crashRestore::restorePrevState () const
{
	if ( m_statepos > 0 )
	{
		--m_statepos;
		return restoreState ();
	}
	return emptyStrRecord;
}

const stringRecord& crashRestore::restoreLastState () const
{
	m_statepos = static_cast<int>(fileCrash->recCount ());
	return restoreState ();
}
