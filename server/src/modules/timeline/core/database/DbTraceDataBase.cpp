// Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
#include "pch.h"
#include "TraceTime.h"
#include "TableDefs.h"
#include "TraceDatabaseHelper.h"
#include "CommonDefs.h"
#include "DataBaseManager.h"
#include "CommonCacheManager.h"
#include "CollectionTimeService.h"
#include "DbTraceDataBase.h"

namespace Dic::Module::FullDb {
using namespace Server;
static std::map<std::string, std::map<std::string, std::string>> stringsCache;

DbTraceDataBase::~DbTraceDataBase()
{
    for (const auto &rankId: rankIds) {
        CommonCacheManager::Instance().EraseFlowByRank(rankId);
    }
    updateCannApiDepthStmt = nullptr;
    insertOverlapStmt = nullptr;
    updateApiDepthStmt = nullptr;
    updateTaskDepthStmt = nullptr;
}

bool DbTraceDataBase::QueryThreads(const Protocol::UnitThreadsParams &requestParams,
                                   Protocol::UnitThreadsBody &responseBody,
                                   uint64_t minTimestamp,
                                   const std::vector<uint64_t> &trackIdList)
{
    uint64_t startTime = requestParams.startTime + minTimestamp;
    uint64_t endTime = requestParams.endTime + minTimestamp;
    // 遍历metadataList,这里不用union all,方便后续返回中拼接pid和tid等信息用于前端过滤
    std::vector<SimpleSlice> simpleSliceVec;
    std::map<std::string, uint64_t> selfTimeKeyValue;
    for (auto &&metadata: requestParams.metadataList) {
        std::string rankId = GetRealRankId(requestParams.rankId);
        std::vector<Protocol::SimpleSlice> completeSlice = QueryThreadByPid(metadata, startTime, endTime, rankId,
                                                                            selfTimeKeyValue);
        simpleSliceVec.insert(simpleSliceVec.end(), completeSlice.begin(), completeSlice.end());
    }
    // process data
    if (simpleSliceVec.empty()) {
        responseBody.emptyFlag = true;
        return true;
    }

    std::vector<Protocol::SimpleSlice> nRows =
        TraceDatabaseHelper::ThreadsInfoFilter(simpleSliceVec, startTime, endTime);
    TraceDatabaseHelper::ReduceThread(nRows, selfTimeKeyValue, responseBody);
    return true;
}

bool DbTraceDataBase::QueryThreadDetail(const Protocol::ThreadDetailParams &requestParams,
    Protocol::UnitThreadDetailBody &responseBody, uint64_t minTimestamp, int64_t trackId)
{
    auto stmt = CreatPreparedStatement();
    if (stmt == nullptr) {
        ServerLog::Error("Query thread detailed. Failed to prepare sql.", sqlite3_errmsg(db));
        return false;
    }
    std::optional<SliceDto> sliceDto;
    try {
        sliceDto = TraceDatabaseHelper::QueryThreadDetail(stmt, requestParams);
    } catch (DatabaseException &e) {
        ServerLog::Error("Query thread detail failed, ", e.What());
        return false;
    }

    if (!sliceDto.has_value()) {
        ServerLog::Error("select slice error!");
        return false;
    }
    responseBody.data.title = sliceDto->name;
    responseBody.data.duration = sliceDto->duration;
    if (requestParams.metaType == TABLE_OVERLAP_ANALYSIS) {
        return true;
    }
    responseBody.emptyFlag = false;
    responseBody.data.cat = sliceDto->cat;
    TraceDatabaseHelper::QueryTaskInfoById(stmt, requestParams, responseBody, stringsCache.at(path), metaVersion);
    return true;
}

bool DbTraceDataBase::QueryUnitsMetadata(const std::string &fileId,
    std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData)
{
    if (CheckTableExist(TABLE_TASK)) {
        QueryOperateMetadata(fileId, metaData);
        GenerateOverlapAnalysisMetadata(fileId, metaData);
    }
    QueryCounterMetadata(fileId, metaData);
    GenerateCounterMetadata(fileId, metaData);
    return false;
}

bool DbTraceDataBase::GenerateOverlapAnalysisMetadata(const std::string &fileId,
                                                      std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData)
{
    auto metaType = ENUM_TO_STR(PROCESS_TYPE::OVERLAP_ANALYSIS).value_or("");
    auto overlap_analysis = GenerateBaseUnitTrack("process", fileId, metaType, metaType, metaType);
    for (size_t index = 0; index < OVERLAP_TYPES.size(); index++) {
        auto thread = GenerateBaseUnitTrack("thread", fileId,
                                            overlap_analysis->metaData.processId, "", metaType);
        thread->metaData.threadId = std::to_string(index);
        thread->metaData.threadName = OVERLAP_TYPES[index];
        thread->metaData.maxDepth = 1;
        overlap_analysis->children.emplace_back(std::move(thread));
        stringsCache.at(path).emplace(metaType + std::to_string(index), OVERLAP_TYPES[index]);
    }
    metaData.emplace_back(std::move(overlap_analysis));
    return true;
}

bool DbTraceDataBase::QueryExtremumTimestamp(uint64_t &min, uint64_t &max)
{
    return false;
}

bool DbTraceDataBase::QueryUintFlows(const Protocol::UnitFlowsParams &requestParams,
    Protocol::UnitFlowsBody &responseBody, uint64_t minTimestamp, uint64_t trackId)
{
    auto stmt = CreatPreparedStatement();
    auto connectionId = TraceDatabaseHelper::QueryConnectionId(stmt, requestParams);
    if (!connectionId.has_value()) {
        return false;
    }
    std::string sql = "with constValue as (select ? as minTime, ? as connectionId)\n";
    sql += PYTORCH_UNIT_FLOW_SQL + " union all ";
    sql += CANN_UNIT_FLOW_SQL + " union all ";
    sql += TASK_UNIT_FLOW_SQL + " union all " + MSTX_UNIT_FLOW_SQL;
    sql += " order by startTime ";
    auto resultSet = TraceDatabaseHelper::ExecuteQuery(stmt, sql, minTimestamp, connectionId.value());
    std::vector<FlowLocation> flowLocations;
    while (resultSet->Next()) {
        auto metaType = resultSet->GetString("metaType");
        auto rankId = resultSet->GetString("deviceId");
        rankId = rankId.empty() ? path : QueryHostInfo() + rankId;
        FlowLocation location {
            .tid = resultSet->GetString("tid"), .id = resultSet->GetString("id"),
            .metaType = metaType, .rankId = rankId,
            .depth = resultSet->GetInt32("depth"), .timestamp = resultSet->GetUint64("startTime"),
            .duration = resultSet->GetUint64("duration"), .pid = resultSet->GetString("pid"),
            .name = stringsCache.at(path)[resultSet->GetString("name")]
        };
        flowLocations.push_back(location);
    }
    if (flowLocations.size() < 2) { // 小于2表示没有连线
        return false;
    }
    std::map<std::string, std::vector<UnitSingleFlow>> flowMap;
    for (size_t index = 1; index < flowLocations.size(); index++) {
        UnitSingleFlow singleFlow;
        singleFlow.id = connectionId.value();
        singleFlow.from = flowLocations[index - 1];
        singleFlow.to = flowLocations[index];
        if (singleFlow.from.metaType == singleFlow.to.metaType && singleFlow.from.metaType == TABLE_API) {
            singleFlow.cat = singleFlow.from.name == "Enqueue" ? "async_task_queue" : "fwdbwd";
        }
        singleFlow.cat = singleFlow.from.metaType == TABLE_CANN_API ? "HostToDevice" : singleFlow.cat;
        singleFlow.cat = singleFlow.from.metaType == TABLE_MSTX_EVENTS ? "MsTx" : singleFlow.cat;
        if (singleFlow.from.metaType == TABLE_API && singleFlow.to.metaType == TABLE_CANN_API) {
            singleFlow.cat = "async_npu";
        }
        flowMap[singleFlow.cat].push_back(singleFlow);
    }
    for (const auto &item: flowMap) {
        responseBody.unitAllFlows.push_back({ .cat = item.first, .flows = item.second });
    }
    return true;
}

int DbTraceDataBase::SearchSliceNameCount(const Protocol::SearchCountParams &params)
{
    int32_t result = 0;
    const std::string &sql = GetSearchSliceNameCountSql(params.isMatchExact, params.isMatchCase, params.rankId);
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Query slice name count failed!.");
        return 0;
    }
    auto resultSet = stmt->ExecuteQuery(params.searchContent, GetRealRankId(params.rankId));
    if (resultSet == nullptr) {
        ServerLog::Error("Query_slice_name_count. Failed to get result set.", stmt->GetErrorMessage());
        return 0;
    }
    if (resultSet->Next()) {
        result = resultSet->GetInt32(resultStartIndex);
    }
    return result;
}

bool DbTraceDataBase::QueryFlowCategoryList(std::vector<std::string> &categories, const std::string& rankId)
{
    CommonCacheManager::Instance().GetCategoryList(rankId, categories);
    return true;
}

bool DbTraceDataBase::SearchSliceName(const Protocol::SearchSliceParams &params, int index, uint64_t minTimestamp,
    Protocol::SearchSliceBody &responseBody)
{
    const std::string &sql = GetSearchSliceNameSql(params.isMatchExact, params.isMatchCase, responseBody.rankId,
                                                   "descend", "timestamp");
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Query slice name failed!.");
        return false;
    }
    auto resultSet = stmt->ExecuteQuery(params.searchContent, minTimestamp, GetRealRankId(responseBody.rankId), index);
    if (resultSet == nullptr || !resultSet->Next()) {
        ServerLog::Error("Query_slice_name. Failed to get result set.", stmt->GetErrorMessage());
        return false;
    }
    responseBody.pid = resultSet->GetString("pid");
    responseBody.tid = resultSet->GetString("tid");
    responseBody.startTime = resultSet->GetUint64("startTime");
    responseBody.duration = resultSet->GetUint64("duration");
    responseBody.depth = resultSet->GetInt32("depth");
    responseBody.id = resultSet->GetString("id");
    return true;
}

bool DbTraceDataBase::QueryFlowCategoryEvents(Protocol::FlowCategoryEventsParams &params, uint64_t minTimestamp,
    std::vector<std::unique_ptr<Protocol::UnitSingleFlow>> &flowDetailList)
{
    std::vector<std::string> deviceIds;
    if (params.rankId.empty() || !DataBaseManager::Instance().GetDbPathByHost(params.rankId).empty()) {
        for (const auto &rankId: rankIds) {
            deviceIds.emplace_back(QueryHostInfo() + rankId);
        }
    } else {
        deviceIds.emplace_back(params.rankId);
    }
    for (const auto &rankId: deviceIds) {
        auto flowCache = CommonCacheManager::Instance().GetFlowCache(rankId, params.category);
        for (const auto &flow: flowCache) {
            if (flow.from.timestamp > params.startTime && flow.from.timestamp < params.endTime) {
                flowDetailList.emplace_back(std::make_unique<UnitSingleFlow>(flow));
                continue;
            }
            if (flow.to.timestamp > params.startTime && flow.to.timestamp < params.endTime) {
                flowDetailList.emplace_back(std::make_unique<UnitSingleFlow>(flow));
            }
        }
    }
    return true;
}

bool DbTraceDataBase::QueryUnitCounter(Protocol::UnitCounterParams &params, uint64_t minTimestamp,
    std::vector<Protocol::UnitCounterData> &dataList)
{
    auto stmt = CreatPreparedStatement();
    if (stmt == nullptr) {
        ServerLog::Error("Query_unit_counter. Failed to prepare sql.", sqlite3_errmsg(db));
        return false;
    }
    std::unique_ptr<SqliteResultSet> resultSet;
    try {
        resultSet = TraceDatabaseHelper::QueryUnitCounter(stmt, params, minTimestamp, GetRealRankId(params.rankId));
    } catch (DatabaseException &e) {
        ServerLog::Error("Query unit counter failed, ", e.What());
        return false;
    }
    while (resultSet->Next()) {
        Protocol::UnitCounterData unitCounterData;
        unitCounterData.timestamp = resultSet->GetInt64("startTime");
        unitCounterData.valueJsonStr = resultSet->GetString("args");
        dataList.emplace_back(unitCounterData);
    }
    return true;
}

bool DbTraceDataBase::QueryComputeStatisticsData(const Protocol::SummaryStatisticParams &requestParams,
    Protocol::SummaryStatisticsBody &responseBody)
{
    std::string stepCondition;
    sqlite3_stmt *stmt = nullptr;
    std::string sql = "SELECT round(sum(endNs - startNs) / 1000.0, 2) as duration, TASKTYPE.value as acceleratorCore "
        "  FROM COMPUTE_TASK_INFO"
        "     JOIN TASK ON COMPUTE_TASK_INFO.globalTaskId = TASK.globalTaskId "
        "     JOIN STRING_IDS AS TASKTYPE ON TASKTYPE.id = COMPUTE_TASK_INFO.taskType"
        " WHERE acceleratorCore in ('AI_CPU','AI_CORE',"
        " 'AI_VECTOR_CORE', 'MIX_AIC', 'MIX_AIV', 'FFTS_PLUS') "
        " GROUP BY acceleratorCore";
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        Server::ServerLog::Error("Query compute statistics data failed!. ", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return false;
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

bool DbTraceDataBase::QueryCommunicationStatisticsData(const Protocol::SummaryStatisticParams &requestParams,
    Protocol::SummaryStatisticsBody &responseBody)
{
    return false;
}

bool DbTraceDataBase::QueryStepDuration(const std::string &stepId, uint64_t &min, uint64_t &max)
{
    return false;
}

bool DbTraceDataBase::QuerySystemViewData(const Protocol::SystemViewParams &requestParams,
    Protocol::SystemViewBody &responseBody)
{
    auto stmt = CreatPreparedStatement(); // 这里不需要判断空指针，TraceDatabaseHelper里面统一进行了判空操作
    std::unique_ptr<SqliteResultSet> resultSet;
    try {
        resultSet = TraceDatabaseHelper::QuerySystemViewData(stmt, requestParams, GetRealRankId(requestParams.rankId));
    } catch (DatabaseException &e) {
        ServerLog::Error("Query system view data failed, ", e.What());
        return false;
    }
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
        if (responseBody.total == 0) {
            responseBody.total = resultSet->GetInt64(col++);
        }
        responseBody.systemViewDetail.emplace_back(systemViewDetail);
    }
    responseBody.pageSize = requestParams.pageSize;
    responseBody.currentPage = requestParams.current;
    return true;
}

bool DbTraceDataBase::QueryKernelDetailData(const Protocol::KernelDetailsParams &requestParams,
    Protocol::KernelDetailsBody &responseBody, uint64_t minTimestamp)
{
    uint64_t offset = (requestParams.current - 1) * requestParams.pageSize;
    std::string sql = GetKernelDetailSql(requestParams);
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        Server::ServerLog::Error("Fail to prepare sql to query kernel detail data.");
        return false;
    }
    stmt->BindParams(GetRealRankId(requestParams.rankId));
    auto resultSet = stmt->ExecuteQuery(requestParams.pageSize, offset);
    if (resultSet == nullptr) {
        ServerLog::Error("Failed to get result set to query kernel detail data.", stmt->GetErrorMessage());
        return false;
    }
    while (resultSet->Next()) {
        Protocol::KernelDetail detail;
        detail.id = resultSet->GetString("id");
        detail.name = resultSet->GetString("name");
        detail.type = GetStringCacheValue(path, resultSet->GetString("type"));
        detail.acceleratorCore = GetStringCacheValue(path, resultSet->GetString("acceleratorCore"));
        detail.startTime = resultSet->GetInt64("startTime") - minTimestamp;
        detail.duration = resultSet->GetDouble("duration");
        detail.waitTime = resultSet->GetDouble("waitTime");
        detail.blockDim = resultSet->GetInt64("blockDim");
        detail.inputShapes = GetStringCacheValue(path, resultSet->GetString("inputShapes"));
        detail.inputDataTypes = GetStringCacheValue(path, resultSet->GetString("inputDataTypes"));
        detail.inputFormats = GetStringCacheValue(path, resultSet->GetString("inputFormats"));
        detail.outputShapes = GetStringCacheValue(path, resultSet->GetString("outputShapes"));
        detail.outputDataTypes = GetStringCacheValue(path, resultSet->GetString("outputDataTypes"));
        detail.outputFormats = GetStringCacheValue(path, resultSet->GetString("outputFormats"));
        if (responseBody.count == 0) {
            responseBody.count = resultSet->GetInt64("num");
        }
        responseBody.kernelDetails.emplace_back(detail);
    }
    responseBody.pageSize = requestParams.pageSize;
    responseBody.currentPage = requestParams.current;
    const std::vector<std::string> cores = QueryCoreType();
    responseBody.acceleratorCoreList = cores;
    return true;
}

std::string DbTraceDataBase::GetStringCacheValue(const std::string& path, std::string key)
{
    if (stringsCache.count(path) == 0 || stringsCache.at(path).count(key) == 0) {
        return key;
    }
    return stringsCache.at(path)[key];
}

bool DbTraceDataBase::GetKernelDetailFilterSql(std::string& sql, const Protocol::KernelDetailsParams &requestParams)
{
    if (!requestParams.filters.empty()) {
        sql += " WHERE ";
    }
    for (uint64_t index = 0; index < requestParams.filters.size(); index++) {
        std::pair<std::string, std::string> filter = requestParams.filters[index];
        if (!StringUtil::CheckSqlValid(filter.first) || !StringUtil::CheckSqlValid(filter.second)) {
            ServerLog::Error("There is an SQL injection attack on this parameter. param: filter");
            sql.clear();
            return false;
        }
        if (index != 0) {
            sql += " AND ";
        }
        if (filter.first == "name") {
            sql += " lower(" + filter.first + ") LIKE lower('%" + filter.second + "%') ";
        } else {
            sql += filter.first + " IN ("
                   "    SELECT id FROM STRING_IDS WHERE lower(value) LIKE lower('%" + filter.second + "%')"
                   ")";
        }
    }
    return true;
}

std::string DbTraceDataBase::GetKernelDetailSql(const Protocol::KernelDetailsParams &requestParams)
{
    std::string blockDimColumnName = isLowCamel ? "blockDim" : "block_dim";
    std::string sql = "with nameIds as (select id, value as realName from STRING_IDS),\n"
      "     main as ("
      "     select info.ROWID, nameIds.realName as name, substr(realName, 0, instr(realName, '__') + 1) as opType,"
      "       'HCCL' as taskType, info.startNs, round((info.endNs - info.startNs)/1000.0, 3) as duration,\n"
      " 0 as " + blockDimColumnName + ",round(waitNs/1000.0, 3) as wait_time,'N/A' as inputShapes, 'N/A' as "
      " inputDataTypes,'N/A' as inputFormats, 'N/A' as outputShapes, 'N/A' as outputDataTypes, 'N/A' as outputFormats"
      "       from COMMUNICATION_OP info JOIN TASK ON info.connectionId = TASK.connectionId "
      "       join nameIds on opName = nameIds.id group by info.opName\n"
      "     UNION all"
      "     select TASK.ROWID, nameIds.realName as name, opType, info.taskType, startNs,"
      "            round((endNs - startNs)/1000.0, 3) as duration,\n"
      "" + blockDimColumnName + ", round(waitNs/1000.0, 3) as wait_time, inputShapes, inputDataTypes, inputFormats,\n"
      "            outputShapes, outputDataTypes, outputFormats  from COMPUTE_TASK_INFO info "
      "      JOIN TASK ON info.globalTaskId = TASK.globalTaskId join nameIds on name = nameIds.id where deviceId = ?), "
      "    total as (select count(*) as num "
      "    from ("
      "        SELECT name, opType as type, taskType AS acceleratorCore, startNs AS startTime, duration ,\n"
      "        wait_time as waitTime, " + blockDimColumnName + " AS blockDim, inputShapes,\n"
      "        inputDataTypes, inputFormats, outputShapes, outputDataTypes, outputFormats FROM main"
      "    ) subquery ";
    if (!GetKernelDetailFilterSql(sql, requestParams)) {
        return sql;
    }
    sql += " )\n"
      "SELECT ROWID as id, total.num, name, opType as type, taskType AS acceleratorCore, startNs AS startTime,\n"
      "       duration, wait_time as waitTime, " + blockDimColumnName + " AS blockDim, inputShapes,\n"
      "       inputDataTypes, inputFormats, outputShapes, outputDataTypes, outputFormats\n"
      "FROM main join total ";
    if (!GetKernelDetailFilterSql(sql, requestParams)) {
        return sql;
    }

    if (!StringUtil::CheckSqlValid(requestParams.orderBy)) {
        ServerLog::Error("There is an SQL injection attack on this parameter. error param: ", requestParams.orderBy);
    } else if (!requestParams.orderBy.empty() && !requestParams.order.empty()) {
        sql += " ORDER by " + requestParams.orderBy + " " + (requestParams.order == "ascend" ? "ASC" : "DESC");
    }
    sql += " LIMIT ? OFFSET ?";

    return sql;
}

uint64_t DbTraceDataBase::QueryTotalKernel(const Protocol::KernelDetailsParams &requestParams)
{
    std::string sql = "SELECT count(1) as num FROM " + TABLE_COMPUTE_TASK_INFO;
    if (!requestParams.coreType.empty()) {
        sql += " AND accelerator_core = ? ";
    }
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        Server::ServerLog::Error("Fail to prepare sql to query total kernel.");
        return 0;
    }
    if (!requestParams.coreType.empty()) {
        stmt->BindParams(requestParams.coreType);
    }
    auto resultSet = stmt->ExecuteQuery();
    if (resultSet == nullptr) {
        ServerLog::Error("Failed to get result set to query total kernel.", stmt->GetErrorMessage());
        return 0;
    }
    uint64_t total = 0;
    if (resultSet->Next()) {
        total = resultSet->GetInt64("num");
    }
    return total;
}

bool DbTraceDataBase::QueryKernelDepthAndThread(const Protocol::KernelParams &params,
    Protocol::OneKernelBody &responseBody, uint64_t minTimestamp)
{
    // 精度缺失，设置500的浮动区间
    std::string sql = "select info.ROWID as id, groupName||'group' as tid, opName as name, 'HCCL' as pid,"
          " 0 as depth from COMMUNICATION_OP info "
          " where name = (select id from STRING_IDS where value = ?) and abs(startNs - ?) <= 500 "
          " UNION all "
          " select info.ROWID as id, T.streamId as tid, name, 'Ascend Hardware' as pid, depth "
          " from COMPUTE_TASK_INFO info join TASK T on info.globalTaskId = T.globalTaskId "
          " where name = (select id from STRING_IDS where value = ?) and abs(startNs - ?) <= 500";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Fail to prepare sql to query kernel depth and thread.");
        return false;
    }
    std::unique_ptr<SqliteResultSet> resultSet;
    uint64_t timestamp = params.timestamp + minTimestamp;
    resultSet = stmt->ExecuteQuery(params.name, timestamp, params.name, timestamp);
    if (resultSet == nullptr) {
        ServerLog::Error("Failed to get result set to query kernel depth and thread.", stmt->GetErrorMessage());
        return false;
    }
    if (resultSet->Next()) {
        responseBody.id = resultSet->GetString("id");
        responseBody.depth = resultSet->GetInt64("depth");
        responseBody.threadId = resultSet->GetString("tid");
        responseBody.pid = resultSet->GetString("pid");
        responseBody.rankId = QueryHostInfo() + GetRealRankId(params.rankId);
    }
    return true;
}

LayerStatData DbTraceDataBase::QueryLayerData(const std::string &layer, const std::string &name)
{
    return LayerStatData();
}

std::vector<std::string> DbTraceDataBase::QueryCoreType()
{
    std::vector<std::string> acceleratorCoreList;
    std::string sql = "SELECT DISTINCT TASKTYPE.value as accelerator_core FROM " + TABLE_COMPUTE_TASK_INFO +
        " JOIN STRING_IDS AS TASKTYPE ON TASKTYPE.id = COMPUTE_TASK_INFO.taskType ORDER BY accelerator_core";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Fail to prepare sql to query core type.");
        return acceleratorCoreList;
    }
    auto resultSet = stmt->ExecuteQuery();
    if (resultSet == nullptr) {
        ServerLog::Error("Failed to get result set to query core type.", stmt->GetErrorMessage());
        return acceleratorCoreList;
    }
    while (resultSet->Next()) {
        std::string res = resultSet->GetString("accelerator_core");
        acceleratorCoreList.emplace_back(res);
    }
    return acceleratorCoreList;
}

OneKernelData DbTraceDataBase::QueryKernelTid(uint64_t trackId)
{
    return OneKernelData();
}

bool DbTraceDataBase::QueryThreadTracesSummary(const Protocol::UnitThreadTracesSummaryParams &requestParams,
    Protocol::UnitThreadTracesSummaryBody &responseBody, uint64_t minTimestamp)
{
    auto stmt = CreatPreparedStatement();
    if (stmt == nullptr) {
        ServerLog::Error("Failed to prepare sql to query thread traces summary. ", sqlite3_errmsg(db));
        return false;
    }
    std::unique_ptr<SqliteResultSet> resultSet;
    try {
        resultSet = TraceDatabaseHelper::QueryThreadTracesSummary(stmt, requestParams,
                                                                  GetRealRankId(requestParams.cardId), minTimestamp);
    } catch (DatabaseException &e) {
        ServerLog::Error("Query thread traces summary failed, ", e.What());
        return false;
    }
    if (resultSet == nullptr) {
        ServerLog::Error("Failed to get result set to query thread traces summary.", stmt->GetErrorMessage());
        return false;
    }
    uint64_t maxTime = 0;
    while (resultSet->Next()) {
        Protocol::ThreadTracesSummary summary;
        uint64_t endTime = resultSet->GetInt64("end_time");
        if (endTime > maxTime) {
            summary.startTime = resultSet->GetInt64("start_time");
            summary.duration = resultSet->GetInt64("duration");
            responseBody.data.emplace_back(summary);
            maxTime = endTime;
        }
    }
    return true;
}
void DbTraceDataBase::UpdateStartTime(const std::string &fileId)
{
    sqlite3_stmt *stmt = nullptr;
    std::string sql = "SELECT startTimeNs, endTimeNs FROM " + TABLE_SESSION_TIME_INFO;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        Server::ServerLog::Error("Failed to Update Start Time. Cmd: ", sql, " Msg: ", sqlite3_errmsg(db), " ", result);
        sqlite3_finalize(stmt);
        return;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        int64_t startTime = sqlite3_column_int64(stmt, col++);
        int64_t endTime = sqlite3_column_int64(stmt, col++);
        TraceTime::Instance().UpdateTime(startTime, endTime);
        TraceTime::Instance().UpdateCardMinTime(fileId, startTime);
        TraceTime::Instance().UpdateCardMinTime(QueryHostInfo()+"Host", startTime);
    }
    Server::ServerLog::Info("Update start and end time. ");
    sqlite3_finalize(stmt);
}

void DbTraceDataBase::GenerateOverlapAnalysis()
{
    if (!CheckTableExist(TABLE_OVERLAP_ANALYSIS)) {
        Server::ServerLog::Error("GenerateOverlapAnalysis:Table OVERLAP_ANALYSIS is not exist.");
        return;
    }
    if (CheckValueFromStatusInfoTable(OVERLAP_ANALYSIS_STATUS, FINISH_STATUS)) {
        Server::ServerLog::Info("GenerateOverlapAnalysis already finish. skip");
        return;
    }
    {
        std::unique_lock<std::recursive_mutex> lockGuard(mutex);
        ExecSql("delete from OVERLAP_ANALYSIS where 1 = 1");
    }
    QueryRankId();
    for (const auto &rankId: rankIds) {
        std::vector<OVERLAP_INFO> timeInfoList; // 包含computing,Communication 覆盖数据
        QueryTaskTimeInfo(true, timeInfoList, rankId);
        QueryTaskTimeInfo(false, timeInfoList, rankId);
        if (timeInfoList.empty()) {
            continue;
        }
        std::sort(timeInfoList.begin(), timeInfoList.end(), std::less<OVERLAP_INFO>());
        std::vector<OVERLAP_INFO> overlapInfoList;
        OVERLAP_INFO curBlock = OVERLAP_INFO(timeInfoList.begin()->startNs, timeInfoList.begin()->endNs,
                                             timeInfoList.begin()->type); // 记录当前最大截结束时间对应的覆盖区块
        for (const auto &timeInfo: timeInfoList) {
            if (curBlock.type == 1) { // Communication = 1
                overlapInfoList.emplace_back(curBlock.startNs,  // Communication(Not Overlapped) = 2
                                             timeInfo.startNs > curBlock.endNs ? curBlock.endNs : timeInfo.startNs, 2);
            }
            if (timeInfo.startNs > curBlock.endNs) {
                overlapInfoList.emplace_back(curBlock.endNs, timeInfo.startNs, 3); // Free = 3
                curBlock.endNs = timeInfo.endNs;
                curBlock.type = timeInfo.type;
                curBlock.startNs = timeInfo.startNs;
            } else {
                curBlock.type = timeInfo.endNs > curBlock.endNs ? timeInfo.type : curBlock.type;
                curBlock.startNs = timeInfo.endNs > curBlock.endNs ? curBlock.endNs : timeInfo.endNs;
                curBlock.endNs = timeInfo.endNs > curBlock.endNs ? timeInfo.endNs : curBlock.endNs;
            }
        }
        if (InsertOverlapAnalysisInfo(timeInfoList, rankId) && InsertOverlapAnalysisInfo(overlapInfoList, rankId)) {
            Server::ServerLog::Info("GenerateOverlapAnalysis success");
        } else {
            Server::ServerLog::Error("GenerateOverlapAnalysis fail");
            return;
        }
    }
    UpdateValueIntoStatusInfoTable(OVERLAP_ANALYSIS_STATUS, FINISH_STATUS);
}

bool DbTraceDataBase::InsertOverlapAnalysisInfo(const std::vector<OVERLAP_INFO> &overlapInfoList,
                                                const std::string &rankId)
{
    std::lock_guard<std::recursive_mutex> lockGuard(mutex);
    int64_t size = overlapInfoList.size();
    int64_t count = size / cacheSize;
    bool result = true;
    for (int64_t index = 0; index <= count; ++index) {
        int64_t start = index * cacheSize;
        int64_t length = cacheSize;
        if (size - start < cacheSize) {
            length = size - start;
        }
        if (!StartTransaction()) {
            ServerLog::Error("Failed to start Transaction.");
            return false;
        }
        for (int64_t tmpIndex = start; tmpIndex < start + length; tmpIndex++) {
            insertOverlapStmt->Reset();
            insertOverlapStmt->BindParams(rankId, overlapInfoList[tmpIndex].startNs,
                                          overlapInfoList[tmpIndex].endNs, overlapInfoList[tmpIndex].type);
            if (!insertOverlapStmt->Execute()) {
                ServerLog::Error("Failed to InsertOverlap");
                result = false;
                break;
            }
        }
        if (!EndTransaction()) {
            ServerLog::Error("Failed to end Transaction.");
            return false;
        }
    }
    return result;
}

void DbTraceDataBase::QueryTaskTimeInfo(bool isComputing, std::vector<OVERLAP_INFO> &timeInfoList,
                                        const std::string &rankId)
{
    std::string sql;
    if (isComputing) {
        sql = "select startNs, endNs from TASK main join COMPUTE_TASK_INFO info "
              " on info.globalTaskId = main.globalTaskId where deviceId=? and startNs != endNs order by startNs, endNs";
    } else {
        sql = "select op.startNs, op.endNs from COMMUNICATION_OP op join TASK task "
              " on task.connectionId = op.connectionId where deviceId=? group by opId  order by op.startNs, op.endNs";
    }
    auto stmt = CreatPreparedStatement();
    try {
        auto resultSet = TraceDatabaseHelper::ExecuteQuery(stmt, sql, rankId);
        OVERLAP_INFO curInfo {};
        bool hasCurInfo = false;
        while (resultSet->Next()) { // Computing = 0, Communication = 1
            auto info = OVERLAP_INFO(resultSet->GetInt64("startNs"), resultSet->GetInt64("endNs"), isComputing ? 0 : 1);
            if (!hasCurInfo) {
                curInfo = info;
                hasCurInfo = true;
            } else if (info.startNs <= curInfo.endNs) {
                curInfo.endNs = info.endNs;
            } else {
                timeInfoList.emplace_back(curInfo);
                curInfo = info;
            }
        }
        if (hasCurInfo) {
            timeInfoList.emplace_back(curInfo);
        }
    } catch (DatabaseException &e) {
        ServerLog::Error("QueryTaskTimeInfo Fail, ", e.What());
        return;
    }
}

void DbTraceDataBase::UpdateWaitTime()
{
    if (!CheckTableExist(TABLE_COMPUTE_TASK_INFO) || !CheckTableExist(TABLE_COMMUNICATION_OP) ||
        !CheckTableDataInvalid(TABLE_TASK)) {
        Server::ServerLog::Error("UpdateWaitTime:Table is not exist.");
        return;
    }
    if (CheckValueFromStatusInfoTable(WAIT_TIME_STATUS, FINISH_STATUS)) { // 已更新数据，跳过更新
        return;
    }
    auto stmt = CreatPreparedStatement(FULL_DB_UPDATE_TIME); // 查询数据
    auto updateComputeStmt = CreatPreparedStatement("UPDATE COMPUTE_TASK_INFO SET waitNs = ? WHERE ROWID = ?;");
    auto updateCommunicationStmt = CreatPreparedStatement("UPDATE COMMUNICATION_OP SET waitNs = ? WHERE ROWID = ?;");
    if (stmt == nullptr || updateComputeStmt == nullptr || updateCommunicationStmt == nullptr) {
        ServerLog::Error("UpdateWaitTime, fail to prepare sql.");
        return;
    }
    auto resultSet = stmt->ExecuteQuery();
    if (resultSet == nullptr) {
        ServerLog::Error("UpdateWaitTime. Failed to get result set.", stmt->GetErrorMessage());
        return;
    }
    std::map<int32_t, int64_t> prevTime;
    std::lock_guard<std::recursive_mutex> lockGuard(mutex);
    while (resultSet->Next()) {
        std::string type = resultSet->GetString("type");
        int64_t startNs = resultSet->GetInt64("startNs");
        int64_t endNs = resultSet->GetInt64("endNs");
        int64_t deviceId = resultSet->GetInt32("deviceId");
        if (prevTime.find(deviceId) == prevTime.end()) {
            prevTime[deviceId] = startNs;
        }
        int64_t waitNs = startNs > prevTime[deviceId] ? startNs - prevTime[deviceId] : 0;
        prevTime[deviceId] = endNs;
        WAIT_TIME task;
        task.id = resultSet->GetInt64("id");
        task.waitTime = waitNs;
        task.type = type;
        taskWaitTimeCache.push_back(task);
        if (taskWaitTimeCache.size() == cacheSize &&
            !UpdateTaskInfoWaitTime(updateComputeStmt, updateCommunicationStmt)) {
            ServerLog::Error("UpdateWaitTime. Failed to update data.");
            return;
        }
    }
    if (!UpdateTaskInfoWaitTime(updateComputeStmt, updateCommunicationStmt)) {
        ServerLog::Error("UpdateWaitTime. Failed to update last data.");
        return;
    }
    UpdateValueIntoStatusInfoTable(WAIT_TIME_STATUS, FINISH_STATUS);
}

bool DbTraceDataBase::UpdateTaskInfoWaitTime(std::unique_ptr<SqlitePreparedStatement> &updateComputeStmt,
                                             std::unique_ptr<SqlitePreparedStatement> &updateCommunicationStmt)
{
    std::lock_guard<std::recursive_mutex> lockGuard(mutex);
    if (!StartTransaction()) {
        ServerLog::Error("Failed to start Transaction.");
        return false;
    }
    auto result = true;
    for (const auto &item : taskWaitTimeCache) {
        std::unique_ptr<SqlitePreparedStatement> &refStmt = item.type == "compute" ?
                updateComputeStmt : updateCommunicationStmt;
        refStmt->Reset();
        refStmt->BindParams(item.waitTime, item.id);
        if (!refStmt->Execute()) {
            ServerLog::Error("Failed to UpdateTaskInfoWaitTime");
            result = false;
            break;
        }
    }
    taskWaitTimeCache.clear();
    if (!EndTransaction()) {
        ServerLog::Error("Failed to end UpdateTaskInfoWaitTime.");
        return false;
    }
    return result;
}

std::string DbTraceDataBase::GetRealRankId(const std::string &fileId)
{
    auto hostStr = QueryHostInfo();
    if (hostStr.empty() || !StringUtil::StartWith(fileId, hostStr)) {
        return fileId;
    }
    return fileId.substr(hostStr.length());
}

std::string DbTraceDataBase::QueryHostInfo()
{
    if (!host.empty() || !CheckTableDataInvalid(TABLE_HOST_INFO)) {
        return host;
    }
    std::string sql = "select hostName||hostUid||'' as host from " + TABLE_HOST_INFO;
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        sqlite3_finalize(stmt);
        return host;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        host = sqlite3_column_string(stmt, resultStartIndex);
    }
    sqlite3_finalize(stmt);

    sqlite3_stmt *timeStmt = nullptr;
    std::string timeSql = "SELECT startTimeNs, endTimeNs FROM " + TABLE_SESSION_TIME_INFO;
    int timeResult = sqlite3_prepare_v2(db, timeSql.c_str(), -1, &timeStmt, nullptr);
    if (timeResult != SQLITE_OK) {
        Server::ServerLog::Error(" Msg: ", sqlite3_errmsg(db), " ", result);
        sqlite3_finalize(timeStmt);
        return host;
    }
    int64_t startTime = 0;
    int64_t endTime = 0;
    while (sqlite3_step(timeStmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        startTime = sqlite3_column_int64(timeStmt, col++);
        endTime = sqlite3_column_int64(timeStmt, col++);
    }
    sqlite3_finalize(timeStmt);
    host = CollectionTimeService::Instance().ComputeMarkHost(host, startTime, endTime);
    return host;
}

std::vector<std::string> DbTraceDataBase::QueryRankId()
{
    if (!rankIds.empty()) {
        return rankIds;
    }
    sqlite3_stmt *stmt = nullptr;
    std::string sql;
    FileType type = DataBaseManager::Instance().GetFileType();
    if (type == FileType::MS_PROF) {
        sql = "SELECT id FROM " + TABLE_NPU_INFO;
    } else if (type == FileType::PYTORCH) {
        if (CheckTableDataInvalid(TABLE_PYTORCH_INFO)) {
            std::string columnNames = isLowCamel ? "rankId" : "rank_id";
            sql = "SELECT DISTINCT " + columnNames + " FROM " + TABLE_PYTORCH_INFO;
        } else {
            sql = "SELECT DISTINCT id FROM " + TABLE_NPU_INFO;
        }
    }
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        Server::ServerLog::Error("Failed to get Statistic Num. Cmd: ", sql, " Msg: ", sqlite3_errmsg(db), " ", result);
        sqlite3_finalize(stmt);
        return rankIds;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string id = sqlite3_column_string(stmt, resultStartIndex);
        rankIds.emplace_back(id);
    }
    sqlite3_finalize(stmt);
    return rankIds;
}

bool DbTraceDataBase::CheckTableDataInvalid(std::string tableName)
{
    if (!CheckTableExist(tableName)) {
        return false;
    }
    sqlite3_stmt *stmt = nullptr;
    std::string sql = "  SELECT COUNT(*) FROM " + tableName;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        Server::ServerLog::Error("Failed to get Memory Data. Cmd: ", sql, " Msg: ", sqlite3_errmsg(db), " ", result);
        sqlite3_finalize(stmt);
        return false;
    }
    int64_t count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int64(stmt, resultStartIndex);
    }
    sqlite3_finalize(stmt);
    return count != 0;
}

bool DbTraceDataBase::NeedUpdateDepth(const std::string &table)
{
    std::string sql = "select count(1) from " + table + " where depth is null ";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Failed prepare sql. ");
        return false;
    }
    auto resultSet = stmt->ExecuteQuery();
    if (resultSet == nullptr) {
        ServerLog::Error("NeedUpdateDepth. Failed to get result set.", stmt->GetErrorMessage());
        return false;
    }
    return resultSet->GetErrorCode() == SQLITE_OK && resultSet->Next() && resultSet->GetInt64(resultStartIndex) > 0;
}

void DbTraceDataBase::UpdateAllDepth()
{
    std::string sql = "select format('%s-%s', deviceId, streamId) as key, ROWID as id,startNs, endNs from TASK "
        "                      order by deviceId, streamId, startNs, globalTaskId;";
    if (CheckTableExist(TABLE_TASK) && NeedUpdateDepth(TABLE_TASK)) {
        UpdateDepth(sql, updateTaskDepthStmt);
    }

    sql = "select format('%s-', globalTid) as key, startNs, endNs, ROWID as id from " + TABLE_API +
        " order by globalTid, startNs;";
    if (isExistPytorch && NeedUpdateDepth(TABLE_API)) {
        UpdateDepth(sql, updateApiDepthStmt);
    }

    sql = "select format('%s-%s', globalTid, type) as key, startNs, endNs, ROWID as id from " + TABLE_CANN_API +
        " order by globalTid, type, startNs;";
    if (isExistCann && NeedUpdateDepth(TABLE_CANN_API)) {
        UpdateDepth(sql, updateCannApiDepthStmt);
    }

    sql = "select format('%s-%s', globalTid, eventType) as key, startNs, endNs, ROWID as id from " + TABLE_MSTX_EVENTS +
          " order by globalTid, eventType, startNs;";
    if (isExistMstx && NeedUpdateDepth(TABLE_MSTX_EVENTS)) {
        auto stmt = CreatPreparedStatement("UPDATE " + TABLE_MSTX_EVENTS + " set depth = ? where ROWID = ?");
        UpdateDepth(sql, stmt);
    }
}

void DbTraceDataBase::UpdateDepth(const std::string &sql, std::unique_ptr<SqlitePreparedStatement> &updateStmt)
{
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr || updateStmt == nullptr) {
        ServerLog::Error("UpdateDepth. Failed to prepare sql.", sqlite3_errmsg(db));
        return;
    }
    auto resultSet = stmt->ExecuteQuery();
    if (resultSet == nullptr) {
        ServerLog::Error("UpdateDepth. Failed to get result set.", stmt->GetErrorMessage());
        return;
    }
    std::map<std::string, std::vector<TASK_INFO>> data;
    while (resultSet->Next()) {
        auto key = resultSet->GetString("key");
        if (data.find(key) == data.end()) {
            data[key] = std::vector<TASK_INFO>();
        }
        TASK_INFO task;
        task.start = resultSet->GetUint64("startNs");
        task.end = resultSet->GetUint64("endNs");
        task.id = resultSet->GetUint64("id");
        data[key].push_back(task);
    }
    std::vector<uint64_t> endList;
    std::lock_guard<std::recursive_mutex> lockGuard(mutex);
    for (auto &deviceData : data) {
        for (auto &task : deviceData.second) {
            for (task.depth = 0; task.depth < endList.size() && endList[task.depth] > task.start; task.depth++) {
            }
            if (task.depth >= endList.size()) {
                endList.emplace_back(task.end);
            } else {
                endList[task.depth] = task.end;
            }
            taskDepthCache.emplace_back(task);
            if (taskDepthCache.size() == cacheSize && !UpdateDepthList(updateStmt)) {
                ServerLog::Error("UpdateDepth. Failed to update data.");
                return;
            }
        }
        endList.clear();
    }
    if (!UpdateDepthList(updateStmt)) {
        ServerLog::Error("UpdateDepth. Failed to update last data.");
    }
}

bool DbTraceDataBase::UpdateDepthList(std::unique_ptr<SqlitePreparedStatement> &stmt)
{
    std::lock_guard<std::recursive_mutex> lockGuard(mutex);
    if (!StartTransaction()) {
        ServerLog::Error("Failed to start Transaction.");
        return false;
    }
    bool result = true;
    for (const auto &item : taskDepthCache) {
        stmt->Reset();
        stmt->BindParams(item.depth, item.id);
        if (!stmt->Execute()) {
            ServerLog::Error("Failed to updateDepth");
            result = false;
            break;
        }
    }
    taskDepthCache.clear();
    if (!EndTransaction()) {
        ServerLog::Error("Failed to end Transaction.");
        return false;
    }
    return result;
}

bool DbTraceDataBase::OpenDb(const std::string &dbPath, bool clearAllTable)
{
    return Database::OpenDb(dbPath, clearAllTable) && GetMetaVersion() && SetConfig() && InitStmt();
}

void DbTraceDataBase::InitFlowCache()
{
    {
        std::lock_guard<std::recursive_mutex> lockGuard(mutex);
        if (!ExecSql("CREATE TEMPORARY TABLE IF NOT EXISTS connectionCats as " + DbSqlDefs::GetConnectionCatSql())) {
            return;
        }
    }
    std::map<std::string, std::map<std::string, FlowLocation>> startFlowLocations; // 连线起始节点
    std::map<std::string, std::map<std::string, std::vector<FlowLocation>>> endFlowLocations; // 连线结束节点
    QueryFlowLocation(DbSqlDefs::GetQueryApiLocationSql(), startFlowLocations, endFlowLocations);
    QueryFlowLocation(QUERY_DEVICE_LOCATION_SQL, startFlowLocations, endFlowLocations);
    auto dealLineData = [](const std::string &cat,  const std::string &connectionId, const FlowLocation &startLocation,
            std::map<std::string, std::map<std::string, std::vector<FlowLocation>>>& endFlowLocations) {
        auto endLocations = endFlowLocations[cat][connectionId];
        for (const auto &endFlowLocation : endLocations) {
            UnitSingleFlow singleFlow {.cat = cat, .id = connectionId,
                    .from = startLocation, .to = endFlowLocation };
            CommonCacheManager::Instance().Put(startLocation.deviceId, cat, singleFlow);
        }
    };
    for (const auto &catGroup: startFlowLocations) {
        for (const auto &startLocation: catGroup.second) {
            dealLineData(catGroup.first, startLocation.first, startLocation.second, endFlowLocations);
        }
    }
    for (const auto &rankId: rankIds) {
        CommonCacheManager::Instance().SetFlowState(QueryHostInfo() + rankId, true);
    }
}

void DbTraceDataBase::QueryFlowLocation(const std::string& sql,
    std::map<std::string, std::map<std::string, FlowLocation>>& startFlowLocations,
    std::map<std::string, std::map<std::string, std::vector<FlowLocation>>>& endFlowLocations)
{
    auto stmt = CreatPreparedStatement();
    auto resultSet = TraceDatabaseHelper::ExecuteQuery(stmt, sql, TraceTime::Instance().GetStartTime());
    while (resultSet->Next()) {
        auto metaType = resultSet->GetString("metaType");
        auto deviceId = QueryHostInfo() + resultSet->GetString("deviceId");
        auto cat = resultSet->GetString("cat");
        auto connectionId = resultSet->GetString("connectionId");
        auto rankId = metaType == TABLE_API || metaType == TABLE_CANN_API || metaType == TABLE_MSTX_EVENTS ?
                path : deviceId;
        FlowLocation location {.tid = resultSet->GetString("tid"), .id = resultSet->GetString("id"),
                .metaType = metaType, .rankId = rankId,
                .depth = resultSet->GetInt32("depth"), .timestamp = resultSet->GetUint64("startTime"),
                .duration = resultSet->GetUint64("duration"), .pid = resultSet->GetString("pid"),
                .name = resultSet->GetString("name"), .deviceId=deviceId};
        if (startFlowLocations[cat].count(connectionId) == 0) {
            startFlowLocations[cat][connectionId] = location;
        } else if (startFlowLocations[cat][connectionId].timestamp > location.timestamp) {
            endFlowLocations[cat][connectionId].emplace_back(startFlowLocations[cat][connectionId]);
            startFlowLocations[cat][connectionId] = location;
        } else {
            endFlowLocations[cat][connectionId].emplace_back(location);
        }
    }
}

void DbTraceDataBase::InitStringsCache()
{
    if (!stringsCache[path].empty()) {
        return;
    }
    auto sql = "select id, value from STRING_IDS";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("InitStringsCache. Failed to prepare sql.", sqlite3_errmsg(db));
        return;
    }
    auto result = stmt->ExecuteQuery();
    if (result == nullptr) {
        ServerLog::Error("InitStringsCache. Failed to get result set.", stmt->GetErrorMessage());
        return;
    }
    while (result->Next()) {
        stringsCache[path].emplace(result->GetString("id"), result->GetString("value"));
    }
}

bool DbTraceDataBase::InitStmt()
{
    if (initStmt) {
        return true;
    }
    initStmt = true;
    std::string sql;
    bool prepareUpdateStmtSuccess = true;
    if (CheckTableExist(TABLE_TASK)) {
        sql = "UPDATE " + TABLE_TASK + " set depth = ? where ROWID = ?";
        updateTaskDepthStmt = CreatPreparedStatement(sql);
        sql = "INSERT INTO " + TABLE_OVERLAP_ANALYSIS + " (deviceId, startNs, endNs, type) VALUES (?,?,?,?)";
        insertOverlapStmt = CreatPreparedStatement(sql);
        prepareUpdateStmtSuccess = prepareUpdateStmtSuccess && updateTaskDepthStmt != nullptr &&
                insertOverlapStmt != nullptr;
    }
    if (CheckTableExist(TABLE_API)) {
        sql = "UPDATE " + TABLE_API + " set depth = ? where ROWID = ?";
        updateApiDepthStmt = CreatPreparedStatement(sql);
        prepareUpdateStmtSuccess = prepareUpdateStmtSuccess && updateApiDepthStmt != nullptr;
    }
    if (CheckTableExist(TABLE_CANN_API)) {
        sql = "UPDATE " + TABLE_CANN_API + " set depth = ? where ROWID = ?";
        updateCannApiDepthStmt = CreatPreparedStatement(sql);
        prepareUpdateStmtSuccess = prepareUpdateStmtSuccess && updateCannApiDepthStmt != nullptr;
    }
    if (!prepareUpdateStmtSuccess) {
        ServerLog::Error("Failed to prepare update statement.");
        return false;
    }
    return true;
}

bool DbTraceDataBase::SetConfig()
{
    if (!isOpen) {
        ServerLog::Error("Failed to set config. Database is not open.");
        return false;
    }

    std::lock_guard<std::recursive_mutex> lock(mutex);
    isExistPytorch = CheckTableExist(TABLE_API);
    isExistCann = CheckTableExist(TABLE_CANN_API);
    isExistMstx = CheckTableExist(TABLE_MSTX_EVENTS);
    // 初始化所有全量查询功能需要的表，空表不影响展示，方便sql扩展
    for (const auto &item: FULL_DB_TABLE_MAP) {
        if (!CheckTableExist(item.first)) {
            ExecSql(item.second);
        }
    }
    QueryRankId();

    if (IsDatabaseVersionChange()) {
        UpdateValueIntoStatusInfoTable(CONFIG_STATUS, NOT_FINISH_STATUS);
        UpdateValueIntoStatusInfoTable(OVERLAP_ANALYSIS_STATUS, NOT_FINISH_STATUS);
        UpdateValueIntoStatusInfoTable(WAIT_TIME_STATUS, NOT_FINISH_STATUS);
    }

    if (!CheckValueFromStatusInfoTable(CONFIG_STATUS, FINISH_STATUS)) {
        if (CheckTableExist(TABLE_TASK)) {
            ExecSql("alter table TASK add depth integer;");
            ExecSql(" create table if not exists OVERLAP_ANALYSIS (id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    " deviceId integer, startNs integer, endNs integer, type integer);");
        }
        if (isExistPytorch) {
            ExecSql("alter table PYTORCH_API add depth integer;");
        }
        if (isExistCann) {
            ExecSql("alter table CANN_API add depth integer;");
        }
        if (isExistMstx) {
            ExecSql("alter table " + TABLE_MSTX_EVENTS + " add depth integer;");
        }
        if (CheckTableExist(TABLE_COMPUTE_TASK_INFO)) {
            ExecSql("alter table COMPUTE_TASK_INFO add column waitNs INTEGER;");
        }
        if (CheckTableExist(TABLE_COMMUNICATION_OP)) {
            ExecSql("alter table COMMUNICATION_OP add column waitNs INTEGER;");
        }
        UpdateValueIntoStatusInfoTable(CONFIG_STATUS, FINISH_STATUS);
    }
    return ExecSql("PRAGMA synchronous = OFF; PRAGMA case_sensitive_like=1; PRAGMA journal_mode = MEMORY;"
                   " PRAGMA user_version = " + GetDataBaseVersion() + ";");
}

bool DbTraceDataBase::QueryHostMetadata(std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData)
{
    PROCESS_TYPE types[] = {PROCESS_TYPE::CANN_API, PROCESS_TYPE::API, PROCESS_TYPE::MS_TX};
    std::map<std::string, std::vector<MetaDataDto>> threadMap;
    for (const auto &type : types) {
        auto typeName = ENUM_TO_STR(type).value_or("");
        std::string sql;
        switch (type) {
            case PROCESS_TYPE::CANN_API:
                sql = " select EAL.name, globalTid, type, max(depth) as maxDepth from " + typeName +
                    " a join ENUM_API_TYPE EAL on a.type = EAL.id "
                    " group by type, globalTid order by globalTid, type desc";
                break;
            case PROCESS_TYPE::API:
                sql = " select 'pytorch' as name, globalTid, 'pytorch' as type,"
                    " max(depth) as maxDepth from " +
                    typeName + " a group by globalTid order by globalTid";
                break;
            case PROCESS_TYPE::MS_TX:
                sql = "select 'MsTx' as name, globalTid,'MsTx' as type, max(depth) as maxDepth from MSTX_EVENTS a "
                      " group by globalTid order by globalTid";
                break;
            default:
                return false;
        }
        auto stmt = CreatPreparedStatement();
        try {
            auto resultSet = TraceDatabaseHelper::ExecuteQuery(stmt, sql);
            while (resultSet->Next()) {
                MetaDataDto metadata;
                metadata.pid = resultSet->GetString("globalTid");
                metadata.metaType = typeName;
                metadata.threadId = resultSet->GetString("type");
                metadata.threadName = resultSet->GetString("name");
                metadata.maxDepth = resultSet->GetInt32("maxDepth") + 1;
                threadMap[metadata.pid].emplace_back(metadata);
            }
        } catch (DatabaseException &e) {
            ServerLog::Error("QueryHostMetadata, MetaType: ", typeName, " reason: ", e.What());
            return false;
        }
    }
    DealHostMetadata(metaData, threadMap);
    return true;
}

void DbTraceDataBase::DealHostMetadata(std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData,
                                       std::map<std::string, std::vector<MetaDataDto>> &threadMap)
{
    int64_t curPid = 0;
    std::unique_ptr<UnitTrack> process;
    for (auto &thread : threadMap) {
        auto globalTid = atoll(thread.first.c_str());
        auto pid = globalTid >> 32;
        auto tid = globalTid & 0XFFFFFFFF;
        if (curPid != pid) {
            if (process.operator bool()) {
                metaData.emplace_back(std::move(process));
            }
            process = GenerateBaseUnitTrack("process", path, std::to_string(pid), "process " + std::to_string(pid),
                ENUM_TO_STR(PROCESS_TYPE::CANN_API).value());
            curPid = pid;
        }
        auto threadUnit = GenerateBaseUnitTrack("process", path, thread.first, "Thread " + std::to_string(tid),
            ENUM_TO_STR(PROCESS_TYPE::CANN_API).value());
        threadUnit->metaData.threadId = std::to_string(tid);
        auto cannApiUnit =
            GenerateBaseUnitTrack("label", path, thread.first, "CANN", ENUM_TO_STR(PROCESS_TYPE::CANN_API).value());
        for (const auto &item : thread.second) {
            auto level = GenerateBaseUnitTrack("thread", path, thread.first, "", item.metaType);
            level->metaData.threadId = item.threadId;
            level->metaData.threadName = item.threadName;
            level->metaData.maxDepth = item.maxDepth;
            if (std::find(CANN_APIS.begin(), CANN_APIS.end(), item.threadName) != CANN_APIS.end()) {
                cannApiUnit->children.emplace_back(std::move(level));
            } else {
                threadUnit->children.emplace_back(std::move(level));
            }
        }
        if (!cannApiUnit->children.empty()) {
            threadUnit->children.emplace_back(std::move(cannApiUnit));
        }
        process->children.emplace_back(std::move(threadUnit));
    }
    if (process.operator bool()) {
        metaData.emplace_back(std::move(process));
    }
}

std::unique_ptr<Protocol::UnitTrack> DbTraceDataBase::GenerateBaseUnitTrack(const std::string &type,
    const std::string &cardId, const std::string &processId, const std::string &processName,
    const std::string &metaType)
{
    std::unique_ptr<Protocol::UnitTrack> unitTrack = std::make_unique<Protocol::UnitTrack>();
    unitTrack->type = type;
    unitTrack->metaData.cardId = cardId;
    unitTrack->metaData.processId = processId;
    unitTrack->metaData.processName = processName;
    unitTrack->metaData.metaType = metaType;
    return unitTrack;
}

bool DbTraceDataBase::QueryOperateMetadata(const std::string &fileId,
    std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData)
{
    PROCESS_TYPE types[] = {PROCESS_TYPE::ASCEND_HARDWARE, PROCESS_TYPE::HCCL};
    for (const auto &type : types) {
        std::string sql;
        switch (type) {
            case PROCESS_TYPE::ASCEND_HARDWARE:
                sql = "select streamId as tid, max(depth) as maxDepth,'Stream '||streamId as name from " + TABLE_TASK +
                      " where deviceId = ? group by streamId";
                break;
            case PROCESS_TYPE::HCCL:
                sql = "with main as (select planeId, op.groupName from " + TABLE_COMMUNICATION_TASK_INFO +
                      " info join " + TABLE_TASK + " task on task.globalTaskId = info.globalTaskId "
                      " join COMMUNICATION_OP op on op.opId = info.opId where deviceId = ?) "
                      " select 'Plane ' || planeId as name, planeId as tid, 0 as maxDepth  from main group by planeId "
                      " union select 'Group ' || row_number() over () || ' Communication' as name, "
                      " groupName||'group' as tid, 0 as maxDepth from main group by groupName";
                break;
            default:
                break;
        }
        auto stmt = CreatPreparedStatement();
        auto metaType = ENUM_TO_STR(type).value_or("");
        auto process = GenerateBaseUnitTrack("process", fileId, metaType, metaType, metaType);
        try {
            auto resultSet = TraceDatabaseHelper::ExecuteQuery(stmt, sql, GetRealRankId(fileId));
            while (resultSet->Next()) {
                auto thread = GenerateBaseUnitTrack("thread", fileId, process->metaData.processId, "", metaType);
                thread->metaData.threadId = resultSet->GetString("tid");
                thread->metaData.threadName = resultSet->GetString("name");
                thread->metaData.maxDepth = resultSet->GetInt32("maxDepth") + 1;
                process->children.emplace_back(std::move(thread));
            }
        } catch (DatabaseException &e) {
            ServerLog::Error("QueryOperateMetadata, MetaType: ", metaType, " reason: ", e.What());
            return false;
        }
        if (!process->children.empty()) {
            metaData.emplace_back(std::move(process));
        }
    }
    return true;
}

bool DbTraceDataBase::QueryCounterMetadata(const std::string &fileId,
    std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData)
{
    PROCESS_TYPE types[] = {PROCESS_TYPE::HBM, PROCESS_TYPE::LLC, PROCESS_TYPE::SAMPLE_PMU,
                            PROCESS_TYPE::NIC, PROCESS_TYPE::ROCE, PROCESS_TYPE::ROH};
    for (const auto &type : types) {
        auto tableName = ENUM_TO_STR(type).value_or("");
        if (!CheckTableExist(tableName)) {
            ServerLog::Info("QueryCounterMetadata failed, table ", tableName, " Not Exist.");
            continue;
        }
        std::string sql;
        switch (type) {
            case PROCESS_TYPE::HBM:
                sql = StringUtil::ReplaceFirst(HBM_MEAT_DATA_SQL, "#", tableName);
                break;
            case PROCESS_TYPE::LLC:
                sql = StringUtil::ReplaceFirst(LLC_MEAT_DATA_SQL, "#", tableName);
                break;
            case PROCESS_TYPE::SAMPLE_PMU:
                sql = StringUtil::ReplaceFirst(SAMPLE_PMU_MEAT_DATA_SQL, "#", tableName);
                break;
            case PROCESS_TYPE::ROCE:
            case PROCESS_TYPE::ROH:
            case PROCESS_TYPE::NIC:
                sql = StringUtil::ReplaceFirst(NIC_MEAT_DATA_SQL, "#", tableName);
                break;
            default:
                break;
        }
        auto counter = GenerateBaseUnitTrack("label", fileId, tableName, tableName, tableName);
        auto stmt = CreatPreparedStatement(sql);
        if (stmt == nullptr) {
            ServerLog::Error("QueryCounterMetadata failed!.");
            return false;
        }
        auto resultSet = stmt->ExecuteQuery(GetRealRankId(fileId));
        if (resultSet == nullptr) {
            ServerLog::Error("QueryCounterMetadata. Failed to get result set.", stmt->GetErrorMessage());
            return false;
        }
        while (resultSet->Next()) {
            auto thread = GenerateBaseUnitTrack("counter", fileId, tableName, "", tableName);
            thread->metaData.threadId = resultSet->GetString("name");
            thread->metaData.threadName = thread->metaData.threadId;
            thread->metaData.dataType = StringUtil::Split(resultSet->GetString("types"), ",");
            counter->children.emplace_back(std::move(thread));
        }
        metaData.emplace_back(std::move(counter));
    }
    return true;
}

void DbTraceDataBase::GenerateCounterMetadata(const std::string &fileId,
    std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData)
{
    PROCESS_TYPE types[] = {PROCESS_TYPE::ACC_PMU, PROCESS_TYPE::DDR, PROCESS_TYPE::STARS_SOC,
                            PROCESS_TYPE::NPU_MEM, PROCESS_TYPE::HCCS, PROCESS_TYPE::PCIE, PROCESS_TYPE::AI_CORE};
    for (const auto &type : types) {
        auto typeName = ENUM_TO_STR(type).value_or("");
        if (!CheckTableExist(typeName)) {
            ServerLog::Info("GenerateCounterMetadata failed, table ", typeName, " Not Exist.");
            continue;
        }
        auto counter = GenerateBaseUnitTrack("label", fileId, typeName, typeName, typeName);
        std::vector<std::string> units;
        std::vector<std::vector<std::string>> dataTypes;
        GetCounterUnitsAndDataTypes(type, units, dataTypes, counter);
        for (size_t index = 0; index < units.size(); index++) {
            auto thread = GenerateBaseUnitTrack("counter", fileId, typeName, units.at(index), typeName);
            thread->metaData.threadName = units.at(index);
            thread->metaData.threadId = units.at(index);
            auto dataType = dataTypes.size() == 1 ? dataTypes[0] : dataTypes[index];
            thread->metaData.dataType.insert(thread->metaData.dataType.end(), dataType.begin(), dataType.end());
            counter->children.emplace_back(std::move(thread));
        }
        metaData.emplace_back(std::move(counter));
    }
}

void DbTraceDataBase::GetCounterUnitsAndDataTypes(PROCESS_TYPE type, std::vector<std::string> &units,
    std::vector<std::vector<std::string>> &dataTypes, std::unique_ptr<Protocol::UnitTrack> &counter)
{
    switch (type) { // 此处type调用场景固定，不会出现特殊情况
        case PROCESS_TYPE::ACC_PMU:
            units = { "readBwLevel", "readOstLevel", "writeBwLevel", "writeOstLevel"};
            dataTypes = { { "value", "acc_id" } };
            break;
        case PROCESS_TYPE::DDR:
            units = { "Read", "Write" };
            dataTypes = { { "Read(B/s)" }, { "Write(B/s)" } };
            break;
        case PROCESS_TYPE::STARS_SOC:
            counter->metaData.processName = "Stars Soc";
            units = {"L2 Buffer Bw Level", "Mata Bw Level"};
            dataTypes = {{"L2 Buffer Bw Level"}, {"Mata Bw Level"}};
            break;
        case PROCESS_TYPE::NPU_MEM:
            units = {"APP/DDR", "APP/HBM", "APP/MEMORY", "Device/DDR", "Device/HBM", "Device/MEMORY"};
            dataTypes = {{"B"}};
            break;
        case PROCESS_TYPE::HCCS:
            units = {"HCCS"};
            dataTypes = {{"txThroughput(B/s)", "rxThroughput(B/s)"}};
            break;
        case PROCESS_TYPE::PCIE:
            units = {"PCIe_post", "PCIe_nonpost", "PCIe_cpl", "PCIe_nonpost_latency"};
            dataTypes = {{"txAvg(B/s)", "rxAvg(B/s)"}};
            break;
        case PROCESS_TYPE::AI_CORE:
            counter->metaData.processName = "AI Core Freq";
            units = { "AI Core Freq" };
            dataTypes = {{ "Mhz" }};
            break;
        default:
            break;
    }
}

std::string DbTraceDataBase::GetSearchSliceNameSql(bool isMatchExact, bool isMatchCase, std::string rankId,
                                                   const std::string &order, const std::string &orderByField)
{
    std::string sql;
    std::string nameMatch = " select id from STRING_IDS where ";
    std::string orderKey = orderByField == "timestamp" ? "startTime" : orderByField;
    std::string orderBy = " ORDER BY " + orderKey + (order == "descend" ? " DESC" : "ASC");
    nameMatch.append(isMatchCase ? " value like " : "lower(value) like lower(");
    nameMatch.append(isMatchExact ? "?" : "'%'||?||'%'");
    nameMatch.append(isMatchCase ? " " : ")");
    if (strcmp(rankId.c_str(), path.c_str()) == 0) {
        sql = "with ids as (" + nameMatch + ") "
            "SELECT globalTid as pid, type as tid, startNs - ? as startTime,endNs - startNs as duration, "
            " depth, api.id, 'HOST' as metaType ,? as rankId"
            " FROM (select globalTid, type, startNs, endNs, depth, ROWID as id, name "
            " from " + TABLE_CANN_API + " api join ids on ids.id = api.name "
            " Union all select globalTid, 'MsTx' as type, startNs, endNs, depth, ROWID as id, message as name "
            " from " + TABLE_MSTX_EVENTS + " api join ids on ids.id = api.message "
            " UNION all select globalTid, 'pytorch' as type, startNs, endNs, depth, ROWID as id, name "
            " from " + TABLE_API + " api join ids on ids.id = api.name) api " + orderBy + " LIMIT 1 OFFSET ?";
    } else {
        sql = "with ids as (" + nameMatch +
            "), minTime as (select ? as value), "
            " tasks as (select ROWID, globalTaskId, taskType, 'Ascend Hardware' as pid, streamId as tid, connectionId, "
            " startNs - minTime.value as startTime, endNs - startNs as duration,depth from TASK join minTime "
            " where deviceId = ? ORDER BY startTime), "
            " com as (select opId, tasks.ROWID as id, 'HCCL' as pid, planeId as tid, startTime, duration, 0 as depth,"
            " info.taskType as name from COMMUNICATION_TASK_INFO info "
            " join tasks on info.globalTaskId=tasks.globalTaskId ORDER BY startTime) "
            " select * from ( select coalesce(compute.name, main.taskType) as name, main.pid, main.pid as metaType,"
            " main.tid, main.startTime, main.duration, main.depth, main.ROWID as id from tasks main "
            " left join COMPUTE_TASK_INFO compute on compute.globalTaskId = main.globalTaskId "
            " join ids on ids.id = coalesce(compute.name, main.taskType) "
            " union ALL select name, pid, pid as meatType, tid, startTime, duration, depth, com.id from com "
            " join ids on ids.id = com.name "
            " union ALL select opName as name,'HCCL' as pid, 'HCCL' as metaType, groupName||'group' as tid,"
            " startNs - minTime.value as startTime, endNs - startNs as duration, 0 as depth, op.ROWID as id "
            " from COMMUNICATION_OP op join minTime join tasks on op.connectionId = tasks.connectionId "
            " join ids on ids.id = opName group by opId ) allNames " + orderBy + " LIMIT 1 OFFSET ?";
    }
    return sql;
}

std::string DbTraceDataBase::GetSearchSliceNameCountSql(bool isMatchExact, bool isMatchCase, std::string rankId)
{
    std::string sql;
    std::string nameMatch;
    if (isMatchExact && isMatchCase) {
        nameMatch = "select id from STRING_IDS where value like ?";
    } else if (isMatchExact) {
        nameMatch = "select id from STRING_IDS where lower(value) like lower(?)";
    } else if (isMatchCase) {
        nameMatch = "select id from STRING_IDS where value like '%'||?||'%'";
    } else {
        nameMatch = "select id from STRING_IDS where lower(value) like lower('%'||?||'%')";
    }
    if (!DataBaseManager::Instance().GetDbPathByHost(rankId).empty()) {
        sql = "with ids as (" + nameMatch +
            ") SELECT count(1),? as id FROM (select name from " + TABLE_CANN_API +
            " union all select message from  " + TABLE_MSTX_EVENTS +
            " union all select name from  " + TABLE_API + ") api join ids on id = api.name";
    } else {
        sql = "with ids as (" + nameMatch + "), "
            "     tasks as (select globalTaskId, taskType, connectionId from TASK where deviceId = ?), "
            "     com as (select opId, info.globalTaskId,info.taskType as name from COMMUNICATION_TASK_INFO info "
            " join tasks on  info.globalTaskId = tasks.globalTaskId), "
            "     compute as (select info.globalTaskId, name from COMPUTE_TASK_INFO info join tasks "
            " on  info.globalTaskId = tasks.globalTaskId) "
            "select count(1) from ( "
            "    select coalesce(compute.name, main.taskType) as name from tasks main "
            "         left join compute on compute.globalTaskId = main.globalTaskId "
            "    union ALL select name from com "
            "    union ALL select opName as name from COMMUNICATION_OP op "
            " join tasks on op.connectionId = tasks.connectionId group by opId"
            ") allNames join ids on id = allNames.name;";
    }
    return sql;
}

bool DbTraceDataBase::SearchAllSlicesDetails(const Protocol::SearchAllSliceParams &params,
                                             Protocol::SearchAllSlicesBody &body, uint64_t minTimestamp)
{
    uint64_t count = 0;
    uint64_t offset = (params.current - 1) * params.pageSize;
    const std::string &sql = GetSearchAllSlicesDetailsSql(params.isMatchExact, params.isMatchCase,
                                                          params.order, params.orderBy);
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QuerySliceName failed!.");
        return false;
    }
    auto resultSet = stmt->ExecuteQuery(params.searchContent, minTimestamp, GetRealRankId(params.rankId),
                                        params.pageSize, offset);
    if (resultSet == nullptr) {
        ServerLog::Error("SearchAllSlicesDetails. Failed to get result set.", stmt->GetErrorMessage());
        return false;
    }
    while (resultSet->Next()) {
        Protocol::SearchAllSlices searchAllSlice{};
        searchAllSlice.name = resultSet->GetString("value");
        searchAllSlice.timestamp = resultSet->GetUint64("startTime");
        searchAllSlice.duration = resultSet->GetUint64("duration");
        searchAllSlice.id = resultSet->GetString("id");
        searchAllSlice.tid = resultSet->GetString("tid");
        searchAllSlice.pid = resultSet->GetString("pid");
        searchAllSlice.depth = resultSet->GetUint64("depth");
        auto deviceId = resultSet->GetString("deviceId");
        searchAllSlice.deviceId = deviceId.empty() ? path : QueryHostInfo() + deviceId;
        body.searchAllSlices.emplace_back(searchAllSlice);
    }
    body.currentPage = params.current;
    body.pageSize = params.pageSize;
    Protocol::SearchCountParams searchCountParams;
    searchCountParams.searchContent = params.searchContent;
    searchCountParams.isMatchCase = params.isMatchCase;
    searchCountParams.isMatchExact = params.isMatchExact;
    searchCountParams.rankId = params.rankId;
    count += SearchSliceNameCount(searchCountParams);
    searchCountParams.rankId = QueryHostInfo() + "Host";
    count += SearchSliceNameCount(searchCountParams);
    body.count = count;
    return true;
}

std::string DbTraceDataBase::GetSearchAllSlicesDetailsSql(bool isMatchExact, bool isMatchCase,
                                                          const std::string &order, const std::string &orderByField)
{
    std::string sql;
    std::string nameMatch;
    std::string orderBy;
    std::string orderKey = orderByField == "timestamp" ? "startTime" : orderByField;
    if (order == "descend") {
        orderBy = " ORDER BY " + orderKey + " DESC";
    } else {
        orderBy = " ORDER BY " + orderKey + " ASC";
    }
    if (isMatchExact && isMatchCase) {
        nameMatch = "select id, value from STRING_IDS where value like ?";
    } else if (isMatchExact) {
        nameMatch = "select id, value from STRING_IDS where lower(value) like lower(?)";
    } else if (isMatchCase) {
        nameMatch = "select id, value from STRING_IDS where value like '%'||?||'%'";
    } else {
        nameMatch = "select id, value from STRING_IDS where lower(value) like lower('%'||?||'%')";
    }
    sql = "with ids as (" + nameMatch + "), minTime as (select ? as value),\n"
          " tasks as (select deviceId, TASK.ROWID, globalTaskId, taskType, 'Ascend Hardware' as pid, streamId as tid, "
          " startNs - minTime.value as startTime,endNs - startNs as duration,depth,connectionId from TASK join minTime "
          " where deviceId = ? ORDER BY startTime),\n"
          " com as (select deviceId, opId, tasks.ROWID as id, 'HCCL' as pid, planeId as tid,"
          " startTime, duration, 0 as depth, info.taskType as name"
          " from COMMUNICATION_TASK_INFO info join tasks on info.globalTaskId=tasks.globalTaskId "
          " ORDER BY startTime)\n"
          " select * from ( select deviceId, coalesce(compute.name, main.taskType) as name, main.pid,"
          " main.pid as metaType,"
          " main.tid, main.startTime, main.duration, main.depth, main.ROWID as id from tasks main\n"
          " left join COMPUTE_TASK_INFO compute on compute.globalTaskId = main.globalTaskId union ALL"
          " select deviceId,name, pid, pid as meatType, tid, startTime, duration, depth, id from com "
          " union ALL select deviceId,opName as name,'HCCL' as pid, 'HCCL' as metaType, groupName||'group' as tid,"
          " startNs - minTime.value as startTime, endNs - startNs as duration, 0 as depth, op.ROWID as id "
          " from COMMUNICATION_OP op join minTime join tasks on op.connectionId = tasks.connectionId group by opId \n"
          " UNION all select '' as deviceId, name, globalTid as pid, 'HOST' as metaType,"
          " type as tid, "
          "startNs - minTime.value AS startTime, endNs - startNs AS duration, depth, CANN_API.ROWID as id from "
          "CANN_API JOIN minTime UNION all select '' as deviceId, name, globalTid as pid, 'HOST' as metaType,"
          " 'pytorch' as tid, "
          "startNs - minTime.value AS startTime, endNs - startNs AS duration, depth, PYTORCH_API.ROWID as id from "
          "PYTORCH_API JOIN minTime ) allNames join ids on ids.id = allNames.name" + orderBy + " LIMIT ? OFFSET ?";
    return sql;
}


bool DbTraceDataBase::QueryAffinityOptimizer(const Protocol::KernelDetailsParams &params, const std::string &optimizers,
    std::vector<Protocol::ThreadTraces> &data, uint64_t minTimestamp)
{
    if (!CheckTableExist(TABLE_API)) {
        ServerLog::Warn("The PYTORCH_API table isn't exist.");
        return false;
    }
    std::string sql = "SELECT py.ROWID as id, py.startNs - ? as startTime, (py.endNs - py.startNs) / 1000 as duration, "
        "str.value as originOptimizer, py.globalTid as pid, 'pytorch' as tid, py.depth as depth "
        "FROM " + TABLE_STRING_IDS + " str JOIN " + TABLE_API + " py ON py.name = str.id "
        "WHERE str.value IN (" + optimizers + ") ORDER BY " + params.orderBy + " " + params.order;
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Fail to prepare sql for Query Affinity Optimizer by DB.", sqlite3_errmsg(db));
        return false;
    }
    auto resultSet = stmt->ExecuteQuery(minTimestamp);
    if (resultSet == nullptr) {
        ServerLog::Error("Failed to get result set for Query Affinity Optimizer by DB.", stmt->GetErrorMessage());
        return false;
    }
    while (resultSet->Next()) {
        Protocol::ThreadTraces one{};
        one.id = resultSet->GetString("id");
        one.startTime = resultSet->GetUint64("startTime");
        one.name = resultSet->GetString("originOptimizer");
        one.duration = resultSet->GetUint64("duration");
        one.threadId = resultSet->GetString("tid");
        one.pid = resultSet->GetString("pid");
        one.depth = resultSet->GetUint64("depth");
        data.emplace_back(one);
    }
    return true;
}

bool DbTraceDataBase::QueryAICpuOpCanBeOptimized(const Protocol::KernelDetailsParams &params,
    const std::vector<std::string> &replace, const std::map<std::string, Timeline::AICpuCheckDataType> &dataType,
    std::vector<Protocol::KernelBaseInfo> &data, uint64_t minTimestamp)
{
    std::string sql = TextSqlConstant::GenerateAICpuQuerySqlDB(replace, params, dataType);
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Fail to prepare sql for AICpuOpCanBeOptimized.");
        return false;
    }
    auto resultSet = stmt->ExecuteQuery(minTimestamp, AICPU_OP_DURATION_THRESHOLD / THOUSAND);
    if (resultSet == nullptr) {
        ServerLog::Error("Failed to get result set for AICpuOpCanBeOptimized.", stmt->GetErrorMessage());
        return false;
    }
    while (resultSet->Next()) {
        Protocol::KernelBaseInfo one{};
        one.id = resultSet->GetString("id");
        one.name = resultSet->GetString("name");
        one.type = resultSet->GetString("type");
        one.startTime = resultSet->GetUint64("startTime");
        one.duration = resultSet->GetUint64("duration");
        one.pid = resultSet->GetString("pid");
        one.tid = resultSet->GetString("tid");
        one.depth = resultSet->GetUint64("depth");
        one.inputType = resultSet->GetString("input");
        one.outputType = resultSet->GetString("output");
        data.emplace_back(one);
    }
    return true;
}

bool DbTraceDataBase::QueryThreadSameOperatorsDetails(const Protocol::UnitThreadsOperatorsParams &requestParams,
    Protocol::UnitThreadsOperatorsBody &responseBody, uint64_t minTimestamp, int64_t traceId)
{
    if (!StringUtil::CheckSqlValid(requestParams.orderBy)) {
        ServerLog::Error("There is an SQL injection attack on this parameter. error param: ", requestParams.orderBy);
        return false;
    }
    std::string orderBy = " ORDER BY " + requestParams.orderBy;
    orderBy.append(requestParams.order == "descend" ? " DESC " : " ASC ");
    auto stmt = CreatPreparedStatement();
    std::unique_ptr <SqliteResultSet> resultSet;
    try {
        resultSet = TraceDatabaseHelper::QueryThreadSameOperatorsDetails(stmt, requestParams,
            GetRealRankId(requestParams.rankId), minTimestamp, orderBy);
    } catch (DatabaseException &e) {
        ServerLog::Error("QueryThreadSameOperatorsDetails Fail, ", e.What());
        return false;
    }
    while (resultSet->Next()) {
        int col = resultStartIndex;
        Protocol::SameOperatorsDetails sameOperatorsDetail{};
        sameOperatorsDetail.timestamp = resultSet->GetUint64(col++);
        sameOperatorsDetail.duration = resultSet->GetUint64(col++);
        sameOperatorsDetail.depth = resultSet->GetUint64(col++);
        sameOperatorsDetail.id = resultSet->GetString(col++);
        responseBody.sameOperatorsDetails.emplace_back(sameOperatorsDetail);
    }
    responseBody.currentPage = requestParams.current;
    responseBody.pageSize = requestParams.pageSize;
    return true;
}

bool DbTraceDataBase::QueryAclnnOpCountExceedThreshold(const KernelDetailsParams &params, uint64_t threshold,
    std::vector<Protocol::KernelBaseInfo> &data, uint64_t minTimestamp)
{
    std::string sql =
        "SELECT info.ROWID as id, s1.value as name, s2.value as op_type, task.taskType, task.startNs - ? as startTime, "
        "(task.endNs - task.startNs) / 1000 as duration, 'Ascend Hardware' as pid, task.streamId as tid,"
        " task.depth as depth "
        "FROM " + TABLE_COMPUTE_TASK_INFO + " info "
        "JOIN " + TABLE_TASK + " task ON info.globalTaskId = task.globalTaskId "
        "JOIN " + TABLE_STRING_IDS + " s1 ON info.name = s1.id "
        "JOIN " + TABLE_STRING_IDS + " s2 ON info.opType = s2.id "
        "WHERE s1.value IN ("
        "    SELECT str.value FROM " + TABLE_COMPUTE_TASK_INFO + " info "
        "    JOIN " + TABLE_STRING_IDS + " str ON info.name = str.id "
        "    WHERE str.value LIKE 'aclnn%' "
        "    GROUP BY str.value HAVING COUNT(str.value) >= ?"
        ") ORDER BY " + params.orderBy + " " + params.order;
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Fail to prepare sql for Aclnn Op Exceed Threshold.");
        return false;
    }
    auto resultSet = stmt->ExecuteQuery(minTimestamp, threshold);
    if (resultSet == nullptr) {
        ServerLog::Error("Failed to get result set for Aclnn Op Exceed Threshold.", stmt->GetErrorMessage());
        return false;
    }
    while (resultSet->Next()) {
        Protocol::KernelBaseInfo one{};
        one.id = resultSet->GetString("id");
        one.name = resultSet->GetString("name");
        one.startTime = resultSet->GetUint64("startTime");
        one.duration = resultSet->GetUint64("duration");
        one.pid = resultSet->GetString("pid");
        one.tid = resultSet->GetString("tid");
        one.depth = resultSet->GetUint64("depth");
        data.emplace_back(one);
    }
    return true;
}

bool DbTraceDataBase::QueryAffinityAPIData(const Protocol::KernelDetailsParams &params,
    const std::set<std::string> &pattern, uint64_t minTimestamp, std::map<uint64_t,
    std::vector<Protocol::FlowLocation>> &data, std::map<uint64_t, std::vector<uint32_t>> &indexes)
{
    std::string sql = "SELECT py.ROWID as id, str.value as name, py.startNs - ? as startTime, "
        "py.endNs - ? as endTime, py.globalTid as pid, 'pytorch' as tid, py.depth as depth "
        "FROM " + TABLE_API + " py JOIN " + TABLE_STRING_IDS + " str ON py.name = str.id "
        "WHERE str.value LIKE 'aten::%' OR str.value LIKE 'npu::%' ORDER BY py.globalTid ASC, py.startNs ASC ";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Failed to prepare sql for Affinity API.");
        return false;
    }
    auto resultSet = stmt->ExecuteQuery(minTimestamp, minTimestamp);
    if (resultSet == nullptr) {
        ServerLog::Error("Failed to get result set for Affinity API data.", stmt->GetErrorMessage());
        return false;
    }
    std::map<uint64_t, std::vector<Protocol::FlowLocation>> filterData;
    while (resultSet->Next()) {
        Protocol::FlowLocation one{};
        uint64_t trackId = resultSet->GetUint64("pid");
        one.id = resultSet->GetString("id");
        one.name = resultSet->GetString("name");
        one.timestamp = resultSet->GetUint64("startTime");
        // Protocol::FlowLocation数据结构中只定义start time和duration，绝大多数场景下也是只用上述两个字段，
        // 此处需要比较start time和end time，是个特例，在不修改数据结构的情况下，duration中实际存的是end time，
        // 过滤顶层API后，在根据end time和start time求出duration
        one.duration = resultSet->GetUint64("endTime");
        one.pid = resultSet->GetString("pid");
        one.tid = resultSet->GetString("tid");
        one.depth = resultSet->GetUint64("depth");

        if (data.count(trackId) == 0) {
            filterData.emplace(trackId, std::vector<Protocol::FlowLocation>{});
            data.emplace(trackId, std::vector<Protocol::FlowLocation>{});
            indexes.emplace(trackId, std::vector<uint32_t>{});
        }

        filterData[trackId].emplace_back(one);
    }
    for (const auto &item : filterData) {
        std::vector<Protocol::FlowLocation> originData = item.second;
        TraceDatabaseHelper::FilterTopLevelApi(originData, pattern, data[item.first], indexes[item.first]);
    }

    return true;
}

bool DbTraceDataBase::QueryFuseableOpData(const KernelDetailsParams &params, const FuseableOpRule &rule,
    std::vector<Protocol::FlowLocation> &data, uint64_t minTimestamp)
{
    std::string sql =
        "WITH data AS ( "
        "SELECT info.ROWID as id, task.deviceId as deviceId, s1.value as name, s2.value as op_type, task.taskType, "
        "task.startNs - ? as startTime, (task.endNs - task.startNs) / 1000 as duration, 'Ascend Hardware' as pid, "
        "task.streamId as tid, task.depth as depth, "
        "ROW_NUMBER() OVER (ORDER BY task.globalPid ASC, task.startNs ASC) AS row_num "
        "FROM " + TABLE_COMPUTE_TASK_INFO + " info "
        "JOIN " + TABLE_TASK + " task ON info.globalTaskId = task.globalTaskId "
        "JOIN " + TABLE_STRING_IDS + " s1 ON info.name = s1.id "
        "JOIN " + TABLE_STRING_IDS + " s2 ON info.opType = s2.id "
        " ) "
        "SELECT d0.* FROM data d0 ";
    for (size_t i = 1; i < rule.opList.size(); ++i) { // 上文保证rule.opList.size() ≥ 2
        std::string table = "d" + std::to_string(i);
        sql += "JOIN data " + table + " ON " + table + ".row_num = d0.row_num + " + std::to_string(i) +
               " AND " + table + ".op_type = '" + rule.opList.at(i) + "' ";
    }
    sql += "WHERE d0.op_type = '" +  rule.opList.at(0) + "' ORDER BY " + params.orderBy + " " + params.order;
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Failed to prepare sql for query Fusionable Operator.");
        return false;
    }
    auto resultSet = stmt->ExecuteQuery(minTimestamp);
    if (resultSet == nullptr) {
        ServerLog::Error("Failed to get result set for query Fuseable Operator.", stmt->GetErrorMessage());
        return false;
    }

    while (resultSet->Next()) {
        Protocol::FlowLocation one{};
        one.id = resultSet->GetString("id");
        one.name = resultSet->GetString("name");
        one.timestamp = resultSet->GetUint64("startTime");
        one.duration = resultSet->GetUint64("duration");
        one.pid = resultSet->GetString("pid");
        one.tid = resultSet->GetString("tid");
        one.depth = resultSet->GetUint64("depth");
        one.type = StringUtil::join(rule.opList, ", ");
        one.metaType = rule.fusedOp;
        one.note = rule.note;
        data.emplace_back(one);
    }

    return true;
}

bool DbTraceDataBase::QueryEventsViewData(const Protocol::EventsViewParams &params, Protocol::EventsViewBody &body,
    uint64_t minTimestamp)
{
    auto stmt = CreatPreparedStatement();
    if (stmt == nullptr) {
        return false;
    }
    return TraceDatabaseHelper::QueryEventsViewData4Db(stmt, params, body, minTimestamp, GetRealRankId(params.rankId));
}

std::vector<Protocol::SimpleSlice> DbTraceDataBase::QueryThreadByPid(const Metadata &metaData,
                                                                     uint64_t startTime,
                                                                     uint64_t endTime,
                                                                     const std::string &rankId,
                                                                     std::map<std::string, uint64_t> &selfTimeKeyValue)
{
    auto stmt = CreatPreparedStatement();
    if (stmt == nullptr) {
        ServerLog::Error("Query_threads. Failed to prepare sql.", sqlite3_errmsg(db));
        return {};
    }
    std::vector<Protocol::SimpleSlice> completeSlice;
    try {
        auto resultSet = TraceDatabaseHelper::QueryThreadsByPid(stmt, startTime, endTime, metaData, rankId);
        while (resultSet->Next()) {
            int col = resultStartIndex;
            Protocol::SimpleSlice simpleSlice{};
            simpleSlice.timestamp = resultSet->GetInt64(col++);
            simpleSlice.duration = resultSet->GetInt64(col++);
            simpleSlice.endTime = resultSet->GetInt64(col++);
            simpleSlice.name = stringsCache.at(path)[resultSet->GetString(col++)];
            simpleSlice.depth = resultSet->GetInt32(col++);
            simpleSlice.tid = metaData.tid;
            simpleSlice.pid = metaData.pid;
            simpleSlice.metaType = metaData.metaType;
            completeSlice.emplace_back(simpleSlice);
        }
    } catch (DatabaseException &e) {
        ServerLog::Error("Query threads failed, ", e.What());
        return {};
    }
    if (completeSlice.empty()) {
        return completeSlice;
    }
    TraceDatabaseHelper::CalculateSelfTime(completeSlice, selfTimeKeyValue, startTime, endTime);
    return completeSlice;
}

void DbTraceDataBase::Reset()
{
    stringsCache.clear();
}
}