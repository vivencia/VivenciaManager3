#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>
#include <QTabWidget>

#include "documenteditor.h"
#include "global.h"
#include "fileops.h"
#include "simplecalculator.h"
#include "configops.h"
#include "vmwidgets.h"
#include "system_init.h"
#include "documenteditorwindow.h"
#include "texteditor.h"
#include "reportgenerator.h"
#include "heapmanager.h"
#include "emailconfigdialog.h"

documentEditor* documentEditor::s_instance ( nullptr );

void deleteEditorInstance ()
{
	documentEditor::s_instance->closeAllTabs ();
	heap_del ( documentEditor::s_instance );
}

static const int MAX_RECENT_FILES ( 6 );
static const auto CFG_FIELD_RECENT_FILES ( QStringLiteral ( "RECENT" ) );
static const auto extSep ( QStringLiteral ( "*." ) );

inline static const QString buildFilter () { return	documentEditorWindow::filter () + QLatin1String ( ";;" ) + textEditor::filter () + 
			QLatin1String ( ";;" ) + reportGenerator::filter (); }

static heapManager<reportGenerator> heap_mngr;

inline const QString configFileName ()
{
	return CONFIG ()->defaultConfigDir () + QLatin1String ( "doc_editor.conf" );
}

documentEditor::~documentEditor ()
{
	static_cast<void>( disconnect () );
}

documentEditor::documentEditor ( QWidget* parent )
	: QMainWindow ( parent ), mb_ClosingAllTabs ( false ), recentFilesList ( emptyString, MAX_RECENT_FILES )
{
	setDockNestingEnabled ( true );
	setDockOptions ( QMainWindow::AnimatedDocks|QMainWindow::AllowNestedDocks|QMainWindow::AllowTabbedDocks|QMainWindow::ForceTabbedDocks|QMainWindow::VerticalTabs );

	tabDocuments = new QTabWidget;
	tabDocuments->setTabsClosable ( true );
	tabDocuments->setMovable ( true );
	setCentralWidget ( tabDocuments );
	static_cast<void>( connect ( tabDocuments, &QTabWidget::currentChanged, this, [&] ( const int index ) { return updateMenus ( index ); } ) );
	static_cast<void>( connect ( tabDocuments, &QTabWidget::tabCloseRequested, this, [&] ( const int index ) { return closeTab ( index ); } ) );

	createActions ();
	createMenus ();
	createToolBars ();
	createStatusBar ();
	updateMenus ( -1 );

	setWindowTitle ( PROGRAM_NAME + TR_FUNC ( " - Document Editor" ) );
	setWindowIcon ( ICON ( "report" ) );

	resize ( 500, 450 );
	Sys_Init::addPostRoutine ( deleteEditorInstance );
}

void documentEditor::addDockWindow ( Qt::DockWidgetArea area, QDockWidget* dockwidget )
{
	if ( area != Qt::NoDockWidgetArea )
	{
		dockwidget->setFloating ( false );
		addDockWidget ( area, dockwidget );
	}
	else
		dockwidget->setFloating ( true );
}

void documentEditor::removeDockWindow ( QDockWidget *dockwidget )
{
	removeDockWidget ( dockwidget );
}

void documentEditor::createActions ()
{
	newAct = new vmAction ( -1, ( ICON ( "document-new" ) ), TR_FUNC ( "&New text file" ), this );
	newAct->setShortcut ( QKeySequence::New );
	newAct->setStatusTip ( TR_FUNC ( "Create a new document" ) );
	static_cast<void>( connect ( newAct, &QAction::triggered, this, [&] ( const bool ) { return startNewTextEditor ()->show (); } ) );

	newReportAct = new vmAction ( -1, ( ICON ( "report" ) ), TR_FUNC ( "New &Report" ), this );
	newReportAct->setShortcut ( QKeySequence::Refresh );
	newReportAct->setStatusTip ( TR_FUNC ( "Create a new report" ) );
	static_cast<void>( connect ( newReportAct, &QAction::triggered, this, [&] ( const bool ) { return startNewReport ()->show (); } ) );

	openAct = new vmAction ( -1, ICON ( "document-open" ), TR_FUNC ( "&Open..." ), this );
	openAct->setShortcut ( QKeySequence::Open );
	openAct->setStatusTip ( TR_FUNC ( "Open an existing file" ) );
	static_cast<void>( connect ( openAct, &QAction::triggered, this, [&] ( const bool ) { return openDocument (); } ) );

	saveAct = new vmAction ( -1, ICON ( "document-save" ), TR_FUNC ( "&Save" ), this );
	saveAct->setShortcut ( QKeySequence::Save );
	saveAct->setStatusTip ( TR_FUNC ( "Save the document to disk" ) );
	static_cast<void>( connect ( saveAct, &QAction::triggered, this, [&] ( const bool ) { return saveDocument (); } ) );

	saveAsAct = new vmAction ( -1, ICON ( "document-save-as" ), TR_FUNC ( "Save &as..." ), this );
	saveAsAct->setShortcut ( QKeySequence::SaveAs );
	saveAsAct->setStatusTip ( TR_FUNC ( "Save the document under a new name" ) );
	static_cast<void>( connect ( saveAsAct, &QAction::triggered, this, [&] ( const bool ) { return saveAsDocument (); } ) );

	mailAct = new vmAction ( -1, ICON ( "email" ), TR_FUNC ( "Send via email" ), this );
	mailAct->setStatusTip ( TR_FUNC ( "Send the document as an email attachment unless the editor provides another option" ) );
	static_cast<void>( connect ( mailAct, &QAction::triggered, this, [&] ( const bool ) { return sendMailAttachment (); } ) );

	cutAct = new vmAction ( -1, ICON ( "edit-cut" ), TR_FUNC ( "Cu&t" ), this );
	cutAct->setShortcut ( QKeySequence::Cut );
	cutAct->setStatusTip ( TR_FUNC ( "Cut the current selection's contents to the clipboard" ) );
	static_cast<void>( connect ( cutAct, &QAction::triggered, this, [&] ( const bool ) { if ( activeDocumentWindow () ) return activeDocumentWindow ()->cut (); } ) );

	copyAct = new vmAction ( -1, ICON ( "edit-copy" ), TR_FUNC ( "&Copy" ), this );
	copyAct->setShortcut ( QKeySequence::Copy );
	copyAct->setStatusTip ( TR_FUNC ( "Copy the current selection's contents to the clipboard" ) );
	static_cast<void>( connect ( copyAct, &QAction::triggered, this, [&] ( const bool ) { if ( activeDocumentWindow () ) return activeDocumentWindow ()->copy (); } ) );

	pasteAct = new vmAction ( -1, ICON ( "edit-paste" ), TR_FUNC ( "&Paste" ), this );
	pasteAct->setShortcut ( QKeySequence::Paste );
	pasteAct->setStatusTip ( TR_FUNC ( "Paste the clipboard's contents into the current selection" ) );
	static_cast<void>( connect ( pasteAct, &QAction::triggered, this, [&] ( const bool ) { if ( activeDocumentWindow () ) return activeDocumentWindow ()->paste (); } ) );

	closeAct = new vmAction ( -1, ICON ( "close-active-window" ), TR_FUNC ( "Cl&ose" ), this );
	closeAct->setShortcut ( QKeySequence::Close );
	closeAct->setStatusTip ( TR_FUNC ( "Close the active window" ) );
	static_cast<void>( connect ( closeAct, &QAction::triggered, this, [&] ( const bool ) { return closeTab ( tabDocuments->currentIndex () ); } ) );

	closeAllAct = new vmAction ( -1, ICON ( "close-all" ), TR_FUNC ( "Close &All" ), this );
	closeAllAct->setStatusTip ( TR_FUNC ( "Close all the windows" ) );
	static_cast<void>( connect ( closeAllAct, &QAction::triggered, this, [&] ( const bool ) { return closeAllTabs (); } ) );

	hideAct = new vmAction ( -1, ICON ( "window-close" ), TR_FUNC ( "Close editor window" ), this );
	hideAct->setShortcut ( QKeySequence::Quit );
	hideAct->setStatusTip ( TR_FUNC ( "Close Document Editor window" ) );
	static_cast<void>( connect ( hideAct, &QAction::triggered, this, [&] ( const bool ) { return hide (); } ) );

	nextAct = new vmAction ( -1, TR_FUNC ( "Ne&xt" ), this );
	nextAct->setShortcut ( QKeySequence::NextChild );
	nextAct->setStatusTip ( TR_FUNC ( "Move the focus to the next window" ) );
	static_cast<void>( connect ( nextAct, &QAction::triggered, this, [&] ( const bool ) { return activateNextTab (); } ) );

	previousAct = new vmAction ( -1, TR_FUNC ( "Pre&vious" ), this );
	previousAct->setShortcut ( QKeySequence::PreviousChild );
	previousAct->setStatusTip ( TR_FUNC ( "Move the focus to the previous window" ) );
	static_cast<void>( connect ( previousAct, &QAction::triggered, this, [&] ( const bool ) { return activatePreviousTab (); } ) );

	separatorAct = new vmAction ( -1, this );
	separatorAct->setSeparator ( true );

	calcAct = new vmAction ( -1, ICON ( "calc" ), TR_FUNC ( "Show calculator" ), this );
	calcAct->setStatusTip ( TR_FUNC ( "Show the calculator window" ) );
	static_cast<void>( connect ( calcAct, &QAction::triggered, this, [&] ( const bool ) { return CALCULATOR ()->showCalc ( mapToGlobal ( pos () ), nullptr, this ); } ) );
}

void documentEditor::createMenus ()
{
	fileMenu = menuBar ()->addMenu ( TR_FUNC ( "&File" ) );
	fileMenu->addAction ( newAct );
	fileMenu->addAction ( newReportAct );
	fileMenu->addAction ( openAct );

	recentFilesSubMenu = new QMenu ( TR_FUNC ( "Recent documents" ) );
	static_cast<void>( connect ( recentFilesSubMenu, &QMenu::triggered, this, [&] ( QAction* action ) { return openRecentFile ( action ); } ) );
	recentFilesList.fromString ( configOps::readConfig ( configFileName (), CFG_FIELD_RECENT_FILES ) );
	if ( recentFilesList.first () )
	{
		do {
			addToRecentFiles ( recentFilesList.curValue (), false );
		} while ( recentFilesList.next () );
	}
	fileMenu->addMenu ( recentFilesSubMenu );
	fileMenu->addSeparator ();

	fileMenu->addAction ( saveAct );
	fileMenu->addAction ( saveAsAct );
	fileMenu->addSeparator ();
	fileMenu->addAction ( closeAllAct );
	fileMenu->addSeparator ();
	fileMenu->addAction ( hideAct );

	editMenu = menuBar ()->addMenu ( TR_FUNC ( "&Edit" ) );
	editMenu->addAction ( cutAct );
	editMenu->addAction ( copyAct );
	editMenu->addAction ( pasteAct );

	windowMenu = menuBar ()->addMenu ( TR_FUNC ( "&Window" ) );
	static_cast<void>( connect ( windowMenu, &QMenu::triggered, this, [&] ( QAction* action ) { return makeWindowActive ( action ); } ) );
	updateWindowMenu ();
	static_cast<void>( connect ( windowMenu, &QMenu::aboutToShow, this, [&] () { return updateWindowMenu (); } ) );
}

void documentEditor::createToolBars ()
{
	fileToolBar = addToolBar ( TR_FUNC ( "File" ) );
	fileToolBar->addAction ( newAct );
	fileToolBar->addAction ( newReportAct );
	fileToolBar->addAction ( openAct );
	fileToolBar->addAction ( separatorAct );
	fileToolBar->addAction ( saveAct );
	fileToolBar->addAction ( saveAsAct );
	fileToolBar->addAction ( separatorAct );
	fileToolBar->addAction ( closeAct );
	fileToolBar->addAction ( separatorAct );
	fileToolBar->addAction ( calcAct );
	fileToolBar->addAction ( separatorAct );

	editToolBar = addToolBar ( TR_FUNC ( "Edit" ) );
	editToolBar->addAction ( cutAct );
	editToolBar->addAction ( copyAct );
	editToolBar->addAction ( pasteAct );
}

void documentEditor::createStatusBar ()
{
	statusBar ()->showMessage ( TR_FUNC ( "Ready" ) );
}

void documentEditor::updateMenus ( const int tab_index )
{
	const bool hasMdiChild ( tab_index != -1 );
	saveAct->setEnabled ( hasMdiChild );
	saveAsAct->setEnabled ( hasMdiChild );
	pasteAct->setEnabled ( hasMdiChild );
	closeAct->setEnabled ( hasMdiChild );
	closeAllAct->setEnabled ( hasMdiChild );
	nextAct->setEnabled ( hasMdiChild );
	previousAct->setEnabled ( hasMdiChild );
	separatorAct->setVisible ( hasMdiChild );

	if ( hasMdiChild )
	{
		documentEditorWindow* dew ( static_cast<documentEditorWindow*>( tabDocuments->widget ( tab_index ) ) );
		if ( dew != nullptr )
		{
			if ( dew->editorType () & TEXT_EDITOR_SUB_WINDOW )
				TEXT_EDITOR_TOOLBAR ()->setCurrentWindow ( static_cast<textEditor*> ( dew ) );
			if ( dew->editorType () & REPORT_GENERATOR_SUB_WINDOW )
			{
				DOCK_QP ()->setCurrentWindow ( static_cast<reportGenerator*>( dew ) );
				DOCK_BJ ()->setCurrentWindow ( static_cast<reportGenerator*>( dew ) );
			}
			saveAct->setEnabled ( dew->isModified () );
		}
	}
}

void documentEditor::updateWindowMenu ()
{
	windowMenu->clear ();
	windowMenu->addAction ( closeAct );
	windowMenu->addAction ( closeAllAct );
	windowMenu->addSeparator ();
	windowMenu->addAction ( nextAct );
	windowMenu->addAction ( previousAct );
	windowMenu->addAction ( separatorAct );

	const uint open_tabs ( static_cast<uint>( tabDocuments->count () ) );
	separatorAct->setVisible ( open_tabs > 0 );

	if ( open_tabs > 0 )
	{
		QString text;
		documentEditorWindow* child ( nullptr );

		for ( uint i = 0; i < open_tabs; ++i )
		{
			child = static_cast<documentEditorWindow*>(tabDocuments->widget ( static_cast<int>( i ) ));
			if ( child )
			{
				text =  (i < 9 ? TR_FUNC ( "&%1 %2" ) : TR_FUNC ( "%1 %2" )).arg ( i + 1 ).arg ( child->title () );
				vmAction* action ( new vmAction ( static_cast<int>( i ), text, this ) );
				action->setCheckable ( true );
				action->setChecked ( child == activeDocumentWindow () );
				windowMenu->addAction ( action );
			}
		}
	}
}

void documentEditor::makeWindowActive ( QAction* action )
{
	const int window_id ( static_cast<vmAction*>( action )->id () );
	if ( window_id < 0 )
		return;
	tabDocuments->setCurrentIndex ( window_id );
}

void documentEditor::changeTabText ( documentEditorWindow* window )
{
	if ( tabDocuments->currentWidget () == static_cast<QWidget*>( window ) )
		tabDocuments->setTabText ( tabDocuments->currentIndex (), window->title () );
	saveAct->setEnabled ( window->isModified () );
}

void documentEditor::closeTab ( int tab_index )
{
	if ( tab_index == -1 )
	{
		tab_index = tabDocuments->currentIndex ();
		if ( tab_index == -1 ) //no tabs
			return;
	}
	if ( tab_index < tabDocuments->count () )
	{
		if ( static_cast<documentEditorWindow*>( tabDocuments->widget ( tab_index ) )->canClose () )
		{
			QWidget* doc ( tabDocuments->widget ( tab_index ) );
			tabDocuments->removeTab ( tab_index );
			if ( !mb_ClosingAllTabs )
			{
				updateWindowMenu ();
				updateMenus ( tabDocuments->currentIndex () );
			}
			delete doc;
		}
	}
}

void documentEditor::closeAllTabs ()
{
	mb_ClosingAllTabs = true;
	int i ( tabDocuments->count () - 1 );
	for ( ; i >= 0; --i )
	{
		closeTab ( i );
	}
	if ( i == 0 )
	{
		updateWindowMenu ();
		updateMenus ( -1 );	
	}
	mb_ClosingAllTabs = false;
	// Mainwindow, when it receives the signal indicating the program is about to be closed
	// will call here. To avoid a destructor, put clean up code here
	if ( Sys_Init::EXITING_PROGRAM )
		recentFilesList.clear ();
}

void documentEditor::activateNextTab ()
{
	const int tab_index ( tabDocuments->currentIndex () );
	if ( tab_index <= ( tabDocuments->count () - 1 ) )
		tabDocuments->setCurrentIndex ( tab_index + 1 );
}

void documentEditor::activatePreviousTab ()
{
	const int tab_index ( tabDocuments->currentIndex () );
	if ( tab_index > 0 )
		tabDocuments->setCurrentIndex ( tab_index - 1 );
}

void documentEditor::closeEvent ( QCloseEvent* ce )
{
	closeAllTabs ();
	ce->accept ();
}

textEditor* documentEditor::startNewTextEditor ( textEditor* editor )
{
	if ( editor == nullptr )
		editor = new textEditor ( this );
	editor->setCallbackForDocumentModified ( [&] ( documentEditorWindow* dew ) { return changeTabText ( dew ); } );
	const int tab_id ( tabDocuments->addTab ( editor, emptyString ) );
	tabDocuments->setCurrentIndex ( tab_id );
	editor->newFile ();
	tabDocuments->setTabText ( tab_id, editor->title () );
	resizeViewPort ( editor );
	return editor;
}

reportGenerator* documentEditor::startNewReport ( const bool b_windowless )
{
	reportGenerator* report ( new reportGenerator ( this ) );
	if ( !b_windowless )
	{
		static_cast<void>( startNewTextEditor ( report ) );
	}
	else
	{
		// In a windowless, we must manage the pointers and delete when we are destroyed
		heap_mngr.register_use ( const_cast<reportGenerator*>( report ) );
		TEXT_EDITOR_TOOLBAR ()->setCurrentWindow ( report );

	}
	return report;
}

void documentEditor::resizeViewPort ( documentEditorWindow* window )
{
	QSize newSize;
	if ( tabDocuments->width () < window->width () )
		newSize.setWidth ( width () + ( window->width () - tabDocuments->width () ) );
	if ( tabDocuments->height () < window->height () )
		newSize.setHeight ( height () + ( window->height () - tabDocuments->height () ) );
	window->setFocus ();
}

uint documentEditor::openByFileType ( const QString& filename )
{
	const QString openFileExt ( fileOps::fileExtension ( filename ) );
	const QString filter ( buildFilter () );
	QString filterExt ( emptyString );
	int idx ( filter.indexOf ( extSep, 0 ) );
	uint type ( 1 );

	while ( idx != -1 )
	{
		filterExt = filter.mid ( idx + 2, filter.indexOf ( CHR_R_PARENTHESIS, idx + 1 ) - idx - 2 );
		if ( !filterExt.isEmpty () )
		{
			if ( filterExt == openFileExt )
				return type;
			type <<= 1;
		}
		idx = filter.indexOf ( extSep, idx + 1 );
	}
	return type;
}

void documentEditor::openDocument ()
{
	static QString initialDir ( emptyString );
	const QString filter ( buildFilter () );
	if ( initialDir.isEmpty () )
	{
		documentEditorWindow* window ( activeDocumentWindow () );
		if ( window != nullptr )
			initialDir = window->initialDir ();
		else
			initialDir = CHR_TILDE;
	}

	const QString filename ( fileOps::getOpenFileName ( initialDir, filter ) );
	if ( !filename.isEmpty () )
	{
		initialDir = fileOps::dirFromPath ( filename );
		openDocument ( filename );
	}
}

void documentEditor::openDocument ( const QString& filename )
{
	documentEditorWindow* window ( activeDocumentWindow () );
	if ( ( window == nullptr ) || ( window && ( !window->isUntitled () | window->isModified () ) ) ) {
		switch ( openByFileType ( filename ) )
		{
			default: // 1, unknown file type
				return;
			
			case TEXT_EDITOR_SUB_WINDOW:
			case RICH_TEXT_EDITOR_SUB_WINDOW:
				window = startNewTextEditor ();
			break;
			case REPORT_GENERATOR_SUB_WINDOW:
				window = startNewReport ();
			break;
		}
	}
	if ( window->load ( filename ) )
	{
		statusBar ()->showMessage ( TR_FUNC ( "File loaded" ), 2000 );
		tabDocuments->setTabText ( tabDocuments->currentIndex (), window->title () );
		addToRecentFiles ( filename );
	}
	window->show ();
	window->setFocus ();
}

void documentEditor::saveDocument ()
{
	if ( activeDocumentWindow () != nullptr )
	{
		documentEditorWindow* window ( activeDocumentWindow () );
		if ( window->isUntitled () )
			saveAsDocument ();
		else
		{
			if ( window->save ( window->currentFile () ) ) {
				statusBar ()->showMessage ( TR_FUNC ( "File saved" ), 2000 );
				tabDocuments->setTabText ( tabDocuments->currentIndex (), window->title () );
			}
			window->setFocus ();
		}
	}
}

void documentEditor::saveAsDocument ()
{
	if ( activeDocumentWindow () )
	{
		documentEditorWindow* window ( activeDocumentWindow () );
		const QString initialDir ( window->initialDir () );
		const QString filter ( buildFilter () );
		const QString filename ( fileOps::getSaveFileName ( initialDir, filter ) );
		if ( !filename.isEmpty () )
		{
			if ( window->saveas ( filename ) )
			{
				statusBar ()->showMessage ( TR_FUNC ( "File saved" ), 2000 );
				tabDocuments->setTabText ( tabDocuments->currentIndex (), window->title () );
				addToRecentFiles ( filename );
			}
		}
		window->setFocus ();
	}
}

void documentEditor::sendMailAttachment ()
{
	if ( activeDocumentWindow () )
	{
		QString address, subject, body, attachment;
		activeDocumentWindow ()->buildMailMessage ( address, subject, body, attachment );
		EMAIL_CONFIG ()->sendEMail ( address, subject, body, attachment );
	}
}

void documentEditor::addToRecentFiles ( const QString& filename, const bool b_AddToList )
{
	const QString menuText ( fileOps::fileNameWithoutPath ( filename ) );
	for ( int i ( 0 ); i < recentFilesSubMenu->actions ().count (); ++i )
	{
		if ( menuText == recentFilesSubMenu->actions ().at ( i )->text () )
			return;
	}

	const int n_menus ( recentFilesSubMenu->actions ().count () );
	if ( n_menus == MAX_RECENT_FILES )
	{
		recentFilesSubMenu->removeAction ( recentFilesSubMenu->actions ().at ( n_menus - 1 ) );
		recentFilesList.removeField ( static_cast<uint>(n_menus) - 1 );
	}
	vmAction* filemenu ( new vmAction ( -1, menuText ) );
	filemenu->setData ( filename );
	recentFilesSubMenu->addAction ( filemenu );

	if ( b_AddToList )
	{
		recentFilesList.fastAppendValue ( filename );
		configOps::writeConfig ( configFileName (), CFG_FIELD_RECENT_FILES, recentFilesList.toString () );
	}
}

void documentEditor::openRecentFile ( QAction* selectedMenu )
{
	const vmAction* action ( static_cast<vmAction*>( selectedMenu ) );
	openDocument ( action->data ().toString () );
}
