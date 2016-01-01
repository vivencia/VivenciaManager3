#ifndef QUICKPROJECTUI_H
#define QUICKPROJECTUI_H

#include "quickproject.h"
#include "vmtablewidget.h"

#include <QDialog>

class Job;
class QPushButton;

class quickProjectUI : public QDialog
{

public:

    enum QP_GUI_FIELDS {
        QP_GUI_ITEM = 0,
		QP_GUI_SELL_QUANTITY = 1,
		QP_GUI_SELL_UNIT_PRICE = 2,
		QP_GUI_SELL_TOTAL_PRICE = 3,
		QP_GUI_PURCHASE_QUANTITY = 4,
		QP_GUI_PURCHASE_UNIT_PRICE = 5,
		QP_GUI_PURCHASE_TOTAL_PRICE = 6,
        QP_GUI_RESULT = 7
    };

	virtual ~quickProjectUI ();

    void setupUI ();
    void prepareToShow ( const Job* const job = nullptr );
	bool loadData ( const QString& jobid, const bool force = false );
    QString getJobIDFromQPString ( const QString& qpIDstr ) const;
    void selectDone ();

    inline void setCallbackForDialogClosed ( std::function<void ()> func ) { funcClosed = func; }
	inline QString jobID () const { return recStrValue ( qp_rec, FLD_QP_JOB_ID ); }
	inline vmTableWidget* table () const { return m_table; }
    inline QString totalPrice () const {
            return m_table->sheetItem ( m_table->totalsRow(), FLD_QP_SELL_TOTALPRICE - 2 )->text (); }
	inline QString qpString () const { return m_qpString; }

    void getHeadersText ( spreadRow* row ) const;
    uint copyItemsFromTable ( const vmTableWidget* const table );

	inline bool qpChanged () const { return mbQPChanged; }
	//void temporarilyHide ();
	//void showAgain ();

private:
    quickProjectUI ();

    void cellModified ( const vmTableItem* const item );
    void completeItem ( const QModelIndex& );
    void editTable ( const bool checked );
	void cancelEdit ();
    void btnCopyPurchasesLists_clicked ();
    void closeClicked ();
    void rowRemoved ( const uint row );

    friend quickProjectUI* QUICK_PROJECT ();
    friend void deleteQPInstance ();
    static quickProjectUI* s_instance;

    void enableControls ( const bool enable );

    QVBoxLayout* mLayoutMain;
    vmTableWidget* m_table;
    quickProject* qp_rec;
    Job* mJob;
    QString m_qpString;
	bool mbQPChanged;

    QPushButton* btnEditTable;
	QPushButton* btnCancel;
    QPushButton* btnCopyPurchasesLists;
    QPushButton* btnClose;

	std::function<void ()> funcClosed;
};

inline quickProjectUI* QUICK_PROJECT ()
{
    if ( !quickProjectUI::s_instance )
        quickProjectUI::s_instance = new quickProjectUI;
    return quickProjectUI::s_instance;
}

#endif // QUICKPROJECTUI_H
