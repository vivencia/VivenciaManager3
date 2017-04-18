#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <QString>

class Tokens;

class Token
{

public:
	enum Op { InvalidOp = 0, Plus, Minus, Asterisk, Slash, Caret, LeftPar, RightPar, Semicolon,
			  Percent, Exclamation, Equal, Modulo, Div
			};
	enum TokenType { stxUnknown, stxNumber, stxOperator, stxOpenPar, stxClosePar, stxSep };

	static const Token null;

	Token (const TokenType type = stxUnknown, const QString& text = QString::null, const int pos = -1 );
	Token ( const Token & );

	Op asOperator () const;
	inline QString asNumber () const {
		return isNumber () ? m_text : QStringLiteral ( "0" );
	}
	inline bool isNumber () const {
		return m_type == stxNumber;
	}
	inline bool isOperator () const {
		return m_type >= stxOperator;
	}
	inline int pos () const {
		return m_pos;
	}
	inline QString text () const {
		return m_text;
	}
	inline TokenType type () const {
		return m_type;
	}

	Token &operator= ( const Token & );

	static Op matchOperator ( const QChar &p );

protected:
	int m_pos;
	QString m_text;
	TokenType m_type;
};

class Calculator
{

public:
	~Calculator ();

	static Calculator* calc;

	static void init () {
		if ( !calc )
			calc = new Calculator;
	}

	void setExpression ( const QString& );
	static Tokens scan ( const QString& );
	QString autoFix ( const QString& ) const;
	void compile ( const Tokens & ) const;
	void eval ( QString& result );

private:
	Calculator ();

	friend void deleteCalculatorInstance ();
	struct Private;
	Private* stc_private;
};

#endif // CALCULATOR_H
