/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "TraceDatabaseHelper.h"


namespace Dic::Module::Timeline {

void TraceDatabaseHelper::QueryTaskInfoById(std::unique_ptr<SqlitePreparedStatement> &stmt,
                                            const Protocol::ThreadDetailParams &requestParams,
                                            Protocol::UnitThreadDetailBody &responseBody,
                                            std::map<std::string, std::string> &stringCache)
{
    auto processType = GetProcessType(requestParams.metaType);
    bool attrInfoExist = isAttrInfoExist(stmt);
    auto resultSet = QueryTaskCacheInfoById(stmt, requestParams, attrInfoExist);
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

    resultSet = QueryTaskStrInfoById(stmt, requestParams);
    if (resultSet.operator bool() && resultSet->Next()) {
        for (auto &item: resultSet->GetColumns()) {
            JsonUtil::AddConstMember(json, item.first, resultSet->GetString(item.second), allocator);
        }
    }
    responseBody.data.args = JsonUtil::JsonDump(json);
}

std::unique_ptr<SqliteResultSet> TraceDatabaseHelper::QueryTaskStrInfoById(
    std::unique_ptr<SqlitePreparedStatement> &stmt, const Protocol::ThreadDetailParams &requestParams)
{
    auto processType = GetProcessType(requestParams.metaType);
    std::string sql;
    std::unique_ptr<SqliteResultSet> resultSet;
    switch (processType) {
        case PROCESS_TYPE::ASCEND_HARDWARE:
            sql = "SELECT block_dim, mixBlockDim "
                  " FROM TASK main join COMPUTE_TASK_INFO CTI on main.globalTaskId = CTI.globalTaskId"
                  "  where main.ROWID = ?";
            return ExecuteQuery(stmt, sql, requestParams.id);
        case PROCESS_TYPE::HCCL:
            sql = "SELECT com.planeId, com.notifyId ,com.srcRank ,com.dstRank, com.size, com.opId "
                  "      FROM TASK main join COMMUNICATION_TASK_INFO com on main.globalTaskId = com.globalTaskId "
                  " where main.ROWID = ?";
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
    bool attrInfoExist)
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
            sql = "SELECT com.taskType, com.groupName ,com.rdmaType,com.transportType, com.dataType, com.linkType "
                  "      FROM TASK main join COMMUNICATION_TASK_INFO com on main.globalTaskId = com.globalTaskId "
                  " where main.ROWID = ?";
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
        mainSql = "with nameIds as ( select id, value as realName from STRING_IDS where lower(value) like ?), "
                  "     main as (select realName as name, endNs - startNs as duration from TASK task "
                  " join COMPUTE_TASK_INFO info on info.globalTaskId = task.globalTaskId join nameIds on name = id "
                  " where deviceId = ?),";
    } else if (requestParams.layer == "HCCL") {
        mainSql = "with nameIds as ( select id, value as realName from STRING_IDS where lower(value) like ?), "
              "     main as (select realName as name, endNs - startNs as duration from TASK task "
              " join COMMUNICATION_TASK_INFO info on info.globalTaskId = task.globalTaskId join nameIds on name = id "
              " where deviceId = ?),";
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

}