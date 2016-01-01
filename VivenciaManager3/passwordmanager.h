#ifndef PASSWORDMANAGER_H
#define PASSWORDMANAGER_H

#include <QHash>
#include <QString>

class QFile;
class QWidget;

class passwordManager
{

public:

	enum PassWdSaveState {
		PWSS_DO_NOT_SAVE = 0, PWSS_SAVE_TEMP = 1, PWSS_SAVE = 2 };

	explicit passwordManager ( const PassWdSaveState s_state = PWSS_DO_NOT_SAVE );
	~passwordManager ();

	bool remove ( const QString& id );
	bool change ( const QString& id, const QString& oldpasswd, const QString& newpasswd );
	bool exists ( const QString& id ) const;
	QString getPassword ( const QString& id ) const;
	QString getID ( const uint pos = 0 ) const;
	inline void clear () { mPairs.clear (); }

	QString toString ( const QString& id = QString::null ) const;
	bool fromString ( const QString& hash_str, const bool overwrite = false );

	bool load ();
	bool toFile ( QFile* file, const QString& id = QString::null ) const;
	bool fromFile ( QFile* const file, const bool overwrite = false );

	bool sudoPassword ( QString& r_passwd, const QString& message );
	bool askPassword ( QString& passwd, const QString& id, const QString& message );

private:
	QHash<QString,QString> mPairs;
	PassWdSaveState ms_state;
	QString mFilename;
	QFile* mFile;
	bool mbUpToDate;

	bool insert ( const QString& id, const QString& password );
};

#endif // PASSWORDMANAGER_H
