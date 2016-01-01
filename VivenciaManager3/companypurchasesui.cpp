#include "companypurchasesui.h"
#include "global.h"
#include "ui_companypurchasesui.h"
#include "inventory.h"
#include "companypurchases.h"
#include "vmwidgets.h"
#include "vivenciadb.h"
#include "completers.h"
#include "vmnumberformats.h"
#include "suppliersdlg.h"
#include "inventoryui.h"
#include "vivenciadb.h"
#include "vmnotify.h"
#include "listitems.h"
#include "searchui.h"
#include "cleanup.h"
#include "heapmanager.h"
#include "fast_library_functions.h"

#include <QTimer>
#include <QKeyEvent>

companyPurchasesUI* companyPurchasesUI::s_instance ( nullptr );

void deleteCompanyPurchasesUiInstance ()
{
    heap_del ( companyPurchasesUI::s_instance );
}

enum btnNames { BTN_FIRST = 0, BTN_PREV = 1, BTN_NEXT = 2, BTN_LAST = 3, BTN_ADD = 4, BTN_EDIT = 5, BTN_DEL = 6, BTN_SEARCH = 7 };
static bool mb_enable_status[8] = { false };

companyPurchasesUI::companyPurchasesUI ( QWidget* parent )
    : QDialog ( parent ), ui ( new Ui::companyPurchasesUI () ),
      cp_rec ( new companyPurchases ( true ) ), widgetList ( INVENTORY_FIELD_COUNT + 1 ),
      mFoundFields ( 5 )
{
    ui->setupUi ( this );
    setupUI ();
    const bool have_items ( VDB ()->lastDBRecord ( TABLE_CP_ORDER ) >= 0 );
    ui->btnCPEdit->setEnabled ( true );
    ui->btnCPRemove->setEnabled ( true );
    ui->btnCPNext->setEnabled ( false );
    ui->btnCPPrev->setEnabled ( have_items );
    ui->btnCPLast->setEnabled ( have_items );
    ui->txtCPSearch->setEditable ( have_items );
    ui->btnCPSearch->setEnabled ( have_items );

    cp_rec->readLastRecord ();
    fillForms ();

    addPostRoutine ( deleteCompanyPurchasesUiInstance );
}

companyPurchasesUI::~companyPurchasesUI ()
{
    heap_del ( cp_rec );
    heap_del ( ui );
}

void companyPurchasesUI::showSearchResult ( vmListItem* item, const bool bshow )
{
    if ( bshow ) {
        if ( cp_rec->readRecord ( item->dbRecID () ) ) {
            if ( !isVisible () )
                showNormal ();
            fillForms ();
        }
    }
    for ( uint i ( 0 ); i < COMPANY_PURCHASES_FIELD_COUNT; ++i ) {
		if ( item->searchFieldStatus ( i ) == SS_SEARCH_FOUND )
			widgetList.at ( i )->highlight ( bshow ? vmBlue : vmDefault_Color, SEARCH_UI ()->searchTerm () );
    }
}

void companyPurchasesUI::showSearchResult_internal ( const bool bshow )
{
    for ( uint i ( 0 ); i < mFoundFields.count (); ++i )
		widgetList.at ( i )->highlight ( bshow ? vmBlue : vmDefault_Color, SEARCH_UI ()->searchTerm () );
    if ( bshow )
        fillForms ();
}

void companyPurchasesUI::setTotalPriceAsItChanges ( const vmTableItem* const item )
{
    setRecValue ( cp_rec, FLD_CP_TOTAL_PRICE, item->text () );
}

void companyPurchasesUI::setPayValueAsItChanges ( const vmTableItem* const item )
{
    ui->txtCPPayValue->lineControl ()->setText ( item->text () );
    setRecValue ( cp_rec, FLD_CP_PAY_VALUE, item->text () );
}

void companyPurchasesUI::tableItemsCellChanged ( const vmTableItem* const item )
{
    if ( item ) {
        stringTable items ( recStrValue ( cp_rec, FLD_CP_ITEMS_REPORT ) );
        items.changeRecord ( item->row (), item->column (), item->text () );
        setRecValue ( cp_rec, FLD_CP_ITEMS_REPORT, items.toString () );
    }
}

void companyPurchasesUI::tablePaymentsCellChanged ( const vmTableItem* const item )
{
    if ( item ) {
        stringTable items ( recStrValue ( cp_rec, FLD_CP_PAY_REPORT ) );
        items.changeRecord ( item->row (), item->column (), item->text () );
        setRecValue ( cp_rec, FLD_CP_PAY_REPORT, items.toString () );
    }
}

void companyPurchasesUI::saveWidget ( vmWidget* widget, const uint id )
{
    widget->setID ( id );
    widgetList[id] = widget;
}

void companyPurchasesUI::setupUI ()
{
    ui->btnCPAdd->connect ( ui->btnCPAdd, static_cast<void (QPushButton::*)( const bool )> ( &QPushButton::clicked ),
    this, [&] ( const bool checked ) {
        return btnCPAdd_clicked ( checked );
    } );
    ui->btnCPEdit->connect ( ui->btnCPEdit, static_cast<void (QPushButton::*)( const bool )> ( &QPushButton::clicked ),
    this, [&] ( const bool checked ) {
        return btnCPEdit_clicked ( checked );
    } );
    ui->btnCPSearch->connect ( ui->btnCPSearch, static_cast<void (QPushButton::*)( const bool )> ( &QPushButton::clicked ),
    this, [&] ( const bool checked ) {
        return btnCPSearch_clicked ( checked );
    } );
	ui->btnCPShowSupplier->connect ( ui->btnCPShowSupplier, static_cast<void (QToolButton::*)( const bool )>( &QToolButton::clicked ),
    this, [&] ( const bool checked ) {
        return btnCPShowSupplier_clicked ( checked );
    } );
    ui->btnCPFirst->connect ( ui->btnCPFirst, &QPushButton::clicked, this, [&] () {
        return btnCPFirst_clicked ();
    } );
    ui->btnCPPrev->connect ( ui->btnCPPrev, &QPushButton::clicked, this, [&] () {
        return btnCPPrev_clicked ();
    } );
    ui->btnCPNext->connect ( ui->btnCPNext, &QPushButton::clicked, this, [&] () {
        return btnCPNext_clicked ();
    } );
    ui->btnCPLast->connect ( ui->btnCPLast, &QPushButton::clicked, this, [&] () {
        return btnCPLast_clicked ();
    } );
    ui->btnCPRemove->connect ( ui->btnCPRemove, &QPushButton::clicked, this, [&] () {
        return btnCPRemove_clicked ();
    } );
    ui->btnClose->connect ( ui->btnClose, &QPushButton::clicked, this, [&] () {
        return hide ();
    } );

    ( void ) vmTableWidget::createPurchasesTable ( ui->tableItems );
    saveWidget ( ui->tableItems, FLD_CP_ITEMS_REPORT );
    ui->tableItems->setKeepModificationRecords ( false );
    ui->tableItems->setParentLayout ( ui->layoutCPTable );
    ui->tableItems->setCallbackForMonitoredCellChanged ( [&] ( const vmTableItem* const item ) {
        return setTotalPriceAsItChanges ( item );
    } );
    ui->tableItems->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
        return relevantKeyPressed ( ke );
    } );
    ui->tableItems->setCallbackForCellChanged ( [&] ( const vmTableItem* const item ) {
        return tableItemsCellChanged ( item );
    } );

    ( void ) vmTableWidget::createPayHistoryTable ( ui->tablePayments );
    saveWidget ( ui->tablePayments, FLD_CP_PAY_REPORT );
    ui->tablePayments->setParentLayout ( ui->layoutCPTable );
    ui->tablePayments->setCallbackForMonitoredCellChanged ( [&] ( const vmTableItem* const item ) {
        return setPayValueAsItChanges ( item );
    } );
    ui->tablePayments->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
        return relevantKeyPressed ( ke );
    } );
    ui->tablePayments->setCallbackForCellChanged ( [&] ( const vmTableItem* const item ) {
        return tablePaymentsCellChanged ( item );
    } );

    APP_COMPLETERS ()->setCompleter ( ui->txtCPSupplier, vmCompleters::SUPPLIER );
    saveWidget ( ui->txtCPSupplier, FLD_CP_SUPPLIER );
    ui->txtCPSupplier->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
        return txtCP_textAltered ( sender );
    } );
    ui->txtCPSupplier->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
        return relevantKeyPressed ( ke );
    } );

    APP_COMPLETERS ()->setCompleter ( ui->txtCPDeliveryMethod, vmCompleters::DELIVERY_METHOD );
    saveWidget ( ui->txtCPDeliveryMethod, FLD_CP_DELIVERY_METHOD );
    ui->txtCPDeliveryMethod->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
        return txtCP_textAltered ( sender );
    } );
    ui->txtCPDeliveryMethod->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
        return relevantKeyPressed ( ke );
    } );

    saveWidget ( ui->txtCPNotes, FLD_CP_NOTES );
    ui->txtCPNotes->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
        return txtCP_textAltered ( sender );
    } );
    ui->txtCPNotes->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
        return relevantKeyPressed ( ke );
    } );

    saveWidget ( ui->dteCPDate, FLD_CP_DATE );
    ui->dteCPDate->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
        return dteCP_dateAltered ( sender );
    } );
    ui->dteCPDate->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
        return relevantKeyPressed ( ke );
    } );

    saveWidget ( ui->dteCPDeliveryDate, FLD_CP_DELIVERY_DATE );
    ui->dteCPDeliveryDate->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
        return dteCP_dateAltered ( sender );
    } );
    ui->dteCPDeliveryDate->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const ke, const vmWidget* const ) {
        return relevantKeyPressed ( ke );
    } );

    ui->txtCPSearch->setCallbackForContentsAltered ( [&] ( const vmWidget* const sender ) {
        txtCPSearch_textAltered ( sender->text () );
    } );
    ui->txtCPSearch->setCallbackForRelevantKeyPressed ( [&] ( const QKeyEvent* const, const vmWidget* const ) {
        return btnCPSearch_clicked ( true );
    } );
}

void companyPurchasesUI::fillForms ()
{
    ui->txtCPID->setText ( recStrValue ( cp_rec, FLD_CP_ID ) );
    ui->dteCPDate->setDate ( cp_rec->date ( FLD_CP_DATE ) );
    ui->dteCPDeliveryDate->setDate ( cp_rec->date ( FLD_CP_DELIVERY_DATE ) );
    ui->txtCPPayValue->lineControl ()->setText ( recStrValue ( cp_rec, FLD_CP_PAY_VALUE ) );
    ui->txtCPDeliveryMethod->setText ( recStrValue ( cp_rec, FLD_CP_DELIVERY_METHOD ) );
    ui->txtCPSupplier->setText ( recStrValue ( cp_rec, FLD_CP_SUPPLIER ) );
    ui->txtCPNotes->setText ( recStrValue ( cp_rec, FLD_CP_NOTES ) );

    ui->tableItems->loadFromStringTable ( recStrValue ( cp_rec, FLD_CP_ITEMS_REPORT ) );
    ui->tableItems->scrollToTop ();
    ui->tablePayments->loadFromStringTable ( recStrValue ( cp_rec, FLD_CP_PAY_REPORT ) );
    ui->tablePayments->scrollToTop ();
}

void companyPurchasesUI::clearForms ()
{
    ui->txtCPID->setText ( emptyString );
    ui->dteCPDate->setDate ( vmNumber::currentDate );
    ui->dteCPDeliveryDate->setDate ( vmNumber::currentDate );
    ui->txtCPPayValue->lineControl ()->setText ( emptyString );
    ui->txtCPDeliveryMethod->setText ( emptyString );
    ui->txtCPSupplier->setText ( emptyString );
    ui->txtCPNotes->setText ( emptyString );
    ui->tableItems->clear ();
    ui->tablePayments->clear ();
}

void companyPurchasesUI::controlForms ()
{
    const bool editable ( cp_rec->action () >= ACTION_ADD );
    ui->dteCPDate->setEditable ( editable );
    ui->dteCPDeliveryDate->setEditable ( editable );
    ui->txtCPPayValue->setEditable ( editable );
    ui->txtCPDeliveryMethod->setEditable ( editable );
    ui->txtCPSupplier->setEditable ( editable );
    ui->txtCPNotes->setEditable ( editable );
    ui->tableItems->setEditable ( editable );
    ui->tablePayments->setEditable ( editable );
    ui->btnCPRemove->setEnabled ( cp_rec->action () == ACTION_READ );

    if ( !editable ) {
        mb_enable_status[BTN_FIRST] = ui->btnCPFirst->isEnabled ();
        mb_enable_status[BTN_PREV] = ui->btnCPPrev->isEnabled ();
        mb_enable_status[BTN_NEXT] = ui->btnCPNext->isEnabled ();
        mb_enable_status[BTN_LAST] = ui->btnCPLast->isEnabled ();
        mb_enable_status[BTN_ADD] = ui->btnCPAdd->isEnabled ();
        mb_enable_status[BTN_EDIT] = ui->btnCPEdit->isEnabled ();
        mb_enable_status[BTN_DEL] = ui->btnCPRemove->isEnabled ();
        mb_enable_status[BTN_SEARCH] = ui->btnCPSearch->isEnabled ();
        ui->btnCPFirst->setEnabled ( false );
        ui->btnCPPrev->setEnabled ( false );
        ui->btnCPNext->setEnabled ( false );
        ui->btnCPLast->setEnabled ( false );
        ui->btnCPAdd->setEnabled ( cp_rec->action () == ACTION_ADD );
        ui->btnCPEdit->setEnabled ( cp_rec->action () == ACTION_EDIT );
        ui->btnCPSearch->setEnabled ( false );
        ui->txtCPSearch->setEnabled ( false );
    }
    else {
        ui->btnCPFirst->setEnabled ( mb_enable_status[BTN_FIRST] );
        ui->btnCPPrev->setEnabled ( mb_enable_status[BTN_PREV] );
        ui->btnCPNext->setEnabled ( mb_enable_status[BTN_NEXT] );
        ui->btnCPLast->setEnabled ( mb_enable_status[BTN_LAST] );
        ui->btnCPAdd->setEnabled ( mb_enable_status[BTN_EDIT] );
        ui->btnCPEdit->setEnabled ( mb_enable_status[BTN_DEL] );
        ui->btnCPSearch->setEnabled ( mb_enable_status[BTN_SEARCH] );
        ui->txtCPSearch->setEnabled ( mb_enable_status[BTN_SEARCH] );
    }
}

void companyPurchasesUI::saveInfo ()
{
    if ( cp_rec->isModified () ) {
        if ( cp_rec->saveRecord () )
            vmTableWidget::exchangePurchaseTablesInfo (
                        ui->tableItems, INVENTORY ()->table (), cp_rec, INVENTORY ()->inventory_rec );
    }
    controlForms ();
    ui->btnCPEdit->setText ( emptyString );
    ui->btnCPEdit->setIcon ( ICON ( "browse-controls/edit.png" ) );
    ui->btnCPAdd->setText ( emptyString );
    ui->btnCPAdd->setIcon ( ICON ( "browse-controls/add.png" ) );
}

void companyPurchasesUI::searchCancel ()
{
    for ( uint i ( 0 ); i < mFoundFields.count (); ++i )
		widgetList.at ( mFoundFields.at ( i ) )->highlight ( vmDefault_Color );
    mFoundFields.clearButKeepMemory ();
    mSearchTerm.clear ();
    mbSearchIsOn = false;
}

bool companyPurchasesUI::searchFirst ()
{
    if ( cp_rec->readFirstRecord ( -1, mSearchTerm ) ) {
        showSearchResult_internal ( false ); // unhighlight current widgets
        cp_rec->contains ( mSearchTerm, mFoundFields );
        return true;
    }
    return false;
}

bool companyPurchasesUI::searchPrev ()
{
    if ( cp_rec->readPrevRecord ( true ) ) {
        showSearchResult_internal ( false ); // unhighlight current widgets
        cp_rec->contains ( mSearchTerm, mFoundFields );
        return true;
    }
    return false;
}

bool companyPurchasesUI::searchNext ()
{
    if ( cp_rec->readNextRecord ( true ) ) {
        showSearchResult_internal ( false ); // unhighlight current widgets
        cp_rec->contains ( mSearchTerm, mFoundFields );
        return true;
    }
    return false;
}

bool companyPurchasesUI::searchLast ()
{
    if ( cp_rec->readLastRecord ( -1, mSearchTerm ) ) {
        showSearchResult_internal ( false ); // unhighlight current widgets
        cp_rec->contains ( mSearchTerm, mFoundFields );
        return true;
    }
    return false;
}

void companyPurchasesUI::btnCPFirst_clicked ()
{
    bool ok ( false );
    if ( mbSearchIsOn ) {
        if ( ( ok = searchFirst () ) )
            showSearchResult_internal ( true );
    }
    else {
        if ( ( ok = cp_rec->readFirstRecord () ) )
            fillForms ();
    }

    ui->btnCPFirst->setEnabled ( !ok );
    ui->btnCPPrev->setEnabled ( !ok );
    ui->btnCPNext->setEnabled ( ui->btnCPNext->isEnabled () || ok );
    ui->btnCPLast->setEnabled ( ui->btnCPLast->isEnabled () || ok );
}

void companyPurchasesUI::btnCPLast_clicked ()
{
    bool ok ( false );
    if ( mbSearchIsOn ) {
        if ( ( ok = searchLast () ) )
            showSearchResult_internal ( true );
    }
    else {
        ok = cp_rec->readLastRecord ();
        fillForms ();
    }

    ui->btnCPFirst->setEnabled ( ui->btnCPFirst->isEnabled () || ok );
    ui->btnCPPrev->setEnabled ( ui->btnCPPrev->isEnabled () || ok );
    ui->btnCPNext->setEnabled ( !ok );
    ui->btnCPLast->setEnabled ( !ok );
}

void companyPurchasesUI::btnCPPrev_clicked ()
{
    bool b_isfirst ( false );
    bool ok ( false );

    if ( mbSearchIsOn ) {
        if ( ( ok = searchPrev () ) ) {
            b_isfirst = ( recIntValue ( cp_rec, FLD_CP_ID ) == VDB ()->firstDBRecord ( cp_rec->t_info.table_order ) );
            showSearchResult_internal ( true );
        }
    }
    else {
        cp_rec->readPrevRecord ();
        b_isfirst = ( recIntValue ( cp_rec, FLD_CP_ID ) == signed ( VDB ()->firstDBRecord ( TABLE_CP_ORDER ) ) );
        fillForms ();
    }

    ui->btnCPFirst->setEnabled ( !b_isfirst );
    ui->btnCPPrev->setEnabled ( !b_isfirst );
    ui->btnCPNext->setEnabled ( ui->btnCPNext->isEnabled () || ok );
    ui->btnCPLast->setEnabled ( ui->btnCPLast->isEnabled () );
}

void companyPurchasesUI::btnCPNext_clicked ()
{
    bool b_islast ( false );
    bool ok ( false );
    if ( mbSearchIsOn ) {
        if ( ( ok = searchPrev () ) ) {
            b_islast = ( recIntValue ( cp_rec, FLD_CP_ID ) == VDB ()->lastDBRecord ( cp_rec->t_info.table_order ) );
            showSearchResult_internal ( true );
        }
    }
    else {
        ok = cp_rec->readNextRecord ();
        b_islast = ( recIntValue ( cp_rec, FLD_CP_ID ) == signed ( VDB ()->lastDBRecord ( TABLE_CP_ORDER ) ) );
        fillForms ();
    }

    ui->btnCPFirst->setEnabled ( ui->btnCPFirst->isEnabled () || ok );
    ui->btnCPPrev->setEnabled ( ui->btnCPPrev->isEnabled () || ok );
    ui->btnCPNext->setEnabled ( !b_islast );
    ui->btnCPLast->setEnabled ( !b_islast );
}

void companyPurchasesUI::btnCPSearch_clicked ( const bool checked )
{
    if ( checked ) {
        if ( ui->txtCPSearch->text ().count () >= 2 && ui->txtCPSearch->text () != mSearchTerm ) {
            searchCancel ();
            const QString searchTerm ( ui->txtCPSearch->text () );
            if ( cp_rec->readFirstRecord ( -1, searchTerm ) ) {
                cp_rec->contains ( searchTerm, mFoundFields );
                mbSearchIsOn = !mFoundFields.isEmpty ();
            }
            ui->btnCPNext->setEnabled ( mbSearchIsOn );
            if ( mbSearchIsOn ) {
                mSearchTerm = searchTerm;
                ui->btnCPSearch->setText ( tr ( "Cancel search" ) );
                showSearchResult_internal ( true );
            }
        }
    }
    else
        searchCancel ();
    ui->btnCPSearch->setChecked ( checked );
}

void companyPurchasesUI::btnCPAdd_clicked ( const bool checked )
{
    if ( checked ) {
        cp_rec->setAction ( ACTION_ADD );
        clearForms ();
        controlForms ();
        ui->btnCPAdd->setText ( tr ( "Save" ) );
        ui->btnCPAdd->setIcon ( ICON ( "document-save.png" ) );
        ui->btnCPRemove->setText ( tr ( "Cancel" ) );
        ui->txtCPSupplier->setFocus ();
    }
    else
        saveInfo ();
}

void companyPurchasesUI::btnCPEdit_clicked ( const bool checked )
{
    if ( checked ) {
        cp_rec->setAction ( ACTION_EDIT );
        controlForms ();
        ui->btnCPEdit->setText ( tr ( "Save" ) );
        ui->btnCPEdit->setIcon ( ICON ( "document-save.png" ) );
        ui->btnCPRemove->setText ( tr ( "Cancel" ) );
        ui->txtCPSupplier->setFocus ();
    }
    else
        saveInfo ();
}

void companyPurchasesUI::btnCPRemove_clicked ()
{
    if ( ui->btnCPAdd->isChecked () || ui->btnCPEdit->isChecked () ) { // cancel
        ui->btnCPAdd->setChecked ( false );
        ui->btnCPEdit->setChecked ( false );
        cp_rec->setAction( ACTION_REVERT );
        controlForms ();
        fillForms ();
    }
    else {
		if ( VM_NOTIFY ()->questionBox ( tr ( "Question" ), tr ( "Remove current displayed record ?" ) ) ) {
            if ( cp_rec->deleteRecord () ) {
                if ( !cp_rec->readNextRecord () ) {
                    if ( !cp_rec->readPrevRecord () )
						cp_rec->readRecord ( 1 );
                }
                fillForms ();
                ui->btnCPFirst->setEnabled ( cp_rec->actualRecordInt ( FLD_CP_ID ) != signed ( VDB ()->firstDBRecord ( TABLE_CP_ORDER ) ) );
                ui->btnCPPrev->setEnabled ( cp_rec->actualRecordInt ( FLD_CP_ID ) != signed ( VDB ()->firstDBRecord ( TABLE_CP_ORDER ) ) );
                ui->btnCPNext->setEnabled ( cp_rec->actualRecordInt ( FLD_CP_ID ) != signed ( VDB ()->lastDBRecord ( TABLE_CP_ORDER ) ) );
                ui->btnCPLast->setEnabled ( cp_rec->actualRecordInt ( FLD_CP_ID ) != signed ( VDB ()->lastDBRecord ( TABLE_CP_ORDER ) ) );
                ui->btnCPSearch->setEnabled ( signed ( VDB ()->lastDBRecord ( TABLE_CP_ORDER ) ) > 0 );
                ui->txtCPSearch->setEditable ( signed ( VDB ()->lastDBRecord ( TABLE_CP_ORDER ) ) > 0 );
            }
        }
    }
}

void companyPurchasesUI::relevantKeyPressed ( const QKeyEvent* ke )
{
    switch ( ke->key () ) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
        if ( ui->btnCPAdd->isChecked () ) {
            ui->btnCPAdd->setChecked ( false );
            btnCPAdd_clicked ( false );
        }
        else if ( ui->btnCPEdit->isChecked () ) {
            ui->btnCPEdit->setChecked ( false );
            btnCPEdit_clicked ( false );
        }
        break;
    case Qt::Key_Escape:
        if ( ui->btnCPAdd->isChecked () || ui->btnCPEdit->isChecked () ) // cancel
            btnCPRemove_clicked ();
        break;
    case Qt::Key_F2:
        if ( !ui->btnCPAdd->isChecked () && !ui->btnCPEdit->isChecked () )
            btnCPPrev_clicked ();
    case Qt::Key_F3:
        if ( !ui->btnCPAdd->isChecked () && !ui->btnCPEdit->isChecked () )
            btnCPNext_clicked ();
        break;
    default:
        break;
    }
}

void companyPurchasesUI::txtCP_textAltered ( const vmWidget* const sender )
{
    setRecValue ( cp_rec, sender->id (), sender->text () );
}

void companyPurchasesUI::dteCP_dateAltered ( const vmWidget* const sender )
{
    setRecValue ( cp_rec, sender->id (), static_cast<const vmDateEdit* const> ( sender )->date ().toString ( DATE_FORMAT_DB ) );
}

void companyPurchasesUI::btnCPShowSupplier_clicked ( const bool checked )
{
    if ( checked )
        SUPPLIERS ()->showDialog ();
    else
        SUPPLIERS ()->hideDialog ();
}

void companyPurchasesUI::txtCPSearch_textAltered ( const QString &text )
{
    if ( text.length () == 0 ) {
        ui->txtCPSearch->setText ( mSearchTerm );
        ui->btnCPSearch->setEnabled ( !mSearchTerm.isEmpty () );
    }
    ui->btnCPSearch->setEnabled ( text.length () >= 3 );
}
