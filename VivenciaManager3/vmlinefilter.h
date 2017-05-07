#ifndef VMLINEFILTER_H
#define VMLINEFILTER_H

#include "vmwidgets.h"
#include "tristatetype.h"

#define DELETE_LEVEL TRI_OFF
#define ADD_LEVEL TRI_ON
#define CLEAR_LEVEL TRI_UNDEF

class vmLineFilter : public vmLineEdit
{

public:
	explicit vmLineFilter ( QWidget* parent = nullptr, QWidget* ownerWindow = nullptr );
	virtual ~vmLineFilter ();

	inline void setCallbackForValidKeyEntered ( const std::function<void ( const triStateType, const int )>& func ) 
			{ validkey_func = func; }
	
	inline const QString& buffer () const { return mBuffer; }
	
	bool matches ( const QString& haystack ) const;

private:
	std::function<void ( const triStateType, const int )> validkey_func;
	QString mBuffer;
	
	void keyPressEvent ( QKeyEvent* const ke ) override;
	void textCleared ();
};

#endif // VMLINEFILTER_H
