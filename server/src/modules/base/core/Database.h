/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_MODULE_CORE_DATABASE_BASE_H
#define DATA_INSIGHT_CORE_MODULE_CORE_DATABASE_BASE_H

#include <vector>
#include <string>
#include <mutex>
#include "ServerLog.h"
#include "sqlite3.h"
#include "SqlitePreparedStatement.h"

namespace Dic {
namespace Module {
class DatabaseException : public std::exception {
public:
    explicit DatabaseException(const char* message): message(message){};
    DatabaseException(const char* message, bool isError): isError(isError), message(message){};
    const char* What()
    {
        return message;
    };
    void Log(std::string prefix)
    {
        if (isError) {
            Server::ServerLog::Error(prefix, message);
        } else {
            Server::ServerLog::Warn(prefix, message);
        }
    }
private:
    bool isError = true;
    const char* message;
};
class Database {
public:
    explicit Database(std::recursive_mutex  &sqlMutex) : mutex(sqlMutex) {};
    virtual ~Database();
    virtual bool CreateDbIfNotExist(const std::string &dbPath);
    virtual bool OpenDb(const std::string &dbPath, bool clearAllTable);
    virtual bool AttachDb(const std::string &dbPath);
    virtual bool IsOpen() const;
    void CloseDb();
    virtual bool StartTransaction();
    virtual bool RollbackTransaction();
    virtual bool EndTransaction();
    virtual std::string GetDbPath();
    virtual void SetDbPath(const std::string& dbPath);
    virtual bool GetTableList(std::vector<std::string> &tableList) const;
    virtual std::unique_ptr<SqlitePreparedStatement> CreatPreparedStatement(const std::string &sql);
    virtual std::unique_ptr<SqlitePreparedStatement> CreatPreparedStatement();
    bool ExecSql(const std::string &sql);
    bool DropSomeTables(const std::vector<std::string>& tableNames);
    bool DropAllTable();
    bool IsDatabaseVersionChange();
    bool QueryMetaVersion();
    std::string GetMetaVersion();
    bool SetDataBaseVersion();
    std::string QueryValueFromMetaDataByName(const std::string &name);

    bool CheckTableExist(const std::string& tableName);
    bool CheckTablesExist(const std::vector<std::string> &tablesName);
    bool CheckColumnExist(const std::string& tableName, const std::string& columnName);
    bool CheckStringInColumn(const std::string& tableName, const std::string& columnName,
                             const std::string& searchString);
    bool ExtendColumns(const std::string &tableName, const std::vector<std::string>& columns);
    bool CreateMetaDataTableForText();
    bool UpdateMetaDataTable(const std::string &name, const std::string &value);
    bool UpdateMetaDataTableWithNoPrimaryKey(const std::string &name, const std::string &value);

protected:
    bool CheckTableContainData(const std::string& tableName);
    virtual bool SetConfig();
    static std::string CheckSqlString(const std::string &src);
    static std::string sqlite3_column_string(sqlite3_stmt *stmt, int iCol);
    static std::string Sqlite3ColumnConvertStr(int colType, sqlite3_stmt *stmt, int iCol);
    std::string GetLastError();
    static std::string GetDataBaseVersion();
    sqlite3 *db = nullptr;
    std::recursive_mutex &mutex;
    bool isOpen = false;
    std::string path;
    const int bindStartIndex = 1;
    const int resultStartIndex = 0;
    const int timeoutMs = 50000;
    const std::string infoTable = "status_info";
    bool isLowCamel = false;
    std::string metaVersion;
    const std::string metaDataTable = "META_DATA";

    std::string GetValueFromMetaDataTable(const std::string& name);
    bool CreateStatusInfoTable(); // 创建表时未加锁，需要在调用处加锁
    std::string GetValueFromStatusInfoTable(const std::string& key);
    bool CheckValueFromStatusInfoTable(const std::string &key, const std::string &refValue);
    bool UpdateValueIntoStatusInfoTable(const std::string &key, const std::string &value);
    bool CheckAndResetDatabaseOnVersionChange();
    template <typename... Args> static inline std::unique_ptr<SqliteResultSet> ExecuteQuery(
            std::unique_ptr<SqlitePreparedStatement> &stmt, const std::string &sql, Args&&... args)
    {
        if (stmt == nullptr) {
            throw DatabaseException("Failed to prepare sql.");
        }
        if (!stmt->Prepare(sql)) {
            throw DatabaseException("Failed to prepare sql.");
        }
        stmt->Reset();
        stmt->BindParams(std::forward<Args>(args)...);
        auto result = stmt->ExecuteQuery();
        if (result == nullptr) {
            throw DatabaseException("Failed to ExecuteQuery.");
        }
        return result;
    };
};
} // end of namespace Module
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_MODULE_CORE_DATABASE_BASE_H
