#ifndef SUPPLIERSDLG_H
#define SUPPLIERSDLG_H

#include "vmlist.h"

#include <QDialog>

class vmWidget;
class vmLineEdit;
class vmComboBox;
class supplierRecord;
class contactsManagerWidget;
class vmListItem;

class QPushButton;

class suppliersDlg : public QDialog
{

friend class VivenciaDB;

public:
	virtual ~suppliersDlg ();

	void displaySupplier ( const QString& supName, const bool b_showdlg );
	static void supplierInfo ( const QString& name, QString& info );
	void hideDialog ();
	void showSearchResult ( vmListItem* item, const bool bshow );

private:
	explicit suppliersDlg ();

	friend suppliersDlg* SUPPLIERS ();
	friend void deleteSuppliersInstance ();
	static suppliersDlg* s_instance;
	supplierRecord* supRec;

	void saveWidget ( vmWidget* widget, const int id );
	void setupUI ();
	void retrieveInfo ();
	void controlForms ();
	void keyPressedSelector ( const QKeyEvent* ke );
	void clearForms ( const bool b_clear_supname = false );
	void txtSupplier_textAltered ( const vmWidget* const sender );
	void contactsAdd ( const QString& info, const vmWidget* const sender );
	void contactsDel ( const int idx, const vmWidget* const sender );
	void btnInsertClicked ( const bool checked );
	void btnEditClicked ( const bool checked );
	void btnCancelClicked ();
	void btnRemoveClicked ();
	void btnCopyToEditorClicked ();
	void btnCopyAllToEditorClicked ();

	bool m_supchanged, m_bEditing;

	PointersList<vmWidget*> widgetList;
	vmLineEdit* txtSupID;
	vmLineEdit* txtSupName;
	vmLineEdit* txtSupStreet;
	vmLineEdit* txtSupNbr;
	vmLineEdit* txtSupDistrict;
	vmLineEdit* txtSupCity;
	vmComboBox* cboSupPhones;
	vmComboBox* cboSupEmail;
	contactsManagerWidget* contactsPhones;
	contactsManagerWidget* contactsEmails;
	QPushButton* btnEdit, *btnInsert, *btnCancel, *btnRemove;
	QPushButton* btnCopyToEditor, *btnCopyAllToEditor;
};

inline suppliersDlg *SUPPLIERS ()
{
	if ( !suppliersDlg::s_instance )
		suppliersDlg::s_instance = new suppliersDlg;
	return suppliersDlg::s_instance;
}

#endif // SUPPLIERSDLG_H
