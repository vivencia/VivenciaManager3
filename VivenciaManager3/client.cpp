#include "client.h"
#include "vmlistitem.h"
#include "completers.h"
#include "configops.h"
#include "fileops.h"
#include "vivenciadb.h"

const unsigned char TABLE_VERSION ( 'A' );

bool updateClientTable ()
{
#ifdef TABLE_UPDATE_AVAILABLE
	VDB ()->optimizeTable ( &Client::t_info );
	return true;
#else
	VDB ()->optimizeTable ( &Client::t_info );
	return false;
#endif //TABLE_UPDATE_AVAILABLE
}

const TABLE_INFO Client::t_info =
{
	CLIENT_TABLE,
	QStringLiteral ( "CLIENTS" ),
	QStringLiteral ( " ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci" ),
	QStringLiteral ( " PRIMARY KEY ( `ID` ) , UNIQUE KEY `id` ( `ID` ) " ),
	QStringLiteral ( "`ID`|`NAME`|`STREET`|`NUMBER`|`DISTRICT`|`CITY`|`ZIPCODE`|`PHONES`|`EMAIL`|`BEGINDATE`|`ENDDATE`|`STATUS`|" ),
	QStringLiteral ( " int ( 9 ) NOT NULL, | varchar ( 60 ) COLLATE utf8_unicode_ci DEFAULT NULL, | varchar ( 60 ) COLLATE utf8_unicode_ci DEFAULT NULL, |"
	" varchar ( 6 ) COLLATE utf8_unicode_ci DEFAULT NULL, | varchar ( 30 ) COLLATE utf8_unicode_ci DEFAULT NULL, | varchar ( 30 ) COLLATE utf8_unicode_ci DEFAULT NULL, |"
	" varchar ( 20 ) COLLATE utf8_unicode_ci DEFAULT NULL, | varchar ( 200 ) COLLATE utf8_unicode_ci DEFAULT NULL, | varchar ( 200 ) COLLATE utf8_unicode_ci DEFAULT NULL, | "
	" varchar ( 60 ) COLLATE utf8_unicode_ci DEFAULT NULL, | varchar ( 60 ) COLLATE utf8_unicode_ci DEFAULT NULL, | int ( 2 ) DEFAULT NULL, |" ),
	QStringLiteral ( "ID|Name|Street|Number|District|City|Zip code|Phones|E-mail|Client since|Client to|Active|" ),
	CLIENTS_FIELDS_TYPE,
	TABLE_VERSION, CLIENT_FIELD_COUNT, TABLE_CLIENT_ORDER, &updateClientTable
	#ifdef TRANSITION_PERIOD
	// it is actually false, but the update routine in generaltable.cpp checks for it, and is the only place in all of the code.
	// Since the code there must not call updateIDs for the client table, this false information here is actually harmless and makes for one less conditional statement there
	, true
	#endif
};

static void clientNameChangeActions ( const DBRecord* db_rec )
{
	if ( db_rec->action () == ACTION_ADD )
	{
		const QString baseClientDir ( CONFIG ()->projectsBaseDir () + recStrValue ( db_rec, FLD_CLIENT_NAME ) + CHR_F_SLASH );
		fileOps::createDir ( baseClientDir );
		fileOps::createDir ( baseClientDir + QLatin1String ( "Pictures/" ) + QString::number ( vmNumber::currentDate.year () ) );
	}
	else if ( db_rec->action () == ACTION_EDIT )
	{
		fileOps::rename ( CONFIG ()->projectsBaseDir () + db_rec->actualRecordStr ( FLD_CLIENT_NAME ),
						  CONFIG ()->projectsBaseDir () + db_rec->backupRecordStr ( FLD_CLIENT_NAME ) );
	}
	
	APP_COMPLETERS ()->updateCompleter ( recStrValue ( db_rec, FLD_CLIENT_NAME ), vmCompleters::CLIENT_NAME );
}

static void updateClientStreetCompleter ( const DBRecord* db_rec )
{
	APP_COMPLETERS ()->updateCompleter ( recStrValue ( db_rec, FLD_CLIENT_STREET ), vmCompleters::ADDRESS );
}

static void updateClientDistrictCompleter ( const DBRecord* db_rec )
{
	APP_COMPLETERS ()->updateCompleter ( recStrValue ( db_rec, FLD_CLIENT_DISTRICT ), vmCompleters::ADDRESS );
}

static void updateClientCityCompleter ( const DBRecord* db_rec )
{
	APP_COMPLETERS ()->updateCompleter ( recStrValue ( db_rec, FLD_CLIENT_CITY ) , vmCompleters::ADDRESS );
}

Client::Client ( const bool connect_helper_funcs )
	: DBRecord ( CLIENT_FIELD_COUNT )
{
	::memset ( this->helperFunction, 0, sizeof ( this->helperFunction ) );
	DBRecord::t_info = &( this->t_info );
	DBRecord::m_RECFIELDS = this->m_RECFIELDS;

	if ( connect_helper_funcs )
	{
		DBRecord::helperFunction = this->helperFunction;
		setHelperFunction ( FLD_CLIENT_NAME, &clientNameChangeActions );
		setHelperFunction ( FLD_CLIENT_STREET, &updateClientStreetCompleter );
		setHelperFunction ( FLD_CLIENT_DISTRICT, &updateClientDistrictCompleter );
		setHelperFunction ( FLD_CLIENT_CITY, &updateClientCityCompleter );
	}
}

Client::~Client () {}

int Client::searchCategoryTranslate ( const SEARCH_CATEGORIES sc ) const
{
	switch ( sc )
	{
		case SC_ID:			return FLD_CLIENT_ID;
		case SC_ADDRESS_1:	return FLD_CLIENT_STREET;
		case SC_ADDRESS_2:	return FLD_CLIENT_NUMBER;
		case SC_ADDRESS_3:	return FLD_CLIENT_DISTRICT;
		case SC_ADDRESS_4:	return FLD_CLIENT_CITY;
		case SC_ADDRESS_5:	return FLD_CLIENT_ZIP;
		case SC_DATE_1:		return FLD_CLIENT_STARTDATE;
		case SC_DATE_2:		return FLD_CLIENT_ENDDATE;
		case SC_TYPE:		return FLD_CLIENT_NAME;
		case SC_EXTRA_1:	return FLD_CLIENT_EMAIL;
		default:			return -1;
	}
}

void Client::copySubRecord ( const uint subrec_field, const stringRecord& subrec )
{
	if ( subrec_field == FLD_CLIENT_PHONES || subrec_field == FLD_CLIENT_EMAIL )
	{
		if ( subrec.curIndex () == -1 )
			subrec.first ();
		stringRecord contact_info;
		
		// These sub record fields can contain any number of entries, so we keep on adding until we find a string that is not of the expected type
		if ( subrec_field == FLD_CLIENT_PHONES )
		{
			vmNumber phone;
			do
			{
				if ( phone.fromTrustedStrPhone ( subrec.curValue (), false ).isPhone () )
					contact_info.fastAppendValue ( phone.toString () );
				else
					break;
			} while ( subrec.next () );
		}
		else
		{
			vmNumber date;	// FLD_CLIENT_STARTDATE is the next field after the emails sub records
			QString email;
			do
			{
				email = subrec.curValue ();
				if ( !email.isEmpty () )
				{
					if ( !date.fromTrustedStrDate ( email, vmNumber::VDF_DB_DATE, false ).isDate () )
						contact_info.fastAppendValue ( email );
					else
						break;
				}
			} while ( subrec.next () );
		}
		
		setRecValue ( this, subrec_field, contact_info.toString () );
	}
}

QString Client::clientName ( const QString& id )
{
	QSqlQuery query;
	if ( VDB ()->runQuery ( QLatin1String ( "SELECT NAME FROM CLIENTS WHERE ID='" ) + id + CHR_CHRMARK, query ) )
		return query.value ( 0 ).toString ();
	return emptyString;
}

uint Client::clientID ( const QString& name )
{
	QSqlQuery query;
	if ( VDB ()->runQuery ( QLatin1String (	"SELECT ID FROM CLIENTS WHERE NAME='" ) + name + CHR_CHRMARK, query ) )
		return query.value ( 0 ).toUInt ();
	return 0;
}

QString Client::concatenateClientInfo ( const Client& client )
{
	QString info;
	info = recStrValue ( &client, FLD_CLIENT_NAME );
	if ( !info.isEmpty () )
	{
		info += QLatin1String ( client.opt ( FLD_CLIENT_STATUS ) == true ? "(*)\n" : "\n" );
		if ( !recStrValue ( &client, FLD_CLIENT_STREET ).isEmpty () )
			info += recStrValue ( &client, FLD_CLIENT_STREET ) + QLatin1String ( ", " );

		if ( !recStrValue ( &client, FLD_CLIENT_NUMBER ).isEmpty () )
			info += recStrValue ( &client, FLD_CLIENT_NUMBER );
		else
			info += QLatin1String ( "S/N" );

		info += QLatin1String ( " - " );

		if ( !recStrValue ( &client, FLD_CLIENT_DISTRICT ).isEmpty () )
			info += recStrValue ( &client, FLD_CLIENT_DISTRICT ) + QLatin1String ( " - " );

		if ( !recStrValue ( &client, FLD_CLIENT_CITY ).isEmpty () )
			info += recStrValue ( &client, FLD_CLIENT_CITY ) + QLatin1String ( "/SP" );

		if ( !recStrValue ( &client, FLD_CLIENT_PHONES ).isEmpty () )
		{
			info += CHR_NEWLINE;
			const stringRecord phones_rec ( recStrValue ( &client, FLD_CLIENT_PHONES ) );
			if ( phones_rec.first () )
			{
				info += QLatin1String ( "Telefone(s): " ) + phones_rec.curValue ();
				while ( phones_rec.next () )
					info += QLatin1String ( " / " ) + phones_rec.curValue ();
			}
		}
		if ( !recStrValue ( &client, FLD_CLIENT_EMAIL ).isEmpty () )
		{
			info += CHR_NEWLINE;
			const stringRecord emails_rec ( recStrValue ( &client, FLD_CLIENT_EMAIL ) );
			if ( emails_rec.first () )
			{
				info += QLatin1String ( "email(s)/site(s): " ) + emails_rec.curValue ();
				while ( emails_rec.next () )
					info += QLatin1String ( " / " ) + emails_rec.curValue ();
			}
		}
	}
	return info;
}

void Client::setListItem ( clientListItem* client_item )
{
	DBRecord::mListItem = static_cast<vmListItem*>( client_item );
}

clientListItem* Client::clientItem () const
{
	return static_cast<clientListItem*>( DBRecord::mListItem );
}
