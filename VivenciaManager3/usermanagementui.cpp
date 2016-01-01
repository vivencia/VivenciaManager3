#include "usermanagementui.h"
#include "vmwidgets.h"
#include "vmtablewidget.h"
#include "vivenciadb.h"
#include "usermanagement.h"
#include "heapmanager.h"

#include <QLabel>
#include <QInputDialog>
#include <QToolButton>
#include <QGridLayout>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QGroupBox>
#include <QPushButton>
#include <QFont>
#include <QApplication>

static const QString tableColHeaders[3] = {
	QApplication::tr ( "Table" ),
	QApplication::tr ( "Can read?" ),
	QApplication::tr ( "Can write?" )
};

//const uint columnWidth[3] = { 120, 50, 50 };

userManagementUI::userManagementUI ( QWidget* parent )
	: QDialog ( parent ), mUserInfo_st ( nullptr )
{
	btnInsertUser = new QToolButton;
	btnInsertUser->setCheckable ( true );
	btnInsertUser->setIcon ( ICON ( "browse-controls/add.png" ) );
	connect ( btnInsertUser, static_cast<void (QToolButton::*)(const bool)>( &QToolButton::clicked ), this, [&] ( const bool checked ) { return btnInsertUser_clicked ( checked ); } );

	btnEditUser = new QToolButton;
	btnEditUser->setCheckable ( true );
	btnEditUser->setIcon ( ICON ( "browse-controls/edit.png" ) );
	btnEditUser->setEnabled ( false );
	connect ( btnEditUser, static_cast<void (QToolButton::*)(const bool)>( &QToolButton::clicked ), this, [&] ( const bool checked ) { return btnEditUser_clicked ( checked ); } );

	btnRemoveUser = new QToolButton;
	btnRemoveUser->setIcon ( ICON ( "remove.png" ) );
	btnRemoveUser->setEnabled ( false );
	connect ( btnRemoveUser, &QToolButton::clicked, this, [&] () { return btnRemoveUser_clicked (); } );

	QHBoxLayout* layoutButtons ( new QHBoxLayout );
	layoutButtons->setSpacing ( 1 );
	layoutButtons->setMargin ( 0 );
	layoutButtons->addWidget ( btnInsertUser );
	layoutButtons->addWidget ( btnEditUser );
	layoutButtons->addWidget ( btnRemoveUser );

	txtEditUserName = new vmLineEdit;
	txtEditUserName->setCallbackForContentsAltered ( [&] ( const vmWidget* const ) { return reloadUserInfo (); } );

	txtEditPassword = new vmLineEdit;
	txtEditPassword->setEchoMode ( QLineEdit::Password );
	txtEditPassword->setEditable ( false );

	chkCanExport = new vmCheckBox ( tr ( "Can export" ) ) ;
	chkCanExport->setEditable ( false );
	chkCanImport = new vmCheckBox ( tr ( "Can import" ) ) ;
	chkCanImport->setEditable ( false );

	tablesView = new vmTableWidget; // General table and users table are only editable/viewable by admin
	tablesView->horizontalHeader ()->setStretchLastSection ( true );

	/*uint i_row ( 0 );
	vmTableItem* sheetItem ( nullptr );
	for ( uint i_col ( 0 ); i_col < 3; ++i_col ) {
		tablesView->insertColumn ( i_col );
		tablesView->setHorizontalHeaderItem ( i_col, new vmTableItem (
					WT_TABLE_ITEM, vmLineEdit::TT_TEXT, tableColHeaders[i_col], tablesView ) );
		tablesView->setColumnWidth ( i_col, columnWidth[i_col] );
		//tablesView->setColumnReadOnly ( i_col, ( i_col == 0 ) );

		for ( i_row = 0; i_row < ( TABLES_IN_DB - 2 ); ++i_row ) {
			if ( i_col == 0 )
				tablesView->insertRow ( i_row );

			sheetItem = new vmTableItem ( WT_LINEEDIT, vmLineEdit::TT_TEXT, emptyString, tablesView );
			tablesView->setItem ( i_row + 1, i_col, sheetItem );
			sheetItem->setEditType ( i_col == 0 ? READ_ONLY : EDITABLE );
			if ( i_col == 0 )
				sheetItem->setText ( VivenciaDB::tableName ( static_cast<TABLE_ORDER> ( i_row + 2 ) ) );
			else {
				//tablesView->setCellWidget ( i_row, i_col, WT_CHECKBOX );
				sheetItem->setText ( CHR_ONE );
			}
		}
	}*/
	tablesView->setEditable ( false );

	btnClose = new QPushButton ( ICON ( "cancel.png" ), tr ( "Close" ) );
	connect ( btnClose, &QPushButton::clicked, this, [&] () { return btnClose_clicked (); } );

	lblStatusBar = new QLabel;
	lblStatusBar->setFrameStyle ( QFrame::StyledPanel | QFrame::Sunken );
	lblStatusBar->setMinimumHeight ( 30 );
	QFont fntStatus ( lblStatusBar->font () );
	fntStatus.setBold ( true );
	lblStatusBar->setFont ( fntStatus );

	QGridLayout* mainLayout ( new QGridLayout );
	mainLayout->setColumnStretch ( 0, 1 );
	mainLayout->setColumnStretch ( 2, 2 );
	mainLayout->setColumnStretch ( 5, 1 );
	mainLayout->setColumnStretch ( 6, 1 );
	mainLayout->setColumnStretch ( 7, 1 );
	mainLayout->addWidget ( new QLabel ( tr ( "Name: " ) ), 0, 0, 1, 1 );
	mainLayout->addWidget ( txtEditUserName, 0, 2, 1, 2 );
	mainLayout->addLayout ( layoutButtons, 0, 4, 1, 2 );
	mainLayout->addWidget ( new QLabel ( tr ( "Password: " ) ), 1, 0, 1, 1 );
	mainLayout->addWidget ( txtEditPassword, 1, 2, 1, 2 );
	mainLayout->addWidget ( chkCanExport, 2, 0, 1, 2 );
	mainLayout->addWidget ( chkCanImport, 2, 3, 1, 5 );
	mainLayout->addWidget ( tablesView, 4, 0, 7, 7 );
	mainLayout->addWidget ( lblStatusBar, 12, 0, 1, 5, Qt::AlignLeft );
	mainLayout->addWidget ( btnClose, 12, 6, 1, 1, Qt::AlignRight );

	mUserInfo_st = new UserManagement::USER_INFO;

	setWindowTitle ( tr ( "User Management" ) );
	setLayout ( mainLayout );
	adjustSize ();
}

userManagementUI::~userManagementUI ()
{
	heap_del ( mUserInfo_st );
}

void userManagementUI::showWindow ( const QString& username, const bool modal )
{
	if ( username.isEmpty () )
		txtEditUserName->setFocus ();
	else
		txtEditUserName->setText ( username, true );
	if ( modal )
		exec ();
	else
		showNormal ();
}

void userManagementUI::reloadUserInfo ()
{
	const bool userExists ( USRMNGR ()->loadUserInfo ( txtEditUserName->text (), mUserInfo_st ) );
	btnEditUser->setEnabled ( userExists );
	btnRemoveUser->setEnabled ( userExists );
	btnInsertUser->setEnabled ( !( btnInsertUser->isChecked () && userExists ) );

	if ( userExists ) {
		chkCanExport->setChecked ( USRMNGR ()->canExport ( mUserInfo_st ) );
		chkCanImport->setChecked ( USRMNGR ()->canImport ( mUserInfo_st ) );

		bool ok ( true );
		for ( uint i_row ( 1 ); i_row < ( TABLES_IN_DB - 2 ) ; ++i_row ) {
			ok = USRMNGR ()->canRead ( i_row + 2, mUserInfo_st );
			tablesView->sheetItem ( i_row, 1 )->setText ( ok ? CHR_ONE : CHR_ZERO, false, false );
			ok = USRMNGR ()->canWrite ( i_row + 2, mUserInfo_st );
			tablesView->sheetItem ( i_row, 2 )->setText ( ok ? CHR_ONE : CHR_ZERO, false, false );
		}
	}
	else {
		chkCanImport->setChecked ( false );
		chkCanExport->setChecked ( false );
		txtEditPassword->clear ();
		tablesView->clear ();
	}
}

void userManagementUI::saveUserInfo ()
{
	mUserInfo_st->name = txtEditUserName->text ();
	mUserInfo_st->passwd = txtEditPassword->text ();
	mUserInfo_st->privileges.can_export = chkCanExport->isChecked ();
	mUserInfo_st->privileges.can_import = chkCanImport->isChecked ();

	for ( uint i_row ( 1 ); i_row < unsigned ( tablesView->rowCount () ); ++i_row ) {
		USRMNGR ()->setCanRead ( ( tablesView->sheetItem ( i_row, 1 )->text () == CHR_ONE ), i_row + 1, mUserInfo_st );
		USRMNGR ()->setCanWrite ( ( tablesView->sheetItem ( i_row, 2 )->text () == CHR_ONE ), i_row + 1, mUserInfo_st );
	}
}

void userManagementUI::btnInsertUser_clicked ( const bool checked )
{
	txtEditUserName->setEditable ( checked );
	txtEditPassword->setEditable ( checked );
	chkCanExport->setEditable ( checked );
	chkCanImport->setEditable ( checked );
	tablesView->setEditable ( checked );
	btnEditUser->setEnabled ( !checked );
	btnRemoveUser->setEnabled ( !checked );
	btnInsertUser->setIcon ( checked ? ICON ( "document-save.png" ) : ICON ( "browse-controls/add.png" ) );
	if ( checked ) {
		txtEditUserName->clear ();
		txtEditUserName->setFocus ();
		txtEditPassword->clear ();
		chkCanExport->setChecked ( false );
		chkCanImport->setChecked ( false );
		tablesView->clear ();
	}
	else {
		saveUserInfo ();
		USRMNGR ()->insertUser ( mUserInfo_st );
		lblStatusBar->setText ( USRMNGR ()->returnCodeString () );
	}
}

void userManagementUI::btnEditUser_clicked ( const bool checked )
{
	txtEditUserName->setEditable ( !checked );
	txtEditPassword->setEditable ( checked );
	chkCanExport->setEditable ( checked );
	chkCanImport->setEditable ( checked );
	tablesView->setEditable ( checked );
	btnInsertUser->setEnabled ( !checked );
	btnRemoveUser->setEnabled ( !checked );
	btnEditUser->setIcon ( checked ? ICON ( "document-save.png" ) : ICON ( "browse-controls/edit.png" ) );
	if ( checked )
		txtEditPassword->setFocus ();
	else {
		saveUserInfo ();
		USRMNGR ()->editUser ( mUserInfo_st );
		lblStatusBar->setText ( USRMNGR ()->returnCodeString () );
	}
}

void userManagementUI::btnRemoveUser_clicked ()
{
	USRMNGR ()->removeUser ( mUserInfo_st );
	lblStatusBar->setText ( USRMNGR ()->returnCodeString () );
}

void userManagementUI::btnClose_clicked ()
{
	if ( isModal () )
		done ( QDialog::Accepted );
	else
		close ();
}
