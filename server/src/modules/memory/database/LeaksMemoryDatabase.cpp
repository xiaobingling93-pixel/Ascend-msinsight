/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include "ProtocolDefs.h"
#include "DataBaseManager.h"
#include "LeaksMemoryTableColumn.h"
#include "LeaksMemoryDatabase.h"

namespace Dic {
namespace Module {
namespace FullDb {
using namespace Dic::Server;
using namespace Dic::Module::Timeline;
using namespace Dic::Module::Memory;
namespace BLOCK = Dic::Module::Memory::MemoryBlockTableColumn;
namespace ALLOCATION = Dic::Module::Memory::MemoryAllocationTableColumn;
namespace EVENT = Dic::Module::Memory::MemoryEventTableColumn;
namespace TRACE = Dic::Module::Memory::MemoryPythonTraceTableColumn;
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
bool LeaksMemoryDatabase::OpenDb(const std::string &dbPath, bool clearAllTable)
{
    if (!Database::OpenDb(dbPath, clearAllTable)) {
        ServerLog::Error("[Leaks] Failed to open leaks memory db with path: %.", dbPath);
        return false;
    }
    if (!IsDatabaseVersionChange()) {
        return true;
    }
    ServerLog::Info("The database version has changed, the table structure and data will be reset.");
    // Create 方法中的建表Sql会在表存在时删除
    if (!CreateMemoryAllocationAndBlockTable()) {
        ServerLog::Error("[Leaks] Failed to drop and create allocation and block table.");
        CloseDb();
        return false;
    }
    if (!UpdateParseStatus(NOT_FINISH_STATUS)) {
        ServerLog::Warn("[Leaks] Failed to update leaks parse status.");
        CloseDb();
        return false;
    }
    return true;
}

std::string LeaksMemoryDatabase::GetCreateMemoryAllocationTableSql()
{
    std::string createSql = "DROP TABLE IF EXISTS {};"
                            "CREATE TABLE {} "
                            "("
                            "  {} INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
                            "  {} integer,"
                            "  {} integer,"
                            "  {} integer,"
                            "  {} text(255),"
                            "  {} text(255)"
                            ");";
    std::string errMsg;
    createSql = StringUtil::FormatSqlUsingPlaceHolder(createSql,
                                                      {memoryAllocationTable,
                                                       memoryAllocationTable,
                                                       std::string(ALLOCATION::ID),
                                                       std::string(ALLOCATION::TIMESTAMP),
                                                       std::string(ALLOCATION::TOTAL_SIZE),
                                                       std::string(ALLOCATION::OPTIMIZED),
                                                       std::string(ALLOCATION::DEVICE_ID),
                                                       std::string(ALLOCATION::EVENT_TYPE)},
                                                      errMsg);
    if (!errMsg.empty()) {
        ServerLog::Error("Failed to prepare sql for creating memory allocation table: ", errMsg);
        return "";
    }
    return createSql;
}

std::string LeaksMemoryDatabase::GetCreateMemoryBlockTableSql()
{
    std::string createSql = "DROP TABLE IF EXISTS {};"
                            "CREATE TABLE {} "
                            "("
                            "  {} INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
                            "  {} text(255),"
                            "  {} TEXT(255),"
                            "  {} integer,"
                            "  {} integer,"
                            "  {} integer,"
                            "  {} text(255),"
                            "  {} text(255),"
                            "  {} TEXT(255)"
                            ");";
    std::string errMsg;
    createSql = StringUtil::FormatSqlUsingPlaceHolder(createSql,
                                                      {memoryBlockTable,
                                                       memoryBlockTable,
                                                       std::string(BLOCK::ID),
                                                       std::string(BLOCK::DEVICE_ID),
                                                       std::string(BLOCK::ADDR),
                                                       std::string(BLOCK::SIZE),
                                                       std::string(BLOCK::START_TIMESTAMP),
                                                       std::string(BLOCK::END_TIMESTAMP),
                                                       std::string(BLOCK::EVENT_TYPE),
                                                       std::string(BLOCK::OWNER),
                                                       std::string(BLOCK::ATTR)},
                                                      errMsg);
    if (!errMsg.empty()) {
        ServerLog::Error("Failed to prepare sql for creating memory block table: ", errMsg);
        return "";
    }
    return createSql;
}

bool LeaksMemoryDatabase::CreateMemoryAllocationAndBlockTable()
{
    if (!isOpen) {
        ServerLog::Error("[LeaksMemory] Failed to create table memory_block/memory_allocation. Database is not open.");
        return false;
    }
    std::string createAllocationSql = GetCreateMemoryAllocationTableSql();
    if (createAllocationSql.empty()) {
        ServerLog::Error("[LeaksMemory] Failed to create table memory_allocation: build creation sql failed.");
        return false;
    }
    std::string createBlockSql = GetCreateMemoryBlockTableSql();
    if (createBlockSql.empty()) {
        ServerLog::Error("[LeaksMemory] Failed to create table memory_block: build creation sql failed.");
        return false;
    }
    std::unique_lock<std::recursive_mutex> lock(mutex);
    return ExecSql(createAllocationSql.append(createBlockSql));
}

bool LeaksMemoryDatabase::HasFinishedParseLastTime()
{
    return CheckValueFromStatusInfoTable(leaksMemoryParseStatus, FINISH_STATUS);
}

bool LeaksMemoryDatabase::UpdateParseStatus(const std::string &status)
{
    return UpdateValueIntoStatusInfoTable(leaksMemoryParseStatus, status);
}

bool LeaksMemoryDatabase::QueryMemoryEventsByStep(sqlite3_stmt* stmt, std::vector<Memory::MemoryEvent> &events)
{
    if (stmt == nullptr) {
        ServerLog::Error("Query memory events by step failed: stmt ptr is null.");
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
        events.emplace_back(event);
    }
    return true;
}

bool LeaksMemoryDatabase::QueryMemoryPythonTracesByStep(sqlite3_stmt *stmt,
                                                        LeaksMemoryPythonTrace &trace)
{
    if (stmt == nullptr) {
        ServerLog::Error("Query memory python trace by step failed: stmt ptr is null.");
        return false;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        PythonTraceSlice slice;
        slice.func = sqlite3_column_string(stmt, col++);
        int64_t tmpInt64 = sqlite3_column_int64(stmt, col++);
        if (tmpInt64 < 0) {
            ServerLog::Warn("Invalid timestamp for func info: ", slice.func);
            continue;
        }
        slice.startTimestamp = tmpInt64;
        tmpInt64 = sqlite3_column_int64(stmt, col++);
        if (tmpInt64 < 0) {
            ServerLog::Warn("Invalid timestamp for func info: ", slice.func);
            continue;
        }
        slice.endTimestamp = tmpInt64;
        trace.minTimestamp = std::min(trace.minTimestamp, slice.startTimestamp);
        trace.maxTimestamp = std::max(trace.maxTimestamp, slice.endTimestamp);
        trace.slices.push_back(slice);
    }
    return true;
}

bool LeaksMemoryDatabase::QueryEntireEventsTable(std::vector<Memory::MemoryEvent> &eventDetails)
{
    std::string sql = "select * from {} where {} not in ('N/A', '', 'host') order by {};";
    std::string errMsg;
    sql = StringUtil::FormatSqlUsingPlaceHolder(sql,
                                                {TABLE_LEAKS_DUMP,
                                                 std::string(EVENT::DEVICE_ID),
                                                 std::string(EVENT::TIMESTAMP)},
                                                 errMsg);
    if (!errMsg.empty() || sql.empty()) {
        ServerLog::Error("Query entire events table. Failed to format sql. Error: " + errMsg);
        return false;
    }
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query entire events table. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return false;
    }
    QueryMemoryEventsByStep(stmt, eventDetails);
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
        sqlite3_bind_int64(stmt, idx++, alloc.timestamp > INT64_MAX ? INT64_MAX : alloc.timestamp);
        sqlite3_bind_int64(stmt, idx++, alloc.totalSize > INT64_MAX ? INT64_MAX : alloc.totalSize);
        sqlite3_bind_int(stmt, idx++, alloc.optimized ? 1 : 0);
        sqlite3_bind_text(stmt, idx++, alloc.deviceId.c_str(), alloc.deviceId.length(), SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, idx++, alloc.eventType.c_str(), alloc.eventType.length(), SQLITE_TRANSIENT);
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
        sqlite3_bind_text(stmt, idx++, block.deviceId.c_str(), block.deviceId.length(), SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, idx++, block.ptr.c_str(), block.ptr.length(), SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, idx++, block.size > INT64_MAX ? INT64_MAX : block.size);
        sqlite3_bind_int64(stmt, idx++, block.startTimestamp > INT64_MAX ? INT64_MAX : block.startTimestamp);
        sqlite3_bind_int64(stmt, idx++, block.endTimestamp > INT64_MAX ? INT64_MAX : block.endTimestamp);
        sqlite3_bind_text(stmt, idx++, block.eventType.c_str(), block.eventType.length(), SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, idx++, block.owner.c_str(), block.owner.length(), SQLITE_TRANSIENT);
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
    // LCOV_EXCL_BR_START
    std::string insertAllocationsSql = "INSERT INTO " + memoryAllocationTable +
            " (timestamp, totalSize, optimized, deviceId, eventType) VALUES (?,?,?,?,?)";
    std::string insertBlocksSql = "INSERT INTO " + memoryBlockTable +
            "(deviceId, addr, size, startTimestamp, endTimestamp, eventType, owner, attr) VALUES (?,?,?,?,?,?,?,?)";
    for (uint64_t i = 0; i < cacheSize - 1; ++i) {
        insertAllocationsSql.append(",(?,?,?,?,?)");
        insertBlocksSql.append(",(?,?,?,?,?,?,?,?)");
    }
    // LCOV_EXCL_BR_STOP
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
        // LCOV_EXCL_BR_START
        std::string insertAllocationsSql =
            "INSERT INTO " + memoryAllocationTable +
            " (timestamp, totalSize, optimized, deviceId, eventType) VALUES (?,?,?,?,?)";
        for (uint64_t i = 0; i < allocationsLen - 1; ++i) {
            insertAllocationsSql.append(",(?,?,?,?,?)");
        }
        // LCOV_EXCL_BR_STOP
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
        // LCOV_EXCL_BR_START
        std::string insertBlocksSql =
            "INSERT INTO " + memoryBlockTable +
            " (deviceId, addr, size, startTimestamp, endTimestamp, eventType, owner, attr) VALUES (?,?,?,?,?,?,?,?)";
        for (uint64_t i = 0; i < blocksLen - 1; ++i) {
            insertBlocksSql.append(",(?,?,?,?,?,?,?,?)");
        }
        // LCOV_EXCL_BR_STOP
        if (sqlite3_prepare_v2(db, insertBlocksSql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            ServerLog::Error("Failed to prepare insert blocks statement. Error: ", sqlite3_errmsg(db));
            return nullptr;
        }
    }
    return stmt;
}

bool LeaksMemoryDatabase::CheckTablesExist()
{
    return Database::CheckTablesExist({memoryBlockTable, memoryAllocationTable, pythonTraceTablePrefix});
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
    // LCOV_EXCL_BR_START
    std::string dropSql = "DROP TABLE IF EXISTS " + memoryAllocationTable + ";"
             "DROP TABLE IF EXISTS " + memoryBlockTable + ";";
    // LCOV_EXCL_BR_STOP
    std::unique_lock<std::recursive_mutex> lock(mutex);
    return ExecSql(dropSql);
}

void LeaksMemoryDatabase::AppendMemoryBlockQueryConditionSqlByParams(const LeaksMemoryBlockParams &queryParams,
                                                                     std::string &querySql)
{
    uint64_t minTimestamp = 0;
    if (queryParams.relativeTime) {
        minTimestamp = QueryMemoryEventExtremumTimestamp(queryParams.deviceId, true);
    }
    std::string minTimestampStr = std::to_string(minTimestamp);
    std::string errMsg;
    if (queryParams.endTimestamp > 0) {
        querySql += " AND ({} - {}) >= {}  AND ({} - {}) <= {}";

        querySql = StringUtil::FormatSqlUsingPlaceHolder(querySql,
                                                         {std::string(BLOCK::START_TIMESTAMP),
                                                          minTimestampStr,
                                                          std::to_string(queryParams.startTimestamp),
                                                          std::string(BLOCK::END_TIMESTAMP),
                                                          minTimestampStr,
                                                          std::to_string(queryParams.endTimestamp)},
                                                         errMsg);
    }
    if (queryParams.maxSize > 0) {
        querySql += " AND {} >= {} AND {} <= {}";
        querySql = StringUtil::FormatSqlUsingPlaceHolder(querySql,
                                                         {std::string(BLOCK::SIZE),
                                                          std::to_string(queryParams.minSize),
                                                          std::string(BLOCK::SIZE),
                                                          std::to_string(queryParams.maxSize)},
                                                         errMsg);
    }
}

bool LeaksMemoryDatabase::QueryMemoryBlocksByStep(sqlite3_stmt* stmt, std::vector<Memory::MemoryBlock> &blocks)
{
    if (stmt == nullptr) {
        ServerLog::Error("Query memory blocks by step failed: stmt ptr is null.");
        return false;
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
        block.owner = sqlite3_column_string(stmt, col++);
        block.otherAttr = sqlite3_column_string(stmt, col++);
        blocks.emplace_back(block);
    }
    return true;
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
    // LCOV_EXCL_BR_START
    querySql = "SELECT {}, {}, {}, "
               "({} - {}) AS {},"
               "({} - {}) AS {},"
               "{}, {} "
               "FROM {} where {}=? AND {}=? ";
    std::string errMsg;
    std::string startTimestampStr = std::string(BLOCK::START_TIMESTAMP);
    std::string endTimestampStr = std::string(BLOCK::END_TIMESTAMP);
    std::string minTimestampStr = std::to_string(minTimestamp);
    querySql = StringUtil::FormatSqlUsingPlaceHolder(querySql,
                                                     {std::string(BLOCK::ID), std::string(BLOCK::ADDR),
                                                      std::string(BLOCK::SIZE), startTimestampStr,
                                                      minTimestampStr, startTimestampStr, endTimestampStr,
                                                      minTimestampStr, endTimestampStr, std::string(BLOCK::OWNER),
                                                      std::string(BLOCK::ATTR), memoryBlockTable,
                                                      std::string(BLOCK::DEVICE_ID), std::string(BLOCK::EVENT_TYPE)},
                                                     errMsg);
    AppendMemoryBlockQueryConditionSqlByParams(queryParams, querySql);
    querySql += " ORDER BY " + startTimestampStr +" ASC";
    // LCOV_EXCL_BR_STOP
    int result = sqlite3_prepare_v2(db, querySql.c_str(), -1, &stmt, nullptr);
    int bindIdx = bindStartIndex;
    sqlite3_bind_text(stmt, bindIdx++, queryParams.deviceId.c_str(), queryParams.deviceId.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, bindIdx++, queryParams.eventType.c_str(), queryParams.eventType.length(), SQLITE_TRANSIENT);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query blocks table. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return;
    }
    QueryMemoryBlocksByStep(stmt, blocks);
    sqlite3_finalize(stmt);
}

bool LeaksMemoryDatabase::QueryMemoryAllocationsByStep(sqlite3_stmt *stmt,
                                                       std::vector<Memory::MemoryAllocation> &allocations)
{
    if (stmt == nullptr) {
        ServerLog::Error("Query memory blocks by step failed: stmt ptr is null.");
        return false;
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
        tmpInt64 = sqlite3_column_int64(stmt, col++);
        allocation.optimized = tmpInt64 != 0;
        allocation.deviceId = sqlite3_column_string(stmt, col++);
        allocation.eventType = sqlite3_column_string(stmt, col++);
        allocations.emplace_back(allocation);
    }
    return true;
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
    std::string minTimestampStr = std::to_string(minTimestamp);
    std::string TIMESTAMP(ALLOCATION::TIMESTAMP);
    std::string DEVICE_ID(ALLOCATION::DEVICE_ID);
    std::string OPTIMIZED(ALLOCATION::OPTIMIZED);
    std::string EVENT_TYPE(ALLOCATION::EVENT_TYPE);
    // LCOV_EXCL_BR_START
    querySql = "SELECT {},"
               "({} - {}) AS {}, "
               "{}, {}, {}, {} "
               "FROM {} WHERE {}=? AND {}=? AND {}=? ";
    std::string errMsg;
    querySql = StringUtil::FormatSqlUsingPlaceHolder(querySql,
                                                     {std::string(ALLOCATION::ID),
                                                      TIMESTAMP, minTimestampStr, TIMESTAMP,
                                                      std::string(ALLOCATION::TOTAL_SIZE),
                                                      OPTIMIZED, DEVICE_ID, EVENT_TYPE,
                                                      memoryAllocationTable,
                                                      DEVICE_ID, OPTIMIZED, EVENT_TYPE,
                                                      },
                                                     errMsg);
    if (queryParams.endTimestamp > 0) {
        querySql.append(StringUtil::FormatSqlUsingPlaceHolder(" AND ({} - {}) >= {} AND ({} - {}) <= {} ",
                                                              {TIMESTAMP, minTimestampStr,
                                                               std::to_string(queryParams.startTimestamp),
                                                               TIMESTAMP, minTimestampStr,
                                                               std::to_string(queryParams.endTimestamp)},
                                                              errMsg));
    }
    querySql += " ORDER BY " + TIMESTAMP + " ASC";
    // LCOV_EXCL_BR_STOP
    int result = sqlite3_prepare_v2(db, querySql.c_str(), -1, &stmt, nullptr);
    int bindIdx = bindStartIndex;
    sqlite3_bind_text(stmt, bindIdx++, queryParams.deviceId.c_str(), queryParams.deviceId.length(), SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, bindIdx++, optimized);
    sqlite3_bind_text(stmt, bindIdx++, queryParams.eventType.c_str(), queryParams.eventType.length(), SQLITE_TRANSIENT);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query allocations table. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return;
    }
    QueryMemoryAllocationsByStep(stmt, allocations);
    sqlite3_finalize(stmt);
}

uint64_t LeaksMemoryDatabase::QueryMemoryEventExtremumTimestamp(const std::string &deviceId, bool isMinimum)
{
    if (deviceId.empty()) {
        ServerLog::Error("Query extremum timestamp failed: deviceId is empty.");
        return false;
    }
    std::string sql;
    std::string errMsg;
    sql = StringUtil::FormatSqlUsingPlaceHolder("select {}({}) from {} where {} == ?;",
                                                {isMinimum ? "min" : "max",
                                                 std::string(EVENT::TIMESTAMP),
                                                 TABLE_LEAKS_DUMP,
                                                 std::string(EVENT::DEVICE_ID)},
                                                errMsg);
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
void LeaksMemoryDatabase::QueryDeviceIds(std::set<std::string> &deviceIdSet)
{
    std::string sql;
    std::string errMsg;
    std::string COL_DEVICE_ID(EVENT::DEVICE_ID);
    sql = StringUtil::FormatSqlUsingPlaceHolder("SELECT DISTINCT({}) FROM {} WHERE {} NOT IN ('N/A', '', 'host')",
                                                {std::string(EVENT::DEVICE_ID), TABLE_LEAKS_DUMP,
                                                 COL_DEVICE_ID},
                                                errMsg);
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query events table. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        std::string deviceId = sqlite3_column_string(stmt, col++);
        deviceIdSet.insert(std::move(deviceId));
    }
    sqlite3_finalize(stmt);
}

using array_map = std::unordered_map<std::string, std::vector<std::string>>;
void LeaksMemoryDatabase::QueryMallocOrFreeEventTypeWithDeviceId(array_map &resultMap)
{
    std::string sql = "SELECT DISTINCT {}, {} FROM {} WHERE {} in ('MALLOC', 'FREE') AND {} NOT IN ('N/A', '', 'host')";
    std::string errMsg;
    sql = StringUtil::FormatSqlUsingPlaceHolder(sql,
                                                {std::string(EVENT::DEVICE_ID),
                                                 std::string(EVENT::EVENT_TYPE),
                                                 TABLE_LEAKS_DUMP,
                                                 std::string(EVENT::EVENT),
                                                 std::string(EVENT::DEVICE_ID)},
                                                errMsg);
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query events table. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        std::string deviceId = sqlite3_column_string(stmt, col++);
        std::string eventType = sqlite3_column_string(stmt, col++);
        resultMap[deviceId].push_back(eventType);
    }
    sqlite3_finalize(stmt);
}

void LeaksMemoryDatabase::QueryThreadIds(std::vector<uint64_t> &threadIds)
{
    std::vector<std::string> traceTables = GetPythonTraceTables();
    for (auto &traceTable : traceTables) {
        uint64_t processId =
                NumberUtil::StringToUnsignedLongLong(traceTable.substr(pythonTraceTablePrefix.size()));
        if (processId > 0) {
            QueryThreadIdsByProcessId(processId, threadIds);
        }
    }
}

bool LeaksMemoryDatabase::QueryEventsWithinTimeRangeByDeviceId(uint64_t startTimestamp,
                                                               uint64_t endTimestamp,
                                                               const std::string &deviceId,
                                                               std::vector<Memory::MemoryEvent> &events)
{
    if (startTimestamp > endTimestamp || startTimestamp >= INT64_MAX || endTimestamp >= INT64_MAX) {
        ServerLog::Error("Query events table: invalid time range");
        return false;
    }
    std::string sql;
    std::string errMsg;
    std::string COL_TIMESTAMP(EVENT::TIMESTAMP);
    std::string COL_DEVICE_ID(EVENT::DEVICE_ID);
    sql = StringUtil::FormatSqlUsingPlaceHolder("select * from {} where {} == ?  and {} >= ? and {} <= ? order by {}",
        {TABLE_LEAKS_DUMP, COL_DEVICE_ID, COL_TIMESTAMP, COL_TIMESTAMP, COL_TIMESTAMP}, errMsg);
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    int bindIdx = bindStartIndex;
    sqlite3_bind_text(stmt, bindIdx++, deviceId.c_str(), deviceId.length(), SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, bindIdx++, startTimestamp);
    sqlite3_bind_int64(stmt, bindIdx++, endTimestamp);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query events table. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return false;
    }
    QueryMemoryEventsByStep(stmt, events);
    sqlite3_finalize(stmt);
    return true;
}

std::optional<Memory::MemoryAllocation> LeaksMemoryDatabase::QueryLatestAllocationWithinTimestamp(
    const std::string &deviceId, const std::string &eventType, uint64_t timestamp)
{
    std::string sql;
    std::string errMsg;
    std::string COL_TIMESTAMP(ALLOCATION::TIMESTAMP);
    sql = StringUtil::FormatSqlUsingPlaceHolder("SELECT * FROM {} WHERE {} == ? AND {} == ? AND {} <= ? "
                                                "ORDER BY {} DESC LIMIT 1",
                                                {memoryAllocationTable,
                                                 std::string(ALLOCATION::DEVICE_ID),
                                                 std::string(ALLOCATION::EVENT_TYPE),
                                                 COL_TIMESTAMP, COL_TIMESTAMP},
                                                errMsg);
    sqlite3_stmt *stmt = nullptr;
    int bindIdx = bindStartIndex;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, bindIdx++, deviceId.c_str(), deviceId.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, bindIdx++, eventType.c_str(), eventType.length(), SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, bindIdx++, timestamp);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query allocation table. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return std::nullopt;
    }
    std::vector<Memory::MemoryAllocation> allocations;
    QueryMemoryAllocationsByStep(stmt, allocations);
    if (allocations.empty()) {
        ServerLog::Warn("Query allocation table. Failed to query latest allocation record: no data.");
        return std::nullopt;
    }
    sqlite3_finalize(stmt);
    return allocations[0];
}

void LeaksMemoryDatabase::QueryMemoryBlocksOwnersReleasedAfterTimestamp(const std::string &deviceId, uint64_t timestamp,
    std::set<std::string> &owners)
{
    std::string sql;
    std::string errMsg;
    sql = StringUtil::FormatSqlUsingPlaceHolder("SELECT DISTINCT {} FROM {} "
                                                "WHERE {} <= ? AND {} > ? AND {} == ?;",
                                                {std::string(BLOCK::OWNER), memoryBlockTable,
                                                 std::string(BLOCK::START_TIMESTAMP), std::string(BLOCK::END_TIMESTAMP),
                                                 std::string(BLOCK::DEVICE_ID)},
                                                errMsg);
    if (sql.empty()) {
        ServerLog::Error("[LeaksMemory] Failed to query block owners, error: ", errMsg);
        return;
    }
    sqlite3_stmt *stmt = nullptr;
    int bindIdx = bindStartIndex;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_int64(stmt, bindIdx++, timestamp);
    sqlite3_bind_int64(stmt, bindIdx++, timestamp);
    sqlite3_bind_text(stmt, bindIdx++, deviceId.c_str(), deviceId.length(), SQLITE_TRANSIENT);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query allocation table. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        std::string owner = sqlite3_column_string(stmt, col++);
        if (owner.empty()) {
            continue;
        }
        owners.insert(std::move(owner));
    }
    sqlite3_finalize(stmt);
}

uint64_t LeaksMemoryDatabase::QueryTotalSizeUtilTimestampUsingOwner(const std::string &deviceId,
                                                                    uint64_t timestamp,
                                                                    const std::string &owner)
{
    std::string sql;
    std::string errMsg;
    sql = StringUtil::FormatSqlUsingPlaceHolder("SELECT SUM({}) FROM {} "
                                                "WHERE {} <= ? AND {} > ? AND {} == ? AND {} like ?;",
                                                {std::string(BLOCK::SIZE), memoryBlockTable,
                                                 std::string(BLOCK::START_TIMESTAMP), std::string(BLOCK::END_TIMESTAMP),
                                                 std::string(BLOCK::DEVICE_ID), std::string(BLOCK::OWNER)},
                                                errMsg);
    if (sql.empty()) {
        ServerLog::Error("[LeaksMemory] Failed to query block total size, error: ", errMsg);
        return 0;
    }
    sqlite3_stmt *stmt = nullptr;
    int bindIdx = bindStartIndex;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_int64(stmt, bindIdx++, timestamp);
    sqlite3_bind_int64(stmt, bindIdx++, timestamp);
    sqlite3_bind_text(stmt, bindIdx++, deviceId.c_str(), deviceId.length(), SQLITE_TRANSIENT);
    std::string ownerPattern = owner + '%';
    sqlite3_bind_text(stmt, bindIdx++, ownerPattern.c_str(), ownerPattern.length(), SQLITE_TRANSIENT);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query allocation table. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return 0;
    }
    uint64_t totalSize = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        int64_t tmpInt64 = sqlite3_column_int64(stmt, col++);
        if (tmpInt64 < 0) {
            ServerLog::Error("The total size of blocks released before % is less than 0.", timestamp);
            totalSize = 0;
            break;
        }
        totalSize = tmpInt64;
    }
    sqlite3_finalize(stmt);
    return totalSize;
}
void BuildTimeConditionForQueryPythonTraces(std::string &timeCondition,
                                            const LeaksMemoryThreadPythonTraceParams &queryParams)
{
    if (queryParams.startTimestamp > 0) {
        timeCondition.append(" END <= ").append(std::to_string(queryParams.startTimestamp));
    }
    if (queryParams.endTimestamp > 0 && queryParams.endTimestamp > queryParams.startTimestamp) {
        if (!timeCondition.empty()) {
            timeCondition.append(" or ");
        }
        timeCondition.append(" START >= ").append(std::to_string(queryParams.endTimestamp));
    }
    if (timeCondition.empty()) {
        timeCondition = "0";
    }
}
void LeaksMemoryDatabase::QueryPythonTracesUsingTableName(const std::string &traceTableName,
                                                          const LeaksMemoryThreadPythonTraceParams &queryParams,
                                                          LeaksMemoryPythonTrace &trace)
{
    std::string errMsg;
    std::string COL_START_TIME(TRACE::START_TIME);
    std::string COL_END_TIME(TRACE::END_TIME);
    uint64_t minTimestamp = 0;
    if (queryParams.relativeTime) {
        minTimestamp = QueryMemoryEventExtremumTimestamp(queryParams.deviceId, true);
    }
    std::string timeCondition;
    BuildTimeConditionForQueryPythonTraces(timeCondition, queryParams);
    std::string sql = StringUtil::FormatSqlUsingPlaceHolder("SELECT {}, ({} - ?) AS START, ({} - ?) AS END FROM {} "
                                                            "WHERE NOT ({}) AND {} == ? AND END >= START "
                                                            "ORDER BY {}",
                                                            {std::string(TRACE::FUNC_INFO),
                                                             COL_START_TIME, COL_END_TIME, traceTableName,
                                                             timeCondition, std::string(TRACE::THREAD_ID),
                                                             COL_START_TIME},
                                                            errMsg);
    if (sql.empty()) {
        ServerLog::Error("[LeaksMemory] Failed to query python trace, error: ", errMsg);
        return;
    }
    sqlite3_stmt *stmt = nullptr;
    int bindIdx = 1;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_int64(stmt, bindIdx++, minTimestamp);
    sqlite3_bind_int64(stmt, bindIdx++, minTimestamp);
    sqlite3_bind_int64(stmt, bindIdx++, queryParams.threadId);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query python trace. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return;
    }
    QueryMemoryPythonTracesByStep(stmt, trace);
    sqlite3_finalize(stmt);
}

void LeaksMemoryDatabase::QueryPythonTrace(const LeaksMemoryThreadPythonTraceParams &queryParams,
    LeaksMemoryPythonTrace &trace)
{
    std::vector<std::string> traceTableNames = GetPythonTraceTables() ;
    if (traceTableNames.empty()) {
        ServerLog::Warn("Get python trace data. "
                         "There are no python trace tables found.");
        return;
    }

    for (auto &traceTableName : traceTableNames) {
        QueryPythonTracesUsingTableName(traceTableName, queryParams, trace);
        // 一张表中查到即可
        if (!trace.slices.empty()) {
            break;
        }
    }
}

std::vector<std::string> LeaksMemoryDatabase::GetPythonTraceTables()
{
    std::string sql = "SELECT name FROM sqlite_master WHERE type = 'table' AND name LIKE ?;";
    sqlite3_stmt *stmt = nullptr;
    int bindIdx = bindStartIndex;
    std::string pythonTraceTableNamePattern = pythonTraceTablePrefix + '%';
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    std::vector<std::string> resultList;
    sqlite3_bind_text(stmt, bindIdx++, pythonTraceTableNamePattern.c_str(),
                      pythonTraceTableNamePattern.length(), SQLITE_TRANSIENT);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query python trace table failed. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return {};
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        std::string table_name = sqlite3_column_string(stmt, col++);
        if (!table_name.empty()) {
            resultList.push_back(table_name);
        }
    }
    sqlite3_finalize(stmt);
    return resultList;
}

void LeaksMemoryDatabase::QueryThreadIdsByProcessId(uint64_t processId, std::vector<uint64_t> &threadIds)
{
    std::string pythonTraceTable = pythonTraceTablePrefix + std::to_string(processId);
    std::string sql = StringUtil::StrJoin("SELECT DISTINCT ", std::string(TRACE::THREAD_ID),
                                          " FROM ", pythonTraceTable);
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query python trace table failed. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        int64_t threadId = sqlite3_column_int64(stmt, col++);
        if (threadId > 0) {
            threadIds.push_back(threadId);
        }
    }
    sqlite3_finalize(stmt);
}
}  // FullDb
}  // Module
}  // Dic