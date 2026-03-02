/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2026 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */

#include "MemSnapshotDatabase.h"
#include "DataBaseManager.h"
#include "ServerLog.h"

namespace Dic::Module::FullDb {
bool MemSnapshotDatabase::CheckAllTableExist()
{ return Database::CheckTablesExist({blockTable, traceEntryTable, dictionaryTable}); }

bool MemSnapshotDatabase::OpenDbReadOnly(const std::string& dbPath)
{
    if (isOpen) {
        Server::ServerLog::Warn(LOG_TAG + "The database connection has been opened. Nothing to do.");
        return true;
    }
    if (!Database::OpenDb(dbPath, false)) {
        Server::ServerLog::Error(LOG_TAG + "Failed to open snapshot db with path: %.", dbPath);
        return false;
    }
    // 检查必要表
    if (!CheckAllTableExist()) {
        Server::ServerLog::Error(LOG_TAG + "Failed to check all table exists.");
        return false;
    }
    // 初始化db实例上下文
    if (!InitContext()) {
        Server::ServerLog::Error(LOG_TAG + "Failed to initialize snapshot database context.");
        return false;
    }
    return true;
}

std::string MemSnapshotDatabase::GetRealValueInTableDictionaryMap(const std::string& tableName,
                                                                  const std::string& colName, int intVal)
{
    std::string colTag = GetTableColumnTag(tableName, colName);
    if (tableDictionaryMap.find(colTag) == tableDictionaryMap.end()) {
        return std::to_string(intVal);
    }
    if (tableDictionaryMap[colTag].find(intVal) == tableDictionaryMap[colTag].end()) {
        return std::to_string(intVal);
    }
    return tableDictionaryMap[colTag][intVal];
}

bool MemSnapshotDatabase::QueryAllBlocks(std::vector<Block>& blocks)
{
    std::string querySql = "SELECT * FROM {} ORDER BY {}";
    querySql = StringUtil::FormatString(querySql, blockTable, BlockTableColumn::ALLOC_EVENT_ID);
    sqlite3_stmt* stmt = nullptr;
    int result = sqlite3_prepare_v2(db, querySql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        Server::ServerLog::Error(LOG_TAG + "Failed to prepared query block sql, error: ", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return false;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        blocks.push_back(QueryBlockByStep(stmt));
    };
    sqlite3_finalize(stmt);
    return true;
}

std::optional<Block> MemSnapshotDatabase::QueryBlockById(const int64_t blockId)
{
    std::string querySql = "SELECT * FROM {} WHERE {} = ?;";
    querySql = StringUtil::FormatString(querySql, blockTable, BlockTableColumn::ID);
    sqlite3_stmt* stmt = nullptr;
    int result = sqlite3_prepare_v2(db, querySql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        Server::ServerLog::Error(LOG_TAG + "Failed prepared query block sql, error: ", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return std::nullopt;
    }
    sqlite3_bind_int64(stmt, bindStartIndex, blockId);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        Block block = QueryBlockByStep(stmt);
        sqlite3_finalize(stmt);
        return std::make_optional(block);
    }
    sqlite3_finalize(stmt);
    return std::nullopt;
}

int64_t MemSnapshotDatabase::QueryMaxEntryId() const
{
    if (maxEntryId > 0) {
        return maxEntryId;
    }
    std::string querySql = "SELECT MAX({}) FROM {};";
    querySql = StringUtil::FormatString(querySql, TraceEntryTableColumn::ID, traceEntryTable);
    sqlite3_stmt* stmt = nullptr;
    int result = sqlite3_prepare_v2(db, querySql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        Server::ServerLog::Error(LOG_TAG + "Failed prepared query max entry id sql, error: ", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 0;
    }
    int64_t maxId = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        maxId = sqlite3_column_int64(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return maxId;
}

void MemSnapshotDatabase::Reset()
{
    Server::ServerLog::Info(LOG_TAG + "MemSnapshot db reset.");
    auto databaseList = Timeline::DataBaseManager::Instance().GetAllMemSnapshotDatabase();
    for (auto& db : databaseList) {
        if (db != nullptr && db->IsOpen()) {
            db->CloseDb();
        }
    }
    Timeline::DataBaseManager::Instance().Clear(Timeline::DatabaseType::MEM_SNAPSHOT);
}

bool MemSnapshotDatabase::InitTableDictionaryMap()
{
    if (!isOpen) {
        Server::ServerLog::Error(LOG_TAG +
                                 "Failed to initialize table dictionary map; "
                                 "database connection has not been established.");
        return false;
    }
    const std::string queryDictionaryTableSql = "SELECT * FROM dictionary;";
    sqlite3_stmt* stmt = nullptr;
    int result = sqlite3_prepare_v2(db, queryDictionaryTableSql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        Server::ServerLog::Error(LOG_TAG +
                                     "Failed to initialize table dictionary map; "
                                     "failed to prepare sql. Error: ",
                                 sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return false;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        std::string table = sqlite3_column_string(stmt, col++);
        std::string column = sqlite3_column_string(stmt, col++);
        int colKey = sqlite3_column_int(stmt, col++);
        std::string colValue = sqlite3_column_string(stmt, col++);
        tableDictionaryMap[GetTableColumnTag(table, column)][colKey] = colValue;
    }
    sqlite3_finalize(stmt);
    return true;
}

bool MemSnapshotDatabase::InitContext()
{
    // 初始化dictionary
    const bool result = InitTableDictionaryMap();
    if (!result) {
        Server::ServerLog::Error(LOG_TAG +
                                 "Failed to initialize mem snapshot database context; "
                                 "failed to initialize table dictionary map.");
        return false;
    }
    // 初始化maxEntryId
    maxEntryId = QueryMaxEntryId();
    return true;
}

Block MemSnapshotDatabase::QueryBlockByStep(sqlite3_stmt* stmt)
{
    int col = resultStartIndex;
    Block block;
    block.id = sqlite3_column_int64(stmt, col++);
    block.address = static_cast<uint64_t>(sqlite3_column_int64(stmt, col++));
    block.size = static_cast<uint64_t>(sqlite3_column_int64(stmt, col++));
    block.requestedSize = static_cast<uint64_t>(sqlite3_column_int64(stmt, col++));
    block.state = GetRealValueInTableDictionaryMap(blockTable, std::string(BlockTableColumn::STATE),
                                                   sqlite3_column_int(stmt, col++));
    block.allocEventId = sqlite3_column_int64(stmt, col++);
    block.freeEventId = sqlite3_column_int64(stmt, col++);
    return block;
}

std::string MemSnapshotDatabase::GetTableColumnTag(const std::string& tableName, const std::string& colName)
{ return StringUtil::FormatString("{}.{}", tableName, colName); }
} // namespace Dic::Module::FullDb
