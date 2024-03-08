/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */


#include "DbTraceDataBase.h"
#include "ServerLog.h"
#include "TraceTime.h"
#include "TableDefs.h"
#include "TraceDatabaseHelper.h"
#include "CommonDefs.h"
#include "DataBaseManager.h"

namespace Dic::Module::FullDb {
using namespace Server;
static std::map<int64_t, std::string> stringsCache;
bool DbTraceDataBase::QueryThreadTraces(const Protocol::UnitThreadTracesParams &requestParams,
    Protocol::UnitThreadTracesBody &responseBody, uint64_t minTimestamp, int64_t traceId)
{
    if (requestParams.timePerPx == 0) {
        ServerLog::Error("QueryThreadTraces. timePerPx is zero.");
        return false;
    }
    auto stmt = CreatPreparedStatement();
    std::unique_ptr<SqliteResultSet> resultSet;
    // rank = round(time / (totalTime / pixel))
    try {
        resultSet = TraceDatabaseHelper::QueryThreadTraces(stmt, requestParams, minTimestamp);
    } catch (DatabaseException &e) {
        ServerLog::Error("QueryThreadTraces Fail, ", e.What());
        return false;
    }
    if (resultSet == nullptr) {
        ServerLog::Error("QueryThreadTraces. Failed to get result set.", stmt->GetErrorMessage());
        return false;
    }
    std::vector<Protocol::RowThreadTrace> rowThreadTraceVec;
    while (resultSet->Next()) {
        Protocol::RowThreadTrace rowThreadTrace {};
        rowThreadTrace.id = resultSet->GetInt64("id");
        rowThreadTrace.startTime = resultSet->GetInt64("start_time");
        rowThreadTrace.duration = resultSet->GetInt64("duration");
        rowThreadTrace.name = stringsCache.at(resultSet->GetInt64("name"));
        rowThreadTrace.depth = resultSet->GetInt32("depth");
        rowThreadTraceVec.emplace_back(std::move(rowThreadTrace));
    }
    std::map<int64_t, std::vector<Protocol::ThreadTraces>> threadTracesMap;
    for (auto &item : rowThreadTraceVec) {
        Protocol::ThreadTraces threadTraces {};
        threadTraces.name = item.name;
        threadTraces.duration = item.duration;
        threadTraces.startTime = item.startTime;
        threadTraces.endTime = item.startTime + item.duration;
        threadTraces.depth = item.depth;
        threadTraces.threadId = requestParams.threadId;
        while (responseBody.data.size() <= item.depth) {
            responseBody.data.emplace_back();
        }
        responseBody.data[item.depth].emplace_back(std::move(threadTraces));
    }
    return true;
}

bool DbTraceDataBase::QueryThreads(const Protocol::UnitThreadsParams &requestParams,
    Protocol::UnitThreadsBody &responseBody, uint64_t minTimestamp, int64_t traceId)
{
    uint64_t startTime = requestParams.startTime + minTimestamp;
    uint64_t endTime = requestParams.endTime + minTimestamp;
    auto stmt = CreatPreparedStatement();
    std::vector<Protocol::SimpleSlice> simpleSliceVec;
    try {
        auto resultSet = TraceDatabaseHelper::QueryThreadsByPid(stmt, requestParams, minTimestamp);
        while (resultSet->Next()) {
            int col = resultStartIndex;
            Protocol::SimpleSlice simpleSlice {};
            simpleSlice.timestamp = resultSet->GetInt64(col++);
            simpleSlice.duration = resultSet->GetInt64(col++);
            simpleSlice.endTime = resultSet->GetInt64(col++);
            simpleSlice.name = stringsCache.at(resultSet->GetInt64(col++));
            simpleSlice.depth = resultSet->GetInt32(col++);
            simpleSliceVec.emplace_back(simpleSlice);
        }
    } catch (DatabaseException &e) {
        ServerLog::Error("QueryThreads Fail, ", e.What());
        return false;
    }

    // process data
    if (simpleSliceVec.empty()) {
        responseBody.emptyFlag = true;
        return true;
    }

    std::map<std::string, uint64_t> selfTimeKeyValue;
    TraceDatabaseHelper::CalculateSelfTime(simpleSliceVec, selfTimeKeyValue, startTime, endTime);
    std::vector<Protocol::SimpleSlice> nRows = TraceDatabaseHelper::ThreadsInfoFilter(simpleSliceVec,
                                                                                      startTime, endTime);
    TraceDatabaseHelper::ReduceThread(nRows, selfTimeKeyValue, responseBody);
    return true;
}

bool DbTraceDataBase::QueryThreadDetail(const Protocol::ThreadDetailParams &requestParams,
                                        Protocol::UnitThreadDetailBody &responseBody, uint64_t minTimestamp,
                                        int64_t trackId)
{
    auto stmt = CreatPreparedStatement();
    std::unique_ptr<SqliteResultSet> resultSet;
    try {
        resultSet = TraceDatabaseHelper::QueryThreadDetail(stmt, requestParams, minTimestamp);
    } catch (DatabaseException &e) {
        ServerLog::Error("QueryThreadDetail Fail, ", e.What());
        return false;
    }

    std::vector<SliceDto> sliceDtoVec;
    while (resultSet->Next()) {
        int col = resultStartIndex;
        SliceDto sliceDto {};
        sliceDto.id = resultSet->GetInt64(col++);
        sliceDto.timestamp = resultSet->GetInt64(col++);
        sliceDto.duration = resultSet->GetInt64(col++);
        sliceDto.depth = resultSet->GetInt32(col++);
        sliceDto.name = stringsCache.at(resultSet->GetInt64(col++));
        sliceDtoVec.emplace_back(sliceDto);
    }

    if (sliceDtoVec.size() != 1) {
        ServerLog::Error("select slice error!");
        return false;
    }
    uint64_t selfTime = sliceDtoVec.at(0).duration;
    std::vector<uint64_t> nextDepthResult;
    QueryDurationFromTaskByTimeRange(requestParams, sliceDtoVec.at(0), nextDepthResult, trackId);
    if (nextDepthResult.empty()) {
        selfTime = 0;
    } else {
        for (const auto &item : nextDepthResult) {
            selfTime -= item;
        }
    }
    responseBody.emptyFlag = false;
    responseBody.data.selfTime = selfTime;
    responseBody.data.title = sliceDtoVec[0].name;
    responseBody.data.duration = sliceDtoVec[0].duration;
    responseBody.data.cat = sliceDtoVec[0].cat;
    uint64_t id = sliceDtoVec.at(0).id;
    if (!QueryComputeTaskInfoById(id, responseBody) || !QueryCommunicationTaskInfoById(id, responseBody)) {
        ServerLog::Error("QueryComputeTaskInfoById or QueryCommunicationTaskInfoById is error!");
        return false;
    }
    return true;
}

bool DbTraceDataBase::QueryComputeTaskInfoById(int64_t id, Protocol::UnitThreadDetailBody &responseBody)
{
    auto stmt = CreatPreparedStatement();
    std::unique_ptr<SqliteResultSet> resultSet;
    try {
        resultSet = TraceDatabaseHelper::QueryComputeTaskInfoById(stmt, id);
    } catch (DatabaseException &e) {
        ServerLog::Error("QueryComputeTaskInfoById Fail, ", e.What());
        return false;
    }
    while (resultSet->Next()) {
        int col = resultStartIndex;
        responseBody.data.inputShapes = stringsCache.at(resultSet->GetInt64(col++));
        responseBody.data.inputDataTypes = stringsCache.at(resultSet->GetInt64(col++));
        responseBody.data.inputFormats = stringsCache.at(resultSet->GetInt64(col++));
        responseBody.data.outputShapes = stringsCache.at(resultSet->GetInt64(col++));
        responseBody.data.outputDataTypes = stringsCache.at(resultSet->GetInt64(col++));
        responseBody.data.outputFormats = stringsCache.at(resultSet->GetInt64(col++));
    }
    return true;
}

bool DbTraceDataBase::QueryCommunicationTaskInfoById(int64_t id, Protocol::UnitThreadDetailBody &responseBody)
{
    auto stmt = CreatPreparedStatement();
    std::unique_ptr<SqliteResultSet> resultSet;
    try {
        resultSet = TraceDatabaseHelper::QueryCommunicationTaskInfoById(stmt, id);
    } catch (DatabaseException &e) {
        ServerLog::Error("QueryCommunicationTaskInfoById Fail, ", e.What());
        return false;
    }
    while (resultSet->Next()) {
        int col = resultStartIndex;
        ArgsDto argsDto;
        argsDto.taskType = stringsCache.at(resultSet->GetInt64(col++));
        argsDto.planeId = resultSet->GetInt64(col++);
        argsDto.groupName = stringsCache.at(resultSet->GetInt64(col++));
        argsDto.notifyId = resultSet->GetInt64(col++);
        argsDto.rdmaType = stringsCache.at(resultSet->GetInt64(col++));
        argsDto.srcRank = resultSet->GetInt64(col++);
        argsDto.dstRank = resultSet->GetInt64(col++);
        argsDto.transportType = stringsCache.at(resultSet->GetInt64(col++));
        argsDto.size = resultSet->GetInt64(col++);
        argsDto.dataType = stringsCache.at(resultSet->GetInt64(col++));
        argsDto.linkType = stringsCache.at(resultSet->GetInt64(col++));
        argsDto.opId = resultSet->GetInt64(col++);
        responseBody.data.args = ArgsDtoToJsonStr(argsDto);
    }
    return true;
}

std::string DbTraceDataBase::ArgsDtoToJsonStr(const ArgsDto& argsDto)
{
    std::string jsonStr;
    jsonStr.append("{")
    .append("\"planeId\":")
    .append(std::to_string(argsDto.planeId))
    .append(",\"groupName\":")
    .append(argsDto.groupName)
    .append(",\"notifyId\":")
    .append(std::to_string(argsDto.notifyId))
    .append(",\"rdmaType\":")
    .append(argsDto.rdmaType)
    .append(",\"srcRank\":")
    .append(std::to_string(argsDto.srcRank))
    .append(",\"dstRank\":")
    .append(std::to_string(argsDto.dstRank))
    .append(",\"transportType\":")
    .append(argsDto.transportType)
    .append(",\"size\":")
    .append(std::to_string(argsDto.size))
    .append(",\"dataType\":")
    .append(argsDto.dataType)
    .append(",\"linkType\":")
    .append(argsDto.linkType)
    .append(",\"opId\":")
    .append(std::to_string(argsDto.opId))
    .append("}");
    return jsonStr;
}

bool DbTraceDataBase::QueryFlowDetail(const Protocol::UnitFlowParams &requestParams,
                                      Protocol::UnitFlowBody &responseBody, uint64_t minTimestamp)
{
    return false;
}

bool DbTraceDataBase::QueryUnitsMetadata(const std::string &fileId,
                                         std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData)
{
    QueryAscendHardwareMetadata(fileId, metaData);
    QueryHcclMetadata(fileId, metaData);
    QueryCounterMetadata(fileId, metaData);
    GenerateCounterMetadata(fileId, metaData);
    return false;
}

bool DbTraceDataBase::QueryExtremumTimestamp(uint64_t &min, uint64_t &max)
{
    return false;
}

bool DbTraceDataBase::QueryFlowName(const Protocol::UnitFlowNameParams &requestParams,
                                    Protocol::UnitFlowNameBody &responseBody, uint64_t minTimestamp,
                                    int64_t trackId)
{
    return false;
}

int DbTraceDataBase::SearchSliceNameCount(const Protocol::SearchCountParams &params)
{
    std::string sql;
    if (strcmp(params.rankId.c_str(), "Host") == 0) {
        sql = "with ids as (select id from STRING_IDS where value like '%'||?||'%') "
              "SELECT count(1),? as id FROM API join ids on id = API.name";
    } else {
        sql = "with ids as (select id from STRING_IDS where value like '%'||?||'%'), "
              "     tasks as (select globalTaskId, taskType from TASK where deviceId = ?), "
              "     com as (select opId, info.globalTaskId, name from COMMUNICATION_TASK_INFO info join tasks "
              " on  info.globalTaskId = tasks.globalTaskId), "
              "     compute as (select info.globalTaskId, name from COMPUTE_TASK_INFO info join tasks "
              " on  info.globalTaskId = tasks.globalTaskId) "
              "select count(1) from ( "
              "    select coalesce(com.name, compute.name, main.taskType) as name from tasks main "
              "         left join com on com.globalTaskId = main.globalTaskId "
              "         left join compute on compute.globalTaskId = main.globalTaskId "
              "    union ALL select name from com "
              "    union ALL select opName as name from COMMUNICATION_OP op join (select opId from com group by opId) a"
              " on op.opId = a.opId "
              ") allNames join ids on id = allNames.name;";
    }
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QuerySliceNameCount failed!.");
        return 0;
    }
    auto resultSet = stmt->ExecuteQuery(params.searchContent, params.rankId);
    if (resultSet->Next()) {
        return resultSet->GetInt32(resultStartIndex);
    }
    return 0;
}

bool DbTraceDataBase::QueryFlowCategoryList(std::vector<std::string> &categories)
{
    return false;
}

bool DbTraceDataBase::SearchSliceName(const std::string &name, int index, uint64_t minTimestamp,
                                      Protocol::SearchSliceBody &responseBody)
{
    std::string sql;
    if (strcmp(responseBody.rankId.c_str(), "Host") == 0) {
        sql = "with ids as (select id from STRING_IDS where value like '%'||?||'%') "
              "SELECT globalTid as pid, type as tid, startNs - ? as startTime,endNs - startNs as duration,depth,? as id"
              " FROM API join ids on id = API.name "
              " ORDER BY startNs LIMIT 1 OFFSET ?";
    } else {
        sql = "with ids as (select id from STRING_IDS where value like '%'||?||'%'), minTime as (select ? as value),\n"
          "tasks as (select globalTaskId, taskType, 0 as pid, streamId as tid, startNs - minTime.value as startTime,"
          " endNs - startNs as duration,depth from TASK join minTime where deviceId = ? ORDER BY startTime),\n"
          "com as (select opId, info.globalTaskId, 1 as pid, planeId as tid, startTime, duration, 0 as depth, name "
          " from COMMUNICATION_TASK_INFO info join tasks on  info.globalTaskId=tasks.globalTaskId ORDER BY startTime), "
          "compute as (select info.globalTaskId, name from COMPUTE_TASK_INFO info join tasks on "
          " info.globalTaskId=tasks.globalTaskId) select * from (\n"
          "    select coalesce(com.name, compute.name, main.taskType) as name, main.pid, main.tid, main.startTime,"
          " main.duration, main.depth from tasks main\n"
          "        left join com on com.globalTaskId = main.globalTaskId\n"
          "        left join compute on compute.globalTaskId = main.globalTaskId\n"
          "    union ALL select name, pid, tid, startTime, duration, depth from com\n"
          "    union ALL select opName as name,1 as pid, groupName as tid, startNs - minTime.value as startTime,"
          " endNs - startNs as duration, 0 as depth\n"
          "        from COMMUNICATION_OP op join minTime join (select opId from com group by opId) a "
          " on op.opId = a.opId ORDER BY startTime\n"
          ") allNames join ids on id = allNames.name LIMIT 1 OFFSET ?";
    }
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QuerySliceName failed!.");
        return false;
    }
    auto resultSet = stmt->ExecuteQuery(name, minTimestamp, responseBody.rankId, index);
    if (!resultSet->Next()) {
        return false;
    }
    responseBody.pid = resultSet->GetString("pid");
    responseBody.tid = resultSet->GetString("tid");
    responseBody.startTime = resultSet->GetUint64("startTime");
    responseBody.duration = resultSet->GetUint64("duration");
    responseBody.depth = resultSet->GetInt32("depth");
    return true;
}

bool DbTraceDataBase::QueryFlowCategoryEvents(Protocol::FlowCategoryEventsParams &params, uint64_t minTimestamp,
                                              std::vector<std::unique_ptr<Protocol::FlowEvent>> &flowDetailList)
{
    return false;
}

bool DbTraceDataBase::QueryUnitCounter(Protocol::UnitCounterParams &params, uint64_t minTimestamp,
                                       std::vector<Protocol::UnitCounterData> &dataList)
{
    auto stmt = CreatPreparedStatement();
    std::unique_ptr<SqliteResultSet> resultSet;
    try {
        resultSet = TraceDatabaseHelper::QueryUnitCounter(stmt, params, minTimestamp);
    } catch (DatabaseException &e) {
        ServerLog::Error("QueryUnitCounter Fail, ", e.What());
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
    int index = bindStartIndex;
    if (!requestParams.stepId.empty() && requestParams.stepId != "ALL") {
        stepCondition.append(" and streamId =? ");
    }
    std::string sql = "SELECT sum(endNs - startNs) as duration, TASKTYPE.value as acceleratorCore "
                      "  FROM COMPUTE_TASK_INFO"
                      "     JOIN TASK ON COMPUTE_TASK_INFO.globalTaskId = TASK.globalTaskId "
                      "     JOIN STRING_IDS AS TASKTYPE ON TASKTYPE.id = COMPUTE_TASK_INFO.taskType"
                      " WHERE accelerator_core in ('AI_CPU','AI_CORE',"
                      " 'AI_VECTOR_CORE', 'MIX_AIC', 'MIX_AIV', 'FFTS_PLUS') "
                      + stepCondition +
                      " GROUP BY accelerator_core";
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        Server::ServerLog::Error("QueryComputeStatisticsData failed!. ", sqlite3_errmsg(db));
        return false;
    }
    if (!requestParams.stepId.empty() && requestParams.stepId != "ALL") {
        sqlite3_bind_text(stmt, index, requestParams.stepId.c_str(),
                          requestParams.stepId.length(), SQLITE_TRANSIENT);
    }
    double totalDuration = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Protocol::SummaryStatisticsItem item;
        int col = resultStartIndex;
        item.duration = static_cast<double>(sqlite3_column_int64(stmt, col++));
        item.acceleratorCore = sqlite3_column_string(stmt, col++);
        totalDuration +=  item.duration;
        responseBody.summaryStatisticsItemList.push_back(item);
    }
    for (auto &item: responseBody.summaryStatisticsItemList) {
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

bool DbTraceDataBase::QueryPythonViewData(const Protocol::SystemViewParams &requestParams,
                                          Protocol::SystemViewBody &responseBody)
{
    return false;
}

bool DbTraceDataBase::QueryKernelDetailData(const Protocol::KernelDetailsParams &requestParams,
                                            Protocol::KernelDetailsBody &responseBody, uint64_t minTimestamp)
{
    uint64_t offset = (requestParams.current - 1) * requestParams.pageSize;
    std::string sql = GetKernelDetailSql(requestParams);
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        Server::ServerLog::Error("QueryKernelDetailData, fail to prepare sql.");
        return false;
    }
    if (!requestParams.coreType.empty()) {
        stmt->BindParams(requestParams.coreType);
    }
    std::string searchName = "%" + requestParams.searchName + "%";
    auto resultSet = stmt->ExecuteQuery(searchName, requestParams.pageSize, offset);
    SetKernelDetail(std::move(resultSet), minTimestamp, responseBody);
    responseBody.pageSize = requestParams.pageSize;
    responseBody.currentPage = requestParams.current;
    const std::vector<std::string> cores = QueryCoreType();
    responseBody.acceleratorCoreList = cores;
    responseBody.count = QueryTotalKernel(requestParams.coreType, searchName);
    return true;
}

std::string DbTraceDataBase::GetKernelDetailSql(const Protocol::KernelDetailsParams &requestParams)
{
    std::string orderBy;
    std::string coreTypes;
    if (requestParams.order == "descend") {
        orderBy = " order by " + requestParams.orderBy + " DESC";
    } else {
        orderBy = " order by " + requestParams.orderBy + " ASC";
    }
    if (!requestParams.coreType.empty()) {
        coreTypes = " AND accelerator_core = ? ";
    }
    uint64_t offset = (requestParams.current - 1) * requestParams.pageSize;
    std::string sql = "SELECT name, op_type as type, accelerator_core AS acceleratorCore, start_time AS startTime, "
                      "duration, wait_time as waitTime, block_dim AS blockDim, input_shapes AS inputShapes, "
                      "input_data_types AS inputDataTypes, input_formats AS inputFormats, "
                      "output_shapes AS outputShapes, output_data_types AS outputDataTypes, "
                      "output_formats AS outputFormats "
                      " FROM ("
                      " SELECT  NAME.value AS name,  OPTYPE.value AS op_type,"
                      "     TASKTYPE.value as accelerator_core, startNs as start_time, endNs - startNs as duration,"
                      "     block_dim, 0 as wait_time,"
                      "     INPUTSHAPES.value as input_shapes, INPUTDATATYPES.value as input_data_types, "
                      "     INPUTFORMATS.value as input_formats, OUTPUTSHAPES.value as output_shapes, "
                      "     OUTPUTDATATYPES.value as output_data_types, OUTPUTFORMATS.value as output_formats "
                      "   FROM " + TABLE_COMPUTE_TASK_INFO +
                      "   JOIN TASK ON COMPUTE_TASK_INFO.globalTaskId = TASK.globalTaskId "
                      "   JOIN STRING_IDS AS NAME ON NAME.id = COMPUTE_TASK_INFO.name"
                      "   JOIN STRING_IDS AS OPTYPE ON OPTYPE.id = COMPUTE_TASK_INFO.opType"
                      "   JOIN STRING_IDS AS TASKTYPE ON TASKTYPE.id = COMPUTE_TASK_INFO.taskType"
                      "   JOIN STRING_IDS AS INPUTSHAPES ON INPUTSHAPES.id = COMPUTE_TASK_INFO.inputShapes"
                      "   JOIN STRING_IDS AS INPUTDATATYPES ON INPUTDATATYPES.id = COMPUTE_TASK_INFO.inputDataTypes"
                      "   JOIN STRING_IDS AS INPUTFORMATS ON INPUTFORMATS.id = COMPUTE_TASK_INFO.inputFormats"
                      "   JOIN STRING_IDS AS OUTPUTSHAPES ON OUTPUTSHAPES.id = COMPUTE_TASK_INFO.outputShapes"
                      "   JOIN STRING_IDS AS OUTPUTDATATYPES ON OUTPUTDATATYPES.id = COMPUTE_TASK_INFO.outputDataTypes"
                      "   JOIN STRING_IDS AS OUTPUTFORMATS ON OUTPUTFORMATS.id = COMPUTE_TASK_INFO.outputFormats"
                      " )subquery "
                      "where 1=1 and name LIKE ? " + coreTypes + orderBy + " limit ? offset ?";
    return sql;
}

void DbTraceDataBase::SetKernelDetail(std::unique_ptr<SqliteResultSet> resultSet, uint64_t minTimestamp,
                                      Protocol::KernelDetailsBody &responseBody) const
{
    while (resultSet->Next()) {
        Protocol::KernelDetail detail;
        detail.name = resultSet->GetString("name");
        detail.type = resultSet->GetString("type");
        detail.acceleratorCore = resultSet->GetString("acceleratorCore");
        detail.startTime = resultSet->GetInt64("startTime") - minTimestamp;
        detail.duration = resultSet->GetDouble("duration");
        detail.waitTime = resultSet->GetDouble("waitTime");
        detail.blockDim = resultSet->GetInt64("blockDim");
        detail.inputShapes = resultSet->GetString("inputShapes");
        detail.inputDataTypes = resultSet->GetString("inputDataTypes");
        detail.inputFormats = resultSet->GetString("inputFormats");
        detail.outputShapes = resultSet->GetString("outputShapes");
        detail.outputDataTypes = resultSet->GetString("outputDataTypes");
        detail.outputFormats = resultSet->GetString("outputFormats");
        responseBody.kernelDetails.emplace_back(detail);
    }
}

uint64_t DbTraceDataBase::QueryTotalKernel(const std::string &coreType, const std::string &name)
{
    std::string sql = "SELECT count(*) FROM " + TABLE_COMPUTE_TASK_INFO + " where name LIKE ?";
    if (!coreType.empty()) {
        sql += " AND accelerator_core = ? ";
    }
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        Server::ServerLog::Error("QueryTotalKernel, fail to prepare sql.");
        return 0;
    }
    if (!coreType.empty()) {
        stmt->BindParams(coreType);
    }
    auto resultSet = stmt->ExecuteQuery(name);
    uint64_t total = 0;
    if (resultSet->Next()) {
        total = resultSet->GetInt64("count(1)");
    }
    return total;
}

bool DbTraceDataBase::QueryKernelDepthAndThread(const Protocol::KernelParams &params,
                                                Protocol::OneKernelBody &responseBody, uint64_t minTimestamp)
{
    return false;
}

LayerStatData DbTraceDataBase::QueryLayerData(const std::string &layer, const std::string &name)
{
    return LayerStatData();
}

std::vector<std::string> DbTraceDataBase::QueryCoreType()
{
    std::vector<std::string> acceleratorCoreList;
    std::string sql = "SELECT DISTINCT TASKTYPE.value as task_type FROM " + TABLE_COMPUTE_TASK_INFO +
            " JOIN STRING_IDS AS TASKTYPE ON TASKTYPE.id = COMPUTE_TASK_INFO.taskType ORDER BY task_type";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryCoreType, fail to prepare sql.");
        return acceleratorCoreList;
    }
    auto resultSet = stmt->ExecuteQuery();
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
                                               Protocol::UnitThreadTracesSummaryBody &responseBody,
                                               uint64_t minTimestamp)
{
    auto stmt = CreatPreparedStatement();

    std::unique_ptr<SqliteResultSet> resultSet;
    try {
        resultSet = TraceDatabaseHelper::QueryThreadTracesSummary(stmt, requestParams, minTimestamp);
    } catch (DatabaseException &e) {
        ServerLog::Error("QueryThreadTracesSummary Fail, ", e.What());
        return false;
    }
    if (resultSet == nullptr) {
        ServerLog::Error("QueryThreadTracesSummary. Failed to get result set.", stmt->GetErrorMessage());
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
void DbTraceDataBase::UpdateStartTime()
{
    sqlite3_stmt *stmt = nullptr;
    std::string sql = "SELECT startTimeNs, endTimeNs FROM " + TABLE_SESSION_TIME_INFO;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        Server::ServerLog::Error("Failed to Update Start Time. Cmd: ", sql, " Msg: ", sqlite3_errmsg(db), " ", result);
        return;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        int64_t startTime = sqlite3_column_int64(stmt, col++);
        int64_t endTime = sqlite3_column_int64(stmt, col++);
        TraceTime::Instance().UpdateTime(startTime, endTime);
    }
    Server::ServerLog::Info("Update start and end time. ");
    sqlite3_finalize(stmt);
}

std::vector<std::string> DbTraceDataBase::QueryRankId()
{
    sqlite3_stmt *stmt = nullptr;
    std::string sql;
    FileType type = DataBaseManager::Instance().GetFileType();
    if (type == FileType::MS_PROF) {
        sql = "SELECT id FROM " + TABLE_NPU_INFO;
    } else if (type == FileType::PYTORCH) {
        sql = "SELECT rankId FROM " + TABLE_PYTORCH_INFO;
    }
    std::vector<std::string> rankIds;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        Server::ServerLog::Error("Failed to get Statistic Num. Cmd: ", sql, " Msg: ", sqlite3_errmsg(db), " ", result);
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
    sqlite3_stmt *stmt = nullptr;
    std::string sql = "  SELECT COUNT(*) FROM " + tableName;
    std::vector<std::string> rankIds;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        Server::ServerLog::Error("Failed to get Memory Data. Cmd: ", sql, " Msg: ", sqlite3_errmsg(db), " ", result);
        return false;
    }
    int64_t count;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int64(stmt, resultStartIndex);
    }
    sqlite3_finalize(stmt);
    return count != 0;
}

void DbTraceDataBase::UpdateAllTaskDepth()
{
    std::string sql = "select deviceId, streamId, globalTaskId, startNs, endNs from TASK "
                      "order by deviceId, streamId, startNs, globalTaskId;";
    auto stmt = CreatPreparedStatement(sql);
    auto resultSet = stmt->ExecuteQuery();
    std::map<std::string, std::vector<TASK_INFO>> data;
    while (resultSet->Next()) {
        auto deviceId = resultSet->GetInt64("deviceId");
        auto streamId = resultSet->GetInt64("streamId");
        auto key = std::to_string(deviceId) + "-" +  std::to_string(streamId);
        if (data.find(key) == data.end()) {
            data[key] = std::vector<TASK_INFO>();
        }
        TASK_INFO task;
        task.start = resultSet->GetInt64("startNs");
        task.end = resultSet->GetInt64("endNs");
        task.globalTaskId = resultSet->GetInt64("globalTaskId");
        data[key].push_back(task);
    }
    std::vector<int64_t> endList;
    for (auto &deviceData: data) {
        for (auto &task: deviceData.second) {
            for (task.depth = 0; task.depth < endList.size() && endList[task.depth] > task.start; task.depth++) {
            }
            if (task.depth >= endList.size()) {
                endList.emplace_back(task.end);
            } else {
                endList[task.depth] = task.end;
            }
            UpdateTaskDepth(task, updateTaskDepthStmt);
        }
        endList.clear();
    }
    UpdateTaskDepthList(updateTaskDepthStmt);
}

void DbTraceDataBase::UpdateAllApiDepth()
{
    std::string sql = "select type, startNs, endNs, connectionId from API order by type, startNs;";
    auto stmt = CreatPreparedStatement(sql);
    auto resultSet = stmt->ExecuteQuery();
    std::map<std::string, std::vector<TASK_INFO>> data;
    while (resultSet->Next()) {
        auto key = resultSet->GetString("type");
        if (data.find(key) == data.end()) {
            data[key] = std::vector<TASK_INFO>();
        }
        TASK_INFO task;
        task.start = resultSet->GetInt64("startNs");
        task.end = resultSet->GetInt64("endNs");
        task.globalTaskId = resultSet->GetInt64("connectionId");
        data[key].push_back(task);
    }
    std::vector<int64_t> endList;
    for (auto &deviceData: data) {
        for (auto &task: deviceData.second) {
            for (task.depth = 0; task.depth < endList.size() && endList[task.depth] > task.start; task.depth++) {
            }
            if (task.depth >= endList.size()) {
                endList.emplace_back(task.end);
            } else {
                endList[task.depth] = task.end;
            }
            UpdateTaskDepth(task, updateApiDepthStmt);
        }
        endList.clear();
    }
    UpdateTaskDepthList(updateApiDepthStmt);
}

void DbTraceDataBase::UpdateTaskDepth(const TASK_INFO &taskInfo, std::unique_ptr<SqlitePreparedStatement> &stmt)
{
    taskDepthCache.emplace_back(taskInfo);
    if (taskDepthCache.size() == cacheSize) {
        UpdateTaskDepthList(stmt);
    }
}

bool DbTraceDataBase::UpdateTaskDepthList(std::unique_ptr<SqlitePreparedStatement> &stmt)
{
    std::unique_lock<std::mutex> lock(mutex);
    if (!StartTransaction()) {
        ServerLog::Error("Failed to start Transaction.");
        return false;
    }
    bool result = true;
    for (const auto &item: taskDepthCache) {
        stmt->Reset();
        stmt->BindParams(item.depth, item.globalTaskId);
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
    Database::OpenDb(dbPath, clearAllTable);
    SetConfig();
    InitStmt();
}

void DbTraceDataBase::InitStringsCache()
{
    stringsCache.clear();
    auto sql = "select id, value from STRING_IDS";
    auto stmt = CreatPreparedStatement(sql);
    auto result = stmt->ExecuteQuery();
    while (result->Next()) {
        stringsCache.emplace(result->GetInt64("id"), result->GetString("value"));
    }
}

bool DbTraceDataBase::InitStmt()
{
    if (initStmt) {
        return true;
    }
    initStmt = true;
    std::string sql = "UPDATE " + TABLE_TASK + " set depth = ? where globalTaskId = ?";
    updateTaskDepthStmt = CreatPreparedStatement(sql);
    sql = "UPDATE " + TABLE_API + " set depth = ? where connectionId = ?";
    updateApiDepthStmt = CreatPreparedStatement(sql);
    if (updateTaskDepthStmt == nullptr || updateApiDepthStmt == nullptr) {
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
    std::unique_lock<std::mutex> lock(mutex);
    std::string dbVersion = GetDataBaseVersion();
    ExecSql("create INDEX connectionId on API(connectionId);");
    ExecSql("alter table TASK add depth integer;");
    ExecSql("alter table API add depth integer;");
    return ExecSql("PRAGMA synchronous = OFF; PRAGMA journal_mode = MEMORY; PRAGMA user_version = " +
                   dbVersion + ";");
}

bool DbTraceDataBase::QueryHostMetadata(std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData)
{
    std::string sql = " select EAL.name, globalTid, type, max(depth) as maxDepth from API a join ENUM_API_TYPE EAL "
                      " on a.type = EAL.id group by type, globalTid";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryHostMetadata failed!.");
        return false;
    }
    auto resultSet = stmt->ExecuteQuery();
    std::map<std::string, std::vector<MetaDataDto>> threadMap;
    while (resultSet->Next()) {
        MetaDataDto metadata;
        metadata.pid = resultSet->GetString("globalTid");
        metadata.processName = "process";
        metadata.threadId = resultSet->GetString("type");
        metadata.threadName = resultSet->GetString("name");
        metadata.maxDepth = resultSet->GetInt32("maxDepth") + 1;
        threadMap[metadata.pid].emplace_back(metadata);
    }
    auto process = GenerateBaseUnitTrack("label", "Host", "Host", "process");
    for (auto &thread: threadMap) {
        auto threadUnit = GenerateBaseUnitTrack("process", "Host", thread.first, "Thread " + thread.first);
        auto cannApiUnit = GenerateBaseUnitTrack("process", "Host", thread.first, "CANN");
        for (const auto &item: thread.second) {
            auto level = GenerateBaseUnitTrack("thread", threadUnit->metaData.cardId, thread.first, "");
            level->metaData.threadId = item.threadId;
            level->metaData.threadName = item.threadName;
            level->metaData.maxDepth = item.maxDepth;
            auto a = std::find(CANN_APIS.begin(), CANN_APIS.end(), item.threadName);
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
    metaData.emplace_back(std::move(process));
    return true;
}

std::unique_ptr<Protocol::UnitTrack> DbTraceDataBase::GenerateBaseUnitTrack(const std::string &type,
    const std::string &cardId, const std::string &processId, const std::string &processName)
{
    std::unique_ptr<Protocol::UnitTrack> unitTrack = std::make_unique<Protocol::UnitTrack>();
    unitTrack->type = type;
    unitTrack->metaData.cardId = cardId;
    unitTrack->metaData.processId = processId;
    unitTrack->metaData.processName = processName;
    return unitTrack;
}

bool DbTraceDataBase::QueryAscendHardwareMetadata(const std::string &fileId,
                                                  std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData)
{
    std::string sql = "select globalPid, streamId, max(depth) as maxDepth from " + TABLE_TASK +
                      " where deviceId = ? group by globalPid, streamId";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryAscendHardwareMetadata failed!.");
        return "";
    }
    auto resultSet = stmt->ExecuteQuery(fileId);
    auto ascendHardware = GenerateBaseUnitTrack("process", fileId,
        std::to_string(static_cast<int8_t>(PROCESS_TYPE::ASCEND_HARDWARE)), "Ascend Hardware");
    ascendHardware->metaData.label = "NPU";
    while (resultSet->Next()) {
        auto thread = GenerateBaseUnitTrack("thread", fileId,
                                            ascendHardware->metaData.processId, "");
        thread->metaData.threadId = resultSet->GetString("streamId");
        thread->metaData.threadName = "Stream " + thread->metaData.threadId;
        thread->metaData.maxDepth = resultSet->GetInt32("maxDepth") + 1;
        ascendHardware->children.emplace_back(std::move(thread));
    }
    metaData.emplace_back(std::move(ascendHardware));
    return true;
}

bool DbTraceDataBase::QueryHcclMetadata(const std::string &fileId,
                                        std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData)
{
    std::string sql = "with main as (select planeId, groupName from " + TABLE_COMMUNICATION_TASK_INFO +
            " info join " + TABLE_TASK + " task on task.globalTaskId = info.globalTaskId where deviceId = ?)\n"
            " select 'Plane ' || planeId as name, planeId as id  from main group by planeId\n"
            " union select 'Group ' || row_number() over () || ' Communication' as name, groupName as id from main"
            " group by groupName";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryHcclMetadata failed!.");
        return "";
    }
    auto resultSet = stmt->ExecuteQuery(fileId);
    auto hccl = GenerateBaseUnitTrack("process", fileId,
        std::to_string(static_cast<int8_t>(PROCESS_TYPE::HCCL)), "HCCL");
    hccl->metaData.label = "NPU";
    while (resultSet->Next()) {
        auto thread = GenerateBaseUnitTrack("thread", fileId,
                                            hccl->metaData.processId, "");
        thread->metaData.threadId = resultSet->GetString("id");
        thread->metaData.threadName = resultSet->GetString("name");
        thread->metaData.maxDepth = 1;
        hccl->children.emplace_back(std::move(thread));
    }
    metaData.emplace_back(std::move(hccl));
    return true;
}

bool DbTraceDataBase::QueryCounterMetadata(const std::string &fileId,
                                           std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData)
{
    PROCESS_TYPE types[] = {PROCESS_TYPE::HBM, PROCESS_TYPE::LLC};
    for (const auto &type: types) {
        auto typeName = ENUM_TO_STR(type).value_or("");
        if (!CheckTableExist(typeName)) {
            ServerLog::Info("QueryCounterMetadata failed, table ", typeName, " Not Exist.");
            continue;
        }
        auto counter = GenerateBaseUnitTrack("label", fileId, typeName, typeName);

        std::string sql;
        switch (type) {
            case PROCESS_TYPE::HBM:
                sql = "select case when name='read' then 'Read(MB/s)' else 'Write(MB/s)' end as typeName,\n"
                      "    hbmId || '/' || case when name='read' then 'Read' else 'Write' end as name\n"
                      "FROM HBM join ENUM_MEMORY on type = id WHERE deviceId = ? GROUP BY hbmId, type";
                break;
            case PROCESS_TYPE::LLC:
                sql = "with main as (select llcId, mode from LLC where deviceId = ? group by llcId, mode)"
                      " select 'Throughput(MB/s)' as typeName, llcId || ' ' || case when name='read' then 'Read' else"
                      " 'Write' end || '/Throughput' as name  from main join ENUM_MEMORY on mode = id "
                      " UNION select 'Hit Rate(%)' as typeName, llcId || ' ' || case when name='read' then 'Read' else"
                      " 'Write' end || '/Hit Rate' as name  from main join ENUM_MEMORY on mode = id ";
                break;
        }
        auto stmt = CreatPreparedStatement(sql);
        if (stmt == nullptr) {
            ServerLog::Error("QueryCounterMetadata failed!.");
            return "";
        }
        auto resultSet = stmt->ExecuteQuery(fileId);
        while (resultSet->Next()) {
            auto thread = GenerateBaseUnitTrack("counter", fileId, typeName,
                                                resultSet->GetString("name"));
            thread->metaData.threadName = typeName + " " + thread->metaData.processName;
            thread->metaData.dataType = StringUtil::Split(resultSet->GetString("typeName"), ",");
            counter->children.emplace_back(std::move(thread));
        }
        metaData.emplace_back(std::move(counter));
    }
    return true;
}

bool DbTraceDataBase::GenerateCounterMetadata(const std::string &fileId,
                                              std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData)
{
    PROCESS_TYPE types[] = {PROCESS_TYPE::ACC_PMU, PROCESS_TYPE::DDR, PROCESS_TYPE::STARS_SOC, PROCESS_TYPE::NPU_MEM};
    for (const auto &type: types) {
        auto typeName = ENUM_TO_STR(type).value_or("");
        if (!CheckTableExist(typeName)) {
            ServerLog::Info("GenerateCounterMetadata failed, table ", typeName, " Not Exist.");
            continue;
        }
        auto counter = GenerateBaseUnitTrack("label", fileId, typeName, typeName);
        std::vector<std::string> units;
        std::vector<std::vector<std::string>> dataTypes;
        switch (type) {
            case PROCESS_TYPE::ACC_PMU:
                units = {"read_bandwidth", "read_ost", "write_bandwidth", "write_ost"};
                dataTypes = {{"value", "acc_id"}};
                break;
            case PROCESS_TYPE::DDR:
                units = {"Read", "Write"};
                dataTypes = {{"Read(MB/s)"}, {"Write(MB/s)"}};
                break;
            case PROCESS_TYPE::STARS_SOC:
                counter->metaData.processName = "Stars Soc";
                units = {"L2 Buffer Bw Level", "Mata Bw Level"};
                dataTypes = {{"L2 Buffer Bw Level"}, {"Mata Bw Level"}};
                break;
            case PROCESS_TYPE::NPU_MEM:
                units = {"APP/DDR", "APP/HBM", "APP/MEMORY", "Device/DDR", "Device/HBM", "Device/MEMORY"};
                dataTypes = {{"KB"}};
                break;
        }
        for (int index = 0; index < units.size(); index++) {
            auto thread = GenerateBaseUnitTrack("counter", fileId, typeName,
                                                units.at(index));
            thread->metaData.threadName = thread->metaData.processName;
            auto dataType = dataTypes.size() == 1 ? dataTypes[0] : dataTypes[index];
            thread->metaData.dataType.insert(thread->metaData.dataType.end(), dataType.begin(), dataType.end());
            counter->children.emplace_back(std::move(thread));
        }
        metaData.emplace_back(std::move(counter));
    }
    return true;
}

bool DbTraceDataBase::QueryDurationFromTaskByTimeRange(const Protocol::ThreadDetailParams &requestParams,
                                                       SliceDto sliceDto, std::vector<uint64_t> &nextDepthResult,
                                                       int64_t trackId)
{
    std::string sql = "SELECT endNs - startNs as duration FROM " + TABLE_TASK +
                      " WHERE depth = ? AND endNs <= ? AND startNs >= ? AND streamId = ?";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryDurationFromTaskByTimeRange. Failed to prepare sql.", sqlite3_errmsg(db));
        return false;
    }
    auto resultSet =
            stmt->ExecuteQuery(requestParams.depth + 1, sliceDto.timestamp + sliceDto.duration,
                               sliceDto.timestamp, trackId);
    while (resultSet->Next()) {
        nextDepthResult.emplace_back(resultSet->GetUint64("duration"));
    }
    return true;
}
}