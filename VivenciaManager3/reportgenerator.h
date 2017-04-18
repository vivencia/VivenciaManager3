#ifndef REPORTGENERATOR_H
#define REPORTGENERATOR_H

#include "texteditor.h"
#include "vmlist.h"
#include "stringrecord.h"
#include "vmnumberformats.h"

#include <QDockWidget>

class Client;
class Job;
class Payment;

class vmComboBox;
class vmLineEdit;
class textEditWithCompleter;

class QFrame;
class QVBoxLayout;

static const uint REPORT_GENERATOR_SUB_WINDOW ( 8 );

class reportGenerator : public textEditor
{

friend class dockQP;
friend class dockBJ;

public:
	explicit reportGenerator ( documentEditor* mdiParent );
	virtual ~reportGenerator ();

	void show ( const uint jobid = 0, const uint payid = 0 );
	
	static inline QString filter () {
		return tr ( "VivenciaManager Report (*.vmr)" );
	}

	bool loadFile ( const QString& filename );
	bool saveAs ( const QString& filename );

	void createJobReport ( const uint c_id, const bool include_all_paid = false,
						   const QString& filename = QString::null, const bool show = true );
	void createPaymentStub ( const uint payID = 0 );
	void buildMailMessage ( QString& address, QString& subject, QString& attachment, QString& body );
	const QString initialDir () const;

private:
	void insertReadCarefullyBlock ();
	void insertPaymentStubText ();
	void getUnpaidPayments ( Payment * const pay );
	void getPaidPayments ( const uint n_months_past = 12, const uint match_payid = 0, vmNumber* const total_paid_value = nullptr );
	void insertReportsText ( const bool include_all_pays );
	void printPayInfo ( const Payment* pay, const uint pay_number, vmNumber& paid_total );
	void btnInsertHeader_clicked ( const uint c_id = 0 );
	void btnInsertFooter_clicked ( const uint c_id = 0 );
	void btnInsertLogo_clicked ( const uint c_id = 0 );
	void btnInsertFootNotes_clicked ( const uint c_id = 0 );
	void btnInsertProjectNumber_clicked ();

	void updateJobIDsAndQPs ( const QString& text );
	void showProjectID ( const int = 0 );
	void changeJobBriefInfo ( const int );
	void btnViewJob_clicked ( const bool );

	void btnCopyTableRow_clicked ();
	void btnCopyEntireTable_clicked ();
	void btnInsertClientAddress_clicked ( const uint c_id = 0 );
	void btnInsertJobReport_clicked ();
	void btnGenerateReport_clicked ( const uint c_id = 0, const bool include_all_paid = false );

	QString mPDFName;

	QPushButton* btnInsertHeader, *btnInsertFooter;
	QPushButton* btnInsertLogo;
	QPushButton* btnInsertProjectNumber, *btnInsertFootNotes;
	QToolButton* btnGenerateReport;
	QToolButton* btnViewJob;

	vmComboBox* cboQPIDs, *cboJobIDs;
	vmComboBox* cboHeaderType, *cboHeaderJobType;

	vmLineEdit* txtClientsIDs;

	QPushButton* btnCopyTableRow, *btnCopyEntireTable;
	QPushButton* btnInsertClientAddress, *btnInsertJobReport;

	bool mb_IgnoreChange;

	Client* rgClient;
	Job* rgJob;
	Payment* rgPay;
};

class dockQP : public QDockWidget
{

public:
	void attach ( reportGenerator* instance );
	void detach ( reportGenerator* instance );
	void setCurrentWindow ( reportGenerator* report );

private:
	explicit dockQP ();

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

public:
	void attach ( reportGenerator* instance );
	void detach ( reportGenerator* instance );
	void setCurrentWindow ( reportGenerator* report );
	void fillControls ( const Job *job );
	void btnCopyJobType_clicked ();
	void btnCopyJobDate_clicked ();
	void btnCopyJobPrice_clicked ();
	void btnCopyJobReport_clicked ();

private:
	explicit dockBJ ();

	friend dockBJ* DOCK_BJ ();
	friend void deleteDockBJInstance ();
	static dockBJ* s_instance;

	PointersList<reportGenerator*> instances;

	QFrame* frmBriefJob;
	vmLineEdit* txtBriefJobType;
	vmLineEdit* txtBriefJobDate;
	vmLineEdit* txtBriefJobPrice;
	textEditWithCompleter* txtBriefJobReport;

	reportGenerator* cur_report;
};

inline dockBJ* DOCK_BJ ()
{
	if ( !dockBJ::s_instance )
		dockBJ::s_instance = new dockBJ;
	return dockBJ::s_instance;
}

#endif // REPORTGENERATOR_H
