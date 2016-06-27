#ifndef DOCUMENTEDITOR_H
#define DOCUMENTEDITOR_H

#include "documenteditorwindow.h"
#include "vmlist.h"

#include <QMainWindow>
#include <QStringList>

extern bool EXITING_PROGRAM;

class textEditor;
class reportGenerator;
class vmAction;

class QToolBar;
class QTabWidget;
class QDockWidget;

class documentEditor : public QMainWindow
{

public:
	virtual ~documentEditor ();
	textEditor* startNewTextEditor ();
	reportGenerator* startNewReport ( const bool b_windowless = false );

	void addDockWindow ( Qt::DockWidgetArea area, QDockWidget* dockwidget );
	void removeDockWindow ( QDockWidget* dockwidget );

	void updateMenus ( const int tab_index );
	void updateWindowMenu ();
	void newTextDocument ();
	void newReport ();
	void openDocument ();
	void saveDocument ();
	void saveAsDocument ();
	void sendMailAttachment ();
	void cut ();
	void copy ();
	void paste ();
	void addToRecentFiles ( const QString& filename, const bool b_AddToList = true );
	void openRecentFile ( QAction* action );
	void makeWindowActive ( QAction* action );
	void changeTabText ( documentEditorWindow* window );
	void closeTab ( int tab_index = -1 );
	void closeAllTabs ();
	void activateNextTab ();
	void activatePreviousTab ();
	void showCalc ();

protected:
	void closeEvent ( QCloseEvent* );
	
private:
	explicit documentEditor ( QWidget* parent = nullptr );

	void openDocument ( const QString& filename );
	void createActions ();
	void createMenus ();
	void createToolBars ();
	void createStatusBar ();

	inline documentEditorWindow* activeDocumentWindow () const
	{
		return static_cast<documentEditorWindow*>( tabDocuments->currentWidget () );
	}

	QString buildFilter () const;
	void resizeViewPort ( documentEditorWindow* window );
	uint openByFileType ( const QString& filename );

	QTabWidget* tabDocuments;

	QMenu* fileMenu;
	QMenu* editMenu;
	QMenu* windowMenu;
	QMenu* helpMenu;
	QMenu* recentFilesSubMenu;
	QToolBar* fileToolBar;
	QToolBar* editToolBar;
	vmAction* newAct, *newReportAct;
	vmAction* openAct;
	vmAction* saveAct;
	vmAction* saveAsAct;
	vmAction* mailAct;
	vmAction* cutAct;
	vmAction* copyAct;
	vmAction* pasteAct;
	vmAction* closeAct;
	vmAction* closeAllAct;
	vmAction* hideAct;
	vmAction* nextAct;
	vmAction* previousAct;
	vmAction* separatorAct;
	vmAction* calcAct;

	friend documentEditor* EDITOR ();
	friend void deleteEditorInstance ();
	static documentEditor* s_instance;

	bool mb_ClosingAllTabs;
	VMList<QString> recentFilesList;
};

inline documentEditor* EDITOR ()
{
	if ( !EXITING_PROGRAM ) {
		if ( !documentEditor::s_instance )
			documentEditor::s_instance = new documentEditor;
		return documentEditor::s_instance;
	}
	return nullptr;
}

#endif // DOCUMENTEDITOR_H
