/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "pch.h"
#include "TableDefs.h"
#include "TraceTime.h"
#include "DbClusterDataBase.h"

namespace Dic {
using namespace Server;
namespace Module {
namespace FullDb {
DbClusterDataBase::~DbClusterDataBase() {}

bool DbClusterDataBase::QuerySummaryData(const Protocol::SummaryTopRankParams &requestParams,
    Protocol::SummaryTopRankResBody &responseBody)
{
    std::string stepCondition;
    std::string rankCondition;
    std::string prepareTimeCondition;
    if (HasColumn(TABLE_STEP_TRACE_TIME, "preparing")) {
        prepareTimeCondition = ",sum(ROUND(preparing,2)) as prepareTime ";
    }
    if (!requestParams.stepIdList.empty()) {
        stepCondition = " and step in (?";
        for (size_t i = 1; i < requestParams.stepIdList.size(); i++) {
            stepCondition.append(",?");
        }
        stepCondition.append(") ");
    }
    if (!requestParams.rankIdList.empty()) {
        rankCondition = " and \"index\" in (?";
        for (size_t i = 1; i < requestParams.rankIdList.size(); i++) {
            rankCondition.append(",?");
        }
        rankCondition.append(") ");
    }
    std::string sql = "SELECT \"index\" as rankId, sum(ROUND(computing,2)) as computingTime, "
                      "sum(ROUND(communication_not_overlapped,2)) as communicationNotOverLappedTime, "
                      "sum(ROUND(overlapped,2)) as communicationOverLappedTime, sum(ROUND(free,2)) as freeTime "+
                      prepareTimeCondition +
                      "FROM " + TABLE_STEP_TRACE_TIME +
                      " WHERE rankId !='' " + stepCondition + rankCondition
                      + "group by rankId ";
    if (!StringUtil::CheckSqlValid(requestParams.orderBy)) {
        ServerLog::Error("There is an SQL injection attack on this parameter. error param: ", requestParams.orderBy);
        return ExecuteQuerySummaryData(requestParams, responseBody, sql);
    }
    if (requestParams.orderBy == "rankId") {
        sql += " ORDER by CAST(rankId AS UNSIGNED) asc";
    } else {
        sql += " ORDER by " + requestParams.orderBy + " desc";
    }

    return ExecuteQuerySummaryData(requestParams, responseBody, sql);
}

std::string DbClusterDataBase::QueryParseClusterStatus()
{
    return parseStatus;
}

void DbClusterDataBase::UpdateClusterParseStatus(std::string status)
{
    parseStatus = status;
}

bool DbClusterDataBase::QueryBaseInfo(Protocol::SummaryTopRankResBody &responseBody)
{
    std::string filePath = responseBody.filePath;
    int64_t dataSize = FileUtil::GetFileSize(filePath.c_str());
    std::string baseInfoSql = "select (select json_group_array(\"index\") as rank from (select DISTINCT \"index\" from "
        "ClusterStepTraceTime where \"index\" !='' AND type = 'rank')) as rank , (select json_group_array(step) from ("
        "select DISTINCT step from " + TABLE_STEP_TRACE_TIME +
        " where \"index\" !='')) as step, '" + std::to_string(dataSize) + "' as dataSize ";
    return ExecuteQueryBaseInfo(responseBody, baseInfoSql);
}

bool DbClusterDataBase::GetStepIdList(Protocol::PipelineStepResponseBody &responseBody)
{
    std::string sql = "select distinct step as stepId "
                      "FROM " + TABLE_STEP_TRACE_TIME +
                      " ORDER BY step";
    return ExecuteGetStepIdList(responseBody, sql);
}

bool DbClusterDataBase::GetStages(Protocol::PipelineStageParam &param,
                                  Protocol::PipelineStageResponseBody &responseBody)
{
    std::string sql = "SELECT DISTINCT stage as stageId "
                      "FROM " + TABLE_STEP_TRACE_TIME + " WHERE stage != '' AND step = ?";
    return ExecuteGetStages(param, responseBody, sql);
}

bool DbClusterDataBase::GetStageAndBubble(Protocol::PipelineStageTimeParam &param,
                                          Protocol::PipelineStageOrRankTimeResponseBody &responseBody)
{
    std::string sql = "SELECT max(ROUND(stage, 4)) as stageTime, "
                      "max(ROUND(bubble, 4)) as bubbleTime "
                      "FROM " + TABLE_STEP_TRACE_TIME + " WHERE step = ?";

    std::vector<std::string> stageIds;
    PrepareForStageId(param.stageId, sql, stageIds);

    return ExecuteGetStageAndBubble(param, stageIds, responseBody, sql);
}

bool DbClusterDataBase::GetRankAndBubble(Protocol::PipelineRankTimeParam &param,
                                         Protocol::PipelineStageOrRankTimeResponseBody &responseBody)
{
    std::string sql = "SELECT \"index\" as rankId, "
                      "ROUND(stage, 4) as stageTime, "
                      "ROUND(bubble, 4) as bubbleTime "
                      " FROM " + TABLE_STEP_TRACE_TIME +
                      " WHERE step = ? ";

    std::vector<std::string> stageIds;
    PrepareForStageId(param.stageId, sql, stageIds);

    return ExecuteGetRankAndBubble(param, std::move(stageIds), responseBody, std::move(sql));
}

bool DbClusterDataBase::GetGroups(Protocol::MatrixGroupParam &param, Protocol::MatrixGroupResponseBody &responseBody)
{
    std::string sql = "SELECT DISTINCT rank_set as rank FROM " + TABLE_COMM_ANALYZER_MATRIX;
    return ExecuteGetGroups(param, responseBody, sql);
}

bool DbClusterDataBase::QueryMatrixList(Protocol::MatrixBandwidthParam &param,
                                        Protocol::MatrixListResponseBody &responseBody)
{
    std::string sql = "SELECT src_rank as srcRank, dst_rank as dstRank, "
                      "transport_type as transportType, "
                      "ROUND(transit_size, 4) as transitSize, "
                      "ROUND(transit_time, 4) as transitTime, "
                      "ROUND(bandwidth, 4) as bandwidth ,"
                      "hccl_op_name as opName "
                      "FROM " + TABLE_COMM_ANALYZER_MATRIX +
                      " WHERE rank_set = ? AND step = ? AND hccl_op_name = ? ";
    param.iterationId = "step" + param.iterationId;
    return ExecuteQueryMatrixList(param, responseBody, sql);
}

bool DbClusterDataBase::QueryExtremumTimestamp(uint64_t &min, uint64_t &max)
{
    std::string sql = "SELECT MIN(start_timestamp) * 1000 as minTime, MAX(start_timestamp) * 1000 as maxTime "
                      "FROM " + TABLE_COMM_ANALYZER_TIME + " WHERE start_timestamp != 0";
    return ExecuteQueryExtremumTimestamp(sql, min, max);
}

bool DbClusterDataBase::QueryIterationAndCommunicationGroup(Protocol::KernelParams &params,
    Protocol::OneKernelBody &responseBody, uint64_t minTimestamp)
{
    std::string sql = "select step, rank_set from " + TABLE_COMM_ANALYZER_TIME +
        " where hccl_op_name = ? and abs(start_timestamp - ? / 1000.0) * 1000 <= 500";

    uint64_t reallyStartTime = params.timestamp + minTimestamp;
    if (ExecuteQueryIterationAndCommunicationGroup(sql, params.name, reallyStartTime, responseBody.step,
        responseBody.group)) {
        // 形如"step0"处理为"0"
        std::string stepIdPrefix = "step";
        size_t found = responseBody.step.find(stepIdPrefix);
        if (found != std::string::npos) {
            responseBody.step.erase(found, stepIdPrefix.length());
        }
        return true;
    }
    return false;
}

bool DbClusterDataBase::QueryAllOperators(Protocol::OperatorDetailsParam &param,
    Protocol::OperatorDetailsResBody &resBody)
{
    uint64_t startTime = Module::Timeline::TraceTime::Instance().GetStartTime();
    std::string sql = "SELECT t.hccl_op_name as operatorName, "
        "ROUND((start_timestamp - ?/1000.0) / 1000.0, 3) as startTime, "
        "ROUND(elapsed_time, 4) as elapseTime, "
        "ROUND(transit_time, 4) as transitTime, "
        "ROUND(synchronization_time, 4) as synchronizationTime, "
        "ROUND(wait_time, 4) as waitTime, "
        "ROUND(idle_time, 4) as idleTime, "
        "CASE WHEN synchronization_time = 0 THEN 0 ELSE ROUND(synchronization_time "
        " / (synchronization_time + transit_time), 4) END AS synchronizationTimeRatio, "
        "CASE WHEN wait_time = 0 THEN 0 ELSE "
        "ROUND(wait_time / (wait_time + transit_time), 4) END AS waitTimeRatio, "
        "bw.sdma_bw as sdma_bw, bw.rdma_bw as rdma_bw "
        "FROM " + TABLE_COMM_ANALYZER_TIME + " t "
        "JOIN ( "
        "    SELECT hccl_op_name, "
        "    MAX(CASE WHEN band_type = 'SDMA' THEN bandwidth ELSE 0 END) AS sdma_bw, "
        "    MAX(CASE WHEN band_type = 'RDMA' THEN bandwidth ELSE 0 END) AS rdma_bw "
        "    FROM " + TABLE_COMM_ANALYZER_BANDWIDTH +
        "    WHERE step = ? AND rank_id = ? AND rank_set = ? AND hccl_op_name != 'Total Op Info'"
        "    GROUP BY hccl_op_name "
        ") bw ON t.hccl_op_name = bw.hccl_op_name "
        "WHERE t.step = ? AND t.rank_id = ? AND t.rank_set = ? AND t.hccl_op_name != 'Total Op Info'";
    return ExecuteQueryAllOperators(param, resBody, sql, startTime);
}

bool DbClusterDataBase::QueryOperatorsCount(Protocol::OperatorDetailsParam &param,
    Protocol::OperatorDetailsResBody &resBody)
{
    std::string sql = "SELECT op_name, count(*) AS nums from (select hccl_op_name as op_name, rank_id, step as "
                      "iteration_id, rank_set as stage_id from " + TABLE_COMM_ANALYZER_TIME + ") where 1=1 ";
    param.iterationId = "step" + param.iterationId;
    return ExecuteQueryOperatorsCount(param, resBody, sql);
}

bool DbClusterDataBase::QueryBandwidthData(Protocol::BandwidthDataParam &param, Protocol::BandwidthDataResBody &resBody)
{
    std::string sql = "SELECT band_type, ROUND(transit_size, 4) as transit_size,"
                      "ROUND(transit_time, 4) as transit_time,"
                      "ROUND(bandwidth, 4) as bandwidth_size,"
                      "ROUND(large_packet_ratio, 4)  as large_packet_ratio from "
                      "(SELECT*,ROW_NUMBER() OVER (PARTITION BY band_type) AS rn FROM (SELECT * FROM "
                      + TABLE_COMM_ANALYZER_BANDWIDTH +
                      " WHERE step = ? AND rank_id = ? AND rank_set = ? AND hccl_op_name = ?))t "
                      "WHERE rn = 1";
    param.iterationId = "step" + param.iterationId;
    return ExecuteQueryBandwidthData(param, resBody, sql);
}

bool DbClusterDataBase::QueryDistributionData(Protocol::DistributionDataParam &param,
    Protocol::DistributionResBody &resBody)
{
    std::string sql = "SELECT package_size, count, total_duration FROM "
                      + TABLE_COMM_ANALYZER_BANDWIDTH +
                      " WHERE step = ? "
                      "AND rank_id = ? "
                      "AND rank_set = ? "
                      "AND hccl_op_name = ? "
                      "AND band_type = ? ;";
    param.iterationId = "step" + param.iterationId;
    sqlite3_stmt *stmt = nullptr;
    int index = bindStartIndex;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        Server::ServerLog::Error("Failed to prepare a statement to query distributed data. error:", sqlite3_errmsg(db));
        return false;
    }
    sqlite3_bind_text(stmt, index++, param.iterationId.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, index++, param.rankId.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, index++, param.stage.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, index++, param.operatorName.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, index, param.transportType.c_str(), -1, SQLITE_STATIC);
    std::string distribution = "{";
    int num = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        std::string package_size = sqlite3_column_string(stmt, col++);
        std::string count = sqlite3_column_string(stmt, col++);
        std::string total_duration = sqlite3_column_string(stmt, col++);
        std::string value = "\"" + package_size + "\":[" + count + "," + total_duration + "],";
        num += 1;
        distribution += value;
    }
    if (num == 0) {
        distribution = "";
    } else {
        distribution.erase(distribution.size() - 1);
        distribution += "}";
    }
    resBody.distributionData = distribution;
    sqlite3_finalize(stmt);
    return true;
}

bool DbClusterDataBase::QueryRanksHandler(std::vector<Protocol::IterationsOrRanksObject> &responseBody)
{
    std::string sql = "SELECT json_group_array (ranks) FROM ( "
                      "SELECT DISTINCT \"index\" as ranks FROM " + TABLE_STEP_TRACE_TIME +
                      " WHERE ranks != '' AND type = 'rank')";
    return ExecuteQueryRanksHandler(responseBody, sql);
}

bool DbClusterDataBase::QueryOperatorNames(Protocol::OperatorNamesParams &requestParams,
    std::vector<Protocol::OperatorNamesObject> &responseBody)
{
    std::vector<std::string> rankList = requestParams.rankList;
    std::string sql;
    if (rankList.empty()) {
        sql = "SELECT DISTINCT hccl_op_name FROM (SELECT hccl_op_name FROM " + TABLE_COMM_ANALYZER_TIME +
              " WHERE step = ?" +
              " AND rank_set = ?" +
              " ORDER BY hccl_op_name)";
    } else {
        std::string ranks = GetRanksSql(rankList);
        sql = "SELECT DISTINCT hccl_op_name FROM (SELECT hccl_op_name FROM " + TABLE_COMM_ANALYZER_TIME +
              " WHERE step = ?" +
              " AND rank_set = ?" +
              " AND rank_id IN " + ranks + " ORDER BY hccl_op_name)";
    }
    requestParams.iterationId = "step" + requestParams.iterationId;
    return ExecuteQueryOperatorNames(requestParams, responseBody, sql);
}

bool DbClusterDataBase::QueryIterations(std::vector<Protocol::IterationsOrRanksObject> &responseBody)
{
    std::string sql = "select json_group_array(step) from ("
                      "select DISTINCT step from "+ TABLE_STEP_TRACE_TIME +
                      " where \"index\" !='')";
    return ExecuteQueryIterations(responseBody, sql);
}

bool DbClusterDataBase::QueryDurationList(Protocol::DurationListParams &requestParams,
    Protocol::DurationListsResponseBody &responseBody)
{
    uint64_t startTime = Module::Timeline::TraceTime::Instance().GetStartTime();
    std::string rankSql;
    std::string rankSqlTime;
    if (!requestParams.rankList.empty()) {
        std::string ranks = GetRanksSql(requestParams.rankList);
        rankSql = " AND rank_id IN " + ranks;
        rankSqlTime = " AND t.rank_id IN " + ranks;
    }

    std::string sql = "SELECT t.rank_id as rank_id, "
        "CASE WHEN start_timestamp = 0 THEN 0 ELSE ROUND((start_timestamp - ?/1000.0) / 1000.0, 4) END, "
        "ROUND(elapsed_time, 4) as elapse_time, ROUND(transit_time, 4) as transit_time, "
        "ROUND(synchronization_time, 4) as synchronization_time, ROUND(wait_time, 4) as wait_time, "
        "ROUND(idle_time, 4) as idle_time, "
        "CASE WHEN synchronization_time = 0 THEN 0 ELSE ROUND(synchronization_time "
        " / (synchronization_time + transit_time), 4) END AS synchronization_time_ratio, "
        "CASE WHEN wait_time = 0 THEN 0 ELSE "
        " ROUND(wait_time / (wait_time + transit_time), 4) END AS wait_time_ratio, "
        "bw.sdma_bw as sdma_bw, bw.rdma_bw as rdma_bw, bw.sdma_time as sdma_time, bw.rdma_time as rdma_time "
        "FROM " + TABLE_COMM_ANALYZER_TIME + " t "
        "JOIN ("
        "    SELECT rank_id, "
        "    MAX(CASE WHEN band_type = 'SDMA' THEN bandwidth ELSE 0 END) AS sdma_bw, "
        "    MAX(CASE WHEN band_type = 'RDMA' THEN bandwidth ELSE 0 END) AS rdma_bw, "
        "    MAX(CASE WHEN band_type = 'SDMA' THEN transit_time ELSE 0 END) AS sdma_time, "
        "    MAX(CASE WHEN band_type = 'RDMA' THEN transit_time ELSE 0 END) AS rdma_time "
        "    FROM " + TABLE_COMM_ANALYZER_BANDWIDTH +
        "    WHERE step = ? AND rank_set = ? AND hccl_op_name = ? " + rankSql +
        "    GROUP BY rank_id "
        ") bw ON t.rank_id = bw.rank_id "
        " WHERE t.step = ? AND t.rank_set = ? AND t.hccl_op_name = ? " + rankSqlTime;
    requestParams.iterationId = "step" + requestParams.iterationId;
    return ExecuteQueryDurationList(requestParams, responseBody, sql, startTime);
}

bool DbClusterDataBase::QueryOperatorList(Protocol::DurationListParams &requestParams,
    Protocol::OperatorListsResponseBody &responseBody)
{
    std::string sql =
        "SELECT rank_id, hccl_op_name as op_name,"
        " CASE WHEN start_timestamp = 0 THEN 0 ELSE (start_timestamp - ?/1000.0)*1000.0 END as start_time, "
        " (elapsed_time * 1000000) as elapse_time From " + TABLE_COMM_ANALYZER_TIME +
        " WHERE step = ? AND rank_set = ? AND hccl_op_name <> 'Total Op Info'";
    std::vector<std::string> rankList = requestParams.rankList;
    if (!rankList.empty()) {
        std::string ranks = GetRanksSql(rankList);
        sql += " AND rank_id IN " + ranks;
    }
    if (requestParams.operatorName != totalOpInfo) {
        sql += " AND hccl_op_name = ?";
    }
    sql += " ORDER by CAST(rank_id AS UNSIGNED) ASC";
    requestParams.iterationId = "step" + requestParams.iterationId;
    uint64_t startTime = Module::Timeline::TraceTime::Instance().GetStartTime();
    return ExecuteQueryOperatorList(requestParams, responseBody, sql, startTime);
}

bool DbClusterDataBase::QueryCommunicationGroup(Document &responseBody)
{
    std::string baseInfoSql =
            " with ComGroup as (select distinct REPLACE(REPLACE(rank_set, '(', '['), ')', ']') as stage, "
            " type from " + TABLE_COMM_GROUP + ") "
            " select (select group_concat(stage, ',') from ComGroup where type = 'collective') as stage, "
            "       (select group_concat(stage, ',') from ComGroup where type = 'p2p') as pp_stages";
    sqlite3_stmt *stmtBaseInfo = nullptr;
    int baseInfoResult = sqlite3_prepare_v2(db, baseInfoSql.c_str(), -1, &stmtBaseInfo, nullptr);
    if (baseInfoResult != SQLITE_OK) {
        Server::ServerLog::Error("Failed to Query CommunicationGroup info statement. error:", sqlite3_errmsg(db));
        return false;
    }
    responseBody.SetObject();
    auto allocator = responseBody.GetAllocator();
    while (sqlite3_step(stmtBaseInfo) == SQLITE_ROW) {
        int coll = resultStartIndex;
        std::string stages(sqlite3_column_string(stmtBaseInfo, coll++));
        if (!stages.empty()) {
            std::string stage = "[" + stages + "]";
            responseBody.AddMember("tpOrDpGroups", Document(kArrayType, &allocator).Parse(stage.c_str()), allocator);
        }
        std::string ppStages(sqlite3_column_string(stmtBaseInfo, coll++));
        if (!ppStages.empty()) {
            std::string ppStage = "[" + ppStages + "]";
            responseBody.AddMember("ppGroups", Document(kArrayType, &allocator).Parse(ppStage.c_str()), allocator);
            responseBody.AddMember("defaultPPSize", responseBody["ppGroups"].Size(), allocator);
        }
    }
    sqlite3_finalize(stmtBaseInfo);
    return true;
}

bool DbClusterDataBase::QueryMatrixSortOpNames(Protocol::OperatorNamesParams &requestParams,
    std::vector<Protocol::OperatorNamesObject> &responseBody)
{
    std::string sql = "SELECT DISTINCT hccl_op_name  FROM " + TABLE_COMM_ANALYZER_MATRIX +
                      " WHERE step = ?" +
                      " AND rank_set = ?" +
                      " ORDER BY hccl_op_name";
    requestParams.iterationId = "step" + requestParams.iterationId;
    return ExecuteQueryMatrixSortOpNames(requestParams, responseBody, sql);
}

void DbClusterDataBase::PrepareForStageId(std::string &stageIdStr, std::string &sql, std::vector<std::string> &stageIds)
{
    stageIdStr.erase(std::remove(stageIdStr.begin(), stageIdStr.end(), '('), stageIdStr.end());
    stageIdStr.erase(std::remove(stageIdStr.begin(), stageIdStr.end(), ')'), stageIdStr.end());
    std::vector<std::string> stageIdArray = StringUtil::Split(stageIdStr, ",");
    const int maxBindParam = 1000;
    if (stageIdStr.size() > maxBindParam) {
        Server::ServerLog::Error("Too many parameters, binding failed. stage id size:", stageIdStr.size());
        return;
    }
    for (std::string stageId : stageIdArray) {
        stageIds.push_back(StringUtil::Trim(stageId));
    }

    std::string rankSql;
    for (size_t i = 0; i < stageIds.size(); i++) {
        if (i == 0) {
            rankSql.append("?");
        }
        rankSql.append(",?");
    }
    if (!rankSql.empty()) {
        sql += "AND \"index\" IN (" + rankSql + ")";
    }
}

bool DbClusterDataBase::QueryParallelStrategyConfig(ParallelStrategyConfig &config, std::string &level)
{
    return false;
}

bool DbClusterDataBase::UpdateParallelStrategyConfig(const ParallelStrategyConfig &config,
    std::string &level, std::string &msg)
{
    return false;
}

bool DbClusterDataBase::GetParallelConfigFromStepTrace(ParallelStrategyConfig &config)
{
    static const std::vector<std::string> validHeads = {"dp_index", "pp_index", "tp_index"};
    for (auto &item : validHeads) {
        if (!HasColumn(TABLE_STEP_TRACE_TIME, item)) {
            ServerLog::Warn(TABLE_STEP_TRACE_TIME, " didn't have parallel strategy config.");
            return false;
        }
    }
    std::string sql = "select max(dp_index) + 1 as dp_size, max(pp_index) + 1 as pp_size, max(tp_index) + 1 as tp_size "
                      " from " + TABLE_STEP_TRACE_TIME + " where type = 'rank'";
    return ExecuteGetParallelConfigFromStepTrace(sql, config);
}
}
}
}