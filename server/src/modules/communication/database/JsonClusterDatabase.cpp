/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */

#include "pch.h"
#include "SummaryProtocolResponse.h"
#include "TimelineProtocolResponse.h"
#include "TableDefs.h"
#include "NumDefs.h"
#include "TraceTime.h"
#include "JsonClusterDatabase.h"

namespace Dic {
namespace Module {
using namespace Server;
using namespace rapidjson;
JsonClusterDatabase::~JsonClusterDatabase()
{
    SaveLastData();
    if (isInitStmt) {
        ReleaseStmt();
    }
}

bool JsonClusterDatabase::SetConfig()
{
    if (!isOpen) {
        ServerLog::Error("Failed to set config. Database is not open.");
        return false;
    }
    return ExecSql("PRAGMA synchronous = OFF; PRAGMA journal_mode = MEMORY;");
}

bool JsonClusterDatabase::SetDbVersion()
{
    if (!isOpen) {
        ServerLog::Error("Failed to set db version. Database is not open.");
        return false;
    }
    std::string dbVersion = GetDataBaseVersion();
    return ExecSql(" PRAGMA user_version = " + dbVersion + ";");
}

bool JsonClusterDatabase::CreateTable()
{
    if (!isOpen) {
        ServerLog::Error("Failed to set config. Cluster Database is not open.");
        return false;
    }
    std::string sql =
            "CREATE TABLE " + TABLE_TIME_INFO +
            " (id INTEGER PRIMARY KEY AUTOINCREMENT, iteration_id VARCHAR(50),"
            " stage_id VARCHAR(200), rank_id VARCHAR(50), op_name VARCHAR(100),"
            " op_suffix VARCHAR(100), start_time integer, elapse_time double, synchronization_time_ratio double, "
            "synchronization_time double, transit_time double, wait_time_ratio double, "
            "wait_time double, idle_time double);"
            "CREATE TABLE " + TABLE_BANDWIDTH +
            " (id INTEGER PRIMARY KEY AUTOINCREMENT, iteration_id VARCHAR(50), "
            "stage_id VARCHAR(200), rank_id VARCHAR(50), op_name VARCHAR(100), "
            "op_suffix VARCHAR(100), transport_type VARCHAR(20), bandwidth_size double,"
            " bandwidth_utilization double, large_package_ratio double, size_distribution json,"
            " transit_size double, transit_time double);" +
            "CREATE TABLE " + TABLE_BASE_INFO +
            " (id INTEGER PRIMARY KEY AUTOINCREMENT, file_path VARCHAR(500), ranks json,"
            " steps json, collect_start_time DATETIME, collect_duration double, data_size double, stages json,"
            " pp_stages json, parse_status VARCHAR(10)); "
            "CREATE TABLE " + TABLE_STEP_TRACE +
            "(id INTEGER PRIMARY KEY AUTOINCREMENT, rank_id VARCHAR(50), step_id VARCHAR(50),"
            " stage_id VARCHAR(50), compute_time double, pure_communication_time double, "
            "overlap_communication_time double, communication_time double, free_time double, "
            "stage_time double, bubble_time double, pure_communication_exclude_receive_time double, preparing double); "
            "CREATE TABLE " + TABLE_COMMUNICATION_MATRIX +
            "(id INTEGER PRIMARY KEY AUTOINCREMENT, group_id VARCHAR(100), iteration_id VARCHAR(50), "
            "op_name VARCHAR(100),op_sort VARCHAR(100), group_name VARCHAR(100), src_rank VARCHAR(50), "
            "dst_rank VARCHAR(50), "
            "transport_type VARCHAR(50), transit_size double, transit_time double, bandwidth double);" +
            "CREATE TABLE " + TABLE_GROUP_ID +
            "(id INTEGER PRIMARY KEY AUTOINCREMENT, group_id VARCHAR(100));";
    return ExecSql(sql);
}

bool JsonClusterDatabase::CreateIndex()
{
    if (!isOpen) {
        ServerLog::Error("Failed to set config. Cluster Database is not open.");
        return false;
    }
    std::string sql = "CREATE INDEX IF NOT EXISTS idx3 on communication_matrix(group_id, op_sort);";
    return ExecSql(sql);
}

bool JsonClusterDatabase::CreateTimeIndex()
{
    if (!isOpen) {
        ServerLog::Error("Failed to set config. Cluster Database is not open.");
        return false;
    }
    std::string sql = "CREATE INDEX IF NOT EXISTS idx1 on communication_time_info(stage_id);"
                      "CREATE INDEX IF NOT EXISTS idx2 on communication_bandwidth_info(op_name);";
    return ExecSql(sql);
}

bool JsonClusterDatabase::InitStmt()
{
    if (isInitStmt) {
        return true;
    }
    std::string timeInfoSql = GetTimeInfoStmtSql(TABLE_CACHE_SIZE);
    if (sqlite3_prepare_v2(db, timeInfoSql.c_str(), -1, &insertTimeInfoStmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare getting TimeInfoStmtSql statement. error:", sqlite3_errmsg(db));
        return false;
    }
    std::string bandSql = GetBandwidthStmtSql(TABLE_CACHE_SIZE);
    if (sqlite3_prepare_v2(db, bandSql.c_str(), -1, &insertBandwidthStmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare inserting bandwidth statement. error:", sqlite3_errmsg(db));
        return false;
    }
    std::string matrixSql = GetMatrixStmtSql(TABLE_CACHE_SIZE);
    if (sqlite3_prepare_v2(db, matrixSql.c_str(), -1, &matrixStmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare getting MatrixStmtSql statement. error:", sqlite3_errmsg(db));
        return false;
    }
    isInitStmt = true;
    return true;
}

std::string JsonClusterDatabase::GetTimeInfoStmtSql(int len)
{
    std::string sql = "INSERT INTO " + TABLE_TIME_INFO +
                      " (iteration_id, stage_id, rank_id, op_name, op_suffix, start_time, elapse_time,"
                      " synchronization_time_ratio,"
                      "synchronization_time, transit_time, wait_time_ratio, wait_time, idle_time )"
                      " VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?)";
    for (int i = 0; i < len - 1; i++) {
        sql.append(",(?,?,?,?,?,?,?,?,?,?,?,?,?)");
    }
    return sql;
}

std::string JsonClusterDatabase::GetBandwidthStmtSql(int len)
{
    std::string sql = "INSERT INTO " + TABLE_BANDWIDTH +
                      " (iteration_id, stage_id, rank_id, op_name, op_suffix, transport_type,"
                      " bandwidth_size, bandwidth_utilization,large_package_ratio, size_distribution,"
                      " transit_size, transit_time) VALUES (?,?,?,?,?,?,?,?,?,?,?,?)";
    for (int i = 0; i < len - 1; i++) {
        sql.append(",(?,?,?,?,?,?,?,?,?,?,?,?)");
    }
    return sql;
}

void JsonClusterDatabase::ReleaseStmt()
{
    if (insertTimeInfoStmt != nullptr) {
        sqlite3_finalize(insertTimeInfoStmt);
    }
    if (insertBandwidthStmt != nullptr) {
        sqlite3_finalize(insertBandwidthStmt);
    }
    if (matrixStmt != nullptr) {
        sqlite3_finalize(matrixStmt);
    }
    if (stepStmt != nullptr) {
        sqlite3_finalize(stepStmt);
    }
}

void JsonClusterDatabase::SaveLastData()
{
    ServerLog::Info("Save_Last_Data ");
    if (!timeInfoCache.empty()) {
        InsertTimeInfoList(timeInfoCache);
        timeInfoCache.clear();
    }
    if (!bandwidthCache.empty()) {
        InsertBandwidthList(bandwidthCache);
        bandwidthCache.clear();
    }
    if (!matrixCache.empty()) {
        InsertCommunicationMatrixInfo(matrixCache);
        matrixCache.clear();
    }
}

void JsonClusterDatabase::InsertTimeInfo(CommunicationTimeInfo &timeInfo)
{
    timeInfoCache.emplace_back(timeInfo);
    if (timeInfoCache.size() == TABLE_CACHE_SIZE) {
        InsertTimeInfoList(timeInfoCache);
        timeInfoCache.clear();
    }
}

void JsonClusterDatabase::InsertTimeInfoList(std::vector<CommunicationTimeInfo> &timeInfoList)
{
    if (timeInfoList.empty()) {
        return;
    }
    sqlite3_stmt* stmt = nullptr;
    if (timeInfoList.size() == TABLE_CACHE_SIZE && isInitStmt) {
        sqlite3_reset(insertTimeInfoStmt);
        stmt = insertTimeInfoStmt;
    } else {
        std::string sql = GetTimeInfoStmtSql(timeInfoList.size());
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            ServerLog::Error("Failed to prepare Inserting TimeInfoList statement. error:", sqlite3_errmsg(db));
            return;
        }
    }
    if (stmt == nullptr) {
        ServerLog::Error("Failed to get timeInfo insert stmt.");
        return;
    }
    int idx = bindStartIndex;
    for (const auto &timeInfo: timeInfoList) {
        sqlite3_bind_text(stmt, idx++, timeInfo.iterationId.c_str(), timeInfo.iterationId.length(),
                          SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, idx++, timeInfo.stageId.c_str(), timeInfo.stageId.length(),
                          SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, idx++, timeInfo.rankId.c_str(), timeInfo.rankId.length(), SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, idx++, timeInfo.opName.c_str(), timeInfo.opName.length(), SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, idx++, timeInfo.opSuffix.c_str(), timeInfo.opSuffix.length(),
                          SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, idx++, timeInfo.startTime);
        sqlite3_bind_double(stmt, idx++, timeInfo.elapseTime);
        sqlite3_bind_double(stmt, idx++, timeInfo.synchronizationTimeRatio);
        sqlite3_bind_double(stmt, idx++, timeInfo.synchronizationTime);
        sqlite3_bind_double(stmt, idx++, timeInfo.transitTime);
        sqlite3_bind_double(stmt, idx++, timeInfo.waitTimeRatio);
        sqlite3_bind_double(stmt, idx++, timeInfo.waitTime);
        sqlite3_bind_double(stmt, idx++, timeInfo.idleTime);
    }

    auto result = sqlite3_step(stmt);
    if (timeInfoList.size() != TABLE_CACHE_SIZE) {
        sqlite3_finalize(stmt);
    }
    if (result != SQLITE_DONE) {
        ServerLog::Error("Insert timeInfo data fail. ", sqlite3_errmsg(db));
    }
}

void JsonClusterDatabase::InsertBandwidth(CommunicationBandWidth &bandWidth)
{
    bandwidthCache.emplace_back(bandWidth);
    if (bandwidthCache.size() == TABLE_CACHE_SIZE) {
        InsertBandwidthList(bandwidthCache);
        bandwidthCache.clear();
    }
}

void JsonClusterDatabase::InsertGroupId(const std::unordered_map<std::string, int64_t> &groupIds)
{
    if (groupIds.empty()) {
        ServerLog::Error("Group id is empty");
        return;
    }
    sqlite3_stmt *stmt = nullptr;
    std::string sql = "INSERT INTO " + TABLE_GROUP_ID +
                      " (id, group_id) VALUES (?, ?)";
    for (size_t i = 0; i < groupIds.size() - 1; ++i) {
        sql.append(",(?, ?)");
    }
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare inserting GroupId statement. error:", sqlite3_errmsg(db));
        return;
    }
    int idx = bindStartIndex;
    for (const auto &groupId: groupIds) {
        sqlite3_bind_int64(stmt, idx++, groupId.second);
        sqlite3_bind_text(stmt, idx++, groupId.first.c_str(), groupId.first.length(),
                          SQLITE_TRANSIENT);
    }
    auto result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (result != SQLITE_DONE) {
        ServerLog::Error("Insert GroupId data fail. ", sqlite3_errmsg(db));
    }
}

void JsonClusterDatabase::InsertBandwidthList(std::vector<CommunicationBandWidth> &bandWidthList)
{
    if (bandWidthList.empty()) {
        return;
    }
    sqlite3_stmt* stmt = nullptr;
    if (bandWidthList.size() == TABLE_CACHE_SIZE && isInitStmt) {
        sqlite3_reset(insertBandwidthStmt);
        stmt = insertBandwidthStmt;
    } else {
        std::string sql = GetBandwidthStmtSql(bandWidthList.size());
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            ServerLog::Error("Failed to prepare inserting bandwidth statement. error:", sqlite3_errmsg(db));
            return;
        }
    }
    if (stmt == nullptr) {
        ServerLog::Error("Failed to get timeInfo stmt.");
        return;
    }
    int idx = bindStartIndex;
    for (const auto &bandwidth: bandWidthList) {
        sqlite3_bind_text(stmt, idx++, bandwidth.iterationId.c_str(), bandwidth.iterationId.length(),
                          SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, idx++, bandwidth.stageId.c_str(), bandwidth.stageId.length(),
                          SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, idx++, bandwidth.rankId.c_str(), bandwidth.rankId.length(),
                          SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, idx++, bandwidth.opName.c_str(), bandwidth.opName.length(),
                          SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, idx++, bandwidth.opSuffix.c_str(), bandwidth.opSuffix.length(),
                          SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, idx++, bandwidth.transportType.c_str(), bandwidth.transportType.length(),
                          SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, idx++, bandwidth.bandwidthSize);
        sqlite3_bind_double(stmt, idx++, bandwidth.bandwidthUtilization);
        sqlite3_bind_double(stmt, idx++, bandwidth.largePackageRatio);
        sqlite3_bind_text(stmt, idx++, bandwidth.sizeDistribution.c_str(),
                          bandwidth.sizeDistribution.length(), SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, idx++, bandwidth.transitSize);
        sqlite3_bind_double(stmt, idx++, bandwidth.transitTime);
    }
    auto result = sqlite3_step(stmt);
    if (bandWidthList.size() != TABLE_CACHE_SIZE) {
        sqlite3_finalize(stmt);
    }
    if (result != SQLITE_DONE) {
        ServerLog::Error("Insert bandwidth data fail. ", sqlite3_errmsg(db));
    }
}

void JsonClusterDatabase::InsertStepStatisticsInfo(StepStatistic &stepStatistic)
{
    if (stepStmt == nullptr) {
        std::string sql = "INSERT INTO " + TABLE_STEP_TRACE +
                          "(rank_id, step_id, stage_id, compute_time,pure_communication_time,"
                          " overlap_communication_time,communication_time, free_time, stage_time,"
                          " bubble_time,pure_communication_exclude_receive_time,preparing) "
                          " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stepStmt, nullptr) != SQLITE_OK) {
            ServerLog::Error("Failed to prepare stepTraceTable statement. error:", sqlite3_errmsg(db));
            return;
        }
        if (stepStmt == nullptr) {
            ServerLog::Error("Failed to get stepTraceTable stmt.");
            return;
        }
    } else {
        sqlite3_reset(stepStmt);
    }
    int idx = bindStartIndex;
    sqlite3_bind_text(stepStmt, idx++, stepStatistic.rankId.c_str(), stepStatistic.rankId.length(),
                      SQLITE_STATIC);
    sqlite3_bind_text(stepStmt, idx++, stepStatistic.stepId.c_str(), stepStatistic.stepId.length(),
                      SQLITE_STATIC);
    sqlite3_bind_text(stepStmt, idx++, stepStatistic.stageId.c_str(), stepStatistic.stageId.length(),
                      SQLITE_STATIC);
    sqlite3_bind_double(stepStmt, idx++, stepStatistic.computingTime);
    sqlite3_bind_double(stepStmt, idx++, stepStatistic.pureCommunicationTime);
    sqlite3_bind_double(stepStmt, idx++, stepStatistic.overlapCommunicationTime);
    sqlite3_bind_double(stepStmt, idx++, stepStatistic.communicationTime);
    sqlite3_bind_double(stepStmt, idx++, stepStatistic.freeTime);
    sqlite3_bind_double(stepStmt, idx++, stepStatistic.stageTime);
    sqlite3_bind_double(stepStmt, idx++, stepStatistic.bubbleTime);
    sqlite3_bind_double(stepStmt, idx++, stepStatistic.pureCommunicationExcludeReceiveTime);
    sqlite3_bind_double(stepStmt, idx, stepStatistic.prepareTime);
    auto result = sqlite3_step(stepStmt);
    if (result != SQLITE_DONE) {
        ServerLog::Error("Insert bandwidth data fail. ", sqlite3_errmsg(db));
    }
}

void JsonClusterDatabase::InsertClusterBaseInfo(ClusterBaseInfo &clusterBaseInfo)
{
    sqlite3_stmt *stmt;
    std::string sql = "INSERT INTO " + TABLE_BASE_INFO +
                      "(file_path, ranks, steps, collect_start_time,collect_duration,data_size,stages,pp_stages)"
                      " VALUES (?, (select json_group_array(rank_id) from "
                      "(select DISTINCT rank_id from step_statistic_info where rank_id !='')), "
                      "(select json_group_array(step_id) from"
                      " (select DISTINCT step_id from step_statistic_info where rank_id !='')) ,"
                      " ?, ?, ?, ?, ?)";
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare baseInfoTable statement. error:", sqlite3_errmsg(db));
        return;
    }
    if (stmt == nullptr) {
        ServerLog::Error("Failed to get baseInfoTable stmt.");
        return;
    }
    int idx = bindStartIndex;
    sqlite3_bind_text(stmt, idx++, clusterBaseInfo.filePath.c_str(), clusterBaseInfo.filePath.length(),
                      SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, idx++, clusterBaseInfo.collectStartTime);
    sqlite3_bind_double(stmt, idx++, clusterBaseInfo.collectDuration);
    sqlite3_bind_double(stmt, idx++, clusterBaseInfo.dataSize);
    sqlite3_bind_text(stmt, idx++, clusterBaseInfo.stages.c_str(), clusterBaseInfo.stages.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, idx, clusterBaseInfo.ppStages.c_str(),
                      clusterBaseInfo.ppStages.length(), SQLITE_TRANSIENT);
    auto result = sqlite3_step(stmt);
    if (result != SQLITE_DONE) {
        ServerLog::Error("Insert baseInfoTable data fail. ", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
}

void JsonClusterDatabase::InsertCommunicationMatrix(Dic::Module::CommunicationMatrixInfo &communicationMatrix)
{
    matrixCache.emplace_back(communicationMatrix);
    if (matrixCache.size() == TABLE_CACHE_SIZE) {
        InsertCommunicationMatrixInfo(matrixCache);
        matrixCache.clear();
    }
}

void JsonClusterDatabase::InsertCommunicationMatrixInfo(std::vector<CommunicationMatrixInfo> &communicationMatrixInfo)
{
    if (communicationMatrixInfo.empty()) {
        return;
    }
    sqlite3_stmt *stmt = nullptr;
    if (communicationMatrixInfo.size() == TABLE_CACHE_SIZE && isInitStmt) {
        sqlite3_reset(matrixStmt);
        stmt = matrixStmt;
    } else {
        std::string sql = GetMatrixStmtSql(communicationMatrixInfo.size());
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            ServerLog::Error("Failed to prepare matrix table statement. error:", sqlite3_errmsg(db));
            return;
        }
    }
    if (stmt == nullptr) {
        ServerLog::Error("Failed to get matrix stmt.");
        return;
    }
    int idx = bindStartIndex;
    for (const auto &communicationMatrix: communicationMatrixInfo) {
        sqlite3_bind_text(stmt, idx++, communicationMatrix.groupId.c_str(), communicationMatrix.groupId.length(),
                          SQLITE_STATIC);
        sqlite3_bind_text(stmt, idx++, communicationMatrix.iterationId.c_str(),
                          communicationMatrix.iterationId.length(), SQLITE_STATIC);
        sqlite3_bind_text(stmt, idx++, communicationMatrix.opName.c_str(), communicationMatrix.opName.length(),
                          SQLITE_STATIC);
        sqlite3_bind_text(stmt, idx++, communicationMatrix.sortOp.c_str(), communicationMatrix.sortOp.length(),
                          SQLITE_STATIC);
        sqlite3_bind_text(stmt, idx++, communicationMatrix.groupName.c_str(), communicationMatrix.groupName.length(),
                          SQLITE_STATIC);
        sqlite3_bind_int(stmt, idx++, communicationMatrix.srcRank);
        sqlite3_bind_int(stmt, idx++, communicationMatrix.dstRank);
        sqlite3_bind_text(stmt, idx++, communicationMatrix.transportType.c_str(),
                          communicationMatrix.transportType.length(), SQLITE_STATIC);
        sqlite3_bind_double(stmt, idx++, communicationMatrix.transitSize);
        sqlite3_bind_double(stmt, idx++, communicationMatrix.transitTime);
        sqlite3_bind_double(stmt, idx++, communicationMatrix.bandwidth);
    }
    auto result = sqlite3_step(stmt);
    if (communicationMatrixInfo.size() != TABLE_CACHE_SIZE) {
        sqlite3_finalize(stmt);
    }
    if (result != SQLITE_DONE) {
        ServerLog::Error("Insert matrix data fail. ", sqlite3_errmsg(db));
        return;
    }
}

std::string JsonClusterDatabase::GetMatrixStmtSql(int len)
{
    sqlite3_stmt *stmt = nullptr;
    std::string sql = "INSERT INTO " + TABLE_COMMUNICATION_MATRIX +
                      " (group_id, iteration_id, op_name, op_sort, group_name, src_rank, "
                      "dst_rank, transport_type, transit_size, transit_time, bandwidth) "
                      "VALUES (?,?,?,?,?,?,?,?,?,?,?)";
    for (int i = 0; i < len - 1; i++) {
        sql.append(",(?,?,?,?,?,?,?,?,?,?,?)");
    }
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare matrix table statement. error:", sqlite3_errmsg(db));
    }
    return sql;
}

bool JsonClusterDatabase::QuerySummaryData(const Protocol::SummaryTopRankParams &requestParams,
    Protocol::SummaryTopRankResBody &responseBody)
{
    std::string sql = BuildCondition(requestParams);
    return ExecuteQuerySummaryData(requestParams, responseBody, sql);
}

bool JsonClusterDatabase::QueryBaseInfo(Protocol::SummaryTopRankResBody &responseBody)
{
    std::string baseInfoSql = "select ranks,steps,data_size as dataSize from " + TABLE_BASE_INFO;
    return ExecuteQueryBaseInfo(responseBody, baseInfoSql);
}

bool JsonClusterDatabase::QueryCommunicationGroup(Document &responseBody)
{
    std::string baseInfoSql =
            "select stages, pp_stages from " + TABLE_BASE_INFO;
    return ExecuteQueryCommunicationGroup(responseBody, baseInfoSql);
}

std::string JsonClusterDatabase::QueryParseClusterStatus()
{
    sqlite3_stmt *stmtBaseInfo = nullptr;
    std::string baseInfoSql =
            "select parse_status from " + TABLE_BASE_INFO;
    int baseInfoResult = sqlite3_prepare_v2(db, baseInfoSql.c_str(), -1, &stmtBaseInfo, nullptr);
    if (baseInfoResult != SQLITE_OK) {
        ServerLog::Error("Query parse status statement failed to prepare sql.", sqlite3_errmsg(db));
        return "";
    }
    std::string parseStatus;
    while (sqlite3_step(stmtBaseInfo) == SQLITE_ROW) {
        int coll = resultStartIndex;
        parseStatus = sqlite3_column_string(stmtBaseInfo, coll++);
    }
    sqlite3_finalize(stmtBaseInfo);
    return parseStatus;
}

void JsonClusterDatabase::UpdateClusterParseStatus(std::string status)
{
    ServerLog::Info("Update_Cluster_Parse_Status status: ", status);
    sqlite3_stmt *stmtBaseInfo = nullptr;
    int index = bindStartIndex;
    std::string baseInfoSql =
            "update " + TABLE_BASE_INFO + " set parse_status=?";
    int baseInfoResult = sqlite3_prepare_v2(db, baseInfoSql.c_str(), -1, &stmtBaseInfo, nullptr);
    sqlite3_bind_text(stmtBaseInfo, index, status.c_str(), status.length(), SQLITE_TRANSIENT);
    if (baseInfoResult != SQLITE_OK) {
        ServerLog::Error("Update cluster parse_status statement failed to prepare sql.", sqlite3_errmsg(db));
        return;
    }
    auto result = sqlite3_step(stmtBaseInfo);
    sqlite3_finalize(stmtBaseInfo);
    if (result != SQLITE_DONE) {
        ServerLog::Error("Fail to update clusterParseStatus. ", sqlite3_errmsg(db));
    }
}

bool JsonClusterDatabase::GetStepIdList(Protocol::PipelineStepResponseBody &responseBody)
{
    std::string sql = "select distinct step_id as stepId "
                      "FROM " + TABLE_STEP_TRACE +
                      " ORDER BY step_id";
    return ExecuteGetStepIdList(responseBody, sql);
}

bool JsonClusterDatabase::GetStages(Protocol::PipelineStageParam param,
    Protocol::PipelineStageResponseBody &responseBody)
{
    std::string sql = "SELECT DISTINCT stage_id as stageId "
                      "FROM " + TABLE_STEP_TRACE + " WHERE stage_id != '' AND step_id = ?";
    return ExecuteGetStages(param, responseBody, sql);
}

bool JsonClusterDatabase::GetStageAndBubble(Protocol::PipelineStageTimeParam param,
    Protocol::PipelineStageOrRankTimeResponseBody &responseBody)
{
    std::string sql = "SELECT max(ROUND(stage_time, 4)) as stageTime, "
                      "max(ROUND(bubble_time, 4)) as bubbleTime "
                      "FROM " + TABLE_STEP_TRACE + " WHERE step_id = ?";

    std::vector<std::string> stageIds;
    PrepareForStageId(param.stageId, sql, stageIds);

    return ExecuteGetStageAndBubble(param, stageIds, responseBody, sql);
}

bool JsonClusterDatabase::GetRankAndBubble(Protocol::PipelineRankTimeParam param,
    Protocol::PipelineStageOrRankTimeResponseBody &responseBody)
{
    std::string sql = "SELECT rank_id as rankId, "
                      "ROUND(stage_time, 4) as stageTime, "
                      "ROUND(bubble_time, 4) as bubbleTime "
                      " FROM " + TABLE_STEP_TRACE +
                      " WHERE step_id = ? ";
    std::vector<std::string> stageIds;
    PrepareForStageId(param.stageId, sql, stageIds);

    return ExecuteGetRankAndBubble(param, stageIds, responseBody, sql);
}

bool JsonClusterDatabase::GetGroups(Protocol::MatrixGroupParam param, Protocol::MatrixGroupResponseBody &responseBody)
{
    std::string sql = "SELECT DISTINCT group_id as groupId "
                      "FROM " + TABLE_GROUP_ID;
    return ExecuteGetGroups(param, responseBody, sql);
}

std::unordered_map<std::string, int64_t> JsonClusterDatabase::GetAllGroupMap()
{
    std::string sql = "SELECT id, group_id FROM " + TABLE_GROUP_ID;
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Failed to Get Groups info. error: ", sqlite3_errmsg(db));
        return {};
    }
    std::unordered_map<std::string, int64_t> res;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        int64_t id = sqlite3_column_int64(stmt, col++);
        std::string groupId = sqlite3_column_string(stmt, col++);
        res.insert(std::make_pair(groupId, id));
    }
    sqlite3_finalize(stmt);
    return res;
}

bool JsonClusterDatabase::QueryMatrixList(Protocol::MatrixBandwidthParam param,
    Protocol::MatrixListResponseBody &responseBody)
{
    std::string sql = "SELECT src_rank as srcRank, dst_rank as dstRank, "
                      "transport_type as transportType, "
                      "ROUND(transit_size, 4) as transitSize, "
                      "ROUND(transit_time, 4) as transitTime, "
                      "ROUND(bandwidth, 4) as bandwidth ,"
                      "op_name as opName "
                      "FROM " + TABLE_COMMUNICATION_MATRIX + " CM"
                      " LEFT JOIN " + TABLE_GROUP_ID + " g ON cm.group_id = g.id"
                      " WHERE g.group_id = ? AND iteration_id = ? AND op_sort = ? ";
    return ExecuteQueryMatrixList(param, responseBody, sql);
}

std::string JsonClusterDatabase::BuildCondition(const Protocol::SummaryTopRankParams &requestParams)
{
    std::string stepCondition;
    std::string rankCondition;
    std::string prepareTimeCondition;
    if (HasColumn(TABLE_STEP_TRACE, "preparing")) {
        prepareTimeCondition = ",sum(ROUND(preparing,2)) as prepareTime ";
    }
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
                      "sum(ROUND(free_time,2)) as freeTime "
                      + prepareTimeCondition +
                      "FROM " + TABLE_STEP_TRACE +
                      " WHERE rank_id !='' " + stepCondition + rankCondition
                      + "group by rank_id ";
    if (!StringUtil::CheckSqlValid(requestParams.orderBy)) {
        ServerLog::Error("There is an SQL injection attack on this parameter. error param: ", requestParams.orderBy);
        return sql;
    }
    if (requestParams.orderBy == "rankId") {
        sql += " ORDER by CAST(rankId AS UNSIGNED) asc";
    } else {
        sql += " ORDER by " + requestParams.orderBy + " desc";
    }
    return sql;
}

bool JsonClusterDatabase::QueryExtremumTimestamp(uint64_t &min, uint64_t &max)
{
    std::string sql = "SELECT MIN(start_time) as minTime, MAX(start_time) as maxTime "
                      "FROM " + TABLE_TIME_INFO + " WHERE start_time != 0";
    return ExecuteQueryExtremumTimestamp(sql, min, max);
}

bool JsonClusterDatabase::QueryIterationAndCommunicationGroup(Protocol::KernelParams &params,
    Protocol::OneKernelBody &responseBody, uint64_t minTimestamp)
{
    std::string sql = "select iteration_id, tg.group_id as stage_id from " + TABLE_TIME_INFO + " t"
        " LEFT JOIN " + TABLE_GROUP_ID + " tg ON t.stage_id = tg.id"
        " where op_name = ? and abs(start_time - ? ) <= 500";
    uint64_t reallyStartTime = params.timestamp + minTimestamp;
    return ExecuteQueryIterationAndCommunicationGroup(sql, params.name, reallyStartTime, responseBody.step,
        responseBody.group);
}

bool JsonClusterDatabase::QueryAllOperators(Protocol::OperatorDetailsParam &param,
    Protocol::OperatorDetailsResBody &resBody)
{
    uint64_t startTime = Timeline::TraceTime::Instance().GetStartTime();
    std::string sql =
        "SELECT t.op_name as operatorName, "
        "CASE WHEN start_time = 0 THEN 0 ELSE ROUND((start_time - ?) / 1000000.0, 4) END as startTime, "
        "ROUND(elapse_time, 4) as elapseTime, "
        "ROUND(transit_time, 4) as transitTime, "
        "ROUND(synchronization_time, 4) as synchronizationTime, "
        "ROUND(wait_time, 4) as waitTime, "
        "ROUND(idle_time, 4) as idleTime, "
        "ROUND(synchronization_time_ratio, 4) as synchronizationTimeRatio, "
        "ROUND(wait_time_ratio, 4) as waitTimeRatio, "
        "bw.sdma_bw as sdma_bw, bw.rdma_bw as rdma_bw "
        "FROM " + TABLE_TIME_INFO + " t "
        "JOIN ( "
        "    SELECT op_name, "
        "    MAX(CASE WHEN transport_type = 'SDMA' THEN bandwidth_size ELSE 0 END) AS sdma_bw, "
        "    MAX(CASE WHEN transport_type = 'RDMA' THEN bandwidth_size ELSE 0 END) AS rdma_bw "
        "    FROM " + TABLE_BANDWIDTH + " b "
        "    LEFT JOIN " + TABLE_GROUP_ID + " g ON b.stage_id = g.id"
        "    WHERE iteration_id = ? AND rank_id = ? AND g.group_id = ? AND op_name != 'Total Op Info' "
        "    GROUP BY op_name "
        ") bw ON t.op_name = bw.op_name "
        " LEFT JOIN " + TABLE_GROUP_ID + " tg ON t.stage_id = tg.id"
        " WHERE t.iteration_id = ? AND t.rank_id = ? AND tg.group_id = ? AND t.op_name != 'Total Op Info'";
    return ExecuteQueryAllOperators(param, resBody, sql, startTime);
}

bool JsonClusterDatabase::QueryOperatorsCount(Protocol::OperatorDetailsParam &param,
    Protocol::OperatorDetailsResBody &resBody)
{
    std::string sql = "SELECT op_name, count(*) AS nums  from "
    " (SELECT t.op_name as op_name, g.group_id as stage_id, t.iteration_id as iteration_id, t.rank_id as rank_id FROM "
    + TABLE_TIME_INFO + " t"
    " LEFT JOIN " + TABLE_GROUP_ID + " g ON t.stage_id = g.id)"
    " where 1=1 ";
    return ExecuteQueryOperatorsCount(param, resBody, sql);
}

bool JsonClusterDatabase::QueryBandwidthData(Protocol::BandwidthDataParam &param,
    Protocol::BandwidthDataResBody &resBody)
{
    std::string sql = "SELECT transport_type,ROUND(transit_size, 4) as transit_size,"
                      "ROUND(transit_time, 4) as transit_time,"
                      "ROUND(bandwidth_size, 4) as bandwidth_size,"
                      "ROUND(large_package_ratio, 4)  as large_package_ratio from "
                      + TABLE_BANDWIDTH + " b "
                      " LEFT JOIN " + TABLE_GROUP_ID + " g ON b.stage_id = g.id"
                      " WHERE iteration_id = ? AND rank_id = ? AND g.group_id = ? AND op_name = ? ";
    return ExecuteQueryBandwidthData(param, resBody, sql);
}

bool JsonClusterDatabase::QueryDistributionData(Protocol::DistributionDataParam &param,
    Protocol::DistributionResBody &resBody)
{
    std::string sql = "SELECT size_distribution FROM "
                      + TABLE_BANDWIDTH + " b"
                      " LEFT JOIN " + TABLE_GROUP_ID + " g ON b.stage_id = g.id"
                      " WHERE iteration_id = ? "
                      "AND rank_id = ? "
                      "AND g.group_id = ? "
                      "AND op_name = ? "
                      "AND transport_type = ? ;";
    return ExecuteQueryDistributionData(param, resBody, sql);
}

bool JsonClusterDatabase::QueryRanksHandler(std::vector<Protocol::IterationsOrRanksObject> &responseBody)
{
    std::string sql = "SELECT ranks FROM " + TABLE_BASE_INFO;
    return ExecuteQueryRanksHandler(responseBody, sql);
}

bool JsonClusterDatabase::QueryOperatorNames(Protocol::OperatorNamesParams &requestParams,
    std::vector<Protocol::OperatorNamesObject> &responseBody)
{
    std::vector<std::string> rankList = requestParams.rankList;
    std::string sql;
    if (rankList.empty()) {
        sql = "SELECT DISTINCT op_name FROM (SELECT op_name FROM " + TABLE_TIME_INFO + " t"
                " LEFT JOIN " + TABLE_GROUP_ID + " tg ON t.stage_id = tg.id"
                " WHERE iteration_id = ?" +
                " AND tg.group_id = ?" +
                " ORDER BY op_name)";
    } else {
        std::string ranks = GetRanksSql(rankList);
        sql = "SELECT DISTINCT op_name FROM (SELECT op_name FROM " + TABLE_TIME_INFO + " t"
                " LEFT JOIN " + TABLE_GROUP_ID + " tg ON t.stage_id = tg.id"
                " WHERE iteration_id = ?" +
                " AND tg.group_id = ?" +
                " AND rank_id IN " + ranks + " ORDER BY op_name)";
    }
    return ExecuteQueryOperatorNames(requestParams, responseBody, sql);
}

bool JsonClusterDatabase::QueryMatrixSortOpNames(Protocol::OperatorNamesParams &requestParams,
    std::vector<Protocol::OperatorNamesObject> &responseBody)
{
    std::string sql = "SELECT DISTINCT op_sort  FROM " + TABLE_COMMUNICATION_MATRIX + " cm"
            " LEFT JOIN " + TABLE_GROUP_ID + " g ON cm.group_id = g.id"
            " WHERE iteration_id = ?" +
            " AND g.group_id = ?" +
            " ORDER BY op_sort";
    return ExecuteQueryMatrixSortOpNames(requestParams, responseBody, sql);
}

bool JsonClusterDatabase::QueryIterations(std::vector<Protocol::IterationsOrRanksObject> &responseBody)
{
    std::string sql = "SELECT steps FROM " + TABLE_BASE_INFO;
    return ExecuteQueryIterations(responseBody, sql);
}

bool JsonClusterDatabase::QueryDurationList(Protocol::DurationListParams &requestParams,
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
    std::string sql =
        "SELECT t.rank_id as rank_id, "
        "CASE WHEN start_time = 0 THEN 0 ELSE ROUND((start_time - ?) / 1000000.0, 4) END as startTime, "
        "ROUND(elapse_time, 4) as elapse_time, ROUND(transit_time, 4) as transit_time, "
        "ROUND(synchronization_time, 4) as synchronization_time, ROUND(wait_time, 4) as wait_time, "
        "ROUND(idle_time, 4) as idle_time, ROUND(synchronization_time_ratio, 4) as synchronization_time_ratio, "
        "ROUND(wait_time_ratio, 4) as wait_time_ratio, "
        "bw.sdma_bw as sdma_bw, bw.rdma_bw as rdma_bw, bw.sdma_time as sdma_time, bw.rdma_time as rdma_time "
        "FROM " + TABLE_TIME_INFO + " t "
        "JOIN ( "
        "    SELECT rank_id, "
        "    MAX(CASE WHEN transport_type = 'SDMA' THEN bandwidth_size ELSE 0 END) AS sdma_bw, "
        "    MAX(CASE WHEN transport_type = 'RDMA' THEN bandwidth_size ELSE 0 END) AS rdma_bw, "
        "    MAX(CASE WHEN transport_type = 'SDMA' THEN transit_time ELSE 0 END) AS sdma_time, "
        "    MAX(CASE WHEN transport_type = 'RDMA' THEN transit_time ELSE 0 END) AS rdma_time "
        "    FROM " + TABLE_BANDWIDTH + " b "
        "    LEFT JOIN " + TABLE_GROUP_ID + " g ON b.stage_id = g.id"
        "    WHERE iteration_id = ? AND g.group_id = ? AND op_name = ? " + rankSql +
        "    GROUP BY rank_id "
        ") bw ON t.rank_id = bw.rank_id"
        " LEFT JOIN " + TABLE_GROUP_ID + " tg ON t.stage_id = tg.id"
        " WHERE t.iteration_id = ? AND tg.group_id = ? AND t.op_name = ? " + rankSqlTime;
    return ExecuteQueryDurationList(requestParams, responseBody, sql, startTime);
}

bool JsonClusterDatabase::QueryOperatorList(Protocol::DurationListParams &requestParams,
    Protocol::OperatorListsResponseBody &responseBody)
{
    std::string sql =
        "SELECT rank_id, op_name,"
        " CASE WHEN start_time = 0 THEN 0 ELSE (start_time - ?) END as startTime,"
        " (elapse_time * 1000000) as elapse_time From " + TABLE_TIME_INFO + " T"
        " LEFT JOIN " + TABLE_GROUP_ID + " tg ON t.stage_id = tg.id"
        " WHERE iteration_id = ? AND tg.group_id = ? AND op_name <> 'Total Op Info'";
    std::vector<std::string> rankList = requestParams.rankList;
    if (!rankList.empty()) {
        std::string ranks = GetRanksSql(rankList);
        sql += " AND rank_id IN " + ranks;
    }
    if (requestParams.operatorName != totalOpInfo) {
        sql += " AND op_name = ?";
    }
    sql += " ORDER by CAST(rank_id AS UNSIGNED) ASC";
    uint64_t startTime = Module::Timeline::TraceTime::Instance().GetStartTime();
    return ExecuteQueryOperatorList(requestParams, responseBody, sql, startTime);
}

void JsonClusterDatabase::PrepareForStageId(std::string &stageIdStr, std::string &sql,
                                            std::vector<std::string> &stageIds)
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
    for (int i = 0; i < stageIds.size(); i++) {
        if (i == 0) {
            rankSql.append("?");
        }
        rankSql.append(",?");
    }
    if (!rankSql.empty()) {
        sql += "AND rank_id IN (" + rankSql + ")";
    }
}
} // end of namespace Module
} // end of namespace Dic