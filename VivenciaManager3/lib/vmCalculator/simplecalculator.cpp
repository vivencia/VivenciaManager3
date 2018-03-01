#include "simplecalculator.h"
#include "ui_simplecalculator.h"

SimpleCalculator::SimpleCalculator(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::SimpleCalculator)
{
	ui->setupUi(this);
}

SimpleCalculator::~SimpleCalculator()
{
	delete ui;
}
