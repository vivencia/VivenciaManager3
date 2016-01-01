#include "spellcheck.h"
#include "global.h"
#include "configops.h"
#include "fileops.h"
#include "heapmanager.h"
#include "vmwidgets.h"
#include "cleanup.h"

#include <QTextStream>
#include <QFile>
#include <QLocale>
#include <QMenu>
#include <QByteArray>
#include <QTextCodec>

#define CONFIG_CATEGORY_NAME QStringLiteral ( "SPELL_LANGUAGE" )

spellCheck* spellCheck::s_instance ( nullptr );

void deleteSpellCheckInstance ()
{
	heap_del ( spellCheck::s_instance );
}

spellCheck::spellCheck ()
    : mChecker ( nullptr ), mCodec ( nullptr ), mMenu ( nullptr ), menuEntrySelected_func ( nullptr )
{
    getDictionariesPath ();
    setDictionaryLanguage ( CONFIG ()->readConfigFile ( -1, CONFIG_CATEGORY_NAME ) );
    createDictionaryInterface ();
    setUserDictionary ();
	addPostRoutine ( deleteSpellCheckInstance );
}

spellCheck::~spellCheck ()
{
	heap_del ( mChecker );
    heap_del ( mMenu );
}

void spellCheck::updateUserDict ()
{
    QFile file ( mUserDict );
	if ( file.open ( QIODevice::WriteOnly | QIODevice::Text ) ) {
		bool ok ( false );
		unknownWordList.at ( 0 ).toInt ( &ok );
		if ( !ok )
			unknownWordList.prepend ( QString::number ( unknownWordList.count () + 2 ) );
		else
			unknownWordList.replace ( 0, QString::number ( unknownWordList.count () + 1 ) );

		QTextStream out ( &file );
		QStringList::const_iterator itr = unknownWordList.constBegin ();
		const QStringList::const_iterator itr_end = unknownWordList.constEnd ();
		while ( itr != itr_end ) {
			out << static_cast<QString> ( *itr ) << CHR_NEWLINE;
			++itr;
		}
		file.close ();
	}
}

bool spellCheck::suggestionsList ( const QString& word, QStringList& wordList )
{
    if ( checkWord ( word ).isFalse () ) {
        char** wlst ( nullptr );
        const int ns ( mChecker->suggest ( &wlst, mCodec->fromUnicode ( word ).constData () ) );
        if ( ns > 0 ) {
            for ( int i = 0; i < ns; ++i )
                wordList.append ( mCodec->toUnicode ( wlst[i] ) );
            mChecker->free_list ( &wlst, ns );
            return true;
        }
    }
    return false;
}

void spellCheck::addWord ( const QString& word, const bool b_add )
{
    if ( mChecker ) {
        mChecker->add ( word.toLocal8Bit ().constData () );
        if ( b_add ) {
            unknownWordList.append ( word );
            updateUserDict ();
        }
    }
}

QMenu* spellCheck::menuAvailableDicts ()
{
    if ( mMenu == nullptr ) {
        PointersList<fileOps::st_fileInfo*> dics;
        fileOps::lsDir ( dics, mDicPath, QStringList () << QStringLiteral ( ".dic" ), fileOps::LS_FILES );
        if ( !dics.isEmpty () ) {
            QAction* qaction ( nullptr );
            QString menuText;

            mMenu = new QMenu ( APP_TR_FUNC ( "Choose spell language" ) );
            mMenu->connect ( mMenu, &QMenu::triggered, this, [&] ( QAction* action ) {
                return menuEntrySelected ( action ); } );
            qaction = new vmAction ( -1, APP_TR_FUNC ( "Disable spell checking" ) );
            mMenu->addAction ( qaction );
            mMenu->addSeparator ();
            for ( uint i ( 0 ); i < dics.count (); ++i ) {
                menuText = dics.at ( i )->filename;
                //menuText.truncate ( menuText.length () - 4 ); // remove .dic
                menuText.chop ( 4 ); // remove ".dic"
                qaction = new vmAction ( i, menuText );
                mMenu->addAction ( qaction );
            }
        }
    }
    return mMenu;
}

void spellCheck::menuEntrySelected ( const QAction* action )
{
    if ( action->text ().startsWith ( QStringLiteral ( "Disab" ) ) ) {
        heap_del ( mChecker );
        mChecker = nullptr;
    }
    else {
        setDictionaryLanguage ( action->text () );
        createDictionaryInterface ();
    }
    if ( menuEntrySelected_func )
        menuEntrySelected_func ( mChecker != nullptr );
}

void spellCheck::getDictionariesPath ()
{
	if ( configOps::isSystem ( UBUNTU ) )
        mDicPath = QStringLiteral ( "/usr/share/hunspell/" );
	else if ( configOps::isSystem ( OPENSUSE ) )
        mDicPath = QStringLiteral ( "/usr/share/myspell/" );
	else
        mDicPath = QStringLiteral ( "/usr/share/myspell/dicts/" );
}

inline void spellCheck::setDictionaryLanguage ( const QString& localeString )
{
    const QString locale ( localeString.isEmpty() ? QLocale::system ().name () : localeString );
    mDictionary = mDicPath + locale + QLatin1String ( ".dic" );
    CONFIG ()->writeConfigFile ( -1, locale, CONFIG_CATEGORY_NAME );
}

#define AFF_STR_LEN 4
inline void spellCheck::getDictionaryAff ( QString& dicAff ) const
{
    dicAff = mDictionary.left ( mDictionary.length () - AFF_STR_LEN ) + QLatin1String ( ".aff" );
}

void spellCheck::createDictionaryInterface ()
{
    if ( fileOps::canRead ( mDictionary ).isOn () ) {
        heap_del ( mChecker );
        QString dicAff;
        getDictionaryAff( dicAff );
        mChecker = new Hunspell ( dicAff.toLatin1 ().constData (), mDictionary.toLatin1 ().constData () );
        mCodec = QTextCodec::codecForName ( mChecker->get_dic_encoding () );
    }
}

bool spellCheck::setUserDictionary ()
{
    mUserDict = CONFIG ()->appDataDir () + QLatin1String ( "User_" ) + fileOps::fileNameWithoutPath ( mDictionary );
    QFile file ( mUserDict );
    if ( file.open ( QIODevice::ReadOnly | QIODevice::Text ) ) {
        QTextStream in ( &file );
        in.readLine (); //skip word count
        while ( !in.atEnd () )
            unknownWordList.append ( in.readLine () );
        file.close ();
        return ( mChecker->add_dic ( mUserDict.toLatin1 ().constData () ) == 0 );
    }
    return false;
}
