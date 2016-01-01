#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>

class emailConfigDialog;

namespace Ui
{
class configDialog;
}

class configDialog : public QDialog
{
	Q_OBJECT

public:
	explicit configDialog ( QWidget* parent = nullptr );
	virtual ~configDialog ();

	void fillForms ();

private slots:
	void on_btnCfgChooseConfigFile_clicked ();
	void on_btnCfgUseDefaultConfigFile_clicked ();
	void on_btnCfgChooseDataFolder_clicked ();
	void on_btnCfgUseDefaultDataFolder_clicked ();
	void on_btnCfgChooseFileManager_clicked ();
	void on_btnCfgUseDefaultFileManager_clicked ();
	void on_btnCfgChoosePictureViewer_clicked ();
	void on_btnCfgUseDefaultPictureViewer_clicked ();
	void on_btnCfgChoosePictureEditor_clicked ();
	void on_btnCfgUseDefaultPictureEditor_clicked ();
	void on_btnCfgChooseEMailClient_clicked ();
	void on_btnCfgChooseDocViewer_clicked ();
	void on_btnCfgUseDefaultDocumentViewer_clicked ();
	void on_btnCfgChooseDocEditor_clicked ();
	void on_btnCfgUseDefaultDocEditor_clicked ();
	void on_btnCfgChooseXlsEditor_clicked ();
	void on_btnCfgUseDefaultXlsEditor_clicked ();
	void on_btnCfgChooseBaseDir_clicked ();
	void on_btnCfgUseDefaultBaseDir_clicked ();
	void on_btnCfgUseDefaultESTIMATEDir_clicked ();
	void on_txtCfgConfigFile_editingFinished ();
	void on_txtCfgDataFolder_editingFinished ();
	void on_txtCfgFileManager_editingFinished ();
	void on_txtCfgPictureViewer_editingFinished ();
	void on_txtCfgPictureEditor_editingFinished ();
	void on_txtCfgDocumentViewer_editingFinished ();
	void on_txtCfgDocEditor_editingFinished ();
	void on_txtCfgXlsEditor_editingFinished ();
	void on_txtCfgJobsPrefix_editingFinished ();
	void on_txtCfgESTIMATE_editingFinished ();
	void on_txtCfgReports_editingFinished ();
	void on_btnCfgUseDefaultReportsDir_clicked ();

private:
	Ui::configDialog* ui;
};

#endif // CONFIGDIALOG_H
