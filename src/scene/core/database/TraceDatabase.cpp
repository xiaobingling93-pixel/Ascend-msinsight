/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

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
    return false;
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
        ServerLog::Error("Insert data fail. ", sqlite3_errmsg(db));
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

} // end of namespace Core
} // end of namespace Scene
} // end of namespace Dic