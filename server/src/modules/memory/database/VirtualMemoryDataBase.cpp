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

std::vector<std::string> VirtualMemoryDataBase::GetStreamLists(std::string rankId)
{
    std::vector<std::string> streams = {};
    DataType type = DataBaseManager::Instance().GetDataType();
    std::string sql = "";
    if (type == DataType::TEXT) {
        sql += "SELECT stream FROM " + recordTable + " WHERE stream <> '' Group BY stream ORDER BY timestamp ASC";
    } else if (type == DataType::DB) {
        FileType fileType = DataBaseManager::Instance().GetFileType();
        if (fileType == FileType::PYTORCH) {
            std::string streamPtrColumnName = isLowCamel ? "streamPtr" : "stream_ptr";
            std::string timeColumnName = isLowCamel ? "timestamp" : "time_stamp";
            sql += "SELECT " + streamPtrColumnName + " FROM " + TABLE_MEMORY_RECORD +
                " WHERE " + streamPtrColumnName + " <> ''"
                " Group BY " + streamPtrColumnName + " ORDER BY " + timeColumnName + " ASC";
        } else {
            ServerLog::Error("Memory tab does not support msprof data.");
            return streams;
        }
    }
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Failed to prepare sql to get stream list. Error: ", sqlite3_errmsg(db));
        return streams;
    }
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
        ServerLog::Error("Query operator size failed!. Error: ", sqlite3_errmsg(db));
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
        ServerLog::Error("Query operator size failed!. Error: ", sqlite3_errmsg(db));
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

bool VirtualMemoryDataBase::ExecuteOperatorSize(double &min, double &max, std::string sql)
{
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query operator size failed!. Error: ", sqlite3_errmsg(db));
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

bool VirtualMemoryDataBase::ExecuteOperatorsTotalNum(Protocol::MemoryOperatorParams &requestParams, int64_t &totalNum,
    std::string sql)
{
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return false;
    }
    int index = bindStartIndex;
    std::string orderName = "%" + requestParams.searchName + "%";
    sqlite3_bind_text(stmt, index++, orderName.c_str(), orderName.length(), nullptr);
    uint64_t startTime = Timeline::TraceTime::Instance().GetStartTime();
    uint64_t offsetTime = Timeline::TraceTime::Instance().GetOffsetByFileId(requestParams.rankId);
    if (requestParams.startTime != -1 && requestParams.endTime != -1) {
        sqlite3_bind_int64(stmt, index++, startTime + offsetTime);
        sqlite3_bind_double(stmt, index++, requestParams.startTime);
        sqlite3_bind_int64(stmt, index++, startTime + offsetTime);
        sqlite3_bind_double(stmt, index++, requestParams.endTime);
    }

    if (requestParams.minSize != std::numeric_limits<int64_t>::min()) {
        sqlite3_bind_double(stmt, index++, requestParams.minSize);
    }
    if (requestParams.maxSize != std::numeric_limits<int64_t>::max()) {
        sqlite3_bind_double(stmt, index++, requestParams.maxSize);
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
        ServerLog::Error("Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return false;
    }
    int index = bindStartIndex;
    std::string searchName = "%" + requestParams.searchName + "%";
    sqlite3_bind_text(stmt, index++, searchName.c_str(), searchName.length(), nullptr);
    if (!requestParams.graphId.empty()) {
        sqlite3_bind_text(stmt, index++, requestParams.graphId.c_str(), requestParams.graphId.length(), nullptr);
    }
    if (!requestParams.modelName.empty()) {
        std::string modelName = "%" + requestParams.modelName + "%";
        sqlite3_bind_text(stmt, index++, modelName.c_str(), modelName.length(), nullptr);
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

bool VirtualMemoryDataBase::ExecuteQueryMemoryViewExecuteSql(Protocol::MemoryComponentParams &requestParams,
                                                             std::vector<Protocol::ComponentDto> &componentDtoVec,
                                                             std::vector<std::string> &streams, std::string &sql)
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
    streams = GetStreamLists(requestParams.rankId);
    return true;
}

bool VirtualMemoryDataBase::ExecuteQueryMemoryViewGetGraph(Protocol::MemoryComponentParams &requestParams,
                                                           std::vector<Protocol::ComponentDto> &componentDtoVec,
                                                           std::vector<std::string> &streams,
                                                           Protocol::MemoryViewData &operatorBody)
{
    Protocol::MemoryPeak peak;
    if (requestParams.type == Protocol::MEMORY_OVERALL_GROUP) {
        GetOverallLines(componentDtoVec, operatorBody.lines, operatorBody.legends, peak, streams);
        operatorBody.title = GetPeakMemory(peak, streams);
    } else {
        GetStreamLines(componentDtoVec, operatorBody.lines, operatorBody.legends, peak, streams);
    }
    return true;
}

bool VirtualMemoryDataBase::ExecuteOperatorDetail(Protocol::MemoryOperatorParams &requestParams,
                                                  std::vector<Protocol::MemoryTableColumnAttr> &columnAttr,
                                                  std::vector<Protocol::MemoryOperator> &opDetails, std::string sql)
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
    std::string orderName = "%" + requestParams.searchName + "%";

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
    opDetails = operatorDtoVec;
    std::vector<std::string> streams = GetStreamLists(requestParams.rankId);
    std::vector<std::string> columns = activeRelatedColumn;
    for (const auto& column : tableColumnAttr) {
        if (streams.empty() && std::find(columns.begin(), columns.end(), column.name) != columns.end()) {
            continue;
        }
        columnAttr.emplace_back(column);
    }
    return true;
}

bool VirtualMemoryDataBase::ExecuteQueryEntireOperatorTable(std::vector<Protocol::MemoryTableColumnAttr> &columnAttr,
                                                            std::vector<Protocol::MemoryOperator> &opDetails,
                                                            const std::string &sql, const std::string rankId)
{
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query entire operator table. Failed to prepare sql. Error: ", sqlite3_errmsg(db));
        return false;
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
    std::vector<std::string> streams = GetStreamLists(rankId);
    std::vector<std::string> columns = activeRelatedColumn;
    for (const auto& column : tableColumnAttr) {
        if (streams.empty() && std::find(columns.begin(), columns.end(), column.name) != columns.end()) {
            continue;
        }
        columnAttr.emplace_back(column);
    }
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
    sqlite3_bind_text(totalStmt, index, requestParams.graphId.c_str(), requestParams.graphId.length(), nullptr);
    if (!requestParams.modelName.empty()) {
        std::string modelName = "%" + requestParams.modelName + "%";
        sqlite3_bind_text(totalStmt, index, modelName.c_str(), modelName.length(), nullptr);
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
        maxIndex = nodeIndex;
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
        if (nodeIndex != maxUnsignedInt && nodeIndex > maxIndex) { // 无符号最大INT值，表示最终释放时终止,这里直接跳过
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
        if (it->first != maxUnsignedInt) {
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
                                                                  std::vector<Protocol::MemoryTableColumnAttr>&
                                                                  columnAttr,
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
    for (const auto& column : staticOpTableColumnAttr) {
        columnAttr.emplace_back(column);
    }
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
        sql += " AND ((";
        sql += (isLowCamel ? "releaseTime" : "release_time");
        sql += " IS NULL OR ";
        sql += (isLowCamel ? "releaseTime" : "release_time");
        sql += " = 0 OR releaseTimestamp >= " + std::to_string(requestParams.startTime) +
               " ) AND allocationTimestamp <= " + std::to_string(requestParams.endTime) + ")";
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

    const std::string stringNull = "NULL";
    for (auto &item: componentDtoVec) {
        std::vector<std::string> points = {};
        if (item.component == COMPONENT_PTA_AND_GE || item.component == MIND_SPORE_GE
            || (isInference && item.component == COMPONENT_GE)) {
            peak.ptaGeAllocated = std::max(peak.ptaGeAllocated, item.totalAllocated);
            peak.ptaGeReserved = std::max(peak.ptaGeReserved, item.totalReserved);
            peak.ptaGeActivated = std::max(peak.ptaGeActivated, item.totalActivated);
            std::string time = std::to_string(item.timesTamp);
            points.emplace_back(time.substr(0, time.length() - exLength + 1));
            std::string allocated = std::to_string(item.totalAllocated);
            points.emplace_back(allocated.substr(0, allocated.length() - exLength));
            if (!streams.empty()) {
                std::string activated = std::to_string(item.totalActivated);
                points.emplace_back(activated.substr(0, activated.length() - exLength));
            }
            std::string reserved = std::to_string(item.totalReserved);
            points.emplace_back(reserved.substr(0, reserved.length() - exLength));
            if (peak.hasApp) {
                points.emplace_back(stringNull);
            }
            lines.emplace_back(points);
        } else if (item.component == COMPONENT_APP) {
            peak.appReserved = std::max(peak.appReserved, item.totalReserved);
            std::string time = std::to_string(item.timesTamp);
            points.emplace_back(time.substr(0, time.length() - exLength + 1));
            if (peak.hasPtaGe) {
                points.emplace_back(stringNull);
            }
            if (!streams.empty()) {
                points.emplace_back(stringNull);
            }
            if (peak.hasPtaGe) {
                points.emplace_back(stringNull);
            }
            std::string reserved = std::to_string(item.totalReserved);
            points.emplace_back(reserved.substr(0, reserved.length() - exLength));
            lines.emplace_back(points);
        }
    }
}

void VirtualMemoryDataBase::GetOverallLinesLegends(const componentDtoVector &componentDtoVec,
    std::vector<std::string> &legends, Protocol::MemoryPeak &peak,
    const std::vector<std::string> &streams)
{
    for (auto &item: componentDtoVec) {
        if (item.component == COMPONENT_PTA_AND_GE || item.component == MIND_SPORE_GE
            || (isInference && item.component == COMPONENT_GE)) {
            peak.hasPtaGe = true;
            } else if (item.component == COMPONENT_APP) {
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