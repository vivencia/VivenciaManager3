#ifndef DBTABLEVIEW_H
#define DBTABLEVIEW_H

#include "vmlist.h"
#include "vmnumberformats.h"

#include <QObject>
#include <QFrame>

class vmListWidget;
class vmTableWidget;
class vmTableItem;
class vmLineEdit;
class tableViewWidget;
		
class QTabWidget;
class QSplitter;
class QVBoxLayout;
class QToolButton;
class QSqlQuery;

class dbTableView : public QObject
{

public:
	virtual ~dbTableView ();
	
	void reload ();
	inline QVBoxLayout* layout () const { return mMainLayout; }
	
private:
	explicit dbTableView ();
	friend dbTableView* DB_TABLE_VIEW ();
	friend void deleteDBTableViewInstance ();
	static dbTableView* s_instance;

	void loadTablesIntoList ();
	void showTable ( const QString& tablename );
	void runPersonalQuery ();
	void maybeRunQuery ( const QKeyEvent* const ke );
	
	vmListWidget* mTablesList;
	QFrame* mLeftFrame;
	QTabWidget* mTabView;
	vmLineEdit* mTxtQuery;
	QToolButton* mBtnRunQuery;
	QSplitter* mMainLayoutSplitter;
	QVBoxLayout* mMainLayout;
};

inline dbTableView* DB_TABLE_VIEW ()
{
	if ( dbTableView::s_instance == nullptr )
		dbTableView::s_instance = new dbTableView ();
	return dbTableView::s_instance;
}

class tableViewWidget : public QFrame
{

public:
	explicit tableViewWidget ( const QString& tablename );
	tableViewWidget ( QSqlQuery* const query );
	virtual ~tableViewWidget ();
	
	void showTable ();
	void load ();
	void reload ();

	inline const QString& tableName () const { return m_tablename; }
	
private:
	void focusInEvent ( QFocusEvent* e );
	void createTable ();
	void getTableInfo ();
	void getTableLastUpdate ( vmNumber& date , vmNumber& time );
	void updateTable ( const vmTableItem* const item );
	void rowInserted ( const uint row );
	bool rowRemoved ( const uint row );
	void tryToFindTableNameFromQuery ( const QString& querycmd );
	
	vmTableWidget* m_table;
	VMList<QString> m_cols;
	QString m_tablename;
	QVBoxLayout* mLayout;
	vmNumber m_updatedate, m_updatetime;
	QSqlQuery* mQuery;
	uint m_nrows;
	bool mb_loaded;
};

#endif // DBTABLEVIEW_H