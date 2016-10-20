#ifndef HTMLDB_H
#define HTMLDB_H

#include <QObject>

class htmlDB
{
public:
	static inline void initHtmlDB ()
	{
		if ( htmlDB::s_instance == nullptr )
			htmlDB::s_instance = new htmlDB ();
	}

	~htmlDB ();

private:
	htmlDB ();
	void checkInit ();
	
	static htmlDB* s_instance;
	friend htmlDB* HTML_DB ();
	friend void deletehtmlDB_instance ();
};

inline htmlDB* HTML_DB ()
{
	return htmlDB::s_instance;
}

#endif // HTMLDB_H