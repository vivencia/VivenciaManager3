#ifndef VMWIDGET_H
#define VMWIDGET_H

#include "global.h"
#include "vmlist.h"
#include "global_enums.h"
#include "stringrecord.h"

#include <functional>

class vmTableItem;

class QKeyEvent;
class QVBoxLayout;

enum PREDEFINED_WIDGET_TYPES {
	WT_WIDGET_ERROR = -1, WT_WIDGET_UNKNOWN = 0,
	WT_LINEEDIT = 2<<0, WT_TEXTEDIT = 2<<1, WT_LABEL = 2<<2, WT_COMBO = 2<<3, WT_DATEEDIT = 2<<4,
	WT_TIMEEDIT = 2<<5, WT_TABLE = 2<<6, WT_LED = 2<<7, WT_LISTITEM = 2<<8, WT_ACTION = 2<<9,
	WT_LISTWIDGET = 2<<10, WT_BUTTON = 2<<11, WT_CHECKBOX = 2<<12, WT_TABLE_ITEM = 2<<13,
	WT_LINEEDIT_WITH_BUTTON = 2<<14, WT_QWIDGET = 2<<15

};

class vmWidget
{

public:
	enum TEXT_TYPE { TT_TEXT = 0x1, TT_UPPERCASE = 0x2, TT_PHONE = 0x4, TT_NUMBER_PLUS_SYMBOL = 0x8,
					 TT_PRICE = 0x10, TT_INTEGER = 0x20, TT_DOUBLE = 0x40
				   };

	vmWidget ( const int type, const int subtype = -1, const int id = -1 );
	//vmWidget ( QWidget* widget );
	virtual ~vmWidget ();

	inline void setVmParent ( vmWidget* const parent ) { mParent = parent; }
	inline vmWidget* vmParent () const { return mParent; }

	inline void setWidgetPtr ( QWidget* ptr ) { mWidgetPtr = ptr; }
	inline QWidget* toQWidget () const { return mWidgetPtr; }
	inline virtual void setText ( const QString& = QString::null, const bool = false ) {}
	inline virtual QString text () const { return emptyString; }
	inline virtual QLatin1String qtClassName () const { return QLatin1String ( "" ); }
	inline virtual QString defaultStyleSheet () const { return emptyString; }
	virtual QString widgetToString () const;
	virtual void highlight ( const VMColors vm_color, const QString& str = QString::null );

	// Can be implemented downstream to produce specific widgets with their particularities.
	// On success, a new object is created on the heap and should be managed by the caller.
	// TODO: consider using heapManager to manage the created object.
	virtual vmWidget* stringToWidget ( const QString& str_widget );

	inline virtual void setEditable ( const bool editable ) { setFontAttributes ( mb_editable = editable ); }
	inline bool isEditable () const { return mb_editable; }

	void setFontAttributes ( const bool italic, const bool bold = false );

	void setTextType ( const TEXT_TYPE t_type );
	inline TEXT_TYPE textType () const { return mTextType; }
	inline int type () const { return m_type; }
	inline int subType () const { return m_subtype; }
	inline void setSubType ( const int subtype ) { m_subtype = subtype; }
	inline int id () const { return m_id; }
	inline void setID ( const int id ) { m_id = id; }

	inline void setCallbackForRelevantKeyPressed
			( const std::function<void ( const QKeyEvent* const, const vmWidget* const )>& func ) { keypressed_func = func; }
	inline virtual void setCallbackForContentsAltered ( const std::function<void ( const vmWidget* const )>& func ) { contentsAltered_func = func; }

	// assumes a derivative of a real QWidget
	void setCallbackForContextMenu ( const std::function<void ( const QPoint& pos, const vmWidget* const )>& func );
	
	void showContextMenu ( const QPoint& pos );

	inline const vmTableItem* ownerItem () const { return m_sheetItem; }
	void setOwnerItem ( vmTableItem* const item );

	/* When data is a vmNumber, its internal data structure, privateData, must have the number member created and operational.
	 * For that, call that number's copyNumber method ( a member of vmBaseType ) with itself as argument before calling this method
	 * Because syncing is not automatic, any change to the derived class must be followed by a call to copyNumber if the number is to be used again
	 */
	inline void setInternalData ( const QVariant& data ) { m_data = data; }
	inline const QVariant& internalData () const { return m_data; }

	inline void setUtilitiesPlaceLayout ( QVBoxLayout* layoutUtilities ) { m_LayoutUtilities = layoutUtilities; }
	inline QVBoxLayout* utilitiesLayout () { return m_LayoutUtilities; }
	inline int addUtilityPanel ( QWidget* panel ) { return m_UtilityWidgetsList.append ( panel ); }
	inline QWidget* utilityPanel ( const int widget_idx ) const { return m_UtilityWidgetsList.at ( widget_idx ); }
	bool toggleUtilityPanel ( const int widget_idx );

protected:
	std::function<void ( const QKeyEvent* const ke, const vmWidget* const widget )> keypressed_func;
	std::function<void ( const QPoint& pos, const vmWidget* const vm_widget )> contextmenu_func;
	std::function<void ( const vmWidget* const )> contentsAltered_func;

private:
	int m_type;
	int m_subtype;
	int m_id;
	bool mb_editable;
	QVariant m_data;

	QWidget* mWidgetPtr;
	vmWidget* mParent;
	vmTableItem* m_sheetItem;
	QVBoxLayout* m_LayoutUtilities;
	PointersList<QWidget*> m_UtilityWidgetsList;
	TEXT_TYPE mTextType;
};

#endif // VMWIDGET_H
