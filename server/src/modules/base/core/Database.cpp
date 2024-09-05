/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

#include "pch.h"
#include "TableDefs.h"
#include "Database.h"
#include "ConstantDefs.h"

namespace Dic {
namespace Module {
using namespace Dic::Server;
Database::~Database()
{
    if (isOpen) {
        CloseDb();
    }
}

bool Database::CreateDbIfNotExist(const std::string &dbPath)
{
    struct stat st;
    std::string dbPathStr = CheckSqlString(dbPath);
    if (stat(dbPathStr.c_str(), &st) == -1) {
        int result = sqlite3_open(dbPathStr.c_str(), &db);
        if (result) {
            sqlite3_close(db); // 异常后关闭数据库
            ServerLog::Error("Open db fail when create Db.");
            return false;
        }
        sqlite3_close(db); // 修改权限前先关闭数据库
        mode_t mode = 0640; // 业务数据权限要求设置为0640 （rw-r-----）
        result = FileUtil::ModifyFilePermissions(dbPathStr, mode);
        if (result) {
            ServerLog::Error("Can't set db file permissions.");
            return false;
        }
    }
    return true;
}

bool Database::OpenDb(const std::string &dbPath, bool clearAllTable)
{
    if (!FileUtil::CheckFilePathLength(dbPath)) {
        ServerLog::Error("This db path is illegal");
        return false;
    }
    if (!Database::CreateDbIfNotExist(dbPath)) {
        ServerLog::Error("Create db fail.");
        return false;
    }
    if (isOpen) {
        ServerLog::Error("The db file has been opened.");
        return false;
    }
    int result = sqlite3_open_v2(CheckSqlString(dbPath).c_str(), &db,
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_NOMUTEX, nullptr);
    if (result == SQLITE_OK) {
        isOpen = true;
        this->path = dbPath;
        if (clearAllTable && !DropAllTable()) {
            ServerLog::Warn("Failed to drop all tables");
        }
        sqlite3_busy_timeout(db, timeoutMs);
        return true;
    }
    ServerLog::Error("Faild when open existed Db.");
    return false;
}

/*
 * connect exist db
 */
bool Database::AttachDb(const std::string &dbPath)
{
    if (isOpen) {
        ServerLog::Error("The db file has been opened.");
        return false;
    }

    int result = sqlite3_open_v2(CheckSqlString(dbPath).c_str(), &db,
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX, nullptr);
    if (result == SQLITE_OK) {
        isOpen = true;
        path = dbPath;
        return true;
    }
    ServerLog::Error("open db fail.");
    return false;
}

void Database::CloseDb()
{
    if (!isOpen) {
        return;
    }
    if (db != nullptr) {
        sqlite3_close_v2(db);
        db = nullptr;
    }
    isOpen = false;
    path.clear();
}

bool Database::ExecSql(const std::string &sql)
{
    if (!isOpen) {
        ServerLog::Error("The db file is not opened.");
        return false;
    }
    int result = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);
    if (result == SQLITE_OK) {
        return true;
    }
    ServerLog::Error("ExecSql fail. sql:", sql, "\terror message:", sqlite3_errmsg(db));
    return false;
}

bool Database::IsOpen() const
{
    return isOpen;
}

// replace ' by ''
std::string Database::CheckSqlString(const std::string &src)
{
    std::string res(src);
    size_t pos = 0;
    while ((pos = res.find('\'', pos)) != std::string::npos) {
        res.replace(pos, 1, "''");
        pos += sizeof("''") - 1;
    }
    return res;
}

std::string Database::sqlite3_column_string(sqlite3_stmt *stmt, int iCol)
{
    if (stmt == nullptr) {
        return "";
    }
    const unsigned char *data = sqlite3_column_text(stmt, iCol);
    if (data == nullptr) {
        return "";
    }
    int len = sqlite3_column_bytes(stmt, iCol);
    if (len <= 0) {
        return "";
    }
    return std::string(reinterpret_cast<const char *>(data), len);
}

bool Database::StartTransaction()
{
    return isOpen && ExecSql("BEGIN;");
}

bool Database::RollbackTransaction()
{
    return isOpen && ExecSql("ROLLBACK;");
}

bool Database::EndTransaction()
{
    return isOpen && ExecSql("COMMIT;");
}

std::string Database::GetDbPath()
{
    return path;
}

void Database::SetDbPath(const std::string& dbPath)
{
    path = dbPath;
}

bool Database::GetMetaVersion()
{
    if (CheckTableExist(TABLE_META_DATA)) {
        isLowCamel = true;
        auto sql = "select value as version from " + TABLE_META_DATA + " where name = ?";
        auto stmt = CreatPreparedStatement();
        try {
            auto resultSet = ExecuteQuery(stmt, sql, "SCHEMA_VERSION");
            if (resultSet->Next()) {
                metaVersion = resultSet->GetString(resultStartIndex);
            }
        } catch (DatabaseException &e) {
            ServerLog::Error("Get Meta Version Fail, ", e.What());
            return false;
        }
    }
    return true;
}

bool Database::IsDatabaseVersionChange()
{
    std::string version;
    if (!isOpen) {
        ServerLog::Error("The db file is not opened.");
        return false;
    }
    static const std::string SQL = "PRAGMA user_version";
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, SQL.c_str(), -1, &stmt, nullptr);
    if (result == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            version = sqlite3_column_string(stmt, 0);
        }
    } else {
        sqlite3_finalize(stmt);
        ServerLog::Error("Fail to get version.", sqlite3_errmsg(db));
        return false;
    }
    sqlite3_finalize(stmt);
    return std::strcmp(version.c_str(), GetDataBaseVersion().c_str()) != 0;
}

bool Database::SetDataBaseVersion()
{
    if (!isOpen) {
        ServerLog::Error("Failed to set db version. Database is not open.");
        return false;
    }
    std::string dbVersion = GetDataBaseVersion();
    std::unique_lock<std::recursive_mutex> lock(mutex);
    return ExecSql(" PRAGMA user_version = " + dbVersion + ";");
}

std::string Database::GetDataBaseVersion()
{
    std::stringstream version;
#ifdef DATABASE_VERSION
    version << DATABASE_VERSION;
#endif
    if (!StringUtil::CheckSqlValid(version.str())) {
        ServerLog::Error("There is an SQL injection attack on this parameter. error param: ", version.str());
        return "";
    }
    return version.str();
}

bool Database::GetTableList(std::vector<std::string> &tableList) const
{
    if (!isOpen) {
        ServerLog::Error("The db file is not opened when get table list.");
        return false;
    }
    static const std::string SQL = "select name from sqlite_master where type='table'";
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, SQL.c_str(), -1, &stmt, nullptr);
    if (result == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string table = sqlite3_column_string(stmt, 0);
            if (table == "sqlite_sequence") {
                continue;
            }
            tableList.emplace_back(table);
        }
    } else {
        sqlite3_finalize(stmt);
        ServerLog::Error("Fail to get table list.", sqlite3_errmsg(db));
        return false;
    }
    sqlite3_finalize(stmt);
    return true;
}

bool Database::DropAllTable()
{
    if (!isOpen) {
        ServerLog::Error("The db file is not opened when drop all table.");
        return false;
    }
    static const std::string SQL = "DROP TABLE IF EXISTS ";
    std::vector<std::string> tableList;
    if (!GetTableList(tableList)) {
        return false;
    }
    if (tableList.empty()) {
        return true;
    }
    std::string dropSql;
    for (const auto &table : tableList) {
        dropSql.append(SQL).append(table).append(";");
    }
    return ExecSql(dropSql);
}

bool Database::DropSomeTables(const std::vector<std::string> &tableNames)
{
    if (!isOpen) {
        ServerLog::Error("The db file is not opened when drop specific tables.");
        return false;
    }

    std::vector<std::string> tableList;
    if (!GetTableList(tableList)) {
        return false;
    }
    if (tableList.empty()) {
        return true;
    }
    static const std::string SQL = "DROP TABLE IF EXISTS ";
    std::string dropSql;
    for (const auto &table : tableNames) {
        if (std::find(tableList.begin(), tableList.end(), table) != tableList.end()) {
            dropSql.append(SQL).append(table).append(";");
        }
    }
    return ExecSql(dropSql);
}

std::unique_ptr<SqlitePreparedStatement> Database::CreatPreparedStatement(const std::string &sql)
{
    if ((!isOpen) || sql.empty()) {
        ServerLog::Error("Failed prepare sql. Database is closed or sql is empty.");
        return nullptr;
    }
    auto stmt = std::make_unique<SqlitePreparedStatement>(db);
    if (!stmt->Prepare(sql)) {
        ServerLog::Error("Failed prepare sql. ", stmt->GetErrorMessage());
        return nullptr;
    }
    return stmt;
}

std::unique_ptr<SqlitePreparedStatement> Database::CreatPreparedStatement()
{
    if ((!isOpen)) {
        ServerLog::Error("Failed prepare sql. Database is closed or sql is empty.");
        return nullptr;
    }
    return std::make_unique<SqlitePreparedStatement>(db);
}

bool Database::CheckTableContainData(const std::string& tableName)
{
    if ((!isOpen)) {
        ServerLog::Error("Failed Check Table. Database is closed or sql is empty.");
        return false;
    }
    std::string sql = "SELECT count(*) FROM sqlite_master WHERE type='table' AND name=?;";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Failed prepare sql. ");
        return false;
    }
    auto result = stmt->ExecuteQuery(tableName);
    if (result != nullptr && result->Next()) {
        if (result->GetInt64("count(*)") > 0) {
            sql = "SELECT count(*) FROM " + tableName + ";";
            stmt = CreatPreparedStatement(sql);
            if (stmt == nullptr) {
                ServerLog::Error("Failed prepare sql. ");
                return false;
            }
            result = stmt->ExecuteQuery();
            if (result != nullptr && result->Next()) {
                return result->GetInt64("count(*)") > 0;
            }
        }
    }
    return false;
}


bool Database::CheckTableExist(const std::string& tableName)
{
    if ((!isOpen)) {
        ServerLog::Error("Failed Check Table. Database is closed or sql is empty.");
        return false;
    }
    std::string sql = "SELECT name FROM sqlite_master WHERE type='table' AND name=?;";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Failed prepare sql. ");
        return false;
    }
    auto resultSet = stmt->ExecuteQuery(tableName);
    if (resultSet->GetErrorCode() == SQLITE_OK && resultSet->Next()) {
        return true;
    }
    return false;
}

bool Database::CreateStatusInfoTable()
{
    if (CheckTableExist(infoTable)) {
        return true;
    }
    std::string sql = "CREATE TABLE " + infoTable + "(id INTEGER PRIMARY KEY AUTOINCREMENT, key TEXT, value TEXT);";
    return ExecSql(sql);
}

std::string Database::GetValueFromStatusInfoTable(const std::string& key)
{
    std::string value;
    if (!CheckTableExist(infoTable)) {
        ServerLog::Warn("Failed to get value from info table because table is not exist.");
        return value;
    }

    std::string sql = "SELECT value From " + infoTable + " WHERE key = ?";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Failed to prepare sql for getting value from StatusInfoTable: ", sqlite3_errmsg(db));
        return value;
    }
    auto results = stmt->ExecuteQuery(key);
    if (results == nullptr) {
        ServerLog::Error("Failed to get result set for getting value from StatusInfoTable: ", stmt->GetErrorMessage());
        return value;
    }
    if (results->Next()) {
        value = results->GetString("value");
    }
    return value;
}

bool Database::CheckValueFromStatusInfoTable(const std::string &key, const std::string &refValue)
{
    if (key.empty() || refValue.empty()) {
        ServerLog::Error("Failed to get status for checking value from StatusInfoTable due to empty key or value.");
        return false;
    }
    std::string status = GetValueFromStatusInfoTable(key);
    if (status.empty() || strcmp(status.c_str(), refValue.c_str()) != 0) {
        return false;
    }
    return true;
}

bool Database::UpdateValueIntoStatusInfoTable(const std::string& key, const std::string& value)
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    if (!CheckTableExist(infoTable) && !CreateStatusInfoTable()) {
        ServerLog::Error("Failed to update status info table because table is not exist: key=", key, ", value=", value);
        return false;
    }
    std::string sql;
    if (GetValueFromStatusInfoTable(key).empty()) {
        sql = "INSERT INTO " + infoTable + " (value, key) VALUES (?, ?);";
    } else {
        sql = "UPDATE " + infoTable + " SET value=? WHERE key=?;";
    }
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Failed to create prepared stmt: key=", key, ", value=", value);
        return false;
    }
    stmt->BindParams(value, key);
    if (!stmt->Execute()) {
        ServerLog::Error("Failed to execute prepared stmt: key=", key, ", value=", value);
        return false;
    }

    return true;
}

std::string Database::GetLastError()
{
    if (!isOpen) {
        return "";
    }
    return sqlite3_errmsg(db);
}

bool Database::SetConfig()
{
    if (!isOpen) {
        ServerLog::Error("Failed to set config. Database is not open.");
        return false;
    }
    std::string dbVersion = GetDataBaseVersion();
    std::lock_guard<std::recursive_mutex> lock(mutex);
    if (IsDatabaseVersionChange()) {
        return ExecSql("PRAGMA synchronous = OFF; PRAGMA journal_mode = MEMORY;"
                       " PRAGMA user_version = " + dbVersion + ";");
    }
    return true;
}
} // end of namespace Module
} // end of namespace Dic