#ifndef CONTACTSMANAGERWIDGET_H
#define CONTACTSMANAGERWIDGET_H

#include "vmwidget.h"
#include "stringrecord.h"

#include <QWidget>

class vmComboBox;
class vmLineEdit;
class QToolButton;

class contactsManagerWidget : public QWidget, public vmWidget
{

public:
	enum CMW_TYPE { CMW_PHONES = 0, CMW_EMAIL = 1 };
	contactsManagerWidget ( QWidget* parent, const CMW_TYPE type = CMW_PHONES );
	virtual ~contactsManagerWidget ();

	inline void setContactType ( const CMW_TYPE type ) { m_contact_type = type; }
	void initInterface ();
	void setEditable ( const bool editable );

	void decodePhones ( const stringRecord& phones );
	void decodeEmails ( const stringRecord& emails );
	void insertItem ();
	bool removeCurrent ( int& removed_idx );

	void clearAll ();

	inline vmComboBox* combo () const { return cboInfoData; }

	inline void setCallbackForInsertion ( std::function<void ( const QString&, const vmWidget* const )> func ) {
		insertFunc = func; }
	inline void setCallbackForRemoval ( std::function<void ( const int idx, const vmWidget* const )> func ) {
		removeFunc = func; }

private:
	vmComboBox* cboInfoData;
	QToolButton* btnAdd, *btnDel, *btnExtra;

	CMW_TYPE m_contact_type;

	void cbo_textAltered ( const QString& text );
	void keyPressedSelector ( const QKeyEvent* const ke, const vmWidget* const );
	void btnAdd_clicked ( const bool checked );
	void btnDel_clicked ();
	void btnExtra_clicked ();

	std::function<void ( const QString& info, const vmWidget* const widget )> insertFunc;
	std::function<void ( const int idx, const vmWidget* const widget )> removeFunc;
};

#endif // CONTACTSMANAGERWIDGET_H
