#ifndef NEWPROJECTDIALOG_H
#define NEWPROJECTDIALOG_H

#include "vmlistitem.h"

#include <QDialog>

class vmWidget;
class vmListWidget;
class vmListItem;
class vmCheckBox;
class vmLineEdit;

class QToolButton;

class newProjectDialog : public QDialog
{

public:
    explicit newProjectDialog ( QWidget* parent = nullptr );
    virtual ~newProjectDialog ();
    void showDialog ( const QString& clientname, const bool b_allow_other_client = false , const bool b_allow_name_change = true );

    inline const QString& projectPath () const { return mProjectPath; }
    inline const QString& projectID () const { return mProjectID; }
    inline jobListItem* jobItem () const { return mJobItem; }
    inline bool resultAccepted () const { return bresult; }

private:
	void loadJobsList ( const int clientid );
    void jobTypeItemSelected ( vmListItem* item );
    void txtProjectNameAltered ( const vmWidget* const );
    void btnChooseExistingDir_clicked ();
    void chkUseDefaultName_checked ();
    void btnOK_clicked ();
    void btnCancel_clicked ();
    void closeCleanUp ();

    vmListWidget* lstJobTypes;
    vmLineEdit* txtProjectName;
	vmComboBox* cboClients;
    vmCheckBox* chkUseDefaultName;
    QPushButton* btnOK, *btnCancel;
    QToolButton* btnChooseExistingDir;
    jobListItem* mJobItem;
    QString mProjectID, mProjectPath;
    clientListItem* mClientItem;
    bool bresult;
};

#endif // NEWPROJECTDIALOG_H
