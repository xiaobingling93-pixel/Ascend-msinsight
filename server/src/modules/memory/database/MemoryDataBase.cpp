/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "MemoryDataBase.h"
#include <vector>
#include <cmath>
#include "ServerLog.h"
#include "TraceTime.h"

namespace Dic {
namespace Module {
namespace Memory {
using namespace Server;
MemoryDataBase::~MemoryDataBase()
{
    if (hasInitStmt) {
        ReleaseStmt();
    }
    CloseDb();
}

bool MemoryDataBase::SetConfig()
{
    if (!isOpen) {
        ServerLog::Error("Failed to set config. Database is not open.");
        return false;
    }
    return ExecSql("PRAGMA synchronous = OFF; PRAGMA journal_mode = MEMORY;");
}

bool MemoryDataBase::CreateTable()
{
    if (!isOpen) {
        ServerLog::Error("Failed to set config. Database is not open.");
        return false;
    }
    std::string sql =
            "CREATE TABLE " + operatorTable + " (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, " +
            "allocation_time INTEGER, release_time INTEGER, size INTEGER, duration INTEGER);" +
            "CREATE TABLE " + recordTable + " (id INTEGER PRIMARY KEY AUTOINCREMENT, component TEXT, " +
            "total_allocated INTEGER, total_reserve INTEGER, device_type TEXT, timestamp INTEGER);";
    return ExecSql(sql);
}

bool MemoryDataBase::InitStmt()
{
    if (hasInitStmt) {
        return true;
    }
    std::string sql = "INSERT INTO " + operatorTable + " (name, allocation_time, release_time, size, duration)" +
          " VALUES (?,?,?,?,?)";
    for (int i = 0; i < cacheSize - 1; ++i) {
        sql.append(",(?,?,?,?,?)");
    }
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &insertOperatorStmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare insert Operator statement. error:", sqlite3_errmsg(db));
        return false;
    }

    sql = "INSERT INTO " + recordTable + " (component, total_allocated, total_reserve, device_type, timestamp)" +
          " VALUES (?,?,?,?,?)";
    for (int i = 0; i < cacheSize - 1; ++i) {
        sql.append(",(?,?,?,?,?)");
    }
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &insertRecordStmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare insert Record statement. error:", sqlite3_errmsg(db));
        return false;
    }

    hasInitStmt = true;
    return true;
}

void MemoryDataBase::ReleaseStmt()
{
    if (insertOperatorStmt != nullptr) {
        sqlite3_finalize(insertOperatorStmt);
    }
    if (insertRecordStmt != nullptr) {
        sqlite3_finalize(insertRecordStmt);
    }
}

bool MemoryDataBase::InsertOperatorDetailList(const std::vector<Operator> &eventList)
{
    sqlite3_stmt *stmt = GetOperatorStmt(eventList.size());
    if (stmt == nullptr) {
        ServerLog::Error("Failed to get operator stmt.");
        return false;
    }
    int idx = bindStartIndex;
    for (const auto &event : eventList) {
        sqlite3_bind_text(stmt, idx++, event.name.c_str(), event.name.length(), SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, idx++, event.allocationTime);
        sqlite3_bind_double(stmt, idx++, event.releaseTime);
        sqlite3_bind_double(stmt, idx++, event.size);
        sqlite3_bind_double(stmt, idx++, event.duration);
    }
    auto result = sqlite3_step(stmt);
    if (eventList.size() != cacheSize) {
        sqlite3_finalize(stmt);
    }
    if (result != SQLITE_DONE) {
        ServerLog::Error("Insert operator fail. ", sqlite3_errmsg(db));
        return false;
    }
    return true;
}

bool MemoryDataBase::insertOperatorDetail(const Operator &event)
{
    operatorCache.emplace_back(event);
    if (operatorCache.size() == cacheSize) {
        InsertOperatorDetailList(operatorCache);
        operatorCache.clear();
    }
    return true;
}

bool MemoryDataBase::InsertRecordDetailList(const std::vector<Record> &eventList)
{
    sqlite3_stmt *stmt = GetRecordStmt(eventList.size());
    if (stmt == nullptr) {
        ServerLog::Error("Failed to get Record stmt.");
        return false;
    }
    int idx = bindStartIndex;
    for (const auto &event : eventList) {
        sqlite3_bind_text(stmt, idx++, event.component.c_str(), event.component.length(), SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, idx++, event.totalAllocated);
        sqlite3_bind_double(stmt, idx++, event.totalReserved);
        sqlite3_bind_text(stmt, idx++, event.deviceType.c_str(), event.deviceType.length(), SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, idx++, event.timesTamp);
    }
    auto result = sqlite3_step(stmt);
    if (eventList.size() != cacheSize) {
        sqlite3_finalize(stmt);
    }
    if (result != SQLITE_DONE) {
        ServerLog::Error("Insert operator fail. ", sqlite3_errmsg(db));
        return false;
    }
    return true;
}

bool MemoryDataBase::insertRecordDetail(const Record &event)
{
    recordCache.emplace_back(event);
    if (recordCache.size() == cacheSize) {
        InsertRecordDetailList(recordCache);
        recordCache.clear();
    }
    return true;
}

std::string  MemoryDataBase::GetOperatorSql(Protocol::MemoryOperatorParams &requestParams)
{
    std::string ascend;
    if (requestParams.order == "ascend") {
        ascend = "ASC";
    } else {
        ascend = "DESC";
    }
    std::string sql = "SELECT name, ROUND(allocation_time / 1000, 2) as allocation_time, "
                      "ROUND(release_time / 1000, 2) as release_time, size, "
                      "ROUND(duration / 1000, 2) as duration FROM " + operatorTable +
                      " WHERE name LIKE ?";

    if (requestParams.startTime != -1) {
        sql += " AND allocation_time >= " + std::to_string(requestParams.startTime);
    }
    if (requestParams.endTime != -1) {
        sql += " AND allocation_time <= " + std::to_string(requestParams.endTime);
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

bool MemoryDataBase::QueryOperatorDetail(Protocol::MemoryOperatorParams &requestParams,
                                         std::vector<Protocol::MemoryOperator> &responseBody)
{
    std::string sql = GetOperatorSql(requestParams);
    int64_t offset = (requestParams.currentPage - 1) * requestParams.pageSize;
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("QueryOperatorDetail. Failed to prepare sql.", sqlite3_errmsg(db));
        return false;
    }
    int index = bindStartIndex;
    std::string orderName = requestParams.orderName + "%";
    sqlite3_bind_text(stmt, index++, orderName.c_str(), orderName.length(), nullptr);
    sqlite3_bind_int64(stmt, index++, requestParams.pageSize);
    sqlite3_bind_int64(stmt, index++, offset);
    std::vector<Protocol::MemoryOperator> operatorDtoVec;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        Protocol::MemoryOperator operatorDto{};
        operatorDto.name = sqlite3_column_string(stmt, col++);
        // 1ms = 1000us
        operatorDto.allocationTime = static_cast<double>(sqlite3_column_double(stmt, col++));
        operatorDto.releaseTime = static_cast<double>(sqlite3_column_double(stmt, col++));
        operatorDto.size = static_cast<double>(sqlite3_column_double(stmt, col++));
        operatorDto.duration = static_cast<double>(sqlite3_column_double(stmt, col++));
        operatorDtoVec.emplace_back(operatorDto);
    }
    responseBody = operatorDtoVec;

    sqlite3_finalize(stmt);
    return true;
}

bool MemoryDataBase::QueryMemoryView(Protocol::MemoryComponentParams &requestParams,
                                     Protocol::OperatorMemory &operatorBody)
{
    std::string sql = "SELECT component, ROUND(timestamp, 2) as timestamp, "
                      "ROUND(total_allocated, 2) as total_allocated, "
                      "ROUND(total_reserve, 2) as total_reserve FROM " + recordTable;
    // 1ms = 1000 * 1000 ns
    double startTime = Timeline::TraceTime::Instance().GetStartTime() / (1000 * 1000);
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("QueryMemoryView. Failed to prepare sql.", sqlite3_errmsg(db));
        return false;
    }
    int index = bindStartIndex;
    std::string peakMemory;
    std::vector<Protocol::ComponentDto> componentDtoVec;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        Protocol::ComponentDto componentDto{};
        componentDto.component = sqlite3_column_string(stmt, col++);
        // 减去timeline开始的时间作为时间戳
        // 1ms = 1000us
        componentDto.timesTamp = static_cast<double>(sqlite3_column_double(stmt, col++)) / 1000 - startTime;
        componentDto.totalAllocated = static_cast<double>(sqlite3_column_double(stmt, col++));
        componentDto.totalReserved = static_cast<double>(sqlite3_column_double(stmt, col++));
        componentDtoVec.emplace_back(componentDto);
    }
    Protocol::ComponentMemory componentMap;
    Protocol::OperatorMemory operatorMap;
    Protocol::MemoryPeak peak;
    for (auto &item: componentDtoVec) {
        if (item.component == "PTA+GE") {
            peak.ptaGeAllocated = std::max(peak.ptaGeAllocated, item.totalAllocated);
            peak.ptaGeReserved = std::max(peak.ptaGeReserved, item.totalReserved);
            GetOperatorLine(item, operatorMap);
            peak.hasPtaGe = true;
        } else if (item.component == "APP") {
            peak.appAllocated = std::max(peak.appAllocated, item.totalAllocated);
            GetAppLine(item, operatorMap);
            peak.hasApp = true;
        }
    }
    operatorBody = operatorMap;
    operatorBody.peakMemoryUsage = GetPeakMemory(peak);

    sqlite3_finalize(stmt);
    return true;
}

std::string MemoryDataBase::GetPeakMemory(const Protocol::MemoryPeak& peak)
{
    std::string peakMemory = "Peak Memory Usage: ";
    if (peak.hasPtaGe) {
        std::string ptaGeAllo = std::to_string(peak.ptaGeAllocated);
        // double转换成string默认生成六位小数，删除后4位小数
        ptaGeAllo = ptaGeAllo.substr(0, ptaGeAllo.length() - 4);
        peakMemory.append("Operator Allocated： ").append(ptaGeAllo).append("MB");
        std::string ptaGeRe = std::to_string(peak.ptaGeReserved);
        // double转换成string默认生成六位小数，删除后4位小数
        ptaGeRe = ptaGeRe.substr(0, ptaGeRe.length() - 4);
        peakMemory.append(" | Operator Allocated： ").append(ptaGeRe).append("MB");
    }
    if (peak.hasApp) {
        std::string appAllo = std::to_string(peak.appAllocated);
        // double转换成string默认生成六位小数，删除后4位小数
        appAllo = appAllo.substr(0, appAllo.length() - 4);
        peakMemory.append(" | APP Reserved： ").append(appAllo).append("MB");
    }
    return peakMemory;
}

void MemoryDataBase::GetOperatorLine(Protocol::ComponentDto item, Protocol::OperatorMemory &operatorMap)
{
    std::vector<double> AllocatesLine;
    AllocatesLine.emplace_back(item.timesTamp);
    AllocatesLine.emplace_back(item.totalAllocated);
    std::vector<double> ReservedLine;
    ReservedLine.emplace_back(item.timesTamp);
    ReservedLine.emplace_back(item.totalReserved);

    operatorMap.reservedLine.emplace_back(ReservedLine);
    operatorMap.allocatesLine.emplace_back(AllocatesLine);
}

void MemoryDataBase::GetComponentMap(Protocol::ComponentDto item, Protocol::ComponentMemory &componentMap)
{
    std::vector<double> AllocatesLine;
    AllocatesLine[1] = item.totalAllocated;
    AllocatesLine[0] = item.timesTamp;
    std::vector<double> ReservedLine;
    ReservedLine[1] = item.totalReserved;
    ReservedLine[0] = item.timesTamp;
    Protocol::ComponentMemory operatorVec;
    if (item.component == "PTA") {
        componentMap.ptaReservedLine.emplace_back(ReservedLine);
        componentMap.ptaAllocatesLine.emplace_back(AllocatesLine);
    } else if (item.component == "GE") {
        componentMap.geReservedLine.emplace_back(ReservedLine);
        componentMap.geAllocatesLine.emplace_back(AllocatesLine);
    } else if (item.component == "APP") {
        componentMap.appLine.emplace_back(ReservedLine);
    }
}

bool MemoryDataBase::SaveOperatorDetail()
{
    if (operatorCache.size() > 0) {
        InsertOperatorDetailList(operatorCache);
        operatorCache.clear();
    }
    return true;
}

bool MemoryDataBase::SaveRecordDetail()
{
    if (recordCache.size() > 0) {
        InsertRecordDetailList(recordCache);
        recordCache.clear();
    }
    return true;
}

void MemoryDataBase::GetAppLine(Protocol::ComponentDto item, Protocol::OperatorMemory &operatorMap)
{
    std::vector<double> AllocatesLine;
    AllocatesLine.emplace_back(item.totalAllocated);
    AllocatesLine.emplace_back(item.timesTamp);
    std::vector<double> ReservedLine;
    ReservedLine.emplace_back(item.timesTamp);
    ReservedLine.emplace_back(item.totalReserved);

    operatorMap.appLine.emplace_back(ReservedLine);
}

sqlite3_stmt *MemoryDataBase::GetOperatorStmt(uint64_t paramLen)
{
    sqlite3_stmt *stmt = nullptr;
    if (paramLen == cacheSize) {
        stmt = insertOperatorStmt;
        sqlite3_reset(stmt);
    } else {
        std::string sql = "INSERT INTO " + operatorTable + " (name, allocation_time, release_time, size, duration)" +
                          " VALUES (?,?,?,?,?)";
        for (int i = 0; i < paramLen - 1; ++i) {
            sql.append(",(?,?,?,?,?)");
        }
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            ServerLog::Error("Failed to prepare insertOperator stat. error:", sqlite3_errmsg(db));
            return nullptr;
        }
    }
    return stmt;
}

sqlite3_stmt *MemoryDataBase::GetRecordStmt(uint64_t paramLen)
{
    sqlite3_stmt *stmt = nullptr;
    if (paramLen == cacheSize) {
        stmt = insertRecordStmt;
        sqlite3_reset(stmt);
    } else {
        std::string sql = "INSERT INTO " + recordTable +
                " (component, total_allocated, total_reserve, device_type, timestamp) VALUES (?,?,?,?,?)";
        for (int i = 0; i < paramLen - 1; ++i) {
            sql.append(",(?,?,?,?,?)");
        }
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            ServerLog::Error("Failed to prepare insertOperator stat. error:", sqlite3_errmsg(db));
            return nullptr;
        }
    }
    return stmt;
}

bool MemoryDataBase::QueryOperatorsTotalNum(int64_t &totalNum)
{
        sqlite3_stmt *stmt = nullptr;
        std::string sql = "SELECT count(*) as nums FROM " + operatorTable;
        int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        if (result != SQLITE_OK) {
            ServerLog::Error("Failed to prepare sql.", sqlite3_errmsg(db));
            return false;
        }
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            totalNum = sqlite3_column_int(stmt, resultStartIndex);
        }
        sqlite3_finalize(stmt);
        return true;
}

bool MemoryDataBase::QueryOperatorSize(double &min, double &max)
{
    std::string sql = "SELECT min(size) as minSize, max(size) as maxSize FROM " + operatorTable;
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("QueryOperatorSize failed!. ", sqlite3_errmsg(db));
        return false;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        min = sqlite3_column_double(stmt, col++);
        max = sqlite3_column_double(stmt, col++);
    }
    sqlite3_finalize(stmt);
    return true;
}

} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic