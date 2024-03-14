/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "DbClusterDataBase.h"
#include "TableDefs.h"
#include "FileUtil.h"

namespace Dic {
namespace Module {
namespace FullDb {
DbClusterDataBase::~DbClusterDataBase() {}

bool DbClusterDataBase::QuerySummaryData(const Protocol::SummaryTopRankParams &requestParams,
    Protocol::SummaryTopRankResBody &responseBody)
{
    std::string stepCondition;
    std::string rankCondition;
    if (!requestParams.stepIdList.empty()) {
        stepCondition = " and step in (?";
        for (int i = 1; i < requestParams.stepIdList.size(); i++) {
            stepCondition.append(",?");
        }
        stepCondition.append(") ");
    }
    if (!requestParams.rankIdList.empty()) {
        rankCondition = " and \"index\" in (?";
        for (int i = 1; i < requestParams.rankIdList.size(); i++) {
            rankCondition.append(",?");
        }
        rankCondition.append(") ");
    }
    std::string sql = "SELECT \"index\" as rankId, sum(ROUND(computing,2)) as computingTime, "
                      "sum(ROUND(communication_not_overlapped,2)) as communicationNotOverLappedTime, "
                      "sum(ROUND(overlapped,2)) as communicationOverLappedTime, "
                      "sum(ROUND(free,2)) as freeTime FROM " + TABLE_STEP_TRACE_TIME +
                      " WHERE rankId !='' " + stepCondition + rankCondition
                      + "group by rankId order by " + requestParams.orderBy + " desc";
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
    responseBody.dataSize = FileUtil::GetFileSize(filePath.c_str());
    return true;
}

bool DbClusterDataBase::GetStepIdList(Protocol::PipelineStepResponseBody &responseBody)
{
    std::string sql = "select distinct step as stepId "
                      "FROM " + TABLE_STEP_TRACE_TIME +
                      " ORDER BY step";
    return ExecuteGetStepIdList(responseBody, sql);
}

bool DbClusterDataBase::GetStages(Protocol::PipelineStageParam param, Protocol::PipelineStageResponseBody &responseBody)
{
    std::string sql = "SELECT DISTINCT stage as stageId "
                      "FROM " + TABLE_STEP_TRACE_TIME + " WHERE stage != '' AND step = ?";
    return ExecuteGetStages(param, responseBody, sql);
}

bool DbClusterDataBase::GetStageAndBubble(Protocol::PipelineStageTimeParam param,
    Protocol::PipelineStageOrRankTimeResponseBody &responseBody)
{
    std::string sql = "SELECT '" + param.stageId + "' as stageId, "
                      "max(ROUND(stage, 4)) as stageTime, "
                      "max(ROUND(bubble, 4)) as bubbleTime "
                      "FROM " + TABLE_STEP_TRACE_TIME + " WHERE \"index\" IN " + param.stageId + " AND step = ?";
    return ExecuteGetStageAndBubble(param, responseBody, sql);
}

bool DbClusterDataBase::GetRankAndBubble(Protocol::PipelineRankTimeParam param,
    Protocol::PipelineStageOrRankTimeResponseBody &responseBody)
{
    std::string sql = "SELECT \"index\" as rankId, "
                      "ROUND(stage, 4) as stageTime, "
                      "ROUND(bubble, 4) as bubbleTime "
                      " FROM " + TABLE_STEP_TRACE_TIME +
                      " WHERE step = ? AND \"index\" IN" + param.stageId + " ";
    return ExecuteGetRankAndBubble(param, responseBody, sql);
}

bool DbClusterDataBase::GetGroups(Protocol::MatrixGroupParam param, Protocol::MatrixGroupResponseBody &responseBody)
{
    std::string sql = "SELECT DISTINCT rank_set as rank FROM " + TABLE_COMM_ANALYZER_MATRIX;
    return ExecuteGetGroups(param, responseBody, sql);
}

bool DbClusterDataBase::QueryMatrixList(Protocol::MatrixBandwidthParam param,
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
    return ExecuteQueryMatrixList(param, responseBody, sql);
}

bool DbClusterDataBase::QueryAllOperators(Protocol::OperatorDetailsParam &param,
    Protocol::OperatorDetailsResBody &resBody)
{
    std::string sql = "SELECT hccl_op_name as operatorName, "
                      " ROUND(elapsed_time, 4) as elapseTime, "
                      " ROUND(transit_time, 4) as transitTime,"
                      " ROUND(synchronization_time, 4) as synchronizationTime,"
                      " ROUND(wait_time, 4) as waitTime,"
                      " ROUND(idle_time, 4) as idleTime,"
                      " ROUND(synchronization_time_ratio, 4) as synchronizationTimeRatio,"
                      " ROUND(wait_time_ratio, 4) as waitTimeRatio "
                      "FROM " + TABLE_COMM_ANALYZER_TIME +
                      " WHERE step = ? AND rank_id = ? AND rank_set = ?"
                      " AND hccl_op_name != 'Total Op Info' ";
    return ExecuteQueryAllOperators(param, resBody, sql);
}

bool DbClusterDataBase::QueryOperatorsCount(Protocol::OperatorDetailsParam &param,
    Protocol::OperatorDetailsResBody &resBody)
{
    std::string sql = "SELECT hccl_op_name, count(*) AS nums  from " + TABLE_COMM_ANALYZER_TIME + " where 1=1 ";
    return ExecuteQueryOperatorsCount(param, resBody, sql);
}

bool DbClusterDataBase::QueryBandwidthData(Protocol::BandwidthDataParam &param, Protocol::BandwidthDataResBody &resBody)
{
    std::string sql = "SELECT band_type, ROUND(transit_size, 4) as transit_size,"
                      "ROUND(transit_time, 4) as transit_time,"
                      "ROUND(bandwidth, 4) as bandwidth_size,"
                      "ROUND(large_packet_ratio, 4)  as large_packet_ratio from "
                      + TABLE_COMM_ANALYZER_BANDWIDTH +
                      " WHERE step = ? AND rank_id = ? AND rank_set = ? AND hccl_op_name = ? ";
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
        Server::ServerLog::Error("Failed to prepare QueryDistributionData statement. error:", sqlite3_errmsg(db));
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
    std::string sql = "SELECT json_group_array (\"index\") FROM ( "
                      "SELECT DISTINCT \"index\" FROM" + TABLE_STEP_TRACE_TIME +
                      " WHERE \"index\" != '' )";
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
    std::vector<Protocol::Duration> &responseBody)
{
    std::vector<std::string> rankList = requestParams.rankList;
    std::string sql;
    if (rankList.empty()) {
        sql = "SELECT rank_id, ROUND(elapsed_time, 4) as elapse_time, ROUND(transit_time, 4) as transit_time, "
              "ROUND(synchronization_time, 4) as synchronization_time, ROUND(wait_time, 4) as wait_time, "
              "ROUND(idle_time, 4) as idle_time, ROUND(synchronization_time_ratio, 4) as synchronization_time_ratio, "
              "ROUND(wait_time_ratio, 4) as wait_time_ratio FROM " + TABLE_COMM_ANALYZER_TIME +
              " WHERE step = ? AND rank_set = ? AND hccl_op_name = ?";
    } else {
        std::string ranks = GetRanksSql(rankList);
        sql = "SELECT rank_id, ROUND(elapsed_time, 4) as elapse_time, ROUND(transit_time, 4) as transit_time, "
              "ROUND(synchronization_time, 4) as synchronization_time, ROUND(wait_time, 4) as wait_time, "
              "ROUND(idle_time, 4) as idle_time, ROUND(synchronization_time_ratio, 4) as synchronization_time_ratio, "
              "ROUND(wait_time_ratio, 4) as wait_time_ratio FROM " + TABLE_COMM_ANALYZER_TIME +
              " WHERE step = ? AND rank_set = ?"
              " AND rank_id IN " + ranks +
              " AND hccl_op_name = ?";
    }
    return ExecuteQueryDurationList(requestParams, responseBody, sql);
}

bool DbClusterDataBase::QueryCommunicationGroup(Document &responseBody)
{
    std::string baseInfoSql =
            "select (select rank_set from " + TABLE_COMM_GROUP + " where type == 'collection' ) as stages, "
            " (select rank_set from " + TABLE_COMM_GROUP + " where type == 'p2p' ) as pp_stages";
    return ExecuteQueryCommunicationGroup(responseBody, baseInfoSql);
}

bool DbClusterDataBase::QueryMatrixSortOpNames(Protocol::OperatorNamesParams &requestParams,
    std::vector<Protocol::OperatorNamesObject> &responseBody)
{
    std::string sql = "SELECT DISTINCT hccl_op_name  FROM " + TABLE_COMM_ANALYZER_MATRIX +
                      " WHERE step = ?" +
                      " AND rank_set = ?" +
                      " ORDER BY hccl_op_name";
    return ExecuteQueryMatrixSortOpNames(requestParams, responseBody, sql);
}
}
}
}