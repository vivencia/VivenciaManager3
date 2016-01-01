#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "vmnumberformats.h"
#include "dbrecord.h"
#include "client.h"
#include "job.h"
#include "payment.h"
#include "purchases.h"
#include "vmnotify.h"
#include "ui_mainwindow.h"

#include <QMainWindow>
#include <QDialog>
#include <QSystemTrayIcon>

class vmTaskPanel;
class vmAction;
class vmListItem;
class clientListItem;
class jobListItem;
class schedListItem;
class payListItem;
class buyListItem;
class separateWindow;
class todoList;
class vmAction;
class vmNotify;
class vmListWidget;
class crashRestore;
class vmLineEdit;
class vmTableWidget;
class dbCalendar;
class vmActionGroup;

class QDateEdit;
class QEvent;
class QKeyEvent;
class QComboBox;
class QMenu;
class QSystemTrayIcon;
class QToolButton;
class QLineEdit;
class QFrame;
class QLabel;
class QListWidgetItem;
class QModelIndex;
class QTimer;
class QTabWidget;

namespace Ui
{
class MainWindow;
}

/*#define CLIENT_CUR_ITEM static_cast<clientListItem*> ( clientsList->currentItem () )
#define JOB_CUR_ITEM static_cast<jobListItem*> ( jobsList->currentItem () )
#define PAY_CUR_ITEM static_cast<payListItem*> ( paysList->currentItem () )
#define BUY_CUR_ITEM static_cast<buyListItem*> ( buysList->currentItem () )*/

class MainWindow : public QMainWindow
{

friend class Data;
friend class searchUI;

public:

	enum TAB_INDEXES { TI_MAIN = 0, TI_CALENDAR, TI_INVENTORY, TI_SUPPLIES };

	enum { BLOCK_HOURS = 0, ALL_HOURS = 1, ADD_NEW_DAY_TODAY = 2, ADD_NEW_DAY_YESTERDAY = 3 };

	enum { CCO_MIGRATE = 0 };

	enum JOB_INFO_LIST_USER_ROLES
	{
		JILUR_DATE = Qt::UserRole + Job::JRF_DATE,
		JILUR_START_TIME = Qt::UserRole + Job::JRF_START_TIME,
		JILUR_END_TIME = Qt::UserRole + Job::JRF_END_TIME,
		JILUR_WHEATHER = Qt::UserRole + Job::JRF_WHEATHER,
		JILUR_REPORT = Qt::UserRole + Job::JRF_REPORT,
		JILUR_ADDED = JILUR_REPORT + 1,
		JILUR_REMOVED = JILUR_REPORT + 2,
		JILUR_DAY_TIME = JILUR_REPORT + 3
	};

	MainWindow ();
	~MainWindow ();

	void continueStartUp ();

//--------------------------------------------CLIENT------------------------------------------------------------
	void saveClientWidget ( vmWidget* widget, const uint id );
	void showClientSearchResult ( vmListItem* item, const bool bshow );
	void setupClientPanel ();
	void clientKeyPressedSelector ( const QKeyEvent* ke );
	void clientsListWidget_currentItemChanged ( vmListItem* item, vmListItem* previous = nullptr );
	void controlClientForms ();
	bool displayClient ( clientListItem* client_item, const bool b_select = false, jobListItem* job_item = nullptr, buyListItem* buy_item = nullptr );
	void displayClientInfo ( const Client* const client );
	clientListItem* getClientItem ( const int id ) const;
	void fillAllLists ( const clientListItem* client_item );
    void btnClientAdd_clicked ();
    void btnClientEdit_clicked ();
    void btnClientDel_clicked ();
    void btnclientSave_clicked ();
    void btnClientCancel_clicked ();
//--------------------------------------------CLIENT------------------------------------------------------------

//--------------------------------------------EDITING-FINISHED-CLIENT-----------------------------------------------------------
	void txtClientName_textAltered ( const QString& text );
	void txtClient_textAltered ( const vmWidget* const sender );
	void txtClientZipCode_textAltered ( const QString& text );
	void dteClient_dateAltered ( const vmWidget* const sender );
	void contactsClientAdd ( const QString& info, const vmWidget* const sender );
	void contactsClientDel ( const int idx, const vmWidget* const sender );
//--------------------------------------------EDITING-FINISHED-CLIENT-----------------------------------------------------------

//--------------------------------------------JOB------------------------------------------------------------
	void saveJobWidget ( vmWidget* widget, const uint id );
	void showJobSearchResult ( vmListItem* item, const bool bshow );
	void setupJobPanel ();
	void setUpJobButtons ( const QString& path );
	void setupJobPictureControl ();
	void displayJobFromCalendar ( QListWidgetItem* cal_item );
	void updateCalendarWithJobInfo ( Job* const job, const RECORD_ACTION action );
	void jobKeyPressedSelector ( const QKeyEvent* ke );
	void jobsListWidget_currentItemChanged ( vmListItem* item, vmListItem* previous = nullptr );
	void jobDayReportListWidget_currentItemChanged ( vmListItem* item, vmListItem* = nullptr );
	void controlJobForms ();
	void controlJobDayForms ();
    void controlJobPictureControls (); //must be called from within displayJobInfo or after because it relies on having read the database for info
	void displayJob ( jobListItem* job_item, const bool b_select = false, buyListItem* buy_item = nullptr );
	void displayJobInfo ( const Job* const job );
	jobListItem* getJobItem ( const clientListItem* const parent_client, const int id ) const;
	void preFillJobInfo ();
	void scanJobImages ();
	void decodeJobReportInfo ( jobListItem* job_item );
	void updateJobInfo ( const QString& text, const uint user_role );
    void updateJobInfoByRemoval ( const uint day , const bool bUndo = false );
	void addJobPayment ( jobListItem* const job_item );
	void saveJobPayment ( const Job* const job );
	void removeJobPayment ( payListItem* pay_item );
	void calcJobTime ();
	void showJobTotalTime ( const vmNumber& time ) const;
	bool dateIsInDaysList ( const QString& str_date );
	void fillCalendarJobsList ( const stringTable& jobids, vmListWidget* list );
	bool setNewProjectName ();
    void createNewProjectDir ();
	bool renameProjectDir ( const bool phase_two = false );
	bool removeProjectDir ( const bool phase_two = false );
	bool chooseExistingDirForProject ( const bool phase_two = false );
	void newProjectAction ( const vmAction* const action = nullptr );
	void selectJob ();
	void btnJobSelect_clicked ();
	void btnJobAddDay_clicked ();
	void btnJobDelDay_clicked ();
	void btnJobCancelDelDay_clicked ();
	void btnJobPrevDay_clicked ();
	void btnJobNextDay_clicked ();
	void btnJobMachines_clicked ();
	void jobOpenProjectFile ( QAction* action );
	void jobEMailProjectFile ( QAction* action );

	inline void setTempCallbackForJobSelect ( std::function<void ( const int )> func ) {
		selJob_callback = func;
	}
//--------------------------------------------JOB------------------------------------------------------------

//--------------------------------------------EDITING-FINISHED-JOB-----------------------------------------------------------
	void cboJobType_textAltered ( const vmWidget* const sender );
	void txtJob_textAltered ( const vmWidget* const sender );
	void txtJobWheather_textAltered ( const vmWidget* const sender );
	void txtJobTotalDayTime_textAltered ( const vmWidget* const sender );
	void timeJobStart_timeAltered ();
	void timeJobEnd_timeAltered ();
	void dteJob_dateAltered ( const vmWidget* const sender );
    void dteJobAddDate_dateAltered ();
//--------------------------------------------EDITING-FINISHED-JOB-----------------------------------------------------------

//--------------------------------------------PAY------------------------------------------------------------
	void savePayWidget ( vmWidget* widget, const uint id );
	void showPaySearchResult ( vmListItem* item, const bool bshow );
	void setupPayPanel ();
	void displayPayFromCalendar ( QListWidgetItem* cal_item );
	void paysListWidget_currentItemChanged ( vmListItem* item, vmListItem* previous = nullptr );
	void paysOverdueListWidget_currentItemChanged ( vmListItem* item, vmListItem* = nullptr );
	void paysOverdueClientListWidget_currentItemChanged ( vmListItem* item, vmListItem* = nullptr );
	void updateCalendarWithPayInfo ( Payment* const pay, const RECORD_ACTION action );
	void removePaymentOverdueItems ( payListItem* pay_item );
	void payOverdueGUIActions ( const Payment* const pay );
	void controlPayForms ();
	void displayPay ( payListItem* pay_item, const bool b_select = false );
	void displayPayInfo ( const Payment* const pay );
	payListItem* getPayItem ( const clientListItem* const parent_client, const int id ) const;
	void preFillPayInfo ();
	void fillCalendarPaysList ( const stringTable& payids, vmListWidget* list, const bool use_date = false );
	void loadClientOverduesList ();
	void loadAllOverduesList ();
	void interceptPaymentCellChange ( const vmTableItem* const item );
	void updatePayTotalPaidValue ();
	void payKeyPressedSelector ( const QKeyEvent* ke );
	void tabPaysLists_currentChanged ( const int index );
//--------------------------------------------PAY------------------------------------------------------------

//------------------------------------EDITING-FINISHED-PAY-----------------------------------------------------------
	void txtPayTotalPrice_textAltered ( const QString& text );
	void txtPay_textAltered ( const vmWidget* const sender );
	void chkPayOverdue_mouseClicked ( const bool clicked );
//------------------------------------EDITING-FINISHED-PAY-----------------------------------------------------------

//--------------------------------------------BUY------------------------------------------------------------
	void saveBuyWidget ( vmWidget* widget, const uint id );
	void showBuySearchResult ( vmListItem* item, const bool bshow );
	void setupBuyPanel ();
	void displayBuyFromCalendar ( QListWidgetItem* cal_item );
	void buysListWidget_currentItemChanged ( vmListItem* item, vmListItem* previous = nullptr );
	void buysJobListWidget_currentItemChanged ( vmListItem* item, vmListItem* previous = nullptr );
	void buySuppliersListWidget_currentItemChanged ( vmListItem* item, vmListItem* );
	void updateCalendarWithBuyInfo ( Buy* const buy, const RECORD_ACTION action );
	void updateCalendarWithBuyPayInfo ( Buy* const buy, const RECORD_ACTION action );
	void controlBuyForms ();
	void displayBuy ( buyListItem* buy_item, const bool b_select = false );
	void displayBuyInfo ( const Buy* const buy );
	buyListItem* getBuyItem ( const clientListItem* const parent_client, const int id ) const;
	void fillJobBuyList ( const jobListItem* parent_job );
	void fillCalendarBuysList ( const stringTable& buyids, vmListWidget* list, const bool pay_date = false );
	void interceptBuyItemsCellChange ( const vmTableItem* const item );
	void interceptBuyPaymentCellChange ( const vmTableItem* const item );
	void updateBuyTotalPaidValue ();
	void preFillBuyInfo ();
	void buyKeyPressedSelector ( const QKeyEvent* ke );
	void getPurchasesForSuppliers ( const QString& supplier );
//--------------------------------------------BUY------------------------------------------------------------

//--------------------------------------------EDITING-FINISHED-BUY-----------------------------------------------------------
	void txtBuy_textAltered ( const vmWidget* const sender );
	void dteBuy_dateAltered ( const vmWidget* const sender );
	void cboBuySuppliers_textAltered ( const QString& text );
	void cboBuySuppliers_indexChanged ( const int idx );
//--------------------------------------------EDITING-FINISHED-BUY-----------------------------------------------------------

//----------------------------------SETUP-CUSTOM-CONTROLS-NAVIGATION--------------------------------------
	void setupCustomControls ();
	bool restoreItem ( const stringRecord& restore_info );
	void restoreLastSession ();
	void searchCallbackSelector ( const QKeyEvent* ke );
    void setupLeftPanel ();
	void setupWorkFlow ();
    void syncLeftPanelWithWorkFlow ( const int value );
    void syncWorkFlowWithLeftPanel ( const int value );
//----------------------------------SETUP-CUSTOM-CONTROLS-NAVIGATION--------------------------------------

//--------------------------------------------CALENDAR-----------------------------------------------------------
	void setupCalendarMethods ();
	void calMain_activated ( const QDate& date, const bool bUserAction = true );
    void updateCalendarView ( const uint year, const uint month );
	void tboxCalJobs_currentChanged ( const int index );
	void tboxCalPays_currentChanged ( const int index );
	void tboxCalBuys_currentChanged ( const int index );
	void changeCalendarToolBoxesText ( const vmNumber& date );
//--------------------------------------------CALENDAR-----------------------------------------------------------

	void showTab ( const TAB_INDEXES ti );
	void tabMain_currentTabChanged ( const int tab_idx );
	void findCalendarsHiddenWidgets ();
	void saveView ();
	void selectBuysItems ( const PROCESS_STEPS step );

//--------------------------------------------TRAY-IMPORT-EXPORT---------------------------------
	void createTrayIcon ( const bool b_setup = true );
	inline QSystemTrayIcon* appTrayIcon () const { return trayIcon; }
//--------------------------------------------TRAY-IMPORT-EXPORT---------------------------------

//--------------------------------------------SHARED-----------------------------------------------------------
    void removeListItem ( vmListItem* item, const bool b_select_another = true );
	void postFieldAlterationActions ( vmListItem* item );
//--------------------------------------------SHARED-----------------------------------------------------------

protected:
	void closeEvent ( QCloseEvent * );
	void changeEvent ( QEvent* e );
	bool eventFilter ( QObject* o, QEvent* e );

private:
	Ui::MainWindow* ui;

	QSystemTrayIcon* trayIcon;
	QMenu* trayIconMenu;

	QWidget* calMainView;
	QMenu* menuMainCal, *subMenuMainCal;
	QPoint mCalPopupMenuPos;
	QMenu* menuJobDoc, *menuJobXls;
	QMenu* menuJobPdf, *subMenuJobPdfView, *subMenuJobPdfEMail;

	QString m_qpStrId;

	//---------------------------CUSTOM-DIALOS-TABS--------------------------------
	todoList* todoListWidget;
	//---------------------------CUSTOM-DIALOS-TABS--------------------------------

	//---------------------------CUSTOM-WIDGETS-------------------------------------
	QTimer* timerUpdate;
	separateWindow* sepWin_JobPictures;
	separateWindow* sepWin_JobReport;

	crashRestore* crash;
	dbCalendar* mCal;

	QAction* jobsPicturesMenuAction;
	vmAction* actCountDayHours, *actCountAllHours, *actAppendDayToday, *actAppendDayYesterday;

	vmTaskPanel* clientTaskPanel;
	vmTaskPanel* mainTaskPanel;

	vmListWidget* clientsList;
	vmListWidget* jobsList;
	vmListWidget* paysList, *paysOverdueList, *paysOverdueClientList;
	vmListWidget* buysList, *buysJobList;

	clientListItem* CLIENT_PREV_ITEM;
	jobListItem* JOB_PREV_ITEM;
	payListItem* PAY_PREV_ITEM;
	buyListItem* BUY_PREV_ITEM;
	buyListItem* BUY_JOB_PREV_ITEM;

	vmActionGroup* grpClients;
	vmActionGroup* grpJobs;
	vmActionGroup* grpPays;
	vmActionGroup* grpBuys;

	std::function<void ( const int )> selJob_callback;
	QTabWidget* tabPaysLists;
	bool mb_jobPosActions;

	vmNumber mCalendarDate;
	static QLatin1String lstJobReportItemPrefix;
	PointersList<vmWidget*> clientWidgetList;
	PointersList<vmWidget*> jobWidgetList;
	PointersList<vmWidget*> payWidgetList;
	PointersList<vmWidget*> buyWidgetList;

	clientListItem* mClientCurItem;
	jobListItem* mJobCurItem;
	payListItem* mPayCurItem;
	buyListItem* mBuyCurItem;

    double leftToRightWorkflowRatio;
//---------------------------CUSTOM-WIDGETS-------------------------------------

//--------------------------------------------JOB-----------------------------------------------------------

	void on_btnJobAdd_clicked ();
	void on_btnJobEdit_clicked ();
	void on_btnJobDel_clicked ();
	void on_btnJobSave_clicked ();
	void on_btnJobCancel_clicked ();

	void btnJobReloadPictures_clicked ();
	void showClientsYearPictures ( QAction* action );
	void showJobImage ( const int index );
	void showJobImageRequested ( const int index );
	void btnJobRenamePicture_clicked ( const bool checked );
	void showJobImageInWindow ( const bool maximized );
	void btnJobSeparateReportWindow_clicked ( const bool checked );
//--------------------------------------------JOB-----------------------------------------------------------

	void execSchedOrPayMenu ( const QPoint& pos, const bool b_sched_menu );
	void execMenuMainCal ();
//--------------------------------------------PAY-----------------------------------------------------------
	void on_btnPayInfoDel_clicked ();
	void on_btnPayInfoEdit_clicked ();
	void on_btnPayInfoSave_clicked ();
	void on_btnPayInfoCancel_clicked ();
//--------------------------------------------PAY-----------------------------------------------------------

//--------------------------------------------BUY-----------------------------------------------------------
	void on_btnBuyAdd_clicked ();
	void on_btnBuyEdit_clicked ();
	void on_btnBuyDel_clicked ();
	void on_btnBuySave_clicked ();
	void on_btnBuyCancel_clicked ();
	void on_btnBuyCopyRows_clicked ();
//--------------------------------------------BUY-----------------------------------------------------------

//-----------------------------------------------DATE-BTNS------------------------------------------------------
	void updateProgramDate ();
//----------------------------------------------CURRENT-DATE-BTNS------------------------------------------------------

//--------------------------------------------SLOTS-----------------------------------------------------------
	void exitRequested ( const bool user_requested = false );
	void quickProjectClosed ();
	void on_btnQuickProject_clicked ();
	void receiveWidgetBack ( QWidget* );

	void btnReportGenerator_clicked ();
    void btnBackupRestore_clicked ();
	void btnCalculator_clicked ();
	void btnServicesPrices_clicked ();
	void btnEstimates_clicked ();
	void btnCompanyPurchases_clicked ();
	void btnConfiguration_clicked ();
	void btnExitProgram_clicked ();
//--------------------------------------------SLOTS-----------------------------------------------------------

//--------------------------------------------SEARCH-----------------------------------------------------------
	void on_txtQuickSearch_textEdited ( const QString& text );
	void on_btnSearchStart_clicked ();
	void on_btnSearchCancel_clicked ();
//--------------------------------------------SEARCH-----------------------------------------------------------

//--------------------------------------------TRAY-----------------------------------------------------------
	void trayMenuTriggered ( QAction* );
	void trayActivated ( QSystemTrayIcon::ActivationReason );
//--------------------------------------------TRAY-----------------------------------------------------------
};

#endif // MAINWINDOW_H
