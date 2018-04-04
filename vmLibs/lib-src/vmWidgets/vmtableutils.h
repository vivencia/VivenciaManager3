#ifndef VMTABLEUTILS_H
#define VMTABLEUTILS_H

#include "vmlist.h"
#include <vmUtils/tristatetype.h>

#include <QtWidgets/QFrame>

class vmLineFilter;
class vmLineEdit;
class vmCheckBox;
class vmTableWidget;
class vmTableItem;

class QToolButton;

class vmTableSearchPanel : public QFrame
{

public:
	inline virtual ~vmTableSearchPanel () {}
	vmTableSearchPanel ( const vmTableWidget* const table );
	inline int utilityIndex () const { return m_utilidx; }
	inline void setUtilityIndex ( const int idx ) { m_utilidx = idx; }
	
protected:
	void showEvent ( QShowEvent* se );
	void hideEvent ( QHideEvent* he );

private:
	void searchFieldsChanged ( const vmCheckBox* const = nullptr );
	bool searchStart ();
	bool searchNext ();
	bool searchPrev ();
	void txtSearchTerm_keyPressed ( const QKeyEvent* ke );
	
	QString m_SearchedWord;
	int m_utilidx;
	vmTableWidget* m_table;
	vmCheckBox* chkSearchAllTable;
	vmLineEdit* txtSearchTerm;
	QToolButton* btnSearchStart;
	QToolButton* btnSearchNext;
	QToolButton* btnSearchPrev;
	QToolButton* btnSearchCancel;
};

class vmTableFilterPanel : public QFrame
{

public:
	explicit vmTableFilterPanel ( const vmTableWidget* const table );
	inline virtual ~vmTableFilterPanel () {}
	inline int utilityIndex () const { return m_utilidx; }
	inline void setUtilityIndex ( const int idx ) { m_utilidx = idx; }
	
protected:
	void showEvent ( QShowEvent* se );
	void hideEvent ( QHideEvent* he );
	
private:
	void doFilter ( const triStateType level, const int startlevel = 0 );
	
	int m_utilidx;
	vmTableWidget* m_table;
	vmLineFilter* m_txtFilter;
	QToolButton* m_btnClose;
	PointersList<podList<int>*> searchLevels;
};

#endif // VMTABLEUTILS_H
