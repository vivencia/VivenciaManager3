#include "vmwidgets.h"
#include "mainwindow.h"
#include "simplecalculator.h"
#include "system_init.h"
#include "configops.h"
#include "vmlistitem.h"
#include "heapmanager.h"

#include <QApplication>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QStyleOptionToolButton>
#include <QMenu>
#include <QHBoxLayout>
#include <QDoubleValidator>
#include <QIntValidator>

vmAction::~vmAction () {}
//------------------------------------------------VM-ACTION-LABEL-------------------------------------------------
vmActionLabel::vmActionLabel ( QWidget *parent )
	: QToolButton ( parent ), vmWidget ( WT_LABEL | WT_BUTTON, WT_ACTION ), labelActivated_func ( nullptr )
{
	init ();
}

vmActionLabel::vmActionLabel ( const QString& text, QWidget* parent )
	: QToolButton ( parent ), vmWidget ( WT_LABEL | WT_BUTTON, WT_ACTION ), labelActivated_func ( nullptr )
{
	QToolButton::setText ( text );
	init ( false );
}

vmActionLabel::vmActionLabel ( vmAction* action, QWidget* parent )
	: QToolButton ( parent ), vmWidget ( WT_LABEL | WT_BUTTON, WT_ACTION ), labelActivated_func ( nullptr )
{
	init ();
	setDefaultAction ( static_cast<QAction*>( action ) );
}

vmActionLabel::~vmActionLabel ()
{}

QString vmActionLabel::defaultStyleSheet () const
{
	QString colorstr;
	if ( !parentWidget () )
		colorstr = QStringLiteral ( " ( 255, 255, 255 ) }" );
	else
	{
		const vmActionLabel* lbl ( new vmActionLabel ( parentWidget () ) );
		colorstr = QLatin1String ( " ( " ) + lbl->palette ().color ( lbl->backgroundRole () ).name ()
				   + QLatin1String ( " ) }" );
		delete lbl;
	}
	return ( QLatin1String ( "QToolButton { background-color: hex" ) + colorstr );
}

void vmActionLabel::init ( const bool b_action )
{
	setWidgetPtr ( static_cast<QWidget*>( this ) );
	setToolButtonStyle ( Qt::ToolButtonTextBesideIcon );
	setSizePolicy ( QSizePolicy::Expanding, QSizePolicy::Expanding );
	
	if ( b_action )
	{
		setCursor ( Qt::PointingHandCursor );
		setFocusPolicy ( Qt::StrongFocus );
	}
}

void vmActionLabel::setCallbackForLabelActivated ( const std::function<void ()>& func )
{
	labelActivated_func = func;
	static_cast<void>( connect ( this, &QToolButton::clicked, this, [&] () { return labelActivated_func (); } ) );
}

QSize vmActionLabel::sizeHint () const
{
	ensurePolished ();

	int w ( 0 ), h ( 0 );
	QStyleOptionToolButton opt;
	initStyleOption ( &opt );

	QString s ( QToolButton::text () );
	const bool empty ( s.isEmpty () );
	if ( empty )
		s = QStringLiteral ( "XXXX" );

	const QSize sz ( fontMetrics ().size ( Qt::TextShowMnemonic, s ) );
	if ( !empty )
		w += sz.width ();
	if ( !empty )
		h = qMax ( h, sz.height () );
	opt.rect.setSize ( QSize ( w, h ) ); // PM_MenuButtonIndicator depends on the height

	if ( !icon ().isNull () )
	{
		const int ih ( opt.iconSize.height () );
		const int iw ( opt.iconSize.width () + 4 );
		w += iw;
		h = qMax ( h, ih );
	}

	if ( menu () )
		w += style ()->pixelMetric ( QStyle::PM_MenuButtonIndicator, &opt, this );

	h += 4;
	w += 8;

	return style ()->sizeFromContents ( QStyle::CT_PushButton, &opt, QSize ( w, h ), this ).
		   expandedTo ( QApplication::globalStrut () );
}

QSize vmActionLabel::minimumSizeHint () const
{
	return sizeHint ();
}
//------------------------------------------------VM-ACTION-LABEL-------------------------------------------------

//------------------------------------------------VM-DATE-EDIT------------------------------------------------
class pvmDateEdit : public QDateEdit, public vmWidget
{

friend class vmDateEdit;

public:
	pvmDateEdit ( vmDateEdit* owner );
	virtual ~pvmDateEdit ();

	inline void setID ( const int id ) {
		mLineEdit->setID ( id );
		vmWidget::setID ( id );
	}
	
	inline QLatin1String qtClassName () const {
		return QLatin1String ( "QDateEdit" );
	}
	QString defaultStyleSheet () const;

	void setDate ( const vmNumber& date, const bool b_notify = false );
	void setEditable ( const bool editable );
	inline QMenu* standardContextMenu () const { return mLineEdit->standardContextMenu (); }

protected:
	void keyPressEvent ( QKeyEvent* );
	void focusInEvent ( QFocusEvent* );
	void focusOutEvent ( QFocusEvent* );
	void contextMenuEvent ( QContextMenuEvent* );

private:
	vmDateEdit* mOwner;
	vmLineEdit* mLineEdit;
	
	QDate mDateBeforeFocus;
	triStateType mEmitSignal;
	bool mbHasFocus;

	void vmDateChanged ( const QDate& date );
};

const uint N_DATES_MENU_STATIC_ENTRIES ( 4 ); // already counting the separator
const uint MAX_RECENT_USE_DATES ( 3 );
QMenu* vmDateEdit::menuDateButtons ( nullptr );

void deleteMenuDateButtons ()
{
	heap_del ( vmDateEdit::menuDateButtons );
}

pvmDateEdit::pvmDateEdit ( vmDateEdit* owner )
		: QDateEdit ( QDate ( 2000, 1, 1 ) ), vmWidget ( WT_DATEEDIT ), mOwner ( owner ),
		  mLineEdit ( new vmLineEdit ), mDateBeforeFocus ( 2000, 1, 1 ), mEmitSignal ( TRI_UNDEF ), mbHasFocus ( false )
{
	setWidgetPtr ( static_cast<QWidget*>( this ) );
	setLineEdit ( mLineEdit );
	mLineEdit->setVmParent ( this );
	setCalendarPopup ( true );
	setDisplayFormat ( DATE_FORMAT_HUMAN );
}

pvmDateEdit::~pvmDateEdit () {}

QString pvmDateEdit::defaultStyleSheet () const
{
	QString colorstr;
	if ( !parentWidget () )
		colorstr = QStringLiteral ( " ( 255, 255, 255 ) }" );
	else
	{
		const QDateEdit* dte ( new QDateEdit ( parentWidget () ) );
		colorstr = QLatin1String ( " ( " ) + dte->palette ().color ( dte->backgroundRole () ).name ()
				   + QLatin1String ( " ) }" );
		delete dte;
	}
	return ( QLatin1String ( "QDateEdit { background-color: hex" ) + colorstr );
}

void pvmDateEdit::setDate ( const vmNumber& date, const bool b_notify )
{
	const triStateType mEmitSignalOriginal ( mEmitSignal );
	mEmitSignal = b_notify && date.isDate ();
	if ( mEmitSignal.isOn () )
	{
		// set date programmatically. Disable Qt's signal
		static_cast<void>( disconnect ( this, nullptr, nullptr, nullptr ) );
		QDateEdit::setDate ( date.toQDate () );
		vmDateChanged ( date.toQDate () );
		static_cast<void>( connect ( this, &QDateEdit::dateChanged, this, [&] ( const QDate& date ) { return vmDateChanged ( date ); } ) );
	}
	else
		QDateEdit::setDate ( date.toQDate () );
	mEmitSignal = mEmitSignalOriginal;
}

void pvmDateEdit::setEditable ( const bool editable )
{
	if ( editable )
	{
		static_cast<void>( connect ( this, &QDateEdit::dateChanged, this, [&] ( const QDate& date ) { return vmDateChanged ( date ); } ) );
		mDateBeforeFocus = date ();
	}
	else
		static_cast<void>( disconnect ( this, nullptr, nullptr, nullptr ) );

	mLineEdit->setEditable ( editable );
	setReadOnly ( !editable );
	vmWidget::setEditable ( editable );
}

/* Because we are connected directly with Qt's QDateEdit::dateChanged signal (the calendar popup
 * mechanism sort of obligates us to do so) we receive te signal both programatically and by user
 * interaction and we do not have control when the signal is emitted. But, because we can control
 * when the signal received is processed, we choose to do so when the control is editable, and when
 * it loses focus (avoids processing each and every date change via keyboard arrows), and the new date
 * is not the same as the date the control had first receive focus or when we made it editable,
 * and not everytime Qt decides to throw the signal
 */
void pvmDateEdit::vmDateChanged ( const QDate& date )
{
	if ( isEditable () && !mEmitSignal.isOff () )
	{
		if ( !hasFocus () )
		{
			if ( mDateBeforeFocus != date )
			{
				vmDateEdit::updateRecentUsedDates ( vmNumber ( date ) );
				mEmitSignal.setUndefined ();
				if ( mOwner->contentsAltered_func != nullptr )
					mOwner->contentsAltered_func ( mOwner );
			}
		}
	}
}

void pvmDateEdit::keyPressEvent ( QKeyEvent* e )
{
	if ( isEditable () )
	{
		if ( e->key () == Qt::Key_Enter || e->key () == Qt::Key_Return || e->key () == Qt::Key_Escape )
		{
			if ( keypressed_func )
				keypressed_func ( e, this );
		}
		QDateEdit::keyPressEvent ( e );
	}
}

void pvmDateEdit::focusInEvent ( QFocusEvent* e )
{
	if ( isEditable () )
	{
		mbHasFocus = true;
		mDateBeforeFocus = date ();
		if ( ownerItem () )
		{
			vmTableWidget* table ( static_cast<vmListWidget*>( const_cast<vmTableItem*>( ownerItem () )->table () ) );
			table->setCurrentItem ( const_cast<vmTableItem*>( ownerItem () ) );
		}
		e->setAccepted ( true );
		QDateEdit::focusInEvent ( e );
	}
	else
		e->setAccepted ( false );
}

void pvmDateEdit::focusOutEvent ( QFocusEvent* e )
{
	if ( isEditable () )
	{
		mbHasFocus = false;
		vmDateChanged ( this->date () );
		e->setAccepted ( true );
		QDateEdit::focusOutEvent ( e );
	}
	else
		e->setAccepted ( false );
}

void pvmDateEdit::contextMenuEvent ( QContextMenuEvent* e )
{
	if ( isEditable () )
	{
		// ignore the context menu for QDateEdit->QAbstractSpinBox. The widget that will receive the event is our mLineEdit
		e->ignore ();
		if ( mOwner->ownerItem () != nullptr )
		{
			// see: void vmLineEdit::contextMenuEvent ( QContextMenuEvent* e )
			mOwner->ownerItem ()->table ()->displayContextMenuForCell ( mapTo ( mOwner->ownerItem ()->table (), e->pos () ) );
		}
		else
			QDateEdit::contextMenuEvent ( e );
	}
}

vmDateEdit::vmDateEdit ( QWidget* parent )
	: QWidget ( parent ), vmWidget ( WT_DATEEDIT ), mDateEdit ( new pvmDateEdit ( this ) ),
	  mButton ( new QToolButton )
{
	setWidgetPtr ( static_cast<QWidget*>( this ) );
	mButton = new QToolButton;
	mButton->setIcon ( ICON ( "x-office-calendar" ) );
	static_cast<void>( connect ( mButton, &QToolButton::clicked, this, [&] () { return datesButtonMenuRequested (); } ) );

	QHBoxLayout* mainLayout ( new QHBoxLayout );
	mainLayout->setMargin ( 0 );
	mainLayout->setSpacing ( 1 );
	mainLayout->addWidget ( mDateEdit, 1 );
	mainLayout->addWidget ( mButton );
	setLayout ( mainLayout );
}

vmDateEdit::~vmDateEdit ()
{
	heap_del ( mDateEdit );
	heap_del ( mButton );
}

void vmDateEdit::setID ( const int id )
{
	mDateEdit->setID ( id );
	vmWidget::setID ( id );
}

QString vmDateEdit::defaultStyleSheet () const
{
	return mDateEdit->defaultStyleSheet ();
}

void vmDateEdit::setDate ( const vmNumber& date, const bool b_notify )
{
	mDateEdit->setDate ( date, b_notify );
}

const QDate vmDateEdit::date () const
{
	return mDateEdit->date ();
}

void vmDateEdit::setEditable ( const bool editable )
{
	mDateEdit->setEditable ( editable );
	mButton->setEnabled ( editable );
	vmWidget::setEditable ( editable );
}

void vmDateEdit::datesButtonMenuRequested ()
{
	static_cast<void>( disconnect ( menuDateButtons, nullptr, nullptr, nullptr ) );
	mButton->setMenu ( menuDateButtons );
	static_cast<void>( connect ( menuDateButtons, &QMenu::triggered, this, [&] ( QAction* action ) {
		return execDateButtonsMenu ( static_cast<vmAction*> ( action ), this->mDateEdit ); } ) );
	mButton->showMenu ();
}

void vmDateEdit::execDateButtonsMenu ( const vmAction* const action, pvmDateEdit* dte )
{
	dte->setDate ( vmNumber ( action->internalData ().toDate () ), true );
}

void vmDateEdit::createDateButtonsMenu ( QWidget* parent )
{
	menuDateButtons = new QMenu ( parent );
	vmAction* dateAction ( nullptr );

	vmNumber date ( vmNumber::currentDate );
	dateAction = new vmAction ( 0, TR_FUNC ( "Today" ), parent );
	dateAction->setInternalData ( date.toQDate () );
	menuDateButtons->addAction ( dateAction );

	date.setDay ( -1, true );
	dateAction = new vmAction ( 1, TR_FUNC ( "Yesterday" ), parent );
	dateAction->setInternalData ( date.toQDate () );
	menuDateButtons->addAction ( dateAction );

	date.setDay ( +2, true );
	dateAction = new vmAction ( 1, TR_FUNC ( "Tomorrow" ), parent );
	dateAction->setInternalData ( date.toQDate () );
	menuDateButtons->addAction ( dateAction );

	Sys_Init::addPostRoutine ( deleteMenuDateButtons );
}

// Called by MainWindow when current date changes
void vmDateEdit::updateDateButtonsMenu ()
{
	vmNumber date ( vmNumber::currentDate );
	static_cast<vmAction*>( menuDateButtons->actions ().at ( 0 ) )->setInternalData ( date.toQDate () );
	date.setDay ( -1, true );
	static_cast<vmAction*>( menuDateButtons->actions ().at ( 1 ) )->setInternalData ( date.toQDate () );
	date.setDay ( +2, true );
	static_cast<vmAction*>( menuDateButtons->actions ().at ( 2 ) )->setInternalData ( date.toQDate () );
}

void vmDateEdit::updateRecentUsedDates ( const vmNumber& date )
{
	uint i ( 0 );
	uint n_actions ( static_cast<uint>(menuDateButtons->actions ().count ()) );
	if ( n_actions < N_DATES_MENU_STATIC_ENTRIES )
	{
		menuDateButtons->addSeparator ();
		n_actions = N_DATES_MENU_STATIC_ENTRIES;
	}
	
	for ( ; i < n_actions; ++i )
	{
		if ( i != 3 ) //separator index
		{
			if ( date.toQDate () == static_cast<vmAction*>( menuDateButtons->actions ().at ( static_cast<int>( i ) ) )->internalData ().toDate () )
				return;
		}
	}
	
	vmAction* action ( nullptr );
	if ( n_actions < ( N_DATES_MENU_STATIC_ENTRIES + MAX_RECENT_USE_DATES ) )
	{
		action = new vmAction ( N_DATES_MENU_STATIC_ENTRIES, date.toDate ( vmNumber::VDF_HUMAN_DATE ) );
		action->setInternalData ( date.toQDate () );
		menuDateButtons->addAction ( action );
		n_actions++;
	}
	else
	{
		static int ins_pos ( N_DATES_MENU_STATIC_ENTRIES );
		action = static_cast<vmAction*>( menuDateButtons->actions ().at ( ins_pos ) );
		action->setInternalData ( date.toQDate () );
		action->QAction::setText ( date.toDate ( vmNumber::VDF_HUMAN_DATE ) );
		if ( ++ins_pos == ( N_DATES_MENU_STATIC_ENTRIES + MAX_RECENT_USE_DATES ) )
			ins_pos = N_DATES_MENU_STATIC_ENTRIES;
	}
}

QMenu* vmDateEdit::standardContextMenu () const
{ 
	return mDateEdit->standardContextMenu ();
}

void vmDateEdit::setTabOrder ( QWidget* formOwner, QWidget* prevWidget, QWidget* nextWidget )
{
	formOwner->setTabOrder ( prevWidget, mDateEdit );
	formOwner->setTabOrder ( mDateEdit, mButton );
	formOwner->setTabOrder ( mButton, nextWidget );
}
//------------------------------------------------VM-DATE-EDIT-BUTTON------------------------------------------------

//------------------------------------------------VM-TIME-EDIT------------------------------------------------
vmTimeEdit::vmTimeEdit ( QWidget* parent )
	: QTimeEdit ( parent ), vmWidget ( WT_TIMEEDIT )
{
	setWidgetPtr ( static_cast<QWidget*> ( this ) );
}

vmTimeEdit::~vmTimeEdit () {}

QString vmTimeEdit::defaultStyleSheet () const
{
	QString colorstr;
	if ( !parentWidget () )
		colorstr = QStringLiteral ( " ( 255, 255, 255 ) }" );
	else
	{
		const QTimeEdit* time ( new QTimeEdit ( parentWidget () ) );
		colorstr = " ( " + time->palette ().color ( time->backgroundRole () ).name ()
				   + " ) }";
		delete time;
	}
	return ( QLatin1String ( "QTimeEdit { background-color: hex" ) + colorstr );
}

void vmTimeEdit::setTime ( const vmNumber& time, const bool b_notify )
{
	QTimeEdit::setTime ( time.toQTime () );
	if ( b_notify && contentsAltered_func )
		contentsAltered_func ( this );
}

void vmTimeEdit::keyPressEvent ( QKeyEvent* e )
{
	if ( isEditable () )
	{
		if ( e->key () == Qt::Key_Enter || e->key () == Qt::Key_Return || e->key () == Qt::Key_Escape )
		{
			if ( keypressed_func )
				keypressed_func ( e, this );
		}
		QTimeEdit::keyPressEvent ( e );
	}
	e->setAccepted ( true );
}

void vmTimeEdit::focusInEvent ( QFocusEvent* e )
{
	if ( isEditable () )
	{
		mTimeBeforeFocus = time ();
		if ( ownerItem () )
		{	
			vmTableWidget* table ( static_cast<vmListWidget*>( const_cast<vmTableItem*> ( ownerItem () )->table () ) );
			table->setCurrentItem ( const_cast<vmTableItem*> ( ownerItem () ) );
		}
		e->setAccepted ( true );
		QTimeEdit::focusInEvent ( e );
	}
	else
		e->setAccepted ( false );
}

void vmTimeEdit::focusOutEvent ( QFocusEvent* e )
{
	if ( isEditable () )
	{
		if ( ( mTimeBeforeFocus != time () ) && contentsAltered_func )
			contentsAltered_func ( this );
		e->setAccepted ( true );
		QTimeEdit::focusOutEvent ( e );
	}
	else
		e->setAccepted ( false );
}
//------------------------------------------------VM-TIME-EDIT------------------------------------------------

//------------------------------------------------LINE-EDIT-LINK----------------------------------------------
vmLineEdit::vmLineEdit ( QWidget* parent, QWidget* ownerWindow )
	: QLineEdit ( parent ), vmWidget ( WT_LINEEDIT ), mCtrlKey ( false ), mCursorChanged ( false ),
	  b_widgetCannotGetFocus ( false ), mbTrack ( false ),
	  mouseClicked_func ( nullptr )
{
	setWidgetPtr ( static_cast<QWidget*>( this ) );
	if  ( ownerWindow != nullptr )
	{
		if ( ( ownerWindow->windowFlags () & Qt::Drawer ) == Qt::Drawer )
			b_widgetCannotGetFocus = true;
	}
}

vmLineEdit::~vmLineEdit () {}

QString vmLineEdit::defaultStyleSheet () const
{
	QString colorstr;
	if ( !parentWidget () )
		colorstr = QStringLiteral ( " ( 255, 255, 255 ) }" );
	else
	{
		const QLineEdit* line ( new QLineEdit ( parentWidget () ) );
		colorstr = QLatin1String ( " ( " ) + line->palette ().color ( line->backgroundRole () ).name ()
				   + QLatin1String ( " ) }" );
		delete line;
	}
	return ( QLatin1String ( "QLineEdit { background-color: hex" ) + colorstr );
}

void vmLineEdit::highlight ( const VMColors vm_color, const QString& str )
{
	vmWidget::highlight ( vm_color );
	if ( !str.isEmpty () )
	{
		if ( vm_color != vmDefault_Color )
		{
			setSelection ( text ().indexOf ( str, 0, Qt::CaseInsensitive ), str.length () );
			return;
		}
	}
	deselect ();
}

void vmLineEdit::setText ( const QString& text, const bool b_notify )
{
	if ( textType () >= TT_PRICE )
	{
		vmNumber n;
		switch ( textType () )
		{
			case TT_PRICE:
				n.fromStrPrice ( text );
			break;
			case TT_DOUBLE:
				n.fromStrDouble ( text );
			break;
			case TT_INTEGER:
				n.fromStrInt ( text );
			break;
			case TT_TEXT:
			case TT_NUMBER_PLUS_SYMBOL:
			case TT_PHONE:
			case TT_UPPERCASE:
			break;
		}
		QLineEdit::setText ( n.toString () );
		QPalette palette;
		palette.setColor ( QPalette::Text, n < 0 ? Qt::red : Qt::blue );
		setPalette ( palette );
	}
	else
		QLineEdit::setText ( text );

	if ( text != mCurrentText )
	{
		if ( b_notify && contentsAltered_func )
			contentsAltered_func ( this ); // call before updating mCurrentText, so that the callee might use the old value
		mCurrentText = text;
	}
	//setToolTip ( isEditable () ? emptyString : QLineEdit::text () );
	setToolTip ( mCurrentText );
	setCursorPosition ( isEditable () ? mCurrentText.count () - 1 : 0 );
}

void vmLineEdit::setTrackingEnabled ( const bool b_tracking )
{
	mbTrack = b_tracking;
}

void vmLineEdit::setEditable ( const bool editable )
{
	setReadOnly ( !editable );
	setClearButtonEnabled ( editable );
	vmWidget::setEditable ( editable );
	setCursorPosition ( editable ? QLineEdit::text ().count () - 1 : 0 );
}

QMenu* vmLineEdit::standardContextMenu () const
{
	QMenu* menu ( const_cast<vmLineEdit*>( this )->createStandardContextMenu () );
	if ( isEditable () )
	{
		menu->addSeparator();
		menu->addAction ( TR_FUNC ( "Clear" ), this, SLOT ( clear () ) );
	}
	return menu;
}

void vmLineEdit::completerClickReceived ( const QString& value )
{
	if ( hasFocus () )
		setText ( value, true );
}

void vmLineEdit::updateText ()
{
	vmNumber nbr;
	switch ( textType () )
	{
		case TT_TEXT:
		case TT_NUMBER_PLUS_SYMBOL:
		break;
		case TT_UPPERCASE:
			setText ( text ().toUpper () );
		break;
		case TT_PRICE:
			nbr.fromStrPrice ( text () );
			setText ( nbr.toPrice () );
		break;
		case TT_DOUBLE:
			nbr.fromStrDouble ( text () );
			setText ( nbr.toStrDouble () );
		break;
		case TT_PHONE:
			nbr.fromStrPhone ( text () );
			setText ( nbr.toPhone () );
		break;
		case TT_INTEGER:
			nbr.fromStrInt ( text () );
			setText ( nbr.toStrInt () );
		break;
	}
	if ( contentsAltered_func )
		contentsAltered_func ( this );
}

void vmLineEdit::keyPressEvent ( QKeyEvent* e )
{
	if ( isEditable () )
	{
		mCtrlKey = ( e->modifiers () == Qt::ControlModifier );
		mCursorChanged = false;
		switch ( e->key () )
		{
			case Qt::Key_Enter:
			case Qt::Key_Return:
			case Qt::Key_Escape:
			case Qt::Key_F2:
			case Qt::Key_F3:
			case Qt::Key_F4:
			case Qt::Key_Tab:
				if ( completer () && completer ()->popup () && completer ()->popup ()->isVisible () )
				{
					e->ignore ();
					return; // let the completer do its default behavior
				}
				else
				{
					if ( e->key () == Qt::Key_Enter || e->key () == Qt::Key_Return )
						updateText ();
					if ( keypressed_func )
						keypressed_func ( e, this );
					e->accept ();
				}
			break;
			default:
				e->ignore ();
				QLineEdit::keyPressEvent ( e );
			break;
		}
	}
	else // propagate only special keys. They might be used by the widgets parent(s)
		e->key () > Qt::Key_Shift ? e->accept () : e->ignore ();
}

void vmLineEdit::keyReleaseEvent ( QKeyEvent* e )
{
	if ( isEditable () )
	{
		mCursorChanged = mCtrlKey = false;
		QLineEdit::keyReleaseEvent ( e );
	}
}

void vmLineEdit::mouseMoveEvent ( QMouseEvent* e )
{
	if ( mbTrack )
	{
		if ( !hasFocus () )
			setFocus ( Qt::MouseFocusReason );

		if ( mCtrlKey || b_widgetCannotGetFocus )
		{
			if ( !mCursorChanged )
			{
				if ( !text().isEmpty () )
				{
					setCursor ( QCursor ( Qt::PointingHandCursor ) );
					mCursorChanged = true;
				}
			}
		}
		else
		{
			if ( !mCursorChanged )
			{
				setCursor ( QCursor ( Qt::ArrowCursor ) );
				mCursorChanged = true;
			}
		}
		QLineEdit::mouseMoveEvent ( e );
		return;
	}
	e->ignore ();
}

void vmLineEdit::mouseReleaseEvent ( QMouseEvent* e )
{
	if ( mbTrack )
	{
		if  ( mCtrlKey || b_widgetCannotGetFocus )
		{
			if ( !text ().isEmpty () )
			{
				if ( mouseClicked_func )
					mouseClicked_func ( this );
				mCtrlKey = mCursorChanged = false;
			}
		}
		QLineEdit::mouseReleaseEvent ( e );
	}
	e->ignore ();
}

void vmLineEdit::contextMenuEvent ( QContextMenuEvent* e )
{
	if ( !ownerItem () )
	{
		mbButtonClicked = false;
		QMenu* menu ( createStandardContextMenu () );
		if ( isEditable () )
		{
			menu->addSeparator();
			menu->addAction ( TR_FUNC ( "Clear" ), this, SLOT ( clear () ) );
		}
		menu->exec ( e->globalPos () );
		delete menu;
		e->accept ();
	}
	else
	{
		//e->ignore ();
		/* Before Ubuntu 17.10 (Qt 5.9.1) this line was not necessary. The table would receive the event signal
		 * because we were ignoring it. Now we have to explicitly ask the table to display the menu and give it
		 * the position too
		 */
		ownerItem ()->table ()->displayContextMenuForCell ( mapTo ( ownerItem ()->table (), e->pos () ) );
	}
}

void vmLineEdit::focusInEvent ( QFocusEvent* e )
{
	/* We bypass Qt::QLineEdit::focusInEvent entirely. Looking up the source code for the
	 * widget I noticed several of the things I do here and those conflict with my code.
	 * This way, execution is faster and cleaner and I can do whatever I want without external interference
	 */
	if ( isEditable () && e->reason () != Qt::ActiveWindowFocusReason )
	{
		if ( completer () != nullptr )
		{
			if ( completer ()->popup () && completer ()->popup ()->isVisible () )
			{
				e->ignore ();
				return;
			}
			if ( completer ()->widget () == nullptr )
			{
				completer ()->setWidget ( this );
				static_cast<void>( connect ( completer (), static_cast<void (QCompleter::*)(const QString&)>( &QCompleter::activated ),
					this, [&] ( const QString& value ) { return completerClickReceived ( value ); } ) );
			}
		}
		mbButtonClicked = false;
		mCurrentText = text ();
		
		if ( ownerItem () )
		{
			vmTableWidget* table ( static_cast<vmListWidget*>( const_cast<vmTableItem*>( ownerItem () )->table () ) );
			table->setCurrentItem ( const_cast<vmTableItem*>( ownerItem () ) );
		}
		e->setAccepted ( true );
	}
	else // not editable
	{
		if ( ownerItem () )
		{	
			vmTableWidget* table ( static_cast<vmListWidget*>( const_cast<vmTableItem*>( ownerItem () )->table () ) );
			if ( table->isPlainTable () && ownerItem ()->row () != table->currentRow () )
			{
				table->setCurrentItem ( const_cast<vmTableItem*>( ownerItem () ) );
				table->callRowActivated_func ( ownerItem ()->row () );
				e->setAccepted ( true );
				return;
			}
		}
		e->setAccepted ( false );
	}
	QLineEdit::focusInEvent ( e );
}

void vmLineEdit::focusOutEvent ( QFocusEvent* e )
{
	if ( isEditable () && ( mCurrentText != text () ) )
	{
		updateText ();
		e->setAccepted ( true );
	}
	else
		e->setAccepted ( false );
	QLineEdit::focusOutEvent ( e );
}
//------------------------------------------------LINE-EDIT-LINK----------------------------------------------

//------------------------------------------------VM-LINE-EDIT-BUTTON-----------------------------------------------
vmLineEditWithButton::vmLineEditWithButton ( QWidget* parent )
	: QWidget ( parent ), vmWidget ( WT_LINEEDIT_WITH_BUTTON ), mLineEdit ( new vmLineEdit ),
	  mButton ( new QToolButton ), buttonclicked_func ( nullptr )
{
	setWidgetPtr ( static_cast<QWidget*>( this ) );
	QHBoxLayout *mainLayout ( new QHBoxLayout );
	mainLayout->setMargin ( 0 );
	mainLayout->setSpacing ( 1 );
	mainLayout->addWidget ( mLineEdit, 1 );
	mainLayout->addWidget ( mButton );
	setLayout ( mainLayout );
}

vmLineEditWithButton::~vmLineEditWithButton ()
{
	delete mLineEdit;
	delete mButton;
}

void vmLineEditWithButton::setButtonType ( const LINE_EDIT_BUTTON_TYPE type )
{
	mBtnType = type;
	switch ( mBtnType )
	{
		case LEBT_CALC_BUTTON: 
			mButton->setIcon ( ICON ( "accessories-calculator" ) ); 
			mButton->setToolTip ( TR_FUNC ( "Use calculator" ) );
		break;
		case LEBT_DIALOG_BUTTON: 
			mButton->setIcon ( ICON ( "folder-templates" ) );
			mButton->setToolTip ( TR_FUNC ( "Select File" ) );
		break;
		case LEBT_NO_BUTTON:
		case LEBT_CUSTOM_BUTTOM:
		break;
	}
	static_cast<void>( connect ( mButton, &QToolButton::clicked, this, [&] () { return execButtonAction (); } ) );
	//mButton->setEnabled ( isEditable () );
}

void vmLineEditWithButton::setEditable ( const bool editable )
{
	mLineEdit->setEditable ( editable );
	mButton->setEnabled ( editable );
	vmWidget::setEditable ( editable );
}

void vmLineEditWithButton::execButtonAction ()
{
	mLineEdit->mbButtonClicked = true;
	if ( buttonclicked_func != nullptr )
	{
		buttonclicked_func ();
		return;
	}
	
	switch ( mBtnType )
	{
		case LEBT_CALC_BUTTON:
		{
			vmNumber price ( mLineEdit->text (), VMNT_DOUBLE );
			CALCULATOR ()->showCalc ( price.toStrDouble (), mButton->mapToGlobal ( mButton->pos () ), mLineEdit, nullptr );
		}
		break;
		case LEBT_DIALOG_BUTTON:
			mLineEdit->setText ( fileOps::getOpenFileName ( CONFIG ()->homeDir () , TR_FUNC ( "All Files ( * ) " ) ), true );
		break;
		case LEBT_CUSTOM_BUTTOM:
		case LEBT_NO_BUTTON:
		break;
	}
}
//------------------------------------------------VM-LINE-EDIT-BUTTON------------------------------------------------

//------------------------------------------------VM-COMBO-BOX------------------------------------------------
vmComboBox::vmComboBox ( QWidget* parent )
	: QComboBox ( parent ), vmWidget ( WT_COMBO ), mbIgnoreChanges ( true ),
		mLineEdit ( new vmLineEdit ), indexChanged_func ( nullptr ), activated_func ( nullptr ),
		keyEnter_func ( nullptr ), keyEsc_func ( nullptr )
{
	setWidgetPtr ( static_cast<QWidget*> ( this ) );
	setLineEdit ( mLineEdit );
	mLineEdit->setVmParent ( this );
	setInsertPolicy ( QComboBox::NoInsert );
}

vmComboBox::~vmComboBox () {}

QString vmComboBox::defaultStyleSheet () const
{
	QString colorstr;
	if ( !parentWidget () )
		colorstr = QStringLiteral ( " ( 255, 255, 255 ) }" );
	else
	{
		const QComboBox* combo ( new QComboBox ( parentWidget () ) );
		colorstr = QLatin1String ( " ( " ) + combo->palette ().color ( combo->backgroundRole () ).name ()
				   + QLatin1String ( " ) }" );
		delete combo;
	}
	return ( QLatin1String ( "QComboBox { background-color: hex" ) + colorstr );
}

void vmComboBox::highlight ( const VMColors vm_color, const QString& str )
{
	vmWidget::highlight ( vm_color );
	if ( !str.isEmpty () )
	{
		if ( vm_color != vmDefault_Color )
		{
			setCurrentIndex ( findText ( str, Qt::MatchContains ) );
			mLineEdit->setSelection ( mLineEdit->text ().indexOf ( str, 0, Qt::CaseInsensitive ), str.length () );
			return;
		}
	}
	mLineEdit->deselect ();
}

void vmComboBox::currentIndexChanged_slot ( const int idx )
{
	if ( !mbIgnoreChanges )
	{
		if ( idx >= 0 )
		{
			if ( vmWidget::isEditable () && contentsAltered_func )
				contentsAltered_func ( mLineEdit );
			else
			{
				if ( indexChanged_func )
					indexChanged_func ( idx );
			}
		}
	}
}

void vmComboBox::setText ( const QString& text, const bool b_notify )
{
	//setEditText ( text );
	mLineEdit->setText ( text, false );
	if ( b_notify && contentsAltered_func )
		contentsAltered_func ( mLineEdit );
}

void vmComboBox::setCompleter ( const vmCompleters::COMPLETER_CATEGORIES completer )
{
	QStringList cboList;
	APP_COMPLETERS ()->fillList ( completer, cboList );
	mLineEdit->setCompleter ( APP_COMPLETERS ()->getCompleter ( completer ) );
	QComboBox::addItems ( cboList );
}

void vmComboBox::setIgnoreChanges ( const bool b_ignore )
{
	mbIgnoreChanges = b_ignore;
	if ( !mbIgnoreChanges )
	{
		if ( indexChanged_func != nullptr )
		{
			static_cast<void>( connect ( this, static_cast<void (QComboBox::*)(int)>( &QComboBox::currentIndexChanged ), this, [&] ( int idx ) {
				return currentIndexChanged_slot ( idx ); } ) );
		}
		if ( activated_func != nullptr )
		{
			static_cast<void>( connect ( this, static_cast<void (QComboBox::*)(int)>( &QComboBox::activated ), this, [&] ( int idx ) {
				if ( idx >= 0 ) activated_func ( idx ); } ) );
		}
	}
	else 
		static_cast<void>( disconnect ( this, nullptr, nullptr, nullptr ) );
}

void vmComboBox::setEditable ( const bool editable )
{
	// Can't change QComboBox editable state because when going from editable to non-editable
	// Qt destroys the line edit. Upon returning to editable, there is no more line edit.
	mLineEdit->setEditable ( editable );
	vmWidget::setEditable ( editable );
	if ( editable )
	{
		mbKeyEnterPressedOnce = false;
		mbKeyEscPressedOnce = false;
	}
}

void vmComboBox::insertItem ( const QString& text, const int idx, const bool b_check )
{
	const bool b_editable ( vmWidget::isEditable () );
	const bool b_ignorechanges ( mbIgnoreChanges );
	setEditable ( false );
	setIgnoreChanges ( true );
	bool ok ( true );
	if ( b_check )
		ok = ( findText ( text, Qt::MatchFixedString ) == -1 );
	if ( ok )
		QComboBox::insertItem ( idx, text );
	setIgnoreChanges ( b_ignorechanges );
	setEditable ( b_editable );
}

void vmComboBox::keyPressEvent ( QKeyEvent *e )
{
	if ( vmWidget::isEditable () )
	{
		switch ( e->key () )
		{
			case Qt::Key_Enter:
			case Qt::Key_Return:
				if ( !completer () || !completer ()->popup () || !completer ()->popup ()->isVisible () )
				{
					if ( !mbKeyEnterPressedOnce && keyEnter_func )
						keyEnter_func ();
					else
					{
						if ( keypressed_func )
							keypressed_func ( e, this );
					}
					mbKeyEnterPressedOnce = !mbKeyEnterPressedOnce;
					e->accept ();
				}
				else
					e->ignore (); // let the completer receive the event and choose the activated item
			return;
			
			case Qt::Key_Escape:
				if ( !completer () || !completer ()->popup () || !completer ()->popup ()->isVisible () )
				{
					if ( !mbKeyEscPressedOnce && keyEsc_func )
						keyEsc_func ();
					else
					{
						if ( keypressed_func )
							keypressed_func ( e, this );
					}
					mbKeyEscPressedOnce = !mbKeyEscPressedOnce;
					e->accept (); // let the completer receive the event end hide itself without activating anything
				}
				else
					e->ignore ();
			return;

			default:
			break;
		}
	}
	QComboBox::keyPressEvent ( e );
}

void vmComboBox::focusInEvent ( QFocusEvent* e )
{
	if ( vmWidget::isEditable () )
	{
		e->setAccepted ( true );
		QComboBox::focusInEvent ( e );
	}
	else
		e->setAccepted ( false );
}

void vmComboBox::focusOutEvent ( QFocusEvent* e )
{
	if ( vmWidget::isEditable () )
	{
		e->setAccepted ( true );
		QComboBox::focusOutEvent ( e );
	}
	else
		e->setAccepted ( false );
}

void vmComboBox::wheelEvent ( QWheelEvent *e )
{
	if ( !vmWidget::isEditable () )
	{
		if ( mbIgnoreChanges || ( completer () && completer ()->popup () && completer ()->popup ()->isVisible () ) )
		{
			e->setAccepted ( false );
			return;
		}
	}
	e->setAccepted ( true );
	QComboBox::wheelEvent ( e );
}
//------------------------------------------------VM-COMBO-BOX------------------------------------------------

//------------------------------------------------VM-CHECK-BOX------------------------------------------------
vmCheckBox::vmCheckBox ( QWidget* parent )
	: QCheckBox ( parent ), vmWidget ( WT_CHECKBOX )
{
	setWidgetPtr ( static_cast<QWidget*>( this ) );
}

vmCheckBox::vmCheckBox ( const QString& text, QWidget* parent )
	: QCheckBox ( text, parent ), vmWidget ( WT_CHECKBOX )
{
	setWidgetPtr ( static_cast<QWidget*>( this ) );
}

vmCheckBox::~vmCheckBox () {}

QString vmCheckBox::defaultStyleSheet () const
{
	QString colorstr;
	if ( !parentWidget () )
		colorstr = QStringLiteral ( " ( 255, 255, 255 ) }" );
	else
	{
		const QCheckBox* check ( new QCheckBox ( parentWidget () ) );
		colorstr = QLatin1String ( " ( " ) + check->palette ().color ( check->backgroundRole () ).name ()
				   + QLatin1String ( " ) }" );
		delete check;
	}
	return ( QLatin1String ( "QCheckBox { background-color: hex" ) + colorstr );
}

void vmCheckBox::setText ( const QString& text, const bool b_notify )
{
	if ( text.length () <= 1 )
	{	
		QCheckBox::setChecked ( text == CHR_ONE );
		if ( b_notify && contentsAltered_func )
			contentsAltered_func ( this );
	}
	else
		QCheckBox::setText ( text );	
}

void vmCheckBox::setEditable ( const bool editable )
{
	if ( editable )
		static_cast<void>( connect ( this, static_cast<void (QCheckBox::*)(const bool)>( &QCheckBox::clicked ), this, [&] ( const bool ) {
			return contentsAltered_func ( this ); } ) ); // clicked means mouse clicked, keyboard activated (enter, space bar) or shutcut activated
	else
		static_cast<void>( disconnect ( this, nullptr, nullptr, nullptr ) );
	setEnabled ( editable );
	vmWidget::setEditable ( editable );
}
//------------------------------------------------VM-CHECK-BOX------------------------------------------------
