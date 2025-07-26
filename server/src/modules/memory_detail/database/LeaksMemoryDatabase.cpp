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
namespace BLOCK = Dic::Module::MemoryDetail::MemoryBlockTableColumn;
namespace ALLOCATION = Dic::Module::MemoryDetail::MemoryAllocationTableColumn;
namespace EVENT = Dic::Module::MemoryDetail::MemoryEventTableColumn;
namespace TRACE = Dic::Module::MemoryDetail::MemoryPythonTraceTableColumn;
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
    createSql = StringUtil::FormatString(createSql,
                                         memoryAllocationTable,
                                         memoryAllocationTable,
                                         ALLOCATION::ID,
                                         ALLOCATION::TIMESTAMP,
                                         ALLOCATION::TOTAL_SIZE,
                                         ALLOCATION::OPTIMIZED,
                                         ALLOCATION::DEVICE_ID,
                                         ALLOCATION::EVENT_TYPE);
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
                            "  {} TEXT(255),"
                            "  {} integer,"
                            "  {} integer"
                            ");";
    createSql = StringUtil::FormatString(createSql,
                                         memoryBlockTable,
                                         memoryBlockTable,
                                         BLOCK::ID,
                                         BLOCK::DEVICE_ID,
                                         BLOCK::ADDR,
                                         BLOCK::SIZE,
                                         BLOCK::START_TIMESTAMP,
                                         BLOCK::END_TIMESTAMP,
                                         BLOCK::EVENT_TYPE,
                                         BLOCK::OWNER,
                                         BLOCK::ATTR,
                                         BLOCK::PROCESS_ID,
                                         BLOCK::THREAD_ID);
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

bool LeaksMemoryDatabase::QueryMemoryEventsByStep(sqlite3_stmt* stmt, std::vector<MemoryEvent> &events)
{
    if (stmt == nullptr) {
        ServerLog::Error("Query memory events by step failed: stmt ptr is null.");
        return false;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        MemoryEvent event{};
        event.id = NumberUtil::Int64ToUint64(sqlite3_column_int64(stmt, col++));
        event.event = sqlite3_column_string(stmt, col++);
        event.eventType = sqlite3_column_string(stmt, col++);
        event.name = sqlite3_column_string(stmt, col++);
        event.timestamp = NumberUtil::Int64ToUint64(sqlite3_column_int64(stmt, col++));
        event.processId = NumberUtil::Int64ToUint64(sqlite3_column_int64(stmt, col++));
        event.threadId = NumberUtil::Int64ToUint64(sqlite3_column_int64(stmt, col++));
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
        slice.startTimestamp = static_cast<uint64_t>(tmpInt64);
        tmpInt64 = sqlite3_column_int64(stmt, col++);
        if (tmpInt64 < 0) {
            ServerLog::Warn("Invalid timestamp for func info: ", slice.func);
            continue;
        }
        slice.endTimestamp = static_cast<uint64_t>(tmpInt64);
        trace.minTimestamp = std::min(trace.minTimestamp, slice.startTimestamp);
        trace.maxTimestamp = std::max(trace.maxTimestamp, slice.endTimestamp);
        trace.slices.push_back(slice);
    }
    return true;
}

bool LeaksMemoryDatabase::QueryEntireEventsTable(std::vector<MemoryEvent> &eventDetails)
{
    std::string sql = "select * from {} where {} not in ('N/A', '', 'host') order by {};";
    sql = StringUtil::FormatString(sql, TABLE_LEAKS_DUMP,
                                   EVENT::DEVICE_ID,
                                   EVENT::TIMESTAMP);
    if (sql.empty()) {
        ServerLog::Error("Query entire events table. Failed to format sql.");
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

void LeaksMemoryDatabase::InsertMemoryAllocationList(const std::vector<MemoryAllocation> &allocList)
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

void LeaksMemoryDatabase::InsertMemoryAllocation(const MemoryAllocation &alloc)
{
    std::lock_guard<std::mutex> lock(cacheMutex);
    allocationCache.emplace_back(alloc);
    if (allocationCache.size() == cacheSize) {
        InsertMemoryAllocationList(allocationCache);
        allocationCache.clear();
    }
}

void LeaksMemoryDatabase::InsertMemoryBlockList(const std::vector<MemoryBlock> &blocklist)
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
        sqlite3_bind_int64(stmt, idx++, block.processId > INT64_MAX ? INT64_MAX : block.processId);
        sqlite3_bind_int64(stmt, idx++, block.threadId > INT64_MAX ? INT64_MAX : block.threadId);
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

void LeaksMemoryDatabase::InsertMemoryBlock(const MemoryBlock &block)
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
    std::string insertAllocationsSql = "INSERT INTO " + memoryAllocationTable +
            "(" + allocationColumnPattern +") VALUES (" + allocationValuePattern +")";
    std::string insertBlocksSql = "INSERT INTO " + memoryBlockTable +
            "("+ blockColumnPattern +") VALUES (" + blockValuePattern + ")";
    for (uint64_t i = 0; i < cacheSize - 1; ++i) {
        insertAllocationsSql.append(",("+allocationValuePattern+")");
        insertBlocksSql.append(",("+blockValuePattern+")");
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
        std::string insertAllocationsSql = "INSERT INTO " + memoryAllocationTable +
                                           "(" + allocationColumnPattern +") VALUES (" + allocationValuePattern +")";
        for (uint64_t i = 0; i < allocationsLen - 1; ++i) {
            insertAllocationsSql.append(",("+allocationValuePattern+")");
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
        std::string insertBlocksSql = "INSERT INTO " + memoryBlockTable +
                                      "("+ blockColumnPattern +") VALUES (" + blockValuePattern + ")";
        for (uint64_t i = 0; i < blocksLen - 1; ++i) {
            insertBlocksSql.append(",("+blockValuePattern+")");
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
    std::string dropSql = "DROP TABLE IF EXISTS " + memoryAllocationTable + ";"
             "DROP TABLE IF EXISTS " + memoryBlockTable + ";";
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
        querySql += " AND ((({} - {}) BETWEEN {} AND {}) OR (({} - {}) BETWEEN {} AND {}))";

        querySql = StringUtil::FormatString(querySql, BLOCK::START_TIMESTAMP, minTimestampStr,
                                            std::to_string(queryParams.startTimestamp), std::to_string(queryParams.endTimestamp),
                                            BLOCK::END_TIMESTAMP, minTimestampStr,
                                            std::to_string(queryParams.startTimestamp), std::to_string(queryParams.endTimestamp));
    }
    if (queryParams.maxSize > 0) {
        querySql += " AND {} >= {} AND {} <= {}";
        querySql = StringUtil::FormatString(querySql, BLOCK::SIZE, std::to_string(queryParams.minSize),
                                            BLOCK::SIZE, std::to_string(queryParams.maxSize));
    }
}

bool LeaksMemoryDatabase::QueryMemoryBlocksByStep(sqlite3_stmt* stmt, std::vector<MemoryBlock> &blocks)
{
    if (stmt == nullptr) {
        ServerLog::Error("Query memory blocks by step failed: stmt ptr is null.");
        return false;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        MemoryBlock block;
        block.id = NumberUtil::Int64ToUint64(sqlite3_column_int64(stmt, col++));
        block.ptr = sqlite3_column_string(stmt, col++);
        block.size = NumberUtil::Int64ToUint64(sqlite3_column_int64(stmt, col++));
        block.startTimestamp = NumberUtil::Int64ToUint64(sqlite3_column_int64(stmt, col++));
        block.endTimestamp = NumberUtil::Int64ToUint64(sqlite3_column_int64(stmt, col++));
        block.owner = sqlite3_column_string(stmt, col++);
        block.otherAttr = sqlite3_column_string(stmt, col++);
        block.processId = NumberUtil::Int64ToUint64(sqlite3_column_int64(stmt, col++));
        block.threadId = NumberUtil::Int64ToUint64(sqlite3_column_int64(stmt, col++));
        blocks.emplace_back(block);
    }
    return true;
}
void LeaksMemoryDatabase::QueryMemoryBlocks(const LeaksMemoryBlockParams &queryParams,
                                            std::vector<MemoryBlock> &blocks)
{
    sqlite3_stmt *stmt;
    std::string querySql;
    uint64_t minTimestamp = 0;
    if (queryParams.relativeTime) {
        minTimestamp = QueryMemoryEventExtremumTimestamp(queryParams.deviceId, true);
    }
    querySql = "SELECT {}, {}, {}, "
               "({} - {}) AS {},"
               "({} - {}) AS {},"
               "{}, {}, {}, {} "
               "FROM {} where {}=? AND {}=? ";
    std::string errMsg;
    std::string minTimestampStr = std::to_string(minTimestamp);
    querySql = StringUtil::FormatString(querySql,
                                        BLOCK::ID, BLOCK::ADDR,
                                        BLOCK::SIZE, BLOCK::START_TIMESTAMP,
                                        minTimestampStr, BLOCK::START_TIMESTAMP, BLOCK::END_TIMESTAMP,
                                        minTimestampStr, BLOCK::END_TIMESTAMP, BLOCK::OWNER,
                                        BLOCK::ATTR, BLOCK::PROCESS_ID, BLOCK::THREAD_ID,
                                        memoryBlockTable, BLOCK::DEVICE_ID, BLOCK::EVENT_TYPE);
    AppendMemoryBlockQueryConditionSqlByParams(queryParams, querySql);
    querySql += " ORDER BY " + std::string(BLOCK::START_TIMESTAMP) + " ASC";
    int result = sqlite3_prepare_v2(db, querySql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query blocks table. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return;
    }
    int bindIdx = bindStartIndex;
    sqlite3_bind_text(stmt, bindIdx++, queryParams.deviceId.c_str(), queryParams.deviceId.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, bindIdx++, queryParams.eventType.c_str(), queryParams.eventType.length(), SQLITE_TRANSIENT);
    QueryMemoryBlocksByStep(stmt, blocks);
    sqlite3_finalize(stmt);
}

bool LeaksMemoryDatabase::QueryMemoryAllocationsByStep(sqlite3_stmt *stmt,
                                                       std::vector<MemoryAllocation> &allocations)
{
    if (stmt == nullptr) {
        ServerLog::Error("Query memory blocks by step failed: stmt ptr is null.");
        return false;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        MemoryAllocation allocation;
        allocation.id = NumberUtil::Int64ToUint64(sqlite3_column_int64(stmt, col++));
        allocation.timestamp = NumberUtil::Int64ToUint64(sqlite3_column_int64(stmt, col++));
        allocation.totalSize = NumberUtil::Int64ToUint64(sqlite3_column_int64(stmt, col++));
        allocation.optimized = sqlite3_column_int64(stmt, col++) != 0;
        allocation.deviceId = sqlite3_column_string(stmt, col++);
        allocation.eventType = sqlite3_column_string(stmt, col++);
        allocations.emplace_back(allocation);
    }
    return true;
}

void LeaksMemoryDatabase::QueryMemoryAllocations(const LeaksMemoryAllocationParams &queryParams,
                                                 std::vector<MemoryAllocation> &allocations)
{
    sqlite3_stmt *stmt;
    std::string querySql;
    uint64_t minTimestamp = 0;
    if (queryParams.relativeTime) {
        minTimestamp = QueryMemoryEventExtremumTimestamp(queryParams.deviceId, true);
    }
    int optimized = queryParams.optimized ? 1 : 0;
    std::string minTimestampStr = std::to_string(minTimestamp);
    querySql = "SELECT {},"
               "({} - {}) AS {}, "
               "{}, {}, {}, {} "
               "FROM {} WHERE {}=? AND {}=? AND {}=? ";
    std::string errMsg;
    querySql = StringUtil::FormatString(querySql,
                                        ALLOCATION::ID,
                                        ALLOCATION::TIMESTAMP, minTimestampStr, ALLOCATION::TIMESTAMP,
                                        ALLOCATION::TOTAL_SIZE,
                                        ALLOCATION::OPTIMIZED, ALLOCATION::DEVICE_ID, ALLOCATION::EVENT_TYPE,
                                        memoryAllocationTable,
                                        ALLOCATION::OPTIMIZED, ALLOCATION::DEVICE_ID, ALLOCATION::EVENT_TYPE);
    if (queryParams.endTimestamp > 0) {
        querySql.append(StringUtil::FormatString(" AND ({} - {}) >= {} AND ({} - {}) <= {} ",
                                                 ALLOCATION::TIMESTAMP, minTimestampStr,
                                                 std::to_string(queryParams.startTimestamp),
                                                 ALLOCATION::TIMESTAMP, minTimestampStr,
                                                 std::to_string(queryParams.endTimestamp)));
    }
    querySql += " ORDER BY " + std::string(ALLOCATION::TIMESTAMP) + " ASC";
    int result = sqlite3_prepare_v2(db, querySql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query allocations table. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return;
    }
    int bindIdx = bindStartIndex;
    sqlite3_bind_int(stmt, bindIdx++, optimized);
    sqlite3_bind_text(stmt, bindIdx++, queryParams.deviceId.c_str(), queryParams.deviceId.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, bindIdx++, queryParams.eventType.c_str(), queryParams.eventType.length(), SQLITE_TRANSIENT);
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
    sql = StringUtil::FormatString("select {}({}) from {} where {} == ?;",
                                   isMinimum ? "min" : "max",
                                   EVENT::TIMESTAMP,
                                   TABLE_LEAKS_DUMP,
                                   EVENT::DEVICE_ID);
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query extremum timestamp failed. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return false;
    }
    sqlite3_bind_text(stmt, 1, deviceId.c_str(), deviceId.length(), SQLITE_TRANSIENT);
    uint64_t extremumTimestamp;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        extremumTimestamp = NumberUtil::Int64ToUint64(sqlite3_column_int64(stmt, col++));
    }
    sqlite3_finalize(stmt);
    return extremumTimestamp;
}
void LeaksMemoryDatabase::QueryDeviceIds(std::set<std::string> &deviceIdSet)
{
    std::string sql;
    std::string errMsg;
    sql = StringUtil::FormatString("SELECT DISTINCT({}) FROM {} WHERE {} NOT IN ('N/A', '', 'host')",
                                   EVENT::DEVICE_ID, TABLE_LEAKS_DUMP, EVENT::DEVICE_ID);
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
    sql = StringUtil::FormatString(sql, EVENT::DEVICE_ID, EVENT::EVENT_TYPE, TABLE_LEAKS_DUMP,
                                   EVENT::EVENT, EVENT::DEVICE_ID);
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
                                                               std::vector<MemoryEvent> &events)
{
    if (startTimestamp > endTimestamp || startTimestamp >= INT64_MAX || endTimestamp >= INT64_MAX) {
        ServerLog::Error("Query events table: invalid time range");
        return false;
    }
    std::string sql;
    std::string errMsg;
    sql = StringUtil::FormatString("select * from {} where {} == ?  and {} >= ? and {} <= ? order by {}",
        TABLE_LEAKS_DUMP, EVENT::DEVICE_ID, EVENT::TIMESTAMP, EVENT::TIMESTAMP, EVENT::TIMESTAMP);
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query events table. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return false;
    }
    int bindIdx = bindStartIndex;
    sqlite3_bind_text(stmt, bindIdx++, deviceId.c_str(), deviceId.length(), SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, bindIdx++, startTimestamp);
    sqlite3_bind_int64(stmt, bindIdx++, endTimestamp);
    QueryMemoryEventsByStep(stmt, events);
    sqlite3_finalize(stmt);
    return true;
}

std::optional<MemoryAllocation> LeaksMemoryDatabase::QueryLatestAllocationWithinTimestamp(
    const std::string &deviceId, const std::string &eventType, uint64_t timestamp)
{
    std::string sql;
    std::string errMsg;
    sql = StringUtil::FormatString("SELECT * FROM {} WHERE {} == ? AND {} == ? AND {} <= ? "
                                   "ORDER BY {} DESC LIMIT 1", memoryAllocationTable,
                                   ALLOCATION::DEVICE_ID, ALLOCATION::EVENT_TYPE,
                                   ALLOCATION::TIMESTAMP, ALLOCATION::TIMESTAMP);
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query allocation table. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return std::nullopt;
    }
    int bindIdx = bindStartIndex;
    sqlite3_bind_text(stmt, bindIdx++, deviceId.c_str(), deviceId.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, bindIdx++, eventType.c_str(), eventType.length(), SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, bindIdx++, timestamp > INT64_MAX ? INT64_MAX : timestamp);
    std::vector<MemoryAllocation> allocations;
    QueryMemoryAllocationsByStep(stmt, allocations);
    if (allocations.empty()) {
        ServerLog::Warn("Query allocation table. Failed to query latest allocation record: no data.");
        sqlite3_finalize(stmt);
        return std::nullopt;
    }
    sqlite3_finalize(stmt);
    return allocations[0];
}

std::optional<MemoryAllocation> LeaksMemoryDatabase::QueryNextAllocationAfterTimestamp(const std::string &deviceId,
                                                                                       const std::string &eventType,
                                                                                       uint64_t timestamp)
{
    std::string sql;
    std::string errMsg;
    sql = StringUtil::FormatString("SELECT * FROM {} WHERE {} == ? AND {} == ? AND {} >= ? ORDER BY {} ASC LIMIT 1",
                                   memoryAllocationTable, ALLOCATION::DEVICE_ID, ALLOCATION::EVENT_TYPE,
                                   ALLOCATION::TIMESTAMP, ALLOCATION::TIMESTAMP);
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query allocation table. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return std::nullopt;
    }
    int bindIdx = bindStartIndex;
    sqlite3_bind_text(stmt, bindIdx++, deviceId.c_str(), deviceId.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, bindIdx++, eventType.c_str(), eventType.length(), SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, bindIdx++, timestamp > INT64_MAX ? INT64_MAX : timestamp);
    std::vector<MemoryAllocation> allocations;
    QueryMemoryAllocationsByStep(stmt, allocations);
    if (allocations.empty()) {
        ServerLog::Warn("Query allocation table. Failed to query latest allocation record: no data.");
        sqlite3_finalize(stmt);
        return std::nullopt;
    }
    sqlite3_finalize(stmt);
    return allocations[0];
}

void LeaksMemoryDatabase::QueryMemoryBlocksOwnersReleasedAfterTimestamp(const std::string &deviceId,
                                                                        const std::string &eventType,
                                                                        uint64_t timestamp,
                                                                        std::set<std::string> &owners)
{
    std::string sql;
    std::string errMsg;
    sql = StringUtil::FormatString("SELECT DISTINCT {} FROM {} "
                                   "WHERE {} <= ? AND {} > ? AND {} == ? AND {} == ?;",
                                   BLOCK::OWNER, memoryBlockTable,
                                   BLOCK::START_TIMESTAMP, BLOCK::END_TIMESTAMP,
                                   BLOCK::DEVICE_ID, BLOCK::EVENT_TYPE);
    if (sql.empty()) {
        ServerLog::Error("[LeaksMemory] Failed to query block owners, error: ", errMsg);
        return;
    }
    if (timestamp > INT64_MAX) {
        ServerLog::Warn("Invalid timestamp: exceeds the limit of ", INT64_MAX);
        timestamp = INT64_MAX;
    }
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query allocation table. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return;
    }
    int bindIdx = bindStartIndex;
    sqlite3_bind_int64(stmt, bindIdx++, timestamp);
    sqlite3_bind_int64(stmt, bindIdx++, timestamp);
    sqlite3_bind_text(stmt, bindIdx++, deviceId.c_str(), deviceId.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, bindIdx++, eventType.c_str(), eventType.length(), SQLITE_TRANSIENT);
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

uint64_t LeaksMemoryDatabase::QueryTotalSizeUntilTimestampUsingOwner(const std::string &deviceId,
                                                                     uint64_t timestamp,
                                                                     const std::string &owner)
{
    std::string sql;
    std::string errMsg;
    sql = StringUtil::FormatString("SELECT SUM({}) FROM {} "
                                   "WHERE {} <= ? AND {} > ? AND {} == ? AND {} like ?;",
                                   BLOCK::SIZE, memoryBlockTable,
                                   BLOCK::START_TIMESTAMP, BLOCK::END_TIMESTAMP,
                                   BLOCK::DEVICE_ID, BLOCK::OWNER);
    if (sql.empty()) {
        ServerLog::Error("[LeaksMemory] Failed to query block total size, error: ", errMsg);
        return 0;
    }
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query allocation table. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return 0;
    }
    int bindIdx = bindStartIndex;
    sqlite3_bind_int64(stmt, bindIdx++, timestamp);
    sqlite3_bind_int64(stmt, bindIdx++, timestamp);
    sqlite3_bind_text(stmt, bindIdx++, deviceId.c_str(), deviceId.length(), SQLITE_TRANSIENT);
    std::string ownerPattern = owner + '%';
    sqlite3_bind_text(stmt, bindIdx++, ownerPattern.c_str(), ownerPattern.length(), SQLITE_TRANSIENT);
    uint64_t totalSize = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        totalSize = NumberUtil::Int64ToUint64(sqlite3_column_int64(stmt, col++));
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
    std::string sql = StringUtil::FormatString("SELECT {}, ({} - ?) AS START, ({} - ?) AS END FROM {} "
                                               "WHERE NOT ({}) AND {} == ? AND END >= START "
                                               "ORDER BY {}", TRACE::FUNC_INFO, COL_START_TIME,
                                               COL_END_TIME, traceTableName, timeCondition,
                                               TRACE::THREAD_ID, COL_START_TIME);
    if (sql.empty()) {
        ServerLog::Error("[LeaksMemory] Failed to query python trace, error: ", errMsg);
        return;
    }
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query python trace. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return;
    }
    int bindIdx = bindStartIndex;
    sqlite3_bind_int64(stmt, bindIdx++, minTimestamp);
    sqlite3_bind_int64(stmt, bindIdx++, minTimestamp);
    sqlite3_bind_int64(stmt, bindIdx++, queryParams.threadId);
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
    std::string pythonTraceTableNamePattern = pythonTraceTablePrefix + '%';
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query python trace table failed. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return {};
    }
    std::vector<std::string> resultList;
    int bindIdx = bindStartIndex;
    sqlite3_bind_text(stmt, bindIdx++, pythonTraceTableNamePattern.c_str(),
                      pythonTraceTableNamePattern.length(), SQLITE_TRANSIENT);
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
            threadIds.push_back(static_cast<uint64_t>(threadId));
        }
    }
    sqlite3_finalize(stmt);
}
}  // FullDb
}  // Module
}  // Dic