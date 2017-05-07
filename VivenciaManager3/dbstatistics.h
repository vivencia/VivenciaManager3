#ifndef DBSTATISTICS_H
#define DBSTATISTICS_H

#include "vivenciadb.h"

#include <QObject>

class textEditWithCompleter;
class QVBoxLayout;

class dbStatistics : public QObject
{

#ifdef USE_THREADS
Q_OBJECT
#endif

friend class VivenciaDB;

public:
	virtual ~dbStatistics ();

	inline QVBoxLayout* layout () const { return mainLayout; }
	void reload ();

#ifdef USE_THREADS
public slots:
	void writeInfo ( const QString& );
#endif

private:
	explicit dbStatistics ( QObject* parent = nullptr );
	friend void deleteDBStatistics ();
	friend dbStatistics* DB_STATISTICS ();
	
	static dbStatistics* s_instance;
	
	textEditWithCompleter* m_textinfo;
	QVBoxLayout* mainLayout;
};

inline dbStatistics* DB_STATISTICS ()
{
	if ( !dbStatistics::s_instance )
		dbStatistics::s_instance = new dbStatistics ( nullptr );
	return dbStatistics::s_instance;
}

class dbStatisticsWorker : public threadedDBOps
{

#ifdef USE_THREADS
Q_OBJECT
#endif

public:
	explicit dbStatisticsWorker ();
	virtual ~dbStatisticsWorker ();
	
#ifdef USE_THREADS
public slots:
	void startWorks ();
	
signals:
	void infoProcessed ( const QString& );
	void finished ();

#else
	void startWorks ();
	void setCallbackForInfoReady ( const std::function<void( const QString& )>& func ) { m_readyFunc = func; }
#endif
	
private:
	void countClients ();
	void clientsPerYear ( const bool b_start );
	void activeClientsPerYear ();
	void clientsPerCity ();
	
#ifndef USE_THREADS
	std::function<void( const QString& )> m_readyFunc;
#endif
};

#endif // DBSTATISTICS_H