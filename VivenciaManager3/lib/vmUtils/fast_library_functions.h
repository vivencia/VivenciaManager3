#ifndef FAST_LIBRARY_FUNCTIONS_H
#define FAST_LIBRARY_FUNCTIONS_H

#include <QString>
#include <QStringList>

class vmComboBox;

namespace VM_LIBRARY_FUNCS
{
	void copyToClipboard ( const QString& str );
	int insertComboItem ( vmComboBox* cbo, const QString& text );
	int insertStringListItem ( QStringList& list, const QString& text );
	void fillJobTypeList ( QStringList& jobList, const QString& clientid );
}

template <typename T>
inline void setBit ( T& __restrict var, const unsigned char bit )
{
	if ( ( bit - 1 ) >= 0 )
		var |= ( 2 << ( bit - 1 ) );
	else
		var |= 1;
}

template <typename T>
inline void unSetBit ( T& __restrict var, const unsigned char bit )
{
	if ( ( bit - 1 ) >= 0 )
		var &= ~ ( 2 << ( bit - 1 ) );
	else
		var &= ~ 1;
}

template <typename T>
inline bool isBitSet ( const T& __restrict var, const unsigned char bit )
{
	if ( ( bit - 1 ) >= 0 )
		return static_cast<bool> ( var & ( 2 << ( bit - 1 ) ) );
	else
		return static_cast<bool> ( var & 1 );
}

template <typename T>
inline void setAllBits ( T& __restrict var )
{
	var = static_cast<T> ( 0xFFFFFFFF ); // all bits set to 1
}

template <typename T>
inline bool allBitsSet ( T& __restrict var )
{
	return ( var == static_cast<T> ( 0xFFFFFFFF ) );
}
#endif // FAST_LIBRARY_FUNCTIONS_H
