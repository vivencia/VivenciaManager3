#ifndef SPELLCHECK_H
#define SPELLCHECK_H

#include "global.h"
#include "tristatetype.h"

#include <QObject>
#include <QStringList>

#include <functional>
#include <hunspell/hunspell.hxx>

class QMenu;

class spellCheck : public QObject
{

public:
	virtual ~spellCheck ();

	static void init () {
		if ( !s_instance )
			s_instance = new spellCheck;
	}

	void updateUserDict ();
	bool suggestionsList ( const QString& word, QStringList& wordList );
	void addWord ( const QString& word, const bool b_add );

	inline triStateType checkWord ( const QString& word )
	{
		return ( mChecker ? static_cast<TRI_STATE>( mChecker->spell ( word.toLatin1 ().toStdString () ) ) : TRI_UNDEF );
	}

	inline void setCallbackForMenuEntrySelected ( const std::function<void ( const bool )>& func ) { menuEntrySelected_func = func; }

	QMenu* menuAvailableDicts ();

private:
	spellCheck ();

	friend spellCheck* SPELLCHECKER ();
	friend void deleteSpellCheckInstance ();
	static spellCheck* s_instance;

	void menuEntrySelected ( const QAction* action );
	void getDictionariesPath ();
	void setDictionaryLanguage ( const QString& localeString );
	void getDictionaryAff ( QString& dicAff ) const;
	void createDictionaryInterface ();
	bool setUserDictionary ();

	QStringList unknownWordList;
	QString mUserDict;
	QString mDicPath;
	QString mDictionary;

	Hunspell* __restrict mChecker;
	QMenu* mMenu;

	std::function<void ( const bool )> menuEntrySelected_func;
};

inline spellCheck* SPELLCHECKER ()
{
	return spellCheck::s_instance;
}

#endif // SPELLCHECK_H
