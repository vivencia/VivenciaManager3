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
class QTimer;
class QTabWidget;

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{

friend class searchUI;

public:

	enum TAB_INDEXES { TI_MAIN = 0, TI_CALENDAR, TI_INVENTORY, TI_SUPPLIES };

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

	explicit MainWindow ();
	virtual ~MainWindow ();
	
	inline Ui::MainWindow* UserInterface () const { return ui; }
	static void initMainWindow ()
	{
		if ( s_instance == nullptr )
			s_instance = new MainWindow;
	}
	
	void continueStartUp ();
	void exitRequested ( const bool user_requested = false );

//--------------------------------------------CLIENT------------------------------------------------------------
	void saveClientWidget ( vmWidget* widget, const int id );
	void showClientSearchResult ( vmListItem* item, const bool bshow );
	void setupClientPanel ();
	void clientKeyPressedSelector ( const QKeyEvent* ke );
	void clientsListWidget_currentItemChanged ( vmListItem* item );
	void controlClientForms ( const clientListItem* const client_item );
	bool saveClient ( clientListItem* client_item );
	clientListItem* addClient ();
	bool editClient ( clientListItem* client_item, const bool b_dogui = true );
	bool delClient ( clientListItem* client_item );
	bool cancelClient ( clientListItem* client_item );
	bool displayClient ( clientListItem* client_item, const bool b_select = false, jobListItem* job_item = nullptr, buyListItem* buy_item = nullptr );
	void loadClientInfo ( const Client* const client );
	clientListItem* getClientItem ( const uint id ) const;
	void fillAllLists ( const clientListItem* client_item );
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
	void saveJobWidget ( vmWidget* widget, const int id );
	void showJobSearchResult ( vmListItem* item, const bool bshow );
	void setupJobPanel ();
	void setUpJobButtons ( const QString& path );
	void setupJobPictureControl ();
	void displayJobFromCalendar ( vmListItem* cal_item );
	void jobKeyPressedSelector ( const QKeyEvent* ke );
	void jobsListWidget_currentItemChanged ( vmListItem* item );
	void jobDayReportListWidget_currentItemChanged ( vmListItem* item );
	void controlJobForms ( const jobListItem* const job_item );
	void controlJobDayForms ( const jobListItem* const job_item );
	void controlJobPictureControls (); //must be called from within loadJobInfo or after because it relies on having read the database for info
	bool saveJob ( jobListItem* job_item );
	jobListItem* addJob ( clientListItem* parent_client );
	bool editJob ( jobListItem* job_item, const bool b_dogui = true );
	bool delJob ( jobListItem* job_item );
	bool cancelJob ( jobListItem* job_item );
	void displayJob ( jobListItem* job_item, const bool b_select = false, buyListItem* buy_item = nullptr );
	void loadJobInfo ( const Job* const job );
	jobListItem* getJobItem ( const clientListItem* const parent_client, const uint id ) const;
	void scanJobImages ();
	void decodeJobReportInfo ( const Job* const job );
	void fixJobDaysList ( jobListItem* const job_item );
	void revertDayItem ( vmListItem* day_item );
	void updateJobInfo ( const QString& text, const uint user_role, vmListItem* const item = nullptr );
	void addJobPayment ( jobListItem* const job_item );
	void saveJobPayment ( jobListItem* const job_item );
	void removeJobPayment ( payListItem* pay_item );
	void alterJobPayPrice ( jobListItem* const job_item );
	void calcJobTime ();
	void showJobTotalTime ( const vmNumber& time ) const;
	triStateType dateIsInDaysList ( const QString& str_date );
	void rescanJobDaysList ( jobListItem* const job_item );
	void fillCalendarJobsList ( const stringTable& jobids, vmListWidget* list );
	void controlFormsForJobSelection ( const bool bStartSelection );
	void selectJob ();
	void btnJobSelect_clicked ();
	void btnJobAddDay_clicked ();
	void btnJobEditDay_clicked ();
	void btnJobDelDay_clicked ();
	void btnJobCancelDelDay_clicked ();
	void btnJobPrevDay_clicked ();
	void btnJobNextDay_clicked ();
	void btnJobMachines_clicked ();
	void jobOpenProjectFile ( QAction* action );
	void jobEMailProjectFile ( QAction* action );

	inline void setTempCallbackForJobSelect ( const std::function<void ( const uint )>& func ) { selJob_callback = func; }
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
	void savePayWidget ( vmWidget* widget, const int id );
	void showPaySearchResult ( vmListItem* item, const bool bshow );
	void setupPayPanel ();
	void displayPayFromCalendar ( vmListItem* cal_item );
	void paysListWidget_currentItemChanged ( vmListItem* item );
	void paysOverdueListWidget_currentItemChanged ( vmListItem* item );
	void paysOverdueClientListWidget_currentItemChanged ( vmListItem* item );
	void addPaymentOverdueItems ( payListItem* pay_item );
	void removePaymentOverdueItems ( payListItem* pay_item );
	void updateTotalPayValue (const vmNumber& nAdd, const vmNumber& nSub );
	void updatePayOverdueTotals ( const vmNumber& nAdd, const vmNumber& nSub );
	void payOverdueGUIActions ( Payment* const pay, const RECORD_ACTION new_action );
	void controlPayForms ( const payListItem* const pay_item );
	bool savePay ( payListItem* pay_item );
	bool editPay ( payListItem* pay_item, const bool b_dogui = true );
	bool delPay ( payListItem* pay_item );
	bool cancelPay ( payListItem* pay_item );
	void displayPay ( payListItem* pay_item, const bool b_select = false );
	void loadPayInfo ( const Payment* const pay );
	payListItem* getPayItem ( const clientListItem* const parent_client, const uint id ) const;
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
	void chkPayOverdue_toggled ( const bool checked );
//------------------------------------EDITING-FINISHED-PAY-----------------------------------------------------------

//--------------------------------------------BUY------------------------------------------------------------
	void saveBuyWidget ( vmWidget* widget, const int id );
	void showBuySearchResult ( vmListItem* item, const bool bshow );
	void setupBuyPanel ();
	void displayBuyFromCalendar ( vmListItem* cal_item );
	void buysListWidget_currentItemChanged ( vmListItem* item );
	void buysJobListWidget_currentItemChanged ( vmListItem* item );
	void buySuppliersListWidget_currentItemChanged ( vmListItem* item );
	void updateBuyTotalPriceWidgets ( const buyListItem* const buy_item );
	void controlBuyForms ( const buyListItem* const buy_item );
	bool saveBuy ( buyListItem* buy_item );
	buyListItem* addBuy ( clientListItem* client_parent, jobListItem* job_parent );
	bool editBuy ( buyListItem* buy_item, const bool b_dogui = true );
	bool delBuy ( buyListItem* buy_item );
	bool cancelBuy ( buyListItem* buy_item );
	void displayBuy ( buyListItem* buy_item, const bool b_select = false );
	void loadBuyInfo ( const Buy* const buy );
	buyListItem* getBuyItem ( const clientListItem* const parent_client, const uint id ) const;
	void fillJobBuyList ( const jobListItem* parent_job );
	void fillCalendarBuysList ( const stringTable& buyids, vmListWidget* list, const bool pay_date = false );
	void interceptBuyItemsCellChange ( const vmTableItem* const item );
	void interceptBuyPaymentCellChange ( const vmTableItem* const item );
	void updateBuyTotalPaidValue ();
	void buyKeyPressedSelector ( const QKeyEvent* ke );
	void getPurchasesForSuppliers ( const QString& supplier );
	void setBuyPayValueToBuyPrice ( const QString& price );
//--------------------------------------------BUY------------------------------------------------------------

//--------------------------------------------EDITING-FINISHED-BUY-----------------------------------------------------------
	void txtBuy_textAltered ( const vmWidget* const sender );
	void dteBuy_dateAltered ( const vmWidget* const sender );
	void cboBuySuppliers_textAltered ( const QString& text );
	void cboBuySuppliers_indexChanged ( const int idx );
//--------------------------------------------EDITING-FINISHED-BUY-----------------------------------------------------------

//----------------------------------SETUP-CUSTOM-CONTROLS-NAVIGATION--------------------------------------
	void setupCustomControls ();
	void restoreCrashedItems ( const crashRestore* crash );
	void restoreLastSession ();
	void searchCallbackSelector ( const QKeyEvent* ke );
	void reOrderTabSequence ();
	void setupWorkFlow ();
	void setupTabNavigationButtons ();
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
	void saveView ();
	void selectBuysItems ( const PROCESS_STEPS step );
	void navigatePrev ();
	void navigateNext ();
	void insertNavItem ( vmListItem* item );
	void displayNav ();
	void insertEditItem ( vmListItem* item );
	void removeEditItem ( vmListItem* item );
	void saveEditItems ();
//--------------------------------------------TRAY-IMPORT-EXPORT---------------------------------
	void createTrayIcon ( const bool b_setup = true );
	inline QSystemTrayIcon* appTrayIcon () const { return trayIcon; }
//--------------------------------------------TRAY-IMPORT-EXPORT---------------------------------

//--------------------------------------------SHARED-----------------------------------------------------------
	void removeListItem ( vmListItem* item, const bool b_delete_item = true, const bool b_auto_select_another = true );
	void postFieldAlterationActions ( vmListItem* item );
	inline clientListItem* currentClient () const { return mClientCurItem; }
	inline jobListItem* currentJob () const { return mJobCurItem; }
	inline payListItem* currentPay () const { return mPayCurItem; }
	inline buyListItem* currentBuy () const { return mBuyCurItem; }
	
//--------------------------------------------SHARED-----------------------------------------------------------

protected:
	void closeEvent ( QCloseEvent * );
	void changeEvent ( QEvent* e );
	bool eventFilter ( QObject* o, QEvent* e );

private:
	static MainWindow* s_instance;
	friend MainWindow* MAINWINDOW ();
	friend void deleteMainWindowInstance ();
	
	Ui::MainWindow* ui;

	QSystemTrayIcon* trayIcon;
	QMenu* trayIconMenu;

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

	crashRestore* m_crash;
	dbCalendar* mCal;

	QAction* jobsPicturesMenuAction;
	vmAction* actCountDayHours, *actCountAllHours, *actAppendDayToday, *actAppendDayYesterday;

	vmTaskPanel* mainTaskPanel;

	vmActionGroup* grpClients;
	vmActionGroup* grpJobs;
	vmActionGroup* grpPays;
	vmActionGroup* grpBuys;
	
	std::function<void ( const uint )> selJob_callback;
	bool mb_jobPosActions;

	vmNumber mCalendarDate;
	static QLatin1String lstJobReportItemPrefix;
	PointersList<vmWidget*> clientWidgetList;
	PointersList<vmWidget*> jobWidgetList;
	PointersList<vmWidget*> payWidgetList;
	PointersList<vmWidget*> buyWidgetList;
	PointersList<vmListItem*> navItems;
	PointersList<vmListItem*> editItems;

	clientListItem* mClientCurItem;
	jobListItem* mJobCurItem;
	payListItem* mPayCurItem;
	buyListItem* mBuyCurItem;

	QToolButton* mBtnNavPrev, *mBtnNavNext;
//---------------------------CUSTOM-WIDGETS-------------------------------------

//--------------------------------------------JOB-----------------------------------------------------------
	void addJobPictures ();
	void btnJobReloadPictures_clicked ();
	void showClientsYearPictures ( QAction* action );
	void showDayPictures ( const vmNumber& date );
	void showJobImage ( const int index );
	void showJobImageRequested ( const int index );
	void btnJobRenamePicture_clicked ( const bool checked );
	void showJobImageInWindow ( const bool maximized );
	void btnJobSeparateReportWindow_clicked ( const bool checked );
//--------------------------------------------JOB-----------------------------------------------------------

//--------------------------------------------BUY-----------------------------------------------------------
	void on_btnBuyCopyRows_clicked ();
//--------------------------------------------BUY-----------------------------------------------------------

//-----------------------------------------------DATE-BTNS--------------------------------------------------
	void updateProgramDate ();
//----------------------------------------------CURRENT-DATE-BTNS-------------------------------------------

//--------------------------------------------SLOTS---------------------------------------------------------
	void quickProjectClosed ();
	void on_btnQuickProject_clicked ();
	void receiveWidgetBack ( QWidget* );

	void btnReportGenerator_clicked ();
	void btnBackupRestore_clicked ();
	void btnCalculator_clicked ();
	void btnEstimates_clicked ();
	void btnCompanyPurchases_clicked ();
	void btnConfiguration_clicked ();
	void btnExitProgram_clicked ();
//--------------------------------------------SLOTS-----------------------------------------------------------

//--------------------------------------------SEARCH-----------------------------------------------------------
	void on_txtSearch_textEdited ( const QString& text );
	void on_btnSearchStart_clicked ();
	void on_btnSearchCancel_clicked ();
//--------------------------------------------SEARCH-----------------------------------------------------------

//--------------------------------------------TRAY-----------------------------------------------------------
	void trayMenuTriggered ( QAction* );
	void trayActivated ( QSystemTrayIcon::ActivationReason );
//--------------------------------------------TRAY-----------------------------------------------------------
};

inline MainWindow* MAINWINDOW ()
{
	return MainWindow::s_instance;
}

#endif // MAINWINDOW_H
