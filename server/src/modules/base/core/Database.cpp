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
    std::string utfDbPath = StringUtil::ToUtf8Str(dbPathStr);
    if (stat(dbPathStr.c_str(), &st) == -1) {
        int result = sqlite3_open(utfDbPath.c_str(), &db);
        if (result) {
            sqlite3_close(db); // 异常后关闭数据库
            ServerLog::Error("Open db fail when create Db. path is: ", utfDbPath);
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
    if (!StringUtil::ValidateStringParam(dbPath)) {
        ServerLog::Error("DB path contains illegal character, such as '|', ';', '&', '$', etc.");
        return false;
    }
    if (!FileUtil::CheckFilePathLength(dbPath)) {
        ServerLog::Error("This db path length is illegal.Db path:", dbPath);
        return false;
    }
    std::string dbDir = FileUtil::GetParentPath(dbPath);
    if (!std::empty(dbDir) && !FileUtil::CheckDirValid(dbDir)) {
        ServerLog::Error("This db dir is illegal path:", dbDir);
        return false;
    }
    if (!Database::CreateDbIfNotExist(dbPath)) {
        ServerLog::Error("Create db failed.Db path:", dbPath);
        return false;
    }
    if (!FileUtil::CheckDirValid(dbPath)) {
        ServerLog::Error("This db path is illegal.Db path:", dbPath);
        return false;
    }
    if (isOpen) {
        ServerLog::Error("The db file has been opened.");
        return false;
    }
    std::string utfDbPath = StringUtil::ToUtf8Str(dbPath);
    int result = sqlite3_open_v2(CheckSqlString(utfDbPath).c_str(), &db,
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX, nullptr);
    if (result == SQLITE_OK) {
        isOpen = true;
        this->path = dbPath;
        if (clearAllTable && !DropAllTable()) {
            ServerLog::Warn("Failed to drop all tables");
        }
        sqlite3_busy_timeout(db, timeoutMs);
        return true;
    }
    ServerLog::Error("Failed when open existed Db.");
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

std::string Database::Sqlite3ColumnConvertStr(int colType, sqlite3_stmt *stmt, int iCol)
{
    std::ostringstream oss;
    if (colType == SQLITE_INTEGER) {
        int value = sqlite3_column_int(stmt, iCol);
        oss << value;
    } else if (colType == SQLITE_FLOAT) {
        double value = sqlite3_column_double(stmt, iCol);
        oss << value;
    } else {
        oss << "Unknown type";
    }
    return oss.str();
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

std::string Database::QueryValueFromMetaDataByName(const std::string &name)
{
    // 外层需要校验TABLE_META_DATA表是否存在
    std::string result;
    auto sql = "select value from " + TABLE_META_DATA + " where name = ?";
    auto stmt = CreatPreparedStatement();
    try {
        auto resultSet = ExecuteQuery(stmt, sql, name);
        if (resultSet->Next()) {
            result = resultSet->GetString(resultStartIndex);
        }
        return result;
    } catch (DatabaseException &e) {
        ServerLog::Error("Get Meta data Fail, name:", name, ", error:", e.What());
        return result;
    }
}

bool Database::QueryMetaVersion()
{
    if (CheckTableExist(TABLE_META_DATA)) {
        isLowCamel = true;
        std::string schemaVersion = QueryValueFromMetaDataByName("SCHEMA_VERSION");
        if (schemaVersion.empty()) {
            ServerLog::Error("Get Meta Version Fail");
            metaVersion = "1.0.0";
            return true;
        }
        metaVersion = schemaVersion;
    }
    return true;
}

std::string Database::GetMetaVersion()
{
    return metaVersion;
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
            if (!StringUtil::CheckSqlValid(table)) {
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
        if (result->GetUint64("count(*)") > 0) {
            sql = "SELECT count(*) FROM " + tableName + ";";
            stmt = CreatPreparedStatement(sql);
            if (stmt == nullptr) {
                ServerLog::Error("Failed prepare sql. ");
                return false;
            }
            result = stmt->ExecuteQuery();
            if (result != nullptr && result->Next()) {
                return result->GetUint64("count(*)") > 0;
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
    if (resultSet == nullptr) {
        ServerLog::Error("Query Table Exist failed!");
        return false;
    }
    if (resultSet->GetErrorCode() == SQLITE_OK && resultSet->Next()) {
        return true;
    }
    return false;
}

bool Database::CheckTablesExist(const std::vector<std::string> &tablesName)
{
    if (tablesName.empty()) {
        ServerLog::Error("Failed to check tables due to empty table name.");
        return false;
    }
    if ((!isOpen)) {
        ServerLog::Error("Failed to check tables due to closed database.");
        return false;
    }
    std::vector<std::string> tableLists{};
    if (!GetTableList(tableLists)) {
        ServerLog::Error("Failed to check tables. Failed to get table list.");
        return false;
    }
    for (const auto& item : tablesName) {
        if (std::find(tableLists.begin(), tableLists.end(), item) == tableLists.end()) {
            ServerLog::Error("Failed to check tables due to no table: ", item);
            return false;
        }
    }
    return true;
}

bool Database::CheckColumnExist(const std::string& tableName, const std::string& columnName)
{
    if (!isOpen) {
        ServerLog::Error("Failed to check column. Database is closed or sql is empty.");
        return false;
    }
    if (!StringUtil::CheckSqlValid(tableName)) {
        ServerLog::Error("There is an SQL injection attack when check column. Table name: ", tableName);
        return false;
    }
    std::string sql = "PRAGMA table_info(" + tableName + ")";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Failed prepare sql when check column.");
        return false;
    }

    auto resultSet = stmt->ExecuteQuery();
    if (resultSet == nullptr) {
        ServerLog::Error("Failed to execute query when check column.");
        return false;
    }
    while (resultSet->Next()) {
        std::string currentColumnName = resultSet->GetString("name");
        if (currentColumnName == columnName) {
            return true;
        }
    }
    return false;
}

bool Database::CheckStringInColumn(const std::string& tableName, const std::string& columnName,
                                   const std::string& searchString)
{
    if ((!isOpen)) {
        ServerLog::Error("Failed to check string in column. Database is closed or sql is empty.");
        return false;
    }
    if (!StringUtil::CheckSqlValid(tableName) || !StringUtil::CheckSqlValid(columnName)) {
        ServerLog::Error("There is an SQL injection attack when check string in column.");
        return false;
    }
    std::string sql = "select count(*) from " + tableName + " where " + columnName + " = ?;";

    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Failed prepare sql when check string in column.");
        return false;
    }
    auto resultSet = stmt->ExecuteQuery(searchString);
    if (resultSet == nullptr) {
        ServerLog::Error("Failed to execute query when check string in column.");
        return false;
    }
    if (resultSet->Next()) {
        return resultSet->GetInt64("count(*)") > 0;
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
        ServerLog::Warn("Get empty value from info table because table is not exist.");
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

bool Database::UpdateValueIntoStatusInfoTable(const std::string &key, const std::string &value)
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

bool Database::CheckAndResetDatabaseOnVersionChange()
{
    bool res = true;
    std::lock_guard<std::recursive_mutex> lock(mutex);
    if (IsDatabaseVersionChange()) {
        ServerLog::Info("The database version has changed, the table structure and data will be reset.");
        res = DropAllTable() && SetDataBaseVersion();
    }
    return res;
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
    if (!ExecSql("PRAGMA journal_mode = WAL;PRAGMA synchronous = OFF;")) {
        return false;
    }
    return true;
}

bool Database::ExtendColumns(const std::string &tableName, const std::vector<std::string>& columns)
{
    if (!isOpen) {
        ServerLog::Error("Failed to extend table. Database is not open.");
        return false;
    }
    std::string sql;
    for (const auto& column : columns) {
        sql += "ALTER TABLE " + tableName + " ADD COLUMN " + column + " TEXT; ";
    }
    std::lock_guard<std::recursive_mutex> lock(mutex);
    return ExecSql(sql);
}

bool Database::CreateMetaDataTableForText()
{
    std::lock_guard<std::recursive_mutex> lock(mutex);
    if (!isOpen) {
        ServerLog::Error("Failed to create meta data table. Database is not open.");
        return false;
    }
    if (CheckTableExist(metaDataTable)) {
        return true;
    }
    std::string sql = "CREATE TABLE " + metaDataTable + " ( name TEXT PRIMARY KEY, value TEXT );";
    return ExecSql(sql);
}

std::string Database::GetValueFromMetaDataTable(const std::string& name)
{
    std::string value;
    if (!StringUtil::CheckSqlValid(name)) {
        ServerLog::Error("There is an SQL injection attack on this parameter, param: name.");
    }
    if (!CheckTableExist(metaDataTable)) {
        ServerLog::Warn("Get empty value from meta data table because table is not exist.");
        return value;
    }

    std::string sql = "SELECT value From " + metaDataTable + " WHERE name = ?";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Failed to prepare sql for getting value from meta data table: ", sqlite3_errmsg(db));
        return value;
    }
    auto results = stmt->ExecuteQuery(name);
    if (results == nullptr) {
        ServerLog::Error("Failed to get result set for getting value from meta data table: ", stmt->GetErrorMessage());
        return value;
    }
    if (results->Next()) {
        value = results->GetString("value");
    }
    return value;
}

bool Database::UpdateMetaDataTable(const std::string &name, const std::string &value)
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    std::string sql = "INSERT INTO " + metaDataTable + " (value, name) VALUES (?, ?)"
                      " ON CONFLICT(name) DO"
                      " UPDATE SET value = excluded.value;";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Failed to create prepared stmt: name=", name, ", value=", value);
        return false;
    }
    stmt->BindParams(value, name);
    if (!stmt->Execute()) {
        ServerLog::Error("Failed to execute prepared stmt: name=", name, ", value=", value);
        return false;
    }

    return true;
}

bool Database::UpdateMetaDataTableWithNoPrimaryKey(const std::string &name, const std::string &value)
{
    std::unique_lock<std::recursive_mutex> lock(mutex);
    std::string checkSql = "SELECT 1 FROM " + metaDataTable + " WHERE name = ? LIMIT 1;";
    auto checkStmt = CreatPreparedStatement(checkSql);
    if (checkStmt == nullptr) {
        ServerLog::Error("Failed to create check stmt: name=", name);
        return false;
    }
    auto exist = checkStmt->ExecuteQuery(name);
    if (exist == nullptr) {
        ServerLog::Error("Failed to get result set for getting value from meta data table: ",
                         checkStmt->GetErrorMessage());
        return false;
    }

    std::string actionSql;
    if (exist->Next()) {
        actionSql = "UPDATE " + metaDataTable + " SET value = ? WHERE name = ?;";
    } else {
        actionSql = "INSERT INTO " + metaDataTable + " (value, name) VALUES (?, ?);";
    }
    auto actionStmt = CreatPreparedStatement(actionSql);
    if (actionStmt == nullptr) {
        ServerLog::Error("Failed to create action stmt: name=", name);
        return false;
    }

    actionStmt->BindParams(value, name);
    if (!actionStmt->Execute()) {
        ServerLog::Error("Failed to execute action stmt: name=", name, ", value=", value);
        return false;
    }
    return true;
}

std::vector<ColumnAtt> Database::QueryTableInfoByName(const std::string& tableName)
{
    if (!StringUtil::CheckSqlValid(tableName)) {
        return {};
    }
    const std::string sql = "PRAGMA table_info(" + tableName + ");";
    auto stmt = CreatPreparedStatement(sql);
    if (!TryOpt(stmt, "Query table info failed to prepare sql!")) {
        return {};
    }
    auto result = stmt->ExecuteQuery();
    if (!TryOpt(result, "Query table info failed to get result!")) {
        return {};
    }
    std::vector<ColumnAtt> res;
    while (result->Next()) {
        ColumnAtt columnAtt;
        columnAtt.name = result->GetString("name");
        columnAtt.type = result->GetString("type");
        columnAtt.key = columnAtt.name;
        res.emplace_back(columnAtt);
    }
    return res;
}

uint64_t Database::QueryCountByTableName(const std::string& tableName)
{
    if (!StringUtil::CheckSqlValid(tableName)) {
        return 0;
    }
    std::string sql = "SELECT COUNT(*) as count FROM " + tableName + " WHERE 1 = 1 ";
    auto stmt = CreatPreparedStatement(sql);
    if (!TryOpt(stmt, "Query count by table name failed to prepare sql!")) {
        return 0;
    }
    auto result = stmt->ExecuteQuery();
    if (!TryOpt(result, "Query count by table name failed to get result!")) {
        return 0;
    }
    if (result->Next()) {
        return result->GetUint64("count");
    }
    return 0;
}

std::vector<std::map<std::string, std::string>> Database::QueryDataByPage(const PageQuery& query,
                                                                          const std::vector<ColumnAtt>& columns)
{
    if (columns.empty()) {
        return {};
    }
    std::vector<std::string> columnName;
    for (const auto& item : columns) {
        if (!StringUtil::CheckSqlValid(item.key)) {
            return {};
        }
        columnName.emplace_back(item.key);
    }
    if (!StringUtil::CheckSqlValid(query.viewName)) {
        return {};
    }
    const std::string columnNames = StringUtil::join(columnName, ",");
    std::string sql = "SELECT " + columnNames + " FROM " + query.viewName + " WHERE 1=1 ";
    std::string orderBySql;
    if (!std::empty(query.order) && !std::empty(query.orderBy) && StringUtil::CheckSqlValid(query.orderBy)) {
        orderBySql = query.order == "descend" ? " ORDER BY " + query.orderBy + " DESC " :
                                                " ORDER BY " + query.orderBy + " ASC ";
    }
    sql += orderBySql;
    const std::string limitSql =
        " LIMIT " + std::to_string(query.size) + " OFFSET " + std::to_string(query.ComputeOffset());
    sql += limitSql;
    auto stmt = CreatPreparedStatement(sql);
    if (!TryOpt(stmt, "Query data by page failed to prepare sql!")) {
        return {};
    }
    auto result = stmt->ExecuteQuery();
    if (!TryOpt(result, "Query data by page failed to get result!")) {
        return {};
    }
    std::vector<std::map<std::string, std::string>> res;
    while (result->Next()) {
        std::map<std::string, std::string> data;
        for (const auto& item : columns) {
            data[item.key] = result->GetString(item.key);
            if (std::empty(data[item.key])) {
                data[item.key] = "0";
            }
        }
        res.emplace_back(data);
    }
    return res;
}
}  // end of namespace Module
}  // end of namespace Dic