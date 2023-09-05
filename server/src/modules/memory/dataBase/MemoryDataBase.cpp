/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "MemoryDataBase.h"
#include <vector>
#include "ServerLog.h"
#include "TraceTime.h"

namespace Dic {
namespace Module {
namespace Memory {
using namespace Server;
MemoryDataBase::~MemoryDataBase()
{
    if (initStmt) {
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
            "allocationTime INTEGER, releaseTime INTEGER, size INTEGER, duration INTEGER);" +
            "CREATE TABLE " + recordTable + " (id INTEGER PRIMARY KEY AUTOINCREMENT, component TEXT, " +
            "totalAllocated INTEGER, totalReserve INTEGER, deviceType TEXT, timestamp INTEGER);";
    return ExecSql(sql);
}

bool MemoryDataBase::InitStmt()
{
    if (initStmt) {
        return true;
    }
    std::string sql = "INSERT INTO " + operatorTable + " (name, allocationTime, releaseTime, size, duration)" +
          " VALUES (?,?,?,?,?)";
    for (int i = 0; i < cacheSize - 1; ++i) {
        sql.append(",(?,?,?,?,?)");
    }
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &insertOperatorStmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare insertOperator statement. error:", sqlite3_errmsg(db));
        return false;
    }

    sql = "INSERT INTO " + recordTable + " (component, totalAllocated, totalReserve, deviceType, timestamp)" +
          " VALUES (?,?,?,?,?)";
    for (int i = 0; i < cacheSize - 1; ++i) {
        sql.append(",(?,?,?,?,?)");
    }
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &insertRecordStmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare insertRecord statement. error:", sqlite3_errmsg(db));
        return false;
    }

    initStmt = true;
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
        sqlite3_bind_double(stmt, idx++, event.timesTamp);
        sqlite3_bind_text(stmt, idx++, event.deviceType.c_str(), event.deviceType.length(), SQLITE_TRANSIENT);
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

bool MemoryDataBase::QueryOperatorDetail(Protocol::MemoryOperatorParams &requestParams,
                                         std::vector<Protocol::MemoryOperator> &responseBody)
{
    std::string sql =
            "SELECT id, allocationTime, releaseTime, size, duration FROM operator "
            "WHERE allocationTime <= ? AND allocationTime >= ?";
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result == SQLITE_OK) {
        int index = bindStartIndex;
        sqlite3_bind_double(stmt, index++, requestParams.endTime);
        sqlite3_bind_double(stmt, index++, requestParams.startTime);
        std::vector<Protocol::MemoryOperator> operatorDtoVec;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int col = resultStartIndex;
            Protocol::MemoryOperator operatorDto{};
            operatorDto.name = sqlite3_column_string(stmt, col++);
            operatorDto.size = static_cast<double>(sqlite3_column_double(stmt, col++));
            operatorDto.allocationTime = static_cast<double>(sqlite3_column_double(stmt, col++));
            operatorDto.releaseTime = static_cast<double>(sqlite3_column_double(stmt, col++));
            operatorDto.duration = static_cast<double>(sqlite3_column_double(stmt, col++));
            operatorDtoVec.emplace_back(operatorDto);
        }
        responseBody = operatorDtoVec;
    } else {
        ServerLog::Error("QueryOperatorDetail failed!");
        return false;
    }
    sqlite3_finalize(stmt);
    return true;
}

bool MemoryDataBase::QueryMemoryView(Protocol::MemoryComponentParams &requestParams,
                                     Protocol::OperatorMemory &operatorBody)
{
    std::string sql = "SELECT component, timesTamp, totalAllocated, totalReserved FROM record";
    double startTime = Timeline::TraceTime::Instance().GetStartTime();
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result == SQLITE_OK) {
        int index = bindStartIndex;
        std::vector<Protocol::ComponentDto> componentDtoVec;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int col = resultStartIndex;
            Protocol::ComponentDto componentDto{};
            componentDto.component = sqlite3_column_string(stmt, col++);
            // 减去timeline开始的时间作为时间戳
            componentDto.timesTamp = static_cast<double>(sqlite3_column_double(stmt, col++)) - startTime;
            componentDto.totalAllocated = static_cast<double>(sqlite3_column_double(stmt, col++));
            componentDto.totalReserved = static_cast<double>(sqlite3_column_double(stmt, col++));
            componentDtoVec.emplace_back(componentDto);
        }
        Protocol::ComponentMemory componentMap;
        Protocol::OperatorMemory operatorMap;
        for (auto &item: componentDtoVec) {
            std::vector<double> recordLine;
            recordLine[0] = item.timesTamp;
            if (item.component == "PTA+GE") {
                GetOperatorLine(item, operatorMap);
            } else if (item.component == "APP") {
                GetAppLine(item, operatorMap);
            }
        }
        operatorBody = operatorMap;
    } else {
        ServerLog::Error("QueryOperatorDetail failed!");
        return false;
    }
    sqlite3_finalize(stmt);
    return true;
}

void MemoryDataBase::GetOperatorLine(Protocol::ComponentDto item, Protocol::OperatorMemory &operatorMap)
{
    std::vector<double> AllocatesLine;
    AllocatesLine[1] = item.totalAllocated;
    AllocatesLine[0] = item.timesTamp;
    std::vector<double> ReservedLine;
    ReservedLine[1] = item.totalReserved;
    ReservedLine[0] = item.timesTamp;

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
    AllocatesLine[1] = item.totalAllocated;
    AllocatesLine[0] = item.timesTamp;
    std::vector<double> ReservedLine;
    ReservedLine[1] = item.totalReserved;
    ReservedLine[0] = item.timesTamp;

    operatorMap.reservedLine.emplace_back(ReservedLine);
    operatorMap.allocatesLine.emplace_back(AllocatesLine);
}

sqlite3_stmt *MemoryDataBase::GetOperatorStmt(uint64_t paramLen)
{
    sqlite3_stmt *stmt = nullptr;
    if (paramLen == cacheSize) {
        stmt = insertOperatorStmt;
        sqlite3_reset(stmt);
    } else {
        std::string sql = "INSERT INTO " + operatorTable + " (name, allocationTime, releaseTime, size, duration)" +
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
                " (component, totalAllocated, totalReserve, deviceType, timestamp) VALUES (?,?,?,?,?)";
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
} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic