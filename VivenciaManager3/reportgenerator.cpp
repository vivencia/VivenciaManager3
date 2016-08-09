#include "reportgenerator.h"
#include "data.h"
#include "quickprojectui.h"
#include "vivenciadb.h"
#include "configops.h"
#include "fileops.h"
#include "vmnumberformats.h"
#include "texteditwithcompleter.h"
#include "client.h"
#include "job.h"
#include "payment.h"
#include "documenteditor.h"
#include "heapmanager.h"
#include "cleanup.h"

#include <QLabel>
#include <QApplication>
#include <QTextTable>
#include <QLineEdit>
#include <QTextEdit>
#include <QSpinBox>
#include <QToolButton>
#include <QVBoxLayout>
#include <QPushButton>

static QString str_curclientname ( emptyString );

static const QString strHeaderType[4] = {
	QStringLiteral	( "Serviços prestados " ),
	QStringLiteral	( "Relatório dos serviços prestados " ),
	QStringLiteral	( "Comprovante de pagamento recebido por serviços prestados " ),
	QStringLiteral	( "Projeto e proposta orçamentária para serviços " )
};

static const QString strJobType[6] = {
	QStringLiteral		( " Outros serviços" ),
	QStringLiteral		( " Paisagismo" ),
	QStringLiteral		( " Reforma e Construção" ),
	QStringLiteral		( " Pintura" ),
	QStringLiteral		( " Jardinagem" ),
	QStringLiteral		( " Serviços Gerais" )
};

enum HEADER_TYPE { OTHER = 0, REPORT = 1, PAYMENT_STUB = 2, PROJECT = 3 };
enum HEADER_JOBTYPE { JOB_OTHER = 0, LANDSCAPE = 1, MASONRY = 2, PAINTING = 3, GARDENING = 4, GENERAL = 5 };

static const QStringList STR_HEADERTYPE_LABEL
( QStringList () << QStringLiteral ( "OUTRO" ) << QStringLiteral ( "RELATÓRIO" )
  << QStringLiteral ( "COMPROVANTE DE PAGAMENTO" ) << QStringLiteral ( "ORÇAMENTO/PROJETO" )
);

static const QStringList STR_HEADERJOBTYPE_LABEL
( QStringList () << QStringLiteral ( "OUTRO" ) << QStringLiteral ( "PAISAGISMO" ) << QStringLiteral ( "REFORMA/CONSTRUÇÃO" )
  << QStringLiteral ( "PINTURA" ) << QStringLiteral ( "JARDINAGEM" ) << QStringLiteral ( "SERVIÇOS GERAIS" )
);

/*static int tryToDeterminateClientFromOpenDoc ( const QString& str )
{
	int clientWordPos ( -1 );
	clientWordPos = str.indexOf ( "Cliente:", 100 );

	if ( clientWordPos != -1 ) {
		const QString searchedLines ( str.mid ( clientWordPos + 40, 200 ) );

		for ( uint i ( 0 ); i < DATA ()->mainClient.allClients.count (); ++i ) {
			if ( searchedLines.indexOf ( Data::clientName ( DATA ()->mainClient.allClients.at ( i )->id ) ) != -1 )
				return DATA ()->mainClient.allClients.at ( i )->id;
		}
	}
	return -1;
}*/

static int tryToDeterminateJobFromOpenDoc ( const QString& str )
{
	int jobProjectID ( -1 );
	const QString prjTag ( "Projeto #:" );
	jobProjectID = str.indexOf ( prjTag, 600 );

	if ( jobProjectID != -1 ) {
		const int projectID_n ( str.indexOf ( '<', jobProjectID + prjTag.count () + 1 ) - ( jobProjectID + prjTag.count () ) );
		QString projectID ( str.mid ( jobProjectID + prjTag.count (), projectID_n ) );
		projectID = projectID.remove ( ' ' );
		Job job;
		if ( job.readRecord ( FLD_JOB_PROJECT_ID, projectID ) )
			return recIntValue ( &job, FLD_JOB_ID );
	}
	return -1;
}

reportGenerator::reportGenerator ( documentEditor* mdiParent )
	: textEditor ( mdiParent ), mb_IgnoreChange ( true ),
	  rgClient ( new Client ), rgJob ( new Job ), rgPay ( new Payment )
{
	setEditorType ( TEXT_EDITOR_SUB_WINDOW | REPORT_GENERATOR_SUB_WINDOW );
	setUseHtml ( true );

	cboHeaderType = new QComboBox;
	cboHeaderType->setEditable ( false );
	cboHeaderType->insertItems ( 0, STR_HEADERTYPE_LABEL );
	cboHeaderType->setCurrentIndex ( REPORT );

	cboHeaderJobType = new QComboBox;
	cboHeaderJobType->setEditable ( false );
	cboHeaderJobType->insertItems ( 0, STR_HEADERJOBTYPE_LABEL );
	cboHeaderJobType->setCurrentIndex ( GENERAL );

	QVBoxLayout* layoutCbos = new QVBoxLayout;
	layoutCbos->setSpacing ( 1 );
	layoutCbos->setContentsMargins ( 1, 1, 1, 1 );
	layoutCbos->setSizeConstraint ( QLayout::SetFixedSize );
	layoutCbos->addWidget ( cboHeaderType, 1 );
	layoutCbos->addWidget ( cboHeaderJobType, 1 );

	btnGenerateReport = new QToolButton;
	btnGenerateReport->setFixedSize ( 30, 30 );
	btnGenerateReport->setToolTip ( tr ( "Generate a report based on the combo boxes selection" ) );
	btnGenerateReport->setIcon ( ICON ( "appicon" ) );
	connect ( btnGenerateReport, SIGNAL ( clicked () ), this, SLOT ( btnGenerateReport_clicked () ) );

	QHBoxLayout* layoutCbos2 = new QHBoxLayout;
	layoutCbos2->setSpacing ( 1 );
	layoutCbos2->setContentsMargins ( 1, 1, 1, 1 );
	layoutCbos2->setSizeConstraint ( QLayout::SetFixedSize );
	layoutCbos2->addLayout ( layoutCbos, 1 );
	layoutCbos2->addWidget ( btnGenerateReport, 1, Qt::AlignCenter );

	btnInsertHeader = new QPushButton ( QString::fromUtf8 ( "Cabealho" ) );
	connect ( btnInsertHeader, SIGNAL ( clicked () ), this, SLOT ( btnInsertHeader_clicked () ) );

	btnInsertFooter = new QPushButton ( tr ( "Seal" ) );
	connect ( btnInsertFooter, SIGNAL ( clicked () ), this, SLOT ( btnInsertFooter_clicked () ) );

	btnInsertLogo = new QPushButton ( tr ( "Logo" ) );
	connect ( btnInsertLogo, SIGNAL ( clicked () ), this, SLOT ( btnInsertLogo_clicked () ) );

	QHBoxLayout* layoutReportBtns1 = new QHBoxLayout;
	layoutReportBtns1->setSpacing ( 1 );
	layoutReportBtns1->setContentsMargins ( 1, 1, 1, 1 );
	layoutReportBtns1->setSizeConstraint ( QLayout::SetFixedSize );
	layoutReportBtns1->addWidget ( btnInsertHeader, 1 );
	layoutReportBtns1->addWidget ( btnInsertFooter, 1 );
	layoutReportBtns1->addWidget ( btnInsertLogo, 1 );

	btnInsertFootNotes = new QPushButton ( tr ( "Insert foot notes" ) );
	btnInsertFootNotes->setMinimumSize ( 100, 30 );
	connect ( btnInsertFootNotes, SIGNAL ( clicked () ), this, SLOT ( btnInsertFootNotes_clicked () ) );

	btnInsertProjectNumber = new QPushButton ( tr ( "Project #" ) );
	btnInsertProjectNumber->setMinimumSize ( 100, 30 );
	connect ( btnInsertProjectNumber, SIGNAL ( clicked () ), this, SLOT ( btnInsertProjectNumber_clicked () ) );

	QHBoxLayout* layoutReportBtns2 = new QHBoxLayout;
	layoutReportBtns2->setSpacing ( 1 );
	layoutReportBtns2->setContentsMargins ( 1, 1, 1, 1 );
	layoutReportBtns2->setSizeConstraint ( QLayout::SetFixedSize );
	layoutReportBtns2->addWidget ( btnInsertFootNotes, 1 );
	layoutReportBtns2->addWidget ( btnInsertProjectNumber, 1 );

	QVBoxLayout* layoutSlot1 = new QVBoxLayout;
	layoutSlot1->setSpacing ( 1 );
	layoutSlot1->setContentsMargins ( 1, 1, 1, 1 );
	layoutSlot1->setSizeConstraint ( QLayout::SetFixedSize );
	layoutSlot1->addLayout ( layoutCbos2, 1 );
	layoutSlot1->addLayout ( layoutReportBtns1, 1 );
	layoutSlot1->addLayout ( layoutReportBtns2, 1 );

	QFrame *vLine = new QFrame;
	vLine->setFrameStyle ( QFrame::VLine|QFrame::Raised );
	//----------------------------------------------------------------------------------------

	//----------------------------------------------------------------------------------------
	QLabel* lblClientID = new QLabel ( tr ( "Client ID:" ) );
	lblClientID->setMaximumHeight ( 30 );

	txtClientsIDs = new QLineEdit;
	APP_COMPLETERS ()->setCompleter ( txtClientsIDs, vmCompleters::CLIENT_NAME );
	connect ( txtClientsIDs, SIGNAL ( textChanged ( const QString& ) ), this, SLOT ( updateJobIDsAndQPs ( const QString& ) ) );

	btnInsertClientAddress = new QPushButton ( tr ( "Address" ) );
	btnInsertClientAddress->setMinimumSize ( 80, 30 );
	connect ( btnInsertClientAddress, SIGNAL ( clicked () ), this, SLOT ( btnInsertClientAddress_clicked () ) );

	QVBoxLayout* layoutClientID = new QVBoxLayout;
	layoutClientID->setSpacing ( 1 );
	layoutClientID->setContentsMargins ( 1, 1, 1, 1 );
	layoutClientID->setSizeConstraint ( QLayout::SetFixedSize );
	layoutClientID->addWidget ( lblClientID, 1 );
	layoutClientID->addWidget ( txtClientsIDs, 1 );
	layoutClientID->addWidget ( btnInsertClientAddress, 1 );

	QFrame *vLine1 = new QFrame;
	vLine1->setFrameStyle ( QFrame::VLine|QFrame::Raised );
	//----------------------------------------------------------------------------------------

	//----------------------------------------------------------------------------------------
	QLabel* lblQPID = new QLabel ( tr ( "Quick Project:" ) );
	lblQPID->setMaximumHeight ( 30 );

	cboQPIDs = new QComboBox;
	cboQPIDs->setEditable ( false );
	connect ( cboQPIDs, SIGNAL ( activated ( int ) ), this, SLOT ( showProjectID ( int ) ) );

	btnCopyTableRow = new QPushButton ( tr ( "Copy selected rows" ) );
	btnCopyTableRow->setMinimumSize ( 100, 30 );
	connect ( btnCopyTableRow, SIGNAL ( clicked () ), this, SLOT ( btnCopyTableRow_clicked () ) );
	btnCopyTableRow->setEnabled ( false );

	btnCopyEntireTable = new QPushButton ( tr ( "Copy table" ) );
	btnCopyEntireTable->setMinimumSize ( 100, 30 );
	connect ( btnCopyEntireTable, SIGNAL ( clicked () ), this, SLOT ( btnCopyEntireTable_clicked () ) );
	btnCopyEntireTable->setEnabled ( false );

	QVBoxLayout* layoutQPID = new QVBoxLayout;
	layoutQPID->setSpacing ( 1 );
	layoutQPID->setContentsMargins ( 1, 1, 1, 1 );
	layoutQPID->setSizeConstraint ( QLayout::SetFixedSize );
	layoutQPID->addWidget ( lblQPID, 1 );
	layoutQPID->addWidget ( cboQPIDs, 1 );
	layoutQPID->addWidget ( btnCopyTableRow, 1 );
	layoutQPID->addWidget ( btnCopyEntireTable, 1 );

	QFrame *vLine2 = new QFrame;
	vLine2->setFrameStyle ( QFrame::VLine|QFrame::Raised );
	//----------------------------------------------------------------------------------------

	//----------------------------------------------------------------------------------------
	QLabel* lblJobID = new QLabel ( tr ( "Job ID:" ) );
	lblJobID->setMaximumHeight ( 30 );

	cboJobIDs = new QComboBox;
	connect ( cboJobIDs, SIGNAL ( currentIndexChanged ( int ) ), this, SLOT ( changeJobBriefInfo ( int ) ) );

	btnViewJob = new QToolButton;
	btnViewJob->setCheckable ( true );
	btnViewJob->setFixedSize ( 30, 30 );
	btnViewJob->setEnabled ( false );
	btnViewJob->setToolTip ( tr ( "View job's brief information" ) );
	btnViewJob->setIcon ( ICON ( "job_info" ) );
	connect ( btnViewJob, SIGNAL ( clicked ( bool ) ), this, SLOT ( btnViewJob_clicked ( bool ) ) );

	QHBoxLayout* layoutBriefJob = new QHBoxLayout;
	layoutBriefJob->setSpacing ( 1 );
	layoutBriefJob->setContentsMargins ( 1, 1, 1, 1 );
	layoutBriefJob->addWidget ( cboJobIDs, 1 );
	layoutBriefJob->addWidget ( btnViewJob, 0 );

	btnInsertJobReport = new QPushButton ( tr ( "Report" ) );
	btnInsertJobReport->setMinimumSize ( 80, 30 );
	connect ( btnInsertJobReport, SIGNAL ( clicked () ), this, SLOT ( btnInsertJobReport_clicked () ) );

	QVBoxLayout* layoutJobID = new QVBoxLayout;
	layoutJobID->setSpacing ( 1 );
	layoutJobID->setContentsMargins ( 1, 1, 1, 1 );
	layoutJobID->setSizeConstraint ( QLayout::SetFixedSize );
	layoutJobID->addWidget ( lblJobID, 1 );
	layoutJobID->addLayout ( layoutBriefJob, 1 );
	layoutJobID->addWidget ( btnInsertJobReport, 1 );
	//----------------------------------------------------------------------------------------

	QHBoxLayout* layoutBottomBar = new QHBoxLayout;
	layoutBottomBar->setSpacing ( 1 );
	layoutBottomBar->setContentsMargins ( 1, 1, 1, 1 );
	layoutBottomBar->addLayout ( layoutSlot1, 0 );
	layoutBottomBar->addWidget ( vLine, 1 );
	layoutBottomBar->addLayout ( layoutClientID, 0 );
	layoutBottomBar->addWidget ( vLine1, 1 );
	layoutBottomBar->addLayout ( layoutQPID, 0 );
	layoutBottomBar->addWidget ( vLine2, 1 );
	layoutBottomBar->addLayout ( layoutJobID, 0 );

	//------------------------------------------------------------------------------------------------------------

	mainLayout->addLayout ( layoutBottomBar, 2, 0, 2, 1 );
}

reportGenerator::~reportGenerator ()
{
	DOCK_QP ()->detach ( this );
	DOCK_BJ ()->detach ( this );
	heap_del ( rgClient );
	heap_del ( rgJob );
	heap_del ( rgPay );
}

bool reportGenerator::loadFile ( const QString& filename )
{
	if ( textEditor::loadFile ( filename ) ) {
		show ( tryToDeterminateJobFromOpenDoc ( mDocument->document ()->toPlainText () ) );
		return true;
	}
	return false;
}

bool reportGenerator::saveAs ( const QString& filename )
{
	QString new_filename ( filename );
	QString extension ( fileOps::fileExtension ( filename ) );
	if ( extension.isEmpty () )
		new_filename = fileOps::replaceFileExtension ( filename, ( extension = "vmr" ) );
	// It is possible to save a report in any supported format, although with losses
	if ( extension == QLatin1String ( "txt" ) )
		setUseHtml ( false );
	else
		setUseHtml ( true );

	return saveFile ( new_filename );
}

void reportGenerator::show ( const int jobid, const int payid )
{
	if ( jobid == -1 ) {
		if ( payid == -1 ) {
			rgJob->readRecord ( recIntValue ( DATA ()->currentJob (), FLD_JOB_ID ) );
			rgClient->readRecord ( recStrValue ( rgJob, FLD_JOB_CLIENTID ).toInt () );
		}
		else {
			if ( rgPay->readRecord ( payid ) ) {
				rgJob->readRecord ( recStrValue ( rgPay, FLD_PAY_JOBID ).toInt () );
				rgClient->readRecord ( recStrValue ( rgPay, FLD_PAY_CLIENTID ).toInt () );
			}
			else
				show ( -1, -1 );
		}
	}
	else {
		rgJob->readRecord ( jobid );
		rgClient->readRecord ( recStrValue ( rgJob, FLD_JOB_CLIENTID ).toInt () );
	}

	txtClientsIDs->setText ( recStrValue ( rgClient, FLD_CLIENT_NAME ) );
	setIgnoreCursorPos ( false );
	QWidget::show ();
}

void reportGenerator::createJobReport ( const int c_id, const bool include_all_paid, const QString &filename, const bool show )
{
	cboHeaderType->setCurrentIndex ( REPORT );
	btnGenerateReport_clicked ( c_id, include_all_paid );
	if ( !filename.isEmpty () ) {
		setIsModified ( true );
		setIsUntitled ( false );
		save ( filename );
	}
	if ( show )
		this->show ();
}

void reportGenerator::createPaymentStub ( const int payID )
{
	if ( payID != -1 ) {
		setIgnoreCursorPos ( true );
		show ( -1, payID );
		cboHeaderType->setCurrentIndex ( cboHeaderType->findText ( STR_HEADERTYPE_LABEL.at ( PAYMENT_STUB ) ) );
		btnGenerateReport_clicked ();
		setIgnoreCursorPos ( false );
	}
}

void reportGenerator::updateJobIDsAndQPs ( const QString& text )
{
	if ( text.count () > 3 ) {
		if ( text != str_curclientname ) {
			str_curclientname = text;
			if ( rgClient->readRecord ( FLD_CLIENT_NAME, text ) ) {
				cboJobIDs->clear ();
				cboQPIDs->clear ();
				if ( rgJob->readRecord ( FLD_JOB_CLIENTID, recStrValue ( rgClient, FLD_CLIENT_ID ) ) ) {
					QString job_project;
					do {
						job_project = recStrValue ( rgJob, FLD_JOB_PROJECT_ID );
						cboJobIDs->insertItem ( 0, recStrValue ( rgJob, FLD_JOB_ID ) );
						if ( !job_project.isEmpty () )
							cboQPIDs->insertItem ( 0, job_project );
					} while ( rgJob->readNextRecord ( true ) );
					cboJobIDs->setCurrentIndex ( cboJobIDs->count () - 1 );
					cboQPIDs->setCurrentIndex ( cboQPIDs->count () - 1 );
					btnCopyTableRow->setEnabled ( cboQPIDs->count () > 0 );
					btnCopyEntireTable->setEnabled ( cboQPIDs->count () > 0 );
					btnViewJob->setEnabled ( cboJobIDs->count () > 0 );
					mb_IgnoreChange = ( cboJobIDs->count () == 0 );
					if ( !mb_IgnoreChange )
						changeJobBriefInfo ( 0 );
					return;
				}
			}
		}
	}
	btnCopyTableRow->setEnabled ( false );
	btnCopyEntireTable->setEnabled ( false );
	btnViewJob->setEnabled ( false );
	mb_IgnoreChange = true;
}

void reportGenerator::changeJobBriefInfo ( const int index )
{
	if ( mb_IgnoreChange )
		return;

	if ( index >= 0 ) {
		const int cboJobID ( cboJobIDs->itemText ( index ).toInt () );
		if ( cboJobID != recIntValue ( rgJob, FLD_JOB_ID ) ) {
			if ( !rgJob->readRecord ( cboJobID ) )
				return;
		}
		DOCK_BJ ()->fillControls ( rgJob );
	}
	else
		DOCK_BJ ()->fillControls ( nullptr );
}

void reportGenerator::btnInsertHeader_clicked ( const int c_id )
{
	setIgnoreCursorPos ( true );

	qreal fontsize ( mCursor.charFormat ().fontPointSize () );
	if ( fontsize <= 0.0 )
		fontsize = QApplication::font ().pointSize ();

	TEXT_EDITOR_TOOLBAR ()->btnAlignCenter_clicked ();
	TEXT_EDITOR_TOOLBAR ()->btnBold_clicked ( true );
	TEXT_EDITOR_TOOLBAR ()->btnItalic_clicked ( true );
	TEXT_EDITOR_TOOLBAR ()->setFontSize ( 14 );
	TEXT_EDITOR_TOOLBAR ()->btnStrikethrough_clicked ( false );

	mCursor.insertText ( strHeaderType[cboHeaderType->currentIndex ()] );

	if ( c_id == -1 ) {
		mCursor.insertText ( QStringLiteral ( "na área de" ) );
		mCursor.insertBlock ();
		mCursor.insertText ( strJobType[cboHeaderJobType->currentIndex ()] );
	}

	mCursor.insertBlock ();
	TEXT_EDITOR_TOOLBAR ()->setFontSize ( fontsize );
	TEXT_EDITOR_TOOLBAR ()->btnItalic_clicked ( false );
	TEXT_EDITOR_TOOLBAR ()->btnBold_clicked ( false );
	TEXT_EDITOR_TOOLBAR ()->btnAlignLeft_clicked ();
	if ( c_id == -1 )
		setIgnoreCursorPos ( false );
	mDocument->setFocus ();
}

void reportGenerator::btnInsertFooter_clicked ( const int c_id )
{
	setIgnoreCursorPos ( true );
	TEXT_EDITOR_TOOLBAR ()->btnAlignCenter_clicked ();
	TEXT_EDITOR_TOOLBAR ()->btnBold_clicked ( true );
	TEXT_EDITOR_TOOLBAR ()->btnUnderline_clicked ( false );
	TEXT_EDITOR_TOOLBAR ()->btnItalic_clicked ( false );
	TEXT_EDITOR_TOOLBAR ()->btnStrikethrough_clicked ( false );

	mCursor.insertText ( QStringLiteral ( "Vivência - Prestação de Serviços" ) );
	mCursor.insertBlock ();
	mCursor.insertText ( QStringLiteral ( " (Construção, Reforma, Pintura, Paisagismo e Jardinagem)" ) );
	mCursor.insertBlock ();
	mCursor.insertText ( QStringLiteral ( "Telefones: (19) 99788-9809 / (19) 99695-3111" ) );
	mCursor.insertBlock ();
	mCursor.insertText ( QStringLiteral ( "e-mail: vivencia@gmail.com" ) );
	mCursor.insertBlock ();

	TEXT_EDITOR_TOOLBAR ()->btnAlignLeft_clicked ();
	TEXT_EDITOR_TOOLBAR ()->btnBold_clicked ( false );

	if ( c_id == -1 )
		setIgnoreCursorPos ( false );
	mDocument->setFocus ();
}

void reportGenerator::btnInsertProjectNumber_clicked ()
{
	setIgnoreCursorPos ( true );
	QTextFrameFormat referenceFrameFormat;
	referenceFrameFormat.setBorder ( 1 );
	referenceFrameFormat.setPadding ( 4 );
	referenceFrameFormat.setPosition ( QTextFrameFormat::FloatRight );
	referenceFrameFormat.setWidth ( QTextLength ( QTextLength::FixedLength, 90 ) );
	mCursor.insertFrame ( referenceFrameFormat );
	TEXT_EDITOR_TOOLBAR ()->btnAlignCenter_clicked ();
	TEXT_EDITOR_TOOLBAR ()->btnBold_clicked ( true );
	mCursor.insertText ( QStringLiteral ( "Projeto #: QP-" ) );
	setIgnoreCursorPos ( false );
}

void reportGenerator::btnInsertFootNotes_clicked ( const int c_id )
{
	setIgnoreCursorPos ( true );
	TEXT_EDITOR_TOOLBAR ()->btnBold_clicked ( false );
	TEXT_EDITOR_TOOLBAR ()->btnItalic_clicked ( false );
	TEXT_EDITOR_TOOLBAR ()->btnStrikethrough_clicked ( false );
	TEXT_EDITOR_TOOLBAR ()->btnUnderline_clicked ( false );

	qreal fontsize ( mCursor.charFormat ().fontPointSize () );
	if ( fontsize <= 0.0 )
		fontsize = QApplication::font ().pointSize ();
	TEXT_EDITOR_TOOLBAR ()->setFontSize ( fontsize - 2.0 );

	TEXT_EDITOR_TOOLBAR ()->btnAlignLeft_clicked ();
	TEXT_EDITOR_TOOLBAR ()->btnInsertNumberedList_clicked ();

	mCursor.insertText ( QStringLiteral ( " O Projeto poderá (e deverá) sofrer modificações, sejam estéticas ou técnicas, quando da sua execução, se o Vivência concluir que tal alteração virá a beneficiá-lo." ) );
	mCursor.insertBlock ();
	mCursor.insertText ( QStringLiteral ( " Contudo, serviços extras ou quaisquer alterações ao projeto solicitados pelo cliente no momento de sua execução, somente serão realizados mediante prévio acerto de valores e se houver disponibilidade de tempo para sua concretização, desde que para isso a boa realização do projeto original não fique comprometida." ) );
	mCursor.insertBlock ();
	mCursor.insertText ( QStringLiteral ( " Revisões sobre o projeto original antes de sua execução poderão sofrer ajustes de valores e tempo, se necessário." ) );
	mCursor.insertBlock ();
	mCursor.insertText ( QStringLiteral ( " O material sobrante é de responsabilidade e propriedade do Vivência, ficando ao encargo deste a responsabilidade de retirá-lo do local da obra em sua integralidade." ) );
	mCursor.insertBlock ();
	mCursor.insertText ( QStringLiteral ( " O Vivência não é responsável pelo jardim ou obra depois de sua execução, a menos que fique encarregado, explicitamente, e mediante acordo, de zelar por sua manutenção." ) );
	mCursor.insertBlock ();
	mCursor.insertText ( QStringLiteral ( " O preço da mão-de-obra está condicionado ao preço do material. O serviço realizado somente com o fornecimento da mão-de-obra tem preço diverso e deverá ser consultado." ) );

	QTextBlockFormat newBlock;
	newBlock.setIndent ( 0 );
	mCursor.insertBlock ( newBlock );
	mCursor.insertBlock ( newBlock );
	mCursor.insertText ( QStringLiteral ( "Orçamento válido por 30 dias" ) );
	mCursor.insertBlock ( newBlock );
	TEXT_EDITOR_TOOLBAR ()->setFontSize ( fontsize );
	if ( c_id == -1 )
		setIgnoreCursorPos ( false );
}

void reportGenerator::btnInsertLogo_clicked ( const int c_id )
{
	setIgnoreCursorPos ( true );
	TEXT_EDITOR_TOOLBAR ()->insertImage ( CONFIG ()->vivienciaReportLogo (), QTextFrameFormat::FloatRight );
	if ( c_id == -1 )
		setIgnoreCursorPos ( false );
}

void reportGenerator::showProjectID ( const int )
{
	if ( mb_IgnoreChange )
		return;

	DOCK_QP ()->attach ( this );
	const bool b_hasData (
		QUICK_PROJECT ()->loadData (
			QUICK_PROJECT ()->getJobIDFromQPString ( cboQPIDs->currentText () ) )
	);
	btnCopyTableRow->setEnabled ( b_hasData );
	btnCopyEntireTable->setEnabled ( b_hasData );
}

const QString reportGenerator::initialDir () const
{
	QString dirstr;
	if ( !recStrValue ( rgJob, FLD_JOB_PROJECT_ID ).isEmpty () ) {
		if ( !recStrValue ( rgJob, FLD_JOB_PROJECT_PATH ).isEmpty () )
			dirstr = recStrValue ( rgJob, FLD_JOB_PROJECT_PATH );
	}
	if ( dirstr.isEmpty () ) {
		const QString client_name ( recStrValue ( rgClient, FLD_CLIENT_NAME ) );
		if ( !client_name.isEmpty () )
			dirstr = CONFIG ()->reportsDir ( client_name );
		else
			dirstr = CONFIG ()->projectsBaseDir ();
		fileOps::createDir ( dirstr ); // creates dir if it does not exist
	}
	return dirstr;
}

void reportGenerator::btnViewJob_clicked ( const bool checked )
{
	if ( checked )
		DOCK_BJ ()->attach ( this );
	else
		DOCK_BJ ()->detach ( this );
}

void reportGenerator::btnCopyTableRow_clicked ()
{
	setIgnoreCursorPos ( true );
	mCursor = mDocument->textCursor ();
	QTextTable* textTable ( mCursor.currentTable () );
	QTextCursor t_cursor = mCursor;
	const uint n_rows ( QUICK_PROJECT ()->table ()->lastUsedRow () );
	const uint n_selRows ( QUICK_PROJECT ()->table ()->selectedRowsCount () );
	const uint n_cols ( 4 ); // name - quantity - sell unity price - sell total price
	uint i_col ( 0 );

	QTextCharFormat boldFormat;
	boldFormat.setFontWeight ( QFont::Bold );

	if ( textTable != NULL ) {
		if ( textTable->columns () != 4 ) { // it's another table
			textTable = NULL;
			do {
				mCursor.movePosition ( QTextCursor::Down, QTextCursor::MoveAnchor );
			} while ( mCursor.currentTable () != NULL );
			mCursor.insertBlock ();
			mCursor.insertBlock ();
		}
	}
	if ( textTable == NULL ) { // No table under cursor. Create one
		spreadRow row;
		QUICK_PROJECT ()->getHeadersText ( &row );
		TEXT_EDITOR_TOOLBAR ()->spinNCols->setValue ( n_cols );
		TEXT_EDITOR_TOOLBAR ()->spinNRows->setValue ( n_selRows + 3 ); // 1 - Header; 2 - Empty row; 3 - Total row
		TEXT_EDITOR_TOOLBAR ()->btnCreateTable_clicked ();
		textTable = mCursor.currentTable ();
		// Despite the ui language, any program output should be in portuguese
		for ( ; i_col < n_cols; ++i_col ) {
			t_cursor = textTable->cellAt ( 0, i_col ).firstCursorPosition ();
			//t_cursor.insertText ( row.field_value.at ( i_col ) );
			switch ( i_col ) {
			case 0:
				t_cursor.insertText ( QStringLiteral ( "Item" ), boldFormat );
				break;
			case 1:
				t_cursor.insertText ( QStringLiteral ( "Quantidade" ), boldFormat );
				break;
			case 2:
				t_cursor.insertText ( QStringLiteral ( "Preço unitário (R$)" ), boldFormat );
				break;
			case 3:
				t_cursor.insertText ( QStringLiteral ( "Preço total (R$)" ), boldFormat );
				break;
			}
		}

		mCursor.movePosition ( QTextCursor::NextRow, QTextCursor::MoveAnchor );
		row.field_value.clear ();
	}
	vmNumber quantity ( 0.0 ), total ( 0.0 );
	int textTable_row ( 0 ), foundRow ( 0 );
	QStringList itemNames;
	QString name;
	bool ok ( true );
	for ( uint i_row ( 0 ); i_row < n_rows; ++i_row ) {
		if ( QUICK_PROJECT ()->table ()->isRowSelected ( i_row ) ) {
			textTable_row = textTable->cellAt ( mCursor ).row ();
			// When concatenating tables, we need to insert more rows. Append, then, the necessary amount of rows to the end of current table
			if ( textTable_row == ( textTable->rows () - 2 ) ) {
				textTable->insertRows ( textTable->cellAt ( mCursor ).row () + 1 ,n_selRows + 1 - i_row );
				TEXT_EDITOR_TOOLBAR ()->spinNRows->setValue ( TEXT_EDITOR_TOOLBAR ()->spinNRows->value () + n_selRows );
			}
			name = QUICK_PROJECT ()->table ()->sheetItem ( i_row, 0 )->text ();
			foundRow = itemNames.indexOf ( QRegExp ( name, Qt::CaseInsensitive ), 0 );
			if ( foundRow == -1 )
				itemNames.append ( name );
			else
				mCursor.movePosition ( QTextCursor::PreviousRow, QTextCursor::MoveAnchor, textTable_row - ( foundRow + 1 ) );
			t_cursor = textTable->cellAt ( textTable->cellAt ( mCursor ).row (), 0 ).firstCursorPosition ();
			t_cursor.insertText ( name );
			for ( i_col = 1; i_col < n_cols; ++i_col ) {
				t_cursor = textTable->cellAt ( textTable->cellAt ( mCursor ).row (), i_col ).firstCursorPosition ();
				quantity.fromTrustedStrPrice ( QUICK_PROJECT ()->table ()->sheetItem ( i_row, i_col )->text () );

				ok = true;
				if ( foundRow != -1 ) {
					if ( !quantity.isNull () ) {
						t_cursor = textTable->cellAt ( textTable->cellAt ( mCursor ).row (), i_col ).firstCursorPosition ();
						t_cursor.select ( QTextCursor::LineUnderCursor );
						t_cursor.selectedText ().toDouble ( &ok );
					}
					if ( !ok ) {
						/* As much as I'd like this code to be completely automatic and independent from any modifications done to the program elsewhere in the code
						I cannot add the individual items price because the resulting document would be pathetic. Since I don't want to create an option in QP to determine
						which columns values can and cannot be added, I manually insert this bit of code to skip adding. Should the program change the column layout, I will
						have to change this bit too.
						*/
						if ( i_col != 2 ) {
							t_cursor = textTable->cellAt ( textTable->cellAt ( mCursor ).row (), i_col ).firstCursorPosition ();
							t_cursor.select ( QTextCursor::LineUnderCursor );
							t_cursor.selectedText ().toDouble ( &ok );
						}
					}
				}
				if ( ok ) {
					quantity += t_cursor.selectedText ().toDouble ( &ok );
					t_cursor.insertText ( quantity.toStrDouble () );
				}
			}
			total += QUICK_PROJECT ()->table ()->sheetItem ( i_row, i_col - 1 )->number ();
			mCursor.movePosition ( QTextCursor::NextRow, QTextCursor::MoveAnchor, foundRow == -1 ? 1 : textTable_row - ( foundRow + 1 ) );
		}
	}
	textTable->removeRows ( textTable_row + 1, textTable->rows () - textTable_row - 2 ); // strip empty rows
	mCursor.movePosition ( QTextCursor::NextRow, QTextCursor::MoveAnchor, 1 );
	t_cursor = textTable->cellAt ( textTable->cellAt ( mCursor ).row (), 0 ).firstCursorPosition ();
	t_cursor.select ( QTextCursor::LineUnderCursor );
	if ( t_cursor.selectedText () != tr ( "TOTAL:" ) )
		t_cursor.insertText ( tr ( "TOTAL:" ), boldFormat );

	t_cursor = textTable->cellAt ( textTable->cellAt ( mCursor ).row (), 3 ).firstCursorPosition ();
	t_cursor.select ( QTextCursor::LineUnderCursor );
	total += vmNumber ( t_cursor.selectedText (), VMNT_PRICE );
	t_cursor.removeSelectedText ();
	t_cursor.insertText ( total.toPrice (), boldFormat );

	mCursor.setPosition ( textTable->cellAt ( 1, 0 ).firstCursorPosition ().position () );
	mDocument->setTextCursor ( mCursor );
	setIgnoreCursorPos ( false );
}

void reportGenerator::btnCopyEntireTable_clicked ()
{
	QUICK_PROJECT ()->table ()->selectAll ();
	btnCopyTableRow_clicked ();
	QUICK_PROJECT ()->table ()->selectRow ( 1 );
}

void reportGenerator::btnInsertClientAddress_clicked ( const int c_id )
{
	setIgnoreCursorPos ( true );

	QTextTable* textTable ( nullptr );
	QTextCursor t_cursor = mCursor;

	TEXT_EDITOR_TOOLBAR ()->spinNRows->setValue ( 3 );
	TEXT_EDITOR_TOOLBAR ()->spinNCols->setValue ( 2 );
	TEXT_EDITOR_TOOLBAR ()->btnCreateTable_clicked ();
	textTable = mCursor.currentTable ();

	if ( textTable != nullptr ) {
		TEXT_EDITOR_TOOLBAR ()->btnBold_clicked ( true );
		t_cursor = textTable->cellAt ( 0, 0 ).firstCursorPosition ();
		t_cursor.insertText ( QStringLiteral ( "Cliente: " ), mCursor.charFormat () );
		t_cursor = textTable->cellAt ( 1, 0 ).firstCursorPosition ();
		t_cursor.insertText ( QStringLiteral ( "Endereço: " ), mCursor.charFormat () );
		t_cursor = textTable->cellAt ( 2, 0 ).firstCursorPosition ();
		t_cursor.insertText ( QStringLiteral ( "Contato (s): " ), mCursor.charFormat () );
		TEXT_EDITOR_TOOLBAR ()->btnBold_clicked ( false );

		if ( recIntValue ( rgClient, FLD_CLIENT_ID ) != -1 ) {
			TEXT_EDITOR_TOOLBAR ()->setFontColor ( QColor ( Qt::blue ) );
			t_cursor = textTable->cellAt ( 0, 1 ).firstCursorPosition ();
			t_cursor.insertText ( recStrValue ( rgClient, FLD_CLIENT_NAME ), mCursor.charFormat () );
			t_cursor = textTable->cellAt ( 1, 1 ).firstCursorPosition ();
			t_cursor.insertText ( recStrValue ( rgClient, FLD_CLIENT_STREET )
								  + ", " + recStrValue ( rgClient, FLD_CLIENT_NUMBER ) + QLatin1String ( " - " )
								  + recStrValue ( rgClient, FLD_CLIENT_DISTRICT ) + QLatin1String ( " - " )
								  + recStrValue ( rgClient, FLD_CLIENT_CITY ), mCursor.charFormat () );
			t_cursor = textTable->cellAt ( 2, 1 ).firstCursorPosition ();
			t_cursor.insertText ( stringRecord::fieldValue ( recStrValue ( rgClient, FLD_CLIENT_PHONES ), 0 ) +
								  QLatin1String ( " / " ) +
								  stringRecord::fieldValue ( recStrValue ( rgClient, FLD_CLIENT_PHONES ), 1 ) +
								  QLatin1String ( " / " ) +
								  recStrValue ( rgClient, FLD_CLIENT_EMAIL ),mCursor.charFormat () );
			TEXT_EDITOR_TOOLBAR ()->setFontColor ( QColor ( Qt::black ) );
		}
		else
			t_cursor.movePosition ( QTextCursor::NextCell, QTextCursor::MoveAnchor );

		mCursor.movePosition ( QTextCursor::Down, QTextCursor::MoveAnchor, 4 );
		mCursor.insertText ( CHR_NEWLINE + CHR_NEWLINE );
	}

	if ( c_id == -1 )
		setIgnoreCursorPos ( false );
}

void reportGenerator::btnInsertJobReport_clicked ()
{
	if ( recIntValue ( rgJob, FLD_JOB_ID ) != -1 ) {
		TEXT_EDITOR_TOOLBAR ()->btnAlignJustify_clicked ();
		mCursor.insertText ( recStrValue ( rgJob, FLD_JOB_REPORT ) );
		mCursor.insertBlock ();
	}
}

void reportGenerator::buildMailMessage ( QString& address, QString& subject, QString& attachment, QString& body )
{
	textEditor::buildMailMessage ( address, subject, attachment, body );
	if ( recIntValue ( rgClient, FLD_CLIENT_ID ) != -1 )
		address = recStrValue ( rgClient, FLD_CLIENT_EMAIL );

	subject = strHeaderType[cboHeaderType->currentIndex ()] + QString::fromUtf8 ( "na área de" ) + strJobType[cboHeaderJobType->currentIndex ()];
}

void reportGenerator::btnGenerateReport_clicked ( const int c_id, const bool include_all_paid )
{
	setIgnoreCursorPos ( true );

	if ( c_id == -1 ) {
		if (
			( recStrValue ( rgJob, FLD_JOB_TYPE ).indexOf ( QStringLiteral ( "Manutenção" ) ) != -1 ) ||
			( recStrValue ( rgJob, FLD_JOB_TYPE ).indexOf ( QStringLiteral ( "Limpeza" ) ) != -1 ) || ( recStrValue ( rgJob, FLD_JOB_TYPE ).indexOf ( QStringLiteral ( "Plantio" ) ) != -1 ) ||
			( recStrValue ( rgJob, FLD_JOB_TYPE ).indexOf ( QStringLiteral ( "Poda" ) ) != -1 )
		)
			cboHeaderJobType->setCurrentIndex ( cboHeaderJobType->findText ( STR_HEADERJOBTYPE_LABEL.at ( GARDENING ) ) );
		else if
		(
			recStrValue ( rgJob, FLD_JOB_TYPE ).indexOf ( QStringLiteral ( "Serviço" ) ) != -1
		)
			cboHeaderJobType->setCurrentIndex ( cboHeaderJobType->findText ( STR_HEADERJOBTYPE_LABEL.at ( GENERAL ) ) );
		else if
		(
			( recStrValue ( rgJob, FLD_JOB_TYPE ).indexOf ( QStringLiteral ( "Reforma" ) ) != -1 ) || ( recStrValue ( rgJob, FLD_JOB_TYPE ).indexOf ( QStringLiteral ( "Conserto" ) ) != -1 ) ||
			( recStrValue ( rgJob, FLD_JOB_TYPE ).indexOf ( QStringLiteral ( "Construção" ) ) != -1 )
		)
			cboHeaderJobType->setCurrentIndex ( cboHeaderJobType->findText ( STR_HEADERJOBTYPE_LABEL.at ( MASONRY ) ) );
		else if ( recStrValue ( rgJob, FLD_JOB_TYPE ).indexOf ( QStringLiteral ( "Pintura" ) ) != -1 )
			cboHeaderJobType->setCurrentIndex ( cboHeaderJobType->findText ( STR_HEADERJOBTYPE_LABEL.at ( PAINTING ) ) );
		else if ( recStrValue ( rgJob, FLD_JOB_TYPE ).indexOf ( QStringLiteral ( "Paisagismo" ) ) != -1 )
			cboHeaderJobType->setCurrentIndex ( cboHeaderJobType->findText ( STR_HEADERJOBTYPE_LABEL.at ( LANDSCAPE ) ) );
		else
			cboHeaderJobType->setCurrentIndex ( cboHeaderJobType->findText ( STR_HEADERJOBTYPE_LABEL.at ( JOB_OTHER ) ) );
	}
	else {
		rgClient->readRecord ( c_id );
		cboHeaderJobType->setCurrentIndex ( cboHeaderJobType->findText ( STR_HEADERJOBTYPE_LABEL.at ( GENERAL ) ) );
	}

	btnInsertHeader_clicked ( c_id );
	btnInsertLogo_clicked ( c_id );
	mCursor.movePosition ( QTextCursor::Down, QTextCursor::MoveAnchor );
	mCursor.movePosition ( QTextCursor::EndOfLine, QTextCursor::MoveAnchor );
	btnInsertClientAddress_clicked ( c_id );
	mCursor.movePosition ( QTextCursor::Down, QTextCursor::MoveAnchor, 7 );
	mCursor.insertText ( CHR_NEWLINE + CHR_NEWLINE );
	mCursor.movePosition ( QTextCursor::Up, QTextCursor::MoveAnchor, 2 );
	mDocument->setTextCursor ( mCursor );

	switch ( cboHeaderType->currentIndex () ) {
	case REPORT:
		insertReportsText ( include_all_paid );
		mCursor.insertText ( QString ( CHR_NEWLINE ) );
		mDocument->setTextCursor ( mCursor );
		btnInsertFooter_clicked ( c_id );
		break;
	case PAYMENT_STUB:
		insertPaymentStubText ();
		mCursor.insertText ( "\n" );
		mDocument->setTextCursor ( mCursor );
		btnInsertFooter_clicked ( c_id );
		break;
	case PROJECT:
	case OTHER:
		insertReadCarefullyBlock ();
		TEXT_EDITOR_TOOLBAR ()->btnInsertBulletList_cliked ();
		mCursor.movePosition ( QTextCursor::Down, QTextCursor::MoveAnchor, 2 );
		mCursor.insertText ( "\n" );
		mDocument->setTextCursor ( mCursor );
		btnInsertFooter_clicked ( c_id );
		mCursor.movePosition ( QTextCursor::Down, QTextCursor::MoveAnchor, 5 );
		mCursor.insertText ( CHR_NEWLINE + CHR_NEWLINE );
		mDocument->setTextCursor ( mCursor );
		btnInsertFootNotes_clicked ( c_id );
		break;
	}
	setIgnoreCursorPos ( false );
}

void reportGenerator::insertReadCarefullyBlock ()
{
	TEXT_EDITOR_TOOLBAR ()->btnAlignCenter_clicked ();
	TEXT_EDITOR_TOOLBAR ()->btnItalic_clicked ( false );
	TEXT_EDITOR_TOOLBAR ()->btnBold_clicked ( true );
	TEXT_EDITOR_TOOLBAR ()->btnUnderline_clicked ( true );
	TEXT_EDITOR_TOOLBAR ()->setFontColor ( QColor ( Qt::black ) );
	mCursor.insertText ( QStringLiteral ( "Descrição dos serviços\n" ) );
	TEXT_EDITOR_TOOLBAR ()->btnAlignCenter_clicked ();
	mCursor.insertText ( QStringLiteral ( "Leia com atenção!" ) );
	TEXT_EDITOR_TOOLBAR ()->btnUnderline_clicked ( false );
	TEXT_EDITOR_TOOLBAR ()->btnBold_clicked ( false );
	TEXT_EDITOR_TOOLBAR ()->btnAlignLeft_clicked ();
	mCursor.insertText ( CHR_NEWLINE + CHR_NEWLINE );
}

void reportGenerator::insertReportsText ( const bool include_all_paid )
{
	if ( recIntValue ( rgClient, FLD_CLIENT_ID ) != -1 ) {
		TEXT_EDITOR_TOOLBAR ()->btnAlignLeft_clicked ();
		mCursor.insertBlock ();
		mCursor.insertBlock ();
		mCursor.movePosition ( QTextCursor::Up, QTextCursor::MoveAnchor, 2 );

		vmNumber jobs_total ( 0.0 ), paid_total ( 0.0 );

		if ( rgPay->readFirstRecord ( FLD_PAY_CLIENTID, recStrValue ( rgClient, FLD_CLIENT_ID ) ) ) {
			TEXT_EDITOR_TOOLBAR ()->btnStrikethrough_clicked ( false );
			TEXT_EDITOR_TOOLBAR ()->btnItalic_clicked ( false );
			TEXT_EDITOR_TOOLBAR ()->btnBold_clicked ( true );
			TEXT_EDITOR_TOOLBAR ()->btnUnderline_clicked ( true );
			mCursor.insertText ( QStringLiteral ( "Pagamento devido ao(s) seguinte(s) serviço(s) executado(s):" ) );
			TEXT_EDITOR_TOOLBAR ()->btnUnderline_clicked ( false );
			TEXT_EDITOR_TOOLBAR ()->btnBold_clicked ( false );
			mCursor.insertBlock ();
			TEXT_EDITOR_TOOLBAR ()->btnInsertBulletList_cliked ();

			do {
				if ( recStrValue ( rgPay, FLD_PAY_OVERDUE ) == CHR_ONE ) {
					if ( rgJob->readRecord ( recStrValue ( rgPay, FLD_PAY_JOBID ).toInt () ) ) {
						if ( !jobs_total.isNull () )
							mCursor.insertBlock ();
						mCursor.insertText ( recStrValue ( rgJob, FLD_JOB_TYPE ) );
						mCursor.insertText (
							rgJob->date ( FLD_JOB_STARTDATE ) == rgJob->date ( FLD_JOB_ENDDATE ) ?
							QStringLiteral ( ", realizado em " ) : QStringLiteral ( ", finalizado em " ) );
						mCursor.insertText ( rgJob->date ( FLD_JOB_ENDDATE ).toDate ( vmNumber::VDF_HUMAN_DATE ) );
						mCursor.insertText ( QStringLiteral ( " - " ) );
						TEXT_EDITOR_TOOLBAR ()->btnBold_clicked ( true );
						mCursor.insertText ( recStrValue ( rgPay, FLD_PAY_OVERDUE_VALUE ) );
						TEXT_EDITOR_TOOLBAR ()->btnBold_clicked ( false );
						jobs_total += rgPay->price ( FLD_PAY_PRICE );
					}
				}
			} while ( rgPay->readNextRecord ( true ) );

			mCursor.movePosition ( QTextCursor::Down, QTextCursor::MoveAnchor, 2 );
			if ( !jobs_total.isNull () ) {
				mCursor.insertBlock ();
				TEXT_EDITOR_TOOLBAR ()->btnAlignCenter_clicked ();
				TEXT_EDITOR_TOOLBAR ()->btnBold_clicked ( true );
				TEXT_EDITOR_TOOLBAR ()->btnItalic_clicked ( true );
				mCursor.insertText ( QStringLiteral ( "Total dos serviços: " ) + jobs_total.toPrice () );
				mCursor.insertBlock ();
				mCursor.insertBlock ();
				mCursor.insertText ( QStringLiteral ( "____________________________________________________________________________" ) );
				TEXT_EDITOR_TOOLBAR ()->btnItalic_clicked ( false );
				TEXT_EDITOR_TOOLBAR ()->btnBold_clicked ( false );
				TEXT_EDITOR_TOOLBAR ()->btnAlignLeft_clicked ();
				mCursor.insertBlock ();
				mCursor.insertBlock ();
			}
		}

		if ( include_all_paid ) {
			if ( rgPay->readFirstRecord ( FLD_PAY_CLIENTID, recStrValue ( rgClient, FLD_CLIENT_ID ) ) ) {
				vmNumber twelveMonthsAgo ( vmNumber::currentDate );
				twelveMonthsAgo.setDate ( 0, -12, 0, true );
				TEXT_EDITOR_TOOLBAR ()->btnBold_clicked ( true );
				TEXT_EDITOR_TOOLBAR ()->btnUnderline_clicked ( true );
				mCursor.insertText ( QStringLiteral ( "Pagamentos realizados nos últimos 12 meses:" ) );
				TEXT_EDITOR_TOOLBAR ()->btnUnderline_clicked ( false );
				TEXT_EDITOR_TOOLBAR ()->btnBold_clicked ( false );
				mCursor.insertBlock ();
				TEXT_EDITOR_TOOLBAR ()->btnInsertBulletList_cliked ();
				/*do {
					if ( rgPay->opt ( FLD_PAY_PAID ) ) {
						if ( rgPay->date ( FLD_PAY_DATE ) >= twelveMonthsAgo ) {
							if ( paid_total != 0.0 )
								mCursor.insertBlock ();
							TEXT_EDITOR_TOOLBAR ()->btnBold_clicked ( true );
							mCursor.insertText ( rgPay->actualRecordStr ( FLD_PAY_PRICE ) );
							TEXT_EDITOR_TOOLBAR ()->btnBold_clicked ( false );
							mCursor.insertText ( " em " );
							mCursor.insertText ( rgPay->date ( FLD_PAY_DATE ).toHumanDate () );
							if ( rgJob->readRecord ( rgPay->actualRecordStr ( FLD_PAY_JOBID ).toInt () ) ) {
								mCursor.insertText ( QString::fromUtf8 ( ", referente ao serviço " ) );
								mCursor.insertText ( recStrValue ( rgJob, FLD_JOB_TYPE ) );
								mCursor.insertText ( ", finalizado em " );
								mCursor.insertText ( rgJob->date ( FLD_JOB_ENDDATE ).toHumanDate () );
							}
							mCursor.insertText ( ";" );
							paid_total += rgPay->price ( FLD_PAY_VALUE );
						}
					}
				} while ( rgPay->readNextRecord ( true ) );*/
			}
			mCursor.movePosition ( QTextCursor::Down, QTextCursor::MoveAnchor, 2 );
			if ( !paid_total.isNull () ) {
				mCursor.insertBlock ();
				TEXT_EDITOR_TOOLBAR ()->btnAlignCenter_clicked ();
				TEXT_EDITOR_TOOLBAR ()->btnBold_clicked ( true );
				TEXT_EDITOR_TOOLBAR ()->btnItalic_clicked ( true );
				mCursor.insertText ( QString ( QStringLiteral ( "Total dos pagamentos: %1" ) ).arg ( paid_total.toPrice () ) );
				mCursor.insertBlock ();
				mCursor.insertBlock ();
				mCursor.insertText ( "____________________________________________________________________________" );
				TEXT_EDITOR_TOOLBAR ()->btnItalic_clicked ( false );
				TEXT_EDITOR_TOOLBAR ()->btnBold_clicked ( false );
				TEXT_EDITOR_TOOLBAR ()->btnAlignLeft_clicked ();
				mCursor.insertBlock ();
				mCursor.insertBlock ();
			}
		}
		if ( !jobs_total.isNull () ) { //there are pending payments
			TEXT_EDITOR_TOOLBAR ()->btnBold_clicked ( true );
			TEXT_EDITOR_TOOLBAR ()->btnUnderline_clicked ( true );
			mCursor.insertText ( QString ( QStringLiteral ( "Total devido até %1: %2" ) )
								 .arg ( vmNumber::currentDate.toString (), jobs_total.toPrice () ) );
			TEXT_EDITOR_TOOLBAR ()->btnUnderline_clicked ( false );
			TEXT_EDITOR_TOOLBAR ()->btnBold_clicked ( false );
			mCursor.insertBlock ();
			mCursor.insertBlock ();
		}
	}
}

void reportGenerator::insertPaymentStubText ()
{
	TEXT_EDITOR_TOOLBAR ()->btnAlignLeft_clicked ();

	TEXT_EDITOR_TOOLBAR ()->btnBold_clicked ( false );
	TEXT_EDITOR_TOOLBAR ()->btnUnderline_clicked ( false );
	mCursor.insertText ( QStringLiteral ( "Eu, " ) );
	TEXT_EDITOR_TOOLBAR ()->btnUnderline_clicked ( true );
	TEXT_EDITOR_TOOLBAR ()->btnBold_clicked ( true );
	mCursor.insertText ( QStringLiteral ( "Guilherme Fortunato, " ) );

	TEXT_EDITOR_TOOLBAR ()->btnBold_clicked ( false );
	TEXT_EDITOR_TOOLBAR ()->btnUnderline_clicked ( false );
	mCursor.insertText ( QStringLiteral ( "declaro que recebi de " ) );
	TEXT_EDITOR_TOOLBAR ()->btnBold_clicked ( true );
	TEXT_EDITOR_TOOLBAR ()->btnUnderline_clicked ( true );
	mCursor.insertText ( txtClientsIDs->text () );

	TEXT_EDITOR_TOOLBAR ()->btnBold_clicked ( false );
	TEXT_EDITOR_TOOLBAR ()->btnUnderline_clicked ( false );
	mCursor.insertText ( QStringLiteral ( ", em " ) );
	TEXT_EDITOR_TOOLBAR ()->btnBold_clicked ( true );
	TEXT_EDITOR_TOOLBAR ()->btnUnderline_clicked ( true );
//	mCursor.insertText ( rgPay->date ( FLD_PAY_DATE ).toLongDate () );

	TEXT_EDITOR_TOOLBAR ()->btnBold_clicked ( false );
	TEXT_EDITOR_TOOLBAR ()->btnUnderline_clicked ( false );
	mCursor.insertText ( QStringLiteral ( ", a quantia de " ) );
	TEXT_EDITOR_TOOLBAR ()->btnBold_clicked ( true );
	TEXT_EDITOR_TOOLBAR ()->btnUnderline_clicked ( true );
	//mCursor.insertText ( rgPay->actualRecordStr ( FLD_PAY_VALUE ) );

	TEXT_EDITOR_TOOLBAR ()->btnBold_clicked ( false );
	TEXT_EDITOR_TOOLBAR ()->btnUnderline_clicked ( false );
	mCursor.insertText ( QStringLiteral ( " , referente ao serviço de " ) );
	TEXT_EDITOR_TOOLBAR ()->btnBold_clicked ( true );
	TEXT_EDITOR_TOOLBAR ()->btnUnderline_clicked ( true );
	mCursor.insertText ( recStrValue ( rgJob, FLD_JOB_TYPE ) );

	TEXT_EDITOR_TOOLBAR ()->btnBold_clicked ( false );
	TEXT_EDITOR_TOOLBAR ()->btnUnderline_clicked ( false );
	mCursor.insertText ( QStringLiteral ( " finalizado em " ) );
	TEXT_EDITOR_TOOLBAR ()->btnBold_clicked ( true );
	TEXT_EDITOR_TOOLBAR ()->btnUnderline_clicked ( true );
	mCursor.insertText ( rgJob->date ( FLD_JOB_ENDDATE ).toDate ( vmNumber::VDF_LONG_DATE ) );

	TEXT_EDITOR_TOOLBAR ()->btnBold_clicked ( false );
	TEXT_EDITOR_TOOLBAR ()->btnUnderline_clicked ( false );
	mCursor.insertText ( "." );
	mCursor.insertBlock ();
	mCursor.insertBlock ();

	mCursor.insertText ( QStringLiteral ( "Para clareza, firmo o presente" ) );
	mCursor.insertBlock ();
	mCursor.insertText ( QStringLiteral ( "São Pedro, " ) );

	TEXT_EDITOR_TOOLBAR ()->btnBold_clicked ( true );
	TEXT_EDITOR_TOOLBAR ()->btnUnderline_clicked ( true );
	mCursor.insertText ( vmNumber::currentDate.toDate ( vmNumber::VDF_LONG_DATE ) );
	TEXT_EDITOR_TOOLBAR ()->btnBold_clicked ( false );
	TEXT_EDITOR_TOOLBAR ()->btnUnderline_clicked ( false );

	mCursor.insertBlock ();
	mCursor.insertBlock ();
	mCursor.insertText ( QStringLiteral ( "Assinatura:__________________________________________" ) );
	mCursor.insertBlock ();
	mCursor.insertText ( QStringLiteral ( "CPF: 294.655.068-00 / RG: 30.258.261-7" ) );
	mCursor.insertBlock ();

	mDocument->setFocus ();
}

dockQP* dockQP::s_instance = nullptr;

void deleteDockQPInstance ()
{
	heap_del ( dockQP::s_instance );
}

dockQP::dockQP ()
	: QDockWidget ( tr ( "Estimates tables" ) ), instances ( 5 )
{
	QWidget *mainWidget = new QWidget;
	mainLayout = new QVBoxLayout;
	mainWidget->setLayout ( mainLayout );
	setWidget ( mainWidget );
	addPostRoutine ( deleteDockQPInstance );
}

void dockQP::attach ( reportGenerator* instance )
{
	if ( !isVisible () ) {
		QUICK_PROJECT ()->table ()->addToLayout ( mainLayout );
		show ();
		EDITOR ()->addDockWindow ( Qt::BottomDockWidgetArea, this );
	}
	if ( instances.contains ( instance ) == -1 )
		instances.append ( instance );
	cur_report = instance;
}

void dockQP::detach ( reportGenerator* instance )
{
	instances.removeOne ( instance );
	if ( instances.isEmpty () ) {
		if ( isVisible () ) {
			hide ();
			EDITOR ()->removeDockWindow ( this );
		}
	}
}

void dockQP::setCurrentWindow ( reportGenerator *report )
{
	if ( report && report != cur_report ) {
		cur_report = report;
		cur_report->showProjectID ();
	}
}

dockBJ* dockBJ::s_instance = nullptr;

void deleteDockBJInstance ()
{
	heap_del ( dockBJ::s_instance );
}

dockBJ::dockBJ ()
	: QDockWidget ( tr ( "Job's brief information" ) ), instances ( 5 )
{
	frmBriefJob = new QFrame;
	frmBriefJob->setFrameStyle ( QFrame::Raised | QFrame::StyledPanel );

	txtBriefJobType = new QLineEdit;
	txtBriefJobDate = new QLineEdit;
	txtBriefJobPrice = new QLineEdit;
	txtBriefJobReport = new QTextEdit;
	txtBriefJobType->setReadOnly ( true );
	txtBriefJobDate->setReadOnly ( true );
	txtBriefJobPrice->setReadOnly ( true );
	txtBriefJobReport->setReadOnly ( true );

	QToolButton* btnCopyJobType = new QToolButton;
	btnCopyJobType->setFixedSize ( 30, 30 );
	btnCopyJobType->setIcon ( ICON ( "document-new" ) );
	btnCopyJobType->setToolTip ( tr ( "Copy to report" ) );
	connect ( btnCopyJobType, SIGNAL ( clicked () ), this, SLOT ( btnCopyJobType_clicked () ) );
	QToolButton* btnCopyJobDate = new QToolButton;
	btnCopyJobDate->setFixedSize ( 30, 30 );
	btnCopyJobDate->setIcon ( ICON ( "document-new" ) );
	btnCopyJobDate->setToolTip ( tr ( "Copy to report" ) );
	connect ( btnCopyJobDate, SIGNAL ( clicked () ), this, SLOT ( btnCopyJobDate_clicked () ) );
	QToolButton* btnCopyJobPrice = new QToolButton;
	btnCopyJobPrice->setFixedSize ( 30, 30 );
	btnCopyJobPrice->setIcon ( ICON ( "document-new" ) );
	btnCopyJobPrice->setToolTip ( tr ( "Copy to report" ) );
	connect ( btnCopyJobPrice, SIGNAL ( clicked () ), this, SLOT ( btnCopyJobPrice_clicked () ) );
	QToolButton* btnCopyJobReport = new QToolButton;
	btnCopyJobReport->setFixedSize ( 30, 30 );
	btnCopyJobReport->setIcon ( ICON ( "document-new" ) );
	btnCopyJobReport->setToolTip ( tr ( "Copy to report" ) );
	connect ( btnCopyJobReport, SIGNAL ( clicked () ), this, SLOT ( btnCopyJobReport_clicked () ) );

	QGridLayout* gLayout = new QGridLayout;
	gLayout->addWidget ( txtBriefJobType, 0, 0 );
	gLayout->addWidget ( btnCopyJobType, 0, 1 );
	gLayout->setColumnStretch ( 0, 1 );
	gLayout->setColumnStretch ( 1, 0 );
	gLayout->addWidget ( txtBriefJobDate, 0, 2 );
	gLayout->addWidget ( btnCopyJobDate, 0, 3 );
	gLayout->setColumnStretch ( 2, 1 );
	gLayout->setColumnStretch ( 3, 0 );
	gLayout->addWidget ( txtBriefJobPrice, 0, 4 );
	gLayout->addWidget ( btnCopyJobPrice, 0, 5 );
	gLayout->setColumnStretch ( 4, 1 );
	gLayout->setColumnStretch ( 5, 0 );
	gLayout->addWidget ( btnCopyJobReport, 0, 6, 0, 6, Qt::AlignRight );
	gLayout->setColumnStretch ( 6, 0 );
	gLayout->addWidget( txtBriefJobReport, 1, 0, 2, 6 );
	gLayout->setRowStretch ( 0, 0 );
	gLayout->setRowStretch ( 1, 1 );

	frmBriefJob->setLayout ( gLayout );
	setWidget ( frmBriefJob );
	addPostRoutine ( deleteDockBJInstance );
}

void dockBJ::attach ( reportGenerator* instance )
{
	if ( !isVisible () ) {
		show ();
		EDITOR ()->addDockWindow ( Qt::BottomDockWidgetArea, this );
	}
	if ( instances.contains ( instance ) == -1 )
		instances.append ( instance );
	cur_report = instance;
}

void dockBJ::detach ( reportGenerator* instance )
{
	instances.removeOne ( instance );
	if ( instances.isEmpty () ) {
		if ( isVisible () ) {
			hide ();
			EDITOR ()->removeDockWindow ( this );
		}
	}
}

void dockBJ::setCurrentWindow ( reportGenerator *report )
{
	if ( report && report != cur_report ) {
		cur_report = report;
		fillControls  ( cur_report->rgJob );
	}
}

void dockBJ::fillControls ( const Job *job )
{
	if ( job ) {
		txtBriefJobType->setText ( recStrValue ( job, FLD_JOB_TYPE ) );
		txtBriefJobDate->setText ( recStrValue ( job, FLD_JOB_STARTDATE ) );
		txtBriefJobPrice->setText ( recStrValue ( job, FLD_JOB_PRICE ) );
		txtBriefJobReport->setPlainText ( recStrValue ( job, FLD_JOB_REPORT ) );
	}
	else {
		txtBriefJobType->clear ();
		txtBriefJobDate->clear ();
		txtBriefJobPrice->clear ();
		txtBriefJobReport->clear ();
	}
}

void dockBJ::btnCopyJobType_clicked ()
{
	if ( cur_report != nullptr ) {
		if ( !txtBriefJobType->text ().isEmpty () ) {
			cur_report->setIgnoreCursorPos ( true );
			cur_report->mCursor.insertText ( txtBriefJobType->text () );
			cur_report->setIgnoreCursorPos ( false );
		}
	}
}

void dockBJ::btnCopyJobDate_clicked ()
{
	if ( cur_report != nullptr ) {
		if ( !txtBriefJobDate->text ().isEmpty () ) {
			cur_report->setIgnoreCursorPos ( true );
			cur_report->mCursor.insertText ( txtBriefJobDate->text () );
			cur_report->setIgnoreCursorPos ( false );
		}
	}
}

void dockBJ::btnCopyJobPrice_clicked ()
{
	if ( cur_report != nullptr ) {
		if ( !txtBriefJobPrice->text ().isEmpty () ) {
			cur_report->setIgnoreCursorPos ( true );
			cur_report->mCursor.setPosition ( cur_report->mDocument->textCursor ().position () );
			cur_report->mCursor.insertText ( txtBriefJobPrice->text () );
			cur_report->setIgnoreCursorPos ( false );
		}
	}
}

void dockBJ::btnCopyJobReport_clicked ()
{
	if ( cur_report != nullptr ) {
		cur_report->setIgnoreCursorPos ( true );
		cur_report->mCursor.insertText ( txtBriefJobReport->document ()->toPlainText () );
		cur_report->setIgnoreCursorPos ( false );
	}
}
