/********************************************************************************
** Form generated from reading UI file 'companypurchasesui.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_COMPANYPURCHASESUI_H
#define UI_COMPANYPURCHASESUI_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <vmtablewidget.h>
#include <vmwidgets.h>

QT_BEGIN_NAMESPACE

class Ui_companyPurchasesUI
{
public:
    QGridLayout *gridLayout_2;
    QGridLayout *gridLayout;
    QHBoxLayout *layoutBrowseControls;
    QPushButton *btnCPFirst;
    QPushButton *btnCPPrev;
    QPushButton *btnCPNext;
    QPushButton *btnCPLast;
    QSpacerItem *horizontalSpacer_3;
    QLabel *lblCPID;
    vmLineEdit *txtCPID;
    QHBoxLayout *layout;
    vmLineEdit *txtCPSearch;
    QPushButton *btnCPSearch;
    QFrame *line;
    QPushButton *btnCPAdd;
    QPushButton *btnCPEdit;
    QPushButton *btnCPRemove;
    vmLineEditWithButton *txtCPPayValue;
    QVBoxLayout *layoutCPTable;
    QLabel *lblCP12;
    vmTableWidget *tableItems;
    QLabel *lblCP13_2;
    vmTableWidget *tablePayments;
    vmDateEdit *dteCPDeliveryDate;
    vmLineEdit *txtCPDeliveryMethod;
    QLabel *lblCPPayValue;
    QLabel *lblCPDeliveryMethod;
    vmLineEdit *txtCPNotes;
    QLabel *lblCPNotes;
    QLabel *lblDeliveryDate;
    vmDateEdit *dteCPDate;
    QLabel *lblCPDate;
    QLabel *lblCPSupplier;
    QHBoxLayout *horizontalLayout_2;
    vmLineEdit *txtCPSupplier;
    QToolButton *btnCPShowSupplier;
    QPushButton *btnClose;

    void setupUi(QDialog *companyPurchasesUI)
    {
        if (companyPurchasesUI->objectName().isEmpty())
            companyPurchasesUI->setObjectName(QStringLiteral("companyPurchasesUI"));
        companyPurchasesUI->resize(731, 544);
        gridLayout_2 = new QGridLayout(companyPurchasesUI);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        gridLayout = new QGridLayout();
        gridLayout->setSpacing(5);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        gridLayout->setContentsMargins(5, 5, 5, 5);
        layoutBrowseControls = new QHBoxLayout();
        layoutBrowseControls->setSpacing(2);
        layoutBrowseControls->setObjectName(QStringLiteral("layoutBrowseControls"));
        btnCPFirst = new QPushButton(companyPurchasesUI);
        btnCPFirst->setObjectName(QStringLiteral("btnCPFirst"));
        btnCPFirst->setEnabled(false);
        btnCPFirst->setMinimumSize(QSize(50, 0));
        btnCPFirst->setMaximumSize(QSize(100, 30));
        QIcon icon;
        icon.addFile(QStringLiteral(":/resources/browse-controls/first_rec.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnCPFirst->setIcon(icon);

        layoutBrowseControls->addWidget(btnCPFirst);

        btnCPPrev = new QPushButton(companyPurchasesUI);
        btnCPPrev->setObjectName(QStringLiteral("btnCPPrev"));
        btnCPPrev->setEnabled(false);
        btnCPPrev->setMinimumSize(QSize(50, 0));
        btnCPPrev->setMaximumSize(QSize(100, 30));
        QIcon icon1;
        icon1.addFile(QStringLiteral(":/resources/browse-controls/prev_rec.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnCPPrev->setIcon(icon1);

        layoutBrowseControls->addWidget(btnCPPrev);

        btnCPNext = new QPushButton(companyPurchasesUI);
        btnCPNext->setObjectName(QStringLiteral("btnCPNext"));
        btnCPNext->setEnabled(false);
        btnCPNext->setMinimumSize(QSize(50, 0));
        btnCPNext->setMaximumSize(QSize(100, 30));
        QIcon icon2;
        icon2.addFile(QStringLiteral(":/resources/browse-controls/next_rec.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnCPNext->setIcon(icon2);

        layoutBrowseControls->addWidget(btnCPNext);

        btnCPLast = new QPushButton(companyPurchasesUI);
        btnCPLast->setObjectName(QStringLiteral("btnCPLast"));
        btnCPLast->setEnabled(false);
        btnCPLast->setMinimumSize(QSize(50, 0));
        btnCPLast->setMaximumSize(QSize(100, 30));
        QIcon icon3;
        icon3.addFile(QStringLiteral(":/resources/browse-controls/last_rec.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnCPLast->setIcon(icon3);

        layoutBrowseControls->addWidget(btnCPLast);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        layoutBrowseControls->addItem(horizontalSpacer_3);

        lblCPID = new QLabel(companyPurchasesUI);
        lblCPID->setObjectName(QStringLiteral("lblCPID"));

        layoutBrowseControls->addWidget(lblCPID);

        txtCPID = new vmLineEdit(companyPurchasesUI);
        txtCPID->setObjectName(QStringLiteral("txtCPID"));
        txtCPID->setEnabled(false);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(txtCPID->sizePolicy().hasHeightForWidth());
        txtCPID->setSizePolicy(sizePolicy);
        txtCPID->setMaximumSize(QSize(80, 30));

        layoutBrowseControls->addWidget(txtCPID);


        gridLayout->addLayout(layoutBrowseControls, 0, 1, 1, 4);

        layout = new QHBoxLayout();
        layout->setSpacing(2);
        layout->setObjectName(QStringLiteral("layout"));
        txtCPSearch = new vmLineEdit(companyPurchasesUI);
        txtCPSearch->setObjectName(QStringLiteral("txtCPSearch"));
        txtCPSearch->setReadOnly(false);

        layout->addWidget(txtCPSearch);

        btnCPSearch = new QPushButton(companyPurchasesUI);
        btnCPSearch->setObjectName(QStringLiteral("btnCPSearch"));
        btnCPSearch->setEnabled(false);
        QIcon icon4;
        icon4.addFile(QStringLiteral(":/resources/replace-all.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnCPSearch->setIcon(icon4);
        btnCPSearch->setCheckable(true);

        layout->addWidget(btnCPSearch);

        line = new QFrame(companyPurchasesUI);
        line->setObjectName(QStringLiteral("line"));
        line->setFrameShape(QFrame::VLine);
        line->setFrameShadow(QFrame::Sunken);

        layout->addWidget(line);

        btnCPAdd = new QPushButton(companyPurchasesUI);
        btnCPAdd->setObjectName(QStringLiteral("btnCPAdd"));
        btnCPAdd->setMaximumSize(QSize(100, 30));
        QIcon icon5;
        icon5.addFile(QStringLiteral(":/resources/browse-controls/add.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnCPAdd->setIcon(icon5);
        btnCPAdd->setCheckable(true);

        layout->addWidget(btnCPAdd);

        btnCPEdit = new QPushButton(companyPurchasesUI);
        btnCPEdit->setObjectName(QStringLiteral("btnCPEdit"));
        btnCPEdit->setEnabled(false);
        btnCPEdit->setMaximumSize(QSize(100, 30));
        QIcon icon6;
        icon6.addFile(QStringLiteral(":/resources/browse-controls/edit.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnCPEdit->setIcon(icon6);
        btnCPEdit->setCheckable(true);

        layout->addWidget(btnCPEdit);

        btnCPRemove = new QPushButton(companyPurchasesUI);
        btnCPRemove->setObjectName(QStringLiteral("btnCPRemove"));
        btnCPRemove->setEnabled(false);
        btnCPRemove->setMaximumSize(QSize(100, 30));
        QIcon icon7;
        icon7.addFile(QStringLiteral(":/resources/browse-controls/remove.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnCPRemove->setIcon(icon7);

        layout->addWidget(btnCPRemove);

        layout->setStretch(0, 2);
        layout->setStretch(1, 1);
        layout->setStretch(3, 1);
        layout->setStretch(4, 1);
        layout->setStretch(5, 1);

        gridLayout->addLayout(layout, 1, 1, 1, 4);

        txtCPPayValue = new vmLineEditWithButton(companyPurchasesUI);
        txtCPPayValue->setObjectName(QStringLiteral("txtCPPayValue"));
        txtCPPayValue->setMinimumSize(QSize(50, 30));

        gridLayout->addWidget(txtCPPayValue, 10, 1, 1, 1);

        layoutCPTable = new QVBoxLayout();
        layoutCPTable->setSpacing(5);
        layoutCPTable->setObjectName(QStringLiteral("layoutCPTable"));
        layoutCPTable->setContentsMargins(5, 5, 5, 5);
        lblCP12 = new QLabel(companyPurchasesUI);
        lblCP12->setObjectName(QStringLiteral("lblCP12"));
        lblCP12->setMaximumSize(QSize(16777215, 30));

        layoutCPTable->addWidget(lblCP12);

        tableItems = new vmTableWidget(companyPurchasesUI);
        tableItems->setObjectName(QStringLiteral("tableItems"));

        layoutCPTable->addWidget(tableItems);

        lblCP13_2 = new QLabel(companyPurchasesUI);
        lblCP13_2->setObjectName(QStringLiteral("lblCP13_2"));

        layoutCPTable->addWidget(lblCP13_2);

        tablePayments = new vmTableWidget(companyPurchasesUI);
        tablePayments->setObjectName(QStringLiteral("tablePayments"));

        layoutCPTable->addWidget(tablePayments);


        gridLayout->addLayout(layoutCPTable, 11, 1, 1, 4);

        dteCPDeliveryDate = new vmDateEdit(companyPurchasesUI);
        dteCPDeliveryDate->setObjectName(QStringLiteral("dteCPDeliveryDate"));
        dteCPDeliveryDate->setMinimumSize(QSize(0, 30));

        gridLayout->addWidget(dteCPDeliveryDate, 7, 4, 1, 1);

        txtCPDeliveryMethod = new vmLineEdit(companyPurchasesUI);
        txtCPDeliveryMethod->setObjectName(QStringLiteral("txtCPDeliveryMethod"));
        txtCPDeliveryMethod->setReadOnly(true);

        gridLayout->addWidget(txtCPDeliveryMethod, 10, 2, 1, 1);

        lblCPPayValue = new QLabel(companyPurchasesUI);
        lblCPPayValue->setObjectName(QStringLiteral("lblCPPayValue"));
        lblCPPayValue->setMaximumSize(QSize(16777215, 30));

        gridLayout->addWidget(lblCPPayValue, 9, 1, 1, 1);

        lblCPDeliveryMethod = new QLabel(companyPurchasesUI);
        lblCPDeliveryMethod->setObjectName(QStringLiteral("lblCPDeliveryMethod"));

        gridLayout->addWidget(lblCPDeliveryMethod, 9, 2, 1, 1);

        txtCPNotes = new vmLineEdit(companyPurchasesUI);
        txtCPNotes->setObjectName(QStringLiteral("txtCPNotes"));
        txtCPNotes->setMinimumSize(QSize(300, 0));
        txtCPNotes->setReadOnly(true);

        gridLayout->addWidget(txtCPNotes, 10, 4, 1, 1);

        lblCPNotes = new QLabel(companyPurchasesUI);
        lblCPNotes->setObjectName(QStringLiteral("lblCPNotes"));

        gridLayout->addWidget(lblCPNotes, 9, 4, 1, 1);

        lblDeliveryDate = new QLabel(companyPurchasesUI);
        lblDeliveryDate->setObjectName(QStringLiteral("lblDeliveryDate"));
        lblDeliveryDate->setMaximumSize(QSize(16777215, 30));

        gridLayout->addWidget(lblDeliveryDate, 6, 4, 1, 1);

        dteCPDate = new vmDateEdit(companyPurchasesUI);
        dteCPDate->setObjectName(QStringLiteral("dteCPDate"));
        dteCPDate->setMinimumSize(QSize(30, 30));

        gridLayout->addWidget(dteCPDate, 7, 2, 1, 1);

        lblCPDate = new QLabel(companyPurchasesUI);
        lblCPDate->setObjectName(QStringLiteral("lblCPDate"));
        lblCPDate->setMaximumSize(QSize(16777215, 30));

        gridLayout->addWidget(lblCPDate, 6, 2, 1, 1);

        lblCPSupplier = new QLabel(companyPurchasesUI);
        lblCPSupplier->setObjectName(QStringLiteral("lblCPSupplier"));
        lblCPSupplier->setMinimumSize(QSize(0, 0));
        lblCPSupplier->setMaximumSize(QSize(16777215, 30));

        gridLayout->addWidget(lblCPSupplier, 6, 1, 1, 1);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(2);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        txtCPSupplier = new vmLineEdit(companyPurchasesUI);
        txtCPSupplier->setObjectName(QStringLiteral("txtCPSupplier"));
        txtCPSupplier->setMaximumSize(QSize(16777215, 16777215));
        txtCPSupplier->setReadOnly(true);

        horizontalLayout_2->addWidget(txtCPSupplier);

        btnCPShowSupplier = new QToolButton(companyPurchasesUI);
        btnCPShowSupplier->setObjectName(QStringLiteral("btnCPShowSupplier"));
        QIcon icon8;
        icon8.addFile(QStringLiteral(":/resources/arrow-down.png"), QSize(), QIcon::Normal, QIcon::Off);
        btnCPShowSupplier->setIcon(icon8);
        btnCPShowSupplier->setCheckable(true);

        horizontalLayout_2->addWidget(btnCPShowSupplier);


        gridLayout->addLayout(horizontalLayout_2, 7, 1, 1, 1);


        gridLayout_2->addLayout(gridLayout, 0, 0, 1, 1);

        btnClose = new QPushButton(companyPurchasesUI);
        btnClose->setObjectName(QStringLiteral("btnClose"));
        btnClose->setMaximumSize(QSize(200, 16777215));

        gridLayout_2->addWidget(btnClose, 1, 0, 1, 1);

        QWidget::setTabOrder(btnCPFirst, btnCPPrev);
        QWidget::setTabOrder(btnCPPrev, btnCPNext);
        QWidget::setTabOrder(btnCPNext, btnCPLast);
        QWidget::setTabOrder(btnCPLast, txtCPSearch);
        QWidget::setTabOrder(txtCPSearch, btnCPSearch);
        QWidget::setTabOrder(btnCPSearch, btnCPAdd);
        QWidget::setTabOrder(btnCPAdd, btnCPEdit);
        QWidget::setTabOrder(btnCPEdit, btnCPRemove);
        QWidget::setTabOrder(btnCPRemove, txtCPSupplier);
        QWidget::setTabOrder(txtCPSupplier, btnCPShowSupplier);
        QWidget::setTabOrder(btnCPShowSupplier, btnClose);

        retranslateUi(companyPurchasesUI);

        QMetaObject::connectSlotsByName(companyPurchasesUI);
    } // setupUi

    void retranslateUi(QDialog *companyPurchasesUI)
    {
        companyPurchasesUI->setWindowTitle(QApplication::translate("companyPurchasesUI", "Dialog", 0));
        btnCPFirst->setText(QString());
        btnCPPrev->setText(QString());
        btnCPNext->setText(QString());
        btnCPLast->setText(QString());
        lblCPID->setText(QApplication::translate("companyPurchasesUI", "ID:", 0));
        btnCPSearch->setText(QApplication::translate("companyPurchasesUI", "Search", 0));
        btnCPAdd->setText(QString());
        btnCPEdit->setText(QString());
        btnCPRemove->setText(QString());
        lblCP12->setText(QApplication::translate("companyPurchasesUI", "Items:", 0));
        lblCP13_2->setText(QApplication::translate("companyPurchasesUI", "Payment history:", 0));
        lblCPPayValue->setText(QApplication::translate("companyPurchasesUI", "Price paid:", 0));
        lblCPDeliveryMethod->setText(QApplication::translate("companyPurchasesUI", "Delivery method:", 0));
        lblCPNotes->setText(QApplication::translate("companyPurchasesUI", "Notes:", 0));
        lblDeliveryDate->setText(QApplication::translate("companyPurchasesUI", "Delivery date:", 0));
        lblCPDate->setText(QApplication::translate("companyPurchasesUI", "Purchase date:", 0));
        lblCPSupplier->setText(QApplication::translate("companyPurchasesUI", "Supplier:", 0));
        btnCPShowSupplier->setText(QApplication::translate("companyPurchasesUI", "...", 0));
        btnClose->setText(QApplication::translate("companyPurchasesUI", "Close", 0));
    } // retranslateUi

};

namespace Ui {
    class companyPurchasesUI: public Ui_companyPurchasesUI {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_COMPANYPURCHASESUI_H
