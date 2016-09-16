#include "vmnumberformats.h"
#include "global.h"
#include "calculator.h"

#include <QCoreApplication>
//#include <QRegExp>

//#include <cstring>
#include <cmath>
//#include <cstdlib>
#include <climits>

#ifdef QT5
/* you cannot use QStringLiteral outside a function in all compilers,
 * since GCC statement expressions don’t support that.
 * Moreover, the code for QT4 would work, but isn’t read-only sharable:
 */
static const auto CURRENCY = QStringLiteral ( "R$" );
#else
static const QLatin1String CURRENCY ( "R$" );
#endif

constexpr const char* const MONTHS[13] = { "", "Janeiro", "Fevereiro", "Março", "Abril", "Maio", "Junho", "Julho", "Agosto", "Setembro", "Outubro", "Novembro", "Dezembro" };

// Source: http://en.wikipedia.org/wiki/Determination_of_the_day_of_the_week
constexpr const int months_table[13] = { -1, 0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5 };

vmNumber vmNumber::currentDate;
const vmNumber vmNumber::zeroedPrice;
const vmNumber vmNumber::emptyNumber;

vmNumber::vmNumber ( const QString& str, const VM_NUMBER_TYPE type, const int format )
    : m_type ( type ), mb_cached ( false ), mb_valid ( false )
{
	if ( type < VMNT_PHONE )
	{
		for ( uint i ( 0 ); i < 5; ++i )
			nbr_part[i] = 0;
	}
	else
	{
		for ( uint i ( 0 ); i < 5; ++i )
			nbr_upart[i] = 0;
	}

	switch ( type )
	{
        case VMNT_UNSET:
		break;
        case VMNT_INT:
            format > 0 ? fromTrustedStrInt ( str ) : fromStrInt ( str );
		break;
        case VMNT_DOUBLE:
            format > 0 ? fromTrustedStrDouble ( str ) : fromStrDouble ( str );
		break;
        case VMNT_PHONE:
            format > 0 ? fromTrustedStrPhone ( str ) : fromStrPhone ( str );
		break;
        case VMNT_PRICE:
            format > 0 ? fromTrustedStrPrice ( str ) : fromStrPrice ( str );
		break;
        case VMNT_TIME:
            format > 0 ? fromTrustedStrTime ( str, static_cast<VM_TIME_FORMAT> ( format ) ) : fromStrTime ( str );
		break;
        case VMNT_DATE:
            format > 0 ? fromTrustedStrDate ( str, static_cast<VM_DATE_FORMAT> ( format ) ) : fromStrDate ( str );
		break;
	}
}

/*vmNumber::vmNumber ( const QString& str, const bool check_number_type )
	: m_type ( VMNT_UNSET ), mb_cached ( false )
{
	if ( check_number_type ) {
		for ( uint i ( 0 ); i < 5; ++i )
			nbr_part[i] = 0;

		if ( str.contains ( CURRENCY ) )
			fromStrPrice ( str );
		else if ( str.contains ( CHR_DOT ) || str.contains ( CHR_COMMA ) )
			fromStrDouble ( str );
		else if ( str.contains ( CHR_F_SLASH ) )
			fromStrDate ( str );
		else if ( str.contains ( CHR_L_PARENTHESIS ) || str.contains ( CHR_HYPHEN ) )
			fromStrPhone ( str );
		else
			fromStrInt ( str );
	}
}*/

QString vmNumber::useCalc ( const vmNumber& n1, const vmNumber& res, const QString& op ) const
{
	QString expression ( QString::number ( res.toDouble (), 'f', 2 ) +
						 op + QString::number ( n1.toDouble (), 'f', 2 ) );
	Calculator::calc->setExpression ( expression );
	Calculator::calc->eval ( expression );
	return expression;
}

void vmNumber::copy ( vmNumber& dest, const vmNumber& src )
{
	for ( uint i ( 0 ); i < 5; ++i )
	{
		dest.nbr_part[i] = src.nbr_part[i];
		dest.nbr_upart[i] = src.nbr_upart[i];
	}
	dest.m_type = src.m_type;
	dest.mb_cached = src.mb_cached;
    dest.mb_valid = src.mb_valid;
	dest.cached_str = src.cached_str;
}

void vmNumber::clear ( const bool b_unset_type )
{
	switch ( type () )
	{
        case VMNT_INT:
        case VMNT_DOUBLE:
        case VMNT_PRICE:
        case VMNT_TIME:
            for ( uint i ( 0 ); i < 5; ++i )
			{
                nbr_part[i] = 0;
			}
        break;
        case VMNT_PHONE:
        case VMNT_DATE:
            for ( uint i ( 0 ); i < 5; ++i )
			{
                nbr_upart[i] = 0;
			}
		break;
        default:
		break;
	}
	if ( b_unset_type )
		setType ( VMNT_UNSET );
	setCached ( false );
    mb_valid = false;
	cached_str.clear ();
}

// needs improvement to make some of these actually useful
// so far, ret is useless, but it is kept here for future use
bool vmNumber::convertTo ( const VM_NUMBER_TYPE new_type, const bool force )
{
    mb_valid = false;
	if ( new_type != m_type )
	{
		if ( m_type == VMNT_UNSET )
		{
			clear ();
			return true;
		}
		switch ( type () )
		{
            case VMNT_INT:
            case VMNT_UNSET:
                switch ( new_type )
				{
                    case VMNT_PHONE:
                    case VMNT_DATE:
                        if ( !force )
					break;
                    case VMNT_TIME:
                        if ( !force )
					break;
                    default:
                        mb_valid = true;
                    break;
                }
                break;
            case VMNT_DATE:
            case VMNT_PHONE:
                switch ( new_type )
				{
                    case VMNT_INT:
                        if ( !force )
                            break;
                        nbr_part[0] = static_cast<int> ( nbr_upart[0] );
                        mb_valid = true;
                    break;
                    case VMNT_DOUBLE:
                    case VMNT_PRICE:
                    case VMNT_TIME:
                        if ( !force )
                            break;
                        nbr_part[0] = static_cast<int> ( nbr_upart[0] );
                        nbr_part[1] = static_cast<int> ( nbr_upart[1] );
                        mb_valid = true;
                    break;
                    case VMNT_DATE:
                    case VMNT_PHONE:
                    default:
                        mb_valid = true;
                    break;
                }
            break;
            case VMNT_PRICE:
            case VMNT_DOUBLE:
            case VMNT_TIME:
                switch ( new_type )
				{
                    case VMNT_INT:
                        if ( nbr_part[1] != 0 )
						{
                            if ( !force )
                                break;
                            nbr_part[1] = 0;
                        }
                        mb_valid = true;
                    break;
                    case VMNT_DOUBLE:
                    case VMNT_PRICE:
                    case VMNT_TIME:
                    default:
                        mb_valid = true;
                    break;
                    case VMNT_DATE:
                    case VMNT_PHONE:
                    if ( !force )
                        break;
                    nbr_upart[0] = static_cast<unsigned int> ( nbr_part[0] );
                    nbr_upart[1] = static_cast<unsigned int> ( nbr_part[1] );
                    break;
                }
            break;
        }
    }
    if ( mb_valid )
	{
		setType ( new_type );
		setCached ( false );
	}
    return mb_valid;
}

QString vmNumber::toString () const
{
	if ( isCached () )
		return cached_str;

	switch ( type () )
	{
		default:
			return emptyString;
		break;
		case VMNT_PRICE:
			return toPrice ();
		break;
		case VMNT_PHONE:
			return toPhone ();
		break;
		case VMNT_TIME:
			return toTime ( VTF_DAYS );
		break;
		case VMNT_DATE:
			return toDate ( VDF_HUMAN_DATE );
		break;
		case VMNT_INT:
			return toStrInt ();
		break;
		case VMNT_DOUBLE:
			return toStrDouble ();
		break;
	}
}

void vmNumber::makeOpposite ()
{
	switch ( type () ) {
        default:
        {
            for ( uint i ( 0 ); i < 5; ++i )
                nbr_part[i] = 0 - nbr_part[i];
        }
        break;
        case VMNT_PHONE:
        case VMNT_DATE:
        case VMNT_UNSET:
		return;
	}
}
//------------------------------------INT-UINT---------------------------------------
//TODO: check for values over the limit. VMNT_INT uses an internal int but accepts uints, so this might cause trouble

vmNumber& vmNumber::fromStrInt ( const QString& integer )
{
	if ( !integer.isEmpty () )
	{
		QChar qchr;
		int chr ( integer.indexOf ( CHR_HYPHEN ) );
		const bool b_negative ( chr != -1 );
		const int len ( integer.length () );
		clear ( false );
		while ( ++chr < len )
		{
			qchr = integer.at ( chr );
			if ( qchr.isDigit () )
			{
				if ( nbr_part[0] != 0 )
					nbr_part[0] *= 10;
				nbr_part[0] += qchr.digitValue ();
			}
		}
		if ( b_negative )
			nbr_part[0] = 0 - nbr_part[0];
		setType ( VMNT_INT );
		setCached ( false );
	}
	return *this;
}

vmNumber& vmNumber::fromTrustedStrInt ( const QString& integer )
{
	clear ( false );
    mb_valid = false;
    nbr_part[0] = integer.toInt ( &mb_valid );
    if ( mb_valid ) {
		setType ( VMNT_INT );
		setCached ( true );
		cached_str = integer;
	}
	return *this;
}

vmNumber& vmNumber::fromInt ( const int n )
{
	setType ( VMNT_INT );
	setCached ( false );
	nbr_part[0] = n;
	return *this;
}

vmNumber& vmNumber::fromUInt ( const unsigned int n )
{
	setType ( VMNT_INT );
	setCached ( false );
	nbr_part[0] = static_cast<int>( n );
	return *this;
}

int vmNumber::toInt () const
{
	switch ( type () )
	{
        case VMNT_INT:
        case VMNT_DOUBLE:
        case VMNT_PRICE:
            return nbr_part[0];
		break;
        case VMNT_PHONE: // all digits
            return static_cast<int> ( nbr_upart[0] * 1000000 + nbr_upart[1] * 1000 + nbr_upart[2] );
		break;
        case VMNT_TIME: // time in minutes
            return static_cast<int> ( nbr_part[0] * 60 + nbr_part[1] );
        case VMNT_DATE: // days since Oct 10, 1582
            return static_cast<int> ( julianDay () );
        default:
            return 0;
	}
}

unsigned int vmNumber::toUInt () const
{
	// This does not work under several circumstances
	return static_cast<unsigned int> ( toInt () );
}

const QString& vmNumber::toStrInt () const
{
	if ( isInt () )
	{
		if ( !isCached () )
		{
			cached_str = QString::number ( nbr_part[0] );
			setCached ( true );
		}
		return cached_str;
	}
	else
	{
		mQString = QString::number ( toInt () );
		return mQString;
	}
}
//------------------------------------INT-UINT---------------------------------------

//-------------------------------------DOUBLE----------------------------------------
vmNumber& vmNumber::fromStrDouble ( const QString& str_double )
{
	int chr ( 0 );
	const int len ( str_double.length () );
	if ( chr < len ) {
		QChar qchr;
		int idx ( 0 );
		bool negative ( false );
		do {
			qchr = str_double.at ( chr );
			if ( qchr.isDigit () )
			{
				if ( nbr_part[idx] != 0 )
					nbr_part[idx] *= 10;
				nbr_part[idx] += qchr.digitValue ();
                if ( nbr_part[VM_IDX_CENTS] >= 10 )
					break; // only two decimal places after separator
            }
			else if ( qchr == CHR_DOT || qchr == CHR_COMMA )
				idx = 1;
			else if ( qchr == CHR_HYPHEN ) {
				if ( nbr_part[VM_IDX_TENS] == 0 )
					negative = true;
			}
		} while ( ++chr < len );
		if ( negative )
			nbr_part[VM_IDX_TENS] = 0 - nbr_part[VM_IDX_TENS];
        if ( nbr_part[VM_IDX_CENTS] < 10 )
            nbr_part[VM_IDX_CENTS] *= 10;
		setType ( VMNT_DOUBLE );
        setCached ( false );
		return *this;
	}
	clear ();
	return *this;
}

vmNumber& vmNumber::fromTrustedStrDouble ( const QString& str_double )
{
    //This is a hack. Certain methods rely on calling trusted string formats, but the old
    //database is formatted wrongly. Since the migration code did not resolve this, we must
    //treat the string as having an unknown format is its length does not reflect the minimum
    // number of characters the correct format must have ( 0.00 = 4, 1.00 = 4, etc. )
    if ( str_double.length () < 4 )
	{
        return fromStrDouble ( str_double );
	}
	
	clear ( false );
    nbr_part[0] = str_double.left ( str_double.indexOf ( CHR_DOT ) ).toInt ( &mb_valid );
	
    if ( mb_valid )
	{
		setType ( VMNT_DOUBLE );
		setCached ( true );
		cached_str = str_double;
		nbr_part[1] = str_double.right ( 2 ).toInt ();
	}
	return *this;
}

vmNumber& vmNumber::fromDoubleNbr ( const double n )
{
	nbr_part[VM_IDX_TENS] = static_cast<int> ( n );
	const double temp ( ( n - static_cast<double> ( nbr_part[VM_IDX_TENS] ) ) * 100.111 );
	nbr_part[VM_IDX_CENTS] = static_cast<int> ( temp );
	setCached ( false );
	setType ( VMNT_DOUBLE );
	return *this;
}

double vmNumber::toDouble () const
{
	switch ( type () ) {
        case VMNT_PRICE:
        case VMNT_DOUBLE:
        {
            const double decimal ( static_cast<double> ( nbr_part[1] ) / 100.0 );
            const double val ( static_cast<double> ( nbr_part[0] ) + ( nbr_part[0] >= 0 ? decimal : 0 - decimal ) );
            return val;
        }
        break;
        case VMNT_INT:
            return static_cast<double> ( nbr_part[0] );
		break;
        case VMNT_PHONE: // all digits
            return static_cast<double> (
				   static_cast<double> ( nbr_upart[0] * 1000000.0 ) +
				   static_cast<double> ( nbr_upart[1] * 1000.0 ) +static_cast<double> ( nbr_upart[2] ) );
		break;
        case VMNT_TIME: // time in minutes
            return static_cast<double> ( nbr_part[0] * 60.0 + nbr_part[1] );
        case VMNT_DATE:
            return static_cast<double> ( julianDay () );
        default:
            return 0.0;
	}
}

const QString& vmNumber::toStrDouble () const
{
	if ( isDouble () ) {
		if ( !isCached () ) {
			cached_str = QString::number ( toDouble (), 'f', 2 );
			setCached ( true );
		}
		return cached_str;
	}
	else {
		mQString = QString::number ( toDouble (), 'f', 2 );
		return mQString;
	}
}
//-------------------------------------DOUBLE----------------------------------------

//-------------------------------------DATE------------------------------------------
void vmNumber::fixDate ()
{
	int m ( month () ), y ( nbr_upart[VM_IDX_YEAR] );
	int d ( 0 ), days_in_month ( 0 );
	if ( nbr_part[VM_IDX_DAY] > 0 ) {
		d = nbr_upart[VM_IDX_DAY] + static_cast<unsigned int> ( ::fabs ( static_cast<double> ( nbr_part[VM_IDX_DAY] ) ) );
		do {
			days_in_month = daysInMonth ( m, y );
			if ( d > days_in_month ) {
				d -= days_in_month;
				if ( m == 12 ) {
					m = 1;
					y++;
				}
				else
					m++;
			}
			else
				break;
		} while ( true );
	}
	else {
		d = nbr_upart[VM_IDX_DAY] + nbr_part[VM_IDX_DAY];
		if ( d <= 0 ) {
			if ( m != 1 )
				m--;
			else {
				m = 12;
				y--;
			}
			do {
				days_in_month = daysInMonth ( m, y );
				d = days_in_month + d;
				if ( d <= 0 ) {
					if ( m == 1 ) {
						m = 12;
						y--;
					}
					else
						m--;
				}
				else
					break;
			} while ( true );
		}
	}

	const int years_by_months ( static_cast<int> ( nbr_part[VM_IDX_MONTH] / 12 ) );
	const int remaining_months ( static_cast<int> ( nbr_part[VM_IDX_MONTH] % 12 ) );
	y += years_by_months;
	m += remaining_months;

	if ( m > 12 ) {
		++y;
		m -= 12;
	}
	else if ( m <= 0 ) {
		--y;
		m += 12;
	}
	y += nbr_part[VM_IDX_YEAR];

	nbr_upart[VM_IDX_DAY] = d;
	nbr_upart[VM_IDX_MONTH] = m;
	nbr_upart[VM_IDX_YEAR] = y;
}

void vmNumber::setDate ( const int day, const int month, const int year, const bool update )
{
	if ( !isDate () ) {
		nbr_upart[VM_IDX_DAY] = nbr_upart[VM_IDX_MONTH] = nbr_upart[VM_IDX_YEAR] = 0;
		setType ( VMNT_DATE );
	}
	else {
		if ( !update )
			nbr_upart[VM_IDX_DAY] = nbr_upart[VM_IDX_MONTH] = nbr_upart[VM_IDX_YEAR] = 0;
	}

	nbr_part[VM_IDX_DAY] = day;
	nbr_part[VM_IDX_MONTH] = month;
	nbr_part[VM_IDX_YEAR] = year;

	if ( !update && year != 0 ) {
		if ( nbr_part[VM_IDX_YEAR] < 100 )
			nbr_part[VM_IDX_YEAR] += 2000;
	}
	if ( !update && day == 0 )
		nbr_part[VM_IDX_DAY] = 1;

	fixDate ();
    mb_valid = true;
	setCached ( false );
}

vmNumber& vmNumber::fromStrDate ( const QString& date )
{
	if ( !date.isEmpty () ) {
		int n ( date.indexOf ( CHR_F_SLASH ) );
		if ( n != -1 ) { //DB_DATE or HUMAN_DATE
			const int idx ( date.indexOf ( CHR_F_SLASH, n + 1 ) );

			if ( n == 4 ) { // day starts with year
				nbr_upart[VM_IDX_YEAR] = date.left ( n ).toInt ();
				nbr_upart[VM_IDX_DAY] = date.right ( date.length () - idx - 1 ).toInt ();
			}
			else { // date may start with day or a two digit year
				nbr_upart[VM_IDX_DAY] = date.left ( n ).toInt ();
				if ( nbr_upart[VM_IDX_DAY] > 31 ) { // year
					nbr_upart[VM_IDX_YEAR] = nbr_upart[VM_IDX_DAY];
					nbr_upart[VM_IDX_DAY] = date.right ( 2 ).toInt ();
				}
				else {
					nbr_upart[VM_IDX_YEAR] = date.right ( date.length () -
														  date.lastIndexOf ( CHR_F_SLASH ) - 1 ).toInt ();
				}
			}
			++n;
			nbr_upart[VM_IDX_MONTH] = date.mid ( n, idx - n ).toInt ();
			if ( nbr_upart[VM_IDX_YEAR] < 100 )
				nbr_upart[VM_IDX_YEAR] += 2000;

			setType ( VMNT_DATE );
		}
		else {
			if ( date.contains ( QStringLiteral ( "de" ) ) )
				return dateFromLongString ( date );
			else
				return dateFromFilenameDate ( date );
		}
		setCached ( false );
	}
	else {
		clear ( false );
		nbr_upart[VM_IDX_YEAR] = 2000;
		nbr_upart[VM_IDX_MONTH] = 1;
		nbr_upart[VM_IDX_DAY] = 1;
	}
	return *this;
}

vmNumber& vmNumber::fromTrustedStrDate ( const QString& date, const VM_DATE_FORMAT format, const bool cache )
{
	clear ( false );
    switch ( format )
	{
		case VDF_HUMAN_DATE:
            return dateFromHumanDate ( date, cache );
        break;
        case VDF_DB_DATE:
            return dateFromDBDate ( date, cache );
        break;
        case VDF_FILE_DATE:
            return dateFromFilenameDate ( date, cache );
        break;
        case VDF_LONG_DATE:
            return dateFromLongString ( date, cache );
        break;
		case VDF_DROPBOX_DATE:
			return dateFromDropboxDate( date, cache );
		break;
	}
	return *this;
}

vmNumber& vmNumber::dateFromHumanDate ( const QString& date, const bool cache )
{
    nbr_upart[VM_IDX_YEAR] = date.right ( 4 ).toInt ();
    nbr_upart[VM_IDX_MONTH] = date.mid ( 3, 2 ).toInt ();
    nbr_upart[VM_IDX_DAY] = date.left ( 2 ).toInt ();
	setType ( VMNT_DATE );
	nbr_upart[VM_IDX_STRFORMAT] = VDF_HUMAN_DATE;
	if ( cache ) {
		setCached ( true );
		cached_str = date;
	}
	return *this;
}

vmNumber& vmNumber::dateFromDBDate ( const QString& date, const bool cache )
{
	nbr_upart[VM_IDX_YEAR] = date.left ( 4 ).toInt ();
	nbr_upart[VM_IDX_MONTH] = date.mid ( 5, 2 ).toInt ();
	nbr_upart[VM_IDX_DAY] = date.right ( 2 ).toInt ();
	setType ( VMNT_DATE );
	nbr_upart[VM_IDX_STRFORMAT] = VDF_DB_DATE;
	if ( cache ) {
		setCached ( true );
		cached_str = date;
	}
	return *this;
}

vmNumber& vmNumber::dateFromDropboxDate ( const QString& date, const bool cache )
{
	nbr_upart[VM_IDX_YEAR] = date.left ( 4 ).toInt ();
	nbr_upart[VM_IDX_MONTH] = date.mid ( 5, 2 ).toInt ();
	nbr_upart[VM_IDX_DAY] = date.right ( 2 ).toInt ();
	setType ( VMNT_DATE );
	nbr_upart[VM_IDX_STRFORMAT] = VDF_DROPBOX_DATE;
	if ( cache ) {
		setCached ( true );
		cached_str = date;
	}
	return *this;
}

/* Won't try to guess if the date string starts with year or day. Assume it's year 
 * because that's what it should be anyway
 */
vmNumber& vmNumber::dateFromFilenameDate ( const QString& date, const bool cache )
{
	if ( date.length () >= 6 )
	{
		const int year_end_idx ( date.length () > 6 ? 4 : 2 );
		nbr_upart[VM_IDX_YEAR] = date.left ( year_end_idx ).toInt ();
		nbr_upart[VM_IDX_MONTH] = date.mid ( year_end_idx, 2 ).toInt ();
		nbr_upart[VM_IDX_DAY] = date.right ( 2 ).toInt ();
		setType ( VMNT_DATE );
		nbr_upart[VM_IDX_STRFORMAT] = VDF_FILE_DATE;
		if ( cache ) {
			setCached ( true );
			cached_str = date;
		}
	}
	return *this;
}

vmNumber& vmNumber::dateFromLongString ( const QString& date, const bool cache )
{
    const QString strDay ( date.section ( QStringLiteral ( "de" ), 0, 0 ).remove ( CHR_SPACE ) );
    const QString strMonth ( date.section ( QStringLiteral ( "de" ), 1, 1 ).remove ( CHR_SPACE ) );
    const QString strYear ( date.section ( QStringLiteral ( "de" ), 2, 2 ).remove ( CHR_SPACE ) );

	if ( !strDay.isEmpty () ) {
        nbr_upart[VM_IDX_DAY] = strDay.toInt ( &mb_valid );
        if ( !mb_valid ) { // day is not a number, but a word
            if ( strDay.startsWith ( QStringLiteral ( "vi" ) ) ) {
                if ( strDay.contains ( QStringLiteral ( "um" ) ) ) nbr_upart[VM_IDX_DAY] = 21;
                else if ( strDay.contains ( QStringLiteral ( "do" ), Qt::CaseInsensitive ) ) nbr_upart[VM_IDX_DAY] = 22;
                else if ( strDay.contains ( QStringLiteral ( "qu" ), Qt::CaseInsensitive ) ) nbr_upart[VM_IDX_DAY] = 24;
                else if ( strDay.contains ( QStringLiteral ( "tr" ), Qt::CaseInsensitive ) ) nbr_upart[VM_IDX_DAY] = 23;
                else if ( strDay.contains ( QStringLiteral ( "ci" ), Qt::CaseInsensitive ) ) nbr_upart[VM_IDX_DAY] = 25;
                else if ( strDay.contains ( QStringLiteral ( "is" ), Qt::CaseInsensitive ) ) nbr_upart[VM_IDX_DAY] = 26;
                else if ( strDay.contains ( QStringLiteral ( "te" ), Qt::CaseInsensitive ) ) nbr_upart[VM_IDX_DAY] = 27;
                else if ( strDay.contains ( QStringLiteral ( "oi" ), Qt::CaseInsensitive ) ) nbr_upart[VM_IDX_DAY] = 28;
				else nbr_upart[VM_IDX_DAY] = 29;
			}
            else if ( strDay.startsWith ( QStringLiteral ( "prim" ), Qt::CaseInsensitive ) ||
                      strDay.startsWith ( QStringLiteral ( "um" ), Qt::CaseInsensitive ) )
				nbr_upart[VM_IDX_DAY] = 1;
            else if ( strDay.startsWith ( QStringLiteral ( "do" ), Qt::CaseInsensitive ) ) {
                if ( strDay.startsWith ( QStringLiteral ( "doi" ), Qt::CaseInsensitive ) ) nbr_upart[VM_IDX_DAY] = 2;
				else nbr_upart[VM_IDX_DAY] = 12;
			}
            else if ( strDay.startsWith ( QStringLiteral ( "tr" ), Qt::CaseInsensitive ) ) {
                if ( strDay.startsWith ( QStringLiteral ( "treze" ), Qt::CaseInsensitive ) ) nbr_upart[VM_IDX_DAY] = 13;
                else if ( strDay.startsWith ( QStringLiteral ( "tri" ), Qt::CaseInsensitive ) ) {
                    if ( strDay.contains ( QStringLiteral ( "um" ), Qt::CaseInsensitive ) ) nbr_upart[VM_IDX_DAY] = 31;
					else nbr_upart[VM_IDX_DAY] = 30;
				}
				else nbr_upart[VM_IDX_DAY] = 3;
			}
            else if ( strDay.startsWith ( QStringLiteral ( "quat" ), Qt::CaseInsensitive ) ) {
                if ( strDay.startsWith ( QStringLiteral ( "quatro" ), Qt::CaseInsensitive ) ) nbr_upart[VM_IDX_DAY] = 4;
				else nbr_upart[VM_IDX_DAY] = 14;
			}
            else if ( strDay.startsWith ( QStringLiteral ( "ci" ), Qt::CaseInsensitive ) ) nbr_upart[VM_IDX_DAY] = 5;
            else if ( strDay.startsWith ( QStringLiteral ( "sei" ), Qt::CaseInsensitive ) ) nbr_upart[VM_IDX_DAY] = 6;
            else if ( strDay.startsWith ( QStringLiteral ( "set" ), Qt::CaseInsensitive ) ) nbr_upart[VM_IDX_DAY] = 7;
            else if ( strDay.startsWith ( QStringLiteral ( "oi" ), Qt::CaseInsensitive ) ) nbr_upart[VM_IDX_DAY] = 8;
            else if ( strDay.startsWith ( QStringLiteral ( "no" ), Qt::CaseInsensitive ) ) nbr_upart[VM_IDX_DAY] = 9;
            else if ( strDay.startsWith ( QStringLiteral ( "dez" ), Qt::CaseInsensitive ) ) {
                if ( strDay.compare ( QStringLiteral ( "dez" ), Qt::CaseInsensitive ) ) nbr_upart[VM_IDX_DAY] = 10;
                else if ( strDay.contains ( QStringLiteral ( "set" ), Qt::CaseInsensitive ) ) nbr_upart[VM_IDX_DAY] = 17;
                else if ( strDay.contains ( QStringLiteral ( "oit" ), Qt::CaseInsensitive ) ) nbr_upart[VM_IDX_DAY] = 18;
                else if ( strDay.contains ( QStringLiteral ( "nov" ), Qt::CaseInsensitive ) ) nbr_upart[VM_IDX_DAY] = 19;
				else nbr_upart[VM_IDX_DAY] = 16;
			}
            else if ( strDay.startsWith ( QStringLiteral ( "on" ), Qt::CaseInsensitive ) ) nbr_upart[VM_IDX_DAY] = 11;
            else if ( strDay.startsWith ( QStringLiteral ( "qui" ), Qt::CaseInsensitive ) ) nbr_upart[VM_IDX_DAY] = 15;
			else
				nbr_upart[VM_IDX_DAY] = -1;
		}
	}
	else // day was not provided
		nbr_upart[VM_IDX_DAY] = vmNumber::currentDate.day ();

	if ( !strMonth.isEmpty () ) {
        nbr_upart[VM_IDX_MONTH] = strMonth.toInt ( &mb_valid );
        if ( !mb_valid ) { // month is not a number, but a word
            if ( strMonth.startsWith ( QStringLiteral ( "ja" ) ) ) nbr_upart[VM_IDX_MONTH] = 1;
            else if ( strMonth.startsWith ( QStringLiteral ( "fe" ), Qt::CaseInsensitive ) ) nbr_upart[VM_IDX_MONTH] = 2;
            else if ( strMonth.startsWith ( QStringLiteral ( "mar" ), Qt::CaseInsensitive ) ) nbr_upart[VM_IDX_MONTH] = 3;
            else if ( strMonth.startsWith ( QStringLiteral ( "abr" ), Qt::CaseInsensitive ) ) nbr_upart[VM_IDX_MONTH] = 4;
            else if ( strMonth.startsWith ( QStringLiteral ( "mai" ), Qt::CaseInsensitive ) ) nbr_upart[VM_IDX_MONTH] = 5;
            else if ( strMonth.startsWith ( QStringLiteral ( "jun" ), Qt::CaseInsensitive ) ) nbr_upart[VM_IDX_MONTH] = 6;
            else if ( strMonth.startsWith ( QStringLiteral ( "jul" ), Qt::CaseInsensitive ) ) nbr_upart[VM_IDX_MONTH] = 7;
            else if ( strMonth.startsWith ( QStringLiteral ( "ago" ), Qt::CaseInsensitive ) ) nbr_upart[VM_IDX_MONTH] = 8;
            else if ( strMonth.startsWith ( QStringLiteral ( "set" ), Qt::CaseInsensitive ) ) nbr_upart[VM_IDX_MONTH] = 9;
            else if ( strMonth.startsWith ( QStringLiteral ( "out" ), Qt::CaseInsensitive ) ) nbr_upart[VM_IDX_MONTH] = 10;
            else if ( strMonth.startsWith ( QStringLiteral ( "nov" ), Qt::CaseInsensitive ) ) nbr_upart[VM_IDX_MONTH] = 11;
			else nbr_upart[VM_IDX_MONTH] = 12;
		}
	}
	else { // month was not provided
		if ( nbr_upart[VM_IDX_DAY] == 0 ) // str is not a date string at all
			nbr_upart[VM_IDX_MONTH] = 0;
		else
			nbr_upart[VM_IDX_MONTH] = vmNumber::currentDate.month ();
	}

	if ( !strYear.isEmpty () ) {
        nbr_upart[VM_IDX_YEAR] = strYear.toInt ( &mb_valid );
        if ( mb_valid ) {
			if ( nbr_upart[VM_IDX_YEAR] < 30 )
				nbr_upart[VM_IDX_YEAR] += 2000;
		}
		else // year is not a number, but a word
			nbr_upart[VM_IDX_YEAR] = vmNumber::currentDate.year (); // too many possibilities. Eventually I will return to this code
	}
	else { // year was not provided
		if ( nbr_upart[VM_IDX_MONTH] == 0 )
			nbr_upart[VM_IDX_YEAR] = 0;
		else
			nbr_upart[VM_IDX_YEAR] = vmNumber::currentDate.year ();
	}
	if ( nbr_upart[VM_IDX_YEAR] != 0 ) {
		nbr_upart[VM_IDX_STRFORMAT] = VDF_LONG_DATE;
		setType ( VMNT_DATE );
		if ( cache ) {
			setCached ( true );
			cached_str = date;
		}
	}
    mb_valid = false;
	return *this;
}

const QString& vmNumber::toDate ( const VM_DATE_FORMAT format ) const
{
	if ( m_type == VMNT_UNSET )
		const_cast<vmNumber*> ( this )->setType ( VMNT_DATE );

	if ( isDate () ) {
		if ( isCached () && format == nbr_upart[VM_IDX_STRFORMAT] )
			return cached_str;
		else {
            const QString strYear ( year () < 100 ? QStringLiteral ( "20" ) + QString::number ( year () ) : QString::number ( year () ) );
            const QString strDay ( day () < 10 ? QStringLiteral ( "0" ) + QString::number ( day () ) : QString::number ( day () ) );
            const QString strMonth ( month () < 10 ? QStringLiteral ( "0" ) + QString::number ( month () ) : QString::number ( month () ) );
			const_cast<vmNumber*> ( this )->nbr_upart[VM_IDX_STRFORMAT] = format;

			switch ( format ) {
				case VDF_HUMAN_DATE:
					cached_str = strDay + CHR_F_SLASH + strMonth + CHR_F_SLASH + strYear;
				break;
				case VDF_DB_DATE:
					cached_str = strYear + CHR_F_SLASH + strMonth + CHR_F_SLASH + strDay;
				break;
				case VDF_FILE_DATE:
					cached_str = strYear + strMonth + strDay;
				break;
				case VDF_LONG_DATE:
					cached_str = strDay + QLatin1String ( " de " ) + QLatin1String ( MONTHS[nbr_upart[VM_IDX_MONTH]] ) +
							 QLatin1String ( " de " ) + strYear;
				break;
				case VDF_DROPBOX_DATE:
					cached_str = strYear + CHR_HYPHEN + strMonth + CHR_HYPHEN + strDay;
				break;
				break;
			}
			setCached ( true );
			return cached_str;
		}
	}
	return emptyString;
}

void vmNumber::fromQDate ( const QDate& date )
{
	if ( !isDate () ) {
		nbr_upart[VM_IDX_DAY] = nbr_upart[VM_IDX_MONTH] = nbr_upart[VM_IDX_YEAR] = 0;
		setType ( VMNT_DATE );
	}
	nbr_upart[VM_IDX_DAY] = date.day ();
	nbr_upart[VM_IDX_MONTH] = date.month ();
	nbr_upart[VM_IDX_YEAR] = date.year ();
	setCached ( false );
}

const QDate& vmNumber::toQDate () const
{
    if ( isDate () )
	{
        mQDate.setDate ( static_cast<int> ( year () ), static_cast<int> ( month () ), static_cast<int> ( day () ) );
	}
    else
	{
        mQDate.setDate ( 2000, 1, 1 );
	}
	return mQDate;
}

bool vmNumber::isDateWithinRange ( const vmNumber& checkDate, const uint years, const uint months, const uint days ) const
{
	vmNumber tempDate ( checkDate );
	tempDate.setDate ( days, months, years, true );
	if ( *this > tempDate )
		return false;
	tempDate = checkDate;
	tempDate.setDate ( -days, -months, -years, true );
	if ( *this < tempDate )
		return false;
	return true;
}

unsigned int vmNumber::julianDay () const
{
	if ( isDate () ) {
		const unsigned int a ( ( 14 - month () ) / 12 );
		const unsigned int y ( year () + 4800 - a );
		const unsigned int m ( month () + 12 * a - 3 );
		/* This is a business application. A gregorian date is the only date we'll ever need */
		//if ( year () > 1582 || ( year () == 1582 && month () > 10 ) || ( year () == 1582 && month () == 10 && day () >= 15 ) )
		return day () + ( 153 * m + 2 ) / 5 + 365 * y + y / 4 - y / 100 + y / 400 - 32045;
		//else
		//	return day () + ( 153 * m + 2 ) / 5 + 365 * y + y / 4 - 32083;
	}
	return 0;
}

unsigned int vmNumber::dayOfYear () const
{
	unsigned int n ( 0 );
	if ( isDate () ) {
		n = day ();
		for ( uint i ( 1 ); i < month (); ++i )
			n += daysInMonth ( i, year () );
	}
	return n;
}

// Source: http://en.wikipedia.org/wiki/Determination_of_the_day_of_the_week
unsigned int vmNumber::dayOfWeek () const
{
	if ( isDate () ) {
		const unsigned int y ( year () - ( ( year () >= 2000 ) ? 2000 : 1900 ) );
		const unsigned int c ( ( year () >= 2000 ) ? 6 : 0 );
		const unsigned int dow ( static_cast<uint> ( ( day () + months_table[month()] + y + ( y / 4 ) + c ) % 7 ) );
		return dow;
	}
	return 0;
}

// Source: http://en.wikipedia.org/wiki/ISO_week_date
unsigned int vmNumber::weekNumber () const
{
	return ( ( dayOfYear () - dayOfWeek () ) + 10 ) / 7;
}

bool vmNumber::isDateWithinWeek ( const vmNumber& date ) const
{
	vmNumber tempDate ( date );
	tempDate.setDate ( -7, 0, 0, true );
	if ( *this >= tempDate ) {
		tempDate.setDate ( 14, 0 ,0, true );
		return ( *this <= tempDate );
	}
	return false;
}

bool vmNumber::isLeapYear ( const unsigned int year )
{
	return ( ( year % 4 ) == 0 );
}

unsigned int vmNumber::daysInMonth ( const unsigned int month, const unsigned int year )
{
	unsigned int days ( 31 );
	switch ( month ) {
        default:
		break;
        case 2:
            days = isLeapYear ( year ) ? 29 : 28;
		break;
        case 4:
        case 6:
        case 9:
        case 11:
            days = 30;
	}
	return days;
}

void vmNumber::updateCurrentDate ()
{
	const time_t t = time ( 0 );   // get time now
	const struct tm* now = localtime( &t );
	vmNumber::currentDate.setDate ( now->tm_mday, now->tm_mon + 1, now->tm_year + 1900, false );
}
//-------------------------------------DATE------------------------------------------

//-------------------------------------PHONE-----------------------------------------

vmNumber& vmNumber::fromStrPhone ( const QString& phone )
{
	if ( !phone.isEmpty () ) {
		unsigned int chr ( 0 );
		QString phone_str;
		phone_str.reserve ( 12 );
		const unsigned int len ( static_cast<unsigned> ( phone.length () ) );
		do {
			if ( phone.at ( chr ).isDigit () )
				phone_str += phone.at ( chr );
		} while ( ++chr < len );
		const uint new_len ( phone_str.length () );
		if ( new_len >= 2 ) {
			if ( new_len >= 10 ) {
				nbr_upart[VM_IDX_PREFIX] = phone_str.left ( 2 ).toUInt ();
				nbr_upart[VM_IDX_PHONE1] = phone_str.mid ( 2, new_len == 10 ? 4 : 5 ).toUInt ();
				nbr_upart[VM_IDX_PHONE2] = phone_str.right ( 4 ).toUInt ();
			}
			else {
				nbr_upart[VM_IDX_PREFIX] = 19; //default prefix
				chr = 0;
				int tens ( 1000 ), idx ( VM_IDX_PHONE1 );
				do {
					if ( tens >= 1 ) {
						nbr_upart[idx] += phone.at ( chr ).digitValue () * tens;
						tens /= 10;
						++chr;
					}
					else {
						tens = 1000;
						idx = VM_IDX_PHONE2;
					}
				} while ( chr < new_len );
			}
			setType ( VMNT_PHONE );
			setCached ( false );
			return *this;
		}
	}
	clear ();
	return *this;
}

vmNumber& vmNumber::fromTrustedStrPhone ( const QString& phone, const bool cache )
{
	clear ( false );
	nbr_upart[VM_IDX_PREFIX] = retrievePhonePrefix ( phone, true ).toUInt ();
	const QString phone_body ( retrievePhoneBody ( phone, true ) );
	nbr_upart[VM_IDX_PHONE1] = phone_body.left ( phone_body.count () == 8 ? 4 : 5 ).toUInt ();
	nbr_upart[VM_IDX_PHONE2] = phone_body.right ( 4 ).toUInt ();
	setType ( VMNT_PHONE );
	if ( cache ) {
		setCached ( true );
		cached_str = phone;
	}
	return *this;
}

const QString& vmNumber::retrievePhoneBody ( const QString& full_phone, const bool numbers_only ) const
{
	const int idx ( full_phone.indexOf ( CHR_SPACE ) );
	if ( idx != -1 ) {
		mQString = full_phone.mid ( idx + 1, full_phone.length () - idx );
		if ( numbers_only )
			mQString.remove ( CHR_HYPHEN );
		return mQString;
	}
	return emptyString;
}

const QString& vmNumber::retrievePhonePrefix ( const QString& full_phone, const bool numbers_only ) const
{
	const int idx ( full_phone.indexOf ( CHR_R_PARENTHESIS ) );
	if ( idx != -1 ) {
		mQString = full_phone.mid ( ( !numbers_only ? 0 : 1 ), ( !numbers_only ? idx + 1 : idx - 1 ) );
		return mQString;
	}
	return emptyString;
}

void vmNumber::makeCachedStringForPhone () const
{
	cached_str = makePhonePrefix () + CHR_SPACE + makePhoneBody () ;
	setCached ( true );
}

const QString& vmNumber::toPhone () const
{
	switch ( type () )
	{
        case VMNT_INT: // will not even attempt to convert these
        case VMNT_DOUBLE:
        case VMNT_PRICE:
        case VMNT_TIME:
        case VMNT_DATE:
            return emptyString;
		break;
        case VMNT_UNSET:
            const_cast<vmNumber*> ( this )->m_type = VMNT_PHONE;
        case VMNT_PHONE:
            if ( !isCached () )
			{
                makeCachedStringForPhone ();
			}
		break;
	}
	return cached_str;
}

const QString& vmNumber::phoneBody () const
{
	if ( isPhone () )
	{
		if ( !isCached () )
			makeCachedStringForPhone ();
		return retrievePhoneBody ( cached_str );
	}
	return emptyString;
}

const QString& vmNumber::phonePrefix () const
{
	if ( isPhone () )
	{
		if ( !isCached () )
			makeCachedStringForPhone ();
		return retrievePhonePrefix ( cached_str );
	}
	return emptyString;
}

void vmNumber::makePhone ( const QString& prefix, const QString& body, const bool trusted )
{
	if ( trusted )
		fromTrustedStrPhone ( prefix + CHR_SPACE + body );
	else
		fromStrPhone ( prefix + body );
}

const QString vmNumber::makePhoneBody () const
{
	if ( nbr_upart[VM_IDX_PHONE1] < 1000 )
		mQString = CHR_ZERO;
	else if ( nbr_upart[VM_IDX_PHONE1] >= 5000 && nbr_upart[VM_IDX_PHONE1] < 10000 )
		mQString = QStringLiteral ( "9" );
	else
		mQString.clear ();

	mQString += QString::number ( nbr_upart[VM_IDX_PHONE1] ) + CHR_HYPHEN;

	if ( nbr_upart[VM_IDX_PHONE2] < 1000 )
		mQString += CHR_ZERO;
	mQString += QString::number ( nbr_upart[VM_IDX_PHONE2] );

	return mQString;
}

const QString vmNumber::makePhonePrefix () const
{
	mQString = CHR_L_PARENTHESIS + QString::number ( nbr_upart[VM_IDX_PREFIX] ) + CHR_R_PARENTHESIS;
	return mQString;
}
//-------------------------------------PHONE-----------------------------------------

//-------------------------------------PRICE-----------------------------------------
static void formatPrice ( QString& new_price, const int part1, const unsigned int part2 )
{
	new_price = CURRENCY + CHR_SPACE + QString::number ( part1 ) + CHR_COMMA +
				( part2 >= 10 ? QString::number ( part2 ) : QLatin1String ( "0" ) + QString::number ( part2 )
				);
	if ( part1 < 0 ) {
		new_price.remove ( CHR_HYPHEN );
		new_price.prepend ( CHR_L_PARENTHESIS );
		new_price += CHR_R_PARENTHESIS;
	}
}

static void formatPrice ( QString& new_price, const double n )
{
	new_price = CURRENCY + CHR_SPACE + QString::number ( n, 'f', 2 );
	new_price.replace ( CHR_DOT, CHR_COMMA );
	if ( n < 0.0 ) {
		new_price.remove ( CHR_HYPHEN );
		new_price.prepend ( CHR_L_PARENTHESIS );
		new_price += CHR_R_PARENTHESIS;
	}
}

void vmNumber::setPrice ( const int tens, const int cents, const bool update )
{
	setType ( VMNT_PRICE );
	setCached ( false );
	if ( !update ) {
		nbr_part[VM_IDX_TENS] = tens;
		nbr_part[VM_IDX_CENTS] = cents <= 99 ? cents : 0;
	}
	else {
		if ( tens != 0 || cents != 0 ) {
			double price ( static_cast<double> ( nbr_part[VM_IDX_TENS] + static_cast<double> ( nbr_part[VM_IDX_CENTS] / 100.0 ) ) );
			const double other_price ( static_cast<double> ( tens + static_cast<double> ( cents / 100.0 ) ) );
			price += other_price;
			nbr_part[VM_IDX_TENS] = static_cast<int> ( price );
			const double new_cents ( ( price - static_cast<double> ( nbr_part[VM_IDX_TENS] ) ) * 100.111 );
			nbr_part[VM_IDX_CENTS] = static_cast<int> ( new_cents );
		}
	}
}

vmNumber& vmNumber::fromStrPrice ( const QString& price )
{
	if ( !price.isEmpty () ) {
		QString simple_price ( price );
		simple_price.remove ( CHR_SPACE );
		uint chr ( 0 ), idx ( VM_IDX_TENS );
		const uint len ( simple_price.length () );
		QChar qchr;
		bool is_negative ( false );
		nbr_part[VM_IDX_TENS] = nbr_part[VM_IDX_CENTS] = 0;
		nbr_upart[VM_IDX_CENTS] = 0;
		while ( chr < len ) {
			qchr = simple_price.at ( chr );
			if ( qchr.isDigit () ) {
				if ( idx == VM_IDX_CENTS ) {
					if ( nbr_upart[VM_IDX_CENTS] == 0 ) {
						nbr_part[VM_IDX_CENTS] += qchr.digitValue ();
						nbr_part[VM_IDX_CENTS] *= 10;
						++nbr_upart[VM_IDX_CENTS];
					}
					else {
						nbr_part[VM_IDX_CENTS] += qchr.digitValue ();
						break; // up to two digits in the cents
					}
				}
				else {
					nbr_part[VM_IDX_TENS] *= 10;
					nbr_part[VM_IDX_TENS] += qchr.digitValue ();
				}
			}
			else if ( qchr == CHR_DOT || qchr == CHR_COMMA )
				idx = VM_IDX_CENTS;
			else if ( qchr == CHR_HYPHEN || qchr == CHR_L_PARENTHESIS ) {
				if ( nbr_part[VM_IDX_TENS] == 0 ) // before any number, accept those chars, after, it is just garbage
					is_negative = true;
			}
			++chr;
		}
		//if ( nbr_part[VM_IDX_CENTS] > 99 || nbr_part[VM_IDX_CENTS] < 0 )
		//	nbr_part[VM_IDX_CENTS] = 0;
		if ( is_negative )
			nbr_part[VM_IDX_TENS] = 0 - nbr_part[VM_IDX_TENS];
		setType ( VMNT_PRICE );
		setCached ( false );
		return *this;
	}
    clear ( false );
	return *this;
}

vmNumber& vmNumber::fromTrustedStrPrice ( const QString& price, const bool cache )
{
	QString newStrPrice;
	if ( !price.isEmpty () ) {
		clear ( false );
		const int idx_sep ( price.indexOf ( CHR_COMMA ) );
		const int idx_space ( price.indexOf ( CHR_SPACE ) );
		nbr_part[VM_IDX_TENS] = price.mid ( idx_space + 1, idx_sep - idx_space - 1 ).toInt ();
		nbr_part[VM_IDX_CENTS] = price.right ( 2 ).toInt ();
		if ( price.at ( 0 ) == CHR_L_PARENTHESIS ) // negative
			nbr_part[VM_IDX_TENS] = 0 - nbr_part[VM_IDX_TENS];
	}
	else {
		nbr_part[VM_IDX_TENS] = 0;
		nbr_part[VM_IDX_CENTS] = 0;
		formatPrice ( newStrPrice, 0, 0 );
	}
	setType ( VMNT_PRICE );
	if ( cache ) {
		setCached ( true );
		cached_str = newStrPrice.isEmpty () ? price : newStrPrice;
	}
	return *this;
}

const QString& vmNumber::priceToDoubleString () const
{
	if ( isPrice () ) {
		formatPrice ( mQString, nbr_part[VM_IDX_TENS], nbr_part[VM_IDX_CENTS] );
		mQString.replace ( CHR_COMMA, CHR_DOT );
		return mQString;
	}
	return emptyString;
}

const QString& vmNumber::toPrice () const
{
	switch ( type () ) {
        case VMNT_DATE: // will not even attempt to convert these
        case VMNT_PHONE:
        case VMNT_TIME:
            return emptyString;
		break;
        case VMNT_INT:
            formatPrice ( mQString, nbr_part[0], 0 );
		break;
        case VMNT_DOUBLE:
            formatPrice ( mQString, toDouble () );
		break;
        case VMNT_UNSET:
            const_cast<vmNumber*> ( this )->m_type = VMNT_PRICE;
        case VMNT_PRICE:
            if ( !isCached () ) {
                formatPrice ( cached_str, nbr_part[VM_IDX_TENS], nbr_part[VM_IDX_CENTS] );
                setCached ( true );
            }
            return cached_str;
		break;
	}
	return mQString;
}
//-------------------------------------PRICE-----------------------------------------

//-------------------------------------TIME------------------------------------------
static void numberToTime ( const unsigned int n, int &hours, unsigned int &minutes )
{
	unsigned int abs_hour ( 0 );

	abs_hour = n / 60;
	minutes = n % 60;
	if ( minutes > 59 ) {
		++abs_hour;
		minutes = 0;
	}
	if ( abs_hour > 9999 )
		hours = 9999;
	else if ( hours < 0 )
		hours = 0 - abs_hour;
	else
		hours = signed ( abs_hour );
}

/*void getTimeFromFancyTimeString ( const QString& str_time, vmNumber& time, const bool check = false )
{

}*/

void vmNumber::setTime ( const int hours, const int minutes, const bool update )
{
	setType ( VMNT_TIME );
	setCached ( false );
	if ( !update ) {
		nbr_part[VM_IDX_HOUR] = ( hours > -10000 && hours < 10000 ) ? hours : 0;
		nbr_part[VM_IDX_MINUTE] = ( minutes >= 0 && minutes <= 59 ) ? minutes : 0;
	}
	else {
		int n ( 0 );
		nbr_part[VM_IDX_MINUTE] += minutes;
		if ( nbr_part[VM_IDX_MINUTE] > 59 ) {
			n = static_cast<int> ( nbr_part[VM_IDX_MINUTE] / 60 );
			nbr_part[VM_IDX_HOUR] += n;
			nbr_part[VM_IDX_MINUTE] -= ( n * 60 );
		}
		else if ( nbr_part[VM_IDX_MINUTE] < 0 ) {
			n = static_cast<int> ( ::fabs ( static_cast<double> ( nbr_part[VM_IDX_MINUTE] ) / 60 ) ) + 1;
			nbr_part[VM_IDX_HOUR] -= n;
			nbr_part[VM_IDX_MINUTE] += ( n * 60 );
		}

		if ( hours >= 1 ) {
			if ( ( nbr_part[VM_IDX_HOUR] + hours ) < 10000 )
				nbr_part[VM_IDX_HOUR] += hours;
		}
		else if ( hours <= -1 ) {
			if ( ( nbr_part[VM_IDX_HOUR] + hours ) > -10000 )
				nbr_part[VM_IDX_HOUR] += hours;
		}
	}
}

vmNumber& vmNumber::fromStrTime ( const QString& time )
{
	if ( !time.isEmpty () ) {
		bool is_negative ( false );
		const int idx ( time.indexOf ( CHR_COLON ) );
		int hours ( 0 ), mins ( 0 );
		if ( idx != -1 ) {
			QString temp_time ( time );
			if ( temp_time.remove ( CHR_SPACE ).startsWith ( CHR_HYPHEN ) ) {
				is_negative = true;
				temp_time.remove ( CHR_HYPHEN );
			}
			hours = temp_time.left ( idx ).toInt ();
			mins = temp_time.mid ( idx + 1, 2 ).toInt ();

			nbr_part[VM_IDX_HOUR] = !is_negative ? hours : 0 - hours;
			nbr_part[VM_IDX_MINUTE] = mins;
			setType ( VMNT_TIME );
			setCached ( false );
			return *this;
		}
		//else
		//	getTimeFromFancyTimeString ( time, mQString, true );
	}
	clear ();
	return *this;
}

vmNumber& vmNumber::fromTrustedStrTime ( const QString& time, const VM_TIME_FORMAT format, const bool cache )
{
	clear ( false );
	switch ( format ) {
        case VTF_24_HOUR:
            nbr_part[VM_IDX_HOUR] = time.left ( 2 ).toInt ();
            nbr_part[VM_IDX_MINUTE] = time.right ( 2 ).toInt ();
		break;
        case VTF_DAYS:
            nbr_part[VM_IDX_HOUR] = time.left ( 4 ).toInt ();
            nbr_part[VM_IDX_MINUTE] = time.right ( 2 ).toInt ();
		break;
        case VTF_FANCY:
            //getTimeFromFancyTimeString ( time, mQString );
		break;
	}
	nbr_part[VM_IDX_STRFORMAT] = format;
	setType ( VMNT_TIME );
	if ( cache ) {
		setCached ( true );
		cached_str = time;
	}
	return *this;
}

void vmNumber::fromQTime ( const QTime &time )
{
	if ( !isTime () ) {
		//::memset ( nbr_part, 0, sizeof ( nbr_part ) );
		nbr_upart[VM_IDX_HOUR] = nbr_upart[VM_IDX_MINUTE] = 0;
		setType ( VMNT_TIME );
	}
	nbr_part[VM_IDX_HOUR] = time.hour ();
	nbr_part[VM_IDX_MINUTE] = time.minute ();
	setCached ( false );
}

const QString& vmNumber::toTime ( const VM_TIME_FORMAT format ) const
{
	int hours ( 0 );
	unsigned int minutes ( 0 );
	switch ( type () ) {
        case VMNT_PRICE: // will not even attempt to convert these
        case VMNT_DATE:
        case VMNT_PHONE:
            return emptyString;
		break;
        case VMNT_INT:
            numberToTime ( static_cast<unsigned int> ( nbr_part[0] ), hours, minutes );
        case VMNT_DOUBLE:
            numberToTime ( static_cast<unsigned int> ( toDouble () ), hours, minutes );
		break;
        case VMNT_UNSET:
            const_cast<vmNumber*> ( this )->m_type = VMNT_TIME;
        case VMNT_TIME:
            if ( !isCached () || format != nbr_part[VM_IDX_STRFORMAT] ) {
                cached_str = formatTime ( nbr_part[VM_IDX_HOUR], nbr_part[VM_IDX_MINUTE], format );
                setCached ( true );
                const_cast<vmNumber*> ( this )->nbr_part[VM_IDX_STRFORMAT] = format;
            }
            return cached_str;
		break;
	}
	return formatTime ( hours, minutes, format );
}

const QTime& vmNumber::toQTime () const
{
	if ( isTime () )
		mQTime.setHMS ( hours (), minutes (), 0 );
	else
		mQTime.setHMS ( 0, 0, 0 );
	return mQTime;
}

const QString& vmNumber::formatTime ( const int hour, const unsigned int min, const VM_TIME_FORMAT format ) const
{
	unsigned int abs_hour ( ::fabs ( static_cast<double> ( hour ) ) );
	abs_hour +=unsigned ( ( min / 60 ) );
	const unsigned int abs_min ( min - unsigned ( ( min / 60 ) ) * 60 );
	QString str_hour, str_min;
	str_hour.setNum ( abs_hour );
	str_min.setNum ( abs_min );
	if ( abs_min < 10 && format != VTF_FANCY )
		str_min.prepend ( CHR_ZERO );

	switch ( format ) {
        case VTF_24_HOUR:
            if ( abs_hour > 24 )
                str_hour = QStringLiteral ( "23" );
            else if ( abs_hour < 10 )
                str_hour.prepend ( CHR_ZERO );
            mQString = str_hour + CHR_COLON + str_min;
		break;
        case VTF_DAYS:
            if ( abs_hour < 10 )
                str_hour.prepend ( QStringLiteral ( "000" ) );
            else if ( abs_hour < 100 )
                str_hour.prepend ( QStringLiteral ( "00" ) );
            else if ( abs_hour < 1000 )
                str_hour.prepend ( CHR_ZERO );
            mQString = str_hour + CHR_COLON + str_min;
		break;
        case VTF_FANCY:
            if ( abs_hour >= 24 )
			{
                const unsigned int abs_days ( abs_hour / 24 );
                abs_hour %= 24;
                str_hour.setNum ( abs_hour );
                mQString.setNum ( abs_days );
                mQString += QLatin1String ( " days, ") + str_hour + QLatin1String ( " hours, and ")
                        + str_min + QLatin1String ( " minutes" );
            }
            else
			{
                mQString = str_hour + QLatin1String ( " hours, and ") + str_min + QLatin1String ( " minutes" );
			}
		break;
	}

	if ( hour < 0 ) {
		if ( format != VTF_FANCY )
			mQString.prepend ( CHR_HYPHEN );
		else
            mQString.prepend ( QStringLiteral ( "Negative " ) );
	}
	return mQString;
}
//-------------------------------------TIME------------------------------------------

//-------------------------------------OPERATORS------------------------------------------
const vmNumber& vmNumber::operator= ( const vmNumber& vmnumber )
{
	if ( this != &vmnumber )
		copy ( *this, vmnumber );
	return *this;
}

vmNumber& vmNumber::operator= ( const QDate& date )
{
	( void )fromQDate ( date );
	return *this;
}

vmNumber& vmNumber::operator= ( const int n )
{
	( void )fromInt ( n );
	return *this;
}

vmNumber& vmNumber::operator= ( const unsigned int n )
{
	( void )fromUInt ( n );
	return *this;
}

vmNumber& vmNumber::operator= ( const double n )
{
	( void )fromDoubleNbr ( n );
	return *this;
}

bool vmNumber::operator== ( const vmNumber& vmnumber ) const
{
	if ( m_type == vmnumber.m_type ) {
		switch ( type () ) {
            case VMNT_UNSET:
                return true;
            case VMNT_INT:
                return nbr_part[0] == vmnumber.nbr_part[0];
            case VMNT_DOUBLE:
            case VMNT_TIME:
            case VMNT_PRICE:
                return ( nbr_part[0] == vmnumber.nbr_part[0] && nbr_part[1] == vmnumber.nbr_part[1] );
            default:
                return ( nbr_upart[0] == vmnumber.nbr_upart[0] && nbr_upart[1] == vmnumber.nbr_upart[1]
					 && nbr_upart[2] == vmnumber.nbr_upart[2] );
		}
	}
	return false;
}

bool vmNumber::operator== ( const int number ) const
{
	switch ( type () ) {
        default:
            return false;
        case VMNT_INT:
            return nbr_part[0] == number;
        case VMNT_DOUBLE:
        case VMNT_TIME:
        case VMNT_PRICE:
            return nbr_part[0] == number && nbr_part[1] == 0;
	}
}

bool vmNumber::operator!= ( const vmNumber& vmnumber ) const
{
	return !operator== ( vmnumber );
}

bool vmNumber::operator>= ( const vmNumber& vmnumber ) const
{
	bool ret ( false );
	switch ( type () ) {
        case VMNT_INT:
            switch ( vmnumber.m_type ) {
                case VMNT_INT:
                case VMNT_DOUBLE:
                case VMNT_PRICE:
                case VMNT_TIME:
                    ret = nbr_part[0] >= vmnumber.nbr_part[0];
                break;
                case VMNT_DATE:
                    ret = nbr_part[0] >= static_cast<int> ( vmnumber.nbr_upart[0] );
                break;
                default:
                break;
            }
		break;
        case VMNT_DOUBLE:
        case VMNT_PRICE:
        case VMNT_TIME:
		switch ( vmnumber.m_type ) {
            case VMNT_INT:
                ret = nbr_part[0] >= vmnumber.nbr_part[0];
			break;
            case VMNT_PHONE:
            case VMNT_DATE:
                if ( nbr_part[0] == static_cast<int> ( vmnumber.nbr_upart[0] ) )
                    ret = nbr_part[1] >= static_cast<int> ( vmnumber.nbr_upart[1] );
                else
                    ret = nbr_part[0] > static_cast<int> ( vmnumber.nbr_upart[0] );
			break;
            case VMNT_DOUBLE:
            case VMNT_PRICE:
            case VMNT_TIME:
                if ( nbr_part[0] >= vmnumber.nbr_part[0] ) {
                    if ( nbr_part[0] == vmnumber.nbr_part[0] )
                        ret = nbr_part[1] >= vmnumber.nbr_part[1];
                    else
                        ret = true;
                }
            break;
            default:
			break;
		}
		break;
        case VMNT_DATE:
        case VMNT_PHONE:
            switch ( vmnumber.m_type ) {
                case VMNT_DATE:
                case VMNT_PHONE:
                    if ( nbr_upart[2] >= vmnumber.nbr_upart[2] ) {
                        if ( nbr_upart[2] == vmnumber.nbr_upart[2] ) {
                            if ( nbr_upart[1] >= vmnumber.nbr_upart[1] ) {
                                if ( nbr_upart[1] == vmnumber.nbr_upart[1] )
                                    ret = nbr_upart[0] >= vmnumber.nbr_upart[0];
                                else // month is greater than
                                    ret = true;
                            }
                        }
                        else // year is greater than
                            ret = true;
                    }
                break;
                default:
                break;
            }
		break;
        default:
		break;
	}
	return ret;
}

bool vmNumber::operator<= ( const vmNumber& vmnumber ) const
{
	bool ret ( false );
    switch ( type () ) {
        case VMNT_INT:
            switch ( vmnumber.m_type ) {
                case VMNT_INT:
                case VMNT_DOUBLE:
                case VMNT_PRICE:
                case VMNT_TIME:
                    ret = nbr_part[0] <= vmnumber.nbr_part[0];
                break;
                case VMNT_DATE:
                    ret = nbr_part[0] <= static_cast<int> ( vmnumber.nbr_upart[0] );
                break;
                default:
                break;
            }
        break;
        case VMNT_DOUBLE:
        case VMNT_PRICE:
        case VMNT_TIME:
            switch ( vmnumber.m_type ) {
                case VMNT_INT:
                    ret = nbr_part[0] <= vmnumber.nbr_part[0];
                break;
                case VMNT_PHONE:
                case VMNT_DATE:
                    if ( nbr_part[0] == static_cast<int> ( vmnumber.nbr_upart[0] ) )
                        ret = nbr_part[1] <= static_cast<int> ( vmnumber.nbr_upart[1] );
                    else
                        ret = nbr_part[0] < static_cast<int> ( vmnumber.nbr_upart[0] );
                break;
                case VMNT_DOUBLE:
                case VMNT_PRICE:
                case VMNT_TIME:
                    if ( nbr_part[0] <= vmnumber.nbr_part[0] ) {
                        if ( nbr_part[0] == vmnumber.nbr_part[0] )
                            ret = nbr_part[1] <= vmnumber.nbr_part[1];
                        else
                            ret = true;
                    }
                break;
                default:
                break;
            }
		break;
        case VMNT_DATE:
        case VMNT_PHONE:
            switch ( vmnumber.m_type ) {
                case VMNT_DATE:
                case VMNT_PHONE:
                    if ( nbr_upart[2] <= vmnumber.nbr_upart[2] ) {
                        if ( nbr_upart[2] == vmnumber.nbr_upart[2] ) {
                            if ( nbr_upart[1] <= vmnumber.nbr_upart[1] ) {
                                if ( nbr_upart[1] == vmnumber.nbr_upart[1] )
                                    ret = nbr_upart[0] <= vmnumber.nbr_upart[0];
                                else // month is less than
                                    ret = true;
                            }
                        }
                        else // year is less than
                            ret = true;
                    }
                break;
                default:
                break;
            }
		break;
        default:
		break;
	}
	return ret;
}

bool vmNumber::operator< ( const vmNumber& vmnumber ) const
{
	bool ret ( false );
	switch ( type () ) {
        case VMNT_INT:
            switch ( vmnumber.m_type ) {
                case VMNT_INT:
                case VMNT_DOUBLE:
                case VMNT_PRICE:
                case VMNT_TIME:
                    ret = nbr_part[0] < vmnumber.nbr_part[0];
                break;
                case VMNT_DATE:
                    ret = nbr_part[0] < static_cast<int> ( vmnumber.nbr_upart[0] );
                break;
                default:
                break;
            }
        break;
        case VMNT_DOUBLE:
        case VMNT_PRICE:
        case VMNT_TIME:
            switch ( vmnumber.m_type ) {
                case VMNT_INT:
                    ret = nbr_part[0] < vmnumber.nbr_part[0];
                break;
                case VMNT_PHONE:
                case VMNT_DATE:
                    if ( nbr_part[0] == static_cast<int> ( vmnumber.nbr_upart[0] ) )
                        ret = nbr_part[1] < static_cast<int> ( vmnumber.nbr_upart[1] );
                    else
                        ret = nbr_part[0] < static_cast<int> ( vmnumber.nbr_upart[0] );
                break;
                case VMNT_DOUBLE:
                case VMNT_PRICE:
                case VMNT_TIME:
                    if ( nbr_part[0] == vmnumber.nbr_part[0] )
                        ret = nbr_part[1] < vmnumber.nbr_part[1];
                    else
                        ret = nbr_part[0] < vmnumber.nbr_part[0];
                break;
                default:
                break;
            }
		break;
        case VMNT_DATE:
        case VMNT_PHONE:
            switch ( vmnumber.m_type ) {
                case VMNT_DATE:
                case VMNT_PHONE:
                    if ( nbr_upart[2] <= vmnumber.nbr_upart[2] ) {
                        if ( nbr_upart[2] == vmnumber.nbr_upart[2] ) {
                            if ( nbr_upart[1] <= vmnumber.nbr_upart[1] ) {
                                if ( nbr_upart[1] == vmnumber.nbr_upart[1] )
                                    ret = nbr_upart[0] < vmnumber.nbr_upart[0];
                                else // month is less than
                                    ret = true;
                            }
                        }
                        else // year is less than
                            ret = true;
                    }
                break;
                default:
                break;
            }
		break;
        default:
		break;
	}
	return ret;
}

bool vmNumber::operator< ( const int number ) const
{
	switch ( type () ) {
        default:
            return nbr_part[0] < number;
        case VMNT_PHONE:
        case VMNT_DATE:
            return static_cast<int> ( nbr_upart[0] ) < number;
	}
}

bool vmNumber::operator> ( const int number ) const
{
	switch ( type () ) {
        default:
            return nbr_part[0] > number;
        case VMNT_PHONE:
        case VMNT_DATE:
            return static_cast<int> ( nbr_upart[0] ) > number;
	}
}

bool vmNumber::operator> ( const vmNumber& vmnumber ) const
{
	bool ret ( false );
	switch ( type () ) {
        case VMNT_INT:
            switch ( vmnumber.m_type ) {
                case VMNT_INT:
                case VMNT_DOUBLE:
                case VMNT_PRICE:
                case VMNT_TIME:
                    ret = nbr_part[0] > vmnumber.nbr_part[0];
                break;
                case VMNT_DATE:
                    ret = nbr_part[0] > static_cast<int> ( vmnumber.nbr_upart[0] );
                break;
                default:
                break;
            }
		break;
        case VMNT_DOUBLE:
        case VMNT_PRICE:
        case VMNT_TIME:
            switch ( vmnumber.m_type ) {
                case VMNT_INT:
                    ret = nbr_part[0] > vmnumber.nbr_part[0];
                break;
                case VMNT_PHONE:
                case VMNT_DATE:
                    if ( nbr_part[0] == static_cast<int> ( vmnumber.nbr_upart[0] ) )
                        ret = nbr_part[1] > static_cast<int> ( vmnumber.nbr_upart[1] );
                    else
                        ret = nbr_part[0] > static_cast<int> ( vmnumber.nbr_upart[0] );
                break;
                case VMNT_DOUBLE:
                case VMNT_PRICE:
                case VMNT_TIME:
                    if ( nbr_part[0] == vmnumber.nbr_part[0] )
                        ret = nbr_part[1] > vmnumber.nbr_part[1];
                    else
                        ret = nbr_part[0] > vmnumber.nbr_part[0];
                break;
                default:
                break;
            }
		break;
        case VMNT_DATE:
        case VMNT_PHONE:
            switch ( vmnumber.m_type ) {
                case VMNT_DATE:
                case VMNT_PHONE:
                    if ( nbr_upart[2] >= vmnumber.nbr_upart[2] ) {
                        if ( nbr_upart[2] == vmnumber.nbr_upart[2] ) {
                            if ( nbr_upart[1] >= vmnumber.nbr_upart[1] ) {
                                if ( nbr_upart[1] == vmnumber.nbr_upart[1] )
                                    ret = nbr_upart[0] > vmnumber.nbr_upart[0];
                                else // month is greater than
                                    ret = true;
                            }
                        }
                        else // year is greater than
                            ret = true;
                    }
                break;
                default:
                break;
            }
		break;
        default:
		break;
	}
	return ret;
}

vmNumber& vmNumber::operator-= ( const vmNumber& vmnumber )
{
	if ( vmnumber.isNull () )
		return *this;

	mb_cached = false;
	cached_str.clear ();
	if ( m_type == VMNT_UNSET )
		m_type = vmnumber.m_type;

	switch ( type () ) {
        case VMNT_INT:
            switch ( vmnumber.m_type ) {
                case VMNT_INT:
                case VMNT_DOUBLE:
                case VMNT_PRICE:
                case VMNT_TIME:
                    nbr_part[0] -= vmnumber.nbr_part[0];
                break;
                case VMNT_DATE:
                    nbr_part[0] -= static_cast<int> ( vmnumber.nbr_upart[0] );
                break;
                default:
                break;
            }
		break;
        case VMNT_DOUBLE:
            switch ( vmnumber.m_type ) {
                case VMNT_INT:
                case VMNT_TIME:
                    nbr_part[0] -= vmnumber.nbr_part[0];
                break;
                case VMNT_DATE:
                    nbr_part[0] -= static_cast<int> ( vmnumber.nbr_upart[0] );
                break;
                case VMNT_DOUBLE:
                case VMNT_PRICE:
                    nbr_part[0] -= vmnumber.nbr_part[0];
                    nbr_part[1] -= vmnumber.nbr_part[1];
                    if ( nbr_part[1] > 0 ) {
                        if ( nbr_part[0] < 0 )
                            ++nbr_part[0];
                    }
                    else if ( nbr_part[1] < 0 ) {
                        if ( nbr_part[0] > 0 )
                            --nbr_part[0];
                        nbr_part[1] = 0 - nbr_part[1]; // decimal part cannot be negative
                    }
                break;
                default:
                break;
            }
		break;
        case VMNT_PRICE:
            switch ( vmnumber.m_type ) {
                case VMNT_INT:
                case VMNT_TIME:
                    nbr_part[0] -= vmnumber.nbr_part[0];
                break;
                case VMNT_DATE:
                    nbr_part[0] -= static_cast<int> ( vmnumber.nbr_upart[0] );
                break;
                case VMNT_DOUBLE:
                case VMNT_PRICE:
                    setPrice ( 0 - vmnumber.nbr_part[0], 0 - vmnumber.nbr_part[1], true );
                break;
                default:
                break;
            }
        break;
        case VMNT_TIME:
            switch ( vmnumber.m_type ) {
                case VMNT_INT:
                case VMNT_DOUBLE:
                case VMNT_PRICE:
                    setTime ( 0 - vmnumber.nbr_part[VM_IDX_HOUR], -1, true );
                break;
                case VMNT_TIME:
                    setTime ( 0 - vmnumber.nbr_part[VM_IDX_HOUR], 0 - vmnumber.nbr_part[VM_IDX_MINUTE], true );
                break;
                default:
                break;
            }
		break;
        case VMNT_DATE:
            switch ( vmnumber.m_type ) {
            case VMNT_DATE:
                setDate ( 0 - vmnumber.nbr_upart[0], 0 - vmnumber.nbr_upart[1], 0 - vmnumber.nbr_upart[2], true );
			break;
            default:
            break;
		}
		break;
        default:
		break;
	}
	return *this;
}

vmNumber& vmNumber::operator+= ( const vmNumber& vmnumber )
{
	if ( vmnumber.isNull () )
		return *this;

	mb_cached = false;
	cached_str.clear ();
	if ( m_type == VMNT_UNSET )
		m_type = vmnumber.m_type;

	switch ( type () ) {
        case VMNT_INT:
            switch ( vmnumber.m_type ) {
                case VMNT_INT:
                case VMNT_DOUBLE:
                case VMNT_PRICE:
                case VMNT_TIME:
                    nbr_part[0] += vmnumber.nbr_part[0];
                break;
                case VMNT_DATE:
                    nbr_part[0] += static_cast<int> ( vmnumber.nbr_upart[0] );
                break;
                default:
                break;
            }
		break;
        case VMNT_DOUBLE:
            switch ( vmnumber.m_type ) {
                case VMNT_INT:
                case VMNT_TIME:
                    nbr_part[0] += vmnumber.nbr_part[0];
                break;
                case VMNT_DATE:
                    nbr_part[0] += static_cast<int> ( vmnumber.nbr_upart[0] );
                break;
                case VMNT_DOUBLE:
                case VMNT_PRICE:
                    nbr_part[0] += vmnumber.nbr_part[0];
                    nbr_part[1] += vmnumber.nbr_part[1];
                break;
                default:
                break;
            }
		break;
        case VMNT_PRICE:
            switch ( vmnumber.m_type ) {
                case VMNT_INT:
                case VMNT_TIME:
                    nbr_part[0] += vmnumber.nbr_part[0];
                break;
                case VMNT_DATE:
                    nbr_part[0] += static_cast<int> ( vmnumber.nbr_upart[0] );
                break;
                case VMNT_DOUBLE:
                case VMNT_PRICE:
                    setPrice ( vmnumber.nbr_part[VM_IDX_TENS], vmnumber.nbr_part[VM_IDX_CENTS], true );
                break;
                default:
                break;
            }
		break;
        case VMNT_TIME:
            switch ( vmnumber.m_type ) {
                case VMNT_INT:
                case VMNT_DOUBLE:
                case VMNT_PRICE:
                    setTime ( vmnumber.nbr_part[VM_IDX_HOUR], -1, true );
                break;
                case VMNT_TIME:
                case VMNT_DATE:
                    setTime ( vmnumber.nbr_part[VM_IDX_HOUR], vmnumber.nbr_part[VM_IDX_MINUTE], true );
                break;
                default:
                break;
            }
		break;
        case VMNT_DATE:
            switch ( vmnumber.m_type ) {
                case VMNT_DATE:
                    setDate ( vmnumber.nbr_upart[0], vmnumber.nbr_upart[1], vmnumber.nbr_upart[2], true );
                break;
                default:
                break;
            }
		break;
        default:
		break;
	}
	return *this;
}

vmNumber& vmNumber::operator+= ( const double number )
{
	if ( number == 0.0 )
		return *this;

	mb_cached = false;
	cached_str.clear ();

	switch ( type () ) {
        case VMNT_INT:
            nbr_part[0] += static_cast<int> ( number );
		break;
        case VMNT_DOUBLE:
        case VMNT_PRICE:
        {
            const int tens ( static_cast<int> ( number ) );
            const double cents ( ( number - static_cast<double> ( number ) ) * 100.111 );
            setPrice ( tens, static_cast<int> ( cents ), true );
        }
        break;
        case VMNT_TIME:
            setTime ( 10000, number, true );
		break;
        case VMNT_DATE:
            setDate ( number, 0, 0, true );
		break;
        default:
		break;
	}
	return *this;
}

vmNumber& vmNumber::operator/= ( const vmNumber& vmnumber )
{
	if ( vmnumber.isNull () )
		return *this;

	mb_cached = false;
	cached_str.clear ();

	switch ( type () ) {
        case VMNT_INT:
            switch ( vmnumber.m_type ) {
                case VMNT_INT:
                case VMNT_DOUBLE:
                case VMNT_PRICE:
                case VMNT_TIME:
                    if ( vmnumber.nbr_part[0] != 0 ) {
                        setType ( vmnumber.m_type );
                        nbr_part[0] /= vmnumber.nbr_part[0];
                        nbr_part[1] = vmnumber.nbr_part[1];
                        nbr_part[2] = vmnumber.nbr_part[2];
                    }
                break;
                case VMNT_DATE: // full convertion
                    if ( vmnumber.nbr_upart[0] != 0 ) {
                        setType ( vmnumber.m_type );
                        nbr_upart[0] /= vmnumber.nbr_upart[0];
                        nbr_upart[1] = vmnumber.nbr_upart[1];
                        nbr_upart[2] = vmnumber.nbr_upart[2];
                    }
                break;
                default:
                break;
            }
		break;
        case VMNT_DOUBLE:
        case VMNT_PRICE:
            switch ( vmnumber.m_type ) {
                case VMNT_INT:
                case VMNT_TIME:
                    if ( vmnumber.nbr_part[0] != 0 )
                        nbr_part[0] /= vmnumber.nbr_part[0];
                break;
                case VMNT_DATE:
                    if ( ( vmnumber.nbr_upart[0] ) != 0 && ( vmnumber.nbr_upart[1] != 0 ) ) {
                        nbr_part[0] /= static_cast<int> ( vmnumber.nbr_upart[0] );
                        nbr_part[1] /= static_cast<int> ( vmnumber.nbr_upart[1] );
                    }
                break;
                case VMNT_DOUBLE:
                    if ( ( vmnumber.nbr_part[0] ) != 0 && ( vmnumber.nbr_part[1] != 0 ) )
                        fromTrustedStrDouble ( useCalc ( vmnumber, *this, QLatin1String ( " / " ) ) );
                break;
                case VMNT_PRICE:
                    if ( ( vmnumber.nbr_part[0] ) != 0 && ( vmnumber.nbr_part[1] != 0 ) )
                        fromStrPrice ( useCalc ( vmnumber, *this, QLatin1String ( " / " ) ) );
                break;
                default:
                break;
            }
		break;
        case VMNT_TIME:
            switch ( vmnumber.m_type ) {
                case VMNT_INT:
                case VMNT_DOUBLE:
                case VMNT_PRICE:
                case VMNT_TIME:
                    setTime ( 10000, static_cast<int> (
						  ( nbr_part[VM_IDX_HOUR] + nbr_part[VM_IDX_MINUTE] ) / ( vmnumber.nbr_part[VM_IDX_HOUR] + vmnumber.nbr_part[VM_IDX_MINUTE] ) ) );
                break;
                default:
                break;
            }
		break;
        default:
		break;
	}
	return *this;
}

vmNumber& vmNumber::operator/= ( const int number )
{
	if ( number == 0 )
		return *this;

	mb_cached = false;
	cached_str.clear ();

	switch ( type () ) {
        case VMNT_INT:
            nbr_part[0] /= number;
		break;
        case VMNT_DOUBLE:
            fromTrustedStrDouble ( useCalc ( vmNumber ( number ), *this, QLatin1String ( " / " ) ) );
		break;
        case VMNT_PRICE:
            fromStrPrice ( useCalc ( vmNumber ( number ), *this, QLatin1String ( " / " ) ) );
		break;
        case VMNT_TIME:
            setTime ( 10000, static_cast<int> (
					  ( nbr_part[VM_IDX_HOUR] + nbr_part[VM_IDX_MINUTE] ) / number ) );
		break;
        case VMNT_DATE:
            setDate ( static_cast<int> ( nbr_part[VM_IDX_DAY] / number ),
				  static_cast<int> ( nbr_part[VM_IDX_MONTH] / number ), static_cast<int> ( nbr_part[VM_IDX_YEAR] / number ) );
		break;
        default:
		break;
	}
	return *this;
}

vmNumber& vmNumber::operator*= ( const vmNumber& vmnumber )
{
	mb_cached = false;
	cached_str.clear ();
	if ( m_type == VMNT_UNSET )
		m_type = vmnumber.m_type;

	switch ( type () ) {
        case VMNT_INT:
            switch ( vmnumber.m_type ) {
                case VMNT_INT:
                case VMNT_DOUBLE:
                case VMNT_PRICE:
                case VMNT_TIME:
                    setType ( vmnumber.m_type );
                    nbr_part[0] *= vmnumber.nbr_part[0];
                    nbr_part[1] = vmnumber.nbr_part[1];
                    nbr_part[2] = vmnumber.nbr_part[2];
                break;
                case VMNT_DATE: // full convertion
                    setType ( vmnumber.m_type );
                    nbr_upart[0] *= vmnumber.nbr_upart[0];
                    nbr_upart[1] = vmnumber.nbr_upart[1];
                    nbr_upart[2] = vmnumber.nbr_upart[2];
                break;
                default:
                break;
            }
		break;
        case VMNT_DOUBLE:
        case VMNT_PRICE:
            switch ( vmnumber.type () ) {
                case VMNT_INT:
                case VMNT_TIME:
                    nbr_part[0] *= vmnumber.nbr_part[0];
                break;
                case VMNT_DATE:
                    nbr_part[0] *= static_cast<int> ( vmnumber.nbr_upart[0] );
                    nbr_part[1] *= static_cast<int> ( vmnumber.nbr_upart[1] );
                break;
                case VMNT_PRICE:
                    fromStrPrice ( useCalc ( vmnumber, *this, QLatin1String ( " / " ) ) );
                break;
                case VMNT_DOUBLE:
                    fromTrustedStrDouble ( useCalc ( vmnumber, *this, QLatin1String ( " / " ) ) );
                break;
                default:
                break;
            }
		break;
        case VMNT_TIME:
            switch ( vmnumber.type () ) {
                case VMNT_INT:
                case VMNT_DOUBLE:
                case VMNT_PRICE:
                case VMNT_TIME:
                    setTime ( 10000, static_cast<int> (
						  ( nbr_part[VM_IDX_HOUR] * nbr_part[VM_IDX_MINUTE] ) + ( vmnumber.nbr_part[VM_IDX_HOUR] * vmnumber.nbr_part[VM_IDX_MINUTE] ) ) );
                break;
                default:
                break;
            }
		break;
        default:
		break;
	}
	return *this;
}

vmNumber& vmNumber::operator*= ( const int number )
{
	mb_cached = false;
	cached_str.clear ();

	switch ( type () ) {
        case VMNT_INT:
            nbr_part[0] *= number;
		break;
        case VMNT_PRICE:
            fromStrPrice ( useCalc ( vmNumber ( number ), *this, QLatin1String ( " * " ) ) );
		break;
        case VMNT_DOUBLE:
            fromTrustedStrDouble ( useCalc ( vmNumber ( number ), *this, QLatin1String ( " * " ) ) );
		break;
        case VMNT_DATE:
            setDate ( nbr_upart[VM_IDX_DAY] * number,
				  nbr_upart[VM_IDX_MONTH] * number, nbr_upart[VM_IDX_YEAR] * number );
		break;
        case VMNT_TIME:
            setTime ( nbr_part[VM_IDX_HOUR] * number, nbr_part[VM_IDX_MINUTE] * number );
		break;
        default:
		break;
	}
	return *this;
}

/*  In the following operator () functions, the type of the returned instance is set accordingly to
	an implicit rule of type precedence. Basic numeric types have lower precedence than complex types.
	The precedence order is thus:
	( int = uint ) < double < the rest
*/

vmNumber vmNumber::operator- ( const vmNumber& vmnumber ) const
{
	if ( vmnumber.isNull () )
		return *this;

	vmNumber ret ( *this );
	ret.setCached ( false );

	switch ( ret.m_type ) {
        case VMNT_INT:
            switch ( vmnumber.m_type ) {
                case VMNT_INT:
                case VMNT_DOUBLE:
                case VMNT_PRICE:
                case VMNT_TIME:
                    ret.m_type = vmnumber.m_type;
                    ret.nbr_part[0] -= vmnumber.nbr_part[0];
                    ret.nbr_part[1] = vmnumber.nbr_part[1];
                    ret.nbr_part[2] = vmnumber.nbr_part[2];
                break;
                case VMNT_DATE: // full convertion
                    ret.m_type = vmnumber.m_type;
                    ret.nbr_upart[0] -= vmnumber.nbr_upart[0];
                    ret.nbr_upart[1] = vmnumber.nbr_upart[1];
                    ret.nbr_upart[2] = vmnumber.nbr_upart[2];
                break;
                default:
                break;
            }
		break;
        case VMNT_DOUBLE:
        case VMNT_PRICE:
            switch ( vmnumber.m_type ) {
                case VMNT_INT:
                case VMNT_TIME:
                    ret.nbr_part[0] = nbr_part[0] - vmnumber.nbr_part[0];
                break;
                case VMNT_DATE:
                    if ( ret.m_type == VMNT_DOUBLE ) {
                        ret.m_type = vmnumber.m_type;
                        ret.nbr_upart[0] = static_cast<unsigned int> ( ret.nbr_part[0] ) - vmnumber.nbr_upart[0];
                        ret.nbr_upart[1] = static_cast<unsigned int> ( ret.nbr_part[1] ) - vmnumber.nbr_upart[1];
                        ret.nbr_upart[2] = vmnumber.nbr_upart[2];
                    }
                    else {
                        ret.nbr_part[0] -= static_cast<int> ( vmnumber.nbr_upart[0] );
                        ret.nbr_part[1] -= static_cast<int> ( vmnumber.nbr_upart[1] );
                    }
                break;
                case VMNT_PRICE:
                    if ( ret.m_type == VMNT_DOUBLE )
                        ret.m_type = VMNT_PRICE;
                case VMNT_DOUBLE:
                    ret.nbr_part[0] -= vmnumber.nbr_part[0];
                    ret.nbr_part[1] -= vmnumber.nbr_part[1];
                    if ( ret.nbr_part[1] > 0 ) {
                        if ( ret.nbr_part[0] < 0 )
                            ++ret.nbr_part[0];
                    }
                    else if ( ret.nbr_part[1] < 0 ) {
                        if ( ret.nbr_part[0] > 0 )
                            --ret.nbr_part[0];
                        ret.nbr_part[1] = 0 - ret.nbr_part[1]; // decimal part cannot be negative
                    }
                break;
                default:
                break;
            }
		break;
        case VMNT_TIME:
            switch ( vmnumber.m_type ) {
                case VMNT_INT:
                case VMNT_DOUBLE:
                case VMNT_PRICE:
                case VMNT_TIME:
                    ret.setTime ( 10000, static_cast<int> (
							  ( ret.nbr_part[VM_IDX_HOUR] - ret.nbr_part[VM_IDX_MINUTE] ) + ( vmnumber.nbr_part[VM_IDX_HOUR] - vmnumber.nbr_part[VM_IDX_MINUTE] ) ) );
                break;
                default:
                break;
            }
		break;
        default:
		break;
	}
	return ret;
}

vmNumber vmNumber::operator+ ( const vmNumber& vmnumber ) const
{
	vmNumber ret ( *this );
	ret.setCached ( false );

	switch ( ret.m_type ) {
        case VMNT_INT:
            switch ( vmnumber.m_type ) {
                case VMNT_INT:
                case VMNT_DOUBLE:
                case VMNT_PRICE:
                case VMNT_TIME:
                    ret.m_type = vmnumber.m_type;
                    ret.nbr_part[0] += vmnumber.nbr_part[0];
                    ret.nbr_part[1] = vmnumber.nbr_part[1];
                    ret.nbr_part[2] = vmnumber.nbr_part[2];
                break;
                case VMNT_DATE: // full convertion
                    ret.m_type = vmnumber.m_type;
                    ret.nbr_upart[0] += vmnumber.nbr_upart[0];
                    ret.nbr_upart[1] = vmnumber.nbr_upart[1];
                    ret.nbr_upart[2] = vmnumber.nbr_upart[2];
                break;
                default:
                break;
            }
		break;
        case VMNT_DOUBLE:
        case VMNT_PRICE:
            switch ( vmnumber.m_type ) {
                case VMNT_INT:
                case VMNT_TIME:
                    ret.nbr_part[0] = nbr_part[0] + vmnumber.nbr_part[0];
                break;
                case VMNT_DATE:
                    if ( ret.m_type == VMNT_DOUBLE ) {
                        ret.m_type = vmnumber.m_type;
                        ret.nbr_upart[0] = static_cast<unsigned int> ( ret.nbr_part[0] ) + vmnumber.nbr_upart[0];
                        ret.nbr_upart[1] = static_cast<unsigned int> ( ret.nbr_part[1] ) + vmnumber.nbr_upart[1];
                        ret.nbr_upart[2] = vmnumber.nbr_upart[2];
                    }
                    else {
                        ret.nbr_part[0] += static_cast<int> ( vmnumber.nbr_upart[0] );
                        ret.nbr_part[1] += static_cast<int> ( vmnumber.nbr_upart[1] );
                    }
                break;
                case VMNT_PRICE:
                    ret.fromStrPrice ( useCalc ( vmnumber, ret, QLatin1String ( " + " ) ) );
                break;
                case VMNT_DOUBLE:
                    ret.fromTrustedStrDouble ( useCalc ( vmnumber, ret, QLatin1String ( " + " ) ) );
                break;
                default:
                break;
            }
		break;
        case VMNT_TIME:
            switch ( vmnumber.m_type ) {
                case VMNT_INT:
                case VMNT_DOUBLE:
                case VMNT_PRICE:
                case VMNT_TIME:
                    ret.setTime ( 10000, static_cast<int> (
							  ( ret.nbr_part[VM_IDX_HOUR] + ret.nbr_part[VM_IDX_MINUTE] ) + ( vmnumber.nbr_part[VM_IDX_HOUR] + vmnumber.nbr_part[VM_IDX_MINUTE] ) ) );
                break;
                default:
                break;
            }
		break;
        default:
		break;
	}
	return ret;
}

vmNumber vmNumber::operator/ ( const vmNumber& vmnumber ) const
{
	vmNumber ret ( *this );
	ret.setCached ( false );

	switch ( ret.m_type ) {
        case VMNT_INT:
            switch ( vmnumber.m_type ) {
                case VMNT_INT:
                case VMNT_DOUBLE:
                case VMNT_PRICE:
                case VMNT_TIME:
                    if ( vmnumber.nbr_part[0] != 0 ) {
                        ret.m_type = vmnumber.m_type;
                        ret.nbr_part[0] /= vmnumber.nbr_part[0];
                        ret.nbr_part[1] = vmnumber.nbr_part[1];
                        ret.nbr_part[2] = vmnumber.nbr_part[2];
                    }
                break;
                case VMNT_DATE: // full convertion
                    if ( vmnumber.nbr_upart[0] != 0 ) {
                        ret.m_type = vmnumber.m_type;
                        ret.nbr_upart[0] /= vmnumber.nbr_upart[0];
                        ret.nbr_upart[1] = vmnumber.nbr_upart[1];
                        ret.nbr_upart[2] = vmnumber.nbr_upart[2];
                    }
                break;
                default:
                break;
            }
		break;
        case VMNT_DOUBLE:
        case VMNT_PRICE:
            switch ( vmnumber.m_type ) {
                case VMNT_INT:
                case VMNT_TIME:
                    if ( vmnumber.nbr_part[0] != 0 )
                        ret.nbr_part[0] = nbr_part[0] / vmnumber.nbr_part[0];
                break;
                case VMNT_DATE:
                    if ( ( vmnumber.nbr_upart[0] ) != 0 && ( vmnumber.nbr_upart[1] != 0 ) ) {
                        if ( ret.m_type == VMNT_DOUBLE ) {
                            ret.m_type = vmnumber.m_type;
                            ret.nbr_upart[0] = static_cast<unsigned int> ( ret.nbr_part[0] ) / vmnumber.nbr_upart[0];
                            ret.nbr_upart[1] = static_cast<unsigned int> ( ret.nbr_part[1] ) / vmnumber.nbr_upart[1];
                            ret.nbr_upart[2] = vmnumber.nbr_upart[2];
                        }
                        else {
                            ret.nbr_part[0] /= static_cast<int> ( vmnumber.nbr_upart[0] );
                            ret.nbr_part[1] /= static_cast<int> ( vmnumber.nbr_upart[1] );
                        }
                    }
                break;
                case VMNT_PRICE:
                    ret.fromStrPrice ( useCalc ( vmnumber, ret, QLatin1String ( " / " ) ) );
                break;
                case VMNT_DOUBLE:
                    ret.fromTrustedStrDouble ( useCalc ( vmnumber, ret, QLatin1String ( " / " ) ) );
                break;
                default:
                break;
            }
		break;
        case VMNT_TIME:
            switch ( vmnumber.m_type ) {
                case VMNT_INT:
                case VMNT_DOUBLE:
                case VMNT_PRICE:
                case VMNT_TIME:
                    ret.setTime ( 10000, static_cast<int> (
							  ( ret.nbr_part[VM_IDX_HOUR] + ret.nbr_part[VM_IDX_MINUTE] ) / ( vmnumber.nbr_part[VM_IDX_HOUR] + vmnumber.nbr_part[VM_IDX_MINUTE] ) ) );
                break;
                default:
                break;
            }
		break;
        default:
		break;
	}
	return ret;
}

vmNumber vmNumber::operator/ ( const int number ) const
{
	vmNumber ret ( *this );
	if ( number == 0 )
		return ret;
	ret.setCached ( false );

	switch ( ret.m_type ) {
        case VMNT_INT:
        case VMNT_DOUBLE:
        case VMNT_PRICE:
        case VMNT_UNSET:
            ret.nbr_part[0] /= number;
		break;
        case VMNT_PHONE:
        case VMNT_DATE:
        break;
        case VMNT_TIME:
            ret.setTime ( 10000, static_cast<int> (
						  ( ret.nbr_part[VM_IDX_HOUR] + ret.nbr_part[VM_IDX_MINUTE] ) / number ) );
		break;
        default:
		break;
	}
	return ret;
}

vmNumber vmNumber::operator* ( const vmNumber& vmnumber ) const
{
	vmNumber ret ( *this );
	ret.setCached ( false );

	switch ( ret.m_type ) {
        case VMNT_INT:
            switch ( vmnumber.m_type ) {
                case VMNT_INT:
                case VMNT_DOUBLE:
                case VMNT_PRICE:
                case VMNT_TIME:
                    ret.m_type = vmnumber.m_type;
                    ret.nbr_part[0] *= vmnumber.nbr_part[0];
                    ret.nbr_part[1] = vmnumber.nbr_part[1];
                    ret.nbr_part[2] = vmnumber.nbr_part[2];
                break;
                case VMNT_DATE: // full convertion
                    ret.m_type = vmnumber.m_type;
                    ret.nbr_upart[0] *= vmnumber.nbr_upart[0];
                    ret.nbr_upart[1] = vmnumber.nbr_upart[1];
                    ret.nbr_upart[2] = vmnumber.nbr_upart[2];
                break;
                default:
                break;
            }
		break;
        case VMNT_DOUBLE:
        case VMNT_PRICE:
            switch ( vmnumber.m_type ) {
                case VMNT_INT:
                case VMNT_TIME:
                    ret.nbr_part[0] = nbr_part[0] * vmnumber.nbr_part[0];
                break;
                case VMNT_DATE:
                    if ( ret.m_type == VMNT_DOUBLE ) {
                        ret.m_type = vmnumber.m_type;
                        ret.nbr_upart[0] = static_cast<unsigned int> ( ret.nbr_part[0] ) * vmnumber.nbr_upart[0];
                        ret.nbr_upart[1] = static_cast<unsigned int> ( ret.nbr_part[1] ) * vmnumber.nbr_upart[1];
                        ret.nbr_upart[2] = vmnumber.nbr_upart[2];
                    }
                    else {
                        ret.nbr_part[0] *= static_cast<int> ( vmnumber.nbr_upart[0] );
                        ret.nbr_part[1] *= static_cast<int> ( vmnumber.nbr_upart[1] );
                    }
                break;
                case VMNT_PRICE:
                    ret.fromStrPrice ( useCalc ( vmnumber, ret, QStringLiteral ( " * " ) ) );
                break;
                case VMNT_DOUBLE:
                    ret.fromTrustedStrDouble ( useCalc ( vmnumber, ret, QStringLiteral ( " * " ) ) );
                break;
                default:
                break;
            }
		break;
        case VMNT_TIME:
            switch ( vmnumber.m_type ) {
                case VMNT_INT:
                case VMNT_DOUBLE:
                case VMNT_PRICE:
                case VMNT_TIME:
                    ret.setTime ( 10000, static_cast<int> (
							  ( ret.nbr_part[VM_IDX_HOUR] * ret.nbr_part[VM_IDX_MINUTE] ) + ( vmnumber.nbr_part[VM_IDX_HOUR] * vmnumber.nbr_part[VM_IDX_MINUTE] ) ) );
                break;
                default:
                break;
            }
		break;
        default:
        break;
	}
	return ret;
}
//-------------------------------------OPERATORS------------------------------------------
