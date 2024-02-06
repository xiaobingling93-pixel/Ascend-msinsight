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
MemoryDataBase::MemoryDataBase(std::mutex &sqlMutex) : mutex(sqlMutex) {}

MemoryDataBase::~MemoryDataBase()
{
    if (hasInitStmt) {
        ReleaseStmt();
        hasInitStmt = false;
    }
    CloseDb();
}

bool MemoryDataBase::SetConfig()
{
    if (!isOpen) {
        ServerLog::Error("Failed to set config. Database is not open.");
        return false;
    }
    std::unique_lock<std::mutex> lock(mutex);
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

bool MemoryDataBase::DropTable()
{
    std::vector<std::string> tables = {operatorTable, recordTable};
    std::unique_lock<std::mutex> lock(mutex);
    return DropSomeTables(tables);
}

bool MemoryDataBase::InitStmt()
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

void MemoryDataBase::ReleaseStmt()
{
    if (insertOperatorStmt != nullptr) {
        insertOperatorStmt = nullptr;
    }
    if (insertRecordStmt != nullptr) {
        insertRecordStmt = nullptr;
    }
}

void MemoryDataBase::InsertOperatorDetailList(const std::vector<Operator> &eventList)
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

void MemoryDataBase::insertOperatorDetail(const Operator &event)
{
    operatorCache.emplace_back(event);
    if (operatorCache.size() == cacheSize) {
        InsertOperatorDetailList(operatorCache);
        operatorCache.clear();
    }
}

void MemoryDataBase::InsertRecordDetailList(const std::vector<Record> &eventList)
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

void MemoryDataBase::insertRecordDetail(const Record &event)
{
    recordCache.emplace_back(event);
    if (recordCache.size() == cacheSize) {
        InsertRecordDetailList(recordCache);
        recordCache.clear();
    }
}

std::string  MemoryDataBase::GetOperatorSql(Protocol::MemoryOperatorParams &requestParams)
{
    std::string ascend;
    if (requestParams.order == "ascend") {
        ascend = "ASC";
    } else {
        ascend = "DESC";
    }
    std::string sql = "SELECT name, size, CASE WHEN allocation_time == 0 THEN 'NA' ELSE "
                      "ROUND((allocation_time- ?) / (1000.0 * 1000.0), 2) END AS allocationTime, "
                      "CASE WHEN release_time == 0 THEN 'NA' ELSE ROUND((release_time - ?) / (1000.0 * 1000.0), 2) "
                      "END AS releaseTime, "
                      "ROUND(duration / 1000.0, 2) as duration, stream FROM " + operatorTable +
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

bool MemoryDataBase::QueryOperatorDetail(Protocol::MemoryOperatorParams &requestParams,
    std::vector<Protocol::MemoryTableColumnAttr> &columnAttr, std::vector<Protocol::MemoryOperator> &opDetails)
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
    std::string orderName = "%" + requestParams.searchName + "%";
    uint64_t startTime = Timeline::TraceTime::Instance().GetStartTime();
    sqlite3_bind_int64(stmt, index++, startTime);
    sqlite3_bind_int64(stmt, index++, startTime);
    sqlite3_bind_text(stmt, index++, orderName.c_str(), orderName.length(), nullptr);
    sqlite3_bind_int64(stmt, index++, requestParams.pageSize);
    sqlite3_bind_int64(stmt, index++, offset);
    std::vector<Protocol::MemoryOperator> operatorDtoVec;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        Protocol::MemoryOperator operatorDto{};
        operatorDto.name = sqlite3_column_string(stmt, col++);
        operatorDto.size = sqlite3_column_double(stmt, col++);
        operatorDto.allocationTime = sqlite3_column_string(stmt, col++);
        operatorDto.releaseTime = sqlite3_column_string(stmt, col++);
        operatorDto.duration = sqlite3_column_double(stmt, col++);
        operatorDto.streamId = sqlite3_column_string(stmt, col++);
        operatorDtoVec.emplace_back(operatorDto);
    }
    opDetails = operatorDtoVec;
    columnAttr = tableColumnAttr;

    sqlite3_finalize(stmt);
    return true;
}

/*
 * 将多个单条线的数据组装成[x,y,y,y,y]的格式，对于x点上不存在的y补为NULL。
 */
void MemoryDataBase::GetLines(const componentDtoVector componentDtoVec, std::vector<std::vector<std::string>> &lines,
    std::vector<std::string> &legends, Protocol::MemoryPeak &peak, const std::vector<std::string>& streams)
{
    for (const auto& legend : baseLegends) {
        if (streams.empty() && legend == "Operators Activated") { // 实现数据兼容
            continue;
        }
        legends.emplace_back(legend);
    }

    for (auto &item: componentDtoVec) {
        std::vector<std::string> points = {};
        if (item.component == COMPONENT_PTA_AND_GE || (isInference && item.component == COMPONENT_GE)) {
            peak.ptaGeAllocated = std::max(peak.ptaGeAllocated, item.totalAllocated);
            peak.ptaGeReserved = std::max(peak.ptaGeReserved, item.totalReserved);
            peak.ptaGeActivated = std::max(peak.ptaGeActivated, item.totalActivated);
            std::string time = std::to_string(item.timesTamp);
            points.emplace_back(time.substr(0, time.length() - exLength));
            std::string allocated = std::to_string(item.totalAllocated);
            points.emplace_back(allocated.substr(0, allocated.length() - exLength));
            if (!streams.empty()) { // 实现数据兼容
                std::string activated = std::to_string(item.totalActivated);
                points.emplace_back(activated.substr(0, activated.length() - exLength));
            }
            std::string reserved = std::to_string(item.totalReserved);
            points.emplace_back(reserved.substr(0, reserved.length() - exLength));
            points.emplace_back("NULL");
            peak.hasPtaGe = true;
            lines.emplace_back(points);
        } else if (item.component == COMPONENT_APP) {
            peak.appReserved = std::max(peak.appReserved, item.totalReserved);
            std::string time = std::to_string(item.timesTamp);
            points.emplace_back(time.substr(0, time.length() - exLength));
            points.emplace_back("NULL");
            if (!streams.empty()) { // 实现数据兼容
                points.emplace_back("NULL");
            }
            points.emplace_back("NULL");
            std::string reserved = std::to_string(item.totalReserved);
            points.emplace_back(reserved.substr(0, reserved.length() - exLength));
            peak.hasApp = true;
            lines.emplace_back(points);
        }
    }
    if (peak.hasApp) {
        legends.emplace_back(appLegend);
    }
}

std::vector<std::string> MemoryDataBase::GetStreamLists()
{
    std::vector<std::string> streams = {};
    std::string sql =
        "SELECT stream FROM " + recordTable + " WHERE stream <> '' Group BY stream ORDER BY timestamp ASC";
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Failed to prepare sql to get stream list.", sqlite3_errmsg(db));
        return streams;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        streams.emplace_back(sqlite3_column_string(stmt, col++));
    }
    sqlite3_finalize(stmt);
    return streams;
}

void MemoryDataBase::GetStreamLines(const componentDtoVector componentDtoVec,
    std::vector<std::vector<std::string>> &lines, std::vector<std::string> &legends, Protocol::MemoryPeak &peak,
    const std::vector<std::string>& streams)
{
    // 组装图例
    if (componentDtoVec.empty() && streams.empty()) {
        legends.insert(legends.end(), baseLegends.begin(), baseLegends.end());
    } else {
        legends.emplace_back(baseLegends[0]);
    }
    for (const auto& stream : streams) {
        legends.emplace_back("Allocated of " + stream);
        legends.emplace_back("Activated of " + stream);
        legends.emplace_back("Reserved of " + stream);
    }

    // 组装数据点
    for (auto &item: componentDtoVec) {
        std::vector<std::string> points = {};
        if (item.component != COMPONENT_PTA_AND_GE) {
            continue;
        }
        std::string time = std::to_string(item.timesTamp);
        points.emplace_back(time.substr(0, time.length() - exLength));
        std::string streamId = item.streamId;
        for (const auto& stream : streams) {
            if (stream == streamId) {
                std::string allocated = std::to_string(item.totalAllocated);
                points.emplace_back(allocated.substr(0, allocated.length() - exLength));
                std::string activated = std::to_string(item.totalActivated);
                points.emplace_back(activated.substr(0, activated.length() - exLength));
                std::string reserved = std::to_string(item.totalReserved);
                points.emplace_back(reserved.substr(0, reserved.length() - exLength));
            } else {
                points.insert(points.end(), {"NULL", "NULL", "NULL"});
            }
        }
        lines.emplace_back(points);
    }
}

bool MemoryDataBase::QueryMemoryView(Protocol::MemoryComponentParams &requestParams,
                                     Protocol::MemoryViewData &operatorBody)
{
    std::string sql = "SELECT component, ROUND((timestamp - ?) / (1000.0 * 1000.0), 2) as timestamp, "
                      "ROUND(total_allocated, 2) as total_allocated, "
                      "ROUND(total_reserve, 2) as total_reserve, "
                      "ROUND(total_active, 2) as total_active, "
                      "stream FROM " + recordTable;
    if (requestParams.type == Protocol::MEMORY_STREAM_GROUP) {
        sql += " WHERE stream <> ''";
    }
    sql += " ORDER BY timestamp ASC";
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("QueryMemoryView. Failed to prepare sql.", sqlite3_errmsg(db));
        return false;
    }
    int index = bindStartIndex;
    // 减去timeline开始的时间作为时间戳
    uint64_t startTime = Timeline::TraceTime::Instance().GetStartTime();
    sqlite3_bind_int64(stmt, index++, startTime);
    std::string peakMemory;
    std::vector<Protocol::ComponentDto> componentDtoVec;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        Protocol::ComponentDto componentDto{};
        componentDto.component = sqlite3_column_string(stmt, col++);
        componentDto.timesTamp = sqlite3_column_double(stmt, col++);
        componentDto.totalAllocated = sqlite3_column_double(stmt, col++);
        componentDto.totalReserved = sqlite3_column_double(stmt, col++);
        componentDto.totalActivated = sqlite3_column_double(stmt, col++);
        componentDto.streamId = sqlite3_column_string(stmt, col++);
        componentDtoVec.emplace_back(componentDto);
    }
    sqlite3_finalize(stmt);

    // 查询是否包含stream信息，如果不包含则不显示stream相关信息，同时也用来判断是否active相关信息
    std::vector<std::string> streams = GetStreamLists();

    Protocol::MemoryPeak peak;
    if (requestParams.type == Protocol::MEMORY_OVERALL_GROUP) {
        GetLines(componentDtoVec, operatorBody.lines, operatorBody.legends, peak, streams);
        operatorBody.title = GetPeakMemory(peak, streams);
    } else {
        GetStreamLines(componentDtoVec, operatorBody.lines, operatorBody.legends, peak, streams);
    }
    return true;
}

std::string MemoryDataBase::GetPeakMemory(const Protocol::MemoryPeak& peak, const std::vector<std::string>& streams)
{
    std::string peakMemory = "Peak Memory Usage: ";
    const size_t decimalPlacesNum = 4;
    if (peak.hasPtaGe) {
        std::string ptaGeAllo = std::to_string(peak.ptaGeAllocated);
        // double转换成string默认生成六位小数，删除后4位小数
        ptaGeAllo = ptaGeAllo.substr(0, ptaGeAllo.length() - decimalPlacesNum);
        peakMemory.append("Operator Allocated： ").append(ptaGeAllo).append("MB");
        if (!streams.empty()) {
            std::string ptaGeActive = std::to_string(peak.ptaGeActivated);
            ptaGeActive = ptaGeActive.substr(0, ptaGeActive.length() - decimalPlacesNum);
            peakMemory.append(" | Operator Activated： ").append(ptaGeActive).append("MB");
        }
        std::string ptaGeRe = std::to_string(peak.ptaGeReserved);
        ptaGeRe = ptaGeRe.substr(0, ptaGeRe.length() - decimalPlacesNum);
        peakMemory.append(" | Operator Reserved： ").append(ptaGeRe).append("MB");
    }
    if (peak.hasApp) {
        std::string appAllo = std::to_string(peak.appReserved);
        appAllo = appAllo.substr(0, appAllo.length() - decimalPlacesNum);
        peakMemory.append(" | APP Reserved： ").append(appAllo).append("MB");
    }
    return peakMemory;
}

void MemoryDataBase::SaveOperatorDetail()
{
    if (operatorCache.size() > 0) {
        InsertOperatorDetailList(operatorCache);
        operatorCache.clear();
    }
}

void MemoryDataBase::SaveRecordDetail()
{
    if (recordCache.size() > 0) {
        InsertRecordDetailList(recordCache);
        recordCache.clear();
    }
}

sqlite3_stmt *MemoryDataBase::GetOperatorStmt(uint64_t paramLen)
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

sqlite3_stmt *MemoryDataBase::GetRecordStmt(uint64_t paramLen)
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

bool MemoryDataBase::QueryOperatorsTotalNum(Protocol::MemoryOperatorParams &requestParams, int64_t &totalNum)
{
    sqlite3_stmt *stmt = nullptr;
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
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Failed to prepare sql.", sqlite3_errmsg(db));
        return false;
    }
    int index = bindStartIndex;
    std::string orderName = "%" + requestParams.searchName + "%";
    sqlite3_bind_text(stmt, index++, orderName.c_str(), orderName.length(), nullptr);
    uint64_t startTime = Timeline::TraceTime::Instance().GetStartTime();
    if (requestParams.startTime != -1) {
        sqlite3_bind_int64(stmt, index++, startTime);
        sqlite3_bind_double(stmt, index++, requestParams.startTime);
    }
    if (requestParams.endTime != -1) {
        sqlite3_bind_int64(stmt, index++, startTime);
        sqlite3_bind_double(stmt, index++, requestParams.endTime);
    }
    if (requestParams.minSize != -1) {
        sqlite3_bind_double(stmt, index++, requestParams.minSize);
    }
    if (requestParams.maxSize != -1) {
        sqlite3_bind_double(stmt, index++, requestParams.maxSize);
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

void MemoryDataBase::SetInferenceType(bool inference)
{
    isInference = inference;
}

bool MemoryDataBase::IsInferenceType() const
{
    return isInference;
}

} // end of namespace Memory
} // end of namespace Module
} // end of namespace Dic