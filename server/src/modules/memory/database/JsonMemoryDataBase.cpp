/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "JsonMemoryDataBase.h"
#include <vector>
#include <cmath>
#include "ServerLog.h"
#include "TraceTime.h"
#include "VirtualMemoryDataBase.h"


namespace Dic {
namespace Module {
namespace Memory {
using namespace Server;
JsonMemoryDataBase::JsonMemoryDataBase(std::mutex &sqlMutex) : VirtualMemoryDataBase(sqlMutex) {}

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
    std::unique_lock<std::mutex> lock(mutex);
    return ExecSql("PRAGMA synchronous = OFF; PRAGMA journal_mode = MEMORY;");
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
        "device_type TEXT, stream TEXT, timestamp INTEGER);";
    std::unique_lock<std::mutex> lock(mutex);
    return ExecSql(sql);
}

bool JsonMemoryDataBase::DropTable()
{
    std::vector<std::string> tables = {operatorTable, recordTable};
    std::unique_lock<std::mutex> lock(mutex);
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
    std::unique_lock<std::mutex> lock(mutex);
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
    std::unique_lock<std::mutex> lock(mutex);
    auto result = sqlite3_step(stmt);
    if (eventList.size() != cacheSize) {
        sqlite3_finalize(stmt);
    }
    if (result != SQLITE_DONE) {
        ServerLog::Error("Insert operator fail. ", sqlite3_errmsg(db));
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

uint64_t JsonMemoryDataBase::QueryMinOperatorAllocationTime()
{
    std::string sql = "Select MIN(allocation_time) FROM " + operatorTable + " WHERE allocation_time != 0";
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Failed to prepare sql for QueryMinOperatorAllocationTime.", sqlite3_errmsg(db));
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
        ServerLog::Error("Failed to prepare sql for QueryMinRecordTimestamp.", sqlite3_errmsg(db));
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
    std::string ascend;
    if (requestParams.order == "ascend") {
        ascend = "ASC";
    } else {
        ascend = "DESC";
    }
    std::string sql =
        "SELECT name, size, CASE WHEN allocation_time == 0 THEN 'NA' ELSE "
        "ROUND((allocation_time- ?) / (1000.0 * 1000.0), 2) END AS allocationTime, "
        "CASE WHEN release_time == 0 THEN 'NA' ELSE ROUND((release_time - ?) / (1000.0 * 1000.0), 2) "
        "END AS releaseTime, ROUND(duration / 1000.0, 2) as duration, "
        "CASE WHEN active_release_time == 0 THEN 'NA' ELSE ROUND((active_release_time - ?) / (1000.0 * 1000.0), 2) "
        "END AS activeReleaseTime, ROUND(active_duration / 1000.0, 2) as active_duration, "
        "allocation_allocated, allocation_reserve, allocation_active, release_allocated, release_reserve, "
        "release_active, stream FROM " + operatorTable +
        " WHERE name LIKE ?";

    if (requestParams.type == Protocol::MEMORY_STREAM_GROUP) {
        sql += " AND stream <> ''";
    }
    if (requestParams.startTime != -1) {
        sql += " AND allocationTime >= " + std::to_string(requestParams.startTime);
    }
    if (requestParams.endTime != -1) {
        sql += " AND allocationTime <= " + std::to_string(requestParams.endTime);
    }

    if (requestParams.minSize != -1) {
        sql += " AND size >= " + std::to_string(requestParams.minSize);
    }
    if (requestParams.maxSize != -1) {
        sql += " AND size <= " + std::to_string(requestParams.maxSize);
    }
    if (!requestParams.orderBy.empty()) {
        sql += " ORDER BY " + requestParams.orderBy + " " + ascend;
    }
    sql += " LIMIT ? offset ?";
    return sql;
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
    std::string sql = "SELECT component, ROUND((timestamp - ?) / (1000.0 * 1000.0), 2) as timestamp, "
                      "ROUND(total_allocated, 2) as total_allocated, ROUND(total_reserve, 2) as total_reserve, "
                      "ROUND(total_active, 2) as total_active, stream FROM " + recordTable + " WHERE 1==1";
    return ExecuteQueryMemoryView(requestParams, operatorBody, sql);
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

sqlite3_stmt *JsonMemoryDataBase::GetOperatorStmt(uint64_t paramLen)
{
    sqlite3_stmt *stmt = nullptr;
    if (paramLen == cacheSize) {
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
        for (int i = 0; i < paramLen - 1; ++i) {
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
    if (paramLen == cacheSize) {
        stmt = insertRecordStmt;
        sqlite3_reset(stmt);
    } else {
        std::string sql = "INSERT INTO " + recordTable +
                " (component, total_allocated, total_reserve, total_active, device_type, stream, timestamp)"
                " VALUES (?,?,?,?,?,?,?)";
        for (int i = 0; i < paramLen - 1; ++i) {
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

bool JsonMemoryDataBase::QueryOperatorSize(double &min, double &max, std::string rankId)
{
    std::string sql = "SELECT min(size) as minSize, max(size) as maxSize FROM " + operatorTable;
    return ExecuteOperatorSize(min, max, sql);
}

void JsonMemoryDataBase::SetInferenceType(bool inference)
{
    isInference = inference;
}

bool JsonMemoryDataBase::IsInferenceType() const
{
    return isInference;
}

} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic