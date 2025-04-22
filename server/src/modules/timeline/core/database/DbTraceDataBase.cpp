// Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
#include "DbTraceDataBase.h"
#include "pch.h"
#include "TraceTime.h"
#include "TableDefs.h"
#include "TraceDatabaseHelper.h"
#include "TraceDatabaseSqlConst.h"
#include "CommonDefs.h"
#include "DataBaseManager.h"
#include "CollectionTimeService.h"
#include "MetaDataParser.h"
#include "MetaDataCacheManager.h"
#include "DbKernelDetailHelper.h"

namespace Dic::Module::FullDb {
using namespace Server;
static std::map<std::string, std::map<std::string, std::string>> stringsCache;

DbTraceDataBase::~DbTraceDataBase()
{
    updateCannApiDepthStmt = nullptr;
    insertOverlapStmt = nullptr;
    updateApiDepthStmt = nullptr;
    updateTaskDepthStmt = nullptr;
}

bool DbTraceDataBase::QueryThreads(const Protocol::UnitThreadsParams &requestParams,
    Protocol::UnitThreadsBody &responseBody, uint64_t minTimestamp, const std::vector<uint64_t> &trackIdList)
{
    uint64_t startTime = requestParams.startTime + minTimestamp;
    uint64_t endTime = requestParams.endTime + minTimestamp;
    // 遍历metadataList,这里不用union all,方便后续返回中拼接pid和tid等信息用于前端过滤
    std::vector<SimpleSlice> simpleSliceVec;
    std::map<std::string, uint64_t> selfTimeKeyValue;
    for (auto &&metadata: requestParams.metadataList) {
        std::string rankId = GetDeviceId(requestParams.rankId);
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
    if (stringsCache.find(path) == stringsCache.end()) {
        stringsCache[path] = {}; // 初始化空的映射
    }
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

std::vector<FlowLocation> DbTraceDataBase::ConvertResultToFlowLocation(std::unique_ptr<SqliteResultSet> resultSet)
{
    std::vector<FlowLocation> flowLocations;
    while (resultSet->Next()) {
        auto metaType = resultSet->GetString("metaType");
        auto rankId = resultSet->GetString("deviceId");
        for (const auto &item: QueryRankIdAndDeviceMap()) {
            if (rankId == item.second) {
                rankId = item.first;
            }
        }
        rankId = rankId.empty() ? path : QueryHostInfo() + rankId;
        FlowLocation location {
                .tid = resultSet->GetString("tid"), .id = resultSet->GetString("id"),
                .metaType = metaType, .rankId = rankId,
                .depth = resultSet->GetUint32("depth"), .timestamp = resultSet->GetUint64("startTime"),
                .duration = resultSet->GetUint64("duration"), .pid = resultSet->GetString("pid"),
                .name = stringsCache.at(path)[resultSet->GetString("name")]
        };
        flowLocations.push_back(location);
    }
    return flowLocations;
}

bool DbTraceDataBase::QueryUintFlows(const Protocol::UnitFlowsParams &requestParams,
    Protocol::UnitFlowsBody &responseBody, uint64_t minTimestamp, uint64_t trackId)
{
    auto stmt = CreatPreparedStatement();
    auto connectionId = TraceDatabaseHelper::QueryConnectionId(stmt, requestParams);
    if (!connectionId.has_value()) {
        return false;
    }
    std::vector<uint64_t> deviceIdList = TraceDatabaseHelper::GetDeviceIdList(requestParams.rankId);
    std::string comSql = deviceIdList.size() == 1 ? COM_OP_UNIT_FLOW_SQL_UNIQUE_DEVICE : COM_OP_UNIT_FLOW_SQL;
    std::string sql = "with constValue as (select ? as minTime, ? as connectionId)\n";
    sql += PYTORCH_UNIT_FLOW_SQL + " union all ";
    sql += CANN_UNIT_FLOW_SQL + " union all ";
    sql += TASK_UNIT_FLOW_SQL + " union all " + comSql + " union all " + MSTX_UNIT_FLOW_SQL;
    sql += " order by startTime ";
    std::unique_ptr<SqliteResultSet> resultSet;
    if (deviceIdList.size() == 1) {
        resultSet = TraceDatabaseHelper::ExecuteQuery(stmt, sql, minTimestamp, connectionId.value(), deviceIdList[0]);
    } else {
        resultSet = TraceDatabaseHelper::ExecuteQuery(stmt, sql, minTimestamp, connectionId.value());
    }
    std::vector<FlowLocation> flowLocations = ConvertResultToFlowLocation(std::move(resultSet));
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

bool DbTraceDataBase::SetCardAlias(const Protocol::SetCardAliasParams &requestParams,
                                   Protocol::SetCardAliasBody &responseBody)
{
    if (!CheckTableExist(metaDataTable)) {
        ServerLog::Error("Failed to set card alias because table is not exist.");
        return false;
    }
    return UpdateMetaDataTableWithNoPrimaryKey(cardAliasName, requestParams.cardAlias);
}

std::string DbTraceDataBase::QueryCardAlias()
{
    std::string cardAlias = GetValueFromMetaDataTable(cardAliasName);
    if (cardAlias.empty()) {
        return "";
    }
    return cardAlias;
}

uint32_t DbTraceDataBase::SearchSliceNameCount(const Protocol::SearchCountParams &params)
{
    uint32_t result = 0;
    const std::string &sql =
        TraceDatabaseHelper::GetSearchSliceNameCountSql(params.isMatchExact, params.isMatchCase, params.rankId);
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Query slice name count failed!.");
        return 0;
    }
    auto resultSet = stmt->ExecuteQuery(params.searchContent, GetDeviceId(params.rankId));
    if (resultSet == nullptr) {
        ServerLog::Error("Query_slice_name_count. Failed to get result set.", stmt->GetErrorMessage());
        return 0;
    }
    if (resultSet->Next()) {
        result = resultSet->GetUint32(resultStartIndex);
    }
    return result;
}

uint32_t DbTraceDataBase::SearchSliceNameCount(const Protocol::SearchCountParams &params,
    const std::vector<TrackQuery> &trackQuery)
{
    if (trackQuery.empty() && !params.metadataList.empty()) {
        return 0;
    }
    if (trackQuery.empty()) {
        return SearchSliceNameCount(params);
    }
    std::string sql = TraceDatabaseHelper::GetSearchCountWithLockSql(params, trackQuery);
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Query slice name count failed!.");
        return 0;
    }
    TraceDatabaseHelper::SearchCountWithLockRangeBindStmt(params, trackQuery, stmt, GetDeviceId(params.rankId));
    auto resultSet = stmt->ExecuteQuery();
    if (resultSet == nullptr) {
        ServerLog::Error("Query slice name count. Failed to get result set.", stmt->GetErrorMessage());
        return 0;
    }
    uint32_t result = 0;
    while (resultSet->Next()) {
        const uint32_t count = resultSet->GetUint32(resultStartIndex);
        if (result > UINT32_MAX - count) {
            ServerLog::Warn("Sum of searching slice name count is overflow.");
            break;
        }
        result = result + count;
    }
    return result;
}

bool DbTraceDataBase::QueryFlowCategoryList(std::vector<std::string> &categories, const std::string& rankId)
{
    auto stmt = CreatPreparedStatement("select cat from connectionCats group by cat");
    if (stmt == nullptr) {
        return false;
    }
    auto resultSet = stmt->ExecuteQuery();
    if (resultSet == nullptr) {
        return false;
    }
    while (resultSet->Next()) {
        categories.emplace_back(resultSet->GetString("cat"));
    }
    return true;
}

bool DbTraceDataBase::SearchSliceName(const Protocol::SearchSliceParams &params, int index, uint64_t minTimestamp,
    Protocol::SearchSliceBody &responseBody)
{
    std::string sql =
        TraceDatabaseHelper::GetSearchSliceNameSql(params.isMatchExact, params.isMatchCase, responseBody.rankId, path);
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Query slice name failed!.");
        return false;
    }
    auto resultSet = stmt->ExecuteQuery(params.searchContent, minTimestamp, GetDeviceId(responseBody.rankId), index);
    if (resultSet == nullptr || !resultSet->Next()) {
        ServerLog::Error("Query_slice_name. Failed to get result set.", stmt->GetErrorMessage());
        return false;
    }
    responseBody.pid = resultSet->GetString("pid");
    responseBody.tid = resultSet->GetString("tid");
    responseBody.startTime = resultSet->GetUint64("startTime");
    responseBody.duration = resultSet->GetUint64("duration");
    responseBody.depth = resultSet->GetUint32("depth");
    responseBody.id = resultSet->GetString("id");
    return true;
}

bool DbTraceDataBase::SearchSliceName(const Protocol::SearchSliceParams &params, int index, uint64_t minTimestamp,
    Protocol::SearchSliceBody &responseBody, const std::vector<TrackQuery> &trackQuery)
{
    if (trackQuery.empty() && !params.metadataList.empty()) {
        return true;
    }
    if (trackQuery.empty()) {
        return SearchSliceName(params, index, minTimestamp, responseBody);
    }
    std::string sql = TraceDatabaseHelper::GetSearchSliceNameWithLockRangeSql(params, trackQuery, path);
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Query slice name failed!.");
        return false;
    }
    TraceDatabaseHelper::SearchSliceNameWithLockRangeBindStmt(params, trackQuery, stmt, path,
        GetDeviceId(params.rankId));
    auto resultSet = stmt->ExecuteQuery(index);
    if (resultSet == nullptr || !resultSet->Next()) {
        ServerLog::Error("Query_slice_name. Failed to get result set.", stmt->GetErrorMessage());
        return false;
    }
    responseBody.pid = resultSet->GetString("pid");
    responseBody.tid = resultSet->GetString("tid");
    responseBody.startTime = resultSet->GetUint64("timestamp");
    uint64_t endTime = resultSet->GetUint64("endTime");
    responseBody.duration = endTime >= responseBody.startTime ? endTime - responseBody.startTime : 0;
    responseBody.startTime -= minTimestamp; // 业务上 minTimestamp 是最小的时间，一定有 item.timestamp > minTimestamp
    responseBody.depth = resultSet->GetUint32("depth");
    responseBody.id = resultSet->GetString("id");
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
        resultSet = TraceDatabaseHelper::QueryUnitCounter(stmt, params, minTimestamp, GetDeviceId(params.rankId));
    } catch (DatabaseException &e) {
        ServerLog::Error("Query unit counter failed, ", e.What());
        return false;
    }
    while (resultSet->Next()) {
        Protocol::UnitCounterData unitCounterData;
        unitCounterData.timestamp = resultSet->GetUint64("startTime");
        unitCounterData.valueJsonStr = resultSet->GetString("args");
        dataList.emplace_back(unitCounterData);
    }
    return true;
}

bool DbTraceDataBase::QueryComputeStatisticsData(const Protocol::SummaryStatisticParams &requestParams,
    Protocol::SummaryStatisticsBody &responseBody)
{
    sqlite3_stmt *stmt = nullptr;
    std::string sql = "SELECT round(sum(endNs - startNs) / 1000.0, 2) as duration, TASKTYPE.value as acceleratorCore "
        "FROM COMPUTE_TASK_INFO JOIN TASK ON COMPUTE_TASK_INFO.globalTaskId = TASK.globalTaskId "
        "JOIN STRING_IDS AS TASKTYPE ON TASKTYPE.id = COMPUTE_TASK_INFO.taskType "
        "WHERE acceleratorCore in ('AI_CPU','AI_CORE', 'AI_VECTOR_CORE', 'MIX_AIC', 'MIX_AIV', 'FFTS_PLUS') "
        "GROUP BY acceleratorCore";
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
        resultSet = TraceDatabaseHelper::QuerySystemViewData(stmt, requestParams, GetDeviceId(requestParams.rankId));
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
            responseBody.total = resultSet->GetUint64(col++);
        }
        responseBody.systemViewDetail.emplace_back(systemViewDetail);
    }
    responseBody.pageSize = requestParams.pageSize;
    responseBody.currentPage = requestParams.current;
    return true;
}

bool DbTraceDataBase::QuerySystemViewAICoreFreqData(const Protocol::SystemViewAICoreFreqParams &requestParams,
                                                    Protocol::SystemViewAICoreFreqBody &responseBody)
{
    return true;
}

bool DbTraceDataBase::QueryKernelDetailData(const Protocol::KernelDetailsParams &requestParams,
    Protocol::KernelDetailsBody &responseBody, uint64_t minTimestamp)
{
    std::string sql = DbKernelDetailHelper::GetKernelDetailSql(requestParams, isLowCamel);
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        Server::ServerLog::Error("Fail to prepare sql to query kernel detail data.");
        return false;
    }
    for (const auto& filter : requestParams.filters) {  // 第一次绑定filter
        std::string bindFilter = "%" + filter.second + "%";
        stmt->BindParams(bindFilter);
    }
    if (!requestParams.rankId.empty()) {
        stmt->BindParams(GetDeviceId(requestParams.rankId));
    }
    for (const auto& filter : requestParams.filters) {  // 第二次绑定filter
        std::string bindFilter = "%" + filter.second + "%";
        stmt->BindParams(bindFilter);
    }
    if (!ExcecuteQueryKernelDetailData(stmt, requestParams, responseBody, minTimestamp)) {
        return false;
    }
    responseBody.pageSize = requestParams.pageSize;
    responseBody.currentPage = requestParams.current;
    responseBody.acceleratorCoreList = QueryCoreType();
    return true;
}

bool DbTraceDataBase::ExcecuteQueryKernelDetailData(std::unique_ptr<SqlitePreparedStatement> &stmt,
    const Protocol::KernelDetailsParams &requestParams, Protocol::KernelDetailsBody &responseBody,
    uint64_t minTimestamp)
{
    uint64_t offset = (requestParams.current - 1) > UINT64_MAX / requestParams.pageSize ? 0 :
        (requestParams.current - 1) * requestParams.pageSize;
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
        uint64_t tempStartTime = resultSet->GetUint64("startTime");
        if (tempStartTime < minTimestamp) {
            continue;
        }
        detail.startTime = tempStartTime - minTimestamp;
        detail.duration = resultSet->GetDouble("duration");
        detail.waitTime = resultSet->GetDouble("waitTime");
        detail.blockDim = resultSet->GetUint64("blockDim");
        detail.inputShapes = GetStringCacheValue(path, resultSet->GetString("inputShapes"));
        detail.inputDataTypes = GetStringCacheValue(path, resultSet->GetString("inputDataTypes"));
        detail.inputFormats = GetStringCacheValue(path, resultSet->GetString("inputFormats"));
        detail.outputShapes = GetStringCacheValue(path, resultSet->GetString("outputShapes"));
        detail.outputDataTypes = GetStringCacheValue(path, resultSet->GetString("outputDataTypes"));
        detail.outputFormats = GetStringCacheValue(path, resultSet->GetString("outputFormats"));
        if (responseBody.count == 0) {
            responseBody.count = resultSet->GetUint64("num");
        }
        detail.taskId = std::to_string(resultSet->GetUint64("taskId"));
        responseBody.kernelDetails.emplace_back(detail);
    }
    return true;
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
        total = resultSet->GetUint64("num");
    }
    return total;
}

bool DbTraceDataBase::QueryCommunicationKernelInfo(const std::string &name, const std::string &rankId,
                                                   CommunicationKernelBody &body)
{
    std::string sql = "SELECT info.ROWID as id, info.groupName||'group' as tid, info.opName as name, 'HCCL' as pid, "
                      "0 as depth, info.startNs from COMMUNICATION_OP info ";
    auto getDeviceStmt = CreatPreparedStatement();
    bool isDeviceIdUnique = TraceDatabaseHelper::IsDeviceIdUnique(rankId);
    if (!isDeviceIdUnique) {
        sql += "LEFT JOIN COMMUNICATION_TASK_INFO taskInfo ON info.opId = taskInfo.opId "
               "LEFT JOIN TASK ON TASK.globalTaskId = taskInfo.globalTaskId "
               " where opName = (select id from STRING_IDS where value = ?) and TASK.deviceId = ?";
    } else {
        sql += "where opName = (select id from STRING_IDS where value = ?)";
    }
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Fail to prepare sql to query kernel depth and thread.");
        return false;
    }
    std::string deviceId = GetDeviceId(rankId);
    std::unique_ptr<SqliteResultSet> resultSet = isDeviceIdUnique ? stmt->ExecuteQuery(name) :
        stmt->ExecuteQuery(name, deviceId);
    if (resultSet == nullptr) {
        ServerLog::Error("Failed to get result set to query kernel depth and thread.", stmt->GetErrorMessage());
        return false;
    }
    if (resultSet->Next()) {
        body.id = resultSet->GetString("id");
        body.depth = resultSet->GetUint64("depth");
        body.threadId = resultSet->GetString("tid");
        body.pid = resultSet->GetString("pid");
        body.rankId = QueryHostInfo() + rankId;
        body.startTime = resultSet->GetUint64("startNs");
        body.startTime = body.startTime > Timeline::TraceTime::Instance().GetStartTime() ?
                         body.startTime - Timeline::TraceTime::Instance().GetStartTime() : body.startTime;
    }
    return true;
}

bool DbTraceDataBase::QueryKernelDepthAndThread(const Protocol::KernelParams &params,
    Protocol::OneKernelBody &responseBody, uint64_t minTimestamp)
{
    // 精度缺失，设置500的浮动区间
    std::string sql = "select info.ROWID as id, groupName||'group' as tid, opName as name, 'HCCL' as pid,"
          " 0 as depth from COMMUNICATION_OP info "
          " where name = (select id from STRING_IDS where value = ?) and abs(startNs - ?) <= 500 "
          " UNION all "
          " select T.ROWID as id, T.streamId as tid, name, 'Ascend Hardware' as pid, depth "
          " from COMPUTE_TASK_INFO info join TASK T on info.globalTaskId = T.globalTaskId "
          " where name = (select id from STRING_IDS where value = ?) and abs(startNs - ?) <= 500"
          " UNION all "
          " select info.ROWID as id, (globalTid & 0xFFFFFFFF) AS tid, message as name, (globalTid / 4294967296) AS pid,"
          " depth from MSTX_EVENTS info"
          " where name = (select id from STRING_IDS where value = ?) and abs(startNs - ?) <= 500";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Fail to prepare sql to query kernel depth and thread.");
        return false;
    }
    std::unique_ptr<SqliteResultSet> resultSet;
    uint64_t timestamp = params.timestamp + minTimestamp;
    resultSet = stmt->ExecuteQuery(params.name, timestamp, params.name, timestamp, params.name, timestamp);
    if (resultSet == nullptr) {
        ServerLog::Error("Failed to get result set to query kernel depth and thread.", stmt->GetErrorMessage());
        return false;
    }
    if (resultSet->Next()) {
        responseBody.id = resultSet->GetString("id");
        responseBody.depth = resultSet->GetUint64("depth");
        responseBody.threadId = resultSet->GetString("tid");
        responseBody.pid = resultSet->GetString("pid");
        responseBody.rankId = params.rankId;
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
                                                                  GetDeviceId(requestParams.cardId), minTimestamp);
    } catch (DatabaseException &e) {
        ServerLog::Error("Query thread traces summary failed, ", e.What());
        return false;
    }
    if (resultSet == nullptr) {
        ServerLog::Error("Failed to get result set to query thread traces summary.", stmt->GetErrorMessage());
        return false;
    }
    const uint64_t maxDataCount = 30000;
    uint64_t unitTime = (requestParams.endTime - requestParams.startTime) / maxDataCount;
    unitTime = unitTime <= 0 ? 1 : unitTime;
    TraceDatabaseHelper::ComputeSummarySlice(resultSet, unitTime, responseBody);
    return true;
}
void DbTraceDataBase::UpdateStartTime(const std::string &fileId)
{
    sqlite3_stmt *stmt = nullptr;
    std::string sql;
    if (CheckTableExist(TABLE_API) && !CheckTableExist(TABLE_SESSION_TIME_INFO)) {
        sql = "SELECT min(startNs) as startTimeNs, max(endNs) as endTimeNs FROM " + TABLE_API;
    } else {
        sql = "SELECT startTimeNs, endTimeNs FROM " + TABLE_SESSION_TIME_INFO;
    }
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        Server::ServerLog::Error("Failed to Update Start Time. Msg: ", sqlite3_errmsg(db), " ", result);
        sqlite3_finalize(stmt);
        return;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        int64_t startTime = sqlite3_column_int64(stmt, col++);
        int64_t endTime = sqlite3_column_int64(stmt, col++);
        if (startTime < 0 || endTime < 0) {
            continue;
        }
        TraceTime::Instance().UpdateTime(startTime, endTime);
        TraceTime::Instance().UpdateCardTimeDuration(fileId, startTime, endTime);
        TraceTime::Instance().UpdateCardTimeDuration(QueryHostInfo() + "Host", startTime, endTime);
    }
    Server::ServerLog::Info("Update start and end time. ");
    sqlite3_finalize(stmt);
}

std::vector<OVERLAP_INFO> BuildOverlapInfoList(const std::vector<OVERLAP_INFO> &timeInfoList)
{
    std::vector<OVERLAP_INFO> overlapInfoList;
    // 记录当前最大截结束时间对应的覆盖区块
    // 此处为了添加第一块数据
    OVERLAP_INFO curBlock = OVERLAP_INFO(timeInfoList.begin()->startNs, timeInfoList.begin()->startNs,
                                         -1);
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
    // 此处为了添加最后一块数据
    if (curBlock.type == 1) { // Communication = 1
        overlapInfoList.emplace_back(curBlock.startNs,  // Communication(Not Overlapped) = 2
                                     curBlock.endNs, 2);
    }
    return overlapInfoList;
}
void DbTraceDataBase::GenerateOverlapAnalysis()
{
    if (!CheckTableExist(TABLE_OVERLAP_ANALYSIS)) {
        Server::ServerLog::Error("Generate overlap analysis:Table OVERLAP_ANALYSIS is not exist.");
        return;
    }
    if (CheckValueFromStatusInfoTable(OVERLAP_ANALYSIS_STATUS, FINISH_STATUS)) {
        Server::ServerLog::Info("Generate overlap analysis already finish. skip");
        return;
    }
    {
        std::unique_lock<std::recursive_mutex> lockGuard(mutex);
        ExecSql("delete from OVERLAP_ANALYSIS where 1 = 1");
    }
    QueryRankId();
    std::unordered_map<std::string, std::string> deviceMap = QueryRankIdAndDeviceMap();
    for (const auto &rankId: rankIds) {
        std::string deviceId = deviceMap.count(rankId) == 0 ? rankId : deviceMap.at(rankId);
        std::vector<OVERLAP_INFO> timeInfoList; // 包含computing,Communication 覆盖数据
        QueryTaskTimeInfo(true, timeInfoList, deviceId);
        QueryTaskTimeInfo(false, timeInfoList, deviceId);
        if (timeInfoList.empty()) {
            continue;
        }
        std::sort(timeInfoList.begin(), timeInfoList.end(), std::less<OVERLAP_INFO>());
        const std::vector<OVERLAP_INFO> overlapInfoList = BuildOverlapInfoList(timeInfoList);
        if (InsertOverlapAnalysisInfo(timeInfoList, deviceId) && InsertOverlapAnalysisInfo(overlapInfoList, deviceId)) {
            Server::ServerLog::Info("Generate overlap analysis success for device: ", deviceId);
        } else {
            Server::ServerLog::Error("Generate overlap analysis fail");
            return;
        }
    }
    UpdateValueIntoStatusInfoTable(OVERLAP_ANALYSIS_STATUS, FINISH_STATUS);
}

bool DbTraceDataBase::InsertOverlapAnalysisInfo(const std::vector<OVERLAP_INFO> &overlapInfoList,
                                                const std::string &rankId)
{
    std::lock_guard<std::recursive_mutex> lockGuard(mutex);
    size_t size = overlapInfoList.size();
    size_t count = size / cacheSize;
    bool result = true;
    for (size_t index = 0; index <= count; ++index) {
        size_t start = index * cacheSize;
        size_t length = cacheSize;
        if (size - start < cacheSize) {
            length = size - start;
        }
        if (!StartTransaction()) {
            ServerLog::Error("Failed to start Transaction.");
            return false;
        }
        for (size_t tmpIndex = start; tmpIndex < start + length; tmpIndex++) {
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
                                        const std::string &deviceId)
{
    std::string sql;
    bool isUniqueDevice = TraceDatabaseHelper::IsDeviceIdUnique(path);
    if (isComputing) {
        sql = "select startNs, endNs from TASK main join COMPUTE_TASK_INFO info "
              " on info.globalTaskId = main.globalTaskId where deviceId=? and startNs != endNs order by startNs, endNs";
    } else {
        sql = "select op.startNs, op.endNs from COMMUNICATION_OP op ";
        if (!isUniqueDevice) {
            sql += " join TASK task on task.connectionId = op.connectionId where deviceId=? ";
        }
        sql += " group by opId  order by op.startNs, op.endNs";
    }
    auto stmt = CreatPreparedStatement();
    try {
        auto resultSet = (!isComputing && isUniqueDevice) ? TraceDatabaseHelper::ExecuteQuery(stmt, sql) :
            TraceDatabaseHelper::ExecuteQuery(stmt, sql, deviceId);
        OVERLAP_INFO curInfo {};
        bool hasCurInfo = false;
        while (resultSet->Next()) { // Computing = 0, Communication = 1
            auto info = OVERLAP_INFO(resultSet->GetInt64("startNs"), resultSet->GetInt64("endNs"), isComputing ? 0 : 1);
            if (!hasCurInfo) {
                curInfo = info;
                hasCurInfo = true;
            } else if (info.startNs <= curInfo.endNs) {
                curInfo.endNs = std::max(info.endNs, curInfo.endNs);
            } else {
                timeInfoList.emplace_back(curInfo);
                curInfo = info;
            }
        }
        if (hasCurInfo) {
            timeInfoList.emplace_back(curInfo);
        }
    } catch (DatabaseException &e) {
        ServerLog::Error("Query task time info fail, ", e.What());
        return;
    }
}

void DbTraceDataBase::UpdateWaitTime()
{
    if (!CheckTableExist(TABLE_COMPUTE_TASK_INFO) || !CheckTableExist(TABLE_COMMUNICATION_OP) ||
        !CheckTableDataInvalid(TABLE_TASK)) {
        Server::ServerLog::Error("Update wait time:Table is not exist.");
        return;
    }
    if (CheckValueFromStatusInfoTable(WAIT_TIME_STATUS, FINISH_STATUS)) { // 已更新数据，跳过更新
        return;
    }
    auto stmt = CreatPreparedStatement(FULL_DB_UPDATE_TIME); // 查询数据
    auto updateComputeStmt = CreatPreparedStatement("UPDATE COMPUTE_TASK_INFO SET waitNs = ? WHERE ROWID = ?;");
    auto updateCommunicationStmt = CreatPreparedStatement("UPDATE COMMUNICATION_OP SET waitNs = ? WHERE ROWID = ?;");
    if (stmt == nullptr || updateComputeStmt == nullptr || updateCommunicationStmt == nullptr) {
        ServerLog::Error("Update wait time, fail to prepare sql.");
        return;
    }
    auto resultSet = stmt->ExecuteQuery();
    if (resultSet == nullptr) {
        ServerLog::Error("Update wait time. failed to get result set.", stmt->GetErrorMessage());
        return;
    }
    std::map<int64_t, int64_t> prevTime;
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
            ServerLog::Error("Update wait time. Failed to update data.");
            return;
        }
    }
    if (!UpdateTaskInfoWaitTime(updateComputeStmt, updateCommunicationStmt)) {
        ServerLog::Error("Update wait time. Failed to update last data.");
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
            ServerLog::Error("Failed to update task info wait time");
            result = false;
            break;
        }
    }
    taskWaitTimeCache.clear();
    if (!EndTransaction()) {
        ServerLog::Error("Failed to end update task info wait time.");
        return false;
    }
    return result;
}

    std::unordered_map<std::string, std::string> DbTraceDataBase::QueryRankIdAndDeviceMap()
    {
        std::unordered_map<std::string, std::string> rankAndDeviceMap;
        sqlite3_stmt *stmt = nullptr;
        std::string sql;
        FileType type = DataBaseManager::Instance().GetFileType();
        if (type == FileType::MS_PROF || !CheckTableDataInvalid(TABLE_PYTORCH_INFO)) {
            return rankAndDeviceMap;
        } else if (type == FileType::PYTORCH) {
            sql = "SELECT DISTINCT deviceId, rankId FROM " + TABLE_PYTORCH_INFO;
        }
        int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        if (result != SQLITE_OK) {
            Server::ServerLog::Error("Failed to query rank id device map. Msg: ", sqlite3_errmsg(db), " ", result);
            sqlite3_finalize(stmt);
            return rankAndDeviceMap;
        }

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string deviceId = sqlite3_column_string(stmt, resultStartIndex);
            std::string rankId = sqlite3_column_string(stmt, resultStartIndex + 1);
            rankAndDeviceMap[rankId] = deviceId;
        }
        sqlite3_finalize(stmt);
        return rankAndDeviceMap;
    }

std::string DbTraceDataBase::GetDeviceId(const std::string &fileId)
{
    auto hostStr = QueryHostInfo();
    auto rankAndDeviceMap = QueryRankIdAndDeviceMap();
    std::string realRankId = fileId;
    if (!hostStr.empty() && StringUtil::StartWith(fileId, hostStr)) {
        realRankId = fileId.substr(hostStr.length());
    }
    if (rankAndDeviceMap.count(realRankId) > 0) {
        return rankAndDeviceMap[realRankId];
    }
    return realRankId;
}

std::string DbTraceDataBase::QueryHostInfo()
{
    return DbTraceDataBase::QueryHostInfoWithHostPath(hostPath);
}

std::string DbTraceDataBase::QueryHostInfoWithHostPath(const std::string &path)
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
        host = host + " ";
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
    host = CollectionTimeService::Instance().ComputeMarkHost(host, path, startTime, endTime);
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
            sql = "SELECT DISTINCT rankId FROM " + TABLE_PYTORCH_INFO;
        } else {
            sql = "SELECT DISTINCT id FROM " + TABLE_NPU_INFO;
        }
    }
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        Server::ServerLog::Error("Failed to get Statistic Num. Msg: ", sqlite3_errmsg(db), " ", result);
        sqlite3_finalize(stmt);
        rankIds.emplace_back(path);
        return rankIds;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string id = sqlite3_column_string(stmt, resultStartIndex);
        rankIds.emplace_back(id);
    }
    sqlite3_finalize(stmt);
    if (rankIds.empty()) {
        rankIds.emplace_back(path);
    }
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
        Server::ServerLog::Error("Failed to get Memory Data. Msg: ", sqlite3_errmsg(db), " ", result);
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
        ServerLog::Error("Need update depth. Failed to get result set.", stmt->GetErrorMessage());
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
        ServerLog::Error("Update depth. Failed to prepare sql.", sqlite3_errmsg(db));
        return;
    }
    auto resultSet = stmt->ExecuteQuery();
    if (resultSet == nullptr) {
        ServerLog::Error("Update depth. Failed to get result set.", stmt->GetErrorMessage());
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
                ServerLog::Error("Update depth. Failed to update data.");
                return;
            }
        }
        endList.clear();
    }
    if (!UpdateDepthList(updateStmt)) {
        ServerLog::Error("Update depth. Failed to update last data.");
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
            ServerLog::Error("Failed to update depth");
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
    this->hostPath = DbTraceDataBase::GetHostPath(dbPath);
    return Database::OpenDb(dbPath, clearAllTable) && QueryMetaVersion() && SetConfig() && InitStmt();
}

std::string DbTraceDataBase::GetHostPath(const std::string &filePath)
{
    if (std::empty(filePath)) {
        return "";
    }
    std::string leavePath = filePath;
    std::vector<std::string> pathList = FileUtil::SplitFilePath(leavePath);

    std::string result;
    // 查找第一个出现的以 "PROF_" 开头或以 "_ascend_pt" 或 "_ascend_ms" 结尾的部分
    for (const auto &fileStr : pathList) {
        if (StringUtil::StartWith(fileStr, "PROF_") ||
            StringUtil::EndWith(fileStr, "_ascend_pt") ||
            StringUtil::EndWith(fileStr, "_ascend_ms")) {
            return result;
        }
        result += fileStr + FILE_SEPARATOR;
    }
    // 不存在以 "PROF_" 开头或以 "_ascend_pt" 或 "_ascend_ms" 结尾的部分，则认为没找到 hostPath，返回空字符串
    return "";
}

void DbTraceDataBase::InitConnectionCats()
{
    if (CheckValueFromStatusInfoTable(CONNECTION_STATUS, FINISH_STATUS)) { // 已更新数据，跳过更新
        return;
    }
    if (ExecSql(DbSqlDefs::GetConnectionCatSql())) {
        UpdateValueIntoStatusInfoTable(CONNECTION_STATUS, FINISH_STATUS);
    }
}

std::string DbTraceDataBase::GetStringCacheValue(const std::string& path, const std::string& key)
{
    if (stringsCache.count(path) == 0 || stringsCache.at(path).count(key) == 0) {
        return key;
    }
    return stringsCache.at(path)[key];
}

void DbTraceDataBase::InitStringsCache()
{
    if (!stringsCache[path].empty()) {
        return;
    }
    auto sql = "select id, value from STRING_IDS";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Init strings cache. Failed to prepare sql.", sqlite3_errmsg(db));
        return;
    }
    auto result = stmt->ExecuteQuery();
    if (result == nullptr) {
        ServerLog::Error("Init strings cache. Failed to get result set.", stmt->GetErrorMessage());
        return;
    }
    while (result->Next()) {
        stringsCache[path].emplace(result->GetString("id"), result->GetString("value"));
    }
}

void DbTraceDataBase::InitMetaDataInfo()
{
    if (CheckTableExist(TABLE_META_DATA)) {
        std::string groupInfoStr = QueryValueFromMetaDataByName("parallel_group_info");
        auto groupInfoList = MetaDataParser::ParserParallelGroupInfoByText(groupInfoStr);
        MetaDataCacheManager::Instance().AddParallelGroupInfo(groupInfoList);
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
    auto isVersionChange = IsDatabaseVersionChange();
    if (!Database::SetConfig()) {
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

    if (isVersionChange) {
        if (CheckTableExist(TABLE_TASK)) {
            if (!CheckColumnExist(TABLE_TASK, "depth")) {
                ExecSql("alter table " + TABLE_TASK + " add depth integer;");
            }
            ExecSql(" create table if not exists OVERLAP_ANALYSIS (id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    " deviceId integer, startNs integer, endNs integer, type integer);");
        }
        if (isExistPytorch && !CheckColumnExist(TABLE_API, "depth")) {
            ExecSql("alter table " + TABLE_API + " add depth integer;");
        }
        if (isExistCann && !CheckColumnExist(TABLE_CANN_API, "depth")) {
            ExecSql("alter table " + TABLE_CANN_API + " add depth integer;");
        }
        if (isExistMstx && !CheckColumnExist(TABLE_MSTX_EVENTS, "depth")) {
            ExecSql("alter table " + TABLE_MSTX_EVENTS + " add depth integer;");
        }
        if (CheckTableExist(TABLE_COMPUTE_TASK_INFO) && !CheckColumnExist(TABLE_COMPUTE_TASK_INFO, "waitNs")) {
            ExecSql("alter table " + TABLE_COMPUTE_TASK_INFO + " add column waitNs INTEGER;");
        }
        if (CheckTableExist(TABLE_COMMUNICATION_OP) && !CheckColumnExist(TABLE_COMMUNICATION_OP, "waitNs")) {
            ExecSql("alter table " + TABLE_COMMUNICATION_OP + " add column waitNs INTEGER;");
        }
        for (const auto &status: DB_STATUS_LIST) {
            UpdateValueIntoStatusInfoTable(status, NOT_FINISH_STATUS);
        }
    }
    return ExecSql("PRAGMA case_sensitive_like=1;");
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
            ServerLog::Error("Failed to query host metadata, MetaType: ", typeName, " reason: ", e.What());
        }
    }
    DealHostMetadata(metaData, threadMap);
    return true;
}

void DbTraceDataBase::DealHostMetadata(std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData,
                                       std::map<std::string, std::vector<MetaDataDto>> &threadMap)
{
    uint64_t curPid = 0;
    std::unique_ptr<UnitTrack> process;
    for (auto &thread : threadMap) {
        uint64_t globalTid = NumberUtil::StringToUnsignedLongLong(thread.first);
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
        if (process.operator bool()) {
            process->children.emplace_back(std::move(threadUnit));
        }
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

std::string DbTraceDataBase::GetHcclOperateMetaData(const std::string &fileId)
{
    std::string sql = "with main as (select planeId, op.groupName, sids.value as groupNameValue from " +
        TABLE_COMMUNICATION_TASK_INFO + " info join " + TABLE_TASK + " task on task.globalTaskId = info.globalTaskId "
       " join COMMUNICATION_OP op on op.opId = info.opId "
       " left join STRING_IDS sids on op.groupName = sids.id where task.deviceId = ?) "
       " select 'Plane ' || planeId as name, groupName || '_' || planeId as tid, 0 as maxDepth, "
       " groupName, groupNameValue, planeId from main group by planeId, groupName union ";
    if (!TraceDatabaseHelper::IsDeviceIdUnique(fileId)) {
        // deviceId不唯一，走老逻辑
        sql += "select 'Group ' || ((row_number() over ()) -1) || ' Communication' as name, "
           " groupName || 'group' as tid, 0 as maxDepth, groupName, groupNameValue, -1 as planeId from main "
           " group by groupName order by groupName ASC, planeId ASC";
    } else {
        // deviceId唯一，走新逻辑
        sql += "SELECT 'Group ' || ( ( row_number() OVER () ) - 1 ) || ' Communication' AS name, "
           "groupName || 'group' AS tid, 0 AS maxDepth, groupName, sids.value as groupNameValue, - 1 AS planeId FROM " +
           TABLE_COMMUNICATION_OP + " op left join STRING_IDS sids on op.groupName = sids.id GROUP BY groupName "
           "order by groupName ASC, planeId ASC";
    }
    return sql;
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
                sql = GetHcclOperateMetaData(fileId);
                break;
            default:
                break;
        }
        auto stmt = CreatPreparedStatement();
        auto metaType = ENUM_TO_STR(type).value_or("");
        // 临时增加对 HCCL 的特殊处理，之后将 PROCESS_TYPE::HCCL 换成 Communication
        auto processName = type == PROCESS_TYPE::HCCL ? "Communication" : metaType;
        auto process = GenerateBaseUnitTrack("process", fileId, metaType, processName, metaType);
        try {
            auto resultSet = TraceDatabaseHelper::ExecuteQuery(stmt, sql, GetDeviceId(fileId));
            while (resultSet->Next()) {
                auto thread = GenerateBaseUnitTrack("thread", fileId, process->metaData.processId, "", metaType);
                std::string threadId = resultSet->GetString("tid");
                ProcessThreadUnit(process, resultSet, thread, threadId, type);
            }
        } catch (DatabaseException &e) {
            ServerLog::Error("Query operate metadata, MetaType: ", metaType, " reason: ", e.What());
            return false;
        }
        UpdataCommucationThreadName(type, process);
        if (!process->children.empty()) {
            metaData.emplace_back(std::move(process));
        }
    }
    return true;
}

// 这是一个补丁，后续需要修改掉
void DbTraceDataBase::UpdataCommucationThreadName(const PROCESS_TYPE &type,
    std::unique_ptr<Protocol::UnitTrack> &process) const
{
    const std::string suffix = "group";
    if (!std::empty(metaVersion) && !StringUtil::StartWith(metaVersion, "1.0") && type == PROCESS_TYPE::HCCL) {
        for (auto &item : process->children) {
            if (StringUtil::StartWith(item->metaData.threadName, "Group") &&
                StringUtil::EndWith(item->metaData.threadName, "Communication")) {
                std::string threadId = item->metaData.threadId;
                // 此时 threadId 是 groupName + "group", 形如 "409group"
                std::string groupName = threadId.substr(0, threadId.size() - suffix.size());
                item->metaData.threadName = "Group " + stringsCache.at(path)[groupName] + " Communication";
            }
        }
    }
}

void DbTraceDataBase::ProcessThreadUnit(std::unique_ptr<Protocol::UnitTrack> &process,
    std::unique_ptr<SqliteResultSet> &resultSet, std::unique_ptr<Protocol::UnitTrack> &thread,
    const std::string &threadId, const PROCESS_TYPE &type) const
{
    const static std::string WRONG_THREAD_ID = std::to_string(UINT32_MAX);
    // hccl的plane泳道约定一个异常数据不做展示
    if (threadId.find(WRONG_THREAD_ID) != std::string::npos) {
        return;
    }
    // 在 metaVersion 版本高于 '1.0' 的情况下，type == PROCESS_TYPE::HCCL 时
    if (!std::empty(metaVersion) && !StringUtil::StartWith(metaVersion, "1.0") && type == PROCESS_TYPE::HCCL) {
        const std::string groupNameValue = resultSet->GetString("groupNameValue");
        const std::string threadName = resultSet->GetString("name");
        if (!StringUtil::StartWith(threadName, "Plane") &&
            TraceDatabaseHelper::IsValidHCCLGroupNameValue(groupNameValue)) {
            thread->metaData.groupNameValue = groupNameValue;
        }
    }
    thread->metaData.threadId = threadId;
    thread->metaData.threadName = resultSet->GetString("name");
    thread->metaData.maxDepth = resultSet->GetInt32("maxDepth") + 1;
    process->children.emplace_back(std::move(thread));
}

bool DbTraceDataBase::QueryCounterMetadata(const std::string &fileId,
    std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData)
{
    PROCESS_TYPE types[] = {PROCESS_TYPE::HBM, PROCESS_TYPE::LLC, PROCESS_TYPE::SAMPLE_PMU,
                            PROCESS_TYPE::NIC, PROCESS_TYPE::ROCE, PROCESS_TYPE::ROH};
    for (const auto &type : types) {
        auto tableName = ENUM_TO_STR(type).value_or("");
        if (!CheckTableExist(tableName)) {
            ServerLog::Info("Query counter metadata failed, table ", tableName, " Not Exist.");
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
            ServerLog::Error("Query counter metadata failed!.");
            return false;
        }
        auto resultSet = stmt->ExecuteQuery(GetDeviceId(fileId));
        if (resultSet == nullptr) {
            ServerLog::Error("Query counter metadata. Failed to get result set.", stmt->GetErrorMessage());
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
            ServerLog::Info("Generate counter metadata failed, table ", typeName, " Not Exist.");
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
// LCOV_EXCL_BR_START
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
// LCOV_EXCL_BR_STOP

bool DbTraceDataBase::SearchAllSlicesDetails(const Protocol::SearchAllSliceParams &params,
    Protocol::SearchAllSlicesBody &body, uint64_t minTimestamp)
{
    uint64_t count = 0;
    uint64_t offset = (params.current - 1) * params.pageSize;
    const std::string &sql = TraceDatabaseHelper::GetSearchAllSlicesDetailsSql(params.isMatchExact, params.isMatchCase,
        params.order, params.orderBy, params.rankId);
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Query slice name failed!.");
        return false;
    }
    auto resultSet =
        stmt->ExecuteQuery(params.searchContent, minTimestamp, GetDeviceId(params.rankId), params.pageSize, offset);
    if (resultSet == nullptr) {
        ServerLog::Error("search All slices details. Failed to get result set.", stmt->GetErrorMessage());
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
        searchAllSlice.rankId = deviceId.empty() ? path : params.rankId;
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
    count += SearchSliceNameCount(searchCountParams, {});
    searchCountParams.rankId = QueryHostInfo() + "Host";
    count += SearchSliceNameCount(searchCountParams, {});
    body.count = count;
    return true;
}

bool DbTraceDataBase::SearchAllSlicesDetails(const Protocol::SearchAllSliceParams &params,
    Protocol::SearchAllSlicesBody &body, uint64_t minTimestamp, const std::vector<TrackQuery> &trackQueryVec)
{
    if (trackQueryVec.empty() && !params.metadataList.empty()) {
        return true;
    }
    if (trackQueryVec.empty()) {
        return SearchAllSlicesDetails(params, body, minTimestamp);
    }
    std::string sql = TraceDatabaseHelper::GetLockRangeSql(params, trackQueryVec);
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Query slice name failed!.");
        return false;
    }
    TraceDatabaseHelper::SearchAllSliceWithLockRangeBindStmt(params, trackQueryVec, stmt, GetDeviceId(params.rankId));
    uint64_t count = 0;
    uint64_t offset = (params.current - 1) * params.pageSize;
    stmt->BindParams(params.pageSize, offset);
    auto resultSet = stmt->ExecuteQuery();
    if (resultSet == nullptr) {
        ServerLog::Error("search All slices details. Failed to get result set.", stmt->GetErrorMessage());
        return false;
    }
    while (resultSet->Next()) {
        Protocol::SearchAllSlices searchAllSlice{};
        searchAllSlice.name = resultSet->GetString("value");
        searchAllSlice.timestamp = resultSet->GetUint64("timestamp");
        searchAllSlice.duration = resultSet->GetUint64("endTime") - searchAllSlice.timestamp; // 保证 endTime > timestamp
        searchAllSlice.timestamp -= minTimestamp; // 业务上 minTimestamp 是最小时间，保证 timestamp > minTimestamp
        searchAllSlice.id = resultSet->GetString("id");
        searchAllSlice.tid = resultSet->GetString("tid");
        searchAllSlice.pid = resultSet->GetString("pid");
        searchAllSlice.depth = resultSet->GetUint64("depth");
        auto deviceId = resultSet->GetString("deviceId");
        searchAllSlice.rankId = deviceId.empty() ? path : params.rankId;
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
    count += SearchSliceNameCount(searchCountParams, trackQueryVec);
    searchCountParams.rankId = QueryHostInfo() + "Host";
    count += SearchSliceNameCount(searchCountParams, trackQueryVec);
    body.count = count;
    return true;
}

// LCOV_EXCL_BR_START
bool DbTraceDataBase::QueryAffinityOptimizer(const Protocol::KernelDetailsParams &params, const std::string &optimizers,
    std::vector<Protocol::ThreadTraces> &data, uint64_t minTimestamp)
{
    if (!CheckTableExist(TABLE_API)) {
        ServerLog::Warn("The PYTORCH_API table isn't exist.");
        return false;
    }
    std::string sql = TextSqlConstant::QueryAffinityOptimizerDbSql(optimizers, params.orderBy, params.order);
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
// LCOV_EXCL_BR_STOP
bool DbTraceDataBase::QueryAICpuOpCanBeOptimized(const Protocol::KernelDetailsParams &params,
    const std::vector<std::string> &replace, const std::map<std::string, Timeline::AICpuCheckDataType> &dataType,
    std::vector<Protocol::KernelBaseInfo> &data, uint64_t minTimestamp)
{
    std::string sql = TextSqlConstant::GenerateAICpuQueryDbSql(replace, params, dataType);
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Failed to prepare sql for AICpuOpCanBeOptimized.");
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
    Protocol::UnitThreadsOperatorsBody &responseBody, uint64_t minTimestamp,
    const std::vector<std::string> &trackIdList)
{
    if (!StringUtil::CheckSqlValid(requestParams.orderBy)) {
        ServerLog::Error("There is an SQL injection attack in request parameter orderBy. Error param: % ",
                         requestParams.orderBy);
        return false;
    }
    for (const auto& tidItem : requestParams.tid) {
        if (!StringUtil::CheckSqlValid(tidItem)) {
            ServerLog::Error("There is an SQL injection attack in track id. Error param: % ", tidItem);
            return false;
        }
    }
    std::string orderBy = " ORDER BY " + requestParams.orderBy;
    orderBy.append(requestParams.order == "descend" ? " DESC " : " ASC ");
    std::string orderByAndPage = orderBy + " limit ? offset ?";
    auto stmt = CreatPreparedStatement();
    std::unique_ptr <SqliteResultSet> resultSet;
    try {
        resultSet = TraceDatabaseHelper::QueryThreadSameOperatorsDetails(stmt, requestParams,
            GetDeviceId(requestParams.rankId), minTimestamp, orderByAndPage);
    } catch (DatabaseException &e) {
        ServerLog::Error("Query thread same operators details fail, ", e.What());
        return false;
    }
    while (resultSet->Next()) {
        int col = resultStartIndex;
        Protocol::SameOperatorsDetails sameOperatorsDetail{};
        sameOperatorsDetail.timestamp = resultSet->GetUint64(col++);
        sameOperatorsDetail.duration = resultSet->GetUint64(col++);
        sameOperatorsDetail.depth = resultSet->GetUint64(col++);
        sameOperatorsDetail.id = resultSet->GetString(col++);
        sameOperatorsDetail.tid = resultSet->GetString(col++);
        if (sameOperatorsDetail.tid.empty()) {  // some process not have tid, use request.tid[0], ex:pytorch
            sameOperatorsDetail.tid = requestParams.tid[0];
        }
        responseBody.sameOperatorsDetails.emplace_back(sameOperatorsDetail);
    }
    responseBody.currentPage = requestParams.current;
    responseBody.pageSize = requestParams.pageSize;
    return true;
}

// LCOV_EXCL_BR_START
bool DbTraceDataBase::QueryAclnnOpCountExceedThreshold(const KernelDetailsParams &params, uint64_t threshold,
    std::vector<Protocol::KernelBaseInfo> &data, uint64_t minTimestamp)
{
    auto stmt = CreatPreparedStatement(TraceDatabaseSqlConst::GenerateAclnnQueryDbSql(params));
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
// LCOV_EXCL_BR_STOP
bool DbTraceDataBase::QueryAffinityAPIData(const Protocol::KernelDetailsParams &params,
    const std::set<std::string> &pattern, uint64_t minTimestamp, std::map<uint64_t,
    std::vector<Protocol::FlowLocation>> &data, std::map<uint64_t, std::vector<uint32_t>> &indexes)
{
    auto stmt = CreatPreparedStatement(QUERY_AFFINITY_API_DB_SQL);
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
// LCOV_EXCL_BR_START
bool DbTraceDataBase::QueryFuseableOpData(const KernelDetailsParams &params, const FuseableOpRule &rule,
    std::vector<Protocol::FlowLocation> &data, uint64_t minTimestamp)
{
    std::string sql = TextSqlConstant::GenerateFuseableOpFilterDbSql(params, rule);
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
// LCOV_EXCL_BR_STOP
bool DbTraceDataBase::QueryEventsViewData(const Protocol::EventsViewParams &params, Protocol::EventsViewBody &body,
    uint64_t minTimestamp)
{
    auto stmt = CreatPreparedStatement();
    if (stmt == nullptr) {
        return false;
    }
    return TraceDatabaseHelper::QueryEventsViewData4Db(stmt, params, body, minTimestamp, GetDeviceId(params.rankId));
}
std::vector<Protocol::SimpleSlice> DbTraceDataBase::QueryThreadByPid(const Metadata &metaData,
    uint64_t startTime, uint64_t endTime, const std::string &rankId, std::map<std::string, uint64_t> &selfTimeKeyValue)
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
            simpleSlice.timestamp = resultSet->GetUint64(col++);
            simpleSlice.duration = resultSet->GetUint64(col++);
            simpleSlice.endTime = resultSet->GetUint64(col++);
            simpleSlice.name = stringsCache.at(path)[resultSet->GetString(col++)];
            simpleSlice.depth = resultSet->GetUint32(col++);
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

bool DbTraceDataBase::QueryFwdBwdDataByFlow(const std::string &rankId, uint64_t offset,
    const Protocol::ExtremumTimestamp &range, std::vector<Protocol::ThreadTraces> &fwdBwdData)
{
    std::vector<std::string> tableList = {TABLE_API, TABLE_CONNECTION_CATS, TABLE_CONNECTION_IDS, TABLE_ENUM_API_TYPE};
    if (!CheckTablesExist(tableList)) {
        ServerLog::Error("Failed to check dependent table for query fwdbwd data in the DB scenario.");
        return false;
    }
    std::unique_lock<std::recursive_mutex> lock(mutex);
    if (!ExecSql(CREATE_TEMP_FWDBWD_FLOW_TABLE_DB_SQL)) {
        ServerLog::Error("Failed to create temp fwdbwd table in the DB scenario.");
        return false;
    }
    auto stmt = CreatPreparedStatement(QUERY_FWDBWD_FLOW_DATA_SQL);
    if (stmt == nullptr) {
        ServerLog::Error("Failed to prepare sql for query fwd/bwd data by flow in the DB scenario.");
        return false;
    }
    return TraceDatabaseHelper::ExecuteQueryFwdBwdDataByFlow(std::move(stmt), rankId, offset, range, fwdBwdData);
}

bool DbTraceDataBase::QueryP2PCommunicationOpData(const std::string &rankId, uint64_t offset,
    const ExtremumTimestamp &range, std::vector<Protocol::ThreadTraces> &p2pOpData)
{
    auto stmt = CreatPreparedStatement(QUERY_P2P_COMMUNICATION_OP_DB_SQL);
    if (stmt == nullptr) {
        ServerLog::Error("Failed to prepare sql for query p2p communication op data in the DB scenario.");
        return false;
    }
    return TraceDatabaseHelper::ExecuteQueryP2POpData(std::move(stmt), rankId, offset, range, p2pOpData);
}
}