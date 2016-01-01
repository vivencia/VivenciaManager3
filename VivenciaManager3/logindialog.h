#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

class vmLineEdit;

class QLabel;
class QPushButton;
class QToolButton;

class loginDialog : public QDialog
{

    Q_OBJECT

public:
    loginDialog ();
    void showDialog ();

public slots:
    void btnTryToLogin_clicked ();
    void btnCancelLogin_clicked ();
    void checkUserName ();
    void checkPassword ();
    void btnConfigUsers_clicked ();

private:
    vmLineEdit* txtUserName;
    vmLineEdit* txtPassword;
    QToolButton* btnConfigUsers;
    QPushButton* btnTryToLogin;
    QPushButton* btnCancelLogin;
    QLabel* lblStatus;
};

#endif // LOGINDIALOG_H
