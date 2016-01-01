#ifndef BUGETSDIALOG_H
#define BUGETSDIALOG_H

#include "global.h"
#include "mainwindow.h"
#include "fileops.h"

#include <QDialog>
#include <QStringList>

class QTreeWidgetItem;
class QAction;

class estimateDlg : public QDialog
{

public:
	virtual ~estimateDlg ();
	void showWindow ( const QString& client_name );

private:
	void convertToProject ( QTreeWidgetItem* item );
	void projectActions ( QAction* action );
    jobListItem* findJobByPath ( QTreeWidgetItem* const item );
    void changeJobData ( jobListItem* const jobItem, const QString& strProjectPath, const QString& strProjectID);
	void estimateActions ( QAction* action );
    void reportActions ( QAction* action );
	void createReport ( QAction* action );
	uint actionIndex ( const QString& filename ) const;
	void updateButtons ();
	void openWithDoubleClick ( QTreeWidgetItem* item );
	void scanDir ();
	void setupActions ();
	void addToTree ( PointersList<fileOps::st_fileInfo *> &files, const QString& c_name, const int newItemType = -1 );
	void removeFiles ( QTreeWidgetItem* item, const bool bSingleFile = false, const bool bAsk = false );
    bool removeDir ( QTreeWidgetItem* item, const bool bAsk = false );
    bool renameDir ( QTreeWidgetItem* item, QString& strNewName );
    void addFilesToDir ( const bool bAddDoc, const bool bAddXls, const QString& projectpath, const QString &projectid, PointersList<fileOps::st_fileInfo*>& files );
	void execAction ( const QTreeWidgetItem* item, const int action_id );

	estimateDlg ( QWidget* parent );
	static estimateDlg* s_instance;
	friend estimateDlg* ESTIMATE ();
	friend void deleteEstimateInstance ();
};

inline estimateDlg* ESTIMATE ()
{
	// QDockWidgets do not need to hook up to qAddPostRoutine () because they are added to MainWindow's stack of widgets and are deleted when MainWindow is deleted
	if ( !estimateDlg::s_instance )
		estimateDlg::s_instance = new estimateDlg ( globalMainWindow );
	return estimateDlg::s_instance;
}

#endif // BUGETSDIALOG_H
