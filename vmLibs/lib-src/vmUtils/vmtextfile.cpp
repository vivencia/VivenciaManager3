#include "vmtextfile.h"
#include "vmlibs.h"
#include "heapmanager.h"
#include "fileops.h"
#include "vmfilemonitor.h"

#include <vmNotify/vmnotify.h>

#include <QtCore/QStringList>
#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>

static const QString HEADER_ID ( QStringLiteral ( "#!VMFILE" ) );
static const QString CFG_TYPE_LINE ( QStringLiteral ( "@CFG,%1\n" ) );
static const QString DATA_TYPE_LINE ( QStringLiteral ( "@CSV,%1\n" ) );

pointersList<textFile::sharedResources*> textFile::sharedResList ( 3 );

textFile::sharedResources* textFile::findSharedResource ( const QString& filename )
{
	for ( uint i ( 0 ); i < textFile::sharedResList.count (); ++i )
	{
		if ( textFile::sharedResList.at ( i )->filename == filename )
			return textFile::sharedResList.at ( i );
	}
	return nullptr;
}

static void msleep ( unsigned long msecs )
{
	QMutex mutex;
	mutex.lock ();
	QWaitCondition waitCondition;
	waitCondition.wait ( &mutex, msecs );
	mutex.unlock ();
}

//--------------------------------------------TEXT-FILE--------------------------------
textFile::textFile ()
	: m_open ( false ), m_needsaving ( false ), mb_needRechecking ( false ), m_headerSize ( 0 ),
	  m_buffersize ( 0 ), m_type ( TF_TEXT ), m_filemonitor ( nullptr ),
	  mb_IgnoreEvents ( false ), mUsedRes ( nullptr )
{}

textFile::textFile ( const QString& filename )
	: textFile ()
{
	setFileName ( filename );
}

textFile::~textFile ()
{
	setIgnoreEvents ( true );
	heap_del ( m_filemonitor );
	clear ();
	if ( mUsedRes && ( --(mUsedRes->counter) == 0 ) )
	{
		sharedResList.removeOne ( mUsedRes, 0, true );
		mUsedRes = nullptr;
	}
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
	const bool b_ignore ( mb_IgnoreEvents );
	setIgnoreEvents ( true );
	m_needsaving = false;
	mb_needRechecking = false;
	fileOps::removeFile ( m_filename );
	clear ();
	setIgnoreEvents ( b_ignore );
}

void textFile::clear ()
{
	clearData ();
	m_buffersize = 0;
	m_file.close ();
	m_open = false;
	m_needsaving = false;
	mb_needRechecking = false;
}

bool textFile::open ()
{
	if ( m_file.fileName ().isEmpty () )
	{
		mUsedRes = findSharedResource ( m_filename );
		if ( mUsedRes == nullptr )
		{
			mUsedRes = new sharedResources;
			mUsedRes->filename = m_filename;
			sharedResList.append ( mUsedRes );
		}
		mUsedRes->counter++;
		m_file.setFileName ( m_filename );
	}
	else
	{
		if ( m_file.fileName () != m_filename )
		{
			clear ();
			m_file.setFileName ( QString () );
			return open ();
		}

		if ( m_file.isOpen () )
			return true;
	}

	return open2 ();
}

bool textFile::open2 ()
{
	const bool b_ignore ( mb_IgnoreEvents );
	setIgnoreEvents ( true );
	const bool b_exists ( fileOps::exists ( m_filename ).isOn () );
	//QIODevice::OpenModeFlag openflag ( b_exists ? QIODevice::ReadWrite : QIODevice::WriteOnly );
	QIODevice::OpenModeFlag openflag ( QIODevice::ReadWrite );

	if ( b_exists && !fileOps::canWrite ( m_filename ).isOn () )
	{
		openflag = QIODevice::ReadOnly;
	}
	m_open = m_file.open ( openflag|QIODevice::Text );
	setIgnoreEvents ( b_ignore );
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
	const bool b_ignore ( mb_IgnoreEvents );
	setIgnoreEvents ( true );

	if ( !m_filemonitor )
	{
		m_filemonitor = new vmFileMonitor;
		m_filemonitor->setCallbackForEvent ( [&] ( const QString& filename, const uint event ) {
							return fileExternallyAltered ( filename, event ); } );
	}
	else
	{
		if ( m_filename != m_file.fileName () )
			m_filemonitor->stopMonitoring ( fileOps::fileNameWithoutPath ( m_file.fileName () ) );
	}
	m_filemonitor->startMonitoring ( m_filename, VM_IN_MODIFY | VM_IN_DELETE_SELF | VM_IN_DELETE | VM_IN_CREATE );

	if ( !m_open )
	{
		if ( !open () )
		{
			setIgnoreEvents ( b_ignore );
			return TRI_OFF;
		}
	}
	if ( isEmpty () )
	{
		setIgnoreEvents ( b_ignore );
		return TRI_UNDEF;
	}

	if ( mUsedRes->counter > 1 )
	{
		if ( mUsedRes->b_inUse )
		{
			msleep ( 300 );
			return load ();
		}
	}

	readType ();
	const triStateType ret ( loadData () );
	setIgnoreEvents ( b_ignore );
	return ret;
}

bool textFile::overwrite ()
{
	if ( mUsedRes->counter > 1 )
	{
		if ( mUsedRes->b_inUse )
		{
			msleep ( 300 );
			return overwrite ();
		}
	}
	mUsedRes->b_inUse = true;
	const bool b_ignore ( mb_IgnoreEvents );
	setIgnoreEvents ( true );
	m_file.close ();
	fileOps::removeFile ( m_filename );
	m_file.setFileName ( m_filename );
	m_open = m_file.open ( QIODevice::ReadWrite|QIODevice::Text );
	setIgnoreEvents ( b_ignore );
	mUsedRes->b_inUse = false;
	return m_open;
}

void textFile::writeHeader ()
{
	if ( m_type != TF_TEXT )
	{
		QString str;
		str = HEADER_ID + ( m_type == TF_CONFIG ? CFG_TYPE_LINE : DATA_TYPE_LINE ).arg ( m_buffersize );
		m_headerSize = qstrlen ( str.toUtf8 ().data () );
		m_file.write ( str.toUtf8 ().data (), m_headerSize );
		m_file.flush ();
	}
}

void textFile::setBufferModifiedInfo ( const uint new_size )
{
	if ( m_buffersize < new_size )
		m_buffersize = new_size + 1;
	m_needsaving = true;
}

void textFile::fileExternallyAltered ( const QString&, const uint event )
{
	if ( !mb_IgnoreEvents )
	{
		if ( event & VM_IN_DELETE )
		{
			mb_needRechecking = true;
			//const bool b_ignore ( mb_IgnoreEvents );
			//setIgnoreEvents ( true );
			//m_needsaving = true;
			//commit ();
			//setIgnoreEvents ( b_ignore );
		}
		if ( event & VM_IN_MODIFY )
		{
			mb_needRechecking = true;
			//const bool b_ignore ( mb_IgnoreEvents );
			//setIgnoreEvents ( true );
			//recheckData ();
			//setIgnoreEvents ( b_ignore );
		}
	}
}

void textFile::commit ()
{
	if ( !m_needsaving ) return;

	if ( mUsedRes->b_inUse )
	{
		msleep ( 300 );
		commit ();
		return;
	}

	if ( mUsedRes->modified_counter > 0 && mUsedRes->modifierInstance != this )
		mb_needRechecking = true;
	if ( mb_needRechecking )
	{
		if ( recheckData () )
			mUsedRes->modified_counter--;
	}

	const bool b_ignore ( mb_IgnoreEvents );
	setIgnoreEvents ( true );

	mUsedRes->b_inUse = true;
	mUsedRes->modifierInstance = this;

	m_file.close ();
	fileOps::removeFile ( m_filename );
	m_file.setFileName ( m_filename );
	m_open = m_file.open ( QIODevice::ReadWrite|QIODevice::Text );

	if ( m_open )
	{
		writeHeader ();
		if ( writeData () )
		{
			m_file.flush ();
			m_needsaving = false;
		}
		mUsedRes->modified_counter = mUsedRes->counter - 1;
	}
	mUsedRes->b_inUse = false;
	msleep ( 400 );
	setIgnoreEvents ( b_ignore );
}

void textFile::setText ( const QString& new_file_text )
{
	m_data = new_file_text;
	m_needsaving = true;
}

void textFile::setIgnoreEvents ( const bool b_ignore )
{
	mb_IgnoreEvents = b_ignore;
	if ( m_filemonitor )
	{
		if ( b_ignore )
			m_filemonitor->pauseMonitoring ();
		else
			m_filemonitor->resumeMonitoring ();
	}
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

bool textFile::recheckData ( const bool b_userInteraction )
{
	if ( b_userInteraction )
	{
		const QString messageStr ( APP_TR_FUNC ( "The contents of the file %1 were modified by an external applicantion."
											 "Load the new file contents? If not, on saving, data might be lost." ) );
		if ( !vmNotify::questionBox ( APP_TR_FUNC ( "Outside changes" ), messageStr.arg ( m_filename ) ) )
			 return false;
	}

	m_file.close ();
	if ( open2 () )
	{
		readType ();
		mb_needRechecking = false;
		return true;
	}
	return false;
}
//--------------------------------------------TEXT-FILE--------------------------------

//--------------------------------------------CONFIG-FILE--------------------------------
struct configFile::configFile_st
{
	bool b_modified { false };
	QString section_name;
	vmList<QString> fields;
	vmList<QString> values;

	configFile_st () : fields ( QString (), 5 ), values ( QString (), 5 ) {}
};

configFile::configFile ()
	: textFile (), cfgData ( 30 )
{
	m_type = TF_CONFIG;
}

configFile::configFile ( const QString& filename, const QString& object_name )
	: textFile ( filename ), cfgData ( 30 ), m_objectName ( object_name )
{
	m_type = TF_CONFIG;
}

configFile::~configFile () {}

bool configFile::setWorkingSection ( const QString& section_name )
{
	if ( mUsedRes->b_inUse )
	{
		msleep ( 300 );
		return setWorkingSection ( section_name );
	}

	if ( mUsedRes->modified_counter > 0 && mUsedRes->modifierInstance != this )
	{
		if ( recheckData () )
			mUsedRes->modified_counter--;
	}

	const int i ( findSection ( section_name ) );
	if ( i >= 0 )
	{
		cfgData.setCurrent ( i );
		return true;
	}
	return false;
}

const QString& configFile::fieldValue ( const QString& field_name ) const
{
	if ( mUsedRes->b_inUse )
	{
		msleep ( 300 );
		return fieldValue ( field_name );
	}

	if ( mUsedRes->modified_counter > 0 && mUsedRes->modifierInstance != this )
		const_cast<configFile*>( this )->mb_needRechecking = true;

	if ( mb_needRechecking )
	{
		if ( const_cast<configFile*>( this )->recheckData () )
			mUsedRes->modified_counter--;
	}

	const int idx ( fieldIndex ( field_name ) );
	if ( idx != -1 )
		return cfgData.current ()->values.at ( idx );
	return emptyString;
}

int configFile::fieldIndex ( const QString& field_name ) const
{
	if ( mUsedRes->b_inUse )
	{
		msleep ( 300 );
		return fieldIndex ( field_name );
	}

	if ( mUsedRes->modified_counter > 0 && mUsedRes->modifierInstance != this )
		const_cast<configFile*>( this )->mb_needRechecking = true;

	if ( mb_needRechecking )
	{
		if ( const_cast<configFile*>( this )->recheckData () )
			mUsedRes->modified_counter--;
	}

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

void configFile::insertNewSection ( const QString& section_name )
{
	if ( !section_name.isEmpty () )
	{
		auto section_info ( new configFile_st );
		section_info->section_name = section_name;
		section_info->b_modified = true;
		cfgData.append ( section_info );
		setBufferModifiedInfo ( static_cast<uint>( section_name.count () + 2 ) );
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
				setBufferModifiedInfo ( 0 );
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
		section_info->b_modified = true;
		setBufferModifiedInfo ( static_cast<uint>(field_name.count () + value.count ()) + 1 );
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
				setBufferModifiedInfo ( 0 );
				break;
			}
		}
	}
}

bool configFile::setFieldValue ( const QString& field_name, const QString& value )
{
	if ( mUsedRes->b_inUse )
	{
		msleep ( 300 );
		return setFieldValue ( field_name, value );
	}

	if ( mUsedRes->modified_counter > 0 && mUsedRes->modifierInstance != this )
		mb_needRechecking = true;

	if ( mb_needRechecking )
	{
		if ( recheckData () )
			mUsedRes->modified_counter--;
	}

	if ( cfgData.currentIndex () != -1 )
	{
		configFile_st* __restrict section_info ( cfgData.current () );
		const int idx ( fieldIndex ( field_name ) );
		if ( idx != -1 )
		{
			section_info->values[idx] = value;
			section_info->b_modified = true;
			setBufferModifiedInfo ( static_cast<uint>(field_name.count () + value.count ()) + 1 );
			return true;
		}
	}
	return false;
}

bool configFile::loadData ()
{
	return parseConfigFile ();
}

bool configFile::writeData ()
{
	QString line;
	configFile_st* __restrict section_info ( nullptr );
	uint n_fields ( 0 );
	qint64 written ( 0 ), written_total ( 0 );
	for ( uint i ( 0 ); i < cfgData.count (); ++i )
	{
		section_info = cfgData.at ( i );
		line = CHR_NEWLINE + CHR_L_BRACKET + section_info->section_name + CHR_R_BRACKET + CHR_NEWLINE;
		n_fields = section_info->fields.count ();
		for ( uint x ( 0 ); x < n_fields; ++x )
			line += section_info->fields.at ( x ) + CHR_EQUAL + section_info->values.at ( x ) + CHR_NEWLINE;

		written = m_file.write ( line.toUtf8 ().constData (), line.size () );
		if ( written > 0 )
		{
			section_info->b_modified = false;
			written_total += written;
		}
	}
	return ( written_total > 0 );
}

void configFile::clearData ()
{
	cfgData.clear ( true );
}

bool configFile::recheckData ( const bool b_userInteraction )
{
	if ( textFile::recheckData ( b_userInteraction ) )
		return parseConfigFile ( true );
	return false;
}

int configFile::findSection ( const QString& section_name ) const
{
	for ( uint i ( 0 ); i < cfgData.count (); ++i )
	{
		if ( cfgData.at ( i )->section_name == section_name )
			return static_cast<int>( i );
	}
	return -1;
}

bool configFile::parseConfigFile ( const bool b_reload )
{
	qint64 buf_size ( m_buffersize * sizeof ( char ) );
	auto* __restrict buf ( new char[static_cast<unsigned long>(buf_size)] );
	int64_t n_chars ( -1 );
	int idx ( -1 ), idx2 ( -1 ), section_idx ( -1 );
	QString line, section_name;
	configFile_st* section_info ( nullptr );
	bool b_skiplineread ( false );

	do
	{
		if ( !b_skiplineread )
		{
			n_chars = m_file.readLine ( buf, static_cast<qint64>( buf_size ) );
			if ( n_chars <= 2 ) continue;
			line = QString::fromUtf8 ( buf );
		}
		else
			b_skiplineread = false;

		idx = line.indexOf ( CHR_L_BRACKET );
		if ( idx != -1 )
		{
			idx2 = line.indexOf ( CHR_R_BRACKET, ++idx );
			if ( idx2 != -1 )
			{
				section_name = line.mid ( idx, idx2 - idx );
				if ( b_reload )
				{
					section_idx = findSection ( section_name );
					if ( section_idx >= 0 )
					{
						if ( cfgData.at ( section_idx )->b_modified )
							continue;
						deleteSection ( section_name ); // delete the section in the buffers to create a new one from the data in the file
					}
				}

				section_info = new configFile_st;
				section_info->section_name = section_name.simplified ();

				do
				{
					n_chars = m_file.readLine ( buf, buf_size );
					if ( n_chars > 0 )
					{
						line = QString::fromUtf8 ( buf );
						idx = line.indexOf ( CHR_EQUAL );
						if ( idx != -1 )
						{
							section_info->fields.append ( line.left ( idx ) );
							section_info->values.append ( line.mid ( idx + 1, static_cast<int>(n_chars - idx - 1) ).simplified () );
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
				} while ( n_chars > 0 ); // whenever there is a blank line, or a line with too few characters, end of session
				cfgData.append ( section_info );
			}
		}
	} while ( n_chars != -1 );

	delete [] buf;
	cfgData.setCurrent ( 0 );
	return !( cfgData.isEmpty () );
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

dataFile::~dataFile () {}

void dataFile::insertRecord ( const int pos, const stringRecord& rec )
{
	if ( pos >= 0 )
	{
		recData.insertRecord ( static_cast<uint>( pos ), rec );
		if ( rec.toString ().count () > static_cast<int>( m_buffersize ) )
			m_buffersize = static_cast<uint>( rec.toString ().count () + 1 );
		m_needsaving = true;
	}
}

void dataFile::changeRecord ( const int pos, const stringRecord& rec )
{
	if ( mUsedRes->b_inUse )
	{
		msleep ( 300 );
		return changeRecord ( pos, rec );
	}

	if ( mUsedRes->modified_counter > 0 && mUsedRes->modifierInstance != this )
	{
		if ( recheckData () )
			mUsedRes->modified_counter--;
	}

	if ( pos >= 0 && pos < static_cast<int>( recData.countRecords () ) )
	{
		recData.changeRecord ( static_cast<uint>( pos ), rec );
		if ( rec.toString ().count () > static_cast<int>(m_buffersize) )
			m_buffersize = static_cast<uint>( rec.toString ().count () + 1 );
		m_needsaving = true;
	}
}

bool dataFile::deleteRecord( const int pos )
{
	if ( pos >= 0 && static_cast<uint>( pos ) < recData.countRecords () )
	{
		recData.removeRecord ( static_cast<uint>(pos) );
		m_needsaving = true;
		return true;
	}
	return false;
}

void dataFile::appendRecord ( const stringRecord& rec )
{
	recData.fastAppendRecord ( rec );
	if ( rec.toString ().count () > static_cast<int>( m_buffersize ) )
		m_buffersize = static_cast<uint>( rec.toString ().count () + 1 );
	m_needsaving = true;
}

bool dataFile::getRecord ( stringRecord& rec, const int pos ) const
{
	if ( mUsedRes->b_inUse )
	{
		msleep ( 300 );
		return getRecord ( rec, pos );
	}

	if ( mUsedRes->modified_counter > 0 && mUsedRes->modifierInstance != this )
	{
		if ( const_cast<dataFile*>( this )->recheckData () )
			mUsedRes->modified_counter--;
	}

	if ( pos >= 0 && static_cast<uint>( pos ) < recData.countRecords () )
	{
		rec = recData.readRecord ( static_cast<uint>( pos ) );
		return true;
	}
	return false;
}

bool dataFile::getRecord ( stringRecord& rec, const QString& value, const uint field ) const
{
	return getRecord ( rec, recData.findRecordRowByFieldValue ( value, field ) );
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
	qint64 written ( 0 );
	if ( recData.isOK () )
	{
		const QByteArray data ( recData.toString ().toUtf8 () );
		written = m_file.write ( data, static_cast<qint64>( data.size () ) );
	}
	return ( written > 0 );
}

bool dataFile::recheckData ( const bool b_userInteraction )
{
	return textFile::recheckData ( b_userInteraction );
}