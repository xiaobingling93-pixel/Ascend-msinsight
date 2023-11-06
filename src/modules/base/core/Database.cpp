/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

#include <thread>
#include "ServerLog.h"
#include "Database.h"

namespace Dic {
namespace Module {
using namespace Dic::Server;
Database::~Database()
{
    if (isOpen) {
        CloseDb();
    }
}

bool Database::OpenDb(const std::string &dbPath, bool clearAllTable)
{
    if (isOpen) {
        return false;
    }
    int result = sqlite3_open_v2(CheckSqlString(dbPath).c_str(), &db,
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX | SQLITE_OPEN_SHAREDCACHE, nullptr);
    if (result == SQLITE_OK) {
        isOpen = true;
        this->path = dbPath;
        if (clearAllTable && !DropAllTable()) {
            ServerLog::Warn("Failed to drop all tables");
        }
        return true;
    }
    ServerLog::Error("open db fail. path:", dbPath);
    return false;
}

/*
 * connect exist db
 */
bool Database::AttachDb(const std::string &dbPath)
{
    if (isOpen) {
        return false;
    }

    int result = sqlite3_open_v2(CheckSqlString(dbPath).c_str(), &db,
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX, nullptr);
    if (result == SQLITE_OK) {
        isOpen = true;
        return true;
    }
    ServerLog::Error("open db fail. path:", dbPath);
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
    uint64_t pos = 0;
    while (res.find('\'', pos) != std::string::npos) {
        pos = res.find('\'', pos);
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
    if (!isOpen) {
        return false;
    }
    return ExecSql("BEGIN;");
}

bool Database::EndTransaction()
{
    if (!isOpen) {
        return false;
    }
    return ExecSql("COMMIT;");
}

std::string Database::GetDbPath()
{
    return path;
}

bool Database::GetTableList(std::vector<std::string> &tableList) const
{
    if (!isOpen) {
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
        return false;
    }
    static const std::string SQL = "DROP TABLE IF EXISTS ";
    std::vector<std::string> tableList;
    if (GetTableList(tableList)) {
        for (const auto &table : tableList) {
            if (ExecSql(SQL + table + ";")) {
                ServerLog::Info("Drop table ", table);
            } else {
                ServerLog::Error("Failed to drop table ", table);
            }
        }
    }
    return ExecSql("VACUUM");
}

std::unique_ptr<SqlitePreparedStatement> Database::CreatPreparedStatement(const std::string &sql)
{
    if (!isOpen || sql.empty()) {
        return nullptr;
    }
    auto stmt = std::make_unique<SqlitePreparedStatement>(db);
    return stmt->Prepare(sql) ? std::move(stmt) : nullptr;
}

std::string Database::GetLastError()
{
    if (!isOpen) {
        return "";
    }
    return sqlite3_errmsg(db);
}
} // end of namespace Module
} // end of namespace Dic