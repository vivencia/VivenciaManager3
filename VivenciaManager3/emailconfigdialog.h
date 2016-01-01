#ifndef EMAILCONFIGDIALOG_H
#define EMAILCONFIGDIALOG_H

#include "configops.h"
#include "vmlist.h"

#include <QDialog>

struct emailOpts
{
	QString fancy_name;
	QString app_name;
	QString addrs_switch;
	QString subject_switch;
	QString attach_switch;
	QString body_switch;
	QString switches_sep;
	bool bNewProfile, bProfileModified;
};

class emailConfigDialog : public QDialog
{

public:
	virtual ~emailConfigDialog ();

	void showDlg ();
	void sendEMail ( const QString& addresses, const QString& subject = QString::null,
					 const QString& attachments = QString::null, const QString& bodyfile = QString::null ) const;
	void done ( const int );

private:
	explicit emailConfigDialog ();

	friend emailConfigDialog* EMAIL_CONFIG ();
	friend void deleteEmailConfigInstance ();
	static emailConfigDialog* s_instance;
	friend void initEmailConfig ();

	void setupUi ();
	void saveProfile ();
	void loadProfiles ();
	void changeForms ( const bool editAction );
	void cboProfiles_indexChanged ( const int idx );
	void btnChooseApp_clicked ();
	void btnAddProfile_clicked ();
	void btnEditProfile_clicked ();
	void btnSaveProfile_clicked ();
	void btnCancelProfile_clicked ();
	void btnClose_clicked ();

	const QString configFileName () const;
	int mSavedIndex;
	bool bProfilesLoaded;
	PointersList<emailOpts*> optsList;
};

inline emailConfigDialog* EMAIL_CONFIG ()
{
	if ( !emailConfigDialog::s_instance )
		emailConfigDialog::s_instance = new emailConfigDialog;
	return emailConfigDialog::s_instance;
}

#endif // EMAILCONFIGDIALOG_H
