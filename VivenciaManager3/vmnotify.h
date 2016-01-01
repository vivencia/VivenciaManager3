#ifndef VMNOTIFY_H
#define VMNOTIFY_H

#include "vmlist.h"
#include "completers.h"
#include "vmtaskpanel.h"
#include "vmactiongroup.h"

#include <QSize>
#include <QPointer>
#include <QEventLoop>

#include <functional>

class vmNotify;

class QTimer;
class QSystemTrayIcon;
class QPixmap;
class QPushButton;
class QVBoxLayout;
class QTimerEvent;

class Message : public QObject
{

friend class vmNotify;

public:
	enum MESSAGE_BTNCLICKED { NO_BUTTON = -1, BUTTON_1 = 0, BUTTON_2 = 1, BUTTON_3 = 2, BUTTON_4 = 3, BUTTON_5 = 4 };

	struct st_widgets
	{
		QWidget* widget;
		uint row;
		QSize size;
		Qt::Alignment alignment;
		bool isButton;
	};

	Message ( vmNotify* parent = nullptr );
	~Message ();

	void showMessage ();
	void addWidget ( QWidget* widget, const uint row,
					 const Qt::Alignment alignment = Qt::AlignLeft,
					 const bool is_button = false );
	void setupWidgets ();
    void slotActionButtonClicked ( QWidget* widget = nullptr );

	inline void setMessageFinishedCallback ( std::function<void ( Message* )> func ) {
		messageFinishedFunc = func; }

	void inputFormKeyPressed ( const QKeyEvent* ke );

	int timeout; // ms
	QString title, bodyText, iconName;
    bool isModal, mbClosable, mbAutoRemove;
	PointersList<st_widgets*> widgets;
	vmNotify* m_parent;
	MESSAGE_BTNCLICKED mBtnID;
	QPixmap* icon;
	vmActionGroup* mGroup;
	QTimer* timer;

	std::function<void ( Message* )> messageFinishedFunc;
};

class vmNotify : public vmTaskPanel
{

friend class Message;

public:
	enum MESSAGE_BOX_ICON { QUESTION = 1, WARNING = 2, CRITICAL = 3 };

	vmNotify ( const QString& position = QString::null, const QWidget* parent = nullptr );
	virtual ~vmNotify ();

	// globalMainWindow will call this
	static void initNotify ();

	void notifyMessage ( const QString& title, const QString& msg, const int msecs = 3000, const bool b_critical = false );
	static void notifyMessage ( const QWidget* referenceWidget, const QString& title, const QString& msg, const int msecs = 3000, const bool b_critical = false );

	void messageBox ( const QString& title, const QString& msg );
	static void messageBox ( const QString& title, const QString& msg, const QWidget* referenceWidget );
	static bool questionBox ( const QString& title, const QString& msg, const QWidget* referenceWidget = nullptr );
	static Message::MESSAGE_BTNCLICKED criticalBox ( const QString& title, const QString& msg,
		const bool b_message_only = true, const QWidget* referenceWidget = nullptr );
	static Message::MESSAGE_BTNCLICKED customBox ( const QString& title, const QString& msg, const MESSAGE_BOX_ICON icon,
			const QString& btnText1, const QString& btnText2 = QString::null, const QString& btnText3 = QString::null, const QWidget* referenceWidget = nullptr );

	static bool inputBox ( QString& result, const QWidget* referenceWidget, const QString& title, const QString& label_text,
					const QString& initial_text = QString::null, const QString& icon = QString::null,
					const QString& opt_check_box = QString::null, const vmCompleters::COMPLETER_CATEGORIES completer = vmCompleters::NONE,
					const bool bPasswordBox = false );

	static vmNotify* progressBox ( vmNotify* box = nullptr, QWidget* parent = nullptr, const uint max_value = 10, uint next_value = 0,
								   const QString& title = QString::null, const QString& label = QString::null );

private:
	friend vmNotify* VM_NOTIFY ();
	friend void deleteNotifyInstance ();
	static vmNotify* s_instance;

	void fadeWidget ();
	void showMenu ();
	void addMessage ( Message* message );
	void removeMessage ( Message* message );
	void setupWidgets ( const Message* message );
	void adjustSizeAndPosition ();
	QPoint displayPosition ( const QSize& widgetSize );

	Message::MESSAGE_BTNCLICKED notifyBox ( const QString& title, const QString& msg,
					const MESSAGE_BOX_ICON icon, const QString btnsText[3], int btnsValue[3], const int m_sec = -1 );

	QString mPos;
	QTimer *fadeTimer;
	QWidget* m_parent;
    QPointer<QEventLoop> mEventLoop;
	PointersList<Message*> messageStack;
};

inline vmNotify* VM_NOTIFY ()
{
	return vmNotify::s_instance;
}

#endif // VMNOTIFY_H
