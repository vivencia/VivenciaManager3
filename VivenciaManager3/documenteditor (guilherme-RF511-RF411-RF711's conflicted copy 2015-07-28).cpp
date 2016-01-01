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
#include "data.h"
#include "documenteditorwindow.h"
#include "texteditor.h"
#include "reportgenerator.h"
#include "cleanup.h"
#include "heapmanager.h"
#include "emailconfigdialog.h"

documentEditor* documentEditor::s_instance = nullptr;

void deleteEditorInstance ()
{
    heap_del ( documentEditor::s_instance );
}

const int MAX_RECENT_FILES ( 6 );
const char* const CFG_FIELD_RECENT_FILES ( "RECENT" );
const char* const extSep ( "*." );
char const CFG_FIELD_RECENT_FILES_SEP ( ',' );

static heapManager<reportGenerator> heap_mngr;

const QString configFileName ()
{
    QString filename ( CONFIG ()->defaultConfigDir () );
    filename.append ( "doc_editor.conf" );
    return filename;
}

const QString joinRecentFilesList ( const VMList<QString>& list )
{
    QString res;
    for ( uint i ( 0 ); i < list.count (); ++i )
        res += list.at ( i ) + CFG_FIELD_RECENT_FILES_SEP;
    return res;
}

void listFromString ( VMList<QString>& list, const QString& str )
{
    int idx ( 0 );
    int idx2 ( 0 );
    QString temp;
    do {
        idx2 = str.indexOf ( CFG_FIELD_RECENT_FILES_SEP, idx );
        if ( idx2 != -1 ) {
            temp = str.mid ( idx, idx2 - idx );
            list.append ( temp );
            idx = idx2 + 1;
        }
        else
            break;
    } while ( true );
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
	connect ( tabDocuments, &QTabWidget::currentChanged, this, [&] ( const int index ) { return updateMenus ( index ); } );
	connect ( tabDocuments, &QTabWidget::tabCloseRequested, this, [&] ( const int index ) { return closeTab ( index ); } );

    createActions ();
    createMenus ();
    createToolBars ();
    createStatusBar ();
    updateMenus ( -1 );

    setWindowTitle ( PROGRAM_NAME + tr ( " - Document Editor" ) );
    setWindowIcon ( ICON ( "report.png" ) );

    resize ( 500, 450 );
    addPostRoutine ( deleteEditorInstance );
}

void documentEditor::addDockWindow ( Qt::DockWidgetArea area, QDockWidget* dockwidget )
{
    if ( area != Qt::NoDockWidgetArea ) {
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
    newAct = new vmAction ( -1, ( ICON ( "document-new.png" ) ), tr ( "&New text file" ), this );
    newAct->setShortcuts ( QKeySequence::New );
    newAct->setStatusTip ( tr ( "Create a new document" ) );
	connect ( newAct, &QAction::triggered, this, [&] ( const bool ) { return newTextDocument (); } );

    newReportAct = new vmAction ( -1, ( ICON ( "report.png" ) ), tr ( "New &Report" ), this );
    newReportAct->setShortcuts ( QKeySequence::Refresh );
    newReportAct->setStatusTip ( tr ( "Create a new report" ) );
	connect ( newReportAct, &QAction::triggered, this, [&] ( const bool ) { return newReport (); } );

    openAct = new vmAction ( -1, ICON ( "document-open.png" ), tr ( "&Open..." ), this );
    openAct->setShortcuts ( QKeySequence::Open );
    openAct->setStatusTip ( tr ( "Open an existing file" ) );
	connect ( openAct, &QAction::triggered, this, [&] ( const bool ) { return openDocument (); } );

    saveAct = new vmAction ( -1, ICON ( "document-save.png" ), tr ( "&Save" ), this );
    saveAct->setShortcuts ( QKeySequence::Save );
    saveAct->setStatusTip ( tr ( "Save the document to disk" ) );
	connect ( saveAct, &QAction::triggered, this, [&] ( const bool ) { return saveDocument (); } );

    saveAsAct = new vmAction ( -1, ICON ( "document-save-as.png" ), tr ( "Save &as..." ), this );
    saveAsAct->setShortcuts ( QKeySequence::SaveAs );
    saveAsAct->setStatusTip ( tr ( "Save the document under a new name" ) );
	connect ( saveAsAct, &QAction::triggered, this, [&] ( const bool ) { return saveAsDocument (); } );

    mailAct = new vmAction ( -1, ICON ( "email.png" ), tr ( "Send via email" ), this );
    mailAct->setStatusTip ( tr ( "Send the document as an email attachment unless the editor provides another option" ) );
	connect ( mailAct, &QAction::triggered, this, [&] ( const bool ) { return sendMailAttachment (); } );

    cutAct = new vmAction ( -1, ICON ( "edit-cut.png" ), tr ( "Cu&t" ), this );
    cutAct->setShortcuts ( QKeySequence::Cut );
    cutAct->setStatusTip ( tr ( "Cut the current selection's contents to the clipboard" ) );
	connect ( cutAct, &QAction::triggered, this, [&] ( const bool ) { return cut (); } );

    copyAct = new vmAction ( -1, ICON ( "edit-copy.png" ), tr ( "&Copy" ), this );
    copyAct->setShortcuts ( QKeySequence::Copy );
    copyAct->setStatusTip ( tr ( "Copy the current selection's contents to the clipboard" ) );
	connect ( copyAct, &QAction::triggered, this, [&] ( const bool ) { return copy (); } );

    pasteAct = new vmAction ( -1, ICON ( "edit-paste.png" ), tr ( "&Paste" ), this );
    pasteAct->setShortcuts ( QKeySequence::Paste );
    pasteAct->setStatusTip ( tr ( "Paste the clipboard's contents into the current selection" ) );
	connect ( pasteAct, &QAction::triggered, this, [&] ( const bool ) { return paste (); } );

    closeAct = new vmAction ( -1, ICON ( "close-active-window.png" ), tr ( "Cl&ose" ), this );
    closeAct->setStatusTip ( tr ( "Close the active window" ) );
	connect ( closeAct, &QAction::triggered, this, [&] ( const bool ) { return closeTab (); } );

    closeAllAct = new vmAction ( -1, ICON ( "close-all.png" ), tr ( "Close &All" ), this );
    closeAllAct->setStatusTip ( tr ( "Close all the windows" ) );
	connect ( closeAllAct, &QAction::triggered, this, [&] ( const bool ) { return closeAllTabs (); } );

    hideAct = new vmAction ( -1, tr ( "Leave" ), this);
    hideAct->setShortcuts ( QKeySequence::Quit );
    hideAct->setStatusTip ( tr ( "Close Document Editor window" ) );
	connect ( hideAct, &QAction::triggered, this, [&] ( const bool ) { return hide (); } );

    nextAct = new vmAction ( -1, tr ( "Ne&xt" ), this );
    nextAct->setShortcuts ( QKeySequence::NextChild );
    nextAct->setStatusTip ( tr ( "Move the focus to the next window" ) );
	connect ( nextAct, &QAction::triggered, this, [&] ( const bool ) { return activateNextTab (); } );

    previousAct = new vmAction ( -1, tr ( "Pre&vious" ), this );
    previousAct->setShortcuts ( QKeySequence::PreviousChild );
    previousAct->setStatusTip ( tr ( "Move the focus to the previous window" ) );
	connect ( previousAct, &QAction::triggered, this, [&] ( const bool ) { return activatePreviousTab (); } );

    separatorAct = new vmAction ( -1, this );
    separatorAct->setSeparator ( true );

    calcAct = new vmAction ( -1, ICON ( "calc.png" ), tr ( "Show calculator" ), this );
    calcAct->setStatusTip ( tr ( "Show the calculator window" ) );
	connect ( calcAct, &QAction::triggered, this, [&] ( const bool ) { return showCalc (); } );
}

void documentEditor::createMenus ()
{
    fileMenu = menuBar ()->addMenu ( tr ( "&File" ) );
    fileMenu->addAction ( newAct );
    fileMenu->addAction ( newReportAct );
    fileMenu->addAction ( openAct );

    recentFilesSubMenu = new QMenu ( tr ( "Recent documents" ) );
    connect ( recentFilesSubMenu, SIGNAL ( triggered ( QAction * ) ), this, SLOT ( openRecentFile ( QAction* ) ) );
    listFromString ( recentFilesList, configOps::readConfig ( configFileName (), CFG_FIELD_RECENT_FILES ) );
    if ( !recentFilesList.isEmpty () ) {
        for ( uint i ( 0 ); i < recentFilesList.count (); ++i )
            addToRecentFiles ( recentFilesList.at ( i ), false );
    }
    fileMenu->addMenu ( recentFilesSubMenu );
    fileMenu->addSeparator ();

    fileMenu->addAction ( saveAct );
    fileMenu->addAction ( saveAsAct );
    fileMenu->addSeparator ();
    fileMenu->addAction ( closeAllAct );
    fileMenu->addSeparator ();
    fileMenu->addAction ( hideAct );

    editMenu = menuBar ()->addMenu ( tr ( "&Edit" ) );
    editMenu->addAction ( cutAct );
    editMenu->addAction ( copyAct );
    editMenu->addAction ( pasteAct );

    windowMenu = menuBar ()->addMenu ( tr ( "&Window" ) );
    connect ( windowMenu, SIGNAL ( triggered ( QAction* ) ), this, SLOT ( makeWindowActive ( QAction* ) ) );
    updateWindowMenu ();
    connect ( windowMenu, SIGNAL ( aboutToShow () ), this, SLOT ( updateWindowMenu () ) );
}

void documentEditor::createToolBars ()
{
    fileToolBar = addToolBar ( tr ( "File" ) );
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

    editToolBar = addToolBar ( tr ( "Edit" ) );
    editToolBar->addAction ( cutAct );
    editToolBar->addAction ( copyAct );
    editToolBar->addAction ( pasteAct );
}

void documentEditor::createStatusBar ()
{
    statusBar ()->showMessage ( tr ( "Ready" ) );
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

    if ( hasMdiChild ) {
        documentEditorWindow* dew = static_cast<documentEditorWindow*> ( tabDocuments->widget ( tab_index ) );
        if ( dew != nullptr ) {
            if ( dew->editorType () & TEXT_EDITOR_SUB_WINDOW )
                TEXT_EDITOR_TOOLBAR ()->setCurrentWindow ( static_cast<textEditor*> ( dew ) );
            if ( dew->editorType () & REPORT_GENERATOR_SUB_WINDOW ) {
                DOCK_QP ()->setCurrentWindow ( static_cast<reportGenerator*> ( dew ) );
                DOCK_BJ ()->setCurrentWindow ( static_cast<reportGenerator*> ( dew ) );
            }
            if ( dew->isUntitled () )
                saveAct->setEnabled ( false );
            else
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

    const uint open_tabs ( unsigned ( tabDocuments->count () ) );
    separatorAct->setVisible ( open_tabs > 0 );

    if ( open_tabs > 0 ) {
        QString text;
        documentEditorWindow* child = NULL;

        for ( uint i = 0; i < open_tabs; ++i ) {
            child = static_cast<documentEditorWindow*> ( tabDocuments->widget ( i ) );
            if ( child == NULL ) continue;

            if ( i < 9 )
                text = tr ( "&%1 %2" ).arg ( i + 1 ).arg ( child->title () );
            else
                text = tr ( "%1 %2" ) .arg ( i + 1 ).arg ( child->title () );

            vmAction* action = new vmAction ( i, text, this );
            action->setCheckable ( true );
            action->setChecked ( child == activeDocumentWindow () );
            windowMenu->addAction ( action );
        }
    }
}

void documentEditor::makeWindowActive ( QAction* action )
{
    const int window_id ( static_cast<vmAction*> ( action )->id () );
    if ( window_id < 0 )
        return;
    tabDocuments->setCurrentIndex ( window_id );
}

void documentEditor::changeTabText ( documentEditorWindow* window )
{
    tabDocuments->setTabText ( tabDocuments->currentIndex (), window->title () );
    saveAct->setEnabled ( !window->isUntitled () );
}

void documentEditor::closeTab ( int tab_index )
{
    if ( !mb_ClosingAllTabs ) {
        if ( tab_index == -1 ) {
            tab_index = tabDocuments->currentIndex ();
            if ( tab_index == -1 ) //no tabs
                return;
        }

        if ( tab_index < tabDocuments->count () ) {
            if ( static_cast<documentEditorWindow*> ( tabDocuments->widget ( tab_index ) )->canClose () ) {
                QWidget* doc ( tabDocuments->widget ( tab_index ) );
                tabDocuments->removeTab ( tab_index );
                updateWindowMenu ();
                updateMenus ( tabDocuments->currentIndex () );
                delete doc;
            }
        }
    }
}

void documentEditor::closeAllTabs ()
{
    mb_ClosingAllTabs = true;
    int i = tabDocuments->count () - 1;
    for ( ; i >= 0; --i ) {
        if ( static_cast<documentEditorWindow*> ( tabDocuments->widget ( i ) )->canClose () )
            tabDocuments->removeTab ( i );
    }
    if ( i == 0 ) {
        updateWindowMenu ();
        updateMenus ( -1 );
    }
    mb_ClosingAllTabs = false;
    // Mainwindow, when it receives the signal indicating the program is about to be closed
    // will call here. To avoid a destructor, put clean up code here
    if ( EXITING_PROGRAM )
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

void documentEditor::showCalc ()
{
    CALCULATOR ()->showCalc ( mapToGlobal ( pos () ), NULL, this );
}

void documentEditor::newTextDocument ()
{
    startNewTextEditor ();
}

textEditor* documentEditor::startNewTextEditor ()
{
    textEditor* editor = new textEditor ( this );
    connect ( editor, SIGNAL ( signalDocumentWasModified ( documentEditorWindow* ) ), this, SLOT ( changeTabText ( documentEditorWindow* ) ) );
    const uint tab_id ( tabDocuments->addTab ( editor, "" ) );
    tabDocuments->setCurrentIndex ( tab_id );
    editor->newFile ();
    tabDocuments->setTabText ( tab_id, editor->title () );
    resizeViewPort ( editor );
    return editor;
}

void documentEditor::newReport ()
{
    startNewReport ()->show ();
}

reportGenerator* documentEditor::startNewReport ( const bool b_windowless )
{
    reportGenerator* report = new reportGenerator ( this );
    if ( !b_windowless ) {
        // In a windowfull editor, closeTab will delete the instance created here.
        connect ( report, SIGNAL ( signalDocumentWasModified ( documentEditorWindow* ) ), this, SLOT ( changeTabText ( documentEditorWindow* ) ) );
        const uint tab_id ( tabDocuments->addTab ( report, "" ) );
        tabDocuments->setCurrentIndex ( tab_id );
        report->newFile ();
        tabDocuments->setTabText ( tab_id, report->title () );
        resizeViewPort ( report );
    }
    else {
        // In a windowless, we must manage the pointers and delete then we are destroyed
        heap_mngr.register_use ( const_cast<reportGenerator*> ( report ) );
        TEXT_EDITOR_TOOLBAR ()->setCurrentWindow ( report );

    }
	show ();
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

QString documentEditor::buildFilter () const
{
    QString ret ( documentEditorWindow::filter () );
    ret += ";;";
    ret += textEditor::filter ();
    ret += ";;";
    ret += reportGenerator::filter ();
    return ret;
}

uint documentEditor::openByFileType ( const QString& filename )
{
    const QString openFileExt ( fileOps::fileExtension ( filename ) );
    const QString filter ( buildFilter () );
    QString filterExt ( emptyString );
    int idx ( filter.indexOf ( extSep, 0 ) );
    uint type ( 1 );

    while ( idx != -1 ) {
        filterExt = filter.mid ( idx + 2, filter.indexOf ( ')', idx + 1 ) - idx - 2 );
        if ( !filterExt.isEmpty () ) {
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
    if ( initialDir.isEmpty () ) {
        documentEditorWindow* window = activeDocumentWindow ();
        if ( window != NULL )
            initialDir = window->initialDir ();
        else
            initialDir = "~";
    }

    const QString filename ( Data::getOpenFileName ( initialDir, filter ) );
    if ( !filename.isEmpty () ) {
        initialDir = fileOps::dirFromPath ( filename );
        openDocument ( filename );
    }
}

void documentEditor::openDocument ( const QString& filename )
{
    documentEditorWindow* window = activeDocumentWindow ();
    if ( ( window == NULL ) || ( window && !window->isUntitled () ) ) {
        switch ( openByFileType ( filename ) ) {
        default: // 1, unknown file type
            return;
            break;
        case TEXT_EDITOR_SUB_WINDOW:
        case RICH_TEXT_EDITOR_SUB_WINDOW:
            window = startNewTextEditor ();
            break;
        case REPORT_GENERATOR_SUB_WINDOW:
            window = startNewReport ();
            break;
        }
    }
    if ( window->load ( filename ) ) {
        statusBar ()->showMessage ( tr ( "File loaded" ), 2000 );
        //tabDocuments->setTabText ( tabDocuments->currentIndex (), window->title () );
        addToRecentFiles ( filename );
    }
    window->setFocus ();
}

void documentEditor::saveDocument ()
{
    if ( activeDocumentWindow () != NULL ) {
        documentEditorWindow* window = activeDocumentWindow ();
        if ( window->isUntitled () )
            saveAsDocument ();
        else {
            if ( window->save ( window->currentFile () ) ) {
                statusBar ()->showMessage ( tr ( "File saved" ), 2000 );
                //tabDocuments->setTabText ( tabDocuments->currentIndex (), window->title () );
            }
            window->setFocus ();
        }
    }
}

void documentEditor::saveAsDocument ()
{
    if ( activeDocumentWindow () ) {
        documentEditorWindow* window = activeDocumentWindow ();
        const QString initialDir ( window->initialDir () );
        const QString filter ( buildFilter () );
        const QString filename = Data::getSaveFileName ( initialDir, filter );
        if ( !filename.isEmpty () ) {
            if ( window->saveas ( filename ) ) {
                statusBar ()->showMessage ( tr ( "File saved" ), 2000 );
                //tabDocuments->setTabText ( tabDocuments->currentIndex (), window->title () );
                addToRecentFiles ( filename );
            }
        }
        window->setFocus ();
    }
}

void documentEditor::sendMailAttachment ()
{
    if ( activeDocumentWindow () ) {
        QString address, subject, body, attachment;
        activeDocumentWindow ()->buildMailMessage ( address, subject, body, attachment );
        EMAIL_CONFIG ()->sendEMail ( address, subject, body, attachment );
    }
}

void documentEditor::cut ()
{
    if ( activeDocumentWindow () )
        activeDocumentWindow ()->cut ();
}

void documentEditor::copy ()
{
    if ( activeDocumentWindow () )
        activeDocumentWindow ()->copy ();
}

void documentEditor::paste ()
{
    if ( activeDocumentWindow () )
        activeDocumentWindow ()->paste ();
}

void documentEditor::addToRecentFiles ( const QString& filename, const bool b_AddToList )
{
    const QString menuText ( fileOps::fileNameWithoutPath ( filename ) );
    for ( int i = 0; i < recentFilesSubMenu->actions ().count (); ++i ) {
        if ( menuText == recentFilesSubMenu->actions ().at ( i )->text () )
            return;
    }

    const int n_menus ( recentFilesSubMenu->actions ().count () );
    if ( n_menus == MAX_RECENT_FILES ) {
        recentFilesSubMenu->removeAction ( recentFilesSubMenu->actions ().at ( n_menus - 1 ) );
        recentFilesList.remove  ( n_menus - 1 );
    }
    vmAction* filemenu ( new vmAction ( -1, menuText ) );
    filemenu->setData ( filename );
    recentFilesSubMenu->addAction ( filemenu );

    if ( b_AddToList ) {
        recentFilesList.prepend ( filename );
        configOps::writeConfig ( configFileName (), CFG_FIELD_RECENT_FILES, joinRecentFilesList ( recentFilesList ) );
    }
}

void documentEditor::openRecentFile ( QAction* selectedMenu )
{
    const vmAction* action = static_cast<vmAction*> ( selectedMenu );
    const QString filename ( action->data ().toString () );
    openDocument ( filename );
}

documentEditorWindow* documentEditor::findMdiChild ( const QString& fileName ) const
{
    const QString canonicalFilePath = QFileInfo ( fileName ).canonicalFilePath ();
    documentEditorWindow* child = NULL;

    const uint open_tabs ( unsigned ( tabDocuments->count () ) );
    for ( uint i = 0; i < open_tabs; ++i ) {
        child = static_cast<documentEditorWindow*> ( tabDocuments->widget ( i ) );
        if ( child == NULL )
            continue;
        if ( child->currentFile () == canonicalFilePath )
            return child;
    }
    return NULL;
}
