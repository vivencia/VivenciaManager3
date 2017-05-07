#ifndef DBSUPPLIESUI_H
#define DBSUPPLIESUI_H

#include <QObject>

class vmTableWidget;
class vmTableItem;
class dbSupplies;
class vmListItem;

class QPushButton;
class QVBoxLayout;

class dbSuppliesUI : public QObject
{

friend class VivenciaDB;
friend class MainWindow;

public:
	virtual ~dbSuppliesUI ();

	static void init () {
		if ( !s_instance )
			s_instance = new dbSuppliesUI;
	}

	bool setupUI ();
	void createTable ();

	inline vmTableWidget* table () const {
		return m_table;
	}
	inline QVBoxLayout* layout () const {
		return suppliesLayout;
	}
	void insertRow ( const int i_row );
	void showSearchResult ( vmListItem* item, const bool bshow );

private:
	explicit dbSuppliesUI ();
	friend dbSuppliesUI* SUPPLIES ();
	friend void deleteDBSuppliesInstance ();
	static dbSuppliesUI* s_instance;

	vmTableWidget* m_table;
	dbSupplies* supplies_rec;

	QVBoxLayout* suppliesLayout;
	QPushButton* btnSuppliesEditTable;
	QPushButton* btnSuppliesCancelEdit;
	QPushButton* btnSuppliesInsertRowAbove;
	QPushButton* btnSuppliesInsertRowBelow;
	QPushButton* btnSuppliesRemoveRow;

	void readRowData ( const uint row, const uint col, const uint prev_row, const uint = 0 );
	void tableChanged ( const vmTableItem* const item );
	void rowRemoved ( const int row = -1 );
	void btnSuppliesCancelEdit_clicked ();
	void btnSuppliesEditTable_clicked ( const bool checked );
};

inline dbSuppliesUI *SUPPLIES ()
{
	return dbSuppliesUI::s_instance;
}

#endif // DBSUPPLIESUI_H
