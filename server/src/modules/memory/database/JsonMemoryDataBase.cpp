/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "JsonMemoryDataBase.h"
#include <vector>
#include "ServerLog.h"
#include "TraceTime.h"
#include "VirtualMemoryDataBase.h"
#include "ConstantDefs.h"


namespace Dic {
namespace Module {
namespace Memory {
using namespace Server;
JsonMemoryDataBase::JsonMemoryDataBase(std::recursive_mutex &sqlMutex) : VirtualMemoryDataBase(sqlMutex) {}

JsonMemoryDataBase::~JsonMemoryDataBase()
{
    if (hasInitStmt) {
        ReleaseStmt();
        hasInitStmt = false;
    }
    CloseDb();
}

bool JsonMemoryDataBase::SetConfig()
{
    if (!isOpen) {
        ServerLog::Error("Failed to set config. Database is not open.");
        return false;
    }
    std::string dbVersion = GetDataBaseVersion();
    std::unique_lock<std::recursive_mutex> lock(mutex);
    return ExecSql("PRAGMA synchronous = OFF; PRAGMA journal_mode = MEMORY; PRAGMA user_version = " + dbVersion + ";");
}

bool JsonMemoryDataBase::CreateTable()
{
    if (!isOpen) {
        ServerLog::Error("Failed to set config. Database is not open.");
        return false;
    }
    std::string sql =
        "CREATE TABLE " + operatorTable + " (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, " +
        "size INTEGER, allocation_time INTEGER, release_time INTEGER, duration INTEGER, "
        "active_release_time INTEGER, active_duration INTEGER, "
        "allocation_allocated INTEGER, allocation_reserve INTEGER, allocation_active INTEGER, "
        "release_allocated INTEGER, release_reserve INTEGER, release_active INTEGER, stream TEXT);" +
        "CREATE TABLE " + recordTable + " (id INTEGER PRIMARY KEY AUTOINCREMENT, component TEXT, " +
        "total_allocated INTEGER, total_reserve INTEGER, total_active INTEGER, "
        "device_type TEXT, stream TEXT, timestamp INTEGER);" +
        "CREATE TABLE " + staticOpTable + " (id INTEGER PRIMARY KEY AUTOINCREMENT, device_id TEXT, " +
        "op_name TEXT, model_name TEXT, graph_id TEXT, node_index_start INTEGER, " +
        "node_index_end INTEGER, size INTEGER);";
    std::unique_lock<std::recursive_mutex> lock(mutex);
    return ExecSql(sql);
}

bool JsonMemoryDataBase::DropTable()
{
    std::vector<std::string> tables = {operatorTable, recordTable, staticOpTable};
    std::unique_lock<std::recursive_mutex> lock(mutex);
    return DropSomeTables(tables);
}

bool JsonMemoryDataBase::InitStmt()
{
    if (hasInitStmt) {
        return true;
    }
    std::string sql = "INSERT INTO " + operatorTable + " (name, size, allocation_time, release_time, duration, "
          "active_release_time, active_duration, allocation_allocated, allocation_reserve, allocation_active, "
          "release_allocated, release_reserve, release_active, stream)" +
          " VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?)";
    for (int i = 0; i < cacheSize - 1; ++i) {
        sql.append(",(?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
    }
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &insertOperatorStmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare insert Operator statement. error:", sqlite3_errmsg(db));
        return false;
    }
    sql = "INSERT INTO " + recordTable +
            " (component, total_allocated, total_reserve, total_active, device_type, stream, timestamp)" +
            " VALUES (?,?,?,?,?,?,?)";
    for (int i = 0; i < cacheSize - 1; ++i) {
        sql.append(",(?,?,?,?,?,?,?)");
    }
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &insertRecordStmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare insert Record statement. error:", sqlite3_errmsg(db));
        return false;
    }
    sql = "INSERT INTO " + staticOpTable +
            " (device_id, op_name, model_name, graph_id, node_index_start, node_index_end, size)" +
            " VALUES (?,?,?,?,?,?,?)";
    for (int i = 0; i < cacheSize - 1; ++i) {
        sql.append(",(?,?,?,?,?,?,?)");
    }
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &insertStaticOpStmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare insert Static Op statement. error:", sqlite3_errmsg(db));
        return false;
    }

    hasInitStmt = true;
    return true;
}

void JsonMemoryDataBase::ReleaseStmt()
{
    if (insertOperatorStmt != nullptr) {
        insertOperatorStmt = nullptr;
    }
    if (insertRecordStmt != nullptr) {
        insertRecordStmt = nullptr;
    }
    if (insertStaticOpStmt != nullptr) {
        insertStaticOpStmt = nullptr;
    }
}

void JsonMemoryDataBase::InsertOperatorDetailList(const std::vector<Operator> &eventList)
{
    sqlite3_stmt *stmt = GetOperatorStmt(eventList.size());
    if (stmt == nullptr) {
        ServerLog::Error("Failed to get operator stmt.");
        return;
    }
    int idx = bindStartIndex;
    for (const auto &event : eventList) {
        sqlite3_bind_text(stmt, idx++, event.name.c_str(), event.name.length(), SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, idx++, event.size);
        sqlite3_bind_int64(stmt, idx++, event.allocationTime);
        sqlite3_bind_int64(stmt, idx++, event.releaseTime);
        sqlite3_bind_double(stmt, idx++, event.duration);
        sqlite3_bind_int64(stmt, idx++, event.activeReleaseTime);
        sqlite3_bind_double(stmt, idx++, event.activeDuration);
        sqlite3_bind_double(stmt, idx++, event.allocationAllocated);
        sqlite3_bind_double(stmt, idx++, event.allocationReserved);
        sqlite3_bind_double(stmt, idx++, event.allocationActive);
        sqlite3_bind_double(stmt, idx++, event.releaseAllocated);
        sqlite3_bind_double(stmt, idx++, event.releaseReserved);
        sqlite3_bind_double(stmt, idx++, event.releaseActive);
        sqlite3_bind_text(stmt, idx++, event.streamId.c_str(), event.streamId.length(), SQLITE_TRANSIENT);
    }
    std::unique_lock<std::recursive_mutex> lock(mutex);
    auto result = sqlite3_step(stmt);
    if (eventList.size() != cacheSize) {
        sqlite3_finalize(stmt);
    }
    if (result != SQLITE_DONE) {
        ServerLog::Error("Insert operator fail. ", sqlite3_errmsg(db));
    }
}

void JsonMemoryDataBase::insertOperatorDetail(const Operator &event)
{
    operatorCache.emplace_back(event);
    if (operatorCache.size() == cacheSize) {
        InsertOperatorDetailList(operatorCache);
        operatorCache.clear();
    }
}

void JsonMemoryDataBase::InsertRecordDetailList(const std::vector<Record> &eventList)
{
    sqlite3_stmt *stmt = GetRecordStmt(eventList.size());
    if (stmt == nullptr) {
        ServerLog::Error("Failed to get Record stmt.");
        return;
    }
    int idx = bindStartIndex;
    for (const auto &event : eventList) {
        sqlite3_bind_text(stmt, idx++, event.component.c_str(), event.component.length(), SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, idx++, event.totalAllocated);
        sqlite3_bind_double(stmt, idx++, event.totalReserved);
        sqlite3_bind_double(stmt, idx++, event.totalActivated);
        sqlite3_bind_text(stmt, idx++, event.deviceType.c_str(), event.deviceType.length(), SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, idx++, event.streamId.c_str(), event.streamId.length(), SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, idx++, event.timesTamp);
    }
    std::unique_lock<std::recursive_mutex> lock(mutex);
    auto result = sqlite3_step(stmt);
    if (eventList.size() != cacheSize) {
        sqlite3_finalize(stmt);
    }
    if (result != SQLITE_DONE) {
        ServerLog::Error("Insert operator fail. ", sqlite3_errmsg(db));
    }
}

void JsonMemoryDataBase::InsertStaticOpDetailList(const std::vector<StaticOp> &eventList)
{
    sqlite3_stmt *stmt = GetStaticOpStmt(eventList.size());
    if (stmt == nullptr) {
        ServerLog::Error("Failed to get Record stmt.");
        return;
    }
    int idx = bindStartIndex;
    for (const auto &event : eventList) {
        sqlite3_bind_text(stmt, idx++, event.deviceId.c_str(), event.deviceId.length(), SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, idx++, event.opName.c_str(), event.opName.length(), SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, idx++, event.modelName.c_str(), event.modelName.length(), SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, idx++, event.graphId.c_str(), event.graphId.length(), SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, idx++, event.nodeIndexStart);
        sqlite3_bind_int64(stmt, idx++, event.nodeIndexEnd);
        sqlite3_bind_double(stmt, idx++, event.size);
    }
    std::unique_lock<std::recursive_mutex> lock(mutex);
    auto result = sqlite3_step(stmt);
    if (eventList.size() != cacheSize) {
        sqlite3_finalize(stmt);
    }
    if (result != SQLITE_DONE) {
        ServerLog::Error("Insert StaticOp fail. ", sqlite3_errmsg(db));
    }
}

void JsonMemoryDataBase::insertRecordDetail(const Record &event)
{
    recordCache.emplace_back(event);
    if (recordCache.size() == cacheSize) {
        InsertRecordDetailList(recordCache);
        recordCache.clear();
    }
}

void JsonMemoryDataBase::insertStaticOpDetail(const StaticOp &event)
{
    staticOpCache.emplace_back(event);
    if (staticOpCache.size() == cacheSize) {
        InsertStaticOpDetailList(staticOpCache);
        staticOpCache.clear();
    }
}

bool JsonMemoryDataBase::UpdateParseStatus(const std::string& status)
{
    return UpdateValueIntoStatusInfoTable(memoryParseStatus, status);
}

bool JsonMemoryDataBase::HasFinishedParseLastTime()
{
    return CheckValueFromStatusInfoTable(memoryParseStatus, FINISH_STATUS);
}

uint64_t JsonMemoryDataBase::QueryMinOperatorAllocationTime()
{
    std::string sql = "Select MIN(allocation_time) FROM " + operatorTable + " WHERE allocation_time != 0";
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Failed to prepare sql for query min operator allocation time.", sqlite3_errmsg(db));
        return 0;
    }
    uint64_t min;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        min = sqlite3_column_int64(stmt, col++);
    }
    sqlite3_finalize(stmt);
    return min;
}

uint64_t  JsonMemoryDataBase::QueryMinRecordTimestamp()
{
    std::string sql = "Select MIN(timestamp) FROM " + recordTable + " WHERE timestamp != 0";
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Failed to prepare sql for query min record timestamp.", sqlite3_errmsg(db));
        return 0;
    }
    uint64_t min;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        min = sqlite3_column_int64(stmt, col++);
    }
    sqlite3_finalize(stmt);
    return min;
}

std::string  JsonMemoryDataBase::GetOperatorSql(Protocol::MemoryOperatorParams &requestParams)
{
    uint64_t startTime = Timeline::TraceTime::Instance().GetStartTime();
    std::string sql =
        "SELECT name, size, CASE WHEN allocation_time == 0 THEN 'NA' ELSE "
        "ROUND((allocation_time - " +
        std::to_string(startTime) + ") / (1000.0 * 1000.0), 2) END AS allocationTimestamp, "
        "CASE WHEN release_time == 0 THEN 'NA' ELSE ROUND((release_time - " +
        std::to_string(startTime) + ") / (1000.0 * 1000.0), 2) "
        "END AS release_time, ROUND(duration / 1000.0, 2) as duration, "
        "CASE WHEN active_release_time == 0 THEN 'NA' ELSE ROUND((active_release_time - " +
        std::to_string(startTime) + ") / (1000.0 * 1000.0), 2) "
        "END AS activeReleaseTime, ROUND(active_duration / 1000.0, 2) as active_duration, "
        "ROUND(allocation_allocated, 2) as allocation_allocated,ROUND(allocation_reserve, 2) as allocation_reserve, " +
        "ROUND(allocation_active, 2) as allocation_active, ROUND(release_allocated, 2) as  release_allocated, " +
        "ROUND(release_reserve, 2) as release_reserve, ROUND(release_active, 2) as release_active, " +
        "stream FROM " + operatorTable + " WHERE name LIKE ?";
    AddOperatorSql(requestParams, sql);
    return sql;
}

std::string  JsonMemoryDataBase::GetStaticOperatorSql(Protocol::StaticOperatorListParams &requestParams)
{
    std::string sql =
        "SELECT device_id, op_name, node_index_start, node_index_end, ROUND(size / 1024.0, 2) as size"
        " FROM " + staticOpTable +
        " WHERE op_name LIKE ? AND op_name <> 'TOTAL'";
    AddStableOperatorSql(requestParams, sql);
    return sql;
}

std::string JsonMemoryDataBase::GetStaticGraphStartSql(Protocol::StaticOperatorGraphParams &requestParams)
{
    std::string sql =
            "SELECT node_index_start, size FROM " + staticOpTable +
            " WHERE op_name <> 'TOTAL' AND size <> 0 AND graph_id = ?";
    if (!requestParams.modelName.empty()) {
        sql += " AND op_name = ?";
    }
    sql += " ORDER BY node_index_start ASC";
    return sql;
}

std::string JsonMemoryDataBase::GetStaticGraphEndSql(Protocol::StaticOperatorGraphParams &requestParams)
{
    std::string sql =
            "SELECT node_index_end, size FROM " + staticOpTable +
            " WHERE op_name <> 'TOTAL' AND size <> 0 AND graph_id = ?";
    if (!requestParams.modelName.empty()) {
        sql += " AND op_name = ?";
    }
    sql += " ORDER BY node_index_end ASC";
    return sql;
}

bool JsonMemoryDataBase::QueryMemoryType(std::string &type, std::vector<std::string> &graphId)
{
    return ExecuteMemoryType(graphId, type);
}

bool JsonMemoryDataBase::QueryMemoryResourceType(std::string &type)
{
    std::string sql = "SELECT count(*) as nums FROM " + recordTable + " WHERE component = 'MindSpore'";
    return ExecuteMemoryResourceType(type, sql);
}

bool JsonMemoryDataBase::QueryOperatorDetail(Protocol::MemoryOperatorParams &requestParams,
    std::vector<Protocol::MemoryTableColumnAttr> &columnAttr, std::vector<Protocol::MemoryOperator> &opDetails)
{
    std::string sql = GetOperatorSql(requestParams);
    return ExecuteOperatorDetail(requestParams, columnAttr, opDetails, sql);
}

bool JsonMemoryDataBase::QueryMemoryView(Protocol::MemoryComponentParams &requestParams,
    Protocol::MemoryViewData &operatorBody)
{
    uint64_t startTime = Timeline::TraceTime::Instance().GetStartTime();
    std::string sql = "SELECT component, ROUND((timestamp - " + std::to_string(startTime) +
        ") / (1000.0 * 1000.0), 2) as timestamp, "
        "ROUND(total_allocated, 2) as total_allocated, ROUND(total_reserve, 2) as total_reserve, "
        "ROUND(total_active, 2) as total_active, stream FROM " +
        recordTable + " WHERE 1==1";
    return ExecuteQueryMemoryView(requestParams, operatorBody, sql);
}

bool JsonMemoryDataBase::QueryStaticOperatorList(Protocol::StaticOperatorListParams &requestParams,
                                                 std::vector<Protocol::MemoryTableColumnAttr> &columnAttr,
                                                 std::vector<Protocol::StaticOperatorItem> &opDetails)
{
    std::string sql = GetStaticOperatorSql(requestParams);
    return ExecuteStaticOperatorDetail(requestParams, columnAttr, opDetails, sql);
}

bool JsonMemoryDataBase::QueryStaticOperatorGraph(Protocol::StaticOperatorGraphParams &requestParams,
                                                  Protocol::StaticOperatorGraphItem &graphItem)
{
    std::string totalSql = "SELECT size FROM " + staticOpTable +
                           " WHERE op_name = 'TOTAL' AND graph_id = ?";
    if (!requestParams.modelName.empty()) {
        totalSql += " AND modelName = ?";
    }
    std::string graphStartSql = GetStaticGraphStartSql(requestParams);
    std::string graphEndSql = GetStaticGraphEndSql(requestParams);
    return ExecuteStaticOperatorGraph(requestParams, graphItem, totalSql, graphStartSql, graphEndSql);
}

void JsonMemoryDataBase::SaveOperatorDetail()
{
    if (operatorCache.size() > 0) {
        InsertOperatorDetailList(operatorCache);
        operatorCache.clear();
    }
}

void JsonMemoryDataBase::SaveRecordDetail()
{
    if (recordCache.size() > 0) {
        InsertRecordDetailList(recordCache);
        recordCache.clear();
    }
}

void JsonMemoryDataBase::SaveStaticOpDetail()
{
    if (staticOpCache.size() > 0) {
        InsertStaticOpDetailList(staticOpCache);
        staticOpCache.clear();
    }
}

sqlite3_stmt *JsonMemoryDataBase::GetOperatorStmt(uint64_t paramLen)
{
    sqlite3_stmt *stmt = nullptr;
    if (paramLen == 0) {
        return stmt;
    } else if (paramLen == cacheSize) {
        if (!hasInitStmt) {
            InitStmt();
        }
        stmt = insertOperatorStmt;
        sqlite3_reset(stmt);
    } else {
        std::string sql = "INSERT INTO " + operatorTable + " (name, size, allocation_time, release_time, duration, "
                "active_release_time, active_duration, allocation_allocated, allocation_reserve, allocation_active, "
                "release_allocated, release_reserve, release_active, stream)"
                          " VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?)";
        for (uint64_t i = 0; i < paramLen - 1; ++i) {
            sql.append(",(?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
        }
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            ServerLog::Error("Failed to prepare insertOperator stat. error:", sqlite3_errmsg(db));
            return nullptr;
        }
    }
    return stmt;
}

sqlite3_stmt *JsonMemoryDataBase::GetRecordStmt(uint64_t paramLen)
{
    sqlite3_stmt *stmt = nullptr;
    if (paramLen == 0) {
        return stmt;
    } else if (paramLen == cacheSize) {
        stmt = insertRecordStmt;
        sqlite3_reset(stmt);
    } else {
        std::string sql = "INSERT INTO " + recordTable +
                " (component, total_allocated, total_reserve, total_active, device_type, stream, timestamp)"
                " VALUES (?,?,?,?,?,?,?)";
        for (uint64_t i = 0; i < paramLen - 1; ++i) {
            sql.append(",(?,?,?,?,?,?,?)");
        }
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            ServerLog::Error("Failed to prepare insertOperator stat. error:", sqlite3_errmsg(db));
            return nullptr;
        }
    }
    return stmt;
}

sqlite3_stmt *JsonMemoryDataBase::GetStaticOpStmt(uint64_t paramLen)
{
    sqlite3_stmt *stmt = nullptr;
    if (paramLen == 0) {
        return stmt;
    } else if (paramLen == cacheSize) {
        stmt = insertStaticOpStmt;
        sqlite3_reset(stmt);
    } else {
        std::string sql = "INSERT INTO " + staticOpTable +
                          " (device_id, op_name, model_name, graph_id, node_index_start, node_index_end, size)"
                          " VALUES (?,?,?,?,?,?,?)";
        for (uint64_t i = 0; i < paramLen - 1; ++i) {
            sql.append(",(?,?,?,?,?,?,?)");
        }
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            ServerLog::Error("Failed to prepare insertOperator stat. error:", sqlite3_errmsg(db));
            return nullptr;
        }
    }
    return stmt;
}

bool JsonMemoryDataBase::QueryOperatorsTotalNum(Protocol::MemoryOperatorParams &requestParams, int64_t &totalNum)
{
    std::string sql = "SELECT count(*) as nums FROM " + operatorTable + " WHERE name LIKE ?";

    if (requestParams.type == Protocol::MEMORY_STREAM_GROUP) {
        sql += " AND stream <> ''";
    }
    if (requestParams.startTime != -1) {
        sql += " AND ROUND((allocation_time - ?) / (1000.0 * 1000.0), 2) >= ? ";
    }
    if (requestParams.endTime != -1) {
        sql += " AND ROUND((allocation_time - ?) / (1000.0 * 1000.0), 2) <= ? ";
    }
    if (requestParams.minSize != -1) {
        sql += " AND size >= ? ";
    }
    if (requestParams.maxSize != -1) {
        sql += " AND size <= ? ";
    }
    return ExecuteOperatorsTotalNum(requestParams, totalNum, sql);
}

bool JsonMemoryDataBase::QueryStaticOperatorsTotalNum(Protocol::StaticOperatorListParams &requestParams,
                                                      int64_t &totalNum)
{
    std::string sql = "SELECT count(*) as nums FROM " + staticOpTable + " WHERE op_name LIKE ? AND op_name <> 'TOTAL'";
    if (!requestParams.graphId.empty()) {
        sql += " AND graph_id = ?";
    }
    if (!requestParams.modelName.empty()) {
        sql += " AND model_name = ? ";
    }
    if (requestParams.startNodeIndex >= 0 && requestParams.endNodeIndex >= requestParams.startNodeIndex) {
        sql += " AND (node_index_start BETWEEN " + std::to_string(requestParams.startNodeIndex) +
               " AND " + std::to_string(requestParams.endNodeIndex) +
               " OR node_index_end BETWEEN " + std::to_string(requestParams.startNodeIndex) +
               " AND " + std::to_string(requestParams.endNodeIndex) +")";
    }
    if (requestParams.minSize >= 0) {
        sql += " AND size >= ? ";
    }
    if (requestParams.maxSize >= 0) {
        sql += " AND size <= ? ";
    }
    return ExecuteStaticOperatorListTotalNum(requestParams, totalNum, sql);
}

bool JsonMemoryDataBase::QueryOperatorSize(double &min, double &max, std::string rankId)
{
    std::string sql = "SELECT min(size) as minSize, max(size) as maxSize FROM " + operatorTable;
    return ExecuteOperatorSize(min, max, sql);
}

} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic