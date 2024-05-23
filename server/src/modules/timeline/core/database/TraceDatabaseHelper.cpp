/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "TraceDatabaseHelper.h"


namespace Dic::Module::Timeline {
std::map<std::string, Protocol::PROCESS_TYPE> TraceDatabaseHelper::metaTypeMap = {
    {"Python", Protocol::PROCESS_TYPE::API},
    {"CANN", Protocol::PROCESS_TYPE::CANN_API},
    {"Ascend Hardware", Protocol::PROCESS_TYPE::ASCEND_HARDWARE},
    {"HCCL", Protocol::PROCESS_TYPE::HCCL},
    {"Overlap Analysis", Protocol::PROCESS_TYPE::OVERLAP_ANALYSIS},
};

const Protocol::EventsViewColumnAttr columnName = {"Name", "string", "name"};
const Protocol::EventsViewColumnAttr columnStart = {"Start", "number", "start"};
const Protocol::EventsViewColumnAttr columnDuration = {"Duration", "number", "duration"};
const Protocol::EventsViewColumnAttr columnTid = {"TID", "string", "tid"};
const Protocol::EventsViewColumnAttr columnPid = {"PID", "string", "pid"};
const Protocol::EventsViewColumnAttr columnRankId = {"Rank ID", "string", "rankId"};
const Protocol::EventsViewColumnAttr columnStreamName = {"Stream Name", "string", "threadName"};
const Protocol::EventsViewColumnAttr columnGroupName = {"Group Name", "string", "threadName"};
const Protocol::EventsViewColumnAttr columnAnalysisType = {"Analysis Type", "string", "threadName"};

std::map<Protocol::PROCESS_TYPE, std::vector<Protocol::EventsViewColumnAttr>>
    TraceDatabaseHelper::eventsViewColumnsMap = {
    {Protocol::PROCESS_TYPE::API, {columnName, columnStart, columnDuration, columnTid, columnPid}},
    {Protocol::PROCESS_TYPE::CANN_API, {columnName, columnStart, columnDuration, columnTid, columnPid}},
    {Protocol::PROCESS_TYPE::ASCEND_HARDWARE,
        {columnName, columnStart, columnDuration, columnStreamName, columnRankId}},
    {Protocol::PROCESS_TYPE::HCCL, {columnName, columnStart, columnDuration, columnGroupName, columnRankId}},
    {Protocol::PROCESS_TYPE::OVERLAP_ANALYSIS,
        {columnName, columnStart, columnDuration, columnAnalysisType, columnRankId}},
};

/* Functions for BbTraceDataBase */
std::optional<std::string> TraceDatabaseHelper::QueryConnectionId(std::unique_ptr<SqlitePreparedStatement> &stmt,
    const Protocol::UnitFlowsParams &requestParams)
{
    std::string sql;
    auto processType = GetProcessType(requestParams.metaType);
    std::unique_ptr<SqliteResultSet> resultSet;
    switch (processType) {
        case PROCESS_TYPE::ASCEND_HARDWARE:
            sql = "select connectionId from TASK where ROWID = ?";
            resultSet = ExecuteQuery(stmt, sql, requestParams.id);
            break;
        case PROCESS_TYPE::HCCL:
            sql = "select connectionId from COMMUNICATION_OP where ROWID = ? and groupName||'group' = ?";
            resultSet = ExecuteQuery(stmt, sql, requestParams.id, requestParams.tid);
            break;
        case PROCESS_TYPE::CANN_API:
            sql = "select connectionId from CANN_API where ROWID = ?";
            resultSet = ExecuteQuery(stmt, sql, requestParams.id);
            break;
        case PROCESS_TYPE::API:
            sql = " select ids.connectionId from PYTORCH_API api "
                  " join CONNECTION_IDS ids on api.connectionId = ids.id where api.ROWID = ?";
            resultSet = ExecuteQuery(stmt, sql, requestParams.id);
            break;
        default:
            return std::nullopt;
    }
    if (!resultSet->Next()) {
        return std::nullopt;
    }
    return resultSet->GetString("connectionId");
}

void TraceDatabaseHelper::QueryTaskInfoById(std::unique_ptr<SqlitePreparedStatement> &stmt,
                                            const Protocol::ThreadDetailParams &requestParams,
                                            Protocol::UnitThreadDetailBody &responseBody,
                                            std::map<std::string, std::string> &stringCache, std::string& metaVersion)
{
    auto processType = GetProcessType(requestParams.metaType);
    bool attrInfoExist = isAttrInfoExist(stmt);
    auto resultSet = QueryTaskCacheInfoById(stmt, requestParams, attrInfoExist, metaVersion);
    std::vector<std::string> types = {"inputShapes", "inputDataTypes", "inputFormats",
                                      "outputShapes", "outputDataTypes", "outputFormats", "attrInfo"};
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    if (resultSet.operator bool() && resultSet->Next()) {
        if (processType == PROCESS_TYPE::ASCEND_HARDWARE) {
            responseBody.data.inputShapes = stringCache[resultSet->GetString("inputShapes")];
            responseBody.data.inputDataTypes = stringCache[resultSet->GetString("inputDataTypes")];
            responseBody.data.inputFormats = stringCache[resultSet->GetString("inputFormats")];
            responseBody.data.outputShapes = stringCache[resultSet->GetString("outputShapes")];
            responseBody.data.outputDataTypes = stringCache[resultSet->GetString("outputDataTypes")];
            responseBody.data.outputFormats = stringCache[resultSet->GetString("outputFormats")];
            // 存在attrInfo，返回给前端展示在界面上
            if (attrInfoExist) {
                responseBody.data.attrInfo = stringCache[resultSet->GetString("attrInfo")];
            }
        }
        for (auto &item: resultSet->GetColumns()) {
            if (std::find(types.begin(), types.end(), item.first) != types.end()) {
                continue;
            }
            JsonUtil::AddConstMember(json, item.first, stringCache[resultSet->GetString(item.second)], allocator);
        }
    }

    resultSet = QueryTaskStrInfoById(stmt, requestParams, metaVersion);
    if (resultSet.operator bool() && resultSet->Next()) {
        for (auto &item: resultSet->GetColumns()) {
            JsonUtil::AddConstMember(json, item.first, resultSet->GetString(item.second), allocator);
        }
    }
    responseBody.data.args = JsonUtil::JsonDump(json);
}

std::unique_ptr<SqliteResultSet> TraceDatabaseHelper::QueryTaskStrInfoById(
    std::unique_ptr<SqlitePreparedStatement> &stmt, const Protocol::ThreadDetailParams &requestParams,
    std::string& metaVersion)
{
    auto processType = GetProcessType(requestParams.metaType);
    std::string sql;
    std::unique_ptr<SqliteResultSet> resultSet;
    switch (processType) {
        case PROCESS_TYPE::ASCEND_HARDWARE:
            sql.append("SELECT ").append(metaVersion.empty() ? "block_dim" : "blockDim").append(", mixBlockDim "
                  " FROM TASK main join COMPUTE_TASK_INFO CTI on main.globalTaskId = CTI.globalTaskId"
                  "  where main.ROWID = ?");
            return ExecuteQuery(stmt, sql, requestParams.id);
        case PROCESS_TYPE::HCCL:
            sql = "SELECT com.planeId, com.notifyId ,com.srcRank ,com.dstRank, com.size, com.opId ";
            sql.append(metaVersion.empty() ? "" : ", rdma.name as rdmaType, tran.name as transportType,"
                                                  " data.name as dataType, link.name as linkType");
            sql.append("  FROM TASK main join COMMUNICATION_TASK_INFO com on main.globalTaskId = com.globalTaskId ");
            sql.append(metaVersion.empty() ? "" : "join ENUM_HCCL_DATA_TYPE data on data.id = com.dataType "
                                                  " join ENUM_HCCL_RDMA_TYPE rdma on rdma.id = com.rdmaType "
                                                  " join ENUM_HCCL_TRANSPORT_TYPE tran on tran.id = com.transportType "
                                                  " join ENUM_HCCL_LINK_TYPE link on link.id = com.linkType");
            sql.append(" where main.ROWID = ?");
            return ExecuteQuery(stmt, sql, requestParams.id);
        case PROCESS_TYPE::API:
            sql = " select group_concat(coalesce(value, stack), ';\n') as 'Call stack', sequenceNumber from ( "
                  "    SELECT stack, sequenceNumber, strs.value FROM PYTORCH_API main "
                  "             left join PYTORCH_CALLCHAINS call on call.id = main.callchainId "
                  " left join STRING_IDS strs on strs.id = call.stack"
                  "    where main.ROWID = ? order by stackDepth )";
            return ExecuteQuery(stmt, sql, requestParams.id);
        case PROCESS_TYPE::CANN_API:
            sql = "SELECT t.name as apiType, connectionId FROM CANN_API main join ENUM_API_TYPE t on t.id = type "
                  " where main.ROWID = ?";
            return ExecuteQuery(stmt, sql, requestParams.id);
        default:
            return resultSet;
    }
}

std::unique_ptr<SqliteResultSet> TraceDatabaseHelper::QueryTaskCacheInfoById(
    std::unique_ptr<SqlitePreparedStatement> &stmt, const Protocol::ThreadDetailParams &requestParams,
    bool attrInfoExist, std::string& metaVersion)
{
    auto processType = GetProcessType(requestParams.metaType);
    std::string sql;
    std::unique_ptr<SqliteResultSet> resultSet;
    switch (processType) {
        case PROCESS_TYPE::ASCEND_HARDWARE:
            // 存在attrInfo字段，则查询出来
            if (attrInfoExist) {
                sql = "SELECT inputShapes, inputDataTypes, inputFormats, outputShapes, outputDataTypes,"
                      " outputFormats, attrInfo, CTI.taskType, opType FROM TASK main join COMPUTE_TASK_INFO CTI"
                      " on main.globalTaskId = CTI.globalTaskId  where main.ROWID = ?";
            } else {
                sql = "SELECT inputShapes, inputDataTypes, inputFormats, outputShapes, outputDataTypes,"
                      " outputFormats, CTI.taskType, opType FROM TASK main join COMPUTE_TASK_INFO CTI"
                      " on main.globalTaskId = CTI.globalTaskId  where main.ROWID = ?";
            }
            return ExecuteQuery(stmt, sql, requestParams.id);
        case PROCESS_TYPE::HCCL:
            sql = "SELECT com.taskType, com.groupName ";
            sql.append(metaVersion.empty() ? ",com.rdmaType,com.transportType, com.dataType, com.linkType " : "");
            sql.append("      FROM TASK main join COMMUNICATION_TASK_INFO com on main.globalTaskId = com.globalTaskId "
                  " where main.ROWID = ?");
            return ExecuteQuery(stmt, sql, requestParams.id);
        case PROCESS_TYPE::API:
            sql = "SELECT inputDtypes, inputShapes FROM PYTORCH_API main "
                  " where main.ROWID = ?";
            return ExecuteQuery(stmt, sql, requestParams.id);
        case PROCESS_TYPE::CANN_API:
        default:
            return resultSet;
    }
}

/**
 * 兼容330的全量db数据，330交付的COMPUTE_TASK_INFO表中没有attrInfo这个字段，查询时先判断此字段是否存在
 * @param stmt
 * @return is attrInfo exist
 */
bool TraceDatabaseHelper::isAttrInfoExist(std::unique_ptr<SqlitePreparedStatement> &stmt)
{
    std::string sql = "SELECT count(*) FROM sqlite_master "
                      "WHERE type = 'table' AND name = 'COMPUTE_TASK_INFO' AND sql LIKE '%attrInfo%';";
    auto resultSet = ExecuteQuery(stmt, sql);
    if (resultSet->Next()) {
        return resultSet->GetInt64("count(*)");
    }
    return false;
}

std::unique_ptr<SqliteResultSet> TraceDatabaseHelper::QuerySystemViewData(
    std::unique_ptr<SqlitePreparedStatement> &stmt, const Protocol::SystemViewParams &requestParams)
{
    std::string searchName = "%" + requestParams.searchName + "%";
    std::string orderBy;
    if (!StringUtil::CheckSqlValid(requestParams.orderBy)) {
        throw DatabaseException("There is an SQL injection attack on this parameter.");
    }
    if (requestParams.order == "descend") {
        orderBy = " ORDER BY " + requestParams.orderBy + " DESC";
    } else {
        orderBy = " ORDER BY " + requestParams.orderBy + " ASC";
    }
    std::string mainSql;
    auto sql = " total as (select sum(duration) as totalTime, count(distinct name) as num from main) "
       " select name, round(sum(duration)*100.0/total.totalTime, 4) as time, sum(duration) / 1000.0 as totalTime, "
       "       count(1) as numberCalls, round(avg(duration) / 1000.0, 2) as avg, min(duration) / 1000.0 as min, "
       "       max(duration) / 1000.0 as max, total.num from main join total group by name ";
    auto limitSql = " limit ? offset ?";

    if (requestParams.layer == "Ascend Hardware") {
        mainSql = "with nameIds as ( select id, value as realName from STRING_IDS where lower(value) like ?),\n"
          "  main as (select coalesce(a.realName, b.realName) as name, endNs - startNs as duration from TASK task\n"
          "     left join COMPUTE_TASK_INFO info on info.globalTaskId = task.globalTaskId "
          "     left join nameIds a on name = a.id left join nameIds b on task.taskType = b.id where deviceId = ?),";
    } else if (requestParams.layer == "HCCL") {
        mainSql = "with nameIds as ( select id, value as realName from STRING_IDS where lower(value) like ?), "
      "     rankId as (select ? as deviceId),\n"
      "  main as (select realName as name, endNs - startNs as duration from TASK task join rankId "
      "  join COMMUNICATION_TASK_INFO info on info.globalTaskId = task.globalTaskId join nameIds on info.taskType = id "
      "  where task.deviceId = rankId.deviceId UNION ALL select realName as name, op.endNs - op.startNs as duration "
      "  from COMMUNICATION_OP op join nameIds on op.opName = id join rankId\n"
      "  join TASK task on task.connectionId = op.connectionId where task.deviceId = rankId.deviceId group by opId),";
    } else if (requestParams.layer == "CANN") {
        mainSql = "with nameIds as ( select id, value as realName from STRING_IDS where lower(value) like ?), "
                  "     tmp as (select globalPid from TASK where deviceId = ? group by globalPid), "
                  "     main as (select realName as name, endNs - startNs as duration from CANN_API api "
                  " join tmp on api.globalTid >> 32 = tmp.globalPid join nameIds on name = id),";
    } else if (requestParams.layer == "Python") {
        mainSql = "with nameIds as ( select id, value as realName from STRING_IDS where lower(value) like ?), "
                  "     tmp as (select globalPid from TASK where deviceId = ? group by globalPid), "
                  "     main as (select realName as name, endNs - startNs as duration from PYTORCH_API api "
                  " join tmp on api.globalTid >> 32 = tmp.globalPid join nameIds on name = id),";
    } else if (requestParams.layer == "Overlap Analysis") {
        mainSql = " with main as (select case type when 0 then 'Computing' when 1 then 'Communication' "
                  "        when 2 then 'Communication(Not Overlapped)' else 'Free' end as name, "
                  "  endNs - startNs as duration from OVERLAP_ANALYSIS task where name like ? and deviceId = ?),";
    } else {
            throw DatabaseException("unsupported type!");
    }
    return ExecuteQuery(stmt, mainSql + sql + orderBy + limitSql, searchName, requestParams.rankId,
                        requestParams.pageSize, (requestParams.current - 1) * requestParams.pageSize);
}
std::unique_ptr<SqliteResultSet> TraceDatabaseHelper::QueryUnitCounter(std::unique_ptr<SqlitePreparedStatement> &stmt,
    const Protocol::UnitCounterParams &requestParams, uint64_t minTimestamp)
{
    auto processType = GetProcessType(requestParams.metaType);
    switch (processType) {
        case PROCESS_TYPE::HBM:
            return ExecuteQuery(stmt, HBM_UNIT_COUNTER_SQL, minTimestamp, requestParams.rankId,
                                requestParams.threadId, requestParams.startTime, requestParams.endTime);
        case PROCESS_TYPE::LLC:
            return ExecuteQuery(stmt, LLC_UNIT_COUNTER_SQL, minTimestamp, requestParams.threadId,
                                requestParams.rankId, requestParams.startTime, requestParams.endTime);
        case PROCESS_TYPE::DDR:
            return ExecuteQuery(stmt, DDR_UNIT_COUNTER_SQL, minTimestamp, requestParams.threadId,
                                requestParams.rankId, requestParams.startTime, requestParams.endTime);
        case PROCESS_TYPE::STARS_SOC:
            return ExecuteQuery(stmt, SOC_UNIT_COUNTER_SQL, minTimestamp, requestParams.threadId,
                                requestParams.rankId, requestParams.startTime, requestParams.endTime);
        case PROCESS_TYPE::ACC_PMU:
            return ExecuteQuery(stmt, ACC_PMU_UNIT_COUNTER_SQL, requestParams.threadId, minTimestamp,
                                requestParams.rankId, requestParams.startTime, requestParams.endTime);
        case PROCESS_TYPE::NPU_MEM:
            return ExecuteQuery(stmt, NPU_UNIT_COUNTER_SQL, requestParams.threadId, minTimestamp,
                                requestParams.rankId, requestParams.startTime, requestParams.endTime);
        case PROCESS_TYPE::SAMPLE_PMU:
            return ExecuteQuery(stmt, SAMPLE_PMU_UNIT_COUNTER_SQL, minTimestamp, requestParams.rankId,
                                requestParams.threadId, requestParams.startTime, requestParams.endTime);
        case PROCESS_TYPE::ROCE:
        case PROCESS_TYPE::ROH:
        case PROCESS_TYPE::NIC:
            return ExecuteQuery(stmt, StringUtil::ReplaceFirst(NIC_UNIT_COUNTER_SQL, "#", requestParams.metaType),
                                requestParams.threadId, minTimestamp, requestParams.rankId,
                                requestParams.startTime, requestParams.endTime);
        case PROCESS_TYPE::HCCS:
            return ExecuteQuery(stmt, HCCS_UNIT_COUNTER_SQL, minTimestamp, requestParams.rankId,
                                requestParams.startTime, requestParams.endTime);
        case PROCESS_TYPE::PCIE:
            return ExecuteQuery(stmt, PCIE_UNIT_COUNTER_SQL, minTimestamp, requestParams.threadId,
                                requestParams.rankId, requestParams.startTime, requestParams.endTime);
        case PROCESS_TYPE::AI_CORE:
            return ExecuteQuery(stmt, AI_CORE_UNIT_COUNTER_SQL, minTimestamp,
                                requestParams.rankId, requestParams.startTime, requestParams.endTime);
        default:
            throw DatabaseException("unsupported type!");
    }
}

std::unique_ptr <SqliteResultSet> TraceDatabaseHelper::QueryThreadSameOperatorsDetails(
    std::unique_ptr <SqlitePreparedStatement> &stmt, const Protocol::UnitThreadsOperatorsParams &requestParams,
    uint64_t minTimestamp, const std::string& orderBy)
{
    auto processType = GetProcessType(requestParams.metaType);
    std::string sql;
    switch (processType) {
        case PROCESS_TYPE::ASCEND_HARDWARE:
            sql = "with nameIds as (select id, value as realName from STRING_IDS where value = ?)\n"
              "select startNs - ? as timestamp, endNs - startNs as duration, depth, main.ROWID as id from TASK  main "
              "     left join COMPUTE_TASK_INFO c on c.globalTaskId = main.globalTaskId\n"
              "     join nameIds on coalesce(c.name, main.taskType) = id  where deviceId = ? and streamId = ? "
              " and timestamp + duration >= ? AND timestamp <= ? " + orderBy;
            return ExecuteQuery(stmt, sql, requestParams.name, minTimestamp, requestParams.rankId, requestParams.tid,
                                requestParams.startTime, requestParams.endTime);
        case PROCESS_TYPE::HCCL:
            sql = "with nameIds as (select id, ? as minTime, ? as rankId, ? as startTime, ? as endTime,"
              "                  ? as tid from STRING_IDS where value = ?) "
              "select startNs-minTime as timestamp,endNs-startNs as duration,0 as depth,c.ROWID as id from  TASK main "
              "   join COMMUNICATION_TASK_INFO c on c.globalTaskId = main.globalTaskId join nameIds on c.name = id "
              " where deviceId=rankId and planeId=tid and timestamp+duration >= startTime AND timestamp <= endTime "
              " UNION ALL select op.startNs - minTime as timestamp, op.endNs - op.startNs as duration, 0 as depth, "
              " op.ROWID as id from COMMUNICATION_OP op join TASK CA on op.connectionId = CA.connectionId "
              " join  nameIds on op.opName = id where deviceId = rankId and op.groupName||'group' = tid "
              " and timestamp + duration >= startTime AND timestamp <= endTime group by opId " + orderBy;
            return ExecuteQuery(stmt, sql, minTimestamp, requestParams.rankId, requestParams.startTime,
                                requestParams.endTime, requestParams.tid, requestParams.name);
        case PROCESS_TYPE::CANN_API:
            sql = "with nameIds as (select id from STRING_IDS where value = ?)\n"
                  "select startNs - ? as timestamp, endNs - startNs as duration, depth, main.ROWID as id "
                  " from CANN_API main join  nameIds on name = id where globalTid = ? and type = ? "
                  " and timestamp + duration >= ? AND timestamp <= ? " + orderBy;
            return ExecuteQuery(stmt, sql, requestParams.name, minTimestamp, requestParams.pid, requestParams.tid,
                                requestParams.startTime, requestParams.endTime);
        case PROCESS_TYPE::API:
            sql = "with nameIds as (select id from STRING_IDS where value = ?)\n"
                  " select startNs - ? as timestamp, endNs - startNs as duration, depth, main.ROWID as id "
                  " from PYTORCH_API main join  nameIds on name = id\n"
                  "     where globalTid = ? and timestamp + duration >= ? AND timestamp <= ? " + orderBy;
            return ExecuteQuery(stmt, sql, requestParams.name, minTimestamp, requestParams.pid,
                                requestParams.startTime, requestParams.endTime);
        case PROCESS_TYPE::OVERLAP_ANALYSIS:
            sql = "select startNs - ? as timestamp, endNs - startNs as duration, 0 as depth, "
                  " main.ROWID as id from OVERLAP_ANALYSIS main "
              " where deviceId = ? and type = ? and timestamp + duration >= ? AND timestamp <= ? " + orderBy;
            return ExecuteQuery(stmt, sql, minTimestamp, requestParams.rankId, requestParams.tid,
                                requestParams.startTime, requestParams.endTime);
        default:
            throw DatabaseException("unsupported type!");
    }
}

/* Functions for JsonTraceDataBase */
std::unique_ptr<SqliteResultSet> TraceDatabaseHelper::QueryEventsViewData4Text(
    std::unique_ptr <SqlitePreparedStatement> &stmt, const Protocol::EventsViewParams &params)
{
    // 检查入参合法性
    if (params.pid.empty()) {
        ServerLog::Error("Can not to query events view data while process id is empty.");
        return nullptr;
    }
    std::string orderBy = params.orderBy.empty() ? "start" : params.orderBy;
    if (!StringUtil::CheckSqlValid(orderBy)) {
        ServerLog::Error("There is an SQL injection attack on this parameter. error param: ", orderBy);
        return nullptr;
    }
    auto metaType = GetProcessTypeByProcessName(params.processName);
    std::string baseSql;
    switch (metaType) {
        case Protocol::PROCESS_TYPE::API:
        case Protocol::PROCESS_TYPE::CANN_API:
            baseSql = "SELECT name, timestamp AS start, duration FROM slice AS s "
                      "LEFT JOIN thread AS t ON s.track_id = t.track_id ";
            break;
        case Protocol::PROCESS_TYPE::HCCL:
            baseSql = "SELECT name, timestamp AS start, duration, thread_name AS threadName FROM slice AS s "
                      "LEFT JOIN thread AS t ON s.track_id = t.track_id AND threadName NOT LIKE 'Plane%' ";
            break;
        case Protocol::PROCESS_TYPE::ASCEND_HARDWARE:
        case Protocol::PROCESS_TYPE::OVERLAP_ANALYSIS:
            baseSql = "SELECT name, timestamp AS start, duration, thread_name AS threadName FROM slice AS s "
                      "LEFT JOIN thread AS t ON s.track_id = t.track_id ";
            break;
        default:
            ServerLog::Error("QueryEventsViewData4Text. Unsupported process type.");
            return nullptr;
    }

    // 拼接SQL语句
    baseSql.append("WHERE t.pid = ? ");
    params.tid.empty() ? baseSql : baseSql.append("AND t.tid = ? ");
    std::string order = params.order == "descend" ? "DESC" : "ASC";
    baseSql.append("ORDER BY " + orderBy + " " + order + " LIMIT ? OFFSET ?");
    uint64_t limit = params.pageSize;
    uint64_t currentPage = params.currentPage > 0 ? params.currentPage : 1;
    uint64_t offset = (currentPage - 1) * limit;

    // 查询数据库
    try {
        if (params.tid.empty()) {
            return ExecuteQuery(stmt, baseSql, params.pid, limit, offset);
        } else {
            return ExecuteQuery(stmt, baseSql, params.pid, params.tid, limit, offset);
        }
    } catch (DatabaseException &e) {
        ServerLog::Error("QueryEventsViewData4Text. Execute query failed: ", e.What());
        return nullptr;
    }
}

void TraceDatabaseHelper::ResolveEventsViewResultSet(std::unique_ptr<SqliteResultSet> &resultSet,
    const Protocol::EventsViewParams &params, EventsViewBody &body, uint64_t minTimestamp)
{
    // 封装结果
    auto metaType = GetProcessTypeByProcessName(params.processName);
    while (resultSet->Next()) {
        auto ptr = std::make_unique<EventDetail>();
        if (metaType == PROCESS_TYPE::API || metaType == PROCESS_TYPE::CANN_API) {
            auto hostPtr = std::make_unique<HostEventDetail>();
            hostPtr->tid = params.tid;
            hostPtr->pid = params.pid;
            ptr = std::move(hostPtr);
        } else {
            auto devicePtr = std::make_unique<DeviceEventDetail>();
            devicePtr->threadName = resultSet->GetString("threadName");
            devicePtr->rankId = params.rankId;
            ptr = std::move(devicePtr);
        }
        ptr->name = resultSet->GetString("name");
        ptr->startTime = resultSet->GetUint64("start") - minTimestamp;
        ptr->duration = resultSet->GetUint64("duration");
        body.eventDetailList.emplace_back(std::move(ptr));
    }
    body.count = body.eventDetailList.size();
    body.currentPage = params.currentPage;
    body.pageSize = params.pageSize;
    for (const auto &item: eventsViewColumnsMap.at(metaType)) {
        body.columnList.emplace_back(item);
    }
}

void TraceDatabaseHelper::QueryThreadTracesHelper(std::vector<Protocol::RowThreadTrace> &rowThreadTraceVec,
    const Protocol::UnitThreadTracesParams &requestParams, Protocol::UnitThreadTracesBody &responseBody)
{
    for (auto &item : rowThreadTraceVec) {
        Protocol::ThreadTraces threadTraces{};
        threadTraces.id = std::to_string(item.id);
        threadTraces.name = item.name;
        threadTraces.duration = item.duration;
        threadTraces.startTime = item.startTime;
        threadTraces.endTime = item.startTime + item.duration;
        threadTraces.depth = item.depth;
        threadTraces.threadId = requestParams.threadId;
        threadTraces.cname = item.cname;
        while (responseBody.data.size() <= item.depth) {
            responseBody.data.emplace_back();
        }
        responseBody.data[item.depth].emplace_back(threadTraces);
    }
}

void TraceDatabaseHelper::QueryAllSliceInRangeByTrackIdHelper(std::unique_ptr<SqliteResultSet> &resultSet,
    uint64_t unitTime, uint64_t minTimestamp, Protocol::UnitThreadTracesSummaryBody &responseBody)
{
    uint64_t tempStartTime = 0;
    uint64_t tempEndTime = 0;
    uint64_t maxTime = 0;
    while (resultSet->Next()) {
        uint64_t curStartTime = resultSet->GetUint64("timestamp");
        uint64_t curEndTime = resultSet->GetUint64("end_time");
        if (tempEndTime + unitTime >= curStartTime) {
            tempEndTime = tempEndTime > curEndTime ? tempEndTime : curEndTime;
            maxTime = tempEndTime;
            continue;
        }
        ThreadTracesSummary summary;
        summary.startTime = tempStartTime - minTimestamp;
        summary.duration = tempEndTime - tempStartTime;
        tempStartTime = curStartTime;
        tempEndTime = curEndTime;
        responseBody.data.emplace_back(summary);
    }
    ThreadTracesSummary summary;
    summary.startTime = tempStartTime - minTimestamp;
    summary.duration = tempEndTime - tempStartTime;
    responseBody.data.emplace_back(summary);
    ServerLog::Info("Summery Size is: ", responseBody.data.size());
}
}