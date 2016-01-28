#include "documenteditorwindow.h"
#include "vmnotify.h"
#include "documenteditor.h"
#include "heapmanager.h"

#include <QDockWidget>
#include <QCloseEvent>

documentEditorWindow::documentEditorWindow ( documentEditor *parent )
	: QWidget (), mainLayout ( nullptr ), m_type ( 0 ), mb_untitled ( true ),
	  mb_programModification ( false ), mb_modified ( false ), mb_HasUndo ( false ),
	  mb_HasRedo ( false ), mb_inPreview ( false ), m_parentEditor ( parent )
{
	setAttribute ( Qt::WA_DeleteOnClose );

	mainLayout = new QGridLayout;
	mainLayout->setMargin ( 1 );
	mainLayout->setSpacing ( 1 );
	setLayout ( mainLayout );
}

documentEditorWindow::~documentEditorWindow ()
{
	heap_del ( mainLayout );
}

void documentEditorWindow::newFile ()
{
	canClose ();
	static uint sequenceNumber = 0;

	mb_untitled = true;
	mb_modified = false;

	curFile = tr ( "document%1" ).arg ( ++sequenceNumber );

	mb_programModification = true;
	createNew ();
	mb_programModification = false;
}

bool documentEditorWindow::load ( const QString& filename, const bool add_to_recent )
{
	if ( canClose () ) {
		mb_programModification = true;
		if ( loadFile ( filename ) ) {
			mb_programModification = false;
			setCurrentFile ( filename );
			if ( add_to_recent )
				parentEditor ()->addToRecentFiles ( filename );
			return true;
		}
		mb_programModification = false;
	}
	return false;
}

bool documentEditorWindow::save ( const QString& filename )
{
	bool ret ( false );
	if ( mb_modified ) {
		if ( !mb_untitled )
			ret = saveFile ( filename );
		else {
			EDITOR ()->saveAsDocument ();
			ret = !mb_untitled;
		}
		if ( ret )
			setCurrentFile ( filename );
	}
	return ret;
}

bool documentEditorWindow::saveas ( const QString& filename )
{
	const bool ret ( saveAs ( filename ) );
	if ( ret )
		setCurrentFile ( filename );
	return ret;
}

void documentEditorWindow::documentWasModified ()
{
	if ( !mb_programModification ) {
		/* When printing and previewing, we want to ignore the modified status. But a QTextEdit emits
		the documentModified signal more than once when previewing (don't know how many when printing only)
		and more than once it does it again when the preview dialog is closed. We need to count those calls
		and ignore them selectively. In my opinion Qt should drop those annoying habits (set the modified flag to
		true when printing and calling it so many times).
		 */
		static int wasInPreview ( 0 );
		if ( mb_inPreview ) {
			++wasInPreview;
			return;
		}
		if ( wasInPreview > 0 ) {
			--wasInPreview;
			return;
		}
		if ( !mb_modified ) {
			mb_modified = true;
			emit signalDocumentWasModified ( this );
		}
	}
}

void documentEditorWindow::documentWasModifiedByUndo ( const bool undo )
{
	if ( !mb_programModification )
		mb_HasUndo = undo;
}

void documentEditorWindow::documentWasModifiedByRedo ( const bool redo )
{
	if ( !mb_programModification ) {
		mb_HasRedo = redo;
		if ( redo && !mb_HasUndo ) {
			mb_modified = false;
			emit signalDocumentWasModified ( this );
		}
	}
}

void documentEditorWindow::cut () {}
void documentEditorWindow::copy () {}
void documentEditorWindow::paste () {}
void documentEditorWindow::buildMailMessage ( QString&  /*address*/, QString&  /*subject*/, QString&  /*attachment*/, QString&  /*body*/ ) {}

const QString documentEditorWindow::initialDir () const
{
	return QString::null;
}

void documentEditorWindow::setCurrentFile ( const QString& fileName )
{
	curFile = QFileInfo ( fileName ).canonicalFilePath ();
	mb_untitled = false;
	mb_modified = false;
	emit signalDocumentWasModified ( this );
}

/*void documentEditorWindow::closeEvent ( QCloseEvent* e )
{
	if ( maybeSave () )
		e->accept ();
	else
		e->ignore ();
}*/

bool documentEditorWindow::canClose ()
{
	if ( mb_modified ) {
		const int btn (
			VM_NOTIFY ()->customBox ( strippedName ( curFile ) + tr ( " was modified" ),  tr ( "Do you want to save your changes?" ),
								  vmNotify::QUESTION, tr ( "Save" ), tr ( "Cancel" ), tr ( " Discard" ) ) );

		switch ( btn ) {
			case 0:		return save ( curFile );	break;
			case -1:	return false;				break;
			default:								break;
		}
	}
	return true;
}
