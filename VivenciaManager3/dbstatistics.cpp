#include "dbstatistics.h"
#include "texteditwithcompleter.h"
#include "cleanup.h"
#include "heapmanager.h"
#include "client.h"
#include "job.h"
#include "payment.h"
#include "purchases.h"

#include <QVBoxLayout>
#include <QApplication>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QMap>

/*
#include <chrono>
#include <thread>
std::this_thread::sleep_for ( std::chrono::milliseconds ( 1000 ) );
std::this_thread::sleep_for ( std::chrono::seconds ( 1 ) );
*/

#ifdef USE_THREADS
#include <QThread>

#define PROCESS_EVENTS qApp->processEvents ();
#define EMIT_INFO emit infoProcessed
#else
#define PROCESS_EVENTS
#define EMIT_INFO m_readyFunc
#endif

dbStatistics* dbStatistics::s_instance ( nullptr );

static const QString HTML_BOLD_ITALIC_UNDERLINE_11 ( QStringLiteral ( "<span style=\" font-size:11pt;text-decoration:underline;font-style:italic;font-weight:600;\">%1</span>") );
static const QString HTML_BOLD_FONT_11 ( QStringLiteral ( "<span style=\" font-size:11pt;font-weight:600;\">%1</span>" ) );

void deleteDBStatistics ()
{
	heap_del ( dbStatistics::s_instance );
}

dbStatistics::dbStatistics ( QObject* parent )
	: QObject ( parent ), m_textinfo ( new textEditWithCompleter ), mainLayout ( new QVBoxLayout )
{
	m_textinfo->setEditable ( false );
	mainLayout->addWidget ( m_textinfo );
	addPostRoutine ( deleteDBStatistics );
}

dbStatistics::~dbStatistics () {}

void dbStatistics::reload ()
{
	m_textinfo->clear ();
	dbStatisticsWorker* worker ( new dbStatisticsWorker );
	
#ifdef USE_THREADS
	QThread* workerThread ( new QThread );
	worker->moveToThread ( workerThread );
	static_cast<void>( connect ( workerThread, SIGNAL ( started () ), worker, SLOT ( startWorks () ) ) );
	static_cast<void>( connect ( worker, SIGNAL ( finished () ), workerThread, SLOT ( quit () ) ) );
	static_cast<void>( connect ( worker, SIGNAL( finished () ), worker, SLOT ( deleteLater () ) ) );
	static_cast<void>( connect ( workerThread, SIGNAL ( finished () ), workerThread, SLOT ( deleteLater () ) ) );
	static_cast<void>( connect ( worker, SIGNAL ( infoProcessed ( const QString& ) ), this, SLOT ( writeInfo ( const QString& ) ) ) );
	workerThread->start();
	
#else
	worker->setCallbackForInfoReady ( [&] ( const QString& more_info ) { return m_textinfo->append ( more_info ); } );
	worker->startWorks ();
	delete worker;
#endif

}

#ifdef USE_THREADS
void dbStatistics::writeInfo ( const QString &info )
{
	m_textinfo->append ( info );
}
#endif

dbStatisticsWorker::dbStatisticsWorker ()
	: threadedDBOps ()
{
#ifndef USE_THREADS
	m_readyFunc = nullptr;
#endif
}

dbStatisticsWorker::~dbStatisticsWorker () {}

void dbStatisticsWorker::startWorks ()
{
#ifndef USE_THREADS
	if ( !m_readyFunc )
		return;
#endif
	
	countClients ();
	PROCESS_EVENTS	
	clientsPerYear ( true );
	PROCESS_EVENTS
	clientsPerYear ( false );
	PROCESS_EVENTS
	activeClientsPerYear ();
	PROCESS_EVENTS
	clientsPerCity ();
	PROCESS_EVENTS
	
#ifdef USE_THREADS
	emit finished ();
#else
	if ( m_finishedFunc )
		m_finishedFunc ();
#endif
}

void dbStatisticsWorker::countClients ()
{
	EMIT_INFO ( TR_FUNC ( "Total number of clients: " ) + 
				HTML_BOLD_ITALIC_UNDERLINE_11.arg ( VDB ()->recordCount ( &Client::t_info ) ) + QStringLiteral ( "\n" ) );
	
	QSqlQuery queryRes;
	if ( VDB ()->runQuery ( QStringLiteral ( "SELECT COUNT(*) FROM CLIENTS WHERE STATUS='1'" ), queryRes ) )
		EMIT_INFO ( TR_FUNC ( "Total number of active clients: " ) + HTML_BOLD_ITALIC_UNDERLINE_11.arg ( queryRes.value ( 0 ).toString () ) + QStringLiteral ( "\n" ) );
}

void dbStatisticsWorker::clientsPerYear ( const bool b_start )
{
	QSqlQuery queryRes;
	if ( VDB ()->runQuery ( b_start ? QStringLiteral ( "SELECT BEGINDATE FROM CLIENTS" ) : QStringLiteral ( "SELECT ENDDATE FROM CLIENTS" ), queryRes ) )
	{
		vmNumber date, db_date;
		uint year ( 2005 );
		podList<uint> count_years ( 0, vmNumber::currentDate.year () - year + 1 );
		do
		{
			date.setDate ( 1, 1, year = 2005 );
			db_date.fromTrustedStrDate ( queryRes.value ( 0 ).toString (), vmNumber::VDF_DB_DATE, false );
			do {
				if ( date.year () == db_date.year () )
				{
					count_years[year-2005]++;
					break;
				}
				date.setDate ( 1, 1, static_cast<int>( ++year ), false );
			} while ( year <= vmNumber::currentDate.year () );
		} while ( queryRes.next () );
		
		EMIT_INFO ( b_start ? TR_FUNC ( "\nNumber of new clients per year since 2005:\n" ) : TR_FUNC ( "\nNumber of lost clients per year since 2005:\n" ) );
		const QString strtemplate ( b_start ? TR_FUNC ( "\t%1: %2 new additions;" ) : TR_FUNC ( "\t%1: %2 clients lost;" ) );
		year = 2005;
		uint count ( count_years.first () );
		while ( year <= vmNumber::currentDate.year () )
		{
			
			EMIT_INFO ( strtemplate.arg ( HTML_BOLD_FONT_11.arg ( QString::number ( year ) ), HTML_BOLD_ITALIC_UNDERLINE_11.arg ( QString::number ( count ) ) ) );
			count = count_years.next ();
			++year;
		}
	}
}

void dbStatisticsWorker::activeClientsPerYear ()
{
	QSqlQuery queryRes;
	if ( VDB ()->runQuery ( QStringLiteral ( "SELECT BEGINDATE,ENDDATE FROM CLIENTS" ), queryRes ) )
	{
		vmNumber date, start_date, end_date;
		podList<uint> count_clients ( 0, vmNumber::currentDate.year () - 2005 + 1 );
		do
		{
			start_date.fromTrustedStrDate ( queryRes.value ( 0 ).toString (), vmNumber::VDF_DB_DATE, false );
			end_date.fromTrustedStrDate ( queryRes.value ( 1 ).toString (), vmNumber::VDF_DB_DATE, false );
			date = start_date;
			if ( date.year () >= 2005 ) // a corrupt database might yield a result we are not expecting, and the result will be nasty
			{
				do
				{
					count_clients[date.year ()-2005]++;
					date.setDate ( 31, 12, static_cast<int>( date.year () + 1 ) );				
				} while ( date.year () <= end_date.year () );
			}
		} while ( queryRes.next () );
		
		EMIT_INFO ( TR_FUNC ( "\nNumber of active clients per year since 2005:\n" ) );
		const QString strtemplate ( TR_FUNC ( "\t%1: %2;" ) );
		uint year ( 2005 );
		uint count ( count_clients.first () );
		while ( year <= vmNumber::currentDate.year () )
		{
			
			EMIT_INFO ( strtemplate.arg ( HTML_BOLD_FONT_11.arg ( QString::number ( year ) ), HTML_BOLD_ITALIC_UNDERLINE_11.arg ( QString::number ( count ) ) ) );
			count = count_clients.next ();
			++year;
		}
	}
}

void dbStatisticsWorker::clientsPerCity ()
{
	QSqlQuery queryRes;
	if ( VDB ()->runQuery ( QStringLiteral ( "SELECT CITY FROM CLIENTS" ), queryRes ) )
	{
		QMap<QString,uint> city_count;
		QString city;
		uint count ( 0 );
		do
		{
			city = queryRes.value ( 0 ).toString ();
			QMap<QString, uint>::const_iterator i = city_count.find ( city );
			if ( i != city_count.end () )
			{
				count = i.value ();
			}
			else
				count = 0;
			++count;
			city_count.insert ( city, count );
		} while ( queryRes.next () );
		
		EMIT_INFO ( TR_FUNC ( "\nNumber of clients per city:\n" ) );
		QMap<QString,uint>::const_iterator itr ( city_count.constBegin () );
		QMap<QString,uint>::const_iterator itr_end ( city_count.constEnd () );
		const QString strtemplate ( TR_FUNC ( "\t%1: %2;" ) );
		while ( itr != itr_end )
		{
			EMIT_INFO ( strtemplate.arg ( HTML_BOLD_FONT_11.arg ( itr.key () ), HTML_BOLD_ITALIC_UNDERLINE_11.arg ( itr.value () ) ) );
			++itr;
		}
	}
}
