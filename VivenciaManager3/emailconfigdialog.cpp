#include "emailconfigdialog.h"
#include "data.h"
#include "global.h"
#include "vmwidgets.h"
#include "heapmanager.h"
#include "cleanup.h"
#include "fileops.h"

#include <QPushButton>
#include <QVBoxLayout>

static const QString PROFILE_NAME ( QLatin1String ( "CUSTOM_APP_" ) );
emailConfigDialog* emailConfigDialog::s_instance ( nullptr );

void deleteEmailConfigInstance ()
{
	heap_del ( emailConfigDialog::s_instance );
}

namespace ui
{
vmComboBox* cboProfiles ( nullptr );
vmLineEdit* txtAddress ( nullptr );
vmLineEdit* txtSubject ( nullptr );
vmLineEdit* txtAttach ( nullptr );
vmLineEdit* txtBody ( nullptr );
vmLineEdit* txtSeparator ( nullptr );
vmCheckBox* chkDefault ( nullptr );
vmLineEdit* txtProgram ( nullptr );
QToolButton* btnChooseApp ( nullptr );
QPushButton* btnAddProfile ( nullptr );
QPushButton* btnEditProfile ( nullptr );
QPushButton* btnSaveProfile ( nullptr );
QPushButton* btnCancelProfile ( nullptr );
QPushButton* btnClose ( nullptr );
}

struct KMAIL_OPTS : public emailOpts
{
	KMAIL_OPTS ()
		:	emailOpts () {
		emailOpts::fancy_name = QStringLiteral ( "KMail" );
		emailOpts::app_name = QStringLiteral ( "kmail" );
		emailOpts::addrs_switch = QStringLiteral ( "" );
		emailOpts::subject_switch = QStringLiteral ( "--subject " );
		emailOpts::attach_switch = QStringLiteral ( "--attach " );
		emailOpts::body_switch = QStringLiteral ( "--msg " );
		emailOpts::switches_sep = CHR_SPACE;
		emailOpts::bNewProfile = false;
		emailOpts::bProfileModified = false;
	}
};
static const KMAIL_OPTS kmail;

struct THUNDERBIRD_OPTS : public emailOpts
{
	THUNDERBIRD_OPTS ()
		:	emailOpts () {
		fancy_name = QStringLiteral ( "Mozilla Thunderbird" );
		app_name = QStringLiteral ( "thunderbird" );
		addrs_switch = QStringLiteral ( "-compose to=" );
		subject_switch = QStringLiteral ( "subject=" );
		attach_switch = QStringLiteral ( "attachment=" );
		body_switch = QStringLiteral ( "body=" );
		switches_sep = CHR_COMMA;
		bNewProfile = false;
		bProfileModified = false;
	}
};
static const THUNDERBIRD_OPTS thunderbird;

emailConfigDialog::emailConfigDialog ()
	: QDialog ( nullptr ), bProfilesLoaded ( false ), optsList ( 3 )
{
	addPostRoutine ( deleteEmailConfigInstance );
}

emailConfigDialog::~emailConfigDialog ()
{
	saveProfile ();
	optsList.clear ( true );
}

void emailConfigDialog::setupUi ()
{
	setWindowTitle ( tr ( "Configure e-email client" ) );
	QGridLayout* gLayout ( new QGridLayout );
	gLayout->setSpacing ( 2 );

	QHBoxLayout* horizontalLayout ( new QHBoxLayout );
	ui::btnAddProfile = new QPushButton ( ICON ( "browse-controls/add" ), tr ( "Add profile" ) );
	connect ( ui::btnAddProfile, &QPushButton::clicked, this, [&] () { return btnAddProfile_clicked (); } );
	ui::btnEditProfile = new QPushButton ( ICON ( "browse-controls/edit" ), tr ( "Edit profile" ) );
	connect ( ui::btnEditProfile, &QPushButton::clicked, this, [&] () { return btnEditProfile_clicked (); } );
	horizontalLayout->addWidget ( ui::btnAddProfile );
	horizontalLayout->addWidget ( ui::btnEditProfile );
	gLayout->addLayout ( horizontalLayout, 0, 0, 1, 1 );

	gLayout->addWidget ( new QLabel ( tr ( "Choose a profile:" ) ), 1, 0, 1, 1 );
	ui::cboProfiles = new vmComboBox;
	ui::cboProfiles->setCallbackForIndexChanged ( [&] ( const int idx ) { return cboProfiles_indexChanged ( idx ); } );
	ui::chkDefault = new vmCheckBox ( tr ( "Default" ) );

	horizontalLayout = new QHBoxLayout;
	horizontalLayout->setSpacing ( 0 );
	horizontalLayout->addWidget ( ui::cboProfiles );
	horizontalLayout->addWidget ( ui::chkDefault );
	gLayout->addLayout ( horizontalLayout, 1, 0, 1, 1 );

	QLabel* lblSwitches ( new QLabel ( tr ( "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
											"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
											"p, li { white-space: pre-wrap; }\n"
											"</style></head><body style=\" font-family:'Ubuntu'; font-size:11pt; font-weight:400; font-style:normal;\">\n"
											"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600; text-decoration: underline;\">Options switches</span></p></body></html>" ) ) );
	gLayout->addWidget ( lblSwitches, 2, 0, 1, 1 );

	gLayout->addWidget ( new QLabel ( tr ( "Program: " ) ), 3, 0, 1, 1 );
	ui::txtProgram = new vmLineEdit;
	ui::txtProgram->setCallbackForContentsAltered ( [&] ( const vmWidget* const ) { return optsList.current ()->bProfileModified = true; } );
	ui::btnChooseApp = new QToolButton ();
	ui::btnChooseApp->setIcon ( ICON ( "folder-brown" ) );
	connect ( ui::btnChooseApp, &QPushButton::clicked, this, [&] () { return btnChooseApp_clicked (); } );

	horizontalLayout = new QHBoxLayout;
	horizontalLayout->setSpacing ( 0 );
	horizontalLayout->addWidget ( ui::txtProgram );
	horizontalLayout->addWidget ( ui::btnChooseApp );
	gLayout->addLayout ( horizontalLayout, 3, 1, 1, 1 );

	gLayout->addWidget ( new QLabel ( tr ( "Address: " ) ), 4, 0, 1, 1 );
	ui::txtAddress = new vmLineEdit;
	ui::txtAddress->setCallbackForContentsAltered ( [&] ( const vmWidget* const ) { return optsList.current ()->bProfileModified = true; } );
	gLayout->addWidget ( ui::txtAddress, 4, 1, 1, 1 );

	gLayout->addWidget ( new QLabel ( tr ( "Subject: " ) ), 5, 0, 1, 1 );
	ui::txtSubject = new vmLineEdit;
	ui::txtSubject->setCallbackForContentsAltered ( [&] ( const vmWidget* const ) { return optsList.current ()->bProfileModified = true; } );
	gLayout->addWidget ( ui::txtSubject, 5, 1, 1, 1 );

	gLayout->addWidget ( new QLabel ( tr ( "Attachments: " ) ), 6, 0, 1, 1 );
	ui::txtAttach = new vmLineEdit;
	ui::txtAttach->setCallbackForContentsAltered ( [&] ( const vmWidget* const ) { return optsList.current ()->bProfileModified = true; } );
	gLayout->addWidget ( ui::txtAttach, 6, 1, 1, 1 );

	gLayout->addWidget ( new QLabel ( tr ( "Message body: " ) ), 7, 0, 1, 1 );
	ui::txtBody = new vmLineEdit;
	ui::txtBody->setCallbackForContentsAltered ( [&] ( const vmWidget* const ) { return optsList.current ()->bProfileModified = true; } );
	gLayout->addWidget ( ui::txtBody, 7, 1, 1, 1 );

	gLayout->addWidget ( new QLabel ( tr ( "Switches separator: " ) ), 8, 0, 1, 1 );
	ui::txtSeparator = new vmLineEdit;
	ui::txtSeparator->setCallbackForContentsAltered ( [&] ( const vmWidget* const ) { return optsList.current ()->bProfileModified = true; } );
	gLayout->addWidget ( ui::txtSeparator, 8, 1, 1, 1 );

	gLayout->setColumnStretch ( 0, 1 );
	gLayout->setColumnStretch ( 1, 1 );

	ui::btnSaveProfile = new QPushButton ( ICON ( "document-save" ), tr ( "Save" ) );
	connect ( ui::btnSaveProfile, &QPushButton::clicked, this, [&] () { return btnSaveProfile_clicked (); } );
	ui::btnCancelProfile = new QPushButton ( ICON ( "cancel" ), tr ( "Cancel" ) );
	connect ( ui::btnCancelProfile, &QPushButton::clicked, this, [&] () { return btnCancelProfile_clicked (); } );
	ui::btnClose = new QPushButton ( tr ( "Close" ) );
	connect ( ui::btnClose, &QPushButton::clicked, this, [&] () { return btnClose_clicked (); } );
	QHBoxLayout* buttonsLayout ( new QHBoxLayout );
	buttonsLayout->addWidget ( ui::btnSaveProfile, 1 );
	buttonsLayout->addWidget ( ui::btnCancelProfile, 1 );
	buttonsLayout->addWidget ( ui::btnClose, 1 );

	QVBoxLayout* mainLayout ( new QVBoxLayout );
	mainLayout->addLayout ( gLayout, 1 );
	mainLayout->addLayout ( buttonsLayout, 1 );
	setLayout ( mainLayout );
}

void emailConfigDialog::saveProfile ()
{
	if ( !optsList.current ()->bProfileModified )
		return;

	emailOpts* mailOpts ( optsList.current () );
	mailOpts->bNewProfile = false;
	mailOpts->bProfileModified = false;
	mailOpts->fancy_name = ui::cboProfiles->currentText ();
	mailOpts->app_name = ui::txtProgram->text ();
	mailOpts->addrs_switch = ui::txtAddress->text ();
	mailOpts->subject_switch = ui::txtSubject->text ();
	mailOpts->attach_switch = ui::txtAttach->text ();
	mailOpts->body_switch = ui::txtBody->text ();
	mailOpts->switches_sep = ui::txtSeparator->text ();

	stringRecord cmd_rec;
	cmd_rec.fastAppendValue ( mailOpts->fancy_name );
	cmd_rec.fastAppendValue ( mailOpts->app_name );
	cmd_rec.fastAppendValue ( mailOpts->addrs_switch );
	cmd_rec.fastAppendValue ( mailOpts->subject_switch );
	cmd_rec.fastAppendValue ( mailOpts->attach_switch );
	cmd_rec.fastAppendValue ( mailOpts->body_switch );
	cmd_rec.fastAppendValue ( mailOpts->switches_sep );

	CONFIG ()->writeConfig ( configFileName (),
							 PROFILE_NAME + QString::number ( ui::cboProfiles->currentIndex () ), cmd_rec.toString () );

	ui::cboProfiles->addItem ( mailOpts->fancy_name );
	if ( ui::chkDefault->isChecked () )
		CONFIG ()->setEmailClient ( mailOpts->fancy_name );
}

void emailConfigDialog::loadProfiles ()
{
	stringRecord strrec_email_opts;
	uint i ( 2 );
	optsList.append ( reinterpret_cast<emailOpts*> ( const_cast<THUNDERBIRD_OPTS*> ( &thunderbird ) ) );
	optsList.append ( reinterpret_cast<emailOpts*> ( const_cast<KMAIL_OPTS*> ( &kmail ) ) );
	do {
		strrec_email_opts.fromString ( CONFIG ()->readConfig ( configFileName (), PROFILE_NAME + QString::number ( i ) ) );
		if ( !strrec_email_opts.isNull () ) {
			emailOpts* mailOpts ( new emailOpts );
			mailOpts->bNewProfile = false;
			mailOpts->bProfileModified = false;
			mailOpts->fancy_name = strrec_email_opts.fieldValue ( 0 );
			mailOpts->app_name = strrec_email_opts.fieldValue ( 1 );
			mailOpts->addrs_switch = strrec_email_opts.fieldValue ( 2 );
			mailOpts->subject_switch = strrec_email_opts.fieldValue ( 3 );
			mailOpts->attach_switch = strrec_email_opts.fieldValue ( 4 );
			mailOpts->body_switch = strrec_email_opts.fieldValue ( 5 );
			mailOpts->switches_sep = strrec_email_opts.fieldValue ( 6 );
			optsList.append ( mailOpts );
			ui::cboProfiles->addItem ( mailOpts->app_name );
			strrec_email_opts.next ();
			i++;
		}
		else
			break;
	} while ( true );
	bProfilesLoaded = true;
	if ( CONFIG ()->emailClient ().isEmpty () )
		CONFIG ()->setEmailClient ( thunderbird.fancy_name );
}

void emailConfigDialog::changeForms ( const bool editAction )
{
	ui::txtProgram->setEditable ( editAction );
	ui::txtAddress->setEditable ( editAction );
	ui::txtSubject->setEditable ( editAction );
	ui::txtAttach->setEditable ( editAction );
	ui::txtBody->setEditable ( editAction );
	ui::txtSeparator->setEditable ( editAction );
	ui::cboProfiles->setEditable ( editAction );
	ui::chkDefault->setChecked ( !editAction );

	ui::btnChooseApp->setEnabled ( editAction );
	ui::btnAddProfile->setEnabled ( !editAction );
	ui::btnEditProfile->setEnabled ( !editAction );
	ui::btnCancelProfile->setEnabled ( editAction );
}

const QString emailConfigDialog::configFileName () const
{
	return CONFIG ()->defaultConfigDir () + QLatin1String ( "email_profiles.conf" );
}

void emailConfigDialog::done ( const int res )
{
	setResult ( res );
}

void emailConfigDialog::showDlg ()
{
	if ( ui::cboProfiles == nullptr ) {
		setWindowModality ( Qt::WindowModal );
		setupUi ();

		if ( !bProfilesLoaded )
			loadProfiles ();

		for ( uint i ( 0 ); i < optsList.count (); ++i )
			ui::cboProfiles->addItem ( optsList.at ( i )->fancy_name );
	}

	ui::cboProfiles->setCurrentIndex ( ui::cboProfiles->findText ( CONFIG ()->emailClient () ) );
	ui::chkDefault->setChecked ( true );
	show ();
}

void emailConfigDialog::sendEMail ( const QString& addresses, const QString& subject,
									const QString& attachments, const QString& bodyfile ) const
{
	if ( !bProfilesLoaded )
		const_cast<emailConfigDialog*> ( this )->loadProfiles ();

	const QString email_app ( CONFIG ()->emailClient () );
	uint i ( 0 );

	for ( ; i < optsList.count (); ++i ) {
		if ( optsList.at ( i )->fancy_name == email_app )
			break;
	}
	if ( i == optsList.count () )
		i = 0;

	const emailOpts* opts ( optsList.at ( i ) );
	const QString email_cmd ( opts->app_name + CHR_SPACE +
							  opts->addrs_switch + CHR_QUOTES + addresses + CHR_QUOTES + opts->switches_sep +
							  opts->subject_switch + CHR_QUOTES + subject + CHR_QUOTES + opts->switches_sep +
							  opts->attach_switch + CHR_QUOTES + attachments + CHR_QUOTES + opts->switches_sep +
							  opts->body_switch + CHR_QUOTES + bodyfile + CHR_QUOTES );
	( void )fileOps::sysExec ( email_cmd, emptyString );
}

void emailConfigDialog::cboProfiles_indexChanged ( const int idx )
{
	ui::cboProfiles->setEditText ( optsList.at ( idx )->fancy_name );
	ui::txtProgram->setText ( optsList.at ( idx )->app_name );
	ui::txtAddress->setText( optsList.at ( idx )->addrs_switch );
	ui::txtSubject->setText ( optsList.at ( idx )->subject_switch );
	ui::txtAttach->setText ( optsList.at ( idx )->attach_switch );
	ui::txtBody->setText ( optsList.at ( idx )->body_switch );
	ui::txtSeparator->setText ( optsList.at ( idx )->switches_sep );
	ui::chkDefault->setChecked ( optsList.at ( idx )->fancy_name == CONFIG ()->emailClient () );
	optsList.setCurrent ( idx );

	ui::btnEditProfile->setEnabled ( idx >= 2 );
}

void emailConfigDialog::btnChooseApp_clicked ()
{
	const QString filename ( fileOps::getOpenFileName ( QLatin1String ( "/usr/bin/" ), QLatin1String ( "*.*" ) ) );
	if ( !filename.isEmpty () ) {
		ui::txtProgram->setText ( filename );
		ui::txtProgram->setFocus ();
	}
}

void emailConfigDialog::btnAddProfile_clicked ()
{
	mSavedIndex = optsList.currentIndex ();
	ui::txtProgram->clear ();
	ui::txtAddress->clear ();
	ui::txtSubject->clear ();
	ui::txtAttach->clear ();
	ui::txtBody->clear ();
	ui::txtSeparator->clear ();
	ui::txtProgram->setFocus ();
	ui::cboProfiles->setEditText ( tr ( "New profile ..." ) );
	changeForms ( true );
	optsList.append ( new emailOpts );
	optsList.last ()->bNewProfile = true;
}

void emailConfigDialog::btnEditProfile_clicked ()
{
	changeForms ( true );
}

void emailConfigDialog::btnSaveProfile_clicked ()
{
	saveProfile ();
	changeForms ( false );
}

void emailConfigDialog::btnCancelProfile_clicked ()
{
	changeForms ( false );
	if ( !optsList.current ()->bNewProfile ) {
		optsList.current ()->bProfileModified = false;
		optsList.current ()->bNewProfile = false;
	}
	else
		optsList.remove ( optsList.currentIndex (), true );
	ui::cboProfiles->setCurrentIndex ( mSavedIndex );
}

void emailConfigDialog::btnClose_clicked ()
{
	if ( !optsList.current ()->bProfileModified && !optsList.current ()->bNewProfile )
		close ();
}
