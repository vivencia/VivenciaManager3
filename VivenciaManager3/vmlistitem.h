#ifndef LISTITEMS_H
#define LISTITEMS_H

#include "vmtableitem.h"
#include "client.h"
#include "job.h"
#include "payment.h"
#include "purchases.h"

enum SEARCH_STATUS { SS_NOT_SEARCHING = TRI_UNDEF, SS_SEARCH_FOUND = TRI_ON, SS_NOT_FOUND = TRI_UNDEF };

class vmListWidget;
class jobListItem;
class payListItem;
class buyListItem;
class crashRestore;
class QIcon;

static const uint client_nBadInputs ( 1 );
static const uint job_nBadInputs ( 2 );
static const uint pay_nBadInputs ( 6 );
static const uint buy_nBadInputs ( 4 );

/* Some switches mix dbRecord fields and infoTable fields, which might inccur in overlaps.
 * To avoid such compile errors, both callers and callees must add this offset whenever using INFO_TABLE_COLUMNS (Payment and Buy, so far)
 */
static const uint INFO_TABLE_COLUMNS_OFFSET ( 100 );

extern QIcon* listIndicatorIcons[4];
									  
enum RELATED_LIST_ITEMS {
	RLI_CLIENTPARENT = 0, RLI_JOBPARENT = 1, RLI_CLIENTITEM = 2, RLI_JOBITEM = 3, 
	RLI_CALENDARITEM = 4, RLI_EXTRAITEM_1 = 5, RLI_EXTRAITEM_2 = 6
};

enum CRASH_FIELDS {
	CF_SUBTYPE = 0, CF_CLIENTID, CF_JOBID, CF_ID, CF_ACTION, CF_DBRECORD
};

#define LAST_RELATION RLI_EXTRAITEM_2
#define LAST_EDITABLE_RELATION RLI_JOBITEM

class vmListItem : public vmTableItem
{

friend class vmListWidget;

public:
	explicit vmListItem ( const uint type_id, const uint nbadInputs = 0, bool* const badinputs_ptr = nullptr );
	vmListItem ( const QString& label );
	virtual ~vmListItem ();
	
	QString defaultStyleSheet () const;
	void highlight ( const VMColors vm_color, const QString& = QString::null );

	inline RECORD_ACTION action () const { return m_action; }
	void setRelation ( const RELATED_LIST_ITEMS relation  );
	inline RELATED_LIST_ITEMS relation () const { return mRelation; }
	void disconnectRelation ( const uint start_relation, vmListItem* item );
	void syncSiblingWithThis ( vmListItem* sibling );

	void addToList ( vmListWidget* const w_list, const bool b_makecall = true );
	inline vmListWidget* listWidget () const { return m_list; }

	inline DBRecord* dbRec () const { return m_dbrec; }
	void setDBRec ( DBRecord* dbrec, const bool self_only = false );

	inline int dbRecID () const { return id (); }
	inline void setDBRecID ( const int recid ) { setID ( recid ); }

	void saveCrashInfo ( crashRestore* crash );

	void setAction ( const RECORD_ACTION action, const bool bSetDBRec = false, const bool bSelfOnly = false );
	virtual void createDBRecord ();
	virtual bool loadData ();
	virtual void update ();
	virtual void relationActions ( vmListItem* = nullptr ) { ; }
	void setRelatedItem ( const RELATED_LIST_ITEMS rel_idx, vmListItem* const item );
	vmListItem* relatedItem ( const RELATED_LIST_ITEMS rel_idx ) const;
	
	virtual uint translatedInputFieldIntoBadInputField ( const uint field ) const;

	inline int crashID () const { return m_crashid; }
	inline bool isGoodToSave () const { return n_badInputs <= 0; }
	inline bool fieldInputStatus ( const uint field ) const { return badInputs_ptr[translatedInputFieldIntoBadInputField( field )]; }

	void setFieldInputStatus ( const uint field, const bool ok, const vmWidget* widget );

	// Will not check field boundary; trust caller
	inline void setSearchFieldStatus ( const uint field, const SEARCH_STATUS ss ) {
			if ( searchFields ) searchFields[field] = ss; }
	inline SEARCH_STATUS searchFieldStatus ( const uint field ) const {
			return searchFields ? static_cast<SEARCH_STATUS> ( searchFields[field].state () ) : SS_NOT_SEARCHING; }

	void setSearchArray ();

protected:
	void changeAppearance ();
	void deleteRelatedItem ( const RELATED_LIST_ITEMS rel_idx );

	int m_crashid;
	DBRecord* m_dbrec;
	RELATED_LIST_ITEMS mRelation;
	triStateType* searchFields;

private:
	vmListItem ( const vmListItem& );
	vmListItem ( const QListWidgetItem& );
	vmListItem& operator=( const vmListItem& );
	vmListItem& operator=( const QListWidgetItem& );

	vmListItem* item_related[LAST_RELATION+1];
	
	RECORD_ACTION m_action;
	vmListWidget* m_list;

	bool* badInputs_ptr;
	int n_badInputs;
	uint mTotal_badInputs;
	bool mbSearchCreated, mbInit;
};

class clientListItem : public vmListItem
{

public:
	explicit inline clientListItem ()
		: vmListItem ( CLIENT_TABLE, client_nBadInputs, badInputs ) {}
	virtual ~clientListItem ();

	// Prevent Qt from deleting these objects
	inline void operator delete ( void* )  { return; }
	
	inline Client* clientRecord () const { return static_cast<Client*> ( dbRec () ); }

	void update ();
	void createDBRecord ();
	bool loadData ();
	void relationActions ( vmListItem* subordinateItem = nullptr );

	PointersList<jobListItem*>* jobs;
	PointersList<payListItem*>* pays;
	PointersList<buyListItem*>* buys;

private:
	bool badInputs[client_nBadInputs];
};

class jobListItem : public vmListItem
{
public:
	explicit inline jobListItem ()
		: vmListItem ( JOB_TABLE, job_nBadInputs, badInputs ),
		  mSearchSubFields ( nullptr ), m_payitem ( nullptr ), m_newproject_opt ( INT_MIN ) {}

	virtual ~jobListItem ();

	// Prevent Qt from deleting these objects
	inline void operator delete ( void* )  { return; }
	
	inline Job* jobRecord () const { return static_cast<Job*> ( dbRec () ); }

	uint translatedInputFieldIntoBadInputField ( const uint field ) const;

	void createDBRecord ();
	bool loadData ();
	void update ();
	void relationActions ( vmListItem* subordinateItem = nullptr );

	inline void setPayItem ( payListItem* const pay ) { m_payitem = pay; }
	inline payListItem* payItem () const { return m_payitem; }

	inline void setNewProjectOpt ( const int opt ) { m_newproject_opt = opt; }
	inline int newProjectOpt () const { return m_newproject_opt; }

	podList<uint>* searchSubFields () const;
	void setReportSearchFieldFound ( const uint report_field, const uint day );

	PointersList<buyListItem*>* buys;
	PointersList<vmListItem*>* daysList;

private:
	podList<uint>* mSearchSubFields;
	payListItem* m_payitem;
	int m_newproject_opt;

	bool badInputs[job_nBadInputs];
};

class payListItem : public vmListItem
{

public:
	explicit inline payListItem ()
		: vmListItem ( PAYMENT_TABLE, pay_nBadInputs, badInputs ) {}
	virtual ~payListItem ();

	// Prevent Qt from deleting these objects
	inline void operator delete ( void* )  { return; }
		
	inline Payment* payRecord () const { return static_cast<Payment*> ( dbRec () ); }

	uint translatedInputFieldIntoBadInputField ( const uint field ) const;

	void createDBRecord ();
	bool loadData ();
	void update ();
	void relationActions ( vmListItem* subordinateItem = nullptr );

private:
	bool badInputs[pay_nBadInputs];
};

class buyListItem : public vmListItem
{

public:
	explicit inline buyListItem ()
		: vmListItem ( PURCHASE_TABLE, buy_nBadInputs, badInputs ) {}
	virtual ~buyListItem ();

	// Prevent Qt from deleting these objects
	inline void operator delete ( void* )  { return; }
		
	inline Buy* buyRecord () const { return static_cast<Buy*> ( dbRec () ); }

	void createDBRecord ();
	bool loadData ();
	void update ();
	void relationActions ( vmListItem* subordinateItem = nullptr );

	uint translatedInputFieldIntoBadInputField ( const uint field ) const;

private:
	bool badInputs[buy_nBadInputs];
};
#endif // LISTITEMS_H
