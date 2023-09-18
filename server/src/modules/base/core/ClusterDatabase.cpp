/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "ClusterDatabase.h"
#include "ServerLog.h"
#include "SummaryProtocolResponse.h"

namespace Dic {
namespace Module {
using namespace Server;
ClusterDatabase::~ClusterDatabase()
{
    SaveLastData();
    if (isInitStmt) {
        ReleaseStmt();
    }
}

bool ClusterDatabase::SetConfig()
{
    if (!isOpen) {
        ServerLog::Error("Failed to set config. Database is not open.");
        return false;
    }
    return ExecSql("PRAGMA synchronous = OFF; PRAGMA journal_mode = MEMORY;");
}

bool ClusterDatabase::CreateTable()
{
    if (!isOpen) {
        ServerLog::Error("Failed to set config. Cluster Database is not open.");
        return false;
    }
    std::string sql =
            "CREATE TABLE " + timeInfoTable +
            " (id INTEGER PRIMARY KEY AUTOINCREMENT, iteration_id VARCHAR(50),"
            " stage_id VARCHAR(200), rank_id VARCHAR(50), op_name VARCHAR(100),"
            " op_suffix VARCHAR(100), elapse_time double, synchronization_time_ratio double, "
            "synchronization_time double, transit_time double, wait_time_ratio double, "
            "wait_time double, idle_time double);"
            "CREATE TABLE " + bandwidthTable +
            " (id INTEGER PRIMARY KEY AUTOINCREMENT, iteration_id VARCHAR(50), "
            "stage_id VARCHAR(200), rank_id VARCHAR(50), op_name VARCHAR(100), "
            "op_suffix VARCHAR(100), transport_type VARCHAR(20), bandwidth_size double,"
            " bandwidth_utilization double, large_package_ratio double, size_distribution json,"
            " transit_size double, transit_time double);" +
            "CREATE TABLE " + baseInfoTable +
            " (id INTEGER PRIMARY KEY AUTOINCREMENT, file_path VARCHAR(500), ranks json,"
            " steps json, collect_start_time DATETIME, collect_duration double, data_size double);" +
            "CREATE TABLE " + stepTraceTable +
            "(id INTEGER PRIMARY KEY AUTOINCREMENT, rank_id VARCHAR(50), step_id VARCHAR(50),"
            " stage_id VARCHAR(50), compute_time double, pure_communication_time double, "
            "overlap_communication_time double, communication_time double, free_time double, "
            "stage_time double, bubble_time double, pure_communication_exclude_receive_time double);";
    return ExecSql(sql);
}

bool ClusterDatabase::InitStmt()
{
    if (isInitStmt) {
        return true;
    }
    insertTimeInfoStmt = GetTimeInfoStmtSql(cacheSize);
    insertBandwidthStmt = GetBandwidthStmtSql(cacheSize);
    isInitStmt = true;
    return true;
}

sqlite3_stmt *ClusterDatabase::GetTimeInfoStmtSql(int len)
{
    if (len == cacheSize && isInitStmt) {
        sqlite3_reset(insertTimeInfoStmt);
        return insertTimeInfoStmt;
    }
    sqlite3_stmt *stmt = nullptr;
    std::string sql = "INSERT INTO " + timeInfoTable +
                      " (iteration_id, stage_id, rank_id, op_name, op_suffix, elapse_time,"
                      " synchronization_time_ratio,"
                      "synchronization_time, transit_time, wait_time_ratio, wait_time, idle_time )"
                      " VALUES (?,?,?,?,?,?,?,?,?,?,?,?)";
    for (int i = 0; i < len - 1; i++) {
        sql.append(",(?,?,?,?,?,?,?,?,?,?,?,?)");
    }
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare timeInfoTable statement. error:", sqlite3_errmsg(db));
    }
    return stmt;
}

sqlite3_stmt *ClusterDatabase::GetBandwidthStmtSql(int len)
{
    if (len == cacheSize && isInitStmt) {
        sqlite3_reset(insertBandwidthStmt);
        return insertBandwidthStmt;
    }
    sqlite3_stmt *stmt = nullptr;
    std::string sql = "INSERT INTO " + bandwidthTable +
                      " (iteration_id, stage_id, rank_id, op_name, op_suffix, transport_type,"
                      " bandwidth_size, bandwidth_utilization,large_package_ratio, size_distribution,"
                      " transit_size, transit_time) VALUES (?,?,?,?,?,?,?,?,?,?,?,?)";
    for (int i = 0; i < len - 1; i++) {
        sql.append(",(?,?,?,?,?,?,?,?,?,?,?,?)");
    }
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare InsertBandwidth statement. error:", sqlite3_errmsg(db));
    }
    return stmt;
}

void ClusterDatabase::ReleaseStmt()
{
    if (insertTimeInfoStmt != nullptr) {
        sqlite3_finalize(insertTimeInfoStmt);
    }
    if (insertBandwidthStmt != nullptr) {
        sqlite3_finalize(insertBandwidthStmt);
    }
}

void ClusterDatabase::SaveLastData()
{
    ServerLog::Info("SaveLastData ");
    if (!timeInfoCache.empty()) {
        InsertTimeInfoList(timeInfoCache);
        timeInfoCache.clear();
    }
    if (!bandwidthCache.empty()) {
        InsertBandwidthList(bandwidthCache);
        bandwidthCache.clear();
    }
}

bool ClusterDatabase::InsertTimeInfo(CommunicationTimeInfo &timeInfo)
{
    timeInfoCache.emplace_back(timeInfo);
    if (timeInfoCache.size() == cacheSize) {
        InsertTimeInfoList(timeInfoCache);
        timeInfoCache.clear();
    }
    return true;
}

bool ClusterDatabase::InsertTimeInfoList(std::vector<CommunicationTimeInfo> &timeInfoList)
{
    if (timeInfoList.empty()) {
        return false;
    }
    sqlite3_stmt *stmt = GetTimeInfoStmtSql(timeInfoList.size());
    if (stmt == nullptr) {
        ServerLog::Error("Failed to get timeInfo stmt.");
        return false;
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
        sqlite3_bind_double(stmt, idx++, timeInfo.elapseTime);
        sqlite3_bind_double(stmt, idx++, timeInfo.synchronizationTimeRatio);
        sqlite3_bind_double(stmt, idx++, timeInfo.synchronizationTime);
        sqlite3_bind_double(stmt, idx++, timeInfo.transitTime);
        sqlite3_bind_double(stmt, idx++, timeInfo.waitTimeRatio);
        sqlite3_bind_double(stmt, idx++, timeInfo.waitTime);
        sqlite3_bind_double(stmt, idx++, timeInfo.idleTime);
    }
    auto result = sqlite3_step(stmt);
    if (timeInfoList.size() != cacheSize) {
        sqlite3_finalize(stmt);
    }
    if (result != SQLITE_DONE) {
        ServerLog::Error("Insert timeInfo data fail. ", sqlite3_errmsg(db));
        return false;
    }
    return true;
}

bool ClusterDatabase::InsertBandwidth(CommunicationBandWidth &bandWidth)
{
    bandwidthCache.emplace_back(bandWidth);
    if (bandwidthCache.size() == cacheSize) {
        InsertBandwidthList(bandwidthCache);
        bandwidthCache.clear();
    }
    return true;
}

bool ClusterDatabase::InsertBandwidthList(std::vector<CommunicationBandWidth> &bandWidthList)
{
    if (bandWidthList.empty()) {
        return false;
    }
    sqlite3_stmt *stmt = GetBandwidthStmtSql(bandWidthList.size());
    if (stmt == nullptr) {
        ServerLog::Error("Failed to get timeInfo stmt.");
        return false;
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
    if (bandWidthList.size() != cacheSize) {
        sqlite3_finalize(stmt);
    }
    if (result != SQLITE_DONE) {
        ServerLog::Error("Insert bandwidth data fail. ", sqlite3_errmsg(db));
        return false;
    }
    return true;
}

bool ClusterDatabase::InsertStepStatisticsInfo(StepStatistic &stepStatistic)
{
    if (stepStmt == nullptr) {
        std::string sql = "INSERT INTO " + stepTraceTable +
                          "(rank_id, step_id, stage_id, compute_time,pure_communication_time,"
                          " overlap_communication_time,communication_time, free_time, stage_time,"
                          " bubble_time,pure_communication_exclude_receive_time) "
                          " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stepStmt, nullptr) != SQLITE_OK) {
            ServerLog::Error("Failed to prepare stepTraceTable statement. error:", sqlite3_errmsg(db));
        }
        if (stepStmt == nullptr) {
            ServerLog::Error("Failed to get stepTraceTable stmt.");
            return false;
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
    auto result = sqlite3_step(stepStmt);
    if (result != SQLITE_DONE) {
        ServerLog::Error("Insert bandwidth data fail. ", sqlite3_errmsg(db));
        return false;
    }
    return true;
}

bool ClusterDatabase::InsertClusterBaseInfo(ClusterBaseInfo &clusterBaseInfo)
{
    sqlite3_stmt *stmt;
    std::string sql = "INSERT INTO " + baseInfoTable +
                      "(file_path, ranks, steps, collect_start_time,collect_duration,data_size)"
                      " VALUES (?, (select json_group_array(rank_id) from "
                      "(select DISTINCT rank_id from step_statistic_info where rank_id !='')), "
                      "(select json_group_array(step_id) from"
                      " (select DISTINCT step_id from step_statistic_info where rank_id !='')) ,"
                      " ?, ?, ?)";
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare baseInfoTable statement. error:", sqlite3_errmsg(db));
    }
    if (stmt == nullptr) {
        ServerLog::Error("Failed to get baseInfoTable stmt.");
        return false;
    }
    int idx = bindStartIndex;
    sqlite3_bind_text(stmt, idx++, clusterBaseInfo.filePath.c_str(), clusterBaseInfo.filePath.length(),
                      SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, idx++, clusterBaseInfo.collectStartTime);
    sqlite3_bind_double(stmt, idx++, clusterBaseInfo.collectDuration);
    sqlite3_bind_double(stmt, idx++, clusterBaseInfo.dataSize);
    auto result = sqlite3_step(stmt);
    if (result != SQLITE_DONE) {
        ServerLog::Error("Insert baseInfoTable data fail. ", sqlite3_errmsg(db));
        return false;
    }
    sqlite3_finalize(stmt);
    return true;
}

bool ClusterDatabase::QuerySummaryData(const Protocol::SummaryTopRankParams &requestParams,
                                       Protocol::SummaryTopRankResBody &responseBody)
{
    sqlite3_stmt *stmt = BuildCondition(requestParams);
    if (stmt == nullptr) {
        return false;
    }
    std::vector<Protocol::SummaryDto> summaryDtoList;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        Protocol::SummaryDto summaryDto;
        summaryDto.rankId = sqlite3_column_string(stmt, col++);
        summaryDto.computingTime = sqlite3_column_double(stmt, col++);
        summaryDto.communicationNotOverLappedTime = sqlite3_column_double(stmt, col++);
        summaryDto.communicationOverLappedTime = sqlite3_column_double(stmt, col++);
        summaryDto.freeTime = sqlite3_column_double(stmt, col++);
        summaryDtoList.emplace_back(summaryDto);
    }
    responseBody.summaryList  = summaryDtoList;
    sqlite3_finalize(stmt);
    return true;
}

bool ClusterDatabase::QueryBaseInfo(Protocol::SummaryTopRankResBody &responseBody)
{
    sqlite3_stmt *stmtBaseInfo = nullptr;
    std::string baseInfoSql =
            "select file_path as filePath,ranks,steps,data_size as dataSize from " + baseInfoTable;
    int baseInfoResult = sqlite3_prepare_v2(db, baseInfoSql.c_str(), -1, &stmtBaseInfo, nullptr);
    if (baseInfoResult != SQLITE_OK) {
        ServerLog::Error("Query base info Failed to prepare sql.", sqlite3_errmsg(db));
        return false;
    }
    while (sqlite3_step(stmtBaseInfo) == SQLITE_ROW) {
        int coll = resultStartIndex;
        responseBody.filePath = sqlite3_column_string(stmtBaseInfo, coll++);
        std::string  ranks = sqlite3_column_string(stmtBaseInfo, coll++);
        if (!ranks.empty()) {
            json_t json = json_t::parse(ranks);
            responseBody.rankList = json.get<std::vector<std::string>>();
        }
        std::string steps = sqlite3_column_string(stmtBaseInfo, coll++);
        if (!steps.empty()) {
            json_t json = json_t::parse(steps);
            responseBody.stepList = json.get<std::vector<std::string>>();
        }
        responseBody.dataSize = sqlite3_column_double(stmtBaseInfo, coll++);
        responseBody.stepNum = responseBody.stepList.size();
        responseBody.rankCount = responseBody.rankList.size();
    }
    sqlite3_finalize(stmtBaseInfo);
    return true;
}

sqlite3_stmt *ClusterDatabase::BuildCondition(const Protocol::SummaryTopRankParams &requestParams)
{
    sqlite3_stmt *stmt = nullptr;
    int index = bindStartIndex;
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
        for (int i = 1; i < requestParams.rankIdList[0].size(); i++) {
            rankCondition.append(",?");
        }
        rankCondition.append(") ");
    }
    std::string sql = "SELECT rank_id as rankId, sum(ROUND(compute_time,2)) as computingTime,"
                      "sum(ROUND(pure_communication_time,2)) as communicationNotOverLappedTime,"
                      "sum(ROUND(overlap_communication_time,2)) as communicationOverLappedTime,"
                      "sum(ROUND(free_time,2)) as freeTime FROM " + stepTraceTable +
                      " WHERE rank_id !='' " + stepCondition + rankCondition
                      + "group by rank_id order by " + requestParams.orderBy + " desc";
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("BuildCondition. Failed to prepare sql.", sqlite3_errmsg(db));
        return stmt;
    }
    for (const auto &item: requestParams.stepIdList) {
        sqlite3_bind_text(stmt, index++, item.c_str(), -1, SQLITE_TRANSIENT);
    }
    for (const auto &item: requestParams.rankIdList) {
        sqlite3_bind_text(stmt, index++, item.c_str(), -1, SQLITE_TRANSIENT);
    }
    return stmt;
}

bool ClusterDatabase::QueryAllOperators(Protocol::OperatorDetailsParam &param,
                                        Protocol::OperatorDetailsResBody &resBody)
{
    sqlite3_stmt *stmt = nullptr;
    int index = bindStartIndex;
    std::string orderBy = param.orderBy.empty() ? "" : " order by " + param.orderBy + " " + param.order;
    std::string sql = "SELECT op_name,"
                      " ROUND(elapse_time, 4) as elapse_time, "
                      " ROUND(transit_time, 4) as transit_time,"
                      " ROUND(synchronization_time, 4) as synchronization_time,"
                      " ROUND(wait_time, 4) as wait_time,"
                      " ROUND(idle_time, 4) as idle_time,"
                      " ROUND(synchronization_time_ratio, 4) as synchronization_time_ratio,"
                      " ROUND(wait_time_ratio, 4) as wait_time_ratio "
                      "FROM ( SELECT * FROM " + timeInfoTable +
                      orderBy +
                      " ) WHERE iteration_id = ? AND rank_id = ? AND stage_id = ?"
                      " AND op_name != 'Total Op Info' LIMIT ?, ?";
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare QueryAllOperators statement. error:", sqlite3_errmsg(db));
        return false;
    }
    sqlite3_bind_text(stmt, index++, param.iterationId.c_str(), param.iterationId.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, index++, param.rankId.c_str(), param.rankId.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, index++, param.stage.c_str(), param.stage.length(), SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, index++, (param.currentPage - 1) * param.pageSize);
    sqlite3_bind_int(stmt, index,  param.pageSize);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        Protocol::OperatorItem operatorItem;
        operatorItem.operatorName = sqlite3_column_string(stmt, col++);
        operatorItem.elapseTime = sqlite3_column_double(stmt, col++);
        operatorItem.transitTime = sqlite3_column_double(stmt, col++);
        operatorItem.synchronizationTime = sqlite3_column_double(stmt, col++);
        operatorItem.waitTime = sqlite3_column_double(stmt, col++);
        operatorItem.idleTime = sqlite3_column_double(stmt, col++);
        operatorItem.synchronizationTimeRatio = sqlite3_column_double(stmt, col++);
        operatorItem.waitTimeRatio = sqlite3_column_double(stmt, col++);
        resBody.allOperators.emplace_back(operatorItem);
    }
    sqlite3_finalize(stmt);
    return true;
}

bool ClusterDatabase::QueryOperatorsCount(Protocol::OperatorDetailsParam &param,
                                          Protocol::OperatorDetailsResBody &resBody)
{
    sqlite3_stmt *stmt = nullptr;
    int index = bindStartIndex;
    std::string sql = "SELECT op_name, count(*) AS nums  from " + timeInfoTable
                      + " where 1=1 ";
    if (!param.iterationId.empty()) {
        sql.append("and iteration_id = ? ");
    }
    if (!param.rankId.empty()) {
        sql.append(" AND rank_id = ? ");
    }
    if (!param.stage.empty()) {
        sql.append(" AND stage_id = ? ");
    }
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare QueryOperatorsCount statement. error:", sqlite3_errmsg(db));
        return false;
    }
    sql.append(" group by op_name");
    if (!param.iterationId.empty()) {
        sqlite3_bind_text(stmt, index++, param.iterationId.c_str(), -1, SQLITE_TRANSIENT);
    }
    if (!param.rankId.empty()) {
        sqlite3_bind_text(stmt, index++, param.rankId.c_str(), -1, SQLITE_TRANSIENT);
    }
    if (!param.stage.empty()) {
        sqlite3_bind_text(stmt, index, param.stage.c_str(), -1, SQLITE_TRANSIENT);
    }
    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        std::string opName = sqlite3_column_string(stmt, col++);
        if (opName != "Total Op Info") {
            count = count + sqlite3_column_int(stmt, col);
        }
    }
    resBody.count = count;
    sqlite3_finalize(stmt);
    return true;
}

bool ClusterDatabase::QueryBandwidthData(Protocol::BandwidthDataParam &param, Protocol::BandwidthDataResBody &resBody)
{
    sqlite3_stmt *stmt = nullptr;
    int index = bindStartIndex;
    std::string sql = "SELECT transport_type,ROUND(transit_size, 4) as transit_size,"
                      "ROUND(transit_time, 4) as transit_time,"
                      "ROUND(bandwidth_size, 4) as bandwidth_size,"
                      "ROUND(large_package_ratio, 4)  as large_package_ratio from "
                      + bandwidthTable +
                      " WHERE iteration_id = ? AND rank_id = ? AND stage_id = ? AND op_name = ? ";
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare QueryBandwidthData statement. error:", sqlite3_errmsg(db));
        return false;
    }
    sqlite3_bind_text(stmt, index++, param.iterationId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, index++, param.rankId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, index++, param.stage.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, index++, param.operatorName.c_str(), -1, SQLITE_TRANSIENT);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        Protocol::BandwidthDataItem bandwidth;
        bandwidth.transportType = sqlite3_column_string(stmt, col++);
        bandwidth.transitSize = sqlite3_column_double(stmt, col++);
        bandwidth.transitTime = sqlite3_column_double(stmt, col++);
        bandwidth.bandwidth = sqlite3_column_double(stmt, col++);
        bandwidth.largePacketRatio = sqlite3_column_double(stmt, col++);
        resBody.items.emplace_back(bandwidth);
    }
    sqlite3_finalize(stmt);
    return true;
}

bool ClusterDatabase::QueryDistributionData(Protocol::DistributionDataParam &param,
                                            Protocol::DistributionResBody &resBody)
{
    sqlite3_stmt *stmt = nullptr;
    int index = bindStartIndex;
    std::string sql = "SELECT size_distribution FROM "
                      + bandwidthTable +
                      " WHERE iteration_id = ? "
                      "AND rank_id = ? "
                      "AND stage_id = ? "
                      "AND op_name = ? "
                      "AND transport_type = ? ;";
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare QueryDistributionData statement. error:", sqlite3_errmsg(db));
        return false;
    }
    sqlite3_bind_text(stmt, index++, param.iterationId.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, index++, param.rankId.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, index++, param.stage.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, index++, param.operatorName.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, index++, param.transportType.c_str(), -1, SQLITE_STATIC);
    resBody.distributionData = "";
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        resBody.distributionData = sqlite3_column_string(stmt, col);
    }
    sqlite3_finalize(stmt);
    return true;
}

bool ClusterDatabase::QueryRanksHandler(Protocol::RanksParams &requestParam,
                                        std::vector<Protocol::IterationsOrRanksObject> &responseBody)
{
    sqlite3_stmt *stmt = nullptr;
    int index = bindStartIndex;
    std::string sql = "SELECT DISTINCT rank_id FROM " + timeInfoTable + " WHERE iteration_id = ? ORDER BY rank_id";
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Failed to prepare QueryRanksHandler statement. error:", sqlite3_errmsg(db));
        return false;
    }
    sqlite3_bind_text(stmt, index++, requestParam.iterationId.c_str(), -1, SQLITE_TRANSIENT);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        Protocol::IterationsOrRanksObject object;
        object.iterationOrRankId = sqlite3_column_string(stmt, col++);
        responseBody.emplace_back(object);
    }
    return true;
}

std::string ClusterDatabase::GetRanksSql(std::vector<std::string> rankList)
{
    std::string ranks = "(";
    if (rankList.empty()) {
        return "";
    } else {
        for (int i = 0; i < rankList.size(); i++) {
            if (i == rankList.size() - 1) {
                ranks += rankList[i];
            } else {
                ranks += rankList[i];
                ranks += ", ";
            }
        }
    }
    ranks += ")";
    return ranks;
}

bool ClusterDatabase::QueryOperatorNames(Protocol::OperatorNamesParams &requestParams,
                                         std::vector<Protocol::OperatorNamesObject> &responseBody)
{
    sqlite3_stmt *stmt = nullptr;
    int index = bindStartIndex;
    std::vector<std::string> rankList = requestParams.rankList;
    std::string iterationId = requestParams.iterationId;
    std::string stage = requestParams.stage;
    std::string sql = "";
    if (rankList.size() == 0) {
        sql = "SELECT DISTINCT op_name FROM (SELECT op_name FROM " + timeInfoTable +
                " WHERE iteration_id = ?" +
                " AND stage_id = ?" +
                " ORDER BY op_name)";
    } else {
        std::string ranks = GetRanksSql(rankList);
        sql = "SELECT DISTINCT op_name FROM (SELECT op_name FROM " + timeInfoTable +
                " WHERE iteration_id = ?" +
                " AND stage_id = ?" +
                " AND rank_id IN " + ranks + " ORDER BY op_name)";
    }
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Failed to prepare QueryOperatorNames statement. error:", sqlite3_errmsg(db));
        return false;
    }
    sqlite3_bind_text(stmt, index++, iterationId.c_str(), iterationId.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, index++, stage.c_str(), stage.length(), SQLITE_TRANSIENT);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        Protocol::OperatorNamesObject object;
        object.operatorName = sqlite3_column_string(stmt, col++);
        responseBody.emplace_back(object);
    }
    return true;
}

bool ClusterDatabase::QueryIterations(std::vector<Protocol::IterationsOrRanksObject> &responseBody)
{
    sqlite3_stmt *stmt = nullptr;
    int index = bindStartIndex;
    std::string sql = "SELECT DISTINCT iteration_id FROM " + timeInfoTable + " ORDER BY iteration_id";
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Failed to prepare QueryIterations statement. error:", sqlite3_errmsg(db));
        return false;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        Protocol::IterationsOrRanksObject object;
        object.iterationOrRankId = sqlite3_column_string(stmt, col++);
        responseBody.emplace_back(object);
    }
    if (responseBody.size() == 0) {
        ServerLog::Error("Failed to obtain the number of iteration ids. At least one id must be contained. "
                     "Check whether communication data files exist in the directory.");
    }
    return true;
}

bool ClusterDatabase::QueryDurationList(Protocol::DurationListParams &requestParams,
                                        std::vector<Protocol::Duration> &responseBody)
{
    sqlite3_stmt *stmt = nullptr;
    int index = bindStartIndex;
    std::vector<std::string> rankList = requestParams.rankList;
    std::string iterationId = requestParams.iterationId;
    std::string stage = requestParams.stage;
    std::string operatorName = requestParams.operatorName;
    std::string sql = "";
    if (rankList.size() == 0) {
        sql = "SELECT rank_id, ROUND(elapse_time, 4) as elapse_time, ROUND(transit_time, 4) as transit_time, "
              "ROUND(synchronization_time, 4) as synchronization_time, ROUND(wait_time, 4) as wait_time, "
              "ROUND(idle_time, 4) as idle_time, ROUND(synchronization_time_ratio, 4) as synchronization_time_ratio, "
              "ROUND(wait_time_ratio, 4) as wait_time_ratio FROM " + timeInfoTable +
              " WHERE iteration_id = ? AND stage_id = ? AND op_name = ?";
    } else {
        std::string ranks = GetRanksSql(rankList);
        sql = "SELECT rank_id, ROUND(elapse_time, 4) as elapse_time, ROUND(transit_time, 4) as transit_time, "
              "ROUND(synchronization_time, 4) as synchronization_time, ROUND(wait_time, 4) as wait_time, "
              "ROUND(idle_time, 4) as idle_time, ROUND(synchronization_time_ratio, 4) as synchronization_time_ratio, "
              "ROUND(wait_time_ratio, 4) as wait_time_ratio FROM " + timeInfoTable +
              " WHERE iteration_id = ? AND stage_id = ?"
              " AND rank_id IN " + ranks +
              " AND op_name = ?";
    }
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Failed to prepare Query Duration List statement. error:", sqlite3_errmsg(db));
        return false;
    }
    sqlite3_bind_text(stmt, index++, iterationId.c_str(), iterationId.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, index++, stage.c_str(), stage.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, index++, operatorName.c_str(), operatorName.length(), SQLITE_TRANSIENT);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        Protocol::Duration object;
        object.rankId = sqlite3_column_string(stmt, col++);
        object.elapseTime = sqlite3_column_double(stmt, col++);
        object.transitTime = sqlite3_column_double(stmt, col++);
        object.synchronizationTime = sqlite3_column_double(stmt, col++);
        object.waitTime = sqlite3_column_double(stmt, col++);
        object.idleTime = sqlite3_column_double(stmt, col++);
        object.synchronizationTimeRatio = sqlite3_column_double(stmt, col++);
        object.waitTimeRatio = sqlite3_column_double(stmt, col++);
        responseBody.emplace_back(object);
    }
    return true;
}
} // end of namespace Module
} // end of namespace Dic