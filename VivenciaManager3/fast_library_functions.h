#ifndef FAST_LIBRARY_FUNCTIONS_H
#define FAST_LIBRARY_FUNCTIONS_H

template <typename T>
inline void setBit ( T& var, const uchar bit )
{
    if ( ( bit - 1 ) >= 0 )
        var |= ( 2 << ( bit - 1 ) );
    else
        var |= 1;
}

template <typename T>
inline void unSetBit ( T& var, const uchar bit )
{
    if ( ( bit - 1 ) >= 0 )
        var &= ~ ( 2 << ( bit - 1 ) );
    else
        var &= ~ 1;
}

template <typename T>
inline bool isBitSet ( const T& var, const uchar bit )
{
    if ( ( bit - 1 ) >= 0 )
        return static_cast<bool> ( var & ( 2 << ( bit - 1 ) ) );
    else
        return static_cast<bool> ( var & 1 );
}

template <typename T>
inline void setAllBits ( T& var )
{
    var = static_cast<T> ( 0xFFFFFFFF ); // all bits set to 1
}

template <typename T>
inline bool allBitsSet ( T& var )
{
    return ( var == static_cast<T> ( 0xFFFFFFFF ) );
}
#endif // FAST_LIBRARY_FUNCTIONS_H
