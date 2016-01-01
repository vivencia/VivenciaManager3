#include "simplecalculator.h"
#include "global.h"
#include "calculator.h"
#include "mainwindow.h"
#include "data.h"
#include "vmwidgets.h"

#include <QDialog>
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
	: QDockWidget ( tr ( "Calculator" ) )
{
	setFeatures ( QDockWidget::AllDockWidgetFeatures );
	setAllowedAreas ( Qt::AllDockWidgetAreas );

	txtResult = new QTextEdit;
	txtResult->setReadOnly ( true );
	txtResult->setMinimumSize ( 110, 70 );
	txtResult->setAcceptRichText ( false );

	txtInput = new vmLineEdit;
	txtInput->setEditable ( true );
	txtInput->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* qe, const vmWidget* const ) {
				return relevantKeyPressed ( qe ); } );
	QLabel* lblInput ( new QLabel ( tr ( "Expression: " ) , this ) );
	lblInput->setBuddy ( txtInput );

	QGridLayout* gLayout ( new QGridLayout );
	gLayout->setMargin ( 5 );
	gLayout->setSpacing ( 2 );
	gLayout->addWidget ( txtResult, 0, 0, 2, -1 );
	gLayout->addWidget ( lblInput, 2, 0, 2, 0 );
	gLayout->addWidget ( txtInput, 2, 1, 2, 2 );

	btnCalc = new QPushButton ( tr ( "Calc" ) );
	connect ( btnCalc, &QPushButton::clicked , this, [&] () { return calculate (); } );
	btnClose = new QPushButton ( tr ( "Close" ) );
	connect ( btnClose, &QPushButton::clicked , this, [&] () { txtReceiveResult = nullptr; hide (); return; } );

	btnCopyResult = new QPushButton ( tr ( "Copy result" ) );
	btnCopyResult->setDefault ( true );
	connect ( btnCopyResult, &QPushButton::clicked , this, [&] () { return btnCopyResultClicked (); } );

	gLayout->addWidget ( btnCalc, 3, 0 );
	gLayout->addWidget ( btnClose, 3, 1 );
	gLayout->addWidget ( btnCopyResult, 3, 2 );

	QWidget* placeHolder ( new QWidget );
	placeHolder->setLayout ( gLayout );
	setWidget ( placeHolder );

	setMinimumSize ( 280, 310 );
}

SimpleCalculator::~SimpleCalculator () {}

void SimpleCalculator::showCalc ( const QPoint& pos, vmLineEdit* line, QWidget* parentWindow )
{
	move ( pos.x () + width () , pos.y () );
	txtReceiveResult = line;
	if ( parentWindow != nullptr ) {
		setParent ( parentWindow );
		static_cast<QMainWindow*> ( parentWindow )->addDockWidget ( Qt::LeftDockWidgetArea, this );
		setFloating ( false );
	}
	else {
		setParent ( globalMainWindow );
		globalMainWindow->addDockWidget ( Qt::AllDockWidgetAreas, this );
		setFloating ( true );
	}
	show ();
	qApp->setActiveWindow ( this );
	txtInput->setFocus ( Qt::ActiveWindowFocusReason );
}

void SimpleCalculator::showCalc ( const QString& input, const QPoint& pos, vmLineEdit* line, QWidget* parentWindow )
{
	txtInput->setText ( input );
	showCalc ( pos, line, parentWindow );
}

void SimpleCalculator::calculate ()
{
	if ( !txtInput->text().isEmpty () ) {
		QString formula = txtInput->text ();

		if ( !formula.at ( 0 ).isDigit () ) {
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
		txtResult->append ( formula + CHR_EQUAL + mStrResult + CHR_NEWLINE );
		txtInput->clear ();
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
