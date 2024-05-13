/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2024. All rights reserved.
 */
#include <algorithm>
#include "ServerLog.h"
#include "TraceTime.h"
#include "TableDefs.h"
#include "TraceFileParser.h"
#include "TraceDatabaseHelper.h"
#include "SliceDepthCacheManager.h"
#include "JsonTraceDatabase.h"

namespace Dic::Module::Timeline {
using namespace Dic::Server;
using namespace Dic::Protocol;
JsonTraceDatabase::JsonTraceDatabase(std::recursive_mutex &sqlMutex) : VirtualTraceDatabase(sqlMutex)
{
    if (importActionAnalyzerPtr == nullptr) {
        importActionAnalyzerPtr = std::make_unique<ImportActionAnalyzer>();
    }
    if (sliceAnalyzerPtr == nullptr) {
        sliceAnalyzerPtr = std::make_unique<SliceAnalyzer>();
    }
    if (flowAnalyzerPtr == nullptr) {
        flowAnalyzerPtr = std::make_unique<FlowAnalyzer>();
    }
}

JsonTraceDatabase::~JsonTraceDatabase()
{
    CommitData();
    ReleaseStmt();
    importActionAnalyzerPtr = nullptr;
    sliceAnalyzerPtr = nullptr;
    flowAnalyzerPtr = nullptr;
}

bool JsonTraceDatabase::OpenDb(const std::string &dbPath, bool clearAllTable)
{
    Database::OpenDb(dbPath, clearAllTable);
    SetConfig();
}

bool JsonTraceDatabase::InitStmt()
{
    if (initStmt) {
        return true;
    }
    initStmt = true;
    return InitSliceFlowCounterStmt() && InitProcessThreadStmt();
}

bool JsonTraceDatabase::InitSliceFlowCounterStmt()
{
    std::string sql = JsonSqlConstant::GetInsertSliceSql();
    insertSliceStmt = CreatPreparedStatement(sql);
    sql = JsonSqlConstant::GetInsertFlowSql();
    insertFlowStmt = CreatPreparedStatement(sql);
    sql = JsonSqlConstant::GetInsertCounterql();
    insertCounterStmt = CreatPreparedStatement(sql);
    if (insertSliceStmt == nullptr || insertFlowStmt == nullptr || insertCounterStmt == nullptr) {
        ServerLog::Error("Failed to prepare slice statement.");
        return false;
    }
    return true;
}

bool JsonTraceDatabase::InitProcessThreadStmt()
{
    std::string sql = UPDATE_PROCESS_NAME_SQL;
    updateProcessNameStmt = CreatPreparedStatement(sql);
    sql = UPDATE_PROCESS_LABLE_SQL;
    updateProcessLabelStmt = CreatPreparedStatement(sql);
    sql = UPDATE_PROCESS_SORTINDEX_SQL;
    updateProcessSortIndexStmt = CreatPreparedStatement(sql);
    sql = UPDATE_THREAD_INFO_SQL;
    updateThreadInfoStmt = CreatPreparedStatement(sql);
    sql = UPDATE_THREAD_NAME_SQL;
    updateThreadNameStmt = CreatPreparedStatement(sql);
    sql = UPDATE_THREAD_SORTINDEX_SQL;
    updateThreadSortIndexStmt = CreatPreparedStatement(sql);
    sql = SIMULATION_UPDATE_THREAD_NAME_SQL;
    simulationInsertThreadNameStmt = CreatPreparedStatement(sql);
    if (updateProcessNameStmt == nullptr || updateProcessLabelStmt == nullptr ||
        updateProcessSortIndexStmt == nullptr || updateThreadInfoStmt == nullptr || updateThreadNameStmt == nullptr ||
        updateThreadSortIndexStmt == nullptr || simulationInsertThreadNameStmt == nullptr) {
        ServerLog::Error("Failed to prepare process and thread statement.");
        return false;
    }
    return true;
}

void JsonTraceDatabase::ReleaseStmt()
{
    if (!initStmt) {
        return;
    }
    initStmt = false;
    // stmt对象需要在关闭数据库之前释放
    insertSliceStmt = nullptr;
    updateProcessNameStmt = nullptr;
    updateProcessLabelStmt = nullptr;
    updateProcessSortIndexStmt = nullptr;
    updateThreadInfoStmt = nullptr;
    updateThreadNameStmt = nullptr;
    updateThreadSortIndexStmt = nullptr;
    insertFlowStmt = nullptr;
    insertCounterStmt = nullptr;
    simulationInsertThreadNameStmt = nullptr;
}

bool JsonTraceDatabase::SetConfig()
{
    if (!isOpen) {
        ServerLog::Error("Failed to set config. Database is not open.");
        return false;
    }
    std::string dbVersion = GetDataBaseVersion();
    std::unique_lock<std::recursive_mutex> lock(mutex);
    // PRAGMA case_sensitive_like=1; 设置数据库大小写敏感。
    return ExecSql("PRAGMA synchronous = OFF; PRAGMA journal_mode = MEMORY; "
        "PRAGMA case_sensitive_like=1; PRAGMA user_version = " +
        dbVersion + ";");
}

bool JsonTraceDatabase::CreateTable()
{
    if (!isOpen) {
        ServerLog::Error("Failed to set config. Database is not open.");
        return false;
    }
    std::string sql = CREATE_TABLE_SQL;
    std::unique_lock<std::recursive_mutex> lock(mutex);
    return ExecSql(sql);
}

bool JsonTraceDatabase::DropTable()
{
    std::vector<std::string> tables = { sliceTable, threadTable, processTable, flowTable, counterTable };
    std::unique_lock<std::recursive_mutex> lock(mutex);
    return DropSomeTables(tables);
}

bool JsonTraceDatabase::CreateIndex()
{
    auto start = std::chrono::system_clock::now();
    if (!isOpen) {
        ServerLog::Error("Failed to creat index. Database is not open.");
        return false;
    }
    std::string sql = CREATE_INDEX_SQL;
    std::unique_lock<std::recursive_mutex> lock(mutex);
    ExecSql(sql);
    auto dur = std::chrono::duration<double, std::milli>(std::chrono::system_clock::now() - start);
    ServerLog::Info("CreateIndex end. time:", dur.count());
    return true;
}

bool JsonTraceDatabase::InsertSlice(const Trace::Slice &event)
{
    sliceCache.emplace_back(event);
    if (sliceCache.size() == CACHE_SIZE) {
        InsertSliceList(sliceCache);
        sliceCache.clear();
    }
    return true;
}


bool JsonTraceDatabase::InsertSliceList(const std::vector<Trace::Slice> &eventList)
{
    std::unique_ptr<SqlitePreparedStatement> stmt;
    std::unique_ptr<SqlitePreparedStatement> &refStmt = (eventList.size() == CACHE_SIZE) ? insertSliceStmt : stmt;
    if (refStmt == nullptr) {
        refStmt = GetSliceStmt(eventList.size()); // 数据长度不是预设的长度则新建一个对象
    } else {
        refStmt->Reset(); // 数据长度是预设长度，需要Reset后再使用
    }
    if (refStmt == nullptr) {
        ServerLog::Error("Failed to get slice stmt.");
        return false;
    }
    for (const auto &event : eventList) {
        refStmt->BindParams(event.ts, event.dur, event.name, event.trackId, event.cat, event.args, event.cname,
            event.end);
    }
    std::unique_lock<std::recursive_mutex> lock(mutex);
    if (!refStmt->Execute()) {
        ServerLog::Error("Insert slice data fail. ", refStmt->GetErrorMessage());
        return false;
    }
    return true;
}

std::unique_ptr<SqlitePreparedStatement> JsonTraceDatabase::GetSliceStmt(uint64_t paramLen)
{
    std::string sql = "INSERT INTO " + sliceTable +
        " (timestamp, duration, name, track_id, cat, args, cname, end_time) VALUES (?,?,?,?,?,?,?,?)";
    for (int i = 0; i < paramLen - 1; ++i) {
        sql.append(",(?,?,?,?,?,?,?,?)");
    }
    return CreatPreparedStatement(sql);
}

bool JsonTraceDatabase::UpdateProcessName(const Trace::MetaData &event)
{
    if (updateProcessNameStmt == nullptr) {
        ServerLog::Error("Update process name fail. ");
        return false;
    }
    updateProcessNameStmt->Reset();
    std::unique_lock<std::recursive_mutex> lock(mutex);
    if (!updateProcessNameStmt->Execute(event.pid, event.args.name)) {
        ServerLog::Error("Update process name fail. ", updateProcessNameStmt->GetErrorMessage());
        return false;
    }
    return true;
}

bool JsonTraceDatabase::UpdateProcessLabel(const Trace::MetaData &event)
{
    if (updateProcessLabelStmt == nullptr) {
        ServerLog::Error("Update process label fail. ");
        return false;
    }
    updateProcessLabelStmt->Reset();
    std::unique_lock<std::recursive_mutex> lock(mutex);
    if (!updateProcessLabelStmt->Execute(event.pid, event.args.labels)) {
        ServerLog::Error("Update process label fail. ", updateProcessLabelStmt->GetErrorMessage());
        return false;
    }
    return true;
}

bool JsonTraceDatabase::UpdateProcessSortIndex(const Trace::MetaData &event)
{
    if (updateProcessSortIndexStmt == nullptr) {
        ServerLog::Error("Update process sort index fail. ");
        return false;
    }
    updateProcessSortIndexStmt->Reset();
    std::unique_lock<std::recursive_mutex> lock(mutex);
    if (!updateProcessSortIndexStmt->Execute(event.pid, event.args.sortIndex)) {
        ServerLog::Error("Update process sort index fail. ", updateProcessSortIndexStmt->GetErrorMessage());
        return false;
    }
    return true;
}

bool JsonTraceDatabase::AddThreadCache(const std::tuple<int64_t, std::string, std::string> &threadInfo)
{
    threadInfoCache.insert(threadInfo);
    return true;
}

bool JsonTraceDatabase::AddSimulationThreadCache(const Trace::ThreadEvent &event)
{
    simulationThreadInfoCache.insert(event);
    return true;
}

bool JsonTraceDatabase::AddSimulationProcessCache(const Trace::ProcessEvent &event)
{
    simulationProcessInfoCache.insert(event);
    return true;
}

bool JsonTraceDatabase::InsertThreadList(const std::set<std::tuple<int64_t, std::string, std::string>> &threadList)
{
    for (const auto &item : threadList) {
        if (!UpdateThreadInfo(item)) {
            ServerLog::Error("Update thread name fail. pid: ", std::get<2>(item), " tid: ", std::get<1>(item)); // 第2个
            return false;
        }
    }
    return true;
}

bool JsonTraceDatabase::InsertSimulationThreadList()
{
    if (simulationInsertThreadNameStmt == nullptr) {
        ServerLog::Error("Insert thread info fail. ");
        return false;
    }
    for (const auto &item : simulationThreadInfoCache) {
        simulationInsertThreadNameStmt->Reset();
        std::unique_lock<std::recursive_mutex> lock(mutex);
        if (!simulationInsertThreadNameStmt->Execute(item.trackId, item.tid, item.pid, item.threadName,
            item.threadSortIndex)) {
            ServerLog::Error("Insert thread info fail. ", simulationInsertThreadNameStmt->GetErrorMessage());
            return false;
        }
    }
    return true;
}

bool JsonTraceDatabase::InsertSimulationProcessList()
{
    if (updateProcessNameStmt == nullptr) {
        ServerLog::Error("Update process info fail. ");
        return false;
    }
    for (const auto &item : simulationProcessInfoCache) {
        updateProcessNameStmt->Reset();
        std::unique_lock<std::recursive_mutex> lock(mutex);
        if (!updateProcessNameStmt->Execute(item.pid, item.processName)) {
            ServerLog::Error("Update process info fail. ", updateProcessNameStmt->GetErrorMessage());
            return false;
        }
    }
    return true;
}

bool JsonTraceDatabase::UpdateThreadInfo(const std::tuple<int64_t, std::string, std::string> &thread)
{
    if (updateThreadInfoStmt == nullptr) {
        ServerLog::Error("Update thread info fail. ");
        return false;
    }
    updateThreadInfoStmt->Reset();
    std::unique_lock<std::recursive_mutex> lock(mutex);
    if (!updateThreadInfoStmt->Execute(std::get<0>(thread), std::get<1>(thread), std::get<2>(thread))) { // 第2个
        ServerLog::Error("Update thread info fail. ", updateThreadInfoStmt->GetErrorMessage());
        return false;
    }
    return true;
}

bool JsonTraceDatabase::UpdateThreadName(const Trace::MetaData &event)
{
    if (updateThreadNameStmt == nullptr) {
        ServerLog::Error("Update thread name fail. ");
        return false;
    }
    updateThreadNameStmt->Reset();
    std::unique_lock<std::recursive_mutex> lock(mutex);
    if (!updateThreadNameStmt->Execute(event.trackId, event.tid, event.pid, event.args.name)) {
        ServerLog::Error("Update thread name fail. ", updateThreadNameStmt->GetErrorMessage());
        return false;
    }
    return true;
}

bool JsonTraceDatabase::UpdateThreadSortIndex(const Trace::MetaData &event)
{
    if (updateThreadSortIndexStmt == nullptr) {
        ServerLog::Error("Update thread sort index fail. ");
        return false;
    }
    updateThreadSortIndexStmt->Reset();
    std::unique_lock<std::recursive_mutex> lock(mutex);
    if (!updateThreadSortIndexStmt->Execute(event.trackId, event.args.sortIndex)) {
        ServerLog::Error("Update thread sort index fail. ", updateThreadSortIndexStmt->GetErrorMessage());
        return false;
    }
    return true;
}

bool JsonTraceDatabase::InsertFlow(const Trace::Flow &event)
{
    flowCache.emplace_back(event);
    if (flowCache.size() == CACHE_SIZE) {
        InsertFlowList(flowCache);
        flowCache.clear();
    }
    return true;
}

bool JsonTraceDatabase::InsertFlowList(const std::vector<Trace::Flow> &eventList)
{
    std::unique_ptr<SqlitePreparedStatement> stmt;
    std::unique_ptr<SqlitePreparedStatement> &refStmt = (eventList.size() == CACHE_SIZE) ? insertFlowStmt : stmt;
    if (refStmt == nullptr) {
        refStmt = GetFlowStmt(eventList.size()); // 数据长度不是预设的长度则新建一个对象
    } else {
        refStmt->Reset(); // 数据长度是预设长度，需要Reset后再使用
    }
    if (refStmt == nullptr) {
        ServerLog::Error("Failed to get flow stmt.");
        return false;
    }
    for (const auto &event : eventList) {
        refStmt->BindParams(event.flowId, event.name, event.trackId, event.ts, event.cat, event.type);
    }
    std::unique_lock<std::recursive_mutex> lock(mutex);
    if (!refStmt->Execute()) {
        ServerLog::Error("Insert flow fail. ", refStmt->GetErrorMessage());
        return false;
    }
    return true;
}

std::unique_ptr<SqlitePreparedStatement> JsonTraceDatabase::GetFlowStmt(uint64_t paramLen)
{
    std::string sql =
        "INSERT INTO " + flowTable + " (flow_id, name, track_id, timestamp, cat, type)" + " VALUES (?,?,?,?,?,?)";
    for (int i = 0; i < paramLen - 1; ++i) {
        sql.append(",(?,?,?,?,?,?)");
    }
    return CreatPreparedStatement(sql);
}

bool JsonTraceDatabase::InsertCounter(const Trace::Counter &event)
{
    counterCache.emplace_back(event);
    if (counterCache.size() == CACHE_SIZE) {
        InsertCounterList(counterCache);
        counterCache.clear();
    }
    return true;
}

bool JsonTraceDatabase::InsertCounterList(const std::vector<Trace::Counter> &eventList)
{
    std::unique_ptr<SqlitePreparedStatement> stmt;
    std::unique_ptr<SqlitePreparedStatement> &refStmt = (eventList.size() == CACHE_SIZE) ? insertCounterStmt : stmt;
    if (refStmt == nullptr) {
        refStmt = GetCounterStmt(eventList.size()); // 数据长度不是预设的长度则新建一个对象
    } else {
        refStmt->Reset(); // 数据长度是预设长度，需要Reset后再使用
    }
    if (refStmt == nullptr) {
        ServerLog::Error("Failed to get counter stmt.");
        return false;
    }
    for (const auto &event : eventList) {
        refStmt->BindParams(event.name, event.pid, event.ts, event.cat, event.args);
    }
    std::unique_lock<std::recursive_mutex> lock(mutex);
    if (!refStmt->Execute()) {
        ServerLog::Error("Insert counter data fail. ", sqlite3_errmsg(db));
        return false;
    }
    return true;
}

std::unique_ptr<SqlitePreparedStatement> JsonTraceDatabase::GetCounterStmt(uint64_t paramLen)
{
    std::string sql = "INSERT INTO " + counterTable + " (name, pid, timestamp, cat, args)" + " VALUES (?,?,?,?,?)";
    for (int i = 0; i < paramLen - 1; ++i) {
        sql.append(",(?,?,?,?,?)");
    }
    return CreatPreparedStatement(sql);
}

void JsonTraceDatabase::SimulationUpdateProcessSortIndex()
{
    std::vector<Protocol::SimpleSlice> simpleSliceVec;
    std::string queryAllProcess = "select pid FROM " + processTable + " ORDER BY process_name, pid;";
    auto processStmt = CreatPreparedStatement(queryAllProcess);
    if (processStmt == nullptr) {
        ServerLog::Error("SimulationUpdateProcessSortIndex. Failed to prepare sql.", GetLastError());
        return;
    }
    auto processResultSet = processStmt->ExecuteQuery();
    if (processResultSet == nullptr) {
        ServerLog::Error("SimulationUpdateProcessSortIndex. Failed to get result set.", processStmt->GetErrorMessage());
        return;
    }
    uint32_t order = 0;
    while (processResultSet->Next()) {
        Trace::MetaData event;
        event.pid = processResultSet->GetString("pid");
        event.args.sortIndex = ++order;
        UpdateProcessSortIndex(event);
    }
}

void JsonTraceDatabase::UpdateSimulationDepthWithNoOverlap()
{
    UpdateSimulationDepthByCodeWithNoOverlap();
}

void JsonTraceDatabase::UpdateSimulationDepthByCodeWithNoOverlap()
{
    Timer timer("UpdateSimulationDepthByCodeWithNoOverlap");
    std::vector<int32_t> trackIdList = QueryAllTrackId();
    if (std::empty(trackIdList)) {
        return;
    }
    ServerLog::Info("trackIdList size: ", trackIdList.size());
    std::vector<Protocol::SimpleSlice> rowThreadTraceVec;
    const uint32_t vectorSize = 5000000;
    rowThreadTraceVec.reserve(vectorSize);
    for (const auto &item : trackIdList) {
        rowThreadTraceVec.clear();
        QueryAllSliceByTrackId(item, rowThreadTraceVec);
        if (std::empty(rowThreadTraceVec)) {
            continue;
        }
        importActionAnalyzerPtr->UpdateAllSimulationSliceDepthWithNoOverlap(rowThreadTraceVec, item);
    }
}

void JsonTraceDatabase::QueryAllSliceByTrackId(const int32_t &trackId,
    std::vector<Protocol::SimpleSlice> &simpleSliceVec)
{
    std::string querySliceByTrackId = QUERY_SLICE_BY_TRACKID_SQL;
    auto sliceStmt = CreatPreparedStatement(querySliceByTrackId);
    if (sliceStmt == nullptr) {
        ServerLog::Error("querySliceByTrackId. Failed to prepare sql.", GetLastError());
        return;
    }
    auto sliceResultSet = sliceStmt->ExecuteQuery(trackId);
    if (sliceResultSet == nullptr) {
        ServerLog::Error("querySliceByTrackId. Failed to get result set.", sliceStmt->GetErrorMessage());
        return;
    }
    while (sliceResultSet->Next()) {
        SimpleSlice simpleSlice;
        simpleSlice.id = sliceResultSet->GetInt64("id");
        simpleSlice.timestamp = sliceResultSet->GetUint64("timestamp");
        simpleSlice.endTime = sliceResultSet->GetUint64("endTime");
        simpleSliceVec.emplace_back(std::move(simpleSlice));
    }
}

std::vector<int32_t> JsonTraceDatabase::QueryAllTrackId()
{
    std::vector<int32_t> trackIdList;
    std::string allTrackIdSql = QUERY_ALL_TRACKID_SQL;
    auto stmt = CreatPreparedStatement(allTrackIdSql);
    if (stmt == nullptr) {
        ServerLog::Error("allTrackIdSql. Failed to prepare sql.", GetLastError());
        return trackIdList;
    }
    auto resultSet = stmt->ExecuteQuery();
    if (resultSet == nullptr) {
        ServerLog::Error("allTrackIdSql. Failed to get result set.", stmt->GetErrorMessage());
        return trackIdList;
    }
    while (resultSet->Next()) {
        trackIdList.emplace_back(resultSet->GetInt64("trackId"));
    }
    return trackIdList;
}

bool JsonTraceDatabase::QueryThreadTraces(const Protocol::UnitThreadTracesParams &requestParams,
    Protocol::UnitThreadTracesBody &responseBody, uint64_t minTimestamp, int64_t traceId)
{
    std::vector<Protocol::RowThreadTrace> rowThreadTraceVec =
        QuerySliceByCondition(requestParams, minTimestamp, traceId);
    for (auto &item : rowThreadTraceVec) {
        Protocol::ThreadTraces threadTraces{};
        threadTraces.id = std::to_string(item.id);
        threadTraces.name = item.name;
        threadTraces.duration = item.duration;
        threadTraces.startTime = item.startTime;
        threadTraces.endTime = item.startTime + item.duration;
        threadTraces.depth = item.depth;
        threadTraces.threadId = requestParams.threadId;
        threadTraces.cname = item.cname;
        while (responseBody.data.size() <= item.depth) {
            responseBody.data.emplace_back();
        }
        responseBody.data[item.depth].emplace_back(threadTraces);
    }
    return true;
}
std::vector<RowThreadTrace> JsonTraceDatabase::QuerySliceByCondition(const UnitThreadTracesParams &requestParams,
    uint64_t minTimestamp, int64_t traceId)
{
    Timer timer("JsonTraceDatabase::QuerySliceByCondition");
    constexpr int minCacheSizeLimit = 200000;
    std::string sliceCacheKey =
        requestParams.cardId + "_" + std::to_string(traceId) + "_JsonTraceDatabase::QuerySliceByCondition";
    std::vector<CacheSlice> cacheSlices = CacheManager::Instance().Get(sliceCacheKey);
    if (std::empty(cacheSlices)) {
        QueryAllSliceInRangeByTrackId(traceId, cacheSlices);
        if (cacheSlices.size() > minCacheSizeLimit) {
            CacheManager::Instance().Put(sliceCacheKey, cacheSlices);
        }
    }
    std::set<int64_t> ids =
        sliceAnalyzerPtr->ComputeResultIds(requestParams.startTime, requestParams.endTime, minTimestamp, cacheSlices);
    std::vector<RowThreadTrace> ans = QuerySliceByIdList(minTimestamp, traceId, ids);
    ServerLog::Info("Ans Size is: ", ans.size());
    return ans;
}

std::vector<RowThreadTrace> JsonTraceDatabase::QuerySliceByIdList(uint64_t minTimestamp, int64_t traceId,
    std::set<int64_t> &ids)
{
    Timer timer3("JsonTraceDatabase::QuerySliceByIdList");
    std::string sliceSql = JsonSqlConstant::GetSliceByIdListSql(ids.size());
    std::vector<RowThreadTrace> ans;
    auto sliceStem = CreatPreparedStatement(sliceSql);
    if (sliceStem == nullptr) {
        ServerLog::Error("Fail to QuerySliceByIdList.", sqlite3_errmsg(db));
        return ans;
    }
    for (const auto &item : ids) {
        sliceStem->BindParams(item);
    }
    auto sliceResultSet = sliceStem->ExecuteQuery();
    if (sliceResultSet == nullptr) {
        ServerLog::Error("QuerySliceByIdList. Failed to get result set.", sliceStem->GetErrorMessage());
        return ans;
    }
    std::unordered_map<uint64_t, int32_t> depthCache =
        SliceDepthCacheManager::Instance().GetSliceDepthCacheStructByTrackId(traceId).sliceIdAndDepthMap;
    while (sliceResultSet->Next()) {
        RowThreadTrace rowThreadTrace{};
        rowThreadTrace.id = sliceResultSet->GetInt64("id");
        rowThreadTrace.startTime = sliceResultSet->GetUint64("timestamp") - minTimestamp;
        rowThreadTrace.duration = sliceResultSet->GetUint64("end_time") - rowThreadTrace.startTime - minTimestamp;
        rowThreadTrace.name = sliceResultSet->GetString("name");
        rowThreadTrace.traceId = traceId;
        rowThreadTrace.cname = sliceResultSet->GetString("cname");
        rowThreadTrace.depth = depthCache[rowThreadTrace.id];
        ans.emplace_back(rowThreadTrace);
    }
    std::sort(ans.begin(), ans.end(), std::less<RowThreadTrace>());
    return ans;
}

void JsonTraceDatabase::QueryAllSliceInRangeByTrackId(int64_t &traceId, std::vector<CacheSlice> &cacheSlices)
{
    Timer timer("JsonTraceDatabase::QueryAllSliceInRangeByTrackId");
    // 此处sql需全部走索引且禁止触发回表
    std::string sql = QUERY_ALL_SLICE_IN_RANGE_BY_TRACKID_SQL;
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryThreadTraces. Failed to prepare sql.", GetLastError());
        return;
    }
    auto resultSet = stmt->ExecuteQuery(traceId);
    if (resultSet == nullptr) {
        ServerLog::Error("QueryThreadTraces. Failed to get result set.", stmt->GetErrorMessage());
        return;
    }
    std::unordered_map<uint64_t, int32_t> depthCache =
        SliceDepthCacheManager::Instance().GetSliceDepthCacheStructByTrackId(traceId).sliceIdAndDepthMap;
    while (resultSet->Next()) {
        CacheSlice cacheSlice{};
        cacheSlice.id = resultSet->GetInt64("id");
        cacheSlice.timestamp = resultSet->GetUint64("timestamp");
        cacheSlice.duration = resultSet->GetUint64("end_time") - cacheSlice.timestamp;
        cacheSlice.depth = depthCache[cacheSlice.id];
        cacheSlices.emplace_back(cacheSlice);
    }
    std::sort(cacheSlices.begin(), cacheSlices.end(), std::less<CacheSlice>());
}

bool JsonTraceDatabase::QueryThreadTracesSummary(const Protocol::UnitThreadTracesSummaryParams &requestParams,
    Protocol::UnitThreadTracesSummaryBody &responseBody, uint64_t minTimestamp)
{
    Timer timer("JsonTraceDatabase::QueryThreadTracesSummary");
    const int64_t maxDataCount = 30000;
    uint64_t unitTime = (requestParams.endTime - requestParams.startTime) / maxDataCount;
    unitTime = unitTime <= 0 ? 1 : unitTime;
    std::vector<uint64_t> trackIds = QueryAllTrackIdsByPid(requestParams.processId);
    std::string sql = JsonSqlConstant::GetSummarySliceSql(trackIds.size());
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryThreadTraces. Failed to prepare sql.", GetLastError());
        return false;
    }
    for (const auto &item : trackIds) {
        stmt->BindParams(item);
    }
    auto resultSet = stmt->ExecuteQuery();
    if (resultSet == nullptr) {
        ServerLog::Error("QueryThreadTracesSummary. Failed to get result set.", stmt->GetErrorMessage());
        return false;
    }
    uint64_t tempStartTime = 0;
    uint64_t tempEndTime = 0;
    uint64_t maxTime = 0;
    while (resultSet->Next()) {
        uint64_t curStartTime = resultSet->GetUint64("timestamp");
        uint64_t curEndTime = resultSet->GetUint64("end_time");
        if (tempEndTime + unitTime >= curStartTime) {
            tempEndTime = tempEndTime > curEndTime ? tempEndTime : curEndTime;
            maxTime = tempEndTime;
            continue;
        }
        ThreadTracesSummary summary;
        summary.startTime = tempStartTime - minTimestamp;
        summary.duration = tempEndTime - tempStartTime;
        tempStartTime = curStartTime;
        tempEndTime = curEndTime;
        responseBody.data.emplace_back(summary);
    }
    ThreadTracesSummary summary;
    summary.startTime = tempStartTime - minTimestamp;
    summary.duration = tempEndTime - tempStartTime;
    responseBody.data.emplace_back(summary);
    ServerLog::Info("Summery Size is: ", responseBody.data.size());
    return true;
}

bool JsonTraceDatabase::QueryThreads(const Protocol::UnitThreadsParams &requestParams,
    Protocol::UnitThreadsBody &responseBody, uint64_t minTimestamp, int64_t traceId)
{
    uint64_t startTime = requestParams.startTime + minTimestamp;
    uint64_t endTime = requestParams.endTime + minTimestamp;
    Protocol::ExtremumTimestamp extremumTimestamp{};
    bool isSuccessQueryExtremumTime = QueryExtremumTimeOfFirstDepth(traceId, startTime, endTime, extremumTimestamp);
    if (!isSuccessQueryExtremumTime) {
        return false;
    }
    std::string sql = QUERY_UINT_THREAD_SQL;
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryThreads. Failed to prepare sql.");
        return false;
    }
    int index = bindStartIndex;
    auto resultSet = stmt->ExecuteQuery(traceId, extremumTimestamp.maxTimestamp, extremumTimestamp.minTimestamp);
    if (resultSet == nullptr) {
        ServerLog::Error("QueryThreads. Failed to get result set.", stmt->GetErrorMessage());
        return false;
    }
    std::unordered_map depthCache =
        SliceDepthCacheManager::Instance().GetSliceDepthCacheStructByTrackId(traceId).sliceIdAndDepthMap;
    std::vector<Protocol::SimpleSlice> simpleSliceVec;
    while (resultSet->Next()) {
        int col = resultStartIndex;
        Protocol::SimpleSlice simpleSlice{};
        uint64_t id = resultSet->GetUint64(col++);
        simpleSlice.timestamp = resultSet->GetUint64(col++);
        simpleSlice.duration = resultSet->GetUint64(col++);
        simpleSlice.endTime = resultSet->GetUint64(col++);
        simpleSlice.name = resultSet->GetString(col++);
        simpleSlice.depth = depthCache[id];
        simpleSliceVec.emplace_back(simpleSlice);
    }
    std::sort(simpleSliceVec.begin(), simpleSliceVec.end(), std::less<Protocol::SimpleSlice>());
    // process data
    if (simpleSliceVec.empty()) {
        responseBody.emptyFlag = true;
        return true;
    }
    std::map<std::string, uint64_t> selfTimeKeyValue;
    CalculateSelfTime(simpleSliceVec, selfTimeKeyValue, startTime, endTime);
    auto nRows = TraceDatabaseHelper::ThreadsInfoFilter(simpleSliceVec, startTime, endTime);
    TraceDatabaseHelper::ReduceThread(nRows, selfTimeKeyValue, responseBody);
    return true;
}

bool JsonTraceDatabase::QueryExtremumTimeOfFirstDepth(int64_t trackId, uint64_t startTime, uint64_t endTime,
    Protocol::ExtremumTimestamp &extremumTimestamp)
{
    std::string sql = QUERY_EXTREMETIME_OF_FIRST_DEPTH_SQL;
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryExtremumTimeOfFirstDepth. Failed to prepare sql.");
        return false;
    }
    auto resultSet = stmt->ExecuteQuery(trackId, endTime, startTime);
    if (resultSet == nullptr) {
        ServerLog::Error("QueryExtremumTimeOfFirstDepth. Failed to get result set.", stmt->GetErrorMessage());
        return false;
    }
    while (resultSet->Next()) {
        extremumTimestamp.minTimestamp = resultSet->GetUint64("minTimestamp");
        extremumTimestamp.maxTimestamp = resultSet->GetUint64("maxTimestamp");
    }
    return true;
}


void JsonTraceDatabase::CalculateSelfTime(std::vector<Protocol::SimpleSlice> &rows,
    std::map<std::string, uint64_t> &selfTimeKeyValue, uint64_t startTime, uint64_t endTime)
{
    int length = rows.size();
    // offset变量用来优化性能
    int offset = 0;
    for (int i = 0; i < length; i++) {
        int32_t curDepth = rows[i].depth;
        uint64_t selfTime = rows[i].duration;
        uint64_t curSliceStartTime = rows[i].timestamp;
        uint64_t curSliceEndTime = rows[i].endTime;
        uint64_t tempStartTime = 0;
        uint64_t tempEndTime = 0;
        for (int j = offset; j < length; ++j) {
            Protocol::SimpleSlice item = rows[j];
            if (item.timestamp < curSliceStartTime) {
                continue;
            }
            if (item.depth < curDepth + 1) {
                continue;
            }
            if (item.depth > curDepth + 1) {
                break;
            }
            if (item.endTime > curSliceEndTime) {
                break;
            }
            if (item.timestamp > tempEndTime) {
                selfTime = selfTime - (tempEndTime - tempStartTime);
                tempStartTime = item.timestamp;
                tempEndTime = item.endTime;
                offset = j;
                continue;
            }
            tempEndTime = tempEndTime < item.endTime ? item.endTime : tempEndTime;
            offset = j;
        }
        selfTime = selfTime - (tempEndTime - tempStartTime);
        AddData(selfTimeKeyValue, rows[i].name, selfTime);
    }
}

void JsonTraceDatabase::AddData(std::map<std::string, uint64_t> &selfTimeKeyValue, const std::string &name,
    uint64_t tmpSelfTime)
{
    if (selfTimeKeyValue.find(name) != selfTimeKeyValue.end()) {
        selfTimeKeyValue.at(name) = selfTimeKeyValue.at(name) + tmpSelfTime;
    } else {
        selfTimeKeyValue.emplace(name, tmpSelfTime);
    }
}

bool JsonTraceDatabase::QueryThreadDetail(const Protocol::ThreadDetailParams &requestParams,
    Protocol::UnitThreadDetailBody &responseBody, uint64_t minTimestamp, int64_t trackId)
{
    std::string sql = QUERY_SLICE_DETAIL_SQL;
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryThreadDetail. Failed to prepare sql.");
        return false;
    }
    auto resultSet = stmt->ExecuteQuery(requestParams.id, trackId, requestParams.startTime + minTimestamp);
    if (resultSet == nullptr) {
        ServerLog::Error("QueryThreadDetail. Failed to get result set.", stmt->GetErrorMessage());
        return false;
    }
    std::unordered_map<uint64_t, int32_t> depthCache =
        SliceDepthCacheManager::Instance().GetSliceDepthCacheStructByTrackId(trackId).sliceIdAndDepthMap;
    std::vector<SliceDto> sliceDtoVec;
    while (resultSet->Next()) {
        SliceDto sliceDto{};
        sliceDto.id = resultSet->GetUint64("id");
        sliceDto.timestamp = resultSet->GetUint64("timestamp");
        sliceDto.duration = resultSet->GetUint64("duration");
        sliceDto.name = resultSet->GetString("name");
        sliceDto.trackId = resultSet->GetInt64("track_id");
        sliceDto.cat = resultSet->GetString("cat");
        sliceDto.args = resultSet->GetString("args");
        sliceDto.depth = depthCache[sliceDto.id];
        sliceDtoVec.emplace_back(sliceDto);
    }
    if (sliceDtoVec.empty()) {
        ServerLog::Error("select slice error!");
        return false;
    }
    uint64_t selfTime = ComputeSingleSliceSelfTime(requestParams, trackId, sliceDtoVec);
    const KernelShapesDataDto &shapesDataDto = QueryKernelShapes(sliceDtoVec);
    responseBody.emptyFlag = false;
    responseBody.data.selfTime = selfTime;
    responseBody.data.args = sliceDtoVec[0].args;
    responseBody.data.title = sliceDtoVec[0].name;
    responseBody.data.duration = sliceDtoVec[0].duration;
    responseBody.data.cat = sliceDtoVec[0].cat;
    responseBody.data.inputShapes = shapesDataDto.inputShapes;
    responseBody.data.inputDataTypes = shapesDataDto.inputDataTypes;
    responseBody.data.inputFormats = shapesDataDto.inputFormats;
    responseBody.data.outputShapes = shapesDataDto.outputShapes;
    responseBody.data.outputDataTypes = shapesDataDto.outputDataTypes;
    responseBody.data.outputFormats = shapesDataDto.outputFormats;
    return true;
}

uint64_t JsonTraceDatabase::ComputeSingleSliceSelfTime(const ThreadDetailParams &requestParams, int64_t trackId,
    std::vector<SliceDto> &sliceDtoVec)
{
    uint64_t selfTime = sliceDtoVec.at(0).duration;
    std::vector<std::pair<uint64_t, uint64_t>> nextDepthResult;
    QueryDurationFromSliceByTimeRange(requestParams, sliceDtoVec, nextDepthResult, trackId);
    if (nextDepthResult.empty()) {
        return 0;
    }
    uint64_t tempStartTime = 0;
    uint64_t tempEndTime = 0;
    // 兼容算子重叠的情况
    for (const auto &item : nextDepthResult) {
        if (item.first > tempEndTime) {
            selfTime = selfTime - (tempEndTime - tempStartTime);
            tempStartTime = item.first;
            tempEndTime = item.first + item.second;
            continue;
        }
        tempEndTime = tempEndTime < (item.first + item.second) ? (item.first + item.second) : tempEndTime;
    }
    selfTime = selfTime - (tempEndTime - tempStartTime);
    return selfTime;
}

bool JsonTraceDatabase::QueryDurationFromSliceByTimeRange(const Protocol::ThreadDetailParams &requestParams,
    const std::vector<SliceDto> &rows, std::vector<std::pair<uint64_t, uint64_t>> &nextDepthResult, int64_t trackId)
{
    if (rows.empty()) {
        ServerLog::Error("sliceDto array is empty!");
        return false;
    }
    std::string sql = QUERY_DURATION_FROM_SLICE_BY_TIME_RANGE_SQL;
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryDurationFromSliceByTimeRange. Failed to prepare sql.", sqlite3_errmsg(db));
        return false;
    }
    auto resultSet = stmt->ExecuteQuery(rows[0].timestamp + rows[0].duration, rows[0].timestamp, trackId);
    if (resultSet == nullptr) {
        ServerLog::Error("QueryDurationFromSliceByTimeRange. Failed to get result set.", stmt->GetErrorMessage());
        return false;
    }
    std::unordered_map<uint64_t, int32_t> depthCache =
        SliceDepthCacheManager::Instance().GetSliceDepthCacheStructByTrackId(trackId).sliceIdAndDepthMap;
    while (resultSet->Next()) {
        uint64_t id = resultSet->GetUint64("id");
        uint64_t startTime = resultSet->GetUint64("timestamp");
        uint64_t duration = resultSet->GetUint64("duration");
        int32_t depth = depthCache[id];
        if (depth == requestParams.depth + 1) {
            nextDepthResult.emplace_back(startTime, duration);
        }
    }
    return true;
}

KernelShapesDataDto JsonTraceDatabase::QueryKernelShapes(const std::vector<SliceDto> &param)
{
    KernelShapesDataDto kernelShapesDataDto;
    if (param.empty()) {
        ServerLog::Error("sliceDto array is empty!");
        return kernelShapesDataDto;
    }
    std::string sql = QUERY_KERNAL_SHAPE_SQL;
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryKernelShapes. Failed to prepare sql.", sqlite3_errmsg(db));
        return kernelShapesDataDto;
    }
    auto resultSet = stmt->ExecuteQuery(param[0].name, param[0].timestamp);
    if (resultSet == nullptr) {
        ServerLog::Error("QueryKernelShapes. Failed to get result set.", stmt->GetErrorMessage());
        return kernelShapesDataDto;
    }
    while (resultSet->Next()) {
        if (resultSet->GetString("accelerator_core") == hcclType) {
            break;
        }
        kernelShapesDataDto.inputShapes = resultSet->GetString("inputShapes");
        kernelShapesDataDto.inputDataTypes = resultSet->GetString("inputDataTypes");
        kernelShapesDataDto.inputFormats = resultSet->GetString("inputFormats");
        kernelShapesDataDto.outputShapes = resultSet->GetString("outputShapes");
        kernelShapesDataDto.outputDataTypes = resultSet->GetString("outputDataTypes");
        kernelShapesDataDto.outputFormats = resultSet->GetString("outputFormats");
    }
    return kernelShapesDataDto;
}

bool JsonTraceDatabase::QueryFlowDetail(const Protocol::UnitFlowParams &requestParams,
    Protocol::UnitFlowBody &responseBody, uint64_t minTimestamp)
{
    std::vector<FlowDetailDto> flowDetailVec = QuerySingleFlowDetail(requestParams.flowId, minTimestamp);
    return FlowDetailToResponse(flowDetailVec, minTimestamp, responseBody);
}

std::vector<FlowDetailDto> JsonTraceDatabase::QuerySingleFlowDetail(const std::string &flowId, uint64_t minTimestamp)
{
    std::string sql = QUERY_FLOW_BY_FLOWID_SQL;
    std::vector<FlowDetailDto> flowDetailVec;
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryFlowDetail. Failed to prepare sql.");
        return flowDetailVec;
    }
    auto resultSet = stmt->ExecuteQuery(flowId);
    if (resultSet == nullptr) {
        ServerLog::Error("QueryFlowDetail. Failed to get result set.", stmt->GetErrorMessage());
        return flowDetailVec;
    }
    while (resultSet->Next()) {
        FlowDetailDto flowDetailDto{};
        flowDetailDto.name = resultSet->GetString("name");
        flowDetailDto.cat = resultSet->GetString("cat");
        flowDetailDto.flowId = resultSet->GetString("flowId");
        flowDetailDto.flowTimestamp = resultSet->GetUint64("timestamp");
        flowDetailDto.type = resultSet->GetString("type");
        flowDetailDto.trackId = resultSet->GetInt64("trackId");
        flowDetailVec.emplace_back(flowDetailDto);
    }
    ServerLog::Info("flowDetailVec size is: ", flowDetailVec.size());
    std::map<uint64_t, std::pair<std::string, std::string>> threadMap = QueryAllThreadMap();
    for (auto &item : flowDetailVec) {
        std::vector<SimpleSlice> simpliceVec =
            QuerySimpleSliceByTimePoint(item.flowTimestamp, minTimestamp, item.trackId);
        flowAnalyzerPtr->ComputeSingleFlowDetail(simpliceVec, item);
        item.tid = threadMap[item.trackId].first;
        item.pid = threadMap[item.trackId].second;
    }
    return flowDetailVec;
}

std::map<uint64_t, std::pair<std::string, std::string>> JsonTraceDatabase::QueryAllThreadMap()
{
    std::string threadSql = QUERY_ALL_THREAD_SQL;
    auto threadStmt = CreatPreparedStatement(threadSql);
    std::map<uint64_t, std::pair<std::string, std::string>> threadMap;
    if (threadStmt == nullptr) {
        ServerLog::Error("QueryFlowDetail. Failed to prepare sql.");
        return threadMap;
    }
    auto threadSet = threadStmt->ExecuteQuery();
    if (threadSet == nullptr) {
        ServerLog::Error("QueryFlowDetail. Failed to get result set.", threadStmt->GetErrorMessage());
    }

    while (threadSet->Next()) {
        uint64_t trackId = threadSet->GetUint64("trackId");
        std::string tid = threadSet->GetString("tid");
        std::string pid = threadSet->GetString("pid");
        threadMap[trackId] = std::make_pair(tid, pid);
    }
    return threadMap;
}

bool JsonTraceDatabase::FlowDetailToResponse(const std::vector<FlowDetailDto> &flowDetailVec, uint64_t minTimestamp,
    Protocol::UnitFlowBody &responseBody)
{
    const static int FLOW_COUNT = 2; // from + to
    if (flowDetailVec.size() != FLOW_COUNT) {
        ServerLog::Warn("Flow detail size is ", flowDetailVec.size());
        return true;
    }
    responseBody.title = flowDetailVec[0].name;
    responseBody.cat = flowDetailVec[0].cat;
    responseBody.id = flowDetailVec[0].flowId;
    FlowDetailDto from(flowDetailVec[0]);
    FlowDetailDto to(flowDetailVec[1]);
    if (from.timestamp > to.timestamp) {
        from = flowDetailVec[1];
        to = flowDetailVec[0];
    }
    responseBody.from.id = from.id;
    responseBody.from.pid = from.pid;
    responseBody.from.tid = from.tid;
    responseBody.from.timestamp = from.timestamp - minTimestamp;
    responseBody.from.duration = from.duration;
    responseBody.from.depth = from.depth;
    responseBody.from.name = from.sliceName;
    responseBody.to.id = to.id;
    responseBody.to.pid = to.pid;
    responseBody.to.tid = to.tid;
    responseBody.to.timestamp = to.timestamp - minTimestamp;
    responseBody.to.duration = to.duration;
    responseBody.to.depth = to.depth;
    responseBody.to.name = to.sliceName;
    return true;
}

void JsonTraceDatabase::QueryFlowName(const Protocol::UnitFlowNameParams &requestParams,
    Protocol::UnitFlowNameBody &responseBody, uint64_t minTimestamp, uint64_t trackId)
{
    std::vector<FlowName> flowNameVec;
    flowNameVec =
        QueryFlowNameByTimeRange(requestParams.startTime + minTimestamp, requestParams.endTime + minTimestamp, trackId);
    std::vector<SimpleSlice> simpleSliceVec =
        QuerySimpleSliceByTimeRange(requestParams.startTime, requestParams.endTime, minTimestamp, trackId);
    std::vector<FlowName> flowNameRes = flowAnalyzerPtr->ComputeFlowBySliceVec(flowNameVec, simpleSliceVec);
    for (const auto &item : flowNameRes) {
        responseBody.flowDetail.emplace_back(item);
    }
}

std::vector<FlowName> JsonTraceDatabase::QueryFlowNameByTimeRange(uint64_t startTime, uint64_t endTime, int64_t trackId)
{
    std::vector<FlowName> flowNameVec;
    std::string sql = QUERY_FLOW_BY_TIME_RANGE_SQL;
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryFlowName. Failed to prepare sql.");
        return flowNameVec;
    }
    auto resultSet = stmt->ExecuteQuery(startTime, endTime, trackId);
    if (resultSet == nullptr) {
        ServerLog::Error("QueryFlowName. Failed to get result set.", stmt->GetErrorMessage());
        return flowNameVec;
    }
    while (resultSet->Next()) {
        FlowName flowName;
        flowName.title = resultSet->GetString("name");
        flowName.flowId = resultSet->GetString("flowId");
        flowName.type = resultSet->GetString("type");
        flowName.timestamp = resultSet->GetUint64("timestamp");
        flowNameVec.emplace_back(flowName);
    }
}

bool JsonTraceDatabase::QueryUintFlows(const Protocol::UnitFlowsParams &requestParams,
    Protocol::UnitFlowsBody &responseBody, uint64_t minTimestamp, uint64_t trackId)
{
    std::vector<FlowName> flowNameVec;
    flowNameVec =
        QueryFlowNameByTimeRange(requestParams.startTime + minTimestamp, requestParams.endTime + minTimestamp, trackId);
    std::vector<SimpleSlice> simpleSliceVec =
        QuerySimpleSliceByTimeRange(requestParams.startTime, requestParams.endTime, minTimestamp, trackId);
    std::set<std::string> flowIdSet;
    std::vector<FlowName> flowNameRes = flowAnalyzerPtr->ComputeFlowBySliceVec(flowNameVec, simpleSliceVec);
    for (const auto &item : flowNameRes) {
        flowIdSet.emplace(item.flowId);
    }
    std::map<std::string, std::vector<UnitSingleFlow>> flowMap;
    std::vector<UnitCatFlows> unitAllFlow;
    for (const auto &item : flowIdSet) {
        std::vector<FlowDetailDto> flowDetailVec = QuerySingleFlowDetail(item, minTimestamp);
        flowAnalyzerPtr->ComputeCategoryAndFlowMap(flowDetailVec, flowMap, minTimestamp);
    }
    for (const auto &item : flowMap) {
        UnitCatFlows unitCatFlows;
        unitCatFlows.cat = item.first;
        unitCatFlows.flows = item.second;
        unitAllFlow.emplace_back(unitCatFlows);
    }
    responseBody.unitAllFlows = unitAllFlow;
    return true;
}

std::vector<SimpleSlice> JsonTraceDatabase::QuerySimpleSliceByTimeRange(uint64_t startTime, uint64_t endTime,
    uint64_t minTimestamp, int64_t trackId)
{
    std::string sliceSql = QUERY_SLICE_BY_TIME_RANGE_SQL;
    auto sliceStmt = CreatPreparedStatement(sliceSql);
    std::vector<SimpleSlice> simpleSliceVec;
    if (sliceStmt == nullptr) {
        ServerLog::Error("QueryFlowName. Failed to prepare sql.");
        return simpleSliceVec;
    }
    auto sliceSet = sliceStmt->ExecuteQuery(startTime + minTimestamp, endTime + minTimestamp, trackId);
    if (sliceSet == nullptr) {
        ServerLog::Error("QuerySimpleSliceByTimeRange. Failed to get result set.", sliceStmt->GetErrorMessage());
        return simpleSliceVec;
    }
    while (sliceSet->Next()) {
        SimpleSlice simpleSlice;
        simpleSlice.timestamp = sliceSet->GetUint64("timestamp");
        simpleSlice.endTime = sliceSet->GetUint64("endTime");
        simpleSliceVec.emplace_back(simpleSlice);
    }
    return simpleSliceVec;
}

std::vector<SimpleSlice> JsonTraceDatabase::QuerySimpleSliceByTimePoint(uint64_t startTime, uint64_t minTimestamp,
    int64_t trackId)
{
    std::string sliceSql = QUERY_SLICE_BY_TIME_POINT_SQL;
    auto sliceStmt = CreatPreparedStatement(sliceSql);
    std::vector<SimpleSlice> simpleSliceVec;
    if (sliceStmt == nullptr) {
        ServerLog::Error("QueryFlowName. Failed to prepare sql.");
        return simpleSliceVec;
    }
    auto sliceSet = sliceStmt->ExecuteQuery(startTime, startTime, trackId);
    if (sliceSet == nullptr) {
        ServerLog::Error("QuerySimpleSliceByTimePoint. Failed to get result set.", sliceStmt->GetErrorMessage());
        return simpleSliceVec;
    }
    std::unordered_map<uint64_t, int32_t> depthCache =
        SliceDepthCacheManager::Instance().GetSliceDepthCacheStructByTrackId(trackId).sliceIdAndDepthMap;
    while (sliceSet->Next()) {
        SimpleSlice simpleSlice;
        uint64_t id = sliceSet->GetUint64("id");
        simpleSlice.id = id;
        simpleSlice.timestamp = sliceSet->GetUint64("timestamp");
        simpleSlice.endTime = sliceSet->GetUint64("endTime");
        simpleSlice.name = sliceSet->GetString("name");
        simpleSlice.depth = depthCache[id];
        simpleSliceVec.emplace_back(simpleSlice);
    }
    std::sort(simpleSliceVec.begin(), simpleSliceVec.end(), std::greater<SimpleSlice>());
    ServerLog::Info("simpleSliceVec size is: ", simpleSliceVec.size());
    return simpleSliceVec;
}

bool JsonTraceDatabase::QueryUnitsMetadata(const std::string &fileId,
    std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData)
{
    std::string sql = QUERY_UNITS_META_SQL;
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryUnitsMetadata failed!.");
        return false;
    }
    auto resultSet = stmt->ExecuteQuery();
    if (resultSet == nullptr) {
        ServerLog::Error("QueryUnitsMetadata. Failed to get result set.", stmt->GetErrorMessage());
        return false;
    }
    std::vector<MetaDataDto> metaDataVec;
    while (resultSet->Next()) {
        int col = resultStartIndex;
        MetaDataDto metaDataDto;
        metaDataDto.pid = resultSet->GetString("pid");
        metaDataDto.processName = resultSet->GetString("processName") + " (" + metaDataDto.pid + ")";
        metaDataDto.label = resultSet->GetString("label");
        metaDataDto.threadId = resultSet->GetString("tid");
        metaDataDto.threadName = resultSet->GetString("threadName");
        metaDataDto.name = resultSet->GetString("name");
        metaDataDto.args = resultSet->GetString("args");
        uint64_t trackId = resultSet->GetUint64("trackId");
        metaDataDto.maxDepth = SliceDepthCacheManager::Instance().QueryMaxDepthByTrackId(trackId) + 1;
        metaDataVec.emplace_back(metaDataDto);
    }
    ServerLog::Info("Query units meta data. size:", metaDataVec.size());
    MetaDataToResponse(metaDataVec, fileId, metaData);
    return true;
}

void JsonTraceDatabase::MetaDataToResponse(const std::vector<MetaDataDto> &metaDataVec, const std::string &fileId,
    std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData)
{
    std::optional<std::string> curPid;
    for (const auto &metaDataDto : metaDataVec) {
        if ((!curPid.has_value()) || metaDataDto.pid != curPid) {
            std::unique_ptr<Protocol::UnitTrack> process = std::make_unique<Protocol::UnitTrack>();
            process->type = "process";
            process->metaData.processName = metaDataDto.processName;
            process->metaData.label = metaDataDto.label;
            process->metaData.cardId = fileId;
            process->metaData.processId = metaDataDto.pid;
            metaData.emplace_back(std::move(process));
            curPid = metaDataDto.pid;
        }
        if (metaData.empty()) {
            continue;
        }
        std::unique_ptr<Protocol::UnitTrack> thread = std::make_unique<Protocol::UnitTrack>();
        if (metaDataDto.name.empty()) { // thread
            thread->type = "thread";
            thread->metaData.cardId = fileId;
            thread->metaData.processId = metaDataDto.pid;
            thread->metaData.threadId = metaDataDto.threadId;
            thread->metaData.threadName = metaDataDto.threadName;
            thread->metaData.maxDepth = metaDataDto.maxDepth;
        } else { // counter
            thread->type = "counter";
            thread->metaData.cardId = fileId;
            thread->metaData.processId = metaDataDto.pid;
            thread->metaData.threadName = metaDataDto.name;
            thread->metaData.dataType = GetCounterDataType(metaDataDto.args);
        }
        metaData.back()->children.emplace_back(std::move(thread));
    }
}

std::vector<std::string> JsonTraceDatabase::GetCounterDataType(const std::string &args)
{
    std::vector<std::string> type{};
    if (args.empty()) {
        return type;
    }
    rapidjson::Document document;
    try {
        document.Parse(args.c_str(), args.length());
    } catch (std::exception &e) {
        ServerLog::Error("Get counter data type. Failed to parse json. ", args, e.what());
        return type;
    }
    for (auto it = document.MemberBegin(); it != document.MemberEnd(); ++it) {
        if (it->name.IsString()) {
            type.emplace_back(it->name.GetString());
        } else {
            ServerLog::Warn("Counter data type is not string. args:", args);
        }
    }
    std::sort(type.begin(), type.end()); // 与metadata数据顺序一致，可能是因为使用json开源软件不一致
    return type;
}

bool JsonTraceDatabase::QueryExtremumTimestamp(uint64_t &min, uint64_t &max)
{
    std::string sql = QUERY_EXETREME_TIME_SQL;
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryExtremumTimestamp failed!.");
        return false;
    }
    auto resultSet = stmt->ExecuteQuery();
    if (resultSet == nullptr) {
        ServerLog::Error("QueryExtremumTimestamp. Failed to get result set.", stmt->GetErrorMessage());
        return false;
    }
    while (resultSet->Next()) {
        int col = resultStartIndex;
        min = resultSet->GetUint64("totalMinTimestamp");
        max = resultSet->GetUint64("totalMaxTimestamp");
    }
    return true;
}

void JsonTraceDatabase::CommitData()
{
    if (!sliceCache.empty()) {
        InsertSliceList(sliceCache);
        sliceCache.clear();
    }
    if (!flowCache.empty()) {
        InsertFlowList(flowCache);
        flowCache.clear();
    }
    if (!counterCache.empty()) {
        InsertCounterList(counterCache);
        counterCache.clear();
    }
    if (!threadInfoCache.empty()) {
        InsertThreadList(threadInfoCache);
        threadInfoCache.clear();
    }
    if (!simulationThreadInfoCache.empty()) {
        InsertSimulationThreadList();
        simulationThreadInfoCache.clear();
    }
    if (!simulationProcessInfoCache.empty()) {
        InsertSimulationProcessList();
        simulationProcessInfoCache.clear();
    }
}

int JsonTraceDatabase::SearchSliceNameCount(const Protocol::SearchCountParams &params)
{
    int32_t result = 0;
    std::string sql = JsonSqlConstant::GetSearchSliceNameCountSql(params.isMatchExact, params.isMatchCase);
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QuerySliceNameCount failed!.");
        return 0;
    }
    auto resultSet = stmt->ExecuteQuery(params.searchContent);
    if (resultSet == nullptr) {
        ServerLog::Error("SearchSliceNameCount. Failed to get result set.", stmt->GetErrorMessage());
        return 0;
    }
    if (resultSet->Next()) {
        result = resultSet->GetInt32(resultStartIndex);
    }
    return result;
}

bool JsonTraceDatabase::SearchSliceName(const Protocol::SearchSliceParams &params, int index, uint64_t minTimestamp,
    Protocol::SearchSliceBody &responseBody)
{
    std::string sql = JsonSqlConstant::GetSearchSliceNameSql(params.isMatchExact, params.isMatchCase);
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QuerySliceName failed!.");
        return false;
    }
    auto resultSet = stmt->ExecuteQuery(minTimestamp, params.searchContent, index);
    if (resultSet == nullptr) {
        ServerLog::Error("SearchSliceName. Failed to get result set.", stmt->GetErrorMessage());
        return false;
    }
    if (!resultSet->Next()) {
        return false;
    }
    int col = resultStartIndex;
    uint64_t id = resultSet->GetUint64("id");
    responseBody.id = std::to_string(id);
    responseBody.pid = resultSet->GetString("pid");
    responseBody.tid = resultSet->GetString("tid");
    responseBody.startTime = resultSet->GetUint64("startTime");
    responseBody.duration = resultSet->GetUint64("duration");
    uint64_t trackId = resultSet->GetInt32("trackId");
    std::unordered_map<uint64_t, int32_t> depthCache =
        SliceDepthCacheManager::Instance().GetSliceDepthCacheStructByTrackId(trackId).sliceIdAndDepthMap;
    responseBody.depth = depthCache[id];
    return true;
}

bool JsonTraceDatabase::QueryFlowCategoryList(std::vector<std::string> &categories)
{
    std::string sql = "SELECT cat FROM flow GROUP BY cat";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryFlowCategoryList failed!.");
        return false;
    }
    auto resultSet = stmt->ExecuteQuery();
    if (resultSet == nullptr) {
        ServerLog::Error("QueryFlowCategoryList. Failed to get result set.", stmt->GetErrorMessage());
        return false;
    }
    while (resultSet->Next()) {
        categories.emplace_back(resultSet->GetString(resultStartIndex));
    }
    return true;
}

std::vector<uint64_t> JsonTraceDatabase::QueryAllTrackIdsByPid(std::string pid)
{
    std::vector<uint64_t> trackIds;
    std::string sql = "Select track_id AS trackId from " + threadTable + " where pid = ? ;";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryAllTrackIdsByPid failed!.");
        return trackIds;
    }
    auto resultSet = stmt->ExecuteQuery(pid);
    if (resultSet == nullptr) {
        ServerLog::Error("QueryAllTrackIdsByPid. Failed to get result set.", stmt->GetErrorMessage());
        return trackIds;
    }
    while (resultSet->Next()) {
        int col = resultStartIndex;
        uint64_t trackId = resultSet->GetUint64(col++);
        trackIds.emplace_back(trackId);
    }
    return trackIds;
}

bool JsonTraceDatabase::QueryFlowCategoryEvents(FlowCategoryEventsParams &params, uint64_t minTimestamp,
    std::vector<std::unique_ptr<FlowEvent>> &flowDetailList)
{
    if (params.timePerPx == 0) {
        ServerLog::Error("QueryFlowCategoryEvents. timePerPx is zero.");
        return false;
    }
    std::string sql = QUERY_FLOWCATEGORY_EVENTS_SQL;
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryFlowCategoryEvents failed!.");
        return false;
    }
    auto resultSet = stmt->ExecuteQuery(minTimestamp, params.timePerPx * lowImage, params.category,
        params.startTime + minTimestamp, params.endTime + minTimestamp);
    if (resultSet == nullptr) {
        ServerLog::Error("QueryFlowCategoryEvents. Failed to get result set.", stmt->GetErrorMessage());
        return false;
    }
    std::vector<FlowCategoryEventsDto> flowEventsVec;
    while (resultSet->Next()) {
        int col = resultStartIndex;
        FlowCategoryEventsDto flowCategoryEventsDto;
        flowCategoryEventsDto.type = resultSet->GetString(col++);
        flowCategoryEventsDto.flowId = resultSet->GetString(col++);
        flowCategoryEventsDto.pid = resultSet->GetString(col++);
        flowCategoryEventsDto.tid = resultSet->GetString(col++);
        uint64_t sliceId = resultSet->GetInt64(col++);
        uint64_t sTrackId = resultSet->GetInt64(col++);
        flowCategoryEventsDto.timestamp = resultSet->GetUint64(col++);
        std::unordered_map<uint64_t, int32_t> depthCache =
            SliceDepthCacheManager::Instance().GetSliceDepthCacheStructByTrackId(sTrackId).sliceIdAndDepthMap;
        flowCategoryEventsDto.depth = depthCache[sliceId];
        flowEventsVec.emplace_back(flowCategoryEventsDto);
    }
    FlowEventsToResponse(flowEventsVec, params.category, flowDetailList);
    ServerLog::Info("Query flow category events. size:", flowDetailList.size());
    return true;
}

void JsonTraceDatabase::FlowEventsToResponse(const std::vector<FlowCategoryEventsDto> &flowEventsVec,
    const std::string &category, std::vector<std::unique_ptr<FlowEvent>> &flowDetailList)
{
    std::string curFlowId;
    FlowEventLocation location;
    FlowEventLocation *locationPtr = &location;
    for (const auto &flow : flowEventsVec) {
        std::string type = flow.type;
        std::string flowId = flow.flowId;
        if (type == LINE_START || flowId != curFlowId) {
            location.pid = flow.pid;
            location.tid = flow.tid;
            location.depth = flow.depth;
            location.timestamp = flow.timestamp;
            location.type = type;
            locationPtr = &location;
        } else if ((type == LINE_END || type == LINE_END_OPTIONAL) && flowId == curFlowId) {
            auto flowEvent = std::make_unique<FlowEvent>();
            flowEvent->category = category;
            flowEvent->from = *locationPtr;
            flowEvent->to.pid = flow.pid;
            flowEvent->to.tid = flow.tid;
            flowEvent->to.depth = flow.depth;
            flowEvent->to.timestamp = flow.timestamp;
            locationPtr = &(flowEvent->to);
            if (flowEvent->from.type == LINE_START) {
                flowDetailList.emplace_back(std::move(flowEvent));
            }
        }
        curFlowId = flowId;
    }
}

bool JsonTraceDatabase::QueryUnitCounter(Protocol::UnitCounterParams &params, uint64_t minTimestamp,
    std::vector<Protocol::UnitCounterData> &dataList)
{
    std::string sql = QUERY_UNIT_COUNTER_SQL;
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryUnitCounter failed!.");
        return false;
    }
    auto resultSet = stmt->ExecuteQuery(minTimestamp, params.pid, params.threadName, params.startTime, params.endTime);
    if (resultSet == nullptr) {
        ServerLog::Error("QueryUnitCounter. Failed to get result set.", stmt->GetErrorMessage());
        return false;
    }
    while (resultSet->Next()) {
        UnitCounterData unitCounterData;
        unitCounterData.timestamp = resultSet->GetUint64("startTime");
        unitCounterData.valueJsonStr = resultSet->GetString("args");
        dataList.emplace_back(unitCounterData);
    }
    return true;
}

bool JsonTraceDatabase::QueryComputeStatisticsData(const Protocol::SummaryStatisticParams &requestParams,
    Protocol::SummaryStatisticsBody &responseBody)
{
    std::string sql = JsonSqlConstant::GetComputeStatisticsSQL(requestParams.stepId);
    sqlite3_stmt *stmt = nullptr;
    int index = bindStartIndex;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("QueryComputeStatisticsData failed!. ", sqlite3_errmsg(db));
        return false;
    }
    if (!requestParams.stepId.empty() && requestParams.stepId != "ALL") {
        sqlite3_bind_text(stmt, index, requestParams.stepId.c_str(), requestParams.stepId.length(), SQLITE_TRANSIENT);
    }
    double totalDuration = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Protocol::SummaryStatisticsItem item;
        int col = resultStartIndex;
        item.duration = static_cast<double>(sqlite3_column_int64(stmt, col++));
        item.acceleratorCore = sqlite3_column_string(stmt, col++);
        totalDuration += item.duration;
        responseBody.summaryStatisticsItemList.push_back(item);
    }
    for (auto &item : responseBody.summaryStatisticsItemList) {
        item.utilization = totalDuration > 0 ? item.duration / totalDuration : 0;
    }
    sqlite3_finalize(stmt);
    return true;
}

bool JsonTraceDatabase::QueryCommunicationStatisticsData(const Protocol::SummaryStatisticParams &requestParams,
    Protocol::SummaryStatisticsBody &responseBody)
{
    sqlite3_stmt *stmt = nullptr;
    int index = bindStartIndex;
    uint64_t min;
    uint64_t max;
    if (!requestParams.stepId.empty()) {
        QueryStepDuration(requestParams.stepId, min, max);
    }
    std::string sql = JsonSqlConstant::GetCommunicationStatisticsSql(requestParams.stepId);
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("QueryCommunicationStatisticsData failed!. ", sqlite3_errmsg(db));
        return false;
    }
    if (!requestParams.stepId.empty()) {
        sqlite3_bind_int64(stmt, index++, min);
        sqlite3_bind_int64(stmt, index, max);
    }
    double communicationTime = 0;
    double notOverlapTime = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        auto duration = static_cast<double>(sqlite3_column_int64(stmt, col++));
        std::string overType = sqlite3_column_string(stmt, col++);
        std::strcmp(overType.c_str(), "Communication") == 0 ? communicationTime = duration : notOverlapTime = duration;
    }
    Protocol::SummaryStatisticsItem overlapItem;
    overlapItem.duration = communicationTime - notOverlapTime;
    overlapItem.overlapType = "Communication(Overlapped)";
    responseBody.summaryStatisticsItemList.push_back(overlapItem);
    Protocol::SummaryStatisticsItem notOverlapItem;
    notOverlapItem.duration = notOverlapTime;
    notOverlapItem.overlapType = "Communication(Not Overlapped)";
    responseBody.summaryStatisticsItemList.push_back(notOverlapItem);
    sqlite3_finalize(stmt);
    return true;
}

bool JsonTraceDatabase::QueryStepDuration(const std::string &stepId, uint64_t &min, uint64_t &max)
{
    std::string profileName = "ProfilerStep#" + stepId;
    std::string sql = "select timestamp, duration from " + sliceTable + " where name=?";
    sqlite3_stmt *stmt = nullptr;
    int index = bindStartIndex;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("QueryStepDuration failed!. ", sqlite3_errmsg(db));
        return false;
    }
    sqlite3_bind_text(stmt, index++, profileName.c_str(), -1, SQLITE_STATIC);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        min = static_cast<uint64_t>(sqlite3_column_int64(stmt, col++));
        max = min + static_cast<uint64_t>(sqlite3_column_int64(stmt, col++));
    }
    sqlite3_finalize(stmt);
    return true;
}


bool JsonTraceDatabase::QuerySystemViewData(const Protocol::SystemViewParams &requestParams,
    Protocol::SystemViewBody &responseBody)
{
    std::string searchName = "%" + requestParams.searchName + "%";
    const LayerStatData &data = QueryLayerData(requestParams.layer, searchName);
    double layerOperatorTime = data.allOperatorTime;
    if (!StringUtil::CheckSqlValid(requestParams.orderBy)) {
        ServerLog::Error("There is an SQL injection attack on this parameter. error param: ", requestParams.orderBy);
        return false;
    }
    std::string sql = JsonSqlConstant::GetQueryPythonViewDataSql(requestParams.order, requestParams.orderBy);
    uint64_t offset = (requestParams.current - 1) * requestParams.pageSize;
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QuerySystemViewData, fail to prepare sql.");
        return false;
    }
    auto resultSet =
        stmt->ExecuteQuery(layerOperatorTime, searchName, requestParams.layer, requestParams.pageSize, offset);
    if (resultSet == nullptr) {
        ServerLog::Error("QueryFlowCategoryEvents. Failed to get result set.", stmt->GetErrorMessage());
        return false;
    }
    while (resultSet->Next()) {
        Protocol::SystemViewDetail systemViewDetail;
        int col = resultStartIndex;
        systemViewDetail.name = resultSet->GetString(col++);
        systemViewDetail.time = resultSet->GetDouble(col++);
        systemViewDetail.totalTime = resultSet->GetDouble(col++);
        systemViewDetail.numberCalls = resultSet->GetUint64(col++);
        systemViewDetail.avg = resultSet->GetDouble(col++);
        systemViewDetail.min = resultSet->GetDouble(col++);
        systemViewDetail.max = resultSet->GetDouble(col++);
        responseBody.systemViewDetail.emplace_back(systemViewDetail);
    }
    responseBody.total = data.total;
    responseBody.pageSize = requestParams.pageSize;
    responseBody.currentPage = requestParams.current;
    return true;
}

LayerStatData JsonTraceDatabase::QueryLayerData(const std::string &layer, const std::string &name)
{
    LayerStatData layerStatData;
    std::string sql = QUERY_LAYER_DATA_SQL;
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryLayerOperatorTime, fail to prepare sql.");
        return layerStatData;
    }
    auto resultSet = stmt->ExecuteQuery(name, layer);
    if (resultSet == nullptr) {
        ServerLog::Error("QueryLayerData. Failed to get result set.", stmt->GetErrorMessage());
        return layerStatData;
    }
    if (resultSet->Next()) {
        layerStatData.allOperatorTime = resultSet->GetDouble("totalTime");
        layerStatData.total = resultSet->GetUint64("count(distinct name)");
    }
    return layerStatData;
}

std::vector<std::string> JsonTraceDatabase::QueryCoreType()
{
    std::vector<std::string> acceleratorCoreList;
    std::string sql = QUERY_QUERY_TYPE_SQL;
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryCoreType, fail to prepare sql.");
        return acceleratorCoreList;
    }
    auto resultSet = stmt->ExecuteQuery();
    if (resultSet == nullptr) {
        ServerLog::Error("QueryCoreType. Failed to get result set.", stmt->GetErrorMessage());
        return acceleratorCoreList;
    }
    while (resultSet->Next()) {
        std::string res = resultSet->GetString("accelerator_core");
        acceleratorCoreList.emplace_back(res);
    }
    return acceleratorCoreList;
}

uint64_t JsonTraceDatabase::QueryTotalKernel(const std::string &coreType, const std::string &name)
{
    uint64_t total = 0;
    std::string sql = "SELECT count(*) FROM kernel_detail where lower(name) LIKE lower(?)";
    if (!coreType.empty()) {
        sql += " AND accelerator_core = ? ";
    }
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryTotalKernel, fail to prepare sql.");
        return total;
    }
    if (!coreType.empty()) {
        stmt->BindParams(coreType);
    }
    auto resultSet = stmt->ExecuteQuery(name);
    if (resultSet == nullptr) {
        ServerLog::Error("QueryTotalKernel. Failed to get result set.", stmt->GetErrorMessage());
        return total;
    }
    if (resultSet->Next()) {
        total = resultSet->GetUint64("count(*)");
    }
    return total;
}

bool JsonTraceDatabase::QueryKernelDetailData(const Protocol::KernelDetailsParams &requestParams,
    Protocol::KernelDetailsBody &responseBody, uint64_t minTimestamp)
{
    if (!StringUtil::CheckSqlValid(requestParams.orderBy)) {
        ServerLog::Error("There is an SQL injection attack on this parameter. error param: ", requestParams.orderBy);
        return false;
    }
    std::string sql =
        JsonSqlConstant::GetKernelDetailSql(requestParams.order, requestParams.orderBy, requestParams.coreType);
    uint64_t offset = (requestParams.current - 1) * requestParams.pageSize;
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryKernelDetailData, fail to prepare sql.");
        return false;
    }
    if (!requestParams.coreType.empty()) {
        stmt->BindParams(requestParams.coreType);
    }
    std::string searchName = "%" + requestParams.searchName + "%";
    auto resultSet = stmt->ExecuteQuery(searchName, requestParams.pageSize, offset);
    if (resultSet == nullptr) {
        ServerLog::Error("QueryKernelDepthAndThread. Failed to get result set.", stmt->GetErrorMessage());
        return false;
    }
    SetKernelDetail(std::move(resultSet), minTimestamp, responseBody);
    responseBody.pageSize = requestParams.pageSize;
    responseBody.currentPage = requestParams.current;
    const std::vector<std::string> cores = QueryCoreType();
    responseBody.acceleratorCoreList = cores;
    responseBody.count = QueryTotalKernel(requestParams.coreType, searchName);
    return true;
}

void JsonTraceDatabase::SetKernelDetail(std::unique_ptr<SqliteResultSet> resultSet, uint64_t minTimestamp,
    Protocol::KernelDetailsBody &responseBody) const
{
    while (resultSet->Next()) {
        Protocol::KernelDetail detail;
        detail.name = resultSet->GetString("name");
        detail.type = resultSet->GetString("type");
        detail.acceleratorCore = resultSet->GetString("acceleratorCore");
        detail.startTime = resultSet->GetUint64("startTime") - minTimestamp;
        detail.duration = resultSet->GetDouble("duration");
        detail.waitTime = resultSet->GetDouble("waitTime");
        detail.blockDim = resultSet->GetUint64("blockDim");
        detail.inputShapes = resultSet->GetString("inputShapes");
        detail.inputDataTypes = resultSet->GetString("inputDataTypes");
        detail.inputFormats = resultSet->GetString("inputFormats");
        detail.outputShapes = resultSet->GetString("outputShapes");
        detail.outputDataTypes = resultSet->GetString("outputDataTypes");
        detail.outputFormats = resultSet->GetString("outputFormats");
        responseBody.kernelDetails.emplace_back(detail);
    }
}

bool JsonTraceDatabase::QueryKernelDepthAndThread(const Protocol::KernelParams &params,
    Protocol::OneKernelBody &responseBody, uint64_t minTimestamp)
{
    std::string sql = "SELECT id, track_id FROM " + sliceTable + " WHERE name = ? AND timestamp > ? AND timestamp < ?";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryKernelDepthAndThread, fail to prepare sql.");
        return false;
    }
    std::unique_ptr<SqliteResultSet> resultSet;
    uint64_t timestamp = params.timestamp + minTimestamp;
    if (timestamp <= tolerance) {
        resultSet = stmt->ExecuteQuery(params.name, 0, timestamp);
    } else if (UINT64_MAX - timestamp > tolerance) {
        resultSet = stmt->ExecuteQuery(params.name, timestamp - tolerance, timestamp + tolerance);
    } else {
        ServerLog::Error("QueryKernelDepthAndThread, The minTimestamp is out of the valid range.");
        return false;
    }

    if (resultSet == nullptr) {
        ServerLog::Error("QueryKernelDepthAndThread. Failed to get result set.", stmt->GetErrorMessage());
        return false;
    }
    uint64_t trackId = 0;
    if (resultSet->Next()) {
        uint64_t id = resultSet->GetUint64("id");
        trackId = resultSet->GetUint64("track_id");
        std::unordered_map<uint64_t, int32_t> depthCache =
            SliceDepthCacheManager::Instance().GetSliceDepthCacheStructByTrackId(trackId).sliceIdAndDepthMap;
        responseBody.id = std::to_string(id);
        responseBody.depth = depthCache[id];
    }
    const OneKernelData &data = QueryKernelTid(trackId);
    responseBody.threadId = data.threadId;
    responseBody.pid = data.pid;
    return true;
}

OneKernelData JsonTraceDatabase::QueryKernelTid(const uint64_t trackId)
{
    std::string sql = "SELECT tid, pid FROM " + threadTable + " WHERE track_id = ? ";
    auto stmt = CreatPreparedStatement(sql);
    OneKernelData oneKernel;
    if (stmt == nullptr) {
        ServerLog::Error("QueryKernelTid, fail to prepare sql.");
        return oneKernel;
    }
    auto resultSet = stmt->ExecuteQuery(trackId);
    if (resultSet == nullptr) {
        ServerLog::Error("QueryKernelTid. Failed to get result set.", stmt->GetErrorMessage());
        return oneKernel;
    }
    uint64_t tid = 0;
    if (resultSet->Next()) {
        oneKernel.threadId = resultSet->GetString("tid");
        oneKernel.pid = resultSet->GetString("pid");
    }
    return oneKernel;
}

bool JsonTraceDatabase::QueryThreadSameOperatorsDetails(const Protocol::UnitThreadsOperatorsParams &requestParams,
    Protocol::UnitThreadsOperatorsBody &responseBody, uint64_t minTimestamp, int64_t traceId)
{
    uint64_t startTime = requestParams.startTime + minTimestamp;
    uint64_t endTime = requestParams.endTime + minTimestamp;
    if (!StringUtil::CheckSqlValid(requestParams.orderBy)) {
        ServerLog::Error("There is an SQL injection attack on this parameter. error param: ", requestParams.orderBy);
        return false;
    }
    std::string sql = JsonSqlConstant::GetThreadSameOperatorsDetailsSql(requestParams.order, requestParams.orderBy);
    uint64_t offset = (requestParams.current - 1) * requestParams.pageSize;
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryThreadSameOperatorsDetails. Failed to prepare sql.", sqlite3_errmsg(db));
        return false;
    }
    auto resultSet =
        stmt->ExecuteQuery(requestParams.name, traceId, endTime, startTime, requestParams.pageSize, offset);
    if (resultSet == nullptr) {
        ServerLog::Error("QueryThreadSameOperatorsDetails. Failed to get result set.", stmt->GetErrorMessage());
        return false;
    }
    while (resultSet->Next()) {
        int col = resultStartIndex;
        Protocol::SameOperatorsDetails sameOperatorsDetail{};
        sameOperatorsDetail.timestamp = resultSet->GetUint64(col++) - minTimestamp;
        sameOperatorsDetail.duration = resultSet->GetUint64(col++);
        responseBody.sameOperatorsDetails.emplace_back(sameOperatorsDetail);
    }
    responseBody.currentPage = requestParams.current;
    responseBody.pageSize = requestParams.pageSize;
    responseBody.count = SameOperatorsCount(requestParams.name, traceId, startTime, endTime);
    return true;
}

uint64_t JsonTraceDatabase::SameOperatorsCount(const std::string &name, int64_t &trackId, uint64_t &startTime,
    uint64_t &endTime)
{
    uint64_t total = 0;
    std::string sql = "SELECT count(*) FROM " + sliceTable +
        " WHERE name = ? AND track_id = ? AND timestamp <= ? AND timestamp + duration >= ?;";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Fail to prepare sql for SameOperatorsCount.", sqlite3_errmsg(db));
        return total;
    }
    auto resultSet = stmt->ExecuteQuery(name, trackId, endTime, startTime);
    if (resultSet == nullptr) {
        ServerLog::Error("SameOperatorsCount. Failed to get result set.", stmt->GetErrorMessage());
        return total;
    }
    if (resultSet->Next()) {
        total = resultSet->GetUint64("count(*)");
    }
    return total;
}

bool JsonTraceDatabase::QueryAffinityOptimizer(const std::string &optimizers,
    std::vector<Protocol::ThreadTraces> &data, uint64_t minTimestamp)
{
    std::string sql = "Select (s.timestamp - ?) as timestamp, s.duration, s.name, t.pid, t.tid "
        "From " + sliceTable + " s Join " + threadTable + " t ON s.track_id = t.track_id "
        "WHERE s.name IN (" + optimizers + ") order by s.timestamp asc";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Fail to prepare sql for QueryAffinityOptimizer.", sqlite3_errmsg(db));
        return false;
    }
    auto resultSet = stmt->ExecuteQuery(minTimestamp);
    if (resultSet == nullptr) {
        ServerLog::Error("Failed to get result set for QueryAffinityOptimizer.", stmt->GetErrorMessage());
        return false;
    }
    while (resultSet->Next()) {
        Protocol::ThreadTraces one{};
        one.startTime = resultSet->GetUint64("timestamp");
        one.name = resultSet->GetString("name");
        one.duration = resultSet->GetUint64("duration");
        one.threadId = resultSet->GetString("tid");
        one.id = resultSet->GetString("pid");
        data.emplace_back(one);
    }
    return true;
}

bool JsonTraceDatabase::UpdateParseStatus(const std::string &status)
{
    return UpdateValueIntoStatusInfoTable(timelineParseStatus, status);
}

bool JsonTraceDatabase::HasFinishedParseLastTime()
{
    return CheckValueFromStatusInfoTable(timelineParseStatus, FINISH_STATUS);
}

bool JsonTraceDatabase::SearchAllSlicesDetails(const Protocol::SearchAllSliceParams &params,
                                               Protocol::SearchAllSlicesBody &body, uint64_t minTimestamp)
{
    std::string sql = JsonSqlConstant::GetSearchSliceDetailSql(params.isMatchExact,
                                                               params.isMatchCase, params.order, params.orderBy);
    uint64_t offset = (params.current - 1) * params.pageSize;
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("SearchAllSlicesDetails failed!.");
        return false;
    }
    auto resultSet = stmt->ExecuteQuery(params.searchContent, params.pageSize, offset);
    if (resultSet == nullptr) {
        ServerLog::Error("SearchAllSlicesDetails. Failed to get result set.", stmt->GetErrorMessage());
        return false;
    }
    while (resultSet->Next()) {
        int col = resultStartIndex;
        Protocol::SearchAllSlices searchAllSlice{};
        searchAllSlice.name = resultSet->GetString(col++);
        searchAllSlice.timestamp = resultSet->GetUint64(col++) - minTimestamp;
        searchAllSlice.duration = resultSet->GetUint64(col++);
        body.searchAllSlices.emplace_back(searchAllSlice);
    }
    body.currentPage = params.current;
    body.pageSize = params.pageSize;
    Protocol::SearchCountParams searchCountParams;
    searchCountParams.searchContent = params.searchContent;
    searchCountParams.isMatchCase = params.isMatchCase;
    searchCountParams.isMatchExact = params.isMatchExact;
    searchCountParams.rankId = params.rankId;
    body.count = SearchSliceNameCount(searchCountParams);
    return true;
}
} // end of namespace Timeline
  // end of namespace Module
  // end of namespace Dic