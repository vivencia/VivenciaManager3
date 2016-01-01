#include "configdialog.h"
#include "ui_configdialog.h"
#include "global.h"
#include "configops.h"
#include "fileops.h"
#include "emailconfigdialog.h"

configDialog::configDialog ( QWidget* parent )
	: QDialog ( parent ), ui ( new Ui::configDialog )
{
	ui->setupUi ( this );
	setWindowTitle ( PROGRAM_NAME + windowTitle () );
	fillForms ();
}

configDialog::~configDialog ()
{
	delete ui;
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
	ui->txtCfgESTIMATE->setText ( CONFIG ()->estimatesDirSuffix () );
	ui->txtCfgReports->setText ( CONFIG ()->reportsDirSuffix () );
}

void configDialog::on_txtCfgConfigFile_editingFinished ()
{
	CONFIG ()->setAppConfigFile ( ui->txtCfgConfigFile->text () );
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

void configDialog::on_txtCfgDataFolder_editingFinished ()
{
	CONFIG ()->setAppDataDir ( ui->txtCfgDataFolder->text () );
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

void configDialog::on_txtCfgFileManager_editingFinished ()
{
	CONFIG ()->setFileManager ( ui->txtCfgFileManager->text () );
}

void configDialog::on_btnCfgChooseFileManager_clicked ()
{
	const QString filename = fileOps::getOpenFileName ( "/usr/bin/", "*.*" );
	if ( !filename.isEmpty () ) {
		ui->txtCfgFileManager->setText ( filename );
		ui->txtCfgFileManager->setFocus ();
	}
}

void configDialog::on_btnCfgUseDefaultFileManager_clicked ()
{
	ui->txtCfgFileManager->setText ( CONFIG ()->fileManager ( true ) );
	ui->txtCfgFileManager->setFocus ();
}

void configDialog::on_txtCfgPictureViewer_editingFinished ()
{
	CONFIG ()->setPictureViewer ( ui->txtCfgPictureViewer->text () );
}

void configDialog::on_btnCfgChoosePictureViewer_clicked ()
{
	const QString filename = fileOps::getOpenFileName ( "/usr/bin/", "*.*" );
	if ( !filename.isEmpty () ) {
		ui->txtCfgPictureViewer->setText ( filename );
		ui->txtCfgPictureViewer->setFocus ();
	}
}

void configDialog::on_btnCfgUseDefaultPictureViewer_clicked ()
{
	ui->txtCfgPictureViewer->setText ( CONFIG ()->pictureViewer ( true ) );
	ui->txtCfgPictureViewer->setFocus ();
}

void configDialog::on_txtCfgPictureEditor_editingFinished ()
{
	const QString corrected_filename ( CONFIG ()->setPictureEditor ( ui->txtCfgPictureEditor->text () ) );
	if ( corrected_filename != ui->txtCfgPictureEditor->text () )
		ui->txtCfgPictureEditor->setText ( corrected_filename );
}

void configDialog::on_btnCfgChoosePictureEditor_clicked ()
{
	const QString filename = fileOps::getOpenFileName ( "/usr/bin/", "*.*" );
	if ( !filename.isEmpty () ) {
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

void configDialog::on_txtCfgDocumentViewer_editingFinished ()
{
	CONFIG ()->setUniversalViewer ( ui->txtCfgDocumentViewer->text () );
}

void configDialog::on_btnCfgChooseDocViewer_clicked ()
{
	const QString filename = fileOps::getOpenFileName ( "/usr/bin/", "*.*" );
	if ( !filename.isEmpty () ) {
		ui->txtCfgDocumentViewer->setText ( filename );
		ui->txtCfgDocumentViewer->setFocus ();
	}
}

void configDialog::on_btnCfgUseDefaultDocumentViewer_clicked ()
{
	ui->txtCfgDocumentViewer->setText ( CONFIG ()->universalViewer ( true ) );
	ui->txtCfgDocumentViewer->setFocus ();
}

void configDialog::on_txtCfgDocEditor_editingFinished ()
{
	CONFIG ()->setDocEditor ( ui->txtCfgDocEditor->text () );
}

void configDialog::on_btnCfgChooseDocEditor_clicked ()
{
	const QString filename = fileOps::getOpenFileName ( "/usr/bin/", "*.*" );
	if ( !filename.isEmpty () ) {
		ui->txtCfgDocEditor->setText ( filename );
		ui->txtCfgDocEditor->setFocus ();
	}
}

void configDialog::on_btnCfgUseDefaultDocEditor_clicked ()
{
	ui->txtCfgDocEditor->setText ( CONFIG ()->docEditor ( true ) );
	ui->txtCfgDocEditor->setFocus ();
}

void configDialog::on_txtCfgXlsEditor_editingFinished()
{
	CONFIG ()->setXlsEditor ( ui->txtCfgXlsEditor->text () );
}

void configDialog::on_btnCfgChooseXlsEditor_clicked ()
{
	const QString filename = fileOps::getOpenFileName ( "/usr/bin/", "*.*" );
	if ( !filename.isEmpty () ) {
		ui->txtCfgXlsEditor->setText ( filename );
		ui->txtCfgXlsEditor->setFocus ();
	}
}

void configDialog::on_btnCfgUseDefaultXlsEditor_clicked ()
{
	ui->txtCfgXlsEditor->setText ( CONFIG ()->docEditor ( true ) );
	ui->txtCfgXlsEditor->setFocus ();
}

void configDialog::on_txtCfgJobsPrefix_editingFinished ()
{
	CONFIG ()->setProjectsBaseDir ( ui->txtCfgJobsPrefix->text () );
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

void configDialog::on_txtCfgESTIMATE_editingFinished ()
{
	CONFIG ()->setEstimatesDir ( ui->txtCfgESTIMATE->text () );
}

void configDialog::on_btnCfgUseDefaultESTIMATEDir_clicked ()
{
	ui->txtCfgESTIMATE->setText ( CONFIG ()->estimatesDirSuffix () );
	ui->txtCfgESTIMATE->setFocus ();
}

void configDialog::on_txtCfgReports_editingFinished ()
{
	CONFIG ()->setReportsDir ( ui->txtCfgReports->text () );
}

void configDialog::on_btnCfgUseDefaultReportsDir_clicked ()
{
	ui->txtCfgReports->setText ( CONFIG ()->reportsDirSuffix () );
	ui->txtCfgReports->setFocus ();
}
