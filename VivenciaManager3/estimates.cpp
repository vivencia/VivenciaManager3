#include "global.h"
#include "estimates.h"
#include "configops.h"
#include "fileops.h"
#include "data.h"
#include "vmnotify.h"
#include "vivenciadb.h"
#include "vmwidgets.h"
#include "mainwindow.h"
#include "documenteditor.h"
#include "reportgenerator.h"
#include "texteditor.h"
#include "emailconfigdialog.h"
#include "cleanup.h"
#include "heapmanager.h"
#include "vmlistitem.h"
#include "newprojectdialog.h"

#include <QDate>
#include <QAction>
#include <QToolButton>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QSqlQuery>

estimateDlg* estimateDlg::s_instance ( nullptr );

static QToolButton* btnDoc ( nullptr );
static QToolButton* btnXls ( nullptr );
static QToolButton* btnPdf ( nullptr );
static QToolButton* btnVmr ( nullptr );
static QToolButton* btnFileManager ( nullptr );
static QToolButton* btnReload ( nullptr );
static QPushButton* btnProject ( nullptr );
static QPushButton* btnEstimate ( nullptr );
static QPushButton* btnReport ( nullptr );
static QPushButton* btnClose ( nullptr );
static QMenu* menuDoc ( nullptr );
static QMenu* menuVmr ( nullptr );
static QMenu* menuXls ( nullptr );
static QMenu* menuPdf ( nullptr );
static QMenu* menuFileMngr ( nullptr );
static QMenu* menuProject ( nullptr );
static QMenu* menuEstimate ( nullptr );
static QMenu* menuReport ( nullptr );
static QTreeWidget* treeView ( nullptr );
static QIcon* iconsType[5] = { nullptr };

static const uint TYPE_PROJECT_ITEM ( 0 );
static const uint TYPE_ESTIMATE_ITEM ( 1 );
static const uint TYPE_REPORT_ITEM ( 2 );
static const uint TYPE_PROJECT_DIR ( 3 );
static const uint TYPE_ESTIMATE_DIR ( 4 );
static const uint TYPE_REPORT_DIR ( 5 );
static const uint TYPE_CLIENT_TOPLEVEL ( 1000 );

const int ROLE_ITEM_TYPE ( Qt::UserRole );
const int ROLE_ITEM_FILENAME ( Qt::UserRole + 1 );
const int ROLE_ITEM_CLIENTNAME ( Qt::UserRole + 2 );

enum FILETYPES
{
	FILETYPE_DOC = 0, FILETYPE_XLS = 1, FILETYPE_PDF = 2, FILETYPE_VMR = 3, FILETYPE_UNKNOWN = 4
};

enum SINGLE_FILE_OPS
{
	SFO_OPEN_FILE = 0, SFO_SEND_EMAIL = 1, SFO_REMOVE_FILE = 3
};

enum FILEMANAGER_ACTIONS
{
	OPEN_SELECTED_DIR = 10, OPEN_ESTIMATES_DIR = 11, OPEN_REPORTS_DIR = 12, OPEN_CLIENT_DIR = 13
};

enum PROJECT_ACTIONS
{
	PA_NEW_FULL = 0, PA_NEW_EMPTY = 1, PA_RENAME = 2, PA_REMOVE = 3, PA_ADD_DOC = 4, PA_ADD_XLS = 5
};

enum ESTIMATE_ACTIONS
{
	EA_NEW_EXTERNAL = 0, EA_NEW_VMR = 1, EA_CONVERT = 2, EA_REMOVE = 3, EA_ADD_DOC = 4, EA_ADD_XLS = 5
};

enum REPORT_ACTIONS
{
	RA_NEW_EXTERNAL = 0, RA_NEW_EXTERNAL_COMPLETE = 1, RA_NEW_VMR = 2, RA_REMOVE = 3
};

static const QString nonClientStr ( QStringLiteral ( "Não-clientes" ) );
static const QString nonClientEstimatesPrefix ( QStringLiteral ( "Orçamento - " ) );
static const QString nonClientReportsPrefix ( QStringLiteral ( "Relatório - " ) );
static const QString itemTypeStr[6] =
{
	QStringLiteral ( "Projeto" ), QStringLiteral ( "Orçamento" ), QStringLiteral ( "Relatório" ),
	QStringLiteral ( "Diretório de Projeto" ), QStringLiteral ( "Diretório de Orçamentos" ), QStringLiteral ( "Diretório de Relatórios" )
};

//------------------------------------ESTIMATES------------------------------------------
void deleteEstimateInstance ()
{
	heap_del ( estimateDlg::s_instance );
}

// This func () is execed by the actions initiated from the buttons, which, in turn, will only
// be enabled if it's guaranteed they will produce a valid item and, therefore, valid filenames
#define execMenuAction(action) \
	execAction ( treeView->selectedItems ().at ( 0 ), \
				 static_cast<vmAction*> ( action )->id () );

inline static QString getNameFromProjectFile ( const QString& filename )
{
	return filename.left ( filename.indexOf ( CHR_DOT, 1 ) );
}

static QString getDateFromProjectFile ( const QString& filename )
{
	bool b_digit ( false );
	QString strDate;
	QString::const_iterator itr ( filename.constBegin () );
	const QString::const_iterator itr_end ( filename.constEnd () );
	for ( ; itr != itr_end; ++itr )
	{
		if ( b_digit )
		{
			b_digit = static_cast<QChar> ( *itr ).isDigit ();
			if ( b_digit )
				strDate += static_cast<QChar> ( *itr );
		}
		else
		{
			b_digit = static_cast<QChar> ( *itr ).isDigit ();
			if ( !b_digit )
			{
				if ( strDate.length () < 4 )
					strDate.clear ();
			}
			else
				strDate += static_cast<QChar> ( *itr );
		}
	}
	if ( strDate.length () >= 4 )
	{
		const vmNumber date ( strDate, VMNT_DATE );
		return date.toDate ( vmNumber::VDF_HUMAN_DATE );
	}
	return emptyString;
}

static QString getDateFromProjectDir ( const QString& filename )
{
	vmNumber date;
	date.fromStrDate ( filename.left ( filename.indexOf ( CHR_SPACE ) ) );
	return date.toDate ( vmNumber::VDF_HUMAN_DATE );
}

static QString getNameFromEstimateFile ( const QString& filename )
{
	const QString estimateFilePrefix ( filename.startsWith ( nonClientEstimatesPrefix ) ?
										 nonClientEstimatesPrefix : emptyString );
	const int idx ( filename.indexOf ( CHR_HYPHEN, estimateFilePrefix.count () + 1 ) );
	if ( idx != -1 )
		return filename.mid ( idx + 2, filename.indexOf ( CHR_DOT, idx + 2 ) - ( idx + 2 ) );
	return emptyString;
}

static QString getDateFromEstimateFile ( const QString& filename )
{
	const QString estimateFilePrefix ( filename.startsWith ( nonClientEstimatesPrefix ) ?
										 nonClientEstimatesPrefix : emptyString );

	const int idx ( filename.indexOf ( CHR_HYPHEN, estimateFilePrefix.count () + 1 ) );
	if ( idx != -1 ) {
		const QString datestr ( filename.mid (
									estimateFilePrefix.count (), idx - estimateFilePrefix.count () - 1 ) );
		vmNumber date ( datestr, VMNT_DATE, vmNumber::VDF_FILE_DATE );
		return date.toDate( vmNumber::VDF_HUMAN_DATE );
	}
	return emptyString;
}

static QString getNameFromReportFile ( const QString& filename )
{
	const QString reportName ( fileOps::fileNameWithoutPath ( filename ) );
	if ( !reportName.isEmpty () )
	{
		return fileOps::filePathWithoutExtension ( reportName );
	}
	return reportName;
}

static QString getDateFromReportFile ( const QString& filename )
{
	const vmNumber date ( fileOps::modifiedDate ( filename ) );
	if ( !date.isNull () )
		return date.toString ();
	return emptyString;
}

static QString tryToGetClientAddress ( const QString& client_name )
{
	Client client;
	if ( client.readRecord ( FLD_CLIENT_NAME, client_name ) )
		return recStrValue ( &client, FLD_CLIENT_EMAIL );
	return emptyString;
}

estimateDlg::estimateDlg ( QWidget* parent )
	: QDialog ( parent ), m_npdlg ( nullptr )
{
	setWindowTitle ( TR_FUNC ( "Projects, Estimates and Reports" ) );
	setWindowIcon ( ICON ( "report.png" ) );

	iconsType[FILETYPE_DOC] = new QIcon ( ICON ( "Microsoft+Office+Word+2007.xpm" ) );
	iconsType[FILETYPE_XLS] = new QIcon ( ICON ( "Microsoft+Office+Excel+2007.xpm" ) );
	iconsType[FILETYPE_PDF] = new QIcon ( ICON ( "application-pdf.png" ) );
	iconsType[FILETYPE_VMR] = new QIcon ( ICON ( "report.png" ) );
	iconsType[FILETYPE_UNKNOWN] = new QIcon ( ICON ( "unknown.png" ) );

	btnDoc = new QToolButton;
	btnDoc->setFixedSize ( 30, 30 );
	btnDoc->setIcon ( *iconsType[FILETYPE_DOC] );
	connect ( btnDoc, &QToolButton::pressed, btnDoc, [&] () { return btnDoc->showMenu (); } );
	btnVmr = new QToolButton;
	btnVmr->setFixedSize ( 30, 30 );
	btnVmr->setIcon ( *iconsType[FILETYPE_VMR] );
	connect ( btnVmr, &QToolButton::pressed, btnVmr, [&] () { return btnVmr->showMenu (); } );
	btnXls = new QToolButton;
	btnXls->setFixedSize ( 30, 30 );
	btnXls->setIcon ( *iconsType[FILETYPE_XLS] );
	connect ( btnXls, &QToolButton::pressed, btnXls, [&] () { return btnXls->showMenu (); } );
	btnPdf = new QToolButton;
	btnPdf->setFixedSize ( 30, 30 );
	btnPdf->setIcon ( *iconsType[FILETYPE_PDF] );
	connect ( btnPdf, &QToolButton::pressed, btnPdf, [&] () { return btnPdf->showMenu (); } );
	btnFileManager = new QToolButton;
	btnFileManager->setFixedSize ( 30, 30 );
	btnFileManager->setIcon ( ICON ( "system-file-manager.png" ) );
	connect ( btnFileManager, &QToolButton::pressed, btnFileManager, [&] () { return btnFileManager->showMenu (); } );
	btnReload = new QToolButton;
	btnReload->setFixedSize ( 30, 30 );
	btnReload->setIcon ( ICON ( "reload.png" ) );
	connect ( btnReload, &QToolButton::pressed, this, [&] () { return scanDir (); } );

	QVBoxLayout* vLayout1 ( new QVBoxLayout );
	vLayout1->setMargin ( 1 );
	vLayout1->setSpacing ( 1 );
	vLayout1->addWidget ( btnDoc );
	vLayout1->addWidget ( btnVmr );
	vLayout1->addWidget ( btnXls );
	vLayout1->addWidget ( btnPdf );
	vLayout1->addWidget ( btnFileManager );
	vLayout1->addWidget ( btnReload );

	treeView = new QTreeWidget;
	treeView->setAlternatingRowColors ( true );
	treeView->setMinimumSize ( 500, 300 );
	treeView->setSortingEnabled ( true );
	treeView->sortByColumn ( 0, Qt::AscendingOrder );
	treeView->setColumnCount ( 3 );
	treeView->setHeaderLabels ( QStringList () << TR_FUNC ( "File name" ) << TR_FUNC ( "Document type" ) << TR_FUNC ( "Date" ) );
	treeView->setColumnWidth ( 0, 350 );
	treeView->setColumnWidth ( 1, 150 );
	treeView->setSelectionMode ( QAbstractItemView::SingleSelection );
	connect ( treeView, &QTreeWidget::itemSelectionChanged, this, [&] () { updateButtons (); } );
	connect ( treeView, &QTreeWidget::itemDoubleClicked, this, [&] ( QTreeWidgetItem* item, int ) { return openWithDoubleClick ( item ); } );

	QHBoxLayout* hLayout2 ( new QHBoxLayout );
	hLayout2->setMargin ( 2 );
	hLayout2->setSpacing ( 2 );
	hLayout2->addWidget ( treeView, 2 );
	hLayout2->addLayout ( vLayout1, 0 );

	btnEstimate = new QPushButton ( TR_FUNC ( "Estimate" ) );
	connect ( btnEstimate, &QPushButton::clicked, btnEstimate, [&] () { return btnEstimate->showMenu (); } );

	btnProject = new QPushButton ( TR_FUNC ( "Project" ) );
	connect ( btnProject, &QPushButton::clicked, btnProject, [&] () { return btnProject->showMenu (); } );

	btnReport = new QPushButton ( TR_FUNC ( "Report" ) );
	connect ( btnReport, &QPushButton::clicked, btnReport, [&] () { return btnReport->showMenu (); } );

	btnClose = new QPushButton ( TR_FUNC ( "Close" ) );
	connect ( btnClose, &QPushButton::clicked, this, [&] () { close (); } );

	QHBoxLayout* hLayout3 ( new QHBoxLayout );
	hLayout3->setMargin ( 2 );
	hLayout3->setSpacing ( 2 );
	hLayout3->addWidget ( btnProject, 1, Qt::AlignCenter );
	hLayout3->addWidget ( btnEstimate, 1, Qt::AlignCenter );
	hLayout3->addWidget ( btnReport, 1, Qt::AlignCenter );
	hLayout3->addWidget ( btnClose, 1, Qt::AlignCenter );

	QVBoxLayout* mainLayout ( new QVBoxLayout );
	mainLayout->setMargin ( 2 );
	mainLayout->setSpacing ( 0 );
	mainLayout->addLayout ( hLayout2, 2 );
	mainLayout->addLayout ( hLayout3, 0 );
	setLayout ( mainLayout );
	resize ( 650, 450 );

	setupActions ();
	scanDir ();
	updateButtons ();
	addPostRoutine ( deleteEstimateInstance );
}

estimateDlg::~estimateDlg () 
{
	/* All the list items created on heap initiated by newProjectDialog will be deleted once m_npdlg is deleted.
	 * The parent list will not delete the items, the heap destruction will. So we defer its destruction when we
	 * are destructed */
	heap_del ( m_npdlg );
}

void estimateDlg::showWindow ( const QString& client_name )
{
	QTreeWidgetItem* item ( nullptr );
	for ( int i ( 0 ); i < treeView->topLevelItemCount (); ++i )
	{
		if ( treeView->topLevelItem ( i )->text ( 0 ) == client_name )
		{
			item = treeView->topLevelItem ( i );
			treeView->setCurrentItem ( item, 0 );
			treeView->expandItem ( item );
			break;
		}
	}
	show ();
}

void estimateDlg::setupActions ()
{
	menuDoc = new QMenu;
	menuDoc->addAction ( new vmAction ( SFO_OPEN_FILE, TR_FUNC ( "Open document for editing" ), this ) );
	menuDoc->addAction ( new vmAction ( SFO_SEND_EMAIL, TR_FUNC ( "Send document via email" ), this ) );
	menuDoc->addSeparator ();
	menuDoc->addAction ( new vmAction ( SFO_REMOVE_FILE, TR_FUNC ( "Remove document file" ), this ) );
	connect ( menuDoc, &QMenu::triggered, this, [&] ( QAction* action ) { return execMenuAction ( action ); } );
	btnDoc->setMenu ( menuDoc );

	menuVmr = new QMenu;
	menuVmr->addAction ( new vmAction ( SFO_OPEN_FILE, TR_FUNC ( "Open report for editing" ), this ) );
	menuVmr->addAction ( new vmAction ( SFO_SEND_EMAIL, TR_FUNC ( "Send report via email" ), this ) );
	menuVmr->addSeparator ();
	menuVmr->addAction ( new vmAction ( SFO_REMOVE_FILE, TR_FUNC ( "Remove document file" ), this ) );
	connect ( menuVmr, &QMenu::triggered, this, [&] ( QAction* action ) { return execMenuAction ( action ); } );
	btnVmr->setMenu ( menuVmr );

	menuXls = new QMenu;
	menuXls->addAction ( new vmAction ( SFO_OPEN_FILE, TR_FUNC ( "Open spreadsheet for editing" ), this ) );
	menuXls->addAction ( new vmAction ( SFO_SEND_EMAIL, TR_FUNC ( "Send spreadsheeet via email" ), this ) );
	menuXls->addSeparator ();
	menuXls->addAction ( new vmAction ( SFO_REMOVE_FILE, TR_FUNC ( "Remove spreadsheet file" ), this ) );
	connect ( menuXls, &QMenu::triggered, this, [&] ( QAction* action ) { return execMenuAction ( action ); } );
	btnXls->setMenu ( menuXls );

	menuPdf = new QMenu;
	menuPdf->addAction ( new vmAction ( SFO_OPEN_FILE, TR_FUNC ( "Open PDF for viewing" ), this ) );
	menuPdf->addAction ( new vmAction ( SFO_SEND_EMAIL, TR_FUNC ( "Send PDF via email" ), this ) );
	menuPdf->addSeparator ();
	menuPdf->addAction ( new vmAction ( SFO_REMOVE_FILE, TR_FUNC ( "Remove PDF file" ), this ) );
	connect ( menuPdf, &QMenu::triggered, this, [&] ( QAction* action ) { return execMenuAction ( action ); } );
	btnPdf->setMenu ( menuPdf );

	menuFileMngr = new QMenu;
	menuFileMngr->addAction ( new vmAction ( OPEN_SELECTED_DIR, TR_FUNC ( "Open selected directory" ), this ) );
	menuFileMngr->addSeparator ();
	menuFileMngr->addAction ( new vmAction ( OPEN_ESTIMATES_DIR, TR_FUNC ( "Open client's estimates dir" ), this ) );
	menuFileMngr->addAction ( new vmAction ( OPEN_REPORTS_DIR, TR_FUNC ( "Open client's reports dir" ), this ) );
	menuFileMngr->addAction ( new vmAction ( OPEN_CLIENT_DIR, TR_FUNC ( "Open client's projects dir" ), this ) );
	connect ( menuFileMngr, &QMenu::triggered, this, [&] ( QAction* action ) { return execMenuAction ( action ); } );
	btnFileManager->setMenu ( menuFileMngr );

	menuProject = new QMenu;
	menuProject->addAction ( new vmAction ( PA_NEW_FULL, TR_FUNC ( "New project folder" ), this ) );
	menuProject->addAction ( new vmAction ( PA_NEW_EMPTY, TR_FUNC ( "New empty project folder" ), this ) );
	menuProject->addAction ( new vmAction ( PA_RENAME, TR_FUNC ( "Rename project" ), this ) );
	menuProject->addAction ( new vmAction ( PA_REMOVE, TR_FUNC ( "Delete project folder" ), this ) );
	menuProject->addAction ( new vmAction ( PA_ADD_DOC, TR_FUNC ( "Add new DOC" ), this ) );
	menuProject->addAction ( new vmAction ( PA_ADD_XLS, TR_FUNC ( "Add new XLS" ), this ) );
	connect ( menuProject, &QMenu::triggered, this, [&] ( QAction* action ) { return projectActions ( action ); } );
	btnProject->setMenu ( menuProject );

	menuEstimate = new QMenu;
	menuEstimate->addAction ( new vmAction ( EA_NEW_EXTERNAL, TR_FUNC ( "New estimage using external document editor" ), this ) );
	menuEstimate->addAction ( new vmAction ( EA_NEW_VMR, TR_FUNC ( "New estimate using internal document editor" ), this ) );
	menuEstimate->addAction ( new vmAction ( EA_CONVERT, TR_FUNC ( "Convert to project" ), this ) );
	menuEstimate->addAction ( new vmAction ( EA_REMOVE, TR_FUNC ( "Delete estimate" ), this ) );
	menuEstimate->addAction ( new vmAction ( EA_ADD_DOC, TR_FUNC ( "Add new DOC" ), this ) );
	menuEstimate->addAction ( new vmAction ( EA_ADD_XLS, TR_FUNC ( "Add new XLS" ), this ) );
	connect ( menuEstimate, &QMenu::triggered, this, [&] ( QAction* action ) { return estimateActions ( action ); } );
	btnEstimate->setMenu ( menuEstimate );

	menuReport = new QMenu;
	menuReport->addAction ( new vmAction ( RA_NEW_EXTERNAL, TR_FUNC ( "New report using external document editor" ), this ) );
	menuReport->addAction ( new vmAction ( RA_NEW_EXTERNAL_COMPLETE, TR_FUNC ( "New complete report, using external spreadsheet and document editor" ), this ) );
	menuReport->addAction ( new vmAction ( RA_NEW_VMR, TR_FUNC ( "New report using internal document editor" ), this ) );
	menuReport->addAction ( new vmAction ( RA_REMOVE, TR_FUNC ( "Delete this report" ), this ) );
	connect ( menuReport, &QMenu::triggered, this, [&] ( QAction* action ) { return reportActions ( action ); } );
	btnReport->setMenu ( menuReport );
}

void estimateDlg::scanDir ()
{
	PointersList<fileOps::st_fileInfo*> filesFound;

	const QStringList nameFilter ( QStringList () <<
								   configOps::projectDocumentFormerExtension () <<
								   configOps::projectSpreadSheetFormerExtension () <<
								   configOps::projectPDFExtension () <<
								   configOps::projectReportExtension () );

	treeView->clear ();
	QString clientName;
	QString dirName;
	for ( int c_id ( 0 ); c_id <= VDB ()->lastDBRecord ( TABLE_CLIENT_ORDER ); ++c_id )
	{
		clientName = Client::clientName ( c_id );
		if ( !clientName.isEmpty () )
		{
			dirName = CONFIG ()->getProjectBasePath ( clientName );
			fileOps::lsDir ( filesFound, dirName, nameFilter, fileOps::LS_ALL, 2 );
			addToTree ( filesFound, clientName );
		}
	}
	// Scan estimates done for non-clients
	clientName = nonClientStr;
	dirName = CONFIG ()->projectsBaseDir () + nonClientStr + CHR_F_SLASH;
	if ( fileOps::exists ( dirName ).isOn () )
	{
		fileOps::lsDir ( filesFound, dirName, nameFilter, fileOps::LS_ALL, 1 );
		addToTree ( filesFound, clientName, TYPE_ESTIMATE_ITEM );
	}
	if ( treeView->topLevelItemCount () > 0 )
		treeView->setCurrentItem ( treeView->topLevelItem ( 0 ) );
}

void estimateDlg::addToTree ( PointersList<fileOps::st_fileInfo*>& files, const QString& clientName, const int newItemType )
{
	QTreeWidgetItem* topLevelItem ( nullptr ), *child_dir ( nullptr ), *child_file ( nullptr );
	for ( int i ( 0 ); i < treeView->topLevelItemCount (); ++i )
	{
		if ( treeView->topLevelItem ( i )->text ( 0 ) == clientName )
		{
			topLevelItem = treeView->topLevelItem ( i );
			break;
		}
	}
	if ( topLevelItem == nullptr )
	{
		topLevelItem = new QTreeWidgetItem;
		topLevelItem->setText ( 0, clientName );
		topLevelItem->setBackground ( 0, QBrush ( Qt::darkCyan ) );
		topLevelItem->setForeground ( 0, QBrush ( Qt::white ) );
		topLevelItem->setData( 0, ROLE_ITEM_TYPE, TYPE_CLIENT_TOPLEVEL );
		treeView->addTopLevelItem ( topLevelItem );
	}

	if ( !files.isEmpty () )
	{
		uint i ( 0 ), itemType ( newItemType == -1 ? TYPE_REPORT_ITEM : newItemType );
		uint dirType ( 0 );
		QString filename;

		do
		{
			filename = files.at ( i )->filename;
			if ( files.at ( i )->is_dir )
			{
				if ( !files.at ( i )->fullpath.contains ( QStringLiteral ( "Pictures" ) ) )
				{
					child_dir = new QTreeWidgetItem ( topLevelItem );
					child_dir->setText ( 0, filename );
					if ( filename == CONFIG ()->estimatesDirSuffix () )
						dirType = TYPE_ESTIMATE_DIR;
					else if ( filename == CONFIG ()->reportsDirSuffix () )
						dirType = TYPE_REPORT_DIR;
					else
					{
						dirType = TYPE_PROJECT_DIR;
						child_dir->setText ( 2, getDateFromProjectDir ( filename ) );
					}
					if ( newItemType == -1 )
						itemType = dirType - 3;
					child_dir->setText ( 1, itemTypeStr[dirType] );
					child_dir->setData ( 0, ROLE_ITEM_TYPE, dirType );
					child_dir->setData ( 0, ROLE_ITEM_FILENAME, files.at ( i )->fullpath );
					child_dir->setData ( 0, ROLE_ITEM_CLIENTNAME, clientName != nonClientStr ? clientName : emptyString );
				}
			}
			else
			{
				if ( child_dir == nullptr ) // not used in the first pass, only when updating
				{
					QString child_dir_name;
					if ( itemType == TYPE_PROJECT_ITEM )
						child_dir_name = fileOps::nthDirFromPath ( filename );
					if ( itemType == TYPE_ESTIMATE_ITEM )
						child_dir_name = CONFIG ()->estimatesDirSuffix ();
					else // itemType == TYPE_REPORT_ITEM
						child_dir_name = CONFIG ()->reportsDirSuffix ();
					
					for ( int i ( 0 ); i < topLevelItem->childCount (); ++i )
					{
						if ( topLevelItem->child ( i )->text ( 0 ) == child_dir_name )
						{
							child_dir = topLevelItem->child ( i );
							break;
						}
					}
					if ( child_dir == nullptr )
						continue;
				}
				child_file = new QTreeWidgetItem ( child_dir );
				switch ( itemType )
				{
					case TYPE_PROJECT_ITEM:
						child_file->setText ( 0, getNameFromProjectFile ( filename ) + QLatin1String ( " (" ) + fileOps::fileExtension ( filename ) + CHR_R_PARENTHESIS );
						child_file->setText ( 2, getDateFromProjectFile ( filename ) );
					break;
					case TYPE_ESTIMATE_ITEM:
						child_file->setText ( 0, getNameFromEstimateFile ( filename ) + QLatin1String ( " (" ) + fileOps::fileExtension ( filename ) + CHR_R_PARENTHESIS );
						child_file->setText ( 2, getDateFromEstimateFile ( filename ) );
					break;
					case TYPE_REPORT_ITEM:
						child_file->setText ( 0, getNameFromReportFile ( filename ) + QLatin1String ( " (" ) + fileOps::fileExtension ( filename ) + CHR_R_PARENTHESIS );
						child_file->setText ( 2, getDateFromReportFile ( CONFIG ()->reportsDir ( clientName ) + filename ) );
					break;
				}
				child_file->setText ( 1, itemTypeStr[itemType] );
				child_file->setIcon ( 0, *iconsType[actionIndex ( filename )] );
				child_file->setData ( 0, ROLE_ITEM_TYPE, itemType );
				child_file->setData ( 0, ROLE_ITEM_FILENAME, files.at ( i )->fullpath );
				child_file->setData ( 0, ROLE_ITEM_CLIENTNAME, clientName != nonClientStr ? clientName : emptyString );
			}
		} while ( ++i < unsigned ( files.count () ) );
		files.clearButKeepMemory ( true );
	}
}

uint estimateDlg::actionIndex ( const QString& filename ) const
{
	if ( filename.contains ( CONFIG ()->projectDocumentFormerExtension () ) )
		return FILETYPE_DOC;
	else if ( filename.contains ( CONFIG ()->projectSpreadSheetFormerExtension () ) )
		return FILETYPE_XLS;
	else if ( filename.contains ( CONFIG ()->projectPDFExtension () ) )
		return FILETYPE_PDF;
	else if ( filename.contains ( CONFIG ()->projectReportExtension () ) )
		return FILETYPE_VMR;
	else
		return FILETYPE_UNKNOWN;
}

void estimateDlg::updateButtons ()
{
	if ( !treeView->selectedItems ().isEmpty () )
	{
		const QTreeWidgetItem* sel_item ( treeView->selectedItems ().at ( 0 ) );
		const uint type ( sel_item->data ( 0, ROLE_ITEM_TYPE ).toUInt () );
		const uint fileTypeIdx ( actionIndex ( sel_item->data ( 0, ROLE_ITEM_FILENAME ).toString () ) );
		btnDoc->setEnabled ( fileTypeIdx == FILETYPE_DOC );
		btnXls->setEnabled ( fileTypeIdx == FILETYPE_XLS );
		btnPdf->setEnabled ( fileTypeIdx == FILETYPE_PDF );
		btnVmr->setEnabled ( fileTypeIdx == FILETYPE_VMR );
		btnProject->setEnabled ( type == TYPE_PROJECT_ITEM || type == TYPE_CLIENT_TOPLEVEL || type == TYPE_PROJECT_DIR );
		if ( btnProject->isEnabled () )
		{
			menuProject->actions ().at ( PA_NEW_FULL )->setEnabled ( type == TYPE_CLIENT_TOPLEVEL );
			menuProject->actions ().at ( PA_NEW_EMPTY )->setEnabled ( type == TYPE_CLIENT_TOPLEVEL );
			menuProject->actions ().at ( PA_RENAME )->setEnabled ( type == TYPE_PROJECT_DIR );
			menuProject->actions ().at ( PA_REMOVE )->setEnabled ( type == TYPE_PROJECT_DIR );
			menuProject->actions ().at ( PA_ADD_DOC )->setEnabled ( type == TYPE_PROJECT_DIR );
			menuProject->actions ().at ( PA_ADD_XLS )->setEnabled ( type == TYPE_PROJECT_DIR );
		}
		btnReport->setEnabled ( type == TYPE_REPORT_ITEM || type == TYPE_CLIENT_TOPLEVEL || type == TYPE_REPORT_DIR );
		if ( btnReport->isEnabled () )
		{
			menuReport->actions ().at ( RA_NEW_EXTERNAL )->setEnabled ( type != TYPE_REPORT_ITEM );
			menuReport->actions ().at ( RA_NEW_EXTERNAL_COMPLETE )->setEnabled ( type != TYPE_REPORT_ITEM );
			menuReport->actions ().at ( RA_NEW_VMR )->setEnabled ( type != TYPE_REPORT_ITEM );
			menuReport->actions ().at ( RA_REMOVE )->setEnabled ( type == TYPE_REPORT_ITEM );
		}
		btnEstimate->setEnabled ( type == TYPE_ESTIMATE_ITEM || type == TYPE_CLIENT_TOPLEVEL || type == TYPE_ESTIMATE_DIR );
		if ( btnEstimate->isEnabled () )
		{
			menuEstimate->actions ().at ( EA_NEW_EXTERNAL )->setEnabled ( type != TYPE_ESTIMATE_ITEM );
			menuEstimate->actions ().at ( EA_NEW_VMR )->setEnabled ( type != TYPE_ESTIMATE_ITEM );
			menuEstimate->actions ().at ( EA_CONVERT )->setEnabled ( type == TYPE_ESTIMATE_ITEM );
			menuEstimate->actions ().at ( EA_REMOVE )->setEnabled ( type == TYPE_ESTIMATE_ITEM );
			menuEstimate->actions ().at ( EA_ADD_DOC )->setEnabled ( type == TYPE_ESTIMATE_DIR );
			menuEstimate->actions ().at ( EA_ADD_XLS )->setEnabled ( type == TYPE_ESTIMATE_DIR );
		}
		btnFileManager->menu ()->actions ().at ( 0 )->setEnabled ( true );
	}
}

void estimateDlg::openWithDoubleClick ( QTreeWidgetItem* item )
{
	const uint type ( item->data ( 0, ROLE_ITEM_TYPE ).toUInt () );
	if ( type <= TYPE_REPORT_ITEM )
		execAction ( item, SFO_OPEN_FILE );
}

void estimateDlg::convertToProject ( QTreeWidgetItem* item )
{
	QString srcPath, targetPath, project_name, nonClientName;
	QString clientName ( item->data ( 0, ROLE_ITEM_CLIENTNAME ).toString () );
	const bool non_client ( clientName.isEmpty () );

	if ( non_client )
	{
		if ( !vmNotify::inputBox ( clientName, this,  TR_FUNC ( "Convert estimate to which client project?" ),
							 TR_FUNC ( "Client name: " ), emptyString, emptyString, emptyString, vmCompleters::CLIENT_NAME ) )
			return;
		srcPath = CONFIG ()->projectsBaseDir () + configOps::estimatesDirSuffix () + CHR_F_SLASH;
	}
	else
		srcPath = fileOps::dirFromPath ( item->data ( 0, ROLE_ITEM_FILENAME ).toString () );

	targetPath = CONFIG ()->projectsBaseDir () + clientName + CHR_F_SLASH;

	const int btn (
		vmNotify::customBox ( TR_FUNC ( "New project" ), TR_FUNC ( "Create new project or choose existing directory?" ),
								  vmNotify::QUESTION, TR_FUNC ( "Choose existing " ), TR_FUNC ( "Create new" ), TR_FUNC ( "Cancel" ), this ) );
	switch ( btn )
	{
		case MESSAGE_BTN_OK: // Choose existing
			project_name = fileOps::getExistingDir ( targetPath );
			if ( project_name.isEmpty () )
				return;
			targetPath = project_name;
		break;
		case MESSAGE_BTN_CANCEL:
		break;
		default: // create new project
			vmNotify::inputBox ( project_name, this, TR_FUNC ( "Project's new name" ),
					 TR_FUNC ( "New name: " ), item->text ( 0 ) );
			if ( project_name.isEmpty () )
				return;
			targetPath += project_name + CHR_F_SLASH;
			if ( !fileOps::createDir ( targetPath ).isOn () ) //maybe first estimate for user, create dir
				return;
	}

	PointersList<fileOps::st_fileInfo*> estimateFiles;
	fileOps::st_fileInfo* f_info ( new fileOps::st_fileInfo );
	f_info->filename = project_name;
	f_info->fullpath = targetPath;
	f_info->is_dir = true;
	f_info->is_file = false;
	estimateFiles.append ( f_info );
		
	const QString baseFilename ( fileOps::fileNameWithoutPath ( fileOps::filePathWithoutExtension ( item->data ( 0, ROLE_ITEM_FILENAME ).toString () ) ) );
	fileOps::lsDir ( estimateFiles, srcPath, QStringList () << baseFilename );
	for ( uint i ( 1 ); i < estimateFiles.count (); ++i )
	{
		fileOps::copyFile ( targetPath + estimateFiles.at ( i )->filename, estimateFiles.at ( i )->fullpath );
	}
	
	addToTree ( estimateFiles, clientName );
	fileOps::createDir ( targetPath + QLatin1String ( "/Pictures/" ) );
	if ( clientName == DATA ()->currentClientName () )
		globalMainWindow->setUpJobButtons ( targetPath );
	VM_NOTIFY ()->messageBox ( TR_FUNC ( "Success!" ), project_name + TR_FUNC ( " was converted!" ) );
}

void estimateDlg::projectActions ( QAction *action )
{
	QTreeWidgetItem* item ( treeView->selectedItems ().at ( 0 ) );
	bool bAddDoc ( false ), bAddXls ( false ), bUseDialog ( false ), bProceed ( false );
	QString strProjectPath, strProjectID, msgTitle, msgBody[2];
	switch ( static_cast<vmAction*> ( action )->id () )
	{
		case PA_NEW_FULL:
			bAddDoc = bAddXls = true;
			bUseDialog = true;
			msgTitle = TR_FUNC ( "New Full Project Creation - " );
			msgBody[0] = TR_FUNC ( "Project %1 could not be created! Check the the error logs." );
			msgBody[1] = TR_FUNC ( "Project %1 was created!" );
		break;
		case PA_NEW_EMPTY:
			bUseDialog = true;
			msgTitle = TR_FUNC ( "New Empty Project Creation - " );
			msgBody[0] = TR_FUNC ( "Project %1 could not be created! Check the the error logs." );
			msgBody[1] = TR_FUNC ( "Project %1 was created!" );
		break;
		case PA_RENAME:
			bProceed = renameDir ( item, strProjectPath );
			msgTitle = TR_FUNC ( "Project Renaming - ");
			msgBody[0] = tr ( "Project " ) + item->text ( 0 ) + tr ( " could not be renamed! Check the the error logs." );
			msgBody[1] = tr ( "Project " ) + item->text ( 0 ) + tr ( " was renamed to " ) + strProjectPath;
		break;
		case PA_REMOVE:
			bProceed = removeDir ( item, true );
			msgTitle = TR_FUNC ( "Project Removal - ");
			msgBody[0] = tr ( "Project " ) + item->text ( 0 ) + tr ( " could not be removed! Check the the error logs." );
			msgBody[1] = tr ( "Project " ) + item->text ( 0 ) + tr ( " was removed." );
		break;
		case PA_ADD_DOC:
			bAddDoc = true;
			msgTitle = TR_FUNC ( "Adding document file - ");
			msgBody[0] = tr ( "Could not add document file to project " ) + item->text( 0 ) + tr ( " Check the the error logs." );
			msgBody[1] = tr ( "A new document file was added to project " ) + item->text ( 0 );
		break;
		case PA_ADD_XLS:
			bAddXls = true;
			msgTitle = TR_FUNC ( "Adding spreadshett file - ");
			msgBody[0] = tr ( "Could not add spreadsheet file to project " ) + item->text( 0 ) + tr ( " Check the the error logs." );
			msgBody[1] = tr ( "A new spreadsheet file was added to project " ) + item->text ( 0 );
		break;
	}
	PointersList<fileOps::st_fileInfo*> files;
	files.setAutoDeleteItem ( true );
	jobListItem* jobItem ( nullptr );

	if ( bUseDialog )
	{
		if ( !m_npdlg )
			m_npdlg = new newProjectDialog ( this );
		m_npdlg->showDialog ( QString::number ( Client::clientID ( item->text ( 0 ) ) ) );
		bProceed = m_npdlg->resultAccepted ();
		if ( bProceed ) {
			fileOps::st_fileInfo* f_info ( nullptr );
			strProjectPath = m_npdlg->projectPath ();
			if ( ( bProceed = !fileOps::exists ( strProjectPath ).isUndefined () ) )
			{
				f_info = new fileOps::st_fileInfo;
				bProceed = fileOps::createDir ( strProjectPath ).isOn ();
				if ( bProceed )
				{
					f_info->filename = fileOps::nthDirFromPath ( strProjectPath );
					if ( f_info->filename.endsWith ( CHR_F_SLASH ) )
						f_info->filename.chop ( 1 );
					f_info->fullpath = strProjectPath;
					f_info->is_dir = true;
					f_info->is_file = false;
					files.append ( f_info );
					strProjectID = m_npdlg->projectID ();
					(void) fileOps::createDir ( strProjectPath + QLatin1String ( "Pictures" ) );
					addFilesToDir ( bAddDoc, bAddXls, strProjectPath, strProjectID, files );
					bAddDoc = bAddXls = false;
					jobItem = m_npdlg->jobItem ();
				}
			}
		}
	}
	if ( bProceed )
	{
		if ( jobItem == nullptr ) // Only when renaming, removing, add_doc or add_xls to existing directory
			jobItem = findJobByPath ( item );
		if ( jobItem != nullptr )
		{
			if ( bAddDoc || bAddXls )
			{ // Last case to handle. There is no change to the database
				strProjectPath = recStrValue ( jobItem->jobRecord (), FLD_JOB_PROJECT_PATH );
				strProjectID = recStrValue ( jobItem->jobRecord (), FLD_JOB_PROJECT_ID );
				addFilesToDir ( bAddDoc, bAddXls, strProjectPath, strProjectID, files );
			}
			else
				changeJobData ( jobItem, strProjectPath, strProjectID );
			if ( DATA ()->currentJob () == jobItem->jobRecord () ) //update view on main window if necessary
				globalMainWindow->displayJob ( jobItem );
			addToTree ( files, item->text ( 0 ) );
		}
	}
	msgTitle += bProceed ? TR_FUNC ( "Succeeded!" ) : TR_FUNC ( "Failed!" );
	if ( msgBody[0].contains ( CHR_PERCENT ) )
	{
		if ( bProceed )
			msgBody[1] = msgBody[1].arg ( strProjectPath );
		else
			msgBody[0] = msgBody[0].arg ( strProjectPath );
	}
	VM_NOTIFY ()->notifyMessage ( msgTitle, msgBody[static_cast<uint>( bProceed )] );
}

jobListItem* estimateDlg::findJobByPath ( QTreeWidgetItem* const item )
{
	jobListItem* jobItem ( nullptr );
	const QString clientID ( Client::clientID ( item->text ( 0 ) ) );
	clientListItem* clientItem ( globalMainWindow->getClientItem ( clientID.toInt () ) );
	if ( clientItem )
	{
		const QString strPath ( item->data ( 0, ROLE_ITEM_FILENAME ).toString () );
		const QString strQuery ( QStringLiteral ( "SELECT ID FROM JOBS WHERE CLIENTID=%1 AND PROJECTPATH=%2" ) );
		QSqlQuery queryRes;
		if ( VDB ()->runQuery ( strQuery.arg ( clientID, strPath ), queryRes ) )
			jobItem = globalMainWindow->getJobItem ( clientItem, queryRes.value ( 0 ).toInt () );
	}
	return jobItem;
}

void estimateDlg::changeJobData ( jobListItem* const jobItem, const QString& strProjectPath, const QString& strProjectID )
{
	if ( jobItem != nullptr ) 
	{
		jobItem->setAction ( ACTION_EDIT, false );
		Job* job ( jobItem->jobRecord () );
		setRecValue ( job, FLD_JOB_PROJECT_PATH, strProjectPath );
		setRecValue ( job, FLD_JOB_PICTURE_PATH, strProjectPath + QLatin1String ( "Pictures/" ) );
		setRecValue ( job, FLD_JOB_PROJECT_ID, strProjectID );
		job->saveRecord ();
		jobItem->setAction ( job->action () );
	}
}

void estimateDlg::estimateActions ( QAction* action )
{
	QTreeWidgetItem* item ( treeView->selectedItems ().at ( 0 ) );
	const QString clientName ( item->parent ()->text ( 0 ) );
	const QString basePath ( clientName != nonClientStr ?
							 CONFIG ()->estimatesDir ( clientName ) :
							 CONFIG ()->projectsBaseDir () + configOps::estimatesDirSuffix () );

	switch ( static_cast<vmAction*> ( action )->id () ) {
		case EA_CONVERT:
			convertToProject ( item );
		break;
		case EA_REMOVE:
			removeFiles ( item, false, true );
		break;
		default:
		{
			QString estimateName;
			vmNotify::inputBox ( estimateName, this, TR_FUNC ( "Please write the estimate's name" ),
								 TR_FUNC ( "Name: " ) );
			if ( estimateName.isEmpty () )
				return;

			PointersList<fileOps::st_fileInfo*> files;
			fileOps::st_fileInfo* f_info ( nullptr );
			
			if ( fileOps::exists ( basePath ).isOff () )
			{
				if ( !fileOps::createDir ( basePath ).isOn () ) //maybe first estimate for user, create dir
					return;
				f_info = new fileOps::st_fileInfo;
				f_info->filename = CONFIG ()->estimatesDirSuffix ();
				f_info->fullpath = basePath;
				f_info->is_dir = true;
				f_info->is_file = false;
				files.append ( f_info );
			}
			
			estimateName.prepend ( vmNumber::currentDate.toDate ( vmNumber::VDF_FILE_DATE ) + QLatin1String ( " - " ) );
			if ( clientName == nonClientStr )
				estimateName.prepend ( nonClientEstimatesPrefix );

			f_info = new fileOps::st_fileInfo;
			if ( static_cast<vmAction*> ( action )->id () == EA_NEW_VMR )
			{
				f_info->filename = estimateName + CONFIG ()->projectReportExtension ();
				f_info->fullpath = basePath + f_info->filename;

				EDITOR ()->startNewReport ( true )->createJobReport ( Client::clientID ( clientName ),
					false, f_info->fullpath, false );
				files.append ( f_info );
			}
			else
			{
				f_info->filename = estimateName + CONFIG ()->projectDocumentExtension ();
				f_info->fullpath = basePath + f_info->filename;

				if ( fileOps::copyFile ( f_info->fullpath, CONFIG ()->projectDocumentFile () ) )
					files.append ( f_info );

				f_info = new fileOps::st_fileInfo;
				f_info->filename = estimateName + CONFIG ()->projectSpreadSheetExtension ();
				f_info->fullpath = basePath + f_info->filename;
				if ( fileOps::copyFile ( f_info->fullpath, CONFIG ()->projectSpreadSheetFile () ) )
					files.append ( f_info );
			}
			addToTree ( files, clientName, TYPE_ESTIMATE_ITEM );
		}
		break;
	}
}

void estimateDlg::reportActions ( QAction* action )
{
	QTreeWidgetItem* item ( treeView->selectedItems ().at ( 0 ) );
	const QString clientName ( item->parent () ? item->parent ()->text ( 0 ) : item->text ( 0 ) );

	const QString basePath ( clientName != nonClientStr ?
							 CONFIG ()->reportsDir ( clientName ) :
							 CONFIG ()->projectsBaseDir () + configOps::reportsDirSuffix () + CHR_F_SLASH
						   );
	if ( static_cast<vmAction*> ( action )->id () == RA_REMOVE )
			removeFiles ( item, true, true );
	else
	{
		QString reportName;
		vmNotify::inputBox ( reportName, this, TR_FUNC ( "Please write the report's name" ),
						 TR_FUNC ( "Name: " ) );
		if ( reportName.isEmpty () )
			return;

		PointersList<fileOps::st_fileInfo*> files;
		fileOps::st_fileInfo* f_info ( nullptr );
		
		if ( fileOps::exists ( basePath ).isOff () )
		{
			if ( !fileOps::createDir ( basePath ).isOn () ) //maybe first report for user, create dir
				return;
			f_info = new fileOps::st_fileInfo;
			f_info->filename = CONFIG ()->reportsDirSuffix ();
			f_info->fullpath = basePath;
			f_info->is_dir = true;
			f_info->is_file = false;
			files.append ( f_info );
		}

		reportName.prepend ( vmNumber::currentDate.toDate ( vmNumber::VDF_FILE_DATE ) + QLatin1String ( " - " ) );
		if ( clientName == nonClientStr )
			reportName.prepend ( nonClientReportsPrefix );

		f_info = new fileOps::st_fileInfo;
		if ( static_cast<vmAction*> ( action )->id () == RA_NEW_VMR )
		{
			f_info->filename = reportName + CONFIG ()->projectReportExtension ();
			f_info->fullpath = basePath + f_info->filename;
			EDITOR ()->startNewReport ( true )->createJobReport (
				Client::clientID ( clientName ), false , f_info->fullpath, false );
			files.append ( f_info );
		}
		else
		{
			f_info->filename = reportName + CONFIG ()->projectDocumentExtension ();
			f_info->fullpath = basePath + f_info->filename;
			if ( fileOps::copyFile ( f_info->fullpath, CONFIG ()->projectDocumentFile () ) )
				files.append ( f_info );

			if ( static_cast<vmAction*> ( action )->id () == RA_NEW_EXTERNAL_COMPLETE )
			{
				f_info = new fileOps::st_fileInfo;
				f_info->filename = reportName + CONFIG ()->projectSpreadSheetExtension ();
				f_info->fullpath = basePath + f_info->filename;
				if ( !fileOps::copyFile ( f_info->fullpath, CONFIG ()->projectSpreadSheetFile () ) )
					files.append ( f_info );
			}
		}
		addToTree ( files, clientName, TYPE_REPORT_ITEM );
	}
}

void estimateDlg::removeFiles ( QTreeWidgetItem* item, const bool bSingleFile, const bool bAsk )
{
	if ( bAsk && !VM_NOTIFY ()->questionBox ( TR_FUNC ( "Atention!" ), TR_FUNC ( "Are you sure you want to remove %1?" ).arg ( item->text ( 0 ) ) ) )
		return;

	const QString fileName ( item->data ( 0, ROLE_ITEM_FILENAME ).toString () );
	if ( bSingleFile )
	{
		if ( fileOps::removeFile ( fileName ).isOn () )
		{
			if ( item->parent () )
				item->parent ()->removeChild ( item );
			delete item;
		}
	}
	else
	{
		const QString baseFileName ( fileName.left ( fileName.lastIndexOf ( CHR_DOT ) + 1 ) );
		QTreeWidgetItem* parentItem ( item->parent () );
		QTreeWidgetItem* child ( nullptr );
		uint i ( 0 );
		if ( parentItem->childCount () > 0 )
		{
			do
			{
				child = parentItem->child ( i );
				if ( child )
				{
					if ( child->text ( 0 ).startsWith ( baseFileName, Qt::CaseInsensitive ) )
					{
						if ( fileOps::removeFile ( child->text ( 0 ) ).isOn () )
						{
							parentItem->removeChild ( child );
							delete child;
						}
					}
					++i;
				}
			} while ( child != nullptr );
		}
	}
}

bool estimateDlg::removeDir ( QTreeWidgetItem *item, const bool bAsk )
{
	if ( bAsk ) {
		if ( VM_NOTIFY ()->questionBox ( TR_FUNC ( "Question" ), TR_FUNC ( "Are you sure you want to delete project " )
					+ item->text ( 0 ) + TR_FUNC ( " and all its contents?" ) ) )
		{

			if ( fileOps::rmDir ( item->data ( 0, ROLE_ITEM_FILENAME ).toString (), QStringList (), fileOps::LS_ALL, -1 ) )
			{
				VM_NOTIFY ()->notifyMessage ( TR_FUNC ( "Success" ), TR_FUNC ( "Project " ) + item->text ( 0 ) + tr (" is now removed" ) );
				item->parent ()->removeChild ( item );
				delete item;
				return true;
			}
		}
	}
	return false;
}

bool estimateDlg::renameDir ( QTreeWidgetItem *item, QString &strNewName )
{
	if ( strNewName.isEmpty () )
	{
		if ( !vmNotify::inputBox ( strNewName, this, TR_FUNC ( "Rename Project" ), TR_FUNC ( "Enter new name for the project" ) ) )
			return false;
	}
	QString strNewFilePath ( item->data ( 0, ROLE_ITEM_FILENAME ).toString () );
	if ( fileOps::rename ( item->data ( 0, ROLE_ITEM_FILENAME ).toString (),
						   strNewFilePath.replace ( item->text ( 0 ), strNewName ) ).isOn () )
	{
		item->setText ( 0, strNewName );
		item->setData ( 0, ROLE_ITEM_FILENAME, strNewFilePath );
		return true;
	}
	return false;
}

void estimateDlg::addFilesToDir ( const bool bAddDoc, const bool bAddXls, const QString& projectpath, QString& projectid,
								  PointersList<fileOps::st_fileInfo*>& files )
{
	QString fileName, fileNameComplete;
	QString sub_version ( QLatin1Char ( 'A' ) );
	uint i ( 1 );
	bool b_ok ( false );
	if ( bAddDoc )
	{
		fileName = QLatin1String ( "Projeto-" ) + projectid + QLatin1String ( "-%1" ) + CONFIG ()->projectDocumentExtension ();
		do
		{
			fileNameComplete = projectpath + fileName.arg ( sub_version );
			if ( fileOps::exists ( fileNameComplete ).isOn () )
				sub_version = QChar ( 'A' + i++ );
			else
				break;
		} while ( true );
		if ( fileOps::copyFile ( fileNameComplete, CONFIG ()->projectDocumentFile () ) )
		{
			fileOps::st_fileInfo* f_info ( new fileOps::st_fileInfo );
			f_info->filename = fileName.arg ( sub_version );
			f_info->fullpath = fileNameComplete;
			files.append ( f_info );
			b_ok = true;
		}
	}
	if ( bAddXls )
	{
		fileName = QLatin1String ( "Planilhas-" ) + projectid + QLatin1String ( "-%1") + CONFIG ()->projectSpreadSheetExtension ();
		do
		{
			fileNameComplete = projectpath + fileName.arg ( sub_version );
			if ( fileOps::exists ( fileNameComplete ).isOn () )
				sub_version = QChar ( 'A' + i++ );
			else
				break;
		} while ( true );
		if ( fileOps::copyFile ( fileNameComplete, CONFIG ()->projectSpreadSheetFile () ) )
		{
			fileOps::st_fileInfo* f_info ( new fileOps::st_fileInfo );
			f_info->filename = fileName.arg ( sub_version );
			f_info->fullpath = fileNameComplete;
			files.append ( f_info );
			b_ok = true;
		}
	}
	if ( b_ok )
		projectid += CHR_HYPHEN + sub_version;
}

void estimateDlg::execAction ( const QTreeWidgetItem* item, const int action_id )
{
	const QString filename ( item->data ( 0, ROLE_ITEM_FILENAME ).toString () );
	const uint fileTypeIdx ( actionIndex ( filename ) );
	const QString clientName ( item->data ( 0, ROLE_ITEM_CLIENTNAME ).toString () );
	QString program;

	switch ( action_id )
	{
		case SFO_OPEN_FILE:
		{
			switch ( fileTypeIdx )
			{
				case FILETYPE_DOC:		program = CONFIG ()->docEditor (); break;
				case FILETYPE_XLS:		program = CONFIG ()->xlsEditor (); break;
				case FILETYPE_PDF:
				case FILETYPE_UNKNOWN:
										program = CONFIG ()->universalViewer (); break;
				case FILETYPE_VMR:
					EDITOR ()->startNewReport ()->load ( filename, true );
					EDITOR ()->show ();
				default:
					return;
			}
			fileOps::execute ( filename, program );
		}
		break;
		case SFO_SEND_EMAIL:
			EMAIL_CONFIG ()->sendEMail ( tryToGetClientAddress ( clientName ), item->text ( 1 ) );
		break;
		case SFO_REMOVE_FILE:
			removeFiles ( const_cast<QTreeWidgetItem*>( item ), true );
		break;
		default:
			program = CONFIG ()->fileManager ();
			switch ( action_id )
			{
				case OPEN_SELECTED_DIR:
					fileOps::execute ( filename, program ); break;
				case OPEN_ESTIMATES_DIR:
					fileOps::execute ( CONFIG ()->estimatesDir ( clientName ), program ); break;
				case OPEN_REPORTS_DIR:
					fileOps::execute ( CONFIG ()->reportsDir ( clientName ), program ); break;
				case OPEN_CLIENT_DIR:
					fileOps::execute ( CONFIG ()->getProjectBasePath ( clientName ), program ); break;
			}
		break;
	}
}
