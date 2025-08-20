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
    ServerLog::Info("[Leaks] Leaks db reset.");
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
    // 查看CallStack列是否存在
    if (!SetCallStackExistsFlagByCheckColumn()) {
        ServerLog::Error("[Leaks] Failed to check callstack columns. Db error.");
        CloseDb();
        return false;
    }
    // 为python_trace表添加depth列(如果不存在)
    if (!AppendDepthColumnForPythonTraceTables()) {
        ServerLog::Error("[Leaks] Failed to add column depth for python trace table.");
        CloseDb();
        return false;
    }
    // 设置db的全局最大最小时间
    if (!QueryAndSetGlobalExtremumTimestamp() || !CheckGlobalExtremumTimestampValid()) {
        ServerLog::Error("[Leaks] Failed to query and set global extremum timestamp or timestamp invalid.");
        CloseDb();
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
                            "  {} integer,"
                            "  {} integer,"
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
                                         BLOCK::THREAD_ID,
                                         BLOCK::FIRST_ACCESS_TIMESTAMP,
                                         BLOCK::LAST_ACCESS_TIMESTAMP,
                                         BLOCK::MAX_ACCESS_INTERVAL);
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

// 当withExtraCountCol = true时, 要求sql必须将COUNT(*) OVER()作为首列
int64_t LeaksMemoryDatabase::QueryMemoryEventsByStep(sqlite3_stmt* stmt, std::vector<MemoryEvent>& events,
                                                     const bool withExtraCountCol)
{
    if (stmt == nullptr) {
        ServerLog::Error("Query memory events by step failed: stmt ptr is null.");
        return -1;
    }
    int64_t count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        MemoryEvent event{};
        if (withExtraCountCol) {
            count = sqlite3_column_int64(stmt, col++);
        }
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
        if (withCallStackPython) {
            event.callStackPython = sqlite3_column_string(stmt, col++);
        }
        if (withCallStackC) {
            event.callStackC = sqlite3_column_string(stmt, col++);
        }
        events.emplace_back(event);
    }
    return count;
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
        slice.id = sqlite3_column_int64(stmt, col++);
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
        slice.threadId = NumberUtil::Int64ToUint64(sqlite3_column_int64(stmt, col++));
        slice.processId = NumberUtil::Int64ToUint64(sqlite3_column_int64(stmt, col++));
        slice.depth = sqlite3_column_int(stmt, col++);
        trace.minTimestamp = std::min(trace.minTimestamp, slice.startTimestamp);
        trace.maxTimestamp = std::max(trace.maxTimestamp, slice.endTimestamp);
        trace.maxDepth = std::max(trace.maxDepth, slice.depth);
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
    // 取绝对时间
    QueryMemoryEventsByStep(stmt, eventDetails, false);
    sqlite3_finalize(stmt);
    return true;
}

void LeaksMemoryDatabase::UpdatePythonTraceSliceList(const std::vector<PythonTraceSlice> &slices, uint64_t processId)
{
    sqlite3_stmt *stmt = GetUpdatePythonTraceSliceStmt(processId);
    if (stmt == nullptr) {
        ServerLog::Error("Failed to get update slice stmt.");
        return;
    }
    std::unique_lock<std::recursive_mutex> lock(mutex);
    // 开始事务
    if (!ExecSql("BEGIN TRANSACTION;")) {
        ServerLog::Error("An error occurred while begin the transaction: ", sqlite3_errmsg(db));
        return;
    }
    for (const auto &slice : slices) {
        int idx = bindStartIndex;
        sqlite3_bind_int64(stmt, idx++, slice.depth);
        sqlite3_bind_int64(stmt, idx++, slice.id);
        auto result  = sqlite3_step(stmt);
        if (result != SQLITE_DONE) {
            ServerLog::Error("Failed to update slice. slice id %, Error: %", slice.id, sqlite3_errmsg(db));
            // 回滚事务
            if (!ExecSql("ROLLBACK;")) {
                ServerLog::Error("An error occurred while rolling back the transaction: ", sqlite3_errmsg(db));
            }
            return;
        }
        sqlite3_reset(stmt);
    }
    // 提交事务
    if (!ExecSql("COMMIT;")) {
        ServerLog::Error("An error occurred while commit the transaction: ", sqlite3_errmsg(db));
    }
}

void LeaksMemoryDatabase::UpdatePythonTraceSlice(const PythonTraceSlice& slice)
{
    auto it = slicePidCache.find(slice.processId);
    if (it == slicePidCache.end()) {
        ServerLog::Warn("Cannot update python trace slice, process id not found");
        return;
    }
    auto &sliceCache = it->second;
    std::unique_lock<std::mutex> lock(cacheMutex);
    sliceCache.emplace_back(slice);
    if (sliceCache.size() == cacheSize) {
        auto tempCache = std::move(sliceCache);
        it->second = std::vector<PythonTraceSlice>(cacheSize);
        lock.unlock();
        UpdatePythonTraceSliceList(tempCache, slice.processId);
    }
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
        sqlite3_bind_text(stmt, idx++, block.attrJsonString.c_str(), block.attrJsonString.length(), SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, idx++, block.processId > INT64_MAX ? INT64_MAX : block.processId);
        sqlite3_bind_int64(stmt, idx++, block.threadId > INT64_MAX ? INT64_MAX : block.threadId);
        sqlite3_bind_int64(stmt, idx++, block.firstAccessTimestamp > INT64_MAX ? INT64_MAX : block.firstAccessTimestamp);
        sqlite3_bind_int64(stmt, idx++, block.lastAccessTimestamp > INT64_MAX ? INT64_MAX : block.lastAccessTimestamp);
        sqlite3_bind_int64(stmt, idx++, block.maxAccessInterval > INT64_MAX ? INT64_MAX : block.maxAccessInterval);
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

    auto pythonTraceTables = GetPythonTraceTables();
    for (auto &tableName : pythonTraceTables) {
        uint64_t processId = GetProcessIdByPythonTraceTableName(tableName);
        if (processId == 0) {
            ServerLog::Warn("Cannot get processId from table: ", tableName);
            continue;
        }
        std::string setDepthSql = StringUtil::FormatString("UPDATE {} SET {} = ? WHERE ROWID = ?;",
                                                           StringUtil::StrJoin(pythonTraceTablePrefix, std::to_string(processId)),
                                                           TRACE_TABLE::DEPTH);
        updatePythonTraceDepthStmtPidMap[processId] = nullptr;
        slicePidCache[processId] = std::vector<PythonTraceSlice>(cacheSize);
        if (sqlite3_prepare_v2(db, setDepthSql.c_str(), -1, &updatePythonTraceDepthStmtPidMap[processId], nullptr)
            != SQLITE_OK) {
            ServerLog::Error("Failed to prepare update python trace depth statement. Error: ", sqlite3_errmsg(db));
            return false;
        }
    }

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
    for (auto& [processId, stmt] : updatePythonTraceDepthStmtPidMap) {
        if (stmt != nullptr) {
            sqlite3_finalize(stmt);
        }
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

sqlite3_stmt* LeaksMemoryDatabase::GetUpdatePythonTraceSliceStmt(uint64_t processId)
{
    sqlite3_stmt *stmt = nullptr;
    auto it = updatePythonTraceDepthStmtPidMap.find(processId);
    if (it == updatePythonTraceDepthStmtPidMap.end()) {
        return stmt;
    }
    if (!hasInitStmt) {
        InitStmt();
    }
    stmt = it->second;
    if (stmt != nullptr) {
        sqlite3_reset(stmt);
    }
    return stmt;
}

sqlite3_stmt* LeaksMemoryDatabase::GetInsertBlocksStmt(uint64_t blocksLen)
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

// 当withExtraCountCol = true时, 要求sql必须将COUNT(*) OVER()作为首列
int64_t LeaksMemoryDatabase::QueryMemoryBlocksByStep(sqlite3_stmt* stmt, std::vector<MemoryBlock>& blocks,
                                                     const bool withExtraCountCol)
{
    if (stmt == nullptr) {
        ServerLog::Error("Query memory blocks by step failed: stmt ptr is null.");
        return -1;
    }
    int64_t count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        MemoryBlock block;
        if (withExtraCountCol) {
            count = sqlite3_column_int64(stmt, col++);
        }
        block.id = NumberUtil::Int64ToUint64(sqlite3_column_int64(stmt, col++));
        block.deviceId = sqlite3_column_string(stmt, col++);
        block.ptr = sqlite3_column_string(stmt, col++);
        block.size = NumberUtil::Int64ToUint64(sqlite3_column_int64(stmt, col++));
        block.startTimestamp = NumberUtil::Int64ToUint64(sqlite3_column_int64(stmt, col++));
        block.endTimestamp = NumberUtil::Int64ToUint64(sqlite3_column_int64(stmt, col++));
        block.eventType = sqlite3_column_string(stmt, col++);
        block.owner = sqlite3_column_string(stmt, col++);
        block.processId = NumberUtil::Int64ToUint64(sqlite3_column_int64(stmt, col++));
        block.threadId = NumberUtil::Int64ToUint64(sqlite3_column_int64(stmt, col++));
        block.attrJsonString = sqlite3_column_string(stmt, col++);
        block.firstAccessTimestamp = sqlite3_column_int64(stmt, col++);
        block.lastAccessTimestamp = sqlite3_column_int64(stmt, col++);
        block.maxAccessInterval = NumberUtil::Int64ToUint64(sqlite3_column_int64(stmt, col++));
        blocks.emplace_back(block);
    }
    return count;
}
int64_t LeaksMemoryDatabase::QueryMemoryBlocks(const LeaksMemoryBlockParams &queryParams,
                                               const bool isTable,
                                               std::vector<MemoryBlock> &blocks)
{
    std::string queryCountColumns = GetSelectBlocksFullColumns(queryParams.relativeTime);
    sqlite3_stmt* stmt = BuildQueryBlocksByQueryParamsAndBindParam(queryCountColumns, queryParams, isTable);
    if (stmt == nullptr) {
        ServerLog::Error("Query blocks table. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return -1;
    }
    int64_t count = QueryMemoryBlocksByStep(stmt, blocks, true);
    sqlite3_finalize(stmt);
    return count;
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
    uint64_t minTimestamp = queryParams.relativeTime ? globalMinTimestamp : 0;
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

std::string LeaksMemoryDatabase::BuildQueryEventsConditionSqlByParams(const LeaksMemoryEventParams &queryParams,
                                                                      bool &timeCondition,
                                                                      bool &filtersCondition)
{
    uint64_t minTimestamp = queryParams.relativeTime ? globalMinTimestamp : 0;
    std::string conditionSql = " where ";
    std::string minTimestampStr = std::to_string(minTimestamp);
    // 构造设备id参数
    conditionSql = StringUtil::StrJoin(conditionSql, StringUtil::FormatString(" {} = ? ", EVENT::DEVICE_ID));
    // 构造时间戳参数
    if (queryParams.endTimestamp >= queryParams.startTimestamp && queryParams.endTimestamp > 0) {
        timeCondition = true;
        std::string timeConditionSql = StringUtil::FormatString(" AND (({} - {}) BETWEEN ? AND ?) ",
                                                                EVENT::TIMESTAMP,  minTimestampStr);
        conditionSql = StringUtil::StrJoin(conditionSql, timeConditionSql);
    }
    // 构造过滤参数，此前filters已经过存在性校验，并已转换成列名
    if (!queryParams.filters.empty() || !queryParams.rangeFilters.empty()) {
        filtersCondition = true;
        conditionSql.append(BuildQueryFiltersConditionSqlByParams(queryParams));
        conditionSql.append(BuildQueryRangeFiltersConditionSqlByParams(queryParams));
    }
    // 构造排序参数, 此前orderBy已经过存在性校验，并已转换成列名，列名无法通过参数化绑定
    if (!queryParams.orderBy.empty()) {
        conditionSql.append(BuildQueryOrderSqlByParams(queryParams));
    }
    // 构造LIMIT OFFSET
    conditionSql.append(" LIMIT ? OFFSET ? ");
    return conditionSql;
}

std::string LeaksMemoryDatabase::BuildQueryRangeFiltersConditionSqlByParams(const RangeFiltersParam& rangeFiltersParam)
{
    std::string sql;
    for (auto [colName, rangePair] : rangeFiltersParam.rangeFilters) {
        sql.append(StringUtil::FormatString(" AND ({} BETWEEN ? AND ?) ", colName));
    }
    return sql;
}

/***
 * 构造查询memory_block的where语句
 * @param queryParams 请求查询参数
 * @param onlyAllocOrFreeInTimeRange 是否在请求区间内申请或释放的内存块
 * @param timeCondition 添加了时间查询条件的flag
 * @param filtersCondition 添加了字段过滤查询条件的flag
 * @return
 */
std::string LeaksMemoryDatabase::BuildQueryBlocksConditionSqlByParams(const LeaksMemoryBlockParams& queryParams,
                                                                      const bool onlyAllocOrFreeInTimeRange,
                                                                      bool& timeCondition,
                                                                      bool& filtersCondition)
{
    uint64_t minTimestamp = queryParams.relativeTime ? globalMinTimestamp : 0;
    std::string conditionSql = " where ";
    std::string minTimestampStr = std::to_string(minTimestamp);
    // 构造设备id参数
    conditionSql = StringUtil::StrJoin(conditionSql, StringUtil::FormatString(" {} = ? ", BLOCK::DEVICE_ID));
    // 构造eventType参数
    conditionSql.append(StringUtil::FormatString(" AND {} = ? ", BLOCK::EVENT_TYPE));
    // 构造Size参数
    if (queryParams.maxSize > 0) {
        conditionSql.append(StringUtil::FormatString(" AND ({} BETWEEN {} AND {}) ", BLOCK::SIZE,
                                                     std::to_string(queryParams.minSize),
                                                     std::to_string(queryParams.maxSize)));
    }
    // 构造时间戳参数
    if (queryParams.endTimestamp >= queryParams.startTimestamp && queryParams.endTimestamp > 0) {
        timeCondition = true;
        // 是否仅在请求区间内申请或释放的内存块
        std::string timeConditionSql = onlyAllocOrFreeInTimeRange ?
                                           " AND (({} - {}) BETWEEN ? AND ? OR ({} - {}) BETWEEN ? AND ?) " :
                                           " AND NOT (({} - {}) < ? OR ({} - {}) > ?) ";
        timeConditionSql = StringUtil::FormatString(timeConditionSql,
                                                    BLOCK::END_TIMESTAMP, minTimestampStr,
                                                    BLOCK::START_TIMESTAMP, minTimestampStr);
        conditionSql.append(timeConditionSql);
    }
    // 构造过滤参数，此前filters已经过存在性校验，并已转换成列名
    if (!queryParams.filters.empty() || !queryParams.rangeFilters.empty()) {
        filtersCondition = true;
        conditionSql.append(BuildQueryFiltersConditionSqlByParams(queryParams));
        conditionSql.append(BuildQueryRangeFiltersConditionSqlByParams(queryParams));
    }
    // 构造排序参数, 此前orderBy已经过存在性校验，并已转换成列名，列名无法通过参数化绑定；如果不指定排序，默认根据startTimestamp排序
    if (!queryParams.orderBy.empty()) {
        conditionSql.append(BuildQueryOrderSqlByParams(queryParams));
    }
    // 构造LIMIT OFFSET
    conditionSql.append(" LIMIT ? OFFSET ? ");
    return conditionSql;
}

sqlite3_stmt* LeaksMemoryDatabase::BuildQueryEventsByQueryParamsAndBindParam(std::string &selectColumns,
                                                                             const LeaksMemoryEventParams &queryParams)
{
    bool timeCondition = false;
    bool filtersCondition = false;
    std::string conditionSql = BuildQueryEventsConditionSqlByParams(queryParams, timeCondition, filtersCondition);
    std::string sql = StringUtil::FormatString("select {} from {} {} ",
                                               selectColumns, TABLE_LEAKS_DUMP, conditionSql);
    sqlite3_stmt *stmt = nullptr;
    int result  = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query events table. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return nullptr;
    }
    int bindIdx = bindStartIndex;
    sqlite3_bind_text(stmt, bindIdx++, queryParams.deviceId.c_str(),
                      queryParams.deviceId.length(), SQLITE_TRANSIENT);
    // 绑定时间戳参数
    if (timeCondition) {
        sqlite3_bind_int64(stmt, bindIdx++, queryParams.startTimestamp);
        sqlite3_bind_int64(stmt, bindIdx++, queryParams.endTimestamp);
    }
    // 绑定filters参数
    if (filtersCondition) {
        CommonBindFiltersParams(queryParams, stmt, bindIdx);
        CommonBindRangeFiltersParams(queryParams, stmt, bindIdx);
    }
    // 绑定分页参数
    CommonBindPaginationParams(queryParams, stmt, bindIdx);
    return stmt;
}

sqlite3_stmt* LeaksMemoryDatabase::BuildQueryBlocksByQueryParamsAndBindParam(const std::string& selectColumns,
                                                                             const LeaksMemoryBlockParams& queryParams,
                                                                             const bool isTable)
{
    bool timeCondition = false;
    bool filtersCondition = false;
    bool onlyAllocOrFreeInTimeRange = !isTable;
    std::string conditionSql = BuildQueryBlocksConditionSqlByParams(queryParams, onlyAllocOrFreeInTimeRange,
                                                                    timeCondition, filtersCondition);
    std::string sql = StringUtil::FormatString("select {} from {} {} ",
                                               selectColumns, memoryBlockTable, conditionSql);
    sqlite3_stmt *stmt = nullptr;
    int result  = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query blocks table. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return nullptr;
    }
    int bindIdx = bindStartIndex;
    sqlite3_bind_text(stmt, bindIdx++, queryParams.deviceId.c_str(),
                      queryParams.deviceId.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, bindIdx++, queryParams.eventType.c_str(),
                      queryParams.eventType.length(), SQLITE_TRANSIENT);
    // 绑定时间戳参数
    if (timeCondition) {
        sqlite3_bind_int64(stmt, bindIdx++, queryParams.startTimestamp);
        sqlite3_bind_int64(stmt, bindIdx++, queryParams.endTimestamp);
        if (onlyAllocOrFreeInTimeRange) {
            sqlite3_bind_int64(stmt, bindIdx++, queryParams.startTimestamp);
            sqlite3_bind_int64(stmt, bindIdx++, queryParams.endTimestamp);
        }
    }
    // 绑定filters参数
    if (filtersCondition) {
        CommonBindFiltersParams(queryParams, stmt, bindIdx);
        CommonBindRangeFiltersParams(queryParams, stmt, bindIdx);
    }
    // 绑定分页参数
    CommonBindPaginationParams(queryParams, stmt, bindIdx);
    return stmt;
}

int64_t LeaksMemoryDatabase::QueryEventsByRequestParams(const LeaksMemoryEventParams &queryParams,
                                                        std::vector<MemoryEvent>& events)
{
    sqlite3_stmt* stmt = nullptr;
    std::string queryCountColumns = GetSelectEventsFullColumns(queryParams.relativeTime);
    stmt = BuildQueryEventsByQueryParamsAndBindParam(queryCountColumns, queryParams);
    if (stmt == nullptr) {
        ServerLog::Error("Query events table by params failed. Failed to prepare sql.");
        return -1;
    }
    int64_t count = QueryMemoryEventsByStep(stmt, events, true);
    sqlite3_finalize(stmt);
    return count;
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
                                   "WHERE {} <= ? AND {} > ? AND {} == ? AND ({} == ? OR {} like ?);",
                                   BLOCK::SIZE, memoryBlockTable, BLOCK::START_TIMESTAMP,
                                   BLOCK::END_TIMESTAMP, BLOCK::DEVICE_ID, BLOCK::OWNER, BLOCK::OWNER);
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
    std::string ownerPattern = StringUtil::StrJoin(owner, "@%");
    sqlite3_bind_text(stmt, bindIdx++, owner.c_str(), owner.length(), SQLITE_TRANSIENT);
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
    uint64_t minTimestamp = queryParams.relativeTime ? globalMinTimestamp : 0;
    std::string timeCondition;
    BuildTimeConditionForQueryPythonTraces(timeCondition, queryParams);
    std::string sql = StringUtil::FormatString("SELECT {}, {}, ({} - ?) AS START, ({} - ?) AS END, {}, {}, {} FROM {} "
                                               "WHERE NOT ({}) AND {} == ? AND END > START "
                                               "ORDER BY {}", TRACE::ID, TRACE::FUNC_INFO, COL_START_TIME,
                                               COL_END_TIME, TRACE::THREAD_ID, TRACE::PROCESS_ID, TRACE::DEPTH,
                                               traceTableName, timeCondition, TRACE::THREAD_ID, COL_START_TIME);
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
    std::vector<std::string> traceTableNames = GetPythonTraceTables();
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
/***
 * 构造过滤sql，要求此前filters已经过存在性校验，并已转换成列名
 * @param filtersParam 过滤参数
 * @return 过滤sql
 */
std::string LeaksMemoryDatabase::BuildQueryFiltersConditionSqlByParams(const FiltersParam& filtersParam)
{
    std::string filtersSql;
    for (auto &filterPair : filtersParam.filters) {
        filtersSql.append(StringUtil::FormatString(" AND {} LIKE ? ", filterPair.first));
    }
    return filtersSql;
}
/***
 * 构造排序sql，此前orderBy已经过存在性校验，并已转换为列名
 * @param orderByParam 排序参数
 * @return 排序sql
 */
std::string LeaksMemoryDatabase::BuildQueryOrderSqlByParams(const OrderByParam& orderByParam)
{
    return StringUtil::FormatString(" ORDER BY {} {} ",
                                    orderByParam.orderBy, orderByParam.desc ? "DESC" : "ASC");
}

void LeaksMemoryDatabase::CommonBindFiltersParams(const FiltersParam& queryParams, sqlite3_stmt* stmt, int &bindIdx)
{
    for (auto &filterPair : queryParams.filters) {
        std::string filterPattern = StringUtil::FormatString("%{}%", filterPair.second);
        sqlite3_bind_text(stmt, bindIdx++, filterPattern.c_str(),
                          filterPattern.length(), SQLITE_TRANSIENT);
    }
}

void LeaksMemoryDatabase::CommonBindRangeFiltersParams(const RangeFiltersParam& queryParams, sqlite3_stmt* stmt,
                                                       int& bindIdx)
{
    for (auto [colName, rangePair] : queryParams.rangeFilters) {
        sqlite3_bind_double(stmt, bindIdx++, rangePair.first);
        sqlite3_bind_double(stmt, bindIdx++, rangePair.second);
    }
}

void LeaksMemoryDatabase::CommonBindPaginationParams(const PaginationParam& queryParams, sqlite3_stmt* stmt,
                                                     int& bindIdx)
{
    // 绑定分页参数
    int64_t limit = -1;
    int64_t offset = 0;
    if (queryParams.pageSize > 0 && queryParams.currentPage > 0) {
        limit = queryParams.pageSize;
        // 入口处CommonCheck已做了校验，此处无需校验溢出/翻转问题
        offset = (queryParams.currentPage - 1) * queryParams.pageSize;
    }
    sqlite3_bind_int64(stmt, bindIdx++, limit);
    sqlite3_bind_int64(stmt, bindIdx++, offset);
}

void LeaksMemoryDatabase::QueryEventsByGroupId(const uint64_t groupId, const std::string &deviceId,
                                               const bool relativeTime, std::vector<MemoryEvent>& events)
{
    std::string sql = StringUtil::FormatString("SELECT {} FROM {} WHERE {} like ? ORDER BY {}",
                                               GetSelectEventsFullColumns(relativeTime), TABLE_LEAKS_DUMP,
                                               EVENT::ATTR, EVENT::TIMESTAMP);
    std::string groupIdPattern = StringUtil::FormatString(R"(%"{}":"{}"%)", BLOCK_EVENT_ATTR_GROUP_ID_FIELD,
                                                          std::to_string(groupId));
    sqlite3_stmt* stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query events by group failed. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return;
    }
    int bindIdx = bindStartIndex;
    sqlite3_bind_text(stmt, bindIdx++, groupIdPattern.c_str(), groupIdPattern.length(), SQLITE_TRANSIENT);
    QueryMemoryEventsByStep(stmt, events, true);
    sqlite3_finalize(stmt);
}

void LeaksMemoryDatabase::QueryAllDeviceExtremumTimestamp(
    std::unordered_map<std::string, std::pair<uint64_t, uint64_t>> &extreTsMap)
{
    std::string sql = StringUtil::FormatString("SELECT {},MIN({}),MAX({}) FROM {} WHERE {} NOT IN ('N/A','host') GROUP BY {}",
                                               EVENT::DEVICE_ID, EVENT::TIMESTAMP, EVENT::TIMESTAMP, TABLE_LEAKS_DUMP,
                                               EVENT::DEVICE_ID, EVENT::DEVICE_ID);
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query devices extremum timestamp failed. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        std::string deviceId = sqlite3_column_string(stmt, col++);
        uint64_t minTimestamp = NumberUtil::Int64ToUint64(sqlite3_column_int64(stmt, col++));
        uint64_t maxTimestamp = NumberUtil::Int64ToUint64(sqlite3_column_int64(stmt, col++));
        extreTsMap[deviceId] = std::make_pair(minTimestamp, maxTimestamp);
    }
    sqlite3_finalize(stmt);
    return;
}

bool LeaksMemoryDatabase::SetCallStackExistsFlagByCheckColumn()
{
    std::string sql = StringUtil::FormatString("SELECT DISTINCT name FROM pragma_table_info('{}') "
                                               "WHERE name LIKE ?", TABLE_LEAKS_DUMP);
    sqlite3_stmt* stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Check callstack columns exist failed. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return false;
    }
    std::string namePattern = StringUtil::StrJoin(callStackPrefix, "%");
    sqlite3_bind_text(stmt, bindStartIndex, namePattern.c_str(), namePattern.length(), SQLITE_TRANSIENT);
    std::set<std::string> callStackCols;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        callStackCols.emplace(sqlite3_column_string(stmt, resultStartIndex));
    }
    sqlite3_finalize(stmt);
    std::string callStackC = std::string(EVENT::CALL_STACK_C);
    StringUtil::StripDbColumnName(callStackC);
    std::string callStackPython = std::string(EVENT::CALL_STACK_PYTHON);
    StringUtil::StripDbColumnName(callStackPython);
    withCallStackC = callStackCols.find(callStackC) != callStackCols.end();
    withCallStackPython = callStackCols.find(callStackPython) != callStackCols.end();
    return true;
}

uint64_t LeaksMemoryDatabase::GetProcessIdByPythonTraceTableName(const std::string& tableName)
{
    if (StringUtil::StartWith(tableName, pythonTraceTablePrefix)) {
        return NumberUtil::StringToUnsignedLongLong(tableName.substr(pythonTraceTablePrefix.length()));
    }
    return 0;
}

bool LeaksMemoryDatabase::AppendDepthColumnForPythonTraceTables()
{
    if (!isOpen) {
        ServerLog::Error("[LeaksMemory] Failed to add depth for python trace. Database is not open.");
        return false;
    }
    std::vector<std::string> sqlList = GetAlterPythonTraceTablesAddDepthColumnSql();
    std::unique_lock<std::recursive_mutex> lock(mutex);
    for (auto &sql : sqlList) {
        if (!ExecSql(sql)) {
            return false;
        }
    }
    return true;
}

std::vector<std::string> LeaksMemoryDatabase::GetAlterPythonTraceTablesAddDepthColumnSql()
{
    std::vector<std::string> sqlList;
    std::vector<std::string> pythonTraceTables = GetPythonTraceTables();
    for (auto &tableName : pythonTraceTables) {
        if (CheckColumnExist(tableName, std::string(TRACE::DEPTH))) {
            continue;
        }
        sqlList.emplace_back(StringUtil::FormatString("ALTER TABLE {} ADD COLUMN {} INTEGER DEFAULT -1;",
                                                      tableName, TRACE::DEPTH));
    }
    return sqlList;
}

void LeaksMemoryDatabase::FlushPythonTraceCache()
{
    if (slicePidCache.empty()) {
        return;
    }
    for (auto &sliceCachePair : slicePidCache) {
        if (sliceCachePair.second.empty()) {
            continue;
        }
        UpdatePythonTraceSliceList(sliceCachePair.second, sliceCachePair.first);
        sliceCachePair.second.clear();
    }
}

bool LeaksMemoryDatabase::QueryAndSetGlobalExtremumTimestamp()
{
    std::string queryEventExTSSql = StringUtil::FormatString("SELECT MIN({}), MAX({}) FROM {} WHERE {} != 'N/A';",
                                                             EVENT::TIMESTAMP, EVENT::TIMESTAMP,
                                                             TABLE_LEAKS_DUMP, EVENT::TIMESTAMP);
    if (!ExecuteQueryAndSetGlobalExtremumTimestamp(queryEventExTSSql)) {
        ServerLog::Error("Failed to query event extremum timestamp");
        return false;
    }
    std::vector<std::string> pythonTraceTables = GetPythonTraceTables();
    for (auto &tableName : pythonTraceTables) {
        std::string queryTraceExTSSql = StringUtil::FormatString("SELECT MIN({}), MAX({}) FROM {} "
                                                                 "WHERE {} != 'N/A' AND {} != 'N/A';",
                                                                 TRACE::START_TIME, TRACE::END_TIME, tableName,
                                                                 TRACE::START_TIME, TRACE::END_TIME);
        if (!ExecuteQueryAndSetGlobalExtremumTimestamp(queryTraceExTSSql)) {
            ServerLog::Error("Failed to query trace extremum timestamp");
            return false;
        }
    }
    return true;
}

bool LeaksMemoryDatabase::ExecuteQueryAndSetGlobalExtremumTimestamp(const std::string &sql)
{
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query extremum timestamp failed. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return false;
    }
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int idx = resultStartIndex;
        int64_t minTimestamp = sqlite3_column_int64(stmt, idx++);
        int64_t maxTimestamp = sqlite3_column_int64(stmt, idx++);
        if (minTimestamp <= 0 || maxTimestamp <= 0) {
            ServerLog::Error("Query event extremum timestamp failed.Invalid timestamp: %,%", minTimestamp, maxTimestamp);
            return false;
        }
        globalMinTimestamp = std::min(globalMinTimestamp, NumberUtil::Int64ToUint64(minTimestamp));
        globalMaxTimestamp = std::max(globalMaxTimestamp, NumberUtil::Int64ToUint64(maxTimestamp));
    }
    sqlite3_finalize(stmt);
    return true;
}

bool LeaksMemoryDatabase::CheckGlobalExtremumTimestampValid() const
{
    return (globalMaxTimestamp > globalMinTimestamp) && globalMaxTimestamp < INT64_MAX && globalMinTimestamp > 0;
}

uint64_t LeaksMemoryDatabase::GetGlobalMinTimestamp() const { return globalMinTimestamp; }

uint64_t LeaksMemoryDatabase::GetGlobalMaxTimestamp() const { return globalMaxTimestamp; }

std::string LeaksMemoryDatabase::GetSelectEventsFullColumns(const bool relativeTime)
{
    std::string columns = "COUNT(*) OVER()";
    for (const auto &columnObj : EVENT::FIELD_FULL_COLUMNS) {
        if (columnObj.name == EVENT::CALL_STACK_C && !withCallStackC) {
            continue;
        }
        if (columnObj.name == EVENT::CALL_STACK_PYTHON && !withCallStackPython) {
            continue;
        }
        if (EVENT::TIMESTAMP_COLUMN_SET.find(columnObj.name) !=  EVENT::TIMESTAMP_COLUMN_SET.end() && relativeTime) {
            columns.append(StringUtil::FormatString(", {} - {} AS _{}", columnObj.name,
                                                    std::to_string(GetGlobalMinTimestamp()), columnObj.key));
            continue;
        }
        columns.append(StringUtil::FormatString(", {} AS _{}", columnObj.name, columnObj.key));
    }
    return columns;
}

std::string LeaksMemoryDatabase::GetSelectBlocksFullColumns(const bool relativeTime)
{
    std::string columns = "COUNT(*) OVER()";
    for (auto &columnObj : BLOCK::FIELD_FULL_COLUMNS) {
        if (BLOCK::TIMESTAMP_COLUMN_SET.find(columnObj.name) !=  BLOCK::TIMESTAMP_COLUMN_SET.end() && relativeTime) {
            columns.append(StringUtil::FormatString(", {} - {} AS _{}", columnObj.name,
                                                    std::to_string(GetGlobalMinTimestamp()), columnObj.key));
            continue;
        }
        columns.append(StringUtil::FormatString(", {} AS _{}", columnObj.name, columnObj.key));
    }
    return columns;
}

}  // FullDb
}  // Module
}  // Dic