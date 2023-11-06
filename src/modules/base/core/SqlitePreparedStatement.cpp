/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "SqlitePreparedStatement.h"

namespace Dic {
namespace Module {
SqlitePreparedStatement::SqlitePreparedStatement(sqlite3 *db) : db(db)
{
}

SqlitePreparedStatement::~SqlitePreparedStatement()
{
    if (stmt != nullptr) {
        sqlite3_finalize(stmt);
        stmt = nullptr;
    }
}

bool SqlitePreparedStatement::Prepare(std::string_view sql)
{
    if (stmt != nullptr) {
        sqlite3_finalize(stmt);
        stmt = nullptr;
    }
    lastErrorCode = sqlite3_prepare_v2(db, sql.data(), sql.length(), &stmt, nullptr);
    if (lastErrorCode != SQLITE_OK) {
        return false;
    }
    return true;
}

int SqlitePreparedStatement::GetErrorCode() const
{
    return lastErrorCode;
}

std::string SqlitePreparedStatement::GetErrorMessage() const
{
    return sqlite3_errmsg(db);
}

void SqlitePreparedStatement::BindParam(int index, std::string_view value)
{
    lastErrorCode = sqlite3_bind_text(stmt, index, value.data(), value.length(), SQLITE_TRANSIENT);
}

void SqlitePreparedStatement::BindParam(int index, int32_t value)
{
    lastErrorCode = sqlite3_bind_int(stmt, index, value);
}

void SqlitePreparedStatement::BindParam(int index, int64_t value)
{
    lastErrorCode = sqlite3_bind_int64(stmt, index, value);
}

void SqlitePreparedStatement::BindParam(int index, uint32_t value)
{
    lastErrorCode = sqlite3_bind_int64(stmt, index, static_cast<int64_t>(value));
}

void SqlitePreparedStatement::BindParam(int index, uint64_t value)
{
    lastErrorCode = sqlite3_bind_int64(stmt, index, static_cast<int64_t>(value));
}

void SqlitePreparedStatement::BindParam(int index, double value)
{
    lastErrorCode = sqlite3_bind_double(stmt, index, value);
}

SqlitePreparedStatement& SqlitePreparedStatement::Reset()
{
    lastErrorCode = sqlite3_reset(stmt);
    bindIndex = 1;
    return *this;
}

bool SqlitePreparedStatement::Execute()
{
    lastErrorCode = sqlite3_step(stmt);
    return lastErrorCode == SQLITE_DONE;
}

std::unique_ptr<SqliteResultSet> SqlitePreparedStatement::ExecuteQuery()
{
    if (lastErrorCode != SQLITE_OK) {
        return nullptr;
    }
    return std::make_unique<SqliteResultSet>(stmt);
}

} // end of namespace Module
} // end of namespace Dic