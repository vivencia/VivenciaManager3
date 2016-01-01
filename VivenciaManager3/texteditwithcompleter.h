#ifndef TEXTEDITWITHCOMPLETER_H
#define TEXTEDITWITHCOMPLETER_H

#include "vmwidget.h"

#include <QTextEdit>

class searchWordPanel;
class separateWindow;
class wordHighlighter;
class vmLineEdit;

class QCompleter;
class QGridLayout;
class QToolButton;
class QMenu;
class QAction;

//------------------------------------------------TEXT-EDIT-COMPLETER--------------------------------------------

const uint WRONG_WORDS_MENUS ( 8 );

class textEditWithCompleter : public QTextEdit, public vmWidget
{

friend class searchWordPanel;

public:
	explicit textEditWithCompleter ( QWidget* parent = nullptr );
    virtual ~textEditWithCompleter ();

	void setEditable ( const bool editable );
	void setPreview ( const bool preview );
	inline bool inPreview () const {
		return document ()->objectName () == QLatin1String ( "np" );
	}

	void showHideSearchPanel ();
	void createContextMenu ();
	void insertCustomActionToContextMenu ( QAction* action );

	QString paragraphText () const;
	void setText ( const QString& text, const bool b_notify = false );
	inline const QString currentText () const {
		return newest_edited_text;
	}

	inline const QString& searchTerm () const {
		return mSearchTerm;
	}
	void replaceWord ( const int cursor_pos, const QString& word );
	void selectFound ();
	bool searchStart ( const QString& searchTerm );
	void searchCancel ();
	bool searchFirst ();
	bool searchNext ();
	bool searchPrev ();
	bool searchLast ();

	inline static QString defaultBackgroundColor () {
		return textEditWithCompleter::defaultBGColor;
	}

	inline QLatin1String qtClassName () const {
		return QLatin1String ( "QTextEdit" );
	}
	QString defaultStyleSheet () const;
	void highlight ( const VMColors wm_color, const QString& str = QString::null );

protected:
	void keyPressEvent ( QKeyEvent* e );
	void focusInEvent ( QFocusEvent* e );
	void focusOutEvent ( QFocusEvent* e );
	void mousePressEvent ( QMouseEvent* e );
	void contextMenuEvent ( QContextMenuEvent* e );

private:
	int mFirstInsertedActionPos;
	QString spell_dic;
	QString m_wordUnderCursor;
	QString mHighlightedWord;
	QString newest_edited_text;
	QPoint lastPos;
	QString mSearchTerm;
	int mCursorPos;

	QAction* misspelledWordsActs[WRONG_WORDS_MENUS];
	separateWindow* sepWin;
	QCompleter* mCompleter;
	wordHighlighter* m_highlighter;
	searchWordPanel* m_searchPanel;
	QMenu* mContextMenu;

	static QString defaultBGColor;

	QString textUnderCursor () const;
    void insertCompletion ( const QString& completion, QCompleter* completer );
    void correctWord ( const QAction* action );
	void addWord ();
	void ignoreWord ();
};
//------------------------------------------------TEXT-EDIT-COMPLETER--------------------------------------------

//------------------------------------------------FIND-WORD-PANEL--------------------------------------------

class searchWordPanel : public QFrame
{

	friend class textEditWithCompleter;

public:
	explicit searchWordPanel ( textEditWithCompleter* );
	void showPanel ();

private:
	textEditWithCompleter *m_texteditor;
	QGridLayout* mLayout;

	QToolButton* btnSearchStart, *btnSearchNext, *btnSearchPrev, *btnRplcWord, *btnRplcAll;
	QToolButton* btnClose;
	vmLineEdit* searchField, *replaceField;

	bool searchStart ();
	void searchCancel ();
	bool searchNext ();
	bool searchPrev ();

	void replaceWord ();
	void rplcAll ();
	void searchField_textAltered ( const QString& text );
	void searchField_keyPressed ( const QKeyEvent* ke );
	void replaceField_textAltered (const QString& );
	void replaceField_keyPressed ( const QKeyEvent* ke );
};

//------------------------------------------------FIND-WORD-PANEL--------------------------------------------

#endif // TEXTEDITWITHCOMPLETER_H
