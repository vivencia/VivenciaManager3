#include "spellcheck.h"
#include "global.h"
#include "configops.h"
#include "fileops.h"
#include "heapmanager.h"
#include "vmwidgets.h"
#include "system_init.h"

#include <QTextCodec>
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
	: mChecker ( nullptr ), mMenu ( nullptr ), menuEntrySelected_func ( nullptr )
{
	getDictionariesPath ();
	setDictionaryLanguage ( CONFIG ()->readConfigFile ( -1, CONFIG_CATEGORY_NAME ) );
	createDictionaryInterface ();
	setUserDictionary ();
	Sys_Init::addPostRoutine ( deleteSpellCheckInstance );
}

spellCheck::~spellCheck ()
{
	heap_del ( mChecker );
	heap_del ( mMenu );
}

void spellCheck::updateUserDict ()
{
	QFile file ( mUserDict );
	if ( file.open ( QIODevice::WriteOnly | QIODevice::Text ) )
	{
		bool ok ( false );
		unknownWordList.at ( 0 ).toInt ( &ok );
		if ( !ok )
			unknownWordList.prepend ( QString::number ( unknownWordList.count () + 2 ) );
		else
			unknownWordList.replace ( 0, QString::number ( unknownWordList.count () + 1 ) );

		QTextStream out ( &file );
		QStringList::const_iterator itr ( unknownWordList.constBegin () );
		const QStringList::const_iterator itr_end ( unknownWordList.constEnd () );
		while ( itr != itr_end )
		{
			out << static_cast<QString> ( *itr ) << CHR_NEWLINE;
			++itr;
		}
		file.close ();
	}
}

/* The dictionaries files (.dic and .aff) are encoded in Western Europe's ISO-8859-15. Using
 * QString's UTF-8, local8Bit and to or from StdString(which assumes std::string::data () to be in UTF-8 format)
 * results in misinterpreted characters. Again, this behavior of QString started showing after the upgrade to Qt
 * 5.9.1, the same version that prompeted errors elsewhere and had me create the QSTRING_ENCODING_FIX to overcome
 * those errors
 */
bool spellCheck::suggestionsList ( const QString& word, QStringList& wordList )
{
	if ( checkWord ( word ).isFalse () )
	{
		std::vector<std::string> suggestList ( mChecker->suggest ( word.toLatin1 ().constData () ) );
		if ( suggestList.size () > 0 )
		{
			for ( size_t i ( 0 ); i < suggestList.size (); ++i )
				wordList.append ( QString::fromLatin1 ( suggestList.at ( i ).data () ) );
			suggestList.clear ();
			return true;
		}
	}
	return false;
}

void spellCheck::addWord ( const QString& word, const bool b_add )
{
	if ( mChecker )
	{
		mChecker->add ( word.toLocal8Bit ().toStdString () );
		if ( b_add )
		{
			unknownWordList.append ( word );
			updateUserDict ();
		}
	}
}

QMenu* spellCheck::menuAvailableDicts ()
{
	if ( mMenu == nullptr )
	{
		// QMenu::addMenu () for some reason crashes if we pass a null pointer. So, regardless of having
		// any dictionary on the system, we must create a menu to pass to it. A bug, in my opinion.
		mMenu = new QMenu ( APP_TR_FUNC ( "Choose spell language" ) );
		
		PointersList<fileOps::st_fileInfo*> dics;
		fileOps::lsDir ( dics, mDicPath, QStringList () << QStringLiteral ( ".dic" ), QStringList (), fileOps::LS_FILES );
		if ( !dics.isEmpty () )
		{
			QAction* qaction ( nullptr );
			QString menuText;	
			mMenu->connect ( mMenu, &QMenu::triggered, this, [&] ( QAction* action ) {
				return menuEntrySelected ( action ); } );
			qaction = new vmAction ( -1, APP_TR_FUNC ( "Disable spell checking" ) );
			mMenu->addAction ( qaction );
			mMenu->addSeparator ();
			for ( uint i ( 0 ); i < dics.count (); ++i )
			{
				menuText = dics.at ( i )->filename;
				menuText.chop ( 4 ); // remove ".dic"
				qaction = new vmAction ( static_cast<int>(i), menuText );
				mMenu->addAction ( qaction );
			}
		}
	}
	return mMenu;
}

void spellCheck::menuEntrySelected ( const QAction* __restrict action )
{
	if ( action->text ().startsWith ( QStringLiteral ( "Disab" ) ) )
		heap_del ( mChecker );
	else
	{
		setDictionaryLanguage ( action->text () );
		createDictionaryInterface ();
	}
	if ( menuEntrySelected_func )
		menuEntrySelected_func ( mChecker != nullptr );
}

void spellCheck::getDictionariesPath ()
{
	if ( fileOps::exists ( QStringLiteral ( "/etc/lsb-release" ) ).isOn () ) // *buntu derivatives
		mDicPath = QStringLiteral ( "/usr/share/hunspell/" );
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
	if ( fileOps::canRead ( mDictionary ).isOn () )
	{
		heap_del ( mChecker );
		QString dicAff;
		getDictionaryAff ( dicAff );
		mChecker = new Hunspell ( dicAff.toLatin1 ().constData (), mDictionary.toLatin1 ().constData () );
		QTextCodec::setCodecForLocale ( QTextCodec::codecForName ( mChecker->get_dic_encoding () ) );
	}
}

bool spellCheck::setUserDictionary ()
{
	mUserDict = CONFIG ()->appDataDir () + QLatin1String ( "User_" ) + fileOps::fileNameWithoutPath ( mDictionary );
	QFile file ( mUserDict );
	if ( file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
	{
		QTextStream in ( &file );
		in.readLine (); //skip word count
		while ( !in.atEnd () )
			unknownWordList.append ( in.readLine () );
		file.close ();
		return ( mChecker->add_dic ( mUserDict.toUtf8 ().constData () ) == 0 );
	}
	return false;
}
