/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_LEAKSMEMORYDATABASE_H
#define PROFILER_SERVER_LEAKSMEMORYDATABASE_H

#include "pch.h"
#include "WsSender.h"
#include "Database.h"
#include "TimelineProtocolEvent.h"
#include "ServerLog.h"
#include "TraceTime.h"
#include "ParserStatusManager.h"
#include "MemoryDetailProtocolRequest.h"
#include "MemoryDetailEntities.h"
#include "MemoryDetailDefs.h"
#include "LeaksMemoryTableColumn.h"

namespace Dic {
namespace Module {
namespace FullDb {
using namespace Dic::Protocol;
using namespace Dic::Module::MemoryDetail;
class LeaksMemoryDatabase : public Database {
public:
    explicit LeaksMemoryDatabase(std::recursive_mutex& sqlMutex) : Database(sqlMutex) {};
    ~LeaksMemoryDatabase() override;
    bool CheckTablesExist();
    static void Reset();
    bool OpenDb(const std::string& dbPath, bool clearAllTable) override;
    bool HasFinishedParseLastTime();
    bool UpdateParseStatus(const std::string& status);
    bool InitStmt();
    void ReleaseStmt();
    sqlite3_stmt* GetInsertAllocationsStmt(uint64_t allocationsLen);
    sqlite3_stmt* GetInsertBlocksStmt(uint64_t blocksLen);
    sqlite3_stmt* GetUpdatePythonTraceSliceStmt(uint64_t processId);
    bool CreateMemoryAllocationAndBlockTable();
    bool AppendDepthColumnForPythonTraceTables();
    bool DropMemoryAllocationAndBlockTable();
    bool QueryEntireEventsTable(std::vector<MemoryEvent>& eventDetails);
    int64_t QueryEventsByRequestParams(const LeaksMemoryEventParams &queryParams, std::vector<MemoryEvent>& events);
    void QueryDeviceIds(std::set<std::string>& deviceIdSet);
    void QueryThreadIdsByProcessId(uint64_t processId, std::vector<uint64_t>& threadIds);
    void QueryMallocOrFreeEventTypeWithDeviceId(std::unordered_map<std::string, std::vector<std::string>>& resultMap);
    void QueryThreadIds(std::vector<uint64_t>& threadIds);
    int64_t QueryMemoryBlocks(const LeaksMemoryBlockParams& queryParams,
                              const bool isTable,
                              std::vector<MemoryBlock>& blocks);
    void QueryMemoryBlocksOwnersReleasedAfterTimestamp(const std::string& deviceId, const std::string& eventType,
                                                       uint64_t timestamp, std::set<std::string>& owners);
    void QueryPythonTrace(const LeaksMemoryThreadPythonTraceParams& queryParams, LeaksMemoryPythonTrace& trace);
    void QueryPythonTracesUsingTableName(const std::string& traceTableName,
                                         const LeaksMemoryThreadPythonTraceParams& queryParams,
                                         LeaksMemoryPythonTrace& trace);
    uint64_t QueryTotalSizeUntilTimestampUsingOwner(const std::string& deviceId, uint64_t timestamp,
                                                    const std::string& owner);
    void QueryMemoryAllocations(const LeaksMemoryAllocationParams& queryParams,
                                std::vector<MemoryAllocation>& allocations);
    std::optional<MemoryAllocation> QueryLatestAllocationWithinTimestamp(const std::string& deviceId,
                                                                         const std::string& eventType,
                                                                         uint64_t timestamp);
    std::optional<MemoryAllocation> QueryNextAllocationAfterTimestamp(const std::string& deviceId,
                                                                      const std::string& eventType, uint64_t timestamp);
    void QueryAllDeviceExtremumTimestamp(std::unordered_map<std::string, std::pair<uint64_t, uint64_t>> &extreTsMap);
    void QueryEventsByGroupId(const uint64_t groupId, const std::string &deviceId,
                              bool relativeTime, std::vector<MemoryEvent> &events);
    std::vector<std::string> GetPythonTraceTables();
    void UpdatePythonTraceSlice(const PythonTraceSlice &slice);
    void UpdatePythonTraceSliceList(const std::vector<PythonTraceSlice> &slices, uint64_t processId);
    void InsertMemoryAllocationList(const std::vector<MemoryAllocation>& allocList);
    void InsertMemoryAllocation(const MemoryAllocation& alloc);
    void InsertMemoryBlockList(const std::vector<MemoryBlock>& blocklist);
    void InsertMemoryBlock(const MemoryBlock& block);
    void FlushMemoryBlocksCache();
    void FlushMemoryAllocationsCache();
    void FlushPythonTraceCache();
    uint64_t GetGlobalMinTimestamp() const;
    uint64_t GetGlobalMaxTimestamp() const;

    bool withCallStackC = false;
    bool withCallStackPython = false;

private:
    int64_t QueryMemoryEventsByStep(sqlite3_stmt* stmt, std::vector<MemoryEvent>& events,
                                    const bool withExtraCountCol);
    int64_t QueryMemoryBlocksByStep(sqlite3_stmt* stmt, std::vector<MemoryBlock>& blocks,
                                    const bool withExtraCountCol);
    bool QueryMemoryAllocationsByStep(sqlite3_stmt* stmt, std::vector<MemoryAllocation>& allocations);
    bool QueryMemoryPythonTracesByStep(sqlite3_stmt* stmt, LeaksMemoryPythonTrace& trace);
    std::string BuildQueryEventsConditionSqlByParams(const LeaksMemoryEventParams &queryParams,
                                                     bool &timeCondition,
                                                     bool &filtersCondition);
    std::string BuildQueryBlocksConditionSqlByParams(const LeaksMemoryBlockParams &queryParams,
                                                     bool onlyAllocOrFreeInTimeRange,
                                                     bool &timeCondition,
                                                     bool &filtersCondition);
    sqlite3_stmt* BuildQueryEventsByQueryParamsAndBindParam(std::string &selectColumns,
                                                            const LeaksMemoryEventParams &queryParams);
    sqlite3_stmt* BuildQueryBlocksByQueryParamsAndBindParam(const std::string &selectColumns,
                                                            const LeaksMemoryBlockParams &queryParams,
                                                            bool isTable);

    static std::string BuildQueryFiltersConditionSqlByParams(const FiltersParam &filtersParam);
    static std::string BuildQueryOrderSqlByParams(const OrderByParam &orderByParam);
    static std::string BuildQueryRangeFiltersConditionSqlByParams(const RangeFiltersParam &rangeFiltersParam);
    static void CommonBindFiltersParams(const FiltersParam &queryParams, sqlite3_stmt* stmt, int &bindIdx);
    static void CommonBindRangeFiltersParams(const RangeFiltersParam &queryParams, sqlite3_stmt* stmt, int &bindIdx);
    static void CommonBindPaginationParams(const PaginationParam &queryParams, sqlite3_stmt* stmt, int &bindIdx);
    bool QueryAndSetGlobalExtremumTimestamp();
    bool CheckGlobalExtremumTimestampValid() const;
    bool ExecuteQueryAndSetGlobalExtremumTimestamp(const std::string &sql);
    uint64_t GetProcessIdByPythonTraceTableName(const std::string &tableName);
    std::string GetCreateMemoryAllocationTableSql();
    std::string GetCreateMemoryBlockTableSql();
    std::string GetSelectEventsFullColumns(const bool relativeTime);
    std::string GetSelectBlocksFullColumns(const bool relativeTime);
    std::vector<std::string> GetAlterPythonTraceTablesAddDepthColumnSql();
    bool SetCallStackExistsFlagByCheckColumn();

    // 内存折线图数据库中存储表名为memory_alloc, 包含优化前、后
    const std::string memoryAllocationTable = "memory_allocation";
    // 内存块图数据
    const std::string memoryBlockTable = "memory_block";
    // 火焰图数据
    const std::string pythonTraceTablePrefix = "python_trace_";
    // 调用栈列前缀
    const std::string callStackPrefix = "Call Stack";

    const std::string allocationColumnPattern =
        StringUtil::GenerateColumnString(MemoryAllocationTableColumn::FULL_COLUMNS_WITHOUT_ID);
    const std::string allocationValuePattern =
        StringUtil::CreateQuestionMarkString(std::size(MemoryAllocationTableColumn::FULL_COLUMNS_WITHOUT_ID));
    const std::string blockColumnPattern =
        StringUtil::GenerateColumnString(MemoryBlockTableColumn::FULL_COLUMNS_WITHOUT_ID);
    const std::string blockValuePattern =
        StringUtil::CreateQuestionMarkString(std::size(MemoryBlockTableColumn::FULL_COLUMNS_WITHOUT_ID));

    // Global Extremum Timestamp
    uint64_t globalMinTimestamp = INT64_MAX;
    uint64_t globalMaxTimestamp = 0;

    // Parse status info
    const std::string leaksMemoryParseStatus = "LEAKS_PARSE_STATUS";
    const uint64_t cacheSize = 100;

    bool hasInitStmt = false;

    sqlite3_stmt* insertAllocationStmt = nullptr;
    sqlite3_stmt* insertBlockStmt = nullptr;
    std::unordered_map<uint64_t, sqlite3_stmt*> updatePythonTraceDepthStmtPidMap;
    std::mutex cacheMutex;
    std::vector<MemoryAllocation> allocationCache;
    std::vector<MemoryBlock> blockCache;
    std::unordered_map<uint64_t, std::vector<PythonTraceSlice>> slicePidCache;
};

}  // namespace FullDb
}  // namespace Module
}  // namespace Dic

#endif  // PROFILER_SERVER_LEAKSMEMORYDATABASE_H
