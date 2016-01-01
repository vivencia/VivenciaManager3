#include "logindialog.h"
#include "global.h"
#include "usermanagement.h"
#include "usermanagementui.h"
#include "vmwidgets.h"
#include "vmnotify.h"
#include "configops.h"
#include "stringrecord.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QToolButton>
#include <QLabel>
#include <QFont>
#include <QApplication>
#include <QDesktopWidget>

static const QString CFG_PASSWORD_FIELD ( QStringLiteral ( "CONVENIENCE" ) );

loginDialog::loginDialog ()
    : QDialog ()
{
    QLabel* lblUserName ( new QLabel ( tr ( "User: " ), this ) );
    QLabel* lblPassword ( new QLabel ( tr ( "Password: " ), this ) );

    txtUserName = new vmLineEdit ( this );
    txtUserName->setMinimumSize ( 150, 30 );
    txtUserName->setEditable ( true );
    connect ( txtUserName, SIGNAL ( textAltered () ), this, SLOT ( checkUserName () ) );

    btnConfigUsers = new QToolButton ( this );
    btnConfigUsers->setFixedSize ( 30, 30 );
    btnConfigUsers->setIcon ( ICON ( "config-users.png" ) );
    btnConfigUsers->setToolTip ( tr ( "Manage users" ) );
    connect ( btnConfigUsers, SIGNAL ( clicked () ), this, SLOT ( btnConfigUsers_clicked () ) );

    txtPassword = new vmLineEdit ( this );
    txtPassword->setEchoMode ( QLineEdit::Password );
    txtPassword->setMinimumSize ( 150, 30 );
    txtPassword->setEditable ( false );
    connect ( txtPassword, SIGNAL ( textAltered () ), this, SLOT ( checkPassword () ) );

    btnTryToLogin = new QPushButton ( QStringLiteral ( "Login" ), this );
    btnTryToLogin->setDefault ( true );
    btnTryToLogin->setEnabled ( false );
    btnTryToLogin->setMinimumSize ( 100, 30 );
    connect ( btnTryToLogin, SIGNAL ( clicked () ), this, SLOT ( btnTryToLogin_clicked () ) );

    btnCancelLogin = new QPushButton ( tr ( "Exit" ), this );
    btnCancelLogin->setMinimumSize ( 100, 30 );
    connect ( btnCancelLogin, SIGNAL ( clicked () ), this, SLOT ( btnCancelLogin_clicked () ) );

    lblStatus = new QLabel ( this );
    lblStatus->setFrameStyle ( QFrame::StyledPanel | QFrame::Sunken );
    lblStatus->setMinimumHeight ( 30 );
    QFont fntStatus ( lblStatus->font () );
    fntStatus.setBold ( true );
    lblStatus->setFont ( fntStatus );

    QHBoxLayout* line1 ( new QHBoxLayout );
    line1->setSpacing ( 1 );
    line1->addWidget ( lblUserName, 0 );
    line1->addWidget ( txtUserName, 1 );
    line1->addWidget ( btnConfigUsers, 0 );

    QHBoxLayout* line2 ( new QHBoxLayout );
    line2->setSpacing ( 1 );
    line2->addWidget ( lblPassword, 0 );
    line2->addWidget ( txtPassword, 1 );

    QHBoxLayout* line3 ( new QHBoxLayout );
    line3->setSpacing ( 1 );
    line3->addWidget ( btnTryToLogin, 1 );
    line3->addWidget ( btnCancelLogin, 1 );

    QVBoxLayout* mainLayout ( new QVBoxLayout );
    mainLayout->setSpacing ( 1 );
    mainLayout->addLayout ( line1, 1 );
    mainLayout->addLayout ( line2, 1 );
    mainLayout->addLayout ( line3, 1 );
    mainLayout->addWidget ( lblStatus, 1 );

    setLayout ( mainLayout );
    setTabOrder ( txtUserName, btnConfigUsers );
    setTabOrder ( btnConfigUsers, txtPassword );
    setTabOrder ( txtPassword, btnTryToLogin );
    setTabOrder ( btnTryToLogin, btnCancelLogin );

    setWindowModality ( Qt::ApplicationModal );
    setWindowTitle ( QLatin1String ( "Login - " ) + PROGRAM_NAME );
    setWindowIcon ( ICON ( "vm-logo-22x22.png" ) );
    setMinimumWidth ( 350 );
}

void loginDialog::showDialog ()
{
    const QString& username ( CONFIG ()->readConfigFile ( LAST_LOGGED_USER ) );
    if ( !username.isEmpty () ) {
        txtUserName->setText ( username, true );
        txtPassword->setFocus ();
    }
    exec ();
}

void loginDialog::btnTryToLogin_clicked ()
{
    CONFIG ()->writeConfigFile ( LAST_LOGGED_USER, txtUserName->text () );
    done ( QDialog::Accepted );
}

void loginDialog::btnCancelLogin_clicked ()
{
    done ( QDialog::Rejected );
}

void loginDialog::checkUserName ()
{
    USRMNGR ()->checkUserName ( txtUserName->text () );
    txtPassword->setEditable ( USRMNGR ()->returnCode () == USER_REGISTERED );
    lblStatus->setText ( USRMNGR ()->returnCodeString () );
}

void loginDialog::checkPassword ()
{
    USRMNGR ()->checkPassword ( txtUserName->text (), txtPassword->text () );
    btnTryToLogin->setEnabled ( USRMNGR ()->returnCode () == PASSWORD_MATCHES );
    lblStatus->setText ( USRMNGR ()->returnCodeString () );
}

void loginDialog::btnConfigUsers_clicked ()
{
    USRMNGR ()->showConfigWindow ( txtUserName->text (), true );
}
