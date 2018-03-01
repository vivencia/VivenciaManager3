#ifndef DBSTATISTICS_H
#define DBSTATISTICS_H

#include <QtCore/QObject>

#include <functional>

class textEditWithCompleter;
class QVBoxLayout;

class dbStatistics : public QObject
{

#ifdef USE_THREADS
Q_OBJECT
#endif

friend class VivenciaDB;

public:
	explicit dbStatistics ( QObject* parent = nullptr );
	virtual ~dbStatistics ();

	inline QVBoxLayout* layout () const { return mainLayout; }
	void reload ();

#ifdef USE_THREADS
public slots:
	void writeInfo ( const QString& );
#endif

private:	
	textEditWithCompleter* m_textinfo;
	QVBoxLayout* mainLayout;
};

class dbStatisticsWorker : public QObject
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
	void countJobs ();
	void jobPrices ();
	void biggestJobs ();
	void countPayments ();
	
#ifndef USE_THREADS
	std::function<void( const QString& )> m_readyFunc;
#endif
};

#endif // DBSTATISTICS_H
