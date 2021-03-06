#ifndef VIVENCIADB_H
#define VIVENCIADB_H

#include "global.h"
#include "global_db.h"
#include "dbrecord.h"
#include "vmlist.h"

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

#include <functional>

class vmTableWidget;
class passwordManager;
class BackupDialog;

class VivenciaDB
{

friend class DBRecord;
friend class UserManagement;
friend class tableView;
friend class fixDatabase;
friend class BackupDialog;

public:
	~VivenciaDB ();

	//-----------------------------------------STATIC---------------------------------------------
	static QString getTableColumnName ( const TABLE_INFO* t_info, uint column );
	static QString getTableColumnFlags ( const TABLE_INFO* t_info, uint column );
	static QString getTableColumnLabel ( const TABLE_INFO* t_info, uint column );
	static QString tableName ( const TABLE_ORDER table );

	static inline void init ()
	{
		if ( !s_instance )
			s_instance = new VivenciaDB;
	}

	static inline const TABLE_INFO* tableInfo ( const uint to ) //to = table order
	{
		return table_info[to];
	}
	
	static inline QString backupApp () { return QStringLiteral ( "mysqldump" ); }
	static inline QString importApp () { return STR_MYSQL; }
	static inline QString restoreApp () { return QStringLiteral ( "mysqlimport" ); }
	//-----------------------------------------STATIC---------------------------------------------
	
	inline QSqlDatabase* database () const
	{
		return const_cast<VivenciaDB*>( this )->m_db;
	}

	bool openDataBase ();
	void doPreliminaryWork ();
	bool deleteDB ( const QString& dbname = QString::null );
	bool optimizeTable ( const TABLE_INFO* t_info ) const;

	inline bool db_oK ()  const { return m_ok; }
	inline bool backUpSynced () const { return mBackupSynced; }

	bool databaseIsEmpty () const;
	bool createAllTables ();
	bool createTable ( const TABLE_INFO* t_info );
	bool deleteTable ( const QString& table_name );
	inline bool deleteTable ( const TABLE_INFO* t_info ) { return deleteTable ( t_info->table_name ); }
	bool clearTable ( const QString& table_name );
	inline bool clearTable ( const TABLE_INFO* t_info ) { return clearTable ( t_info->table_name ); }
	bool tableExists ( const TABLE_INFO* t_info );
	bool insertColumn ( const uint column, const TABLE_INFO* t_info );
	bool removeColumn ( const QString& column_name, const TABLE_INFO* t_info );
	bool renameColumn ( const QString& old_column_name, const uint col_idx, const TABLE_INFO* t_info );
	bool changeColumnProperties ( const uint column, const TABLE_INFO* t_info );
	uint recordCount ( const TABLE_INFO* t_info ) const;
	void populateTable ( const TABLE_INFO* t_info, vmTableWidget* table ) const;
	void updateTable ( const TABLE_INFO* t_info, vmTableWidget* table, const bool b_only_new ) const;
	void updateTable ( const TABLE_INFO* t_info, vmTableWidget* table, const uint id ) const;
	void updateTable ( const TABLE_INFO* t_info, vmTableWidget* table, const podList<uint>& ids ) const;

	//-----------------------------------------COMMOM-DBRECORD-------------------------------------------
	bool insertRecord ( const DBRecord* db_rec ) const;
	bool updateRecord ( const DBRecord* db_rec ) const;
	bool removeRecord ( const DBRecord* db_rec ) const;
	void loadDBRecord ( DBRecord* db_rec, const QSqlQuery* const query );
	bool insertDBRecord ( DBRecord* db_rec );
	bool getDBRecord ( DBRecord* db_rec, const uint field = 0, const bool load_data = true );
	bool getDBRecord ( DBRecord* db_rec, DBRecord::st_Query& stquery, const bool load_data = true );
	
	uint getHighestID ( const uint table ) const;
	inline void setHighestID ( const uint table, const uint new_id = 1 ) {
		if ( table < TABLES_IN_DB ) highest_id[table] = new_id; }
	uint getLowestID ( const uint table ) const;
	inline void setLowestID ( const uint table, const uint new_id = 1 ) {
		if ( table < TABLES_IN_DB ) lowest_id[table] = new_id; }

	uint getNextID ( const uint table );
	
	bool recordExists ( const QString& table_name, const int id ) const;

	bool runSelectLikeQuery ( const QString& query_cmd, QSqlQuery& queryRes ) const;
	bool runModifyingQuery ( const QString& query_cmd, QSqlQuery& queryRes ) const;
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
	bool createDatabase ();
	bool createUser ();

	bool lockTable ( const TABLE_INFO* t_info ) const;
	bool unlockTable ( const TABLE_INFO* t_info ) const;
	bool unlockAllTables () const;
	bool flushAllTables () const;
	//-----------------------------------------CREATE-DESTROY-MODIFY------------------------------------

	//----------------------------------------VARIABLE-MEMBERS--------------------------------------
	QSqlDatabase* m_db;

	podList<uint> lowest_id;
	podList<uint> highest_id;

	static const TABLE_INFO* table_info[TABLES_IN_DB];

	bool m_ok, mNewDB;
	mutable bool mBackupSynced;
	//----------------------------------------VARIABLE-MEMBERS--------------------------------------

	static VivenciaDB* s_instance;
	friend VivenciaDB* VDB ();
	friend void deleteVivenciaDBInstance ();
};

inline VivenciaDB* VDB ()
{
	return VivenciaDB::s_instance;
}

#undef USE_THREADS
#include <QObject>

#ifdef USE_THREADS
class threadedDBOps : public QObject
{
Q_OBJECT

public:
	virtual ~threadedDBOps ();
	
#else
class threadedDBOps
{

public:
	~threadedDBOps ();
#endif
	
	explicit threadedDBOps ();
	void populateTable ( const TABLE_INFO* t_info, vmTableWidget* table );
	void updateTable ( const TABLE_INFO* t_info, vmTableWidget* table, const QString& cmd, uint startrow = 0 );
	void updateTable ( const TABLE_INFO* t_info, vmTableWidget* table, const bool b_only_new );
	void updateTable ( const TABLE_INFO* t_info, vmTableWidget* table, const uint id );
	void updateTable ( const TABLE_INFO* t_info, vmTableWidget* table, const podList<uint>& ids );
	inline void setCallbackForFinished ( const std::function<void ()>& func ) { m_finishedFunc = func; }
	
protected:
	std::function<void ()> m_finishedFunc;
};
#endif // VIVENCIADB_H
