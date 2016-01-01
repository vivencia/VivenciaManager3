#include "wordhighlighter.h"
#include "spellcheck.h"
#include "heapmanager.h"

#undef QT_NO_CAST_FROM_ASCII
#undef QT_NO_CAST_TO_ASCII

#include <QTextDocument>
#include <QTextCharFormat>
#include <QStringMatcher>
#include <QRegExp>

static const QRegExp word_split_syntax ( QStringLiteral ( "([^\\w,^\\\\]|(?=\\\\))+" ) );

QTextCharFormat* wordHighlighter::m_spellCheckFormat ( nullptr );
QTextCharFormat* wordHighlighter::m_HighlightFormat ( nullptr );

wordHighlighter::wordHighlighter ( QTextDocument* parent )
	: QSyntaxHighlighter ( parent ), m_bspellCheckEnabled ( false ), m_bHighlightEnabled ( false )
{}

wordHighlighter::~wordHighlighter ()
{
	heap_del ( m_spellCheckFormat );
	heap_del ( m_HighlightFormat );
}

void wordHighlighter::enableHighlighting ( const bool enable )
{
	if ( enable ) {
		if ( !m_bHighlightEnabled && !m_HighlightFormat ) {
			m_HighlightFormat = new QTextCharFormat;
			m_HighlightFormat->setBackground ( Qt::yellow );
			m_HighlightFormat->setUnderlineStyle ( QTextCharFormat::WaveUnderline );
		}
	}
	m_bHighlightEnabled = enable;
	rehighlight ();
}

void wordHighlighter::enableSpellChecking ( const bool enable )
{
	if ( enable ) {
		if ( !m_bspellCheckEnabled && !m_spellCheckFormat ) {
			m_spellCheckFormat = new QTextCharFormat;
			m_spellCheckFormat->setForeground ( Qt::red );
			m_spellCheckFormat->setUnderlineColor ( QColor ( Qt::red ) );
			m_spellCheckFormat->setUnderlineStyle ( QTextCharFormat::SpellCheckUnderline );
		}
	}
	m_bspellCheckEnabled = enable;
	rehighlight ();
}

void wordHighlighter::unHighlightWord ( const QString& word )
{
	const int i ( highlightedWordsList.indexOf ( word ) );
	if ( i != -1 ) {
		highlightedWordsList.removeAt ( i );
		rehighlight ();
	}
}

void wordHighlighter::highlightWord ( const QString& word )
{
	if ( !word.isEmpty () ) {
		highlightedWordsList.clear ();
		highlightedWordsList.append ( word );
		rehighlight ();
	}
}

void wordHighlighter::highlightWords ( const QStringList &words )
{
	if ( !words.isEmpty () ) {
		highlightedWordsList = words;
		rehighlight ();
	}
}

void wordHighlighter::highlightBlock ( const QString& text )
{
	if ( m_bHighlightEnabled | m_bspellCheckEnabled ) {
		if ( objectName () == QStringLiteral ( "p" ) ) // In preview mode. Disable any highlight
			return;

		// split text into words
		QString str ( text.simplified () );
		if ( !str.isEmpty () ) {
			QStringMatcher str_match;
			str_match.setCaseSensitivity ( Qt::CaseInsensitive );
			//const QStringList wordsInBlock ( str.split ( word_split_syntax, QString::SkipEmptyParts ) );
			const QStringList wordsInBlock ( str.split ( CHR_SPACE, QString::SkipEmptyParts ) );

			int l ( -1 ), number ( -1 );
			int i ( 0 ), j ( 0 ), x ( 0 );
			bool proceed_highlight ( false );

			// check all words
			for ( ; i < wordsInBlock.size (); ++i ) {
				str = wordsInBlock.at ( i );
				if ( str.length () > 1 ) {
					if ( m_bspellCheckEnabled ) {
						if ( SPELLCHECKER ()->checkWord ( str ).isFalse () ) {
							number = text.count ( QRegExp ( QLatin1Literal ("\\b" ) + str + QLatin1Literal ("\\b" ) ) );
							// underline all incorrect occurences of misspelled word
							str_match.setPattern ( str );
							for ( j = 0; j < number; ++j ) {
								l = str_match.indexIn ( text, l + 1 );
								if ( l >= 0 )
									setFormat ( l, str.length (), *m_spellCheckFormat );
							} // for j
						}
					}
					if ( m_bHighlightEnabled ) {
						for ( x = 0; x < highlightedWordsList.count (); ++x ) {
							// The highlighting is not done by a word-by-word basis, instead, it's by
							// characters. We might want do find only a part of a word, or even a letter
							number = highlightedWordsList.at ( x ).count ();
							str_match.setPattern ( highlightedWordsList.at ( x ) );
							if ( str.count () >= number )
								proceed_highlight = str_match.indexIn ( text );
							else // The word in the text cannot be smaller than the word ( or partial word ) we a looking for
								proceed_highlight = false;

							if ( proceed_highlight ) {
								for ( j = 0; j < number; ++j ) {
									l = str_match.indexIn ( text, l + 1 );
									if ( l >= 0 )
										setFormat ( l, number, *m_HighlightFormat );
								}
							}
						}
					}
				}
			}
		}
	}
}
