#ifndef CRASHRESTORE_H
#define CRASHRESTORE_H

#include "tristatetype.h"
#include "stringrecord.h"

#include <QString>

class dataFile;

class crashRestore
{

public:
    explicit crashRestore ( const QString& str_id );
    ~crashRestore ();

    bool needRestore ();

    inline bool isRestoring () const {
        return crashInfoLoaded.isOn ();
    }
    inline void setInfoLoaded ( const bool bloaded ) {
        mbInfoLoaded = bloaded;
    }
    inline bool infoLoaded () const {
        return mbInfoLoaded;
    }

    int commitState ( const int id, const stringRecord& value );
    void eliminateRestoreInfo ( const int id = -1 );

    const stringRecord& restoreFirstState () const;
    const stringRecord& restoreNextState () const;
    const stringRecord& restorePrevState () const;
    const stringRecord& restoreLastState () const;

	static inline void setNewDBSession () { crashRestore::bNewDBSession = true; }

private:
	friend void cleanCrashInfo ();
    const stringRecord& restoreState () const;
    void done ();

    mutable int m_statepos;
    QString m_filename;
    dataFile* fileCrash;
    triStateType crashInfoLoaded;
    stringRecord m_stateinfo;
    bool mbInfoLoaded;

	static bool bNewDBSession;
};

#endif // CRASHRESTORE_H
