#ifndef BACKUPDIALOG_H
#define BACKUPDIALOG_H

#include "textdb.h"
#include "vmnumberformats.h"
#include "ui_backupdialog.h"

#include <QDialog>

class vmNotify;

class QPushButton;
class QRadioButton;
class QLabel;
class QProgressBar;
class QListWidgetItem;

namespace Ui
{
class BackupDialog;
}

class BackupDialog : public QDialog
{

friend class Data;

public:
	virtual ~BackupDialog ();

	void showWindow ();
	static bool doBackup ( const QString& filename, const QString& path, const bool bUserInteraction = false );
    static void doDayBackup ();
	bool doExport ( const QString& prefix, const QString& path, const bool bUserInteraction = false );
	bool doRestore ( const QString& filepath );

	static void addToRestoreList ( const QString& filepath, BackupDialog* bDlg );
	void readFromBackupList ();
	static void incrementProgress ( BackupDialog* bDlg );

	void showNoDatabaseOptionsWindow ();

	inline bool actionSuccess () const { return mb_success; }

	static inline QString standardDefaultBackupFilename () {
		return QLatin1String ( "vmdb-" ) + vmNumber::currentDate.toDate ( vmNumber::VDF_FILE_DATE ); }

private:
	explicit BackupDialog ();

	static BackupDialog* s_instance;
	friend void deleteBackupDialogInstance ();
	friend BackupDialog* BACKUP ();

	enum AFTER_CLOSE_ACTION
	{
        ACA_RETURN_TO_PREV_WINDOW, ACA_RESTART, ACA_CONTINUE
	};

	Ui::BackupDialog* ui;
	dataFile* tdb;

	bool b_IgnoreItemListChange, mb_success, mb_nodb;
	uint progress_bar_steps;

	QRadioButton* rdImport;
	QRadioButton* rdStartNew;
	QRadioButton* rdNothing;
	QPushButton* btnProceed;
	QDialog* dlgNoDB;
	AFTER_CLOSE_ACTION m_after_close_action;
	vmNotify* backupNotify;

	void fillTable ();
	bool canDoBackup () const;
	bool canDoRestore () const;
	bool getSelectedItems ( QString& selected );

	void initProgressBar ( const uint max );
	static bool checkThatFileDoesNotExist ( const QString& filepath, const bool bUserInteraction );
	static bool checkDir ( const QString& dir );
	static bool checkFile ( const QString& file );
	void addDocuments ( const QString& tar_file );
	void addImages ( const QString& tar_file );

	void setupConnections ();
	void selectAll ( QListWidgetItem* );
	void btnNoDBProceed_clicked ();
	void btnApply_clicked ();
	void btnClose_clicked ();
	void rdChooseKnownFile_toggled ( const bool checked );
	void rdChooseAnotherFile_toggled ( const bool checked );
	void grpBackup_clicked ( const bool checked );
	void grpExportToText_clicked ( const bool checked );

	void btnChooseImportFile_clicked ();
	void btnRemoveFromList_clicked ();
	void btnRemoveAndDelete_clicked ();
	void btnSendMail_clicked ();
};

inline BackupDialog *BACKUP ()
{
	if ( !BackupDialog::s_instance )
		BackupDialog::s_instance = new BackupDialog;
	return BackupDialog::s_instance;
}

#endif // BACKUPDIALOG_H
