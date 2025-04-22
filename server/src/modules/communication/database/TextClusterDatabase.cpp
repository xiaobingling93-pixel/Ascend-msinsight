/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */

#include "pch.h"
#include "SummaryProtocolResponse.h"
#include "TimelineProtocolResponse.h"
#include "TableDefs.h"
#include "NumDefs.h"
#include "TraceTime.h"
#include "TextClusterDatabase.h"
// LCOV_EXCL_BR_START
namespace Dic {
namespace Module {
using namespace Server;
using namespace rapidjson;
TextClusterDatabase::~TextClusterDatabase()
{
    SaveLastData();
    if (isInitStmt) {
        ReleaseStmt();
    }
}

bool TextClusterDatabase::SetConfig()
{
    return Database::SetConfig();
}

bool TextClusterDatabase::SetDbVersion()
{
    if (!isOpen) {
        ServerLog::Error("Failed to set db version. Database is not open.");
        return false;
    }
    std::string dbVersion = GetDataBaseVersion();
    return ExecSql(" PRAGMA user_version = " + dbVersion + ";");
}

bool TextClusterDatabase::CreateTable()
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
            " (key VARCHAR(50) PRIMARY KEY, value TEXT); "
            "CREATE TABLE " + TABLE_STEP_TRACE +
            "(id INTEGER PRIMARY KEY AUTOINCREMENT, rank_id VARCHAR(50), step_id VARCHAR(50),"
            " stage_id VARCHAR(50), compute_time double, pure_communication_time double, "
            "overlap_communication_time double, communication_time double, free_time double, "
            "stage_time double, bubble_time double, pure_communication_exclude_receive_time double, preparing double, "
            "dp_index INTEGER, pp_index INTEGER, tp_index INTEGER); "
            "CREATE TABLE " + TABLE_COMMUNICATION_MATRIX +
            "(id INTEGER PRIMARY KEY AUTOINCREMENT, group_id VARCHAR(100), iteration_id VARCHAR(50), "
            "op_name VARCHAR(100),op_sort VARCHAR(100), group_name VARCHAR(100), src_rank VARCHAR(50), "
            "dst_rank VARCHAR(50), "
            "transport_type VARCHAR(50), transit_size double, transit_time double, bandwidth double);" +
            "CREATE TABLE " + TABLE_GROUP_ID +
            "(id INTEGER PRIMARY KEY AUTOINCREMENT, group_id VARCHAR(100));";
    sql += commonSql;
    return ExecSql(sql);
}

bool TextClusterDatabase::CreateIndex()
{
    if (!isOpen) {
        ServerLog::Error("Failed to set config. Cluster Database is not open.");
        return false;
    }
    std::string sql = "CREATE INDEX IF NOT EXISTS idx3 on communication_matrix(group_id, op_sort);";
    return ExecSql(sql);
}

bool TextClusterDatabase::CreateTimeIndex()
{
    if (!isOpen) {
        ServerLog::Error("Failed to set config. Cluster Database is not open.");
        return false;
    }
    std::string sql = "CREATE INDEX IF NOT EXISTS idx1 on communication_time_info(stage_id);"
                      "CREATE INDEX IF NOT EXISTS idx2 on communication_bandwidth_info(op_name);";
    return ExecSql(sql);
}

bool TextClusterDatabase::InitStmt()
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

std::string TextClusterDatabase::GetTimeInfoStmtSql(int len)
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

std::string TextClusterDatabase::GetBandwidthStmtSql(int len)
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

void TextClusterDatabase::ReleaseStmt()
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

void TextClusterDatabase::SaveLastData()
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

void TextClusterDatabase::InsertTimeInfo(CommunicationTimeInfo &timeInfo)
{
    timeInfoCache.emplace_back(timeInfo);
    if (timeInfoCache.size() == TABLE_CACHE_SIZE) {
        InsertTimeInfoList(timeInfoCache);
        timeInfoCache.clear();
    }
}

void TextClusterDatabase::InsertTimeInfoList(std::vector<CommunicationTimeInfo> &timeInfoList)
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
        sqlite3_bind_int64(stmt, idx++, NumberUtil::CeilingClamp(timeInfo.startTime, static_cast<uint64_t>(INT64_MAX)));
        sqlite3_bind_double(stmt, idx++, timeInfo.elapseTime);
        sqlite3_bind_double(stmt, idx++, timeInfo.synchronizationTimeRatio);
        sqlite3_bind_double(stmt, idx++, timeInfo.synchronizationTime);
        sqlite3_bind_double(stmt, idx++, timeInfo.transitTime);
        sqlite3_bind_double(stmt, idx++, timeInfo.waitTimeRatio);
        sqlite3_bind_double(stmt, idx++, timeInfo.waitTime);
        sqlite3_bind_double(stmt, idx++, timeInfo.idleTime);
    }

    auto result = sqlite3_step(stmt);
    if (!(timeInfoList.size() == TABLE_CACHE_SIZE && isInitStmt)) {
        sqlite3_finalize(stmt);
    }
    if (result != SQLITE_DONE) {
        ServerLog::Error("Insert timeInfo data fail. ", sqlite3_errmsg(db));
    }
}

void TextClusterDatabase::InsertBandwidth(CommunicationBandWidth &bandWidth)
{
    bandwidthCache.emplace_back(bandWidth);
    if (bandwidthCache.size() == TABLE_CACHE_SIZE) {
        InsertBandwidthList(bandwidthCache);
        bandwidthCache.clear();
    }
}

void TextClusterDatabase::InsertGroupId(const std::unordered_map<std::string, int64_t> &groupIds)
{
    if (groupIds.empty()) {
        ServerLog::Error("Group id is empty");
        return;
    }
    sqlite3_stmt *stmt = nullptr;
    std::string sql = "INSERT INTO " + TABLE_GROUP_ID + " (id, group_id) VALUES (?, ?)";
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
        sqlite3_bind_text(stmt, idx++, groupId.first.c_str(), groupId.first.length(), SQLITE_TRANSIENT);
    }
    auto result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (result != SQLITE_DONE) {
        ServerLog::Error("Insert GroupId data fail. ", sqlite3_errmsg(db));
    }
}

void TextClusterDatabase::InsertBandwidthList(std::vector<CommunicationBandWidth> &bandWidthList)
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
        sqlite3_bind_text(stmt, idx++, bandwidth.iterationId.c_str(), bandwidth.iterationId.length(), SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, idx++, bandwidth.stageId.c_str(), bandwidth.stageId.length(), SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, idx++, bandwidth.rankId.c_str(), bandwidth.rankId.length(), SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, idx++, bandwidth.opName.c_str(), bandwidth.opName.length(), SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, idx++, bandwidth.opSuffix.c_str(), bandwidth.opSuffix.length(), SQLITE_TRANSIENT);
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
    if (!(bandWidthList.size() == TABLE_CACHE_SIZE && isInitStmt)) {
        sqlite3_finalize(stmt);
    }
    if (result != SQLITE_DONE) {
        ServerLog::Error("Insert bandwidth data fail. ", sqlite3_errmsg(db));
    }
}

void TextClusterDatabase::InsertStepStatisticsInfo(StepStatistic &stepStatistic)
{
    if (stepStmt == nullptr) {
        std::string sql = "INSERT INTO " + TABLE_STEP_TRACE +
            "(rank_id, step_id, stage_id, compute_time, pure_communication_time, overlap_communication_time, "
            "communication_time, free_time, stage_time, bubble_time, pure_communication_exclude_receive_time, "
            "preparing, dp_index, pp_index, tp_index) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
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
    sqlite3_bind_text(stepStmt, idx++, stepStatistic.rankId.c_str(), stepStatistic.rankId.length(), SQLITE_STATIC);
    sqlite3_bind_text(stepStmt, idx++, stepStatistic.stepId.c_str(), stepStatistic.stepId.length(), SQLITE_STATIC);
    sqlite3_bind_text(stepStmt, idx++, stepStatistic.stageId.c_str(), stepStatistic.stageId.length(), SQLITE_STATIC);
    sqlite3_bind_double(stepStmt, idx++, stepStatistic.computingTime);
    sqlite3_bind_double(stepStmt, idx++, stepStatistic.pureCommunicationTime);
    sqlite3_bind_double(stepStmt, idx++, stepStatistic.overlapCommunicationTime);
    sqlite3_bind_double(stepStmt, idx++, stepStatistic.communicationTime);
    sqlite3_bind_double(stepStmt, idx++, stepStatistic.freeTime);
    sqlite3_bind_double(stepStmt, idx++, stepStatistic.stageTime);
    sqlite3_bind_double(stepStmt, idx++, stepStatistic.bubbleTime);
    sqlite3_bind_double(stepStmt, idx++, stepStatistic.pureCommunicationExcludeReceiveTime);
    sqlite3_bind_double(stepStmt, idx++, stepStatistic.prepareTime);
    sqlite3_bind_int64(stepStmt, idx++, stepStatistic.dpIndex);
    sqlite3_bind_int64(stepStmt, idx++, stepStatistic.ppIndex);
    sqlite3_bind_int64(stepStmt, idx++, stepStatistic.tpIndex);
    auto result = sqlite3_step(stepStmt);
    if (result != SQLITE_DONE) {
        ServerLog::Error("Failed to insert step trace data. ", sqlite3_errmsg(db));
    }
}

void TextClusterDatabase::InsertClusterBaseInfo(ClusterBaseInfo &baseInfo)
{
    sqlite3_stmt *stmt;
    std::string sql = "INSERT INTO " + TABLE_BASE_INFO +
                      " (key, value) VALUES ('file_path', ?), "
                      " ('ranks', (SELECT json_group_array(rank_id) FROM "
                      " (SELECT DISTINCT rank_id FROM step_statistic_info WHERE rank_id !=''))), "
                      " ('steps', (SELECT json_group_array(step_id) FROM "
                      " (SELECT DISTINCT step_id FROM step_statistic_info WHERE rank_id !=''))), "
                      " ('collect_start_time', ?), ('collect_duration', ?), ('data_size', ?), "
                      " ('stages', ?), ('pp_stages', ?), ('algorithm', ?), ('dp_size', ?), ('pp_size', ?), "
                      " ('tp_size', ?), ('cp_size', '1'), ('ep_size', '1'), ('level', ?), ('parse_status', NULL);";
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare baseInfoTable statement. error:", sqlite3_errmsg(db));
        return;
    }
    if (stmt == nullptr) {
        ServerLog::Error("Failed to get baseInfoTable stmt.");
        return;
    }
    std::string stringStartTime = std::to_string(baseInfo.collectStartTime);
    std::string stringDuration = std::to_string(baseInfo.collectDuration);
    std::string stringDataSize = std::to_string(baseInfo.dataSize);
    std::string stringDpSize = std::to_string(baseInfo.config.dpSize);
    std::string stringPpSize = std::to_string(baseInfo.config.ppSize);
    std::string stringTpSize = std::to_string(baseInfo.config.tpSize);
    int idx = bindStartIndex;
    sqlite3_bind_text(stmt, idx++, baseInfo.filePath.c_str(), baseInfo.filePath.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, idx++, stringStartTime.c_str(), stringStartTime.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, idx++, stringDuration.c_str(), stringDuration.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, idx++, stringDataSize.c_str(), stringDataSize.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, idx++, baseInfo.stages.c_str(), baseInfo.stages.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, idx++, baseInfo.ppStages.c_str(), baseInfo.ppStages.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, idx++, baseInfo.config.algorithm.c_str(),
                      baseInfo.config.algorithm.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, idx++, stringDpSize.c_str(), stringDpSize.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, idx++, stringPpSize.c_str(), stringPpSize.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, idx++, stringTpSize.c_str(), stringTpSize.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, idx++, baseInfo.level.c_str(), baseInfo.level.length(), SQLITE_TRANSIENT);
    auto result = sqlite3_step(stmt);
    if (result != SQLITE_DONE) {
        ServerLog::Error("Insert baseInfoTable data fail. ", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
}

void TextClusterDatabase::InsertCommunicationMatrix(Dic::Module::CommunicationMatrixInfo &communicationMatrix)
{
    matrixCache.emplace_back(communicationMatrix);
    if (matrixCache.size() == TABLE_CACHE_SIZE) {
        InsertCommunicationMatrixInfo(matrixCache);
        matrixCache.clear();
    }
}

void TextClusterDatabase::InsertCommunicationMatrixInfo(std::vector<CommunicationMatrixInfo> &matrixInfos)
{
    if (matrixInfos.empty()) {
        return;
    }
    sqlite3_stmt *stmt = nullptr;
    if (matrixInfos.size() == TABLE_CACHE_SIZE && isInitStmt) {
        sqlite3_reset(matrixStmt);
        stmt = matrixStmt;
    } else {
        std::string sql = GetMatrixStmtSql(matrixInfos.size());
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
    for (const auto &info: matrixInfos) {
        sqlite3_bind_text(stmt, idx++, info.groupId.c_str(), info.groupId.length(), SQLITE_STATIC);
        sqlite3_bind_text(stmt, idx++, info.iterationId.c_str(), info.iterationId.length(), SQLITE_STATIC);
        sqlite3_bind_text(stmt, idx++, info.opName.c_str(), info.opName.length(), SQLITE_STATIC);
        sqlite3_bind_text(stmt, idx++, info.sortOp.c_str(), info.sortOp.length(), SQLITE_STATIC);
        sqlite3_bind_text(stmt, idx++, info.groupName.c_str(), info.groupName.length(), SQLITE_STATIC);
        sqlite3_bind_int(stmt, idx++, info.srcRank);
        sqlite3_bind_int(stmt, idx++, info.dstRank);
        sqlite3_bind_text(stmt, idx++, info.transportType.c_str(), info.transportType.length(), SQLITE_STATIC);
        sqlite3_bind_double(stmt, idx++, info.transitSize);
        sqlite3_bind_double(stmt, idx++, info.transitTime);
        sqlite3_bind_double(stmt, idx++, info.bandwidth);
    }
    auto result = sqlite3_step(stmt);
    if (!(matrixInfos.size() == TABLE_CACHE_SIZE && isInitStmt)) {
        sqlite3_finalize(stmt);
    }
    if (result != SQLITE_DONE) {
        ServerLog::Error("Insert matrix data fail. ", sqlite3_errmsg(db));
        return;
    }
}

std::string TextClusterDatabase::GetMatrixStmtSql(int len)
{
    std::string sql = "INSERT INTO " + TABLE_COMMUNICATION_MATRIX +
                      " (group_id, iteration_id, op_name, op_sort, group_name, src_rank, "
                      "dst_rank, transport_type, transit_size, transit_time, bandwidth) "
                      "VALUES (?,?,?,?,?,?,?,?,?,?,?)";
    for (int i = 0; i < len - 1; i++) {
        sql.append(",(?,?,?,?,?,?,?,?,?,?,?)");
    }
    return sql;
}

bool TextClusterDatabase::QueryBaseInfo(Protocol::SummaryBaseInfo &baseInfo)
{
    baseInfo.filePath = GetDbPath();
    std::string baseInfoSql = "SELECT key, value FROM " + TABLE_BASE_INFO +
        " WHERE key IN ('ranks', 'steps', 'data_size');";
    return ExecuteQueryBaseInfo(baseInfo, baseInfoSql);
}

std::string TextClusterDatabase::QueryParseClusterStatus()
{
    sqlite3_stmt *stmtBaseInfo = nullptr;
    std::string baseInfoSql = "SELECT key, value FROM " + TABLE_BASE_INFO + " WHERE key IN ('parse_status');";
    int baseInfoResult = sqlite3_prepare_v2(db, baseInfoSql.c_str(), -1, &stmtBaseInfo, nullptr);
    if (baseInfoResult != SQLITE_OK) {
        ServerLog::Error("Query parse status statement failed to prepare sql.", sqlite3_errmsg(db));
        return "";
    }
    std::map<std::string, std::string> info;
    std::string parseStatus;
    while (sqlite3_step(stmtBaseInfo) == SQLITE_ROW) {
        int coll = resultStartIndex;
        std::string key = sqlite3_column_string(stmtBaseInfo, coll++);
        std::string value = sqlite3_column_string(stmtBaseInfo, coll++);
        info.insert({key, value});
    }
    std::string valueParseStatus = FindValueByKey(info, "parse_status", "");
    parseStatus = valueParseStatus;
    sqlite3_finalize(stmtBaseInfo);
    return parseStatus;
}

void TextClusterDatabase::UpdateClusterParseStatus(std::string status)
{
    ServerLog::Info("Update_Cluster_Parse_Status status: ", status);
    sqlite3_stmt *stmtBaseInfo = nullptr;
    int index = bindStartIndex;
    std::string baseInfoSql = "UPDATE " + TABLE_BASE_INFO + " SET value = ? WHERE key = 'parse_status';";
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

bool TextClusterDatabase::GetStepIdList(Protocol::PipelineStepResponseBody &responseBody)
{
    std::string sql = "select distinct step_id as stepId FROM " + TABLE_STEP_TRACE + " ORDER BY step_id";
    return ExecuteGetStepIdList(responseBody, sql);
}

bool TextClusterDatabase::GetStages(Protocol::PipelineStageParam &param,
                                    Protocol::PipelineStageResponseBody &responseBody)
{
    std::string sql = "SELECT DISTINCT stage_id as stageId "
                      "FROM " + TABLE_STEP_TRACE + " WHERE stage_id != '' AND step_id = ?";
    return ExecuteGetStages(param, responseBody, sql);
}

bool TextClusterDatabase::GetStageAndBubble(Protocol::PipelineStageTimeParam &param,
                                            Protocol::PipelineStageOrRankTimeResponseBody &responseBody)
{
    std::string sql = "SELECT max(ROUND(stage_time, 4)) as stageTime, max(ROUND(bubble_time, 4)) as bubbleTime "
                      "FROM " + TABLE_STEP_TRACE + " WHERE step_id = ?";

    std::vector<std::string> stageIds;
    PrepareForStageId(param.stageId, sql, stageIds);

    return ExecuteGetStageAndBubble(param, stageIds, responseBody, sql);
}

bool TextClusterDatabase::GetRankAndBubble(Protocol::PipelineRankTimeParam &param,
                                           Protocol::PipelineStageOrRankTimeResponseBody &responseBody)
{
    std::string sql = "SELECT rank_id as rankId, "
                      "ROUND(stage_time, 4) as stageTime, "
                      "ROUND(bubble_time, 4) as bubbleTime "
                      " FROM " + TABLE_STEP_TRACE +
                      " WHERE step_id = ? ";
    std::vector<std::string> stageIds;
    PrepareForStageId(param.stageId, sql, stageIds);

    return ExecuteGetRankAndBubble(param, std::move(stageIds), responseBody, std::move(sql));
}

std::vector<std::string> TextClusterDatabase::GetAllRankFromStepStatisticInfo()
{
    std::string sql = "SELECT DISTINCT rank_id as rankId FROM " + TABLE_STEP_TRACE + " WHERE rankId != ''";
    return ExecuteGetAllRankFromStepStatisticInfo(sql);
}

bool TextClusterDatabase::GetGroups(const std::string &iterationId, std::vector<std::string> &groupList)
{
    std::string sql = "SELECT DISTINCT group_id as groupId FROM " + TABLE_GROUP_ID;
    return ExecuteGetGroups(iterationId, groupList, sql);
}

std::unordered_map<std::string, int64_t> TextClusterDatabase::GetAllGroupMap()
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

bool TextClusterDatabase::QueryMatrixList(Protocol::MatrixBandwidthParam &param,
                                          std::vector<MatrixInfoDo> &matrixInfoDoList)
{
    // 查询集群通信
    std::vector<MatrixInfoDo> collectiveMatrixInfoDoList;
    std::string sql = "SELECT src_rank as srcRank, dst_rank as dstRank, "
                      "transport_type as transportType, "
                      "ROUND(transit_size, 4) as transitSize, "
                      "ROUND(transit_time, 4) as transitTime, "
                      "ROUND(bandwidth, 4) as bandwidth ,"
                      "op_name as opName "
                      "FROM " + TABLE_COMMUNICATION_MATRIX + " CM"
                      " LEFT JOIN " + TABLE_GROUP_ID + " g ON cm.group_id = g.id";
    std::string collectiveCondition = " WHERE g.group_id = ? AND iteration_id = ? AND op_sort = ? ";
    auto collectiveRes = ExecuteQueryMatrixList(param, collectiveMatrixInfoDoList, sql + collectiveCondition);
    // 符合两种情况直接返回：1.如果不是pp通信域；2.是pp通信域但非send receive算子
    if (param.pgName != ppVal || !CheckIsPpOp(param.operatorName)) {
        matrixInfoDoList = collectiveMatrixInfoDoList;
        return collectiveRes;
    }

    // 查询p2p内容
    Protocol::MatrixBandwidthParam paramTemp{p2pVal, param.operatorName, param.iterationId};
    std::vector<MatrixInfoDo> ppMatrixInfoDoList;
    std::string rankSqlCondition = GetRankStrForSql(param.stage);
    std::string ppCondition = " WHERE g.group_id = ? AND iteration_id = ? AND op_sort = ? AND src_rank in "
        + rankSqlCondition + " AND dst_rank in " + rankSqlCondition;
    auto ppRes = ExecuteQueryMatrixList(paramTemp, ppMatrixInfoDoList, sql + ppCondition);
    if (param.operatorName == totalOpInfo) {
        matrixInfoDoList = MergeMatrixInfoDoList(collectiveMatrixInfoDoList, ppMatrixInfoDoList);
    } else {
        matrixInfoDoList = ppMatrixInfoDoList;
    }
    return ppRes || collectiveRes;
}

std::vector<MatrixInfoDo> TextClusterDatabase::MergeMatrixInfoDoList(const std::vector<MatrixInfoDo> &collective,
                                                                     const std::vector<MatrixInfoDo> &p2p)
{
    std::unordered_map<std::string, MatrixInfoDo> collectiveMap;
    std::unordered_set<std::string> keySet;
    for (const auto &item: collective) {
        std::string key = std::to_string(item.srcRank) + "-" + std::to_string(item.dstRank);
        keySet.insert(key);
        collectiveMap[key] = item;
    }

    std::unordered_map<std::string, MatrixInfoDo> p2pMap;
    for (const auto &item: p2p) {
        std::string key = std::to_string(item.srcRank) + "-" + std::to_string(item.dstRank);
        keySet.insert(key);
        p2pMap[key] = item;
    }
    std::vector<MatrixInfoDo> res;
    for (const auto &key: keySet) {
        MatrixInfoDo info;
        info.transitSize = NumberUtil::DoubleReservedNDigits(
            collectiveMap[key].transitSize + p2pMap[key].transitSize, reservedNumber);
        info.transitTime = NumberUtil::DoubleReservedNDigits(
            collectiveMap[key].transitTime + p2pMap[key].transitTime, reservedNumber);
        if (info.transitTime != 0) {
            info.bandwidth =
                NumberUtil::DoubleReservedNDigits(info.transitSize / info.transitTime, reservedNumber);
        }
        // 此处key值是由上面的代码使用 “-” 进行拼接而成，其大小必定为2
        std::vector<std::string> rankList = StringUtil::Split(key, "-");
        info.srcRank = StringUtil::StringToInt(rankList[0]);
        info.dstRank = StringUtil::StringToInt(rankList[1]);
        if (collectiveMap[key].transportType == p2pMap[key].transportType) {
            info.transportType = collectiveMap[key].transportType;
        } else {
            info.transportType = collectiveMap[key].transportType == "" ? p2pMap[key].transportType :
                collectiveMap[key].transportType;
        }
        res.push_back(info);
    }
    return res;
}

bool TextClusterDatabase::CheckIsPpOp(const std::string &opName)
{
    std::string opNameLowercase = StringUtil::ToLower(opName);
    // 判断算子是否为Total Op Info 或send或receive
    return opName == totalOpInfo || opNameLowercase.find(sendOpKey) != std::string::npos ||
        opNameLowercase.find(receiveOpKey) != std::string::npos;
}

bool TextClusterDatabase::QueryExtremumTimestamp(uint64_t &min, uint64_t &max)
{
    std::string sql = "SELECT MIN(start_time) as minTime, MAX(start_time) as maxTime "
                      "FROM " + TABLE_TIME_INFO + " WHERE start_time != 0";
    return ExecuteQueryExtremumTimestamp(sql, min, max);
}

bool TextClusterDatabase::QueryIterationAndCommunicationGroup(Protocol::CommunicationKernelParams &params,
                                                              Protocol::CommunicationKernelBody &responseBody)
{
    std::string sql = "select iteration_id, tg.group_id as stage_id from " + TABLE_TIME_INFO + " t"
        " LEFT JOIN " + TABLE_GROUP_ID + " tg ON t.stage_id = tg.id"
        " where op_name = ? and rank_id = ?";
    return ExecuteQueryIterationAndCommunicationGroup(sql, params.name, params.rankId, responseBody.step,
        responseBody.group);
}

std::string TextClusterDatabase::GetAllOperatorsSql(const std::string &startTime, const std::string &bandwidthCondition,
                                                    const std::string &timeCondition)
{
    std::string sql =
        "SELECT t.op_name as operatorName, "
        "CASE WHEN start_time = 0 THEN 0 ELSE ROUND((start_time - " + startTime + ") / 1000000.0, 4) END as startTime, "
        "ROUND(elapse_time, 4) as elapseTime, "
        "ROUND(transit_time, 4) as transitTime, "
        "ROUND(synchronization_time, 4) as synchronizationTime, "
        "ROUND(wait_time, 4) as waitTime, "
        "ROUND(idle_time, 4) as idleTime, "
        "ROUND(synchronization_time_ratio, 4) as synchronizationTimeRatio, "
        "ROUND(wait_time_ratio, 4) as waitTimeRatio, "
        "bw.sdma_bw as sdmaBw, bw.rdma_bw as rdmaBw "
        "FROM " + TABLE_TIME_INFO + " t "
        "JOIN ( "
        "    SELECT op_name, "
        "    MAX(CASE WHEN transport_type = 'SDMA' THEN bandwidth_size ELSE 0 END) AS sdma_bw, "
        "    MAX(CASE WHEN transport_type = 'RDMA' THEN bandwidth_size ELSE 0 END) AS rdma_bw "
        "    FROM " + TABLE_BANDWIDTH + bandwidthCondition +
        "    GROUP BY op_name "
        ") bw ON t.op_name = bw.op_name " + timeCondition;
    return sql;
}

std::string TextClusterDatabase::GetAllOperatorsSql(uint64_t &startTime, const Protocol::OperatorDetailsParam &param)
{
    std::string bandwidthCondition =
        " WHERE iteration_id = ? AND rank_id = ? AND stage_id = ? AND op_name != 'Total Op Info' ";
    std::string timeCondition =
        " WHERE t.iteration_id = ? AND t.rank_id = ? AND t.stage_id = ? AND t.op_name != 'Total Op Info'";
    std::string sql = GetAllOperatorsSql("?", bandwidthCondition, timeCondition);
    if (param.pgName != ppVal) {
        return sql;
    }
    std::string stageId = GetStageIdByGroupId(p2pVal);
    bool checkParamValid = !StringUtil::CheckSqlValid(param.iterationId) || !StringUtil::CheckSqlValid(param.rankId)
        || !StringUtil::CheckSqlValid(stageId);
    if (checkParamValid) {
        ServerLog::Error("Fail to get all operators sql when query pipeline data, invalid param");
        return sql;
    }
    std::string ppBwCondition = " WHERE iteration_id = '" + param.iterationId + "' AND rank_id = '" +
        param.rankId + "' AND stage_id = '" + stageId + "' AND op_name != 'Total Op Info' ";
    std::string ppTimeCondition = " WHERE t.iteration_id = '" + param.iterationId + "' AND t.rank_id = '" +
        param.rankId + "' AND t.stage_id = '" + stageId + "' AND t.op_name != 'Total Op Info'";
    std::string ppSql = GetAllOperatorsSql(std::to_string(startTime), ppBwCondition, ppTimeCondition);
    std::string finalSql = "SELECT operatorName, startTime, elapseTime, transitTime, synchronizationTime, waitTime, "
       "idleTime, synchronizationTimeRatio, waitTimeRatio, sdmaBw, rdmaBw FROM (" + sql + " UNION ALL " + ppSql + ")";
    return finalSql;
}

bool TextClusterDatabase::QueryAllOperators(Protocol::OperatorDetailsParam &param,
    Protocol::OperatorDetailsResBody &resBody)
{
    uint64_t startTime = Timeline::TraceTime::Instance().GetStartTime();
    std::string sql = GetAllOperatorsSql(startTime, param);
    // 深拷贝避免修改原param中的内容
    Protocol::OperatorDetailsParam copyParam(param);
    copyParam.stage = GetStageIdByGroupId(copyParam.stage);
    return ExecuteQueryAllOperators(copyParam, resBody, sql, startTime);
}

bool TextClusterDatabase::QueryOperatorsCount(Protocol::OperatorDetailsParam &param,
    Protocol::OperatorDetailsResBody &resBody)
{
    std::string sql = "SELECT op_name, count(*) AS nums from " + TABLE_TIME_INFO + " where 1=1 ";
    Protocol::OperatorDetailsParam copyParam(param);
    copyParam.stage = GetStageIdByGroupId(copyParam.stage);
    Protocol::OperatorDetailsResBody collective;
    bool collectiveRes = ExecuteQueryOperatorsCount(copyParam, collective, sql);
    if (param.pgName != ppVal) {
        resBody.count = collective.count;
        return collectiveRes;
    }

    Protocol::OperatorDetailsParam paramsP2p(copyParam);
    paramsP2p.stage = GetStageIdByGroupId(p2pVal);
    Protocol::OperatorDetailsResBody p2pBody;
    bool ppRes = ExecuteQueryOperatorsCount(paramsP2p, p2pBody, sql);
    resBody.count = collective.count + p2pBody.count;
    return ppRes || collectiveRes;
}

Protocol::BandwidthDataResBody TextClusterDatabase::MergeBandwidthData(const Protocol::BandwidthDataResBody &collective,
                                                                       const Protocol::BandwidthDataResBody &p2p)
{
    std::unordered_set<std::string> index;
    std::unordered_map<std::string, Protocol::BandwidthDataItem> colMap;
    for (const auto &item: collective.items) {
        colMap[item.transportType] = item;
        index.insert(item.transportType);
    }

    std::unordered_map<std::string, Protocol::BandwidthDataItem> p2pMap;
    for (const auto &item: p2p.items) {
        p2pMap[item.transportType] = item;
        index.insert(item.transportType);
    }

    Protocol::BandwidthDataResBody res;
    // 大包占比求平均被除数
    int divisor = 2;
    for (const auto &item: index) {
        Protocol::BandwidthDataItem info;
        info.transportType = item;
        info.transitSize = NumberUtil::DoubleReservedNDigits(
            colMap[item].transitSize + p2pMap[item].transitSize, reservedNumber);
        info.transitTime = NumberUtil::DoubleReservedNDigits(
            colMap[item].transitTime + p2pMap[item].transitTime, reservedNumber);
        if (info.transitTime != 0) {
            info.bandwidth = NumberUtil::DoubleReservedNDigits(info.transitSize / info.transitTime, reservedNumber);
        }
        info.largePacketRatio = NumberUtil::DoubleReservedNDigits(
            (colMap[item].largePacketRatio + p2pMap[item].largePacketRatio) / divisor, reservedNumber);
        res.items.push_back(info);
    }
    return res;
}

bool TextClusterDatabase::QueryBandwidthData(Protocol::BandwidthDataParam &param,
                                             Protocol::BandwidthDataResBody &resBody)
{
    std::string sql = "SELECT transport_type,ROUND(transit_size, 4) as transit_size,"
                      "ROUND(transit_time, 4) as transit_time,"
                      "ROUND(bandwidth_size, 4) as bandwidth_size,"
                      "ROUND(large_package_ratio, 4)  as large_package_ratio from "
                      + TABLE_BANDWIDTH + " b "
                      " LEFT JOIN " + TABLE_GROUP_ID + " g ON b.stage_id = g.id"
                      " WHERE iteration_id = ? AND rank_id = ? AND g.group_id = ? AND op_name = ? ";
    Protocol::BandwidthDataResBody collective;
    bool collectiveRes = ExecuteQueryBandwidthData(param, collective, sql);
    // 符合两种情况直接返回：1.如果不是pp通信域；2.是pp通信域但非send receive算子
    if (param.pgName != ppVal || !CheckIsPpOp(param.operatorName)) {
        resBody = collective;
        return collectiveRes;
    }

    Protocol::BandwidthDataParam p2pParam{param.iterationId, param.rankId, param.operatorName, p2pVal};
    Protocol::BandwidthDataResBody p2p;
    bool p2pRes = ExecuteQueryBandwidthData(p2pParam, p2p, sql);
    resBody = MergeBandwidthData(collective, p2p);
    return collectiveRes || p2pRes;
}

std::string TextClusterDatabase::MergeDistributionJson(const std::optional<document_t> &colData,
                                                       const std::optional<document_t> &p2pData)
{
    std::unordered_set<std::string> index;
    std::unordered_map<std::string, std::vector<double>> colMap;
    for (const auto &item: colData->GetObj()) {
        if (!item.name.IsString() || !item.value.IsArray()) {
            continue;
        }
        auto name = item.name.GetString();
        index.insert(name);
        for (const auto &num: item.value.GetArray()) {
            double temp = num.IsString() ? NumberUtil::StringToDouble(num.GetString()) : 0;
            colMap[name].push_back(temp);
        }
    }
    std::unordered_map<std::string, std::vector<double>> p2pMap;
    for (const auto &item: p2pData->GetObj()) {
        if (!item.name.IsString() || !item.value.IsArray()) {
            continue;
        }
        auto name = item.name.GetString();
        index.insert(name);
        for (const auto &num: item.value.GetArray()) {
            double temp = num.IsString() ? NumberUtil::StringToDouble(num.GetString()) : 0;
            p2pMap[name].push_back(temp);
        }
    }
    document_t json(kObjectType);
    auto &allocator = json.GetAllocator();
    for (const auto &item: index) {
        std::vector<double> temp;
        size_t maxSize = std::max(colMap[item].size(), p2pMap[item].size());
        colMap[item].resize(maxSize, 0.0);
        p2pMap[item].resize(maxSize, 0.0);
        temp.resize(maxSize);
        std::transform(colMap[item].begin(), colMap[item].end(), p2pMap[item].begin(),
            temp.begin(), std::plus<double>());
        JsonUtil::AddMember(json, item, temp, allocator);
    }
    return JsonUtil::JsonDump(json);
}

Protocol::DistributionResBody TextClusterDatabase::MergeDistribution(Protocol::DistributionResBody &collective,
                                                                     Protocol::DistributionResBody &p2p)
{
    collective.distributionData = collective.distributionData == "" ? "{}" : collective.distributionData;
    p2p.distributionData = p2p.distributionData == "" ? "{}" : p2p.distributionData;
    if (collective.distributionData == "{}") {
        return p2p;
    }
    if (p2p.distributionData == "{}") {
        return collective;
    }

    try {
        std::string error;
        auto colData = JsonUtil::TryParse<kParseNumbersAsStringsFlag>(collective.distributionData, error);
        auto p2pData = JsonUtil::TryParse<kParseNumbersAsStringsFlag>(p2p.distributionData, error);
        if (!colData.has_value() || !colData->IsObject()) {
            return p2p;
        }
        if (!p2pData.has_value() || !p2pData->IsObject()) {
            return collective;
        }
        Protocol::DistributionResBody res;
        res.distributionData = MergeDistributionJson(colData, p2pData);
        return res;
    } catch (std::exception &e) {
        // 异常解析失败返回空列表
        Server::ServerLog::Error("Merge distribution failed by system exception (%s)", e.what());
        return {};
    }
}

bool TextClusterDatabase::QueryDistributionData(Protocol::DistributionDataParam &param,
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
    Protocol::DistributionResBody collective;
    bool collectiveRes = ExecuteQueryDistributionData(param, collective, sql);
    // 符合两种情况直接返回：1.如果不是pp通信域；2.是pp通信域但非send receive算子
    if (param.pgName != ppVal || !CheckIsPpOp(param.operatorName)) {
        resBody = collective;
        return collectiveRes;
    }

    Protocol::DistributionDataParam p2pParam{param.iterationId, param.rankId, param.operatorName,
        param.transportType, p2pVal};
    Protocol::DistributionResBody p2p;
    bool p2pRes = ExecuteQueryDistributionData(p2pParam, p2p, sql);
    resBody = MergeDistribution(collective, p2p);
    return collectiveRes || p2pRes;
}

bool TextClusterDatabase::QueryRanksHandler(std::vector<Protocol::IterationsOrRanksObject> &responseBody)
{
    std::string sql = "SELECT key, value FROM " + TABLE_BASE_INFO + " WHERE key IN ('ranks');";
    return ExecuteQueryRanksHandler(responseBody, sql);
}

bool TextClusterDatabase::QueryOperatorNames(Protocol::OperatorNamesParams &requestParams,
                                             std::vector<Protocol::OperatorNamesObject> &responseBody)
{
    std::vector<std::string> rankList = requestParams.rankList;
    std::string sql = "SELECT DISTINCT op_name FROM " + TABLE_TIME_INFO;
    std::string collectiveCondition = " WHERE iteration_id = ? AND stage_id = ? ORDER BY op_name";
    Protocol::OperatorNamesParams copyParams(requestParams);
    copyParams.stage = GetStageIdByGroupId(copyParams.stage);
    std::vector<Protocol::OperatorNamesObject> collectiveOpNameList;
    auto collectiveRes = ExecuteQueryOperatorNames(copyParams, collectiveOpNameList, sql + collectiveCondition);
    if (requestParams.pgName != ppVal) {
        responseBody = collectiveOpNameList;
        return collectiveRes;
    }
    std::string p2pCondition = " WHERE iteration_id = ? AND stage_id = ? AND rank_id in "
       + GetRankStrForSql(requestParams.stage);
    Protocol::OperatorNamesParams p2pParams(copyParams);
    p2pParams.stage = GetStageIdByGroupId(p2pVal);
    std::vector<Protocol::OperatorNamesObject> p2pOpNameList;
    auto p2pRes = ExecuteQueryOperatorNames(p2pParams, p2pOpNameList, sql + p2pCondition);
    responseBody = MergeOperatorNameObject(collectiveOpNameList, p2pOpNameList);
    return collectiveRes || p2pRes;
}

std::string TextClusterDatabase::GetRankStrForSql(const std::string &rankListStr)
{
    std::vector<std::string> rankList = StringUtil::SplitStringWithParenthesesByComma(rankListStr);
    return GetRanksSql(rankList);
}

bool TextClusterDatabase::QueryMatrixSortOpNames(Protocol::OperatorNamesParams &requestParams,
                                                 std::vector<Protocol::OperatorNamesObject> &responseBody)
{
    std::vector<Protocol::OperatorNamesObject> collectiveOpNameList;
    std::string sql = "SELECT DISTINCT op_sort  FROM " + TABLE_COMMUNICATION_MATRIX + " cm"
            " LEFT JOIN " + TABLE_GROUP_ID + " g ON cm.group_id = g.id";
    std::string collectiveCondition = " WHERE iteration_id = ? AND g.group_id = ? ORDER BY op_sort";
    auto collectiveRes = ExecuteQueryMatrixSortOpNames(requestParams, collectiveOpNameList, sql + collectiveCondition);
    if (requestParams.pgName != ppVal) {
        responseBody = collectiveOpNameList;
        return collectiveRes;
    }
    std::string rankCondition = GetRankStrForSql(requestParams.stage);
    std::string p2pCondition = " WHERE iteration_id = ? AND g.group_id = ? AND src_rank in "
       + rankCondition + " AND dst_rank in " + rankCondition;
    std::vector<Protocol::OperatorNamesObject> p2pOpNameList;
    Protocol::OperatorNamesParams paramsTemp;
    paramsTemp.iterationId = requestParams.iterationId;
    paramsTemp.stage = p2pVal;
    auto p2pRes = ExecuteQueryMatrixSortOpNames(paramsTemp, p2pOpNameList, sql + p2pCondition);
    responseBody = MergeOperatorNameObject(collectiveOpNameList, p2pOpNameList);
    return collectiveRes || p2pRes;
}

std::vector<Protocol::OperatorNamesObject> TextClusterDatabase::MergeOperatorNameObject(
    const std::vector<Protocol::OperatorNamesObject> &collective, const std::vector<Protocol::OperatorNamesObject> &p2p)
{
    std::vector<Protocol::OperatorNamesObject> res;
    res.insert(res.end(), collective.begin(), collective.end());
    res.insert(res.end(), p2p.begin(), p2p.end());
    std::sort(res.begin(), res.end());
    // 去重
    auto temp = std::unique(res.begin(), res.end());
    res.erase(temp, res.end());
    return res;
}

bool TextClusterDatabase::QueryIterations(std::vector<Protocol::IterationsOrRanksObject> &responseBody)
{
    std::string sql = "SELECT key, value FROM " + TABLE_BASE_INFO + " WHERE key IN ('steps');";
    return ExecuteQueryIterations(responseBody, sql);
}

std::string TextClusterDatabase::GetDurationListSql(const std::string &bandwidthCondition,
                                                    const std::string &timeCondition)
{
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
        "    FROM " + TABLE_BANDWIDTH + bandwidthCondition +
        "    GROUP BY rank_id "
        ") bw ON t.rank_id = bw.rank_id" + timeCondition;
    return sql;
}

std::vector<DurationDo> TextClusterDatabase::MergeDurationDoList(const std::vector<DurationDo> &collective,
                                                                 const std::vector<DurationDo> &p2p)
{
    std::unordered_set<std::string> rankList;
    std::unordered_map<std::string, DurationDo> collectiveMap;
    for (const auto &item: collective) {
        rankList.insert(item.rankId);
        collectiveMap[item.rankId] = item;
    }
    std::unordered_map<std::string, DurationDo> p2pMap;
    for (const auto &item: p2p) {
        rankList.insert(item.rankId);
        p2pMap[item.rankId] = item;
    }
    std::vector<DurationDo> res;
    for (const auto &item: rankList) {
        DurationDo durationDo;
        durationDo.rankId = item;
        durationDo.startTime = NumberUtil::DoubleReservedNDigits(
            std::min(collectiveMap[item].startTime, p2pMap[item].startTime), reservedNumber);
        durationDo.elapseTime = NumberUtil::DoubleReservedNDigits(
            collectiveMap[item].elapseTime + p2pMap[item].elapseTime, reservedNumber);
        durationDo.transitTime = NumberUtil::DoubleReservedNDigits(
            collectiveMap[item].transitTime + p2pMap[item].transitTime, reservedNumber);
        durationDo.synchronizationTime = NumberUtil::DoubleReservedNDigits(
            collectiveMap[item].synchronizationTime + p2pMap[item].synchronizationTime, reservedNumber);
        durationDo.transitTime = NumberUtil::DoubleReservedNDigits(
            collectiveMap[item].transitTime + p2pMap[item].transitTime, reservedNumber);
        durationDo.waitTime = NumberUtil::DoubleReservedNDigits(
            collectiveMap[item].waitTime + p2pMap[item].waitTime, reservedNumber);
        durationDo.idleTime = NumberUtil::DoubleReservedNDigits(
            collectiveMap[item].idleTime + p2pMap[item].idleTime, reservedNumber);
        if (durationDo.synchronizationTime + durationDo.transitTime != 0) {
            durationDo.synchronizationTimeRatio = NumberUtil::DoubleReservedNDigits(durationDo.synchronizationTime
                / (durationDo.synchronizationTime + durationDo.transitTime), reservedNumber);
        }
        if (durationDo.transitTime + durationDo.waitTime != 0) {
            durationDo.waitTimeRatio = NumberUtil::DoubleReservedNDigits(
                durationDo.waitTime / (durationDo.transitTime + durationDo.waitTime), reservedNumber);
        }
        durationDo.sdmaBw = NumberUtil::DoubleReservedNDigits(
            std::max(collectiveMap[item].sdmaBw, p2pMap[item].sdmaBw), reservedNumber);
        durationDo.rdmaBw = NumberUtil::DoubleReservedNDigits(
            std::max(collectiveMap[item].rdmaBw, p2pMap[item].rdmaBw), reservedNumber);
        durationDo.sdmaTime = NumberUtil::DoubleReservedNDigits(
            std::max(collectiveMap[item].sdmaTime, p2pMap[item].sdmaTime), reservedNumber);
        durationDo.rdmaTime = NumberUtil::DoubleReservedNDigits(
            std::max(collectiveMap[item].rdmaTime, p2pMap[item].rdmaTime), reservedNumber);
        res.push_back(durationDo);
    }
    return res;
}

bool TextClusterDatabase::QueryDurationList(Protocol::DurationListParams &requestParams,
                                            std::vector<DurationDo> &durationDoList)
{
    uint64_t startTime = Module::Timeline::TraceTime::Instance().GetStartTime();
    std::string collectiveCondition = " WHERE iteration_id = ? AND stage_id = ? AND op_name = ? ";
    std::string collectiveSql = GetDurationListSql(collectiveCondition, collectiveCondition);
    Protocol::DurationListParams copyParams(requestParams);
    copyParams.stage = GetStageIdByGroupId(copyParams.stage);
    std::vector<DurationDo> collectiveDurations;
    bool collectiveRes = ExecuteQueryDurationList(copyParams, collectiveDurations, collectiveSql, startTime);
    // 符合两种情况直接返回：1.如果不是pp通信域；2.是pp通信域但非send receive算子
    if (requestParams.pgName != ppVal || !CheckIsPpOp(requestParams.operatorName)) {
        durationDoList = collectiveDurations;
        return collectiveRes;
    }

    // 查询p2p内容
    Protocol::DurationListParams paramsP2p(copyParams);
    paramsP2p.stage = GetStageIdByGroupId(p2pVal);
    std::string rankCondition = GetRankStrForSql(requestParams.stage);
    std::string ppCondition = " WHERE iteration_id = ? AND stage_id = ? AND op_name = ? AND rank_id in " +
        rankCondition;
    std::string ppTimeCondition = " WHERE iteration_id = ? AND stage_id = ? AND op_name = ? AND t.rank_id in " +
        rankCondition;
    std::string ppSql = GetDurationListSql(ppCondition, ppTimeCondition);
    std::vector<DurationDo> ppDurations;
    auto ppRes = ExecuteQueryDurationList(paramsP2p, ppDurations, ppSql, startTime);
    if (requestParams.operatorName == totalOpInfo) {
        durationDoList = MergeDurationDoList(collectiveDurations, ppDurations);
    } else {
        durationDoList = ppDurations;
    }
    return ppRes || collectiveRes;
}

std::string TextClusterDatabase::GetStageIdByGroupId(const std::string &groupId)
{
    if (groupId.empty()) {
        return "";
    }
    std::string sql = "SELECT id From " + TABLE_GROUP_ID + " WHERE group_id = ?";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Get stage id by group id failed to prepare sql.");
        return "";
    }
    std::unique_ptr<SqliteResultSet> resultSet;
    resultSet = stmt->ExecuteQuery(groupId);
    if (!resultSet) {
        ServerLog::Error("Failed to execute query.");
        return "";
    }
    std::string stageId;
    if (resultSet->Next()) {
        stageId = resultSet->GetString("id");
    }
    return stageId;
}

bool TextClusterDatabase::QueryOperatorList(Protocol::DurationListParams &requestParams,
                                            std::vector<OperatorTimeDo> &operatorTimeDoList)
{
    uint64_t startTime = Module::Timeline::TraceTime::Instance().GetStartTime();
    // 基础sql
    std::string sql =
        "SELECT rank_id, op_name,"
        " CASE WHEN start_time = 0 THEN 0 ELSE (start_time - ?) END as startTime,"
        " (elapse_time * 1000000) as elapse_time From " + TABLE_TIME_INFO;
    // 拼接集群通信查询sql
    std::string collectiveCondition = " WHERE iteration_id = ? AND stage_id = ? ";
    std::string opNameQuerySql = requestParams.operatorName == totalOpInfo ? " AND op_name <> 'Total Op Info'" :
        " AND op_name = ?";
    std::string collectiveSql = sql + collectiveCondition + opNameQuerySql + " ORDER by CAST(rank_id AS UNSIGNED) ASC";
    // 组装集群通信参数
    Protocol::DurationListParams copyParams(requestParams);
    copyParams.stage = GetStageIdByGroupId(copyParams.stage);
    // 集群通信数据查询
    std::vector<OperatorTimeDo> collectiveOpTimeList;
    auto collectiveRes = ExecuteQueryOperatorList(copyParams, collectiveOpTimeList, collectiveSql, startTime);

    // 符合两种情况直接返回：1.如果不是pp通信域；2.是pp通信域但非send receive算子
    if (requestParams.pgName != ppVal || !CheckIsPpOp(requestParams.operatorName)) {
        operatorTimeDoList = collectiveOpTimeList;
        return collectiveRes;
    }
    // 查询p2p内容
    Protocol::DurationListParams paramP2p(copyParams);
    paramP2p.stage = GetStageIdByGroupId(p2pVal);
    std::vector<OperatorTimeDo> ppOpTimeList;
    std::string ppCondition = " WHERE iteration_id = ? AND stage_id = ? AND rank_id in "
                              + GetRankStrForSql(requestParams.stage) + opNameQuerySql;
    auto ppRes = ExecuteQueryOperatorList(paramP2p, ppOpTimeList, sql + ppCondition, startTime);
    if (requestParams.operatorName == totalOpInfo) {
        operatorTimeDoList.insert(operatorTimeDoList.end(), collectiveOpTimeList.begin(), collectiveOpTimeList.end());
        operatorTimeDoList.insert(operatorTimeDoList.end(), ppOpTimeList.begin(), ppOpTimeList.end());
    } else {
        operatorTimeDoList = ppOpTimeList;
    }
    return ppRes || collectiveRes;
}

void TextClusterDatabase::PrepareForStageId(std::string &stageIdStr, std::string &sql,
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
    for (size_t i = 0; i < stageIds.size(); i++) {
        if (i == 0) {
            rankSql.append("?");
        }
        rankSql.append(",?");
    }
    if (!rankSql.empty()) {
        sql += "AND rank_id IN (" + rankSql + ")";
    }
}

bool TextClusterDatabase::QueryParallelStrategyConfig(ParallelStrategyConfig &config, std::string &level)
{
    std::string sql = "SELECT key, value FROM " + TABLE_BASE_INFO + " WHERE key IN " +
        "('algorithm', 'dp_size', 'pp_size', 'tp_size', 'cp_size', 'ep_size', 'level');";
    return ExecuteQueryParallelStrategyConfig(sql, config, level);
}

bool TextClusterDatabase::UpdateParallelStrategyConfig(const ParallelStrategyConfig &config,
    std::string &level, std::string &msg)
{
    std::string sql = "UPDATE " + TABLE_BASE_INFO + " SET value = "
                      " CASE WHEN key = 'algorithm' THEN ?"
                      " WHEN key = 'dp_size' THEN ?"
                      " WHEN key = 'pp_size' THEN ?"
                      " WHEN key = 'tp_size' THEN ?"
                      " WHEN key = 'cp_size' THEN ?"
                      " WHEN key = 'ep_size' THEN ?"
                      " WHEN key = 'level' THEN ?"
                      " ELSE value END;";
    return ExecuteSetParallelStrategyConfig(sql, config, level);
}

bool TextClusterDatabase::GetParallelConfigFromStepTrace(ParallelStrategyConfig &config, std::string &level)
{
    std::string sql = "select dp_index, pp_index, tp_index from " + TABLE_STEP_TRACE + " "
        "where rank_id != '' and step_id = (select DISTINCT step_id FROM " + TABLE_STEP_TRACE + " limit 1) "
        "order by step_id asc, CAST(rank_id AS INTEGER) asc";
    return ExecuteGetParallelConfigFromStepTrace(sql, config, level);
}

bool TextClusterDatabase::QueryAllPerformanceDataByStep(const std::string &step,
                                                        std::unordered_map<std::uint32_t, StepStatistic> &data)
{
    std::string sql;
    if (step.empty() || step == "All") {
        sql = "select rank_id as rank, round(sum(compute_time), 3) as compute, "
            "round(sum(pure_communication_time), 3) as not_overlap, "
            "round(sum(overlap_communication_time), 3) as overlap, round(sum(communication_time), 3) as communication, "
            "round(sum(free_time), 3) as free, "
            "round(sum(pure_communication_exclude_receive_time), 3) as exclude_receive, "
            "case WHEN sum(preparing) < 0 then 0 else round(sum(preparing), 3) end as preparing "
            "FROM " + TABLE_STEP_TRACE + " WHERE rank_id <> '' GROUP BY rank_id";
    } else {
        sql = "select rank_id as rank, compute_time as compute, pure_communication_time as not_overlap, "
            "overlap_communication_time as overlap, communication_time as communication, "
            "free_time as free, pure_communication_exclude_receive_time as exclude_receive, "
            "case WHEN preparing < 0 then 0 else preparing end as preparing FROM " + TABLE_STEP_TRACE + " "
            "WHERE rank_id <> '' and step_id = ?";
    }
    return ExecuteQueryAllPerformanceDataByStep(sql, step, data);
}

std::vector<CommInfoUnderRank> TextClusterDatabase::GetCommTimeForRankDim(const std::string &stepId)
{
    std::string sql;
    if (stepId.empty() || stepId == "All") {
        sql = "SELECT t.rank_id as rankId, ROUND(sum(t.elapse_time) * 1000, 3) as commTime, g.group_id as rankSet FROM "
            + TABLE_TIME_INFO + " as t LEFT JOIN " + TABLE_GROUP_ID + " as g ON t.stage_id = g.id "
            "WHERE op_name = 'Total Op Info' group by t.rank_id, g.group_id";
    } else {
        sql = "SELECT t.rank_id as rankId, ROUND(t.elapse_time * 1000, 3) as commTime, g.group_id as rankSet FROM "
            + TABLE_TIME_INFO + " as t LEFT JOIN " + TABLE_GROUP_ID + " as g ON t.stage_id = g.id "
            "WHERE op_name = 'Total Op Info' AND iteration_id = ?";
    }
    return ExecuteGetCommTimeForRankDim(sql, stepId);
}
} // end of namespace Module
} // end of namespace Dic
// LCOV_EXCL_BR_STOP