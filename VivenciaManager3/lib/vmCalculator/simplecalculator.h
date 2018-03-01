#ifndef SIMPLECALCULATOR_H
#define SIMPLECALCULATOR_H

#include <QDialog>

namespace Ui {
	class SimpleCalculator;
	}

class SimpleCalculator : public QDialog
{
	Q_OBJECT

public:
	explicit SimpleCalculator(QWidget *parent = 0);
	~SimpleCalculator();

private:
	Ui::SimpleCalculator *ui;
};

#endif // SIMPLECALCULATOR_H
