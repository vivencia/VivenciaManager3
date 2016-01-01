/********************************************************************************
** Form generated from reading UI file 'configdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CONFIGDIALOG_H
#define UI_CONFIGDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QToolButton>

QT_BEGIN_NAMESPACE

class Ui_configDialog
{
public:
    QHBoxLayout *horizontalLayout_2;
    QHBoxLayout *horizontalLayout;
    QGroupBox *grpProgramOptions;
    QGridLayout *gLayoutProgramOptions;
    QToolButton *btnCfgUseDefaultESTIMATEDir;
    QLabel *lblCfgFileManager;
    QLineEdit *txtCfgPictureViewer;
    QToolButton *btnCfgChoosePictureViewer;
    QToolButton *btnCfgChooseConfigFile;
    QLabel *lblCfgConfigFile;
    QToolButton *btnCfgChooseXlsEditor;
    QLineEdit *txtCfgXlsEditor;
    QLineEdit *txtCfgEmailClient;
    QLabel *lblCfgESTIMATE;
    QToolButton *btnCfgChoosePictureEditor;
    QToolButton *btnCfgUseDefaultPictureEditor;
    QLabel *lblCfgPictureEditor;
    QToolButton *btnCfgUseDefaultPictureViewer;
    QToolButton *btnCfgUseDefaultBaseDir;
    QToolButton *btnCfgChooseDocViewer;
    QToolButton *btnCfgUseDefaultReportsDir;
    QLineEdit *txtCfgReports;
    QLabel *lblCfgJobsPrefix;
    QToolButton *btnCfgUseDefaultFileManager;
    QLabel *lblCfgDocApp;
    QLineEdit *txtCfgDocumentViewer;
    QLineEdit *txtCfgDataFolder;
    QToolButton *btnCfgUseDefaultDataFolder;
    QToolButton *btnCfgChooseDataFolder;
    QLineEdit *txtCfgFileManager;
    QLabel *lblCfgReports;
    QToolButton *btnCfgUseDefaultXlsEditor;
    QLabel *lblCfgXlsApp;
    QToolButton *btnCfgUseDefaultDocEditor;
    QToolButton *btnCfgChooseDocEditor;
    QLineEdit *txtCfgPictureEditor;
    QLineEdit *txtCfgESTIMATE;
    QLabel *lblCfgDocumentViewer;
    QToolButton *btnCfgChooseFileManager;
    QLabel *lblCfgEmailClient;
    QLineEdit *txtCfgConfigFile;
    QLineEdit *txtCfgDocEditor;
    QToolButton *btnCfgUseDefaultConfigFile;
    QToolButton *btnCfgChooseBaseDir;
    QToolButton *btnCfgChooseEMailClient;
    QLabel *lblCfgPictureViewer;
    QToolButton *btnCfgUseDefaultDocumentViewer;
    QLineEdit *txtCfgJobsPrefix;
    QLabel *lblCfgDataFolder;

    void setupUi(QDialog *configDialog)
    {
        if (configDialog->objectName().isEmpty())
            configDialog->setObjectName(QStringLiteral("configDialog"));
        configDialog->resize(641, 486);
        horizontalLayout_2 = new QHBoxLayout(configDialog);
        horizontalLayout_2->setSpacing(0);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(2, 0, 0, 0);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(0);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        grpProgramOptions = new QGroupBox(configDialog);
        grpProgramOptions->setObjectName(QStringLiteral("grpProgramOptions"));
        grpProgramOptions->setEnabled(true);
        grpProgramOptions->setMinimumSize(QSize(0, 150));
        QFont font;
        font.setBold(true);
        font.setUnderline(true);
        font.setWeight(75);
        grpProgramOptions->setFont(font);
        grpProgramOptions->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        grpProgramOptions->setFlat(false);
        grpProgramOptions->setCheckable(false);
        grpProgramOptions->setChecked(false);
        gLayoutProgramOptions = new QGridLayout(grpProgramOptions);
        gLayoutProgramOptions->setSpacing(2);
        gLayoutProgramOptions->setObjectName(QStringLiteral("gLayoutProgramOptions"));
        gLayoutProgramOptions->setContentsMargins(0, 0, 0, 0);
        btnCfgUseDefaultESTIMATEDir = new QToolButton(grpProgramOptions);
        btnCfgUseDefaultESTIMATEDir->setObjectName(QStringLiteral("btnCfgUseDefaultESTIMATEDir"));
        btnCfgUseDefaultESTIMATEDir->setMinimumSize(QSize(30, 30));
        QIcon icon;
        icon.addFile(QStringLiteral(":/resources/fill_default.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnCfgUseDefaultESTIMATEDir->setIcon(icon);

        gLayoutProgramOptions->addWidget(btnCfgUseDefaultESTIMATEDir, 8, 7, 1, 1);

        lblCfgFileManager = new QLabel(grpProgramOptions);
        lblCfgFileManager->setObjectName(QStringLiteral("lblCfgFileManager"));
        lblCfgFileManager->setEnabled(false);
        QFont font1;
        font1.setBold(false);
        font1.setUnderline(false);
        font1.setWeight(50);
        lblCfgFileManager->setFont(font1);

        gLayoutProgramOptions->addWidget(lblCfgFileManager, 1, 0, 1, 1);

        txtCfgPictureViewer = new QLineEdit(grpProgramOptions);
        txtCfgPictureViewer->setObjectName(QStringLiteral("txtCfgPictureViewer"));
        txtCfgPictureViewer->setMinimumSize(QSize(0, 30));
        txtCfgPictureViewer->setFont(font1);

        gLayoutProgramOptions->addWidget(txtCfgPictureViewer, 3, 1, 1, 1);

        btnCfgChoosePictureViewer = new QToolButton(grpProgramOptions);
        btnCfgChoosePictureViewer->setObjectName(QStringLiteral("btnCfgChoosePictureViewer"));
        btnCfgChoosePictureViewer->setMinimumSize(QSize(30, 30));
        btnCfgChoosePictureViewer->setFont(font1);
        QIcon icon1;
        icon1.addFile(QStringLiteral(":/resources/folder-brown.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnCfgChoosePictureViewer->setIcon(icon1);

        gLayoutProgramOptions->addWidget(btnCfgChoosePictureViewer, 3, 2, 1, 1);

        btnCfgChooseConfigFile = new QToolButton(grpProgramOptions);
        btnCfgChooseConfigFile->setObjectName(QStringLiteral("btnCfgChooseConfigFile"));
        btnCfgChooseConfigFile->setMinimumSize(QSize(30, 30));
        btnCfgChooseConfigFile->setFont(font1);
        btnCfgChooseConfigFile->setIcon(icon1);

        gLayoutProgramOptions->addWidget(btnCfgChooseConfigFile, 0, 2, 1, 1);

        lblCfgConfigFile = new QLabel(grpProgramOptions);
        lblCfgConfigFile->setObjectName(QStringLiteral("lblCfgConfigFile"));
        lblCfgConfigFile->setEnabled(false);
        lblCfgConfigFile->setFont(font1);

        gLayoutProgramOptions->addWidget(lblCfgConfigFile, 0, 0, 1, 1);

        btnCfgChooseXlsEditor = new QToolButton(grpProgramOptions);
        btnCfgChooseXlsEditor->setObjectName(QStringLiteral("btnCfgChooseXlsEditor"));
        btnCfgChooseXlsEditor->setMinimumSize(QSize(30, 30));
        btnCfgChooseXlsEditor->setFont(font1);
        btnCfgChooseXlsEditor->setIcon(icon1);

        gLayoutProgramOptions->addWidget(btnCfgChooseXlsEditor, 7, 6, 1, 1);

        txtCfgXlsEditor = new QLineEdit(grpProgramOptions);
        txtCfgXlsEditor->setObjectName(QStringLiteral("txtCfgXlsEditor"));
        txtCfgXlsEditor->setMinimumSize(QSize(0, 30));
        txtCfgXlsEditor->setFont(font1);

        gLayoutProgramOptions->addWidget(txtCfgXlsEditor, 7, 5, 1, 1);

        txtCfgEmailClient = new QLineEdit(grpProgramOptions);
        txtCfgEmailClient->setObjectName(QStringLiteral("txtCfgEmailClient"));
        txtCfgEmailClient->setMinimumSize(QSize(0, 30));
        txtCfgEmailClient->setFont(font1);
        txtCfgEmailClient->setReadOnly(true);

        gLayoutProgramOptions->addWidget(txtCfgEmailClient, 5, 1, 1, 1);

        lblCfgESTIMATE = new QLabel(grpProgramOptions);
        lblCfgESTIMATE->setObjectName(QStringLiteral("lblCfgESTIMATE"));
        lblCfgESTIMATE->setFont(font1);

        gLayoutProgramOptions->addWidget(lblCfgESTIMATE, 8, 4, 1, 1);

        btnCfgChoosePictureEditor = new QToolButton(grpProgramOptions);
        btnCfgChoosePictureEditor->setObjectName(QStringLiteral("btnCfgChoosePictureEditor"));
        btnCfgChoosePictureEditor->setMinimumSize(QSize(30, 30));
        btnCfgChoosePictureEditor->setFont(font1);
        btnCfgChoosePictureEditor->setIcon(icon1);

        gLayoutProgramOptions->addWidget(btnCfgChoosePictureEditor, 3, 6, 1, 1);

        btnCfgUseDefaultPictureEditor = new QToolButton(grpProgramOptions);
        btnCfgUseDefaultPictureEditor->setObjectName(QStringLiteral("btnCfgUseDefaultPictureEditor"));
        btnCfgUseDefaultPictureEditor->setMinimumSize(QSize(30, 30));
        btnCfgUseDefaultPictureEditor->setFont(font1);
        btnCfgUseDefaultPictureEditor->setIcon(icon);

        gLayoutProgramOptions->addWidget(btnCfgUseDefaultPictureEditor, 3, 7, 1, 1);

        lblCfgPictureEditor = new QLabel(grpProgramOptions);
        lblCfgPictureEditor->setObjectName(QStringLiteral("lblCfgPictureEditor"));
        lblCfgPictureEditor->setEnabled(false);
        lblCfgPictureEditor->setFont(font1);

        gLayoutProgramOptions->addWidget(lblCfgPictureEditor, 3, 4, 1, 1);

        btnCfgUseDefaultPictureViewer = new QToolButton(grpProgramOptions);
        btnCfgUseDefaultPictureViewer->setObjectName(QStringLiteral("btnCfgUseDefaultPictureViewer"));
        btnCfgUseDefaultPictureViewer->setMinimumSize(QSize(30, 30));
        btnCfgUseDefaultPictureViewer->setFont(font1);
        btnCfgUseDefaultPictureViewer->setIcon(icon);

        gLayoutProgramOptions->addWidget(btnCfgUseDefaultPictureViewer, 3, 3, 1, 1);

        btnCfgUseDefaultBaseDir = new QToolButton(grpProgramOptions);
        btnCfgUseDefaultBaseDir->setObjectName(QStringLiteral("btnCfgUseDefaultBaseDir"));
        btnCfgUseDefaultBaseDir->setMinimumSize(QSize(30, 30));
        btnCfgUseDefaultBaseDir->setFont(font1);
        btnCfgUseDefaultBaseDir->setIcon(icon);

        gLayoutProgramOptions->addWidget(btnCfgUseDefaultBaseDir, 1, 7, 1, 1);

        btnCfgChooseDocViewer = new QToolButton(grpProgramOptions);
        btnCfgChooseDocViewer->setObjectName(QStringLiteral("btnCfgChooseDocViewer"));
        btnCfgChooseDocViewer->setMinimumSize(QSize(30, 30));
        btnCfgChooseDocViewer->setFont(font1);
        btnCfgChooseDocViewer->setIcon(icon1);

        gLayoutProgramOptions->addWidget(btnCfgChooseDocViewer, 5, 6, 1, 1);

        btnCfgUseDefaultReportsDir = new QToolButton(grpProgramOptions);
        btnCfgUseDefaultReportsDir->setObjectName(QStringLiteral("btnCfgUseDefaultReportsDir"));
        btnCfgUseDefaultReportsDir->setMinimumSize(QSize(30, 30));
        btnCfgUseDefaultReportsDir->setFont(font1);
        btnCfgUseDefaultReportsDir->setIcon(icon);

        gLayoutProgramOptions->addWidget(btnCfgUseDefaultReportsDir, 8, 3, 1, 1);

        txtCfgReports = new QLineEdit(grpProgramOptions);
        txtCfgReports->setObjectName(QStringLiteral("txtCfgReports"));

        gLayoutProgramOptions->addWidget(txtCfgReports, 8, 1, 1, 1);

        lblCfgJobsPrefix = new QLabel(grpProgramOptions);
        lblCfgJobsPrefix->setObjectName(QStringLiteral("lblCfgJobsPrefix"));
        lblCfgJobsPrefix->setEnabled(false);
        lblCfgJobsPrefix->setMinimumSize(QSize(0, 30));
        lblCfgJobsPrefix->setFont(font1);

        gLayoutProgramOptions->addWidget(lblCfgJobsPrefix, 1, 4, 1, 1);

        btnCfgUseDefaultFileManager = new QToolButton(grpProgramOptions);
        btnCfgUseDefaultFileManager->setObjectName(QStringLiteral("btnCfgUseDefaultFileManager"));
        btnCfgUseDefaultFileManager->setMinimumSize(QSize(30, 30));
        btnCfgUseDefaultFileManager->setFont(font1);
        btnCfgUseDefaultFileManager->setIcon(icon);

        gLayoutProgramOptions->addWidget(btnCfgUseDefaultFileManager, 1, 3, 1, 1);

        lblCfgDocApp = new QLabel(grpProgramOptions);
        lblCfgDocApp->setObjectName(QStringLiteral("lblCfgDocApp"));
        lblCfgDocApp->setEnabled(false);
        lblCfgDocApp->setFont(font1);

        gLayoutProgramOptions->addWidget(lblCfgDocApp, 7, 0, 1, 1);

        txtCfgDocumentViewer = new QLineEdit(grpProgramOptions);
        txtCfgDocumentViewer->setObjectName(QStringLiteral("txtCfgDocumentViewer"));
        txtCfgDocumentViewer->setMinimumSize(QSize(0, 30));
        txtCfgDocumentViewer->setFont(font1);

        gLayoutProgramOptions->addWidget(txtCfgDocumentViewer, 5, 5, 1, 1);

        txtCfgDataFolder = new QLineEdit(grpProgramOptions);
        txtCfgDataFolder->setObjectName(QStringLiteral("txtCfgDataFolder"));
        txtCfgDataFolder->setMinimumSize(QSize(0, 30));
        txtCfgDataFolder->setFont(font1);

        gLayoutProgramOptions->addWidget(txtCfgDataFolder, 0, 5, 1, 1);

        btnCfgUseDefaultDataFolder = new QToolButton(grpProgramOptions);
        btnCfgUseDefaultDataFolder->setObjectName(QStringLiteral("btnCfgUseDefaultDataFolder"));
        btnCfgUseDefaultDataFolder->setMinimumSize(QSize(30, 30));
        btnCfgUseDefaultDataFolder->setFont(font1);
        btnCfgUseDefaultDataFolder->setIcon(icon);

        gLayoutProgramOptions->addWidget(btnCfgUseDefaultDataFolder, 0, 7, 1, 1);

        btnCfgChooseDataFolder = new QToolButton(grpProgramOptions);
        btnCfgChooseDataFolder->setObjectName(QStringLiteral("btnCfgChooseDataFolder"));
        btnCfgChooseDataFolder->setMinimumSize(QSize(30, 30));
        btnCfgChooseDataFolder->setFont(font1);
        btnCfgChooseDataFolder->setIcon(icon1);

        gLayoutProgramOptions->addWidget(btnCfgChooseDataFolder, 0, 6, 1, 1);

        txtCfgFileManager = new QLineEdit(grpProgramOptions);
        txtCfgFileManager->setObjectName(QStringLiteral("txtCfgFileManager"));
        txtCfgFileManager->setMinimumSize(QSize(0, 30));
        txtCfgFileManager->setFont(font1);

        gLayoutProgramOptions->addWidget(txtCfgFileManager, 1, 1, 1, 1);

        lblCfgReports = new QLabel(grpProgramOptions);
        lblCfgReports->setObjectName(QStringLiteral("lblCfgReports"));
        lblCfgReports->setFont(font1);

        gLayoutProgramOptions->addWidget(lblCfgReports, 8, 0, 1, 1);

        btnCfgUseDefaultXlsEditor = new QToolButton(grpProgramOptions);
        btnCfgUseDefaultXlsEditor->setObjectName(QStringLiteral("btnCfgUseDefaultXlsEditor"));
        btnCfgUseDefaultXlsEditor->setMinimumSize(QSize(30, 30));
        btnCfgUseDefaultXlsEditor->setFont(font1);
        btnCfgUseDefaultXlsEditor->setIcon(icon);

        gLayoutProgramOptions->addWidget(btnCfgUseDefaultXlsEditor, 7, 7, 1, 1);

        lblCfgXlsApp = new QLabel(grpProgramOptions);
        lblCfgXlsApp->setObjectName(QStringLiteral("lblCfgXlsApp"));
        lblCfgXlsApp->setEnabled(false);
        lblCfgXlsApp->setFont(font1);

        gLayoutProgramOptions->addWidget(lblCfgXlsApp, 7, 4, 1, 1);

        btnCfgUseDefaultDocEditor = new QToolButton(grpProgramOptions);
        btnCfgUseDefaultDocEditor->setObjectName(QStringLiteral("btnCfgUseDefaultDocEditor"));
        btnCfgUseDefaultDocEditor->setMinimumSize(QSize(30, 30));
        btnCfgUseDefaultDocEditor->setFont(font1);
        btnCfgUseDefaultDocEditor->setIcon(icon);

        gLayoutProgramOptions->addWidget(btnCfgUseDefaultDocEditor, 7, 3, 1, 1);

        btnCfgChooseDocEditor = new QToolButton(grpProgramOptions);
        btnCfgChooseDocEditor->setObjectName(QStringLiteral("btnCfgChooseDocEditor"));
        btnCfgChooseDocEditor->setMinimumSize(QSize(30, 30));
        btnCfgChooseDocEditor->setFont(font1);
        btnCfgChooseDocEditor->setIcon(icon1);

        gLayoutProgramOptions->addWidget(btnCfgChooseDocEditor, 7, 2, 1, 1);

        txtCfgPictureEditor = new QLineEdit(grpProgramOptions);
        txtCfgPictureEditor->setObjectName(QStringLiteral("txtCfgPictureEditor"));
        txtCfgPictureEditor->setMinimumSize(QSize(0, 30));
        txtCfgPictureEditor->setFont(font1);

        gLayoutProgramOptions->addWidget(txtCfgPictureEditor, 3, 5, 1, 1);

        txtCfgESTIMATE = new QLineEdit(grpProgramOptions);
        txtCfgESTIMATE->setObjectName(QStringLiteral("txtCfgESTIMATE"));
        txtCfgESTIMATE->setMinimumSize(QSize(0, 30));
        txtCfgESTIMATE->setFont(font1);

        gLayoutProgramOptions->addWidget(txtCfgESTIMATE, 8, 5, 1, 1);

        lblCfgDocumentViewer = new QLabel(grpProgramOptions);
        lblCfgDocumentViewer->setObjectName(QStringLiteral("lblCfgDocumentViewer"));
        lblCfgDocumentViewer->setEnabled(false);
        lblCfgDocumentViewer->setFont(font1);

        gLayoutProgramOptions->addWidget(lblCfgDocumentViewer, 5, 4, 1, 1);

        btnCfgChooseFileManager = new QToolButton(grpProgramOptions);
        btnCfgChooseFileManager->setObjectName(QStringLiteral("btnCfgChooseFileManager"));
        btnCfgChooseFileManager->setMinimumSize(QSize(30, 30));
        btnCfgChooseFileManager->setFont(font1);
        btnCfgChooseFileManager->setIcon(icon1);

        gLayoutProgramOptions->addWidget(btnCfgChooseFileManager, 1, 2, 1, 1);

        lblCfgEmailClient = new QLabel(grpProgramOptions);
        lblCfgEmailClient->setObjectName(QStringLiteral("lblCfgEmailClient"));
        lblCfgEmailClient->setEnabled(false);
        lblCfgEmailClient->setFont(font1);

        gLayoutProgramOptions->addWidget(lblCfgEmailClient, 5, 0, 1, 1);

        txtCfgConfigFile = new QLineEdit(grpProgramOptions);
        txtCfgConfigFile->setObjectName(QStringLiteral("txtCfgConfigFile"));
        txtCfgConfigFile->setMinimumSize(QSize(0, 30));
        txtCfgConfigFile->setFont(font1);

        gLayoutProgramOptions->addWidget(txtCfgConfigFile, 0, 1, 1, 1);

        txtCfgDocEditor = new QLineEdit(grpProgramOptions);
        txtCfgDocEditor->setObjectName(QStringLiteral("txtCfgDocEditor"));
        txtCfgDocEditor->setMinimumSize(QSize(0, 30));
        txtCfgDocEditor->setFont(font1);

        gLayoutProgramOptions->addWidget(txtCfgDocEditor, 7, 1, 1, 1);

        btnCfgUseDefaultConfigFile = new QToolButton(grpProgramOptions);
        btnCfgUseDefaultConfigFile->setObjectName(QStringLiteral("btnCfgUseDefaultConfigFile"));
        btnCfgUseDefaultConfigFile->setMinimumSize(QSize(30, 30));
        btnCfgUseDefaultConfigFile->setFont(font1);
        btnCfgUseDefaultConfigFile->setIcon(icon);

        gLayoutProgramOptions->addWidget(btnCfgUseDefaultConfigFile, 0, 3, 1, 1);

        btnCfgChooseBaseDir = new QToolButton(grpProgramOptions);
        btnCfgChooseBaseDir->setObjectName(QStringLiteral("btnCfgChooseBaseDir"));
        btnCfgChooseBaseDir->setMinimumSize(QSize(30, 30));
        btnCfgChooseBaseDir->setFont(font1);
        btnCfgChooseBaseDir->setIcon(icon1);

        gLayoutProgramOptions->addWidget(btnCfgChooseBaseDir, 1, 6, 1, 1);

        btnCfgChooseEMailClient = new QToolButton(grpProgramOptions);
        btnCfgChooseEMailClient->setObjectName(QStringLiteral("btnCfgChooseEMailClient"));
        btnCfgChooseEMailClient->setMinimumSize(QSize(30, 30));
        btnCfgChooseEMailClient->setFont(font1);
        btnCfgChooseEMailClient->setIcon(icon1);

        gLayoutProgramOptions->addWidget(btnCfgChooseEMailClient, 5, 2, 1, 1);

        lblCfgPictureViewer = new QLabel(grpProgramOptions);
        lblCfgPictureViewer->setObjectName(QStringLiteral("lblCfgPictureViewer"));
        lblCfgPictureViewer->setEnabled(false);
        lblCfgPictureViewer->setFont(font1);

        gLayoutProgramOptions->addWidget(lblCfgPictureViewer, 3, 0, 1, 1);

        btnCfgUseDefaultDocumentViewer = new QToolButton(grpProgramOptions);
        btnCfgUseDefaultDocumentViewer->setObjectName(QStringLiteral("btnCfgUseDefaultDocumentViewer"));
        btnCfgUseDefaultDocumentViewer->setMinimumSize(QSize(30, 30));
        btnCfgUseDefaultDocumentViewer->setFont(font1);
        btnCfgUseDefaultDocumentViewer->setIcon(icon);

        gLayoutProgramOptions->addWidget(btnCfgUseDefaultDocumentViewer, 5, 7, 1, 1);

        txtCfgJobsPrefix = new QLineEdit(grpProgramOptions);
        txtCfgJobsPrefix->setObjectName(QStringLiteral("txtCfgJobsPrefix"));
        txtCfgJobsPrefix->setMinimumSize(QSize(0, 30));
        txtCfgJobsPrefix->setFont(font1);

        gLayoutProgramOptions->addWidget(txtCfgJobsPrefix, 1, 5, 1, 1);

        lblCfgDataFolder = new QLabel(grpProgramOptions);
        lblCfgDataFolder->setObjectName(QStringLiteral("lblCfgDataFolder"));
        lblCfgDataFolder->setEnabled(false);
        lblCfgDataFolder->setFont(font1);

        gLayoutProgramOptions->addWidget(lblCfgDataFolder, 0, 4, 1, 1);


        horizontalLayout->addWidget(grpProgramOptions);


        horizontalLayout_2->addLayout(horizontalLayout);


        retranslateUi(configDialog);

        QMetaObject::connectSlotsByName(configDialog);
    } // setupUi

    void retranslateUi(QDialog *configDialog)
    {
        configDialog->setWindowTitle(QApplication::translate("configDialog", "Config options", 0));
        grpProgramOptions->setTitle(QString());
        btnCfgUseDefaultESTIMATEDir->setText(QApplication::translate("configDialog", "...", 0));
        lblCfgFileManager->setText(QApplication::translate("configDialog", "File manager", 0));
#ifndef QT_NO_TOOLTIP
        btnCfgChoosePictureViewer->setToolTip(QApplication::translate("configDialog", "Browse ...", 0));
#endif // QT_NO_TOOLTIP
        btnCfgChoosePictureViewer->setText(QApplication::translate("configDialog", "...", 0));
#ifndef QT_NO_TOOLTIP
        btnCfgChooseConfigFile->setToolTip(QApplication::translate("configDialog", "Browse ...", 0));
#endif // QT_NO_TOOLTIP
        btnCfgChooseConfigFile->setText(QApplication::translate("configDialog", "...", 0));
        lblCfgConfigFile->setText(QApplication::translate("configDialog", "Config file:", 0));
#ifndef QT_NO_TOOLTIP
        btnCfgChooseXlsEditor->setToolTip(QApplication::translate("configDialog", "Browse ...", 0));
#endif // QT_NO_TOOLTIP
        btnCfgChooseXlsEditor->setText(QApplication::translate("configDialog", "...", 0));
        lblCfgESTIMATE->setText(QApplication::translate("configDialog", "Estimates directory:", 0));
#ifndef QT_NO_TOOLTIP
        btnCfgChoosePictureEditor->setToolTip(QApplication::translate("configDialog", "Browse ...", 0));
#endif // QT_NO_TOOLTIP
        btnCfgChoosePictureEditor->setText(QApplication::translate("configDialog", "...", 0));
#ifndef QT_NO_TOOLTIP
        btnCfgUseDefaultPictureEditor->setToolTip(QApplication::translate("configDialog", "Use default", 0));
#endif // QT_NO_TOOLTIP
        btnCfgUseDefaultPictureEditor->setText(QApplication::translate("configDialog", "...", 0));
        lblCfgPictureEditor->setText(QApplication::translate("configDialog", "Picture editor:", 0));
#ifndef QT_NO_TOOLTIP
        btnCfgUseDefaultPictureViewer->setToolTip(QApplication::translate("configDialog", "Use default", 0));
#endif // QT_NO_TOOLTIP
        btnCfgUseDefaultPictureViewer->setText(QApplication::translate("configDialog", "...", 0));
#ifndef QT_NO_TOOLTIP
        btnCfgUseDefaultBaseDir->setToolTip(QApplication::translate("configDialog", "Use default", 0));
#endif // QT_NO_TOOLTIP
        btnCfgUseDefaultBaseDir->setText(QApplication::translate("configDialog", "...", 0));
#ifndef QT_NO_TOOLTIP
        btnCfgChooseDocViewer->setToolTip(QApplication::translate("configDialog", "Browse ...", 0));
#endif // QT_NO_TOOLTIP
        btnCfgChooseDocViewer->setText(QApplication::translate("configDialog", "...", 0));
        btnCfgUseDefaultReportsDir->setText(QApplication::translate("configDialog", "...", 0));
        lblCfgJobsPrefix->setText(QApplication::translate("configDialog", "Job's project base folder:", 0));
#ifndef QT_NO_TOOLTIP
        btnCfgUseDefaultFileManager->setToolTip(QApplication::translate("configDialog", "Use default", 0));
#endif // QT_NO_TOOLTIP
        btnCfgUseDefaultFileManager->setText(QApplication::translate("configDialog", "...", 0));
        lblCfgDocApp->setText(QApplication::translate("configDialog", "MS Document editor:", 0));
#ifndef QT_NO_TOOLTIP
        btnCfgUseDefaultDataFolder->setToolTip(QApplication::translate("configDialog", "Use default", 0));
#endif // QT_NO_TOOLTIP
        btnCfgUseDefaultDataFolder->setText(QApplication::translate("configDialog", "...", 0));
#ifndef QT_NO_TOOLTIP
        btnCfgChooseDataFolder->setToolTip(QApplication::translate("configDialog", "Browse ...", 0));
#endif // QT_NO_TOOLTIP
        btnCfgChooseDataFolder->setText(QApplication::translate("configDialog", "...", 0));
        lblCfgReports->setText(QApplication::translate("configDialog", "Reports directory", 0));
#ifndef QT_NO_TOOLTIP
        btnCfgUseDefaultXlsEditor->setToolTip(QApplication::translate("configDialog", "Use default", 0));
#endif // QT_NO_TOOLTIP
        btnCfgUseDefaultXlsEditor->setText(QApplication::translate("configDialog", "...", 0));
        lblCfgXlsApp->setText(QApplication::translate("configDialog", "Spreadsheet editor:", 0));
#ifndef QT_NO_TOOLTIP
        btnCfgUseDefaultDocEditor->setToolTip(QApplication::translate("configDialog", "Use default", 0));
#endif // QT_NO_TOOLTIP
        btnCfgUseDefaultDocEditor->setText(QApplication::translate("configDialog", "...", 0));
#ifndef QT_NO_TOOLTIP
        btnCfgChooseDocEditor->setToolTip(QApplication::translate("configDialog", "Browse ...", 0));
#endif // QT_NO_TOOLTIP
        btnCfgChooseDocEditor->setText(QApplication::translate("configDialog", "...", 0));
        lblCfgDocumentViewer->setText(QApplication::translate("configDialog", "Document viewer:", 0));
#ifndef QT_NO_TOOLTIP
        btnCfgChooseFileManager->setToolTip(QApplication::translate("configDialog", "Browse ...", 0));
#endif // QT_NO_TOOLTIP
        btnCfgChooseFileManager->setText(QApplication::translate("configDialog", "...", 0));
        lblCfgEmailClient->setText(QApplication::translate("configDialog", "EMail client:", 0));
#ifndef QT_NO_TOOLTIP
        btnCfgUseDefaultConfigFile->setToolTip(QApplication::translate("configDialog", "Use default", 0));
#endif // QT_NO_TOOLTIP
        btnCfgUseDefaultConfigFile->setText(QApplication::translate("configDialog", "...", 0));
#ifndef QT_NO_TOOLTIP
        btnCfgChooseBaseDir->setToolTip(QApplication::translate("configDialog", "Browse ...", 0));
#endif // QT_NO_TOOLTIP
        btnCfgChooseBaseDir->setText(QApplication::translate("configDialog", "...", 0));
#ifndef QT_NO_TOOLTIP
        btnCfgChooseEMailClient->setToolTip(QApplication::translate("configDialog", "Browse ...", 0));
#endif // QT_NO_TOOLTIP
        btnCfgChooseEMailClient->setText(QApplication::translate("configDialog", "...", 0));
        lblCfgPictureViewer->setText(QApplication::translate("configDialog", "Picture viewer:", 0));
#ifndef QT_NO_TOOLTIP
        btnCfgUseDefaultDocumentViewer->setToolTip(QApplication::translate("configDialog", "Use default", 0));
#endif // QT_NO_TOOLTIP
        btnCfgUseDefaultDocumentViewer->setText(QApplication::translate("configDialog", "...", 0));
        lblCfgDataFolder->setText(QApplication::translate("configDialog", "Data folder:", 0));
    } // retranslateUi

};

namespace Ui {
    class configDialog: public Ui_configDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CONFIGDIALOG_H
