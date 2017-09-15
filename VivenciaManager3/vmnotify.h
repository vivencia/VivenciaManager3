#ifndef VMNOTIFY_H
#define VMNOTIFY_H

#include "vmlist.h"
#include "completers.h"

#include <QDialog>
#include <QSize>
#include <QPointer>
#include <QEventLoop>

#include <functional>

class vmTaskPanel;
class vmActionGroup;
class vmNotify;

class QTimer;
class QSystemTrayIcon;
class QPixmap;
class QPushButton;
class QVBoxLayout;
class QTimerEvent;

enum { MESSAGE_BTN_OK = 1, MESSAGE_BTN_CANCEL = 0, MESSAGE_BTN_CUSTOM = -1 };

class Message
{

friend class vmNotify;

public:

	struct st_widgets
	{
		QWidget* widget;
		uint row;
		QSize size;
		Qt::Alignment alignment;
		bool isButton;
	};

	explicit Message ( vmNotify* parent = nullptr );
	~Message ();

	void addWidget ( QWidget* widget, const uint row,
					 const Qt::Alignment alignment = Qt::AlignLeft,
					 const bool is_button = false );

	inline void setMessageFinishedCallback ( const std::function<void ( Message* )>& func ) {
		messageFinishedFunc = func; }

	void inputFormKeyPressed ( const QKeyEvent* ke );

	int timeout; // ms
	QString title, bodyText, iconName;
	bool isModal, mbClosable, mbAutoRemove;
	PointersList<st_widgets*> widgets;
	vmNotify* m_parent;
	int mBtnID;
	QPixmap* icon;
	vmActionGroup* mGroup;
	QTimer* timer;

	std::function<void ( Message* )> messageFinishedFunc;
};

class vmNotify : public QDialog
{

friend class Message;

public:
	enum MESSAGE_BOX_ICON { QUESTION = 1, WARNING = 2, CRITICAL = 3 };

	vmNotify ( const QString& position = QString::null, const QWidget* parent = nullptr );
	virtual ~vmNotify ();

	// MAINWINDOW () will call this
	static void initNotify ();

	void notifyMessage ( const QString& title, const QString& msg, const int msecs = 3000, const bool b_critical = false );
	static void notifyMessage ( QWidget* referenceWidget, const QString& title, const QString& msg, const int msecs = 3000, const bool b_critical = false );

	void messageBox ( const QString& title, const QString& msg, const int msec = -1 );
	static void messageBox ( const QWidget* referenceWidget, const QString& title, const QString& msg, const int msec = -1 );
	static bool questionBox ( const QString& title, const QString& msg, const QWidget* referenceWidget = nullptr );
	static int criticalBox ( const QString& title, const QString& msg,
		const bool b_message_only = true, const QWidget* referenceWidget = nullptr );
	static int customBox ( const QString& title, const QString& msg, const MESSAGE_BOX_ICON icon,
			const QString& btnText1, const QString& btnText2 = QString::null, const QString& btnText3 = QString::null, const QWidget* referenceWidget = nullptr );

	static bool inputBox ( QString& result, const QWidget* referenceWidget, const QString& title, const QString& label_text,
					const QString& initial_text = QString::null, const QString& icon = QString::null,
					const QString& opt_check_box = QString::null, const vmCompleters::COMPLETER_CATEGORIES completer = vmCompleters::NONE );

	static bool passwordBox ( QString& result, const QWidget* referenceWidget, const QString& title,
					const QString& label_text, const QString& icon = QString::null );
	
	static vmNotify* progressBox ( vmNotify* box = nullptr, QWidget* parent = nullptr, const uint max_value = 10, uint next_value = 0,
								   const QString& title = QString::null, const QString& label = QString::null );

	int notifyBox ( const QString& title, const QString& msg,
					const MESSAGE_BOX_ICON icon, const QString btnsText[3], const int m_sec = -1 );
	
private:
	friend vmNotify* VM_NOTIFY ();
	friend void deleteNotifyInstance ();
	static vmNotify* s_instance;

	void buttonClicked ( QPushButton* btn, Message* const message );
	void setupWidgets ( Message* const message );
	void startMessageTimer ( Message* const message );
	void fadeWidget ();
	void showMenu ();
	void enterEventLoop ();
	void addMessage ( Message* message );
	void removeMessage ( Message* message );
	void setupWidgets ( const Message* message );
	void adjustSizeAndPosition ();
	QPoint displayPosition ( const QSize& widgetSize );

	bool mbDeleteWhenStackIsEmpty;
	vmTaskPanel* mPanel;
	QTimer *fadeTimer;
	QWidget* m_parent;
	QString mPos;
	QPointer<QEventLoop> mEventLoop;
	PointersList<Message*> messageStack;
};

inline vmNotify* VM_NOTIFY ()
{
	return vmNotify::s_instance;
}

#endif // VMNOTIFY_H
