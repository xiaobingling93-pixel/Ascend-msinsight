/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "TraceDatabaseHelper.h"


namespace Dic::Module::Timeline {
std::map<std::string, PROCESS_TYPE> metaTypeMap = {
    {"Python", PROCESS_TYPE::API},
    {"CANN", PROCESS_TYPE::CANN_API},
    {"Ascend Hardware", PROCESS_TYPE::ASCEND_HARDWARE},
    {"HCCL", PROCESS_TYPE::HCCL},
    {"Overlap Analysis", PROCESS_TYPE::OVERLAP_ANALYSIS},
};

const Protocol::EventsViewColumnAttr columnName = {"Name", "string", "name"};
const Protocol::EventsViewColumnAttr columnStart = {"Start", "number", "start"};
const Protocol::EventsViewColumnAttr columnDuration = {"Duration(ns)", "number", "duration"};
const Protocol::EventsViewColumnAttr columnTid = {"TID", "string", "tid"};
const Protocol::EventsViewColumnAttr columnPid = {"PID", "string", "pid"};
const Protocol::EventsViewColumnAttr columnRankId = {"Rank ID", "string", "rankId"};
const Protocol::EventsViewColumnAttr columnStreamName = {"Stream Name", "string", "threadName"};
const Protocol::EventsViewColumnAttr columnGroupName = {"Group Name", "string", "threadName"};
const Protocol::EventsViewColumnAttr columnAnalysisType = {"Analysis Type", "string", "threadName"};

std::map<std::string, std::string> analysisType = {
    {"0", "Computing"},
    {"1", "Communication"},
    {"2", "Communication(Not Overlapped)"},
    {"3", "Free"},
};

std::map<PROCESS_TYPE, std::vector<Protocol::EventsViewColumnAttr>> eventsViewColumnsMap = {
    {PROCESS_TYPE::API, {columnName, columnStart, columnDuration, columnTid, columnPid}},
    {PROCESS_TYPE::CANN_API, {columnName, columnStart, columnDuration, columnTid, columnPid}},
    {PROCESS_TYPE::ASCEND_HARDWARE,
        {columnName, columnStart, columnDuration, columnStreamName, columnRankId}},
    {PROCESS_TYPE::HCCL, {columnName, columnStart, columnDuration, columnGroupName, columnRankId}},
    {PROCESS_TYPE::OVERLAP_ANALYSIS,
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
        case PROCESS_TYPE::MS_TX:
            sql = " select api.connectionId from " + TABLE_MSTX_EVENTS + " api where api.ROWID = ?";
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
        case PROCESS_TYPE::MS_TX:
            sql = "SELECT t.name as eventType, connectionId FROM " + TABLE_MSTX_EVENTS +
                  " main join " + TABLE_ENUM_MSTX_EVENTS + " t on t.id = eventType "
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
    std::unique_ptr<SqlitePreparedStatement> &stmt, const Protocol::SystemViewParams &requestParams,
    const std::string& rankId)
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
    return ExecuteQuery(stmt, mainSql + sql + orderBy + limitSql, searchName, rankId,
                        requestParams.pageSize, (requestParams.current - 1) * requestParams.pageSize);
}
std::unique_ptr<SqliteResultSet> TraceDatabaseHelper::QueryUnitCounter(std::unique_ptr<SqlitePreparedStatement> &stmt,
    const Protocol::UnitCounterParams &requestParams, uint64_t minTimestamp, const std::string& rankId)
{
    auto processType = GetProcessType(requestParams.metaType);
    switch (processType) {
        case PROCESS_TYPE::HBM:
            return ExecuteQuery(stmt, HBM_UNIT_COUNTER_SQL, minTimestamp, rankId,
                                requestParams.threadId, requestParams.startTime, requestParams.endTime);
        case PROCESS_TYPE::LLC:
            return ExecuteQuery(stmt, LLC_UNIT_COUNTER_SQL, minTimestamp, requestParams.threadId,
                                rankId, requestParams.startTime, requestParams.endTime);
        case PROCESS_TYPE::DDR:
            return ExecuteQuery(stmt, DDR_UNIT_COUNTER_SQL, minTimestamp, requestParams.threadId,
                                rankId, requestParams.startTime, requestParams.endTime);
        case PROCESS_TYPE::STARS_SOC:
            return ExecuteQuery(stmt, SOC_UNIT_COUNTER_SQL, minTimestamp, requestParams.threadId,
                                rankId, requestParams.startTime, requestParams.endTime);
        case PROCESS_TYPE::ACC_PMU:
            return ExecuteQuery(stmt, ACC_PMU_UNIT_COUNTER_SQL, requestParams.threadId, minTimestamp,
                                rankId, requestParams.startTime, requestParams.endTime);
        case PROCESS_TYPE::NPU_MEM:
            return ExecuteQuery(stmt, NPU_UNIT_COUNTER_SQL, requestParams.threadId, minTimestamp,
                                rankId, requestParams.startTime, requestParams.endTime);
        case PROCESS_TYPE::SAMPLE_PMU:
            return ExecuteQuery(stmt, SAMPLE_PMU_UNIT_COUNTER_SQL, minTimestamp, rankId,
                                requestParams.threadId, requestParams.startTime, requestParams.endTime);
        case PROCESS_TYPE::ROCE:
        case PROCESS_TYPE::ROH:
        case PROCESS_TYPE::NIC:
            return ExecuteQuery(stmt, StringUtil::ReplaceFirst(NIC_UNIT_COUNTER_SQL, "#", requestParams.metaType),
                                requestParams.threadId, minTimestamp, rankId,
                                requestParams.startTime, requestParams.endTime);
        case PROCESS_TYPE::HCCS:
            return ExecuteQuery(stmt, HCCS_UNIT_COUNTER_SQL, minTimestamp, rankId,
                                requestParams.startTime, requestParams.endTime);
        case PROCESS_TYPE::PCIE:
            return ExecuteQuery(stmt, PCIE_UNIT_COUNTER_SQL, minTimestamp, requestParams.threadId,
                                rankId, requestParams.startTime, requestParams.endTime);
        case PROCESS_TYPE::AI_CORE:
            return ExecuteQuery(stmt, AI_CORE_UNIT_COUNTER_SQL, minTimestamp,
                                rankId, requestParams.startTime, requestParams.endTime);
        default:
            throw DatabaseException("unsupported type!");
    }
}

std::unique_ptr <SqliteResultSet> TraceDatabaseHelper::QueryThreadSameOperatorsDetails(
    std::unique_ptr <SqlitePreparedStatement> &stmt, const Protocol::UnitThreadsOperatorsParams &requestParams,
    const std::string& rankId, uint64_t minTimestamp, const std::string& orderBy)
{
    auto processType = GetProcessType(requestParams.metaType);
    std::string sql;
    switch (processType) {
        case PROCESS_TYPE::ASCEND_HARDWARE:
            Prepare(stmt, ASCEND_SAME_NAME_DETAIL_SQL + orderBy)->BindParams(requestParams.name, minTimestamp);
            return Execute(stmt, rankId, requestParams.tid, requestParams.startTime, requestParams.endTime);
        case PROCESS_TYPE::HCCL:
            Prepare(stmt, HCCL_SAME_NAME_DETAIL_SQL + orderBy)->BindParams(minTimestamp, rankId);
            return Execute(stmt, requestParams.startTime, requestParams.endTime, requestParams.tid, requestParams.name);
        case PROCESS_TYPE::CANN_API:
            sql = "with nameIds as (select id from STRING_IDS where value = ?)\n"
                  "select startNs - ? as timestamp, endNs - startNs as duration, depth, main.ROWID as id "
                  " from CANN_API main join  nameIds on name = id where globalTid = ? and type = ? "
                  " and timestamp + duration >= ? AND timestamp <= ? " + orderBy;
            return ExecuteQuery(stmt, sql, requestParams.name, minTimestamp, requestParams.pid, requestParams.tid,
                                requestParams.startTime, requestParams.endTime);
        case PROCESS_TYPE::MS_TX:
            sql = "with nameIds as (select id from STRING_IDS where value = ?) "
                  " select startNs - ? as timestamp, endNs - startNs as duration, depth, main.ROWID as id "
                  " from MSTX_EVENTS main join  nameIds on message = id where globalTid = ? "
                  " and timestamp + duration >= ? AND timestamp <= ?" + orderBy;
            return ExecuteQuery(stmt, sql, requestParams.name, minTimestamp, requestParams.pid,
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
            return ExecuteQuery(stmt, sql, minTimestamp, rankId, requestParams.tid,
                                requestParams.startTime, requestParams.endTime);
        default:
            throw DatabaseException("unsupported type!");
    }
}

std::optional<SliceDto> TraceDatabaseHelper::QueryThreadDetail(std::unique_ptr<SqlitePreparedStatement> &stmt,
    const Protocol::ThreadDetailParams &requestParams)
{
    std::string sql;
    auto processType = GetProcessType(requestParams.metaType);
    std::unique_ptr<SqliteResultSet> resultSet;
    switch (processType) {
        case PROCESS_TYPE::ASCEND_HARDWARE:
            resultSet = ExecuteQuery(stmt, ASCEND_THREAD_DETAIL, requestParams.id);
            break;
        case PROCESS_TYPE::HCCL:
            resultSet = ExecuteQuery(stmt, HCCL_THREAD_DETAIL, requestParams.id, requestParams.tid,
                                     requestParams.id, requestParams.tid);
            break;
        case PROCESS_TYPE::CANN_API:
            resultSet = ExecuteQuery(stmt, CANN_API_THREAD_DETAIL, requestParams.id);
            break;
        case PROCESS_TYPE::API:
            resultSet = ExecuteQuery(stmt, PYTORCH_API_THREAD_DETAIL, requestParams.id);
            break;
        case PROCESS_TYPE::OVERLAP_ANALYSIS:
            resultSet = ExecuteQuery(stmt, OVERLAP_THREAD_DETAIL, requestParams.id);
            break;
        case PROCESS_TYPE::MS_TX:
            resultSet = ExecuteQuery(stmt, MSTX_THREAD_DETAIL, requestParams.id);
            break;
        default:
            throw DatabaseException("unsupported type!");
    }
    if (resultSet->Next()) {
        SliceDto sliceDto{};
        sliceDto.cardId = requestParams.rankId;
        sliceDto.metaType = requestParams.metaType;
        sliceDto.id = resultSet->GetInt64("id");
        sliceDto.timestamp = resultSet->GetInt64("startNs");
        sliceDto.duration = resultSet->GetInt64("duration");
        sliceDto.depth = resultSet->GetInt32("depth");
        sliceDto.pid = resultSet->GetString("pid");
        sliceDto.tid = resultSet->GetString("tid");
        auto path = DataBaseManager::Instance().GetDbPath(requestParams.rankId);
        sliceDto.name = FullDb::DbTraceDataBase::GetStringCacheValue(path, resultSet->GetString("name"));
        return sliceDto;
    }
    return std::nullopt;
}

std::unique_ptr<SqliteResultSet> TraceDatabaseHelper::QueryThreadTracesSummary(
    std::unique_ptr<SqlitePreparedStatement> &stmt, const Protocol::UnitThreadTracesSummaryParams &requestParams,
    const std::string& rankId, uint64_t minTimestamp)
{
    std::string sql;
    auto processType = GetProcessType(requestParams.metaType);
    switch (processType) {
        case PROCESS_TYPE::ASCEND_HARDWARE:
            sql = "SELECT startNs - ? as start_time, endNs - startNs as duration, endNs - ? as end_time "
                  "FROM " + TABLE_TASK + " WHERE deviceId = ? AND start_time >= ? "
                  "AND start_time <= ? AND depth = 0 ORDER BY startNs;";
            return ExecuteQuery(stmt, sql, minTimestamp, minTimestamp, rankId,
                                requestParams.startTime, requestParams.endTime);
        case PROCESS_TYPE::HCCL:
            sql = "SELECT startNs - ? as start_time, endNs - startNs as duration, endNs - ? as end_time "
                  "FROM " + TABLE_TASK + " main join " + TABLE_COMMUNICATION_TASK_INFO + " info "
                  " on main.globalTaskId = info.globalTaskId"
                  " WHERE deviceId = ? AND start_time >= ? AND start_time <= ? ORDER BY startNs;";
            return ExecuteQuery(stmt, sql, minTimestamp, minTimestamp, rankId,
                                requestParams.startTime, requestParams.endTime);
        case PROCESS_TYPE::CANN_API:
            sql = "with constValue as (select ? as minTime, ? as pid, ? as startTime, ? as endTime) "
                  "SELECT startNs - minTime as start_time, endNs - startNs as duration, endNs - minTime as end_time "
                  "    FROM " + TABLE_CANN_API + " join constValue WHERE globalTid = pid "
                  " AND start_time >= startTime AND start_time <= endTime "
                  " UNION SELECT startNs -minTime as start_time,endNs - startNs as duration,"
                  "    endNs - minTime as end_time from  " + TABLE_API + " join constValue "
                  " WHERE globalTid = pid AND start_time >= startTime AND start_time <= endTime ORDER BY start_time;";
            return ExecuteQuery(stmt, sql, minTimestamp, requestParams.processId,
                                requestParams.startTime, requestParams.endTime);
        default:
            throw DatabaseException("unsupported type!");
    }
}

std::unique_ptr<SqliteResultSet> TraceDatabaseHelper::QueryThreadsByPid(std::unique_ptr<SqlitePreparedStatement> &stmt,
                                                                        uint64_t startTime,
                                                                        uint64_t endTime,
                                                                        const Dic::Protocol::Metadata &metaData,
                                                                        const std::string &rankId)
{
    std::string sql;
    auto processType = GetProcessType(metaData.metaType);
    switch (processType) {
        case PROCESS_TYPE::ASCEND_HARDWARE:
            return ExecuteQuery(stmt, ASCEND_THREADS_BY_PID, rankId, metaData.tid,
                                startTime, endTime);
        case PROCESS_TYPE::HCCL:
            return ExecuteQuery(stmt, HCCL_THREADS_BY_PID, rankId, metaData.tid, metaData.tid,
                                startTime, endTime);
        case PROCESS_TYPE::CANN_API:
            return ExecuteQuery(stmt, CANN_API_THREADS_BY_PID, metaData.tid, metaData.pid,
                                startTime, endTime);
        case PROCESS_TYPE::API:
            return ExecuteQuery(stmt, API_THREADS_BY_PID, metaData.pid,
                                startTime, endTime);
        case PROCESS_TYPE::OVERLAP_ANALYSIS:
            return ExecuteQuery(stmt, OVERLAP_ANALYSIS_THREAD_BY_PID, rankId, metaData.tid,
                                startTime, endTime);
        case PROCESS_TYPE::MS_TX:
            return ExecuteQuery(stmt, MS_TX_THREAD_BY_PID, metaData.pid, startTime,
                                endTime);
        default:
            throw DatabaseException("unsupported type!");
    }
};

std::unique_ptr <SqliteResultSet> QueryEventsView4Process(std::unique_ptr <SqlitePreparedStatement> &stmt,
    std::string &orderByCondition, const Protocol::EventsViewParams &params)
{
    // pid匹配的入参为globalTid的高32位
    std::string sql = "SELECT pa.ROWID as id, value AS name, startNs AS start, (endNs - startNs) AS duration, "
        "(globalTid & 0xFFFFFFFF) AS tid, (globalTid / 4294967296) AS pid, depth, globalTid as processId, "
        "'pytorch' as threadId FROM PYTORCH_API AS pa LEFT JOIN STRING_IDS AS si ON pa.name = si.id "
        "WHERE pid||'' = ? UNION "
        "SELECT ca.ROWID as id, value AS name, startNs AS start, (endNs - startNs) AS duration, "
        "(globalTid & 0xFFFFFFFF) AS tid, (globalTid / 4294967296) AS pid, depth, "
        "globalTid as processId, type as threadId "
        "FROM CANN_API AS ca LEFT JOIN STRING_IDS AS si ON ca.name = si.id "
        "WHERE pid||'' = ? ";
    return TraceDatabaseHelper::ExecuteQuery(stmt, sql.append(orderByCondition), params.pid, params.pid);
}

std::unique_ptr <SqliteResultSet> QueryEventsView4Thread(std::unique_ptr <SqlitePreparedStatement> &stmt,
    std::string &orderByCondition, const Protocol::EventsViewParams &params)
{
    std::string sql = "SELECT pa.ROWID as id, value AS name, startNs AS start, (endNs - startNs) AS duration, "
        "(globalTid & 0xFFFFFFFF) AS tid, (globalTid / 4294967296) AS pid, depth, globalTid as processId, "
        "'pytorch' as threadId FROM PYTORCH_API AS pa LEFT JOIN STRING_IDS AS si ON pa.name = si.id "
        "WHERE globalTid = ? UNION "
        "SELECT ca.ROWID as id, value AS name, startNs AS start, (endNs - startNs) AS duration, "
        "(globalTid & 0xFFFFFFFF) AS tid, (globalTid / 4294967296) AS pid, "
        "depth, globalTid as processId, type as threadId "
        "FROM CANN_API AS ca LEFT JOIN STRING_IDS AS si ON ca.name = si.id "
        "WHERE globalTid = ? ";
    return TraceDatabaseHelper::ExecuteQuery(stmt, sql.append(orderByCondition), params.pid, params.pid);
}

std::unique_ptr <SqliteResultSet> QueryEventsView4Pytorch(std::unique_ptr <SqlitePreparedStatement> &stmt,
    std::string &orderByCondition, const Protocol::EventsViewParams &params)
{
    std::string sql = "SELECT pa.ROWID as id, value AS name, startNs AS start, (endNs - startNs) AS duration, "
        "(globalTid & 0xFFFFFFFF) AS tid, depth, globalTid as processId, 'pytorch' as threadId, "
        "(globalTid / 4294967296) AS pid FROM PYTORCH_API AS pa LEFT JOIN STRING_IDS AS si ON pa.name = si.id "
        "WHERE globalTid = ? ";
    return TraceDatabaseHelper::ExecuteQuery(stmt, sql.append(orderByCondition), params.pid);
}

std::unique_ptr <SqliteResultSet> QueryEventsView4HostHccl(std::unique_ptr <SqlitePreparedStatement> &stmt,
    std::string &orderByCondition, const Protocol::EventsViewParams &params)
{
    std::string sql = "SELECT ca.ROWID as id, value AS name, startNs AS start, (endNs - startNs) AS duration, "
        "(globalTid & 0xFFFFFFFF) AS tid, depth, globalTid as processId, type as threadId, "
        "(globalTid / 4294967296) AS pid FROM CANN_API AS ca LEFT JOIN STRING_IDS AS si ON ca.name = si.id "
        "WHERE globalTid = ? AND ca.type = 5500 ";
    return TraceDatabaseHelper::ExecuteQuery(stmt, sql.append(orderByCondition), params.pid);
}

std::unique_ptr <SqliteResultSet> QueryEventsView4CANN(std::unique_ptr <SqlitePreparedStatement> &stmt,
    std::string &orderByCondition, const Protocol::EventsViewParams &params)
{
    std::string sql = "SELECT ca.ROWID as id, value AS name, startNs AS start, (endNs - startNs) AS duration, "
        "(globalTid & 0xFFFFFFFF) AS tid, depth, globalTid as processId, type as threadId, "
        "(globalTid / 4294967296) AS pid FROM CANN_API AS ca LEFT JOIN STRING_IDS AS si ON ca.name = si.id "
        "WHERE globalTid = ? AND ca.type IN (5000, 10000, 15000, 20000) ";
    return TraceDatabaseHelper::ExecuteQuery(stmt, sql.append(orderByCondition), params.pid);
}

std::unique_ptr <SqliteResultSet> QueryEventsView4SubCANN(std::unique_ptr <SqlitePreparedStatement> &stmt,
    std::string &orderByCondition, const Protocol::EventsViewParams &params)
{
    std::string sql = "SELECT ca.ROWID as id, value AS name, startNs AS start, (endNs - startNs) AS duration, "
        "(globalTid & 0xFFFFFFFF) AS tid, depth, globalTid as processId, type as threadId, "
        "(globalTid / 4294967296) AS pid FROM CANN_API AS ca LEFT JOIN STRING_IDS AS si ON ca.name = si.id "
        "WHERE globalTid = ? AND ca.type = ? ";
    return TraceDatabaseHelper::ExecuteQuery(stmt, sql.append(orderByCondition), params.pid, params.tid);
}

std::unique_ptr <SqliteResultSet> QueryEventsView4Hardware(std::unique_ptr <SqlitePreparedStatement> &stmt,
    std::string &orderByCondition, const Protocol::EventsViewParams &params, const std::string& rankId)
{
    std::string sql = "SELECT main.ROWID as id, si.value AS name, startNs AS start, endNs - startNs as duration, "
        "'Stream '||streamId as threadName, depth, 'Ascend Hardware' as processId, streamId as threadId, "
        "deviceId AS rankId FROM  TASK AS main LEFT JOIN COMPUTE_TASK_INFO AS CTI "
        "on CTI.globalTaskId = main.globalTaskId "
        "LEFT JOIN STRING_IDS AS si ON si.id = coalesce(CTI.name, main.taskType) WHERE main.deviceId = ? ";
    return TraceDatabaseHelper::ExecuteQuery(stmt, sql.append(orderByCondition), rankId);
}

std::unique_ptr <SqliteResultSet> QueryEventsView4Stream(std::unique_ptr <SqlitePreparedStatement> &stmt,
    std::string &orderByCondition, const Protocol::EventsViewParams &params, const std::string& rankId)
{
    std::string sql = "SELECT main.ROWID as id, si.value AS name, startNs AS start, endNs - startNs as duration, "
        "'Stream '||streamId as threadName, deviceId AS rankId, depth, 'Ascend Hardware' as processId, "
        "streamId as threadId FROM TASK AS main "
        "LEFT JOIN COMPUTE_TASK_INFO AS CTI on CTI.globalTaskId = main.globalTaskId "
        "LEFT JOIN STRING_IDS AS si ON si.id = coalesce(CTI.name, main.taskType) "
        "WHERE main.deviceId = ? AND main.streamId = ? ";
    return TraceDatabaseHelper::ExecuteQuery(stmt, sql.append(orderByCondition), rankId, params.tid);
}

std::unique_ptr <SqliteResultSet> QueryEventsView4DeviceHCCL(std::unique_ptr <SqlitePreparedStatement> &stmt,
    std::string &orderByCondition, const Protocol::EventsViewParams &params, const std::string& rankId)
{
    std::string sql = "with tmp as (select * from TASK main join COMMUNICATION_TASK_INFO "
        "info on info.globalTaskId = main.globalTaskId where main.deviceId = ?), "
        "sub as (select COMMUNICATION_OP.ROWID, startNs, endNs-startNs as duration, si.value as name, groupName "
        "from COMMUNICATION_OP LEFT JOIN STRING_IDS AS si ON si.id = opName "
        "where opId in (select opId from tmp group by opId)) "
        "select ROWID as id, name, startNs as start, duration, 0 as depth, 'HCCL' as processId, "
        "groupName||'group' as threadId, "
        "'Group '||(DENSE_RANK() OVER (ORDER BY groupName))||' Communication' AS threadName, "
        "? AS rankId from sub ";
    return TraceDatabaseHelper::ExecuteQuery(stmt, sql.append(orderByCondition), rankId, rankId);
}

std::unique_ptr <SqliteResultSet> QueryEventsView4Group(std::unique_ptr <SqlitePreparedStatement> &stmt,
    std::string &orderByCondition, const Protocol::EventsViewParams &params, const std::string& rankId)
{
    std::string sql = "with tmp as (select * from TASK main join COMMUNICATION_TASK_INFO "
        "info on info.globalTaskId = main.globalTaskId where main.deviceId = ?), "
        "sub as (select COMMUNICATION_OP.ROWID, startNs, endNs-startNs as duration, si.value as name, groupName "
        "from COMMUNICATION_OP LEFT JOIN STRING_IDS AS si ON si.id = opName "
        "where opId in (select opId from tmp group by opId)) "
        "select ROWID as id, name, startNs as start, duration, 0 as depth, 'HCCL' as processId, "
        "groupName||'group' as threadId, ? AS threadName, ? AS rankId from sub WHERE groupName||'group' = ? ";
    return TraceDatabaseHelper::ExecuteQuery(stmt, sql.append(orderByCondition),
        params.rankId, params.threadName, rankId, params.tid);
}

std::unique_ptr <SqliteResultSet> QueryEventsView4Overlap(std::unique_ptr <SqlitePreparedStatement> &stmt,
    std::string &orderByCondition, const Protocol::EventsViewParams &params)
{
    std::string sql = "select OVERLAP_ANALYSIS.ROWID as id, type AS name, startNs as start, "
                      "endNs - startNs as duration, type AS threadName, "
                      "0 as depth, 'OVERLAP_ANALYSIS' as processId, type as threadId, "
                      "deviceId AS rankId from OVERLAP_ANALYSIS where deviceId = ? ";
    return TraceDatabaseHelper::ExecuteQuery(stmt, sql.append(orderByCondition), params.rankId);
}

std::unique_ptr <SqliteResultSet> QueryEventsView4OverlapSub(std::unique_ptr <SqlitePreparedStatement> &stmt,
    std::string &orderByCondition, const Protocol::EventsViewParams &params)
{
    std::string sql = "select OVERLAP_ANALYSIS.ROWID as id, type AS name, startNs as start, "
                      "endNs - startNs as duration, type AS threadName, "
                      "0 as depth, 'OVERLAP_ANALYSIS' as processId, type as threadId, "
                      "deviceId AS rankId from OVERLAP_ANALYSIS where deviceId = ? AND type = ? ";
    return TraceDatabaseHelper::ExecuteQuery(stmt, sql.append(orderByCondition), params.rankId, params.tid);
}

std::unique_ptr <SqliteResultSet> GetEventsViewResult4CANNAPI(std::unique_ptr <SqlitePreparedStatement> &stmt,
    std::string &orderByCondition, const Protocol::EventsViewParams &params)
{
    // 根据泳道的类型，分别调用不同的query语句
    std::string processName = params.processName;
    if (StringUtil::StartWith(processName, "process")) {
        return QueryEventsView4Process(stmt, orderByCondition, params);
    } else if (StringUtil::StartWith(processName, "Thread")) {
        if (params.tid.empty() && params.threadName.empty()) {
            return QueryEventsView4Thread(stmt, orderByCondition, params);
        } else if (params.threadName == "hccl") {
            return QueryEventsView4HostHccl(stmt, orderByCondition, params);
        }
    } else if (StringUtil::StartWith(processName, "CANN")) {
        if (params.tid.empty() && params.threadName.empty()) {
            return QueryEventsView4CANN(stmt, orderByCondition, params);
        } else {
            return QueryEventsView4SubCANN(stmt, orderByCondition, params);
        }
    }
    return nullptr;
}

void ResolveEventsViewResultSet4Db(std::unique_ptr <SqliteResultSet> &resultSet,
    const Protocol::EventsViewParams &params, Protocol::EventsViewBody &body, uint64_t minTimestamp)
{
    auto metaType = TraceDatabaseHelper::GetProcessType(params.metaType);
    std::vector<std::unique_ptr<EventDetail>> details;
    while (resultSet->Next()) {
        auto ptr = std::make_unique<EventDetail>();
        // 根据泳道类型，创建对应子类对象的指针，填充特有数据
        if (metaType == PROCESS_TYPE::API || metaType == PROCESS_TYPE::CANN_API) {
            auto hostPtr = std::make_unique<HostEventDetail>();
            hostPtr->tid = resultSet->GetString("tid");
            hostPtr->pid = resultSet->GetString("pid");
            ptr = std::move(hostPtr);
        } else {
            auto devicePtr = std::make_unique<DeviceEventDetail>();
            devicePtr->threadName = resultSet->GetString("threadName");
            devicePtr->rankId =  resultSet->GetString("rankId");
            ptr = std::move(devicePtr);
        }
        // 父类指针，填充公共数据
        ptr->id = resultSet->GetString("id");
        ptr->name = resultSet->GetString("name");
        ptr->startTime = resultSet->GetUint64("start") - minTimestamp;
        ptr->duration = resultSet->GetUint64("duration");
        ptr->depth = resultSet->GetUint64("depth");
        ptr->threadId = resultSet->GetString("threadId");
        ptr->processId = resultSet->GetString("processId");
        details.emplace_back(std::move(ptr));
    }
    body.count = details.size();
    body.currentPage = params.currentPage;
    body.pageSize = params.pageSize;
    // 计算分页信息，获取对应分页的数据
    auto indexStart = (params.currentPage - 1) * params.pageSize;
    auto indexEnd = indexStart + params.pageSize;
    indexEnd = indexEnd > details.size() ? details.size() : indexEnd;
    for (uint64_t i = indexStart; i < indexEnd; ++i) {
        body.eventDetailList.emplace_back(std::move(details.at(i)));
    }
    if (metaType == Protocol::PROCESS_TYPE::OVERLAP_ANALYSIS) {
        for (const auto &item: body.eventDetailList) {
            auto overlapPtr = dynamic_cast<DeviceEventDetail*>(item.get());
            if (overlapPtr) {
                overlapPtr->name = analysisType.at(overlapPtr->name);
                overlapPtr->threadName = analysisType.at(overlapPtr->threadName);
            }
        }
    }
    for (const auto &item: eventsViewColumnsMap.at(metaType)) {
        body.columnList.emplace_back(item);
    }
}

std::string TraceDatabaseHelper::GetOrderByCondition(const Protocol::EventsViewParams &params)
{
    std::string orderBy = params.orderBy.empty() ? "start" : params.orderBy;
    if (!StringUtil::CheckSqlValid(orderBy)) {
        return std::string{};
    }
    std::string order = params.order == "descend" ? "DESC" : "ASC";
    std::string orderByCondition = " ORDER BY " + orderBy + " " + order;
    return orderByCondition;
}

bool TraceDatabaseHelper::QueryEventsViewData4Db(std::unique_ptr <SqlitePreparedStatement> &stmt,
    const Protocol::EventsViewParams &params, Protocol::EventsViewBody &body, uint64_t minTimestamp,
    const std::string& rankId)
{
    std::string orderByCondition = GetOrderByCondition(params);
    if (orderByCondition.empty()) {
        return false;
    }
    auto metaType = GetProcessType(params.metaType);
    std::unique_ptr <SqliteResultSet> resultSet;
    try {
        switch (metaType) { // 根据不同的泳道类型，调用不同的query查询数据表
            case Protocol::PROCESS_TYPE::API:
                resultSet = QueryEventsView4Pytorch(stmt, orderByCondition, params);
                break;
            case Protocol::PROCESS_TYPE::CANN_API:
                resultSet = GetEventsViewResult4CANNAPI(stmt, orderByCondition, params);
                break;
            case Protocol::PROCESS_TYPE::ASCEND_HARDWARE:
                if (params.tid.empty() && params.threadName.empty()) {
                    resultSet = QueryEventsView4Hardware(stmt, orderByCondition, params, rankId);
                } else {
                    resultSet = QueryEventsView4Stream(stmt, orderByCondition, params, rankId);
                }
                break;
            case Protocol::PROCESS_TYPE::HCCL:
                if (params.tid.empty() && params.threadName.empty()) {
                    resultSet = QueryEventsView4DeviceHCCL(stmt, orderByCondition, params, rankId);
                } else {
                    resultSet = QueryEventsView4Group(stmt, orderByCondition, params, rankId);
                }
                break;
            case Protocol::PROCESS_TYPE::OVERLAP_ANALYSIS:
                if (params.tid.empty() && params.threadName.empty()) {
                    resultSet = QueryEventsView4Overlap(stmt, orderByCondition, params);
                } else {
                    resultSet = QueryEventsView4OverlapSub(stmt, orderByCondition, params);
                }
                break;
            default:
                ServerLog::Warn("No defined query way");
        }
    } catch (DatabaseException &de) {
        ServerLog::Error("Query events view data for DB. Execute query failed: ", de.What());
        return false;
    }
    if (resultSet == nullptr) {
        ServerLog::Error("Query events view data for DB. Sqlite result set is null.");
        return false;
    }
    // 解析查询结果，并封装到Response body中
    ResolveEventsViewResultSet4Db(resultSet, params, body, minTimestamp);
    return true;
}

/* Functions for JsonTraceDataBase */
PROCESS_TYPE GetProcessTypeByProcessName(const std::string &processName)
{
    for (const auto &item: metaTypeMap) {
        if (StringUtil::StartWith(processName, item.first)) {
            return item.second;
        }
    }
    return PROCESS_TYPE::NONE;
}

std::string GetSql4QueryEventsViewDetailsInText(const Protocol::EventsViewParams &params)
{
    std::string baseSql;
    auto metaType = GetProcessTypeByProcessName(params.processName);
    switch (metaType) {
        case PROCESS_TYPE::API:
        case PROCESS_TYPE::CANN_API:
            baseSql = "SELECT id, name, timestamp AS start, duration, tid, pid, s.track_id, tid AS threadId, "
                      "pid AS processId FROM slice AS s LEFT JOIN thread AS t ON s.track_id = t.track_id ";
            break;
        case PROCESS_TYPE::HCCL:
            baseSql = "SELECT id, name, timestamp AS start, duration, thread_name AS threadName, s.track_id, "
                      "tid AS threadId, pid AS processId FROM slice AS s "
                      "LEFT JOIN thread AS t ON s.track_id = t.track_id AND threadName NOT LIKE 'Plane%' ";
            break;
        case PROCESS_TYPE::ASCEND_HARDWARE:
        case PROCESS_TYPE::OVERLAP_ANALYSIS:
            baseSql = "SELECT id, name, timestamp AS start, duration, thread_name AS threadName, s.track_id, "
                      "tid AS threadId, pid AS processId FROM slice AS s "
                      "LEFT JOIN thread AS t ON s.track_id = t.track_id ";
            break;
        default:
            ServerLog::Error("Query events view data for text. Unsupported process type.");
    }
    return baseSql;
}

void ResolveEventsViewResultSet(std::unique_ptr<SqliteResultSet> &resultSet,
    const Protocol::EventsViewParams &params, EventsViewBody &body, uint64_t minTimestamp)
{
    // 封装结果
    auto metaType = GetProcessTypeByProcessName(params.processName);
    std::vector<std::unique_ptr<EventDetail>> details;
    while (resultSet->Next()) {
        auto ptr = std::make_unique<EventDetail>();
        if (metaType == PROCESS_TYPE::API || metaType == PROCESS_TYPE::CANN_API) {
            auto hostPtr = std::make_unique<HostEventDetail>();
            hostPtr->tid = resultSet->GetString("tid");
            hostPtr->pid = resultSet->GetString("pid");
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
        ptr->threadId = resultSet->GetString("threadId");
        ptr->processId = resultSet->GetString("processId");
        ptr->id = std::to_string(resultSet->GetUint64("id"));
        details.emplace_back(std::move(ptr));
    }
    body.count = details.size();
    body.currentPage = params.currentPage;
    body.pageSize = params.pageSize;
    // 计算分页信息，获取对应分页的数据
    auto indexStart = (params.currentPage - 1) * params.pageSize;
    auto indexEnd = indexStart + params.pageSize;
    indexEnd = indexEnd > details.size() ? details.size() : indexEnd;
    for (uint64_t i = indexStart; i < indexEnd; ++i) {
        body.eventDetailList.emplace_back(std::move(details.at(i)));
    }
    for (const auto &item: eventsViewColumnsMap.at(metaType)) {
        body.columnList.emplace_back(item);
    }
}

bool TraceDatabaseHelper::QueryEventsViewData4Text(std::unique_ptr <SqlitePreparedStatement> &stmt,
    const Protocol::EventsViewParams &params, Protocol::EventsViewBody &body, uint64_t minTimestamp)
{
    // 检查入参合法性
    if (params.pid.empty()) {
        ServerLog::Error("Can not to query events view data while process id is empty.");
        return false;
    }
    std::string orderBy = params.orderBy.empty() ? "start" : params.orderBy;
    if (!StringUtil::CheckSqlValid(orderBy)) {
        ServerLog::Error("Query events view data text is an SQL injection attack");
        return false;
    }
    std::string sql4Details = GetSql4QueryEventsViewDetailsInText(params);

    // 拼接SQL语句
    sql4Details.append("WHERE t.pid = ? ");
    if (!params.tid.empty()) {
        sql4Details.append("AND t.tid = ? ");
    }

    // 拼接分页相关的条件
    std::string order = params.order == "descend" ? "DESC" : "ASC";
    sql4Details.append("ORDER BY " + orderBy + " " + order);

    // 查询数据库
    std::unique_ptr<SqliteResultSet> resultSet;
    try {
        if (params.tid.empty()) {
            resultSet = ExecuteQuery(stmt, sql4Details, params.pid);
        } else {
            resultSet = ExecuteQuery(stmt, sql4Details, params.pid, params.tid);
        }
    } catch (DatabaseException &e) {
        ServerLog::Error("Query events view data for text. Execute query failed: ", e.What());
        return false;
    }
    ResolveEventsViewResultSet(resultSet, params, body, minTimestamp);
    return true;
}

void TraceDatabaseHelper::QueryAllSliceInRangeByTrackIdHelper(std::unique_ptr<SqliteResultSet> &resultSet,
    uint64_t unitTime, uint64_t minTimestamp, Protocol::UnitThreadTracesSummaryBody &responseBody)
{
    uint64_t tempStartTime = 0;
    uint64_t tempEndTime = 0;
    while (resultSet->Next()) {
        uint64_t curStartTime = resultSet->GetUint64("timestamp");
        uint64_t curEndTime = resultSet->GetUint64("end_time");
        if (tempEndTime + unitTime >= curStartTime) {
            tempEndTime = tempEndTime > curEndTime ? tempEndTime : curEndTime;
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
    ServerLog::Info("Summary Size is: ", responseBody.data.size());
}

void TraceDatabaseHelper::SetKernelDetailHelpler(std::unique_ptr<SqliteResultSet> resultSet, uint64_t minTimestamp,
                                                 Protocol::KernelDetailsBody &responseBody)
{
    while (resultSet->Next()) {
        Protocol::KernelDetail detail;
        detail.id = resultSet->GetString("id");
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

void TraceDatabaseHelper::FilterTopLevelApi(std::vector<Protocol::FlowLocation> &originData,
    const std::set<std::string> &pattern, std::vector<Protocol::FlowLocation> &filterData,
    std::vector<uint32_t> &indexes)
{
    std::vector<uint64_t> endList;
    uint32_t index = 0;
    for (auto &item : originData) {
        uint32_t depth = 0;
        while (depth < endList.size() && endList[depth] > item.timestamp) {
            depth++;
        }
        if (depth < endList.size()) {
            endList[depth] = item.duration;
        } else {
            endList.emplace_back(item.duration);
        }
        if (depth != 0) { // 过滤顶层API
            continue;
        }
        filterData.emplace_back(item);
        if (pattern.find(item.name) != pattern.end()) {
            indexes.emplace_back(index); // 标记潜在的亲和API第一个索引位置
        }
        index++;
    }
}
}