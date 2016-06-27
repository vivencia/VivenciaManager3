#ifndef TODOLIST_H
#define TODOLIST_H

#include <QWidget>

class dataFile;
class vmTableWidget;
class textEditWithCompleter;
class stringRecord;
class vmNumber;

class todoList : public QObject
{

public:
	explicit todoList ( QWidget* parent );
	virtual ~todoList ();

	void displayList ( const vmNumber& );
	bool saveList ();

private:
	void cellActivated ( const int current_row, const int current_col, const int prev_row, int prev_col );
	void showMessage ( const bool );
	void playSound ( const bool );
	void execApp ( const bool );
	bool dateInRange ( const vmNumber& date, const stringRecord& str_appointment ) const;
	void recordNotification ( const uint row );
	void recordDescription ( const uint row );

	QString mListFileName;
	QWidget* mParent;
	vmTableWidget* mLstToDo;
	dataFile* mTdbList;
	textEditWithCompleter* mTxtInfo;
};

#endif // TODOLIST_H
