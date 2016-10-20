#include "simplecalculator.h"
#include "global.h"
#include "calculator.h"
#include "mainwindow.h"
#include "data.h"
#include "vmwidgets.h"
#include "ui_simplecalculator.h"

#include <QCloseEvent>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QGridLayout>
#include <QLabel>
#include <QGroupBox>
#include <QFrame>
#include <QApplication>
#include <QKeyEvent>

//------------------------------------------DIALOG-----------------------------------------------
SimpleCalculator *SimpleCalculator::s_instance ( nullptr );

SimpleCalculator::SimpleCalculator ()
	: QDialog ( globalMainWindow ), ui ( new Ui::SimpleCalculator )
{
	ui->setupUi ( this );
	
	ui->txtInput->setEditable ( true );
	ui->txtInput->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* qe, const vmWidget* const ) {
				return relevantKeyPressed ( qe ); } );
	connect ( ui->btnCalc, &QPushButton::clicked , this, [&] () { return calculate (); } );
	connect ( ui->btnClose, &QPushButton::clicked , this, [&] () { txtReceiveResult = nullptr; hide (); return; } );
	ui->btnCopyResult->setDefault ( true );
	connect ( ui->btnCopyResult, &QPushButton::clicked , this, [&] () { return btnCopyResultClicked (); } );
}

SimpleCalculator::~SimpleCalculator () {}

void SimpleCalculator::showCalc ( const QPoint& pos, vmLineEdit* line, QWidget* parentWindow )
{
	txtReceiveResult = line;
	setParent ( parentWindow != nullptr ? parentWindow : globalMainWindow );
	show ();
	move ( pos.x () + width () , pos.y () );
	qApp->setActiveWindow ( this );
	ui->txtInput->setFocus ( Qt::ActiveWindowFocusReason );
}

void SimpleCalculator::showCalc ( const QString& input, const QPoint& pos, vmLineEdit* line, QWidget* parentWindow )
{
	ui->txtInput->setText ( input );
	showCalc ( pos, line, parentWindow );
}

void SimpleCalculator::calculate ()
{
	if ( !ui->txtInput->text().isEmpty () )
	{
		QString formula ( ui->txtInput->text () );
 		if ( !formula.at ( 0 ).isDigit () )
		{
			const Token::Op op ( Token::matchOperator ( formula.at ( 0 ) ) );
			if ( op == Token::InvalidOp )
				return;
			if ( op != Token::Equal )
				formula.prepend ( mStrResult );
		}

		formula = Calculator::calc->autoFix ( formula );
		if ( formula.isEmpty () )
			return;

		Calculator::calc->setExpression ( formula );
		Calculator::calc->eval ( mStrResult );
		mStrResult.replace ( CHR_DOT, CHR_COMMA );
		ui->txtResult->appendPlainText ( formula + CHR_EQUAL + mStrResult + CHR_NEWLINE );
		ui->txtInput->clear ();
	}
}

void SimpleCalculator::relevantKeyPressed ( const QKeyEvent* qe )
{
	if ( qe->key () == Qt::Key_Escape )
		hide ();
	else
		calculate ();
}

void SimpleCalculator::btnCopyResultClicked ()
{
	calculate ();
	Data::copyToClipboard ( mStrResult );
	if ( txtReceiveResult )
		txtReceiveResult->setText ( mStrResult, true );
}
//------------------------------------------DIALOG-----------------------------------------------
