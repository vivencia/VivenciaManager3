#ifndef SINGLEAPP_H
#define SINGLEAPP_H

#include <QString>
#include <QSharedMemory>

class SingleApp
{
public:
	inline SingleApp( const QString& app )
		: m_QSharedMem ( new QSharedMemory ( app ) ), m_AlreadyRunning ( false )
		{
			if ( m_QSharedMem->create ( 1 ) == false )
				m_AlreadyRunning = true;
		}
	inline ~SingleApp () { delete m_QSharedMem; }
    inline bool IsAlreadyRunning () const { return m_AlreadyRunning; }

private:
	QSharedMemory* m_QSharedMem;
	bool m_AlreadyRunning;
};
#endif // SINGLEAPP_H
