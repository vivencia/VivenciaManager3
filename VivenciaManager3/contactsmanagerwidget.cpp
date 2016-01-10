#include "contactsmanagerwidget.h"
#include "vmwidgets.h"
#include "fileops.h"
#include "data.h"
#include "vmnotify.h"

#include <QToolButton>
#include <QHBoxLayout>

const uint CMW_N_TYPES ( 2 );

const char* const BTN_ADD_TOOLTIP_CHECKED[CMW_N_TYPES] = {
	"Click again to save phone number", "Click again to save the email address"
};

const char* const BTN_ADD_TOOLTIP_UNCHECKED[CMW_N_TYPES] = {
	"Insert a new phone number", "Insert a new email address"
};

const char* const BTN_DEL_TOOLTIP[CMW_N_TYPES] = {
	"Remove selected telephone number", "Remove selected address"
};

const char* const BTN_CANCEL_TOOLTIP[CMW_N_TYPES] = {
	"Cancel insertion of telephone number", "Cancel insertion of email or site address"
};

contactsManagerWidget::contactsManagerWidget ( QWidget* parent, const CMW_TYPE type )
	: QWidget ( parent ), vmWidget ( WT_COMBO ), cboInfoData ( nullptr ), btnExtra ( nullptr ),
	  m_contact_type ( type ), insertFunc ( nullptr ), removeFunc ( nullptr ) {}

contactsManagerWidget::~contactsManagerWidget () {}

void contactsManagerWidget::initInterface ()
{
	if ( cboInfoData != nullptr )
		return;

	cboInfoData = new vmComboBox ( this );
	cboInfoData->setMinimumWidth ( 200 );
	if ( m_contact_type == CMW_PHONES )
		cboInfoData->setTextType ( vmWidget::TT_PHONE );
	cboInfoData->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
		return cbo_textAltered ( sender->text () ); });
	cboInfoData->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const sender ) {
		return keyPressedSelector ( ke, sender ); } );
	cboInfoData->setCallbackForEnterKeyPressed ( [&] () {
		return btnAdd_clicked ( false ); } );
	cboInfoData->setCallbackForEscKeyPressed ( [&] () {
		return btnDel_clicked (); } );

	btnAdd = new QToolButton ( this );
	btnAdd->setCheckable ( true );
	btnAdd->setIcon ( ICON ( "add.png" ) );
	connect ( btnAdd, static_cast<void (QToolButton::*)( bool )>( &QToolButton::clicked ), this, [&] ( const bool checked ) {
		return btnAdd_clicked ( checked ); } );
	btnDel = new QToolButton ( this );
	btnDel->setIcon ( ICON ( "remove.png" ) );
	connect ( btnDel, &QToolButton::clicked, this, [&] () {
		return btnDel_clicked (); } );

	QHBoxLayout* mainLayout ( new QHBoxLayout );
	mainLayout->setMargin ( 0 );
	mainLayout->setSpacing ( 5 );
	mainLayout->addWidget ( cboInfoData, 1 );
	mainLayout->addWidget ( btnAdd );
	mainLayout->addWidget ( btnDel );

	if ( m_contact_type == CMW_EMAIL ) {
		btnExtra = new QToolButton ( this );
		btnExtra->setIcon ( ICON ( "email.png" ) );
		connect ( btnExtra, &QToolButton::clicked, this, [&] () {
			return btnExtra_clicked (); } );
		btnExtra->setEnabled ( false );
		mainLayout->addWidget ( btnExtra );
	}

	setLayout ( mainLayout );
}

void contactsManagerWidget::setEditable ( const bool editable )
{
	if ( !editable ) {
		btnAdd->setChecked ( false );
		btnAdd->setToolTip ( BTN_ADD_TOOLTIP_CHECKED[static_cast<uint> ( m_contact_type )] );
		btnDel->setToolTip ( BTN_DEL_TOOLTIP[static_cast<uint> ( m_contact_type )] );
	}
	else {
		btnAdd->setChecked ( cboInfoData->currentText ().isEmpty () ? false : cboInfoData->findText ( cboInfoData->currentText () ) == -1 );
		btnAdd->setToolTip ( BTN_ADD_TOOLTIP_UNCHECKED[static_cast<uint> ( m_contact_type )] );
		btnDel->setToolTip ( btnAdd->isChecked () ? BTN_CANCEL_TOOLTIP[static_cast<uint> ( m_contact_type )] : BTN_DEL_TOOLTIP[static_cast<uint> ( m_contact_type )] );
	}
	btnAdd->setEnabled ( editable );
	cboInfoData->setEditable ( btnAdd->isChecked () );
	btnDel->setEnabled ( editable );
	vmWidget::setEditable ( editable );
}

void contactsManagerWidget::cbo_textAltered ( const QString& text )
{
	bool input_ok ( false );
	QString new_text;
	if ( m_contact_type == CMW_PHONES ) {
		const vmNumber phone ( cboInfoData->text (), VMNT_PHONE );
		input_ok = phone.isPhone ();
		if ( input_ok )
			new_text = phone.toPhone ();
	}
	else {
		input_ok = Data::isEmailAddress ( text );
		if ( input_ok )
			new_text = text.toLower ();
	}
	if ( input_ok )
		cboInfoData->setEditText ( new_text );
}

void contactsManagerWidget::keyPressedSelector ( const QKeyEvent* const ke, const vmWidget* const )
{
	if ( keypressed_func )
		keypressed_func ( ke, this );
}

void contactsManagerWidget::btnAdd_clicked ( const bool checked )
{
	cboInfoData->setEditable ( checked );
	if ( checked ) {
		cboInfoData->setText ( emptyString );
		cboInfoData->setFocus ();
		btnAdd->setToolTip ( tr ( BTN_ADD_TOOLTIP_CHECKED[static_cast<uint> ( m_contact_type )] ) );
		btnDel->setToolTip ( BTN_CANCEL_TOOLTIP[static_cast<uint> ( m_contact_type )] );
		btnDel->setEnabled ( true );
	}
	else {
		cboInfoData->editor ()->updateText ();
		bool input_ok ( false );
		if ( m_contact_type == CMW_PHONES ) {
			const vmNumber phone ( cboInfoData->text (), VMNT_PHONE );
			input_ok = phone.isPhone ();
		}
		else {
			input_ok = Data::isEmailAddress ( cboInfoData->text () );
			if ( input_ok )
				cboInfoData->setText ( cboInfoData->text () );
		}
		if ( input_ok ) {
			btnDel->setToolTip ( BTN_DEL_TOOLTIP[static_cast<uint> ( m_contact_type )] );
			btnDel->setEnabled ( true );
			btnAdd->setToolTip ( tr ( BTN_ADD_TOOLTIP_UNCHECKED[static_cast<uint> ( m_contact_type )] ) );
			insertItem ();
			if ( insertFunc )
				insertFunc ( cboInfoData->text (), this );
		}
		else {
			VM_NOTIFY ()->notifyMessage ( tr ( "Error" ), cboInfoData->text () + tr ( "is not a valid " ) +
										  ( m_contact_type == CMW_PHONES ? tr ( "phone number." ) : tr ( "email or site address." ) ) +
										  tr ( "\nCannot add it." ) );
		}
	}
	btnAdd->setChecked ( checked ); // for indirect calls, i.e. enter or esc keys pressed when within combo's line edit
}

void contactsManagerWidget::btnDel_clicked ()
{
	if ( cboInfoData->vmWidget::isEditable () ) { // now adding, then do cancel operation
		cboInfoData->setEditable ( false );
		cboInfoData->setCurrentIndex ( 0 );
		btnAdd->setChecked ( false );
		btnAdd->setToolTip ( BTN_ADD_TOOLTIP_UNCHECKED[static_cast<uint> ( m_contact_type )] );
		btnDel->setToolTip ( BTN_DEL_TOOLTIP[static_cast<uint> ( m_contact_type )] );
	}
	else { // now viewing, then do del operation
		int removed_idx ( -1 );
		const bool ok ( removeCurrent ( removed_idx ) );
		btnDel->setEnabled ( cboInfoData->count () > 0 );
		if ( ok && removeFunc )
			removeFunc ( removed_idx, this );
	}
}

void contactsManagerWidget::btnExtra_clicked ()
{
	switch ( m_contact_type ) {
	case CMW_PHONES:
		break;
	case CMW_EMAIL:
		fileOps::openAddress ( cboInfoData->currentText () );
		break;
	}
}

void contactsManagerWidget::decodePhones ( const stringRecord& phones, const bool bClear )
{
    if ( bClear )
        clearAll ();
	if ( !phones.isNull () ) {
		phones.first ();
		vmNumber phone ( phones.curValue (), VMNT_PHONE, 1 );
		cboInfoData->setIgnoreChanges ( true );
		cboInfoData->setEditText ( phone.toPhone () );
		do {
			cboInfoData->addItem ( phone.toPhone () );
			if ( phones.next () )
				phone.fromTrustedStrPhone ( phones.curValue () );
			else
				break;
		} while ( true );
		cboInfoData->setIgnoreChanges ( false );
	}
}

void contactsManagerWidget::decodeEmails ( const stringRecord& emails, const bool bClear )
{
    if ( bClear )
        clearAll ();
	if ( emails.first () ) {
		cboInfoData->setIgnoreChanges ( true );
		do {
			cboInfoData->addItem ( emails.curValue () );
		} while ( emails.next () );
		cboInfoData->setIgnoreChanges ( false );
	}
	btnExtra->setEnabled ( cboInfoData->count () > 0 );
}

void contactsManagerWidget::insertItem ()
{
	cboInfoData->setIgnoreChanges ( true );
	cboInfoData->setEditable ( false );
	btnAdd->setEnabled ( true );
	cboInfoData->addItem ( cboInfoData->text () );
	if ( btnExtra )
		btnExtra->setEnabled ( true );
	cboInfoData->setIgnoreChanges ( false );
}

bool contactsManagerWidget::removeCurrent ( int& removed_idx )
{
	removed_idx = cboInfoData->currentIndex ();
	if ( removed_idx >= 0 ) {
		cboInfoData->setIgnoreChanges ( true );
		cboInfoData->removeItem ( removed_idx );
		if ( btnExtra )
			btnExtra->setEnabled ( cboInfoData->count () > 0 );
		cboInfoData->setIgnoreChanges ( false );
	}
	return ( removed_idx >= 0 );
}


void contactsManagerWidget::clearAll ()
{
	cboInfoData->setIgnoreChanges ( true );
	cboInfoData->clear ();
	cboInfoData->clearEditText ();
	if ( btnExtra )
		btnExtra->setEnabled ( false );
	cboInfoData->setIgnoreChanges ( false );
}
