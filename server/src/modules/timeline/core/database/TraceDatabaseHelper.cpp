/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "TraceDatabaseHelper.h"
#include <sstream>
#include <algorithm>
#include "CounterEventHelper.h"
#include "NpuInfoRepo.h"
#include "Database.h"
#include "TraceDatabaseHelper.h"
#include "OverlapAnsRepo.h"

namespace Dic::Module::Timeline {
std::map<std::string, PROCESS_TYPE> metaTypeMap = {
    {"Python", PROCESS_TYPE::API},
    {"python3", PROCESS_TYPE::API},
    {"MindSpore", PROCESS_TYPE::API},
    {"CANN", PROCESS_TYPE::CANN_API},
    {"Ascend Hardware", PROCESS_TYPE::ASCEND_HARDWARE},
    {"HCCL", PROCESS_TYPE::HCCL},
    {"Communication", PROCESS_TYPE::HCCL},
    {"Overlap Analysis", PROCESS_TYPE::OVERLAP_ANALYSIS},
    {"Python GC", PROCESS_TYPE::PYTHON_GC},
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
    {PROCESS_TYPE::MS_TX, {columnName, columnStart, columnDuration, columnTid, columnPid}},
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

std::string TraceDatabaseHelper::GetSystemViewSqlByLayer(const std::string &layer, const std::string &rankId)
{
    std::string mainSql;
    if (layer == "Ascend Hardware") {
        mainSql = "with nameIds as ( select id, value as realName from STRING_IDS where lower(value) like ?),\n"
                  "  main as (select coalesce(a.realName, c.realName, b.realName) as name, "
                  "  endNs - startNs as duration from TASK task\n"
                  "  left join COMPUTE_TASK_INFO info on info.globalTaskId = task.globalTaskId "
                  "  left join COMMUNICATION_SCHEDULE_TASK_INFO schedule on task.globalTaskId = schedule.globalTaskId"
                  "  left join nameIds a on info.name = a.id "
                  "  left join nameIds b on task.taskType = b.id"
                  "  left join nameIds c on schedule.name = c.id"
                  "  where deviceId = ?),";
    } else if (layer == "HCCL" || layer == "COMMUNICATION") {
        std::string comSql;
        if (IsDeviceIdUnique(rankId)) {
            comSql = " select realName as name, op.endNs - op.startNs as duration "
                    "  from COMMUNICATION_OP op join nameIds on op.opName = id join rankId"
                    "  group by opId";
        } else {
            comSql = "select realName as name, op.endNs - op.startNs as duration "
                     "  from COMMUNICATION_OP op join nameIds on op.opName = id join rankId"
                     "  join TASK task on task.connectionId = op.connectionId"
                     "  where task.deviceId = rankId.deviceId group by opId";
        }
        mainSql = "with nameIds as ( select id, value as realName from STRING_IDS where lower(value) like ?), "
                  "     rankId as (select ? as deviceId),\n"
                  "  main as (select realName as name, endNs - startNs as duration from TASK task join rankId "
                  "  join COMMUNICATION_TASK_INFO info on info.globalTaskId = task.globalTaskId "
                  "  join nameIds on info.taskType = id "
                  "  where task.deviceId = rankId.deviceId "
                  "  UNION ALL " + comSql + "),";
    } else if (layer == "CANN") {
        mainSql = "with nameIds as ( select id, value as realName from STRING_IDS where lower(value) like ?), "
                  "     tmp as (select globalPid from TASK where deviceId = ? group by globalPid), "
                  "     main as (select realName as name, endNs - startNs as duration from CANN_API api "
                  " join tmp on api.globalTid >> 32 = tmp.globalPid join nameIds on name = id),";
    } else if (layer == "Python") {
        mainSql = "with nameIds as ( select id, value as realName from STRING_IDS where lower(value) like ?), "
                  "     tmp as (select globalPid from TASK where deviceId = ? group by globalPid), "
                  "     main as (select realName as name, endNs - startNs as duration from PYTORCH_API api "
                  " join tmp on api.globalTid >> 32 = tmp.globalPid join nameIds on name = id),";
    } else if (layer == "Overlap Analysis") {
        mainSql = " with main as (select case type when 0 then 'Computing' when 1 then 'Communication' "
                  "        when 2 then 'Communication(Not Overlapped)' else 'Free' end as name, "
                  "  endNs - startNs as duration from OVERLAP_ANALYSIS task where name like ? and deviceId = ?),";
    } else {
        throw DatabaseException("unsupported type!");
    }
    return mainSql;
}

std::unique_ptr<SqliteResultSet> TraceDatabaseHelper::QuerySystemViewData(
    std::unique_ptr<SqlitePreparedStatement> &stmt, const Protocol::SystemViewParams &requestParams,
    const std::string& rankId)
{
    std::string searchName = "%" + requestParams.searchName + "%";
    std::transform(searchName.begin(), searchName.end(), searchName.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    std::string orderBy;
    if (!StringUtil::CheckSqlValid(requestParams.orderBy)) {
        throw DatabaseException("There is an SQL injection attack on this parameter.");
    }
    if (requestParams.order == "descend") {
        orderBy = " ORDER BY " + requestParams.orderBy + " DESC";
    } else {
        orderBy = " ORDER BY " + requestParams.orderBy + " ASC";
    }
    auto sql = " total as (select sum(case when name != 'Communication' then duration else 0 end) as totalTime, "
     " count(distinct name) as num from main) select name, round(sum(duration)*100.0/total.totalTime, 4) as time, "
     "sum(duration) / 1000.0 as totalTime, count(1) as numberCalls, round(avg(duration) / 1000.0, 2) as avg, "
     "min(duration) / 1000.0 as min, max(duration) / 1000.0 as max, total.num from main join total group by name ";
    auto limitSql = " limit ? offset ?";
    std::string mainSql = GetSystemViewSqlByLayer(requestParams.layer, requestParams.rankId);
    return ExecuteQuery(stmt, mainSql + sql + orderBy + limitSql, searchName, rankId,
                        requestParams.pageSize, (requestParams.current - 1) * requestParams.pageSize);
}

bool TraceDatabaseHelper::QueryFusibleOpDataForDB(const KernelDetailsParams &params,
                                                  std::unique_ptr<SqlitePreparedStatement> &stmt,
                                                  const Dic::Module::Timeline::FuseableOpRule &rule,
                                                  std::vector<Protocol::FlowLocation> &data, uint64_t minTimestamp)
{
    int deviceId = StringUtil::StringToInt(params.deviceId);
    auto resultSet = stmt->ExecuteQuery(minTimestamp, deviceId);
    if (resultSet == nullptr) {
        ServerLog::Error("Failed to get result set for query Fusible Operator.", stmt->GetErrorMessage());
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

bool TraceDatabaseHelper::QueryOpDispatchDataForDB(std::unique_ptr<SqlitePreparedStatement> &stmt,
                                                   uint64_t minTimestamp, uint64_t threshold,
                                                   std::vector<Protocol::KernelBaseInfo> &data,
                                                   const std::string filePath)
{
    auto resultSet = stmt->ExecuteQuery(minTimestamp);
    if (resultSet == nullptr) {
        ServerLog::Error("Failed to get result set for Operator Dispatch data.", stmt->GetErrorMessage());
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
        one.rankId = filePath;
        data.emplace_back(one);
    }
    if (data.size() < threshold) {
        ServerLog::Error(
            "Failed to get Operator Dispatch data because the total count should greater than or equal to "
            + std::to_string(threshold) + " ."
        );
        return false;
    }
    return true;
}

std::unique_ptr<SqliteResultSet> TraceDatabaseHelper::QueryHostUnitCounter(
    std::unique_ptr<SqlitePreparedStatement> &stmt,
    const Protocol::UnitCounterParams &requestParams, uint64_t minTimestamp)
{
    CounterEventHelper helper;
    helper.RegisterHostMap();
    auto processType = GetProcessType(requestParams.metaType);
    std::string sql = helper.GenerateHostCounterSQL(processType);
    return ExecuteQuery(stmt, sql, minTimestamp,
        requestParams.threadId, requestParams.startTime, requestParams.endTime);
}

std::unique_ptr<SqliteResultSet> TraceDatabaseHelper::QueryDeviceUnitCounter(
    std::unique_ptr<SqlitePreparedStatement> &stmt, const Protocol::UnitCounterParams &requestParams,
    uint64_t minTimestamp, const std::string &rankId)
{
    CounterEventHelper helper;
    helper.RegisterDeviceMap();
    auto processType = GetProcessType(requestParams.metaType);
    std::string sql = helper.GenerateDeviceCounterSQL(processType, requestParams.threadId);
    return ExecuteQuery(stmt, sql, minTimestamp,
        requestParams.threadId, requestParams.startTime, requestParams.endTime, rankId);
}

std::unique_ptr <SqliteResultSet> TraceDatabaseHelper::QueryThreadSameOperatorsDetails(
    std::unique_ptr <SqlitePreparedStatement> &stmt, const Protocol::UnitThreadsOperatorsParams &requestParams,
    const QUERY_THREAD_SAME_OPERATORS_PARAMS& params)
{
    const auto rankId = params.rankId;
    const auto minTimestamp = params.minTimestamp;
    const auto orderBy = params.orderBy;
    // 已经在DbTraceDataBase::QueryThreadSameOperatorsDetails中检查过tid sql注入风险
    const std::string tidListStr = StringUtil::Join4SqlGroup(params.tidList);
    // pid 从内部数据中获取，无sql注入风险
    const std::string pidListStr = StringUtil::Join4SqlGroup(params.pidList);
    std::string sql;
    uint64_t offset = (requestParams.current - 1) > UINT64_MAX / requestParams.pageSize ? 0 :
        (requestParams.current - 1) * requestParams.pageSize;
    std::string withHeadSql = "with params as (SELECT ? as rankId, ? as minTime, ? as startTime, ? as endTime), "
        "  nameIds as (select id from STRING_IDS where value = ?) ";
    std::vector<PROCESS_TYPE> types = { PROCESS_TYPE::ASCEND_HARDWARE, PROCESS_TYPE::HCCL, PROCESS_TYPE::CANN_API,
        PROCESS_TYPE::MS_TX, PROCESS_TYPE::API, PROCESS_TYPE::OVERLAP_ANALYSIS
    };
    const bool uniqueDevice = IsDeviceIdUnique(requestParams.rankId);
    const int overlapType = OverlapAnsRepo::GetTypeByName(requestParams.name);
    for (const auto type : types) {
        withHeadSql += ", " +
            GetQueryThreadSameOperatorsDetailsHeadSql(tidListStr, pidListStr, uniqueDevice, overlapType, type);
    }
    const auto sameOperatorsDetailsSql = withHeadSql +
        " , all_same_operator_detail as (SELECT * from ascend UNION ALL " // PROCESS_TYPE::ASCEND_HARDWARE
        "   SELECT * from hccl UNION ALL " // PROCESS_TYPE::HCCL
        "   SELECT * from cann UNION ALL " // PROCESS_TYPE::CANN_API
        "   SELECT * from mstx UNION ALL " // PROCESS_TYPE::MS_TX
        "   SELECT * from python UNION ALL " // PROCESS_TYPE::API
        "   SELECT * from overlap)\n" // PROCESS_TYPE::OVERLAP_ANALYSIS
        " SELECT * from all_same_operator_detail ";
    Prepare(stmt, sameOperatorsDetailsSql + orderBy)->BindParams(rankId, minTimestamp,
        requestParams.startTime, requestParams.endTime, requestParams.name);
    return Execute(stmt, requestParams.pageSize, offset);
}

std::string TraceDatabaseHelper::GetQueryThreadSameOperatorsDetailsHeadSql(const std::string &tidListStr,
    const std::string &pidListStr, const bool uniqueDevice, const int overlapType, const PROCESS_TYPE type)
{
    switch (type) {
        case PROCESS_TYPE::ASCEND_HARDWARE:
            return GetAscendSameNameDetailSql(tidListStr);
        case PROCESS_TYPE::HCCL:
            return GetHcclSameNameDetailSql(tidListStr, uniqueDevice);
        case PROCESS_TYPE::CANN_API:
            return GetCannSameNameDetailSql(pidListStr, tidListStr);
        case PROCESS_TYPE::MS_TX:
            return GetMstxSameNameDetailSql(pidListStr);
        case PROCESS_TYPE::API:
            return GetPythonSameNameDetailSql(pidListStr);
        case PROCESS_TYPE::OVERLAP_ANALYSIS:
            return GetOverlapAnalysisSameNameDetailSql(overlapType);
        default:
            return "";
    }
}

std::vector<uint64_t> TraceDatabaseHelper::GetDeviceIdList(const std::string &fileId)
{
    return npuInfoRepo->QueryDeviceIdByFileId(fileId);
}

bool TraceDatabaseHelper::IsDeviceIdUnique(const std::string &fileId)
{
    std::vector<uint64_t> deviceIdList = GetDeviceIdList(fileId);
    return deviceIdList.size() == 1;
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
            if (!IsDeviceIdUnique(requestParams.cardId)) {
                sql = "SELECT startNs - ? as start_time, endNs - startNs as duration, endNs - ? as end_time "
                      "FROM " + TABLE_TASK + " main join " + TABLE_COMMUNICATION_TASK_INFO + " info "
                      " on main.globalTaskId = info.globalTaskId"
                      " WHERE deviceId = ? AND start_time >= ? AND start_time <= ? ORDER BY startNs;";
                return ExecuteQuery(stmt, sql, minTimestamp, minTimestamp, rankId,
                                    requestParams.startTime, requestParams.endTime);
            } else {
                sql = "SELECT startNs - ? as start_time, endNs - startNs as duration, endNs - ? as end_time "
                      "FROM " + TABLE_COMMUNICATION_OP + " where start_time >= ? AND start_time <= ? ORDER BY startNs;";
                return ExecuteQuery(stmt, sql, minTimestamp, minTimestamp, requestParams.startTime,
                                    requestParams.endTime);
            }
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
    // pid匹配的入参为globalTid的高32位 // pid, tid 实际用于展示，processId, threadId 才是跳转用的 pid, tid
    std::string sql = "SELECT pa.ROWID as id, value AS name, startNs AS start, (endNs - startNs) AS duration, "
        "(globalTid & 0xFFFFFFFF) AS tid, (globalTid / 4294967296) AS pid, depth, globalTid as processId, "
        "'pytorch' as threadId FROM PYTORCH_API AS pa LEFT JOIN STRING_IDS AS si ON pa.name = si.id "
        "WHERE pid||'' = ? UNION "
        "SELECT ca.ROWID as id, value AS name, startNs AS start, (endNs - startNs) AS duration, "
        "(globalTid & 0xFFFFFFFF) AS tid, (globalTid / 4294967296) AS pid, depth, "
        "globalTid as processId, type as threadId "
        "FROM CANN_API AS ca LEFT JOIN STRING_IDS AS si ON ca.name = si.id "
        "WHERE pid||'' = ? UNION "
        "SELECT me.ROWID as id, value AS name, startNs AS start, (endNs - startNs) AS duration, "
        "(globalTid & 0xFFFFFFFF) AS tid, (globalTid / 4294967296) AS pid, depth, "
        "globalTid as processId, 'MsTx' as threadId "
        "FROM MSTX_EVENTS AS me LEFT JOIN STRING_IDS AS si ON me.message = si.id "
        "WHERE pid||'' = ? ";
    return TraceDatabaseHelper::ExecuteQuery(stmt, sql.append(orderByCondition), params.pid, params.pid, params.pid);
}

std::unique_ptr <SqliteResultSet> QueryEventsView4Thread(std::unique_ptr <SqlitePreparedStatement> &stmt,
    std::string &orderByCondition, const Protocol::EventsViewParams &params)
{   // pid, tid 实际用于展示，processId, threadId 才是跳转用的 pid, tid
    std::string sql = "SELECT pa.ROWID as id, value AS name, startNs AS start, (endNs - startNs) AS duration, "
        "(globalTid & 0xFFFFFFFF) AS tid, (globalTid / 4294967296) AS pid, depth, globalTid as processId, "
        "'pytorch' as threadId FROM PYTORCH_API AS pa LEFT JOIN STRING_IDS AS si ON pa.name = si.id "
        "WHERE globalTid = ? UNION "
        "SELECT ca.ROWID as id, value AS name, startNs AS start, (endNs - startNs) AS duration, "
        "(globalTid & 0xFFFFFFFF) AS tid, (globalTid / 4294967296) AS pid, "
        "depth, globalTid as processId, type as threadId "
        "FROM CANN_API AS ca LEFT JOIN STRING_IDS AS si ON ca.name = si.id "
        "WHERE globalTid = ? UNION "
        "SELECT me.ROWID as id, value AS name, startNs AS start, (endNs - startNs) AS duration, "
        "(globalTid & 0xFFFFFFFF) AS tid, (globalTid / 4294967296) AS pid, "
        "depth, globalTid as processId, 'MsTx' as threadId "
        "FROM MSTX_EVENTS AS me LEFT JOIN STRING_IDS AS si ON me.message = si.id "
        "WHERE globalTid = ? ";
    return TraceDatabaseHelper::ExecuteQuery(stmt, sql.append(orderByCondition), params.pid, params.pid, params.pid);
}

std::unique_ptr <SqliteResultSet> QueryEventsView4MSTX(std::unique_ptr <SqlitePreparedStatement> &stmt,
    std::string &orderByCondition, const Protocol::EventsViewParams &params)
{   // pid, tid 实际用于展示，processId, threadId 才是跳转用的 pid, tid
    std::string sql = "SELECT me.ROWID as id, value AS name, startNs AS start, (endNs - startNs) AS duration, "
        "(globalTid & 0xFFFFFFFF) AS tid, depth, globalTid as processId, 'MsTx' as threadId, "
        "(globalTid / 4294967296) AS pid FROM MSTX_EVENTS AS me LEFT JOIN STRING_IDS AS si ON me.message = si.id "
        "WHERE globalTid = ? ";
    return TraceDatabaseHelper::ExecuteQuery(stmt, sql.append(orderByCondition), params.pid);
}

std::unique_ptr <SqliteResultSet> QueryEventsView4Pytorch(std::unique_ptr <SqlitePreparedStatement> &stmt,
    std::string &orderByCondition, const Protocol::EventsViewParams &params)
{   // pid, tid 实际用于展示，processId, threadId 才是跳转用的 pid, tid
    std::string sql = "SELECT pa.ROWID as id, value AS name, startNs AS start, (endNs - startNs) AS duration, "
        "(globalTid & 0xFFFFFFFF) AS tid, depth, globalTid as processId, 'pytorch' as threadId, "
        "(globalTid / 4294967296) AS pid FROM PYTORCH_API AS pa LEFT JOIN STRING_IDS AS si ON pa.name = si.id "
        "WHERE globalTid = ? ";
    return TraceDatabaseHelper::ExecuteQuery(stmt, sql.append(orderByCondition), params.pid);
}

std::unique_ptr <SqliteResultSet> QueryEventsView4HostHccl(std::unique_ptr <SqlitePreparedStatement> &stmt,
    std::string &orderByCondition, const Protocol::EventsViewParams &params)
{   // pid, tid 实际用于展示，processId, threadId 才是跳转用的 pid, tid
    std::string sql = "SELECT ca.ROWID as id, value AS name, startNs AS start, (endNs - startNs) AS duration, "
        "(globalTid & 0xFFFFFFFF) AS tid, depth, globalTid as processId, type as threadId, "
        "(globalTid / 4294967296) AS pid FROM CANN_API AS ca LEFT JOIN STRING_IDS AS si ON ca.name = si.id "
        "WHERE globalTid = ? AND ca.type = 5500 ";
    return TraceDatabaseHelper::ExecuteQuery(stmt, sql.append(orderByCondition), params.pid);
}

std::unique_ptr <SqliteResultSet> QueryEventsView4CANN(std::unique_ptr <SqlitePreparedStatement> &stmt,
    std::string &orderByCondition, const Protocol::EventsViewParams &params)
{   // pid, tid 实际用于展示，processId, threadId 才是跳转用的 pid, tid
    std::string sql = "SELECT ca.ROWID as id, value AS name, startNs AS start, (endNs - startNs) AS duration, "
        "(globalTid & 0xFFFFFFFF) AS tid, depth, globalTid as processId, type as threadId, "
        "(globalTid / 4294967296) AS pid FROM CANN_API AS ca LEFT JOIN STRING_IDS AS si ON ca.name = si.id "
        "WHERE globalTid = ? AND ca.type IN (5000, 10000, 15000, 20000) ";
    return TraceDatabaseHelper::ExecuteQuery(stmt, sql.append(orderByCondition), params.pid);
}

std::unique_ptr <SqliteResultSet> QueryEventsView4SubCANN(std::unique_ptr <SqlitePreparedStatement> &stmt,
    std::string &orderByCondition, const Protocol::EventsViewParams &params)
{   // pid, tid 实际用于展示，processId, threadId 才是跳转用的 pid, tid
    std::string sql = "SELECT ca.ROWID as id, value AS name, startNs AS start, (endNs - startNs) AS duration, "
        "(globalTid & 0xFFFFFFFF) AS tid, depth, globalTid as processId, type as threadId, "
        "(globalTid / 4294967296) AS pid FROM CANN_API AS ca LEFT JOIN STRING_IDS AS si ON ca.name = si.id "
        "WHERE globalTid = ? AND ca.type = ? ";
    return TraceDatabaseHelper::ExecuteQuery(stmt, sql.append(orderByCondition), params.pid, params.tid);
}

std::unique_ptr <SqliteResultSet> QueryEventsView4Hardware(std::unique_ptr <SqlitePreparedStatement> &stmt,
    std::string &orderByCondition, const Protocol::EventsViewParams &params, const std::string& rankId)
{
    std::string sql =
        "SELECT main.ROWID as id, si.value AS name, main.startNs AS start, main.endNs - main.startNs as duration, "
        "'Stream '||streamId as threadName, main.depth, 'Ascend Hardware' as processId, streamId as threadId, "
        "deviceId AS rankId FROM  TASK AS main LEFT JOIN COMPUTE_TASK_INFO AS CTI "
        "on CTI.globalTaskId = main.globalTaskId "
        " LEFT JOIN COMMUNICATION_SCHEDULE_TASK_INFO schedule ON main.globalTaskId = schedule.globalTaskId "
        " LEFT JOIN MSTX_EVENTS mstx ON main.connectionId = mstx.connectionId "
        " LEFT JOIN STRING_IDS AS si ON si.id = coalesce(CTI.name, schedule.name, mstx.message, main.taskType)"
        " WHERE main.deviceId = ? ";
    return TraceDatabaseHelper::ExecuteQuery(stmt, sql.append(orderByCondition), rankId);
}

std::unique_ptr <SqliteResultSet> QueryEventsView4Stream(std::unique_ptr <SqlitePreparedStatement> &stmt,
    std::string &orderByCondition, const Protocol::EventsViewParams &params, const std::string& rankId)
{
    std::string sql =
        "SELECT main.ROWID as id, si.value AS name, main.startNs AS start, main.endNs - main.startNs as duration, "
        "'Stream '||streamId as threadName, deviceId AS rankId, main.depth, 'Ascend Hardware' as processId, "
        "streamId as threadId FROM TASK AS main "
        " LEFT JOIN COMPUTE_TASK_INFO AS CTI on CTI.globalTaskId = main.globalTaskId "
        " LEFT JOIN COMMUNICATION_SCHEDULE_TASK_INFO schedule ON main.globalTaskId = schedule.globalTaskId "
        " LEFT JOIN MSTX_EVENTS mstx ON main.connectionId = mstx.connectionId "
        " LEFT JOIN STRING_IDS AS si ON si.id = coalesce(CTI.name, schedule.name, mstx.message, main.taskType) "
        "WHERE main.deviceId = ? ";
    std::vector<std::string> temp = std::vector<std::string>(params.threadIdList.size(), "?");
    std::string tempSql = StringUtil::join(temp, ",");
    sql.append("AND main.streamId IN ( ");
    sql.append(tempSql);
    sql.append(" ) ");
    sql.append(orderByCondition);
    stmt->Prepare(sql);
    if (stmt == nullptr) {
        throw DatabaseException("Failed to prepare sql.");
    }
    stmt->BindParams(rankId);
    for (const auto &item: params.threadIdList) {
        stmt->BindParams(item);
    }
    auto resultSet = stmt->ExecuteQuery();
    if (resultSet == nullptr) {
        throw DatabaseException("Failed to ExecuteQuery.");
    }
    return resultSet;
}

std::unique_ptr <SqliteResultSet> QueryEventsView4DeviceHCCL(std::unique_ptr <SqlitePreparedStatement> &stmt,
    std::string &orderByCondition, const Protocol::EventsViewParams &params, const std::string& rankId)
{
    if (!TraceDatabaseHelper::IsDeviceIdUnique(params.rankId)) {
        std::string sql = "with tmp as (select * from TASK main join COMMUNICATION_TASK_INFO "
                          "info on info.globalTaskId = main.globalTaskId where main.deviceId = ?), "
                          "sub as (select COMMUNICATION_OP.ROWID, startNs, endNs-startNs as duration, si.value as name,"
                          "groupName from COMMUNICATION_OP LEFT JOIN STRING_IDS AS si ON si.id = opName "
                          "where opId in (select opId from tmp group by opId)) "
                          "select ROWID as id, name, startNs as start, duration, 0 as depth, 'HCCL' as processId, "
                          "groupName||'group' as threadId, "
                          "'Group '||((DENSE_RANK() OVER (ORDER BY groupName)) - 1)||' Communication' AS threadName, "
                          "? AS rankId from sub ";
        return TraceDatabaseHelper::ExecuteQuery(stmt, sql.append(orderByCondition), rankId, rankId);
    } else {
        std::string sql = "with sub as (select COMMUNICATION_OP.ROWID, startNs, endNs-startNs as duration, "
                          "si.value as name, groupName from COMMUNICATION_OP "
                          "LEFT JOIN STRING_IDS AS si ON si.id = opName) "
                          "select ROWID as id, name, startNs as start, duration, 0 as depth, 'HCCL' as processId, "
                          "groupName||'group' as threadId, "
                          "'Group '||((DENSE_RANK() OVER (ORDER BY groupName)) - 1)||' Communication' AS threadName, "
                          "? AS rankId from sub ";
        return TraceDatabaseHelper::ExecuteQuery(stmt, sql.append(orderByCondition), rankId);
    }
}

std::unique_ptr <SqliteResultSet> QueryEventsView4Group(std::unique_ptr <SqlitePreparedStatement> &stmt,
    std::string &orderByCondition, const Protocol::EventsViewParams &params, const std::string& rankId)
{
    if (!TraceDatabaseHelper::IsDeviceIdUnique(params.rankId)) {
        std::string sql = "with tmp as (select * from TASK main join COMMUNICATION_TASK_INFO "
                          "info on info.globalTaskId = main.globalTaskId where main.deviceId = ?), "
                          "sub as (select COMMUNICATION_OP.ROWID, startNs, endNs-startNs as duration, si.value as name,"
                          "groupName from COMMUNICATION_OP LEFT JOIN STRING_IDS AS si ON si.id = opName "
                          "where opId in (select opId from tmp group by opId)) "
                          "select ROWID as id, name, startNs as start, duration, 0 as depth, 'HCCL' as processId, "
                          "groupName||'group' as threadId, ? AS threadName, ? AS rankId from sub "
                          "WHERE groupName||'group' = ? ";
        return TraceDatabaseHelper::ExecuteQuery(stmt, sql.append(orderByCondition),
                                                 rankId, params.threadName, rankId, params.tid);
    } else {
        std::string sql = "with sub as (select COMMUNICATION_OP.ROWID, startNs, endNs-startNs as duration, "
                          "si.value as name, groupName "
                          "from COMMUNICATION_OP LEFT JOIN STRING_IDS AS si ON si.id = opName) "
                          "select ROWID as id, name, startNs as start, duration, 0 as depth, 'HCCL' as processId, "
                          "groupName||'group' as threadId, ? AS threadName, ? AS rankId from sub "
                          "WHERE groupName||'group' = ? ";
        return TraceDatabaseHelper::ExecuteQuery(stmt, sql.append(orderByCondition), params.threadName,
                                                 rankId, params.tid);
    }
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
        if (params.threadName.empty()) {
            return QueryEventsView4Thread(stmt, orderByCondition, params);
        } else if (params.threadName == "hccl") { // 存疑，"Thread*" 下面是否有 "hccl" 的情况
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

void GetEventsViewResultSet4DbDetails(std::unique_ptr<SqliteResultSet>& resultSet, PROCESS_TYPE metaType,
                                      uint64_t minTimestamp, std::vector<std::unique_ptr<EventDetail>>& details)
{
    while (resultSet->Next()) {
        auto ptr = std::make_unique<EventDetail>();
        // 根据泳道类型，创建对应子类对象的指针，填充特有数据
        if (metaType == PROCESS_TYPE::API || metaType == PROCESS_TYPE::CANN_API || metaType == PROCESS_TYPE::MS_TX) {
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
        uint64_t tempStartTime = resultSet->GetUint64("start");
        if (tempStartTime < minTimestamp) {
            continue;
        }
        ptr->startTime = tempStartTime - minTimestamp;
        ptr->duration = resultSet->GetUint64("duration");
        ptr->depth = resultSet->GetUint64("depth");
        ptr->threadId = resultSet->GetString("threadId");
        ptr->processId = resultSet->GetString("processId");
        details.emplace_back(std::move(ptr));
    }
}

void ResolveEventsViewResultSet4Db(std::unique_ptr <SqliteResultSet> &resultSet,
    const Protocol::EventsViewParams &params, Protocol::EventsViewBody &body, uint64_t minTimestamp)
{
    auto metaType = TraceDatabaseHelper::GetProcessType(params.metaType);
    std::vector<std::unique_ptr<EventDetail>> details;
    GetEventsViewResultSet4DbDetails(resultSet, metaType, minTimestamp, details);
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

std::unique_ptr<SqliteResultSet> QueryEventsViewResultSet(std::unique_ptr <SqlitePreparedStatement> &stmt,
    const Protocol::EventsViewParams &params, std::string &orderByCondition, const std::string& rankId,
    const PROCESS_TYPE &metaType)
{
    switch (metaType) { // 根据不同的泳道类型，调用不同的query查询数据表
        case Protocol::PROCESS_TYPE::MS_TX:
            return QueryEventsView4MSTX(stmt, orderByCondition, params);
        case Protocol::PROCESS_TYPE::API:
            return QueryEventsView4Pytorch(stmt, orderByCondition, params);
        case Protocol::PROCESS_TYPE::CANN_API:
            return GetEventsViewResult4CANNAPI(stmt, orderByCondition, params);
        case Protocol::PROCESS_TYPE::ASCEND_HARDWARE:
            if (params.threadIdList.empty() && params.threadName.empty()) {
                return QueryEventsView4Hardware(stmt, orderByCondition, params, rankId);
            } else {
                return QueryEventsView4Stream(stmt, orderByCondition, params, rankId);
            }
        case Protocol::PROCESS_TYPE::HCCL:
            if (params.tid.empty() && params.threadName.empty()) {
                return QueryEventsView4DeviceHCCL(stmt, orderByCondition, params, rankId);
            } else {
                return QueryEventsView4Group(stmt, orderByCondition, params, rankId);
            }
        case Protocol::PROCESS_TYPE::OVERLAP_ANALYSIS:
            if (params.tid.empty() && params.threadName.empty()) {
                return QueryEventsView4Overlap(stmt, orderByCondition, params);
            } else {
                return QueryEventsView4OverlapSub(stmt, orderByCondition, params);
            }
        default:
            ServerLog::Warn("No defined query way");
            return nullptr;
    }
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
        resultSet = QueryEventsViewResultSet(stmt, params, orderByCondition, rankId, metaType);
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
        uint64_t tempStartTime = resultSet->GetUint64("start");
        if (tempStartTime < minTimestamp) {
            continue;
        }
        ptr->startTime = tempStartTime - minTimestamp;
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
    if (!params.threadIdList.empty()) {
        std::vector<std::string> temp = std::vector<std::string>(params.threadIdList.size(), "?");
        std::string tempSql = StringUtil::join(temp, ",");
        sql4Details.append("AND t.tid IN ( ");
        sql4Details.append(tempSql);
        sql4Details.append(" ) ");
    }

    // 拼接分页相关的条件
    std::string order = params.order == "descend" ? "DESC" : "ASC";
    sql4Details.append("ORDER BY " + orderBy + " " + order);

    // 查询数据库
    std::unique_ptr<SqliteResultSet> resultSet;
    try {
        if (params.threadIdList.empty()) {
            resultSet = ExecuteQuery(stmt, sql4Details, params.pid);
        } else {
            stmt->Prepare(sql4Details);
            if (stmt == nullptr) {
                throw DatabaseException("Failed to prepare sql.");
            }
            stmt->BindParams(params.pid);
            for (const auto &item: params.threadIdList) {
                stmt->BindParams(item);
            }
            resultSet = stmt->ExecuteQuery();
            if (resultSet == nullptr) {
                throw DatabaseException("Failed to ExecuteQuery.");
            }
        }
    } catch (DatabaseException &e) {
        ServerLog::Error("Query events view data for text. Execute query failed: ", e.What());
        return false;
    }
    ResolveEventsViewResultSet(resultSet, params, body, minTimestamp);
    return true;
}

void TraceDatabaseHelper::ComputeSummarySlice(std::unique_ptr<SqliteResultSet> &resultSet,
    uint64_t unitTime, Protocol::UnitThreadTracesSummaryBody &responseBody)
{
    uint64_t tempStartTime = 0;
    uint64_t tempEndTime = 0;
    while (resultSet->Next()) {
        uint64_t curEndTime = resultSet->GetUint64("end_time");
        uint64_t curStartTime = resultSet->GetUint64("start_time");
        if (curEndTime < curStartTime) {
            continue;
        }
        if (tempEndTime + unitTime >= curStartTime) {
            tempEndTime = tempEndTime > curEndTime ? tempEndTime : curEndTime;
            continue;
        }
        ThreadTracesSummary summary;
        summary.startTime = tempStartTime;
        summary.duration = tempEndTime >= tempStartTime ? tempEndTime - tempStartTime : 0;
        tempStartTime = curStartTime;
        tempEndTime = curEndTime;
        responseBody.data.emplace_back(summary);
    }
    ThreadTracesSummary summary;
    summary.startTime = tempStartTime;
    summary.duration = tempEndTime > tempStartTime ? tempEndTime - tempStartTime : 0;
    responseBody.data.emplace_back(summary);
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
        summary.startTime = tempStartTime >= minTimestamp ? tempStartTime - minTimestamp : 0;
        summary.duration = tempEndTime >= tempStartTime ? tempEndTime - tempStartTime : 0;
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

void TraceDatabaseHelper::SetSystemViewHelpler(std::unique_ptr<SqliteResultSet> resultSet, const LayerStatData &data,
    const Protocol::SystemViewParams &requestParams, Protocol::SystemViewBody &responseBody)
{
    while (resultSet->Next()) {
        Protocol::SystemViewDetail systemViewDetail;
        int col = 0;
        systemViewDetail.name = resultSet->GetString(col++);
        systemViewDetail.time = resultSet->GetDouble(col++);
        systemViewDetail.totalTime = resultSet->GetDouble(col++);
        systemViewDetail.numberCalls = resultSet->GetUint64(col++);
        systemViewDetail.avg = resultSet->GetDouble(col++);
        systemViewDetail.min = resultSet->GetDouble(col++);
        systemViewDetail.max = resultSet->GetDouble(col++);
        responseBody.systemViewDetail.emplace_back(systemViewDetail);
    }
    responseBody.total = data.total;
    responseBody.pageSize = requestParams.pageSize;
    responseBody.currentPage = requestParams.current;
}

void TraceDatabaseHelper::SetKernelDetailHelpler(std::unique_ptr<SqliteResultSet> resultSet, uint64_t minTimestamp,
                                                 Protocol::KernelDetailsBody &responseBody)
{
    while (resultSet->Next()) {
        Protocol::KernelDetail detail;
        detail.id = resultSet->GetString("id");
        detail.taskId = resultSet->GetString("taskId"),
        detail.name = resultSet->GetString("name");
        detail.type = resultSet->GetString("type");
        detail.acceleratorCore = resultSet->GetString("acceleratorCore");
        uint64_t tempStartTime = resultSet->GetUint64("startTime");
        if (tempStartTime < minTimestamp) {
            continue;
        }
        detail.startTime = tempStartTime - minTimestamp;
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

std::string TraceDatabaseHelper::GeneratorCommunicationSummarySql4Text(
    const OrderParam &orderParam, const PageParam &pageParam)
{
    std::string sql =
        "WITH data as ("
        "    SELECT s.name, s.timestamp as start_time, s.duration, s.end_time as end_time, t.pid, t.tid, s.track_id, "
        "    t.thread_name, CASE WHEN t.thread_name like 'Group%' THEN 1 ELSE 0 END as type, "
        "    row_number() OVER (ORDER by s.track_id ASC, s.timestamp ASC) as row_num FROM " + SLICE_TABLE + " s "
        "    JOIN " + THREAD_TABLE + " t ON s.track_id = t.track_id WHERE s.track_id in ( "
        "        SELECT track_id FROM " + THREAD_TABLE + " WHERE pid in ( "
        "            SELECT pid FROM " + PROCESS_TABLE +
        " WHERE (pid & 0x1f) = ? AND process_name in ('HCCL', 'COMMUNICATION', 'Communication') "
        "        ) "
        "    ) "
        ") "
        "SELECT d1.name as name, d1.start_time as startTime, d1.duration as duration, d1.end_time as endTime, "
        "d1.pid as groupName, d1.tid as plane, d1.thread_name as threadName, d1.type as type, "
        "CASE "
        "    WHEN d1.name = 'Notify_Wait' THEN "
        "        CASE "
        "            WHEN d2.name = 'RDMASend' AND d3.name = 'RDMASend' OR "
        "                (d3.name = 'Notify_Wait' AND d4.name = 'RDMASend' AND d5.name = 'RDMASend') THEN '0' "
        "            ELSE '1' "
        "        END "
        "    ELSE '0' "
        "END as flag "
        "FROM data d1 "
        "LEFT JOIN data d2 ON d2.row_num = d1.row_num - 1 AND d2.track_id = d1.track_id "
        "LEFT JOIN data d3 ON d3.row_num = d1.row_num - 2 AND d3.track_id = d1.track_id "
        "LEFT JOIN data d4 ON d4.row_num = d1.row_num - 3 AND d4.track_id = d1.track_id "
        "LEFT JOIN data d5 ON d5.row_num = d1.row_num - 4 AND d5.track_id = d1.track_id "
        "ORDER BY d1.pid, d1.tid, d1.start_time";
    return sql;
}

std::string TraceDatabaseHelper::GeneratorCommunicationSummarySql4Db(const OrderParam &orderParam,
    const PageParam &pageParam, const std::string &sqlForVersion)
{
    std::string sql = sqlForVersion +
        "SELECT d1.name as name, d1.start_time as startTime, d1.duration as duration, d1.end_time as endTime, "
        "d1.groupName as groupName, d1.planeId as plane, d1.thread_name as threadName, d1.type as type, "
        "CASE "
        "    WHEN d1.name = 'Notify_Wait' THEN "
        "        CASE "
        "           WHEN d2.name = 'RDMASend' AND d3.name = 'RDMASend' OR "
        "               (d3.name = 'Notify_Wait' AND d4.name = 'RDMASend' AND d5.name = 'RDMASend') THEN '0' "
        "           ELSE '1' "
        "        END "
        "    ELSE '0' "
        "END as flag "
        "FROM data d1 "
        "LEFT JOIN data d2 ON d2.groupName = d1.groupName AND d2.planeId = d1.planeId AND d2.row_num = d1.row_num - 1 "
        "LEFT JOIN data d3 ON d3.groupName = d1.groupName AND d3.planeId = d1.planeId AND d3.row_num = d1.row_num - 2 "
        "LEFT JOIN data d4 ON d4.groupName = d1.groupName AND d4.planeId = d1.planeId AND d4.row_num = d1.row_num - 3 "
        "LEFT JOIN data d5 ON d5.groupName = d1.groupName AND d5.planeId = d1.planeId AND d5.row_num = d1.row_num - 4 "
        "ORDER BY d1.groupName, d1.planeId, d1.start_time";
    return sql;
}

bool TraceDatabaseHelper::CalculateParallelParameter(const std::vector<Protocol::ThreadTraces> &fwdTraceList,
    const std::vector<Protocol::ThreadTraces> &bwdTraceList,
    uint64_t minBwdStartTime, std::pair<uint16_t, uint16_t> &parameter)
{
    uint16_t minBwdIndex = 1;
    uint16_t microBatchSize = 1;
    // 计算micro batch size
    for (size_t i = 1; i < bwdTraceList.size(); i++) {
        if (bwdTraceList.at(i).startTime > bwdTraceList.at(i - 1).startTime) {
            microBatchSize++;
        } else {
            break;
        }
    }
    for (size_t i = 0; i < bwdTraceList.size(); i++) {
        if (minBwdStartTime == bwdTraceList.at(i).startTime) {
            minBwdIndex = i; // (vppSize - 1) * microBatchSize
            break;
        }
    }
    if (microBatchSize == 0 || minBwdIndex % microBatchSize != 0) {
        return false;
    }

    uint32_t vppSize = (minBwdIndex / microBatchSize) + 1;
    if (vppSize > UINT16_MAX) {
        vppSize = 1;
    }
    parameter = {microBatchSize, static_cast<uint16_t>(vppSize)};
    return true;
}

// 使用sql命令查询前反向连线(fwdbwd)，并拼接同一条连线，按前向的start time升序排序，查询形成每一条连线FlowStartAndEndTime结构体
// 遍历所有的连线，对于同一个前反向内的数据，前向的start time越来越大，后向的start time越来越小
// 当不满足前面的条件时，表明开始了一个新的前反向，即tmp.fStartTime > first.fStartTime，此时根据第一个连线和上一个连线，计算前反向的时长
// 前向的起点应该是first中前向开始时间sStartTime，终点应该是last中前向的结束时间sEndTime
// 反向的起点应该是last中反向开始世家fStartTime，终点应该是first中反向的结束时间fEndTime
// 计算完成后tmp变成新的first，即开始新的前反向
bool TraceDatabaseHelper::ExecuteQueryFwdBwdDataByFlow(std::unique_ptr<SqlitePreparedStatement> stmt,
    const std::string &rankId, uint64_t offset, const Protocol::ExtremumTimestamp &range,
    std::vector<Protocol::ThreadTraces> &fwdBwdData)
{
    auto resultSet = stmt->ExecuteQuery(range.minTimestamp, range.maxTimestamp);
    if (resultSet == nullptr) {
        ServerLog::Error("Failed to get result set for query fwd/bwd data.", stmt->GetErrorMessage());
        return false;
    }
    std::vector<Protocol::ThreadTraces> fwdTraceList{};
    std::vector<Protocol::ThreadTraces> bwdTraceList{};
    uint64_t minBwdStartTime = UINT64_MAX;
    while (resultSet->Next()) {
        Protocol::ThreadTraces fwdTrace = {
            .name = std::to_string(0), .duration = resultSet->GetUint64("fpDuration"),
            .startTime = resultSet->GetUint64("fpStart") > offset ? resultSet->GetUint64("fpStart") - offset : 0,
            .endTime = resultSet->GetUint64("fpEnd") > offset ? resultSet->GetUint64("fpEnd") - offset : 0,
            .depth = 0, .threadId = LANE_FP_BP, .pid = rankId, .id = "", .cname = MARKER_FP
        };
        Protocol::ThreadTraces bwdTrace = {
            .name = std::to_string(0), .duration = resultSet->GetUint64("bpDuration"),
            .startTime = resultSet->GetUint64("bpStart") > offset ? resultSet->GetUint64("bpStart") - offset : 0,
            .endTime = resultSet->GetUint64("bpEnd") > offset ? resultSet->GetUint64("bpEnd") - offset : 0,
            .depth = 0, .threadId = LANE_FP_BP, .pid = rankId, .id = "", .cname = MARKER_BP
        };

        fwdTraceList.push_back(fwdTrace);
        bwdTraceList.push_back(bwdTrace);
        minBwdStartTime = std::min(minBwdStartTime, bwdTrace.startTime);
    }

    if (fwdTraceList.empty() || bwdTraceList.empty()) {
        ServerLog::Error("Failed to query fwd/bwd data due to empty result.");
        return false;
    }
    std::pair<uint16_t, uint16_t> parallelParameter = {1, 1};
    if (bwdTraceList.at(0).startTime != minBwdStartTime) {
        if (!CalculateParallelParameter(fwdTraceList, bwdTraceList, minBwdStartTime, parallelParameter)) {
            ServerLog::Error("Failed to calculate parallel parallel.");
            return false;
        }
    }
    uint32_t repetitionPeriod = parallelParameter.second * parallelParameter.first;
    for (size_t i = 0; i < bwdTraceList.size(); i++) {
        uint16_t gradientAccIndex = i / repetitionPeriod;
        uint16_t tmpMicroBatch = i % repetitionPeriod;
        uint16_t tmpMicroIndex = tmpMicroBatch % parallelParameter.first;
        uint16_t realBatchIndex = gradientAccIndex * parallelParameter.first + tmpMicroIndex;
        fwdTraceList.at(i).name = std::to_string(realBatchIndex);
        bwdTraceList.at(i).name = std::to_string(realBatchIndex);
        fwdBwdData.push_back(fwdTraceList.at(i));
        fwdBwdData.push_back(bwdTraceList.at(i));
    }

    return true;
}

bool TraceDatabaseHelper::ExecuteQueryP2POpData(std::unique_ptr<SqlitePreparedStatement> stmt,
    const std::string &rankId, uint64_t offset, const ExtremumTimestamp &range,
    std::vector<Protocol::ThreadTraces> &p2pOpData)
{
    if (stmt == nullptr || rankId.empty()) {
        Server::ServerLog::Error("Failed to query p2p operator data due to null statement or empty rand id.");
        return false;
    }
    auto resultSet = stmt->ExecuteQuery(offset, range.minTimestamp, range.maxTimestamp);
    if (resultSet == nullptr) {
        Server::ServerLog::Error("Failed to get result set for query fwd/bwd data.", stmt->GetErrorMessage());
        return false;
    }
    while (resultSet->Next()) {
        Protocol::ThreadTraces tmp{};
        tmp.pid = resultSet->GetString("pid");
        tmp.threadId = resultSet->GetString("tid");
        tmp.name = resultSet->GetString("name");
        tmp.startTime = resultSet->GetUint64("startTime");
        tmp.duration = resultSet->GetUint64("duration");
        if (StringUtil::StartWith(tmp.name, "hcom_send")) {
            tmp.cname = MARKER_SEND;
        } else if (StringUtil::StartWith(tmp.name, "hcom_receive")) {
            tmp.cname = MARKER_RECV;
        } else {
            tmp.cname = MARKER_BATCH_SEND_RECV;
        }
        p2pOpData.push_back(tmp);
    }
    return true;
}

void TraceDatabaseHelper::CalculateSelfTime(std::vector<Protocol::SimpleSlice> &rows,
    std::map<std::string, uint64_t> &selfTimeKeyValue, uint64_t startTime, uint64_t endTime)
{
    if (rows.empty()) {
        Server::ServerLog::Error("simpleSlice array size is zero!");
        return;
    }
    uint64_t tmpSelfTime = rows.at(0).duration;
    size_t i = 0;
    size_t j = 0;
    while (i < rows.size()) {
        // j滑完直接滑完所有i
        if (j == rows.size()) {
            // 处理当前tmpSelfTime
            AddData(selfTimeKeyValue, rows.at(i).name, tmpSelfTime);
            // 处理剩余元素
            DealLastData(rows, selfTimeKeyValue, startTime, endTime, i);
            break;
        }
        Protocol::SimpleSlice rowI = rows.at(i);
        Protocol::SimpleSlice rowJ = rows.at(j);
        // 层数相等 or 同一元素, j右移
        if (rowI.depth == rowJ.depth || i >= j) {
            j++;
            continue;
        }
        // rows[i]不属于框选范围内，跳过
        if (rows.at(i).timestamp > endTime || rows.at(i).endTime < startTime) {
            if (i + 1 == rows.size()) { // i滑完结束
                break;
            }
            i++;
            tmpSelfTime = rows.at(i).duration;
            continue;
        }
        // j元素超出i元素覆盖范围，或者j右移到下一层, 记录i元素selfTime并i右移(隐式|| rowJ.timestamp < rowI.timestamp)
        if (rowJ.endTime > rowI.endTime || rowI.depth + 1 < rowJ.depth) {
            AddData(selfTimeKeyValue, rowI.name, tmpSelfTime);
            if (i + 1 == rows.size()) { // i滑完结束
                break;
            }
            i++;
            tmpSelfTime = rows.at(i).duration;
            continue;
        }
        // 符合要求的元素
        if (rowJ.timestamp >= rowI.timestamp && rowJ.endTime <= rowI.endTime && tmpSelfTime >= rowJ.duration) {
            tmpSelfTime -= rowJ.duration;
        }
        j++;
    }
}

void TraceDatabaseHelper::ReduceThread(const std::vector<Protocol::SimpleSlice> &rows,
    const std::map<std::string, uint64_t> &selfTimeKeyValue, Protocol::UnitThreadsBody &responseBody)
{
    for (auto &cur : rows) {
        size_t index = 0;
        bool find = false;
        for (; index < responseBody.data.size(); index++) {
            if (responseBody.data[index].title == cur.name) {
                find = true;
                break;
            }
        }
        if (!find) {
            Protocol::SliceGroupItem sliceGroupItem {};
            sliceGroupItem.title = cur.name;
            sliceGroupItem.wallDuration = cur.duration;
            sliceGroupItem.occurrences = 1;
            sliceGroupItem.avgWallDuration = cur.duration;
            if (cur.name.empty()) {
                continue;
            } else {
                sliceGroupItem.selfTime = selfTimeKeyValue.at(cur.name);
            }
            sliceGroupItem.processMap[cur.pid] = { cur.tid };
            sliceGroupItem.metaType = cur.metaType;
            responseBody.data.emplace_back(sliceGroupItem);
        } else {
            responseBody.data[index].wallDuration += cur.duration;
            responseBody.data[index].occurrences += 1;
            responseBody.data[index].avgWallDuration = responseBody.data[index].occurrences == 0 ? 0 :
                responseBody.data[index].wallDuration / responseBody.data[index].occurrences;
            if (responseBody.data[index].processMap.count(cur.pid)) {
                responseBody.data[index].processMap[cur.pid].insert(cur.tid);
            } else {
                responseBody.data[index].processMap[cur.pid] = { cur.tid };
            }
        }
    }
}

void TraceDatabaseHelper::ReduceThread(const std::vector<CompeteSliceDomain> &rows,
    const std::map<std::string, uint64_t> &selfTimeKeyValue, Protocol::UnitThreadsBody &responseBody)
{
    for (auto &cur : rows) {
        size_t index = 0;
        bool find = false;
        for (; index < responseBody.data.size(); index++) {
            if (responseBody.data[index].title == cur.name) {
                find = true;
                break;
            }
        }
        if (!find) {
            Protocol::SliceGroupItem sliceGroupItem {};
            sliceGroupItem.title = cur.name;
            sliceGroupItem.wallDuration = cur.duration;
            sliceGroupItem.occurrences = 1;
            sliceGroupItem.avgWallDuration = cur.duration;
            sliceGroupItem.selfTime = selfTimeKeyValue.at(cur.name);
            sliceGroupItem.processMap[cur.pid] = { cur.tid };
            sliceGroupItem.metaType = cur.metaType;
            responseBody.data.emplace_back(sliceGroupItem);
        } else {
            responseBody.data[index].wallDuration += cur.duration;
            responseBody.data[index].occurrences += 1;
            responseBody.data[index].avgWallDuration = responseBody.data[index].occurrences == 0 ? 0 :
                responseBody.data[index].wallDuration / responseBody.data[index].occurrences;
            if (responseBody.data[index].processMap.count(cur.pid)) {
                responseBody.data[index].processMap[cur.pid].insert(cur.tid);
            } else {
                responseBody.data[index].processMap[cur.pid] = { cur.tid };
            }
        }
    }
}

uint64_t TraceDatabaseHelper::CalculateUncoveredTime(const std::vector<Protocol::ThreadTraces> &uncovered,
    size_t &index, const ThreadTraces &element)
{
    uint64_t totalUncoveredTime = 0;
    if (uncovered.empty() || index >= uncovered.size()) {
        return totalUncoveredTime;
    }
    // sql语句能够保证uncovered按start_time升序排列
    while (index < uncovered.size()) {
        Protocol::ThreadTraces uncoveredEle = uncovered.at(index);
        // 未掩盖部分的分片小于通信Op或Task时，二者无交集，需要跳到下一个未掩盖部分的分片
        if (element.startTime >= uncoveredEle.endTime) {
            index++;
            continue;
        }
        // 未掩盖部分的分片大于通信Op或Task时，二者也无交集，退出循环
        if (element.endTime <= uncoveredEle.startTime) {
            break;
        }
        // 二者有交集时，取其交集部分，就是通信Op或Task真实的未掩盖部分
        uint64_t startMax = uncoveredEle.startTime > element.startTime ? uncoveredEle.startTime : element.startTime;
        uint64_t endMin = uncoveredEle.endTime > element.endTime ? element.endTime : uncoveredEle.endTime;
        uint64_t uncoveredTime = endMin >= startMax ? endMin - startMax : 0;

        if (UINT64_MAX - totalUncoveredTime > uncoveredTime) {
            totalUncoveredTime += uncoveredTime;
        } else {
            // 实际数据很小，正常情况下不会溢出
            ServerLog::Error("Accumulation overflow occurred when calculating total uncovered time: ", uncoveredTime);
            totalUncoveredTime += 0;
        }
        if (element.endTime > uncoveredEle.endTime) {
            index++;
        } else {
            break;
        }
    }
    return totalUncoveredTime;
}

void TraceDatabaseHelper::SetNpuInfoRepo(std::unique_ptr<NpuInfoRepo> npuInfoRepoPtr)
{
    if (npuInfoRepoPtr != nullptr) {
        npuInfoRepo = std::move(npuInfoRepoPtr);
    }
}

std::string TraceDatabaseHelper::GetLockRangeSql(const SearchAllSliceParams &params,
    const std::vector<TrackQuery> &trackQueryVec)
{
    std::string sql;
    std::string nameMatch;
    std::string orderBy;
    if (params.order == "descend") {
        orderBy = " ORDER BY " + params.orderBy + " DESC";
    } else {
        orderBy = " ORDER BY " + params.orderBy + " ASC";
    }
    if (params.isMatchExact && params.isMatchCase) {
        nameMatch = "select id, value from STRING_IDS where value like ?";
    } else if (params.isMatchExact) {
        nameMatch = "select id, value from STRING_IDS where lower(value) like lower(?)";
    } else if (params.isMatchCase) {
        nameMatch = "select id, value from STRING_IDS where value like '%'||?||'%'";
    } else {
        nameMatch = "select id, value from STRING_IDS where lower(value) like lower('%'||?||'%')";
    }
    sql = "with ids as (" + nameMatch + ") ";
    std::vector<std::string> sqls;
    for (const auto &item : trackQueryVec) {
        std::string tempSql = GetSingleLockRangeSql(item);
        if (!tempSql.empty()) {
            sqls.emplace_back(tempSql);
        }
    }
    sql = sql + StringUtil::join(sqls, " UNION ALL ");
    std::string orderByFiled = " ORDER BY timestamp DESC  LIMIT ? OFFSET ?";
    sql += orderByFiled;
    return sql;
}

std::string TraceDatabaseHelper::GetSingleLockRangeSql(const TrackQuery &item)
{
    PROCESS_TYPE type = STR_TO_ENUM<PROCESS_TYPE>(item.metaType).value();
    std::string tempSql;
    if (type == PROCESS_TYPE::API) {
        tempSql = " SELECT api.ROWID as id, 'pytorch' as tid, api.globalTid as pid, api.startNs as timestamp, "
            "api.endNs as endTime, api.depth, '' as deviceId, ids.value as value from " + TABLE_API +
            "  api join ids on ids.id = api.name WHERE api.globalTid = ? AND api.startNs >= ? AND api.endNs <= ? ";
    } else if (type == PROCESS_TYPE::CANN_API) {
        tempSql = " SELECT cann.connectionId as id, cann.globalTid as pid, cann.type as tid, cann.startNs as "
            "timestamp, cann.endNs as endTime, cann.depth, '' as deviceId, ids.value from " +
            TABLE_CANN_API +
            "  cann join ids on ids.id = cann.name WHERE globalTid = ? AND type = ? AND startNs >= ? AND endNs <= "
            "? ";
    } else if (type == PROCESS_TYPE::MS_TX) {
        tempSql = " SELECT mstx.ROWID as id, mstx.globalTid as pid, 'MsTx' as tid, mstx.startNs as timestamp, "
            "mstx.endNs as endTime, mstx.depth, '' as deviceId, ids.value from " +
            TABLE_MSTX_EVENTS +
            "  mstx join ids on ids.id = mstx.message WHERE globalTid = ? AND startNs >= ? AND endNs <= ? ";
    } else if (type == PROCESS_TYPE::ASCEND_HARDWARE) {
        tempSql = "SELECT hadware.id as id, hadware.pid as pid, hadware.tid as tid, hadware.timestamp as "
            "timestamp, hadware.endTime as endTime, hadware.depth as depth, hadware.deviceId as deviceId, "
            "ids.value  FROM (SELECT coalesce(c.name, m.message, s.name, main.taskType) as "
            "name, main.ROWID AS id, 'Ascend Hardware' as pid, main.streamId as tid,main.startNs as timestamp, "
            "main.endNs as endTime, main.depth as depth, main.deviceId as deviceId FROM " +
            TABLE_TASK + " main left join " + TABLE_COMPUTE_TASK_INFO +
            " c on c.globalTaskId = main.globalTaskId left join " + TABLE_MSTX_EVENTS +
            " m on (m.connectionId = main.connectionId and  m.connectionId != " +
            WRONG_DATA + " ) left join " + TABLE_COMMUNICATION_SCHEDULE_TASK +
            " s on main.globalTaskId = s.globalTaskId WHERE main.deviceId = ? AND main.streamId = ? AND "
            "main.startNs >= ? AND main.endNs <= ?) hadware  join ids on ids.id = hadware.name ";
    } else if (type == PROCESS_TYPE::HCCL) {
        if (StringUtil::EndWith(item.threadId, "group")) {
            tempSql = " SELECT op.opId as id, 'HCCL' as pid, op.groupName||'group' as tid, op.startNs as "
                "timestamp, op.endNs as endTime, 0 as depth, '0' as deviceId, ids.value from " +
                TABLE_COMMUNICATION_OP +
                " op join ids on id = op.opName WHERE op.groupName = ? AND op.startNs >= ? AND op.endNs <= ? ";
        } else {
            tempSql = "SELECT main.ROWID as id, 'HCCL' as pid, ci.groupName||'_'||ci.planeId as tid, main.startNs "
                "as timestamp, main.endNs as endTime, main.depth, main.deviceId as deviceId, ids.value from "
                "TASK main join " + TABLE_COMMUNICATION_TASK_INFO +
                " ci on ci.globalTaskId = main.globalTaskId join ids on ids.id = ci.taskType" +
                " WHERE main.deviceId = ? and ci.groupName = ? AND ci.planeId = ? AND main.startNs >= ? AND "
                "main.endNs <= ?";
        }
    }
    return tempSql;
}

void TraceDatabaseHelper::SearchAllSliceWithLockRangeBindStmt(const SearchAllSliceParams &params,
    const std::vector<TrackQuery> &trackQueryVec, std::unique_ptr<SqlitePreparedStatement> &stmt,
    const std::string &deviceId)
{
    stmt->BindParams(params.searchContent);
    for (const auto &item : trackQueryVec) {
        BindSearchAllSliceSingleTrack(stmt, deviceId, item);
    }
}

void TraceDatabaseHelper::BindSearchAllSliceSingleTrack(std::unique_ptr<SqlitePreparedStatement> &stmt,
    const std::string &deviceId, const TrackQuery &item)
{
    PROCESS_TYPE type = STR_TO_ENUM<PROCESS_TYPE>(item.metaType).value();
    if (type == PROCESS_TYPE::API) {
        stmt->BindParams(item.processId, item.startTime, item.endTime);
    } else if (type == PROCESS_TYPE::CANN_API) {
        stmt->BindParams(item.processId, item.threadId, item.startTime, item.endTime);
    } else if (type == PROCESS_TYPE::MS_TX) {
        stmt->BindParams(item.processId, item.startTime, item.endTime);
    } else if (type == PROCESS_TYPE::ASCEND_HARDWARE) {
        stmt->BindParams(deviceId, item.threadId, item.startTime, item.endTime);
    } else if (type == PROCESS_TYPE::HCCL) {
        if (StringUtil::EndWith(item.threadId, "group")) {
            std::string tid = item.threadId.substr(0, item.threadId.size() - 5);
            stmt->BindParams(tid, item.startTime, item.endTime);
        } else {
            std::string groupName = item.threadId;
            std::string threadId = item.threadId;
            size_t pos = item.threadId.find_last_of("_");
            if (pos != std::string::npos && item.threadId.size() > pos) {
                threadId = item.threadId.substr(pos + 1);
                groupName = item.threadId.substr(0, pos);
            }
            stmt->BindParams(deviceId, groupName, threadId, item.startTime, item.endTime);
        }
    }
}

std::string TraceDatabaseHelper::GetSearchSliceNameWithLockRangeSql(const SearchSliceParams &params,
    const std::vector<TrackQuery> &trackQuery, const std::string &path)
{
    std::string sql;
    std::string nameMatch;
    if (params.isMatchExact && params.isMatchCase) {
        nameMatch = "select id from STRING_IDS where value like ?";
    } else if (params.isMatchExact) {
        nameMatch = "select id from STRING_IDS where lower(value) like lower(?)";
    } else if (params.isMatchCase) {
        nameMatch = "select id from STRING_IDS where value like '%'||?||'%'";
    } else {
        nameMatch = "select id from STRING_IDS where lower(value) like lower('%'||?||'%')";
    }
    sql = "with ids as (" + nameMatch + ") ";
    std::vector<std::string> sqls;
    for (const auto &item : trackQuery) {
        std::string tempSql = GetSingleSearchNameWithLockRangeSql(path, item);
        if (!tempSql.empty()) {
            sqls.emplace_back(tempSql);
        }
    }
    sql = sql + StringUtil::join(sqls, " UNION ALL ");
    std::string orderBy = " ORDER BY timestamp DESC  LIMIT 1 OFFSET ?";
    sql += orderBy;
    return sql;
}

std::string TraceDatabaseHelper::GetSingleSearchNameWithLockRangeSql(const std::string &path,
    const TrackQuery &singleQuery)
{
    PROCESS_TYPE type = STR_TO_ENUM<PROCESS_TYPE>(singleQuery.metaType).value();
    std::string tempSql;
    if (type == PROCESS_TYPE::API && singleQuery.rankId == path) {
        tempSql = " SELECT api.ROWID as id, 'pytorch' as tid, api.globalTid as pid, api.startNs as timestamp, "
            "api.endNs as endTime, api.depth from " +
            TABLE_API +
            "  api join ids on ids.id = api.name WHERE api.globalTid = ? AND api.startNs >= ? AND api.endNs <= ? ";
    } else if (type == PROCESS_TYPE::CANN_API && singleQuery.rankId == path) {
        tempSql = " SELECT cann.connectionId as id, cann.globalTid as pid, cann.type as tid, cann.startNs as "
            "timestamp, cann.endNs as endTime, cann.depth from " +
            TABLE_CANN_API +
            "  cann join ids on ids.id = cann.name WHERE globalTid = ? AND type = ? AND startNs >= ? AND endNs <= "
            "? ";
    } else if (type == PROCESS_TYPE::MS_TX && singleQuery.rankId == path) {
        tempSql = " SELECT mstx.ROWID as id, mstx.globalTid as pid, 'MsTx' as tid, mstx.startNs as timestamp, "
            "mstx.endNs as endTime, mstx.depth from " +
            TABLE_MSTX_EVENTS +
            "  mstx join ids on ids.id = mstx.message WHERE globalTid = ? AND startNs >= ? AND endNs <= ? ";
    } else if (type == PROCESS_TYPE::ASCEND_HARDWARE && singleQuery.rankId != path) {
        tempSql = "SELECT hadware.id as id, hadware.pid as pid, hadware.tid as tid, hadware.timestamp as "
            "timestamp, hadware.endTime as endTime, hadware.depth as depth  FROM (SELECT coalesce(c.name, "
            "m.message, s.name, main.taskType) as "
            "name, main.ROWID AS id, 'Ascend Hardware' as pid, main.streamId as tid,main.startNs as timestamp, "
            "main.endNs as endTime, main.depth as depth FROM " +
            TABLE_TASK + " main left join " + TABLE_COMPUTE_TASK_INFO +
            " c on c.globalTaskId = main.globalTaskId left join " + TABLE_MSTX_EVENTS + " m on "
            " (m.connectionId = main.connectionId and  m.connectionId != " +
            WRONG_DATA + " ) left join " + TABLE_COMMUNICATION_SCHEDULE_TASK +
            " s on main.globalTaskId = s.globalTaskId WHERE main.deviceId = ? AND main.streamId = ? AND "
            "main.startNs >= ? AND main.endNs <= ?) hadware  join ids on ids.id = hadware.name ";
    } else if (type == PROCESS_TYPE::HCCL && singleQuery.rankId != path) {
        if (StringUtil::EndWith(singleQuery.threadId, "group")) {
            tempSql = " SELECT op.opId as id, 'HCCL' as pid, op.groupName||'group' as tid, op.startNs as "
                "timestamp, op.endNs as endTime, 0 as depth from " +
                TABLE_COMMUNICATION_OP +
                " op join ids on id = op.opName WHERE op.groupName = ? AND op.startNs >= ? AND op.endNs <= ? ";
        } else {
            tempSql = "SELECT main.ROWID as id, 'HCCL' as pid, ci.groupName||'_'||ci.planeId as tid, main.startNs "
                "as timestamp, main.endNs as endTime, main.depth from TASK main join " +
                TABLE_COMMUNICATION_TASK_INFO +
                " ci on ci.globalTaskId = main.globalTaskId join ids on ids.id = ci.taskType" +
                " WHERE main.deviceId = ? and ci.groupName = ? AND ci.planeId = ? AND main.startNs >= ? AND "
                "main.endNs <= ?";
        }
    }
    return tempSql;
}

void TraceDatabaseHelper::SearchSliceNameWithLockRangeBindStmt(const SearchSliceParams &params,
    const std::vector<TrackQuery> &trackQuery, std::unique_ptr<SqlitePreparedStatement> &stmt, const std::string &path,
    const std::string &deviceId)
{
    stmt->BindParams(params.searchContent);
    for (const auto &item : trackQuery) {
        BindSearchNameWithLockRangeStmt(stmt, path, deviceId, item);
    }
}

void TraceDatabaseHelper::BindSearchNameWithLockRangeStmt(std::unique_ptr<SqlitePreparedStatement> &stmt,
    const std::string &path, const std::string &deviceId, const TrackQuery &item)
{
    PROCESS_TYPE type = STR_TO_ENUM<PROCESS_TYPE>(item.metaType).value();
    if (type == PROCESS_TYPE::API && item.rankId == path) {
        stmt->BindParams(item.processId, item.startTime, item.endTime);
    } else if (type == PROCESS_TYPE::CANN_API && item.rankId == path) {
        stmt->BindParams(item.processId, item.threadId, item.startTime, item.endTime);
    } else if (type == PROCESS_TYPE::MS_TX && item.rankId == path) {
        stmt->BindParams(item.processId, item.startTime, item.endTime);
    } else if (type == PROCESS_TYPE::ASCEND_HARDWARE && item.rankId != path) {
        stmt->BindParams(deviceId, item.threadId, item.startTime, item.endTime);
    } else if (type == PROCESS_TYPE::HCCL && item.rankId != path) {
        if (StringUtil::EndWith(item.threadId, "group")) {
            std::string tid = item.threadId.substr(0, item.threadId.size() - 5);
            stmt->BindParams(tid, item.startTime, item.endTime);
        } else {
            std::string groupName = item.threadId;
            std::string threadId = item.threadId;
            size_t pos = item.threadId.find_last_of("_");
            if (pos != std::string::npos && item.threadId.size() > pos) {
                threadId = item.threadId.substr(pos + 1);
                groupName = item.threadId.substr(0, pos);
            }
            stmt->BindParams(deviceId, groupName, threadId, item.startTime, item.endTime);
        }
    }
}

std::string TraceDatabaseHelper::GetSearchCountWithLockSql(const SearchCountParams &params,
    const std::vector<TrackQuery> &trackQuery)
{
    std::string sql;
    std::string nameMatch;
    if (params.isMatchExact && params.isMatchCase) {
        nameMatch = "select id from STRING_IDS where value like ?";
    } else if (params.isMatchExact) {
        nameMatch = "select id from STRING_IDS where lower(value) like lower(?)";
    } else if (params.isMatchCase) {
        nameMatch = "select id from STRING_IDS where value like '%'||?||'%'";
    } else {
        nameMatch = "select id from STRING_IDS where lower(value) like lower('%'||?||'%')";
    }
    sql = "with ids as (" + nameMatch + ") ";
    std::vector<std::string> sqls;
    for (const auto &item : trackQuery) {
        std::string tempSql = GetSingleSearchCountLockRangeSql(params, item);
        if (!tempSql.empty()) {
            sqls.emplace_back(tempSql);
        }
    }
    sql = sql + StringUtil::join(sqls, " UNION ALL ");
    return sql;
}

std::string TraceDatabaseHelper::GetSingleSearchCountLockRangeSql(const SearchCountParams &params,
    const TrackQuery &item)
{
    PROCESS_TYPE type = STR_TO_ENUM<PROCESS_TYPE>(item.metaType).value();
    std::string tempSql;
    if (type == PROCESS_TYPE::API && StringUtil::EndWith(params.rankId, "Host")) {
        tempSql = "SELECT count(1) FROM (SELECT name from " + TABLE_API +
            " WHERE globalTid = ? AND startNs >= ? AND endNs <= ?) api join ids on id = api.name ";
    } else if (type == PROCESS_TYPE::CANN_API && StringUtil::EndWith(params.rankId, "Host")) {
        tempSql = "SELECT count(1) FROM (SELECT name from " + TABLE_CANN_API +
            " WHERE globalTid = ? AND type = ? AND startNs >= ? AND endNs <= ?) cann join ids on id = cann.name ";
    } else if (type == PROCESS_TYPE::MS_TX && StringUtil::EndWith(params.rankId, "Host")) {
        tempSql = "SELECT count(1) FROM (SELECT message from " + TABLE_MSTX_EVENTS +
            " WHERE globalTid = ? AND startNs >= ? AND endNs <= ?) mstx join ids on id = mstx.message ";
    } else if (type == PROCESS_TYPE::ASCEND_HARDWARE && !StringUtil::EndWith(params.rankId, "Host")) {
        tempSql = "SELECT count(1) FROM (SELECT coalesce(c.name, m.message, s.name, main.taskType) as "
            "name FROM " +
            TABLE_TASK +
            " main "
            " left join " +
            TABLE_COMPUTE_TASK_INFO +
            " c on c.globalTaskId = main.globalTaskId "
            " left join " +
            TABLE_MSTX_EVENTS +
            " m on "
            " (m.connectionId = main.connectionId and  m.connectionId != " +
            WRONG_DATA + " ) left join " + TABLE_COMMUNICATION_SCHEDULE_TASK +
            " s on main.globalTaskId = s.globalTaskId WHERE main.deviceId = ? AND main.streamId = ? AND "
            "main.startNs >= ? AND main.endNs <= ?) hadware  join ids on id = hadware.name ";
    } else if (type == PROCESS_TYPE::HCCL && !StringUtil::EndWith(params.rankId, "Host")) {
        if (StringUtil::EndWith(item.threadId, "group")) {
            tempSql = "SELECT count(1) FROM (SELECT opName as name from " + TABLE_COMMUNICATION_OP +
                " WHERE groupName = ? AND startNs >= ? AND endNs <= ?) op join ids on id = op.name ";
        } else {
            tempSql = "SELECT count(1) FROM (SELECT ci.taskType as name from TASK main left join " +
                TABLE_COMMUNICATION_TASK_INFO + " ci on ci.globalTaskId = main.globalTaskId " +
                " WHERE main.deviceId = ? and ci.groupName = ? AND ci.planeId = ? AND main.startNs >= ? AND "
                "main.endNs <= ?) info join ids on id = info.name ";
        }
    }
    return tempSql;
}

void TraceDatabaseHelper::SearchCountWithLockRangeBindStmt(const SearchCountParams &params,
    const std::vector<TrackQuery> &trackQuery, std::unique_ptr<SqlitePreparedStatement> &stmt,
    const std::string &deviceId)
{
    stmt->BindParams(params.searchContent);
    for (const auto &item : trackQuery) {
        BindSingleTrackStmt(params, stmt, deviceId, item);
    }
}

void TraceDatabaseHelper::BindSingleTrackStmt(const SearchCountParams &params,
    std::unique_ptr<SqlitePreparedStatement> &stmt, const std::string &deviceId, const TrackQuery &item)
{
    PROCESS_TYPE type = STR_TO_ENUM<PROCESS_TYPE>(item.metaType).value();
    if (type == PROCESS_TYPE::API && StringUtil::EndWith(params.rankId, "Host")) {
        stmt->BindParams(item.processId, item.startTime, item.endTime);
    } else if (type == PROCESS_TYPE::CANN_API && StringUtil::EndWith(params.rankId, "Host")) {
        stmt->BindParams(item.processId, item.threadId, item.startTime, item.endTime);
    } else if (type == PROCESS_TYPE::MS_TX && StringUtil::EndWith(params.rankId, "Host")) {
        stmt->BindParams(item.processId, item.startTime, item.endTime);
    } else if (type == PROCESS_TYPE::ASCEND_HARDWARE && !StringUtil::EndWith(params.rankId, "Host")) {
        stmt->BindParams(deviceId, item.threadId, item.startTime, item.endTime);
    } else if (type == PROCESS_TYPE::HCCL && !StringUtil::EndWith(params.rankId, "Host")) {
        if (StringUtil::EndWith(item.threadId, "group")) {
            std::string tid = item.threadId.substr(0, item.threadId.size() - 5);
            stmt->BindParams(tid, item.startTime, item.endTime);
        } else {
            std::string groupName = item.threadId;
            std::string threadId = item.threadId;
            size_t pos = item.threadId.find_last_of("_");
            if (pos != std::string::npos && item.threadId.size() > pos) {
                threadId = item.threadId.substr(pos + 1);
                groupName = item.threadId.substr(0, pos);
            }
            stmt->BindParams(deviceId, groupName, threadId, item.startTime, item.endTime);
        }
    }
}

std::string TraceDatabaseHelper::GetSearchSliceNameCountSql(bool isMatchExact, bool isMatchCase, std::string rankId)
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
        sql = "with ids as (" + nameMatch + ") SELECT count(1),? as id FROM (select name from " + TABLE_CANN_API +
            " union all select message from  " + TABLE_MSTX_EVENTS + " union all select name from  " + TABLE_API +
            ") api join ids on id = api.name";
        return sql;
    }
    std::string communicationOpSql;
    if (!TraceDatabaseHelper::IsDeviceIdUnique(rankId)) {
        communicationOpSql = "select opName as name from COMMUNICATION_OP op "
            " join tasks on op.connectionId = tasks.connectionId group by opId";
    } else {
        communicationOpSql = "select opName as name from COMMUNICATION_OP op";
    }
    sql = "with ids as (" + nameMatch +
        "), "
        "     tasks as (select globalTaskId, taskType, connectionId from TASK where deviceId = ?), "
        "     com as (select opId, info.globalTaskId,info.taskType as name from COMMUNICATION_TASK_INFO info "
        " join tasks on  info.globalTaskId = tasks.globalTaskId), "
        "     compute as (select info.globalTaskId, name from COMPUTE_TASK_INFO info join tasks "
        " on  info.globalTaskId = tasks.globalTaskId), "
        " schedule as (select info.globalTaskId, name from COMMUNICATION_SCHEDULE_TASK_INFO info left join tasks "
        " on info.globalTaskId = tasks.globalTaskId)"
        "select count(1) from ( "
        "    select coalesce(compute.name, schedule.name, main.taskType) as name from tasks main "
        "         left join compute on compute.globalTaskId = main.globalTaskId "
        " left join schedule ON main.globalTaskId = schedule.globalTaskId"
        "    union ALL select name from com "
        "    union ALL " +
        communicationOpSql + ") allNames join ids on id = allNames.name;";
    return sql;
}

std::string TraceDatabaseHelper::GetComOpSliceDetailsSql(const std::string &rankId)
{
    std::string communicationOpSql;
    std::vector<uint64_t> deviceId = TraceDatabaseHelper::GetDeviceIdList(rankId);
    if (deviceId.size() == 1) {
        communicationOpSql = " select " + std::to_string(deviceId[0]) +
            " as deviceId,opName as name,'HCCL' as pid,"
            "'HCCL' as metaType, groupName||'group' as tid, op.startNs - minTime.value as startTime, "
            "op.endNs - op.startNs as duration, 0 as depth, op.ROWID as id "
            " from COMMUNICATION_OP op join minTime";
    } else {
        communicationOpSql = " select tasks.deviceId,opName as name,'HCCL' as pid, 'HCCL' as metaType, "
            " groupName||'group' as tid, op.startNs - minTime.value as startTime, "
            " op.endNs - op.startNs as duration, 0 as depth, op.ROWID as id from COMMUNICATION_OP op "
            " join minTime join tasks on op.connectionId = tasks.connectionId group by opId";
    }
    return communicationOpSql;
}

std::string TraceDatabaseHelper::GetMsTxEventsSliceDetailSql()
{
    return " SELECT '' AS deviceId, message as name, globalTid AS pid, 'HOST' AS metaType,"
        "'MsTx' AS tid, startNs - minTime.value AS startTime, endNs - startNs AS duration,"
        "depth, MSTX_EVENTS.ROWID AS id "
        "FROM MSTX_EVENTS JOIN minTime ";
}


std::string TraceDatabaseHelper::GetSearchAllSlicesDetailsSql(bool isMatchExact, bool isMatchCase,
    const std::string &order, const std::string &orderByField, const std::string &rankId)
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
    std::string communicationOpSql = GetComOpSliceDetailsSql(rankId);
    std::string mstxEventsSql = GetMsTxEventsSliceDetailSql();
    sql = "with ids as (" + nameMatch +
        "), minTime as (select ? as value),\n"
        " tasks as (select deviceId, TASK.ROWID, globalTaskId, taskType, 'Ascend Hardware' as pid, streamId as tid, "
        " startNs - minTime.value as startTime,endNs - startNs as duration,depth,connectionId from TASK join minTime "
        " where deviceId = ? ORDER BY startTime),\n"
        " com as (select deviceId, opId, tasks.ROWID as id, 'HCCL' as pid, groupName || '_' || planeId as tid,"
        " startTime, duration, 0 as depth, info.taskType as name"
        " from COMMUNICATION_TASK_INFO info join tasks on info.globalTaskId=tasks.globalTaskId "
        " ORDER BY startTime)\n"
        " select * from ( select deviceId, coalesce(compute.name, schedule.name, main.taskType) as name, main.pid,"
        " main.pid as metaType,"
        " main.tid, main.startTime, main.duration, main.depth, main.ROWID as id from tasks main\n"
        " left join COMPUTE_TASK_INFO compute on compute.globalTaskId = main.globalTaskId "
        " LEFT JOIN COMMUNICATION_SCHEDULE_TASK_INFO schedule ON main.globalTaskId = schedule.globalTaskId union ALL"
        " select deviceId,name, pid, pid as meatType, tid, startTime, duration, depth, id from com union ALL " +
        communicationOpSql +
        " UNION all select '' as deviceId, name, globalTid as pid, 'HOST' as metaType,"
        " type as tid, "
        "startNs - minTime.value AS startTime, endNs - startNs AS duration, depth, CANN_API.ROWID as id from "
        "CANN_API JOIN minTime UNION all " + mstxEventsSql + "UNION all select '' as deviceId, name, globalTid as pid,"
        "'HOST' as metaType, 'pytorch' as tid, "
        "startNs - minTime.value AS startTime, endNs - startNs AS duration, depth, PYTORCH_API.ROWID as id from "
        "PYTORCH_API JOIN minTime ) allNames join ids on ids.id = allNames.name" +
        orderBy + " LIMIT ? OFFSET ?";
    return sql;
}

std::string TraceDatabaseHelper::GetSearchSliceNameSql(bool isMatchExact, bool isMatchCase, std::string rankId,
    const std::string &path)
{
    const std::string order = "descend";
    const std::string orderByField = "timestamp";
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
        std::string associationTaskSql;
        if (!TraceDatabaseHelper::IsDeviceIdUnique(path)) {
            associationTaskSql = "join tasks on op.connectionId = tasks.connectionId";
        }
        std::string comSql = "select opName as name,'HCCL' as pid, 'HCCL' as metaType, groupName||'group' as tid,"
            " startNs - minTime.value as startTime, endNs - startNs as duration, 0 as depth, op.ROWID"
            " as id from COMMUNICATION_OP op join minTime " +
            associationTaskSql + " join ids on ids.id = opName group by opId";
        sql = "with ids as (" + nameMatch + "), minTime as (select ? as value), "
            " tasks as (select ROWID, globalTaskId, taskType, 'Ascend Hardware' as pid, streamId as tid, connectionId, "
            " startNs - minTime.value as startTime, endNs - startNs as duration,depth from TASK join minTime "
            " where deviceId = ? ORDER BY startTime), "
            " com as (select opId, tasks.ROWID as id, 'HCCL' as pid, groupName || '_' || planeId as tid, "
            " startTime, duration, 0 as depth, info.taskType as name from COMMUNICATION_TASK_INFO info "
            " join tasks on info.globalTaskId=tasks.globalTaskId ORDER BY startTime) "
            " select * from ( select coalesce(compute.name, schedule.name, main.taskType) as name, main.pid, main.pid "
            " as metaType, main.tid, main.startTime, main.duration, main.depth, main.ROWID as id from tasks main "
            " left join COMPUTE_TASK_INFO compute on compute.globalTaskId = main.globalTaskId "
            " left join COMMUNICATION_SCHEDULE_TASK_INFO schedule ON main.globalTaskId = schedule.globalTaskId "
            " join ids on ids.id = coalesce(compute.name, schedule.name, main.taskType) "
            " union ALL select name, pid, pid as meatType, tid, startTime, duration, depth, com.id from com "
            " join ids on ids.id = com.name  union ALL " + comSql + " ) allNames " + orderBy + " LIMIT 1 OFFSET ?";
    }
    return sql;
}

void TraceDatabaseHelper::ProcessByteAlignmentAnalyzerDataForText(std::vector<CommunicationLargeOperatorInfo> &result,
    std::vector<std::pair<std::string, std::string>> rawData)
{
    bool hasOneHcom = false;
    CommunicationLargeOperatorInfo op;
    for (const auto &item : rawData) {
        if (item.first.find("hcom") == 0) {
            if (hasOneHcom) {
                result.push_back(op);
            } else {
                hasOneHcom = true;
            }
            op.name = item.first;
            op.memcpyTasks.clear();
            op.reduceInlineTasks.clear();
        } else {
            if (!hasOneHcom) {
                continue;
            }
            Dic::document_t json;
            json.Parse(item.second.c_str());
            if (!json.IsObject()) {
                ServerLog::Error("Args is not valid json format. raw: %", item.second);
                continue;
            }
            if (!json.HasMember("size(Byte)") || !json["size(Byte)"].IsString()) {
                ServerLog::Error("Args has no member size(Byte) or member is not int. raw: %", item.second);
                continue;
            }
            if (!json.HasMember("transport type") || !json["transport type"].IsString()) {
                ServerLog::Error("Args has no member transport type or member is not string. raw: %", item.second);
                continue;
            }
            if (!json.HasMember("link type") || !json["link type"].IsString()) {
                ServerLog::Error("Args has no member link type or member is not string. raw: %", item.second);
                continue;
            }
            CommunicationSmallOperatorInfo info;
            int64_t tempSize = NumberUtil::StringToLongLong(json["size(Byte)"].GetString());
            info.size = (tempSize < 0 ? 0 : static_cast<uint64_t>(tempSize));
            info.transportType = json["transport type"].GetString();
            info.linkType = json["link type"].GetString();
            if (item.first.find("Memcpy") == 0) {
                op.memcpyTasks.emplace_back(info);
            } else {
                op.reduceInlineTasks.emplace_back(info);
            }
        }
    }
    result.push_back(op);
}

void TraceDatabaseHelper::ProcessByteAlignmentAnalyzerDataForDb(std::vector<CommunicationLargeOperatorInfo> &result,
    std::vector<ByteAlignmentAnalyzerLargeOperatorInfo> &largeOpInfo,
    std::vector<ByteAlignmentAnalyzerSmallOperatorInfo> &smallOpInfo)
{
    std::map<std::string, CommunicationLargeOperatorInfo> resultMap;
    for (const auto &singleLargeOp : largeOpInfo) {
        CommunicationLargeOperatorInfo info;
        info.name = singleLargeOp.name;
        resultMap[singleLargeOp.name] = info;
    }
    for (const auto &singleSmallOp : smallOpInfo) {
        if (resultMap.find(singleSmallOp.name) == resultMap.end()) {
            continue;
        }
        CommunicationSmallOperatorInfo smallOpInfo;
        smallOpInfo.size = singleSmallOp.size;
        smallOpInfo.transportType = singleSmallOp.transportType;
        smallOpInfo.linkType = singleSmallOp.linkType;
        if (singleSmallOp.taskType.find("Memcpy") == 0) {
            resultMap[singleSmallOp.name].memcpyTasks.emplace_back(smallOpInfo);
        } else {
            resultMap[singleSmallOp.name].reduceInlineTasks.emplace_back(smallOpInfo);
        }
    }
    for (const auto &item : resultMap) {
        result.emplace_back(item.second);
    }
}

void TraceDatabaseHelper::ComputeTree(std::vector<std::unique_ptr<Protocol::UnitTrack>>& metaData,
                                      std::vector<Process>& processes,
                                      std::vector<std::unique_ptr<Protocol::UnitTrack>>& tempMetaData)
{
    std::unordered_map<std::string, UnitTrack*> idToRawPtr;
    std::unordered_map<std::string, std::unique_ptr<UnitTrack>> idToOwnerPtr;
    // 创建所有节点，放到 owner map 中
    for (auto& item : tempMetaData) {
        std::string pid = item->metaData.processId;
        idToRawPtr[pid] = item.get();
        idToOwnerPtr[pid] = std::move(item);
    }
    // 建立树结构
    for (auto& item : processes) {
        std::string id = item.pid;
        UnitTrack* node = idToRawPtr[id];
        if (node->metaData.parentProcessId != "0" && idToRawPtr.count(node->metaData.parentProcessId)) {
            UnitTrack* parent = idToRawPtr[node->metaData.parentProcessId];
            parent->children.push_back(std::move(idToOwnerPtr[id]));
        } else {
            metaData.push_back(std::move(idToOwnerPtr[id]));
        }
    }
}
}