/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCalendarWidget>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QToolBox>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <texteditwithcompleter.h>
#include <vmtablewidget.h>
#include <vmwidgets.h>
#include "contactsmanagerwidget.h"
#include "db_image.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QGridLayout *gridLayout;
    QHBoxLayout *horizontalLayout_7;
    QToolButton *btnReportGenerator;
    QToolButton *btnBackupRestore;
    QToolButton *btnCalculator;
    QToolButton *btnServicesPrices;
    QToolButton *btnEstimates;
    QToolButton *btnCompanyPurchases;
    QToolButton *btnConfiguration;
    QToolButton *btnExitProgram;
    QFrame *line;
    QSplitter *splitterMain;
    QWidget *wdgLeftContainer;
    QVBoxLayout *verticalLayout_2;
    QSplitter *splitterClient;
    QScrollArea *scrollLeftPanel;
    QWidget *scrollAreaWidgetContents;
    QTabWidget *tabMain;
    QWidget *tabWorkFlow;
    QHBoxLayout *horizontalLayout_5;
    QScrollArea *scrollWorkFlow;
    QWidget *scrollAreaWidgetContents_2;
    QWidget *tabCalendar;
    QVBoxLayout *verticalLayout_11;
    QVBoxLayout *verticalLayout_10;
    QSplitter *splitterCalendarTab;
    QFrame *frmCalJobs;
    QHBoxLayout *horizontalLayout_4;
    QCalendarWidget *calMain;
    QFrame *frame;
    QHBoxLayout *horizontalLayout_3;
    QSplitter *splitterCalLists;
    QFrame *frmBuyJobList_2;
    QVBoxLayout *verticalLayout_15;
    QToolBox *tboxCalJobs;
    QWidget *tboxPageJobDay;
    QVBoxLayout *verticalLayout_6;
    QLabel *lblCalPriceJobDay;
    vmLineEdit *txtCalPriceJobDay;
    vmListWidget *lstCalJobsDay;
    QWidget *tboxPageJobWeek;
    QVBoxLayout *verticalLayout_21;
    QLabel *lblCalPriceJobWeek;
    vmLineEdit *txtCalPriceJobWeek;
    vmListWidget *lstCalJobsWeek;
    QWidget *tboxPageJobMonth;
    QVBoxLayout *verticalLayout_22;
    QLabel *lblCalPriceJobMonth;
    vmLineEdit *txtCalPriceJobMonth;
    vmListWidget *lstCalJobsMonth;
    QToolBox *tboxCalPays;
    QWidget *tboxPagePayDay;
    QVBoxLayout *verticalLayout_5;
    QLabel *lblCalPricePayDay;
    vmLineEdit *txtCalPricePayDay;
    vmListWidget *lstCalPaysDay;
    QWidget *tboxPagePayWeek;
    QVBoxLayout *verticalLayout_17;
    QLabel *lblCalPricePayWeek;
    vmLineEdit *txtCalPricePayWeek;
    vmListWidget *lstCalPaysWeek;
    QWidget *tboxPagePayMonth;
    QVBoxLayout *verticalLayout_18;
    QLabel *lblCalPricePayMonth;
    vmLineEdit *txtCalPricePayMonth;
    vmListWidget *lstCalPaysMonth;
    QToolBox *tboxCalBuys;
    QWidget *tboxPageBuyDay;
    QVBoxLayout *verticalLayout_8;
    QLabel *lblCalPriceBuyDay;
    vmLineEdit *txtCalPriceBuyDay;
    vmListWidget *lstCalBuysDay;
    QWidget *tboxPageBuyWeek;
    QVBoxLayout *verticalLayout_23;
    QLabel *lblCalPriceBuyWeek;
    vmLineEdit *txtCalPriceBuyWeek;
    vmListWidget *lstCalBuysWeek;
    QWidget *tboxPageBuyMonth;
    QVBoxLayout *verticalLayout_24;
    QLabel *lblCalPriceBuyMonth;
    vmLineEdit *txtCalPriceBuyMonth;
    vmListWidget *lstCalBuysMonth;
    QWidget *tabInventory;
    QWidget *tabSupplies;
    QWidget *tabDesign_1;
    QVBoxLayout *verticalLayout_3;
    QFrame *frmPayInfo;
    QGridLayout *gLayoutPayForms;
    vmLineEdit *txtPayTotalPrice;
    QFrame *frmPayToolbar;
    QVBoxLayout *verticalLayout_14;
    QToolButton *btnPayInfoEdit;
    QToolButton *btnPayInfoDel;
    QToolButton *btnPayInfoSave;
    QToolButton *btnPayInfoCancel;
    QToolButton *btnPayPayReceipt;
    QToolButton *btnPayPaymentsReportOnlyUnPaid;
    QToolButton *btnPayPaymentsReport;
    QSpacerItem *verticalSpacer_2;
    vmLineEdit *txtPayID;
    QLabel *lblTotalPayments;
    vmLineEdit *txtPayObs;
    QLabel *lblPayObs;
    vmTableWidget *tablePayments;
    vmLineEdit *txtPayTotalPaid;
    QLabel *lblPayID;
    vmCheckBox *chkPayOverdue;
    QFrame *frmClientToolbar;
    QVBoxLayout *verticalLayout_20;
    QToolButton *btnClientAdd;
    QToolButton *btnClientEdit;
    QToolButton *btnClientDel;
    QToolButton *btnClientSave;
    QToolButton *btnClientCancel;
    QSpacerItem *verticalSpacer_4;
    vmLineEdit *txtPayTotalPayments;
    QLabel *lblPayTotalPaid;
    QWidget *widget;
    QLabel *lblTotalPay;
    QFrame *frmClientInfo;
    QGridLayout *gridLayout_3;
    vmLineEdit *txtClientZipCode;
    vmLineEdit *txtClientCity;
    QLabel *lblClientID;
    vmLineEdit *txtClientName;
    QLabel *lblClientPhones;
    vmLineEdit *txtClientDistrict;
    vmLineEdit *txtClientID;
    QLabel *lblClientCity;
    QFrame *frame_2;
    QVBoxLayout *verticalLayout_4;
    QLabel *lblClientDateFrom;
    vmDateEdit *dteClientDateFrom;
    QLabel *lblClientDateTo;
    vmDateEdit *dteClientDateTo;
    vmCheckBox *chkClientActive;
    QLabel *lblClientDistrict;
    QLabel *lblClientName;
    QLabel *lblClientStreetAdress;
    QLabel *lblClientEmail;
    contactsManagerWidget *contactsClientPhones;
    vmLineEdit *txtClientStreetAddress;
    contactsManagerWidget *contactsClientEmails;
    QLabel *lblClientZipCode;
    vmLineEdit *txtClientNumberAddress;
    QLabel *lblClientNumberAdress;
    QWidget *tab;
    QHBoxLayout *horizontalLayout_2;
    QFrame *frmJobInfo;
    QVBoxLayout *verticalLayout;
    QGridLayout *gridLayout_15;
    QLabel *lblJobAddress;
    QFrame *frmJobToolbar;
    QVBoxLayout *verticalLayout_13;
    QToolButton *btnJobAdd;
    QToolButton *btnJobEdit;
    QToolButton *btnJobDel;
    QToolButton *btnJobSave;
    QToolButton *btnJobCancel;
    QSpacerItem *verticalSpacer;
    QToolButton *btnJobSelectJob;
    QLabel *lblJobProjectID;
    vmLineEdit *txtJobProjectPath;
    QToolButton *btnNewProject;
    QLabel *lblJobType;
    QToolButton *btnJobOpenDoc;
    QLabel *lblJobID;
    vmComboBox *cboJobType;
    QToolButton *btnJobOpenXls;
    QSpacerItem *horizontalSpacer_3;
    QToolButton *btnJobOpenFileFolder;
    QLabel *lblJobFilepath;
    QPushButton *btnQuickProject;
    QToolButton *btnJobOpenPdf;
    vmLineEdit *txtJobProjectID;
    vmLineEdit *txtJobID;
    vmLineEdit *txtJobAddress;
    QWidget *frmJobInfo2;
    QGridLayout *gridLayout_6;
    QLabel *lblJobDateStart;
    vmLineEdit *txtJobPrice;
    vmDateEdit *dteJobEnd;
    vmDateEdit *dteJobStart;
    QLabel *lblJobPrice;
    QLabel *lblJobDateEnd;
    QSpacerItem *horizontalSpacer_6;
    QWidget *tabDesign3;
    QVBoxLayout *verticalLayout_27;
    QFrame *frmJobInfo_2;
    QGridLayout *gLayoutJobExtraInfo;
    QGroupBox *grpJobTotalTime;
    QGridLayout *gridLayout_4;
    QLabel *lblJobDayTime;
    QLabel *lblJobAllDaysTime;
    vmLineEdit *txtJobTotalAllDaysTime;
    vmLineEdit *txtJobTotalDayTime;
    QLabel *lblJobReport;
    QLabel *lblJobPictures;
    QLabel *lblJobEndTime;
    vmTimeEdit *timeJobEnd;
    QFrame *frmJobReportControls;
    QHBoxLayout *horizontalLayout_13;
    QToolButton *btnJobPrevDay;
    QToolButton *btnJobNextDay;
    QFrame *line_17;
    QToolButton *btnJobAddDay;
    vmDateEdit *dteJobAddDate;
    QFrame *line_16;
    QToolButton *btnJobDelDay;
    QToolButton *btnJobCancelDelDay;
    textEditWithCompleter *txtJobReport;
    vmTimeEdit *timeJobStart;
    QToolButton *btnJobSeparateReportWindow;
    QFrame *frmJobPicturesControls;
    QHBoxLayout *horizontalLayout_23;
    QLabel *lblJobPicturesCount;
    vmComboBox *cboJobPictures;
    QToolButton *btnJobPrevPicture;
    QToolButton *btnJobNextPicture;
    QToolButton *btnJobReloadPictures;
    QToolButton *btnJobRenamePicture;
    QFrame *line_19;
    QToolButton *btnJobOpenPictureEditor;
    QToolButton *btnJobOpenPictureFolder;
    QToolButton *btnJobOpenPictureViewer;
    QFrame *line_21;
    QToolButton *btnJobClientsYearPictures;
    QFrame *line_18;
    QSpacerItem *horizontalSpacer;
    QToolButton *btnJobSeparatePicture;
    QHBoxLayout *hLayoutImgViewer;
    QSpacerItem *horizontalSpacer_5;
    DB_Image *jobImageViewer;
    QSpacerItem *horizontalSpacer_4;
    QLabel *lblJobStartTime;
    vmLineEdit *txtJobWheather;
    QLabel *lblWeatherConditions;
    vmListWidget *lstJobDayReport;
    QToolButton *btnJobMachines;
    vmLineEdit *txtJobPicturesPath;
    QWidget *tabDesign_4;
    QSplitter *splitterBuyInfo;
    QFrame *frmBuyJobList;
    QVBoxLayout *verticalLayout_19;
    QVBoxLayout *verticalLayout_12;
    QHBoxLayout *horizontalLayout_6;
    QFrame *frmBuyToolbar;
    QVBoxLayout *verticalLayout_16;
    QToolButton *btnBuyAdd;
    QToolButton *btnBuyEdit;
    QToolButton *btnBuyDel;
    QToolButton *btnBuySave;
    QToolButton *btnBuyCancel;
    QSpacerItem *verticalSpacer_3;
    QLabel *lblBuySupplier_2;
    vmComboBox *cboBuySuppliers;
    QToolButton *btnShowSuppliersDlg;
    vmListWidget *buysJobListWidget;
    QLabel *lblBuySupplierBuys;
    vmListWidget *lstBuySuppliers;
    QFrame *frmBuyInfo;
    QHBoxLayout *horizontalLayout;
    QGridLayout *gLayoutBuyForms;
    vmLineEdit *txtBuyTotalPrice;
    QToolButton *btnBuyCopyRows;
    QLabel *lblBuyID_2;
    vmLineEdit *txtBuyNotes;
    vmDateEdit *dteBuyDate;
    vmLineEdit *txtBuyDeliveryMethod;
    vmDateEdit *dteBuyDeliveryDate;
    QLabel *lblBuyPayments;
    vmLineEdit *txtBuyID;
    vmTableWidget *tableBuyItems;
    QLabel *lblBuyItems;
    QLabel *lblBuyDeliverDate;
    QLabel *lblBuyNotes_2;
    vmTableWidget *tableBuyPayments;
    QWidget *widget_3;
    QWidget *widget_2;
    QLabel *lblBuyTotalPrice;
    QLabel *lblBuyTotalPaid;
    QLabel *lblBuyDeliverMethod_2;
    vmLineEditWithButton *txtBuyTotalPaid;
    QLabel *lblBuyDate_2;
    QHBoxLayout *horizontalLayout_8;
    QLabel *lblCurInfoClient;
    QFrame *line_2;
    QLabel *lblCurInfoJob;
    QFrame *line_3;
    QLabel *lblCurInfoPay;
    QFrame *line_4;
    QLabel *lblCurInfoBuy;
    QSpacerItem *horizontalSpacer_2;
    QFrame *line_5;
    QLabel *lblFastSearch;
    vmLineEdit *txtQuickSearch;
    QToolButton *btnSearchStart;
    QToolButton *btnSearchCancel;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(890, 700);
        MainWindow->setMinimumSize(QSize(890, 700));
        MainWindow->setStyleSheet(QStringLiteral(""));
        MainWindow->setDockNestingEnabled(false);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QStringLiteral("centralwidget"));
        gridLayout = new QGridLayout(centralwidget);
        gridLayout->setSpacing(3);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        gridLayout->setContentsMargins(5, 5, 0, 0);
        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setSpacing(2);
        horizontalLayout_7->setObjectName(QStringLiteral("horizontalLayout_7"));
        horizontalLayout_7->setSizeConstraint(QLayout::SetFixedSize);
        btnReportGenerator = new QToolButton(centralwidget);
        btnReportGenerator->setObjectName(QStringLiteral("btnReportGenerator"));
        btnReportGenerator->setMinimumSize(QSize(30, 30));
        QIcon icon;
        icon.addFile(QStringLiteral(":/resources/report.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnReportGenerator->setIcon(icon);

        horizontalLayout_7->addWidget(btnReportGenerator);

        btnBackupRestore = new QToolButton(centralwidget);
        btnBackupRestore->setObjectName(QStringLiteral("btnBackupRestore"));
        btnBackupRestore->setMinimumSize(QSize(30, 30));
        QIcon icon1;
        icon1.addFile(QStringLiteral(":/resources/db-import-export.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnBackupRestore->setIcon(icon1);
        btnBackupRestore->setCheckable(false);
        btnBackupRestore->setPopupMode(QToolButton::DelayedPopup);

        horizontalLayout_7->addWidget(btnBackupRestore);

        btnCalculator = new QToolButton(centralwidget);
        btnCalculator->setObjectName(QStringLiteral("btnCalculator"));
        btnCalculator->setMinimumSize(QSize(30, 30));
        QIcon icon2;
        icon2.addFile(QStringLiteral(":/resources/calc.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnCalculator->setIcon(icon2);
        btnCalculator->setCheckable(false);
        btnCalculator->setPopupMode(QToolButton::DelayedPopup);

        horizontalLayout_7->addWidget(btnCalculator);

        btnServicesPrices = new QToolButton(centralwidget);
        btnServicesPrices->setObjectName(QStringLiteral("btnServicesPrices"));
        btnServicesPrices->setMinimumSize(QSize(30, 30));
        btnServicesPrices->setMaximumSize(QSize(30, 30));
        QIcon icon3;
        icon3.addFile(QStringLiteral(":/resources/table-prices.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnServicesPrices->setIcon(icon3);

        horizontalLayout_7->addWidget(btnServicesPrices);

        btnEstimates = new QToolButton(centralwidget);
        btnEstimates->setObjectName(QStringLiteral("btnEstimates"));
        btnEstimates->setMinimumSize(QSize(30, 30));
        QIcon icon4;
        icon4.addFile(QStringLiteral(":/resources/estimatives.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnEstimates->setIcon(icon4);
        btnEstimates->setCheckable(true);

        horizontalLayout_7->addWidget(btnEstimates);

        btnCompanyPurchases = new QToolButton(centralwidget);
        btnCompanyPurchases->setObjectName(QStringLiteral("btnCompanyPurchases"));
        btnCompanyPurchases->setMinimumSize(QSize(30, 30));
        QIcon icon5;
        icon5.addFile(QStringLiteral(":/resources/companypurchases.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnCompanyPurchases->setIcon(icon5);
        btnCompanyPurchases->setCheckable(true);

        horizontalLayout_7->addWidget(btnCompanyPurchases);

        btnConfiguration = new QToolButton(centralwidget);
        btnConfiguration->setObjectName(QStringLiteral("btnConfiguration"));
        btnConfiguration->setMinimumSize(QSize(30, 30));
        QIcon icon6;
        icon6.addFile(QStringLiteral(":/resources/configure.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnConfiguration->setIcon(icon6);

        horizontalLayout_7->addWidget(btnConfiguration);

        btnExitProgram = new QToolButton(centralwidget);
        btnExitProgram->setObjectName(QStringLiteral("btnExitProgram"));
        btnExitProgram->setMinimumSize(QSize(30, 30));
        QIcon icon7;
        icon7.addFile(QStringLiteral(":/resources/exit_app.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnExitProgram->setIcon(icon7);

        horizontalLayout_7->addWidget(btnExitProgram);

        line = new QFrame(centralwidget);
        line->setObjectName(QStringLiteral("line"));
        line->setFrameShape(QFrame::VLine);
        line->setFrameShadow(QFrame::Sunken);

        horizontalLayout_7->addWidget(line);


        gridLayout->addLayout(horizontalLayout_7, 0, 0, 1, 1);

        splitterMain = new QSplitter(centralwidget);
        splitterMain->setObjectName(QStringLiteral("splitterMain"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(splitterMain->sizePolicy().hasHeightForWidth());
        splitterMain->setSizePolicy(sizePolicy);
        splitterMain->setMinimumSize(QSize(500, 0));
        splitterMain->setMaximumSize(QSize(16777215, 16777215));
        splitterMain->setFrameShape(QFrame::NoFrame);
        splitterMain->setFrameShadow(QFrame::Plain);
        splitterMain->setLineWidth(5);
        splitterMain->setOrientation(Qt::Horizontal);
        wdgLeftContainer = new QWidget(splitterMain);
        wdgLeftContainer->setObjectName(QStringLiteral("wdgLeftContainer"));
        wdgLeftContainer->setMaximumSize(QSize(16777215, 16777215));
        verticalLayout_2 = new QVBoxLayout(wdgLeftContainer);
        verticalLayout_2->setSpacing(0);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        splitterClient = new QSplitter(wdgLeftContainer);
        splitterClient->setObjectName(QStringLiteral("splitterClient"));
        sizePolicy.setHeightForWidth(splitterClient->sizePolicy().hasHeightForWidth());
        splitterClient->setSizePolicy(sizePolicy);
        splitterClient->setMinimumSize(QSize(200, 0));
        splitterClient->setMaximumSize(QSize(16777215, 16777215));
        splitterClient->setFrameShape(QFrame::NoFrame);
        splitterClient->setFrameShadow(QFrame::Plain);
        splitterClient->setLineWidth(5);
        splitterClient->setOrientation(Qt::Vertical);
        splitterClient->setHandleWidth(5);
        scrollLeftPanel = new QScrollArea(splitterClient);
        scrollLeftPanel->setObjectName(QStringLiteral("scrollLeftPanel"));
        scrollLeftPanel->setMinimumSize(QSize(240, 0));
        scrollLeftPanel->setFrameShape(QFrame::NoFrame);
        scrollLeftPanel->setFrameShadow(QFrame::Plain);
        scrollLeftPanel->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scrollLeftPanel->setSizeAdjustPolicy(QAbstractScrollArea::AdjustIgnored);
        scrollLeftPanel->setWidgetResizable(true);
        scrollLeftPanel->setAlignment(Qt::AlignJustify|Qt::AlignTop);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName(QStringLiteral("scrollAreaWidgetContents"));
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 240, 629));
        scrollLeftPanel->setWidget(scrollAreaWidgetContents);
        splitterClient->addWidget(scrollLeftPanel);

        verticalLayout_2->addWidget(splitterClient);

        splitterMain->addWidget(wdgLeftContainer);
        tabMain = new QTabWidget(splitterMain);
        tabMain->setObjectName(QStringLiteral("tabMain"));
        tabMain->setEnabled(true);
        tabMain->setMinimumSize(QSize(645, 0));
        tabMain->setUsesScrollButtons(true);
        tabMain->setDocumentMode(true);
        tabWorkFlow = new QWidget();
        tabWorkFlow->setObjectName(QStringLiteral("tabWorkFlow"));
        horizontalLayout_5 = new QHBoxLayout(tabWorkFlow);
        horizontalLayout_5->setSpacing(0);
        horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
        horizontalLayout_5->setContentsMargins(0, 0, 0, 0);
        scrollWorkFlow = new QScrollArea(tabWorkFlow);
        scrollWorkFlow->setObjectName(QStringLiteral("scrollWorkFlow"));
        scrollWorkFlow->setFrameShape(QFrame::NoFrame);
        scrollWorkFlow->setFrameShadow(QFrame::Plain);
        scrollWorkFlow->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scrollWorkFlow->setSizeAdjustPolicy(QAbstractScrollArea::AdjustIgnored);
        scrollWorkFlow->setWidgetResizable(true);
        scrollWorkFlow->setAlignment(Qt::AlignJustify|Qt::AlignVCenter);
        scrollAreaWidgetContents_2 = new QWidget();
        scrollAreaWidgetContents_2->setObjectName(QStringLiteral("scrollAreaWidgetContents_2"));
        scrollAreaWidgetContents_2->setGeometry(QRect(0, 0, 100, 30));
        scrollWorkFlow->setWidget(scrollAreaWidgetContents_2);

        horizontalLayout_5->addWidget(scrollWorkFlow);

        tabMain->addTab(tabWorkFlow, QString());
        tabCalendar = new QWidget();
        tabCalendar->setObjectName(QStringLiteral("tabCalendar"));
        verticalLayout_11 = new QVBoxLayout(tabCalendar);
        verticalLayout_11->setObjectName(QStringLiteral("verticalLayout_11"));
        verticalLayout_10 = new QVBoxLayout();
        verticalLayout_10->setObjectName(QStringLiteral("verticalLayout_10"));
        splitterCalendarTab = new QSplitter(tabCalendar);
        splitterCalendarTab->setObjectName(QStringLiteral("splitterCalendarTab"));
        sizePolicy.setHeightForWidth(splitterCalendarTab->sizePolicy().hasHeightForWidth());
        splitterCalendarTab->setSizePolicy(sizePolicy);
        splitterCalendarTab->setMinimumSize(QSize(200, 0));
        splitterCalendarTab->setMaximumSize(QSize(16777215, 16777215));
        splitterCalendarTab->setFrameShape(QFrame::NoFrame);
        splitterCalendarTab->setFrameShadow(QFrame::Plain);
        splitterCalendarTab->setLineWidth(5);
        splitterCalendarTab->setOrientation(Qt::Vertical);
        splitterCalendarTab->setHandleWidth(5);
        frmCalJobs = new QFrame(splitterCalendarTab);
        frmCalJobs->setObjectName(QStringLiteral("frmCalJobs"));
        frmCalJobs->setFrameShape(QFrame::NoFrame);
        frmCalJobs->setFrameShadow(QFrame::Plain);
        frmCalJobs->setLineWidth(3);
        horizontalLayout_4 = new QHBoxLayout(frmCalJobs);
        horizontalLayout_4->setSpacing(5);
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        horizontalLayout_4->setContentsMargins(2, 2, 2, 2);
        calMain = new QCalendarWidget(frmCalJobs);
        calMain->setObjectName(QStringLiteral("calMain"));
        calMain->setGridVisible(true);

        horizontalLayout_4->addWidget(calMain);

        splitterCalendarTab->addWidget(frmCalJobs);
        frame = new QFrame(splitterCalendarTab);
        frame->setObjectName(QStringLiteral("frame"));
        frame->setMinimumSize(QSize(0, 300));
        frame->setFrameShape(QFrame::NoFrame);
        frame->setFrameShadow(QFrame::Plain);
        horizontalLayout_3 = new QHBoxLayout(frame);
        horizontalLayout_3->setSpacing(5);
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        horizontalLayout_3->setContentsMargins(5, 5, 5, 5);
        splitterCalLists = new QSplitter(frame);
        splitterCalLists->setObjectName(QStringLiteral("splitterCalLists"));
        sizePolicy.setHeightForWidth(splitterCalLists->sizePolicy().hasHeightForWidth());
        splitterCalLists->setSizePolicy(sizePolicy);
        splitterCalLists->setMinimumSize(QSize(500, 0));
        splitterCalLists->setMaximumSize(QSize(16777215, 16777215));
        splitterCalLists->setFrameShape(QFrame::NoFrame);
        splitterCalLists->setFrameShadow(QFrame::Plain);
        splitterCalLists->setLineWidth(5);
        splitterCalLists->setOrientation(Qt::Horizontal);
        frmBuyJobList_2 = new QFrame(splitterCalLists);
        frmBuyJobList_2->setObjectName(QStringLiteral("frmBuyJobList_2"));
        frmBuyJobList_2->setMinimumSize(QSize(100, 100));
        frmBuyJobList_2->setFrameShape(QFrame::NoFrame);
        frmBuyJobList_2->setFrameShadow(QFrame::Plain);
        verticalLayout_15 = new QVBoxLayout(frmBuyJobList_2);
        verticalLayout_15->setSpacing(2);
        verticalLayout_15->setObjectName(QStringLiteral("verticalLayout_15"));
        verticalLayout_15->setContentsMargins(0, 0, 0, 0);
        tboxCalJobs = new QToolBox(frmBuyJobList_2);
        tboxCalJobs->setObjectName(QStringLiteral("tboxCalJobs"));
        tboxCalJobs->setMinimumSize(QSize(200, 0));
        tboxCalJobs->setFrameShape(QFrame::NoFrame);
        tboxCalJobs->setFrameShadow(QFrame::Plain);
        tboxPageJobDay = new QWidget();
        tboxPageJobDay->setObjectName(QStringLiteral("tboxPageJobDay"));
        tboxPageJobDay->setGeometry(QRect(0, 0, 267, 166));
        verticalLayout_6 = new QVBoxLayout(tboxPageJobDay);
        verticalLayout_6->setObjectName(QStringLiteral("verticalLayout_6"));
        lblCalPriceJobDay = new QLabel(tboxPageJobDay);
        lblCalPriceJobDay->setObjectName(QStringLiteral("lblCalPriceJobDay"));
        QFont font;
        font.setPointSize(10);
        lblCalPriceJobDay->setFont(font);
        lblCalPriceJobDay->setFrameShape(QFrame::StyledPanel);
        lblCalPriceJobDay->setFrameShadow(QFrame::Raised);

        verticalLayout_6->addWidget(lblCalPriceJobDay);

        txtCalPriceJobDay = new vmLineEdit(tboxPageJobDay);
        txtCalPriceJobDay->setObjectName(QStringLiteral("txtCalPriceJobDay"));
        QFont font1;
        font1.setBold(true);
        font1.setWeight(75);
        txtCalPriceJobDay->setFont(font1);
        txtCalPriceJobDay->setReadOnly(true);

        verticalLayout_6->addWidget(txtCalPriceJobDay);

        lstCalJobsDay = new vmListWidget(tboxPageJobDay);
        lstCalJobsDay->setObjectName(QStringLiteral("lstCalJobsDay"));
        lstCalJobsDay->setProperty("showDropIndicator", QVariant(false));
        lstCalJobsDay->setAlternatingRowColors(true);
        lstCalJobsDay->setSortingEnabled(true);

        verticalLayout_6->addWidget(lstCalJobsDay);

        tboxCalJobs->addItem(tboxPageJobDay, QStringLiteral("Jobs"));
        tboxPageJobWeek = new QWidget();
        tboxPageJobWeek->setObjectName(QStringLiteral("tboxPageJobWeek"));
        tboxPageJobWeek->setGeometry(QRect(0, 0, 280, 166));
        verticalLayout_21 = new QVBoxLayout(tboxPageJobWeek);
        verticalLayout_21->setObjectName(QStringLiteral("verticalLayout_21"));
        lblCalPriceJobWeek = new QLabel(tboxPageJobWeek);
        lblCalPriceJobWeek->setObjectName(QStringLiteral("lblCalPriceJobWeek"));
        lblCalPriceJobWeek->setFont(font);
        lblCalPriceJobWeek->setFrameShape(QFrame::StyledPanel);
        lblCalPriceJobWeek->setFrameShadow(QFrame::Raised);
        lblCalPriceJobWeek->setScaledContents(true);

        verticalLayout_21->addWidget(lblCalPriceJobWeek);

        txtCalPriceJobWeek = new vmLineEdit(tboxPageJobWeek);
        txtCalPriceJobWeek->setObjectName(QStringLiteral("txtCalPriceJobWeek"));
        txtCalPriceJobWeek->setFont(font1);
        txtCalPriceJobWeek->setReadOnly(true);

        verticalLayout_21->addWidget(txtCalPriceJobWeek);

        lstCalJobsWeek = new vmListWidget(tboxPageJobWeek);
        lstCalJobsWeek->setObjectName(QStringLiteral("lstCalJobsWeek"));
        lstCalJobsWeek->setProperty("showDropIndicator", QVariant(false));
        lstCalJobsWeek->setAlternatingRowColors(true);
        lstCalJobsWeek->setSortingEnabled(true);

        verticalLayout_21->addWidget(lstCalJobsWeek);

        tboxCalJobs->addItem(tboxPageJobWeek, QStringLiteral("Jobs"));
        tboxPageJobMonth = new QWidget();
        tboxPageJobMonth->setObjectName(QStringLiteral("tboxPageJobMonth"));
        tboxPageJobMonth->setGeometry(QRect(0, 0, 292, 166));
        verticalLayout_22 = new QVBoxLayout(tboxPageJobMonth);
        verticalLayout_22->setObjectName(QStringLiteral("verticalLayout_22"));
        lblCalPriceJobMonth = new QLabel(tboxPageJobMonth);
        lblCalPriceJobMonth->setObjectName(QStringLiteral("lblCalPriceJobMonth"));
        lblCalPriceJobMonth->setFont(font);
        lblCalPriceJobMonth->setFrameShape(QFrame::StyledPanel);
        lblCalPriceJobMonth->setFrameShadow(QFrame::Raised);

        verticalLayout_22->addWidget(lblCalPriceJobMonth);

        txtCalPriceJobMonth = new vmLineEdit(tboxPageJobMonth);
        txtCalPriceJobMonth->setObjectName(QStringLiteral("txtCalPriceJobMonth"));
        txtCalPriceJobMonth->setFont(font1);
        txtCalPriceJobMonth->setReadOnly(true);

        verticalLayout_22->addWidget(txtCalPriceJobMonth);

        lstCalJobsMonth = new vmListWidget(tboxPageJobMonth);
        lstCalJobsMonth->setObjectName(QStringLiteral("lstCalJobsMonth"));
        lstCalJobsMonth->setProperty("showDropIndicator", QVariant(false));
        lstCalJobsMonth->setAlternatingRowColors(true);
        lstCalJobsMonth->setSortingEnabled(true);

        verticalLayout_22->addWidget(lstCalJobsMonth);

        tboxCalJobs->addItem(tboxPageJobMonth, QStringLiteral("Jobs"));

        verticalLayout_15->addWidget(tboxCalJobs);

        splitterCalLists->addWidget(frmBuyJobList_2);
        tboxCalPays = new QToolBox(splitterCalLists);
        tboxCalPays->setObjectName(QStringLiteral("tboxCalPays"));
        tboxCalPays->setMinimumSize(QSize(200, 0));
        tboxCalPays->setFrameShape(QFrame::NoFrame);
        tboxCalPays->setFrameShadow(QFrame::Plain);
        tboxPagePayDay = new QWidget();
        tboxPagePayDay->setObjectName(QStringLiteral("tboxPagePayDay"));
        tboxPagePayDay->setGeometry(QRect(0, 0, 247, 168));
        verticalLayout_5 = new QVBoxLayout(tboxPagePayDay);
        verticalLayout_5->setObjectName(QStringLiteral("verticalLayout_5"));
        lblCalPricePayDay = new QLabel(tboxPagePayDay);
        lblCalPricePayDay->setObjectName(QStringLiteral("lblCalPricePayDay"));
        lblCalPricePayDay->setFrameShape(QFrame::StyledPanel);
        lblCalPricePayDay->setFrameShadow(QFrame::Raised);

        verticalLayout_5->addWidget(lblCalPricePayDay);

        txtCalPricePayDay = new vmLineEdit(tboxPagePayDay);
        txtCalPricePayDay->setObjectName(QStringLiteral("txtCalPricePayDay"));
        txtCalPricePayDay->setFont(font1);
        txtCalPricePayDay->setReadOnly(true);

        verticalLayout_5->addWidget(txtCalPricePayDay);

        lstCalPaysDay = new vmListWidget(tboxPagePayDay);
        lstCalPaysDay->setObjectName(QStringLiteral("lstCalPaysDay"));
        lstCalPaysDay->setProperty("showDropIndicator", QVariant(false));
        lstCalPaysDay->setAlternatingRowColors(true);
        lstCalPaysDay->setSortingEnabled(true);

        verticalLayout_5->addWidget(lstCalPaysDay);

        tboxCalPays->addItem(tboxPagePayDay, QStringLiteral("Payment"));
        tboxPagePayWeek = new QWidget();
        tboxPagePayWeek->setObjectName(QStringLiteral("tboxPagePayWeek"));
        tboxPagePayWeek->setGeometry(QRect(0, 0, 261, 168));
        verticalLayout_17 = new QVBoxLayout(tboxPagePayWeek);
        verticalLayout_17->setObjectName(QStringLiteral("verticalLayout_17"));
        lblCalPricePayWeek = new QLabel(tboxPagePayWeek);
        lblCalPricePayWeek->setObjectName(QStringLiteral("lblCalPricePayWeek"));
        lblCalPricePayWeek->setFrameShape(QFrame::StyledPanel);
        lblCalPricePayWeek->setFrameShadow(QFrame::Raised);

        verticalLayout_17->addWidget(lblCalPricePayWeek);

        txtCalPricePayWeek = new vmLineEdit(tboxPagePayWeek);
        txtCalPricePayWeek->setObjectName(QStringLiteral("txtCalPricePayWeek"));
        txtCalPricePayWeek->setFont(font1);
        txtCalPricePayWeek->setReadOnly(true);

        verticalLayout_17->addWidget(txtCalPricePayWeek);

        lstCalPaysWeek = new vmListWidget(tboxPagePayWeek);
        lstCalPaysWeek->setObjectName(QStringLiteral("lstCalPaysWeek"));
        lstCalPaysWeek->setProperty("showDropIndicator", QVariant(false));
        lstCalPaysWeek->setAlternatingRowColors(true);
        lstCalPaysWeek->setSortingEnabled(true);

        verticalLayout_17->addWidget(lstCalPaysWeek);

        tboxCalPays->addItem(tboxPagePayWeek, QStringLiteral("Payment"));
        tboxPagePayMonth = new QWidget();
        tboxPagePayMonth->setObjectName(QStringLiteral("tboxPagePayMonth"));
        tboxPagePayMonth->setGeometry(QRect(0, 0, 273, 168));
        verticalLayout_18 = new QVBoxLayout(tboxPagePayMonth);
        verticalLayout_18->setObjectName(QStringLiteral("verticalLayout_18"));
        lblCalPricePayMonth = new QLabel(tboxPagePayMonth);
        lblCalPricePayMonth->setObjectName(QStringLiteral("lblCalPricePayMonth"));
        lblCalPricePayMonth->setFrameShape(QFrame::StyledPanel);
        lblCalPricePayMonth->setFrameShadow(QFrame::Raised);

        verticalLayout_18->addWidget(lblCalPricePayMonth);

        txtCalPricePayMonth = new vmLineEdit(tboxPagePayMonth);
        txtCalPricePayMonth->setObjectName(QStringLiteral("txtCalPricePayMonth"));
        txtCalPricePayMonth->setFont(font1);
        txtCalPricePayMonth->setReadOnly(true);

        verticalLayout_18->addWidget(txtCalPricePayMonth);

        lstCalPaysMonth = new vmListWidget(tboxPagePayMonth);
        lstCalPaysMonth->setObjectName(QStringLiteral("lstCalPaysMonth"));
        lstCalPaysMonth->setAlternatingRowColors(true);
        lstCalPaysMonth->setSortingEnabled(true);

        verticalLayout_18->addWidget(lstCalPaysMonth);

        tboxCalPays->addItem(tboxPagePayMonth, QStringLiteral("Payment"));
        splitterCalLists->addWidget(tboxCalPays);
        tboxCalBuys = new QToolBox(splitterCalLists);
        tboxCalBuys->setObjectName(QStringLiteral("tboxCalBuys"));
        tboxCalBuys->setMinimumSize(QSize(200, 0));
        tboxCalBuys->setFrameShape(QFrame::NoFrame);
        tboxCalBuys->setFrameShadow(QFrame::Plain);
        tboxPageBuyDay = new QWidget();
        tboxPageBuyDay->setObjectName(QStringLiteral("tboxPageBuyDay"));
        tboxPageBuyDay->setGeometry(QRect(0, 0, 214, 168));
        verticalLayout_8 = new QVBoxLayout(tboxPageBuyDay);
        verticalLayout_8->setObjectName(QStringLiteral("verticalLayout_8"));
        lblCalPriceBuyDay = new QLabel(tboxPageBuyDay);
        lblCalPriceBuyDay->setObjectName(QStringLiteral("lblCalPriceBuyDay"));
        lblCalPriceBuyDay->setFrameShape(QFrame::StyledPanel);
        lblCalPriceBuyDay->setFrameShadow(QFrame::Raised);

        verticalLayout_8->addWidget(lblCalPriceBuyDay);

        txtCalPriceBuyDay = new vmLineEdit(tboxPageBuyDay);
        txtCalPriceBuyDay->setObjectName(QStringLiteral("txtCalPriceBuyDay"));
        txtCalPriceBuyDay->setFont(font1);
        txtCalPriceBuyDay->setReadOnly(true);

        verticalLayout_8->addWidget(txtCalPriceBuyDay);

        lstCalBuysDay = new vmListWidget(tboxPageBuyDay);
        lstCalBuysDay->setObjectName(QStringLiteral("lstCalBuysDay"));
        lstCalBuysDay->setProperty("showDropIndicator", QVariant(false));
        lstCalBuysDay->setAlternatingRowColors(true);
        lstCalBuysDay->setSortingEnabled(true);

        verticalLayout_8->addWidget(lstCalBuysDay);

        tboxCalBuys->addItem(tboxPageBuyDay, QStringLiteral("Purchase"));
        tboxPageBuyWeek = new QWidget();
        tboxPageBuyWeek->setObjectName(QStringLiteral("tboxPageBuyWeek"));
        tboxPageBuyWeek->setGeometry(QRect(0, 0, 228, 168));
        verticalLayout_23 = new QVBoxLayout(tboxPageBuyWeek);
        verticalLayout_23->setObjectName(QStringLiteral("verticalLayout_23"));
        lblCalPriceBuyWeek = new QLabel(tboxPageBuyWeek);
        lblCalPriceBuyWeek->setObjectName(QStringLiteral("lblCalPriceBuyWeek"));
        lblCalPriceBuyWeek->setFrameShape(QFrame::StyledPanel);
        lblCalPriceBuyWeek->setFrameShadow(QFrame::Raised);

        verticalLayout_23->addWidget(lblCalPriceBuyWeek);

        txtCalPriceBuyWeek = new vmLineEdit(tboxPageBuyWeek);
        txtCalPriceBuyWeek->setObjectName(QStringLiteral("txtCalPriceBuyWeek"));
        txtCalPriceBuyWeek->setFont(font1);
        txtCalPriceBuyWeek->setReadOnly(true);

        verticalLayout_23->addWidget(txtCalPriceBuyWeek);

        lstCalBuysWeek = new vmListWidget(tboxPageBuyWeek);
        lstCalBuysWeek->setObjectName(QStringLiteral("lstCalBuysWeek"));
        lstCalBuysWeek->setProperty("showDropIndicator", QVariant(false));
        lstCalBuysWeek->setAlternatingRowColors(true);
        lstCalBuysWeek->setSortingEnabled(true);

        verticalLayout_23->addWidget(lstCalBuysWeek);

        tboxCalBuys->addItem(tboxPageBuyWeek, QStringLiteral("Purchase"));
        tboxPageBuyMonth = new QWidget();
        tboxPageBuyMonth->setObjectName(QStringLiteral("tboxPageBuyMonth"));
        tboxPageBuyMonth->setGeometry(QRect(0, 0, 240, 168));
        verticalLayout_24 = new QVBoxLayout(tboxPageBuyMonth);
        verticalLayout_24->setObjectName(QStringLiteral("verticalLayout_24"));
        lblCalPriceBuyMonth = new QLabel(tboxPageBuyMonth);
        lblCalPriceBuyMonth->setObjectName(QStringLiteral("lblCalPriceBuyMonth"));
        lblCalPriceBuyMonth->setFrameShape(QFrame::StyledPanel);
        lblCalPriceBuyMonth->setFrameShadow(QFrame::Raised);

        verticalLayout_24->addWidget(lblCalPriceBuyMonth);

        txtCalPriceBuyMonth = new vmLineEdit(tboxPageBuyMonth);
        txtCalPriceBuyMonth->setObjectName(QStringLiteral("txtCalPriceBuyMonth"));
        txtCalPriceBuyMonth->setFont(font1);
        txtCalPriceBuyMonth->setReadOnly(true);

        verticalLayout_24->addWidget(txtCalPriceBuyMonth);

        lstCalBuysMonth = new vmListWidget(tboxPageBuyMonth);
        lstCalBuysMonth->setObjectName(QStringLiteral("lstCalBuysMonth"));
        lstCalBuysMonth->setProperty("showDropIndicator", QVariant(false));
        lstCalBuysMonth->setAlternatingRowColors(true);
        lstCalBuysMonth->setSortingEnabled(true);

        verticalLayout_24->addWidget(lstCalBuysMonth);

        tboxCalBuys->addItem(tboxPageBuyMonth, QStringLiteral("Purchase"));
        splitterCalLists->addWidget(tboxCalBuys);

        horizontalLayout_3->addWidget(splitterCalLists);

        splitterCalendarTab->addWidget(frame);

        verticalLayout_10->addWidget(splitterCalendarTab);


        verticalLayout_11->addLayout(verticalLayout_10);

        tabMain->addTab(tabCalendar, QString());
        tabInventory = new QWidget();
        tabInventory->setObjectName(QStringLiteral("tabInventory"));
        tabMain->addTab(tabInventory, QString());
        tabSupplies = new QWidget();
        tabSupplies->setObjectName(QStringLiteral("tabSupplies"));
        tabMain->addTab(tabSupplies, QString());
        tabDesign_1 = new QWidget();
        tabDesign_1->setObjectName(QStringLiteral("tabDesign_1"));
        verticalLayout_3 = new QVBoxLayout(tabDesign_1);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        frmPayInfo = new QFrame(tabDesign_1);
        frmPayInfo->setObjectName(QStringLiteral("frmPayInfo"));
        frmPayInfo->setFrameShape(QFrame::NoFrame);
        frmPayInfo->setFrameShadow(QFrame::Plain);
        gLayoutPayForms = new QGridLayout(frmPayInfo);
        gLayoutPayForms->setSpacing(2);
        gLayoutPayForms->setObjectName(QStringLiteral("gLayoutPayForms"));
        gLayoutPayForms->setContentsMargins(2, 2, 2, 2);
        txtPayTotalPrice = new vmLineEdit(frmPayInfo);
        txtPayTotalPrice->setObjectName(QStringLiteral("txtPayTotalPrice"));
        txtPayTotalPrice->setMinimumSize(QSize(0, 0));
        txtPayTotalPrice->setMaximumSize(QSize(180, 16777215));
        txtPayTotalPrice->setInputMethodHints(Qt::ImhDigitsOnly|Qt::ImhNoPredictiveText);
        txtPayTotalPrice->setReadOnly(false);

        gLayoutPayForms->addWidget(txtPayTotalPrice, 4, 3, 1, 1);

        frmPayToolbar = new QFrame(frmPayInfo);
        frmPayToolbar->setObjectName(QStringLiteral("frmPayToolbar"));
        frmPayToolbar->setMinimumSize(QSize(30, 0));
        frmPayToolbar->setFrameShape(QFrame::NoFrame);
        frmPayToolbar->setFrameShadow(QFrame::Plain);
        frmPayToolbar->setLineWidth(3);
        verticalLayout_14 = new QVBoxLayout(frmPayToolbar);
        verticalLayout_14->setSpacing(2);
        verticalLayout_14->setObjectName(QStringLiteral("verticalLayout_14"));
        verticalLayout_14->setContentsMargins(2, 2, 2, 2);
        btnPayInfoEdit = new QToolButton(frmPayToolbar);
        btnPayInfoEdit->setObjectName(QStringLiteral("btnPayInfoEdit"));
        btnPayInfoEdit->setMinimumSize(QSize(0, 0));
        QIcon icon8;
        icon8.addFile(QStringLiteral(":/resources/browse-controls/edit.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnPayInfoEdit->setIcon(icon8);

        verticalLayout_14->addWidget(btnPayInfoEdit);

        btnPayInfoDel = new QToolButton(frmPayToolbar);
        btnPayInfoDel->setObjectName(QStringLiteral("btnPayInfoDel"));
        btnPayInfoDel->setMinimumSize(QSize(0, 0));
        QIcon icon9;
        icon9.addFile(QStringLiteral(":/resources/browse-controls/remove.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnPayInfoDel->setIcon(icon9);

        verticalLayout_14->addWidget(btnPayInfoDel);

        btnPayInfoSave = new QToolButton(frmPayToolbar);
        btnPayInfoSave->setObjectName(QStringLiteral("btnPayInfoSave"));
        btnPayInfoSave->setMinimumSize(QSize(0, 0));
        QIcon icon10;
        icon10.addFile(QStringLiteral(":/resources/document-save.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnPayInfoSave->setIcon(icon10);

        verticalLayout_14->addWidget(btnPayInfoSave);

        btnPayInfoCancel = new QToolButton(frmPayToolbar);
        btnPayInfoCancel->setObjectName(QStringLiteral("btnPayInfoCancel"));
        btnPayInfoCancel->setMinimumSize(QSize(0, 0));
        QIcon icon11;
        icon11.addFile(QStringLiteral(":/resources/cancel.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnPayInfoCancel->setIcon(icon11);

        verticalLayout_14->addWidget(btnPayInfoCancel);

        btnPayPayReceipt = new QToolButton(frmPayToolbar);
        btnPayPayReceipt->setObjectName(QStringLiteral("btnPayPayReceipt"));
        btnPayPayReceipt->setMinimumSize(QSize(0, 0));
        QIcon icon12;
        icon12.addFile(QStringLiteral(":/resources/generate_paystub.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnPayPayReceipt->setIcon(icon12);

        verticalLayout_14->addWidget(btnPayPayReceipt);

        btnPayPaymentsReportOnlyUnPaid = new QToolButton(frmPayToolbar);
        btnPayPaymentsReportOnlyUnPaid->setObjectName(QStringLiteral("btnPayPaymentsReportOnlyUnPaid"));
        btnPayPaymentsReportOnlyUnPaid->setMinimumSize(QSize(0, 0));
        QIcon icon13;
        icon13.addFile(QStringLiteral(":/resources/generate_report_unpaid.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnPayPaymentsReportOnlyUnPaid->setIcon(icon13);

        verticalLayout_14->addWidget(btnPayPaymentsReportOnlyUnPaid);

        btnPayPaymentsReport = new QToolButton(frmPayToolbar);
        btnPayPaymentsReport->setObjectName(QStringLiteral("btnPayPaymentsReport"));
        btnPayPaymentsReport->setMinimumSize(QSize(0, 0));
        QIcon icon14;
        icon14.addFile(QStringLiteral(":/resources/generate_report.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnPayPaymentsReport->setIcon(icon14);

        verticalLayout_14->addWidget(btnPayPaymentsReport);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_14->addItem(verticalSpacer_2);


        gLayoutPayForms->addWidget(frmPayToolbar, 1, 0, 5, 1);

        txtPayID = new vmLineEdit(frmPayInfo);
        txtPayID->setObjectName(QStringLiteral("txtPayID"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(txtPayID->sizePolicy().hasHeightForWidth());
        txtPayID->setSizePolicy(sizePolicy1);
        txtPayID->setMaximumSize(QSize(120, 16777215));
        txtPayID->setReadOnly(true);

        gLayoutPayForms->addWidget(txtPayID, 4, 6, 1, 1);

        lblTotalPayments = new QLabel(frmPayInfo);
        lblTotalPayments->setObjectName(QStringLiteral("lblTotalPayments"));
        sizePolicy1.setHeightForWidth(lblTotalPayments->sizePolicy().hasHeightForWidth());
        lblTotalPayments->setSizePolicy(sizePolicy1);
        lblTotalPayments->setMinimumSize(QSize(0, 0));
        lblTotalPayments->setMaximumSize(QSize(50, 40));
        lblTotalPayments->setFrameShape(QFrame::NoFrame);
        lblTotalPayments->setFrameShadow(QFrame::Plain);
        lblTotalPayments->setTextFormat(Qt::PlainText);

        gLayoutPayForms->addWidget(lblTotalPayments, 2, 5, 1, 1);

        txtPayObs = new vmLineEdit(frmPayInfo);
        txtPayObs->setObjectName(QStringLiteral("txtPayObs"));
        txtPayObs->setMinimumSize(QSize(0, 0));
        txtPayObs->setFrame(false);

        gLayoutPayForms->addWidget(txtPayObs, 5, 4, 1, 1);

        lblPayObs = new QLabel(frmPayInfo);
        lblPayObs->setObjectName(QStringLiteral("lblPayObs"));
        lblPayObs->setMinimumSize(QSize(0, 30));
        lblPayObs->setMaximumSize(QSize(16777215, 40));
        lblPayObs->setFrameShape(QFrame::NoFrame);
        lblPayObs->setFrameShadow(QFrame::Plain);
        lblPayObs->setTextFormat(Qt::PlainText);

        gLayoutPayForms->addWidget(lblPayObs, 5, 3, 1, 1);

        tablePayments = new vmTableWidget(frmPayInfo);
        tablePayments->setObjectName(QStringLiteral("tablePayments"));
        tablePayments->setFrameShape(QFrame::StyledPanel);
        tablePayments->setFrameShadow(QFrame::Raised);
        tablePayments->setLineWidth(1);

        gLayoutPayForms->addWidget(tablePayments, 1, 2, 1, 5);

        txtPayTotalPaid = new vmLineEdit(frmPayInfo);
        txtPayTotalPaid->setObjectName(QStringLiteral("txtPayTotalPaid"));
        txtPayTotalPaid->setReadOnly(true);

        gLayoutPayForms->addWidget(txtPayTotalPaid, 4, 4, 1, 1);

        lblPayID = new QLabel(frmPayInfo);
        lblPayID->setObjectName(QStringLiteral("lblPayID"));
        sizePolicy1.setHeightForWidth(lblPayID->sizePolicy().hasHeightForWidth());
        lblPayID->setSizePolicy(sizePolicy1);
        lblPayID->setMaximumSize(QSize(50, 16777215));

        gLayoutPayForms->addWidget(lblPayID, 2, 6, 1, 1);

        chkPayOverdue = new vmCheckBox(frmPayInfo);
        chkPayOverdue->setObjectName(QStringLiteral("chkPayOverdue"));

        gLayoutPayForms->addWidget(chkPayOverdue, 5, 5, 1, 2);

        frmClientToolbar = new QFrame(frmPayInfo);
        frmClientToolbar->setObjectName(QStringLiteral("frmClientToolbar"));
        QSizePolicy sizePolicy2(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(frmClientToolbar->sizePolicy().hasHeightForWidth());
        frmClientToolbar->setSizePolicy(sizePolicy2);
        frmClientToolbar->setMinimumSize(QSize(30, 200));
        frmClientToolbar->setFrameShape(QFrame::NoFrame);
        frmClientToolbar->setFrameShadow(QFrame::Plain);
        verticalLayout_20 = new QVBoxLayout(frmClientToolbar);
        verticalLayout_20->setSpacing(2);
        verticalLayout_20->setObjectName(QStringLiteral("verticalLayout_20"));
        verticalLayout_20->setContentsMargins(2, 2, 2, 2);
        btnClientAdd = new QToolButton(frmClientToolbar);
        btnClientAdd->setObjectName(QStringLiteral("btnClientAdd"));
        QIcon icon15;
        icon15.addFile(QStringLiteral(":/resources/browse-controls/add.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnClientAdd->setIcon(icon15);

        verticalLayout_20->addWidget(btnClientAdd);

        btnClientEdit = new QToolButton(frmClientToolbar);
        btnClientEdit->setObjectName(QStringLiteral("btnClientEdit"));
        btnClientEdit->setIcon(icon8);

        verticalLayout_20->addWidget(btnClientEdit);

        btnClientDel = new QToolButton(frmClientToolbar);
        btnClientDel->setObjectName(QStringLiteral("btnClientDel"));
        btnClientDel->setIcon(icon9);

        verticalLayout_20->addWidget(btnClientDel);

        btnClientSave = new QToolButton(frmClientToolbar);
        btnClientSave->setObjectName(QStringLiteral("btnClientSave"));
        btnClientSave->setIcon(icon10);

        verticalLayout_20->addWidget(btnClientSave);

        btnClientCancel = new QToolButton(frmClientToolbar);
        btnClientCancel->setObjectName(QStringLiteral("btnClientCancel"));
        btnClientCancel->setIcon(icon11);

        verticalLayout_20->addWidget(btnClientCancel);

        verticalSpacer_4 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_20->addItem(verticalSpacer_4);


        gLayoutPayForms->addWidget(frmClientToolbar, 1, 1, 5, 1);

        txtPayTotalPayments = new vmLineEdit(frmPayInfo);
        txtPayTotalPayments->setObjectName(QStringLiteral("txtPayTotalPayments"));
        sizePolicy1.setHeightForWidth(txtPayTotalPayments->sizePolicy().hasHeightForWidth());
        txtPayTotalPayments->setSizePolicy(sizePolicy1);
        txtPayTotalPayments->setMinimumSize(QSize(0, 0));
        txtPayTotalPayments->setMaximumSize(QSize(50, 16777215));
        txtPayTotalPayments->setInputMethodHints(Qt::ImhFormattedNumbersOnly);
        txtPayTotalPayments->setReadOnly(true);

        gLayoutPayForms->addWidget(txtPayTotalPayments, 4, 5, 1, 1);

        lblPayTotalPaid = new QLabel(frmPayInfo);
        lblPayTotalPaid->setObjectName(QStringLiteral("lblPayTotalPaid"));
        QSizePolicy sizePolicy3(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(lblPayTotalPaid->sizePolicy().hasHeightForWidth());
        lblPayTotalPaid->setSizePolicy(sizePolicy3);

        gLayoutPayForms->addWidget(lblPayTotalPaid, 2, 4, 1, 1);

        widget = new QWidget(frmPayInfo);
        widget->setObjectName(QStringLiteral("widget"));

        gLayoutPayForms->addWidget(widget, 0, 3, 1, 1);

        lblTotalPay = new QLabel(frmPayInfo);
        lblTotalPay->setObjectName(QStringLiteral("lblTotalPay"));
        sizePolicy3.setHeightForWidth(lblTotalPay->sizePolicy().hasHeightForWidth());
        lblTotalPay->setSizePolicy(sizePolicy3);
        lblTotalPay->setMinimumSize(QSize(0, 0));
        lblTotalPay->setMaximumSize(QSize(16777215, 40));
        lblTotalPay->setFrameShape(QFrame::NoFrame);
        lblTotalPay->setFrameShadow(QFrame::Plain);
        lblTotalPay->setTextFormat(Qt::PlainText);

        gLayoutPayForms->addWidget(lblTotalPay, 3, 3, 1, 1);


        verticalLayout_3->addWidget(frmPayInfo);

        frmClientInfo = new QFrame(tabDesign_1);
        frmClientInfo->setObjectName(QStringLiteral("frmClientInfo"));
        frmClientInfo->setMinimumSize(QSize(0, 250));
        frmClientInfo->setFrameShape(QFrame::NoFrame);
        frmClientInfo->setFrameShadow(QFrame::Plain);
        gridLayout_3 = new QGridLayout(frmClientInfo);
        gridLayout_3->setSpacing(2);
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        gridLayout_3->setContentsMargins(0, 0, 0, 0);
        txtClientZipCode = new vmLineEdit(frmClientInfo);
        txtClientZipCode->setObjectName(QStringLiteral("txtClientZipCode"));
        sizePolicy3.setHeightForWidth(txtClientZipCode->sizePolicy().hasHeightForWidth());
        txtClientZipCode->setSizePolicy(sizePolicy3);
        txtClientZipCode->setMinimumSize(QSize(0, 0));
        txtClientZipCode->setInputMethodHints(Qt::ImhFormattedNumbersOnly);
        txtClientZipCode->setInputMask(QStringLiteral(""));
        txtClientZipCode->setReadOnly(false);

        gridLayout_3->addWidget(txtClientZipCode, 5, 6, 1, 1);

        txtClientCity = new vmLineEdit(frmClientInfo);
        txtClientCity->setObjectName(QStringLiteral("txtClientCity"));
        sizePolicy3.setHeightForWidth(txtClientCity->sizePolicy().hasHeightForWidth());
        txtClientCity->setSizePolicy(sizePolicy3);
        txtClientCity->setMinimumSize(QSize(0, 0));
        txtClientCity->setInputMask(QStringLiteral(""));
        txtClientCity->setReadOnly(false);

        gridLayout_3->addWidget(txtClientCity, 5, 3, 1, 3);

        lblClientID = new QLabel(frmClientInfo);
        lblClientID->setObjectName(QStringLiteral("lblClientID"));
        lblClientID->setMinimumSize(QSize(50, 30));
        lblClientID->setFrameShape(QFrame::NoFrame);
        lblClientID->setFrameShadow(QFrame::Plain);
        lblClientID->setTextFormat(Qt::PlainText);

        gridLayout_3->addWidget(lblClientID, 0, 6, 1, 1);

        txtClientName = new vmLineEdit(frmClientInfo);
        txtClientName->setObjectName(QStringLiteral("txtClientName"));
        sizePolicy3.setHeightForWidth(txtClientName->sizePolicy().hasHeightForWidth());
        txtClientName->setSizePolicy(sizePolicy3);
        txtClientName->setMinimumSize(QSize(0, 0));
        txtClientName->setMaximumSize(QSize(16777215, 16777215));
        txtClientName->setInputMask(QStringLiteral(""));
        txtClientName->setReadOnly(false);

        gridLayout_3->addWidget(txtClientName, 1, 0, 1, 4);

        lblClientPhones = new QLabel(frmClientInfo);
        lblClientPhones->setObjectName(QStringLiteral("lblClientPhones"));

        gridLayout_3->addWidget(lblClientPhones, 6, 0, 1, 1);

        txtClientDistrict = new vmLineEdit(frmClientInfo);
        txtClientDistrict->setObjectName(QStringLiteral("txtClientDistrict"));
        sizePolicy3.setHeightForWidth(txtClientDistrict->sizePolicy().hasHeightForWidth());
        txtClientDistrict->setSizePolicy(sizePolicy3);
        txtClientDistrict->setMinimumSize(QSize(0, 0));
        txtClientDistrict->setInputMask(QStringLiteral(""));
        txtClientDistrict->setReadOnly(false);

        gridLayout_3->addWidget(txtClientDistrict, 5, 0, 1, 1);

        txtClientID = new vmLineEdit(frmClientInfo);
        txtClientID->setObjectName(QStringLiteral("txtClientID"));
        sizePolicy3.setHeightForWidth(txtClientID->sizePolicy().hasHeightForWidth());
        txtClientID->setSizePolicy(sizePolicy3);
        txtClientID->setMinimumSize(QSize(0, 0));
        txtClientID->setMaximumSize(QSize(120, 16777215));
        txtClientID->setFocusPolicy(Qt::NoFocus);
        txtClientID->setAcceptDrops(false);
#ifndef QT_NO_ACCESSIBILITY
        txtClientID->setAccessibleDescription(QStringLiteral(""));
#endif // QT_NO_ACCESSIBILITY
        txtClientID->setInputMask(QStringLiteral(""));
        txtClientID->setText(QStringLiteral(""));
        txtClientID->setReadOnly(true);

        gridLayout_3->addWidget(txtClientID, 1, 6, 1, 1);

        lblClientCity = new QLabel(frmClientInfo);
        lblClientCity->setObjectName(QStringLiteral("lblClientCity"));
        lblClientCity->setMinimumSize(QSize(0, 30));
        lblClientCity->setFrameShape(QFrame::NoFrame);
        lblClientCity->setFrameShadow(QFrame::Plain);
        lblClientCity->setTextFormat(Qt::PlainText);

        gridLayout_3->addWidget(lblClientCity, 4, 3, 1, 1);

        frame_2 = new QFrame(frmClientInfo);
        frame_2->setObjectName(QStringLiteral("frame_2"));
        frame_2->setMinimumSize(QSize(50, 0));
        frame_2->setFrameShape(QFrame::NoFrame);
        frame_2->setFrameShadow(QFrame::Plain);
        verticalLayout_4 = new QVBoxLayout(frame_2);
        verticalLayout_4->setSpacing(5);
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        verticalLayout_4->setContentsMargins(-1, 2, 2, 2);
        lblClientDateFrom = new QLabel(frame_2);
        lblClientDateFrom->setObjectName(QStringLiteral("lblClientDateFrom"));
        lblClientDateFrom->setMinimumSize(QSize(0, 30));
        lblClientDateFrom->setFrameShape(QFrame::NoFrame);
        lblClientDateFrom->setFrameShadow(QFrame::Plain);
        lblClientDateFrom->setTextFormat(Qt::PlainText);

        verticalLayout_4->addWidget(lblClientDateFrom);

        dteClientDateFrom = new vmDateEdit(frame_2);
        dteClientDateFrom->setObjectName(QStringLiteral("dteClientDateFrom"));
        sizePolicy3.setHeightForWidth(dteClientDateFrom->sizePolicy().hasHeightForWidth());
        dteClientDateFrom->setSizePolicy(sizePolicy3);
        dteClientDateFrom->setMinimumSize(QSize(0, 20));
        dteClientDateFrom->setProperty("dateTime", QVariant(QDateTime(QDate(2010, 1, 1), QTime(0, 0, 0))));
        dteClientDateFrom->setProperty("calendarPopup", QVariant(true));

        verticalLayout_4->addWidget(dteClientDateFrom);

        lblClientDateTo = new QLabel(frame_2);
        lblClientDateTo->setObjectName(QStringLiteral("lblClientDateTo"));
        lblClientDateTo->setMinimumSize(QSize(0, 30));
        lblClientDateTo->setFrameShape(QFrame::NoFrame);
        lblClientDateTo->setFrameShadow(QFrame::Plain);
        lblClientDateTo->setTextFormat(Qt::PlainText);

        verticalLayout_4->addWidget(lblClientDateTo);

        dteClientDateTo = new vmDateEdit(frame_2);
        dteClientDateTo->setObjectName(QStringLiteral("dteClientDateTo"));
        sizePolicy3.setHeightForWidth(dteClientDateTo->sizePolicy().hasHeightForWidth());
        dteClientDateTo->setSizePolicy(sizePolicy3);
        dteClientDateTo->setMinimumSize(QSize(0, 20));
        dteClientDateTo->setProperty("dateTime", QVariant(QDateTime(QDate(2010, 1, 1), QTime(0, 0, 0))));
        dteClientDateTo->setProperty("calendarPopup", QVariant(true));

        verticalLayout_4->addWidget(dteClientDateTo);

        chkClientActive = new vmCheckBox(frame_2);
        chkClientActive->setObjectName(QStringLiteral("chkClientActive"));
        chkClientActive->setChecked(true);

        verticalLayout_4->addWidget(chkClientActive);


        gridLayout_3->addWidget(frame_2, 0, 7, 6, 1);

        lblClientDistrict = new QLabel(frmClientInfo);
        lblClientDistrict->setObjectName(QStringLiteral("lblClientDistrict"));
        lblClientDistrict->setMinimumSize(QSize(0, 30));
        lblClientDistrict->setFrameShape(QFrame::NoFrame);
        lblClientDistrict->setFrameShadow(QFrame::Plain);
        lblClientDistrict->setTextFormat(Qt::PlainText);

        gridLayout_3->addWidget(lblClientDistrict, 4, 0, 1, 1);

        lblClientName = new QLabel(frmClientInfo);
        lblClientName->setObjectName(QStringLiteral("lblClientName"));
        lblClientName->setMinimumSize(QSize(0, 30));
        lblClientName->setFrameShape(QFrame::NoFrame);
        lblClientName->setFrameShadow(QFrame::Plain);
        lblClientName->setTextFormat(Qt::PlainText);

        gridLayout_3->addWidget(lblClientName, 0, 0, 1, 1);

        lblClientStreetAdress = new QLabel(frmClientInfo);
        lblClientStreetAdress->setObjectName(QStringLiteral("lblClientStreetAdress"));
        lblClientStreetAdress->setMinimumSize(QSize(0, 30));
        lblClientStreetAdress->setFrameShape(QFrame::NoFrame);
        lblClientStreetAdress->setFrameShadow(QFrame::Plain);
        lblClientStreetAdress->setTextFormat(Qt::PlainText);

        gridLayout_3->addWidget(lblClientStreetAdress, 2, 0, 1, 1);

        lblClientEmail = new QLabel(frmClientInfo);
        lblClientEmail->setObjectName(QStringLiteral("lblClientEmail"));

        gridLayout_3->addWidget(lblClientEmail, 6, 3, 1, 5);

        contactsClientPhones = new contactsManagerWidget(frmClientInfo);
        contactsClientPhones->setObjectName(QStringLiteral("contactsClientPhones"));
        contactsClientPhones->setMinimumSize(QSize(100, 25));

        gridLayout_3->addWidget(contactsClientPhones, 7, 0, 1, 1);

        txtClientStreetAddress = new vmLineEdit(frmClientInfo);
        txtClientStreetAddress->setObjectName(QStringLiteral("txtClientStreetAddress"));
        sizePolicy3.setHeightForWidth(txtClientStreetAddress->sizePolicy().hasHeightForWidth());
        txtClientStreetAddress->setSizePolicy(sizePolicy3);
        txtClientStreetAddress->setMinimumSize(QSize(0, 0));
        txtClientStreetAddress->setInputMask(QStringLiteral(""));
        txtClientStreetAddress->setReadOnly(false);

        gridLayout_3->addWidget(txtClientStreetAddress, 3, 0, 1, 4);

        contactsClientEmails = new contactsManagerWidget(frmClientInfo);
        contactsClientEmails->setObjectName(QStringLiteral("contactsClientEmails"));
        contactsClientEmails->setMinimumSize(QSize(0, 25));

        gridLayout_3->addWidget(contactsClientEmails, 7, 3, 1, 5);

        lblClientZipCode = new QLabel(frmClientInfo);
        lblClientZipCode->setObjectName(QStringLiteral("lblClientZipCode"));
        lblClientZipCode->setMinimumSize(QSize(0, 30));
        lblClientZipCode->setFrameShape(QFrame::NoFrame);
        lblClientZipCode->setFrameShadow(QFrame::Plain);

        gridLayout_3->addWidget(lblClientZipCode, 4, 6, 1, 1);

        txtClientNumberAddress = new vmLineEdit(frmClientInfo);
        txtClientNumberAddress->setObjectName(QStringLiteral("txtClientNumberAddress"));
        sizePolicy3.setHeightForWidth(txtClientNumberAddress->sizePolicy().hasHeightForWidth());
        txtClientNumberAddress->setSizePolicy(sizePolicy3);
        txtClientNumberAddress->setMinimumSize(QSize(80, 0));
        txtClientNumberAddress->setMaximumSize(QSize(120, 16777215));
        txtClientNumberAddress->setInputMask(QStringLiteral(""));
        txtClientNumberAddress->setReadOnly(false);

        gridLayout_3->addWidget(txtClientNumberAddress, 3, 6, 1, 1);

        lblClientNumberAdress = new QLabel(frmClientInfo);
        lblClientNumberAdress->setObjectName(QStringLiteral("lblClientNumberAdress"));
        lblClientNumberAdress->setMinimumSize(QSize(55, 30));
        lblClientNumberAdress->setFrameShape(QFrame::NoFrame);
        lblClientNumberAdress->setFrameShadow(QFrame::Plain);
        lblClientNumberAdress->setTextFormat(Qt::PlainText);

        gridLayout_3->addWidget(lblClientNumberAdress, 2, 6, 1, 1);

        gridLayout_3->setColumnStretch(0, 2);
        lblClientID->raise();
        txtClientID->raise();
        lblClientNumberAdress->raise();
        txtClientNumberAddress->raise();
        lblClientCity->raise();
        lblClientZipCode->raise();
        txtClientZipCode->raise();
        txtClientCity->raise();
        lblClientEmail->raise();
        contactsClientEmails->raise();
        frame_2->raise();
        txtClientName->raise();
        lblClientName->raise();
        txtClientStreetAddress->raise();
        lblClientStreetAdress->raise();
        lblClientDistrict->raise();
        txtClientDistrict->raise();
        lblClientPhones->raise();
        contactsClientPhones->raise();

        verticalLayout_3->addWidget(frmClientInfo);

        tabMain->addTab(tabDesign_1, QString());
        tab = new QWidget();
        tab->setObjectName(QStringLiteral("tab"));
        horizontalLayout_2 = new QHBoxLayout(tab);
        horizontalLayout_2->setSpacing(5);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(2, 2, 2, 2);
        frmJobInfo = new QFrame(tab);
        frmJobInfo->setObjectName(QStringLiteral("frmJobInfo"));
        frmJobInfo->setFrameShape(QFrame::NoFrame);
        frmJobInfo->setFrameShadow(QFrame::Plain);
        verticalLayout = new QVBoxLayout(frmJobInfo);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(2, 2, 2, 2);
        gridLayout_15 = new QGridLayout();
        gridLayout_15->setObjectName(QStringLiteral("gridLayout_15"));
        gridLayout_15->setHorizontalSpacing(5);
        gridLayout_15->setVerticalSpacing(2);
        gridLayout_15->setContentsMargins(2, 2, 2, 2);
        lblJobAddress = new QLabel(frmJobInfo);
        lblJobAddress->setObjectName(QStringLiteral("lblJobAddress"));

        gridLayout_15->addWidget(lblJobAddress, 0, 3, 1, 1);

        frmJobToolbar = new QFrame(frmJobInfo);
        frmJobToolbar->setObjectName(QStringLiteral("frmJobToolbar"));
        frmJobToolbar->setMaximumSize(QSize(16777215, 16777215));
        frmJobToolbar->setFrameShape(QFrame::NoFrame);
        frmJobToolbar->setFrameShadow(QFrame::Plain);
        frmJobToolbar->setLineWidth(3);
        verticalLayout_13 = new QVBoxLayout(frmJobToolbar);
        verticalLayout_13->setSpacing(5);
        verticalLayout_13->setObjectName(QStringLiteral("verticalLayout_13"));
        verticalLayout_13->setContentsMargins(2, 2, 2, 2);
        btnJobAdd = new QToolButton(frmJobToolbar);
        btnJobAdd->setObjectName(QStringLiteral("btnJobAdd"));
        btnJobAdd->setIcon(icon15);

        verticalLayout_13->addWidget(btnJobAdd);

        btnJobEdit = new QToolButton(frmJobToolbar);
        btnJobEdit->setObjectName(QStringLiteral("btnJobEdit"));
        btnJobEdit->setIcon(icon8);

        verticalLayout_13->addWidget(btnJobEdit);

        btnJobDel = new QToolButton(frmJobToolbar);
        btnJobDel->setObjectName(QStringLiteral("btnJobDel"));
        btnJobDel->setIcon(icon9);

        verticalLayout_13->addWidget(btnJobDel);

        btnJobSave = new QToolButton(frmJobToolbar);
        btnJobSave->setObjectName(QStringLiteral("btnJobSave"));
        btnJobSave->setIcon(icon10);

        verticalLayout_13->addWidget(btnJobSave);

        btnJobCancel = new QToolButton(frmJobToolbar);
        btnJobCancel->setObjectName(QStringLiteral("btnJobCancel"));
        btnJobCancel->setIcon(icon11);

        verticalLayout_13->addWidget(btnJobCancel);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_13->addItem(verticalSpacer);

        btnJobSelectJob = new QToolButton(frmJobToolbar);
        btnJobSelectJob->setObjectName(QStringLiteral("btnJobSelectJob"));

        verticalLayout_13->addWidget(btnJobSelectJob);


        gridLayout_15->addWidget(frmJobToolbar, 0, 0, 7, 1);

        lblJobProjectID = new QLabel(frmJobInfo);
        lblJobProjectID->setObjectName(QStringLiteral("lblJobProjectID"));

        gridLayout_15->addWidget(lblJobProjectID, 4, 1, 1, 1);

        txtJobProjectPath = new vmLineEdit(frmJobInfo);
        txtJobProjectPath->setObjectName(QStringLiteral("txtJobProjectPath"));
        txtJobProjectPath->setReadOnly(true);

        gridLayout_15->addWidget(txtJobProjectPath, 3, 1, 1, 7);

        btnNewProject = new QToolButton(frmJobInfo);
        btnNewProject->setObjectName(QStringLiteral("btnNewProject"));
        btnNewProject->setMinimumSize(QSize(30, 30));
        btnNewProject->setMaximumSize(QSize(30, 30));
        QIcon icon16;
        icon16.addFile(QStringLiteral(":/resources/new_project.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnNewProject->setIcon(icon16);
        btnNewProject->setPopupMode(QToolButton::InstantPopup);

        gridLayout_15->addWidget(btnNewProject, 2, 2, 1, 1);

        lblJobType = new QLabel(frmJobInfo);
        lblJobType->setObjectName(QStringLiteral("lblJobType"));
        lblJobType->setFrameShape(QFrame::NoFrame);
        lblJobType->setFrameShadow(QFrame::Plain);

        gridLayout_15->addWidget(lblJobType, 0, 1, 1, 1);

        btnJobOpenDoc = new QToolButton(frmJobInfo);
        btnJobOpenDoc->setObjectName(QStringLiteral("btnJobOpenDoc"));
        sizePolicy1.setHeightForWidth(btnJobOpenDoc->sizePolicy().hasHeightForWidth());
        btnJobOpenDoc->setSizePolicy(sizePolicy1);
        btnJobOpenDoc->setMinimumSize(QSize(30, 30));
        btnJobOpenDoc->setMaximumSize(QSize(30, 30));
        QIcon icon17;
        icon17.addFile(QStringLiteral(":/resources/Microsoft+Office+Word+2007.xpm"), QSize(), QIcon::Normal, QIcon::Off);
        btnJobOpenDoc->setIcon(icon17);

        gridLayout_15->addWidget(btnJobOpenDoc, 2, 5, 1, 1);

        lblJobID = new QLabel(frmJobInfo);
        lblJobID->setObjectName(QStringLiteral("lblJobID"));
        lblJobID->setMinimumSize(QSize(40, 30));
        lblJobID->setMaximumSize(QSize(40, 30));
        lblJobID->setFrameShape(QFrame::NoFrame);
        lblJobID->setFrameShadow(QFrame::Plain);

        gridLayout_15->addWidget(lblJobID, 0, 6, 1, 1);

        cboJobType = new vmComboBox(frmJobInfo);
        cboJobType->setObjectName(QStringLiteral("cboJobType"));
        cboJobType->setMinimumSize(QSize(300, 0));

        gridLayout_15->addWidget(cboJobType, 1, 1, 1, 2);

        btnJobOpenXls = new QToolButton(frmJobInfo);
        btnJobOpenXls->setObjectName(QStringLiteral("btnJobOpenXls"));
        sizePolicy1.setHeightForWidth(btnJobOpenXls->sizePolicy().hasHeightForWidth());
        btnJobOpenXls->setSizePolicy(sizePolicy1);
        btnJobOpenXls->setMinimumSize(QSize(30, 30));
        btnJobOpenXls->setMaximumSize(QSize(30, 30));
        QIcon icon18;
        icon18.addFile(QStringLiteral(":/resources/Microsoft+Office+Excel+2007.xpm"), QSize(), QIcon::Normal, QIcon::Off);
        btnJobOpenXls->setIcon(icon18);

        gridLayout_15->addWidget(btnJobOpenXls, 2, 6, 1, 1);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_15->addItem(horizontalSpacer_3, 2, 3, 1, 1);

        btnJobOpenFileFolder = new QToolButton(frmJobInfo);
        btnJobOpenFileFolder->setObjectName(QStringLiteral("btnJobOpenFileFolder"));
        sizePolicy1.setHeightForWidth(btnJobOpenFileFolder->sizePolicy().hasHeightForWidth());
        btnJobOpenFileFolder->setSizePolicy(sizePolicy1);
        btnJobOpenFileFolder->setMinimumSize(QSize(30, 30));
        btnJobOpenFileFolder->setMaximumSize(QSize(30, 30));
        QIcon icon19;
        icon19.addFile(QStringLiteral(":/resources/system-file-manager.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnJobOpenFileFolder->setIcon(icon19);

        gridLayout_15->addWidget(btnJobOpenFileFolder, 2, 4, 1, 1);

        lblJobFilepath = new QLabel(frmJobInfo);
        lblJobFilepath->setObjectName(QStringLiteral("lblJobFilepath"));
        lblJobFilepath->setMaximumSize(QSize(150, 16777215));
        lblJobFilepath->setFrameShape(QFrame::NoFrame);
        lblJobFilepath->setFrameShadow(QFrame::Plain);

        gridLayout_15->addWidget(lblJobFilepath, 2, 1, 1, 1);

        btnQuickProject = new QPushButton(frmJobInfo);
        btnQuickProject->setObjectName(QStringLiteral("btnQuickProject"));

        gridLayout_15->addWidget(btnQuickProject, 5, 6, 1, 2);

        btnJobOpenPdf = new QToolButton(frmJobInfo);
        btnJobOpenPdf->setObjectName(QStringLiteral("btnJobOpenPdf"));
        sizePolicy1.setHeightForWidth(btnJobOpenPdf->sizePolicy().hasHeightForWidth());
        btnJobOpenPdf->setSizePolicy(sizePolicy1);
        btnJobOpenPdf->setMinimumSize(QSize(30, 30));
        btnJobOpenPdf->setMaximumSize(QSize(30, 30));
        QIcon icon20;
        icon20.addFile(QStringLiteral(":/resources/application-pdf.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnJobOpenPdf->setIcon(icon20);

        gridLayout_15->addWidget(btnJobOpenPdf, 2, 7, 1, 1);

        txtJobProjectID = new vmLineEdit(frmJobInfo);
        txtJobProjectID->setObjectName(QStringLiteral("txtJobProjectID"));

        gridLayout_15->addWidget(txtJobProjectID, 5, 1, 1, 5);

        txtJobID = new vmLineEdit(frmJobInfo);
        txtJobID->setObjectName(QStringLiteral("txtJobID"));
        txtJobID->setMinimumSize(QSize(40, 0));
        txtJobID->setMaximumSize(QSize(120, 30));
        txtJobID->setFocusPolicy(Qt::NoFocus);
        txtJobID->setReadOnly(true);

        gridLayout_15->addWidget(txtJobID, 0, 7, 1, 1);

        txtJobAddress = new vmLineEdit(frmJobInfo);
        txtJobAddress->setObjectName(QStringLiteral("txtJobAddress"));

        gridLayout_15->addWidget(txtJobAddress, 1, 3, 1, 5);


        verticalLayout->addLayout(gridLayout_15);

        frmJobInfo2 = new QWidget(frmJobInfo);
        frmJobInfo2->setObjectName(QStringLiteral("frmJobInfo2"));
        gridLayout_6 = new QGridLayout(frmJobInfo2);
        gridLayout_6->setObjectName(QStringLiteral("gridLayout_6"));
        gridLayout_6->setHorizontalSpacing(2);
        gridLayout_6->setVerticalSpacing(0);
        gridLayout_6->setContentsMargins(2, 2, 2, 2);
        lblJobDateStart = new QLabel(frmJobInfo2);
        lblJobDateStart->setObjectName(QStringLiteral("lblJobDateStart"));

        gridLayout_6->addWidget(lblJobDateStart, 0, 2, 1, 1);

        txtJobPrice = new vmLineEdit(frmJobInfo2);
        txtJobPrice->setObjectName(QStringLiteral("txtJobPrice"));

        gridLayout_6->addWidget(txtJobPrice, 1, 0, 1, 1);

        dteJobEnd = new vmDateEdit(frmJobInfo2);
        dteJobEnd->setObjectName(QStringLiteral("dteJobEnd"));

        gridLayout_6->addWidget(dteJobEnd, 1, 3, 1, 1);

        dteJobStart = new vmDateEdit(frmJobInfo2);
        dteJobStart->setObjectName(QStringLiteral("dteJobStart"));

        gridLayout_6->addWidget(dteJobStart, 1, 2, 1, 1);

        lblJobPrice = new QLabel(frmJobInfo2);
        lblJobPrice->setObjectName(QStringLiteral("lblJobPrice"));
        sizePolicy3.setHeightForWidth(lblJobPrice->sizePolicy().hasHeightForWidth());
        lblJobPrice->setSizePolicy(sizePolicy3);
        lblJobPrice->setMinimumSize(QSize(0, 30));
        lblJobPrice->setMaximumSize(QSize(250, 16777215));
        lblJobPrice->setFrameShape(QFrame::NoFrame);
        lblJobPrice->setFrameShadow(QFrame::Plain);

        gridLayout_6->addWidget(lblJobPrice, 0, 0, 1, 1);

        lblJobDateEnd = new QLabel(frmJobInfo2);
        lblJobDateEnd->setObjectName(QStringLiteral("lblJobDateEnd"));

        gridLayout_6->addWidget(lblJobDateEnd, 0, 3, 1, 1);

        horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_6->addItem(horizontalSpacer_6, 0, 1, 1, 1);


        verticalLayout->addWidget(frmJobInfo2);


        horizontalLayout_2->addWidget(frmJobInfo);

        tabMain->addTab(tab, QString());
        tabDesign3 = new QWidget();
        tabDesign3->setObjectName(QStringLiteral("tabDesign3"));
        verticalLayout_27 = new QVBoxLayout(tabDesign3);
        verticalLayout_27->setSpacing(2);
        verticalLayout_27->setObjectName(QStringLiteral("verticalLayout_27"));
        verticalLayout_27->setContentsMargins(2, 2, 2, 2);
        frmJobInfo_2 = new QFrame(tabDesign3);
        frmJobInfo_2->setObjectName(QStringLiteral("frmJobInfo_2"));
        frmJobInfo_2->setFrameShape(QFrame::NoFrame);
        frmJobInfo_2->setFrameShadow(QFrame::Plain);
        frmJobInfo_2->setLineWidth(3);
        frmJobInfo_2->setMidLineWidth(0);
        gLayoutJobExtraInfo = new QGridLayout(frmJobInfo_2);
        gLayoutJobExtraInfo->setSpacing(2);
        gLayoutJobExtraInfo->setObjectName(QStringLiteral("gLayoutJobExtraInfo"));
        gLayoutJobExtraInfo->setContentsMargins(2, 2, 2, 2);
        grpJobTotalTime = new QGroupBox(frmJobInfo_2);
        grpJobTotalTime->setObjectName(QStringLiteral("grpJobTotalTime"));
        grpJobTotalTime->setFlat(true);
        gridLayout_4 = new QGridLayout(grpJobTotalTime);
        gridLayout_4->setSpacing(5);
        gridLayout_4->setObjectName(QStringLiteral("gridLayout_4"));
        gridLayout_4->setSizeConstraint(QLayout::SetNoConstraint);
        gridLayout_4->setContentsMargins(2, 2, 2, 2);
        lblJobDayTime = new QLabel(grpJobTotalTime);
        lblJobDayTime->setObjectName(QStringLiteral("lblJobDayTime"));

        gridLayout_4->addWidget(lblJobDayTime, 0, 0, 1, 1);

        lblJobAllDaysTime = new QLabel(grpJobTotalTime);
        lblJobAllDaysTime->setObjectName(QStringLiteral("lblJobAllDaysTime"));

        gridLayout_4->addWidget(lblJobAllDaysTime, 2, 0, 1, 1);

        txtJobTotalAllDaysTime = new vmLineEdit(grpJobTotalTime);
        txtJobTotalAllDaysTime->setObjectName(QStringLiteral("txtJobTotalAllDaysTime"));
        QSizePolicy sizePolicy4(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(txtJobTotalAllDaysTime->sizePolicy().hasHeightForWidth());
        txtJobTotalAllDaysTime->setSizePolicy(sizePolicy4);
        txtJobTotalAllDaysTime->setMinimumSize(QSize(150, 0));
        txtJobTotalAllDaysTime->setFrame(false);
        txtJobTotalAllDaysTime->setReadOnly(true);

        gridLayout_4->addWidget(txtJobTotalAllDaysTime, 2, 1, 1, 1);

        txtJobTotalDayTime = new vmLineEdit(grpJobTotalTime);
        txtJobTotalDayTime->setObjectName(QStringLiteral("txtJobTotalDayTime"));
        sizePolicy4.setHeightForWidth(txtJobTotalDayTime->sizePolicy().hasHeightForWidth());
        txtJobTotalDayTime->setSizePolicy(sizePolicy4);
        txtJobTotalDayTime->setMinimumSize(QSize(150, 0));
        txtJobTotalDayTime->setFrame(false);
        txtJobTotalDayTime->setReadOnly(false);

        gridLayout_4->addWidget(txtJobTotalDayTime, 0, 1, 1, 1);


        gLayoutJobExtraInfo->addWidget(grpJobTotalTime, 2, 1, 1, 3);

        lblJobReport = new QLabel(frmJobInfo_2);
        lblJobReport->setObjectName(QStringLiteral("lblJobReport"));

        gLayoutJobExtraInfo->addWidget(lblJobReport, 0, 5, 1, 1);

        lblJobPictures = new QLabel(frmJobInfo_2);
        lblJobPictures->setObjectName(QStringLiteral("lblJobPictures"));

        gLayoutJobExtraInfo->addWidget(lblJobPictures, 7, 0, 1, 1);

        lblJobEndTime = new QLabel(frmJobInfo_2);
        lblJobEndTime->setObjectName(QStringLiteral("lblJobEndTime"));
        lblJobEndTime->setMaximumSize(QSize(16777215, 16777215));

        gLayoutJobExtraInfo->addWidget(lblJobEndTime, 0, 3, 1, 1);

        timeJobEnd = new vmTimeEdit(frmJobInfo_2);
        timeJobEnd->setObjectName(QStringLiteral("timeJobEnd"));

        gLayoutJobExtraInfo->addWidget(timeJobEnd, 1, 3, 1, 1);

        frmJobReportControls = new QFrame(frmJobInfo_2);
        frmJobReportControls->setObjectName(QStringLiteral("frmJobReportControls"));
        frmJobReportControls->setFrameShape(QFrame::NoFrame);
        frmJobReportControls->setFrameShadow(QFrame::Plain);
        horizontalLayout_13 = new QHBoxLayout(frmJobReportControls);
        horizontalLayout_13->setSpacing(2);
        horizontalLayout_13->setObjectName(QStringLiteral("horizontalLayout_13"));
        horizontalLayout_13->setContentsMargins(0, 2, 0, 2);
        btnJobPrevDay = new QToolButton(frmJobReportControls);
        btnJobPrevDay->setObjectName(QStringLiteral("btnJobPrevDay"));
        QIcon icon21;
        icon21.addFile(QStringLiteral(":/resources/browse-controls/prev_rec.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnJobPrevDay->setIcon(icon21);

        horizontalLayout_13->addWidget(btnJobPrevDay);

        btnJobNextDay = new QToolButton(frmJobReportControls);
        btnJobNextDay->setObjectName(QStringLiteral("btnJobNextDay"));
        QIcon icon22;
        icon22.addFile(QStringLiteral(":/resources/browse-controls/next_rec.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnJobNextDay->setIcon(icon22);

        horizontalLayout_13->addWidget(btnJobNextDay);

        line_17 = new QFrame(frmJobReportControls);
        line_17->setObjectName(QStringLiteral("line_17"));
        line_17->setFrameShape(QFrame::VLine);
        line_17->setFrameShadow(QFrame::Sunken);

        horizontalLayout_13->addWidget(line_17);

        btnJobAddDay = new QToolButton(frmJobReportControls);
        btnJobAddDay->setObjectName(QStringLiteral("btnJobAddDay"));
        btnJobAddDay->setIcon(icon15);

        horizontalLayout_13->addWidget(btnJobAddDay);

        dteJobAddDate = new vmDateEdit(frmJobReportControls);
        dteJobAddDate->setObjectName(QStringLiteral("dteJobAddDate"));
        QSizePolicy sizePolicy5(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        sizePolicy5.setHorizontalStretch(0);
        sizePolicy5.setVerticalStretch(0);
        sizePolicy5.setHeightForWidth(dteJobAddDate->sizePolicy().hasHeightForWidth());
        dteJobAddDate->setSizePolicy(sizePolicy5);

        horizontalLayout_13->addWidget(dteJobAddDate);

        line_16 = new QFrame(frmJobReportControls);
        line_16->setObjectName(QStringLiteral("line_16"));
        line_16->setFrameShape(QFrame::VLine);
        line_16->setFrameShadow(QFrame::Sunken);

        horizontalLayout_13->addWidget(line_16);

        btnJobDelDay = new QToolButton(frmJobReportControls);
        btnJobDelDay->setObjectName(QStringLiteral("btnJobDelDay"));
        btnJobDelDay->setIcon(icon9);

        horizontalLayout_13->addWidget(btnJobDelDay);

        btnJobCancelDelDay = new QToolButton(frmJobReportControls);
        btnJobCancelDelDay->setObjectName(QStringLiteral("btnJobCancelDelDay"));
        QIcon icon23;
        icon23.addFile(QStringLiteral(":/resources/cancel_remove.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnJobCancelDelDay->setIcon(icon23);

        horizontalLayout_13->addWidget(btnJobCancelDelDay);


        gLayoutJobExtraInfo->addWidget(frmJobReportControls, 5, 0, 2, 5);

        txtJobReport = new textEditWithCompleter(frmJobInfo_2);
        txtJobReport->setObjectName(QStringLiteral("txtJobReport"));
        QSizePolicy sizePolicy6(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy6.setHorizontalStretch(1);
        sizePolicy6.setVerticalStretch(0);
        sizePolicy6.setHeightForWidth(txtJobReport->sizePolicy().hasHeightForWidth());
        txtJobReport->setSizePolicy(sizePolicy6);

        gLayoutJobExtraInfo->addWidget(txtJobReport, 1, 5, 6, 3);

        timeJobStart = new vmTimeEdit(frmJobInfo_2);
        timeJobStart->setObjectName(QStringLiteral("timeJobStart"));

        gLayoutJobExtraInfo->addWidget(timeJobStart, 1, 1, 1, 1);

        btnJobSeparateReportWindow = new QToolButton(frmJobInfo_2);
        btnJobSeparateReportWindow->setObjectName(QStringLiteral("btnJobSeparateReportWindow"));
        QIcon icon24;
        icon24.addFile(QStringLiteral(":/resources/separate_window.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnJobSeparateReportWindow->setIcon(icon24);
        btnJobSeparateReportWindow->setCheckable(true);

        gLayoutJobExtraInfo->addWidget(btnJobSeparateReportWindow, 0, 7, 1, 1);

        frmJobPicturesControls = new QFrame(frmJobInfo_2);
        frmJobPicturesControls->setObjectName(QStringLiteral("frmJobPicturesControls"));
        frmJobPicturesControls->setFrameShape(QFrame::NoFrame);
        frmJobPicturesControls->setFrameShadow(QFrame::Plain);
        frmJobPicturesControls->setLineWidth(3);
        horizontalLayout_23 = new QHBoxLayout(frmJobPicturesControls);
        horizontalLayout_23->setSpacing(2);
        horizontalLayout_23->setObjectName(QStringLiteral("horizontalLayout_23"));
        horizontalLayout_23->setContentsMargins(2, 2, 2, 2);
        lblJobPicturesCount = new QLabel(frmJobPicturesControls);
        lblJobPicturesCount->setObjectName(QStringLiteral("lblJobPicturesCount"));
        lblJobPicturesCount->setFrameShape(QFrame::NoFrame);
        lblJobPicturesCount->setFrameShadow(QFrame::Plain);

        horizontalLayout_23->addWidget(lblJobPicturesCount);

        cboJobPictures = new vmComboBox(frmJobPicturesControls);
        cboJobPictures->setObjectName(QStringLiteral("cboJobPictures"));
        cboJobPictures->setMinimumSize(QSize(200, 0));
        cboJobPictures->setMaxVisibleItems(10);
        cboJobPictures->setFrame(false);

        horizontalLayout_23->addWidget(cboJobPictures);

        btnJobPrevPicture = new QToolButton(frmJobPicturesControls);
        btnJobPrevPicture->setObjectName(QStringLiteral("btnJobPrevPicture"));
        QIcon icon25;
        icon25.addFile(QStringLiteral(":/resources/arrow-left.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnJobPrevPicture->setIcon(icon25);

        horizontalLayout_23->addWidget(btnJobPrevPicture);

        btnJobNextPicture = new QToolButton(frmJobPicturesControls);
        btnJobNextPicture->setObjectName(QStringLiteral("btnJobNextPicture"));
        QIcon icon26;
        icon26.addFile(QStringLiteral(":/resources/arrow-right.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnJobNextPicture->setIcon(icon26);

        horizontalLayout_23->addWidget(btnJobNextPicture);

        btnJobReloadPictures = new QToolButton(frmJobPicturesControls);
        btnJobReloadPictures->setObjectName(QStringLiteral("btnJobReloadPictures"));
        QIcon icon27;
        icon27.addFile(QStringLiteral(":/resources/reload.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnJobReloadPictures->setIcon(icon27);

        horizontalLayout_23->addWidget(btnJobReloadPictures);

        btnJobRenamePicture = new QToolButton(frmJobPicturesControls);
        btnJobRenamePicture->setObjectName(QStringLiteral("btnJobRenamePicture"));
        QIcon icon28;
        icon28.addFile(QStringLiteral(":/resources/edit-rename.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnJobRenamePicture->setIcon(icon28);
        btnJobRenamePicture->setCheckable(true);

        horizontalLayout_23->addWidget(btnJobRenamePicture);

        line_19 = new QFrame(frmJobPicturesControls);
        line_19->setObjectName(QStringLiteral("line_19"));
        line_19->setFrameShape(QFrame::VLine);
        line_19->setFrameShadow(QFrame::Sunken);

        horizontalLayout_23->addWidget(line_19);

        btnJobOpenPictureEditor = new QToolButton(frmJobPicturesControls);
        btnJobOpenPictureEditor->setObjectName(QStringLiteral("btnJobOpenPictureEditor"));
        QIcon icon29;
        icon29.addFile(QStringLiteral(":/resources/gimp.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnJobOpenPictureEditor->setIcon(icon29);

        horizontalLayout_23->addWidget(btnJobOpenPictureEditor);

        btnJobOpenPictureFolder = new QToolButton(frmJobPicturesControls);
        btnJobOpenPictureFolder->setObjectName(QStringLiteral("btnJobOpenPictureFolder"));
        btnJobOpenPictureFolder->setIcon(icon19);

        horizontalLayout_23->addWidget(btnJobOpenPictureFolder);

        btnJobOpenPictureViewer = new QToolButton(frmJobPicturesControls);
        btnJobOpenPictureViewer->setObjectName(QStringLiteral("btnJobOpenPictureViewer"));
        QIcon icon30;
        icon30.addFile(QStringLiteral(":/resources/gwenview.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnJobOpenPictureViewer->setIcon(icon30);

        horizontalLayout_23->addWidget(btnJobOpenPictureViewer);

        line_21 = new QFrame(frmJobPicturesControls);
        line_21->setObjectName(QStringLiteral("line_21"));
        line_21->setFrameShape(QFrame::VLine);
        line_21->setFrameShadow(QFrame::Sunken);

        horizontalLayout_23->addWidget(line_21);

        btnJobClientsYearPictures = new QToolButton(frmJobPicturesControls);
        btnJobClientsYearPictures->setObjectName(QStringLiteral("btnJobClientsYearPictures"));
        QIcon icon31;
        icon31.addFile(QStringLiteral(":/resources/clients-year-pictures.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnJobClientsYearPictures->setIcon(icon31);

        horizontalLayout_23->addWidget(btnJobClientsYearPictures);

        line_18 = new QFrame(frmJobPicturesControls);
        line_18->setObjectName(QStringLiteral("line_18"));
        line_18->setFrameShape(QFrame::VLine);
        line_18->setFrameShadow(QFrame::Sunken);

        horizontalLayout_23->addWidget(line_18);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_23->addItem(horizontalSpacer);

        btnJobSeparatePicture = new QToolButton(frmJobPicturesControls);
        btnJobSeparatePicture->setObjectName(QStringLiteral("btnJobSeparatePicture"));
        btnJobSeparatePicture->setIcon(icon24);
        btnJobSeparatePicture->setCheckable(true);

        horizontalLayout_23->addWidget(btnJobSeparatePicture);

        horizontalLayout_23->setStretch(1, 2);

        gLayoutJobExtraInfo->addWidget(frmJobPicturesControls, 8, 0, 1, 8);

        hLayoutImgViewer = new QHBoxLayout();
        hLayoutImgViewer->setSpacing(0);
        hLayoutImgViewer->setObjectName(QStringLiteral("hLayoutImgViewer"));
        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hLayoutImgViewer->addItem(horizontalSpacer_5);

        jobImageViewer = new DB_Image(frmJobInfo_2);
        jobImageViewer->setObjectName(QStringLiteral("jobImageViewer"));
        jobImageViewer->setMinimumSize(QSize(300, 300));
        jobImageViewer->setFrameShape(QFrame::NoFrame);
        jobImageViewer->setFrameShadow(QFrame::Plain);
        jobImageViewer->setLineWidth(2);

        hLayoutImgViewer->addWidget(jobImageViewer);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        hLayoutImgViewer->addItem(horizontalSpacer_4);

        hLayoutImgViewer->setStretch(0, 1);
        hLayoutImgViewer->setStretch(1, 2);
        hLayoutImgViewer->setStretch(2, 1);

        gLayoutJobExtraInfo->addLayout(hLayoutImgViewer, 9, 0, 1, 8);

        lblJobStartTime = new QLabel(frmJobInfo_2);
        lblJobStartTime->setObjectName(QStringLiteral("lblJobStartTime"));
        lblJobStartTime->setMaximumSize(QSize(16777215, 16777215));

        gLayoutJobExtraInfo->addWidget(lblJobStartTime, 0, 1, 1, 1);

        txtJobWheather = new vmLineEdit(frmJobInfo_2);
        txtJobWheather->setObjectName(QStringLiteral("txtJobWheather"));

        gLayoutJobExtraInfo->addWidget(txtJobWheather, 4, 1, 1, 3);

        lblWeatherConditions = new QLabel(frmJobInfo_2);
        lblWeatherConditions->setObjectName(QStringLiteral("lblWeatherConditions"));
        lblWeatherConditions->setMaximumSize(QSize(16777215, 16777215));

        gLayoutJobExtraInfo->addWidget(lblWeatherConditions, 3, 1, 1, 1);

        lstJobDayReport = new vmListWidget(frmJobInfo_2);
        lstJobDayReport->setObjectName(QStringLiteral("lstJobDayReport"));
        QSizePolicy sizePolicy7(QSizePolicy::Minimum, QSizePolicy::Expanding);
        sizePolicy7.setHorizontalStretch(0);
        sizePolicy7.setVerticalStretch(0);
        sizePolicy7.setHeightForWidth(lstJobDayReport->sizePolicy().hasHeightForWidth());
        lstJobDayReport->setSizePolicy(sizePolicy7);

        gLayoutJobExtraInfo->addWidget(lstJobDayReport, 0, 0, 5, 1);

        btnJobMachines = new QToolButton(frmJobInfo_2);
        btnJobMachines->setObjectName(QStringLiteral("btnJobMachines"));
        QIcon icon32;
        icon32.addFile(QStringLiteral(":/resources/job_machines.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnJobMachines->setIcon(icon32);

        gLayoutJobExtraInfo->addWidget(btnJobMachines, 0, 6, 1, 1);

        txtJobPicturesPath = new vmLineEdit(frmJobInfo_2);
        txtJobPicturesPath->setObjectName(QStringLiteral("txtJobPicturesPath"));
        txtJobPicturesPath->setFrame(false);

        gLayoutJobExtraInfo->addWidget(txtJobPicturesPath, 7, 1, 1, 7);

        gLayoutJobExtraInfo->setRowStretch(0, 1);

        verticalLayout_27->addWidget(frmJobInfo_2);

        tabMain->addTab(tabDesign3, QString());
        tabDesign_4 = new QWidget();
        tabDesign_4->setObjectName(QStringLiteral("tabDesign_4"));
        splitterBuyInfo = new QSplitter(tabDesign_4);
        splitterBuyInfo->setObjectName(QStringLiteral("splitterBuyInfo"));
        splitterBuyInfo->setGeometry(QRect(7, 7, 631, 585));
        sizePolicy.setHeightForWidth(splitterBuyInfo->sizePolicy().hasHeightForWidth());
        splitterBuyInfo->setSizePolicy(sizePolicy);
        splitterBuyInfo->setMinimumSize(QSize(500, 0));
        splitterBuyInfo->setMaximumSize(QSize(16777215, 16777215));
        splitterBuyInfo->setFrameShape(QFrame::NoFrame);
        splitterBuyInfo->setFrameShadow(QFrame::Plain);
        splitterBuyInfo->setLineWidth(5);
        splitterBuyInfo->setOrientation(Qt::Horizontal);
        frmBuyJobList = new QFrame(splitterBuyInfo);
        frmBuyJobList->setObjectName(QStringLiteral("frmBuyJobList"));
        frmBuyJobList->setMinimumSize(QSize(100, 100));
        frmBuyJobList->setFrameShape(QFrame::NoFrame);
        frmBuyJobList->setFrameShadow(QFrame::Plain);
        verticalLayout_19 = new QVBoxLayout(frmBuyJobList);
        verticalLayout_19->setObjectName(QStringLiteral("verticalLayout_19"));
        verticalLayout_12 = new QVBoxLayout();
        verticalLayout_12->setSpacing(2);
        verticalLayout_12->setObjectName(QStringLiteral("verticalLayout_12"));
        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setSpacing(0);
        horizontalLayout_6->setObjectName(QStringLiteral("horizontalLayout_6"));
        frmBuyToolbar = new QFrame(frmBuyJobList);
        frmBuyToolbar->setObjectName(QStringLiteral("frmBuyToolbar"));
        frmBuyToolbar->setMinimumSize(QSize(0, 30));
        frmBuyToolbar->setFrameShape(QFrame::NoFrame);
        frmBuyToolbar->setFrameShadow(QFrame::Plain);
        frmBuyToolbar->setLineWidth(3);
        verticalLayout_16 = new QVBoxLayout(frmBuyToolbar);
        verticalLayout_16->setSpacing(2);
        verticalLayout_16->setObjectName(QStringLiteral("verticalLayout_16"));
        verticalLayout_16->setContentsMargins(2, 2, 2, 2);
        btnBuyAdd = new QToolButton(frmBuyToolbar);
        btnBuyAdd->setObjectName(QStringLiteral("btnBuyAdd"));
        btnBuyAdd->setIcon(icon15);

        verticalLayout_16->addWidget(btnBuyAdd);

        btnBuyEdit = new QToolButton(frmBuyToolbar);
        btnBuyEdit->setObjectName(QStringLiteral("btnBuyEdit"));
        btnBuyEdit->setIcon(icon8);

        verticalLayout_16->addWidget(btnBuyEdit);

        btnBuyDel = new QToolButton(frmBuyToolbar);
        btnBuyDel->setObjectName(QStringLiteral("btnBuyDel"));
        btnBuyDel->setIcon(icon9);

        verticalLayout_16->addWidget(btnBuyDel);

        btnBuySave = new QToolButton(frmBuyToolbar);
        btnBuySave->setObjectName(QStringLiteral("btnBuySave"));
        btnBuySave->setIcon(icon10);

        verticalLayout_16->addWidget(btnBuySave);

        btnBuyCancel = new QToolButton(frmBuyToolbar);
        btnBuyCancel->setObjectName(QStringLiteral("btnBuyCancel"));
        btnBuyCancel->setIcon(icon11);

        verticalLayout_16->addWidget(btnBuyCancel);

        verticalSpacer_3 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_16->addItem(verticalSpacer_3);


        horizontalLayout_6->addWidget(frmBuyToolbar);

        lblBuySupplier_2 = new QLabel(frmBuyJobList);
        lblBuySupplier_2->setObjectName(QStringLiteral("lblBuySupplier_2"));
        lblBuySupplier_2->setMinimumSize(QSize(0, 0));
        QFont font2;
        font2.setFamily(QStringLiteral("Arial"));
        font2.setPointSize(10);
        font2.setBold(false);
        font2.setWeight(50);
        lblBuySupplier_2->setFont(font2);
        lblBuySupplier_2->setFrameShape(QFrame::StyledPanel);
        lblBuySupplier_2->setFrameShadow(QFrame::Raised);
        lblBuySupplier_2->setTextFormat(Qt::PlainText);

        horizontalLayout_6->addWidget(lblBuySupplier_2);

        cboBuySuppliers = new vmComboBox(frmBuyJobList);
        cboBuySuppliers->setObjectName(QStringLiteral("cboBuySuppliers"));

        horizontalLayout_6->addWidget(cboBuySuppliers);

        btnShowSuppliersDlg = new QToolButton(frmBuyJobList);
        btnShowSuppliersDlg->setObjectName(QStringLiteral("btnShowSuppliersDlg"));
        btnShowSuppliersDlg->setMinimumSize(QSize(30, 30));
        QIcon icon33;
        icon33.addFile(QStringLiteral(":/resources/arrow-down.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnShowSuppliersDlg->setIcon(icon33);
        btnShowSuppliersDlg->setCheckable(false);

        horizontalLayout_6->addWidget(btnShowSuppliersDlg);

        horizontalLayout_6->setStretch(2, 1);

        verticalLayout_12->addLayout(horizontalLayout_6);

        buysJobListWidget = new vmListWidget(frmBuyJobList);
        buysJobListWidget->setObjectName(QStringLiteral("buysJobListWidget"));

        verticalLayout_12->addWidget(buysJobListWidget);

        lblBuySupplierBuys = new QLabel(frmBuyJobList);
        lblBuySupplierBuys->setObjectName(QStringLiteral("lblBuySupplierBuys"));

        verticalLayout_12->addWidget(lblBuySupplierBuys);

        lstBuySuppliers = new vmListWidget(frmBuyJobList);
        lstBuySuppliers->setObjectName(QStringLiteral("lstBuySuppliers"));

        verticalLayout_12->addWidget(lstBuySuppliers);


        verticalLayout_19->addLayout(verticalLayout_12);

        splitterBuyInfo->addWidget(frmBuyJobList);
        frmBuyInfo = new QFrame(splitterBuyInfo);
        frmBuyInfo->setObjectName(QStringLiteral("frmBuyInfo"));
        QSizePolicy sizePolicy8(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy8.setHorizontalStretch(0);
        sizePolicy8.setVerticalStretch(0);
        sizePolicy8.setHeightForWidth(frmBuyInfo->sizePolicy().hasHeightForWidth());
        frmBuyInfo->setSizePolicy(sizePolicy8);
        frmBuyInfo->setMinimumSize(QSize(300, 100));
        frmBuyInfo->setFrameShape(QFrame::NoFrame);
        frmBuyInfo->setFrameShadow(QFrame::Plain);
        horizontalLayout = new QHBoxLayout(frmBuyInfo);
        horizontalLayout->setSpacing(2);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setContentsMargins(2, 2, 2, 2);
        gLayoutBuyForms = new QGridLayout();
        gLayoutBuyForms->setSpacing(2);
        gLayoutBuyForms->setObjectName(QStringLiteral("gLayoutBuyForms"));
        gLayoutBuyForms->setContentsMargins(2, 2, 2, 2);
        txtBuyTotalPrice = new vmLineEdit(frmBuyInfo);
        txtBuyTotalPrice->setObjectName(QStringLiteral("txtBuyTotalPrice"));

        gLayoutBuyForms->addWidget(txtBuyTotalPrice, 9, 3, 1, 1);

        btnBuyCopyRows = new QToolButton(frmBuyInfo);
        btnBuyCopyRows->setObjectName(QStringLiteral("btnBuyCopyRows"));
        btnBuyCopyRows->setEnabled(false);
        btnBuyCopyRows->setMinimumSize(QSize(0, 0));
        btnBuyCopyRows->setLayoutDirection(Qt::LeftToRight);
        QIcon icon34;
        icon34.addFile(QStringLiteral(":/resources/copy_buy_to_qp.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnBuyCopyRows->setIcon(icon34);

        gLayoutBuyForms->addWidget(btnBuyCopyRows, 6, 3, 1, 1);

        lblBuyID_2 = new QLabel(frmBuyInfo);
        lblBuyID_2->setObjectName(QStringLiteral("lblBuyID_2"));
        lblBuyID_2->setMaximumSize(QSize(50, 30));
        lblBuyID_2->setFont(font2);
        lblBuyID_2->setFrameShape(QFrame::NoFrame);
        lblBuyID_2->setFrameShadow(QFrame::Plain);
        lblBuyID_2->setTextFormat(Qt::PlainText);

        gLayoutBuyForms->addWidget(lblBuyID_2, 0, 3, 1, 1);

        txtBuyNotes = new vmLineEdit(frmBuyInfo);
        txtBuyNotes->setObjectName(QStringLiteral("txtBuyNotes"));
        txtBuyNotes->setMinimumSize(QSize(0, 30));

        gLayoutBuyForms->addWidget(txtBuyNotes, 5, 0, 1, 4);

        dteBuyDate = new vmDateEdit(frmBuyInfo);
        dteBuyDate->setObjectName(QStringLiteral("dteBuyDate"));

        gLayoutBuyForms->addWidget(dteBuyDate, 1, 0, 1, 1);

        txtBuyDeliveryMethod = new vmLineEdit(frmBuyInfo);
        txtBuyDeliveryMethod->setObjectName(QStringLiteral("txtBuyDeliveryMethod"));
        txtBuyDeliveryMethod->setMinimumSize(QSize(0, 30));

        gLayoutBuyForms->addWidget(txtBuyDeliveryMethod, 3, 1, 1, 3);

        dteBuyDeliveryDate = new vmDateEdit(frmBuyInfo);
        dteBuyDeliveryDate->setObjectName(QStringLiteral("dteBuyDeliveryDate"));

        gLayoutBuyForms->addWidget(dteBuyDeliveryDate, 3, 0, 1, 1);

        lblBuyPayments = new QLabel(frmBuyInfo);
        lblBuyPayments->setObjectName(QStringLiteral("lblBuyPayments"));

        gLayoutBuyForms->addWidget(lblBuyPayments, 6, 0, 1, 1);

        txtBuyID = new vmLineEdit(frmBuyInfo);
        txtBuyID->setObjectName(QStringLiteral("txtBuyID"));
        txtBuyID->setMinimumSize(QSize(0, 30));
        txtBuyID->setMaximumSize(QSize(120, 30));
        txtBuyID->setFocusPolicy(Qt::NoFocus);
        txtBuyID->setReadOnly(true);

        gLayoutBuyForms->addWidget(txtBuyID, 1, 3, 1, 1);

        tableBuyItems = new vmTableWidget(frmBuyInfo);
        tableBuyItems->setObjectName(QStringLiteral("tableBuyItems"));
        tableBuyItems->setMinimumSize(QSize(500, 0));
        tableBuyItems->setFrameShape(QFrame::StyledPanel);
        tableBuyItems->setFrameShadow(QFrame::Raised);

        gLayoutBuyForms->addWidget(tableBuyItems, 8, 0, 1, 4);

        lblBuyItems = new QLabel(frmBuyInfo);
        lblBuyItems->setObjectName(QStringLiteral("lblBuyItems"));

        gLayoutBuyForms->addWidget(lblBuyItems, 10, 0, 1, 1);

        lblBuyDeliverDate = new QLabel(frmBuyInfo);
        lblBuyDeliverDate->setObjectName(QStringLiteral("lblBuyDeliverDate"));
        lblBuyDeliverDate->setMaximumSize(QSize(16777215, 40));
        lblBuyDeliverDate->setFrameShape(QFrame::NoFrame);
        lblBuyDeliverDate->setFrameShadow(QFrame::Plain);
        lblBuyDeliverDate->setTextFormat(Qt::PlainText);

        gLayoutBuyForms->addWidget(lblBuyDeliverDate, 2, 0, 1, 1);

        lblBuyNotes_2 = new QLabel(frmBuyInfo);
        lblBuyNotes_2->setObjectName(QStringLiteral("lblBuyNotes_2"));
        lblBuyNotes_2->setMaximumSize(QSize(16777215, 30));

        gLayoutBuyForms->addWidget(lblBuyNotes_2, 4, 0, 1, 1);

        tableBuyPayments = new vmTableWidget(frmBuyInfo);
        tableBuyPayments->setObjectName(QStringLiteral("tableBuyPayments"));
        tableBuyPayments->setMinimumSize(QSize(500, 0));
        tableBuyPayments->setFrameShape(QFrame::StyledPanel);
        tableBuyPayments->setFrameShadow(QFrame::Raised);

        gLayoutBuyForms->addWidget(tableBuyPayments, 12, 0, 1, 4);

        widget_3 = new QWidget(frmBuyInfo);
        widget_3->setObjectName(QStringLiteral("widget_3"));

        gLayoutBuyForms->addWidget(widget_3, 11, 0, 1, 1);

        widget_2 = new QWidget(frmBuyInfo);
        widget_2->setObjectName(QStringLiteral("widget_2"));

        gLayoutBuyForms->addWidget(widget_2, 7, 0, 1, 1);

        lblBuyTotalPrice = new QLabel(frmBuyInfo);
        lblBuyTotalPrice->setObjectName(QStringLiteral("lblBuyTotalPrice"));

        gLayoutBuyForms->addWidget(lblBuyTotalPrice, 9, 0, 1, 1);

        lblBuyTotalPaid = new QLabel(frmBuyInfo);
        lblBuyTotalPaid->setObjectName(QStringLiteral("lblBuyTotalPaid"));

        gLayoutBuyForms->addWidget(lblBuyTotalPaid, 13, 0, 1, 1);

        lblBuyDeliverMethod_2 = new QLabel(frmBuyInfo);
        lblBuyDeliverMethod_2->setObjectName(QStringLiteral("lblBuyDeliverMethod_2"));
        lblBuyDeliverMethod_2->setMinimumSize(QSize(0, 30));
        lblBuyDeliverMethod_2->setMaximumSize(QSize(16777215, 40));
        lblBuyDeliverMethod_2->setFrameShape(QFrame::NoFrame);
        lblBuyDeliverMethod_2->setFrameShadow(QFrame::Plain);
        lblBuyDeliverMethod_2->setTextFormat(Qt::PlainText);

        gLayoutBuyForms->addWidget(lblBuyDeliverMethod_2, 2, 1, 1, 2);

        txtBuyTotalPaid = new vmLineEditWithButton(frmBuyInfo);
        txtBuyTotalPaid->setObjectName(QStringLiteral("txtBuyTotalPaid"));
        txtBuyTotalPaid->setInputMethodHints(Qt::ImhFormattedNumbersOnly);

        gLayoutBuyForms->addWidget(txtBuyTotalPaid, 13, 3, 1, 1);

        lblBuyDate_2 = new QLabel(frmBuyInfo);
        lblBuyDate_2->setObjectName(QStringLiteral("lblBuyDate_2"));
        lblBuyDate_2->setMaximumSize(QSize(16777215, 40));
        lblBuyDate_2->setFrameShape(QFrame::NoFrame);
        lblBuyDate_2->setFrameShadow(QFrame::Plain);
        lblBuyDate_2->setTextFormat(Qt::PlainText);

        gLayoutBuyForms->addWidget(lblBuyDate_2, 0, 0, 1, 3);

        gLayoutBuyForms->setColumnStretch(0, 1);

        horizontalLayout->addLayout(gLayoutBuyForms);

        splitterBuyInfo->addWidget(frmBuyInfo);
        tabMain->addTab(tabDesign_4, QString());
        splitterMain->addWidget(tabMain);

        gridLayout->addWidget(splitterMain, 1, 0, 1, 4);

        horizontalLayout_8 = new QHBoxLayout();
        horizontalLayout_8->setSpacing(5);
        horizontalLayout_8->setObjectName(QStringLiteral("horizontalLayout_8"));
        horizontalLayout_8->setContentsMargins(-1, -1, 0, -1);
        lblCurInfoClient = new QLabel(centralwidget);
        lblCurInfoClient->setObjectName(QStringLiteral("lblCurInfoClient"));

        horizontalLayout_8->addWidget(lblCurInfoClient);

        line_2 = new QFrame(centralwidget);
        line_2->setObjectName(QStringLiteral("line_2"));
        line_2->setFrameShape(QFrame::VLine);
        line_2->setFrameShadow(QFrame::Sunken);

        horizontalLayout_8->addWidget(line_2);

        lblCurInfoJob = new QLabel(centralwidget);
        lblCurInfoJob->setObjectName(QStringLiteral("lblCurInfoJob"));

        horizontalLayout_8->addWidget(lblCurInfoJob);

        line_3 = new QFrame(centralwidget);
        line_3->setObjectName(QStringLiteral("line_3"));
        line_3->setFrameShape(QFrame::VLine);
        line_3->setFrameShadow(QFrame::Sunken);

        horizontalLayout_8->addWidget(line_3);

        lblCurInfoPay = new QLabel(centralwidget);
        lblCurInfoPay->setObjectName(QStringLiteral("lblCurInfoPay"));

        horizontalLayout_8->addWidget(lblCurInfoPay);

        line_4 = new QFrame(centralwidget);
        line_4->setObjectName(QStringLiteral("line_4"));
        line_4->setFrameShape(QFrame::VLine);
        line_4->setFrameShadow(QFrame::Sunken);

        horizontalLayout_8->addWidget(line_4);

        lblCurInfoBuy = new QLabel(centralwidget);
        lblCurInfoBuy->setObjectName(QStringLiteral("lblCurInfoBuy"));

        horizontalLayout_8->addWidget(lblCurInfoBuy);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_8->addItem(horizontalSpacer_2);

        line_5 = new QFrame(centralwidget);
        line_5->setObjectName(QStringLiteral("line_5"));
        line_5->setFrameShape(QFrame::VLine);
        line_5->setFrameShadow(QFrame::Sunken);

        horizontalLayout_8->addWidget(line_5);

        lblFastSearch = new QLabel(centralwidget);
        lblFastSearch->setObjectName(QStringLiteral("lblFastSearch"));
        sizePolicy2.setHeightForWidth(lblFastSearch->sizePolicy().hasHeightForWidth());
        lblFastSearch->setSizePolicy(sizePolicy2);
        lblFastSearch->setFrameShape(QFrame::NoFrame);
        lblFastSearch->setFrameShadow(QFrame::Plain);
        lblFastSearch->setTextFormat(Qt::PlainText);

        horizontalLayout_8->addWidget(lblFastSearch);

        txtQuickSearch = new vmLineEdit(centralwidget);
        txtQuickSearch->setObjectName(QStringLiteral("txtQuickSearch"));
        txtQuickSearch->setMinimumSize(QSize(200, 30));

        horizontalLayout_8->addWidget(txtQuickSearch);

        btnSearchStart = new QToolButton(centralwidget);
        btnSearchStart->setObjectName(QStringLiteral("btnSearchStart"));
        btnSearchStart->setEnabled(false);
        btnSearchStart->setMinimumSize(QSize(30, 30));
        QIcon icon35;
        icon35.addFile(QStringLiteral(":/resources/search.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnSearchStart->setIcon(icon35);
        btnSearchStart->setCheckable(false);

        horizontalLayout_8->addWidget(btnSearchStart);

        btnSearchCancel = new QToolButton(centralwidget);
        btnSearchCancel->setObjectName(QStringLiteral("btnSearchCancel"));
        btnSearchCancel->setEnabled(false);
        btnSearchCancel->setMinimumSize(QSize(30, 30));
        btnSearchCancel->setIcon(icon11);

        horizontalLayout_8->addWidget(btnSearchCancel);


        gridLayout->addLayout(horizontalLayout_8, 0, 1, 1, 1);

        MainWindow->setCentralWidget(centralwidget);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QStringLiteral("statusbar"));
        QFont font3;
        font3.setPointSize(11);
        font3.setBold(true);
        font3.setItalic(true);
        font3.setWeight(75);
        statusbar->setFont(font3);
        MainWindow->setStatusBar(statusbar);
        QWidget::setTabOrder(txtClientNumberAddress, txtClientCity);
        QWidget::setTabOrder(txtClientCity, txtClientZipCode);
        QWidget::setTabOrder(txtClientZipCode, chkClientActive);
        QWidget::setTabOrder(chkClientActive, btnPayInfoDel);
        QWidget::setTabOrder(btnPayInfoDel, btnPayInfoSave);
        QWidget::setTabOrder(btnPayInfoSave, btnPayInfoCancel);
        QWidget::setTabOrder(btnPayInfoCancel, btnPayPayReceipt);
        QWidget::setTabOrder(btnPayPayReceipt, btnPayPaymentsReportOnlyUnPaid);
        QWidget::setTabOrder(btnPayPaymentsReportOnlyUnPaid, btnPayPaymentsReport);
        QWidget::setTabOrder(btnPayPaymentsReport, txtPayTotalPrice);
        QWidget::setTabOrder(txtPayTotalPrice, txtPayTotalPaid);
        QWidget::setTabOrder(txtPayTotalPaid, txtPayTotalPayments);
        QWidget::setTabOrder(txtPayTotalPayments, txtPayObs);
        QWidget::setTabOrder(txtPayObs, chkPayOverdue);
        QWidget::setTabOrder(chkPayOverdue, btnJobAdd);
        QWidget::setTabOrder(btnJobAdd, btnJobEdit);
        QWidget::setTabOrder(btnJobEdit, btnJobDel);
        QWidget::setTabOrder(btnJobDel, btnJobSave);
        QWidget::setTabOrder(btnJobSave, btnJobCancel);
        QWidget::setTabOrder(btnJobCancel, cboJobType);
        QWidget::setTabOrder(cboJobType, btnNewProject);
        QWidget::setTabOrder(btnNewProject, btnJobOpenFileFolder);
        QWidget::setTabOrder(btnJobOpenFileFolder, btnJobOpenDoc);
        QWidget::setTabOrder(btnJobOpenDoc, btnJobOpenXls);
        QWidget::setTabOrder(btnJobOpenXls, btnJobOpenPdf);
        QWidget::setTabOrder(btnJobOpenPdf, txtJobProjectPath);
        QWidget::setTabOrder(txtJobProjectPath, txtJobProjectID);
        QWidget::setTabOrder(txtJobProjectID, btnQuickProject);
        QWidget::setTabOrder(btnQuickProject, btnJobSelectJob);
        QWidget::setTabOrder(btnJobSelectJob, txtJobPrice);
        QWidget::setTabOrder(txtJobPrice, btnJobPrevDay);
        QWidget::setTabOrder(btnJobPrevDay, btnJobNextDay);
        QWidget::setTabOrder(btnJobNextDay, btnJobAddDay);
        QWidget::setTabOrder(btnJobAddDay, btnJobDelDay);
        QWidget::setTabOrder(btnJobDelDay, btnJobCancelDelDay);
        QWidget::setTabOrder(btnJobCancelDelDay, lstJobDayReport);
        QWidget::setTabOrder(lstJobDayReport, timeJobStart);
        QWidget::setTabOrder(timeJobStart, timeJobEnd);
        QWidget::setTabOrder(timeJobEnd, txtJobTotalDayTime);
        QWidget::setTabOrder(txtJobTotalDayTime, txtJobTotalAllDaysTime);
        QWidget::setTabOrder(txtJobTotalAllDaysTime, txtJobWheather);
        QWidget::setTabOrder(txtJobWheather, txtJobReport);
        QWidget::setTabOrder(txtJobReport, btnJobMachines);
        QWidget::setTabOrder(btnJobMachines, btnJobSeparateReportWindow);
        QWidget::setTabOrder(btnJobSeparateReportWindow, txtJobPicturesPath);
        QWidget::setTabOrder(txtJobPicturesPath, btnJobPrevPicture);
        QWidget::setTabOrder(btnJobPrevPicture, btnJobNextPicture);
        QWidget::setTabOrder(btnJobNextPicture, btnJobOpenPictureEditor);
        QWidget::setTabOrder(btnJobOpenPictureEditor, btnJobOpenPictureFolder);
        QWidget::setTabOrder(btnJobOpenPictureFolder, btnJobOpenPictureViewer);
        QWidget::setTabOrder(btnJobOpenPictureViewer, btnJobClientsYearPictures);
        QWidget::setTabOrder(btnJobClientsYearPictures, btnJobSeparatePicture);
        QWidget::setTabOrder(btnJobSeparatePicture, txtBuyDeliveryMethod);
        QWidget::setTabOrder(txtBuyDeliveryMethod, txtBuyNotes);
        QWidget::setTabOrder(txtBuyNotes, btnBuyCopyRows);
        QWidget::setTabOrder(btnBuyCopyRows, txtBuyTotalPrice);
        QWidget::setTabOrder(txtBuyTotalPrice, txtCalPriceBuyMonth);
        QWidget::setTabOrder(txtCalPriceBuyMonth, btnReportGenerator);
        QWidget::setTabOrder(btnReportGenerator, btnBackupRestore);
        QWidget::setTabOrder(btnBackupRestore, txtCalPricePayDay);
        QWidget::setTabOrder(txtCalPricePayDay, btnCalculator);
        QWidget::setTabOrder(btnCalculator, btnServicesPrices);
        QWidget::setTabOrder(btnServicesPrices, btnEstimates);
        QWidget::setTabOrder(btnEstimates, btnCompanyPurchases);
        QWidget::setTabOrder(btnCompanyPurchases, btnConfiguration);
        QWidget::setTabOrder(btnConfiguration, btnExitProgram);
        QWidget::setTabOrder(btnExitProgram, scrollLeftPanel);
        QWidget::setTabOrder(scrollLeftPanel, tabMain);
        QWidget::setTabOrder(tabMain, txtCalPriceBuyDay);
        QWidget::setTabOrder(txtCalPriceBuyDay, txtCalPriceJobMonth);
        QWidget::setTabOrder(txtCalPriceJobMonth, lstCalJobsDay);
        QWidget::setTabOrder(lstCalJobsDay, txtPayID);
        QWidget::setTabOrder(txtPayID, lstCalBuysDay);
        QWidget::setTabOrder(lstCalBuysDay, lstCalBuysWeek);
        QWidget::setTabOrder(lstCalBuysWeek, txtCalPricePayMonth);
        QWidget::setTabOrder(txtCalPricePayMonth, lstCalJobsMonth);
        QWidget::setTabOrder(lstCalJobsMonth, txtCalPriceJobDay);
        QWidget::setTabOrder(txtCalPriceJobDay, calMain);
        QWidget::setTabOrder(calMain, txtCalPricePayWeek);
        QWidget::setTabOrder(txtCalPricePayWeek, txtCalPriceJobWeek);
        QWidget::setTabOrder(txtCalPriceJobWeek, lstCalJobsWeek);
        QWidget::setTabOrder(lstCalJobsWeek, lstCalPaysWeek);
        QWidget::setTabOrder(lstCalPaysWeek, lstCalBuysMonth);
        QWidget::setTabOrder(lstCalBuysMonth, scrollWorkFlow);
        QWidget::setTabOrder(scrollWorkFlow, lstCalPaysDay);
        QWidget::setTabOrder(lstCalPaysDay, lstCalPaysMonth);
        QWidget::setTabOrder(lstCalPaysMonth, txtCalPriceBuyWeek);

        retranslateUi(MainWindow);

        tabMain->setCurrentIndex(7);
        tboxCalJobs->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
#ifndef QT_NO_TOOLTIP
        btnReportGenerator->setToolTip(QApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Ubuntu'; font-size:11pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Tahoma'; font-weight:600;\">Documents Editor</span><span style=\" font-family:'Sans Serif'; font-weight:600;\"> </span><span style=\" font-family:'Tahoma'; font-weight:600; font-style:italic;\">(F5)</span></p></body></html>", 0));
#endif // QT_NO_TOOLTIP
        btnReportGenerator->setText(QApplication::translate("MainWindow", "...", 0));
#ifndef QT_NO_TOOLTIP
        btnBackupRestore->setToolTip(QApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Ubuntu'; font-size:11pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Tahoma'; font-weight:600;\">Open backup/restore dialog </span><span style=\" font-family:'Tahoma'; font-weight:600; font-style:italic;\">(F6)</span></p></body></html>", 0));
#endif // QT_NO_TOOLTIP
        btnBackupRestore->setText(QApplication::translate("MainWindow", "...", 0));
#ifndef QT_NO_TOOLTIP
        btnCalculator->setToolTip(QApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Ubuntu'; font-size:11pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Tahoma'; font-weight:600;\">Calculator </span><span style=\" font-family:'Tahoma'; font-weight:600; font-style:italic;\">(F7)</span></p></body></html>", 0));
#endif // QT_NO_TOOLTIP
        btnCalculator->setText(QApplication::translate("MainWindow", "...", 0));
#ifndef QT_NO_TOOLTIP
        btnServicesPrices->setToolTip(QApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Ubuntu'; font-size:11pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Tahoma'; font-weight:600;\">Services Prices and Descriptions </span><span style=\" font-family:'Tahoma'; font-weight:600; font-style:italic;\">(F8)</span></p></body></html>", 0));
#endif // QT_NO_TOOLTIP
        btnServicesPrices->setText(QApplication::translate("MainWindow", "...", 0));
#ifndef QT_NO_TOOLTIP
        btnEstimates->setToolTip(QApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Ubuntu'; font-size:11pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Tahoma'; font-weight:600;\">Open the Estimatives and Reports Dialog </span><span style=\" font-family:'Tahoma'; font-weight:600; font-style:italic;\">(F9)</span></p></body></html>", 0));
#endif // QT_NO_TOOLTIP
        btnEstimates->setText(QApplication::translate("MainWindow", "...", 0));
#ifndef QT_NO_TOOLTIP
        btnCompanyPurchases->setToolTip(QApplication::translate("MainWindow", "<html><head/><body><p><span style=\" font-family:'Tahoma'; font-weight:600;\">Open the Company Purchases Log </span><span style=\" font-family:'Tahoma'; font-weight:600; font-style:italic;\">(F10)</span></p></body></html>", 0));
#endif // QT_NO_TOOLTIP
        btnCompanyPurchases->setText(QApplication::translate("MainWindow", "...", 0));
#ifndef QT_NO_TOOLTIP
        btnConfiguration->setToolTip(QApplication::translate("MainWindow", "<html><head/><body><p><span style=\" font-weight:600;\">Open Configuration Dialog </span><span style=\" font-weight:600; font-style:italic;\">(F11)</span></p></body></html>", 0));
#endif // QT_NO_TOOLTIP
        btnConfiguration->setText(QApplication::translate("MainWindow", "...", 0));
#ifndef QT_NO_TOOLTIP
        btnExitProgram->setToolTip(QApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Ubuntu'; font-size:11pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Tahoma'; font-weight:600;\">Exit program </span><span style=\" font-family:'Tahoma'; font-weight:600; font-style:italic;\">(F12)</span></p></body></html>", 0));
#endif // QT_NO_TOOLTIP
        btnExitProgram->setText(QApplication::translate("MainWindow", "...", 0));
#ifndef QT_NO_TOOLTIP
        tabMain->setToolTip(QString());
#endif // QT_NO_TOOLTIP
        tabMain->setTabText(tabMain->indexOf(tabWorkFlow), QApplication::translate("MainWindow", "Workflow", 0));
#ifndef QT_NO_ACCESSIBILITY
        calMain->setAccessibleName(QApplication::translate("MainWindow", "calMain", 0));
#endif // QT_NO_ACCESSIBILITY
        lblCalPriceJobDay->setText(QApplication::translate("MainWindow", "Scheduled income for the day:", 0));
        tboxCalJobs->setItemText(tboxCalJobs->indexOf(tboxPageJobDay), QApplication::translate("MainWindow", "Jobs", 0));
        lblCalPriceJobWeek->setText(QApplication::translate("MainWindow", "Scheduled income for the week:", 0));
        tboxCalJobs->setItemText(tboxCalJobs->indexOf(tboxPageJobWeek), QApplication::translate("MainWindow", "Jobs", 0));
        lblCalPriceJobMonth->setText(QApplication::translate("MainWindow", "Scheduled income for the month:", 0));
        tboxCalJobs->setItemText(tboxCalJobs->indexOf(tboxPageJobMonth), QApplication::translate("MainWindow", "Jobs", 0));
        lblCalPricePayDay->setText(QApplication::translate("MainWindow", "Actual income for the day:", 0));
        tboxCalPays->setItemText(tboxCalPays->indexOf(tboxPagePayDay), QApplication::translate("MainWindow", "Payment", 0));
        lblCalPricePayWeek->setText(QApplication::translate("MainWindow", "Actual income for the week:", 0));
        tboxCalPays->setItemText(tboxCalPays->indexOf(tboxPagePayWeek), QApplication::translate("MainWindow", "Payment", 0));
        lblCalPricePayMonth->setText(QApplication::translate("MainWindow", "Actual income for the month:", 0));
        tboxCalPays->setItemText(tboxCalPays->indexOf(tboxPagePayMonth), QApplication::translate("MainWindow", "Payment", 0));
        lblCalPriceBuyDay->setText(QApplication::translate("MainWindow", "Purchases for the day:", 0));
        tboxCalBuys->setItemText(tboxCalBuys->indexOf(tboxPageBuyDay), QApplication::translate("MainWindow", "Purchase", 0));
        lblCalPriceBuyWeek->setText(QApplication::translate("MainWindow", "Purchases for the week:", 0));
        tboxCalBuys->setItemText(tboxCalBuys->indexOf(tboxPageBuyWeek), QApplication::translate("MainWindow", "Purchase", 0));
        lblCalPriceBuyMonth->setText(QApplication::translate("MainWindow", "Purchases for the month:", 0));
        tboxCalBuys->setItemText(tboxCalBuys->indexOf(tboxPageBuyMonth), QApplication::translate("MainWindow", "Purchase", 0));
        tabMain->setTabText(tabMain->indexOf(tabCalendar), QApplication::translate("MainWindow", "Calendar", 0));
        tabMain->setTabText(tabMain->indexOf(tabInventory), QApplication::translate("MainWindow", "&Inventory", 0));
        tabMain->setTabText(tabMain->indexOf(tabSupplies), QApplication::translate("MainWindow", "S&upplies", 0));
#ifndef QT_NO_TOOLTIP
        btnPayInfoEdit->setToolTip(QApplication::translate("MainWindow", "Edit currently selected payment record", 0));
#endif // QT_NO_TOOLTIP
        btnPayInfoEdit->setText(QApplication::translate("MainWindow", "...", 0));
#ifndef QT_NO_TOOLTIP
        btnPayInfoDel->setToolTip(QApplication::translate("MainWindow", "Remove current payment day", 0));
#endif // QT_NO_TOOLTIP
        btnPayInfoDel->setText(QApplication::translate("MainWindow", "...", 0));
#ifndef QT_NO_TOOLTIP
        btnPayInfoSave->setToolTip(QApplication::translate("MainWindow", "Save payment record", 0));
#endif // QT_NO_TOOLTIP
        btnPayInfoSave->setText(QApplication::translate("MainWindow", "...", 0));
#ifndef QT_NO_TOOLTIP
        btnPayInfoCancel->setToolTip(QApplication::translate("MainWindow", "Cancel current payment operation", 0));
#endif // QT_NO_TOOLTIP
        btnPayInfoCancel->setText(QApplication::translate("MainWindow", "...", 0));
#ifndef QT_NO_TOOLTIP
        btnPayPayReceipt->setToolTip(QApplication::translate("MainWindow", "Generate a Payment Stub for the current viewing payment", 0));
#endif // QT_NO_TOOLTIP
        btnPayPayReceipt->setText(QApplication::translate("MainWindow", "...", 0));
#ifndef QT_NO_TOOLTIP
        btnPayPaymentsReportOnlyUnPaid->setToolTip(QApplication::translate("MainWindow", "<html><head/><body><p>Generate a simple Payments Report for all this client's payments, including only unpaid payments</p></body></html>", 0));
#endif // QT_NO_TOOLTIP
        btnPayPaymentsReportOnlyUnPaid->setText(QApplication::translate("MainWindow", "...", 0));
#ifndef QT_NO_TOOLTIP
        btnPayPaymentsReport->setToolTip(QApplication::translate("MainWindow", "<html><head/><body><p>Generate a complete Payments Report for all this client's payments, both paid and unpaid</p></body></html>", 0));
#endif // QT_NO_TOOLTIP
        btnPayPaymentsReport->setText(QApplication::translate("MainWindow", "...", 0));
        lblTotalPayments->setText(QApplication::translate("MainWindow", "Pays", 0));
        lblPayObs->setText(QApplication::translate("MainWindow", "Observations:", 0));
        lblPayID->setText(QApplication::translate("MainWindow", "ID:", 0));
        chkPayOverdue->setText(QString());
        btnClientAdd->setText(QApplication::translate("MainWindow", "...", 0));
        btnClientEdit->setText(QApplication::translate("MainWindow", "...", 0));
        btnClientDel->setText(QApplication::translate("MainWindow", "...", 0));
        btnClientSave->setText(QApplication::translate("MainWindow", "...", 0));
        btnClientCancel->setText(QApplication::translate("MainWindow", "...", 0));
        lblPayTotalPaid->setText(QApplication::translate("MainWindow", "Total paid:", 0));
        lblTotalPay->setText(QApplication::translate("MainWindow", "Total price:", 0));
        lblClientID->setText(QApplication::translate("MainWindow", "ID:", 0));
        lblClientPhones->setText(QApplication::translate("MainWindow", "Telephones:", 0));
        lblClientCity->setText(QApplication::translate("MainWindow", "City:", 0));
        lblClientDateFrom->setText(QApplication::translate("MainWindow", "Client since:", 0));
        dteClientDateFrom->setProperty("displayFormat", QVariant(QApplication::translate("MainWindow", "dd/MM/yyyy", 0)));
        lblClientDateTo->setText(QApplication::translate("MainWindow", "Client through:", 0));
        dteClientDateTo->setProperty("displayFormat", QVariant(QApplication::translate("MainWindow", "dd/MM/yyyy", 0)));
        chkClientActive->setText(QApplication::translate("MainWindow", "Active?", 0));
        lblClientDistrict->setText(QApplication::translate("MainWindow", "District:", 0));
        lblClientName->setText(QApplication::translate("MainWindow", "Name:", 0));
        lblClientStreetAdress->setText(QApplication::translate("MainWindow", "Street:", 0));
        lblClientEmail->setText(QApplication::translate("MainWindow", "e-mail/site:", 0));
        lblClientZipCode->setText(QApplication::translate("MainWindow", "ZIP Code:", 0));
        lblClientNumberAdress->setText(QApplication::translate("MainWindow", "Number:", 0));
        tabMain->setTabText(tabMain->indexOf(tabDesign_1), QApplication::translate("MainWindow", "Design1", 0));
        lblJobAddress->setText(QApplication::translate("MainWindow", "Location:", 0));
#ifndef QT_NO_TOOLTIP
        btnJobAdd->setToolTip(QApplication::translate("MainWindow", "Insert new job information", 0));
#endif // QT_NO_TOOLTIP
        btnJobAdd->setText(QApplication::translate("MainWindow", "...", 0));
#ifndef QT_NO_TOOLTIP
        btnJobEdit->setToolTip(QApplication::translate("MainWindow", "Edit current job info", 0));
#endif // QT_NO_TOOLTIP
        btnJobEdit->setText(QApplication::translate("MainWindow", "...", 0));
#ifndef QT_NO_TOOLTIP
        btnJobDel->setToolTip(QApplication::translate("MainWindow", "Remove current job info from data base", 0));
#endif // QT_NO_TOOLTIP
        btnJobDel->setText(QApplication::translate("MainWindow", "...", 0));
#ifndef QT_NO_TOOLTIP
        btnJobSave->setToolTip(QApplication::translate("MainWindow", "Save alterations to job info", 0));
#endif // QT_NO_TOOLTIP
        btnJobSave->setText(QApplication::translate("MainWindow", "...", 0));
#ifndef QT_NO_TOOLTIP
        btnJobCancel->setToolTip(QApplication::translate("MainWindow", "Cancel alterations to job info", 0));
#endif // QT_NO_TOOLTIP
        btnJobCancel->setText(QApplication::translate("MainWindow", "...", 0));
#ifndef QT_NO_TOOLTIP
        btnJobSelectJob->setToolTip(QApplication::translate("MainWindow", "SELECT THIS JOB", 0));
#endif // QT_NO_TOOLTIP
        btnJobSelectJob->setText(QApplication::translate("MainWindow", "...", 0));
        lblJobProjectID->setText(QApplication::translate("MainWindow", "Project ID:", 0));
#ifndef QT_NO_TOOLTIP
        btnNewProject->setToolTip(QApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Tahoma'; font-size:10pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">Create a new project folder</p>\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">-&gt; New project folder </span></p>\n"
"<p align=\"justify\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">    Will create a new folder in the client's"
                        " folder. You will be asked for a project name, which will be part of the new folder name, along with this job's start date. To the folder it will be copied a single instance of the MS Word document for the project's description and the equivalent MS Excel spreadsheet. It will alse be created a folder called Pictures for the job's pictures.</p>\n"
"<p style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">-&gt; Add a new document to folder </span></p>\n"
"<p style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">    Will copy another MS Word file to the current project folder, if it exists</p>\n"
"<p style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">-&gt; Add a new spredsheet to folder </span></p>\n"
"<p style=\" margin-top:12px; margin-bottom:12px"
                        "; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">    Will copy another MS Excel file to the current project folder, if it exists</p></body></html>", 0));
#endif // QT_NO_TOOLTIP
        btnNewProject->setText(QApplication::translate("MainWindow", "...", 0));
        lblJobType->setText(QApplication::translate("MainWindow", "Type:", 0));
#ifndef QT_NO_TOOLTIP
        btnJobOpenDoc->setToolTip(QApplication::translate("MainWindow", "Abrir arquivo doc", 0));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        btnJobOpenDoc->setStatusTip(QApplication::translate("MainWindow", "Abrir arquivo do projeto para edi\303\247\303\243o - se existir", 0));
#endif // QT_NO_STATUSTIP
        btnJobOpenDoc->setText(QString());
        lblJobID->setText(QApplication::translate("MainWindow", "ID:", 0));
#ifndef QT_NO_TOOLTIP
        btnJobOpenXls->setToolTip(QApplication::translate("MainWindow", "Abrir Planilha para edi\303\247\303\243o - se existir", 0));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        btnJobOpenXls->setStatusTip(QApplication::translate("MainWindow", "Abrir arquivo de planilha do projeto - se houver", 0));
#endif // QT_NO_STATUSTIP
        btnJobOpenXls->setText(QString());
#ifndef QT_NO_TOOLTIP
        btnJobOpenFileFolder->setToolTip(QApplication::translate("MainWindow", "Manager job's folder", 0));
#endif // QT_NO_TOOLTIP
        btnJobOpenFileFolder->setText(QString());
        lblJobFilepath->setText(QApplication::translate("MainWindow", "Project folder: ", 0));
        btnQuickProject->setText(QApplication::translate("MainWindow", "Quick Estimate", 0));
#ifndef QT_NO_TOOLTIP
        btnJobOpenPdf->setToolTip(QApplication::translate("MainWindow", "Visualizar PDF ou enviar por e-mail", 0));
#endif // QT_NO_TOOLTIP
        btnJobOpenPdf->setText(QString());
        lblJobDateStart->setText(QApplication::translate("MainWindow", "Begining:", 0));
        lblJobPrice->setText(QApplication::translate("MainWindow", "Price:", 0));
        lblJobDateEnd->setText(QApplication::translate("MainWindow", "End:", 0));
        tabMain->setTabText(tabMain->indexOf(tab), QApplication::translate("MainWindow", "Design 2", 0));
        grpJobTotalTime->setTitle(QApplication::translate("MainWindow", "Total time:", 0));
        lblJobDayTime->setText(QApplication::translate("MainWindow", "Day:", 0));
        lblJobAllDaysTime->setText(QApplication::translate("MainWindow", "All days:", 0));
        lblJobReport->setText(QApplication::translate("MainWindow", "Report:", 0));
        lblJobPictures->setText(QApplication::translate("MainWindow", "Job pictures path:", 0));
        lblJobEndTime->setText(QApplication::translate("MainWindow", "End time:", 0));
#ifndef QT_NO_TOOLTIP
        btnJobPrevDay->setToolTip(QApplication::translate("MainWindow", "Select previous day, if available", 0));
#endif // QT_NO_TOOLTIP
        btnJobPrevDay->setText(QApplication::translate("MainWindow", "...", 0));
#ifndef QT_NO_TOOLTIP
        btnJobNextDay->setToolTip(QApplication::translate("MainWindow", "Select nect day, if available", 0));
#endif // QT_NO_TOOLTIP
        btnJobNextDay->setText(QApplication::translate("MainWindow", "...", 0));
#ifndef QT_NO_TOOLTIP
        btnJobAddDay->setToolTip(QApplication::translate("MainWindow", "Add another day to job. Set the desired date in the calendar beside this button", 0));
#endif // QT_NO_TOOLTIP
        btnJobAddDay->setText(QApplication::translate("MainWindow", "...", 0));
#ifndef QT_NO_TOOLTIP
        btnJobDelDay->setToolTip(QApplication::translate("MainWindow", "<html><head/><body><p>Remove the selected date from the list. While the change is not saved it is possible to undo this operation. Select the strikedthrough item from the list and press the button just next to this one.</p></body></html>", 0));
#endif // QT_NO_TOOLTIP
        btnJobDelDay->setText(QApplication::translate("MainWindow", "...", 0));
#ifndef QT_NO_TOOLTIP
        btnJobCancelDelDay->setToolTip(QApplication::translate("MainWindow", "<html><head/><body><p>Deselect the strikedthrough item from the days list, canceling its removal. Only before saving the changes.</p></body></html>", 0));
#endif // QT_NO_TOOLTIP
        btnJobCancelDelDay->setText(QApplication::translate("MainWindow", "...", 0));
#ifndef QT_NO_TOOLTIP
        btnJobSeparateReportWindow->setToolTip(QApplication::translate("MainWindow", "View report in a separate window", 0));
#endif // QT_NO_TOOLTIP
        btnJobSeparateReportWindow->setText(QApplication::translate("MainWindow", "...", 0));
        lblJobPicturesCount->setText(QApplication::translate("MainWindow", "0 pictures", 0));
        btnJobPrevPicture->setText(QApplication::translate("MainWindow", "...", 0));
        btnJobNextPicture->setText(QApplication::translate("MainWindow", "...", 0));
        btnJobReloadPictures->setText(QApplication::translate("MainWindow", "...", 0));
#ifndef QT_NO_TOOLTIP
        btnJobRenamePicture->setToolTip(QApplication::translate("MainWindow", "Rename current picture", 0));
#endif // QT_NO_TOOLTIP
        btnJobRenamePicture->setText(QApplication::translate("MainWindow", "...", 0));
        btnJobOpenPictureEditor->setText(QApplication::translate("MainWindow", "...", 0));
        btnJobOpenPictureFolder->setText(QApplication::translate("MainWindow", "...", 0));
        btnJobOpenPictureViewer->setText(QApplication::translate("MainWindow", "...", 0));
        btnJobClientsYearPictures->setText(QApplication::translate("MainWindow", "...", 0));
#ifndef QT_NO_TOOLTIP
        btnJobSeparatePicture->setToolTip(QApplication::translate("MainWindow", "View pictures in a separate window", 0));
#endif // QT_NO_TOOLTIP
        btnJobSeparatePicture->setText(QApplication::translate("MainWindow", "...", 0));
        lblJobStartTime->setText(QApplication::translate("MainWindow", "Start time:", 0));
        lblWeatherConditions->setText(QApplication::translate("MainWindow", "Weather Conditions:", 0));
#ifndef QT_NO_TOOLTIP
        btnJobMachines->setToolTip(QApplication::translate("MainWindow", "Machines use", 0));
#endif // QT_NO_TOOLTIP
        btnJobMachines->setText(QApplication::translate("MainWindow", "...", 0));
        tabMain->setTabText(tabMain->indexOf(tabDesign3), QApplication::translate("MainWindow", "Design3", 0));
#ifndef QT_NO_TOOLTIP
        btnBuyAdd->setToolTip(QApplication::translate("MainWindow", "Insert a new purchase record", 0));
#endif // QT_NO_TOOLTIP
        btnBuyAdd->setText(QApplication::translate("MainWindow", "...", 0));
#ifndef QT_NO_TOOLTIP
        btnBuyEdit->setToolTip(QApplication::translate("MainWindow", "Edit current selected purchase record", 0));
#endif // QT_NO_TOOLTIP
        btnBuyEdit->setText(QApplication::translate("MainWindow", "...", 0));
#ifndef QT_NO_TOOLTIP
        btnBuyDel->setToolTip(QApplication::translate("MainWindow", "Remove current purchase record", 0));
#endif // QT_NO_TOOLTIP
        btnBuyDel->setText(QApplication::translate("MainWindow", "...", 0));
#ifndef QT_NO_TOOLTIP
        btnBuySave->setToolTip(QApplication::translate("MainWindow", "<html><head/><body><p>Save alterations to the current purchase record</p></body></html>", 0));
#endif // QT_NO_TOOLTIP
        btnBuySave->setText(QApplication::translate("MainWindow", "...", 0));
#ifndef QT_NO_TOOLTIP
        btnBuyCancel->setToolTip(QApplication::translate("MainWindow", "Cancel alterations to currentpurchase record", 0));
#endif // QT_NO_TOOLTIP
        btnBuyCancel->setText(QApplication::translate("MainWindow", "...", 0));
        lblBuySupplier_2->setText(QApplication::translate("MainWindow", "Supplier: ", 0));
        btnShowSuppliersDlg->setText(QApplication::translate("MainWindow", "...", 0));
        lblBuySupplierBuys->setText(QApplication::translate("MainWindow", "list os purchases", 0));
#ifndef QT_NO_TOOLTIP
        btnBuyCopyRows->setToolTip(QApplication::translate("MainWindow", "Copy list to quick project", 0));
#endif // QT_NO_TOOLTIP
        btnBuyCopyRows->setText(QApplication::translate("MainWindow", "...", 0));
        lblBuyID_2->setText(QApplication::translate("MainWindow", "ID:", 0));
        lblBuyPayments->setText(QApplication::translate("MainWindow", "Descrimination:", 0));
        lblBuyItems->setText(QApplication::translate("MainWindow", "Payments:", 0));
        lblBuyDeliverDate->setText(QApplication::translate("MainWindow", "Delivery date:", 0));
        lblBuyNotes_2->setText(QApplication::translate("MainWindow", "Notes:", 0));
        lblBuyTotalPrice->setText(QApplication::translate("MainWindow", "Total price:", 0));
        lblBuyTotalPaid->setText(QApplication::translate("MainWindow", "Total paid:", 0));
        lblBuyDeliverMethod_2->setText(QApplication::translate("MainWindow", "Delivery method:", 0));
        lblBuyDate_2->setText(QApplication::translate("MainWindow", "Purchase date:", 0));
        tabMain->setTabText(tabMain->indexOf(tabDesign_4), QApplication::translate("MainWindow", "Design4", 0));
        lblCurInfoClient->setText(QApplication::translate("MainWindow", "TextLabel", 0));
        lblCurInfoJob->setText(QApplication::translate("MainWindow", "TextLabel", 0));
        lblCurInfoPay->setText(QApplication::translate("MainWindow", "TextLabel", 0));
        lblCurInfoBuy->setText(QApplication::translate("MainWindow", "TextLabel", 0));
        lblFastSearch->setText(QApplication::translate("MainWindow", "Search: ", 0));
#ifndef QT_NO_TOOLTIP
        txtQuickSearch->setToolTip(QApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Ubuntu'; font-size:11pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Sans Serif'; font-size:9pt;\">Procura por texto em todo banco de dados.</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Sans Serif'; font-size:9pt;\">Procuras espec\303\255ficas nos campos atrav\303\251s do caractere </span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Sans Serif'; font-size:9pt;\">'%' em quaisqu"
                        "er combina\303\247\303\265es</span></p>\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:'Sans Serif'; font-size:9pt;\"><br /></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Sans Serif'; font-size:9pt;\">       </span><span style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:600;\">-&gt; %c:</span><span style=\" font-family:'Sans Serif'; font-size:9pt;\">  Procura somente no cliente atual</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Sans Serif'; font-size:9pt;\">       </span><span style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:600;\">-&gt; %i:</span><span style=\" font-family:'Sans Serif'; font-size:9pt;\">  Procura nos campos ID</span></p>\n"
"<p sty"
                        "le=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Sans Serif'; font-size:9pt;\">       </span><span style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:600;\">-&gt; %r:</span><span style=\" font-family:'Sans Serif'; font-size:9pt;\">  Procura nos campos Relat\303\263rio</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Sans Serif'; font-size:9pt;\">       </span><span style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:600;\">-&gt; %$:</span><span style=\" font-family:'Sans Serif'; font-size:9pt;\">  Procura nos campos de valores monet\303\241rios</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Sans Serif'; font-size:9pt;\">       </span><span style=\" font-family:'Sans Ser"
                        "if'; font-size:9pt; font-weight:600;\">-&gt; %n:</span><span style=\" font-family:'Sans Serif'; font-size:9pt;\">  Procura no campo Nome do Cliente</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Sans Serif'; font-size:9pt;\">       </span><span style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:600;\">-&gt; %e:</span><span style=\" font-family:'Sans Serif'; font-size:9pt;\">  Procura no campo Endere\303\247o do Cliente</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Sans Serif'; font-size:9pt;\">       </span><span style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:600;\">-&gt; %f:</span><span style=\" font-family:'Sans Serif'; font-size:9pt;\">  Procura nos campos Telefones do Cliente</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0"
                        "px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Sans Serif'; font-size:9pt;\">       </span><span style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:600;\">-&gt; %d:</span><span style=\" font-family:'Sans Serif'; font-size:9pt;\">  Procura nos campos de Datas</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Sans Serif'; font-size:9pt;\">       </span><span style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:600;\">-&gt; %h:</span><span style=\" font-family:'Sans Serif'; font-size:9pt;\">  Procura nos campos de Tempo</span></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Sans Serif'; font-size:9pt;\">       </span><span style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:600;\">-&gt; %t:</span><span style=\" font-family:"
                        "'Sans Serif'; font-size:9pt;\">  Procura nos campos de Tipo de Servi\303\247o ou Agenda</span></p>\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-family:'Sans Serif'; font-size:9pt;\"><br /></p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Sans Serif'; font-size:9pt;\">Tecla de atalho: </span><span style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:600; font-style:italic; text-decoration: underline;\">CTRL+F</span></p></body></html>", 0));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        btnSearchStart->setToolTip(QApplication::translate("MainWindow", "Start search", 0));
#endif // QT_NO_TOOLTIP
        btnSearchStart->setText(QApplication::translate("MainWindow", "...", 0));
#ifndef QT_NO_TOOLTIP
        btnSearchCancel->setToolTip(QApplication::translate("MainWindow", "Cancel search", 0));
#endif // QT_NO_TOOLTIP
        btnSearchCancel->setText(QApplication::translate("MainWindow", "...", 0));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
