/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */


#include "DbTraceDataBase.h"
#include "ServerLog.h"
#include "TraceTime.h"
#include "TableDefs.h"
#include "set"
#include "TraceDatabaseHelper.h"

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
        rowThreadTrace.startTime = resultSet->GetUint64("start_time");
        rowThreadTrace.duration = resultSet->GetUint64("duration");
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
            simpleSlice.timestamp = resultSet->GetUint64(col++);
            simpleSlice.duration = resultSet->GetUint64(col++);
            simpleSlice.endTime = resultSet->GetUint64(col++);
            simpleSlice.name = resultSet->GetString(col++);
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
        sliceDto.id = resultSet->GetUint64(col++);
        sliceDto.timestamp = resultSet->GetUint64(col++);
        sliceDto.duration = resultSet->GetUint64(col++);
        sliceDto.depth = resultSet->GetInt32(col++);
        sliceDto.name = stringsCache.at(resultSet->GetInt64(col++));
        sliceDtoVec.emplace_back(sliceDto);
    }

    if (sliceDtoVec.size() != 1) {
        ServerLog::Error("select slice error!");
        return false;
    }

    responseBody.emptyFlag = false;
    responseBody.data.selfTime = 0;
    responseBody.data.args = sliceDtoVec[0].args;
    responseBody.data.title = sliceDtoVec[0].name;
    responseBody.data.duration = sliceDtoVec[0].duration;
    responseBody.data.cat = sliceDtoVec[0].cat;
    return true;
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

int DbTraceDataBase::SearchSliceNameCount(const std::string &name)
{
    return 0;
}

bool DbTraceDataBase::QueryFlowCategoryList(std::vector<std::string> &categories)
{
    return false;
}

bool DbTraceDataBase::SearchSliceName(const std::string &name, int index, uint64_t minTimestamp,
                                      Protocol::SearchSliceBody &responseBody)
{
    return false;
}

bool DbTraceDataBase::QueryFlowCategoryEvents(Protocol::FlowCategoryEventsParams &params, uint64_t minTimestamp,
                                              std::vector<std::unique_ptr<Protocol::FlowEvent>> &flowDetailList)
{
    return false;
}

bool DbTraceDataBase::QueryUnitCounter(Protocol::UnitCounterParams &params, uint64_t minTimestamp,
                                       std::vector<Protocol::UnitCounterData> &dataList)
{
    return false;
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
    std::string sql = "SELECT sum(end - start) as duration, TASKTYPE.value as accelerator_core "
                      "  FROM COMPUTE_TASK_INFO"
                      "     JOIN TASK ON COMPUTE_TASK_INFO.correlationId = TASK.correlationId "
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
                      "     TASKTYPE.value as accelerator_core, start as start_time, end - start as duration,"
                      "     block_dim, 0 as wait_time,"
                      "     INPUTSHAPES.value as input_shapes, INPUTDATATYPES.value as input_data_types, "
                      "     INPUTFORMATS.value as input_formats, OUTPUTSHAPES.value as output_shapes, "
                      "     OUTPUTDATATYPES.value as output_data_types, OUTPUTFORMATS.value as output_formats "
                      "   FROM " + TABLE_COMPUTE_TASK_INFO +
                      "   JOIN TASK ON COMPUTE_TASK_INFO.correlationId = TASK.correlationId "
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
        detail.startTime = resultSet->GetUint64("startTime") - minTimestamp;
        detail.duration = resultSet->GetDouble("duration");
        detail.waitTime = resultSet->GetDouble("waitTime");
        detail.blockDim = resultSet->GetUint64("blockDim");
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
        total = resultSet->GetUint64("count(*)");
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
            "JOIN STRING_IDS AS TASKTYPE ON TASKTYPE.id = COMPUTE_TASK_INFO.taskType ORDER BY task_type";
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
        uint64_t endTime = resultSet->GetUint64("end_time");
        if (endTime > maxTime) {
            summary.startTime = resultSet->GetUint64("start_time");
            summary.duration = resultSet->GetUint64("duration");
            responseBody.data.emplace_back(summary);
            maxTime = endTime;
        }
    }
    return true;
}
void DbTraceDataBase::UpdateStartTime()
{
    sqlite3_stmt *stmt = nullptr;
    std::string sql = "SELECT startTimeNs, endTimeNs, baseTimeNs FROM TARGET_INFO_SESSION_TIME";
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        Server::ServerLog::Error("Failed to Update Start Time. Cmd: ", sql, " Msg: ", sqlite3_errmsg(db), " ", result);
        return;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        int64_t startTime = sqlite3_column_int64(stmt, col++);
        int64_t endTime = sqlite3_column_int64(stmt, col++);
        int64_t baseTime = sqlite3_column_int64(stmt, col++);
        TraceTime::Instance().UpdateTime(startTime, endTime);
        TraceTime::Instance().SetBaseTime(baseTime);
    }
    Server::ServerLog::Info("Update start and end time. ");
    sqlite3_finalize(stmt);
}

std::vector<std::string> DbTraceDataBase::QueryRankId()
{
    sqlite3_stmt *stmt = nullptr;
    std::string sql = "SELECT id FROM TARGET_INFO_NPU";
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
    std::string sql = "select deviceId, streamId, correlationId, start, end from TASK "
                      "order by deviceId, streamId, start, correlationId;";
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
        task.start = resultSet->GetInt64("start");
        task.end = resultSet->GetInt64("end");
        task.correlationId = resultSet->GetInt64("correlationId");
        data[key].push_back(task);
    }
    std::multiset<int64_t> endList;
    for (auto &deviceData: data) {
        for (auto &task: deviceData.second) {
            endList.insert(task.end);
            task.depth = std::distance(endList.upper_bound(task.start), endList.end());
            if (task.depth > 0) task.depth--;
            UpdateTaskDepth(task);
        }
        endList.clear();
    }
    UpdateTaskDepthList();
}

void DbTraceDataBase::UpdateTaskDepth(const TASK_INFO &taskInfo)
{
    taskDepthCache.emplace_back(taskInfo);
    if (taskDepthCache.size() == cacheSize) {
        UpdateTaskDepthList();
        taskDepthCache.clear();
    }
}

bool DbTraceDataBase::UpdateTaskDepthList()
{
    std::unique_lock<std::mutex> lock(mutex);
    if (!StartTransaction()) {
        ServerLog::Error("Failed to start Transaction.");
        return false;
    }
    bool result = true;
    for (const auto &item: taskDepthCache) {
        updateTaskDepthStmt->Reset();
        updateTaskDepthStmt->BindParams(item.depth, item.correlationId);
        if (!updateTaskDepthStmt->Execute()) {
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
    std::string sql = "UPDATE " + TABLE_TASK + " set depth = ? where correlationId = ?";
    updateTaskDepthStmt = CreatPreparedStatement(sql);
    if (updateTaskDepthStmt == nullptr) {
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
    ExecSql("alter table TASK add depth integer;");
    return ExecSql("PRAGMA synchronous = OFF; PRAGMA journal_mode = MEMORY; PRAGMA user_version = " +
                   dbVersion + ";");
}

bool DbTraceDataBase::QueryAscendHardwareMetadata(const std::string &fileId,
                                                  std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData)
{
    std::unique_ptr<Protocol::UnitTrack> ascendHardware = std::make_unique<Protocol::UnitTrack>();

    ascendHardware->type = "process";
    ascendHardware->metaData.processName = "Ascend Hardware";
    ascendHardware->metaData.label = "NPU";
    ascendHardware->metaData.cardId = fileId;

    std::string sql = "select globalPid, streamId, max(depth) as maxDepth from " + TABLE_TASK +
                      " where deviceId = ? group by globalPid, streamId";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryPidByDeviceId failed!.");
        return "";
    }
    auto resultSet = stmt->ExecuteQuery(fileId);
    ascendHardware->metaData.processId = std::to_string(static_cast<int8_t>(PROCESS_TYPE::ASCEND_HARDWARE));
    while (resultSet->Next()) {
        std::unique_ptr<Protocol::UnitTrack> thread = std::make_unique<Protocol::UnitTrack>();
        thread->type =  "thread";
        thread->metaData.cardId = fileId;
        thread->metaData.processId = ascendHardware->metaData.processId;
        thread->metaData.threadId = resultSet->GetInt64("streamId");
        thread->metaData.threadName = "Stream " + thread->metaData.threadId;
        thread->metaData.maxDepth = resultSet->GetInt32("maxDepth") + 1;
        ascendHardware->children.emplace_back(std::move(thread));
    }
    metaData.emplace_back(std::move(ascendHardware));
    return true;
}
}