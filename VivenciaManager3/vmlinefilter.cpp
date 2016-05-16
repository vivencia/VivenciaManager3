#include "vmlinefilter.h"

#include <QKeyEvent>

static const ushort CHR_MIN_VALID ( 0x30 ); //( QChar ( '0' ).unicode () )
static const ushort CHR_MAX_VALID ( 0xFC ); //( QString ( "ü" ).unicode ()->unicode () )
static const ushort CHR_a ( 0x61 ); //( QChar ( 'a' ).unicode () );
static const ushort CHR_z ( 0x7A ); //( QChar ( 'z' ).unicode () );

static const ushort CHR_A ( 0x41 ); //( QChar ( 'A' ).unicode () )
static const ushort CHR_E ( 0x45 ); //( QChar ( 'E' ).unicode () )
static const ushort CHR_I ( 0x49 ); //( QChar ( 'I' ).unicode () )
static const ushort CHR_O ( 0x4F ); //( QChar ( 'O' ).unicode () )
static const ushort CHR_U ( 0x55 ); //( QChar ( 'U' ).unicode () )
static const ushort CHR_C ( 0x43 ); //( QChar ( 'C' ).unicode () )

static const ushort CHR_A0 ( 0xE0 ); //( QString ( "à" ).unicode ()->unicode () )
static const ushort CHR_A1 ( 0xC0 ); //( QString ( "À" ).unicode ()->unicode () )
static const ushort CHR_A2 ( 0xC1 ); //( QString ( "Á" ).unicode ()->unicode () )
static const ushort CHR_A3 ( 0xC2 ); //( QString ( "Â" ).unicode ()->unicode () )
static const ushort CHR_A4 ( 0xC3 ); //( QString ( "Ã" ).unicode ()->unicode () )

static const ushort CHR_CEDILLA ( 0xC7 ); //( QString ( "Ç" ).unicode ()->unicode () )

static const ushort CHR_E1 ( 0xC8 ); //( QString ( "È" ).unicode ()->unicode () ) 
static const ushort CHR_E2 ( 0xC9 ); //( QString ( "É" ).unicode ()->unicode () )
static const ushort CHR_E3 ( 0xCA ); //( QString ( "Ê" ).unicode ()->unicode () )
static const ushort CHR_E4 ( 0xCB ); //( QString ( "Ë" ).unicode ()->unicode () )

static const ushort CHR_I1 ( 0xCC ); //( QString ( "Ì" ).unicode ()->unicode () )
static const ushort CHR_I2 ( 0xCD ); //( QString ( "Í" ).unicode ()->unicode () )
static const ushort CHR_I3 ( 0xCE ); //( QString ( "Î" ).unicode ()->unicode () )
static const ushort CHR_I4 ( 0xCF ); //( QString ( "Ï" ).unicode ()->unicode () )

static const ushort CHR_O1 ( 0xD2 ); // ( QString ( "Ò" ).unicode ()->unicode () )
static const ushort CHR_O2 ( 0xD3 ); // ( QString ( "Ó" ).unicode ()->unicode () )
static const ushort CHR_O3 ( 0xD4 ); //( QString ( "Ô" ).unicode ()->unicode () )
static const ushort CHR_O4 ( 0xD5 ); //( QString ( "Õ" ).unicode ()->unicode () )
static const ushort CHR_O5 ( 0xD6 ); //( QString ( "Ö" ).unicode ()->unicode () )

static const ushort CHR_U1 ( 0xD9 ); //( QString ( "Ù" ).unicode ()->unicode () )
static const ushort CHR_U2 ( 0xDA ); //( QString ( "Ú" ).unicode ()->unicode () )
static const ushort CHR_U3 ( 0xDB ); //( QString ( "Û" ).unicode ()->unicode () )
static const ushort CHR_U4 ( 0xDC ); //( QString ( "Ü" ).unicode ()->unicode () )

static ushort getCode ( const ushort base_code )
{
	ushort chr_code ( base_code );
	if ( chr_code < CHR_MIN_VALID )
		return 0;
	if ( chr_code > CHR_MAX_VALID )
		return 0;

	if ( chr_code >= CHR_A0 && chr_code <= CHR_MAX_VALID ) // reduce from CHR_MAX_VALID to range CHR_A1 through CHR_A0 - 1
		chr_code -= 0x20; // 32 decimal
	
	if ( chr_code >= CHR_A1 && chr_code < CHR_A0 ) // reduce from 'À' to 'a'
	{
		switch ( chr_code )
		{
			case CHR_A1: case CHR_A2: case CHR_A3: case CHR_A4:
				chr_code = CHR_A; break;
				
			case CHR_CEDILLA:
				chr_code = CHR_C; break;
				
			case CHR_E1: case CHR_E2: case CHR_E3: case CHR_E4:
				chr_code = CHR_E; break;
			
			case CHR_I1: case CHR_I2: case CHR_I3: case CHR_I4:
				chr_code = CHR_I; break;
			
			case CHR_O1: case CHR_O2: case CHR_O3: case CHR_O4: case CHR_O5:
				chr_code = CHR_O; break;
			
			case CHR_U1: case CHR_U2: case CHR_U3: case CHR_U4:
				chr_code = CHR_U; break;
				
			default:break;
		}
	}
	else
	{
		if ( chr_code >= CHR_a && chr_code <= CHR_z ) // reduce from 'a' to 'A'
			chr_code -= 0x20; // 32 decimal
	}
	
	return chr_code;
}

vmLineFilter::vmLineFilter ( QWidget* parent, QWidget* ownerWindow )
	: vmLineEdit ( parent, ownerWindow ), validkey_func ( nullptr )
{
	connect ( this, &QLineEdit::textChanged, this, [&] ( const QString& text ) { if ( text.isEmpty () ) return textCleared (); } );
}

vmLineFilter::~vmLineFilter () {}

bool vmLineFilter::matches ( const QString& haystack ) const
{
	if ( mBuffer.isEmpty () )
		return false;
	
	QString::const_iterator h_itr ( haystack.constBegin () );
	const QString::const_iterator h_itr_end ( haystack.constEnd () );
	ushort haystack_code ( 0 );
	
	QString::const_iterator n_itr ( mBuffer.constBegin () );
	const QString::const_iterator n_itr_end ( mBuffer.constEnd () );
	ushort needle_code ( 0 );
	bool bCanContinue ( false );
	
	for ( ; n_itr != n_itr_end; ++n_itr ) // iterate through buffer
	{
		needle_code = getCode ( static_cast<QChar>( *n_itr ).unicode () );
		for ( ; h_itr != h_itr_end; ++h_itr ) // iterate through searched string
		{
			haystack_code = getCode ( static_cast<QChar>( *h_itr ).unicode () );
			if ( needle_code == haystack_code ) // character match
			{
				bCanContinue = true;
				++h_itr;
				break;
			}
			else
			{
				if ( bCanContinue ) 
				{	// the next character does not match but the previous does
					bCanContinue = false;
					break;
				}
			}
		}
		if ( h_itr == h_itr_end && n_itr != n_itr_end )
			bCanContinue = false;
		
		if ( !bCanContinue )
			break;
	}
	return bCanContinue;
}

void vmLineFilter::keyPressEvent ( QKeyEvent* const ke )
{
	ke->accept ();
	QLineEdit::keyPressEvent ( ke );
	
	if ( text ().isEmpty () )
	{
		mBuffer.clear ();
		return;
	}
	
	qDebug () << endl << "------------------------------" << endl;
	qDebug () << "vmLineFilter::keyPressEvent" << endl;
	
	triStateType t_accept ( TRI_UNDEF );
	const int key ( ke->key () );
	uint startlevel ( 0 );
	
	qDebug () << "mBuffer is  " << mBuffer;
	qDebug () << "Key is " << key;
	if ( key == Qt::Key_Backspace || key == Qt::Key_Delete )
	{
		if ( hasSelectedText () )
		{
			startlevel = selectionStart ();
			qDebug () << "Text selected; startlevel = " << startlevel;
			
			mBuffer.remove ( startlevel, selectedText ().length () );
			t_accept.setOff ();
		}
		else
		{
			//if ( cursorPosition () > 0 && key == Qt::Key_Backspace )
				startlevel = cursorPosition ();
			//else if ( cursorPosition () >= 0 && cursorPosition () < text ().length () && key == Qt::Key_Delete )
				//startlevel = cursorPosition ();
			
			qDebug () << "Text not selected; startlevel = " << startlevel;
			mBuffer.remove ( startlevel, 1 );
			t_accept.setOff ();
		}
	}
	else
	{
		startlevel = cursorPosition () - 1;
		if ( key >= Qt::Key_Exclam && key <= Qt::Key_AsciiTilde )
		{
			mBuffer.insert ( startlevel, QChar ( key ) );
			t_accept.setOn ();
		}
		else if ( key == Qt::Key_cedilla )
		{
			mBuffer.insert ( startlevel, Qt::Key_C );
			t_accept.setOn ();
		}
		qDebug () << "Adding to mBuffer; startlevel = " << startlevel;
	}
	qDebug () << endl << "---------------------------";
	if ( !t_accept.isNeither () && validkey_func )
		validkey_func ( t_accept, startlevel );
}

void vmLineFilter::textCleared ()
{
	mBuffer.clear ();
	if ( validkey_func )
		validkey_func ( CLEAR_LEVEL, 0 );
}
