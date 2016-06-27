#ifndef REPORTGENERATOR_H
#define REPORTGENERATOR_H

#include "texteditor.h"
#include "vmlist.h"

#include <QDockWidget>

class Client;
class Job;
class Payment;

class QComboBox;
class QLineEdit;
class QFrame;
class QTextEdit;
class QVBoxLayout;

static const uint REPORT_GENERATOR_SUB_WINDOW ( 8 );

class reportGenerator : public textEditor
{

    Q_OBJECT

    friend class dockQP;
    friend class dockBJ;

public:
    explicit reportGenerator ( documentEditor* mdiParent );
    ~reportGenerator ();

    void show ( const int jobid = -1, const int payid = -1 );
    \
    static inline QString filter () {
        return tr ( "VivenciaManager Report (*.vmr)" );
    }

    bool loadFile ( const QString& filename );
    bool saveAs ( const QString& filename );

    void createJobReport ( const int c_id, const bool include_all_paid = false,
                           const QString& filename = QString::null, const bool show = true );
    void createPaymentStub ( const int payID = -1 );
    void buildMailMessage ( QString& address, QString& subject, QString& attachment, QString& body );
    const QString initialDir () const;

private slots:
    void btnInsertHeader_clicked ( const int c_id = -1 );
    void btnInsertFooter_clicked ( const int c_id = -1 );
    void btnInsertLogo_clicked ( const int c_id = -1 );
    void btnInsertFootNotes_clicked ( const int c_id = -1 );
    void btnInsertProjectNumber_clicked ();

    void updateJobIDsAndQPs ( const QString& text );
    void showProjectID ( const int = 0 );
    void changeJobBriefInfo ( const int );
    void btnViewJob_clicked ( const bool );

    void btnCopyTableRow_clicked ();
    void btnCopyEntireTable_clicked ();
    void btnInsertClientAddress_clicked ( const int c_id = -1 );
    void btnInsertJobReport_clicked ();
    void btnGenerateReport_clicked ( const int c_id = -1, const bool include_all_paid = false );

private:
    void insertReadCarefullyBlock ();
    void insertPaymentStubText ();
    void insertReportsText ( const bool include_all_pays );

    QString mPDFName;

    QPushButton* btnInsertHeader, *btnInsertFooter;
    QPushButton* btnInsertLogo;
    QPushButton* btnInsertProjectNumber, *btnInsertFootNotes;
    QToolButton* btnGenerateReport;
    QToolButton* btnViewJob;

    QComboBox* cboQPIDs, *cboJobIDs;
    QComboBox* cboHeaderType, *cboHeaderJobType;

    QLineEdit* txtClientsIDs;

    QPushButton* btnCopyTableRow, *btnCopyEntireTable;
    QPushButton* btnInsertClientAddress, *btnInsertJobReport;

    bool mb_IgnoreChange;

    Client* rgClient;
    Job* rgJob;
    Payment* rgPay;
};

class dockQP : public QDockWidget
{

    Q_OBJECT

public:
    void attach ( reportGenerator* instance );
    void detach ( reportGenerator* instance );
    void setCurrentWindow ( reportGenerator* report );

private:
    dockQP ();

    friend dockQP* DOCK_QP ();
    friend void deleteDockQPInstance ();
    static dockQP* s_instance;

    PointersList<reportGenerator*> instances;
    reportGenerator* cur_report;
    QVBoxLayout* mainLayout;
};

inline dockQP* DOCK_QP ()
{
    if ( !dockQP::s_instance )
        dockQP::s_instance = new dockQP;
    return dockQP::s_instance;
}

class dockBJ : public QDockWidget
{

    Q_OBJECT

public:
    void attach ( reportGenerator* instance );
    void detach ( reportGenerator* instance );
    void setCurrentWindow ( reportGenerator* report );
    void fillControls ( const Job *job );

public slots:
    void btnCopyJobType_clicked ();
    void btnCopyJobDate_clicked ();
    void btnCopyJobPrice_clicked ();
    void btnCopyJobReport_clicked ();

private:
    dockBJ ();

    friend dockBJ* DOCK_BJ ();
    friend void deleteDockBJInstance ();
    static dockBJ* s_instance;

    PointersList<reportGenerator*> instances;

    QFrame* frmBriefJob;
    QLineEdit* txtBriefJobType;
    QLineEdit* txtBriefJobDate;
    QLineEdit* txtBriefJobPrice;
    QTextEdit* txtBriefJobReport;

    reportGenerator* cur_report;
};

inline dockBJ* DOCK_BJ ()
{
    if ( !dockBJ::s_instance )
        dockBJ::s_instance = new dockBJ;
    return dockBJ::s_instance;
}

#endif // REPORTGENERATOR_H
