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

class QComboBox;
class QLineEdit;
class QDateEdit;
class QTimeEdit;
class QListWidget;
class QMenu;
class QPoint;

inline int monthInYearToMonthForDataOperations ( const vmNumber& date, uint startyear )
{
	return ( ( date.year () - startyear ) * 12 ) + date.month ();
}

class Data
{

friend class searchUI;
friend class MainWindow;

public:
	~Data ();

	static inline void init ()
	{
		if ( !s_instance )
			s_instance = new Data;
	}

	void startUserInteface ();
	void loadDataIntoMemory ();

	const QString currentClientName () const;

	Client* currentClient () const;
	Job* currentJob () const;
	Payment* currentPay () const;
	Buy* currentBuy () const;

	//--------------------------------------STATIC-HELPER-FUNCTIONS---------------------------------------------
    static bool checkSystem ( const bool bFirstPass = true );
	static void checkSetup ();
	static void checkDatabase ();
	static void restartProgram ();
	static bool isMySQLRunning();
	static QString commandMySQLServer ( const QString& command, const QString& message, const bool only_return_cmd_line = false );
	//--------------------------------------STATIC-HELPER-FUNCTIONS---------------------------------------------

	//--------------------------------------EXTRAS-----------------------------------------------
	static void copyToClipboard ( const QString& str );
	static int insertComboItem ( vmComboBox* cbo, const QString& text );
	static int insertStringListItem ( QStringList& list, const QString& text );
	static QPoint getGlobalWidgetPosition ( const QWidget* widget );
	static void execMenuWithinWidget ( QMenu* menu, const QWidget* widget, const QPoint& mouse_pos );

    static inline void fillClientsNamesList ( QStringList& namesList ) {
        for ( int i ( 0 ); i <= VDB ()->lastDBRecord ( TABLE_CLIENT_ORDER ); ++i )
            insertStringListItem ( namesList, Client::clientName ( i ) );
    }
    static void fillJobTypeList ( QStringList& jobList, const QString& clientid );

	static inline bool isEmailAddress ( const QString& address )
	{
		return address.contains ( QLatin1Char ( '@' ) ) && address.contains ( CHR_DOT );
	}

	static int vmColorIndex ( const VMColors vmcolor )
	{
		int idx ( -1 );
		switch ( vmcolor )
		{
			case vmGray: idx = 0; break;
			case vmRed: idx = 1; break;
			case vmYellow: idx = 2; break;
			case vmGreen: idx = 3; break;
			case vmBlue: idx = 4; break;
			case vmWhite: idx = 5; break;
			default:;
		}
		return idx;
	}

	QComboBox* MainWindowSupplierCombo;

private:
	explicit Data ();
	friend Data *DATA ();
	friend void deleteDataInstance ();

	static Data* s_instance;

	struct st_migrateInfo
	{
		vmListItem* migrateItem;
		clientListItem* clientTo;
	} m_migrateInfo;
};

inline Data *DATA ()
{
	return Data::s_instance;
}

#endif // DATA_H
