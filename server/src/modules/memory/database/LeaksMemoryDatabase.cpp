/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include "ProtocolDefs.h"
#include "DataBaseManager.h"
#include "LeaksMemoryDatabase.h"

namespace Dic {
namespace Module {
namespace FullDb {
using namespace Dic::Server;
using namespace Dic::Module::Timeline;
void LeaksMemoryDatabase::Reset()
{
    ServerLog::Info("Memory reset.");
    auto databaseList = Timeline::DataBaseManager::Instance().GetAllLeaksMemoryDatabase();
    for (auto &db : databaseList) {
        auto database = dynamic_cast<LeaksMemoryDatabase *>(db);
        if (database != nullptr) {
            database->CloseDb();
        }
    }
    Timeline::DataBaseManager::Instance().Clear(Timeline::DatabaseType::LEAKS);
}

bool LeaksMemoryDatabase::CreateMemoryAllocationAndBlockTable()
{
    if (!isOpen) {
        ServerLog::Error("[LeaksMemory] Failed to create table memory_block/memory_allocation. Database is not open.");
        return false;
    }
    std::string createSql = "DROP TABLE IF EXISTS " + memoryAllocationTable + ";"
                            "DROP TABLE IF EXISTS " + memoryBlockTable + ";"
                            "CREATE TABLE " + memoryAllocationTable +
                            "("
                            "  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
                            "  timestamp integer,"
                            "  totalSize integer,"
                            "  optimized integer,"
                            "  deviceId text(255)"
                            ");"
                            "CREATE TABLE " + memoryBlockTable +
                            "("
                            "  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
                            "  deviceId text(255),"
                            "  addr TEXT(255),"
                            "  size integer,"
                            "  startTimestamp integer,"
                            "  endTimestamp integer,"
                            "  attr TEXT(255)"
                            ");";
    std::unique_lock<std::recursive_mutex> lock(mutex);
    return ExecSql(createSql);
}

bool LeaksMemoryDatabase::HasFinishedParseLastTime()
{
    return CheckValueFromStatusInfoTable(leaksMemoryParseStatus, FINISH_STATUS);
}

bool LeaksMemoryDatabase::UpdateParseStatus(const std::string &status)
{
    return UpdateValueIntoStatusInfoTable(leaksMemoryParseStatus, status);
}

bool LeaksMemoryDatabase::QueryEntireEventsTable(std::vector<Memory::MemoryEvent> &eventDetails)
{
    std::string sql = "select * from " + TABLE_LEAKS_DUMP + " where deviceId != 'N/A' order by timestamp";
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query entire events table. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return false;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        Memory::MemoryEvent event{};
        event.id = std::abs(sqlite3_column_int64(stmt, col++));
        event.event = sqlite3_column_string(stmt, col++);
        event.eventType = sqlite3_column_string(stmt, col++);
        event.name = sqlite3_column_string(stmt, col++);
        event.timestamp = std::abs(sqlite3_column_int64(stmt, col++));
        event.processId = std::abs(sqlite3_column_int64(stmt, col++));
        event.threadId = std::abs(sqlite3_column_int64(stmt, col++));
        event.deviceId = sqlite3_column_string(stmt, col++);
        event.ptr = sqlite3_column_string(stmt, col++);
        event.attr = sqlite3_column_string(stmt, col++);
        eventDetails.emplace_back(event);
    }
    sqlite3_finalize(stmt);
    return true;
}

void LeaksMemoryDatabase::InsertMemoryAllocationList(const std::vector<Memory::MemoryAllocation> &allocList)
{
    sqlite3_stmt *stmt = GetInsertAllocationsStmt(allocList.size());
    if (stmt == nullptr) {
        ServerLog::Error("Failed to get Insert allocation stmt.");
        return;
    }

    int idx = bindStartIndex;
    for (const auto &alloc : allocList) {
        // (timestamp, totalSize, optimized, deviceId)
        sqlite3_bind_int64(stmt, idx++, alloc.timestamp > INT64_MAX ? INT64_MAX : alloc.timestamp);
        sqlite3_bind_int64(stmt, idx++, alloc.totalSize > INT64_MAX ? INT64_MAX : alloc.totalSize);
        sqlite3_bind_int(stmt, idx++, alloc.optimized ? 1 : 0);
        sqlite3_bind_text(stmt, idx++, alloc.deviceId.c_str(), alloc.deviceId.length(), SQLITE_TRANSIENT);
    }
    std::unique_lock<std::recursive_mutex> lock(mutex);
    auto result = sqlite3_step(stmt);
    if (allocList.size() != cacheSize) {
        sqlite3_finalize(stmt);
    }
    if (result != SQLITE_DONE) {
        ServerLog::Error("Failed to insert allocations. Error: ", sqlite3_errmsg(db));
    }
}

void LeaksMemoryDatabase::InsertMemoryAllocation(const Memory::MemoryAllocation &alloc)
{
    std::lock_guard<std::mutex> lock(cacheMutex);
    allocationCache.emplace_back(alloc);
    if (allocationCache.size() == cacheSize) {
        InsertMemoryAllocationList(allocationCache);
        allocationCache.clear();
    }
}

void LeaksMemoryDatabase::InsertMemoryBlockList(const std::vector<Memory::MemoryBlock> &blocklist)
{
    sqlite3_stmt *stmt = GetInsertBlocksStmt(blocklist.size());
    if (stmt == nullptr) {
        ServerLog::Error("Failed to get Insert Blocks Stmt.");
        return;
    }

    int idx = bindStartIndex;
    for (const auto &block : blocklist) {
        // (deviceId, addr, size, startTimestamp, endTimestamp, attr)
        sqlite3_bind_text(stmt, idx++, block.deviceId.c_str(), block.deviceId.length(), SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, idx++, block.ptr.c_str(), block.ptr.length(), SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, idx++, block.size > INT64_MAX ? INT64_MAX : block.size);
        sqlite3_bind_int64(stmt, idx++, block.startTimestamp > INT64_MAX ? INT64_MAX : block.startTimestamp);
        sqlite3_bind_int64(stmt, idx++, block.endTimestamp > INT64_MAX ? INT64_MAX : block.endTimestamp);
        sqlite3_bind_text(stmt, idx++, block.otherAttr.c_str(), block.otherAttr.length(), SQLITE_TRANSIENT);
    }
    std::unique_lock<std::recursive_mutex> lock(mutex);
    auto result = sqlite3_step(stmt);
    if (blocklist.size() != cacheSize) {
        sqlite3_finalize(stmt);
    }
    if (result != SQLITE_DONE) {
        ServerLog::Error("Failed to insert block. Error: ", sqlite3_errmsg(db));
    }
}

void LeaksMemoryDatabase::InsertMemoryBlock(const Memory::MemoryBlock &block)
{
    std::lock_guard<std::mutex> lock(cacheMutex);
    blockCache.emplace_back(block);
    if (blockCache.size() == cacheSize) {
        InsertMemoryBlockList(blockCache);
        blockCache.clear();
    }
}

bool LeaksMemoryDatabase::InitStmt()
{
    if (hasInitStmt) {
        return true;
    }

    std::string insertAllocationsSql =
        "INSERT INTO " + memoryAllocationTable + " (timestamp, totalSize, optimized, deviceId) VALUES (?,?,?,?)";
    std::string insertBlocksSql = "INSERT INTO " + memoryBlockTable +
                                  " (deviceId, addr, size, startTimestamp, endTimestamp, attr) VALUES (?,?,?,?,?,?)";
    for (uint64_t i = 0; i < cacheSize - 1; ++i) {
        insertAllocationsSql.append(",(?,?,?,?)");
        insertBlocksSql.append(",(?,?,?,?,?,?)");
    }

    if (sqlite3_prepare_v2(db, insertAllocationsSql.c_str(), -1, &insertAllocationStmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare insert allocations statement. Error: ", sqlite3_errmsg(db));
        return false;
    }

    if (sqlite3_prepare_v2(db, insertBlocksSql.c_str(), -1, &insertBlockStmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare insert blocks statement. Error: ", sqlite3_errmsg(db));
        return false;
    }

    hasInitStmt = true;
    return true;
}

LeaksMemoryDatabase::~LeaksMemoryDatabase()
{
    if (hasInitStmt) {
        ReleaseStmt();
        hasInitStmt = false;
    }
    CloseDb();
}

void LeaksMemoryDatabase::ReleaseStmt()
{
    if (insertAllocationStmt != nullptr) {
        sqlite3_finalize(insertAllocationStmt);
        insertAllocationStmt = nullptr;
    }
    if (insertBlockStmt != nullptr) {
        sqlite3_finalize(insertBlockStmt);
        insertBlockStmt = nullptr;
    }
}

sqlite3_stmt *LeaksMemoryDatabase::GetInsertAllocationsStmt(uint64_t allocationsLen)
{
    sqlite3_stmt *stmt = nullptr;
    if (allocationsLen == 0) {
        return stmt;
    }
    if (allocationsLen == cacheSize) {
        if (!hasInitStmt) {
            InitStmt();
        }
        stmt = insertAllocationStmt;
        sqlite3_reset(stmt);
    } else {
        std::string insertAllocationsSql =
            "INSERT INTO " + memoryAllocationTable + " (timestamp, totalSize, optimized, deviceId) VALUES (?,?,?,?)";
        for (uint64_t i = 0; i < allocationsLen - 1; ++i) {
            insertAllocationsSql.append(",(?,?,?,?)");
        }

        if (sqlite3_prepare_v2(db, insertAllocationsSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            ServerLog::Error("Failed to prepare insert allocations statement. Error: ", sqlite3_errmsg(db));
            return nullptr;
        }
    }
    return stmt;
}

sqlite3_stmt *LeaksMemoryDatabase::GetInsertBlocksStmt(uint64_t blocksLen)
{
    sqlite3_stmt *stmt = nullptr;
    if (blocksLen == 0) {
        return stmt;
    }
    if (blocksLen == cacheSize) {
        if (!hasInitStmt) {
            InitStmt();
        }
        stmt = insertBlockStmt;
        sqlite3_reset(stmt);
    } else {
        std::string insertBlocksSql =
            "INSERT INTO " + memoryBlockTable +
            " (deviceId, addr, size, startTimestamp, endTimestamp, attr) VALUES (?,?,?,?,?,?)";
        for (uint64_t i = 0; i < blocksLen - 1; ++i) {
            insertBlocksSql.append(",(?,?,?,?,?,?)");
        }

        if (sqlite3_prepare_v2(db, insertBlocksSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            ServerLog::Error("Failed to prepare insert blocks statement. Error: ", sqlite3_errmsg(db));
            return nullptr;
        }
    }
    return stmt;
}

bool LeaksMemoryDatabase::CheckTablesExist()
{
    return Database::CheckTablesExist({memoryBlockTable, memoryAllocationTable, pythonTraceTable});
}

void LeaksMemoryDatabase::FlushMemoryBlocksCache()
{
    if (blockCache.size() > 0) {
        InsertMemoryBlockList(blockCache);
        blockCache.clear();
    }
}

void LeaksMemoryDatabase::FlushMemoryAllocationsCache()
{
    if (allocationCache.size() > 0) {
        InsertMemoryAllocationList(allocationCache);
        allocationCache.clear();
    }
}

bool LeaksMemoryDatabase::DropMemoryAllocationAndBlockTable()
{
    if (!isOpen) {
        ServerLog::Error("Failed to drop table. Database is not open.");
        return false;
    }
    std::string dropSql = "DROP TABLE IF EXISTS " + memoryAllocationTable + ";"
             "DROP TABLE IF EXISTS " + memoryBlockTable + ";";
    std::unique_lock<std::recursive_mutex> lock(mutex);
    return ExecSql(dropSql);
}

void LeaksMemoryDatabase::QueryMemoryBlocks(const LeaksMemoryBlockParams &queryParams,
                                            std::vector<Memory::MemoryBlock> &blocks)
{
    sqlite3_stmt *stmt;
    std::string querySql;
    uint64_t minTimestamp = 0;
    if (queryParams.relativeTime) {
        minTimestamp = QueryMemoryEventExtremumTimestamp(queryParams.deviceId, true);
    }

    querySql = "SELECT id, addr, size, "
               "(startTimestamp - " + std::to_string(minTimestamp) + ") AS startTimestamp,"
               "(endTimestamp - "+ std::to_string(minTimestamp) + ") AS endTimestamp "
               "FROM "+ memoryBlockTable + " where deviceId=? ";
    if (queryParams.endTimestamp > 0) {
        querySql += " AND (startTimestamp - " + std::to_string(minTimestamp) + ") >= " +
                std::to_string(queryParams.startTimestamp) + " AND "
                "(endTimestamp - "+ std::to_string(minTimestamp) + ") <= " +
                std::to_string(queryParams.endTimestamp);
    }
    if (queryParams.maxSize > 0) {
        querySql += " AND size >= " + std::to_string(queryParams.minSize) +
                " AND size <= " + std::to_string(queryParams.maxSize);
    }
    querySql += " ORDER BY startTimestamp ASC";
    int result = sqlite3_prepare_v2(db, querySql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, queryParams.deviceId.c_str(), queryParams.deviceId.length(), SQLITE_TRANSIENT);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query blocks table. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        Memory::MemoryBlock block;
        int64_t tmpInt64 = sqlite3_column_int64(stmt, col++);
        block.id = tmpInt64 < 0 ? 0 : tmpInt64;
        block.ptr = sqlite3_column_string(stmt, col++);
        tmpInt64 = sqlite3_column_int64(stmt, col++);
        block.size = tmpInt64 < 0 ? 0 : tmpInt64;
        tmpInt64 = sqlite3_column_int64(stmt, col++);
        block.startTimestamp = tmpInt64 < 0 ? 0 : tmpInt64;
        tmpInt64 = sqlite3_column_int64(stmt, col++);
        block.endTimestamp = tmpInt64 < 0 ? 0 : tmpInt64;
        block.otherAttr = sqlite3_column_string(stmt, col++);
        blocks.emplace_back(block);
    }
    sqlite3_finalize(stmt);
}

void LeaksMemoryDatabase::QueryMemoryAllocations(const LeaksMemoryAllocationParams &queryParams,
                                                 std::vector<Memory::MemoryAllocation> &allocations)
{
    sqlite3_stmt *stmt;
    std::string querySql;
    uint64_t minTimestamp = 0;
    if (queryParams.relativeTime) {
        minTimestamp = QueryMemoryEventExtremumTimestamp(queryParams.deviceId, true);
    }
    int optimized = queryParams.optimized ? 1 : 0;
    querySql = "SELECT id,"
               "(timestamp - " + std::to_string(minTimestamp) + ") AS timestamp, "
               "totalSize "
               "FROM "+ memoryAllocationTable + " WHERE deviceId=? AND optimized=? ";
    if (queryParams.endTimestamp > 0) {
        querySql += " AND (timestamp - " + std::to_string(minTimestamp) + ") >= " +
                    std::to_string(queryParams.startTimestamp) +
                    " AND (timestamp - "+ std::to_string(minTimestamp) + ") <= " +
                    std::to_string(queryParams.endTimestamp);
    }
    querySql += " ORDER BY timestamp ASC";
    int result = sqlite3_prepare_v2(db, querySql.c_str(), -1, &stmt, nullptr);
    int bindIdx = bindStartIndex;
    sqlite3_bind_text(stmt, bindIdx++, queryParams.deviceId.c_str(), queryParams.deviceId.length(), SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, bindIdx++, optimized);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query allocations table. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        Memory::MemoryAllocation allocation;
        int64_t tmpInt64 = sqlite3_column_int64(stmt, col++);
        allocation.id = tmpInt64 < 0 ? 0 : tmpInt64;
        tmpInt64 = sqlite3_column_int64(stmt, col++);
        allocation.timestamp = tmpInt64 < 0 ? 0 : tmpInt64;
        tmpInt64 = sqlite3_column_int64(stmt, col++);
        allocation.totalSize = tmpInt64 < 0 ? 0 : tmpInt64;
        allocations.emplace_back(allocation);
    }
    sqlite3_finalize(stmt);
}

uint64_t LeaksMemoryDatabase::QueryMemoryEventExtremumTimestamp(const std::string &deviceId, bool isMinimum)
{
    if (deviceId.empty()) {
        ServerLog::Error("Query extremum timestamp failed: deviceId is empty.");
        return false;
    }
    std::string sql;
    if (isMinimum) {
        sql = "select min(timestamp) from " + TABLE_LEAKS_DUMP + " where deviceId == ?;";
    } else {
        sql = "select max(timestamp) from " + TABLE_LEAKS_DUMP + " where deviceId == ?;";
    }
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, deviceId.c_str(), deviceId.length(), SQLITE_TRANSIENT);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query extremum timestamp failed. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return false;
    }
    uint64_t extremumTimestamp;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        int64_t resExtremumTs = sqlite3_column_int64(stmt, col++);
        extremumTimestamp = resExtremumTs > 0 ? resExtremumTs : 0;
    }
    sqlite3_finalize(stmt);
    return extremumTimestamp;
}

}  // FullDb
}  // Module
}  // Dic