#include "textdb.h"
#include "global.h"
#include "fileops.h"
#include "dbrecord.h"

#include <QStringList>

static const QString HEADER_ID ( QStringLiteral ( "#!VMFILE" ) );
static const QString CFG_TYPE_LINE ( QStringLiteral ( "@CFG,%1\n" ) );
static const QString DATA_TYPE_LINE ( QStringLiteral ( "@CSV,%1\n" ) );

//--------------------------------------------TEXT-FILE--------------------------------
textFile::textFile ()
	: m_open ( false ), m_needsaving ( false ), m_headerSize ( 0 ), m_buffersize ( 0 ), m_type ( TF_TEXT )
{}

textFile::textFile ( const QString& filename )
	: m_open ( false ), m_needsaving ( false ), m_headerSize ( 0 ), m_buffersize ( 0 ), m_type ( TF_TEXT ),
	  m_filename ( filename )
{}

textFile::~textFile ()
{
	clear ();
}

bool textFile::isTextFile ( const QString& filename, const TF_TYPE type )
{
	if ( !fileOps::canRead ( filename ).isOn () )
		return false;

	QFile file ( filename );
	if ( !file.open ( QIODevice::ReadOnly ) )
		return false;

	char buf[20] = { '\0' };
	int64_t n_chars ( -1 );
	QString data;
	bool ret ( false );

	n_chars = file.readLine ( buf, sizeof ( buf ) );
	if ( n_chars != -1 )
	{
		data = QString::fromUtf8 ( buf, static_cast<int>(strlen ( buf ) - 1) );
		if ( data.contains ( HEADER_ID ) )
		{
			if ( type == TF_CONFIG && data.contains ( QStringLiteral( "@CFG" ) ) )
				ret = true;
			else if ( type == TF_DATA && data.contains ( QStringLiteral ("@CSV" ) ) )
				ret = true;
		}
		else
			ret = ( type == TF_TEXT );
	}
	return ret;
}

void textFile::remove ()
{
	m_needsaving = false;
	fileOps::removeFile ( m_filename );
	clear ();
}

void textFile::clear ()
{
	if ( m_needsaving )
		commit ();
	clearData ();
	m_buffersize = 0;
	m_file.close ();
	m_open = false;
	m_needsaving = false;
}

bool textFile::open ()
{
	if ( m_file.fileName () != m_filename )
	{
		clear ();
		m_file.setFileName ( m_filename );
	}
	else
	{
		if ( m_file.isOpen () )
			return true;
	}

	const bool b_exists ( fileOps::exists ( m_filename ).isOn () );
	QIODevice::OpenModeFlag openflag ( b_exists ? QIODevice::ReadWrite : QIODevice::WriteOnly );

	if ( b_exists && !fileOps::canWrite ( m_filename ).isOn () )
	{
		openflag = QIODevice::ReadOnly;
	}
	m_open = m_file.open ( openflag|QIODevice::Text );
	return m_open;
}

void textFile::readType ()
{
	if ( m_open )
	{
		char buf[20] = { '\0' };
		QString data;

		m_file.seek ( 0 );

		m_headerSize = m_file.readLine ( buf, sizeof ( buf ) );
		if ( m_headerSize > 0 )
		{
			data = QString::fromUtf8 ( buf, static_cast<int>(strlen ( buf ) - 1) );
			m_type = TF_TEXT;
			if ( data.contains ( HEADER_ID ) )
			{
				if ( data.contains ( QStringLiteral ( "@CFG" ) ) )
					m_type = TF_CONFIG;
				else if ( data.contains ( QStringLiteral ( "@CSV" ) ) )
					m_type = TF_DATA;
				if ( m_type != TF_TEXT )
					m_buffersize = data.midRef ( data.indexOf ( CHR_COMMA ) + 1, 4 ) .toUInt ();
			}
		}
		if ( m_buffersize == 0 )
			m_buffersize = 200;
	}
}

triStateType textFile::load ()
{
	if ( !m_open )
	{
		if ( !open () )
			return TRI_OFF;
	}
	if ( isEmpty () )
		return TRI_UNDEF;
	readType ();
	return ( loadData () ? TRI_ON : TRI_OFF );
}

bool textFile::overwrite ()
{
	m_file.close ();
	fileOps::removeFile ( m_filename );
	m_file.setFileName ( m_filename );
	m_open = m_file.open ( QIODevice::WriteOnly|QIODevice::Text );
	return m_open;
}

void textFile::writeHeader ()
{
	if ( m_type != TF_TEXT )
	{
		QString str;
		str = HEADER_ID + ( m_type == TF_CONFIG ? CFG_TYPE_LINE : DATA_TYPE_LINE )
			  .arg ( QString::number ( m_buffersize ) );
		m_headerSize = qstrlen ( str.toUtf8 ().data () );
		m_file.write ( str.toUtf8 ().data (), m_headerSize );
		m_file.flush ();
	}
}

void textFile::commit ()
{
	if ( !m_needsaving ) return;
	if ( !overwrite () ) return;
	writeHeader ();

	if ( writeData () )
	{
		m_file.flush ();
		m_needsaving = false;
	}
}

void textFile::setText ( const QString& new_file_text )
{
	m_data = new_file_text;
	m_needsaving = true;
}

bool textFile::loadData ()
{
	m_data = m_file.readAll ();
	return ( !m_data.isEmpty () );
}

bool textFile::writeData ()
{
	const QByteArray data ( m_data.toUtf8 () );
	return ( m_file.write ( data.data (), data.count () ) > 0 );
}

void textFile::clearData ()
{
	m_data.clear ();
}
//--------------------------------------------TEXT-FILE--------------------------------

//--------------------------------------------CONFIG-FILE--------------------------------
struct configFile::configFile_st
{
	QString section_name;
	VMList<QString> fields;
	VMList<QString> values;

	configFile_st () : fields ( QString::null, 5 ), values ( QString::null ) {}
};

configFile::configFile ()
	: textFile (), cfgData ( 30 )
{
	m_type = TF_CONFIG;
}

configFile::configFile ( const QString& filename )
	: textFile ( filename ), cfgData ( 30 )
{
	m_type = TF_CONFIG;
}

configFile::~configFile ()
{
	if ( m_needsaving )
		commit ();
	clear ();
}

bool configFile::setWorkingSection ( const QString& section_name )
{
	bool ret ( false );
	for ( uint i ( 0 ); i < cfgData.count (); ++i )
	{
		if ( cfgData.at ( i )->section_name == section_name )
		{
			cfgData.setCurrent ( static_cast<int>(i) );
			ret = true;
			break;
		}
	}
	return ret;
}

bool configFile::setWorkingSection ( const uint section_pos )
{
	bool ret ( false );
	if ( section_pos < cfgData.count () )
	{
		cfgData.setCurrent ( static_cast<int>(section_pos) );
		ret = true;
	}
	return ret;
}

const QString& configFile::fieldValue ( const QString& field_name ) const
{
	const int idx ( fieldIndex ( field_name ) );
	if ( idx != -1 )
		return cfgData.current ()->values.at ( idx );
	return emptyString;
}

const QString& configFile::fieldValue ( const uint field_index ) const
{
	if ( cfgData.currentIndex () != -1 )
	{
		if ( field_index < unsigned ( cfgData.current ()->values.count () ) )
			return cfgData.current ()->values.at ( field_index );
	}
	return emptyString;
}

int configFile::fieldIndex ( const QString& field_name ) const
{
	int ret ( -1 );
	if ( cfgData.currentIndex () != -1 )
	{
		configFile_st* __restrict section_info ( cfgData.current () );
		const uint n_fields ( section_info->fields.count () );
		for ( int i ( 0 ); i < static_cast<int>(n_fields); ++i )
		{
			if ( section_info->fields.at ( i ) == field_name )
			{
				ret = i;
				break;
			}
		}
	}
	return ret;
}

void configFile::insertNewSection ( const QString& section_name, const bool b_makecurrent )
{
	if ( !section_name.isEmpty () )
	{
		configFile_st* section_info ( new configFile_st );
		section_info->section_name = section_name;
		if ( b_makecurrent )
			cfgData.setCurrent ( cfgData.count () );
		cfgData.append ( section_info );
		if ( static_cast<int>(m_buffersize) < section_name.count () )
			m_buffersize = static_cast<uint>(section_name.count () + 3);
		m_needsaving = true;
	}
}

void configFile::deleteSection ( const QString& section_name )
{
	if ( !section_name.isEmpty () )
	{
		for ( uint i ( 0 ); i < cfgData.count (); ++i )
		{
			if ( cfgData.at ( i )->section_name == section_name )
			{
				if ( cfgData.currentIndex () == static_cast<int>(i) )
					cfgData.setCurrent ( -1 );
				cfgData.remove ( static_cast<int>(i), true );
				m_needsaving = true;
				break;
			}
		}
	}
}

void configFile::insertField ( const QString& field_name, const QString& value )
{
	if ( cfgData.currentIndex () != -1 )
	{
		configFile_st* section_info ( cfgData.current () );
		section_info->fields.append ( field_name );
		section_info->values.append ( value );
		const uint line_len ( static_cast<uint>(field_name.count () + value.count ()) );
		if ( m_buffersize < line_len )
			m_buffersize = line_len + 5;
		m_needsaving = true;
	}
}

void configFile::deleteField ( const QString& field_name )
{
	if ( cfgData.currentIndex () != -1 )
	{
		configFile_st* __restrict section_info ( cfgData.current () );
		const uint n_fields ( section_info->fields.count () );
		for ( uint i ( 0 ); i < n_fields; ++i )
		{
			if ( section_info->fields.at ( i ) == field_name )
			{
				section_info->fields.remove ( static_cast<int>(i) );
				section_info->values.remove ( static_cast<int>(i) );
				m_needsaving = true;
				break;
			}
		}
	}
}

bool configFile::setFieldValue ( const QString& field_name, const QString& value )
{
	if ( cfgData.currentIndex () != -1 )
	{
		configFile_st* __restrict section_info ( cfgData.current () );
		const int idx ( fieldIndex ( field_name ) );
		if ( idx != -1 )
		{
			section_info->values[idx] = value;
			const uint line_len ( static_cast<uint>(field_name.count () + value.count ()) );
			if ( m_buffersize < line_len )
				m_buffersize = line_len + 5;
			m_needsaving = true;
			return true;
		}
	}
	return false;
}

bool configFile::setFieldValue ( const uint field_index, const QString& value )
{
	if ( cfgData.currentIndex () != -1 )
	{
		configFile_st* __restrict section_info ( cfgData.current () );
		if ( field_index < static_cast<uint>(section_info->values.count ()) )
		{
			section_info->values[field_index] = value;
			const uint line_len ( static_cast<uint>(section_info->fields.at ( field_index ).count () + value.count ()) );
			if ( m_buffersize < line_len )
				m_buffersize = line_len + 5;
			m_needsaving = true;
			return true;
		}
	}
	return false;
}

bool configFile::loadData ()
{
	qint64 buf_size ( m_buffersize * sizeof ( char ) );
	char* __restrict buf ( new char[buf_size] );
	int64_t n_chars ( -1 );
	int idx ( -1 ), idx2 ( -1 );
	QString line;
	configFile_st* section_info ( nullptr );
	bool b_skiplineread ( false );

	do
	{
		if ( !b_skiplineread )
		{
			n_chars = m_file.readLine ( buf, buf_size );
			if ( n_chars <= 2 ) continue;
			line = QString::fromUtf8 ( buf, static_cast<int>(n_chars - 1) );
		}
		else
			b_skiplineread = false;

		idx = line.indexOf ( CHR_L_BRACKET );
		if ( idx != -1 )
		{
			idx2 = line.indexOf ( CHR_R_BRACKET, ++idx );
			if ( idx2 != -1 )
			{
				section_info = new configFile_st;
				section_info->section_name = line.mid ( idx, idx2 - idx );
				do
				{
					n_chars = m_file.readLine ( buf, buf_size );
					if ( n_chars >= 3 )
					{
						line = QString::fromUtf8 ( buf, static_cast<int>(n_chars - 1) );
						idx = line.indexOf ( CHR_EQUAL );
						if ( idx != -1 ) {
							section_info->fields.append ( line.left ( idx ) );
							section_info->values.append ( line.mid ( idx + 1, static_cast<int>(n_chars - idx - 1) ) );
						}
						else
						{
							if ( line.contains ( CHR_L_BRACKET ) )
							{ // file was recorded without blank lines between this session
								b_skiplineread = true;
								break;
							}
						}
					}
				} while ( n_chars >= 3 ); // whenever there is a blank line, or a line with too few characters, end of session
				cfgData.append ( section_info );
			}
		}
	} while ( n_chars != -1 );

	delete [] buf;
	cfgData.setCurrent ( 0 );
	return !( cfgData.isEmpty () );
}

bool configFile::writeData ()
{
	QString line;
	configFile_st* __restrict section_info ( nullptr );
	uint n_fields ( 0 );
	int written ( 0 );
	for ( uint i ( 0 ); i < cfgData.count (); ++i )
	{
		section_info = cfgData.at ( i );
		line = CHR_L_BRACKET + section_info->section_name + CHR_R_BRACKET + CHR_NEWLINE;
		n_fields = section_info->fields.count ();
		for ( uint x ( 0 ); x < n_fields; ++x )
			line += section_info->fields.at ( x ) + CHR_EQUAL + section_info->values.at ( x ) + CHR_NEWLINE;
		written += m_file.write ( line.toUtf8 ().data (), qstrlen ( line.toUtf8 ().data () ) );
	}
	return ( written > 0 );
}

void configFile::clearData ()
{
	cfgData.clear ( true );
}
//--------------------------------------------CONFIG-FILE--------------------------------

//--------------------------------------------DATA-FILE--------------------------------
dataFile::dataFile ()
	: textFile ()
{
	m_type = TF_DATA;
}

dataFile::dataFile ( const QString& filename )
	: textFile ( filename )
{
	m_type = TF_DATA;
}

dataFile::~dataFile ()
{
	if ( m_needsaving )
		commit ();
	clear ();
}

void dataFile::insertRecord ( const int pos, const stringRecord& rec )
{
	if ( pos >= 0 )
	{
		recData.insertRecord ( static_cast<uint>(pos), rec );
		if ( rec.toString ().count () > static_cast<int>(m_buffersize) )
			m_buffersize = static_cast<uint>(rec.toString ().count () + 1);
		m_needsaving = true;
	}
}

void dataFile::changeRecord ( const int pos, const stringRecord& rec )
{
	if ( pos >= 0 && pos < static_cast<int>(recData.countRecords ()) )
	{
		recData.changeRecord ( static_cast<uint>(pos), rec );
		if ( rec.toString ().count () > static_cast<int>(m_buffersize) )
			m_buffersize = static_cast<uint>(rec.toString ().count () + 1);
		m_needsaving = true;
	}
}

void dataFile::deleteRecord ( const int pos )
{
	if ( pos >= 0 && static_cast<uint>(pos) < recData.countRecords () )
	{
		recData.removeRecord ( static_cast<uint>(pos) );
		m_needsaving = true;
	}
}

void dataFile::appendRecord ( const stringRecord& rec )
{
	recData.fastAppendRecord ( rec );
	if ( rec.toString ().count () > static_cast<int>(m_buffersize) )
		m_buffersize = static_cast<uint>(rec.toString ().count () + 1);
	m_needsaving = true;
}

bool dataFile::getRecord ( stringRecord& rec, const int pos ) const
{
	if ( pos >= 0 && static_cast<uint>(pos) < recData.countRecords () )
	{
		rec = recData.readRecord ( static_cast<uint>(pos) );
		return true;
	}
	return false;
}

bool dataFile::getRecord ( stringRecord& rec, const QString& value, const uint field ) const
{
	const int row ( recData.findRecordRowByFieldValue ( value, field ) );
	if ( row >= 0 )
	{
		rec = recData.readRecord ( static_cast<uint>(row) );
		return rec.isOK ();
	}
	else
		return false;
}

void dataFile::clearData ()
{
	recData.clear ();
}

bool dataFile::loadData ()
{
	QString buf ( m_file.readAll () );
	if ( buf.length () > 1 )
	{
		// buf.remove ( 0, m_headerSize ); 
		recData.fromString ( buf );
	}
	return recData.isOK ();
}

bool dataFile::writeData ()
{
	int written ( 0 );
	if ( recData.isOK () )
	{
		const QByteArray data ( recData.toString ().toUtf8() );
		written = m_file.write ( data, data.size () );
	}
	return ( written > 0 );
}
