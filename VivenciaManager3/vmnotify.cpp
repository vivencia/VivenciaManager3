#include "vmnotify.h"
#include "vmtaskpanel.h"
#include "data.h"
#include "vmwidgets.h"
#include "global.h"
#include "heapmanager.h"
#include "mainwindow.h"
#include "cleanup.h"
#include "vmactiongroup.h"

#include <QPixmap>
#include <QBitmap>
#include <QPainter>
#include <QApplication>
#include <QTimer>
#include <QSystemTrayIcon>
#include <QRect>
#include <QDesktopWidget>
#include <QPointer>
#include <QKeyEvent>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QEvent>
#include <QProgressBar>
#include <QHBoxLayout>
#include <QPoint>

extern "C"
{
#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
}

// X11/X.h defines a KeyPress constant which conflicts with QEvent's.
// But we don't need X's definition, so we discard it
#undef KeyPress

vmNotify* vmNotify::s_instance ( nullptr );

const char* const PROPERTY_BUTTON_ID ( "pbid" );
const char* const PROPERTY_BUTTON_RESULT ( "pbr" );

void deleteNotifyInstance ()
{
	heap_del ( vmNotify::s_instance );
}

const uint FADE_TIMER_TIMEOUT ( 20 );
static const QString urgency_tag[3] = { QStringLiteral ( "<b>" ), QStringLiteral ( "<b><u>" ), QStringLiteral ( "<b><u><font color=red>" ) };

Message::Message ( vmNotify* parent )
	: timeout ( -1 ), isModal ( false ), mbClosable ( true ),
	  mbAutoRemove ( true ), widgets ( 5 ), m_parent ( parent ), mBtnID ( -1 ),
	  icon ( nullptr ), mGroup ( nullptr ), timer ( nullptr ), messageFinishedFunc ( nullptr )
{}

Message::~Message ()
{
	if ( m_parent != nullptr )
		m_parent->mPanel->removeGroup ( mGroup, true );
	heap_del ( timer );
}

void Message::addWidget ( QWidget* widget, const uint row, const Qt::Alignment alignment, const bool is_button )
{
	st_widgets* st ( new st_widgets );
	st->widget = widget;
	st->row = row;
	st->alignment = alignment;
	st->isButton = is_button;
	if ( row != 0 )
		widgets.append ( st );
	else
		widgets.prepend ( st );
}

void Message::inputFormKeyPressed ( const QKeyEvent* ke )
{
	mBtnID = ke->key () == Qt::Key_Escape ? MESSAGE_BTN_CANCEL : MESSAGE_BTN_OK;
	if ( messageFinishedFunc )
		messageFinishedFunc ( this );
}

vmNotify::vmNotify ( const QString& position, const QWidget* parent )
	: QDialog ( nullptr, Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint ),
	  mbDeleteWhenStackIsEmpty ( false ), mPanel ( new vmTaskPanel ( emptyString, this ) ), fadeTimer ( new QTimer ), m_parent ( const_cast<QWidget*>( parent ) ),
	  mPos ( position ), mEventLoop ( nullptr ), messageStack ( 5 )
{
	QHBoxLayout* mainLayout ( new QHBoxLayout );
	mainLayout->setMargin ( 0 );
	mainLayout->setSpacing ( 0 );
	mainLayout->addWidget ( mPanel, 1 );
	setLayout ( mainLayout );
	connect ( fadeTimer, &QTimer::timeout, this, [&] () { return fadeWidget (); } );
}

vmNotify::~vmNotify ()
{
	messageStack.clear ( true );
}

void vmNotify::initNotify ()
{
	if ( !s_instance )
	{
		s_instance = new vmNotify ( QString::null, MAINWINDOW () );
		addPostRoutine ( deleteNotifyInstance );
	}
}

void vmNotify::enterEventLoop ()
{
	setWindowModality ( Qt::ApplicationModal );
	show ();
	activateWindow ();
	QPointer<QDialog> guard = this;
	QEventLoop eventLoop;
	mEventLoop = &eventLoop;
	QTimer::singleShot ( 0, this, [&] () { mEventLoop->processEvents ( QEventLoop::AllEvents );
		return mEventLoop->exit ( 0 ); } );
	(void) eventLoop.exec ( QEventLoop::DialogExec );
	if ( guard.isNull () )
		return;
	
	mEventLoop = nullptr;
	setAttribute ( Qt::WA_ShowModal, false );
	setAttribute( Qt::WA_SetWindowModality, false );
	setWindowModality ( Qt::NonModal );
}

void vmNotify::addMessage ( Message* message )
{
	// if a message was issued as modal or with an undefined timeout
	// we will probably need to probe the results (which button was clicked, or some text
	message->mbAutoRemove = ( message->timeout != -1 || !message->isModal );
	setupWidgets ( message );
	messageStack.append ( message );
	adjustSizeAndPosition ();
	setWindowOpacity ( 0.9 );

	startMessageTimer ( message );
	if ( message->isModal )
	{
		if ( messageStack.count () == 1  ) // only the one message, we can change the window modality
		{
			setWindowModality ( Qt::ApplicationModal );
			exec ();
		}
	}
	else
		show ();
}

void vmNotify::removeMessage ( Message* message )
{
	if ( message != nullptr )
	{
		messageStack.removeOne ( message );
		if ( messageStack.isEmpty () )
		{
			fadeTimer->start ( FADE_TIMER_TIMEOUT );
			if ( mbDeleteWhenStackIsEmpty )
				this->deleteLater ();
		}
		if ( message->mbAutoRemove )
			delete message;
		adjustSizeAndPosition ();
		if ( mEventLoop )
			mEventLoop->exit ();
	}
}

void vmNotify::buttonClicked ( QPushButton* btn, Message* const message )
{
	int result ( QDialog::Rejected );
	if ( btn != nullptr )
	{
		message->mBtnID = btn->property ( PROPERTY_BUTTON_ID ).toInt ();
		result = btn->property ( PROPERTY_BUTTON_RESULT ).toInt ();
		if ( result == MESSAGE_BTN_CUSTOM )
			result = QDialog::Rejected;
	}
	else
		result = message->mBtnID;

	if ( message->isModal )
	{
		if ( result == QDialog::Accepted )
			accept ();
		else
			reject ();
	}
	else
		setResult ( result );
	if ( windowModality () != Qt::NonModal && messageStack.count () == 1 )
		setWindowModality ( Qt::NonModal );
	removeMessage ( message );
}

void vmNotify::setupWidgets ( Message* const message )
{
	QHBoxLayout* rowLayout ( nullptr );
	uint row ( UINT_MAX );

	if ( !message->iconName.isEmpty () )
		message->icon = new QPixmap ( message->iconName );

	message->mGroup = mPanel->createGroup ( message->icon != nullptr ? QIcon ( *message->icon ) : QIcon (), message->title, false, false, message->mbClosable );
	message->mGroup->setCallbackForClosed ( [&, message] () { return buttonClicked ( nullptr, message ); } );
	if ( !message->bodyText.isEmpty () )
	{
		QLabel* lblMessage ( new QLabel ( message->bodyText ) );
		lblMessage->setWordWrap ( true );
		message->addWidget ( lblMessage , 0 );
	}

	QWidget* widget ( nullptr );
	for ( uint i ( 0 ); i < message->widgets.count (); ++i )
	{
		if ( row != message->widgets.at ( i )->row )
		{
			if ( rowLayout != nullptr )
				rowLayout->addStretch ( 1 ); // inserts a stretchable space at the end of the previous row
			rowLayout = new QHBoxLayout;
			rowLayout->setSpacing ( 2 );
			rowLayout->insertStretch ( 0, 1 );
			row = message->widgets.at ( i )->row;
		}
		widget = message->widgets.at ( i )->widget;
		message->mGroup->addQEntry ( widget, rowLayout );
		if ( message->widgets.at ( i )->isButton )
		{
			static_cast<void>(connect ( static_cast<QPushButton*>( widget ), &QPushButton::clicked, this, 
					  [&, widget, message] () { return buttonClicked ( static_cast<QPushButton*>( widget ), message ); } ));
		}
	}
	if ( rowLayout != nullptr )
		rowLayout->addStretch ( 1 ); // inserts a stretchable space at the end of the last row
}

void vmNotify::startMessageTimer ( Message* const message )
{
	if ( message->timeout > 0 )
	{
		message->timer = new QTimer ();
		message->timer->setSingleShot ( true );
		connect ( message->timer, &QTimer::timeout, this, [&,message] () { return removeMessage ( message ); } );
		message->timer->start ( message->timeout );
	}
}


void vmNotify::fadeWidget ()
{
	if ( windowOpacity () > 0 && isVisible () )
	{
		setWindowOpacity ( windowOpacity () - 0.08 );
		fadeTimer->start ( FADE_TIMER_TIMEOUT );
	}
	else {
		hide ();
		fadeTimer->stop ();
	}
}

void vmNotify::adjustSizeAndPosition ()
{
	adjustSize ();

	QBitmap objBitmap ( size () );
	QPainter painter ( &objBitmap );
	painter.fillRect ( rect (), Qt::white );
	painter.setBrush ( QColor ( 0, 0, 0 ) );
	painter.drawRoundedRect ( rect (), 10, 10 );
	setMask ( objBitmap );

	const QPoint pos ( displayPosition ( size () ) );
	int x ( pos.x () - rect ().size ().width () - 10 );
	int y ( pos.y () - rect ().size ().height () - 10 );
	if ( x < 0 )
		x = 0;
	if ( y < 0 )
		y = 0;
	move ( x, y );
}

QPoint vmNotify::displayPosition ( const QSize& widgetSize )
{
	QRect parentGeometry;

	if ( m_parent == nullptr || m_parent == MAINWINDOW () )
	{
		parentGeometry = qApp->desktop ()->availableGeometry ();

		if ( mPos.isEmpty () )
		{
			const QDesktopWidget* desktop ( qApp->desktop () );
			const QRect displayRect ( desktop->screenGeometry () );
			const QRect availableRect ( desktop->availableGeometry () );

			if ( availableRect.height () < displayRect.height () )
			{
				mPos = QStringLiteral ( "BR" );
			}
			else
			{
				mPos = ( availableRect.x () > displayRect.x () ) ? QStringLiteral ( "LC" ) : QStringLiteral ( "BC" );
			}
		}
	}
	else
	{
		int x ( 0 ), y ( 0 );
		if ( MAINWINDOW () && ( m_parent->parent () == MAINWINDOW () ) )
		{ // mapTo: first argument must be a parent of caller object
			const QPoint mainwindowpos ( MAINWINDOW ()->pos () );
			x = mainwindowpos.x () + m_parent->mapTo ( MAINWINDOW (), m_parent->pos () ).x () - m_parent->x ();
			y = mainwindowpos.y () + m_parent->mapTo ( MAINWINDOW (), m_parent->pos () ).y () - m_parent->y ();
		}
		else
		{
			x = m_parent->x ();
			y = m_parent->y ();
		}
		parentGeometry.setCoords ( x, y, x + width (), y + height () );
		if ( mPos.isEmpty () )
			mPos = CHR_ZERO;
	}

	QPoint p;
	if ( mPos == QStringLiteral ( "0" ) || mPos == QStringLiteral ( "BL" ) )
		p = parentGeometry.bottomLeft ();
	else if ( mPos == QStringLiteral ( "1" ) || mPos == QStringLiteral ( "BR" ) )
		p = parentGeometry.bottomRight ();
	else if ( mPos == QStringLiteral ( "2" ) || mPos == QStringLiteral ( "TR" ) )
		p = parentGeometry.topRight ();
	else if ( mPos == QStringLiteral ( "3" ) || mPos == QStringLiteral ( "TL" ) )
		p = parentGeometry.topLeft ();
	else if ( mPos == QStringLiteral ( "4" ) || mPos == QStringLiteral ( "C" ) ) {
		p = parentGeometry.bottomRight ();
		p.setX ( p.x () / 2 + widgetSize.width () / 2 );
		p.setY ( p.y () / 2 + widgetSize.height () / 2 );
	}
	else if ( mPos == QStringLiteral ( "5" ) || mPos == QStringLiteral ( "RC" ) )
	{
		p = parentGeometry.bottomRight ();
		p.setY ( p.y () / 2 + widgetSize.height () / 2 );
	}
	else if ( mPos == QStringLiteral ( "6" ) || mPos == QStringLiteral ( "TC" ) )
	{
		p = parentGeometry.topRight ();
		p.setX ( p.x () / 2 + widgetSize.width () / 2 );
	}
	else if ( mPos == QStringLiteral ( "7" ) || mPos == QStringLiteral ( "LC" ) )
	{
		p = parentGeometry.bottomLeft ();
		p.setY ( p.y () / 2 + widgetSize.height () / 2 );
	}
	else if ( mPos == QStringLiteral ( "8" ) || mPos == QStringLiteral ( "BC" ) )
	{
		p = parentGeometry.bottomRight ();
		p.setX ( p.x () / 2 + widgetSize.width () / 2 );
	}
	else
		p = parentGeometry.bottomRight ();

	return p;
}

int vmNotify::notifyBox ( const QString& title, const QString& msg,
			const MESSAGE_BOX_ICON icon, const QString btnsText[3], const int m_sec )
{
	QPushButton* btn0 ( nullptr ), *btn1 ( nullptr ), *btn2 ( nullptr );
	Message* message ( new Message ( this ) );
	message->title = title;
	message->bodyText = msg;
	message->timeout = m_sec;
	message->iconName = QLatin1String ( ":/resources/notify-" ) + QString::number ( icon ) + QLatin1String ( "" );

	if ( !btnsText[0].isEmpty () ) // this button is always a ok button
	{
		btn0 = new QPushButton ( btnsText[0] );
		btn0->setProperty ( PROPERTY_BUTTON_ID, MESSAGE_BTN_OK );
		btn0->setProperty ( PROPERTY_BUTTON_RESULT, QDialog::Accepted );
		message->addWidget ( btn0, 1, Qt::AlignCenter, true );
	}
	if ( !btnsText[1].isEmpty () ) // this second button is a cancel when there is not a third button, or another ok, when there is a third button
	{
		int nextBtnId ( MESSAGE_BTN_CANCEL );
		if ( !btnsText[2].isEmpty () )
			nextBtnId = MESSAGE_BTN_CUSTOM;
		btn1 = new QPushButton ( btnsText[1] );
		btn1->setProperty ( PROPERTY_BUTTON_ID, nextBtnId );
		btn1->setProperty ( PROPERTY_BUTTON_RESULT, nextBtnId == MESSAGE_BTN_CANCEL ? MESSAGE_BTN_CUSTOM : static_cast<int>(QDialog::Rejected) );
		message->addWidget ( btn1, 1, Qt::AlignCenter, true );
	}
	if ( !btnsText[2].isEmpty () ) // this button is always a cancel button
	{
		btn2 = new QPushButton ( btnsText[2] );
		btn2->setProperty ( PROPERTY_BUTTON_ID, MESSAGE_BTN_CANCEL );
		btn2->setProperty ( PROPERTY_BUTTON_RESULT, QDialog::Rejected );
		message->addWidget ( btn2, 1, Qt::AlignCenter, true );
	}
	message->isModal = ( btn1 != nullptr ) || ( btn2 != nullptr );
	addMessage ( message ); // now we wait for control return to this event loop

	const int btnID ( message->mBtnID );
	delete message;
	return btnID;
}

void vmNotify::notifyMessage ( const QString& title, const QString& msg, const int msecs, const bool b_critical )
{
	Message* message ( new Message ( this ) );
	message->title = title;
	message->bodyText = msg;
	message->timeout = msecs;
	message->iconName = QLatin1String ( ":/resources/notify-" ) + QLatin1String ( b_critical ? "3" : "2" ) + QLatin1String ( "" );
	addMessage ( message );
}

void vmNotify::notifyMessage ( const QWidget* referenceWidget, const QString& title, const QString& msg, const int msecs, const bool b_critical )
{
	vmNotify* newNotify ( new vmNotify ( referenceWidget != nullptr ? QStringLiteral ( "C" ) : QString::null, referenceWidget ) );
	newNotify->mbDeleteWhenStackIsEmpty = true;
	newNotify->notifyMessage ( title, msg, msecs, b_critical );
}

void vmNotify::messageBox ( const QString& title, const QString& msg , const int msec )
{
	const QString btnsText[3] = { QStringLiteral ( "OK" ), emptyString, emptyString };
	( void ) notifyBox ( title, msg, vmNotify::WARNING, btnsText, msec );
}

void vmNotify::messageBox ( const QWidget* referenceWidget, const QString& title, const QString& msg , const int msec )
{
	vmNotify* newNotify ( new vmNotify ( referenceWidget != nullptr ? QStringLiteral ( "C" ) : QString::null, referenceWidget ) );
	newNotify->mbDeleteWhenStackIsEmpty = true;
	newNotify->messageBox ( title, msg, msec );
}

bool vmNotify::questionBox ( const QString& title, const QString& msg, const QWidget* referenceWidget )
{
	const QString btnsText[3] = { TR_FUNC ( "Yes" ), TR_FUNC ( "No" ), emptyString };
	vmNotify* newNotify ( new vmNotify ( referenceWidget != nullptr ? QStringLiteral ( "C" ) : QString::null, referenceWidget ) );
	const bool bAnswer ( newNotify->notifyBox ( title, msg, vmNotify::QUESTION, btnsText ) == MESSAGE_BTN_OK );
	delete newNotify;
	return bAnswer;
}

int vmNotify::criticalBox ( const QString& title, const QString& msg, const bool b_message_only, const QWidget* referenceWidget )
{
	const QString btnsText[3] = { b_message_only ? TR_FUNC ( "OK" ) : TR_FUNC ( "Retry" ), b_message_only ? emptyString : TR_FUNC ( "Cancel" ), emptyString };
	vmNotify* newNotify ( new vmNotify ( referenceWidget != nullptr ? QStringLiteral ( "C" ) : QString::null, referenceWidget ) );
	const int btn ( newNotify->notifyBox ( title, msg, vmNotify::CRITICAL, btnsText ) );
	delete newNotify;
	return btn;
}

int vmNotify::customBox ( const QString& title, const QString& msg,
		const vmNotify::MESSAGE_BOX_ICON icon, const QString& btnText1, const QString& btnText2,
		const QString& btnText3, const QWidget* referenceWidget )
{
	const QString btnsText[3] = { btnText1, btnText2, btnText3 };
	vmNotify* newNotify ( new vmNotify ( referenceWidget != nullptr ? QStringLiteral ( "C" ) : QString::null, referenceWidget ) );
	const int btn ( newNotify->notifyBox ( title, msg, icon, btnsText ) );
	delete newNotify;
	return btn;
}

bool vmNotify::inputBox ( QString& result, const QWidget* referenceWidget, const QString& title, const QString& label_text,
						  const QString& initial_text, const QString& icon, const QString& opt_check_box,
						  const vmCompleters::COMPLETER_CATEGORIES completer )
{
	vmNotify* newNotify ( new vmNotify ( referenceWidget != nullptr ? QStringLiteral ( "C" ) : emptyString, referenceWidget ) );
	newNotify->setWindowFlags ( Qt::FramelessWindowHint );
	uint row ( 1 );
	Message* message ( new Message ( newNotify ) );
	message->title = title;
	message->bodyText = label_text;
	message->timeout = -1;
	message->isModal = true;
	message->iconName = icon;
	message->mbAutoRemove = false;

	vmLineEdit* inputForm ( new vmLineEdit );
	inputForm->setEditable ( true );
	inputForm->setText ( initial_text );
	inputForm->setMaximumHeight ( 30 );
	inputForm->setMinimumWidth ( 200 );
	
	if ( completer != vmCompleters::NONE )
		APP_COMPLETERS ()->setCompleter ( inputForm, completer );
	inputForm->setCallbackForRelevantKeyPressed ( [&, message] ( const QKeyEvent* ke, const vmWidget* const ) { return message->inputFormKeyPressed ( ke ); } );
	message->setMessageFinishedCallback ( [&newNotify ] ( Message* msg ) { return newNotify->buttonClicked ( nullptr, msg ); } );
	message->addWidget ( inputForm, row );

	vmCheckBox* optCheck ( nullptr );
	if ( !opt_check_box.isEmpty () )
	{
		optCheck = new vmCheckBox ( opt_check_box );
		optCheck->setCheckable ( true );
		optCheck->setEditable ( true );
		optCheck->setMaximumHeight ( 30 );
		message->addWidget ( static_cast<QWidget*> ( optCheck ), ++row, Qt::AlignLeft, false );
	}

	QPushButton* btn0 ( new QPushButton ( QStringLiteral ( "OK" ) ) );
	btn0->setProperty ( PROPERTY_BUTTON_ID, MESSAGE_BTN_OK );
	btn0->setProperty ( PROPERTY_BUTTON_RESULT, QDialog::Accepted );
	message->addWidget ( btn0, ++row, Qt::AlignCenter, true );
	QPushButton* btn1 ( new QPushButton ( QApplication::tr ( "Cancel" ) ) );
	btn1->setProperty ( PROPERTY_BUTTON_ID, MESSAGE_BTN_CANCEL );
	btn1->setProperty ( PROPERTY_BUTTON_RESULT, QDialog::Rejected );
	message->addWidget ( static_cast<QWidget*> ( btn1 ), row, Qt::AlignCenter, true );
	
	newNotify->addMessage ( message );
	bool b_ok ( message->mBtnID == MESSAGE_BTN_OK );

	if ( b_ok )
	{
		if ( optCheck != nullptr )
		{
			if ( optCheck->isChecked () )
				result = optCheck->text ();
		}
		if ( result.isEmpty () )
			result = inputForm->text ();
		b_ok = !result.isEmpty ();
	}
	delete message;
	delete newNotify;
	return b_ok;
}

bool vmNotify::passwordBox ( QString& result, const QWidget* referenceWidget, const QString& title,
							 const QString& label_text, const QString& icon )
{
	vmNotify* newNotify ( new vmNotify ( referenceWidget != nullptr ? QStringLiteral ( "C" ) : emptyString, referenceWidget ) );
	newNotify->setWindowFlags ( Qt::FramelessWindowHint );
	uint row ( 1 );
	Message* message ( new Message ( newNotify ) );
	message->title = title;
	message->bodyText = label_text;
	message->timeout = -1;
	message->isModal = true;
	message->iconName = icon;
	message->mbAutoRemove = false;

	vmLineEdit* inputForm ( new vmLineEdit );
	inputForm->setEditable ( true );
	inputForm->setMaximumHeight ( 30 );
	inputForm->setMinimumWidth ( 200 );
	inputForm->setEchoMode ( QLineEdit::Password );
	inputForm->setCallbackForRelevantKeyPressed ( [&, message] ( const QKeyEvent* ke, const vmWidget* const ) { return message->inputFormKeyPressed ( ke ); } );
	message->setMessageFinishedCallback ( [&newNotify ] ( Message* msg ) { return newNotify->buttonClicked ( nullptr, msg ); } );
	message->addWidget ( inputForm, row );

	QPushButton* btn0 ( new QPushButton ( QStringLiteral ( "OK" ) ) );
	btn0->setProperty ( PROPERTY_BUTTON_ID, MESSAGE_BTN_OK );
	btn0->setProperty ( PROPERTY_BUTTON_RESULT, QDialog::Accepted );
	message->addWidget ( btn0, ++row, Qt::AlignCenter, true );
	QPushButton* btn1 ( new QPushButton ( QApplication::tr ( "Cancel" ) ) );
	btn1->setProperty ( PROPERTY_BUTTON_ID, MESSAGE_BTN_CANCEL );
	btn1->setProperty ( PROPERTY_BUTTON_RESULT, QDialog::Rejected );
	message->addWidget ( static_cast<QWidget*> ( btn1 ), row, Qt::AlignCenter, true );
	
	newNotify->addMessage ( message );
	bool b_ok ( message->mBtnID == MESSAGE_BTN_OK );

	if ( b_ok )
	{
		result = inputForm->text ();
		b_ok = !result.isEmpty ();
	}
	delete message;
	delete newNotify;
	return b_ok;
}

vmNotify* vmNotify::progressBox ( vmNotify* box, QWidget* parent, const uint max_value, uint next_value,
										 const QString& title, const QString& label )
{
	const bool bNewDlg ( box == nullptr );
	Message* message ( nullptr );
	QProgressBar* pBar ( nullptr );

	if ( bNewDlg )
	{
		box = new vmNotify ( QStringLiteral ( "C" ), const_cast<QWidget*> ( parent ) );
		message = new Message ( box );
		message->title = title;
		message->bodyText = label;
		message->timeout = -1;
		message->isModal = false;
		message->mbAutoRemove = false;

		pBar = new QProgressBar ();
		pBar->setTextVisible ( true );
		pBar->setMinimumWidth ( 200 );
		pBar->setMaximum ( static_cast<int>(max_value) );
		message->addWidget ( pBar, 1 );
		QPushButton* btnCancel ( new QPushButton ( QApplication::tr ( "Cancel" ) ) );
		btnCancel->setProperty ( PROPERTY_BUTTON_ID, MESSAGE_BTN_CANCEL );
		btnCancel->setProperty ( PROPERTY_BUTTON_RESULT, QDialog::Rejected );
		message->addWidget ( btnCancel, 2, Qt::AlignCenter, true );
		box->addMessage ( message );
	}
	else
	{
		message = box->messageStack.at ( 0 );
		QLabel* labelWdg ( static_cast<QLabel*>( message->widgets.at ( 0 )->widget ) );
		labelWdg->setText ( label );
		pBar = static_cast<QProgressBar*>( message->widgets.at ( 1 )->widget );
		pBar->setValue ( static_cast<int>(next_value) );
		box->enterEventLoop ();
	}

	if ( next_value >= max_value || message->mBtnID == MESSAGE_BTN_CANCEL )
	{
		box->removeMessage ( message );
		box->deleteLater ();
		return nullptr;
	}
	return box;
}
