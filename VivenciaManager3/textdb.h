#ifndef textFile_H
#define textFile_H

#include "vmlist.h"
#include "stringrecord.h"
#include "tristatetype.h"

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
	inline const QString& filename () { return m_filename; }

	static bool isTextFile ( const QString& filename, const TF_TYPE type );
	void remove ();

	inline bool isOpen () const { return m_open; }
	inline bool isEmpty () const { return m_file.size () <= 0; }

	void clear ();
	bool open ();
	triStateType load ();
	void commit ();

	void setText ( const QString& new_file_text );
	inline QString text () const { return m_data; }

protected:
	void readType ();
	bool overwrite ();
	void writeHeader ();

	virtual bool loadData ();
	virtual bool writeData();
	virtual void clearData ();

	bool m_open;
	bool m_needsaving;
	int64_t m_headerSize;
	uint m_buffersize;
	TF_TYPE m_type;
	QString m_filename;
	QString m_data;
	QFile m_file;
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

	inline uint sectionCount () const { return cfgData.count (); }

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

private:
	stringTable recData;
};
//--------------------------------------------DATA-FILE--------------------------------

#endif // textFile_H
