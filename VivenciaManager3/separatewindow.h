#ifndef SEPARATEWINDOW_H
#define SEPARATEWINDOW_H

#include <QDialog>

#include <functional>

class QCloseEvent;
class QVBoxLayout;
class QHBoxLayout;
class QPushButton;

class separateWindow : public QDialog
{

public:
	explicit separateWindow ( QWidget* child );

	void addChild ( QWidget* child );
	void addToolBar ( QWidget* toolbar );

	void setCallbackForReturningToParent ( const std::function<void ( QWidget*)>& func ) { funcReturnToParent = func; }
	void showSeparate ( const QString& window_title, const bool b_exec = false, const Qt::WindowStates w_state = Qt::WindowActive );
	void returnToParent ();

protected:
	void closeEvent ( QCloseEvent* e );
	bool eventFilter ( QObject* o, QEvent* e );

	int exec ();
	void open ();
	void show ();
	void accept ();
	void reject ();
	void done ( int );
	void hide ();

	void childCloseRequested ( const int ); // intercept a child'd close request and handle and accept it
	void childHideRequested ();
	void childShowRequested ();

private:
	QWidget* m_child;
	QWidget* m_toolbar;
	QHBoxLayout* mToolBarLayout;
	QVBoxLayout* mainLayout;
	QPushButton* btnReturn;
	bool mb_Active, mb_Visible;

	std::function<void ( QWidget*)> funcReturnToParent;
};

#endif // SEPARATEWINDOW_H
