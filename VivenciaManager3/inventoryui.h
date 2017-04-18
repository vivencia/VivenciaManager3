#ifndef INVENTORYITEMS_H
#define INVENTORYITEMS_H

#include <QObject>

class Inventory;
class vmTableWidget;
class vmTableItem;
struct spreadRow;

class QPushButton;
class QVBoxLayout;

class InventoryUI : public QObject
{

friend class VivenciaDB;
friend class companyPurchasesUI;

public:
	virtual ~InventoryUI ();

	static void init () {
		if ( !s_instance )
			s_instance = new InventoryUI;
	}

	bool setupUI ();

	inline vmTableWidget* table () const {
		const_cast<InventoryUI*> ( this )->createTable ();
		return m_table;
	}

	inline QVBoxLayout* layout () const { return inventoryLayout; }

	void insertRow ( const uint );

private:
	explicit InventoryUI ();
	static InventoryUI* s_instance;
	friend void deleteInventoryInstance ();
	friend InventoryUI* INVENTORY ();

	void createTable ();

	Inventory* inventory_rec;
	vmTableWidget* m_table;

	QVBoxLayout* inventoryLayout;
	QPushButton* btnInventoryEditTable;
	QPushButton* btnInventoryCancelEdit;
	QPushButton* btnInventoryInsertRowAbove;
	QPushButton* btnInventoryInsertRowBelow;
	QPushButton* btnInventoryRemoveRow;

	void readRowData ( const uint row, const uint col, const uint prev_row, const uint = 0 );
	void tableChanged ( const vmTableItem* const item );
	void rowRemoved ( const uint row = 0 );
	void btnInventoryInsertRowAbove_clicked ();
	void btnInventoryInsertRowBelow_clicked ();
	void btnInventoryRemoveRow_clicked ();
	void btnInventoryCancelEdit_clicked ();
	void btnInventoryEditTable_clicked ( const bool checked );
};

inline InventoryUI* INVENTORY ()
{
	return InventoryUI::s_instance;
}

#endif // INVENTORYITEMS_H
