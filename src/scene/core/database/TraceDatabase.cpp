/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include <algorithm>
#include "TraceDatabase.h"
#include "ServerLog.h"

namespace Dic {
namespace Scene {
namespace Core {
using namespace Dic::Server;
TraceDatabase::~TraceDatabase()
{
    if (initStmt) {
        ReleaseStmt();
    }
}

bool TraceDatabase::InitStmt()
{
    if (initStmt) {
        return true;
    }
    std::string sql = "INSERT INTO " + sliceTable +
                      " (timestamp, duration, name, track_id, cat, args) VALUES (round(? * 1000),round(? * 1000),?,?,?,?);";
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &insertSliceStmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare slice statement. error:", sqlite3_errmsg(db));
        return false;
    }
    sql = "INSERT INTO " + processTable + " (pid, process_name) VALUES (?, ?) " +
          "ON CONFLICT (pid) DO UPDATE SET process_name = excluded.process_name;";
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &updateProcessNameStmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare updateProcessName statement. error:", sqlite3_errmsg(db));
        return false;
    }
    sql = "INSERT INTO " + processTable + " (pid, label) VALUES (?, ?)" +
          " ON CONFLICT (pid) DO UPDATE SET label = excluded.label;";
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &updateProcessLabelStmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare updateProcessLabel statement. error:", sqlite3_errmsg(db));
        return false;
    }
    sql = "INSERT INTO " + processTable + " (pid, process_sort_index) VALUES (?, ?)" +
          "ON CONFLICT (pid) DO UPDATE SET process_sort_index = excluded.process_sort_index;";
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &updateProcessSortIndexStmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare updateProcessSortIndex statement. error:", sqlite3_errmsg(db));
        return false;
    }
    sql = "INSERT INTO " + threadTable + " (track_id, tid, pid, thread_name) VALUES (?, ?, ?, ?)" +
          " ON CONFLICT (track_id) DO UPDATE " +
          " SET tid = excluded.tid, pid = excluded.pid, thread_name = excluded.thread_name;";
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &updateThreadNameStmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare updateThreadName statement. error:", sqlite3_errmsg(db));
        return false;
    }
    sql = "INSERT INTO " + threadTable + " (track_id, thread_sort_index) VALUES (?, ?) " +
          " ON CONFLICT (track_id) DO UPDATE SET thread_sort_index = excluded.thread_sort_index;";
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &updateThreadSortIndexStmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare updateThreadSortIndex statement. error:", sqlite3_errmsg(db));
        return false;
    }
    sql = "INSERT INTO " + flowTable + " (flow_id, name, track_id, timestamp, cat, type)" +
          " VALUES (?,?,?,round(? * 1000),?,?);";
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &insertFlowStmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare insertFlow statement. error:", sqlite3_errmsg(db));
        return false;
    }
    return true;
}

void TraceDatabase::ReleaseStmt()
{
    if (insertSliceStmt != nullptr) {
        sqlite3_finalize(insertSliceStmt);
    }
    if (updateProcessNameStmt != nullptr) {
        sqlite3_finalize(updateProcessNameStmt);
    }
    if (updateProcessLabelStmt != nullptr) {
        sqlite3_finalize(updateProcessLabelStmt);
    }
    if (updateProcessSortIndexStmt != nullptr) {
        sqlite3_finalize(updateProcessSortIndexStmt);
    }
    if (updateThreadNameStmt != nullptr) {
        sqlite3_finalize(updateThreadNameStmt);
    }
    if (updateThreadSortIndexStmt != nullptr) {
        sqlite3_finalize(updateThreadSortIndexStmt);
    }
    if (insertFlowStmt != nullptr) {
        sqlite3_finalize(insertFlowStmt);
    }
}

bool TraceDatabase::SetConfig()
{
    if (!isOpen) {
        ServerLog::Error("Failed to set config. Database is not open.");
        return false;
    }
    return ExecSql("PRAGMA synchronous = OFF; PRAGMA journal_mode = MEMORY;");
}

bool TraceDatabase::CreateTable()
{
    if (!isOpen) {
        ServerLog::Error("Failed to set config. Database is not open.");
        return false;
    }
    if (!ExecSql("DROP INDEX IF EXISTS " + idIndex + "; DROP INDEX IF EXISTS " + trackIdTimeIndex + ";")) {
        ServerLog::Warn("Failed to drop index.");
    }
    std::string sql =
        "CREATE TABLE " + sliceTable + " (id INTEGER PRIMARY KEY AUTOINCREMENT, timestamp INTEGER, duration INTEGER,"
                                       " name TEXT, depth INTEGER, track_id INTEGER, cat TEXT, args TEXT);" +
        "CREATE TABLE " + threadTable + " (track_id INTEGER PRIMARY KEY, tid INTEGER, pid TEXT, thread_name TEXT," +
                                        " thread_sort_index INTEGER);" +
        "CREATE TABLE " + processTable + " (pid TEXT PRIMARY KEY, process_name TEXT, label TEXT," +
                                         " process_sort_index INTEGER);" +
        "CREATE TABLE " + flowTable + " (id INTEGER PRIMARY KEY AUTOINCREMENT, flow_id TEXT, name TEXT, cat TEXT," +
                                      " track_id INTEGER, timestamp INTEGER, type TEXT);";
    return ExecSql(sql);
}

bool TraceDatabase::CreateIndex()
{
    if (!isOpen) {
        ServerLog::Error("Failed to creat index. Database is not open.");
        return false;
    }
    std::string sql = "CREATE INDEX " + idIndex + " ON " + sliceTable + " (id);" +
        "CREATE INDEX " + trackIdTimeIndex + " ON " + sliceTable + " (track_id, timestamp)";
    return ExecSql(sql);
}

bool TraceDatabase::InsertSlice(const json_t &json)
{
    double ts, dur;
    int64_t trackId;
    std::string name, cat, args;
    try {
        ts = json["ts"];
        dur = json["dur"];
        name = json["name"];
        trackId = json["track_id"];
        cat = json.contains("cat") ? json["cat"] : "";
        args = json["args"].dump();
    } catch (std::exception &e) {
        ServerLog::Error("Failed to insert slice. error:", e.what(), ". json:", json.dump());
        return false;
    }
    sqlite3_reset(insertSliceStmt);
    int idx = bindStartIndex;
    sqlite3_bind_double(insertSliceStmt, idx++, ts);
    sqlite3_bind_double(insertSliceStmt, idx++, dur);
    sqlite3_bind_text(insertSliceStmt, idx++, name.c_str(), name.length(), SQLITE_TRANSIENT);
    sqlite3_bind_int64(insertSliceStmt, idx++, trackId);
    sqlite3_bind_text(insertSliceStmt, idx++, cat.c_str(), cat.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(insertSliceStmt, idx++, args.c_str(), args.length(), SQLITE_TRANSIENT);
    auto result = sqlite3_step(insertSliceStmt);
    if (result != SQLITE_DONE) {
        ServerLog::Error("Insert slice data fail. ", sqlite3_errmsg(db));
        return false;
    }
    return true;
}

bool TraceDatabase::UpdateProcessName(const json_t &json)
{
    std::string pid, name;
    try {
        pid = json["pid"];
        name = json["args"]["name"];
    } catch (std::exception &e) {
        ServerLog::Error("Failed to update process name. error:", e.what());
        return false;
    }
    sqlite3_reset(updateProcessNameStmt);
    int idx = bindStartIndex;
    sqlite3_bind_text(updateProcessNameStmt, idx++, pid.c_str(), pid.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(updateProcessNameStmt, idx++, name.c_str(), name.length(), SQLITE_TRANSIENT);
    auto result = sqlite3_step(updateProcessNameStmt);
    if (result != SQLITE_DONE) {
        ServerLog::Error("Update process name fail. ", sqlite3_errmsg(db));
        return false;
    }
    return true;
}

bool TraceDatabase::UpdateProcessLabel(const json_t &json)
{
    std::string pid, label;
    try {
        pid = json["pid"];
        label = json["args"]["labels"];
    } catch (std::exception &e) {
        ServerLog::Error("Failed to update process label. error:", e.what());
        return false;
    }
    sqlite3_reset(updateProcessLabelStmt);
    int idx = bindStartIndex;
    sqlite3_bind_text(updateProcessLabelStmt, idx++, pid.c_str(), pid.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(updateProcessLabelStmt, idx++, label.c_str(), label.length(), SQLITE_TRANSIENT);
    auto result = sqlite3_step(updateProcessLabelStmt);
    if (result != SQLITE_DONE) {
        ServerLog::Error("Update process label fail. ", sqlite3_errmsg(db));
        return false;
    }
    return true;
}

bool TraceDatabase::UpdateProcessSortIndex(const json_t &json)
{
    std::string pid;
    int64_t sortIndex;
    try {
        pid = json["pid"];
        sortIndex = json["args"]["sort_index"];
    } catch (std::exception &e) {
        ServerLog::Error("Failed to update process sort index. error:", e.what());
        return false;
    }
    sqlite3_reset(updateProcessSortIndexStmt);
    int idx = bindStartIndex;
    sqlite3_bind_text(updateProcessSortIndexStmt, idx++, pid.c_str(), pid.length(), SQLITE_TRANSIENT);
    sqlite3_bind_int64(updateProcessSortIndexStmt, idx++, sortIndex);
    auto result = sqlite3_step(updateProcessSortIndexStmt);
    if (result != SQLITE_DONE) {
        ServerLog::Error("Update process sort index fail. ", sqlite3_errmsg(db));
        return false;
    }
    return true;
}

bool TraceDatabase::UpdateThreadName(const json_t &json)
{
    int64_t trackId, tid;
    std::string pid, threadName;
    try {
        trackId = json["track_id"];
        tid = json["tid"];
        pid = json["pid"];
        threadName = json["args"]["name"];
    } catch (std::exception &e) {
        ServerLog::Error("Failed to update thread name. error:", e.what());
        return false;
    }
    sqlite3_reset(updateThreadNameStmt);
    int idx = bindStartIndex;
    sqlite3_bind_int64(updateThreadNameStmt, idx++, trackId);
    sqlite3_bind_int64(updateThreadNameStmt, idx++, tid);
    sqlite3_bind_text(updateThreadNameStmt, idx++, pid.c_str(), pid.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(updateThreadNameStmt, idx++, threadName.c_str(), threadName.length(), SQLITE_TRANSIENT);
    auto result = sqlite3_step(updateThreadNameStmt);
    if (result != SQLITE_DONE) {
        ServerLog::Error("Update thread name fail. ", sqlite3_errmsg(db));
        return false;
    }
    return true;
}

bool TraceDatabase::UpdateThreadSortIndex(const json_t &json)
{
    int64_t trackId, sortIndex;
    try {
        trackId = json["track_id"];
        sortIndex = json["args"]["sort_index"];
    } catch (std::exception &e) {
        ServerLog::Error("Failed to update thread sort index. error:", e.what());
        return false;
    }
    sqlite3_reset(updateThreadSortIndexStmt);
    int idx = bindStartIndex;
    sqlite3_bind_int64(updateThreadSortIndexStmt, idx++, trackId);
    sqlite3_bind_int64(updateThreadSortIndexStmt, idx++, sortIndex);
    auto result = sqlite3_step(updateThreadSortIndexStmt);
    if (result != SQLITE_DONE) {
        ServerLog::Error("Update thread sort index fail. ", sqlite3_errmsg(db));
        return false;
    }
    return true;
}

bool TraceDatabase::InsertFlow(const json_t &json)
{
    int64_t trackId, ts;
    std::string flowId, name, cat, ph;
    try {
        flowId = to_string(json["id"]);
        name = json["name"];
        trackId = json["track_id"];
        ts = json["ts"];
        cat = json["cat"];
        ph = json["ph"];
    } catch (std::exception &e) {
        ServerLog::Error("Failed to insert flow. error:", e.what(), json.dump());
        return false;
    }
    sqlite3_reset(insertFlowStmt);
    int idx = bindStartIndex;
    sqlite3_bind_text(insertFlowStmt, idx++, flowId.c_str(), flowId.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(insertFlowStmt, idx++, name.c_str(), name.length(), SQLITE_TRANSIENT);
    sqlite3_bind_int64(insertFlowStmt, idx++, trackId);
    sqlite3_bind_int64(insertFlowStmt, idx++, ts);
    sqlite3_bind_text(insertFlowStmt, idx++, cat.c_str(), cat.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(insertFlowStmt, idx++, ph.c_str(), ph.length(), SQLITE_TRANSIENT);
    auto result = sqlite3_step(insertFlowStmt);
    if (result != SQLITE_DONE) {
        ServerLog::Error("Insert flow fail. ", sqlite3_errmsg(db));
        return false;
    }
    return true;
}

void TraceDatabase::UpdateDepth()
{
    ServerLog::Info("UpdateDepth.");
    StartTransaction();
    auto trackList = GetTrackIdList();
    for (auto &trackId : trackList) {
        UpdateOneTrackDepth(trackId);
    }
    EndTransaction();
    ServerLog::Info("UpdateDepth end.");
}

std::vector<int64_t> TraceDatabase::GetTrackIdList()
{
    std::vector<int64_t> trackIdList;
    sqlite3_stmt *stmt = nullptr;
    std::string sql = "SELECT track_id FROM " + threadTable;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Failed to prepare sql.");
        return {};
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        trackIdList.push_back(sqlite3_column_int64(stmt, resultStartIndex));
    }
    sqlite3_finalize(stmt);
    return trackIdList;
}

void TraceDatabase::UpdateOneTrackDepth(int64_t trackId)
{
    std::vector<SliceTimeData> sliceTimeList;
    if (!SearchSliceTimeData(trackId, sliceTimeList)) {
        ServerLog::Error("Failed to search slice time data.");
        return;
    }
    std::map<int, std::vector<int64_t>> depthMap;
    CalcDepth(sliceTimeList, depthMap);
    for (auto &it : depthMap) {
        UpdateDepthByID(it.second, it.first);
    }
}

bool TraceDatabase::SearchSliceTimeData(int64_t trackId, std::vector<SliceTimeData> &sliceTimeList) {
    sqlite3_stmt *stmt = nullptr;
    std::string sql = "SELECT id, timestamp, duration FROM " + sliceTable + " WHERE track_id = ? ORDER BY timestamp;";
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Failed to prepare sql.");
        return false;
    }
    sqlite3_bind_int64(stmt, bindStartIndex, trackId);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int64_t id = sqlite3_column_int64(stmt, 0);
        int64_t ts = sqlite3_column_int64(stmt, 1);
        int64_t dur = sqlite3_column_int64(stmt, 2);
        sliceTimeList.push_back({id, ts, dur});
    }
    sqlite3_finalize(stmt);
    return true;
}

void TraceDatabase::CalcDepth(const std::vector<SliceTimeData> &sliceData, std::map<int, std::vector<int64_t>> &depthMap)
{
    std::vector<int64_t> depthCache;
    for (const auto &slice : sliceData) {
        int depth = -1;
        for (int i = 0; i < depthCache.size(); ++i) {
            if (slice.time >= depthCache[i]) {
                depthCache[i] = slice.time + slice.dur;
                depth = i;
                break;
            }
        }
        if (depth < 0) {
            depth = depthCache.size();
            depthCache.push_back(slice.time + slice.dur);
        }
        depthMap[depth].emplace_back(slice.id);
    }
}

void TraceDatabase::UpdateDepthByID(const std::vector<int64_t> &idList, int depth)
{
    static const uint64_t maxParams = 10000;
    std::string sql = "UPDATE " + sliceTable + " set depth = " + std::to_string(depth) + " WHERE id in ";
    uint64_t start = 0;
    while (start < idList.size()) {
        std::string updateSql = sql;
        uint64_t end = std::min(start + maxParams, idList.size());
        updateSql.append("(");
        for (auto i = start; i < end - 1; ++i) {
            updateSql.append(std::to_string(idList[i]) + ",");
        }
        updateSql.append(std::to_string(idList[end - 1]) + ");");
        sqlite3_stmt *stmt = nullptr;
        int result = sqlite3_prepare_v2(db, updateSql.c_str(), updateSql.length(), &stmt, nullptr);
        if (result != SQLITE_OK) {
            ServerLog::Error("Failed to prepare sql. error:", sqlite3_errmsg(db));
            return;
        }
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        start = end;
    }
}

bool TraceDatabase::QueryThreadTraces(Protocol::UnitThreadTracesParams &requestParams, Protocol::UnitThreadTracesBody &responseBody,
                                      int64_t minTimestamp, int64_t traceId) {
    std::string sql = "SELECT id, timestamp - ? as start_time, duration, name, depth, track_id "
    "FROM slice WHERE track_id = ? AND start_time >= ? AND start_time <= ? GROUP BY depth, id ORDER BY start_time;";
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result == SQLITE_OK) {
        int index = bindStartIndex;
        sqlite3_bind_int64(stmt, index++, minTimestamp);
        sqlite3_bind_int64(stmt, index++, traceId);
        sqlite3_bind_int64(stmt, index++, requestParams.startTime);
        sqlite3_bind_int64(stmt, index++, requestParams.endTime);
        std::vector<Protocol::RowThreadTrace> rowThreadTraceVec;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int col = resultStartIndex;
            Protocol::RowThreadTrace rowThreadTrace {};
            rowThreadTrace.id = static_cast<uint64_t>(sqlite3_column_int64(stmt, col++));
            rowThreadTrace.start_time = static_cast<uint64_t>(sqlite3_column_int64(stmt, col++));
            rowThreadTrace.duration = static_cast<uint64_t>(sqlite3_column_int64(stmt, col++));
            rowThreadTrace.name = sqlite3_column_string(stmt, col++);
            rowThreadTrace.depth = static_cast<uint64_t>(sqlite3_column_int64(stmt, col++));
            rowThreadTrace.trace_id = static_cast<uint64_t>(sqlite3_column_int64(stmt, col++));
            rowThreadTraceVec.emplace_back(rowThreadTrace);
        }
        std::map<int64_t, std::vector<Protocol::ThreadTraces>> threadTracesMap;
        for(auto &item : rowThreadTraceVec) {
            Protocol::ThreadTraces threadTraces {};
            threadTraces.name = item.name;
            threadTraces.duration = item.duration;
            threadTraces.startTime = item.start_time;
            threadTraces.endTime = item.start_time + item.duration;
            threadTraces.depth = item.depth;
            threadTraces.threadId = requestParams.threadId;
            if (threadTracesMap.find(item.depth) == threadTracesMap.end()) {
                std::vector<Protocol::ThreadTraces> threadTracesVec;
                threadTracesVec.emplace_back(threadTraces);
                threadTracesMap.emplace(item.depth, threadTracesVec);
            } else {
                threadTracesMap.at(item.depth).emplace_back(threadTraces);
            }
        }
        for(int i = 0; i < rowThreadTraceVec.size(); i++) {
            if (threadTracesMap.find(i) != threadTracesMap.end()) {
                responseBody.data.emplace_back(threadTracesMap.at(i));
            } else {
                std::vector<Protocol::ThreadTraces> threadTracesVec;
                responseBody.data.emplace_back(threadTracesVec);
            }
        }
    } else {
        ServerLog::Error("QueryThreadTraces failed!");
        return false;
    }
    sqlite3_finalize(stmt);
    return true;
}

bool TraceDatabase::QueryThreads(Protocol::UnitThreadsParams &requestParams, Protocol::UnitThreadsBody &responseBody,
                                 int64_t minTimestamp, int64_t traceId) {
    int64_t startTime = requestParams.startTime + minTimestamp;
    int64_t endTime = requestParams.endTime + minTimestamp;
    Protocol::ExtremumTimestamp extremumTimestamp {};
    bool isSuccessQueryExtremumTime = QueryExtremumTimeOfFirstDepth(traceId, startTime, endTime, extremumTimestamp);
    if (!isSuccessQueryExtremumTime) {
        return false;
    }
    std::string sql = "SELECT timestamp, duration, timestamp + duration AS endTime, name, depth FROM slice WHERE "
                      "TRACK_ID = ? AND TIMESTAMP <= ? AND TIMESTAMP + DURATION >= ? ORDER BY  depth ASC,timestamp ASC";
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result == SQLITE_OK) {
        int index = bindStartIndex;
        sqlite3_bind_int64(stmt, index++, traceId);
        sqlite3_bind_int64(stmt, index++, extremumTimestamp.maxTimestamp);
        sqlite3_bind_int64(stmt, index++, extremumTimestamp.minTimestamp);
        std::vector<Protocol::SimpleSlice> simpleSliceVec;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int col = resultStartIndex;
            Protocol::SimpleSlice simpleSlice {};
            simpleSlice.timestamp = static_cast<uint64_t>(sqlite3_column_int64(stmt, col++));
            simpleSlice.duration = static_cast<uint64_t>(sqlite3_column_int64(stmt, col++));
            simpleSlice.endTime = static_cast<uint64_t>(sqlite3_column_int64(stmt, col++));
            simpleSlice.name = sqlite3_column_string(stmt, col++);
            simpleSlice.depth = static_cast<uint64_t>(sqlite3_column_int64(stmt, col++));
            simpleSliceVec.emplace_back(simpleSlice);
        }
        // process data
        if (simpleSliceVec.size() == 0) {
            responseBody.emptyFlag = true;
            return true;
        }
        std::map<std::string, int64_t> selfTimeKeyValue;
        CalculateSelfTime(simpleSliceVec, selfTimeKeyValue, startTime, endTime);
        std::vector<Protocol::SimpleSlice> nRows = ThreadsInfoFilter(simpleSliceVec, startTime, endTime);
        ReduceThread(nRows, selfTimeKeyValue, responseBody);
    } else {
        ServerLog::Error("QueryThreads failed!");
        return false;
    }
    sqlite3_finalize(stmt);
    return true;
}

bool TraceDatabase::QueryExtremumTimeOfFirstDepth(int64_t trackId, int64_t startTime, int64_t endTime,  Protocol::ExtremumTimestamp &extremumTimestamp) {
    std::string sql = "SELECT min(timestamp) as minTimestamp, max(timestamp + duration) AS maxTimestamp FROM slice where "
                      "TRACK_ID = ? AND TIMESTAMP <= ? AND TIMESTAMP + DURATION >= ? AND DEPTH = 0;";
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result == SQLITE_OK) {
        int index = bindStartIndex;
        sqlite3_bind_int64(stmt, index++, trackId);
        sqlite3_bind_int64(stmt, index++, endTime);
        sqlite3_bind_int64(stmt, index++, startTime);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int col = resultStartIndex;
            extremumTimestamp.minTimestamp = static_cast<uint64_t>(sqlite3_column_int64(stmt, col++));
            extremumTimestamp.maxTimestamp = static_cast<uint64_t>(sqlite3_column_int64(stmt, col++));
        }
    } else {
        ServerLog::Error("QueryExtremumTimeOfFirstDepth failed!");
        return false;
    }
    sqlite3_finalize(stmt);
    return true;
}

void TraceDatabase::CalculateSelfTime(std::vector<Protocol::SimpleSlice> &rows, std::map<std::string, int64_t> &selfTimeKeyValue, int64_t startTime, int64_t endTime) {
    int32_t i = 0;
    int32_t j = 0;
    if (rows.size() == 0) {
        ServerLog::Error("simpleSlice array size is zero!");
        return;
    }
    int64_t tmpSelfTime = rows.at(0).duration;
    while (i < rows.size()) {
        // j滑完直接滑完所有i
        if (j == rows.size()) {
            // 处理当前tmpSelfTime
            AddData(selfTimeKeyValue, rows.at(i).name, tmpSelfTime);
            // 处理剩余元素
            while (++i < rows.size()) {
                if (rows.at(i).timestamp <= endTime && rows.at(i).endTime >= startTime) {
                    AddData(selfTimeKeyValue, rows.at(i).name, rows.at(i).duration);
                }
            }
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

void TraceDatabase::AddData(std::map<std::string, int64_t> &selfTimeKeyValue, std::string name, int64_t tmpSelfTime) {
    if (selfTimeKeyValue.find(name) != selfTimeKeyValue.end()) {
        selfTimeKeyValue.at(name) = selfTimeKeyValue.at(name) + tmpSelfTime;
    } else {
        selfTimeKeyValue.emplace(name, tmpSelfTime);
    }
}

std::vector<Protocol::SimpleSlice> TraceDatabase::ThreadsInfoFilter(std::vector<Protocol::SimpleSlice> &simpleSliceVec, int64_t startTime, int64_t endTime) {
    std::vector<Protocol::SimpleSlice> nRows;
    for (auto &row : simpleSliceVec) {
        if (row.timestamp <= endTime && row.endTime >= startTime) {
            nRows.emplace_back(row);
        }
    }
    return nRows;
}

void TraceDatabase::ReduceThread(std::vector<Protocol::SimpleSlice> &rows, std::map<std::string, int64_t> &selfTimeKeyValue, Protocol::UnitThreadsBody &responseBody) {
    for(auto &cur : rows) {
        int index = -1;
        for (int i = 0; i < responseBody.data.size(); i++) {
            if (responseBody.data[i].title == cur.name) {
                index = i;
                break;
            }
        }
        if (index == -1) {
            Protocol::Threads threads {};
            threads.title = cur.name;
            threads.wallDuration = cur.duration;
            threads.occurrences = 1;
            threads.avgWallDuration = cur.duration;
            threads.selfTime = selfTimeKeyValue.at(cur.name);
            responseBody.data.emplace_back(threads);
        } else {
            responseBody.data[index].wallDuration += cur.duration;
            responseBody.data[index].occurrences += 1;
            responseBody.data[index].avgWallDuration = responseBody.data[index].wallDuration / responseBody.data[index].occurrences;
        }
    }
}

bool TraceDatabase::QueryThreadDetail(Protocol::ThreadDetailParams &requestParams, Protocol::UnitThreadDetailBody &responseBody,
                                      int64_t minTimestamp, int64_t trackId) {
    std::string sql = "SELECT * FROM slice WHERE DEPTH = ? AND TRACK_ID = ? AND TIMESTAMP = ?";
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result == SQLITE_OK) {
        int index = bindStartIndex;
        sqlite3_bind_int64(stmt, index++, requestParams.depth);
        sqlite3_bind_int64(stmt, index++, trackId);
        sqlite3_bind_int64(stmt, index++, requestParams.startTime + minTimestamp);
        std::vector<Protocol::SliceDto> sliceDtoVec;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int col = resultStartIndex;
            Protocol::SliceDto sliceDto {};
            sliceDto.id = static_cast<uint64_t>(sqlite3_column_int64(stmt, col++));
            sliceDto.timestamp = static_cast<uint64_t>(sqlite3_column_int64(stmt, col++));
            sliceDto.duration = static_cast<uint64_t>(sqlite3_column_int64(stmt, col++));
            sliceDto.name = sqlite3_column_string(stmt, col++);
            sliceDto.depth = static_cast<uint64_t>(sqlite3_column_int64(stmt, col++));
            sliceDto.track_id = static_cast<uint64_t>(sqlite3_column_int64(stmt, col++));
            sliceDto.cat = sqlite3_column_string(stmt, col++);
            sliceDto.args = sqlite3_column_string(stmt, col++);
            sliceDtoVec.emplace_back(sliceDto);
        }
        if (sliceDtoVec.size() != 1) {
            ServerLog::Error("select slice error!");
        }
        int64_t selfTime = sliceDtoVec.at(0).duration;
        std::vector<int64_t> nextDepthResult;
        QueryDurationFromSliceByTimeRange(requestParams, sliceDtoVec, nextDepthResult, trackId);
        if (nextDepthResult.size() == 0) {
            selfTime = 0;
        } else {
            for(const auto &item : nextDepthResult) {
                selfTime -= item;
            }
        }
        responseBody.emptyFlag = false;
        responseBody.data.selfTime = selfTime;
        responseBody.data.args = sliceDtoVec[0].args;
        responseBody.data.title = sliceDtoVec[0].name;
        responseBody.data.duration = sliceDtoVec[0].duration;
        responseBody.data.cat = !sliceDtoVec[0].cat.empty() ? sliceDtoVec[0].cat : "";
    } else {
        ServerLog::Error("QueryThreadDetail failed!");
        return false;
    }
    sqlite3_finalize(stmt);
    return true;
}

bool TraceDatabase::QueryDurationFromSliceByTimeRange(Protocol::ThreadDetailParams &requestParams, const std::vector<Protocol::SliceDto> &rows,
        std::vector<int64_t> &nextDepthResult, int64_t trackId) {
    if (rows.empty()) {
        ServerLog::Error("sliceDto array is empty!");
        return false;
    }
    std::string sql = "SELECT DURATION FROM slice WHERE DEPTH = ? AND TIMESTAMP + DURATION <= ? AND TIMESTAMP >= ? AND TRACK_ID = ?";
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result == SQLITE_OK) {
        int index = bindStartIndex;
        sqlite3_bind_int64(stmt, index++, requestParams.depth + 1);
        sqlite3_bind_int64(stmt, index++, rows[0].timestamp + rows[0].duration);
        sqlite3_bind_int64(stmt, index++, rows[0].timestamp);
        sqlite3_bind_int64(stmt, index++, trackId);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int col = resultStartIndex;
            nextDepthResult.emplace_back(static_cast<uint64_t>(sqlite3_column_int64(stmt, col++)));
        }
    } else {
        ServerLog::Error("QueryDurationFromSliceByTimeRange failed!");
        return false;
    }
    sqlite3_finalize(stmt);
    return true;
}

bool TraceDatabase::QueryFlowDetail(Protocol::UnitFlowParams &requestParams, Protocol::UnitFlowBody &responseBody, int64_t minTimestamp, int64_t trackId) {
    std::string sql = "SELECT FL.NAME, FL.CAT, FL.FLOW_ID as flowId, TH.PID, TH.TID, SL.DEPTH, SL.TIMESTAMP, FL.TYPE "
                      "FROM thread TH LEFT JOIN slice SL ON SL.TRACK_ID = TH.TRACK_ID LEFT JOIN flow FL ON FL.TRACK_ID = SL.TRACK_ID "
                      "WHERE FL.TIMESTAMP = SL.TIMESTAMP AND FL.flow_id = ?";
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result == SQLITE_OK) {
        int index = bindStartIndex;
        sqlite3_bind_text(stmt, index++, requestParams.flowId.c_str(), -1, NULL);
        std::vector<Protocol::FlowDetailDto> flowDetailVec;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int col = resultStartIndex;
            Protocol::FlowDetailDto flowDetailDto {};
            flowDetailDto.name = sqlite3_column_string(stmt, col++);
            flowDetailDto.cat = sqlite3_column_string(stmt, col++);
            flowDetailDto.flowId = sqlite3_column_string(stmt, col++);
            flowDetailDto.pid = sqlite3_column_string(stmt, col++);
            flowDetailDto.tid = static_cast<uint64_t>(sqlite3_column_int64(stmt, col++));
            flowDetailDto.depth = static_cast<uint64_t>(sqlite3_column_int64(stmt, col++));
            flowDetailDto.timestamp = static_cast<uint64_t>(sqlite3_column_int64(stmt, col++));
            flowDetailDto.type = sqlite3_column_string(stmt, col++);
            flowDetailVec.emplace_back(flowDetailDto);
        }
        if(flowDetailVec.size() != 2) {
            ServerLog::Error("select location error");
            return false;
        }
        Protocol::FromTo fromLocation;
        Protocol::FromTo toLocation;
        for (auto &item : flowDetailVec) {
            item.timestamp -= minTimestamp;
            Protocol::FromTo tmpLocation;
            tmpLocation.pid = item.pid;
            tmpLocation.tid = item.tid;
            tmpLocation.timestamp = item.timestamp;
            tmpLocation.depth = item.depth;
            if (item.type == "s") {
                fromLocation = tmpLocation;
            }
            if (item.type == "f") {
                toLocation = tmpLocation;
            }
        }
        responseBody.title = flowDetailVec[0].name;
        responseBody.cat = flowDetailVec[0].cat;
        responseBody.id = flowDetailVec[0].flowId;
        responseBody.from = fromLocation;
        responseBody.to = toLocation;
    } else {
        ServerLog::Error("QueryFlowDetail failed!");
        return false;
    }
    sqlite3_finalize(stmt);
    return true;
}
} // end of namespace Core
} // end of namespace Scene
} // end of namespace Dic