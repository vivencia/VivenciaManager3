#ifndef VMTABLEITEM_H
#define VMTABLEITEM_H

#include "vmwidget.h"
#include "vmwidgets.h"
#include "completers.h"
#include "global_enums.h"
#include "stringrecord.h"

#include <QTableWidgetItem>
#include <QVariant>
#include <QString>

class vmTableWidget;

class vmTableItem : public QTableWidgetItem, public vmWidget
{

friend class vmTableWidget;

public:
	explicit vmTableItem ( const PREDEFINED_WIDGET_TYPES wtype,
						   const vmLineEdit::TEXT_TYPE ttype,
						   const QString& text, const vmTableWidget* table );

	~vmTableItem ();
	void setEditable ( const bool editable );

	inline vmWidget* widget () const {
		return m_widget;
	}
	inline void setWidget ( vmWidget* widget ) {
		m_widget = widget;
	}

	void highlight ( const VMColors color, const QString& str = QString::null );

	QVariant data ( const int role ) const;

	inline QString text () const {
		return mCache.toString ();
	}
	inline QString prevText () const {
		return mprev_datacache.toString ();
	}
	//inline void setOriginalText ( const QString& text ) { mBackupData_cache = text; }
	inline QString originalText () const {
		return mBackupData_cache.toString ();
	}
	inline void setCellIsAltered ( const bool altered ) {
		mb_CellAltered = altered;
	}
	inline bool cellIsAltered () const {
		return mb_CellAltered;
	}
	inline void syncOriginalTextWithCurrent () {
		mBackupData_cache = mCache;
		mb_CellAltered = false;
	}

	inline void setText ( const QString& text, const bool force_notify = false ) {
		return setText ( text, false, force_notify );	// avoid compiler (clang) error
	}
	void setText ( const QString& text, const bool b_from_cell_itself = false,
				   const bool force_notify = false, const bool b_formulaResult = false );
	inline void setTextToDefault ( const bool force_notify = false ) {
		setText ( mDefaultValue, false, force_notify );
	}

	void setDate ( const vmNumber& date );
	vmNumber date ( const bool bCurText = true ) const;
	vmNumber number ( const bool bCurText = true ) const;

	inline bool hasFormula () const { return mb_hasFormula; }
	inline bool formulaOverride () const { return mb_formulaOverride; }
	inline void setFormulaOverride ( const bool b_override ) { mb_formulaOverride = b_override; }
	inline const QString& formula () const { return mStr_Formula; }
	inline const QString& formulaTemplate () const { return mStr_FormulaTemplate; }

	void targetsFromFormula ();
	void setFormula ( const QString& formula_template, const QString& firstValue, const QString& secondValue = QString::null );
	bool setCustomFormula ( const QString& strFormula );
	void computeFormula ();

	inline void setDefaultValue ( const QString& text ) { mDefaultValue = text; }
	inline const QString& defaultValue () const { return mDefaultValue; }

	inline void setCompleterType ( vmCompleters::COMPLETER_CATEGORIES completer_type ) {
		mcompleter_type = completer_type;
	}
	inline vmCompleters::COMPLETER_CATEGORIES completerType () const {
		return mcompleter_type;
	}

	inline void setWidgetType ( const PREDEFINED_WIDGET_TYPES wtype ) { m_wtype = wtype; }
	inline PREDEFINED_WIDGET_TYPES widgetType () const { return m_wtype; }

	inline void setTextType ( const vmWidget::TEXT_TYPE ttype ) { m_texttype = ttype; }
	inline vmWidget::TEXT_TYPE textType () const { return m_texttype; }

	inline void setButtonType ( const vmLineEditWithButton::LINE_EDIT_BUTTON_TYPE btype ) { m_btype = btype; }
	inline vmLineEditWithButton::LINE_EDIT_BUTTON_TYPE buttonType () const { return m_btype; }

	QString spreadItemToString () const;

private:
	PREDEFINED_WIDGET_TYPES m_wtype;
	vmWidget::TEXT_TYPE m_texttype;
	vmLineEditWithButton::LINE_EDIT_BUTTON_TYPE m_btype;
	vmCompleters::COMPLETER_CATEGORIES mcompleter_type;

	bool mb_hasFormula, mb_formulaOverride, mb_customFormula;
	bool mb_CellAltered;

	QString mStr_Formula, mStr_FormulaTemplate, mStrOp, mDefaultValue;
	QVariant mCache, mprev_datacache, mBackupData_cache;

	vmTableWidget* m_table;
	PointersList<vmTableItem*> m_targets;
	vmWidget* m_widget;
};

#endif // VMTABLEITEM_H
