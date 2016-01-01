#include "separatewindow.h"
#include "global.h"
#include "mainwindow.h"

#include <QPushButton>
#include <QCloseEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>

separateWindow::separateWindow ( QWidget* child )
	: QDialog (), m_child ( child ), m_toolbar ( nullptr ),
	  mToolBarLayout ( new QHBoxLayout ), mainLayout ( new QVBoxLayout ),
	  mb_Active ( false ), mb_Visible ( false )
{
	btnReturn = new QPushButton ( tr ( "Return" ), this );
	connect ( btnReturn, &QPushButton::clicked, this, [&] () { return returnToParent (); } );

	mToolBarLayout->addWidget ( btnReturn, 0, Qt::AlignRight );
	mainLayout->addLayout ( mToolBarLayout, 0 );

	setLayout ( mainLayout );
	resize ( m_child->sizeHint () );
	globalMainWindow->installEventFilter ( this );
}

void separateWindow::addChild ( QWidget* child )
{
	if ( child ) {
		if ( child != m_child ) {
			if ( m_child )
				mainLayout->removeWidget ( m_child );
			m_child = child;
			if ( mb_Active ) {
				QDialog::hide ();
				mb_Active = false;
				showSeparate ( windowTitle () ); // cannot be in exec mode if this function was called because the program runs single threaded
			}
		}
	}
}

void separateWindow::addToolBar ( QWidget* toolbar )
{
	if ( toolbar ) {
		if ( toolbar != m_toolbar ) {
			QDialog::hide ();
			if ( m_toolbar )
				mToolBarLayout->removeWidget ( m_toolbar );
			m_toolbar = toolbar;
			mToolBarLayout->removeWidget ( btnReturn ); // a return button must be implemented in the toolbar, otherwise the interface becomes clunky
			btnReturn->hide ();
			if ( mb_Active ) {
				mb_Active = false;
				showSeparate ( windowTitle () ); // cannot be in exec mode if this function was called because the program runs single threaded
			}
		}
	}
}

void separateWindow::closeEvent ( QCloseEvent* e )
{
	e->ignore ();
}

// Hides and shows this windows whenever the parent window is minimized/hidden or visible/shown
bool separateWindow::eventFilter ( QObject* o, QEvent* e )
{
	if ( globalMainWindow && ( o == globalMainWindow ) ) {
		switch ( e->type () ) {
			default:
				e->ignore ();
				return false;
			break;
			case QEvent::WindowStateChange:
				if ( globalMainWindow->isMinimized () || globalMainWindow->isHidden () )
					childHideRequested ();
				else
					childShowRequested ();
			break;
			case QEvent::Hide:
				childHideRequested ();
			break;
			case QEvent::Show:
				childShowRequested ();
			break;
		}
		e->accept ();
		return true;
	}
	e->ignore ();
	return false;
}

// The next functions are here to avoid exploitation of the multi show/hide
// capabilities of QDialog; only one function is allowed to show the dialog:
// void separateWindow::showSeparate () , and only one to close it void separateWindow::returnToParent ()
int separateWindow::exec ()
{
	return QDialog::Rejected;
}

void separateWindow::open ()
{
	setResult ( QDialog::Rejected );
}

void separateWindow::show ()
{
	setResult ( QDialog::Rejected );
}

void separateWindow::accept ()
{
	setResult ( QDialog::Rejected );
}

void separateWindow::reject ()
{
	setResult ( QDialog::Rejected );
}

void separateWindow::done ( int )
{
	setResult ( QDialog::Rejected );
}

void separateWindow::hide ()
{
	if ( mb_Visible && !mb_Active && ( result () == QDialog::Accepted ) ) {
		mb_Visible = false;
		QDialog::hide ();
	}
}

void separateWindow::showSeparate ( const QString& window_title, const bool b_exec, const Qt::WindowStates w_state )
{
	if ( mb_Active ) {
		if ( w_state & windowState () )
			return;
	}

	if ( !mb_Active ) {
		mb_Active = true;
		if ( m_toolbar ) {
			mToolBarLayout->insertWidget ( 0, m_toolbar );
			m_toolbar->show ();
		}
		if ( m_child )  {
			mainLayout->addWidget ( m_child, 2 );
			m_child->show ();
			m_child->setFocus ();
		}
		mb_Visible = true;
		setWindowTitle ( window_title );
	}
	//toggle state. The first if check makes sure that w_state is different from current windowState (),
	// so now we only have to toggle it
	setWindowState ( windowState () ^ Qt::WindowMaximized );
	if ( !b_exec )
		QDialog::show ();
	else
		QDialog::exec ();
}

void separateWindow::childHideRequested ()
{
	if ( mb_Visible && mb_Active ) {
		mb_Visible = false;
		QDialog::hide ();
	}
}

void separateWindow::childShowRequested ()
{
	if ( !mb_Visible && mb_Active ) {
		mb_Visible = true;
		QDialog::show ();
	}
}

void separateWindow::childCloseRequested ( const int )
{
	returnToParent ();
}

void separateWindow::returnToParent ()
{
	if ( mb_Visible && mb_Active )  {
		mb_Active = false;
		setResult ( QDialog::Accepted );
		if ( m_toolbar )
			mToolBarLayout->removeWidget ( m_toolbar );
		if ( m_child )
			mainLayout->removeWidget ( m_child );
		if ( funcReturnToParent )
			funcReturnToParent ( m_child );
		hide ();
	}
}
