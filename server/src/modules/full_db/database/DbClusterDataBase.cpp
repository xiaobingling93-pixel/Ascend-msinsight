/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "DbClusterDataBase.h"
#include "TableDefs.h"

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
        stepCondition = " and step_id in (?";
        for (int i = 1; i < requestParams.stepIdList.size(); i++) {
            stepCondition.append(",?");
        }
        stepCondition.append(") ");
    }
    if (!requestParams.rankIdList.empty()) {
        rankCondition = " and rank_id in (?";
        for (int i = 1; i < requestParams.rankIdList.size(); i++) {
            rankCondition.append(",?");
        }
        rankCondition.append(") ");
    }
    std::string sql = "SELECT rank_id as rankId, sum(ROUND(compute_time,2)) as computingTime,"
                      "sum(ROUND(pure_communication_time,2)) as communicationNotOverLappedTime,"
                      "sum(ROUND(overlap_communication_time,2)) as communicationOverLappedTime,"
                      "sum(ROUND(free_time,2)) as freeTime FROM " + TABLE_STEP_TRACE_TIME +
                      " WHERE rank_id !='' " + stepCondition + rankCondition
                      + "group by rank_id order by " + requestParams.orderBy + " desc";
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
    std::string baseInfoSql =
            "select file_path as filePath,ranks,steps,data_size as dataSize from " + TABLE_BASE_INFO;
    return ExecuteQueryBaseInfo(responseBody, baseInfoSql);
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
                      "max(ROUND(stage_time, 4)) as stageTime, "
                      "max(ROUND(bubble_time, 4)) as bubbleTime "
                      "FROM " + TABLE_STEP_TRACE_TIME + " WHERE rank IN " + param.stageId + " AND step = ?";
    return ExecuteGetStageAndBubble(param, responseBody, sql);
}

bool DbClusterDataBase::GetRankAndBubble(Protocol::PipelineRankTimeParam param,
    Protocol::PipelineStageOrRankTimeResponseBody &responseBody)
{
    std::string sql = "SELECT rank_id as rankId, "
                      "ROUND(stage_time, 4) as stageTime, "
                      "ROUND(bubble_time, 4) as bubbleTime "
                      " FROM " + TABLE_STEP_TRACE_TIME +
                      " WHERE step_id = ? AND rank_id IN" + param.stageId + " ";
    return ExecuteGetRankAndBubble(param, responseBody, sql);
}

bool DbClusterDataBase::GetGroups(Protocol::MatrixGroupParam param, Protocol::MatrixGroupResponseBody &responseBody)
{
    std::string sql = "SELECT DISTINCT group_id as groupId FROM " + TABLE_COMM_GROUP;
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
                      "op_name as opName "
                      "FROM " + TABLE_COMM_ANALYZER_MATRIX +
                      " WHERE group_id = ? AND iteration_id = ? AND op_sort = ? ";
    return ExecuteQueryMatrixList(param, responseBody, sql);
}

bool DbClusterDataBase::QueryAllOperators(Protocol::OperatorDetailsParam &param,
    Protocol::OperatorDetailsResBody &resBody)
{
    std::string sql = "SELECT op_name as operatorName, "
                      " ROUND(elapse_time, 4) as elapseTime, "
                      " ROUND(transit_time, 4) as transitTime,"
                      " ROUND(synchronization_time, 4) as synchronizationTime,"
                      " ROUND(wait_time, 4) as waitTime,"
                      " ROUND(idle_time, 4) as idleTime,"
                      " ROUND(synchronization_time_ratio, 4) as synchronizationTimeRatio,"
                      " ROUND(wait_time_ratio, 4) as waitTimeRatio "
                      "FROM " + TABLE_COMM_ANALYZER_TIME +
                      " WHERE iteration_id = ? AND rank_id = ? AND stage_id = ?"
                      " AND op_name != 'Total Op Info' ";
    return ExecuteQueryAllOperators(param, resBody, sql);
}

bool DbClusterDataBase::QueryOperatorsCount(Protocol::OperatorDetailsParam &param,
    Protocol::OperatorDetailsResBody &resBody)
{
    std::string sql = "SELECT op_name, count(*) AS nums  from " + TABLE_COMM_ANALYZER_TIME + " where 1=1 ";
    return ExecuteQueryOperatorsCount(param, resBody, sql);
}

bool DbClusterDataBase::QueryBandwidthData(Protocol::BandwidthDataParam &param, Protocol::BandwidthDataResBody &resBody)
{
    std::string sql = "SELECT transport_type,ROUND(transit_size, 4) as transit_size,"
                      "ROUND(transit_time, 4) as transit_time,"
                      "ROUND(bandwidth_size, 4) as bandwidth_size,"
                      "ROUND(large_package_ratio, 4)  as large_package_ratio from "
                      + TABLE_COMM_ANALYZER_BANDWIDTH +
                      " WHERE iteration_id = ? AND rank_id = ? AND stage_id = ? AND op_name = ? ";
    return ExecuteQueryBandwidthData(param, resBody, sql);
}

bool DbClusterDataBase::QueryDistributionData(Protocol::DistributionDataParam &param,
    Protocol::DistributionResBody &resBody)
{
    std::string sql = "SELECT size_distribution FROM "
                      + TABLE_COMM_ANALYZER_BANDWIDTH +
                      " WHERE iteration_id = ? "
                      "AND rank_id = ? "
                      "AND stage_id = ? "
                      "AND op_name = ? "
                      "AND transport_type = ? ;";
    return ExecuteQueryDistributionData(param, resBody, sql);
}

bool DbClusterDataBase::QueryRanksHandler(std::vector<Protocol::IterationsOrRanksObject> &responseBody)
{
    std::string sql = "SELECT json_group_array (rank_id) FROM ( "
                      "SELECT DISTINCT rank_id FROM" + TABLE_STEP_TRACE_TIME +
                      " WHERE rank_id != '' )";
    return ExecuteQueryRanksHandler(responseBody, sql);
}

bool DbClusterDataBase::QueryOperatorNames(Protocol::OperatorNamesParams &requestParams,
    std::vector<Protocol::OperatorNamesObject> &responseBody)
{
    std::vector<std::string> rankList = requestParams.rankList;
    std::string sql;
    if (rankList.empty()) {
        sql = "SELECT DISTINCT op_name FROM (SELECT op_name FROM " + TABLE_COMM_ANALYZER_TIME +
              " WHERE iteration_id = ?" +
              " AND stage_id = ?" +
              " ORDER BY op_name)";
    } else {
        std::string ranks = GetRanksSql(rankList);
        sql = "SELECT DISTINCT op_name FROM (SELECT op_name FROM " + TABLE_COMM_ANALYZER_TIME +
              " WHERE iteration_id = ?" +
              " AND stage_id = ?" +
              " AND rank_id IN " + ranks + " ORDER BY op_name)";
    }
    return ExecuteQueryOperatorNames(requestParams, responseBody, sql);
}

bool DbClusterDataBase::QueryIterations(std::vector<Protocol::IterationsOrRanksObject> &responseBody)
{
    std::string sql = "select json_group_array(step_id) from ("
                      "select DISTINCT step_id from "+ TABLE_STEP_TRACE_TIME +
                      " where rank_id !='')";
    return ExecuteQueryIterations(responseBody, sql);
}

bool DbClusterDataBase::QueryDurationList(Protocol::DurationListParams &requestParams,
    std::vector<Protocol::Duration> &responseBody)
{
    std::vector<std::string> rankList = requestParams.rankList;
    std::string sql;
    if (rankList.empty()) {
        sql = "SELECT rank_id, ROUND(elapse_time, 4) as elapse_time, ROUND(transit_time, 4) as transit_time, "
              "ROUND(synchronization_time, 4) as synchronization_time, ROUND(wait_time, 4) as wait_time, "
              "ROUND(idle_time, 4) as idle_time, ROUND(synchronization_time_ratio, 4) as synchronization_time_ratio, "
              "ROUND(wait_time_ratio, 4) as wait_time_ratio FROM " + TABLE_TIME_INFO +
              " WHERE iteration_id = ? AND stage_id = ? AND op_name = ?";
    } else {
        std::string ranks = GetRanksSql(rankList);
        sql = "SELECT rank_id, ROUND(elapse_time, 4) as elapse_time, ROUND(transit_time, 4) as transit_time, "
              "ROUND(synchronization_time, 4) as synchronization_time, ROUND(wait_time, 4) as wait_time, "
              "ROUND(idle_time, 4) as idle_time, ROUND(synchronization_time_ratio, 4) as synchronization_time_ratio, "
              "ROUND(wait_time_ratio, 4) as wait_time_ratio FROM " + TABLE_TIME_INFO +
              " WHERE iteration_id = ? AND stage_id = ?"
              " AND rank_id IN " + ranks +
              " AND op_name = ?";
    }
    return ExecuteQueryDurationList(requestParams, responseBody, sql);
}

bool DbClusterDataBase::QueryCommunicationGroup(Document &responseBody)
{
    std::string baseInfoSql =
            "select (select rank_set from " + TABLE_COMM_GROUP + " where type == collection ) as stages, "
            " (select rank_set from " + TABLE_COMM_GROUP + " where type == p2p ) as pp_stages";
    return ExecuteQueryCommunicationGroup(responseBody, baseInfoSql);
}

bool DbClusterDataBase::QueryMatrixSortOpNames(Protocol::OperatorNamesParams &requestParams,
    std::vector<Protocol::OperatorNamesObject> &responseBody)
{
    std::string sql = "SELECT DISTINCT op_sort  FROM " + TABLE_COMM_ANALYZER_MATRIX +
                      " WHERE iteration_id = ?" +
                      " AND group_id = ?" +
                      " ORDER BY op_sort";
    return ExecuteQueryMatrixSortOpNames(requestParams, responseBody, sql);
}
}
}
}