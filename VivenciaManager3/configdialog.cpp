#include "configdialog.h"
#include "ui_configdialog.h"
#include "global.h"
#include "configops.h"
#include "fileops.h"
#include "vmwidgets.h"
#include "emailconfigdialog.h"
#include "actionpanelscheme.h"
#include "mainwindow.h"

#include <QMenu>

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
	getAppSchemes ();

	static_cast<void>( connect ( ui->btnCfgChooseDataFolder, &QToolButton::clicked, this, [&] () { return on_btnCfgChooseDataFolder_clicked (); } ) );
	static_cast<void>( connect ( ui->btnCfgUseDefaultDataFolder, &QToolButton::clicked, this, [&] () { return on_btnCfgUseDefaultDataFolder_clicked (); } ) );
	static_cast<void>( connect ( ui->btnCfgChooseFileManager, &QToolButton::clicked, this, [&] () { return on_btnCfgChooseFileManager_clicked (); } ) );
	static_cast<void>( connect ( ui->btnCfgUseDefaultFileManager, &QToolButton::clicked, this, [&] () { return on_btnCfgUseDefaultFileManager_clicked (); } ) );
	static_cast<void>( connect ( ui->btnCfgChoosePictureViewer, &QToolButton::clicked, this, [&] () { return on_btnCfgChoosePictureViewer_clicked (); } ) );
	static_cast<void>( connect ( ui->btnCfgUseDefaultPictureViewer, &QToolButton::clicked, this, [&] () { return on_btnCfgUseDefaultPictureViewer_clicked (); } ) );
	static_cast<void>( connect ( ui->btnCfgChoosePictureEditor, &QToolButton::clicked, this, [&] () { return on_btnCfgChoosePictureEditor_clicked (); } ) );
	static_cast<void>( connect ( ui->btnCfgUseDefaultPictureEditor, &QToolButton::clicked, this, [&] () { return on_btnCfgUseDefaultPictureEditor_clicked (); } ) );
	static_cast<void>( connect ( ui->btnCfgChooseEMailClient, &QToolButton::clicked, this, [&] () { return on_btnCfgChooseEMailClient_clicked (); } ) );
	static_cast<void>( connect ( ui->btnCfgChooseDocViewer, &QToolButton::clicked, this, [&] () { return on_btnCfgChooseDocViewer_clicked (); } ) );
	static_cast<void>( connect ( ui->btnCfgUseDefaultDocumentViewer, &QToolButton::clicked, this, [&] () { return on_btnCfgUseDefaultDocumentViewer_clicked (); } ) );
	static_cast<void>( connect ( ui->btnCfgChooseDocEditor, &QToolButton::clicked, this, [&] () { return on_btnCfgChooseDocEditor_clicked (); } ) );
	static_cast<void>( connect ( ui->btnCfgUseDefaultDocEditor, &QToolButton::clicked, this, [&] () { return on_btnCfgUseDefaultDocEditor_clicked (); } ) );
	static_cast<void>( connect ( ui->btnCfgChooseXlsEditor, &QToolButton::clicked, this, [&] () { return on_btnCfgChooseXlsEditor_clicked (); } ) );
	static_cast<void>( connect ( ui->btnCfgUseDefaultXlsEditor, &QToolButton::clicked, this, [&] () { return on_btnCfgUseDefaultXlsEditor_clicked (); } ) );
	static_cast<void>( connect ( ui->btnCfgChooseBaseDir, &QToolButton::clicked, this, [&] () { return on_btnCfgChooseBaseDir_clicked (); } ) );
	static_cast<void>( connect ( ui->btnCfgUseDefaultBaseDir, &QToolButton::clicked, this, [&] () { return on_btnCfgUseDefaultBaseDir_clicked (); } ) );
	static_cast<void>( connect ( ui->btnCfgChooseEstimatesDir, &QToolButton::clicked, this, [&] () { return on_btnCfgChooseESTIMATEDir_clicked (); } ) );
	static_cast<void>( connect ( ui->btnCfgUseDefaultEstimatesDir, &QToolButton::clicked, this, [&] () { return on_btnCfgUseDefaultESTIMATEDir_clicked (); } ) );
	static_cast<void>( connect ( ui->btnCfgChooseReportsDir, &QToolButton::clicked, this, [&] () { return on_btnCfgChooseReportsDir_clicked (); } ) );
	static_cast<void>( connect ( ui->btnCfgUseDefaultReportsDir, &QToolButton::clicked, this, [&] () { return on_btnCfgUseDefaultReportsDir_clicked (); } ) );
	static_cast<void>( connect ( ui->btnCfgChooseDropboxDir, &QToolButton::clicked, this, [&] () { return on_btnCfgChooseDropBoxDir_clicked (); } ) );
	static_cast<void>( connect ( ui->btnCfgUseDefaultDropboxDir, &QToolButton::clicked, this, [&] () { return on_btnCfgUseDefaultDropBoxDir_clicked (); } ) );
			
			
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
	ui->txtCfgScheme->setCallbackForContentsAltered ( [&] (const vmWidget* txtWidget ) {
			return CONFIG ()->setAppScheme ( txtWidget->text () ); } );
	
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
	ui->txtCfgScheme->setEditable ( true );
	
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
	ui->txtCfgDropbox->setText ( CONFIG ()->dropboxDir () );
	ui->txtCfgScheme->setText ( CONFIG ()->appScheme () );
}

void configDialog::getAppSchemes ()
{
	QMenu* schemes_menu ( new QMenu );
	schemes_menu->addAction ( ActionPanelScheme::PanelStyleStr[ActionPanelScheme::PANEL_NONE], this, 
			[&] () { return setAppScheme ( ActionPanelScheme::PanelStyleStr[ActionPanelScheme::PANEL_NONE] ); } );
	schemes_menu->addAction ( ActionPanelScheme::PanelStyleStr[ActionPanelScheme::PANEL_DEFAULT], this, 
			[&] () { return setAppScheme ( ActionPanelScheme::PanelStyleStr[ActionPanelScheme::PANEL_DEFAULT] ); } );
	schemes_menu->addAction ( ActionPanelScheme::PanelStyleStr[ActionPanelScheme::PANEL_DEFAULT_2], this, 
			[&] () { return setAppScheme ( ActionPanelScheme::PanelStyleStr[ActionPanelScheme::PANEL_DEFAULT_2] ); } );
	schemes_menu->addAction ( ActionPanelScheme::PanelStyleStr[ActionPanelScheme::PANEL_VISTA], this, 
			[&] () { return setAppScheme ( ActionPanelScheme::PanelStyleStr[ActionPanelScheme::PANEL_VISTA] ); } );
	schemes_menu->addAction ( ActionPanelScheme::PanelStyleStr[ActionPanelScheme::PANEL_XP_1], this, 
			[&] () { return setAppScheme ( ActionPanelScheme::PanelStyleStr[ActionPanelScheme::PANEL_XP_1] ); } );
	schemes_menu->addAction ( ActionPanelScheme::PanelStyleStr[ActionPanelScheme::PANEL_XP_2], this, 
			[&] () { return setAppScheme ( ActionPanelScheme::PanelStyleStr[ActionPanelScheme::PANEL_XP_2] ); } );
	
	ui->btnCfgScheme->setMenu ( schemes_menu );
}

void configDialog::setAppScheme ( const QString& style )
{
	ui->txtCfgScheme->setText ( style, true );
	MAINWINDOW ()->changeSchemeStyle ( style );
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
	ui->txtCfgConfigFile->setText ( CONFIG ()->appConfigFile ( true ), true );
	ui->txtCfgConfigFile->setFocus ();
}

void configDialog::on_btnCfgChooseDataFolder_clicked ()
{
	const QString dir ( fileOps::getExistingDir ( CONFIG ()->appDataDir ( true ) ) );
	if ( !dir.isEmpty () ) {
		ui->txtCfgDataFolder->setText ( dir, true );
		ui->txtCfgDataFolder->setFocus ();
	}
}

void configDialog::on_btnCfgUseDefaultDataFolder_clicked ()
{
	ui->txtCfgDataFolder->setText ( CONFIG ()->appDataDir ( true ), true );
	ui->txtCfgDataFolder->setFocus ();
}

void configDialog::on_btnCfgChooseFileManager_clicked ()
{
	const QString filename ( fileOps::getOpenFileName ( QStringLiteral ( "/usr/bin/" ), QStringLiteral ( "*.*" ) ) );
	if ( !filename.isEmpty () )
	{
		ui->txtCfgFileManager->setText ( filename, true );
		ui->txtCfgFileManager->setFocus ();
	}
}

void configDialog::on_btnCfgUseDefaultFileManager_clicked ()
{
	ui->txtCfgFileManager->setText ( CONFIG ()->fileManager ( true ), true );
	ui->txtCfgFileManager->setFocus ();
}

void configDialog::on_btnCfgChoosePictureViewer_clicked ()
{
	const QString filename ( fileOps::getOpenFileName ( QStringLiteral ( "/usr/bin/" ), QStringLiteral ( "*.*" ) ) );
	if ( !filename.isEmpty () )
	{
		ui->txtCfgPictureViewer->setText ( filename, true );
		ui->txtCfgPictureViewer->setFocus ();
	}
}

void configDialog::on_btnCfgUseDefaultPictureViewer_clicked ()
{
	ui->txtCfgPictureViewer->setText ( CONFIG ()->pictureViewer ( true ), true );
	ui->txtCfgPictureViewer->setFocus ();
}

void configDialog::on_btnCfgChoosePictureEditor_clicked ()
{
	const QString filename ( fileOps::getOpenFileName ( QStringLiteral ( "/usr/bin/" ), QStringLiteral ( "*.*" ) ) );
	if ( !filename.isEmpty () )
	{
		ui->txtCfgPictureEditor->setText ( filename, true );
		ui->txtCfgPictureEditor->setFocus ();
	}
}

void configDialog::on_btnCfgUseDefaultPictureEditor_clicked ()
{
	ui->txtCfgPictureEditor->setText ( CONFIG ()->pictureEditor ( true ), true );
	ui->txtCfgPictureEditor->setFocus ();
}

void configDialog::on_btnCfgChooseEMailClient_clicked ()
{
	EMAIL_CONFIG ()->showDlg ();
	ui->txtCfgEmailClient->setText ( CONFIG ()->emailClient (), true ); // EMAIL_CONFIG will set the client app
}

void configDialog::on_btnCfgChooseDocViewer_clicked ()
{
	const QString filename ( fileOps::getOpenFileName ( QStringLiteral ( "/usr/bin/" ), QStringLiteral ( "*.*" ) ) );
	if ( !filename.isEmpty () )
	{
		ui->txtCfgDocumentViewer->setText ( filename, true );
		ui->txtCfgDocumentViewer->setFocus ();
	}
}

void configDialog::on_btnCfgUseDefaultDocumentViewer_clicked ()
{
	ui->txtCfgDocumentViewer->setText ( CONFIG ()->universalViewer ( true ), true );
	ui->txtCfgDocumentViewer->setFocus ();
}

void configDialog::on_btnCfgChooseDocEditor_clicked ()
{
	const QString filename ( fileOps::getOpenFileName ( QStringLiteral ( "/usr/bin/" ), QStringLiteral ( "*.*" ) ) );
	if ( !filename.isEmpty () )
	{
		ui->txtCfgDocEditor->setText ( filename, true );
		ui->txtCfgDocEditor->setFocus ();
	}
}

void configDialog::on_btnCfgUseDefaultDocEditor_clicked ()
{
	ui->txtCfgDocEditor->setText ( CONFIG ()->docEditor ( true ), true );
	ui->txtCfgDocEditor->setFocus ();
}

void configDialog::on_btnCfgChooseXlsEditor_clicked ()
{
	const QString filename ( fileOps::getOpenFileName ( QStringLiteral ( "/usr/bin/" ), QStringLiteral ( "*.*" ) ) );
	if ( !filename.isEmpty () )
	{
		ui->txtCfgXlsEditor->setText ( filename, true );
		ui->txtCfgXlsEditor->setFocus ();
	}
}

void configDialog::on_btnCfgUseDefaultXlsEditor_clicked ()
{
	ui->txtCfgXlsEditor->setText ( CONFIG ()->docEditor ( true ), true );
	ui->txtCfgXlsEditor->setFocus ();
}

void configDialog::on_btnCfgChooseBaseDir_clicked ()
{
	const QString dir ( fileOps::getExistingDir ( ui->txtCfgJobsPrefix->text () ) );
	if ( !dir.isEmpty () )
	{
		ui->txtCfgJobsPrefix->setText ( dir, true );
		ui->txtCfgJobsPrefix->setFocus ();
	}
}

void configDialog::on_btnCfgUseDefaultBaseDir_clicked ()
{
	ui->txtCfgJobsPrefix->setText ( CONFIG ()->projectsBaseDir ( true ), true );
	ui->txtCfgJobsPrefix->setFocus ();
}

void configDialog::on_btnCfgChooseESTIMATEDir_clicked ()
{
	const QString dir ( fileOps::getExistingDir ( CONFIG ()->projectsBaseDir ( true ) + CONFIG ()->estimatesDirSuffix () ) );
	if ( !dir.isEmpty () )
	{
		ui->txtCfgEstimate->setText ( dir, true );
		ui->txtCfgEstimate->setFocus ();
	}
}

void configDialog::on_btnCfgUseDefaultESTIMATEDir_clicked ()
{
	ui->txtCfgEstimate->setText ( CONFIG ()->estimatesDirSuffix (), true );
	ui->txtCfgEstimate->setFocus ();
}

void configDialog::on_btnCfgChooseReportsDir_clicked ()
{
	const QString dir ( fileOps::getExistingDir ( CONFIG ()->projectsBaseDir ( true ) + CONFIG ()->reportsDirSuffix () ) );
	if ( !dir.isEmpty () )
	{
		ui->txtCfgReports->setText ( dir, true );
		ui->txtCfgReports->setFocus ();
	}
}

void configDialog::on_btnCfgUseDefaultReportsDir_clicked ()
{
	ui->txtCfgReports->setText ( CONFIG ()->reportsDirSuffix (), true );
	ui->txtCfgReports->setFocus ();
}

void configDialog::on_btnCfgChooseDropBoxDir_clicked ()
{
	const QString dir ( fileOps::getExistingDir ( CONFIG ()->dropboxDir() ) );
	if ( !dir.isEmpty () )
	{
		ui->txtCfgDropbox->setText ( dir, true );
		ui->txtCfgDropbox->setFocus ();
	}
}

void configDialog::on_btnCfgUseDefaultDropBoxDir_clicked ()
{
	ui->txtCfgDropbox->setText ( CONFIG ()->dropboxDir ( true ), true );
	ui->txtCfgDropbox->setFocus ();
}
