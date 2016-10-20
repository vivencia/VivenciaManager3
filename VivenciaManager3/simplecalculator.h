#ifndef SIMPLECALCULATOR_H
#define SIMPLECALCULATOR_H

#include <QDialog>

class vmLineEdit;

namespace Ui
{
	class SimpleCalculator;
}

class SimpleCalculator : public QDialog
{

public:
	virtual ~SimpleCalculator ();
	void showCalc ( const QPoint&, vmLineEdit* line = nullptr, QWidget* parentWindow = nullptr );
	void showCalc ( const QString& input, const QPoint &, vmLineEdit* line = nullptr, QWidget* parentWindow = nullptr );

	inline const QString getResult () const { return mStrResult; }

private:
	explicit SimpleCalculator ();
	static SimpleCalculator* s_instance;
	friend SimpleCalculator* CALCULATOR ();
	void calculate ();
	void relevantKeyPressed ( const QKeyEvent* qe );
	void btnCopyResultClicked ();

	QString mStrResult;
	vmLineEdit* txtReceiveResult;
	Ui::SimpleCalculator *ui;
};

inline SimpleCalculator *CALCULATOR ()
{
	if ( SimpleCalculator::s_instance == nullptr )
		SimpleCalculator::s_instance = new SimpleCalculator;
	return SimpleCalculator::s_instance;
}

#endif // SIMPLECALCULATOR_H
