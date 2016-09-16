#ifndef VMLIST_H
#define VMLIST_H

#include <cstdlib>
#include <cstring>
#include <cmath>
#include <type_traits>
#include <algorithm>

using namespace std;

typedef unsigned int uint;

typedef char TIsPointerType;
typedef char TIsNotPointerType[2];
typedef char TIsFuncPointerType[4];

template <typename T>
class VMList
{

	template <typename TT> friend class PointersList;
	template <typename TT> friend class podList;

public:

	template<typename X>
	struct IsPointer
	{
		typedef TIsNotPointerType Result;
	};

	template<typename X>
	struct IsPointer<X*>
	{
		typedef TIsPointerType Result;
	};

	template<typename X>
	struct IsPointer<X (*) ()>
	{
		typedef TIsFuncPointerType Result;
	};

	VMList ();
	explicit VMList ( const T& default_value, const uint n_prealloc = 10 );
	inline VMList ( const VMList<T>& other ) {
		operator= ( other );
	}

	inline virtual ~VMList () { clear (); }

	int
		contains ( const T&, int from_pos = 0 ) const,
		insert ( const int pos, const T& item ), //returns pos
		replace ( const int pos, const T& item ), //returns pos
		removeOne ( const T& item, int from_pos = 0, const bool delete_item = false ), //removes an item from pos
		remove ( const int = 0, const bool delete_item = false ); //returns nItems

	void
		clear ( const uint from, const uint to, const bool keep_memory = false, const bool delete_items = false ),
		setAutoDeleteItem ( const bool bauto_del ),
		deletePointer ( TIsPointerType*, const uint pos ),
		deletePointer ( TIsNotPointerType*, const uint pos ),
		deletePointer ( TIsFuncPointerType*, const uint pos ),
		resize ( const uint n ),
		setPreAllocNumber ( const uint n ),
		setDefaultValue ( const T& default_value ),
		setCurrent ( const int pos ) const;

	T
		&operator[] ( const int pos ),
		&operator[] ( const uint pos );

	virtual const VMList<T>&
		operator= ( const VMList<T>& other );

	inline T
		defaultValue () const { return end_value; }

	const T
		&at ( int ) const,
		&at ( uint ) const,
		&operator[] ( const int pos ) const,
		&operator[] ( const uint pos ) const,
		&first () const,
		&last () const,
		&next () const,
		&prev () const,
		&current () const,

		&peekFirst () const,
		&peekLast () const,
		&peekNext () const,
		&peekPrev () const,

		&begin () const,
		&end () const;

	inline uint
	count () const {
		return nItems;
	}

	inline int
	currentIndex () const {
		return ptr;
	}

	inline uint
	preallocNumber () const {
		return m_nprealloc;
	}

	bool
	getAlwaysPrealloc () const,
					  isEmpty () const,
					  currentIsLast () const,
					  currentIsFirst () const,
					  TIsPointer () const;
	
	virtual bool
		operator!= ( const VMList<T>& other ) const,
		operator== ( const VMList<T>& other ) const;
	
	virtual uint
		realloc ( const uint );

	virtual void
		clear ( const bool delete_items = false ),
		clearButKeepMemory ( const bool delete_items = false ),
		reserve ( const uint ),
		moveItems ( const uint to, const uint from, const uint amount ),
		copyItems ( T* dest, const T* src, const uint amount ),
		resetMemory ( const T& , uint = 0 );

	int
		append ( const T&item ),
		prepend ( const T&item );

	uint
		getCapacity () const,
		getMemCapacity () const;

protected:
	void
		setCapacity ( const uint i_capacity ),
		setMemCapacity ( const uint i_memcapacity ),
		setNItems ( const uint nitems ),
		setData ( T* data_ptr );

	inline T*
		data () const { return _data; }

	bool
		checkIsPointer ( TIsPointerType* ) const,
		checkIsPointer ( TIsNotPointerType* ) const,
		checkIsPointer ( TIsFuncPointerType* ) const;

private:
	T* _data;
	T end_value;

	int
		ptr;

	uint
		nItems,
		capacity,
		memCapacity, // only when memory is allocated
		m_nprealloc;

	bool
		mb_ispointer,
		mb_autodel;

	typename IsPointer<T>::Result r;
};

template <typename T>
inline bool VMList<T>::checkIsPointer ( TIsPointerType* ) const
{
	return true;
}

template <typename T>
inline bool VMList<T>::checkIsPointer ( TIsNotPointerType* ) const
{
	return false;
}

template <typename T>
inline bool VMList<T>::checkIsPointer ( TIsFuncPointerType* ) const
{
	return false;
}

template <typename T>
inline void VMList<T>::setPreAllocNumber ( const uint n )
{
	m_nprealloc = n;
}

template <typename T>
inline void VMList<T>::setDefaultValue ( const T &default_value )
{
	end_value = default_value;
}

template <typename T>
inline bool VMList<T>::getAlwaysPrealloc () const
{
	return static_cast<bool> ( m_nprealloc > 0 );
}

template <typename T>
inline bool VMList<T>::isEmpty () const
{
	return static_cast<bool> ( nItems == 0 );
}

template <typename T>
inline bool VMList<T>::currentIsLast () const
{
	return ptr == ( nItems - 1 );
}

template <typename T>
inline bool VMList<T>::currentIsFirst () const
{
	return ptr == 0;
}

template <typename T>
inline bool VMList<T>::TIsPointer () const
{
	return mb_ispointer;
}

template <typename T>
inline uint VMList<T>::getCapacity () const
{
	return capacity;
}

template <typename T>
inline uint VMList<T>::getMemCapacity () const
{
	return memCapacity;
}

template <typename T>
inline void VMList<T>::setCapacity ( const uint i_capacity )
{
	capacity = i_capacity;
}

template <typename T>
inline void VMList<T>::setMemCapacity ( const uint i_memcapacity )
{
	memCapacity = i_memcapacity;
}

template <typename T>
inline void VMList<T>::setNItems ( const uint nitems )
{
	nItems = nitems;
}

template <typename T>
inline void VMList<T>::setData ( T* data_ptr )
{
	_data = data_ptr;
}

template <typename T>
inline int VMList<T>::append ( const T &item )
{
	return insert ( nItems, item );
}

template <typename T>
inline int VMList<T>::prepend ( const T &item )
{
	return insert ( 0, item );
}

template <typename T>
void VMList<T>::moveItems ( const uint to, const uint from, const uint amount )
{
	int i ( 0 );
	if ( to > from )
	{
		i = amount - 1;
		do
		{
			_data[to + i] = _data[from + i];
			_data[from + i] = end_value;
			--i;
		} while ( i >= 0 );
	}
	else
	{
		do
		{
			_data[to+i] = _data[from + i];
			_data[from + i] = end_value;
			++i;
		} while ( i < signed ( amount )  );
	}
}

template <typename T>
void VMList<T>::resetMemory ( const T& value, uint length )
{
	if ( value != end_value )
		end_value = value;

	if ( length == 0 )
		length = capacity;
	else if ( length > capacity )
	{
		reserve ( length ); //already resets the memory
		return;
	}

	std::fill ( _data, _data + length, value );
	/*uint i ( 0 );
	while ( i < length ) {
		_data[i] = value;
		++i;
	};*/
}

template <typename T>
void VMList<T>::reserve ( const uint length )
{
	if ( length <= capacity )
		return;

	const int prev_capacity = capacity;
	this->realloc ( length );

	std::fill ( _data + prev_capacity, _data + capacity, end_value );
}

template <typename T>
inline void VMList<T>::copyItems ( T* dest, const T* src, const uint amount )
{
	 std::copy ( src, src + amount, dest );
}

template <typename T>
inline const T& VMList<T>::first () const
{
	if ( nItems >= 1 )
	{
		const_cast<VMList<T>*> ( this )->ptr = 0;
		return _data[0];
	}
	return end_value;
}

template <typename T>
inline const T& VMList<T>::last () const
{
	if ( nItems >= 1 )
	{
		const_cast<VMList<T>*> ( this )->ptr = nItems - 1;
		return _data[ptr];
	}
	return end_value;
}

template <typename T>
inline const T& VMList<T>::next () const
{
	if ( ptr < static_cast<int>( nItems - 1 ) && nItems >= 1 )
	{
		++( const_cast<VMList<T>*> ( this )->ptr );
		return _data[ptr];
	}
	return end_value;
}

template <typename T>
inline const T& VMList<T>::prev () const
{
	if ( ptr > 0 && nItems >= 1 )
	{
		--( const_cast<VMList<T>*> ( this )->ptr );
		return _data[ptr];
	}
	return end_value;
}

template <typename T>
inline const T& VMList<T>::current () const
{
	if ( ptr >= 0 && ptr < static_cast<int>( nItems ) )
		return _data[ptr];
	return end_value;
}

template <typename T>
inline void VMList<T>::setCurrent ( const int pos ) const
{
	if ( pos < static_cast<int> ( nItems ) )
		const_cast<VMList<T>*> ( this )->ptr = pos;
}

template <typename T>
inline const T& VMList<T>::peekFirst () const
{
	if ( nItems >= 1 )
		return _data[0];
	return end_value;
}

template <typename T>
inline const T& VMList<T>::peekLast () const
{
	if ( nItems >= 1 )
		return _data[nItems - 1];
	return end_value;
}

template <typename T>
inline const T& VMList<T>::peekNext () const
{
	if ( ptr < static_cast<int>( nItems - 1 ) && nItems >= 1 )
		return _data[ptr + 1];
	return end_value;
}

template <typename T>
inline const T& VMList<T>::peekPrev () const
{
	if ( ptr >= 1 && nItems >= 1 )
		return _data[ptr - 1];
	return end_value;
}

template <typename T>
inline const T& VMList<T>::begin () const
{
	return end_value;
}

template <typename T>
inline const T& VMList<T>::end () const
{
	return end_value;
}

template <typename T>
inline const T& VMList<T>::operator[] ( const int pos ) const
{
	if ( (pos >= 0) && (pos < static_cast<int>( nItems )) ) // only return actually inserted items; therefore we use nItems
		return const_cast<T&> ( ( const_cast<VMList<T>*> ( this ) )->_data[pos] );
	return const_cast<T&> ( ( const_cast<VMList<T>*> ( this ) )->end_value );
}

template <typename T>
inline T& VMList<T>::operator[] ( const int pos )
{
	if ( pos >= 0 )
	{
		if ( pos >= static_cast<int>( capacity ) ) // capacity was used because the non-const operator[] may be used to insert items into the list
			realloc ( pos + m_nprealloc );
		if ( pos >= static_cast<int>( nItems ) )
			nItems = pos + 1;
		return _data[pos];
	}
	return end_value;
}

template <typename T>
inline const T& VMList<T>::operator[] ( const uint pos ) const
{
	if ( pos < nItems ) // only return actually inserted items; therefore we use nItems
		return const_cast<T&> ( ( const_cast<VMList<T>*> ( this ) )->_data[pos] );
	return const_cast<T&> ( ( const_cast<VMList<T>*> ( this ) )->end_value );
}

template <typename T>
inline T& VMList<T>::operator[] ( const uint pos )
{
	if ( pos >= capacity ) // capacity was used because the non-const operator[] may be used to insert items into the list
		realloc ( pos + m_nprealloc );
	if ( pos >= nItems )
		nItems = pos + 1;
	return _data[pos];
}

template <typename T>
const VMList<T>& VMList<T>::operator= ( const VMList<T>& other )
{
	if ( this != &other )
	{
		this->clear ();
		this->nItems = other.nItems;
		this->capacity = other.capacity;
		this->memCapacity = other.memCapacity;
		this->m_nprealloc = other.m_nprealloc;
		this->mb_autodel = other.mb_autodel;
		this->ptr = other.ptr;
		this->end_value = other.end_value;
		this->mb_ispointer = other.mb_ispointer;

		_data = new T[this->capacity];
		copyItems ( this->_data, other._data, this->capacity );
	}
	return *this;
}

template <typename T>
inline const T& VMList<T>::at ( const int pos ) const
{
	if ( (pos >= 0) && (pos < static_cast<int>( nItems )) ) // only return actually inserted items; therefore we use nItems
		return const_cast<T&> ( ( const_cast<VMList<T>*> ( this ) )->_data[pos] );
	return const_cast<T&> ( ( const_cast<VMList<T>*> ( this ) )->end_value );
}

template <typename T>
inline const T& VMList<T>::at ( const uint pos ) const
{
	if ( pos < nItems ) // only return actually inserted items; therefore we use nItems
		return const_cast<T&> ( ( const_cast<VMList<T>*> ( this ) )->_data[pos] );
	return const_cast<T&> ( ( const_cast<VMList<T>*> ( this ) )->end_value );
}

template <typename T>
VMList<T>::VMList ( const T& default_value, const uint n_prealloc )
	: _data ( nullptr ), ptr ( -1 ), nItems ( 0 ), capacity ( 0 ), memCapacity ( 0 ), mb_autodel ( false )
{
	m_nprealloc = n_prealloc;
	capacity = m_nprealloc;
	memCapacity = capacity * sizeof ( T );
	end_value = default_value;
	mb_ispointer = checkIsPointer ( &r );

	if ( capacity > 0 )
	{
		_data = new T[capacity];
		for ( uint i ( 0 ); i < capacity; ++i )
			_data[i] = end_value;
	}
}

template <typename T>
inline VMList<T>::VMList ()
	: _data ( nullptr ), ptr ( -1 ), nItems ( 0 ), capacity ( 0 ),
	  memCapacity ( 0 ), m_nprealloc ( 10 ), mb_autodel ( false )
{
	mb_ispointer = checkIsPointer ( &r );
}

template<typename T>
bool VMList<T>::operator!= ( const VMList<T>& other ) const //simple and incomplete
{
	if ( other.count () != count () )
		return ( other.preallocNumber () != preallocNumber () );
	return false;
}

template<typename T>
bool VMList<T>::operator== ( const VMList<T>& other ) const //simple and incomplete
{
	if ( other.count () == count () )
		return ( other.preallocNumber () == preallocNumber () );
	return true;
}

template <typename T>
uint VMList<T>::realloc ( const uint newsize )
{
	if ( newsize == capacity )
		return newsize;

	if ( newsize < nItems )
		nItems = newsize;

	T* new_data = new T[newsize];
	if ( nItems > 0 )
	{
		copyItems ( new_data, _data, nItems );
		delete [] _data;
	}
	_data = new_data;

	if ( newsize >= capacity )
	{
		std::fill ( _data + capacity, _data + newsize, end_value );
		/*uint i ( capacity );
		do {
			_data[i] = end_value;
		} while ( ++i < newsize );*/
	}

	memCapacity = newsize * sizeof ( T );
	capacity = newsize;
	return memCapacity;
}

template <typename T>
void VMList<T>::clear ( const bool delete_items )
{
	if ( _data != nullptr )
	{
		if ( delete_items || mb_autodel )
		{
			for ( uint pos ( 0 ); pos < nItems; ++pos )
				deletePointer ( &r, pos );
		}
		delete [] _data;
		_data = nullptr;
		nItems = capacity = memCapacity = 0;
		ptr = -1;
	}
}

template <typename T>
void VMList<T>::deletePointer ( TIsPointerType*, const uint pos )
{
	if ( _data[pos] )
	{
		delete _data[pos];
		_data[pos] = nullptr;
	}
}

template <typename T>
void VMList<T>::deletePointer ( TIsNotPointerType*, const uint ) {}

template <typename T>
void VMList<T>::deletePointer ( TIsFuncPointerType*, const uint ) {}

template <typename T>
void VMList<T>::clear ( const uint from, const uint to, const bool keep_memory, const bool delete_items )
{
	if ( _data != nullptr && from < capacity && to < capacity && from < to )
	{
		uint pos ( from );
		for ( ; pos < to; ++pos )
		{
			if ( delete_items || mb_autodel )
				deletePointer ( &r, pos );
			if ( !keep_memory )
				delete _data[pos];
			_data[pos] = nullptr;
		}
		if ( ptr != -1 )
		{
			if ( unsigned ( ptr ) >= from )
			{
				if ( from != 0 )
					ptr = from - 1;
				else
					ptr = -1;
			}
		}
		pos = to - from;
		nItems -= pos;
		capacity -= pos;
		memCapacity -= pos;
	}
}

template<typename T>
inline void VMList<T>::setAutoDeleteItem ( const bool bauto_del )
{
	mb_autodel = bauto_del;
}

template <typename T>
void VMList<T>::clearButKeepMemory ( const bool delete_items )
{
	if ( _data != nullptr )
	{
		uint i ( 0 );
		if ( delete_items || mb_autodel )
		{
			while ( i < nItems )
				deletePointer ( &r, i++ );
		}
		std::fill ( _data, _data + nItems, end_value );
	}
	nItems = 0;
	ptr = -1;
}

template <typename T>
void VMList<T>::resize ( const uint n )
{
	if ( signed ( n ) >= nItems )
	{
		if ( n > capacity )
			reserve ( n );
		nItems = n;
	}
	else {
		realloc ( n );
		nItems = n;
	}
}

template <typename T>
int VMList<T>::remove ( int pos, const bool delete_item )
{
	int ret ( -1 );
	if ( (pos >= 0) && (pos < static_cast<int>( nItems ) ) )
	{
		if ( delete_item )
			deletePointer ( &r, pos );
		if ( pos < static_cast<int>( nItems - 1 ) )
			moveItems ( pos, pos + 1, nItems - pos - 1 );
		_data[nItems-1] = end_value;
		--nItems;
		ret = nItems;
		if ( ptr >= pos )
			ptr = nItems - 1;
	}
	return ret;
}

template <typename T>
int VMList<T>::removeOne ( const T& item, int pos, const bool delete_item )
{
	int ret ( -1 );
	while ( (pos >= 0) && (pos < static_cast<int>( nItems ) ) )
	{
		if ( _data[pos] == item )
		{
			ret = remove ( pos, delete_item );
			break;
		}
		++pos;
	}
	return ret;
}

template <typename T>
int VMList<T>::replace ( const int pos, const T &item )
{
	int ret ( -1 );
	if ( (pos >= 0) && (pos < static_cast<int>( nItems )) )
	{
		_data[pos] = item;
		ret = pos;
	}
	return ret;
}

template <typename T>
int VMList<T>::contains ( const T& item, int from_pos ) const
{
	if ( nItems > 0 ) {
		bool forward ( true );

		if ( from_pos < 0 || from_pos >= static_cast<int>( nItems ) ) {
			from_pos = nItems - 1;
			forward = false;
		}
		int i = from_pos;
		if ( forward ) {
			for ( ; i < static_cast<int>( nItems ); ++i ) {
				if ( _data[i] == item )
					return i;
			}
		}
		else {
			for ( ; i >= 0; --i ) {
				if ( _data[i] == item )
					return i;
			}
		}
	}
	return -1;
}

template <typename T>
int VMList<T>::insert ( const int pos, const T &item )
{
	int ret ( 1 );

	if ( pos <= (signed) capacity )
	{
		if ( ( m_nprealloc > 0 ) && ( pos == static_cast<int>( capacity ) ) )
		{
			if ( nItems != 0 )
				ret = static_cast<int>( this->realloc ( capacity + m_nprealloc ) );
			else
			{
				capacity = m_nprealloc;
				memCapacity = capacity * sizeof ( T );
				_data = new T[capacity];
				resetMemory ( end_value, capacity );
			}
		}
	}
	else
	{
		if ( m_nprealloc > 0 )
			ret = static_cast<int>( this->realloc ( pos + m_nprealloc ) );
	}

	if ( ret != 0 )
	{
		if ( pos <= static_cast<int>( nItems - 1 ) )
			moveItems ( pos + 1, pos, nItems - pos );
		_data[pos] = item;
		ret = pos;
		ptr = pos; // the inserted item becomes the current item
		++nItems;
	}
	return ret;
}

//-------------------------------------------------NUMBERS-LIST------------------------------------------------

template<typename T>
class podList : public VMList<T>
{

public:
	explicit podList ( const T& default_value = 0, const uint n_prealloc = 100 );
	explicit inline podList ( const podList<T>& other )
		: VMList<T> (){
		operator= ( other );
	}

	virtual inline ~podList () { this->clear (); }

	uint
	realloc ( const uint );

	void
	clear ( const bool = false ),
		  clearButKeepMemory ( const bool = false );

	void
	resetMemory ( const T& value, uint length = 0 ),
				moveItems ( const uint to, const uint from, const uint amount ),
				copyItems ( T* dest, const T* src, const uint amount ),
				reserve ( const uint );

	const podList<T>&
		operator= ( const podList<T>& other );
};

template <typename T>
inline void podList<T>::moveItems ( const uint to, const uint from, const uint amount )
{
	if ( to > from ) {
		::memmove ( static_cast<void*> ( VMList<T>::data () + to ),
					static_cast<void*> ( VMList<T>::data () + from ), amount * sizeof ( T ) );
		::memset ( static_cast<void*> ( VMList<T>::data () + from ),
				   VMList<T>::defaultValue (), ( ( to - from - 1 ) * sizeof ( T ) ) );
	}
	else {
		::memmove ( static_cast<void*> ( VMList<T>::data () + to ),
					static_cast<void*> ( VMList<T>::data () + from ), amount * sizeof ( T ) );
		::memset ( static_cast<void*> ( VMList<T>::data () + from ),
				   VMList<T>::defaultValue (), ( ( from - to - 1 ) * sizeof ( T ) ) );
	}
}

template <typename T>
inline void podList<T>::copyItems ( T* dest, const T* src, const uint amount )
{
	::memcpy ( static_cast<void*> ( dest ), static_cast<void*> ( const_cast<T*> ( src ) ), amount * sizeof ( T ) );
}

template <typename T>
void podList<T>::resetMemory ( const T& value, uint length )
{
	if ( value != VMList<T>::defaultValue () )
		VMList<T>::setDefaultValue ( value );

	if ( length == 0 )
		length = VMList<T>::getCapacity ();
	else if ( length > VMList<T>::getCapacity () ) {
		/*
		 * cheat: this function is called many times after the declaration of the list object.
		 * By setting the capacity to 0, reserve () will actually reset all items, from 0 to length, to de default value
		 */
		if ( VMList<T>::count () == 0 )
			VMList<T>::setCapacity ( 0 );
		reserve ( length ); //already resets the memory
		return;
	}

	::memset ( static_cast<void*> ( VMList<T>::data () ), VMList<T>::defaultValue (), length * sizeof ( T ) );
}

template<typename T>
podList<T>::podList ( const T& default_value, const uint n_prealloc )
	: VMList<T> ()
{
	VMList<T>::setDefaultValue ( default_value );
	VMList<T>::setPreAllocNumber ( n_prealloc );
	VMList<T>::setCapacity ( n_prealloc );
	VMList<T>::setMemCapacity ( n_prealloc * sizeof ( T ) );
	VMList<T>::setData ( static_cast<T*> ( ::malloc ( VMList<T>::getMemCapacity () ) ) );
	::memset ( VMList<T>::data (), default_value, VMList<T>::getMemCapacity () );
}

template<typename T>
const podList<T>& podList<T>::operator= ( const podList<T>& other )
{
	if ( this != &other ) {
		this->clear ();
		VMList<T>::setPreAllocNumber ( other.preallocNumber () );
		VMList<T>::setCapacity ( other.getCapacity () );
		VMList<T>::setMemCapacity ( other.getMemCapacity () );
		VMList<T>::setDefaultValue ( other.defaultValue () );
		VMList<T>::setNItems ( other.count () );
		VMList<T>::setCurrent ( other.currentIndex () );

		VMList<T>::setData ( static_cast<T*> ( ::malloc ( VMList<T>::getMemCapacity () ) ) );
		::memcpy ( VMList<T>::data (), other.data (), VMList<T>::getMemCapacity () );
	}
	return *this;
}

template <typename T>
uint podList<T>::realloc ( const uint newsize )
{
	if ( newsize == VMList<T>::getCapacity () )
		return newsize;

	const uint newMemCapacity ( newsize * sizeof ( T ) );

	VMList<T>::setData ( static_cast<T*> (
							 ::realloc ( static_cast<void*> ( VMList<T>::data () ), newMemCapacity ) ) );
	if ( newsize < VMList<T>::count () )
		VMList<T>::setNItems ( newsize );
	else {
		const uint i ( VMList<T>::getCapacity () );
		::memset ( ( VMList<T>::data () + i ), VMList<T>::defaultValue (), newsize - i );
	}
	VMList<T>::setMemCapacity ( newMemCapacity );
	VMList<T>::setCapacity ( newsize );
	return newMemCapacity;
}

template <typename T>
void podList<T>::reserve ( const uint length )
{
	if ( length <= VMList<T>::getCapacity () )
		return;

	uint newMemCapacity ( length * sizeof ( T ) );
	VMList<T>::setData ( static_cast<T*> ( ::realloc ( static_cast<void*> ( VMList<T>::data () ), newMemCapacity ) ) );
	::memset ( VMList<T>::data (), VMList<T>::defaultValue (), newMemCapacity );
	VMList<T>::setMemCapacity ( newMemCapacity );
	VMList<T>::setCapacity ( length );
}

template <typename T>
void podList<T>::clear ( const bool )
{
	if ( VMList<T>::data () != nullptr ) {
		::free ( static_cast<void*> ( VMList<T>::data () ) );
		VMList<T>::setData ( nullptr );
		VMList<T>::setMemCapacity ( 0 );
		VMList<T>::setCapacity ( 0 );
		VMList<T>::setNItems ( 0 );
	}
}

template <typename T>
void podList<T>::clearButKeepMemory ( const bool )
{
	if ( VMList<T>::data () != nullptr ) {
		::memset ( VMList<T>::data (), VMList<T>::defaultValue (), VMList<T>::getMemCapacity () );
		//VMList<T>::setData ( nullptr );
	}
	VMList<T>::setNItems ( 0 );
	VMList<T>::setCurrent ( -1 );
}
//-------------------------------------------------NUMBERS-LIST------------------------------------------------

//-------------------------------------------------POINTERS-LIST------------------------------------------------
template<typename T>
class PointersList : public VMList<T>
{

public:
	explicit PointersList ( const uint n_prealloc = 100 );
	explicit inline PointersList ( const PointersList<T>& other )
		: VMList<T> () {
		operator= ( other );
	}

	virtual inline ~PointersList () { this->clear (); }

	uint
	realloc ( const uint );

	void
	clear ( const bool delete_items = false ),
		  clearButKeepMemory ( const bool delete_items = false );

	void
	resetMemory ( const T&, uint length = 0 ),
				moveItems ( const uint to, const uint from, const uint amount ),
				copyItems ( T* dest, const T* src, const uint amount ),
				reserve ( const uint );

	const PointersList<T>&
			operator= ( const PointersList<T>& other );

private:
	typename VMList<T>::template IsPointer<T>::Result r;
};

template <typename T>
inline void PointersList<T>::moveItems ( const uint to, const uint from, const uint amount )
{
	if ( to > from ) {
		::memmove ( static_cast<void*> ( VMList<T>::data () + to ),
					static_cast<void*> ( VMList<T>::data () + from ), amount * sizeof ( T ) );
		::memset ( static_cast<void*> ( VMList<T>::data () + from ),
				   0, ( ( to - from - 1 ) * sizeof ( T ) ) );
	}
	else {
		::memmove ( static_cast<void*> ( VMList<T>::data () + to ),
					static_cast<void*> ( VMList<T>::data () + from ), amount * sizeof ( T ) );
		::memset ( static_cast<void*> ( VMList<T>::data () + from ),
				   0, ( ( from - to - 1 ) * sizeof ( T ) ) );
	}
}

template <typename T>
inline void PointersList<T>::copyItems ( T* dest, const T* src, const uint amount )
{
	::memcpy ( static_cast<void*> ( dest ), static_cast<void*> ( const_cast<T*> ( src ) ), amount * sizeof ( T ) );
}

template <typename T>
void PointersList<T>::resetMemory ( const T&, uint length )
{
	if ( length == 0 )
		length = VMList<T>::getCapacity ();
	else if ( length > VMList<T>::getCapacity () ) {
		/*
		 * cheat: this function is called many times after the declaration of the list object.
		 * By setting the capacity to 0, reserve () will actually reset all items, from 0 to length, to de default value
		 */
		if ( VMList<T>::count () == 0 )
			VMList<T>::setCapacity ( 0 );
		reserve ( length ); //already resets the memory
		return;
	}

	::memset ( static_cast<void*> ( VMList<T>::data () ), 0, length * sizeof ( T ) );
}

template<typename T>
PointersList<T>::PointersList ( const uint n_prealloc )
	: VMList<T> ()
{
	VMList<T>::setPreAllocNumber ( n_prealloc );
	VMList<T>::setCapacity ( n_prealloc );
	VMList<T>::setMemCapacity ( n_prealloc * sizeof ( T ) );
	VMList<T>::setDefaultValue ( nullptr );
	VMList<T>::setData ( static_cast<T*> ( ::malloc ( VMList<T>::getMemCapacity () ) ) );
	::memset ( VMList<T>::data (), 0, VMList<T>::getMemCapacity () );
}

template<typename T>
const PointersList<T>& PointersList<T>::operator= ( const PointersList<T>& other )
{
	if ( this != &other ) {
		this->clear ();
		VMList<T>::setPreAllocNumber ( other.preallocNumber () );
		VMList<T>::setCapacity ( other.getCapacity () );
		VMList<T>::setMemCapacity ( other.getMemCapacity () );
		VMList<T>::setDefaultValue ( nullptr );
		VMList<T>::setNItems ( other.count () );
		VMList<T>::setCurrent ( other.currentIndex () );

		VMList<T>::setData ( static_cast<T*> ( ::malloc ( VMList<T>::getMemCapacity () ) ) );
		::memcpy ( VMList<T>::data (), other.data (), VMList<T>::getMemCapacity () );
	}
	return *this;
}

template <typename T>
uint PointersList<T>::realloc ( const uint newsize )
{
	if ( newsize == VMList<T>::getCapacity () )
		return newsize;

	const uint newMemCapacity ( newsize * sizeof ( T ) );

	VMList<T>::setData ( static_cast<T*> (
							 ::realloc ( static_cast<void*> ( VMList<T>::data () ), newMemCapacity ) ) );
	if ( newsize < VMList<T>::count () )
		VMList<T>::setNItems ( newsize );
	else {
		const uint i ( VMList<T>::getCapacity () );
		::memset ( ( VMList<T>::data () + i ), 0, newsize - i );
	}
	VMList<T>::setMemCapacity ( newMemCapacity );
	VMList<T>::setCapacity ( newsize );
	return newMemCapacity;
}

template <typename T>
void PointersList<T>::reserve ( const uint length )
{
	if ( length <= VMList<T>::getCapacity () )
		return;

	const uint newMemCapacity ( length * sizeof ( T ) );
	VMList<T>::setData ( static_cast<T*> ( ::realloc ( static_cast<void*> ( VMList<T>::data () ), newMemCapacity ) ) );
	::memset ( VMList<T>::data (), 0, newMemCapacity );
	VMList<T>::setMemCapacity ( newMemCapacity );
	VMList<T>::setCapacity ( length );
}

template <typename T>
void PointersList<T>::clear ( const bool delete_items )
{
	if ( VMList<T>::data () != nullptr ) {
		if ( delete_items || VMList<T>::mb_autodel ) {
			for ( uint pos ( 0 ); pos < VMList<T>::count (); ++pos )
				this->deletePointer ( &r, pos );
		}
		::free ( static_cast<void*> ( VMList<T>::data () ) );
		VMList<T>::setData ( nullptr );
		VMList<T>::setMemCapacity ( 0 );
		VMList<T>::setCapacity ( 0 );
		VMList<T>::setNItems ( 0 );
	}
}

template <typename T>
void PointersList<T>::clearButKeepMemory ( const bool delete_items )
{
	if ( VMList<T>::data () != nullptr ) {
		if ( delete_items || VMList<T>::mb_autodel ) {
			for ( uint pos ( 0 ); pos < VMList<T>::count (); ++pos )
				this->deletePointer ( &r, pos );
		}
		::memset ( VMList<T>::data (), 0, VMList<T>::getMemCapacity () );
	}
	VMList<T>::setNItems ( 0 );
	VMList<T>::setCurrent ( -1 );
}
//-------------------------------------------------POINTERS-LIST------------------------------------------------

#endif // VMLIST_H
