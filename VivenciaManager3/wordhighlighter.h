#ifndef WORDHIGHLIGHTER_H
#define WORDHIGHLIGHTER_H

#undef QT_NO_CAST_FROM_ASCII
#undef QT_NO_CAST_TO_ASCII

#include <QSyntaxHighlighter>
#include <QStringList>

class QTextCharFormat;
class QTextDocument;

class wordHighlighter : public QSyntaxHighlighter
{

public:
    explicit wordHighlighter ( QTextDocument* parent );
	virtual ~wordHighlighter ();

	inline bool spellCheckingEnbled () const { return mb_spellCheckEnabled; }
    void enableSpellChecking ( const bool state );

	inline bool highlightingEnabled () const { return mb_HighlightEnabled; }
    void enableHighlighting ( const bool );

    void unHighlightWord ( const QString& );
    void highlightWord ( const QString& );
    void highlightWords ( const QStringList& );

protected:
    void highlightBlock ( const QString& );

private:
    QStringList highlightedWordsList;
	bool mb_spellCheckEnabled, mb_HighlightEnabled;

    static QTextCharFormat* m_spellCheckFormat, *m_HighlightFormat;
};

#endif // WORDHIGHLIGHTER_H
