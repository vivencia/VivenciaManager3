#include "newprojectdialog.h"
#include "vmwidgets.h"
#include "vmlistitem.h"
#include "global.h"
#include "data.h"
#include "mainwindow.h"
#include "configops.h"

#include <QPushButton>
#include <QVBoxLayout>

newProjectDialog::newProjectDialog ( QWidget *parent )
	: QDialog ( parent, Qt::Tool ), mJobItem ( nullptr ), bresult ( false )
{
	QLabel* lblClient ( new QLabel ( TR_FUNC ( "Client:" ) ) );
	cboClients = new vmComboBox;
	cboClients->setCallbackForIndexChanged ( [&] ( const int idx ) { return loadJobsList ( Client::clientID ( cboClients->itemText ( idx ) ) ); } );
	
	QLabel* lblJobType ( new QLabel ( TR_FUNC ( "Select target job ..." ) ) );
	lstJobTypes = new vmListWidget;
	lstJobTypes->setCallbackForCurrentItemChanged ( [&] ( vmListItem* current ) {
		return jobTypeItemSelected ( current ); } );

	QLabel* lblProjectName ( new QLabel ( TR_FUNC ( "Project Name:" ) ) );
	txtProjectName = new vmLineEdit;
	txtProjectName->setCallbackForContentsAltered ( [&] ( const vmWidget* const widget ) {
		return txtProjectNameAltered ( widget ); } );

	btnChooseExistingDir = new QToolButton;
	btnChooseExistingDir->setIcon( ICON ( "folder-brown.png" ) );
	connect ( btnChooseExistingDir, &QToolButton::clicked, this, [&] () { return btnChooseExistingDir_clicked (); } );

	chkUseDefaultName = new vmCheckBox ( TR_FUNC ( "Use automatically generated name" ) );
	chkUseDefaultName->setChecked ( true );
	chkUseDefaultName->setCallbackForContentsAltered ( [&] ( const vmWidget* const ) { return chkUseDefaultName_checked (); } );
	chkUseDefaultName->setEditable ( true );

	btnOK = new QPushButton ( TR_FUNC ( "Accept" ) );
	connect ( btnOK, &QPushButton::clicked, this, [&] () { return btnOK_clicked (); } );
	btnCancel = new QPushButton ( TR_FUNC ( "Cancel" ) );
	connect ( btnCancel, &QPushButton::clicked, this, [&] () { return btnCancel_clicked (); } );

	QVBoxLayout* vbLayout ( new QVBoxLayout );
	vbLayout->setMargin ( 2 );
	vbLayout->setSpacing ( 2 );
	vbLayout->addWidget ( lblClient );
	vbLayout->addWidget ( cboClients, 2 );
	vbLayout->addWidget( lblJobType );
	vbLayout->addWidget( lstJobTypes, 2 );

	QGridLayout* gLayout ( new QGridLayout );
	gLayout->setMargin ( 2 );
	gLayout->setSpacing ( 2 );
	gLayout->setColumnStretch ( 0, 3 );
	gLayout->addWidget ( lblProjectName, 0, 0, 1, 3 );
	gLayout->addWidget ( txtProjectName, 1, 0, 1, 3 );
	gLayout->addWidget ( btnChooseExistingDir, 1, 4, 1, 1 );
	gLayout->addWidget ( chkUseDefaultName, 2, 0, 2, 4 );
	gLayout->addItem ( new QSpacerItem ( 0, 0 ), 3, 0, 5, 4 );

	QHBoxLayout* btnsLayout ( new QHBoxLayout );
	btnsLayout->addWidget ( btnOK );
	btnsLayout->addWidget ( btnCancel );
	gLayout->addLayout ( btnsLayout, 6, 0, 6, 5 );

	QHBoxLayout* mainLayout ( new QHBoxLayout );
	mainLayout->setMargin ( 0 );
	mainLayout->setSpacing ( 2 );
	mainLayout->addLayout ( vbLayout, 1 );
	mainLayout->addLayout ( gLayout, 1 );
	setLayout ( mainLayout );
}

newProjectDialog::~newProjectDialog () {}

void newProjectDialog::showDialog ( const QString& clientname, const bool b_allow_other_client )
{
	if ( cboClients->count () == 0 )
	{
		QStringList client_names;
		Data::fillClientsNamesList ( client_names );
		cboClients->addItems ( client_names );
	}
	if ( !clientname.isEmpty () ) {
		cboClients->setCurrentIndex ( cboClients->findText ( clientname ) );
		cboClients->setCurrentText ( clientname );
		loadJobsList ( Client::clientID ( clientname ) );
	}
	cboClients->setEditable ( b_allow_other_client );
	cboClients->setIgnoreChanges ( !b_allow_other_client );
	exec ();
}

void newProjectDialog::loadJobsList ( const int clientid )
{
	lstJobTypes->setIgnoreChanges ( true );
	lstJobTypes->clear ();
	txtProjectName->clear ();
	mProjectID.clear ();
	mProjectPath.clear ();

	QStringList jobTypesList;
	Data::fillJobTypeList ( jobTypesList, QString::number ( clientid ) );

	QString jobid;
	mClientItem = globalMainWindow->getClientItem ( clientid );
	jobListItem* job_parent ( nullptr );
	QStringList::const_iterator itr ( jobTypesList.constBegin () );
	const QStringList::const_iterator itr_end ( jobTypesList.constEnd () );
	for ( ; itr != itr_end; ++itr ) {
		jobid = (*itr).mid ( 1, (*itr).indexOf ( CHR_R_PARENTHESIS, 2 ) - 1 );
		job_parent = globalMainWindow->getJobItem ( mClientItem, jobid.toInt () );
		if ( job_parent ) {
			mJobItem = static_cast<jobListItem*>( job_parent->relatedItem ( RLI_EXTRAITEM ) );
			if ( mJobItem == nullptr ) {
				mJobItem = new jobListItem;
				mJobItem->setRelation ( RLI_EXTRAITEM );
				job_parent->syncSiblingWithThis ( mJobItem );
				mJobItem->setText ( *itr, false, false, false );
			}
			mJobItem->addToList ( lstJobTypes );
		}
	}
	lstJobTypes->setIgnoreChanges ( false );
	lstJobTypes->setCurrentItem ( mJobItem, true );
}

void newProjectDialog::jobTypeItemSelected ( vmListItem* item )
{
	if ( item != nullptr ) {
		mJobItem = static_cast<jobListItem*> ( item );
		if ( mJobItem->loadData () ) {
			if ( chkUseDefaultName->isChecked () ) {
				txtProjectName->setText ( mJobItem->jobRecord ()->date ( FLD_JOB_STARTDATE ).toDate ( vmNumber::VDF_FILE_DATE ) +
							QLatin1String ( " - " ) + recStrValue ( mJobItem->jobRecord (), FLD_JOB_TYPE ), true );
			}
			else
				txtProjectName->setText ( item->data ( Qt::UserRole ).toString (), true );
		}
	}
}

void newProjectDialog::txtProjectNameAltered ( const vmWidget* const )
{
	if ( !txtProjectName->text ().contains ( CHR_F_SLASH ) ) {
		mProjectID = mJobItem->jobRecord ()->date ( FLD_JOB_STARTDATE ).toDate ( vmNumber::VDF_FILE_DATE );
		mProjectPath = CONFIG ()->getProjectBasePath ( recStrValue ( mClientItem->clientRecord (), FLD_CLIENT_NAME ) ) + txtProjectName->text () + CHR_F_SLASH;
	}
	else {
		mProjectID = fileOps::nthDirFromPath ( txtProjectName->text () );
		mProjectID = mProjectID.left ( mProjectID.indexOf ( CHR_SPACE ) );
		mProjectPath = txtProjectName->text ();
		if ( mProjectPath.at ( mProjectPath.count () - 1 ) != CHR_F_SLASH )
			mProjectPath += CHR_F_SLASH;
	}
	lstJobTypes->currentItem ()->setData ( Qt::UserRole, txtProjectName->text () );
}

void newProjectDialog::btnChooseExistingDir_clicked ()
{
	QString newProjectDir ( fileOps::getExistingDir ( CONFIG ()->getProjectBasePath (
								recStrValue ( static_cast<clientListItem*> ( mJobItem->relatedItem ( RLI_CLIENTPARENT ) )->clientRecord (), FLD_CLIENT_NAME ) ) ) );
	if ( !newProjectDir.isEmpty () ) {
		newProjectDir += CHR_F_SLASH;
		txtProjectName->setText ( newProjectDir, true );
	}
}

void newProjectDialog::chkUseDefaultName_checked ()
{
	jobTypeItemSelected ( lstJobTypes->currentItem () );
	txtProjectName->setEditable ( !chkUseDefaultName->isChecked () );
	//qDebug () << !chkUseDefaultName->isChecked ();
}

void newProjectDialog::btnOK_clicked ()
{
	if ( VM_NOTIFY ()->questionBox ( TR_FUNC ( "Confirmation" ), TR_FUNC ( "Proceed with creating a project for the following job: " ) +
								mJobItem->jobRecord ()->jobSummary () + CHR_QUESTION_MARK ) )
	{
		bresult = true;
		close ();
	}
}

void newProjectDialog::btnCancel_clicked ()
{
	bresult = false;
	close ();
}

void newProjectDialog::closeCleanUp ()
{

}
