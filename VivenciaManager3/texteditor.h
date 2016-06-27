#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include "documenteditorwindow.h"

#include <QMap>
#include <QDockWidget>
#include <QTextCursor>
#include <QTextObjectInterface>

class textEditWithCompleter;
class vmComboBox;

class QTextObjectInterface;
class QPushButton;
class QTextCursor;
class QSpinBox;
class QToolButton;
class QFontComboBox;
class QRadioButton;
class QDockWidget;
class QPrinter;

static const uint TEXT_EDITOR_SUB_WINDOW ( 2 );
static const uint RICH_TEXT_EDITOR_SUB_WINDOW ( 4 );

class textEditor : public documentEditorWindow
{

friend class textEditorToolBar;
friend class dockQP;
friend class dockBJ;

public:
	explicit textEditor ( documentEditor* mdiParent );

	static inline QString filter () {
		return tr ( "Text files (*.txt);;Richtext files (*.rtf)" );
	}

	void cut ();
	void copy ();
	void paste ();

	void createNew ();
	bool loadFile ( const QString& filename );
	bool displayText ( const QString& text );
	bool saveAs ( const QString& filename );
	bool saveFile ( const QString& filename );

	void buildMailMessage ( QString& address, QString& subject, QString& attachment, QString& body );

	bool useHtml () const {
		return mb_UseHtml;
	}
	void setUseHtml ( const bool html ) {
		mb_UseHtml = html;
	}

	bool ignoreCursorPos () const {
		return mb_IgnoreCursorPos;
	}
	void setIgnoreCursorPos ( const bool ignore ) {
		mb_IgnoreCursorPos = ignore;
	}

	const QString initialDir () const;

protected:
	QTextCursor mCursor;
	textEditWithCompleter* mDocument;
	QString mailAddress, mailSubject;

private:
	QString mPDFName;
	bool mb_UseHtml;
	bool mb_IgnoreCursorPos;
	uint m_ImageNumber;

	QMap<QString,QString> mapImages;
};

class textEditorToolBar : public QDockWidget
{

friend class textEditor;
friend class reportGenerator;

public:
	textEditorToolBar ();
	virtual ~textEditorToolBar ();
	void setCurrentWindow ( textEditor *ed );
	inline textEditor *currentWindow () const {
		return mEditorWindow;
	}

	void show ( documentEditor* mdiParent );

	void updateControls ();
	void checkAlignment ( const Qt::Alignment align );

	void btnCreateTable_clicked ();
	void btnRemoveTable_clicked ();
	void btnInsertTableRow_clicked ();
	void btnRemoveTableRow_clicked ();
	void btnInsertTableCol_clicked ();
	void btnRemoveTableCol_clicked ();
	void btnInsertBulletList_cliked ();
	void btnInsertNumberedList_clicked ();
	void btnInsertImage_clicked ();
	void btnBold_clicked ( const bool );
	void btnItalic_clicked ( const bool );
	void btnUnderline_clicked ( const bool );
	void btnStrikethrough_clicked ( const bool );
	void btnAlignLeft_clicked  ();
	void btnAlignRight_clicked  ();
	void btnAlignCenter_clicked  ();
	void btnAlignJustify_clicked  ();
	void btnTextColor_clicked ();
	void btnHighlightColor_clicked ();
	void btnPrint_clicked ();
	void btnPrintPreview_clicked ();
	void btnExportToPDF_clicked ();
	void previewPrint ( QPrinter* );

	void setFontType ( const QString& type );
	void setFontSize ( const int size );
	void setFontSizeFromComboBox ( const int index );
	void insertImage ( const QString& imageFile, QTextFrameFormat::Position pos = QTextFrameFormat::InFlow );

private:
	void setFontColor ( const QColor& color );
	void setHighlight  ( const QColor& color );
	void createList ( const QTextListFormat::Style = QTextListFormat::ListDisc );
	void alignText ( const Qt::Alignment align = Qt::AlignLeft );

	QPushButton* btnCreateTable, *btnRemoveTable;
	QToolButton* btnInsertTableRow, *btnRemoveTableRow;
	QToolButton* btnInsertTableCol, *btnRemoveTableCol;
	QPushButton* btnInsertBulletList, *btnInsertNumberedList;
	QPushButton* btnInsertImage;

	QSpinBox *spinNRows, *spinNCols;

	QToolButton* btnBold, *btnItalic, *btnUnderline, *btnStrikethrough;
	QToolButton* btnAlignLeft, *btnAlignRight, *btnAlignCenter, *btnAlignJustify;
	QToolButton* btnTextColor, *btnHighlightColor;
	QToolButton* btnPrint, *btnPrintPreview, *btnExportToPDF;

	QFontComboBox *cboFontType;
	vmComboBox* cboFontSizes;

	QRadioButton* optEmailAttached, *optEmailBody;

	void mergeFormatOnWordOrSelection ( const QTextCharFormat& format );
	static textEditorToolBar *s_instance;
	friend textEditorToolBar *TEXT_EDITOR_TOOLBAR ();
	textEditor *mEditorWindow;
};

class imageTextObject : public QObject, public QTextObjectInterface
{

Q_INTERFACES ( QTextObjectInterface )

public:
	QSizeF intrinsicSize ( QTextDocument* /*doc*/, int /*posInDocument*/, const QTextFormat &/*format*/ );
	void drawObject ( QPainter* /*painter*/, const QRectF &/*rect*/, QTextDocument* /*doc*/, int /*posInDocument*/, const QTextFormat &/*format*/ );
};

inline textEditorToolBar *TEXT_EDITOR_TOOLBAR ()
{
	if ( textEditorToolBar::s_instance == nullptr )
		textEditorToolBar::s_instance = new textEditorToolBar ();
	return textEditorToolBar::s_instance;
}

#endif // TEXTEDITOR_H
