/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

#ifndef DATA_INSIGHT_CORE_MODULE_CORE_DATABASE_BASE_H
#define DATA_INSIGHT_CORE_MODULE_CORE_DATABASE_BASE_H

#include <vector>
#include <string>
#include <mutex>
#include "sqlite3.h"
#include "SqlitePreparedStatement.h"

namespace Dic {
namespace Module {
class DatabaseException : public std::exception {
public:
    explicit DatabaseException(const char* message): message(message){};
    const char* What()
    {
        return message;
    };
private:
    const char* message;
};
class Database {
public:
    Database() = default;
    virtual ~Database();
    virtual bool OpenDb(const std::string &dbPath, bool clearAllTable);
    virtual bool AttachDb(const std::string &dbPath);
    virtual bool IsOpen() const;
    void CloseDb();
    virtual bool StartTransaction();
    virtual bool EndTransaction();
    virtual std::string GetDbPath();
    virtual void SetDbPath(const std::string& dbPath);
    virtual bool GetTableList(std::vector<std::string> &tableList) const;
    virtual std::unique_ptr<SqlitePreparedStatement> CreatPreparedStatement(const std::string &sql);
    virtual std::unique_ptr<SqlitePreparedStatement> CreatPreparedStatement();
    bool DropSomeTables(const std::vector<std::string>& tableNames);
    bool DropAllTable();
    bool IsDatabaseVersionChange();

protected:
    bool ExecSql(const std::string &sql);
    bool CheckTableExist(const std::string& tableName);
    static std::string CheckSqlString(const std::string &src);
    static std::string sqlite3_column_string(sqlite3_stmt *stmt, int iCol);
    std::string GetLastError();
    static std::string GetDataBaseVersion();
    sqlite3 *db = nullptr;
    bool isOpen = false;
    std::string path;
    const int bindStartIndex = 1;
    const int resultStartIndex = 0;
    const int timeoutMs = 50000;
    const std::string infoTable = "status_info";

    bool CreateStatusInfoTable(); // 创建表时未加锁，需要在调用处加锁
    std::string GetValueFromStatusInfoTable(const std::string& key);
    bool CheckValueFromStatusInfoTable(const std::string &key, const std::string &refValue);
    bool UpdateValueIntoStatusInfoTable(const std::string &key, const std::string &value, std::mutex &mutex);
};
} // end of namespace Module
} // end of namespace Dic

#endif // DATA_INSIGHT_CORE_MODULE_CORE_DATABASE_BASE_H
