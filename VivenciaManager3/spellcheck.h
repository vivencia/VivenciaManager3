#ifndef SPELLCHECK_H
#define SPELLCHECK_H

#include "global.h"
#include "tristatetype.h"

#include <QObject>
#include <QStringList>
#include <QTextCodec>

#include <functional>
#include <hunspell/hunspell.hxx>

class QMenu;

class spellCheck : public QObject
{

public:
    ~spellCheck ();

    static void init () {
        if ( !s_instance )
            s_instance = new spellCheck;
    }

    void updateUserDict ();
    bool suggestionsList ( const QString& word, QStringList& wordList );
    void addWord ( const QString& word, const bool b_add );

    inline triStateType checkWord ( const QString& word )
    {
        return ( mChecker ? mChecker->spell ( mCodec->fromUnicode ( word ).constData () ) : TRI_UNDEF );
    }

    inline void setCallbackForMenuEntrySelected ( std::function<void ( const bool )> func ) {
        menuEntrySelected_func = func; }

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

    Hunspell* mChecker;
    QTextCodec* mCodec;
    QMenu* mMenu;

    std::function<void ( const bool )> menuEntrySelected_func;
};

inline spellCheck* SPELLCHECKER ()
{
    return spellCheck::s_instance;
}

#endif // SPELLCHECK_H
