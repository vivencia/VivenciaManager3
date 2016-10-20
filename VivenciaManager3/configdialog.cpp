#include "configdialog.h"
#include "ui_configdialog.h"
#include "global.h"
#include "configops.h"
#include "fileops.h"
#include "vmwidgets.h"
#include "emailconfigdialog.h"

configDialog::configDialog ( QWidget* parent )
	: QDialog ( parent ), ui ( new Ui::configDialog )
{
	setupUi ();
	setWindowTitle ( PROGRAM_NAME + windowTitle () );
	fillForms ();
}

configDialog::~configDialog ()
{
	delete ui;
}

void configDialog::setupUi ()
{
	ui->setupUi ( this );
	ui->txtCfgConfigFile->setCallbackForContentsAltered ( [&] (const vmWidget* txtWidget ) {
			return CONFIG ()->setAppConfigFile ( txtWidget->text () ); } );
	ui->txtCfgDataFolder->setCallbackForContentsAltered ( [&] (const vmWidget* txtWidget ) {
			return CONFIG ()->setAppDataDir ( txtWidget->text () ); } );
	ui->txtCfgFileManager->setCallbackForContentsAltered ( [&] (const vmWidget* txtWidget ) {
			return CONFIG ()->setFileManager ( txtWidget->text () ); } );
	ui->txtCfgPictureViewer->setCallbackForContentsAltered ( [&] (const vmWidget* txtWidget ) {
			return CONFIG ()->setPictureViewer ( txtWidget->text () ); } );
	ui->txtCfgPictureEditor->setCallbackForContentsAltered ( [&] (const vmWidget* txtWidget ) {
			return CONFIG ()->setPictureEditor ( txtWidget->text () ); } );
	ui->txtCfgDocumentViewer->setCallbackForContentsAltered ( [&] (const vmWidget* txtWidget ) {
			return CONFIG ()->setUniversalViewer ( txtWidget->text () ); } );
	ui->txtCfgDocEditor->setCallbackForContentsAltered ( [&] (const vmWidget* txtWidget ) {
			return CONFIG ()->setDocEditor ( txtWidget->text () ); } );
	ui->txtCfgXlsEditor->setCallbackForContentsAltered ( [&] (const vmWidget* txtWidget ) {
			return CONFIG ()->setXlsEditor ( txtWidget->text () ); } );
	ui->txtCfgJobsPrefix->setCallbackForContentsAltered ( [&] (const vmWidget* txtWidget ) {
			return CONFIG ()->setProjectsBaseDir ( txtWidget->text () ); } );
	ui->txtCfgEstimate->setCallbackForContentsAltered ( [&] (const vmWidget* txtWidget ) {
			return CONFIG ()->setEstimatesDir ( txtWidget->text () ); } );
	ui->txtCfgReports->setCallbackForContentsAltered ( [&] (const vmWidget* txtWidget ) {
			return CONFIG ()->setReportsDir ( txtWidget->text () ); } );
	ui->txtCfgHtmlDir->setCallbackForContentsAltered ( [&] (const vmWidget* txtWidget ) {
			return CONFIG ()->setHTMLDir ( txtWidget->text () ); } );
	ui->txtCfgDropbox->setCallbackForContentsAltered ( [&] (const vmWidget* txtWidget ) {
			return CONFIG ()->setDropboxDir ( txtWidget->text () ); } );
	
	ui->txtCfgConfigFile->setEditable ( true );
	ui->txtCfgDataFolder->setEditable ( true );
	ui->txtCfgFileManager->setEditable ( true );
	ui->txtCfgPictureViewer->setEditable ( true );
	ui->txtCfgPictureEditor->setEditable ( true );
	ui->txtCfgDocumentViewer->setEditable ( true );
	ui->txtCfgDocEditor->setEditable ( true );
	ui->txtCfgXlsEditor->setEditable ( true );
	ui->txtCfgJobsPrefix->setEditable ( true );
	ui->txtCfgEstimate->setEditable ( true );
	ui->txtCfgReports->setEditable ( true );
	ui->txtCfgHtmlDir->setEditable ( true );
	ui->txtCfgDropbox->setEditable ( true );
	
	ui->btnClose->connect ( ui->btnClose, &QPushButton::clicked, this, [&] ( const bool ) { return close (); } );
}

void configDialog::fillForms ()
{
	ui->txtCfgConfigFile->setText ( CONFIG ()->appConfigFile () );
	ui->txtCfgDataFolder->setText ( CONFIG ()->appDataDir () );
	ui->txtCfgFileManager->setText ( CONFIG ()->fileManager () );
	ui->txtCfgPictureViewer->setText ( CONFIG ()->pictureViewer () );
	ui->txtCfgPictureEditor->setText ( CONFIG ()->pictureEditor () );
	ui->txtCfgDocumentViewer->setText ( CONFIG ()->universalViewer () );
	ui->txtCfgDocEditor->setText ( CONFIG ()->docEditor () );
	ui->txtCfgXlsEditor->setText ( CONFIG ()->xlsEditor () );
	ui->txtCfgJobsPrefix->setText ( CONFIG ()->projectsBaseDir () );
	ui->txtCfgEmailClient->setText ( CONFIG ()->emailClient () );
	ui->txtCfgEstimate->setText ( CONFIG ()->estimatesDirSuffix () );
	ui->txtCfgReports->setText ( CONFIG ()->reportsDirSuffix () );
	ui->txtCfgHtmlDir->setText ( CONFIG ()->HTMLDir () );
	ui->txtCfgDropbox->setText ( CONFIG ()->dropboxDir() );
}

void configDialog::on_btnCfgChooseConfigFile_clicked ()
{
	const QString filename = fileOps::getSaveFileName ( CONFIG ()->defaultConfigDir (), "*.*" );
	if ( !filename.isEmpty () ) {
		ui->txtCfgConfigFile->setText ( filename );
		ui->txtCfgConfigFile->setFocus (); // use setFocus () for force a editingFinished () signal when the user does clicks away from the form
	}
}

void configDialog::on_btnCfgUseDefaultConfigFile_clicked ()
{
	ui->txtCfgConfigFile->setText ( CONFIG ()->appConfigFile ( true ) );
	ui->txtCfgConfigFile->setFocus ();
}

void configDialog::on_btnCfgChooseDataFolder_clicked ()
{
	const QString dir ( fileOps::getExistingDir ( CONFIG ()->appDataDir ( true ) ) );
	if ( !dir.isEmpty () ) {
		ui->txtCfgDataFolder->setText ( dir );
		ui->txtCfgDataFolder->setFocus ();
	}
}

void configDialog::on_btnCfgUseDefaultDataFolder_clicked ()
{
	ui->txtCfgDataFolder->setText ( CONFIG ()->appDataDir ( true ) );
	ui->txtCfgDataFolder->setFocus ();
}

void configDialog::on_btnCfgChooseFileManager_clicked ()
{
	const QString filename ( fileOps::getOpenFileName ( QStringLiteral ( "/usr/bin/" ), QStringLiteral ( "*.*" ) ) );
	if ( !filename.isEmpty () )
	{
		ui->txtCfgFileManager->setText ( filename );
		ui->txtCfgFileManager->setFocus ();
	}
}

void configDialog::on_btnCfgUseDefaultFileManager_clicked ()
{
	ui->txtCfgFileManager->setText ( CONFIG ()->fileManager ( true ) );
	ui->txtCfgFileManager->setFocus ();
}

void configDialog::on_btnCfgChoosePictureViewer_clicked ()
{
	const QString filename ( fileOps::getOpenFileName ( QStringLiteral ( "/usr/bin/" ), QStringLiteral ( "*.*" ) ) );
	if ( !filename.isEmpty () )
	{
		ui->txtCfgPictureViewer->setText ( filename );
		ui->txtCfgPictureViewer->setFocus ();
	}
}

void configDialog::on_btnCfgUseDefaultPictureViewer_clicked ()
{
	ui->txtCfgPictureViewer->setText ( CONFIG ()->pictureViewer ( true ) );
	ui->txtCfgPictureViewer->setFocus ();
}

void configDialog::on_btnCfgChoosePictureEditor_clicked ()
{
	const QString filename ( fileOps::getOpenFileName ( QStringLiteral ( "/usr/bin/" ), QStringLiteral ( "*.*" ) ) );
	if ( !filename.isEmpty () )
	{
		ui->txtCfgPictureEditor->setText ( filename );
		ui->txtCfgPictureEditor->setFocus ();
	}
}

void configDialog::on_btnCfgUseDefaultPictureEditor_clicked ()
{
	ui->txtCfgPictureEditor->setText ( CONFIG ()->pictureEditor ( true ) );
	ui->txtCfgPictureEditor->setFocus ();
}

void configDialog::on_btnCfgChooseEMailClient_clicked ()
{
	EMAIL_CONFIG ()->showDlg ();
	ui->txtCfgEmailClient->setText ( CONFIG ()->emailClient () ); // EMAIL_CONFIG will set the client
}

void configDialog::on_btnCfgChooseDocViewer_clicked ()
{
	const QString filename ( fileOps::getOpenFileName ( QStringLiteral ( "/usr/bin/" ), QStringLiteral ( "*.*" ) ) );
	if ( !filename.isEmpty () )
	{
		ui->txtCfgDocumentViewer->setText ( filename );
		ui->txtCfgDocumentViewer->setFocus ();
	}
}

void configDialog::on_btnCfgUseDefaultDocumentViewer_clicked ()
{
	ui->txtCfgDocumentViewer->setText ( CONFIG ()->universalViewer ( true ) );
	ui->txtCfgDocumentViewer->setFocus ();
}

void configDialog::on_btnCfgChooseDocEditor_clicked ()
{
	const QString filename ( fileOps::getOpenFileName ( QStringLiteral ( "/usr/bin/" ), QStringLiteral ( "*.*" ) ) );
	if ( !filename.isEmpty () )
	{
		ui->txtCfgDocEditor->setText ( filename );
		ui->txtCfgDocEditor->setFocus ();
	}
}

void configDialog::on_btnCfgUseDefaultDocEditor_clicked ()
{
	ui->txtCfgDocEditor->setText ( CONFIG ()->docEditor ( true ) );
	ui->txtCfgDocEditor->setFocus ();
}

void configDialog::on_btnCfgChooseXlsEditor_clicked ()
{
	const QString filename ( fileOps::getOpenFileName ( QStringLiteral ( "/usr/bin/" ), QStringLiteral ( "*.*" ) ) );
	if ( !filename.isEmpty () )
	{
		ui->txtCfgXlsEditor->setText ( filename );
		ui->txtCfgXlsEditor->setFocus ();
	}
}

void configDialog::on_btnCfgUseDefaultXlsEditor_clicked ()
{
	ui->txtCfgXlsEditor->setText ( CONFIG ()->docEditor ( true ) );
	ui->txtCfgXlsEditor->setFocus ();
}

void configDialog::on_btnCfgChooseBaseDir_clicked ()
{
	const QString dir = fileOps::getExistingDir ( ui->txtCfgJobsPrefix->text () );
	if ( !dir.isEmpty () ) {
		ui->txtCfgJobsPrefix->setText ( dir );
		ui->txtCfgJobsPrefix->setFocus ();
	}
}

void configDialog::on_btnCfgUseDefaultBaseDir_clicked ()
{
	ui->txtCfgJobsPrefix->setText ( CONFIG ()->projectsBaseDir ( true ) );
	ui->txtCfgJobsPrefix->setFocus ();
}

void configDialog::on_btnCfgUseDefaultESTIMATEDir_clicked ()
{
	ui->txtCfgEstimate->setText ( CONFIG ()->estimatesDirSuffix () );
	ui->txtCfgEstimate->setFocus ();
}

void configDialog::on_btnCfgUseDefaultReportsDir_clicked ()
{
	ui->txtCfgReports->setText ( CONFIG ()->reportsDirSuffix () );
	ui->txtCfgReports->setFocus ();
}
