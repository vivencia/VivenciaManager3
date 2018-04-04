#ifndef textFile_H
#define textFile_H

#include "vmlist.h"
#include "tristatetype.h"

#include <vmStringRecord/stringrecord.h>

#include <QtCore/QFile>

class vmFileMonitor;

//--------------------------------------------TEXT-FILE--------------------------------
class textFile
{

public:

	enum TF_TYPE { TF_TEXT, TF_CONFIG, TF_DATA };

	explicit textFile ();
	textFile ( const QString& filename );
	virtual ~textFile ();

	inline void setFileName ( const QString& filename ) { m_filename = filename; }
	inline const QString& fileName () { return m_filename; }

	static bool isTextFile ( const QString& filename, const TF_TYPE type );
	void remove ();

	inline bool isOpen () const { return m_open; }
	inline bool isEmpty () const { return m_file.size () <= 0; }

	void clear ();
	triStateType load ( const bool b_force );
	void commit ();

	void setText ( const QString& new_file_text );
	inline QString text () const { return m_data; }

	void setIgnoreEvents ( const bool b_ignore );

protected:
	bool open ();
	bool open2 ();
	void readType ();
	bool overwrite ();
	void writeHeader ();
	void setBufferModifiedInfo ( const uint new_size );
	void fileExternallyAltered ( const QString&, const uint event );

	virtual bool loadData ();
	virtual bool writeData();
	virtual void clearData ();
	virtual bool recheckData ( const bool b_userInteraction = false );

	bool m_open;
	bool m_needsaving;
	int64_t m_headerSize;
	uint m_buffersize;
	TF_TYPE m_type;
	QString m_filename;
	QString m_data;
	QFile m_file;
	vmFileMonitor* m_filemonitor;
	bool mb_IgnoreEvents;

	struct sharedResources
	{
		QString filename;
		bool b_inUse;
		uint modified_counter;
		uint counter;
		textFile* modifierInstance;

		sharedResources () : b_inUse ( false ), modified_counter ( 0 ),
								counter ( 0 ), modifierInstance ( nullptr ) {}
	};

	static PointersList<sharedResources*> sharedResList;
	sharedResources* mUsedRes;

	sharedResources* findSharedResource ( const QString& filename );
};
//--------------------------------------------TEXT-FILE--------------------------------

//--------------------------------------------CONFIG-FILE--------------------------------
class configFile : public textFile
{

public:
	explicit configFile ();
	configFile ( const QString& filename );
	virtual ~configFile ();

	bool setWorkingSection ( const QString& section_name );
	const QString& fieldValue ( const QString& field_name ) const;
	int fieldIndex ( const QString& field_name ) const;

	void insertNewSection ( const QString& section_name );
	void deleteSection ( const QString& section_name );
	void insertField ( const QString& field_name, const QString& value );
	void deleteField ( const QString& field_name );

	bool setFieldValue ( const QString& field_name, const QString& value );

	inline uint sectionCount () const { return cfgData.count (); }

protected:
	bool loadData();
	bool writeData ();
	void clearData ();
	bool recheckData ( const bool b_userInteraction = false );

	int findSection ( const QString& section_name ) const;
	bool parseConfigFile ( const bool b_reload = false );

private:
	struct configFile_st;
	PointersList<configFile_st*> cfgData;
};
//--------------------------------------------CONFIG-FILE--------------------------------

//--------------------------------------------DATA-FILE--------------------------------
class dataFile : public textFile
{
public:
	explicit dataFile ();
	dataFile ( const QString& filename );
	virtual ~dataFile ();

	inline void setRecordSeparationChar ( const QChar& table_sep = table_separator, const QChar& rec_sep = record_separator )
	{
		recData.setRecordSeparationChar ( table_sep );
		recData.setRecordFieldSeparationChar ( rec_sep );
	}

	void insertRecord ( const int pos, const stringRecord& rec );
	void changeRecord ( const int pos, const stringRecord& rec );
	bool deleteRecord ( const int pos );
	void appendRecord ( const stringRecord& rec );
	bool getRecord ( stringRecord& rec, const int pos ) const;
	bool getRecord ( stringRecord& rec, const QString& value, const uint field ) const;

	inline uint recCount () const { return recData.countRecords (); }

protected:
	void clearData ();
	bool loadData();
	bool writeData ();
	bool recheckData ( const bool b_userInteraction = false );

private:
	stringTable recData;
};
//--------------------------------------------DATA-FILE--------------------------------

#endif // textFile_H
