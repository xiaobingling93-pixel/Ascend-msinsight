/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "pch.h"
#include "TraceTime.h"
#include "DataBaseManager.h"
#include "VirtualMemoryDataBase.h"

namespace Dic {
namespace Module {
namespace Memory {
using namespace Server;
using namespace Dic::Module::Timeline;

std::string VirtualMemoryDataBase::ExecuteQueryDeviceId(std::string &sql)
{
    std::string deviceId;
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        return "";
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        std::string res = sqlite3_column_string(stmt, col++);
        deviceId = res;
    }
    sqlite3_finalize(stmt);
    return deviceId;
}

std::vector<std::string> VirtualMemoryDataBase::GetStreamLists(std::string deviceId, std::string deviceIdColumnName)
{
    std::vector<std::string> streams = {};
    DataType type = DataBaseManager::Instance().GetDataType();
    std::string sql = "";
    if (type == DataType::TEXT) {
        sql += "SELECT stream FROM " + recordTable + " WHERE deviceId = ? AND stream <> '' "
            "Group BY stream ORDER BY timestamp ASC";
    } else if (type == DataType::DB) {
        FileType fileType = DataBaseManager::Instance().GetFileType();
        if (fileType == FileType::PYTORCH) {
            std::string streamPtrColumnName = isLowCamel ? "streamPtr" : "stream_ptr";
            std::string timeColumnName = isLowCamel ? "timestamp" : "time_stamp";
            sql += "SELECT " + streamPtrColumnName + " FROM " + TABLE_MEMORY_RECORD +
                " WHERE " + deviceIdColumnName + " = ? AND " + streamPtrColumnName + " <> ''"
                " Group BY " + streamPtrColumnName + " ORDER BY " + timeColumnName + " ASC";
        } else {
            ServerLog::Error("Memory tab does not support msprof data.");
            return streams;
        }
    }
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Get stream lists. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return streams;
    }
    int index = bindStartIndex;
    sqlite3_bind_text(stmt, index++, deviceId.c_str(), deviceId.length(), nullptr);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        streams.emplace_back(sqlite3_column_string(stmt, col++));
    }
    sqlite3_finalize(stmt);
    return streams;
}

bool VirtualMemoryDataBase::ExecuteMemoryType(std::vector<std::string> &graphId, std::string &type)
{
    if (!Database::CheckTableContainData(TABLE_STATIC_OPERATOR)) {
        type = Module::Memory::MEMORY_TYPE_DYNAMIC;
        return true;
    }
    type = Module::Memory::MEMORY_TYPE_STATIC;
    std::string sql = "SELECT DISTINCT graph_id as graphId FROM " + TABLE_STATIC_OPERATOR;
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query memory type. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return false;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        std::string res = sqlite3_column_string(stmt, col++);
        graphId.emplace_back(res);
    }
    sqlite3_finalize(stmt);
    return true;
}

bool VirtualMemoryDataBase::ExecuteMemoryResourceType(std::string &type, std::string sql)
{
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query memory resource type. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return false;
    }
    int64_t totalNum = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        totalNum = sqlite3_column_int(stmt, resultStartIndex);
    }
    type = (totalNum > 0) ? MEMORY_RESOURCE_TYPE_MIND_SPORE : MEMORY_RESOURCE_TYPE_PYTORCH;
    sqlite3_finalize(stmt);
    return true;
}

bool VirtualMemoryDataBase::ExecuteOperatorSize(Protocol::MemoryOperatorSizeParams &requestParams, double &min,
    double &max, std::string sql)
{
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query operator size. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return false;
    }
    int index = bindStartIndex;
    if (Timeline::DataBaseManager::Instance().GetDataType() == DataType::TEXT) {
        sqlite3_bind_text(stmt, index++, requestParams.deviceId.c_str(), requestParams.deviceId.length(), nullptr);
    } else {
        int deviceId = StringUtil::StringToInt(requestParams.deviceId);
        sqlite3_bind_int64(stmt, index++, deviceId);
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        min = sqlite3_column_double(stmt, col++);
        max = sqlite3_column_double(stmt, col++);
    }
    sqlite3_finalize(stmt);
    return true;
}

bool VirtualMemoryDataBase::ExecuteStaticOperatorSize(Protocol::StaticOperatorSizeParams &requestParams,
                                                      double &min, double &max, const std::string &sql)
{
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query static operator size. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return false;
    }
    int index = bindStartIndex;
    if (!requestParams.graphId.empty()) {
        sqlite3_bind_text(stmt, index++, requestParams.graphId.c_str(), requestParams.graphId.length(), nullptr);
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        min = sqlite3_column_double(stmt, col++);
        max = sqlite3_column_double(stmt, col++);
    }
    sqlite3_finalize(stmt);
    return true;
}

bool VirtualMemoryDataBase::ExecuteOperatorsTotalNum(Protocol::MemoryOperatorParams &requestParams, int64_t &totalNum,
    std::string sql)
{
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query operators total num. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return false;
    }
    int index = bindStartIndex;
    if (Timeline::DataBaseManager::Instance().GetDataType() == DataType::TEXT) {
        sqlite3_bind_text(stmt, index++, requestParams.deviceId.c_str(), requestParams.deviceId.length(), nullptr);
    } else {
        int deviceId = StringUtil::StringToInt(requestParams.deviceId);
        sqlite3_bind_int64(stmt, index++, deviceId);
    }
    std::string orderName = "%" + requestParams.searchName + "%";
    sqlite3_bind_text(stmt, index++, orderName.c_str(), orderName.length(), nullptr);
    uint64_t startTime = Timeline::TraceTime::Instance().GetStartTime();
    uint64_t offsetTime = Timeline::TraceTime::Instance().GetOffsetByFileIdUsingMinTimestamp(requestParams.rankId);
    if (requestParams.startTime != -1 && requestParams.endTime != -1) {
        if (requestParams.isOnlyShowAllocatedOrReleasedWithinInterval) {
            // 只显示在时间区间内分配或释放内存的数据
            sqlite3_bind_int64(stmt, index++, NumberUtil::CeilingClamp(
                startTime + offsetTime, static_cast<uint64_t>(INT64_MAX)));
            sqlite3_bind_double(stmt, index++, requestParams.startTime);
            sqlite3_bind_double(stmt, index++, requestParams.endTime);
            sqlite3_bind_int64(stmt, index++, NumberUtil::CeilingClamp(
                startTime + offsetTime, static_cast<uint64_t>(INT64_MAX)));
            sqlite3_bind_double(stmt, index++, requestParams.startTime);
            sqlite3_bind_double(stmt, index++, requestParams.endTime);
        } else {
            // 显示全部
            sqlite3_bind_int64(stmt, index++, NumberUtil::CeilingClamp(
                startTime + offsetTime, static_cast<uint64_t>(INT64_MAX)));
            sqlite3_bind_double(stmt, index++, requestParams.startTime);
            sqlite3_bind_int64(stmt, index++, NumberUtil::CeilingClamp(
                startTime + offsetTime, static_cast<uint64_t>(INT64_MAX)));
            sqlite3_bind_double(stmt, index++, requestParams.endTime);
        }
    }

    if (requestParams.minSize != std::numeric_limits<int64_t>::min()) {
        sqlite3_bind_int64(stmt, index++, requestParams.minSize);
    }
    if (requestParams.maxSize != std::numeric_limits<int64_t>::max()) {
        sqlite3_bind_int64(stmt, index++, requestParams.maxSize);
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        totalNum = sqlite3_column_int(stmt, resultStartIndex);
    }
    sqlite3_finalize(stmt);
    return true;
}

bool VirtualMemoryDataBase::ExecuteComponentTotalNum(Protocol::MemoryComponentParams &requestParams, int64_t &totalNum,
                                                     std::string &sql)
{
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query components total num. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return false;
    }

    int index = bindStartIndex;
    if (Timeline::DataBaseManager::Instance().GetDataType() == DataType::TEXT) {
        sqlite3_bind_text(stmt, index++, requestParams.deviceId.c_str(), requestParams.deviceId.length(), nullptr);
    } else {
        int deviceId = StringUtil::StringToInt(requestParams.deviceId);
        sqlite3_bind_int64(stmt, index++, deviceId);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        totalNum = sqlite3_column_int(stmt, resultStartIndex);
    }
    sqlite3_finalize(stmt);
    return true;
}

bool VirtualMemoryDataBase::ExecuteStaticOperatorListTotalNum(Protocol::StaticOperatorListParams &requestParams,
                                                              int64_t &totalNum,
                                                              std::string sql)
{
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query static operators total num. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return false;
    }
    int index = bindStartIndex;
    std::string searchName = "%" + requestParams.searchName + "%";
    sqlite3_bind_text(stmt, index++, searchName.c_str(), searchName.length(), nullptr);
    if (!requestParams.graphId.empty()) {
        sqlite3_bind_text(stmt, index++, requestParams.graphId.c_str(), requestParams.graphId.length(), nullptr);
    }
    if (requestParams.minSize != std::numeric_limits<int64_t>::min()) {
        sqlite3_bind_int64(stmt, index++, requestParams.minSize);
    }
    if (requestParams.maxSize != std::numeric_limits<int64_t>::max()) {
        sqlite3_bind_int64(stmt, index++, requestParams.maxSize);
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        totalNum = sqlite3_column_int(stmt, resultStartIndex);
    }
    sqlite3_finalize(stmt);
    return true;
}

bool VirtualMemoryDataBase::ExecuteQueryMemoryViewExecuteSql(Protocol::MemoryViewParams &requestParams,
    std::vector<Protocol::ComponentDto> &componentDtoVec, std::vector<std::string> &streams,
    std::string &sql, std::string deviceIdColumnName)
{
    if (requestParams.type == Protocol::MEMORY_STREAM_GROUP) {
        sql += " AND stream <> ''";
    }
    sql += " ORDER BY timestamp ASC";
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query memory view. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return false;
    }
    int index = bindStartIndex;
    if (Timeline::DataBaseManager::Instance().GetDataType() == DataType::TEXT) {
        sqlite3_bind_text(stmt, index++, requestParams.deviceId.c_str(), requestParams.deviceId.length(), nullptr);
    } else {
        int deviceId = StringUtil::StringToInt(requestParams.deviceId);
        sqlite3_bind_int64(stmt, index++, deviceId);
    }
    std::string peakMemory;
    std::set<std::string> componentSets;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        Protocol::ComponentDto componentDto{};
        componentDto.component = sqlite3_column_string(stmt, col++);
        componentDto.timesTamp = sqlite3_column_double(stmt, col++);
        componentDto.totalAllocated = sqlite3_column_double(stmt, col++);
        componentDto.totalReserved = sqlite3_column_double(stmt, col++);
        componentDto.totalActivated = sqlite3_column_double(stmt, col++);
        componentDto.streamId = sqlite3_column_string(stmt, col++);
        componentSets.emplace(componentDto.component);
        componentDtoVec.emplace_back(componentDto);
    }
    sqlite3_finalize(stmt);

    if (componentSets.size() == 1 && *componentSets.begin() == COMPONENT_GE) {
        isInference = true;
    }

    // 查询是否包含stream信息，如果不包含则不显示stream相关信息，同时也用来判断是否active相关信息
    streams = GetStreamLists(requestParams.deviceId, deviceIdColumnName);
    return true;
}

bool VirtualMemoryDataBase::ExecuteQueryMemoryViewGetGraph(Protocol::MemoryViewParams &requestParams,
                                                           std::vector<Protocol::ComponentDto> &componentDtoVec,
                                                           std::vector<std::string> &streams,
                                                           Protocol::MemoryViewData &operatorBody)
{
    Protocol::MemoryPeak peak;
    if (requestParams.type == Protocol::MEMORY_OVERALL_GROUP) {
        GetOverallLines(componentDtoVec, operatorBody.lines, operatorBody.legends, peak, streams);
        operatorBody.title = GetPeakMemory(peak, streams);
    } else if (requestParams.type == Protocol::MEMORY_STREAM_GROUP) {
        GetStreamLines(componentDtoVec, operatorBody.lines, operatorBody.legends, peak, streams);
    } else {
        GetComponentLines(componentDtoVec, operatorBody.lines, operatorBody.legends, peak, streams);
        operatorBody.title = GetPeakMemory(peak, streams);
    }
    return true;
}

bool VirtualMemoryDataBase::ExecuteOperatorDetail(Protocol::MemoryOperatorParams &requestParams,
                                                  std::vector<Protocol::MemoryTableColumnAttr> &columnAttr,
                                                  std::vector<Protocol::MemoryOperator> &opDetails,
                                                  std::string sql, std::string deviceIdColumnName)
{
    int64_t pageSize = requestParams.pageSize == 0 ? defaultPageSize : requestParams.pageSize;
    int64_t currentPage = requestParams.currentPage - 1;
    currentPage = currentPage < 0 ? 0 : currentPage;
    if (pageSize > maxPageSize || currentPage > maxCurrentPage) {
        ServerLog::Error("Error param: pageSize or currentPage");
        return false;
    }
    int64_t offset = currentPage * pageSize;
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query operator detail. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return false;
    }
    int index = bindStartIndex;
    if (Timeline::DataBaseManager::Instance().GetDataType() == DataType::TEXT) {
        sqlite3_bind_text(stmt, index++, requestParams.deviceId.c_str(), requestParams.deviceId.length(), nullptr);
    } else {
        int deviceId = StringUtil::StringToInt(requestParams.deviceId);
        sqlite3_bind_int64(stmt, index++, deviceId);
    }
    std::string orderName = "%" + requestParams.searchName + "%";
    sqlite3_bind_text(stmt, index++, orderName.c_str(), orderName.length(), nullptr);
    sqlite3_bind_int64(stmt, index++, requestParams.pageSize);
    sqlite3_bind_int64(stmt, index++, offset);
    opDetails = QueryOperatorDetail(stmt);
    std::vector<std::string> streams = GetStreamLists(requestParams.rankId, deviceIdColumnName);
    std::vector<std::string> columns = activeRelatedColumn;
    for (const auto& column : tableColumnAttr) {
        if (streams.empty() && std::find(columns.begin(), columns.end(), column.name) != columns.end()) {
            continue;
        }
        columnAttr.emplace_back(column);
    }
    return true;
}

std::vector<Protocol::MemoryOperator> VirtualMemoryDataBase::QueryOperatorDetail(sqlite3_stmt *stmt)
{
    std::vector<Protocol::MemoryOperator> operatorDtoVec;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        Protocol::MemoryOperator operatorDto{};
        operatorDto.id = std::to_string(sqlite3_column_int64(stmt, col++));
        operatorDto.name = sqlite3_column_string(stmt, col++);
        operatorDto.size = sqlite3_column_double(stmt, col++);
        operatorDto.allocationTime = sqlite3_column_string(stmt, col++);
        operatorDto.releaseTime = sqlite3_column_string(stmt, col++);
        operatorDto.duration = sqlite3_column_double(stmt, col++);
        operatorDto.activeReleaseTime = sqlite3_column_string(stmt, col++);
        operatorDto.activeDuration = sqlite3_column_double(stmt, col++);
        operatorDto.allocationAllocated = sqlite3_column_double(stmt, col++);
        operatorDto.allocationReserved = sqlite3_column_double(stmt, col++);
        operatorDto.allocationActive = sqlite3_column_double(stmt, col++);
        operatorDto.releaseAllocated = sqlite3_column_double(stmt, col++);
        operatorDto.releaseReserved = sqlite3_column_double(stmt, col++);
        operatorDto.releaseActive = sqlite3_column_double(stmt, col++);
        operatorDto.streamId = sqlite3_column_string(stmt, col++);
        operatorDtoVec.emplace_back(operatorDto);
    }
    sqlite3_finalize(stmt);
    return operatorDtoVec;
}

bool VirtualMemoryDataBase::ExecuteQueryEntireOperatorTable(Protocol::MemoryOperatorParams &requestParams,
    std::vector<Protocol::MemoryOperator> &opDetails, const std::string &sql)
{
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query entire operator table. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return false;
    }

    int index = bindStartIndex;
    if (Timeline::DataBaseManager::Instance().GetDataType() == DataType::TEXT) {
        sqlite3_bind_text(stmt, index++, requestParams.deviceId.c_str(), requestParams.deviceId.length(), nullptr);
    } else {
        int deviceId = StringUtil::StringToInt(requestParams.deviceId);
        sqlite3_bind_int64(stmt, index++, deviceId);
    }

    std::vector<Protocol::MemoryOperator> &operatorDtoVec = opDetails;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        Protocol::MemoryOperator operatorDto{};
        operatorDto.name = sqlite3_column_string(stmt, col++);
        operatorDto.size = sqlite3_column_double(stmt, col++);
        operatorDto.allocationTime = sqlite3_column_string(stmt, col++);
        operatorDto.releaseTime = sqlite3_column_string(stmt, col++);
        operatorDto.duration = sqlite3_column_double(stmt, col++);
        operatorDto.activeReleaseTime = sqlite3_column_string(stmt, col++);
        operatorDto.activeDuration = sqlite3_column_double(stmt, col++);
        operatorDto.allocationAllocated = sqlite3_column_double(stmt, col++);
        operatorDto.allocationReserved = sqlite3_column_double(stmt, col++);
        operatorDto.allocationActive = sqlite3_column_double(stmt, col++);
        operatorDto.releaseAllocated = sqlite3_column_double(stmt, col++);
        operatorDto.releaseReserved = sqlite3_column_double(stmt, col++);
        operatorDto.releaseActive = sqlite3_column_double(stmt, col++);
        operatorDto.streamId = sqlite3_column_string(stmt, col++);
        operatorDtoVec.emplace_back(operatorDto);
    }
    sqlite3_finalize(stmt);
    return true;
}

bool VirtualMemoryDataBase::ExecuteComponentDetail(Protocol::MemoryComponentParams &requestParams,
                                                   std::vector<Protocol::MemoryTableColumnAttr> &columnAttr,
                                                   std::vector<Protocol::MemoryComponent> &componentDetails,
                                                   std::string &sql)
{
    int64_t pageSize = requestParams.pageSize == 0 ? defaultPageSize : requestParams.pageSize;
    int64_t currentPage = requestParams.currentPage - 1;
    currentPage = currentPage < 0 ? 0 : currentPage;
    if (pageSize > maxPageSize || currentPage > maxCurrentPage) {
        ServerLog::Error("Error param: pageSize or currentPage");
        return false;
    }
    int64_t offset = currentPage * pageSize;
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query component detail. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return false;
    }
    int index = bindStartIndex;
    if (Timeline::DataBaseManager::Instance().GetDataType() == DataType::TEXT) {
        sqlite3_bind_text(stmt, index++, requestParams.deviceId.c_str(), requestParams.deviceId.length(), nullptr);
    } else {
        sqlite3_bind_int64(stmt, index++, StringUtil::StringToInt(requestParams.deviceId));
    }
    sqlite3_bind_int64(stmt, index++, requestParams.pageSize);
    sqlite3_bind_int64(stmt, index++, offset);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        Protocol::MemoryComponent componentElement;
        componentElement.component = sqlite3_column_string(stmt, col++);
        componentElement.totalReserved = sqlite3_column_double(stmt, col++);
        componentElement.timestamp = sqlite3_column_string(stmt, col++);
        componentDetails.emplace_back(componentElement);
    }
    sqlite3_finalize(stmt);
    for (const auto &column : componentTableColumnAttr) {
        columnAttr.emplace_back(column);
    }
    return true;
}

bool VirtualMemoryDataBase::ExecuteQueryEntireComponentTable(Protocol::MemoryComponentParams &requestParams,
    std::vector<Protocol::MemoryComponent> &componentDetails, std::string &sql)
{
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query entire component table. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return false;
    }

    int index = bindStartIndex;
    if (Timeline::DataBaseManager::Instance().GetDataType() == DataType::TEXT) {
        sqlite3_bind_text(stmt, index++, requestParams.deviceId.c_str(), requestParams.deviceId.length(), nullptr);
    } else {
        int deviceId = StringUtil::StringToInt(requestParams.deviceId);
        sqlite3_bind_int64(stmt, index++, deviceId);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        Protocol::MemoryComponent componentElement;
        componentElement.component = sqlite3_column_string(stmt, col++);
        componentElement.totalReserved = sqlite3_column_double(stmt, col++);
        componentElement.timestamp = sqlite3_column_string(stmt, col++);
        componentDetails.emplace_back(componentElement);
    }
    sqlite3_finalize(stmt);
    return true;
}

bool VirtualMemoryDataBase::ExecuteStaticGraphTotalSize(Protocol::StaticOperatorGraphParams &requestParams,
                                                        const std::string& totalSql, double &totalSize)
{
    sqlite3_stmt *totalStmt = nullptr;
    int totalResult = sqlite3_prepare_v2(db, totalSql.c_str(), -1, &totalStmt, nullptr);
    if (totalResult != SQLITE_OK) {
        ServerLog::Error("Query static graph total size. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return false;
    }
    int index = bindStartIndex;
    sqlite3_bind_text(totalStmt, index++, requestParams.graphId.c_str(), requestParams.graphId.length(), nullptr);
    if (!requestParams.modelName.empty()) {
        std::string modelName = "%" + requestParams.modelName + "%";
        sqlite3_bind_text(totalStmt, index++, modelName.c_str(), modelName.length(), nullptr);
    }
    if (sqlite3_step(totalStmt) == SQLITE_ROW) {
        totalSize = sqlite3_column_double(totalStmt, resultStartIndex);
    }
    sqlite3_finalize(totalStmt);
    return true;
}

bool VirtualMemoryDataBase::ExecuteStaticGraphStartIndex(Protocol::StaticOperatorGraphParams &requestParams,
                                                         const std::string& graphStartSql,
                                                         std::map<int64_t, double> &graphSizeMap, int64_t &maxIndex)
{
    sqlite3_stmt *startStmt = nullptr;
    int graphStartResult = sqlite3_prepare_v2(db, graphStartSql.c_str(), -1, &startStmt, nullptr);
    if (graphStartResult != SQLITE_OK) {
        ServerLog::Error("Query static graph start index. Failed to prepare sql. Error:", sqlite3_errmsg(db));
        return false;
    }
    int index = bindStartIndex;
    sqlite3_bind_text(startStmt, index++, requestParams.graphId.c_str(), requestParams.graphId.length(), nullptr);
    if (!requestParams.modelName.empty()) {
        std::string modelName = "%" + requestParams.modelName + "%";
        sqlite3_bind_text(startStmt, index, modelName.c_str(), modelName.length(), nullptr);
    }
    while (sqlite3_step(startStmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        int64_t nodeIndex = sqlite3_column_int64(startStmt, col++);
        double size = sqlite3_column_double(startStmt, col++);
        if (graphSizeMap.find(nodeIndex) != graphSizeMap.end()) {
            graphSizeMap[nodeIndex] += size;
        } else {
            graphSizeMap.insert({nodeIndex, size});
        }
        if (nodeIndex >= maxUnsignedInt) {
            maxIndex = maxUnsignedInt;
        } else if (nodeIndex > maxIndex) {
            maxIndex = nodeIndex;
        }
    }
    sqlite3_finalize(startStmt);
    return true;
}

bool VirtualMemoryDataBase::ExecuteStaticGraphEndIndex(Protocol::StaticOperatorGraphParams &requestParams,
                                                       const std::string& graphEndSql,
                                                       std::map<int64_t, double> &graphSizeMap, int64_t &maxIndex)
{
    sqlite3_stmt *endStmt = nullptr;
    int graphEndResult = sqlite3_prepare_v2(db, graphEndSql.c_str(), -1, &endStmt, nullptr);
    if (graphEndResult != SQLITE_OK) {
        ServerLog::Error("Query static graph end index. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return false;
    }
    int index = bindStartIndex;
    sqlite3_bind_text(endStmt, index++, requestParams.graphId.c_str(), requestParams.graphId.length(), nullptr);
    if (!requestParams.modelName.empty()) {
        std::string modelName = "%" + requestParams.modelName + "%";
        sqlite3_bind_text(endStmt, index, modelName.c_str(), modelName.length(), nullptr);
    }
    while (sqlite3_step(endStmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        int64_t nodeIndex = sqlite3_column_int64(endStmt, col++);
        double size = sqlite3_column_double(endStmt, col++);
        if (nodeIndex >= maxUnsignedInt) {
            maxIndex = maxUnsignedInt;
        } else if (nodeIndex > maxIndex) {
            maxIndex = nodeIndex;
        }
        if (graphSizeMap.find(nodeIndex) != graphSizeMap.end()) {
            graphSizeMap[nodeIndex] -= size;
        } else {
            graphSizeMap.insert({nodeIndex, -size});
        }
    }
    sqlite3_finalize(endStmt);
    return true;
}

bool VirtualMemoryDataBase::ExecuteStaticOperatorGraph(Protocol::StaticOperatorGraphParams &requestParams,
                                                       Protocol::StaticOperatorGraphItem &graphItem,
                                                       const std::string& totalSql,
                                                       const std::string& graphStartSql,
                                                       const std::string& graphEndSql)
{
    double totalSize = staticDefaultTotalSize;
    if (!ExecuteStaticGraphTotalSize(requestParams, totalSql, totalSize)) {
        return false;
    }
    if (totalSize == staticDefaultTotalSize) {
        ServerLog::Error("Query static operator graph. Failed get TOTAL number. Error: ", sqlite3_errmsg(db));
        return false;
    }
    std::map<int64_t, double> graphSizeMap;
    int64_t maxIndex = 0;
    if (!ExecuteStaticGraphStartIndex(requestParams, graphStartSql, graphSizeMap, maxIndex)) {
        return false;
    }
    if (!ExecuteStaticGraphEndIndex(requestParams, graphEndSql, graphSizeMap, maxIndex)) {
        return false;
    }
    if (graphSizeMap.empty()) {
        ServerLog::Info("Query static operator graph. No data.");
        return false;
    }
    // 组装图例和图像数据
    graphItem.legends.insert(graphItem.legends.end(), staticGraphLegends.begin(), staticGraphLegends.end());
    double size = 0;
    std::string totalSizeStr = StringUtil::DoubleToStringWithTwoDecimalPlaces(totalSize / kbSizeDouble); // 结果保留两位有效数字
    for (auto it = graphSizeMap.begin(); it != graphSizeMap.end(); ++it) {
        std::vector<std::string> points = {};
        size += it->second; // 遍历有序map，逐点计算总需分配内存
        if (it->first < maxUnsignedInt) {
            points.emplace_back(std::to_string(it->first)); // 正常的Node Index
        } else {
            points.emplace_back(std::to_string(maxIndex + 1)); // 存储值为maxUnsignedInt时，Node Index = maxIndex + 1
        }
        points.emplace_back(StringUtil::DoubleToStringWithTwoDecimalPlaces(size / kbSizeDouble)); // Size，结果保留两位有效数字
        points.emplace_back(totalSizeStr); // Total Size
        graphItem.lines.emplace_back(points);
    }

    return true;
}

bool VirtualMemoryDataBase::ExecuteStaticOperatorDetail(Protocol::StaticOperatorListParams &requestParams,
                                                        std::vector<Protocol::MemoryTableColumnAttr> &columnAttr,
                                                        std::vector<Protocol::StaticOperatorItem> &opDetails,
                                                        const std::string& sql)
{
    int64_t pageSize = requestParams.pageSize;
    if (pageSize == 0) {
        pageSize = defaultPageSize;
    }
    int64_t currentPage = requestParams.currentPage - 1;
    if (currentPage < 0) {
        currentPage = 0;
    }
    if (pageSize > maxPageSize || currentPage > maxCurrentPage) {
        ServerLog::Error("Error param: pageSize or currentPage");
        return false;
    }
    int64_t offset = currentPage * pageSize;
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query static operator detail. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return false;
    }
    int index = bindStartIndex;
    std::string searchName = "%" + requestParams.searchName + "%";
    sqlite3_bind_text(stmt, index++, searchName.c_str(), searchName.length(), nullptr);
    if (!requestParams.graphId.empty()) {
        sqlite3_bind_text(stmt, index++, requestParams.graphId.c_str(), requestParams.graphId.length(), nullptr);
    }
    sqlite3_bind_int64(stmt, index++, pageSize);
    sqlite3_bind_int64(stmt, index++, offset);
    std::vector<Protocol::StaticOperatorItem> operatorDtoVec;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        Protocol::StaticOperatorItem operatorDto{};
        operatorDto.deviceId = sqlite3_column_string(stmt, col++);
        operatorDto.opName = sqlite3_column_string(stmt, col++);
        operatorDto.nodeIndexStart = sqlite3_column_int64(stmt, col++);
        operatorDto.nodeIndexEnd = sqlite3_column_int64(stmt, col++);
        operatorDto.size = sqlite3_column_double(stmt, col++);
        operatorDtoVec.emplace_back(operatorDto);
    }
    sqlite3_finalize(stmt);
    opDetails = operatorDtoVec;
    for (const auto& column : staticOpTableColumnAttr) {
        columnAttr.emplace_back(column);
    }
    return true;
}

bool VirtualMemoryDataBase::ExecuteQueryEntireStaticOperatorTable(Protocol::StaticOperatorListParams& requestParams,
                                                                  std::vector<Protocol::StaticOperatorItem>& opDetails,
                                                                  const std::string& sql)
{
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query entire static operator table. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return false;
    }
    int index = bindStartIndex;
    if (!requestParams.graphId.empty()) {
        sqlite3_bind_text(stmt, index++, requestParams.graphId.c_str(), requestParams.graphId.length(), nullptr);
    }
    std::vector<Protocol::StaticOperatorItem> operatorDtoVec;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        Protocol::StaticOperatorItem operatorDto{};
        operatorDto.deviceId = sqlite3_column_string(stmt, col++);
        operatorDto.opName = sqlite3_column_string(stmt, col++);
        operatorDto.nodeIndexStart = sqlite3_column_int64(stmt, col++);
        operatorDto.nodeIndexEnd = sqlite3_column_int64(stmt, col++);
        operatorDto.size = sqlite3_column_double(stmt, col++);
        operatorDtoVec.emplace_back(operatorDto);
    }
    sqlite3_finalize(stmt);
    opDetails = operatorDtoVec;
    return true;
}

void VirtualMemoryDataBase::AddOperatorSql(Protocol::MemoryOperatorParams requestParams, std::string &sql)
{
    std::string ascend;
    if (requestParams.order == "ascend") {
        ascend = "ASC";
    } else {
        ascend = "DESC";
    }
    if (requestParams.type == Protocol::MEMORY_STREAM_GROUP) {
        sql += " AND stream <> ''";
    }
    if (requestParams.startTime != -1 && requestParams.endTime != -1) {
        if (requestParams.isOnlyShowAllocatedOrReleasedWithinInterval) {
            sql += " AND (allocationTimestamp BETWEEN " + std::to_string(requestParams.startTime) +
               " AND " + std::to_string(requestParams.endTime) +
               " OR releaseTimestamp BETWEEN " + std::to_string(requestParams.startTime) +
               " AND " + std::to_string(requestParams.endTime) + ")";
        } else {
            sql += " AND ((";
            sql += (isLowCamel ? "releaseTime" : "release_time");
            sql += " IS NULL OR ";
            sql += (isLowCamel ? "releaseTime" : "release_time");
            sql += " = 0 OR releaseTimestamp >= " + std::to_string(requestParams.startTime) +
                   " ) AND allocationTimestamp <= " + std::to_string(requestParams.endTime) + ")";
        }
    }

    if (requestParams.minSize != std::numeric_limits<int64_t>::min()) {
        sql += " AND size >= " + std::to_string(requestParams.minSize);
    }
    if (requestParams.maxSize != std::numeric_limits<int64_t>::max()) {
        sql += " AND size <= " + std::to_string(requestParams.maxSize);
    }
    if (!requestParams.orderBy.empty() &&
        std::find(Protocol::operatorTableColumn.begin(), Protocol::operatorTableColumn.end(), requestParams.orderBy) !=
        Protocol::operatorTableColumn.end()) {
        auto columnName = isLowCamel ? StringUtil::ToCamelCase(requestParams.orderBy) : requestParams.orderBy;
        sql += " ORDER BY " + columnName + " " + ascend;
    }
    sql += " LIMIT ? offset ?";
}

void VirtualMemoryDataBase::AddStableOperatorSql(Protocol::StaticOperatorListParams requestParams, std::string &sql)
{
    std::string ascend;
    if (requestParams.order == "ascend") {
        ascend = "ASC";
    } else {
        ascend = "DESC";
    }

    if (!requestParams.graphId.empty()) {
        sql += " AND graph_id = ?" ;
    }

    if (requestParams.startNodeIndex >= 0 && requestParams.endNodeIndex >= requestParams.startNodeIndex) {
        sql += " AND (node_index_start BETWEEN " + std::to_string(requestParams.startNodeIndex) +
                " AND " + std::to_string(requestParams.endNodeIndex) +
                " OR node_index_end BETWEEN " + std::to_string(requestParams.startNodeIndex) +
                " AND " + std::to_string(requestParams.endNodeIndex) +")";
    }

    if (requestParams.minSize != std::numeric_limits<int64_t>::min()) {
        sql += " AND size >= " + std::to_string(requestParams.minSize);
    }
    if (requestParams.maxSize != std::numeric_limits<int64_t>::max()) {
        sql += " AND size <= " + std::to_string(requestParams.maxSize);
    }
    if (!requestParams.orderBy.empty() &&
        std::find(Protocol::staticOperatorTableColumn.begin(), Protocol::staticOperatorTableColumn.end(),
        requestParams.orderBy) != Protocol::staticOperatorTableColumn.end()) {
        auto columnName = isLowCamel ? StringUtil::ToCamelCase(requestParams.orderBy) : requestParams.orderBy;
        sql += " ORDER BY " + columnName + " " + ascend;
    }
    sql += " LIMIT ? offset ?";
}

static void PaddingNULL(std::vector<std::string> &points, const uint8_t count)
{
    if (count == 0) {
        return;
    }
    const std::string stringNull = "NULL";
    for (uint8_t i = 0; i < count; i++) {
        points.emplace_back(stringNull);
    }
}

void VirtualMemoryDataBase::BuildOverallLinesComponentPoints(const Protocol::ComponentDto &item,
                                                             const std::vector<std::string> &streams,
                                                             Protocol::MemoryPeak &peak,
                                                             Points &points)
{
    peak.appReserved = std::max(peak.appReserved, item.totalReserved);
    if (peak.hasPtaGe) {
        PaddingNULL(points, 1);
    }
    if (!streams.empty()) {
        PaddingNULL(points, 1);
    }
    if (peak.hasPtaGe) {
        PaddingNULL(points, 1);
    }
    std::string reserved = std::to_string(item.totalReserved);
    points.emplace_back(reserved.substr(0, reserved.length() - exLength));
    if (peak.hasWorkspace) {
        // workspaceLegends为内部定义vector, 其size不可能超过uint8, 此处无溢出风险
        PaddingNULL(points, workspaceLegends.size());
    }
}

void VirtualMemoryDataBase::BuildOverallLinesFrameworkPoints(const Protocol::ComponentDto &item,
                                                             const std::vector<std::string> &streams,
                                                             Protocol::MemoryPeak &peak,
                                                             Points &points)
{
    peak.ptaGeAllocated = std::max(peak.ptaGeAllocated, item.totalAllocated);
    peak.ptaGeReserved = std::max(peak.ptaGeReserved, item.totalReserved);
    peak.ptaGeActivated = std::max(peak.ptaGeActivated, item.totalActivated);
    std::string allocated = std::to_string(item.totalAllocated);
    points.emplace_back(allocated.substr(0, allocated.length() - exLength));
    if (!streams.empty()) {
        std::string activated = std::to_string(item.totalActivated);
        points.emplace_back(activated.substr(0, activated.length() - exLength));
    }
    std::string reserved = std::to_string(item.totalReserved);
    points.emplace_back(reserved.substr(0, reserved.length() - exLength));
    if (peak.hasApp) {
        PaddingNULL(points, 1);
    }
    if (peak.hasWorkspace) {
        PaddingNULL(points, workspaceLegends.size());
    }
}

void VirtualMemoryDataBase::BuildOverallLinesWorkspacePoints(const Protocol::ComponentDto &item,
                                                             const std::vector<std::string> &streams,
                                                             Protocol::MemoryPeak &peak,
                                                             Points &points)
{
    if (peak.hasPtaGe) {
        PaddingNULL(points, 1);
    }
    if (!streams.empty()) {
        PaddingNULL(points, 1);
    }
    if (peak.hasPtaGe) {
        PaddingNULL(points, 1);
    }
    if (peak.hasApp) {
        PaddingNULL(points, 1);
    }
    std::string allocated = std::to_string(item.totalAllocated);
    std::string reserved = std::to_string(item.totalReserved);
    points.emplace_back(allocated.substr(0, allocated.length() - exLength));
    points.emplace_back(reserved.substr(0, reserved.length() - exLength));
}

/*
 * 将多个单条线的数据组装成[x,y,y,y,y]的格式。
 * 各元素分别表示标签"Time (ms)", "Operators Allocated", "Operators Activated", "Operators Reserved" "App Reserved"。
 * 如果整组数据中某个标签的数据都不存在，不仅在标签中删除，也在[x,y,y,y,y]中删除该元素。
 * 如果只是部分时间上某些标签的数据不存在，则补NULL。
 */
void VirtualMemoryDataBase::GetOverallLines(const componentDtoVector &componentDtoVec,
    std::vector<std::vector<std::string>> &lines, std::vector<std::string> &legends, Protocol::MemoryPeak &peak,
    const std::vector<std::string> &streams)
{
    GetOverallLinesLegends(componentDtoVec, legends, peak, streams);
    for (auto &item: componentDtoVec) {
        Points points = {};
        std::string time = std::to_string(item.timesTamp);
        points.emplace_back(time.substr(0, time.length() - exLength + 1));
        if (item.component == COMPONENT_APP) {
            BuildOverallLinesComponentPoints(item, streams, peak, points);
            lines.emplace_back(points);
            continue;
        }
        if (item.component == COMPONENT_PTA_AND_GE || item.component == MIND_SPORE_GE
            || (isInference && item.component == COMPONENT_GE)) {
            BuildOverallLinesFrameworkPoints(item, streams, peak, points);
            lines.emplace_back(points);
            continue;
        }
        if (item.component == COMPONENT_WORKSPACE) {
            BuildOverallLinesWorkspacePoints(item, streams, peak, points);
            lines.emplace_back(points);
        }
    }
}

void VirtualMemoryDataBase::GetOverallLinesLegends(const componentDtoVector &componentDtoVec,
    std::vector<std::string> &legends, Protocol::MemoryPeak &peak,
    const std::vector<std::string> &streams)
{
    for (auto &item: componentDtoVec) {
        if (item.component == COMPONENT_WORKSPACE) {
            peak.hasWorkspace = true;
            continue;
        }
        if (item.component == COMPONENT_PTA_AND_GE || item.component == MIND_SPORE_GE
            || (isInference && item.component == COMPONENT_GE)) {
            peak.hasPtaGe = true;
            continue;
        }
        if (item.component == COMPONENT_APP) {
            peak.hasApp = true;
        }
    }

    for (const auto& legend : baseLegends) {
        if (!peak.hasPtaGe && legend == "Operators Allocated") {
            continue;
        }
        if (streams.empty() && legend == "Operators Activated") {
            continue;
        }
        if (!peak.hasPtaGe && legend == "Operators Reserved") {
            continue;
        }
        legends.emplace_back(legend);
    }
    if (peak.hasApp) {
        legends.emplace_back(appLegend);
    }
    if (peak.hasWorkspace) {
        legends.insert(legends.end(), workspaceLegends.begin(), workspaceLegends.end());
    }
}

void VirtualMemoryDataBase::GetComponentLines(const Dic::Module::Memory::componentDtoVector &componentDtoVec,
                                              std::vector<std::vector<std::string>> &lines,
                                              std::vector<std::string> &legends, Protocol::MemoryPeak &peak,
                                              const std::vector<std::string> &streams)
{
    GetComponentLinesLegends(componentDtoVec, legends, peak);

    // 3表示无PTA数据或无GE数据插入3个NULL
    const int sizeMembers = 3;
    for (auto &item: componentDtoVec) {
        std::vector<std::string> points = {};
        if (item.component == COMPONENT_PTA) {
            peak.ptaAllocated = std::max(peak.ptaAllocated, item.totalAllocated);
            peak.ptaReserved = std::max(peak.ptaReserved, item.totalReserved);
            peak.ptaActivated = std::max(peak.ptaActivated, item.totalActivated);

            std::string time = std::to_string(item.timesTamp);
            points.emplace_back(time.substr(0, time.length() - exLength + 1));
            InsertSize(points, item);
            if (peak.hasGe) {
                InsertStringNull(points, sizeMembers);
            }
            if (peak.hasApp) {
                InsertStringNull(points, 1);
            }
            lines.emplace_back(points);
        } else if (item.component == COMPONENT_GE) {
            peak.geAllocated = std::max(peak.geAllocated, item.totalAllocated);
            peak.geReserved = std::max(peak.geReserved, item.totalReserved);
            peak.geActivated = std::max(peak.geActivated, item.totalActivated);

            std::string time = std::to_string(item.timesTamp);
            points.emplace_back(time.substr(0, time.length() - exLength + 1));
            if (peak.hasPta) {
                InsertStringNull(points, sizeMembers);
            }
            InsertSize(points, item);
            if (peak.hasApp) {
                InsertStringNull(points, 1);
            }
            lines.emplace_back(points);
        } else if (item.component == COMPONENT_APP) {
            peak.appReserved = std::max(peak.appReserved, item.totalReserved);

            std::string time = std::to_string(item.timesTamp);
            points.emplace_back(time.substr(0, time.length() - exLength + 1));
            if (peak.hasPta) {
                InsertStringNull(points, sizeMembers);
            }
            if (peak.hasGe) {
                InsertStringNull(points, sizeMembers);
            }
            std::string reserved = std::to_string(item.totalReserved);
            points.emplace_back(reserved.substr(0, reserved.length() - exLength));
            lines.emplace_back(points);
        }
    }
}

void VirtualMemoryDataBase::InsertSize(std::vector<std::string> &points, const Protocol::ComponentDto &item)
{
    std::string allocated = std::to_string(item.totalAllocated);
    points.emplace_back(allocated.substr(0, allocated.length() - exLength));
    std::string activated = std::to_string(item.totalActivated);
    points.emplace_back(activated.substr(0, activated.length() - exLength));
    std::string reserved = std::to_string(item.totalReserved);
    points.emplace_back(reserved.substr(0, reserved.length() - exLength));
}

void VirtualMemoryDataBase::InsertStringNull(std::vector<std::string> &points, const int times)
{
    const std::string stringNull = "NULL";
    for (int i = 0; i < times; ++i) {
        points.emplace_back(stringNull);
    }
}

void VirtualMemoryDataBase::GetComponentLinesLegends(const Dic::Module::Memory::componentDtoVector &componentDtoVec,
                                                     std::vector<std::string> &legends, Protocol::MemoryPeak &peak)
{
    for (auto &item: componentDtoVec) {
        if (item.component == COMPONENT_PTA) {
            peak.hasPta = true;
        } else if (item.component == COMPONENT_GE) {
            peak.hasGe = true;
        } else if (item.component == COMPONENT_APP) {
            peak.hasApp = true;
        }
    }

    legends = componentTimeLegends;
    if (peak.hasPta) {
        legends.insert(legends.end(), componentPtaLegends.begin(), componentPtaLegends.end());
    }
    if (peak.hasGe) {
        legends.insert(legends.end(), componentGeLegends.begin(), componentGeLegends.end());
    }

    if (peak.hasApp) {
        legends.emplace_back(appLegend);
    }
}

void VirtualMemoryDataBase::GetStreamLines(const componentDtoVector &componentDtoVec,
    std::vector<std::vector<std::string>> &lines, std::vector<std::string> &legends, Protocol::MemoryPeak &peak,
    const std::vector<std::string> &streams)
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

std::string VirtualMemoryDataBase::GetPeakMemory(const Protocol::MemoryPeak &peak,
    const std::vector<std::string> &streams)
{
    std::string peakMemory = "Peak Memory Usage: ";
    const size_t decimalPlacesNum = 4;
    if (peak.hasPtaGe) {
        std::string ptaGeAllo = std::to_string(peak.ptaGeAllocated);
        // double转换成string默认生成六位小数，删除后4位小数
        ptaGeAllo = ptaGeAllo.substr(0, ptaGeAllo.length() - decimalPlacesNum);
        peakMemory.append("Operator Allocated: ").append(ptaGeAllo).append("MB");
        if (!streams.empty()) {
            std::string ptaGeActive = std::to_string(peak.ptaGeActivated);
            ptaGeActive = ptaGeActive.substr(0, ptaGeActive.length() - decimalPlacesNum);
            peakMemory.append(" | Operator Activated: ").append(ptaGeActive).append("MB");
        }
        std::string ptaGeRe = std::to_string(peak.ptaGeReserved);
        ptaGeRe = ptaGeRe.substr(0, ptaGeRe.length() - decimalPlacesNum);
        peakMemory.append(" | Operator Reserved: ").append(ptaGeRe).append("MB");
    }
    if (peak.hasPta) {
        std::string ptaAllo = std::to_string(peak.ptaAllocated);
        ptaAllo = ptaAllo.substr(0, ptaAllo.length() - decimalPlacesNum);
        peakMemory.append("PTA Allocated: ").append(ptaAllo).append("MB");
        std::string ptaActi = std::to_string(peak.ptaActivated);
        ptaActi = ptaActi.substr(0, ptaActi.length() - decimalPlacesNum);
        peakMemory.append(" | PTA Activated: ").append(ptaActi).append("MB");
        std::string ptaRes = std::to_string(peak.ptaReserved);
        ptaRes = ptaRes.substr(0, ptaRes.length() - decimalPlacesNum);
        peakMemory.append(" | PTA Reserved: ").append(ptaRes).append("MB");
    }
    if (peak.hasGe) {
        std::string geAllo = std::to_string(peak.geAllocated);
        geAllo = geAllo.substr(0, geAllo.length() - decimalPlacesNum);
        peakMemory.append(" | GE Allocated: ").append(geAllo).append("MB");
        std::string geActi = std::to_string(peak.geActivated);
        geActi = geActi.substr(0, geActi.length() - decimalPlacesNum);
        peakMemory.append(" | GE Activated: ").append(geActi).append("MB");
        std::string geRes = std::to_string(peak.geReserved);
        geRes = geRes.substr(0, geRes.length() - decimalPlacesNum);
        peakMemory.append(" | GE Reserved: ").append(geRes).append("MB");
    }
    if (peak.hasApp) {
        std::string appAllo = std::to_string(peak.appReserved);
        appAllo = appAllo.substr(0, appAllo.length() - decimalPlacesNum);
        peakMemory.append(" | APP Reserved: ").append(appAllo).append("MB");
    }
    return peakMemory;
}
}
}
}