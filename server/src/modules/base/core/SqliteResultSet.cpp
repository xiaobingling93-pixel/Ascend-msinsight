/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "SqliteResultSet.h"

namespace Dic {
namespace Module {
SqliteResultSet::SqliteResultSet(sqlite3_stmt *stmt) : stmt(stmt)
{
    if (stmt != nullptr) {
        int columnCount = sqlite3_column_count(stmt);
        for (int i = 0; i < columnCount; ++i) {
            columns.emplace(sqlite3_column_name(stmt, i), i);
        }
    }
}

int SqliteResultSet::GetErrorCode() const
{
    return lastErrorCode;
}

std::string SqliteResultSet::GetErrorMessage() const
{
    return sqlite3_errmsg(sqlite3_db_handle(stmt));
}

const std::unordered_map<std::string_view, int> SqliteResultSet::GetColumns() const
{
    return columns;
}
} // end of namespace Module
} // end of namespace Dic