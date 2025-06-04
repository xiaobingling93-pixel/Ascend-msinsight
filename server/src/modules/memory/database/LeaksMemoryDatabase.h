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
    void QueryDeviceIds(std::set<std::string> &deviceIdSet);
    void QueryMallocOrFreeEventTypeWithDeviceId(std::unordered_map<std::string, std::vector<std::string>> &resultMap);
    void QueryMemoryBlocks(const LeaksMemoryBlockParams &queryParams, std::vector<Memory::MemoryBlock> &blocks);
    void QueryMemoryAllocations(const LeaksMemoryAllocationParams &queryParams,
                                std::vector<Memory::MemoryAllocation> &allocations);
    uint64_t QueryMemoryEventExtremumTimestamp(const std::string &deviceId, bool isMinimum);
    void InsertMemoryAllocationList(const std::vector<Memory::MemoryAllocation> &allocList);
    void InsertMemoryAllocation(const Memory::MemoryAllocation &alloc);
    void InsertMemoryBlockList(const std::vector<Memory::MemoryBlock> &blocklist);
    void InsertMemoryBlock(const Memory::MemoryBlock &block);
    void FlushMemoryBlocksCache();
    void FlushMemoryAllocationsCache();

private:
    // 内存折线图数据库中存储表名为memory_alloc, 包含优化前、后
    const std::string memoryAllocationTable = "memory_allocation";
    // 内存块图数据
    const std::string memoryBlockTable = "memory_block";
    // 火焰图数据
    const std::string pythonTraceTable = "python_trace";
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
