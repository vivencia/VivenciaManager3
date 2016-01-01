/********************************************************************************
** Form generated from reading UI file 'backupdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_BACKUPDIALOG_H
#define UI_BACKUPDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QToolBox>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <vmwidgets.h>

QT_BEGIN_NAMESPACE

class Ui_BackupDialog
{
public:
    QVBoxLayout *verticalLayout_2;
    QToolBox *toolBox;
    QWidget *pageBackup;
    QGridLayout *gLayoutPage;
    QLabel *lblSelectTables;
    QListWidget *tablesList;
    QGroupBox *grpExportToText;
    QGridLayout *gridLayout_5;
    QGridLayout *gridLayout_4;
    QLabel *lblExportPrefix;
    QLabel *lblExportFolder;
    vmLineEdit *txtExportPrefix;
    vmLineEdit *txtExportFolder;
    QToolButton *btnDefaultPrefix;
    QToolButton *btnChooseExportFolder;
    QGroupBox *grpBackup;
    QGridLayout *gridLayout;
    QGridLayout *gridLayout_3;
    QLabel *lblBackupFilename;
    vmLineEdit *txtBackupFilename;
    QLabel *lblBackupFolder;
    vmLineEdit *txtBackupFolder;
    QToolButton *btnDefaultFilename;
    QToolButton *btnChooseBackupFolder;
    QGroupBox *grpIncludes;
    QHBoxLayout *horizontalLayout_4;
    QHBoxLayout *horizontalLayout_3;
    vmCheckBox *chkTables;
    vmCheckBox *chkDocs;
    vmCheckBox *chkImages;
    QWidget *pageRestore;
    QGridLayout *gridLayout_7;
    vmLineEdit *txtRestoreFileName;
    QVBoxLayout *verticalLayout_3;
    QPushButton *btnSendMail;
    QPushButton *btnRemoveFromList;
    QPushButton *btnRemoveAndDelete;
    QRadioButton *rdChooseAnotherFile;
    QListWidget *restoreList;
    QRadioButton *rdChooseKnownFile;
    QToolButton *btnChooseImportFile;
    QVBoxLayout *verticalLayout;
    QFrame *frmBottomBar;
    QHBoxLayout *horizontalLayout_2;
    QHBoxLayout *horizontalLayout;
    QWidget *widget;
    QHBoxLayout *horizontalLayout_5;
    QProgressBar *pBar;
    QPushButton *btnClose;
    QPushButton *btnApply;

    void setupUi(QDialog *BackupDialog)
    {
        if (BackupDialog->objectName().isEmpty())
            BackupDialog->setObjectName(QStringLiteral("BackupDialog"));
        BackupDialog->resize(642, 525);
        BackupDialog->setModal(true);
        verticalLayout_2 = new QVBoxLayout(BackupDialog);
        verticalLayout_2->setSpacing(2);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(2, 2, 2, 2);
        toolBox = new QToolBox(BackupDialog);
        toolBox->setObjectName(QStringLiteral("toolBox"));
        toolBox->setFrameShape(QFrame::StyledPanel);
        toolBox->setFrameShadow(QFrame::Raised);
        pageBackup = new QWidget();
        pageBackup->setObjectName(QStringLiteral("pageBackup"));
        pageBackup->setGeometry(QRect(0, 0, 634, 419));
        gLayoutPage = new QGridLayout(pageBackup);
        gLayoutPage->setSpacing(2);
        gLayoutPage->setObjectName(QStringLiteral("gLayoutPage"));
        gLayoutPage->setContentsMargins(2, 2, 2, 2);
        lblSelectTables = new QLabel(pageBackup);
        lblSelectTables->setObjectName(QStringLiteral("lblSelectTables"));
        lblSelectTables->setMinimumSize(QSize(0, 30));
        lblSelectTables->setMaximumSize(QSize(16777215, 30));
        lblSelectTables->setFrameShape(QFrame::NoFrame);
        lblSelectTables->setFrameShadow(QFrame::Plain);

        gLayoutPage->addWidget(lblSelectTables, 1, 0, 1, 1);

        tablesList = new QListWidget(pageBackup);
        tablesList->setObjectName(QStringLiteral("tablesList"));
        tablesList->setProperty("showDropIndicator", QVariant(false));
        tablesList->setAlternatingRowColors(true);
        tablesList->setUniformItemSizes(true);
        tablesList->setSortingEnabled(false);

        gLayoutPage->addWidget(tablesList, 2, 0, 2, 1);

        grpExportToText = new QGroupBox(pageBackup);
        grpExportToText->setObjectName(QStringLiteral("grpExportToText"));
        grpExportToText->setCheckable(true);
        grpExportToText->setChecked(false);
        gridLayout_5 = new QGridLayout(grpExportToText);
        gridLayout_5->setSpacing(1);
        gridLayout_5->setObjectName(QStringLiteral("gridLayout_5"));
        gridLayout_5->setContentsMargins(1, 1, 1, 1);
        gridLayout_4 = new QGridLayout();
        gridLayout_4->setSpacing(1);
        gridLayout_4->setObjectName(QStringLiteral("gridLayout_4"));
        lblExportPrefix = new QLabel(grpExportToText);
        lblExportPrefix->setObjectName(QStringLiteral("lblExportPrefix"));
        lblExportPrefix->setMinimumSize(QSize(0, 30));

        gridLayout_4->addWidget(lblExportPrefix, 0, 0, 1, 1);

        lblExportFolder = new QLabel(grpExportToText);
        lblExportFolder->setObjectName(QStringLiteral("lblExportFolder"));
        lblExportFolder->setMinimumSize(QSize(0, 30));

        gridLayout_4->addWidget(lblExportFolder, 2, 0, 1, 1);

        txtExportPrefix = new vmLineEdit(grpExportToText);
        txtExportPrefix->setObjectName(QStringLiteral("txtExportPrefix"));
        txtExportPrefix->setMinimumSize(QSize(0, 30));

        gridLayout_4->addWidget(txtExportPrefix, 1, 0, 1, 1);

        txtExportFolder = new vmLineEdit(grpExportToText);
        txtExportFolder->setObjectName(QStringLiteral("txtExportFolder"));
        txtExportFolder->setMinimumSize(QSize(0, 30));

        gridLayout_4->addWidget(txtExportFolder, 3, 0, 1, 1);

        btnDefaultPrefix = new QToolButton(grpExportToText);
        btnDefaultPrefix->setObjectName(QStringLiteral("btnDefaultPrefix"));
        btnDefaultPrefix->setMinimumSize(QSize(30, 30));
        btnDefaultPrefix->setMaximumSize(QSize(30, 30));
        QIcon icon;
        icon.addFile(QStringLiteral(":/resources/fill_default.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnDefaultPrefix->setIcon(icon);

        gridLayout_4->addWidget(btnDefaultPrefix, 1, 1, 1, 1);

        btnChooseExportFolder = new QToolButton(grpExportToText);
        btnChooseExportFolder->setObjectName(QStringLiteral("btnChooseExportFolder"));
        btnChooseExportFolder->setMinimumSize(QSize(30, 30));
        btnChooseExportFolder->setMaximumSize(QSize(30, 30));
        QIcon icon1;
        icon1.addFile(QStringLiteral(":/resources/folder-brown.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnChooseExportFolder->setIcon(icon1);

        gridLayout_4->addWidget(btnChooseExportFolder, 3, 1, 1, 1);


        gridLayout_5->addLayout(gridLayout_4, 0, 0, 1, 1);


        gLayoutPage->addWidget(grpExportToText, 3, 1, 1, 1);

        grpBackup = new QGroupBox(pageBackup);
        grpBackup->setObjectName(QStringLiteral("grpBackup"));
        grpBackup->setMinimumSize(QSize(100, 100));
        grpBackup->setCheckable(true);
        gridLayout = new QGridLayout(grpBackup);
        gridLayout->setSpacing(1);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        gridLayout->setContentsMargins(1, 1, 1, 1);
        gridLayout_3 = new QGridLayout();
        gridLayout_3->setSpacing(1);
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        lblBackupFilename = new QLabel(grpBackup);
        lblBackupFilename->setObjectName(QStringLiteral("lblBackupFilename"));
        lblBackupFilename->setMinimumSize(QSize(0, 30));

        gridLayout_3->addWidget(lblBackupFilename, 0, 0, 1, 1);

        txtBackupFilename = new vmLineEdit(grpBackup);
        txtBackupFilename->setObjectName(QStringLiteral("txtBackupFilename"));
        txtBackupFilename->setMinimumSize(QSize(0, 30));

        gridLayout_3->addWidget(txtBackupFilename, 1, 0, 1, 1);

        lblBackupFolder = new QLabel(grpBackup);
        lblBackupFolder->setObjectName(QStringLiteral("lblBackupFolder"));
        lblBackupFolder->setMinimumSize(QSize(0, 30));

        gridLayout_3->addWidget(lblBackupFolder, 2, 0, 1, 1);

        txtBackupFolder = new vmLineEdit(grpBackup);
        txtBackupFolder->setObjectName(QStringLiteral("txtBackupFolder"));
        txtBackupFolder->setMinimumSize(QSize(0, 30));

        gridLayout_3->addWidget(txtBackupFolder, 3, 0, 1, 1);

        btnDefaultFilename = new QToolButton(grpBackup);
        btnDefaultFilename->setObjectName(QStringLiteral("btnDefaultFilename"));
        btnDefaultFilename->setMinimumSize(QSize(30, 30));
        btnDefaultFilename->setMaximumSize(QSize(30, 30));
        btnDefaultFilename->setIcon(icon);

        gridLayout_3->addWidget(btnDefaultFilename, 1, 1, 1, 1);

        btnChooseBackupFolder = new QToolButton(grpBackup);
        btnChooseBackupFolder->setObjectName(QStringLiteral("btnChooseBackupFolder"));
        btnChooseBackupFolder->setMinimumSize(QSize(30, 30));
        btnChooseBackupFolder->setMaximumSize(QSize(30, 30));
        btnChooseBackupFolder->setIcon(icon1);

        gridLayout_3->addWidget(btnChooseBackupFolder, 3, 1, 1, 1);


        gridLayout->addLayout(gridLayout_3, 0, 0, 1, 1);


        gLayoutPage->addWidget(grpBackup, 2, 1, 1, 1);

        grpIncludes = new QGroupBox(pageBackup);
        grpIncludes->setObjectName(QStringLiteral("grpIncludes"));
        grpIncludes->setMinimumSize(QSize(0, 60));
        horizontalLayout_4 = new QHBoxLayout(grpIncludes);
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        chkTables = new vmCheckBox(grpIncludes);
        chkTables->setObjectName(QStringLiteral("chkTables"));
        chkTables->setChecked(true);

        horizontalLayout_3->addWidget(chkTables);

        chkDocs = new vmCheckBox(grpIncludes);
        chkDocs->setObjectName(QStringLiteral("chkDocs"));

        horizontalLayout_3->addWidget(chkDocs);

        chkImages = new vmCheckBox(grpIncludes);
        chkImages->setObjectName(QStringLiteral("chkImages"));

        horizontalLayout_3->addWidget(chkImages);


        horizontalLayout_4->addLayout(horizontalLayout_3);


        gLayoutPage->addWidget(grpIncludes, 0, 0, 1, 2);

        toolBox->addItem(pageBackup, QStringLiteral("Backup"));
        pageRestore = new QWidget();
        pageRestore->setObjectName(QStringLiteral("pageRestore"));
        pageRestore->setGeometry(QRect(0, 0, 634, 419));
        gridLayout_7 = new QGridLayout(pageRestore);
        gridLayout_7->setSpacing(2);
        gridLayout_7->setObjectName(QStringLiteral("gridLayout_7"));
        gridLayout_7->setContentsMargins(2, 2, 2, 2);
        txtRestoreFileName = new vmLineEdit(pageRestore);
        txtRestoreFileName->setObjectName(QStringLiteral("txtRestoreFileName"));
        txtRestoreFileName->setEnabled(false);
        txtRestoreFileName->setMinimumSize(QSize(0, 30));

        gridLayout_7->addWidget(txtRestoreFileName, 4, 2, 1, 1);

        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setSpacing(0);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        btnSendMail = new QPushButton(pageRestore);
        btnSendMail->setObjectName(QStringLiteral("btnSendMail"));
        btnSendMail->setMinimumSize(QSize(0, 30));
        QIcon icon2;
        icon2.addFile(QStringLiteral(":/resources/email.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnSendMail->setIcon(icon2);

        verticalLayout_3->addWidget(btnSendMail);

        btnRemoveFromList = new QPushButton(pageRestore);
        btnRemoveFromList->setObjectName(QStringLiteral("btnRemoveFromList"));
        btnRemoveFromList->setMinimumSize(QSize(0, 30));
        QIcon icon3;
        icon3.addFile(QStringLiteral(":/resources/remove.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnRemoveFromList->setIcon(icon3);

        verticalLayout_3->addWidget(btnRemoveFromList);

        btnRemoveAndDelete = new QPushButton(pageRestore);
        btnRemoveAndDelete->setObjectName(QStringLiteral("btnRemoveAndDelete"));
        btnRemoveAndDelete->setMinimumSize(QSize(0, 30));
        QIcon icon4;
        icon4.addFile(QStringLiteral(":/resources/edit-delete.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnRemoveAndDelete->setIcon(icon4);

        verticalLayout_3->addWidget(btnRemoveAndDelete);


        gridLayout_7->addLayout(verticalLayout_3, 1, 3, 3, 1);

        rdChooseAnotherFile = new QRadioButton(pageRestore);
        rdChooseAnotherFile->setObjectName(QStringLiteral("rdChooseAnotherFile"));
        rdChooseAnotherFile->setMinimumSize(QSize(0, 30));
        rdChooseAnotherFile->setAutoExclusive(true);

        gridLayout_7->addWidget(rdChooseAnotherFile, 4, 0, 1, 1);

        restoreList = new QListWidget(pageRestore);
        restoreList->setObjectName(QStringLiteral("restoreList"));
        restoreList->setAlternatingRowColors(true);
        restoreList->setSelectionMode(QAbstractItemView::ExtendedSelection);

        gridLayout_7->addWidget(restoreList, 3, 0, 1, 3);

        rdChooseKnownFile = new QRadioButton(pageRestore);
        rdChooseKnownFile->setObjectName(QStringLiteral("rdChooseKnownFile"));
        rdChooseKnownFile->setMinimumSize(QSize(0, 30));
        rdChooseKnownFile->setChecked(true);
        rdChooseKnownFile->setAutoExclusive(true);

        gridLayout_7->addWidget(rdChooseKnownFile, 0, 0, 1, 1);

        btnChooseImportFile = new QToolButton(pageRestore);
        btnChooseImportFile->setObjectName(QStringLiteral("btnChooseImportFile"));
        btnChooseImportFile->setEnabled(false);
        btnChooseImportFile->setMinimumSize(QSize(30, 30));
        btnChooseImportFile->setMaximumSize(QSize(30, 30));
        QIcon icon5;
        icon5.addFile(QStringLiteral(":/resources/find-service.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnChooseImportFile->setIcon(icon5);

        gridLayout_7->addWidget(btnChooseImportFile, 4, 3, 1, 1);

        toolBox->addItem(pageRestore, QStringLiteral("Restore"));

        verticalLayout_2->addWidget(toolBox);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(1);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));

        verticalLayout_2->addLayout(verticalLayout);

        frmBottomBar = new QFrame(BackupDialog);
        frmBottomBar->setObjectName(QStringLiteral("frmBottomBar"));
        frmBottomBar->setMinimumSize(QSize(0, 30));
        horizontalLayout_2 = new QHBoxLayout(frmBottomBar);
        horizontalLayout_2->setSpacing(0);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(2);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        widget = new QWidget(frmBottomBar);
        widget->setObjectName(QStringLiteral("widget"));
        horizontalLayout_5 = new QHBoxLayout(widget);
        horizontalLayout_5->setSpacing(0);
        horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
        horizontalLayout_5->setContentsMargins(0, 0, 0, 0);
        pBar = new QProgressBar(widget);
        pBar->setObjectName(QStringLiteral("pBar"));
        pBar->setValue(0);

        horizontalLayout_5->addWidget(pBar);


        horizontalLayout->addWidget(widget);

        btnClose = new QPushButton(frmBottomBar);
        btnClose->setObjectName(QStringLiteral("btnClose"));

        horizontalLayout->addWidget(btnClose);

        btnApply = new QPushButton(frmBottomBar);
        btnApply->setObjectName(QStringLiteral("btnApply"));
        btnApply->setFlat(false);

        horizontalLayout->addWidget(btnApply);

        horizontalLayout->setStretch(0, 2);

        horizontalLayout_2->addLayout(horizontalLayout);


        verticalLayout_2->addWidget(frmBottomBar);


        retranslateUi(BackupDialog);

        toolBox->setCurrentIndex(1);
        btnApply->setDefault(true);


        QMetaObject::connectSlotsByName(BackupDialog);
    } // setupUi

    void retranslateUi(QDialog *BackupDialog)
    {
        BackupDialog->setWindowTitle(QApplication::translate("BackupDialog", "Dialog", 0));
        lblSelectTables->setText(QApplication::translate("BackupDialog", "Select tables to backup/export", 0));
        grpExportToText->setTitle(QApplication::translate("BackupDialog", "Export to &CSV text file", 0));
        lblExportPrefix->setText(QApplication::translate("BackupDialog", "Name prefix:", 0));
        lblExportFolder->setText(QApplication::translate("BackupDialog", "Folder where the tables will be stored", 0));
#ifndef QT_NO_TOOLTIP
        txtExportPrefix->setToolTip(QApplication::translate("BackupDialog", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Tahoma'; font-size:10pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">To <span style=\" font-style:italic;\">Prefix</span> it will be added the table name if backup is successfull</p></body></html>", 0));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_TOOLTIP
        btnDefaultPrefix->setToolTip(QApplication::translate("BackupDialog", "Use program's default prefix pattern, based on the current date", 0));
#endif // QT_NO_TOOLTIP
        btnDefaultPrefix->setText(QApplication::translate("BackupDialog", "...", 0));
        btnChooseExportFolder->setText(QApplication::translate("BackupDialog", "...", 0));
        grpBackup->setTitle(QApplication::translate("BackupDialog", "Standard backup &method", 0));
        lblBackupFilename->setText(QApplication::translate("BackupDialog", "Name:", 0));
#ifndef QT_NO_TOOLTIP
        txtBackupFilename->setToolTip(QApplication::translate("BackupDialog", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Tahoma'; font-size:10pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">To <span style=\" font-style:italic;\">Name</span> it will be added the suffix <span style=\" font-style:italic;\">.bz2</span> if the backup is successfull.</p></body></html>", 0));
#endif // QT_NO_TOOLTIP
        lblBackupFolder->setText(QApplication::translate("BackupDialog", "Folder to save backup file:", 0));
#ifndef QT_NO_TOOLTIP
        btnDefaultFilename->setToolTip(QApplication::translate("BackupDialog", "Use program's default name based on the current date", 0));
#endif // QT_NO_TOOLTIP
        btnDefaultFilename->setText(QApplication::translate("BackupDialog", "...", 0));
        btnChooseBackupFolder->setText(QApplication::translate("BackupDialog", "...", 0));
        grpIncludes->setTitle(QApplication::translate("BackupDialog", "What to include in the backup", 0));
        chkTables->setText(QApplication::translate("BackupDialog", "Tables", 0));
        chkDocs->setText(QApplication::translate("BackupDialog", "External documents", 0));
        chkImages->setText(QApplication::translate("BackupDialog", "Photos and images", 0));
        toolBox->setItemText(toolBox->indexOf(pageBackup), QApplication::translate("BackupDialog", "Backup", 0));
        btnSendMail->setText(QApplication::translate("BackupDialog", "Attatch to message", 0));
        btnRemoveFromList->setText(QApplication::translate("BackupDialog", "Remove selected", 0));
        btnRemoveAndDelete->setText(QApplication::translate("BackupDialog", "Remove selected \n"
"and delete file", 0));
        rdChooseAnotherFile->setText(QApplication::translate("BackupDialog", "&Choose another file ...", 0));
        rdChooseKnownFile->setText(QApplication::translate("BackupDialog", "&Known backups and exports", 0));
        btnChooseImportFile->setText(QApplication::translate("BackupDialog", "...", 0));
        toolBox->setItemText(toolBox->indexOf(pageRestore), QApplication::translate("BackupDialog", "Restore", 0));
        btnClose->setText(QApplication::translate("BackupDialog", "Close", 0));
        btnApply->setText(QApplication::translate("BackupDialog", "Apply", 0));
    } // retranslateUi

};

namespace Ui {
    class BackupDialog: public Ui_BackupDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_BACKUPDIALOG_H
