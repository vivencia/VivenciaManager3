#ifndef USERMANAGEMENTUI_H
#define USERMANAGEMENTUI_H

#include "usermanagement.h"

#include <QDialog>

class vmLineEdit;
class vmCheckBox;
class vmTableWidget;

struct USER_INFO;

class QToolButton;
class QGroupBox;
class QLabel;

class userManagementUI : public QDialog
{

public:
	explicit userManagementUI ( QWidget* parent = nullptr );
	virtual ~userManagementUI ();

	void showWindow ( const QString& username = QString::null, const bool modal = false );

private:
	void btnInsertUser_clicked ( const bool checked );
	void btnEditUser_clicked ( const bool checked );
	void btnRemoveUser_clicked ();
	void btnClose_clicked ();
	void reloadUserInfo ();
	void saveUserInfo ();

	UserManagement::USER_INFO* mUserInfo_st;
	QToolButton* btnInsertUser;
	QToolButton* btnEditUser;
	QToolButton* btnRemoveUser;
	vmLineEdit* txtEditUserName;
	vmLineEdit* txtEditPassword;
	vmCheckBox* chkCanExport;
	vmCheckBox* chkCanImport;
	vmTableWidget* tablesView;
	QPushButton* btnClose;
	QLabel* lblStatusBar;
};

#endif // USERMANAGEMENTUI_H
