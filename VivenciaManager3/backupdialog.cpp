#include "backupdialog.h"
#include "global.h"
#include "vmlist.h"
#include "vivenciadb.h"
#include "vmcompress.h"
#include "data.h"
#include "configops.h"
#include "fileops.h"
#include "vmnotify.h"
#include "cleanup.h"
#include "heapmanager.h"
#include "emailconfigdialog.h"
#include "crashrestore.h"

#include <QVBoxLayout>
#include <QProgressBar>
#include <QRadioButton>
#include <QPushButton>
#include <QLabel>
#include <QListWidgetItem>
#include <QDesktopWidget>

BackupDialog* BackupDialog::s_instance ( nullptr );

void deleteBackupDialogInstance ()
{
	heap_del ( BackupDialog::s_instance );
}

const uint BACKUP_IDX ( 0 );
const uint RESTORE_IDX ( 1 );

BackupDialog::BackupDialog ()
	: QDialog ( nullptr ), ui ( new Ui::BackupDialog ), tdb ( nullptr ),
	  b_IgnoreItemListChange ( false ), mb_success ( false ), mb_nodb ( false ),
	  progress_bar_steps ( 5 ), dlgNoDB ( nullptr ), m_after_close_action ( ACA_CONTINUE )
{
	backupNotify = new vmNotify ( QStringLiteral ( "TL" ), this );
	ui->setupUi ( this );
	setWindowTitle ( TR_FUNC ( "Backup/Restore - " ) + PROGRAM_NAME );
	setWindowIcon ( ICON ( "vm-logo-22x22" ) );

	setupConnections ();
	fillTable ();
	readFromBackupList ();
	addPostRoutine ( deleteBackupDialogInstance );
}

BackupDialog::~BackupDialog ()
{
	heap_del ( tdb );
	heap_del ( ui );
}

void BackupDialog::setupConnections ()
{
	ui->pBar->setVisible ( false );
	ui->btnApply->setEnabled ( false );
	connect ( ui->btnApply, &QPushButton::clicked, this, [&] () { return btnApply_clicked (); } );
	connect ( ui->btnClose, &QPushButton::clicked, this, [&] () { return btnClose_clicked (); } );

	connect ( ui->txtBackupFilename, &QLineEdit::textChanged, this, [&] ( const QString& text ) { return ui->btnApply->setEnabled ( text.length () > 3 ); } );
	connect ( ui->txtExportPrefix, &QLineEdit::textChanged, this, [&] ( const QString& text ) { return ui->btnApply->setEnabled ( text.length () > 3 ); } );
	connect ( ui->txtExportFolder, &QLineEdit::textChanged, this, [&] ( const QString& text ) { return ui->btnApply->setEnabled ( fileOps::exists ( text ).isOn () ); } );
	ui->txtRestoreFileName->setCallbackForContentsAltered ( [&] ( const vmWidget* ) {
		return ui->btnApply->setEnabled ( checkFile ( ui->txtRestoreFileName->text () ) ); } );

	connect ( ui->rdChooseKnownFile, &QRadioButton::toggled, this, [&] ( const bool checked ) {
		return rdChooseKnownFile_toggled ( checked ); } );
	connect ( ui->rdChooseAnotherFile, &QRadioButton::toggled, this, [&] ( const bool checked ) {
		return rdChooseAnotherFile_toggled ( checked ); } );

	connect ( ui->btnDefaultFilename, &QToolButton::clicked, this, [&] () {
		return ui->txtBackupFilename->setText ( standardDefaultBackupFilename () ); } );
	connect ( ui->btnDefaultPrefix, &QToolButton::clicked, this, [&] () {
		return ui->txtExportPrefix->setText ( vmNumber::currentDate.toDate ( vmNumber::VDF_FILE_DATE ) + QLatin1String ( "-Table_" ) ); } );
	connect ( ui->btnChooseBackupFolder, &QToolButton::clicked, this, [&] () {
		return ui->txtBackupFolder->setText ( fileOps::getExistingDir ( CONFIG ()->backupDir () ) ); } );
	connect ( ui->btnChooseExportFolder, &QToolButton::clicked, this, [&] () {
		return ui->txtExportFolder->setText ( fileOps::getExistingDir ( CONFIG ()->backupDir () ) ); } );
	connect ( ui->btnChooseImportFile, &QToolButton::clicked, this, [&] () {
		return btnChooseImportFile_clicked (); } );
	connect ( ui->btnRemoveFromList, &QToolButton::clicked, this, [&] () {
		return btnRemoveFromList_clicked (); } );
	connect ( ui->btnRemoveAndDelete, &QToolButton::clicked, this, [&] () {
		return btnRemoveAndDelete_clicked (); } );
	connect ( ui->btnSendMail, &QToolButton::clicked, this, [&] () {
		return btnSendMail_clicked () ; } );

	connect ( ui->grpBackup, &QGroupBox::clicked, this, [&] ( const bool checked ) {
		return grpBackup_clicked ( checked ); } );
	connect ( ui->grpExportToText, &QGroupBox::clicked, this, [&] ( const bool checked ) {
		return grpExportToText_clicked ( checked ); } );

	connect ( ui->restoreList, &QListWidget::currentRowChanged, this, [&] ( const int row ) {
		return ui->btnApply->setEnabled ( row != -1 ); } );
	connect ( ui->restoreList, &QListWidget::itemActivated, this, [&] ( QListWidgetItem* item ) {
		return ui->btnApply->setEnabled ( item != nullptr ); } );

	ui->chkTables->setCallbackForContentsAltered ( [&] ( const vmWidget* ) {
		return ui->tablesList->setEnabled ( ui->chkTables->isChecked () ); } );
}

bool BackupDialog::canDoBackup () const
{
	bool ret ( ui->tablesList->count () > 0 );
	if ( ret )
	{
		ret = !( fileOps::appPath ( VivenciaDB::backupApp () ).isEmpty () );
		backupNotify->notifyMessage ( TR_FUNC ( "Backup" ), ret
				? TR_FUNC ( "Choose backup method" ) : VivenciaDB::backupApp () + TR_FUNC ( " must be installed to do backups" ) );
	}
	else
		backupNotify->notifyMessage ( TR_FUNC ( "Backup - Error" ), TR_FUNC ( "Database has no tables to backup" ) );
	return ret;
}

bool BackupDialog::canDoRestore () const
{
	if ( fileOps::appPath ( VivenciaDB::restoreApp () ).isEmpty () )
	{
		backupNotify->notifyMessage ( TR_FUNC ( "Restore - Error" ),
				VivenciaDB::restoreApp () + TR_FUNC ( " must be installed to restore a database" ) );
		return false;
	}
	else
	{
		backupNotify->notifyMessage ( TR_FUNC ( "Restore - Next step" ), ( ui->restoreList->count () > 0 )
			? TR_FUNC ( "Choose restore method" ) : TR_FUNC ( "Choose a file containing a saved database to be restored" ) );
	}
	return true;
}

void BackupDialog::showWindow ()
{
	if ( ui->tablesList->count () == 0 )
		fillTable ();

	const bool b_backupEnabled ( canDoBackup () );
	const bool b_restoreEnabled ( canDoRestore () );

	ui->pageBackup->setEnabled ( b_backupEnabled );
	ui->pageRestore->setEnabled ( b_restoreEnabled );
	ui->toolBox->setCurrentIndex ( b_backupEnabled ? BACKUP_IDX : ( b_restoreEnabled ? RESTORE_IDX : BACKUP_IDX ) );
	ui->txtBackupFilename->clear ();
	ui->txtBackupFolder->clear ();
	ui->txtExportFolder->clear ();
	ui->txtExportPrefix->clear ();
	ui->txtRestoreFileName->clear ();
	ui->btnApply->setEnabled ( false );

	const bool hasRestoreList ( ui->restoreList->count () > 0 );
	ui->btnSendMail->setEnabled ( hasRestoreList );
	ui->rdChooseKnownFile->setChecked ( hasRestoreList );
	ui->rdChooseAnotherFile->setChecked ( !hasRestoreList );

	m_after_close_action = ACA_RETURN_TO_PREV_WINDOW;
	this->exec ();
}

void BackupDialog::fillTable ()
{
	if ( VDB () != nullptr )
	{
		// this function will be called by Data after a restore. Because the tables might be different, we must clear the last used list
		if ( ui->tablesList->count () != 0 )
		{
			ui->tablesList->clear ();
			disconnect ( ui->tablesList, nullptr, nullptr, nullptr );
		}

		QListWidgetItem* widgetitem = new QListWidgetItem ( ui->tablesList );
		widgetitem->setText ( TR_FUNC ( "Select all " ) );
		widgetitem->setFlags ( Qt::ItemIsEnabled|Qt::ItemIsTristate|Qt::ItemIsSelectable|Qt::ItemIsUserCheckable );
		widgetitem->setCheckState ( Qt::Checked );

		for ( uint i ( 0 ); i < TABLES_IN_DB; ++i )
		{
			widgetitem = new QListWidgetItem ( ui->tablesList );
			widgetitem->setText ( VivenciaDB::tableInfo ( i )->table_name );
			widgetitem->setFlags ( Qt::ItemIsEnabled|Qt::ItemIsSelectable|Qt::ItemIsUserCheckable );
			widgetitem->setCheckState ( Qt::Checked );
		}
		connect ( ui->tablesList, &QListWidget::itemChanged, this,
				  [&] ( QListWidgetItem* item ) { return selectAll ( item ); } );
	}
}

bool BackupDialog::doBackup ( const QString& filename, const QString& path, const bool bUserInteraction )
{
	BackupDialog* bDlg ( bUserInteraction ? BACKUP () : nullptr );
	if ( !BackupDialog::checkDir ( path ) )
	{
		if ( bDlg )
			bDlg->backupNotify->notifyMessage ( TR_FUNC ( "Backup - Error"), TR_FUNC ( "You must select a valid directory before proceeding" ) );
		return false;
	}
	if ( filename.isEmpty () )
	{
		if ( bDlg )
			bDlg->backupNotify->notifyMessage ( TR_FUNC ( "Backup - Error"), TR_FUNC ( "A filename must be supplied" ) );
		return false;
	}

	QString backupFile ( path + filename + QLatin1String ( ".sql" ) );

	bool ok ( false );
	if ( EXITING_PROGRAM || checkThatFileDoesNotExist ( backupFile + QLatin1String ( ".bz2" ), bUserInteraction ) )
	{
		QString tables;
		if ( bDlg )
		{
			if ( bDlg->ui->chkDocs->isChecked () )
				bDlg->addDocuments ( backupFile );
			if ( bDlg->ui->chkImages->isChecked () )
				bDlg->addImages ( backupFile );

			if ( bDlg->ui->chkTables->isChecked () )
			{
				bDlg->initProgressBar ( 7 );
				if ( bDlg->ui->tablesList->item ( 0 )->checkState () == Qt::Unchecked )
				{
					bDlg->backupNotify->notifyMessage ( TR_FUNC ( "Backup - Error" ), TR_FUNC ( "One table at least must be selected." ) );
					return false;
				}

				if ( bDlg->ui->tablesList->item ( 0 )->checkState () == Qt::PartiallyChecked )
				{
					for ( int i ( 1 ); i < bDlg->ui->tablesList->count (); ++i )
					{
						if ( bDlg->ui->tablesList->item ( i )->checkState () == Qt::Checked )
							tables += bDlg->ui->tablesList->item ( i )->text () + CHR_SPACE;
					}
				}
			}
			BackupDialog::incrementProgress ( bDlg ); //1
		}
		ok = VDB ()->doBackup ( backupFile, tables, bDlg );
		BackupDialog::incrementProgress ( bDlg ); //5

		if ( ok )
		{
			if ( VMCompress::compress ( backupFile, backupFile + QLatin1String ( ".bz2" ) ) )
			{
				fileOps::removeFile ( backupFile );
				backupFile += QLatin1String ( ".bz2" );
				BackupDialog::incrementProgress ( bDlg ); //6

				const QString dropBoxDir ( CONFIG ()->dropboxDir () );
				if ( fileOps::isDir ( dropBoxDir ).isOn () )
					fileOps::copyFile ( dropBoxDir, backupFile );
				BackupDialog::incrementProgress ( bDlg ); //7
			}
		}
		if ( bDlg )
		{
			bDlg->backupNotify->notifyMessage ( TR_FUNC ( "Backup" ), TR_FUNC ( "Standard backup to file %1 was %2" ).arg (
					filename, QLatin1String ( ok ? " successfull" : " unsuccessfull" ) ) );
		}
		if ( ok )
			BackupDialog::addToRestoreList ( backupFile, bDlg );
	}
	return ok;
}

void BackupDialog::doDayBackup ()
{
	if ( !VDB ()->backUpSynced () )
		BackupDialog::doBackup ( standardDefaultBackupFilename (), CONFIG ()->backupDir () );
}

bool BackupDialog::doExport ( const QString& prefix, const QString& path, const bool bUserInteraction )
{
	BackupDialog* bDlg ( bUserInteraction ? BACKUP () : nullptr );
	if ( bDlg )
	{
		if ( bDlg->ui->tablesList->item ( 0 )->checkState () == Qt::Unchecked )
		{
			bDlg->backupNotify->notifyMessage ( TR_FUNC ( "Export - Error" ), TR_FUNC ( "One table at least must be selected." ) );
			return false;
		}
	}

	if ( !BackupDialog::checkDir ( path ) )
	{
		if ( bDlg )
			bDlg->backupNotify->notifyMessage ( TR_FUNC ( "Export - Error" ), TR_FUNC ( "Error: you must select a valid directory before proceeding" ) );
		return false;
	}
	if ( prefix.isEmpty () )
	{
		if ( bDlg )
			bDlg->backupNotify->notifyMessage ( TR_FUNC ( "Export - Error" ), TR_FUNC ( "Error: a prefix must be supplied" ) );
		return false;
	}

	QString filepath;
	bool ok ( false );

	if ( bDlg )
	{
		bDlg->initProgressBar ( (bDlg->ui->tablesList->count () - 1) * 5 );

		for ( int i ( 1 ); i < bDlg->ui->tablesList->count (); ++i )
		{
			if ( bDlg->ui->tablesList->item ( i )->checkState () == Qt::Checked )
			{
				filepath = path + CHR_F_SLASH + prefix + bDlg->ui->tablesList->item ( i )->text ();
				if ( checkThatFileDoesNotExist ( filepath, true ) )
				{
					BackupDialog::incrementProgress ( bDlg ); //1
					ok = VDB ()->exportToCSV ( static_cast<uint>(2<<(i-1)), filepath, bDlg );
					if ( ok )
						BackupDialog::addToRestoreList ( filepath, bDlg );
				}
			}
		}
		
		ui->pBar->hide ();
		backupNotify->notifyMessage ( TR_FUNC ( "Export" ), TR_FUNC ( "Export to CSV file was " ) + ( ok ? TR_FUNC ( " successfull" ) : TR_FUNC ( " unsuccessfull" ) ) );
	}
	else
	{
		for ( uint i ( 0 ); i < TABLES_IN_DB; ++i )
		{
			filepath = path + CHR_F_SLASH + prefix + VDB ()->tableName ( static_cast<TABLE_ORDER>(i) );
			if ( checkThatFileDoesNotExist ( filepath, false ) )
			{
				ok = VDB ()->exportToCSV ( static_cast<uint>(2)<<i, filepath, bDlg );
				if ( ok )
					BackupDialog::addToRestoreList ( filepath, bDlg );
			}
		}	
	}
	return ok;
}

bool BackupDialog::doRestore ( const QString& files )
{
	QStringList files_list ( files.split ( CHR_SPACE, QString::SkipEmptyParts ) );
	if ( files_list.count () <= 0 )
		return false;

	QString filepath;
	initProgressBar ( 5 * files_list.count () );
	bool ok ( false );

	QStringList::const_iterator itr ( files_list.constBegin () );
	const QStringList::const_iterator itr_end ( files_list.constEnd () );
	BackupDialog::incrementProgress ( this ); //1
	for ( ; itr != itr_end; ++itr )
	{
		filepath = static_cast<QString> ( *itr );
		if ( !checkFile ( filepath ) )
		{
			backupNotify->notifyMessage ( TR_FUNC ( "Restore - Error " ), filepath + TR_FUNC ( "is an invalid file" ) );
			continue;
		}

		if ( fileOps::exists ( filepath ).isOn () )
		{
			BackupDialog::incrementProgress ( this ); //2
			if ( textFile::isTextFile ( filepath, textFile::TF_DATA ) )
				ok |= VDB ()->importFromCSV ( filepath, this );
			else
			{
				ok |= VDB ()->doRestore ( filepath, this );
				BackupDialog::incrementProgress ( this );//5
			}
		}
	}
	if ( ok )
	{
		if ( files_list.count () == 1 )
			addToRestoreList ( files, this );

		/* Force rescan of all tables, which is done in updateGeneralTable (called by VivenciaDB
		 * when general table cannot be found or there is a version mismatch
		 */
		VDB ()->clearTable ( VDB ()->tableInfo ( TABLE_GENERAL_ORDER ) );
		m_after_close_action = mb_nodb ? ACA_RETURN_TO_PREV_WINDOW : ACA_RESTART;
		backupNotify->notifyMessage ( TR_FUNC ( "Importing was sucessfull." ),
									  TR_FUNC ( "Click on the Close button to resume the application." ) );
	}
	else
	{
		m_after_close_action = mb_nodb ? ACA_RETURN_TO_PREV_WINDOW : ACA_CONTINUE;
		backupNotify->notifyMessage ( TR_FUNC ( "Importing failed!" ),
									  TR_FUNC ( "Try importing from another file or click on Close to exit the program." ) );
	}
	return ok;
}

void BackupDialog::addToRestoreList ( const QString& filepath, BackupDialog* bDlg )
{
	if ( bDlg )
	{
		for ( int i ( 0 ); i < bDlg->ui->restoreList->count (); ++i )
			if ( bDlg->ui->restoreList->item ( i )->text () == filepath ) return;
		bDlg->ui->restoreList->addItem ( filepath );
		bDlg->ui->btnSendMail->setEnabled ( bDlg->ui->restoreList->count () != 0 );
		bDlg->tdb->appendRecord ( filepath );
		bDlg->tdb->commit ();
	}
	else
	{
		dataFile* df ( new dataFile ( CONFIG ()->appDataDir () + QLatin1String ( "/backups.db" ) ) );
		df->load ();
		df->appendRecord ( filepath );
		df->commit ();
		delete df;
	}
}

void BackupDialog::readFromBackupList ()
{
	if ( !tdb )
		tdb = new dataFile ( CONFIG ()->appDataDir () + QLatin1String ( "/backups.db" ) );
	if ( tdb->load ().isOn () )
	{
		stringRecord files;
		if ( tdb->getRecord ( files, 0 ) )
		{
			if ( files.first () )
			{
				do
				{
					if ( checkFile ( files.curValue () ) )
						ui->restoreList->addItem ( files.curValue () );
					else // file does not exist; do not add to the list and remove it from database
					{
						files.removeFieldByValue ( files.curValue (), true );
						tdb->changeRecord ( 0, files );
					}
				} while ( files.next () );
				tdb->commit ();
			}
		}
	}
}

void BackupDialog::incrementProgress ( BackupDialog* bDlg )
{
	if ( bDlg != nullptr )
	{
		static int current_val ( 0 );
		if ( current_val < bDlg->progress_bar_steps )
			bDlg->ui->pBar->setValue ( ++current_val );
		else
			current_val = 0;
	}
}

void BackupDialog::showNoDatabaseOptionsWindow ()
{
	mb_nodb = true;
	m_after_close_action = ACA_RETURN_TO_PREV_WINDOW;
	if ( dlgNoDB == nullptr )
	{
		dlgNoDB = new QDialog ( this );
		dlgNoDB->setWindowIcon ( ICON ( "vm-logo-22x22" ) );
		dlgNoDB->setWindowTitle ( TR_FUNC ( "Database inexistent" ) );

		QLabel* lblExplanation ( new QLabel ( TR_FUNC (
				"There is no database present. This either means that this is the first time this program "
				"is used or the current database was deleted. You must choose one of the options below:" ) ) );
		lblExplanation->setAlignment ( Qt::AlignJustify );
		lblExplanation->setWordWrap ( true );

		rdImport = new QRadioButton ( TR_FUNC ( "Import from backup" ) );
		rdImport->setMinimumHeight ( 30 );
		rdImport->setChecked ( true );
		rdStartNew = new QRadioButton ( TR_FUNC ( "Start a new database" ) );
		rdStartNew->setMinimumHeight ( 30 );
		rdStartNew->setChecked ( false );
		rdNothing = new QRadioButton ( TR_FUNC ( "Do nothing and exit" ) );
		rdNothing->setMinimumHeight ( 30 );
		rdNothing->setChecked ( false );

		btnProceed = new QPushButton ( TR_FUNC ( "&Proceed" ) );
		btnProceed->setMinimumSize ( 100, 30 );
		btnProceed->setDefault ( true );
		connect ( btnProceed, &QPushButton::clicked, this, [&] () { return btnNoDBProceed_clicked (); } );

		QVBoxLayout* layout ( new QVBoxLayout );
		layout->addWidget ( lblExplanation, 1, Qt::AlignJustify );
		layout->addWidget ( rdImport, 1, Qt::AlignLeft );
		layout->addWidget ( rdStartNew, 1, Qt::AlignLeft );
		layout->addWidget ( rdNothing, 1, Qt::AlignLeft );
		layout->addWidget ( btnProceed, 1, Qt::AlignCenter );

		layout->setMargin ( 0 );
		layout->setSpacing ( 1 );
		dlgNoDB->adjustSize ();
		dlgNoDB->setLayout ( layout );
		const QRect desktopGeometry ( qApp->desktop ()->availableGeometry ( -1 ) );
		dlgNoDB->move ( ( desktopGeometry.width () - width () ) / 2, ( desktopGeometry.height () - height () ) / 2  );
	}
	dlgNoDB->exec ();
}

void BackupDialog::btnNoDBProceed_clicked ()
{
	if ( !rdNothing->isChecked () )
	{
		ui->toolBox->setCurrentIndex ( 1 );
		ui->toolBox->setEnabled ( true );
		ui->toolBox->widget ( 0 )->setEnabled ( false );
		APP_RESTORER ()->setNewDBSession ();

		if ( VDB ()->createDatabase () )
		{
			if ( VDB ()->createUser () )
			{
				if ( rdImport->isChecked ( ) )
				{
					dlgNoDB->hide ();
					exec ();
				}
				else
					mb_success = VDB ()->createAllTables ();
				if ( actionSuccess () )
				{
					mb_nodb = false;
					dlgNoDB->done ( QDialog::Accepted );
					return;
				}
			}
		}
		dlgNoDB->done ( QDialog::Rejected );
		backupNotify->notifyMessage ( TR_FUNC ( "Database creation failed" ),
									  TR_FUNC ( "Please try again. If the problem persists ... who knows?" ), 3000, true );
		close ();
	}
	else
		qApp->exit ( 1 );
}

void BackupDialog::selectAll ( QListWidgetItem* item )
{
	if ( b_IgnoreItemListChange ) return;

	b_IgnoreItemListChange = true;
	if ( item == ui->tablesList->item ( 0 ) )
	{
		for ( int i ( 1 ); i < ui->tablesList->count (); ++i )
			ui->tablesList->item ( i )->setCheckState ( item->checkState () );
	}
	else
	{
		int checked ( 0 );
		for ( int i ( 1 ); i < ui->tablesList->count (); ++i )
		{
			if ( ui->tablesList->item ( i )->checkState () == Qt::Checked )
				++checked;
			else
				--checked;
		}
		if ( checked == ui->tablesList->count () - 1 )
			ui->tablesList->item ( 0 )->setCheckState ( Qt::Checked );
		else if ( checked == 0 - ( ui->tablesList->count () - 1 ) )
			ui->tablesList->item ( 0 )->setCheckState ( Qt::Unchecked );
		else
			ui->tablesList->item ( 0 )->setCheckState ( Qt::PartiallyChecked );
	}
	b_IgnoreItemListChange = false;
}

void BackupDialog::btnApply_clicked ()
{
	ui->btnClose->setEnabled ( false );
	mb_success = false;
	if ( ui->toolBox->currentIndex () == 0 )
	{ // backup
		if ( ui->grpBackup->isChecked () )
		{
			if ( !ui->txtBackupFolder->text ().isEmpty () )
			{
				if ( ui->txtBackupFolder->text ().at ( ui->txtBackupFolder->text ().length () - 1 ) != '/' )
					ui->txtBackupFolder->setText ( ui->txtBackupFolder->text () + CHR_F_SLASH );
				mb_success = doBackup ( ui->txtBackupFilename->text (), ui->txtBackupFolder->text (), true );
			}
		}
		else
			mb_success = doExport ( ui->txtExportPrefix->text (), ui->txtExportFolder->text (), true );
	}
	else
	{ // restore
		QString selected;
		if ( ui->rdChooseKnownFile->isChecked () )
		{
			if ( ui->restoreList->currentRow () <= 0 )
				ui->restoreList->setCurrentRow ( ui->restoreList->count () - 1 );
			selected = ui->restoreList->currentItem ()->text ();
		}
		else
			selected = ui->txtRestoreFileName->text ();
		mb_success = doRestore ( selected );
	}
	ui->btnApply->setEnabled ( false );
	ui->btnClose->setEnabled ( true );
}

void BackupDialog::btnClose_clicked ()
{
	switch ( m_after_close_action )
	{
		case ACA_RETURN_TO_PREV_WINDOW:
			done ( actionSuccess () );
		break;
		case ACA_RESTART:
			Data::restartProgram ();
		break;
		case ACA_CONTINUE:
			ui->btnApply->setEnabled ( true );
		break;
	}
}

void BackupDialog::grpBackup_clicked ( const bool checked )
{
	ui->grpExportToText->setChecked ( !checked );
	ui->chkTables->setChecked ( checked );
	ui->tablesList->setEnabled ( checked );
	ui->btnApply->setEnabled ( true );
}

void BackupDialog::grpExportToText_clicked ( const bool checked )
{
	ui->grpBackup->setChecked ( !checked );
	ui->btnApply->setEnabled ( true );
}
//-----------------------------------------------------------RESTORE---------------------------------------------------

bool BackupDialog::getSelectedItems ( QString& selected )
{
	if ( !ui->restoreList->selectedItems ().isEmpty () )
	{
		selected.clear ();
		QList<QListWidgetItem*>::const_iterator itr ( ui->restoreList->selectedItems ().constBegin () );
		const QList<QListWidgetItem*>::const_iterator itr_end ( ui->restoreList->selectedItems ().constEnd () );
		for ( ; itr != itr_end; ++itr )
		{
			if ( !selected.isEmpty () )
				selected += CHR_SPACE;
			selected += static_cast<QListWidgetItem*> ( *itr )->text ();
		}
		return true;
	}
	else
		backupNotify->notifyMessage ( TR_FUNC ( "Restore - Error" ), TR_FUNC ( "No file selected" ) );
	return false;
}

void BackupDialog::initProgressBar ( const int max )
{
	progress_bar_steps = max;
	ui->pBar->setRange ( 0, progress_bar_steps );
	ui->pBar->setValue ( 0 );
	ui->pBar->setVisible ( true );
}

bool BackupDialog::checkThatFileDoesNotExist ( const QString& filepath, const bool bUserInteraction )
{
	if ( fileOps::exists ( filepath ).isOn () )
	{
		if ( bUserInteraction )
		{
			const QString question ( QString ( TR_FUNC ( "The file %1 already exists. Overwrite it?" ) ).arg ( filepath ) );
			if ( !BACKUP ()->backupNotify->questionBox ( TR_FUNC ( "Same filename" ), question ) )
				return false;
			fileOps::removeFile ( filepath );
		}
		else
			return false;
	}
	return true;
}

bool BackupDialog::checkDir ( const QString& dir )
{
	if ( fileOps::isDir ( dir ).isOn () )
	{
		if ( fileOps::canWrite ( dir ).isOn () )
		{
			if ( fileOps::canRead ( dir ).isOn () )
				return fileOps::canExecute ( dir ).isOn ();
		}
	}
	return false;
}

bool BackupDialog::checkFile ( const QString& file )
{
	if ( fileOps::isFile ( file ).isOn () )
	{
		return fileOps::canRead ( file ).isOn ();
	}
	return false;
}

void BackupDialog::addDocuments ( const QString& tar_file )
{
	tar_file.toInt ();
// Browse CONFIG ()->projectsBaseDir () for files with CONFIG ()->xxxExtension
	// Copy them to the temporary dir (in /tmp)
	// Add temp dir to tar_file
	// erase temp dir whatever the result
	//TODO
}


void BackupDialog::addImages ( const QString& tar_file )
{
	tar_file.toInt ();
}

void BackupDialog::rdChooseKnownFile_toggled ( const bool checked )
{
	ui->rdChooseAnotherFile->setChecked ( !checked );
	ui->txtRestoreFileName->setEnabled ( !checked );
	ui->btnChooseImportFile->setEnabled ( !checked );
	ui->restoreList->setEnabled ( checked );
	ui->btnApply->setEnabled ( checked && ui->restoreList->count () > 0 );
}

void BackupDialog::rdChooseAnotherFile_toggled ( const bool checked )
{
	ui->rdChooseKnownFile->setChecked ( !checked );
	ui->txtRestoreFileName->setEnabled ( checked );
	ui->btnChooseImportFile->setEnabled ( checked );
	ui->restoreList->setEnabled ( !checked );
	ui->btnApply->setEnabled ( checked && !ui->txtRestoreFileName->text ().isEmpty () );
}

void BackupDialog::btnChooseImportFile_clicked ()
{
	const QString selectedFile ( fileOps::getOpenFileName ( CONFIG ()->backupDir (), TR_FUNC ( "Database files ( *.bz2 *.sql )" ) ) );
	if ( !selectedFile.isEmpty () )
		ui->txtRestoreFileName->setText ( selectedFile );
	ui->btnApply->setEnabled ( !selectedFile.isEmpty () );
}

void BackupDialog::btnRemoveFromList_clicked ()
{
	if ( ui->restoreList->currentItem () != nullptr )
	{
		QListWidgetItem* item ( ui->restoreList->currentItem () );
		stringRecord files;
		if ( tdb->getRecord ( files, 0 ) )
		{
			files.removeField ( static_cast<uint>(ui->restoreList->currentRow ()) );
			tdb->changeRecord ( 0, files );
			tdb->commit ();
		}
		ui->restoreList->removeItemWidget ( item );
		delete item;
	}
}

void BackupDialog::btnRemoveAndDelete_clicked ()
{
	if ( ui->restoreList->currentItem () != nullptr )
	{
		fileOps::removeFile( ui->restoreList->currentItem ()->text () );
		btnRemoveFromList_clicked ();
	}
}

void BackupDialog::btnSendMail_clicked ()
{
	QString selected;
	if ( getSelectedItems ( selected ) )
		EMAIL_CONFIG ()->sendEMail ( CONFIG ()->defaultEmailAddress (), TR_FUNC ( "Backup file" ), selected );
}

