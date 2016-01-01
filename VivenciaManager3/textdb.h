#ifndef textFile_H
#define textFile_H

#include "vmlist.h"
#include "stringrecord.h"

#include <QFile>

//--------------------------------------------TEXT-FILE--------------------------------
class textFile
{

public:

	enum TF_TYPE { TF_TEXT, TF_CONFIG, TF_DATA };

	explicit textFile ();
	textFile ( const QString& filename );
	virtual ~textFile ();

	inline void setName ( const QString& filename ) { m_filename = filename; }

	static bool isTextFile ( const QString& filename, const TF_TYPE type );
	void remove ();

	inline bool isOpen () const { return m_open; }

	void clear ();
	bool open ();
	bool load ();
	void commit ();

	void setText ( const QString& new_file_text );
	inline QString text () const { return data; }

protected:
	void readType ();
	bool overwrite ();
	void writeHeader ();

	virtual bool loadData ();
	virtual bool writeData();
	virtual void clearData ();

	QFile m_file;
	TF_TYPE m_type;
	bool m_open;
	bool m_needsaving;
	uint m_buffersize;
	QString m_filename;
	QString data;
};
//--------------------------------------------TEXT-FILE--------------------------------

//--------------------------------------------CONFIG-FILE--------------------------------
class configFile : public textFile
{

public:
	explicit configFile ();
	configFile ( const QString& filename );
	~configFile ();

	bool setWorkingSection ( const QString& section_name );
	bool setWorkingSection ( const uint section_pos );
	const QString& fieldValue ( const QString& field_name ) const;
	const QString& fieldValue ( const uint field_index ) const;
	int fieldIndex ( const QString& field_name ) const;

	void insertNewSection ( const QString& section_name, const bool b_makecurrent = true );
	void deleteSection ( const QString& section_name );
	void insertField ( const QString& field_name, const QString& value );
	void deleteField ( const QString& field_name );

	bool setFieldValue ( const QString& field_name, const QString& value );
	bool setFieldValue ( const uint field_index, const QString& value );

	inline int sectionCount () const { return cfgData.count (); }

protected:
	bool loadData();
	bool writeData ();
	void clearData ();

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
	~dataFile ();

	void insertRecord ( const int pos, const stringRecord& rec );
	void changeRecord ( const int pos, const stringRecord& rec );
	void deleteRecord ( const int pos );
	void appendRecord ( const stringRecord& rec );
	bool getRecord ( stringRecord& rec, const int pos ) const;
	bool getRecord ( stringRecord& rec, const QString& value, const int field = -1 ) const;

	inline uint recCount () const { return recData.countRecords (); }

protected:
	void clearData ();
	bool loadData();
	bool writeData ();

private:
	stringTable recData;
};
//--------------------------------------------DATA-FILE--------------------------------

#endif // textFile_H
