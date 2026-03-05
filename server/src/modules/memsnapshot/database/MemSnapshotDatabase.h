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
#ifndef PROFILER_SERVER_MEMSNAPSHOTDATABASE_H
#define PROFILER_SERVER_MEMSNAPSHOTDATABASE_H

#include "pch.h"
#include "Database.h"
#include "MemSnapshotProtocolRequest.h"
#include "MemSnapshotDefs.h"
#include "MemSnapshotTableColumn.h"

namespace Dic::Module::FullDb {
using namespace Dic::Protocol;
using namespace Dic::Module::MemSnapshot;

class MemSnapshotDatabase : public Database {
public:
    explicit MemSnapshotDatabase(std::recursive_mutex& sqlMutex) : Database(sqlMutex) {};
    bool CheckAllTableExist();
    bool OpenDbReadOnly(const std::string& dbPath);

    // API
    // 查询所有的block，用于内存块生命周期图展示
    bool QueryAllBlocks(std::vector<Block> &blocks);
    // 基于id查询单个block的详细信息
    std::optional<Block> QueryBlockById(int64_t blockId);
    // 查询指定事件ID时活跃的blocks
    bool QueryActiveBlocksByEventId(int64_t eventId, std::vector<Block>& blocks);
    // 查询blocks表格数据，支持分页、过滤、排序等
    int64_t QueryBlocksTable(const MemSnapshotBlockParams& queryParams, std::vector<Block>& blocks);

    // 查询最大事件id
    [[nodiscard]] int64_t QueryMaxEntryId() const;
    static void Reset();

    // 查询内存记录（allocated/reserved/active）用于内存总量曲线
    void QueryMemoryRecords(const MemSnapshotAllocationParams& queryParams, std::vector<MemoryRecord>& records) const;
    // 查询trace_entry表格数据，支持分页、过滤、排序等
    int64_t QueryTraceEntriesTable(const MemSnapshotEventParams& queryParams, std::vector<TraceEntry>& entries);
    // 基于id查询单个trace_entry的详细信息
    std::optional<TraceEntry> QueryTraceEntryById(int64_t eventId);
    // 查询内存块的freeRequested事件
    std::optional<TraceEntry> QueryFreeRequestedTraceEntryByBlock(const Block& block);

    // 查询指定事件ID之前的segment相关事件（segment_alloc, segment_free, segment_map, segment_unmap）
    bool QuerySegmentEventsUntil(int64_t eventId, std::vector<TraceEntry>& events);

    // 字典表
    static std::string GetTableColumnTag(const std::string& tableName, const std::string& colName);
    std::string GetRealValueInTableDictionaryMap(const std::string& tableName, const std::string& colName, int intVal);
    int GetKeyInTableDictionaryMap(const std::string& tableName, const std::string& colName, const std::string& realVal);
private:
    static inline const std::string LOG_TAG = "[MemSnapshotDb] ";
    const std::string blockTable = "block";
    const std::string traceEntryTable = "trace_entry";
    const std::string dictionaryTable = "dictionary";
    int64_t maxEntryId = 0;
    std::map<std::string, std::map<int, std::string>> tableDictionaryMap;
    bool InitTableDictionaryMap();
    bool InitContext();
    Block QueryBlockByStep(sqlite3_stmt* stmt, int startIdx = 0);
    TraceEntry QueryTraceEntryByStep(sqlite3_stmt* stmt, int startIdx = 0);


    std::string BuildMemSnapshotFiltersParamSql(FiltersParam& queryParams, const std::string& tableName);

    // Blocks table query helpers
    int64_t QueryBlocksTableByStep(sqlite3_stmt* stmt, std::vector<Block>& blocks, bool withExtraCountCol);
    static std::string GetSelectBlocksTableFullColumns();
    std::string BuildQueryBlocksTableConditionSqlByParams(MemSnapshotBlockParams& queryParams,
                                                          bool& eventIdxRangeCondition, bool& filtersCondition);
    [[nodiscard]] sqlite3_stmt* BuildQueryBlocksTableByQueryParamsAndBindParam(const std::string& selectColumns,
                                                                               const MemSnapshotBlockParams& queryParams);
    // TraceEntry table query helpers
    int64_t QueryTraceEntriesTableByStep(sqlite3_stmt* stmt, std::vector<TraceEntry>& entries, bool withExtraCountCol);
    static std::string GetSelectTraceEntriesTableFullColumns();
    std::string BuildQueryTraceEntriesTableConditionSqlByParams(MemSnapshotEventParams& queryParams,
                                                                bool& eventIdxRangeCondition, bool& filtersCondition);
    [[nodiscard]] sqlite3_stmt* BuildQueryTraceEntriesTableByQueryParamsAndBindParam(const std::string& selectColumns,
                                                                                   const MemSnapshotEventParams& queryParams);
};
}


#endif //PROFILER_SERVER_MEMSNAPSHOTDATABASE_H