/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2024. All rights reserved.
 */

#include <algorithm>
#include "ServerLog.h"
#include "TraceTime.h"
#include "TableDefs.h"
#include "Timer.h"
#include "TraceFileParser.h"
#include "TraceDatabaseHelper.h"
#include "JsonTraceDatabase.h"

namespace Dic::Module::Timeline {
using namespace Dic::Server;
using namespace Dic::Protocol;
JsonTraceDatabase::JsonTraceDatabase(std::mutex &sqlMutex) : VirtualTraceDatabase(sqlMutex) {}

JsonTraceDatabase::~JsonTraceDatabase()
{
    CommitData();
    ReleaseStmt();
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
    std::string sql = "INSERT INTO " + sliceTable +
        " (timestamp, duration, name, track_id, cat, args, cname) VALUES"
        " (?,?,?,?,?,?,?)";
    for (int i = 0; i < cacheSize - 1; ++i) {
        sql.append(",(?,?,?,?,?,?,?)");
    }
    insertSliceStmt = CreatPreparedStatement(sql);
    sql = "INSERT INTO " + flowTable + " (flow_id, name, track_id, timestamp, cat, type)" + " VALUES (?,?,?,?,?,?)";
    for (int i = 0; i < cacheSize - 1; ++i) {
        sql.append(",(?,?,?,?,?,?)");
    }
    insertFlowStmt = CreatPreparedStatement(sql);
    sql = "INSERT INTO " + counterTable + " (name, pid, timestamp, cat, args)" + " VALUES (?,?,?,?,?)";
    for (int i = 0; i < cacheSize - 1; ++i) {
        sql.append(",(?,?,?,?,?)");
    }
    insertCounterStmt = CreatPreparedStatement(sql);
    if (insertSliceStmt == nullptr || insertFlowStmt == nullptr || insertCounterStmt == nullptr) {
        ServerLog::Error("Failed to prepare slice statement.");
        return false;
    }
    return true;
}

bool JsonTraceDatabase::InitProcessThreadStmt()
{
    std::string sql = "INSERT INTO " + processTable + " (pid, process_name) VALUES (?, ?) " +
        "ON CONFLICT (pid) DO UPDATE SET process_name = excluded.process_name;";
    updateProcessNameStmt = CreatPreparedStatement(sql);
    sql = "INSERT INTO " + processTable + " (pid, label) VALUES (?, ?)" +
        " ON CONFLICT (pid) DO UPDATE SET label = excluded.label;";
    updateProcessLabelStmt = CreatPreparedStatement(sql);
    sql = "INSERT INTO " + processTable + " (pid, process_sort_index) VALUES (?, ?)" +
        "ON CONFLICT (pid) DO UPDATE SET process_sort_index = excluded.process_sort_index;";
    updateProcessSortIndexStmt = CreatPreparedStatement(sql);
    sql = "INSERT INTO " + threadTable + " (track_id, tid, pid) VALUES (?, ?, ?)" +
        " ON CONFLICT (track_id) DO UPDATE SET tid = excluded.tid, pid = excluded.pid;";
    updateThreadInfoStmt = CreatPreparedStatement(sql);
    sql = "INSERT INTO " + threadTable + " (track_id, tid, pid, thread_name) VALUES (?, ?, ?, ?)" +
        " ON CONFLICT (track_id) DO UPDATE " +
        " SET tid = excluded.tid, pid = excluded.pid, thread_name = excluded.thread_name;";
    updateThreadNameStmt = CreatPreparedStatement(sql);
    sql = "INSERT INTO " + threadTable + " (track_id, thread_sort_index) VALUES (?, ?) " +
        " ON CONFLICT (track_id) DO UPDATE SET thread_sort_index = excluded.thread_sort_index;";
    updateThreadSortIndexStmt = CreatPreparedStatement(sql);
    if (updateProcessNameStmt == nullptr || updateProcessLabelStmt == nullptr ||
        updateProcessSortIndexStmt == nullptr || updateThreadInfoStmt == nullptr || updateThreadNameStmt == nullptr ||
        updateThreadSortIndexStmt == nullptr) {
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
    updateSliceStmt = nullptr;
    updateProcessNameStmt = nullptr;
    updateProcessLabelStmt = nullptr;
    updateProcessSortIndexStmt = nullptr;
    updateThreadInfoStmt = nullptr;
    updateThreadNameStmt = nullptr;
    updateThreadSortIndexStmt = nullptr;
    insertFlowStmt = nullptr;
    insertCounterStmt = nullptr;
}

bool JsonTraceDatabase::SetConfig()
{
    if (!isOpen) {
        ServerLog::Error("Failed to set config. Database is not open.");
        return false;
    }
    std::unique_lock<std::mutex> lock(mutex);
    std::string dbVersion = GetDataBaseVersion();
    return ExecSql("PRAGMA synchronous = OFF; PRAGMA journal_mode = MEMORY; PRAGMA user_version = " + dbVersion + ";");
}

bool JsonTraceDatabase::CreateTable()
{
    if (!isOpen) {
        ServerLog::Error("Failed to set config. Database is not open.");
        return false;
    }
    std::string sql = "CREATE TABLE " + sliceTable +
        " (id INTEGER PRIMARY KEY AUTOINCREMENT, timestamp INTEGER, duration INTEGER,"
        " overlapDuration INTEGER, notOverlapDuration INTEGER,"
        " name TEXT, depth INTEGER, track_id INTEGER, cat TEXT, args TEXT, cname TEXT);" +
        "CREATE TABLE " + threadTable + " (track_id INTEGER PRIMARY KEY, tid TEXT, pid TEXT, thread_name TEXT," +
        " thread_sort_index INTEGER);" + "CREATE TABLE " + processTable +
        " (pid TEXT PRIMARY KEY, process_name TEXT, label TEXT," + " process_sort_index INTEGER);" + "CREATE TABLE " +
        flowTable + " (id INTEGER PRIMARY KEY AUTOINCREMENT, flow_id TEXT, name TEXT, cat TEXT," +
        " track_id INTEGER, timestamp INTEGER, type TEXT);" + "CREATE TABLE " + counterTable +
        " (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, pid TEXT," + "timestamp INTEGER, cat TEXT, args TEXT);";
    std::unique_lock<std::mutex> lock(mutex);
    return ExecSql(sql);
}

bool JsonTraceDatabase::DropTable()
{
    std::vector<std::string> tables = { sliceTable, threadTable, processTable, flowTable, counterTable };
    std::unique_lock<std::mutex> lock(mutex);
    return DropSomeTables(tables);
}

bool JsonTraceDatabase::CreateIndex()
{
    auto start = std::chrono::system_clock::now();
    if (!isOpen) {
        ServerLog::Error("Failed to creat index. Database is not open.");
        return false;
    }
    std::string sql = "CREATE INDEX " + idIndex + " ON " + sliceTable + " (id);" + "CREATE INDEX " + trackIdTimeIndex +
        " ON " + sliceTable + " (track_id, timestamp);" + "CREATE INDEX " + flowIndex + " ON " + flowTable +
        " (track_id, timestamp);";
    ExecSql(sql);
    auto dur = std::chrono::duration<double, std::milli>(std::chrono::system_clock::now() - start);
    ServerLog::Info("CreateIndex end. time:", dur.count());
    return true;
}

bool JsonTraceDatabase::InsertSlice(const Trace::Slice &event)
{
    sliceCache.emplace_back(event);
    if (sliceCache.size() == cacheSize) {
        InsertSliceList(sliceCache);
        sliceCache.clear();
    }
    return true;
}

bool JsonTraceDatabase::InsertSimulationSlice(const Trace::Slice &event)
{
    InsertSlice(event);
    updateProcessNameStmt->Reset();
    if (!updateProcessNameStmt->Execute(event.pid, event.processName)) {
        ServerLog::Error("Update process name fail. ", updateProcessNameStmt->GetErrorMessage());
        return false;
    }
    updateThreadNameStmt->Reset();
    if (!updateThreadNameStmt->Execute(event.trackId, event.tid, event.pid, event.threadName)) {
        ServerLog::Error("Update thread name fail. ", updateThreadNameStmt->GetErrorMessage());
        return false;
    }
    return true;
}

bool JsonTraceDatabase::InsertSliceList(const std::vector<Trace::Slice> &eventList)
{
    std::unique_ptr<SqlitePreparedStatement> stmt;
    std::unique_ptr<SqlitePreparedStatement> &refStmt = (eventList.size() == cacheSize) ? insertSliceStmt : stmt;
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
        refStmt->BindParams(event.ts, event.dur, event.name, event.trackId, event.cat, event.args, event.cname);
    }
    std::unique_lock<std::mutex> lock(mutex);
    if (!refStmt->Execute()) {
        ServerLog::Error("Insert slice data fail. ", refStmt->GetErrorMessage());
        return false;
    }
    return true;
}

std::unique_ptr<SqlitePreparedStatement> JsonTraceDatabase::GetSliceStmt(uint64_t paramLen)
{
    std::string sql = "INSERT INTO " + sliceTable +
        " (timestamp, duration, name, track_id, cat, args, cname) VALUES "
        " (?,?,?,?,?,?,?)";
    for (int i = 0; i < paramLen - 1; ++i) {
        sql.append(",(?,?,?,?,?,?,?)");
    }
    return CreatPreparedStatement(sql);
}

bool JsonTraceDatabase::UpdateProcessName(const Trace::MetaData &event)
{
    updateProcessNameStmt->Reset();
    std::unique_lock<std::mutex> lock(mutex);
    if (!updateProcessNameStmt->Execute(event.pid, event.args.name)) {
        ServerLog::Error("Update process name fail. ", updateProcessNameStmt->GetErrorMessage());
        return false;
    }
    return true;
}

bool JsonTraceDatabase::UpdateProcessLabel(const Trace::MetaData &event)
{
    updateProcessLabelStmt->Reset();
    std::unique_lock<std::mutex> lock(mutex);
    if (!updateProcessLabelStmt->Execute(event.pid, event.args.labels)) {
        ServerLog::Error("Update process label fail. ", updateProcessLabelStmt->GetErrorMessage());
        return false;
    }
    return true;
}

bool JsonTraceDatabase::UpdateProcessSortIndex(const Trace::MetaData &event)
{
    updateProcessSortIndexStmt->Reset();
    std::unique_lock<std::mutex> lock(mutex);
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

bool JsonTraceDatabase::UpdateThreadInfo(const std::tuple<int64_t, std::string, std::string> &thread)
{
    updateThreadInfoStmt->Reset();
    std::unique_lock<std::mutex> lock(mutex);
    if (!updateThreadInfoStmt->Execute(std::get<0>(thread), std::get<1>(thread), std::get<2>(thread))) { // 第2个
        ServerLog::Error("Update thread info fail. ", updateThreadInfoStmt->GetErrorMessage());
        return false;
    }
    return true;
}

bool JsonTraceDatabase::UpdateThreadName(const Trace::MetaData &event)
{
    updateThreadNameStmt->Reset();
    std::unique_lock<std::mutex> lock(mutex);
    if (!updateThreadNameStmt->Execute(event.trackId, event.tid, event.pid, event.args.name)) {
        ServerLog::Error("Update thread name fail. ", updateThreadNameStmt->GetErrorMessage());
        return false;
    }
    return true;
}

bool JsonTraceDatabase::UpdateThreadSortIndex(const Trace::MetaData &event)
{
    updateThreadSortIndexStmt->Reset();
    std::unique_lock<std::mutex> lock(mutex);
    if (!updateThreadSortIndexStmt->Execute(event.trackId, event.args.sortIndex)) {
        ServerLog::Error("Update thread sort index fail. ", updateThreadSortIndexStmt->GetErrorMessage());
        return false;
    }
    return true;
}

bool JsonTraceDatabase::InsertFlow(const Trace::Flow &event)
{
    flowCache.emplace_back(event);
    if (flowCache.size() == cacheSize) {
        InsertFlowList(flowCache);
        flowCache.clear();
    }
    return true;
}

bool JsonTraceDatabase::InsertFlowList(const std::vector<Trace::Flow> &eventList)
{
    std::unique_ptr<SqlitePreparedStatement> stmt;
    std::unique_ptr<SqlitePreparedStatement> &refStmt = (eventList.size() == cacheSize) ? insertFlowStmt : stmt;
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
    std::unique_lock<std::mutex> lock(mutex);
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
    if (counterCache.size() == cacheSize) {
        InsertCounterList(counterCache);
        counterCache.clear();
    }
    return true;
}

bool JsonTraceDatabase::InsertCounterList(const std::vector<Trace::Counter> &eventList)
{
    std::unique_ptr<SqlitePreparedStatement> stmt;
    std::unique_ptr<SqlitePreparedStatement> &refStmt = (eventList.size() == cacheSize) ? insertCounterStmt : stmt;
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
    std::unique_lock<std::mutex> lock(mutex);
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

void JsonTraceDatabase::UpdateDepth()
{
    ServerLog::Info("UpdateDepth.");
    CreateDepthTempTable();
    UpdateSliceDepth();
    DropDepthTempTable();
    ServerLog::Info("UpdateDepth end.");
}

void JsonTraceDatabase::UpdateSimulationDepth()
{
    sqlite3_exec(db, "begin;", 0, 0, 0);
    UpdateSimulationDepthByCode();
    sqlite3_exec(db, "commit;", 0, 0, 0);
}

void JsonTraceDatabase::UpdateSimulationDepthByCode()
{
    Timer timer("UpdateSimulationDepthByCode");
    std::vector<int32_t> trackIdList = QueryAllTrackId();
    if (std::empty(trackIdList)) {
        return;
    }
    ServerLog::Info("trackIdList size: ", trackIdList.size());
    for (const auto &item : trackIdList) {
        std::vector<Protocol::RowThreadTrace> rowThreadTraceVec = QueryAllSliceByTrackId(item);
        if (std::empty(rowThreadTraceVec)) {
            continue;
        }
        UpdateAllSimulationSliceDepth(rowThreadTraceVec);
    }
}

void JsonTraceDatabase::UpdateAllSimulationSliceDepth(std::vector<Protocol::RowThreadTrace> &rowThreadTraceVec)
{
    sliceDepthHelper.clear();
    for (auto &rowThreadTrace : rowThreadTraceVec) {
        int64_t index = 0;
        std::set<int32_t> depthSet;
        if (sliceDepthHelper.empty()) {
            rowThreadTrace.depth = depthSet.size();
            sliceDepthHelper.push_back(rowThreadTrace);
            continue;
        }
        auto iterator = sliceDepthHelper.begin();
        for (const auto &traceVec : sliceDepthHelper) {
            if (rowThreadTrace.startTime + rowThreadTrace.duration > traceVec.startTime + traceVec.duration) {
                rowThreadTrace.depth = depthSet.size();
                std::advance(iterator, index);
                sliceDepthHelper.insert(iterator, rowThreadTrace);
                depthSet.clear();
                index = 0;
                break;
            }
            depthSet.insert(traceVec.depth);
            index++;
            if (index > sliceDepthHelper.size() - 1) {
                rowThreadTrace.depth = depthSet.size();
                sliceDepthHelper.push_back(rowThreadTrace);
                depthSet.clear();
                index = 0;
                break;
            }
        }
    }
    UpdateSimulationSliceDepth(sliceDepthHelper);
}

std::vector<Protocol::RowThreadTrace> JsonTraceDatabase::QueryAllSliceByTrackId(const int32_t &trackId)
{
    std::vector<Protocol::RowThreadTrace> rowThreadTraceVec;
    std::string querySliceByTrackId =
        "select id, timestamp, depth, duration from slice where track_id = ? order by timestamp, id;";
    auto sliceStmt = CreatPreparedStatement(querySliceByTrackId);
    if (sliceStmt == nullptr) {
        ServerLog::Error("querySliceByTrackId. Failed to prepare sql.", GetLastError());
        return rowThreadTraceVec;
    }
    auto sliceResultSet = sliceStmt->ExecuteQuery(trackId);
    if (sliceResultSet == nullptr) {
        ServerLog::Error("querySliceByTrackId. Failed to get result set.", sliceStmt->GetErrorMessage());
        return rowThreadTraceVec;
    }
    while (sliceResultSet->Next()) {
        RowThreadTrace rowThreadTrace{};
        rowThreadTrace.id = sliceResultSet->GetInt64("id");
        rowThreadTrace.startTime = sliceResultSet->GetUint64("timestamp");
        rowThreadTrace.duration = sliceResultSet->GetUint64("duration");
        rowThreadTrace.depth = sliceResultSet->GetInt32("depth");
        rowThreadTraceVec.emplace_back(rowThreadTrace);
    }
    return rowThreadTraceVec;
}

std::vector<int32_t> JsonTraceDatabase::QueryAllTrackId()
{
    std::vector<int32_t> trackIdList;
    std::string allTrackIdSql = "select track_id as trackId from thread;";
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

bool JsonTraceDatabase::UpdateSimulationSliceDepth(std::list<Protocol::RowThreadTrace> &sliceLinkedList)
{
    Timer timer("JsonTraceDatabase::UpdateSimulationSliceDepth");
    std::string updateSql = "Update slice set depth = ? where id = ? ;";
    for (const auto &singleSlice : sliceLinkedList) {
        std::unique_ptr<SqlitePreparedStatement> updateStmt = CreatPreparedStatement(updateSql);
        if (!updateStmt->Execute(singleSlice.depth, singleSlice.id)) {
            ServerLog::Error("updateSliceDepthSql fail. ", sqlite3_errmsg(db));
        }
    }
}

void JsonTraceDatabase::CreateDepthTempTable()
{
    Timer timer("CreateDepthTempTable");
    std::string sql = "CREATE TEMPORARY TABLE temps AS "
        "SELECT S2.id, S0.id AS parent_id "
        "FROM slice AS S2 JOIN slice AS S0 "
        "WHERE (S2.track_id = S0.track_id AND S2.timestamp > S0.timestamp "
        "AND S2.timestamp < S0.timestamp + S0.duration) "
        "OR (S2.track_id = S0.track_id AND S2.timestamp = S0.timestamp AND S2.id > S0.id);";

    if (!ExecSql(sql)) {
        ServerLog::Error("Creat temp table fail. ", GetLastError());
    }
}

void JsonTraceDatabase::DropDepthTempTable()
{
    Timer timer("DropDepthTempTable");
    std::string sql = "DROP table temp.temps";
    if (!ExecSql(sql)) {
        ServerLog::Error("Drop temp table fail. ", GetLastError());
    }
}

void JsonTraceDatabase::UpdateSliceDepth()
{
    Timer timer("UpdateSliceDepth");
    std::string sql = "UPDATE slice AS S SET depth = ("
        "SELECT COALESCE(tmp.count, 0) FROM slice LEFT JOIN ("
        "SELECT id, COUNT(*) as count "
        "FROM temps "
        "GROUP BY id) AS tmp "
        "ON tmp.id = S.id);";
    if (!ExecSql(sql)) {
        ServerLog::Error("Update slice depth fail. ", GetLastError());
    }
}

bool JsonTraceDatabase::QueryThreadTraces(const Protocol::UnitThreadTracesParams &requestParams,
    Protocol::UnitThreadTracesBody &responseBody, uint64_t minTimestamp, int64_t traceId)
{
    if (requestParams.timePerPx == 0) {
        ServerLog::Error("QueryThreadTraces. timePerPx is zero.");
        return false;
    }
    std::vector<Protocol::RowThreadTrace> rowThreadTraceVec =
        QuerySliceByCondition(requestParams, minTimestamp, traceId);
    if (std::empty(rowThreadTraceVec)) {
        return false;
    }
    std::map<int64_t, std::vector<Protocol::ThreadTraces>> threadTracesMap;
    for (auto &item : rowThreadTraceVec) {
        Protocol::ThreadTraces threadTraces{};
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
    std::string sql = "SELECT id, timestamp - ? as start_time, duration, name, depth, track_id, cname,"
        " ROUND(timestamp / ? ) as rank "
        " FROM " +
        sliceTable +
        " WHERE track_id = ? AND start_time + duration >= ? AND start_time <= ?"
        " GROUP BY depth, rank, duration HAVING max(timestamp)"
        " ORDER BY depth, start_time;";
    std::vector<RowThreadTrace> rowThreadTraceVec;
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryThreadTraces. Failed to prepare sql.", GetLastError());
        return rowThreadTraceVec;
    }
    auto resultSet = stmt->ExecuteQuery(minTimestamp, requestParams.timePerPx * middleImage, traceId,
        requestParams.startTime, requestParams.endTime);
    if (resultSet == nullptr) {
        ServerLog::Error("QueryThreadTraces. Failed to get result set.", stmt->GetErrorMessage());
        return rowThreadTraceVec;
    }
    while (resultSet->Next()) {
        RowThreadTrace rowThreadTrace{};
        rowThreadTrace.id = resultSet->GetInt64("id");
        rowThreadTrace.startTime = resultSet->GetUint64("start_time");
        rowThreadTrace.duration = resultSet->GetUint64("duration");
        rowThreadTrace.name = resultSet->GetString("name");
        rowThreadTrace.depth = resultSet->GetInt32("depth");
        rowThreadTrace.traceId = resultSet->GetInt64("track_id");
        rowThreadTrace.cname = resultSet->GetString("cname");
        rowThreadTraceVec.emplace_back(rowThreadTrace);
    }
    return rowThreadTraceVec;
}

bool JsonTraceDatabase::QueryThreadTracesSummary(const Protocol::UnitThreadTracesSummaryParams &requestParams,
    Protocol::UnitThreadTracesSummaryBody &responseBody, uint64_t minTimestamp)
{
    std::string sql = "SELECT timestamp - ? as start_time, duration, timestamp + duration - ? as end_time, "
        "ROUND(timestamp / ? ) as rank "
        "FROM " +
        sliceTable + " LEFT JOIN " + threadTable +
        " USING (track_id) "
        "WHERE pid = ? AND start_time >= ? AND start_time <= ? "
        "ORDER BY timestamp;";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryThreadTraces. Failed to prepare sql.", GetLastError());
        return false;
    }
    auto resultSet = stmt->ExecuteQuery(minTimestamp, minTimestamp, summaryPerpix, requestParams.processId,
        requestParams.startTime, requestParams.endTime);
    if (resultSet == nullptr) {
        ServerLog::Error("QueryThreadTracesSummary. Failed to get result set.", stmt->GetErrorMessage());
        return false;
    }
    uint64_t maxTime = 0;
    while (resultSet->Next()) {
        ThreadTracesSummary summary;
        uint64_t endTime = resultSet->GetUint64("end_time");
        if (endTime > maxTime) {
            summary.startTime = resultSet->GetUint64("start_time");
            summary.duration = resultSet->GetUint64("duration");
            responseBody.data.emplace_back(summary);
            maxTime = endTime;
        }
    }
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
    std::string sql = "SELECT timestamp, duration, timestamp + duration AS endTime, name, depth"
        " FROM " +
        sliceTable +
        " WHERE track_id = ? AND timestamp <= ? AND timestamp + duration >= ?"
        " ORDER BY depth ASC, timestamp ASC;";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryThreads. Failed to prepare sql.");
        return false;
    }
    int index = bindStartIndex;
    auto resultSet = stmt->ExecuteQuery(traceId, extremumTimestamp.maxTimestamp, extremumTimestamp.minTimestamp);
    std::vector<Protocol::SimpleSlice> simpleSliceVec;
    while (resultSet->Next()) {
        int col = resultStartIndex;
        Protocol::SimpleSlice simpleSlice{};
        simpleSlice.timestamp = resultSet->GetUint64(col++);
        simpleSlice.duration = resultSet->GetUint64(col++);
        simpleSlice.endTime = resultSet->GetUint64(col++);
        simpleSlice.name = resultSet->GetString(col++);
        simpleSlice.depth = resultSet->GetInt32(col++);
        simpleSliceVec.emplace_back(simpleSlice);
    }
    // process data
    if (simpleSliceVec.empty()) {
        responseBody.emptyFlag = true;
        return true;
    }
    std::map<std::string, uint64_t> selfTimeKeyValue;
    CalculateSelfTime(simpleSliceVec, selfTimeKeyValue, startTime, endTime);
    std::vector<Protocol::SimpleSlice> nRows = ThreadsInfoFilter(simpleSliceVec, startTime, endTime);
    ReduceThread(nRows, selfTimeKeyValue, responseBody);
    return true;
}

bool JsonTraceDatabase::QueryExtremumTimeOfFirstDepth(int64_t trackId, uint64_t startTime, uint64_t endTime,
    Protocol::ExtremumTimestamp &extremumTimestamp)
{
    std::string sql = "SELECT min(timestamp) as minTimestamp, max(timestamp + duration) AS maxTimestamp"
        " FROM " +
        sliceTable + " WHERE track_id = ? AND timestamp <= ? AND timestamp + duration >= ? AND depth = 0;";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryExtremumTimeOfFirstDepth. Failed to prepare sql.");
        return false;
    }
    auto resultSet = stmt->ExecuteQuery(trackId, endTime, startTime);
    while (resultSet->Next()) {
        extremumTimestamp.minTimestamp = resultSet->GetUint64("minTimestamp");
        extremumTimestamp.maxTimestamp = resultSet->GetUint64("maxTimestamp");
    }
    return true;
}

bool JsonTraceDatabase::DealLastData(std::vector<Protocol::SimpleSlice> &rows,
    std::map<std::string, uint64_t> &selfTimeKeyValue, uint64_t startTime, uint64_t endTime, uint64_t index)
{
    while (++index < rows.size()) {
        if (rows.at(index).timestamp <= endTime && rows.at(index).endTime >= startTime) {
            AddData(selfTimeKeyValue, rows.at(index).name, rows.at(index).duration);
        }
    }
}

void JsonTraceDatabase::CalculateSelfTime(std::vector<Protocol::SimpleSlice> &rows,
    std::map<std::string, uint64_t> &selfTimeKeyValue, uint64_t startTime, uint64_t endTime)
{
    int32_t i = 0;
    int32_t j = 0;
    if (rows.empty()) {
        ServerLog::Error("simpleSlice array size is zero!");
        return;
    }
    uint64_t tmpSelfTime = rows.at(0).duration;
    while (i < rows.size()) {
        // j滑完直接滑完所有i
        if (j == rows.size()) {
            // 处理当前tmpSelfTime
            AddData(selfTimeKeyValue, rows.at(i).name, tmpSelfTime);
            // 处理剩余元素
            DealLastData(rows, selfTimeKeyValue, startTime, endTime, i);
            break;
        }
        Protocol::SimpleSlice rowI = rows.at(i);
        Protocol::SimpleSlice rowJ = rows.at(j);
        // 层数相等 or 同一元素, j右移
        if (rowI.depth == rowJ.depth || i >= j) {
            j++;
            continue;
        }
        // rows[i]不属于框选范围内，跳过
        if (rows.at(i).timestamp > endTime || rows.at(i).endTime < startTime) {
            if (i + 1 == rows.size()) { // i滑完结束
                break;
            }
            i++;
            tmpSelfTime = rows.at(i).duration;
            continue;
        }
        // j元素超出i元素覆盖范围，或者j右移到下一层, 记录i元素selfTime并i右移(隐式|| rowJ.timestamp < rowI.timestamp)
        if (rowJ.endTime > rowI.endTime || rowI.depth + 1 < rowJ.depth) {
            AddData(selfTimeKeyValue, rowI.name, tmpSelfTime);
            if (i + 1 == rows.size()) { // i滑完结束
                break;
            }
            i++;
            tmpSelfTime = rows.at(i).duration;
            continue;
        }
        // 符合要求的元素
        if (rowJ.timestamp >= rowI.timestamp && rowJ.endTime <= rowI.endTime) {
            tmpSelfTime -= rowJ.duration;
        }
        j++;
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

std::vector<Protocol::SimpleSlice> JsonTraceDatabase::ThreadsInfoFilter(
    const std::vector<Protocol::SimpleSlice> &simpleSliceVec, uint64_t startTime, uint64_t endTime)
{
    std::vector<Protocol::SimpleSlice> nRows;
    for (auto &row : simpleSliceVec) {
        if (row.timestamp <= endTime && row.endTime >= startTime) {
            nRows.emplace_back(row);
        }
    }
    return nRows;
}

void JsonTraceDatabase::ReduceThread(const std::vector<Protocol::SimpleSlice> &rows,
    const std::map<std::string, uint64_t> &selfTimeKeyValue, Protocol::UnitThreadsBody &responseBody)
{
    for (auto &cur : rows) {
        int index = -1;
        for (int i = 0; i < responseBody.data.size(); i++) {
            if (responseBody.data[i].title == cur.name) {
                index = i;
                break;
            }
        }
        if (index == -1) {
            Protocol::Threads threads{};
            threads.title = cur.name;
            threads.wallDuration = cur.duration;
            threads.occurrences = 1;
            threads.avgWallDuration = cur.duration;
            threads.selfTime = selfTimeKeyValue.at(cur.name);
            responseBody.data.emplace_back(threads);
        } else {
            responseBody.data[index].wallDuration += cur.duration;
            responseBody.data[index].occurrences += 1;
            responseBody.data[index].avgWallDuration =
                responseBody.data[index].wallDuration / responseBody.data[index].occurrences;
        }
    }
}

bool JsonTraceDatabase::QueryThreadDetail(const Protocol::ThreadDetailParams &requestParams,
    Protocol::UnitThreadDetailBody &responseBody, uint64_t minTimestamp, int64_t trackId)
{
    std::string sql = "SELECT id, timestamp, duration, name, depth, track_id, cat, args"
        " FROM " +
        sliceTable + " WHERE depth = ? AND track_id = ? AND timestamp = ?";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryThreadDetail. Failed to prepare sql.");
        return false;
    }
    auto resultSet = stmt->ExecuteQuery(requestParams.depth, trackId, requestParams.startTime + minTimestamp);
    std::vector<SliceDto> sliceDtoVec;
    while (resultSet->Next()) {
        SliceDto sliceDto{};
        sliceDto.id = resultSet->GetUint64("id");
        sliceDto.timestamp = resultSet->GetUint64("timestamp");
        sliceDto.duration = resultSet->GetUint64("duration");
        sliceDto.name = resultSet->GetString("name");
        sliceDto.depth = resultSet->GetInt32("depth");
        sliceDto.trackId = resultSet->GetInt64("track_id");
        sliceDto.cat = resultSet->GetString("cat");
        sliceDto.args = resultSet->GetString("args");
        sliceDtoVec.emplace_back(sliceDto);
    }

    if (sliceDtoVec.size() != 1) {
        ServerLog::Error("select slice error!");
        return false;
    }
    uint64_t selfTime = sliceDtoVec.at(0).duration;
    std::vector<uint64_t> nextDepthResult;
    QueryDurationFromSliceByTimeRange(requestParams, sliceDtoVec, nextDepthResult, trackId);
    if (nextDepthResult.empty()) {
        selfTime = 0;
    } else {
        for (const auto &item : nextDepthResult) {
            selfTime -= item;
        }
    }
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

bool JsonTraceDatabase::QueryDurationFromSliceByTimeRange(const Protocol::ThreadDetailParams &requestParams,
    const std::vector<SliceDto> &rows, std::vector<uint64_t> &nextDepthResult, int64_t trackId)
{
    if (rows.empty()) {
        ServerLog::Error("sliceDto array is empty!");
        return false;
    }
    std::string sql = "SELECT duration FROM " + sliceTable +
        " WHERE depth = ? AND timestamp + duration <= ? AND timestamp >= ? AND track_id = ?";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryDurationFromSliceByTimeRange. Failed to prepare sql.", sqlite3_errmsg(db));
        return false;
    }
    auto resultSet =
        stmt->ExecuteQuery(requestParams.depth + 1, rows[0].timestamp + rows[0].duration, rows[0].timestamp, trackId);
    while (resultSet->Next()) {
        nextDepthResult.emplace_back(resultSet->GetUint64("duration"));
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
    std::string sql = "SELECT input_shapes AS inputShapes, input_data_types AS inputDataTypes, "
                      "input_formats AS inputFormats, output_shapes AS outputShapes, "
                      "output_data_types AS outputDataTypes, output_formats AS outputFormats "
                      "FROM " + kernelDetail +
                      " WHERE name = ? AND start_time = ?";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryKernelShapes. Failed to prepare sql.", sqlite3_errmsg(db));
        return kernelShapesDataDto;
    }
    auto resultSet = stmt->ExecuteQuery(param[0].name, param[0].timestamp);
    while (resultSet->Next()) {
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
    std::string sql = "SELECT FL.name, FL.cat, FL.flow_id as flowId, TH.pid, TH.tid, SL.depth, SL.timestamp,"
        " SL.duration, FL.type, SL.name as sliceName"
        " FROM " +
        threadTable + " TH LEFT JOIN " + sliceTable + " SL" +
        " ON SL.track_id = TH.track_id LEFT JOIN flow FL"
        " ON FL.track_id = SL.track_id "
        " WHERE FL.timestamp = SL.timestamp AND FL.flow_id = ?";
    if (requestParams.type == "s") {
        sql.append(" AND FL.timestamp >= ? ORDER BY FL.timestamp ASC LIMIT 2");
    } else {
        sql.append(" AND FL.timestamp <= ? ORDER BY FL.timestamp DESC LIMIT 2");
    }
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryFlowDetail. Failed to prepare sql.");
        return false;
    }
    auto resultSet = stmt->ExecuteQuery(requestParams.flowId, requestParams.startTime + minTimestamp);
    std::vector<FlowDetailDto> flowDetailVec;
    while (resultSet->Next()) {
        int col = resultStartIndex;
        FlowDetailDto flowDetailDto{};
        flowDetailDto.name = resultSet->GetString("name");
        flowDetailDto.cat = resultSet->GetString("cat");
        flowDetailDto.flowId = resultSet->GetString("flowId");
        flowDetailDto.pid = resultSet->GetString("pid");
        flowDetailDto.tid = resultSet->GetString("tid");
        flowDetailDto.depth = resultSet->GetInt32("depth");
        flowDetailDto.timestamp = resultSet->GetUint64("timestamp");
        flowDetailDto.duration = resultSet->GetUint64("duration");
        flowDetailDto.type = resultSet->GetString("type");
        flowDetailDto.sliceName = resultSet->GetString("sliceName");
        flowDetailVec.emplace_back(flowDetailDto);
    }
    return FlowDetailToResponse(flowDetailVec, minTimestamp, responseBody);
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
    responseBody.from.pid = from.pid;
    responseBody.from.tid = from.tid;
    responseBody.from.timestamp = from.timestamp - minTimestamp;
    responseBody.from.duration = from.duration;
    responseBody.from.depth = from.depth;
    responseBody.from.name = from.sliceName;
    responseBody.to.pid = to.pid;
    responseBody.to.tid = to.tid;
    responseBody.to.timestamp = to.timestamp - minTimestamp;
    responseBody.to.duration = to.duration;
    responseBody.to.depth = to.depth;
    responseBody.to.name = to.sliceName;
    return true;
}

bool JsonTraceDatabase::QueryFlowName(const Protocol::UnitFlowNameParams &requestParams,
    Protocol::UnitFlowNameBody &responseBody, uint64_t minTimestamp, int64_t trackId)
{
    std::string sql = "SELECT name, flow_id as flowId, type"
        " FROM " +
        flowTable + " WHERE timestamp = ? AND track_id = ? GROUP BY flowId";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryFlowName. Failed to prepare sql.");
        return false;
    }
    auto resultSet = stmt->ExecuteQuery(requestParams.startTime + minTimestamp, trackId);
    while (resultSet->Next()) {
        std::string name = resultSet->GetString("name");
        std::string flowId = resultSet->GetString("flowId");
        std::string type = resultSet->GetString("type");
        if (type == lineStart || type == lineEnd) {
            responseBody.flowDetail.emplace_back(name, flowId, type);
        } else if (type == lineEndOptional) {
            responseBody.flowDetail.emplace_back(name, flowId, lineStart);
            responseBody.flowDetail.emplace_back(name, flowId, lineEnd);
        } else {
            ServerLog::Warn("Unknown flow type. type:", type);
        }
    }
    return true;
}

bool JsonTraceDatabase::QueryUnitsMetadata(const std::string &fileId,
    std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData)
{
    std::string sql = "SELECT pt.pid, pt.process_name AS processName, pt.label, pt.tid, pt.thread_name AS threadName,"
        " s.maxDepth, pt.name, pt.args"
        " FROM ("
        " SELECT p.pid,"
        " CASE WHEN p.process_name IS NULL THEN 'Process ' || p.pid ELSE p.process_name END AS process_name, "
        " p.label, p.process_sort_index, t.tid, t.thread_name,"
        " t.track_id, t.thread_sort_index, c.name, c.args"
        " FROM " +
        processTable + " p LEFT JOIN " + threadTable + " t ON p.pid = t.pid" +
        " LEFT JOIN (SELECT pid, name, args FROM " + counterTable +
        " GROUP BY name, pid) c "
        " ON c.pid = p.pid) AS pt "
        " LEFT JOIN ("
        " SELECT max( depth ) + 1 AS maxDepth, track_id"
        " FROM " +
        sliceTable +
        " INNER JOIN thread USING (track_id) GROUP BY track_id"
        " ) AS s ON s.track_id = pt.track_id"
        " WHERE pt.process_name is not null AND (maxDepth is not null OR pt.tid is null"
        " OR pt.name == pt.process_name OR pt.name == pt.thread_name)"
        " ORDER BY pt.process_sort_index ASC, pt.thread_sort_index ASC, pt.name ASC;";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryUnitsMetadata failed!.");
        return false;
    }
    auto resultSet = stmt->ExecuteQuery();
    std::vector<MetaDataDto> metaDataVec;
    while (resultSet->Next()) {
        int col = resultStartIndex;
        MetaDataDto metaDataDto;
        metaDataDto.pid = resultSet->GetString("pid");
        metaDataDto.processName = resultSet->GetString("processName");
        metaDataDto.label = resultSet->GetString("label");
        metaDataDto.threadId = resultSet->GetString("tid");
        metaDataDto.threadName = resultSet->GetString("threadName");
        metaDataDto.maxDepth = resultSet->GetInt32("maxDepth");
        metaDataDto.name = resultSet->GetString("name");
        metaDataDto.args = resultSet->GetString("args");
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
    std::string sql = "SELECT min(timestamp) as minTimestamp, max(timestamp) as maxTimestamp FROM "
        "(SELECT timestamp FROM " +
        sliceTable + " UNION SELECT timestamp FROM " + counterTable + ")";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryExtremumTimestamp failed!.");
        return false;
    }
    auto resultSet = stmt->ExecuteQuery();
    while (resultSet->Next()) {
        int col = resultStartIndex;
        min = resultSet->GetUint64("minTimestamp");
        max = resultSet->GetUint64("maxTimestamp");
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
}

int JsonTraceDatabase::SearchSliceNameCount(const std::string &name)
{
    std::string sql = "SELECT count(*) FROM " + sliceTable + " WHERE name like '%'||?||'%'";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QuerySliceNameCount failed!.");
        return false;
    }
    auto resultSet = stmt->ExecuteQuery(name);
    if (resultSet->Next()) {
        return resultSet->GetInt32(resultStartIndex);
    }
    return 0;
}

bool JsonTraceDatabase::SearchSliceName(const std::string &name, int index, uint64_t minTimestamp,
    Protocol::SearchSliceBody &responseBody)
{
    std::string sql = "SELECT pid, tid, timestamp - ? as startTime, duration, depth"
        " FROM " +
        sliceTable + " JOIN " + threadTable +
        " USING (track_id)"
        " WHERE name like '%'||?||'%'"
        " ORDER BY timestamp LIMIT 1 OFFSET ?";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QuerySliceName failed!.");
        return false;
    }
    auto resultSet = stmt->ExecuteQuery(minTimestamp, name, index);
    if (!resultSet->Next()) {
        return false;
    }
    int col = resultStartIndex;
    responseBody.pid = resultSet->GetString("pid");
    responseBody.tid = resultSet->GetString("tid");
    responseBody.startTime = resultSet->GetUint64("startTime");
    responseBody.duration = resultSet->GetUint64("duration");
    responseBody.depth = resultSet->GetInt32("depth");
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
    while (resultSet->Next()) {
        categories.emplace_back(resultSet->GetString(resultStartIndex));
    }
    return true;
}

void JsonTraceDatabase::DeleteInvalidFlowData()
{
    std::string sql = "DELETE from " + flowTable +
        " Where id IN"
        " (select id FROM " +
        flowTable + " GROUP BY flow_id HAVING count(flow_id) < 2)";
    if (!ExecSql(sql)) {
        ServerLog::Error("DeleteInvalidFlowData failed!. ", sqlite3_errmsg(db));
    }
}

bool JsonTraceDatabase::QueryFlowCategoryEvents(FlowCategoryEventsParams &params, uint64_t minTimestamp,
    std::vector<std::unique_ptr<FlowEvent>> &flowDetailList)
{
    if (params.timePerPx == 0) {
        ServerLog::Error("QueryFlowCategoryEvents. timePerPx is zero.");
        return false;
    }
    std::string sql = "SELECT flow.type, flow.flow_id, thread.pid, thread.tid, slice.depth, flow.timestamp - ?"
        "FROM " +
        flowTable + " " + "LEFT JOIN " + sliceTable + " USING (track_id, timestamp) " + "JOIN " + threadTable +
        " USING (track_id) "
        "WHERE flow_id IN "
        "(SELECT flow_id "
        "from "
        "(SELECT flow_id, ROUND(flow.timestamp / ?) as rank "
        "FROM flow "
        "WHERE flow_id IN "
        "(SELECT flow_id FROM flow "
        "WHERE cat = ? "
        "AND ((timestamp >= ? AND (type = 'f' OR type = 't')) "
        "OR (timestamp <= ? AND (type = 's' OR type = 't'))"
        ")"
        "GROUP BY flow_id HAVING COUNT(flow_id) >= 2) "
        "GROUP BY track_id, type, rank HAVING max(timestamp)"
        ")"
        ")"
        "ORDER BY flow.flow_id, timestamp;";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryFlowCategoryEvents failed!.");
        return false;
    }
    auto resultSet = stmt->ExecuteQuery(minTimestamp, params.timePerPx * lowImage, params.category,
        params.startTime + minTimestamp, params.endTime + minTimestamp);
    std::vector<FlowCategoryEventsDto> flowEventsVec;
    while (resultSet->Next()) {
        int col = resultStartIndex;
        FlowCategoryEventsDto flowCategoryEventsDto;
        flowCategoryEventsDto.type = resultSet->GetString(col++);
        flowCategoryEventsDto.flowId = resultSet->GetString(col++);
        flowCategoryEventsDto.pid = resultSet->GetString(col++);
        flowCategoryEventsDto.tid = resultSet->GetString(col++);
        flowCategoryEventsDto.depth = resultSet->GetInt32(col++);
        flowCategoryEventsDto.timestamp = resultSet->GetUint64(col++);
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
        if (type == lineStart || flowId != curFlowId) {
            location.pid = flow.pid;
            location.tid = flow.tid;
            location.depth = flow.depth;
            location.timestamp = flow.timestamp;
            location.type = type;
            locationPtr = &location;
        } else if ((type == lineEnd || type == lineEndOptional) && flowId == curFlowId) {
            auto flowEvent = std::make_unique<FlowEvent>();
            flowEvent->category = category;
            flowEvent->from = *locationPtr;
            flowEvent->to.pid = flow.pid;
            flowEvent->to.tid = flow.tid;
            flowEvent->to.depth = flow.depth;
            flowEvent->to.timestamp = flow.timestamp;
            locationPtr = &(flowEvent->to);
            if (flowEvent->from.type == lineStart) {
                flowDetailList.emplace_back(std::move(flowEvent));
            }
        }
        curFlowId = flowId;
    }
}

bool JsonTraceDatabase::QueryUnitCounter(Protocol::UnitCounterParams &params, uint64_t minTimestamp,
    std::vector<Protocol::UnitCounterData> &dataList)
{
    std::string sql = "SELECT timestamp - ? as startTime, args"
        " FROM " +
        counterTable +
        " WHERE pid = ? AND name = ?"
        " AND startTime >= ? AND startTime <= ?"
        " ORDER BY timestamp ASC";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryUnitCounter failed!.");
        return false;
    }
    auto resultSet = stmt->ExecuteQuery(minTimestamp, params.pid, params.threadName, params.startTime, params.endTime);
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
    std::string stepCondition;
    sqlite3_stmt *stmt = nullptr;
    int index = bindStartIndex;
    if (!requestParams.stepId.empty() && requestParams.stepId != "ALL") {
        stepCondition.append(" and step_id =? ");
    }
    std::string sql = "SELECT sum(duration) as duration,accelerator_core as acceleratorCore FROM kernel_detail"
        " WHERE accelerator_core in ('AI_CPU','AI_CORE',"
        " 'AI_VECTOR_CORE', 'MIX_AIC', 'MIX_AIV', 'FFTS_PLUS') " +
        stepCondition + " GROUP BY accelerator_core";
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
    std::string timestampCondition;
    uint64_t min;
    uint64_t max;
    if (!requestParams.stepId.empty()) {
        QueryStepDuration(requestParams.stepId, min, max);
        timestampCondition = " and timestamp >= ? and timestamp <= ? ";
    }
    std::string sql = "select duration / 1000, t.thread_name as overlapType from (select sum(duration) as duration,"
        " track_id from " +
        sliceTable +
        " where track_id in (select track_id from thread where thread_name "
        " in ('Communication(Not Overlapped)', 'Communication')) " +
        timestampCondition + " group by track_id) s left join thread t on s.track_id=t.track_id";
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


bool JsonTraceDatabase::QueryPythonViewData(const Protocol::SystemViewParams &requestParams,
    Protocol::SystemViewBody &responseBody)
{
    std::string searchName = "%" + requestParams.searchName + "%";
    const LayerStatData &data = QueryLayerData(requestParams.layer, searchName);
    double layerOperatorTime = data.allOperatorTime;
    std::string orderBy;
    if (requestParams.order == "descend") {
        orderBy = " order by " + requestParams.orderBy + " DESC ";
    } else {
        orderBy = " order by " + requestParams.orderBy + " ASC ";
    }
    std::string sql = "SELECT name, ROUND(cast(sum(duration) as double) * 100 / ?, 2) as "
        "time, sum(duration) / 1000.0 as totalTime, count(1) as numberCalls, "
        "ROUND(avg(duration) / 1000.0, 4) as avg, "
        "min(duration) / 1000.0 as min, max(duration) / 1000.0 as max "
        "FROM slice WHERE name LIKE ? AND slice.track_id IN ( SELECT track_id "
        "FROM process JOIN thread t ON process.pid = t.pid "
        "WHERE process_name = ? ) "
        "GROUP BY name " +
        orderBy + " limit ? offset ?";
    uint64_t offset = (requestParams.current - 1) * requestParams.pageSize;
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryPythonViewData, fail to prepare sql.");
        return false;
    }
    auto resultSet =
        stmt->ExecuteQuery(layerOperatorTime, searchName, requestParams.layer, requestParams.pageSize, offset);
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
    std::string sql = "SELECT sum(duration) AS totalTime, count(distinct name) FROM slice "
        "WHERE name LIKE ? and slice.track_id IN "
        "( SELECT track_id FROM process JOIN thread t ON process.pid = t.pid "
        "WHERE process_name = ? ) ";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryLayerOperatorTime, fail to prepare sql.");
        return layerStatData;
    }
    auto resultSet = stmt->ExecuteQuery(name, layer);
    if (resultSet->Next()) {
        layerStatData.allOperatorTime = resultSet->GetDouble("totalTime");
        layerStatData.total = resultSet->GetUint64("count(distinct name)");
    }
    return layerStatData;
}

std::vector<std::string> JsonTraceDatabase::QueryCoreType()
{
    std::vector<std::string> acceleratorCoreList;
    std::string sql = "SELECT DISTINCT accelerator_core FROM " + kernelDetail + " ORDER BY accelerator_core";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryCoreType, fail to prepare sql.");
        return acceleratorCoreList;
    }
    auto resultSet = stmt->ExecuteQuery();
    while (resultSet->Next()) {
        std::string res = resultSet->GetString("accelerator_core");
        acceleratorCoreList.emplace_back(res);
    }
    return acceleratorCoreList;
}

uint64_t JsonTraceDatabase::QueryTotalKernel(const std::string &coreType, const std::string &name)
{
    std::string sql = "SELECT count(*) FROM kernel_detail where name LIKE ?";
    if (!coreType.empty()) {
        sql += " AND accelerator_core = ? ";
    }
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryTotalKernel, fail to prepare sql.");
        return 0;
    }
    if (!coreType.empty()) {
        stmt->BindParams(coreType);
    }
    auto resultSet = stmt->ExecuteQuery(name);
    uint64_t total = 0;
    if (resultSet->Next()) {
        total = resultSet->GetUint64("count(*)");
    }
    return total;
}

bool JsonTraceDatabase::QueryKernelDetailData(const Protocol::KernelDetailsParams &requestParams,
    Protocol::KernelDetailsBody &responseBody, uint64_t minTimestamp)
{
    std::string orderBy;
    std::string coreTypes;
    if (requestParams.order == "descend") {
        orderBy = " order by " + requestParams.orderBy + " DESC";
    } else {
        orderBy = " order by " + requestParams.orderBy + " ASC";
    }
    if (!requestParams.coreType.empty()) {
        coreTypes = " AND accelerator_core = ? ";
    }
    uint64_t offset = (requestParams.current - 1) * requestParams.pageSize;
    std::string sql = "SELECT name, op_type as type, accelerator_core AS acceleratorCore, start_time AS startTime, "
        "duration, wait_time as waitTime, block_dim AS blockDim, input_shapes AS inputShapes, "
        "input_data_types AS inputDataTypes, input_formats AS inputFormats, "
        "output_shapes AS outputShapes, output_data_types AS outputDataTypes, "
        "output_formats AS outputFormats FROM kernel_detail "
        "where 1=1 and name LIKE ? " +
        coreTypes + orderBy + " limit ? offset ?";
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
    std::string sql = "SELECT depth, track_id FROM " + sliceTable + " WHERE name = ? AND timestamp = ?";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryKernelDepthAndThread, fail to prepare sql.");
        return false;
    }
    auto resultSet = stmt->ExecuteQuery(params.name, params.timestamp + minTimestamp);
    uint64_t trackId = 0;
    if (resultSet->Next()) {
        responseBody.depth = resultSet->GetUint64("depth");
        trackId = resultSet->GetUint64("track_id");
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
    std::string orderBy;
    if (requestParams.order == "descend") {
        orderBy = " ORDER BY " + requestParams.orderBy + " DESC";
    } else {
        orderBy = " ORDER BY " + requestParams.orderBy + " ASC";
    }
    std::string sql = "SELECT timestamp, duration FROM " + sliceTable +
                      " WHERE name = ? AND track_id = ? AND timestamp <= ? AND timestamp + duration >= ? "
                      + orderBy + " limit ? offset ?";
    uint64_t offset = (requestParams.current - 1) * requestParams.pageSize;
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryThreadSameOperatorsDetails. Failed to prepare sql.", sqlite3_errmsg(db));
        return false;
    }
    auto resultSet = stmt->ExecuteQuery(requestParams.name, traceId, endTime, startTime,
                                        requestParams.pageSize, offset);
    while (resultSet->Next()) {
        int col = resultStartIndex;
        Protocol::SameOperatorsDetails sameOperatorsDetail{};
        sameOperatorsDetail.timestamp = resultSet->GetUint64(col++);
        sameOperatorsDetail.duration = resultSet->GetUint64(col++);
        responseBody.sameOperatorsDetails.emplace_back(sameOperatorsDetail);
    }
    responseBody.currentPage = requestParams.current;
    responseBody.pageSize = requestParams.pageSize;
    responseBody.count = SameOperatorsCount(requestParams.name, traceId, startTime, endTime);
    return true;
}

uint64_t JsonTraceDatabase::SameOperatorsCount(const std::string &name, int64_t &trackId,
                                               uint64_t &startTime, uint64_t &endTime)
{
    std::string sql = "SELECT count(*) FROM "+ sliceTable +
                      " WHERE name = ? AND track_id = ? AND timestamp <= ? AND timestamp + duration >= ?;";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Fail to prepare sql for SameOperatorsCount.", sqlite3_errmsg(db));
        return 0;
    }
    auto resultSet = stmt->ExecuteQuery(name, trackId, endTime, startTime);
    uint64_t total = 0;
    if (resultSet->Next()) {
        total = resultSet->GetUint64("count(*)");
    }
    return total;
}
} // end of namespace Timeline
  // end of namespace Module
  // end of namespace Dic