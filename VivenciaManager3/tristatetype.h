#ifndef TRISTATETYPE_H
#define TRISTATETYPE_H

typedef enum { TRI_UNDEF = -1, TRI_ON = 1, TRI_OFF = 0 } TRI_STATE;

class triStateType
{

public:
	constexpr triStateType ()
		: m_state ( TRI_UNDEF )
	{}

	inline triStateType ( const int initial_state )
	{
		fromInt ( initial_state );
	}

	constexpr triStateType ( const TRI_STATE initial_state )
		: m_state ( initial_state )
	{}

	constexpr triStateType ( const triStateType& other )
		: m_state ( other.m_state )
	{}

	inline const triStateType& operator= ( const bool state )
	{
		fromInt ( static_cast<int> ( state ) );
		return *this;
	}

	inline const triStateType& operator= ( const int state )
	{
		fromInt ( state );
		return *this;
	}

	inline const triStateType& operator= ( const TRI_STATE state )
	{
		m_state = state;
		return *this;
	}

	inline const triStateType& operator= ( const triStateType& other )
	{
		m_state = other.m_state;
		return *this;
	}

	constexpr bool operator== ( const int state ) const
	{
		return ( static_cast<int> ( m_state ) == state );
	}

	constexpr bool operator== ( const TRI_STATE state ) const
	{
		return ( m_state == state );
	}

	constexpr bool operator== ( const triStateType& other ) const
	{
		return ( m_state == other.m_state );
	}

	bool operator!= ( const TRI_STATE state ) const
	{
		return ( m_state != state );
	}

	inline void setState ( const TRI_STATE state )
	{
		m_state = state;
	}

	inline void setState ( const int state )
	{
		fromInt ( state );
	}

	TRI_STATE toggleNext ();
	TRI_STATE togglePrev ();
	TRI_STATE toggleOnOff ();

	constexpr inline TRI_STATE state () const {
		return m_state;
	}
	constexpr inline int toInt () const {
		return static_cast<int> ( m_state );
	}

	inline void setOn () {
		setState ( TRI_ON );
	}
	inline void setOff () {
		setState ( TRI_OFF );
	}
	inline void setUndefined () {
		setState ( TRI_UNDEF );
	}

	constexpr inline bool isOn () const {
		return m_state == TRI_ON;
	}
	constexpr inline bool isOff () const {
		return m_state == TRI_OFF;
	}
	constexpr inline bool isUndefined () const {
		return m_state == TRI_UNDEF;
	}

	constexpr inline bool isTrue () const {
		return m_state == TRI_ON;
	}
	constexpr inline bool isFalse () const {
		return m_state == TRI_OFF;
	}
	constexpr inline bool isNeither () const {
		return m_state == TRI_UNDEF;
	}

private:
	void fromInt ( const int state );

	TRI_STATE m_state;
};

#endif // TRISTATETYPE_H
