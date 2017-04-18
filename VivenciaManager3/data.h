#ifndef DATA_H
#define DATA_H

#include "client.h"
#include "job.h"
#include "payment.h"
#include "purchases.h"
#include "stringrecord.h"
#include "vivenciadb.h"

#include <QDate>

class vmListItem;
class clientListItem;
class jobListItem;
class schedListItem;
class payListItem;
class buyListItem;
class delegatedItem;
class vmComboBox;
class vmTableWidget;

class QMenu;

namespace Data
{

	extern void init ();
	extern void loadDataIntoMemory ();

	//--------------------------------------STATIC-HELPER-FUNCTIONS---------------------------------------------
	extern bool checkSystem ( const bool bFirstPass = true );
	extern void checkSetup ();
	extern void checkDatabase ();
	extern void restartProgram ();
	extern bool isMySQLRunning();
	extern QString commandMySQLServer ( const QString& command, const QString& message, const bool only_return_cmd_line = false );
	//--------------------------------------STATIC-HELPER-FUNCTIONS---------------------------------------------

	//--------------------------------------EXTRAS-----------------------------------------------
	extern void copyToClipboard ( const QString& str );
	extern int insertComboItem ( vmComboBox* cbo, const QString& text );
	extern int insertStringListItem ( QStringList& list, const QString& text );
	extern QPoint getGlobalWidgetPosition ( const QWidget* widget );
	extern void execMenuWithinWidget ( QMenu* menu, const QWidget* widget, const QPoint& mouse_pos );

	static inline void fillClientsNamesList ( QStringList& namesList ) {
		for ( uint i ( VDB ()->getLowestID ( TABLE_CLIENT_ORDER ) ); i <= VDB ()->getHighestID ( TABLE_CLIENT_ORDER ); ++i )
			insertStringListItem ( namesList, Client::clientName ( i ) );
	}
	extern void fillJobTypeList ( QStringList& jobList, const QString& clientid );

	static inline bool isEmailAddress ( const QString& address )
	{
		return address.contains ( QLatin1Char ( '@' ) ) && address.contains ( CHR_DOT );
	}

	extern int vmColorIndex ( const VMColors vmcolor );
}
#endif // DATA_H
