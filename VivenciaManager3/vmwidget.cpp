#include "vmwidget.h"
#include "stringrecord.h"
#include "texteditwithcompleter.h"
#include "vmtablewidget.h"
#include "vmtableitem.h"
#include "data.h"

vmWidget::vmWidget ( const int type, const int subtype, const int id )
	: keypressed_func ( nullptr ), contextmenu_func ( nullptr ), contentsAltered_func ( nullptr ),
	  m_type ( type ), m_subtype ( subtype ), m_id ( id ), mb_editable ( false ),
	  mWidgetPtr ( nullptr ), mParent ( nullptr ), m_sheetItem ( nullptr ), mTextType ( TT_TEXT )
{}

/*vmWidget::vmWidget ( QWidget* widget )
	: keypressed_func ( nullptr ), contextmenu_func ( nullptr ), contentsAltered_func ( nullptr ),
	  m_type ( WT_QWIDGET ), m_subtype ( -1 ), m_id ( -1 ), mb_editable ( false ),
	  mWidgetPtr ( widget ), mParent ( nullptr ), m_sheetItem ( nullptr ), mTextType ( TT_TEXT )
{}*/

vmWidget::~vmWidget () {}

static const QString strWidgetPrefix ( QStringLiteral ( "##%%" ) );

QString vmWidget::widgetToString () const
{
	stringRecord strWidget;
	strWidget.fastAppendValue ( strWidgetPrefix );
	strWidget.fastAppendValue ( QString::number ( type () ) );
	strWidget.fastAppendValue ( QString::number ( subType () ) );
	strWidget.fastAppendValue ( QString::number ( id () ) );
	return strWidget.toString ();
}

void vmWidget::highlight ( const VMColors vm_color, const QString& )
{
	toQWidget ()->setStyleSheet ( vm_color == vmDefault_Color ? defaultStyleSheet () :
			qtClassName () + QLatin1String ( " { background-color: rgb(" ) + colorsStr[Data::vmColorIndex (vm_color)] + QLatin1String ( " ) }" ) );
}

vmWidget* vmWidget::stringToWidget ( const QString& /*str_widget*/ )
{
	vmWidget* widget ( nullptr );
	/*vmWidget::vmWidgetStructure widget_st ( stringToWidgetSt ( str_widget ) );
	if ( widget_st.type > WT_WIDGET_UNKNOWN ) {
		widget = new vmWidget ( 0 );
		widget->m_type = widget_st.type;
		widget->m_subtype = widget_st.subtype;
		widget->m_id = widget_st.id;
	}*/
	return widget;
}

void vmWidget::setFontAttributes ( const bool italic, const bool bold )
{
	if ( toQWidget () ) {
		QFont fnt ( toQWidget ()->font () );
		fnt.setItalic ( italic );
		fnt.setBold ( bold );
		toQWidget ()->setFont ( fnt );
	}
}

void vmWidget::setTextType ( const TEXT_TYPE t_type )
{
	if ( t_type != mTextType ) {
		mTextType = t_type;
		QWidget* widget ( toQWidget () );
		if ( widget ) {
			vmLineEdit* line ( nullptr );
			switch ( type () ) {
				case WT_LINEEDIT:
					line = static_cast<vmLineEdit*> ( widget );
				break;
				case WT_LINEEDIT_WITH_BUTTON:
					line = static_cast<vmLineEditWithButton*> ( widget )->lineControl ();
				break;
				case WT_COMBO:
					line = static_cast<vmComboBox*> ( widget )->editor ();
				break;
				case WT_TEXTEDIT:
				break;
				default:
				return;
			}

			QValidator* qval ( nullptr );
			Qt::InputMethodHints imh ( Qt::ImhNone );
			switch ( t_type ) {
				case TT_PHONE:
					qval = new QDoubleValidator ( 0.0, 99999999999.0, 0 );
					imh = Qt::ImhDigitsOnly;
				break;
				case TT_NUMBER_PLUS_SYMBOL:
					imh = Qt::ImhFormattedNumbersOnly;
				break;
				case TT_PRICE:
					qval = new QDoubleValidator ( -99.999, 99.999, 2 );
					imh = Qt::ImhFormattedNumbersOnly;
				break;
				case TT_INTEGER:
					qval = new QIntValidator ( INT_MIN, INT_MAX );
					imh = Qt::ImhDigitsOnly;
				break;
				case TT_DOUBLE:
					qval = new QDoubleValidator ( 0.0, 9.999, 2 );
					imh = Qt::ImhFormattedNumbersOnly;
				break;
				case TT_UPPERCASE:
				case TT_TEXT:
				break;
			}
			if ( line ) {
				line->mTextType = this->mTextType;
				line->setValidator ( qval );
			}
			widget->setInputMethodHints ( imh );
		}
	}
}

void vmWidget::setCallbackForContextMenu
( std::function<void ( const QPoint& pos, const vmWidget* const )> func )
{
	toQWidget ()->setContextMenuPolicy ( Qt::CustomContextMenu );
	contextmenu_func = func;
}

void vmWidget::showContextMenu ( const QPoint& pos )
{
	if ( contextmenu_func )
		contextmenu_func ( pos, this );
}
