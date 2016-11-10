#include "texteditor.h"
#include "fileops.h"
#include "configops.h"
#include "data.h"
#include "vmnotify.h"
#include "vmwidgets.h"
#include "texteditwithcompleter.h"
#include "documenteditor.h"

#include <QPushButton>
#include <QSpinBox>
#include <QTextCodec>
#include <QToolButton>
#include <QFontComboBox>
#include <QRadioButton>
#include <QFile>
#include <QTextDocumentWriter>
#include <QApplication>

#include <QtPrintSupport/QPrintDialog>
#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrintPreviewDialog>

#include <QTextTable>
#include <QPainter>
#include <QLabel>
#include <QColorDialog>
#include <QVariant>

/*
I am using the object's property objectName () pointer by doc because I cannot use any info that its parent holds that is derived
from Qt. That's because doc gets copied from its original source to be used by the printer class, and only Qt's original info is copied,
not my subclassed variations. It is a hack, but it works.
*/
QSizeF imageTextObject::intrinsicSize ( QTextDocument* doc, int /*posInDocument*/, const QTextFormat &format )
{
	const QImage bufferedImage ( qvariant_cast<QImage> ( format.property ( 1 ) ) );
	QSizeF size ( bufferedImage.size () );

	if ( doc->property ( PROPERTY_PRINT_PREVIEW ).toBool () == true )
		size *= 4.0;

	return size;
}

void imageTextObject::drawObject ( QPainter* painter, const QRectF& rect, QTextDocument* doc, int posInDocument,
								   const QTextFormat& format )
{
	const QImage bufferedImage ( qvariant_cast<QImage> ( format.property ( 1 ) ) );
	painter->drawImage ( QRectF ( rect.topLeft (), intrinsicSize ( doc, posInDocument, format ) ), bufferedImage );
}

textEditor::textEditor ( documentEditor* mdiParent )
	: documentEditorWindow ( mdiParent ), mPDFName ( emptyString ),
	  mb_UseHtml ( false ), mb_IgnoreCursorPos ( true ), m_ImageNumber ( 0 )
{
	setEditorType ( TEXT_EDITOR_SUB_WINDOW );
	mDocument = new textEditWithCompleter ( nullptr );
	mDocument->resize ( 300, 300 );
	mDocument->document ()->documentLayout ()->registerHandler ( QTextFormat::UserObject + 1, new imageTextObject );
	connect ( mDocument, &textEditWithCompleter::cursorPositionChanged, TEXT_EDITOR_TOOLBAR (), [&] () { return TEXT_EDITOR_TOOLBAR ()->updateControls (); } );
	mCursor = mDocument->textCursor ();
	mCursor.movePosition ( QTextCursor::Start );
	connect ( mDocument->document (), &QTextDocument::contentsChanged, this, [&] () { return documentWasModified (); } );
	connect ( mDocument->document (), &QTextDocument::undoAvailable, this, [&] ( const bool undo ) { return documentWasModifiedByUndo ( undo ); } );
	connect ( mDocument->document (), &QTextDocument::redoAvailable, this, [&] ( const bool redo ) { return documentWasModifiedByRedo ( redo ); } );
	mainLayout->addWidget ( mDocument, 0, 0, 1, 1 );

	TEXT_EDITOR_TOOLBAR ()->show ( mdiParent );
	mDocument->setFocus ();
}

void textEditor::cut ()
{
	if ( mDocument != nullptr )
		mDocument->cut ();
}

void textEditor::copy ()
{
	if ( mDocument != nullptr )
		mDocument->copy ();
}

void textEditor::paste ()
{
	if ( mDocument != nullptr )
		mDocument->paste ();
}

void textEditor::createNew ()
{
	mDocument->document ()->clear ();
	mDocument->document ()->setModified ( false );
	mCursor = mDocument->textCursor ();
	mapImages.clear ();
	mDocument->setFocus ();
	mb_IgnoreCursorPos = false;
}

bool textEditor::loadFile ( const QString& filename )
{
	int error_num ( -1 );
	if ( fileOps::exists ( filename ).isOn () ) {
		QFile file ( filename );
		if ( file.open ( QFile::ReadOnly ) ) {
			QByteArray data = file.readAll ();
			if ( !data.isEmpty () ) {
				if ( fileOps::fileExtension ( filename ) == QStringLiteral ( "txt" ) ) {
					mb_UseHtml = false;
					mDocument->setPlainText ( QString::fromUtf8 ( data.constData () ) );
				}
				else {
					mb_UseHtml = true;
					QTextCodec* codec ( Qt::codecForHtml ( data ) );
					mDocument->setHtml ( codec->toUnicode ( data ) );
				}
			}
			if ( mDocument->document ()->isEmpty () )
				error_num = 0; // read fail

		}
		else
			error_num = 1;
	}
	else
		error_num = 2;

	if ( error_num >= 0 ) {
		const QString msg_err[3] = {
			TR_FUNC ( "File %1 could not be read" ),
			TR_FUNC ( "File %1 does not exist" ),
			TR_FUNC ( "Please speficy the filaname for opening" )
		};
		if ( VM_NOTIFY ()->criticalBox ( TR_FUNC ( "Error loading file" ), msg_err[error_num].arg ( filename ), false ) == MESSAGE_BTN_OK )
			return loadFile ( filename );
		else
			return false;
	}
	else {
		mCursor = mDocument->textCursor ();
		mDocument->document ()->setModified ( false );
		setCurrentFile ( filename );
		mDocument->setFocus ();
		return true;
	}
}

bool textEditor::displayText ( const QString& text )
{
	if ( !text.isEmpty () )
	{
		if ( text.startsWith ( QStringLiteral ( "<html>" ), Qt::CaseInsensitive ) )
		{
			mb_UseHtml = true;
			mDocument->setHtml ( text );
		}
		else
		{
			mb_UseHtml = false;
			mDocument->setPlainText ( text );
		}
		return true;
	}
	return false;
}

bool textEditor::saveFile ( const QString& filename )
{
	bool written ( false );
	QFile file ( filename );
	if ( file.open ( QFile::WriteOnly | QFile::Text ) )
	{
		QString str = mb_UseHtml ?
					  mDocument->document()->toHtml ( "utf-8" ) :
					  mDocument->document()->toPlainText ();

		if ( !mapImages.isEmpty () )
		{
			int idxImageID ( 0 );
			int idxEndOfTable ( 0 );
			QMapIterator<QString, QString> itr ( mapImages );
			while ( itr.hasNext () )
			{
				itr.next ();
				idxImageID = str.indexOf ( itr.key (), idxEndOfTable );
				if ( idxImageID == -1 ) continue;
				idxEndOfTable = str.indexOf ( QStringLiteral ( "</p></td></tr></table>" ), idxImageID );
				if ( idxEndOfTable != -1 )
					str.insert ( idxEndOfTable, QStringLiteral ( "<img border=0 src=%1>" ).arg ( itr.value () ) );
			}
		}

		/*.remove ( 0, 94 ) : This will remove the following line from the output file
		  !DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0//EN" "http://www.w3.org/TR/REC-html40/strict.dtd">
		  Instead of putting QString to the "hard" task of trying to find the dumb string, just
		  ask it to remove the length of it. If it changes, it would not matter whether I put it to
		  comparison, it would fail any way
		*/
		const char* data ( str.remove ( 0, 94 ).toUtf8 ().data () );
		const int length ( ::strlen ( data ) );
		written = file.write ( data, length );
		file.close ();
		if ( written )
			mDocument->document ()->setModified ( false );
	}
	return written;
}

bool textEditor::saveAs ( const QString& filename )
{
	QString new_filename ( filename );
	QString extension ( fileOps::fileExtension ( filename ) );
	if ( extension.isEmpty () )
		// rtf is the safe option. since we don't know whether the user formatted the document or not
		new_filename = fileOps::replaceFileExtension ( filename, ( extension = QLatin1String ( "rtf" ) ) );
	mb_UseHtml = !( extension == QStringLiteral ( "txt" ) );
	return saveFile ( new_filename );
}

void textEditor::buildMailMessage ( QString& address, QString& subject, QString& attachment, QString& body )
{
	if ( TEXT_EDITOR_TOOLBAR ()->optEmailBody->isChecked () ) {
		if ( mDocument->document ()->isModified () )
			save ( currentFile () );
		if ( !currentFile ().isEmpty () )
			body = currentFile ();
	}
	else {
		if ( mDocument->document ()->isModified () || mPDFName.isEmpty () )
			TEXT_EDITOR_TOOLBAR ()->btnExportToPDF_clicked ();
		if ( !mPDFName.isEmpty () )
			attachment = mPDFName;
	}
	address.clear ();
	subject.clear ();
}

const QString textEditor::initialDir () const
{
	return CONFIG ()->projectsBaseDir ();
}

//---------------------------------------TOOL-BAR----------------------------------------------------

static QString stringFloatKey ( const uint increment )
{
	if ( increment >= 100 )
		return emptyString;

	QString magicNumber ( QStringLiteral ( "1.1111" ) );
	int multiplier ( static_cast<int>( increment / 10 ) );
	QChar uns ( QLatin1Char ( '0' ) );
	QChar tens ( QLatin1Char ( '0' ) );

	switch ( increment - ( multiplier * 10 ) ) {
		case 0:
			break;
		case 1:
			uns = '2';
			break;
		case 2:
			uns = '3';
		break;
		case 3:
			uns = '4';
		break;
		case 4:
			uns = '5';
		break;
		case 5:
			uns = '6';
		break;
		case 6:
			uns = '7';
		break;
		case 7:
			uns = '8';
		break;
		case 8:
			uns = '9';
		break;
		case 9:
			uns = '0';
			++multiplier;
		break;
	}

	switch ( multiplier ) {
		case 0:
			break;
		case 1:
			tens = '2';
		break;
		case 2:
			tens = '3';
		break;
		case 3:
			tens = '4';
		break;
		case 4:
			tens = '5';
		break;
		case 5:
			tens = '6';
		break;
		case 6:
			tens = '7';
		break;
		case 7:
			tens = '8';
		break;
		case 8:
			tens = '9';
		break;
		case 9:
			tens = '0';
			magicNumber.replace ( 3, 1, '2' );
		break;
	}

	if ( uns != CHR_ZERO )
		magicNumber.replace ( 5, 1, uns );

	if ( tens != CHR_ZERO )
		magicNumber.replace ( 4, 1, tens );

	if ( magicNumber.at ( 5 ) == CHR_ZERO )
		magicNumber.truncate ( 4 );

	return magicNumber;
}

static float floatFromStringKey ( const QString str )
{
	float magicNumber ( 1.1100 );
	char digit ( '0' );

	// trailling 0 (zero) is truncated in the string because the last zeroes of a float number are omitted by the compiler
	// therefore, str, in this case, would be, for instance, 1.112, instead of 1.1120
	if ( str.length () == 6 )
		digit = str.at ( 5 ).toLatin1 ();

	switch ( digit ) {
		case '0':
		break;
		case '1':
			magicNumber += 0.0001;
		break;
		case '2':
			magicNumber += 0.0002;
		break;
		case '3':
			magicNumber += 0.0003;
		break;
		case '4':
			magicNumber += 0.0004;
		break;
		case '5':
			magicNumber += 0.0005;
		break;
		case '6':
			magicNumber += 0.0006;
		break;
		case '7':
			magicNumber += 0.0007;
		break;
		case '8':
			magicNumber += 0.0008;
		break;
		case '9':
			magicNumber += 0.0009;
		break;
	}

	if ( str.length () == 6 )
		digit = str.at ( 4 ).toLatin1 ();
	else
		digit = str.at ( 3 ).toLatin1 ();

	switch ( digit ) {
		case '0':
		break;
		case '1':
			magicNumber += 0.0010;
		break;
		case '2':
			magicNumber += 0.0020;
		break;
		case '3':
			magicNumber += 0.0030;
		break;
		case '4':
			magicNumber += 0.0040;
		break;
		case '5':
			magicNumber += 0.0050;
		break;
		case '6':
			magicNumber += 0.0060;
		break;
		case '7':
			magicNumber += 0.0070;
		break;
		case '8':
			magicNumber += 0.0080;
		break;
		case '9':
			magicNumber += 0.0090;
		break;
	}

	return magicNumber;
}

textEditorToolBar *textEditorToolBar::s_instance ( nullptr );

textEditorToolBar::textEditorToolBar ()
	: QDockWidget ( TR_FUNC ( "Document Editor" ) ), mEditorWindow ( nullptr )
{
	btnBold = new QToolButton;
	btnBold->setCheckable ( true );
	btnBold->setIcon ( ICON ( "format-text-bold" ) );
	btnBold->setToolTip ( TR_FUNC ( "Bold font" ) );
	connect ( btnBold, &QToolButton::clicked, this, [&] ( const bool checked ) { return btnBold_clicked ( checked  ); } );

	btnItalic = new QToolButton;
	btnItalic->setCheckable ( true );
	btnItalic->setIcon ( ICON ( "format-text-italic" ) );
	btnItalic->setToolTip ( TR_FUNC ( "Italic font" ) );
	connect ( btnItalic, &QToolButton::clicked, this, [&] ( const bool checked ) { return btnItalic_clicked ( checked  ); } );

	btnUnderline = new QToolButton;
	btnUnderline->setCheckable ( true );
	btnUnderline->setIcon ( ICON ( "format-text-underline" ) );
	btnUnderline->setToolTip ( TR_FUNC ( "Underline font" ) );
	connect ( btnUnderline, &QToolButton::clicked, this, [&] ( const bool checked ) { return btnUnderline_clicked ( checked	); } );

	btnStrikethrough = new QToolButton;
	btnStrikethrough->setCheckable ( true );
	btnStrikethrough->setIcon ( ICON ( "format-text-strikethrough" ) );
	btnStrikethrough->setToolTip ( TR_FUNC ( "Strike through font" ) );
	connect ( btnStrikethrough, &QToolButton::clicked, this, [&] ( const bool checked ) { return btnStrikethrough_clicked ( checked ); } );

	QFrame* vLine2 ( new QFrame );
	vLine2->setFrameStyle ( QFrame::VLine|QFrame::Raised );

	btnTextColor = new QToolButton;
	btnTextColor->setIcon ( ICON ( "format-text-color" ) );
	btnTextColor->setToolTip ( TR_FUNC ( "Font color" ) );
	connect ( btnTextColor, &QToolButton::clicked, this, [&] () { return btnTextColor_clicked (); } );

	btnHighlightColor = new QToolButton;
	btnHighlightColor->setIcon ( ICON ( "format-stroke-color" ) );
	btnHighlightColor->setToolTip ( TR_FUNC ( "Font highlight color" ) );
	connect ( btnHighlightColor, &QToolButton::clicked, this, [&] () { return btnHighlightColor_clicked (); } );

	QHBoxLayout* layoutFontStyleButtons = new QHBoxLayout;
	layoutFontStyleButtons->setSpacing ( 1 );
	layoutFontStyleButtons->setContentsMargins ( 1, 1, 1, 1 );
	layoutFontStyleButtons->addWidget ( btnBold );
	layoutFontStyleButtons->addWidget ( btnItalic );
	layoutFontStyleButtons->addWidget ( btnUnderline );
	layoutFontStyleButtons->addWidget ( btnStrikethrough );
	layoutFontStyleButtons->addWidget ( vLine2 );
	layoutFontStyleButtons->addWidget ( btnTextColor );
	layoutFontStyleButtons->addWidget ( btnHighlightColor );

	cboFontType = new QFontComboBox;
	cboFontType->setCurrentFont ( QApplication::font () );
	connect ( cboFontType, static_cast<void (QFontComboBox::*) (const QString&)>( &QFontComboBox::activated ), this, [&] ( const QString& fontname ) { return setFontType ( fontname ); } );
	
	cboFontSizes = new vmComboBox;
	for ( uint i ( 6 ); i < 50; ++i )
		cboFontSizes->addItem ( QString::number ( i ) );
	cboFontSizes->setCurrentIndex ( cboFontSizes->findText ( QString::number ( QApplication::font ().pointSize () ) ) );
	cboFontSizes->setCallbackForIndexChanged ( [&] ( const int idx ) { return setFontSizeFromComboBox ( idx ); } );
	cboFontSizes->setEditable ( true );
	cboFontSizes->setIgnoreChanges ( false );
	
	QGridLayout* gLayoutFontCombos ( new QGridLayout );
	gLayoutFontCombos->setSpacing ( 1 );
	gLayoutFontCombos->setContentsMargins ( 1, 1, 1, 1 );
	gLayoutFontCombos->addWidget ( new QLabel ( TR_FUNC ( "Size: " ) ), 0, 0 );
	gLayoutFontCombos->addWidget ( cboFontSizes, 0, 1 );

	QFrame* hLine2 ( new QFrame );
	hLine2->setFrameStyle ( QFrame::HLine|QFrame::Raised );
	//----------------------------------------------------------------------------------------

	btnAlignLeft = new QToolButton;
	btnAlignLeft->setCheckable ( true );
	btnAlignLeft->setChecked ( true );
	btnAlignLeft->setIcon ( ICON ( "format-justify-left" ) );
	btnAlignLeft->setToolTip ( TR_FUNC ( "Align text with the left margin" ) );
	connect ( btnAlignLeft, &QToolButton::clicked, this, [&] () { return btnAlignLeft_clicked (); } );

	btnAlignRight = new QToolButton;
	btnAlignRight->setCheckable ( true );
	btnAlignRight->setIcon ( ICON ( "format-justify-right" ) );
	btnAlignRight->setToolTip ( TR_FUNC ( "Align text with the right margin" ) );
	connect ( btnAlignRight, &QToolButton::clicked, this, [&] () { return btnAlignRight_clicked (); } );

	btnAlignCenter = new QToolButton;
	btnAlignCenter->setCheckable ( true );
	btnAlignCenter->setIcon ( ICON ( "format-justify-center" ) );
	btnAlignCenter->setToolTip ( TR_FUNC ( "Align text in the center" ) );
	connect ( btnAlignCenter, &QToolButton::clicked, this, [&] () { return btnAlignCenter_clicked (); } );

	btnAlignJustify = new QToolButton;
	btnAlignJustify->setCheckable ( true );
	btnAlignJustify->setIcon ( ICON ( "format-justify-fill" ) );
	btnAlignJustify->setToolTip ( TR_FUNC ( "Justify text within margins" ) );
	connect ( btnAlignJustify, &QToolButton::clicked, this, [&] () { return btnAlignJustify_clicked (); } );

	QHBoxLayout* layoutTextAlignmentButtons ( new QHBoxLayout );
	layoutTextAlignmentButtons->setSpacing ( 1 );
	layoutTextAlignmentButtons->setContentsMargins ( 1, 1, 1, 1 );
	layoutTextAlignmentButtons->addWidget ( btnAlignLeft );
	layoutTextAlignmentButtons->addWidget ( btnAlignRight );
	layoutTextAlignmentButtons->addWidget ( btnAlignCenter );
	layoutTextAlignmentButtons->addWidget ( btnAlignJustify );

	QFrame* hLine3 ( new QFrame );
	hLine3->setFrameStyle ( QFrame::HLine|QFrame::Raised );
	//----------------------------------------------------------------------------------------

	spinNRows = new QSpinBox;
	spinNRows->setMinimumWidth ( 30 );
	spinNRows->setValue ( 2 );
	spinNRows->setMinimum ( 1 );

	spinNCols = new QSpinBox;
	spinNCols->setMinimumWidth ( 30 );
	spinNCols->setValue ( 2 );
	spinNCols->setMinimum ( 1 );

	btnInsertTableRow = new QToolButton;
	btnInsertTableRow->setIcon ( ICON ( "add" ) );
	btnInsertTableRow->setToolTip ( TR_FUNC ( "Insert n rows into table from current row" ) );
	connect ( btnInsertTableRow, &QToolButton::clicked, this, [&] () { return btnInsertTableRow_clicked (); } );

	btnRemoveTableRow = new QToolButton;
	btnRemoveTableRow->setIcon ( ICON ( "remove" ) );
	btnRemoveTableRow->setToolTip ( TR_FUNC ( "Remove n rows down from table starting at current row" ) );
	connect ( btnRemoveTableRow, &QToolButton::clicked, this, [&] () { return btnRemoveTableRow_clicked (); } );

	btnInsertTableCol = new QToolButton;
	btnInsertTableCol->setIcon ( ICON ( "add" ) );
	btnInsertTableCol->setToolTip ( TR_FUNC ( "Insert n columns into table to the right of the current column" ) );
	connect ( btnInsertTableCol, &QToolButton::clicked, this, [&] () { return btnInsertTableCol_clicked (); } );

	btnRemoveTableCol = new QToolButton;
	btnRemoveTableCol->setIcon ( ICON ( "remove" ) );
	btnRemoveTableCol->setToolTip ( TR_FUNC ( "Remove n columns from table from the left of the current column" ) );
	connect ( btnRemoveTableCol, &QToolButton::clicked, this, [&] () { return btnRemoveTableCol_clicked (); } );

	btnCreateTable = new QPushButton ( TR_FUNC ( "Insert table" ) );
	connect ( btnCreateTable, &QToolButton::clicked, this, [&] () { return btnCreateTable_clicked (); } );

	btnRemoveTable = new QPushButton ( TR_FUNC ( "Remove table" ) );
	connect ( btnRemoveTable, &QToolButton::clicked, this, [&] () { return btnRemoveTable_clicked (); } );

	QHBoxLayout* layoutTableBtns ( new QHBoxLayout );
	layoutTableBtns->setSpacing ( 1 );
	layoutTableBtns->setContentsMargins ( 1, 1, 1, 1 );
	layoutTableBtns->addWidget ( btnCreateTable, 1 );
	layoutTableBtns->addWidget ( btnRemoveTable, 1 );

	QGridLayout* gLayoutRowsAndCols ( new QGridLayout );
	gLayoutRowsAndCols->setSpacing ( 1 );
	gLayoutRowsAndCols->setContentsMargins ( 1, 1, 1, 1 );
	gLayoutRowsAndCols->addWidget ( new QLabel ( TR_FUNC ( "Rows:" ) ), 0, 0 );
	gLayoutRowsAndCols->addWidget ( spinNRows, 0, 1 );
	gLayoutRowsAndCols->addWidget ( btnInsertTableRow, 0, 2 );
	gLayoutRowsAndCols->addWidget ( btnRemoveTableRow, 0, 3 );
	gLayoutRowsAndCols->addWidget ( new QLabel ( TR_FUNC ( "Columns:" ) ), 1, 0 );
	gLayoutRowsAndCols->addWidget ( spinNCols, 1, 1 );
	gLayoutRowsAndCols->addWidget ( btnInsertTableCol, 1, 2 );
	gLayoutRowsAndCols->addWidget ( btnRemoveTableCol, 1, 3 );

	QFrame* hLine4 = new QFrame;
	hLine4->setFrameStyle ( QFrame::HLine|QFrame::Raised );
	//----------------------------------------------------------------------------------------

	btnInsertBulletList = new QPushButton ( ICON ( "bullet-list" ), TR_FUNC ( "Bullet list" ) );
	connect ( btnInsertBulletList, &QToolButton::clicked, this, [&] () { return btnInsertBulletList_cliked (); } );

	btnInsertNumberedList = new QPushButton ( ICON ( "numbered-list" ), TR_FUNC ( "Numbered list" ) );
	connect ( btnInsertNumberedList, &QToolButton::clicked, this, [&] () { return btnInsertNumberedList_clicked (); } );

	QHBoxLayout* layoutLists ( new QHBoxLayout );
	layoutLists->setSpacing ( 1 );
	layoutLists->setContentsMargins ( 1, 1, 1, 1 );
	layoutLists->addWidget ( btnInsertBulletList, 1 );
	layoutLists->addWidget ( btnInsertNumberedList, 1 );

	QFrame* hLine5 = new QFrame;
	hLine5->setFrameStyle ( QFrame::HLine|QFrame::Raised );
	//----------------------------------------------------------------------------------------

	btnInsertImage = new QPushButton ( ICON ( "insert-image" ), TR_FUNC ( "Insert image" ) );
	connect ( btnInsertImage, &QToolButton::clicked, this, [&] () { return btnInsertImage_clicked (); } );
	QFrame* hLine6 = new QFrame;
	hLine6->setFrameStyle ( QFrame::HLine|QFrame::Raised );
	//----------------------------------------------------------------------------------------

	btnPrint = new QToolButton;
	btnPrint->setIcon ( ICON ( "document-print" ) );
	btnPrint->setToolTip ( TR_FUNC ( "Print" ) );
	connect ( btnPrint, &QToolButton::clicked, this, [&] () { return btnPrint_clicked (); } );

	btnPrintPreview = new QToolButton;
	btnPrintPreview->setIcon ( ICON ( "document-print-preview" ) );
	btnPrintPreview->setToolTip ( TR_FUNC ( "Print preview" ) );
	connect ( btnPrintPreview, &QToolButton::clicked, this, [&] () { return btnPrintPreview_clicked (); } );

	btnExportToPDF = new QToolButton;
	btnExportToPDF->setIcon ( ICON ( "application-pdf" ) );
	btnExportToPDF->setToolTip ( TR_FUNC ( "Export to PDF" ) );
	connect ( btnExportToPDF, &QToolButton::clicked, this, [&] () { return btnExportToPDF_clicked (); } );

	QHBoxLayout* layoutPrint = new QHBoxLayout;
	layoutPrint->setSpacing ( 1 );
	layoutPrint->setContentsMargins ( 1, 1, 1, 1 );
	layoutPrint->addWidget ( btnPrint );
	layoutPrint->addWidget ( btnPrintPreview );
	layoutPrint->addWidget ( btnExportToPDF );

	optEmailAttached = new QRadioButton ( TR_FUNC ( "Send as attachment to e-mail" ) );
	optEmailBody = new QRadioButton ( TR_FUNC ( "Send as e-mail message body" ) );
	optEmailAttached->setChecked ( true );

	QVBoxLayout* layoutEmail ( new QVBoxLayout );
	layoutEmail->setSpacing ( 1 );
	layoutEmail->setContentsMargins ( 1, 1, 1, 1 );
	layoutEmail->addWidget ( optEmailAttached, 0 );
	layoutEmail->addWidget ( optEmailBody, 0 );

	QHBoxLayout* layoutEmail2 ( new QHBoxLayout );
	layoutEmail2->setSpacing ( 1 );
	layoutEmail2->setContentsMargins ( 1, 1, 1, 1 );
	layoutEmail2->addLayout ( layoutEmail, 1 );
	//----------------------------------------------------------------------------------------

	QVBoxLayout* layoutMainToolBar ( new QVBoxLayout );
	layoutMainToolBar->setSpacing ( 1 );
	layoutMainToolBar->setContentsMargins ( 1, 1, 1, 1 );

	layoutMainToolBar->addLayout ( layoutFontStyleButtons, 1 );
	layoutMainToolBar->addWidget ( cboFontType, 1 );
	layoutMainToolBar->addLayout ( gLayoutFontCombos, 1 );
	layoutMainToolBar->addWidget ( hLine2, 1 );
	layoutMainToolBar->addLayout ( layoutTextAlignmentButtons, 1 );
	layoutMainToolBar->addWidget ( hLine3, 1 );
	layoutMainToolBar->addLayout ( gLayoutRowsAndCols, 1 );
	layoutMainToolBar->addLayout ( layoutTableBtns, 1 );
	layoutMainToolBar->addWidget ( hLine4, 1 );
	layoutMainToolBar->addLayout ( layoutLists, 1 );
	layoutMainToolBar->addWidget ( hLine5, 1 );
	layoutMainToolBar->addWidget ( btnInsertImage, 1 );
	layoutMainToolBar->addWidget ( hLine6, 1 );
	layoutMainToolBar->addLayout ( layoutPrint, 1 );
	layoutMainToolBar->addLayout ( layoutEmail2, 1 );

	QFrame* frmMainToolBar ( new QFrame );
	frmMainToolBar->setFrameStyle ( QFrame::Raised | QFrame::StyledPanel );
	frmMainToolBar->setLayout ( layoutMainToolBar );

	setWidget ( frmMainToolBar );
}

textEditorToolBar::~textEditorToolBar () {}

void textEditorToolBar::setCurrentWindow ( textEditor* ed )
{
	if ( ed != mEditorWindow )
	{
		mEditorWindow = ed;
		updateControls ();
	}
}

void textEditorToolBar::show ( documentEditor* mdiParent )
{
	if ( mdiParent != nullptr )
	{
		setFloating ( false );
		mdiParent->addDockWidget ( Qt::LeftDockWidgetArea, this );
	}
	else
		setFloating ( true );

	QDockWidget::show ();
}

void textEditorToolBar::mergeFormatOnWordOrSelection ( const QTextCharFormat& format )
{
	if ( mEditorWindow == nullptr ) return;

	mEditorWindow->mCursor.mergeCharFormat ( format );
	mEditorWindow->mDocument->mergeCurrentCharFormat ( format );
}

void textEditorToolBar::updateControls ()
{
	if ( mEditorWindow != nullptr )
	{
		if ( !mEditorWindow->ignoreCursorPos () )
		{
			QTextCharFormat charFormat = mEditorWindow->mDocument->textCursor ().charFormat ();
			btnItalic->setChecked ( charFormat.font ().italic () );
			btnBold->setChecked ( charFormat.font ().weight () != QFont::Normal );
			btnUnderline->setChecked ( charFormat.font ().underline () );
			btnStrikethrough->setChecked ( charFormat.font ().strikeOut () );
			cboFontSizes->setCurrentIndex ( cboFontSizes->findText ( QString::number ( charFormat.font ().pointSize () ) ) );
			cboFontType->setCurrentIndex ( cboFontType->findText ( charFormat.font ().family () ) );

			const QTextTable* table ( mEditorWindow->mDocument->textCursor ().currentTable () );
			if ( table != nullptr )
				{
				spinNRows->setValue ( table->rows () );
				spinNCols->setValue ( table->columns () );
			}
			checkAlignment ( mEditorWindow->mDocument->alignment () );
		}
	}
}

void textEditorToolBar::checkAlignment ( const Qt::Alignment align )
{
	if ( mEditorWindow != nullptr )
	{
		if ( !mEditorWindow->ignoreCursorPos () )
		{
			btnAlignLeft->setChecked ( align == Qt::AlignLeft );
			btnAlignRight->setChecked ( align == Qt::AlignRight );
			btnAlignJustify ->setChecked ( align == Qt::AlignJustify );
			btnAlignCenter->setChecked ( align == Qt::AlignHCenter );
		}
	}
}

void textEditorToolBar::btnCreateTable_clicked ()
{
	if ( mEditorWindow == nullptr ) return;

	QTextTable* textTable ( mEditorWindow->mCursor.currentTable () );
	if ( textTable != nullptr )
		return;

	mEditorWindow->setIgnoreCursorPos ( true );

	QTextTableFormat tableFormat;
	tableFormat.setCellPadding ( 2 );
	textTable = mEditorWindow->mCursor.insertTable ( spinNRows->value (), spinNCols->value (), tableFormat );

	QTextFrameFormat frameFormat = mEditorWindow->mCursor.currentFrame ()->frameFormat ();
	frameFormat.setBorder ( 2 );
	frameFormat.setPosition ( QTextFrameFormat::InFlow );
	mEditorWindow->mCursor.currentFrame ()->setFrameFormat ( frameFormat );

	textTable->resize ( spinNRows->value (), spinNCols->value () );
	QTextCursor tableCursor = mEditorWindow->mCursor;
	for ( int col ( 0 ); col < spinNCols->value (); ++col )
		{
		tableCursor = textTable->cellAt ( 0, col ).firstCursorPosition ();
		tableCursor.insertText ( QStringLiteral ( "	" ) );
	}
	mEditorWindow->mCursor.setPosition ( textTable->cellAt ( 0, 0 ).firstCursorPosition ().position () ) ;
	mEditorWindow->setIgnoreCursorPos ( false );
	mEditorWindow->mDocument->setFocus ();
}

void textEditorToolBar::btnRemoveTable_clicked ()
{
	if ( mEditorWindow == nullptr ) return;

	QTextTable* textTable ( mEditorWindow->mCursor.currentTable () );
	if ( textTable != nullptr )
	{
		mEditorWindow->setIgnoreCursorPos ( true );
		textTable->removeRows ( 0, textTable->rows () );
		mEditorWindow->setIgnoreCursorPos ( false );
	}
	mEditorWindow->mDocument->setFocus ();
}

void textEditorToolBar::btnInsertTableRow_clicked ()
{
	if ( mEditorWindow == nullptr ) return;

	QTextTable* textTable ( mEditorWindow->mCursor.currentTable () );
	if ( textTable != nullptr )
	{
		mEditorWindow->setIgnoreCursorPos ( true );

		textTable->insertRows ( textTable->cellAt ( mEditorWindow->mCursor ).row () + 1, spinNRows->value () );
		int row ( textTable->cellAt ( mEditorWindow->mCursor ).row () );
		if ( textTable->cellAt ( mEditorWindow->mCursor ).column () == 0 )
			++row;
		mEditorWindow->mCursor.setPosition ( textTable->cellAt ( row, 0 ).firstCursorPosition ().position () ) ;
		mEditorWindow->mDocument->setTextCursor ( mEditorWindow->mCursor );
		mEditorWindow->setIgnoreCursorPos ( false );
	}
	mEditorWindow->mDocument->setFocus ();
}

void textEditorToolBar::btnRemoveTableRow_clicked ()
{
	if ( mEditorWindow == nullptr ) return;

	QTextTable* textTable ( nullptr );
	textTable = mEditorWindow->mCursor.currentTable ();
	if ( textTable != nullptr )
	{
		mEditorWindow->setIgnoreCursorPos ( true );
		textTable->removeRows ( textTable->cellAt ( mEditorWindow->mCursor ).row (), spinNRows->value () );
		mEditorWindow->setIgnoreCursorPos ( false );
	}
}

void textEditorToolBar::btnInsertTableCol_clicked ()
{
	if ( mEditorWindow == nullptr ) return;

	QTextTable* textTable ( mEditorWindow->mCursor.currentTable () );
	if ( textTable != nullptr )
	{
		mEditorWindow->setIgnoreCursorPos ( true );
		textTable->insertColumns ( textTable->cellAt ( mEditorWindow->mCursor ).column () + 1, spinNCols->value () );
		mEditorWindow->setIgnoreCursorPos ( false );
	}
}

void textEditorToolBar::btnRemoveTableCol_clicked ()
{
	if ( mEditorWindow == nullptr ) return;

	QTextTable* textTable ( mEditorWindow->mCursor.currentTable () );
	if ( textTable != nullptr )
	{
		mEditorWindow->setIgnoreCursorPos ( true );
		textTable->removeColumns ( textTable->cellAt ( mEditorWindow->mCursor ).column (), spinNCols->value () );
		mEditorWindow->setIgnoreCursorPos ( false );
	}
}

void textEditorToolBar::btnInsertBulletList_cliked ()
{
	createList ();
}

void textEditorToolBar::btnInsertNumberedList_clicked ()
{
	createList ( QTextListFormat::ListUpperRoman );
}

void textEditorToolBar::btnInsertImage_clicked ()
{
	if ( mEditorWindow == nullptr ) return;

	const QString imageFile ( fileOps::getOpenFileName ( mEditorWindow->initialDir (), TR_FUNC ( "Images (*.png *.xpm *.jpg)" ) ) );
	if ( !imageFile.isEmpty () )
		insertImage ( imageFile );
}

void textEditorToolBar::btnBold_clicked ( const bool checked )
{
	QTextCharFormat charFormat;
	charFormat.setFontWeight ( checked ? QFont::Bold : QFont::Normal );
	mergeFormatOnWordOrSelection ( charFormat );
}

void textEditorToolBar::btnItalic_clicked ( const bool checked )
{
	QTextCharFormat charFormat;
	charFormat.setFontItalic ( checked );
	mergeFormatOnWordOrSelection ( charFormat );
}

void textEditorToolBar::btnUnderline_clicked ( const bool checked )
{
	QTextCharFormat chrFormat;
	chrFormat.setUnderlineStyle ( checked ? QTextCharFormat::SingleUnderline : QTextCharFormat::NoUnderline );
	mergeFormatOnWordOrSelection ( chrFormat );
}

void textEditorToolBar::btnStrikethrough_clicked ( const bool checked )
{
	QTextCharFormat charFormat;
	charFormat.setFontStrikeOut ( checked );
	mergeFormatOnWordOrSelection ( charFormat );
}

void textEditorToolBar::alignText ( const Qt::Alignment align )
{
	if ( mEditorWindow == nullptr ) return;

	mEditorWindow->mDocument->setAlignment ( align );
	checkAlignment ( align );
}

void textEditorToolBar::btnAlignLeft_clicked ()
{
	alignText ();
}

void textEditorToolBar::btnAlignRight_clicked ()
{
	alignText ( Qt::AlignRight );
}

void textEditorToolBar::btnAlignCenter_clicked ()
{
	alignText ( Qt::AlignHCenter );
}

void textEditorToolBar::btnAlignJustify_clicked ()
{
	alignText ( Qt::AlignJustify );
}

void textEditorToolBar::btnTextColor_clicked ()
{
	const QColor color ( QColorDialog::getColor ( mEditorWindow->mDocument->textColor (), this ) );
	if ( color.isValid () )
		setFontColor ( color );
}

void textEditorToolBar::setFontColor ( const QColor &color )
{
	QTextCharFormat charFormat;
	charFormat.setForeground ( color );
	mergeFormatOnWordOrSelection ( charFormat );
}

void textEditorToolBar::btnHighlightColor_clicked ()
{
	const QColor color ( QColorDialog::getColor ( mEditorWindow->mDocument->textColor (), this ) );
	if ( color.isValid () )
		setHighlight ( color );
}

void textEditorToolBar::setHighlight( const QColor &color )
{
	QTextCharFormat charFormat;
	charFormat.setBackground ( color );
	charFormat.setProperty ( QTextFormat::FullWidthSelection, true );
	mergeFormatOnWordOrSelection ( charFormat );
}

#include <QTextList>
void textEditorToolBar::createList ( const QTextListFormat::Style style )
{
	if ( mEditorWindow == nullptr ) return;

	mEditorWindow->mCursor = mEditorWindow->mDocument->textCursor ();

	QTextListFormat listformat;
	QTextList* list ( mEditorWindow->mCursor.currentList () );

	listformat.setStyle ( list ? list->format ().style () : style );
	listformat.setIndent ( list ? list->format ().indent () + 1 : 1 );
	mEditorWindow->mCursor.createList ( listformat );
	mEditorWindow->mDocument->setTextCursor ( mEditorWindow->mCursor );
	mEditorWindow->mDocument->setFocus ();
}

void textEditorToolBar::btnPrint_clicked ()
{
	if ( mEditorWindow == nullptr ) return;

	mEditorWindow->setPreview ( true );
	mEditorWindow->mDocument->setPreview ( true );
	QPrinter printer ( QPrinter::ScreenResolution );
	printer.setPageMargins ( 12.00, 12.00, 12.00, 12.00, QPrinter::Millimeter );
	QPrintDialog* printerDlg = new QPrintDialog ( &printer, this );
	if ( mEditorWindow->mDocument->textCursor ().hasSelection () )
		printerDlg->addEnabledOption ( QAbstractPrintDialog::PrintSelection );
	printerDlg->setWindowTitle ( TR_FUNC ( "Print document" ) );

	if ( printerDlg->exec () == QDialog::Accepted )
		mEditorWindow->mDocument->print ( &printer );
	mEditorWindow->setPreview ( false );
	mEditorWindow->mDocument->setPreview ( false );
	delete printerDlg;
}

void textEditorToolBar::btnPrintPreview_clicked ()
{
	if ( mEditorWindow == nullptr ) return;

	mEditorWindow->setPreview ( true );
	mEditorWindow->mDocument->setPreview ( true );
	QPrinter printer ( QPrinter::HighResolution );
	printer.setPageMargins ( 12.00, 12.00, 12.00, 12.00, QPrinter::Millimeter );
	QPrintPreviewDialog preview ( &printer, this );
	connect ( &preview, &QPrintPreviewDialog::paintRequested, this, [&] ( QPrinter* printer ) { return previewPrint ( printer ); } );
	preview.exec ();
	mEditorWindow->setPreview ( false );
	mEditorWindow->mDocument->setPreview ( false );
}

void textEditorToolBar::previewPrint ( QPrinter* printer )
{
	if ( mEditorWindow == nullptr ) return;
	mEditorWindow->setPreview ( true );
	mEditorWindow->mDocument->setPreview ( true );
	mEditorWindow->mDocument->print ( printer );
}

void textEditorToolBar::btnExportToPDF_clicked ()
{
	if ( mEditorWindow == nullptr ) return;

	const QString fileName ( fileOps::getSaveFileName ( mEditorWindow->isUntitled () ?
					   mEditorWindow->initialDir () :
					   fileOps::filePathWithoutExtension ( mEditorWindow->currentFile () ),
					   TR_FUNC ( "Portable document format (*.pdf)" )
					) );

	if ( !fileName.isEmpty () )
	{
		mEditorWindow->setPreview ( true );
		mEditorWindow->mDocument->setPreview ( true );
		fileOps::replaceFileExtension ( fileName, QStringLiteral ( "pdf" ) ); // ensure filename ends with .pdf extension
		mEditorWindow->mPDFName = fileName;
		QPrinter printer ( QPrinter::HighResolution );
		printer.setPageMargins ( 12.00, 12.00, 12.00, 12.00, QPrinter::Millimeter );
		printer.setOutputFormat ( QPrinter::PdfFormat );
		printer.setOutputFileName ( fileName );
		mEditorWindow->mDocument->document ()->print ( &printer );
		mEditorWindow->setPreview ( false );
		mEditorWindow->mDocument->setPreview ( false );
	}
}

void textEditorToolBar::setFontType ( const QString& type )
{
	QTextCharFormat charFormat;
	charFormat.setFontFamily ( type );
	mergeFormatOnWordOrSelection ( charFormat );
}

void textEditorToolBar::setFontSize ( const int size )
{
	QTextCharFormat charFormat;
	charFormat.setFontPointSize ( static_cast<qreal> ( size ) );
	mergeFormatOnWordOrSelection ( charFormat );
}

void textEditorToolBar::setFontSizeFromComboBox ( const int index )
{
	setFontSize ( cboFontSizes->itemText ( index ).toInt () );
}

void textEditorToolBar::insertImage ( const QString& imageFile, QTextFrameFormat::Position pos )
{
	const uint key ( ++( mEditorWindow->m_ImageNumber ) );
	const QString imageKey ( stringFloatKey ( key ) );
	mEditorWindow->mapImages.insert ( imageKey, imageFile );
	QImage imagesrc ( imageFile );

	QTextFrameFormat referenceFrameFormat;
	referenceFrameFormat.setBorder ( 0 );
	referenceFrameFormat.setPadding ( 4 );
	referenceFrameFormat.setBottomMargin ( floatFromStringKey ( imageKey ) );
	referenceFrameFormat.setPosition ( pos );
	referenceFrameFormat.setWidth ( static_cast<qreal> ( imagesrc.size ().rwidth () / 3 ) );

	mEditorWindow->mCursor = mEditorWindow->mDocument->textCursor ();
	mEditorWindow->mCursor.insertBlock ();
	mEditorWindow->mCursor.insertBlock ();
	mEditorWindow->mCursor.movePosition ( QTextCursor::Up, QTextCursor::MoveAnchor, 2 );
	mEditorWindow->mCursor.insertFrame ( referenceFrameFormat );

	QTextCharFormat imageCharFormat;
	imageCharFormat.setObjectType ( QTextFormat::UserObject + 1 );
	imageCharFormat.setProperty ( 1, imagesrc );
	mEditorWindow->mCursor.insertText ( QString ( QChar::ObjectReplacementCharacter ), imageCharFormat );
	mEditorWindow->mCursor.movePosition ( QTextCursor::Down, QTextCursor::MoveAnchor );
	mergeFormatOnWordOrSelection ( imageCharFormat );
	mEditorWindow->mDocument->setFocus ();
}
