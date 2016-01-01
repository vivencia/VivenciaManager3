#include "actionpanelscheme.h"
#include "global.h"

const char* const ActionPanelNoStyle ( "" );

const char* const ActionPanelDefaultStyle (

	"QFrame[class='panel'] {"
	"background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 #99cccc, stop: 1 #EAF7FF);"
	"}"

	"vmActionGroup QFrame[class='header'] {"
	"background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #F9FDFF, stop: 1 #EAF7FF);"
	"border: 1px solid #00aa99;"
	"border-bottom: 1px solid #99cccc;"
	"border-top-left-radius: 3px;"
	"border-top-right-radius: 3px;"
	"}"

	"vmActionGroup QFrame[class='header']:hover {"
	"background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #EAF7FF, stop: 1 #F9FDFF);"
	"}"

	"vmActionGroup QToolButton[class='header'] {"
	"text-align: left;"
	"font: 14px;"
	"color: #006600;"
	"background-color: transparent;"
	"border: 1px solid transparent;"
	"}"

	"vmActionGroup QToolButton[class='header']:hover {"
	"color: #00cc00;"
	"text-decoration: underline;"
	"}"

	"vmActionGroup QFrame[class='content'] {"
	"background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #F9FDFF, stop: 1 #EAF7FF);"
	"border: 1px solid #00aa99;"
	"}"

	"vmActionGroup QFrame[class='content'][header='true'] {"
	"border-top: none;"
	"}"

	"vmActionGroup QToolButton[class='action'] {"
	"background-color: transparent;"
	"border: 1px solid transparent;"
	"color: #0033ff;"
	"text-align: left;"
	"font: 11px;"
	"}"

	"vmActionGroup QToolButton[class='action']:!enabled {"
	"color: #999999;"
	"}"

	"vmActionGroup QToolButton[class='action']:hover {"
	"color: #0099ff;"
	"text-decoration: underline;"
	"}"

	"vmActionGroup QToolButton[class='action']:focus {"
	"border: 1px dotted black;"
	"}"

	"vmActionGroup QToolButton[class='action']:on {"
	"background-color: #ddeeff;"
	"color: #006600;"
	"}"
);

const char* const ActionPanelVistaStyle (

	"QFrame[class='panel'] {"
	"background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 #5C9EEC, stop: 1 #86E78A);"
	"}"

	"vmActionGroup QFrame[class='header'] {"
	"background: transparent;"
	"border: 1px solid transparent;"
	"}"

	"vmActionGroup QFrame[class='header']:hover {"
	"background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 rgba(249,253,255,100), stop: 0.5 rgba(234,247,255,20), stop: 1 rgba(249,253,255,100));"
	"border: 1px solid transparent;"
	"}"

	"vmActionGroup QToolButton[class='header'] {"
	"text-align: left;"
	"color: #ffffff;"
	"background-color: transparent;"
	"border: 1px solid transparent;"
	"font-size: 12px;"
	"}"

	"vmActionGroup QFrame[class='content'] {"
	"background-color: transparent;"
	"color: #ffffff;"
	"}"

	"vmActionGroup QComboBox[class='content'] {"
	"background-color: transparent;"
	"color: #ffffff;"
	"}"

	"vmActionGroup QToolButton[class='action'] {"
	"background-color: transparent;"
	"border: 1px solid transparent;"
	"color: #ffffff;"
	"text-align: left;"
	"}"

	"vmActionGroup QToolButton[class='action']:!enabled {"
	"color: #666666;"
	"}"

	"vmActionGroup QToolButton[class='action']:hover {"
	"color: #DAF2FC;"
	"text-decoration: underline;"
	"}"

	"vmActionGroup QToolButton[class='action']:focus {"
	"border: 1px dotted black;"
	"}"

	"vmActionGroup QToolButton[class='action']:on {"
	"background-color: #ddeeff;"
	"color: #006600;"
	"}"
);

ActionPanelScheme::ActionPanelScheme ( const PanelStyle style )
{
	headerSize = 26;
	headerAnimation = true;

	headerButtonFold = ICON ( "stylepanel/def-Fold.png" );
	headerButtonFoldOver = ICON ( "stylepanel/def-FoldOver.png" );
	headerButtonUnfold = ICON ( "stylepanel/def-Unfold.png" );
	headerButtonUnfoldOver = ICON ( "stylepanel/def-UnfoldOver.png" );
	headerButtonClose = ICON ( "stylepanel/panelClose.png" );
	headerButtonCloseOver = ICON ( "stylepanel/panelCloseOver.png" );
	headerButtonSize = QSize ( 18, 18 );

	groupFoldSteps = 20;
	groupFoldDelay = 15;
	groupFoldEffect = ShrunkFolding;
	groupFoldThaw = true;

	switch ( style ) {
		case PANEL_NONE:
			actionStyle = ActionPanelNoStyle;
		break;
		case PANEL_DEFAULT:
			actionStyle = ActionPanelDefaultStyle;
		break;
		case PANEL_VISTA:
			actionStyle = ActionPanelVistaStyle;
		break;
	}
}

ActionPanelScheme* ActionPanelScheme::noScheme ()
{
	static ActionPanelScheme scheme ( PANEL_NONE );
	return &scheme;
}

ActionPanelScheme* ActionPanelScheme::defaultScheme ()
{
	static ActionPanelScheme scheme ( PANEL_DEFAULT );
	return &scheme;
}

ActionPanelScheme* ActionPanelScheme::vistaScheme ()
{
    static ActionPanelScheme scheme ( PANEL_VISTA );
	return &scheme;
}
