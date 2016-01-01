#include "todolist.h"
#include "configops.h"
#include "separatewindow.h"
#include "global.h"
#include "vmtablewidget.h"
#include "stringrecord.h"
#include "textdb.h"
#include "texteditwithcompleter.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCoreApplication>
#include <QToolButton>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>

static const uint TOTAL_FIELDS ( 7 );

static QToolButton* btnChooseSound ( nullptr );
static QCheckBox* chkPlaySound ( nullptr );
static QCheckBox* chkExecApp ( nullptr );
static QCheckBox* chkShowMessage ( nullptr );
static QToolButton* btnChooseApp ( nullptr );
static QLineEdit* txtMessage ( nullptr );
static QLineEdit* txtSound ( nullptr );
static QLineEdit* txtApp ( nullptr );

enum { FLD_TODO_ID = 0, FLD_TODO_STARTDATE = 1, FLD_TODO_IMPORTANCE = 2, FLD_TODO_STATUS =3,
       FLD_TODO_REPEAT = 4, FLD_TODO_NOTIFICATION = 5, FLD_TODO_DESCRIPTION = 6
     };

static const QStringList listFieldNames ( QStringList () << "ID" << QCoreApplication::tr ( "Initial Date" ) <<
        QCoreApplication::tr ( "Importance" ) << QCoreApplication::tr ( "Status" ) << QCoreApplication::tr ( "Repeat" ) <<
        QCoreApplication::tr ( "Notification" ) << QCoreApplication::tr ( "Description" ) );

static const QStringList importanceList ( QStringList () << QCoreApplication::tr ( "Highly important" ) <<
        QCoreApplication::tr ( "Important" ) << QCoreApplication::tr ( "Not important" ) );

static const QStringList statusList ( QStringList () << QCoreApplication::tr ( "Executed" ) << QCoreApplication::tr ( "Pending" ) );

static const QStringList repeatList ( QStringList () << QCoreApplication::tr ( "Today only" ) <<
                                      QCoreApplication::tr ( "This week" ) << QCoreApplication::tr ( "This month" ) << QCoreApplication::tr ( "Until status executed" ) );

static const QString notification_col_text ( QCoreApplication::tr ( "Modify notification" ) );
static const QString description_col_text ( QCoreApplication::tr ( "Edit description" ) );

static void decodeNotificationString ( const QString& notify_str )
{
    if ( notify_str.isEmpty () ) return;

    stringRecord notify_rec ( notify_str );
    if ( notify_rec.first () )
        txtMessage->setText ( notify_rec.curValue () );
    else
        txtMessage->clear ();
    chkShowMessage->setChecked ( !txtMessage->text ().isEmpty () );

    if ( notify_rec.next () )
        txtSound->setText ( notify_rec.curValue () );
    else
        txtSound->clear ();
    chkPlaySound->setChecked ( !txtSound->text ().isEmpty () );

    if ( notify_rec.next () )
        txtApp->setText ( notify_rec.curValue () );
    else
        txtApp->clear ();
    chkExecApp->setChecked ( !txtApp->text ().isEmpty () );
}

static void encodeNotificationString ( stringRecord& strrec )
{
    strrec.fastAppendValue ( txtMessage->text () );
    strrec.fastAppendValue ( txtSound->text () );
    strrec.fastAppendValue ( txtApp->text () );
}

todoList::todoList ( QWidget* parent )
    : mParent ( parent ), mLstToDo ( nullptr ), mTdbList ( nullptr ),
      mTxtInfo ( nullptr )
{
    mListFileName = CONFIG ()->appDataDir () + CONFIG ()->loggedUser () + "-todolist";
    mTdbList = new dataFile ( mListFileName );
    mTdbList->load ();

    vmTableColumn *fields[TOTAL_FIELDS+1] = { nullptr };
    uint i ( 0 );
    for ( ; i < ( TOTAL_FIELDS ); ++i ) { //exclude description
        fields[i] = new vmTableColumn;
        fields[i]->label = listFieldNames.at ( i );
        switch ( i ) {
        case FLD_TODO_ID:
            fields[FLD_TODO_ID]->editable = false;
            fields[FLD_TODO_ID]->width = 40;
            break;
        case FLD_TODO_NOTIFICATION:
        case FLD_TODO_DESCRIPTION:
            fields[i]->editable = false;
            break;
        case FLD_TODO_STARTDATE:
            fields[FLD_TODO_STARTDATE]->wtype = WT_DATEEDIT;
            break;
        default:
            fields[i]->wtype = WT_COMBO;
            break;
        }
    }
    mLstToDo = new vmTableWidget ( 5, fields );
    //mLstToDo->setComboList ( FLD_TODO_IMPORTANCE, importanceList );
    //mLstToDo->setComboList ( FLD_TODO_STATUS, statusList );
    //mLstToDo->setComboList ( FLD_TODO_REPEAT, repeatList );

    mLstToDo->renameAction ( ADD_ROW, tr ( "Insert new appointment/task" ) );
    mLstToDo->renameAction ( DEL_ROW, tr ( "Remove this appointment/task" ) );
    mLstToDo->renameAction ( CLEAR_ROW, tr ( "Clear this appointment/task" ) );

    connect ( mLstToDo, SIGNAL ( currentCellChanged ( const int, const int, const int, const int ) ), this, SLOT ( cellActivated ( const int, const int, const int, const int ) ) );

    mTxtInfo = new textEditWithCompleter ( nullptr );
    mTxtInfo->setEnabled ( false );

    QHBoxLayout* hLayout = new QHBoxLayout;
    hLayout->setMargin ( 2 );
    hLayout->setSpacing ( 2 );
    hLayout->addWidget ( mLstToDo, 2 );
    hLayout->addWidget ( mTxtInfo, 1 );

    chkShowMessage = new QCheckBox ( tr ( "Show Message" ) );
    chkShowMessage->setEnabled ( false );
    connect ( chkShowMessage, SIGNAL( clicked ( const bool ) ), this, SLOT ( showMessage ( const bool ) ) );
    txtMessage = new QLineEdit;
    txtMessage->setEnabled ( false );

    chkPlaySound = new QCheckBox ( tr ( "Play sound" ) );
    chkPlaySound->setEnabled ( false );
    connect ( chkPlaySound, SIGNAL( clicked ( const bool ) ), this, SLOT ( playSound ( const bool ) ) );
    btnChooseSound = new QToolButton;
    btnChooseSound->setFixedSize ( 30, 30 );
    btnChooseSound->setEnabled ( false );
    txtSound = new QLineEdit;
    txtSound->setEnabled ( false );

    chkExecApp = new QCheckBox ( tr ( "Run app" ) );
    chkExecApp->setEnabled ( false );
    connect ( chkExecApp, SIGNAL( clicked ( const bool ) ), this, SLOT ( execApp ( const bool ) ) );
    btnChooseApp = new QToolButton;
    btnChooseApp->setFixedSize ( 30, 30 );
    btnChooseApp->setEnabled ( false );
    txtApp = new QLineEdit;
    txtApp->setEnabled ( false );

    QHBoxLayout* hLayout2 = new QHBoxLayout;
    hLayout2->setMargin ( 2 );
    hLayout2->setSpacing ( 2 );
    hLayout2->addWidget ( chkShowMessage, 1 );
    hLayout2->addWidget ( txtMessage, 2 );
    hLayout2->addWidget ( chkPlaySound, 1 );
    hLayout2->addWidget ( txtSound, 2 );
    hLayout2->addWidget ( btnChooseSound, 0 );
    hLayout2->addWidget ( chkExecApp, 1 );
    hLayout2->addWidget ( txtApp, 2 );
    hLayout2->addWidget ( btnChooseApp, 0 );

    QVBoxLayout* vLayout2 = new QVBoxLayout;
    vLayout2->setMargin ( 0 );
    vLayout2->setSpacing ( 0 );
    vLayout2->addLayout ( hLayout );
    vLayout2->addLayout ( hLayout2 );
    mParent->setLayout ( vLayout2 );
}

todoList::~todoList ()
{
    saveList ();
    delete mTdbList;
}

void todoList::recordNotification ( const uint row )
{
    stringRecord str;
    encodeNotificationString ( str );
    if ( str.toString () != mLstToDo->hiddenText ( row, FLD_TODO_NOTIFICATION ) ) {
        mLstToDo->setHiddenText ( row, FLD_TODO_NOTIFICATION, str.toString () );
        mLstToDo->sheetItem ( row, FLD_TODO_NOTIFICATION )->setText ( notification_col_text, false, false );
    }
}

void todoList::recordDescription ( const uint row )
{
    if ( mTxtInfo->document()->isModified () ) {
        mLstToDo->setHiddenText ( row, FLD_TODO_DESCRIPTION, mTxtInfo->document ()->toPlainText () );
        mLstToDo->sheetItem ( row, FLD_TODO_NOTIFICATION )->setText ( description_col_text, false, false );
    }
}

void todoList::cellActivated ( const int current_row, const int current_col, const int prev_row, int prev_col )
{
    if ( current_row != prev_row ) {
        if ( prev_col == current_col ) {
            switch ( prev_col ) {
            default:
                break;
            case FLD_TODO_NOTIFICATION:
                recordNotification ( current_row != prev_row ? prev_row : current_row );
                break;
            case FLD_TODO_DESCRIPTION:
                recordDescription ( current_row != prev_row ? prev_row : current_row );
                break;
            }
        }
    }

    if ( prev_col != current_col ) {
        switch ( prev_col ) {
        default:
            break;
        case FLD_TODO_NOTIFICATION:
            recordNotification ( current_row != prev_row ? prev_row : current_row );
            break;
        case FLD_TODO_DESCRIPTION:
            recordDescription ( current_row != prev_row ? prev_row : current_row );
            break;
        }
    }

    if ( current_col == FLD_TODO_NOTIFICATION ) {
        decodeNotificationString ( mLstToDo->hiddenText ( current_row, current_col ) );
        chkShowMessage->setEnabled ( true );
        txtMessage->setEnabled ( true );
        chkPlaySound->setEnabled ( true );
        btnChooseSound->setEnabled ( true );
        txtSound->setEnabled ( true );
        chkExecApp->setEnabled ( true );
        btnChooseApp->setEnabled ( true );
    }
    else {
        chkShowMessage->setEnabled ( false );
        chkShowMessage->setChecked ( false );
        txtMessage->setEnabled ( false );
        txtMessage->clear ();
        chkPlaySound->setEnabled ( false );
        chkPlaySound->setChecked ( false );
        btnChooseSound->setEnabled ( false );
        txtSound->setEnabled ( false );
        txtSound->clear ();
        chkExecApp->setEnabled ( false );
        chkExecApp->setChecked ( false );
        btnChooseApp->setEnabled ( false );
        txtApp->setEnabled ( false );
        txtApp->clear ();
    }

    mTxtInfo->setEnabled ( current_col == FLD_TODO_DESCRIPTION );
    if ( current_col == FLD_TODO_DESCRIPTION )
        mTxtInfo->setFocus ();
    if ( current_row != prev_row ) {
        mTxtInfo->setText ( mLstToDo->hiddenText ( current_row, FLD_TODO_DESCRIPTION ) );
        mTxtInfo->document ()->setModified ( false );
    }
}

void todoList::showMessage ( const bool checked )
{
    txtMessage->setEnabled ( checked );
    if ( checked )
        txtMessage->setFocus ();
}

void todoList::playSound ( const bool checked )
{
    btnChooseSound->setEnabled ( checked );
    if ( checked )
        txtSound->setFocus ();
}

void todoList::execApp ( const bool checked )
{
    btnChooseApp->setEnabled ( checked );
    if ( checked )
        txtApp->setFocus ();
}

bool todoList::dateInRange ( const vmNumber& date, const stringRecord& str_appointment ) const
{
    vmNumber recDate ( str_appointment.fieldValue ( FLD_TODO_STARTDATE ), VMNT_DATE, vmNumber::VDF_HUMAN_DATE );
    if ( date <= recDate )
        return true; // future appointments will all be shown. We will only exclude those after the recorded date

    const QString strRepeat ( str_appointment.fieldValue ( FLD_TODO_REPEAT ) );
    uint n ( 0 );

    if ( strRepeat.startsWith ( tr ( "This" ) ) ) {
        if ( strRepeat.contains ( tr ( "week" ) ) )
            n = 7;
        else
            n = unsigned ( date.daysInMonth ( date.month (), date.year () ) );
    }
    else if ( strRepeat.startsWith ( tr ( "Until" ) ) )
        n = 100; //any number

    if ( n != 100 ) {
        recDate.setDate ( n, 0 ,0, true );
        if ( recDate >= date )
            return true;
    }
    else {
        if ( str_appointment.fieldValue ( FLD_TODO_STATUS ).startsWith ( tr ( "Pend" ) ) )
            return true; // show until Executed
    }
    return false;
}

void todoList::displayList ( const vmNumber& date )
{
    saveList ();
    mLstToDo->clear();

    spreadRow* s_row = new spreadRow;
    uint entry_n ( 0 );

    // Initialize, outside loop, column values that will be used by setRowData ()
    for ( uint i_row ( 0 ); i_row < TOTAL_FIELDS; i_row++ )
        s_row->column[i_row] = i_row;

    s_row->row = 0;
    stringRecord str_appointment;
    while ( mTdbList->getRecord ( str_appointment, entry_n ) ) { // browse through all records and selected the ones we need
        if ( dateInRange ( date, str_appointment ) ) {
            s_row->field_value[FLD_TODO_ID] = str_appointment.first ();
            s_row->field_value[FLD_TODO_STARTDATE] = str_appointment.next ();
            s_row->field_value[FLD_TODO_IMPORTANCE] = str_appointment.next ();
            s_row->field_value[FLD_TODO_STATUS] = str_appointment.next ();
            s_row->field_value[FLD_TODO_REPEAT] = str_appointment.next ();
            s_row->field_value[FLD_TODO_NOTIFICATION] = notification_col_text;
            s_row->field_value[FLD_TODO_DESCRIPTION] = description_col_text;
            mLstToDo->setRowData ( s_row );
            mLstToDo->setHiddenText ( s_row->row, FLD_TODO_NOTIFICATION,
                                      s_row->field_value[FLD_TODO_NOTIFICATION], false );
            mLstToDo->setHiddenText ( s_row->row, FLD_TODO_DESCRIPTION,
                                      s_row->field_value[FLD_TODO_DESCRIPTION], false );
            ++( s_row->row );
        }
        ++entry_n;
    }
    delete s_row;
    mLstToDo->scrollToTop ();
}

bool todoList::saveList ()
{
    if ( mLstToDo->currentColumn () == FLD_TODO_NOTIFICATION ) // changes are not yet recored
        recordNotification ( mLstToDo->currentRow () );
    else if ( mLstToDo->currentColumn() == FLD_TODO_DESCRIPTION )
        recordDescription ( mLstToDo->currentRow () );

    if ( mLstToDo->tableChanged () ) {
        uint i_row ( 0 );
        spreadRow* s_row ( new spreadRow );
        stringRecord str_appointment;

        for ( uint idx ( 0 ); idx < mLstToDo->totalChangedRows (); ++idx ) {
            i_row = mLstToDo->modifiedRow ( idx );
            mLstToDo->rowData ( i_row, s_row );
            // We save into the database the hidden text, the one that has useful info
            s_row->field_value[FLD_TODO_NOTIFICATION] = mLstToDo->hiddenText ( i_row, FLD_TODO_NOTIFICATION );
            s_row->field_value[FLD_TODO_DESCRIPTION] = mLstToDo->hiddenText ( i_row, FLD_TODO_DESCRIPTION );

            if ( mLstToDo->sheetItem ( i_row, FLD_TODO_ID )->text ().isEmpty () )
                s_row->field_value[FLD_TODO_ID] = QString::number ( mTdbList->recCount () );

            str_appointment.fastAppendValue ( s_row->field_value.at ( FLD_TODO_ID ) );
            str_appointment.fastAppendValue ( s_row->field_value.at ( FLD_TODO_STARTDATE ) );
            str_appointment.fastAppendValue ( s_row->field_value.at ( FLD_TODO_IMPORTANCE ) );
            str_appointment.fastAppendValue ( s_row->field_value.at ( FLD_TODO_STATUS ) );
            str_appointment.fastAppendValue ( s_row->field_value.at ( FLD_TODO_REPEAT ) );
            str_appointment.fastAppendValue ( s_row->field_value.at ( FLD_TODO_NOTIFICATION ) );
            str_appointment.fastAppendValue ( s_row->field_value.at ( FLD_TODO_DESCRIPTION ) );
            if ( mLstToDo->sheetItem ( i_row, FLD_TODO_ID )->text ().isEmpty () ) {
                mTdbList->appendRecord ( str_appointment );
                mLstToDo->sheetItem ( i_row, FLD_TODO_ID )->setText ( s_row->field_value.at ( FLD_TODO_ID ), false, false );
            }
            else
                mTdbList->changeRecord ( i_row, str_appointment );
            mTdbList->commit();
            str_appointment.clear ();
        }
        mLstToDo->setTableUpdated ();
        delete s_row;
        return true;
    }
    return false;
}
