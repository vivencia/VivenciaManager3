#ifndef VIVENCIADB_H
#define VIVENCIADB_H

#include "global.h"
#include "global_db.h"
#include "dbrecord.h"
#include "vmlist.h"

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

class vmTableWidget;
class passwordManager;
class BackupDialog;

class VivenciaDB
{

friend class Data;
friend class DBRecord;
friend class UserManagement;
friend class tableView;
friend class fixDatabase;
friend class BackupDialog;

public:

	//-----------------------------------------STATIC---------------------------------------------
	static QString getTableColumnName ( const TABLE_INFO* t_info, uint column );
	static QString getTableColumnFlags ( const TABLE_INFO* t_info, uint column );
	static QString getTableColumnLabel ( const TABLE_INFO* t_info, uint column );
	static QString tableName ( const TABLE_ORDER table );
	//-----------------------------------------STATIC---------------------------------------------

	~VivenciaDB ();

	static void init () {
		if ( !s_instance )
			s_instance = new VivenciaDB;
	}

	static inline const TABLE_INFO* tableInfo ( const int ti ) { //ti = table order
		return table_info[ti];
	}

	inline QSqlDatabase* database () {
		return &m_db;
	}

	bool deleteDB ( const QString& dbname = QString::null );
	bool optimizeTable ( const TABLE_INFO* t_info ) const;

    inline bool db_oK ()  const { return m_ok; }
    inline bool backUpSynced () const { return mBackupSynced; }

	static inline QString backupApp () {
		return QStringLiteral ( "mysqldump" );
	}
	static inline QString importApp () {
		return STR_MYSQL;
	}
	static inline QString restoreApp () {
		return QStringLiteral ( "mysqlimport" );
	}

	bool databaseIsEmpty () const;
	bool createAllTables ();
	bool createTable ( const TABLE_INFO* t_info );
	bool deleteTable ( const TABLE_INFO* t_info );
	bool clearTable ( const TABLE_INFO* t_info );
	bool tableExists ( const TABLE_INFO* t_info );
	bool insertColumn ( const uint column, const TABLE_INFO* t_info );
	bool removeColumn ( const QString& column_name, const TABLE_INFO* t_info );
	bool renameColumn ( const QString& old_column_name, const uint col_idx, const TABLE_INFO* t_info );
	bool changeColumnProperties ( const uint column, const TABLE_INFO* t_info );
	uint recordCount ( const TABLE_INFO* t_info ) const;
	void populateTable ( const DBRecord* db_rec, vmTableWidget* table ) const;

	//-----------------------------------------COMMOM-DBRECORD-------------------------------------------
	bool insertRecord ( const DBRecord* db_rec ) const;
	bool updateRecord ( const DBRecord* db_rec ) const;
	bool removeRecord ( const DBRecord* db_rec ) const;
	void loadDBRecord ( DBRecord* db_rec, const QSqlQuery* const query );
	int lastDBRecord ( const uint table );
	int firstDBRecord ( const uint table );
	bool insertDBRecord ( DBRecord* db_rec );
	bool getDBRecord ( DBRecord* db_rec, const uint field = 0, const bool load_data = true );
	bool getDBRecord ( DBRecord* db_rec, DBRecord::st_Query& stquery, const bool load_data = true );
	uint getHighLowID ( const uint table, const bool high = true ) const;
	/* Used on table updates. If one table update makes use of information from another table by calling lastDBRecord
	 * or getHighLowID to find the last record, highest_id will be set with current information. If later, this second table
	 * must uptade its contents, might be that the last id must be reset for accurate results.
	 */
	inline void setHighestID ( const uint table, const int new_id = 0 ) {
		if ( table < TABLES_IN_DB ) highest_id[table] = new_id; }

	inline uint getNextID ( const uint table ) const { return getHighLowID ( table ) + 1; }
	
	bool recordExists ( const QString& table_name, const int id ) const;

	// Call this when there is need for using the result of the query. Use VDB ()->database ()->exec () when
	// then goal is to command the driver to do something
	bool runQuery ( const QString& query_cmd, QSqlQuery& queryRes ) const;
	//-----------------------------------------COMMOM-DBRECORD-------------------------------------------

	//-----------------------------------------IMPORT-EXPORT--------------------------------------------
	bool checkDumpFile ( const QString& dumpfile );
	bool doBackup ( const QString& filepath, const QString& tables, BackupDialog* bDlg );
	bool doRestore ( const QString& filepath, BackupDialog* bDlg );
	bool importFromCSV (const QString& filename, BackupDialog* bDlg );
	bool exportToCSV (const uint table, const QString& filename, BackupDialog* bDlg );
	
	passwordManager* rootPwdManager ();
	//-----------------------------------------IMPORT-EXPORT--------------------------------------------

private:

	explicit VivenciaDB ();
	//-----------------------------------------READ-OPEN-LOAD-------------------------------------------
	bool openDataBase ();
	void doPreliminaryWork ();
	//-----------------------------------------READ-OPEN-LOAD-------------------------------------------

	//-----------------------------------------CREATE-DESTROY-MODIFY------------------------------------
	bool createDatabase ();
	bool createUser ();

	bool lockTable ( const TABLE_INFO* t_info ) const;
	bool unlockTable ( const TABLE_INFO* t_info ) const;
	bool unlockAllTables () const;
	bool flushAllTables () const;
	//-----------------------------------------CREATE-DESTROY-MODIFY------------------------------------

	//----------------------------------------VARIABLE-MEMBERS--------------------------------------
	QSqlDatabase m_db;

	mutable podList<uint> lowest_id;
	mutable podList<uint> highest_id;

	static const TABLE_INFO* table_info[TABLES_IN_DB];

    bool m_ok, mNewDB;
    mutable bool mBackupSynced;
	passwordManager* mRootPasswdMngr;
	//----------------------------------------VARIABLE-MEMBERS--------------------------------------

	static VivenciaDB* s_instance;
	friend VivenciaDB* VDB ();
	friend void deleteVivenciaDBInstance ();
};

inline VivenciaDB* VDB ()
{
	return VivenciaDB::s_instance;
}

#endif // VIVENCIADB_H
