/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_LEAKSMEMORYDATABASE_H
#define PROFILER_SERVER_LEAKSMEMORYDATABASE_H

#include "pch.h"
#include "WsSender.h"
#include "Database.h"
#include "MemoryProtocolRequest.h"
#include "TimelineProtocolEvent.h"
#include "ServerLog.h"
#include "TraceTime.h"
#include "ParserStatusManager.h"
#include "MemoryDef.h"
#include "LeaksMemoryPythonTrace.h"
#include "LeaksMemoryTableColumn.h"

namespace Dic {
namespace Module {
namespace FullDb {
using namespace Dic::Protocol;
class LeaksMemoryDatabase : public Database {
public:
    explicit LeaksMemoryDatabase(std::recursive_mutex &sqlMutex) : Database(sqlMutex){};
    ~LeaksMemoryDatabase() override;
    bool CheckTablesExist();
    static void Reset();
    bool OpenDb(const std::string &dbPath, bool clearAllTable) override;
    bool HasFinishedParseLastTime();
    bool UpdateParseStatus(const std::string &status);
    bool InitStmt();
    void ReleaseStmt();
    sqlite3_stmt *GetInsertAllocationsStmt(uint64_t allocationsLen);
    sqlite3_stmt *GetInsertBlocksStmt(uint64_t blocksLen);
    bool CreateMemoryAllocationAndBlockTable();
    bool DropMemoryAllocationAndBlockTable();
    bool QueryEntireEventsTable(std::vector<Memory::MemoryEvent> &eventDetails);
    bool QueryEventsWithinTimeRangeByDeviceId(uint64_t startTimestamp, uint64_t endTimestamp,
                                              const std::string &deviceId,
                                              std::vector<Memory::MemoryEvent> &events);
    void QueryDeviceIds(std::set<std::string> &deviceIdSet);
    void QueryThreadIdsByProcessId(uint64_t processId, std::vector<uint64_t> &threadIds);
    void QueryMallocOrFreeEventTypeWithDeviceId(std::unordered_map<std::string, std::vector<std::string>> &resultMap);
    void QueryThreadIds(std::vector<uint64_t > &threadIds);
    void QueryMemoryBlocks(const LeaksMemoryBlockParams &queryParams, std::vector<Memory::MemoryBlock> &blocks);
    void QueryMemoryBlocksOwnersReleasedAfterTimestamp(const std::string &deviceId, uint64_t timestamp,
                                                       std::set<std::string> &owners);
    void QueryPythonTrace(const LeaksMemoryThreadPythonTraceParams &queryParams,
                          Memory::LeaksMemoryPythonTrace &trace);
    void QueryPythonTracesUsingTableName(const std::string &traceTableName,
                                         const LeaksMemoryThreadPythonTraceParams &queryParams,
                                         Memory::LeaksMemoryPythonTrace &trace);
    uint64_t QueryTotalSizeUtilTimestampUsingOwner(const std::string &deviceId,
                                                                        uint64_t timestamp,
                                                                        const std::string &owner);
    void QueryMemoryAllocations(const LeaksMemoryAllocationParams &queryParams,
                                std::vector<Memory::MemoryAllocation> &allocations);
    std::optional<Memory::MemoryAllocation> QueryLatestAllocationWithinTimestamp(const std::string &deviceId,
                                                                  const std::string &eventType,
                                                                  uint64_t timestamp);
    std::optional<Memory::MemoryAllocation> QueryNextAllocationAfterTimestamp(const std::string &deviceId,
                                                                              const std::string &eventType,
                                                                              uint64_t timestamp);
    uint64_t QueryMemoryEventExtremumTimestamp(const std::string &deviceId, bool isMinimum);
    void InsertMemoryAllocationList(const std::vector<Memory::MemoryAllocation> &allocList);
    void InsertMemoryAllocation(const Memory::MemoryAllocation &alloc);
    void InsertMemoryBlockList(const std::vector<Memory::MemoryBlock> &blocklist);
    void InsertMemoryBlock(const Memory::MemoryBlock &block);
    void FlushMemoryBlocksCache();
    void FlushMemoryAllocationsCache();

private:
    bool QueryMemoryEventsByStep(sqlite3_stmt* stmt, std::vector<Memory::MemoryEvent> &events);
    bool QueryMemoryBlocksByStep(sqlite3_stmt* stmt, std::vector<Memory::MemoryBlock> &blocks);
    bool QueryMemoryAllocationsByStep(sqlite3_stmt* stmt, std::vector<Memory::MemoryAllocation> &allocations);
    bool QueryMemoryPythonTracesByStep(sqlite3_stmt* stmt, Memory::LeaksMemoryPythonTrace &trace);
    void AppendMemoryBlockQueryConditionSqlByParams(const LeaksMemoryBlockParams &queryParams, std::string &querySql);
    std::vector<std::string> GetPythonTraceTables();
    std::string GetCreateMemoryAllocationTableSql();
    std::string GetCreateMemoryBlockTableSql();
    // 内存折线图数据库中存储表名为memory_alloc, 包含优化前、后
    const std::string memoryAllocationTable = "memory_allocation";
    // 内存块图数据
    const std::string memoryBlockTable = "memory_block";
    // 火焰图数据
    const std::string pythonTraceTablePrefix = "python_trace_";

    // Parse status info
    const std::string leaksMemoryParseStatus = "LEAKS_PARSE_STATUS";
    const uint64_t cacheSize = 100;

    bool hasInitStmt = false;

    sqlite3_stmt *insertAllocationStmt = nullptr;
    sqlite3_stmt *insertBlockStmt = nullptr;
    std::mutex cacheMutex;
    std::vector<Memory::MemoryAllocation> allocationCache;
    std::vector<Memory::MemoryBlock> blockCache;
};

}  // FullDb
}  // Module
}  // Dic

#endif  // PROFILER_SERVER_LEAKSMEMORYDATABASE_H
