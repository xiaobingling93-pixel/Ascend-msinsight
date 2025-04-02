/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <map>
#include <ConstantDefs.h>
#include "pch.h"
#include "NumDefs.h"
#include "VirtualClusterDatabase.h"

namespace Dic {
namespace Module {
using namespace Dic::Server;
bool VirtualClusterDatabase::HasColumn(const std::string &tableName, const std::string &columnName)
{
    if (!Database::CheckTableExist(tableName)) {
        ServerLog::Error("Failed to check table: ", tableName);
        return false;
    }
    std::string sql = "PRAGMA table_info(" + tableName + ")";
    sqlite3_stmt *stmt;
    bool result = false;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare statement. error:", sqlite3_errmsg(db));
        return result;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *name = sqlite3_column_text(stmt, 1);
        if (columnName == reinterpret_cast<const char *>(name)) {
            result = true;
            break;
        }
    }
    sqlite3_finalize(stmt);

    return result;
}

bool VirtualClusterDatabase::ExecuteQueryBaseInfo(Protocol::SummaryBaseInfo &baseInfo, std::string sql)
{
    sqlite3_stmt *stmtBaseInfo = nullptr;
    int baseInfoResult = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmtBaseInfo, nullptr);
    if (baseInfoResult != SQLITE_OK) {
        ServerLog::Error("Failed to prepare Query base info statement. error:", sqlite3_errmsg(db));
        return false;
    }
    std::map<std::string, std::string> info;
    while (sqlite3_step(stmtBaseInfo) == SQLITE_ROW) {
        int coll = resultStartIndex;
        std::string key = sqlite3_column_string(stmtBaseInfo, coll++);
        std::string value = sqlite3_column_string(stmtBaseInfo, coll++);
        info.insert({key, value});
    }
    std::string valueRanks = FindValueByKey(info, "ranks", "");
    std::string valueSteps = FindValueByKey(info, "steps", "");
    std::string valueDataSize = FindValueByKey(info, "data_size", "");
    if (!valueRanks.empty()) {
        baseInfo.rankList = JsonUtil::JsonToVector(valueRanks);
    }
    if (!valueSteps.empty()) {
        baseInfo.stepList = JsonUtil::JsonToVector(valueSteps);
    }
    baseInfo.dataSize = NumberUtil::StringToDouble(valueDataSize) / MB_SIZE;
    baseInfo.stepNum = NumberUtil::CeilingClamp(baseInfo.stepList.size(), static_cast<size_t>(INT_MAX));
    baseInfo.rankCount = NumberUtil::CeilingClamp(baseInfo.rankList.size(), static_cast<size_t>(INT_MAX));
    sqlite3_finalize(stmtBaseInfo);
    return true;
}

bool VirtualClusterDatabase::ExecuteGetStepIdList(Protocol::PipelineStepResponseBody &responseBody, std::string sql)
{
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare get step id list statement. error:", sqlite3_errmsg(db));
        return false;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        std::string res = sqlite3_column_string(stmt, col++);
        responseBody.stepList.emplace_back(res);
    }
    sqlite3_finalize(stmt);
    return true;
}

bool VirtualClusterDatabase::ExecuteGetStages(Protocol::PipelineStageParam param,
    Protocol::PipelineStageResponseBody &responseBody, std::string sql)
{
    sqlite3_stmt *stmt = nullptr;
    int index = bindStartIndex;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare get stages statement. error:", sqlite3_errmsg(db));
        return false;
    }
    sqlite3_bind_text(stmt, index, param.stepId.c_str(), param.stepId.length(), SQLITE_TRANSIENT);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        std::string res = sqlite3_column_string(stmt, col++);
        responseBody.stageList.emplace_back(res);
    }
    sqlite3_finalize(stmt);
    return true;
}

bool VirtualClusterDatabase::ExecuteGetStageAndBubble(Protocol::PipelineStageTimeParam param,
                                                      std::vector<std::string> stageIds,
                                                      Protocol::PipelineStageOrRankTimeResponseBody &responseBody,
                                                      std::string sql)
{
    sqlite3_stmt *stmt = nullptr;
    int index = bindStartIndex;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare get stage and bubble statement. error:", sqlite3_errmsg(db));
        return false;
    }
    sqlite3_bind_text(stmt, index++, param.stepId.c_str(), param.stepId.length(), SQLITE_TRANSIENT);
    for (size_t i = 0; i < stageIds.size(); ++i) {
        sqlite3_bind_text(stmt, index++, stageIds[i].c_str(), stageIds[i].length(), SQLITE_TRANSIENT);
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        Protocol::BubbleDetail bubbleDetail;
        bubbleDetail.stageOrRankId = param.stageId;
        bubbleDetail.stageTime = sqlite3_column_double(stmt, col++);
        bubbleDetail.bubbleTime = sqlite3_column_double(stmt, col++);
        responseBody.bubbleDetails.emplace_back(bubbleDetail);
    }
    sqlite3_finalize(stmt);
    return true;
}

bool VirtualClusterDatabase::ExecuteGetRankAndBubble(const Protocol::PipelineRankTimeParam &param,
                                                     std::vector<std::string> &&stageIds,
                                                     Protocol::PipelineStageOrRankTimeResponseBody &responseBody,
                                                     std::string &&sql)
{
    int index = bindStartIndex;
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare get rank and bubble statement. error:", sqlite3_errmsg(db));
        return false;
    }
    sqlite3_bind_text(stmt, index++, param.stepId.c_str(), param.stepId.length(), SQLITE_TRANSIENT);
    for (size_t i = 0; i < stageIds.size(); ++i) {
        sqlite3_bind_text(stmt, index++, stageIds[i].c_str(), stageIds[i].length(), SQLITE_TRANSIENT);
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        Protocol::BubbleDetail bubbleDetail;
        bubbleDetail.stageOrRankId = sqlite3_column_string(stmt, col++);
        bubbleDetail.stageTime = sqlite3_column_double(stmt, col++);
        bubbleDetail.bubbleTime = sqlite3_column_double(stmt, col++);
        responseBody.bubbleDetails.emplace_back(bubbleDetail);
    }
    sqlite3_finalize(stmt);
    return true;
}

bool VirtualClusterDatabase::ExecuteGetGroups(const std::string &iterationId,
    std::vector<std::string> &groupList, std::string sql)
{
    sqlite3_stmt *stmt = nullptr;
    int index = bindStartIndex;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare get groups statement. error:", sqlite3_errmsg(db));
        return false;
    }
    sqlite3_bind_text(stmt, index, iterationId.c_str(), iterationId.length(), SQLITE_TRANSIENT);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        std::string res = sqlite3_column_string(stmt, col++);
        groupList.emplace_back(res);
    }
    sqlite3_finalize(stmt);
    return true;
}

bool VirtualClusterDatabase::ExecuteQueryMatrixList(Protocol::MatrixBandwidthParam &param,
    std::vector<MatrixInfoDo> &matrixInfoDoList, const std::string &sql)
{
    int index = bindStartIndex;
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare query matrix list statement. error:", sqlite3_errmsg(db));
        return false;
    }
    sqlite3_bind_text(stmt, index++, param.stage.c_str(), param.stage.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, index++, param.iterationId.c_str(), param.iterationId.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, index, param.operatorName.c_str(), param.operatorName.length(), SQLITE_TRANSIENT);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        MatrixInfoDo matrixInfoDo;
        matrixInfoDo.srcRank = sqlite3_column_int(stmt, col++);
        matrixInfoDo.dstRank = sqlite3_column_int(stmt, col++);
        matrixInfoDo.transportType = sqlite3_column_string(stmt, col++);
        matrixInfoDo.transitSize = sqlite3_column_double(stmt, col++);
        matrixInfoDo.transitTime = sqlite3_column_double(stmt, col++);
        matrixInfoDo.bandwidth = sqlite3_column_double(stmt, col++);
        matrixInfoDo.opName = sqlite3_column_string(stmt, col++);
        matrixInfoDoList.emplace_back(matrixInfoDo);
    }
    sqlite3_finalize(stmt);
    return true;
}

bool VirtualClusterDatabase::ExecuteQueryExtremumTimestamp(std::string &sql, uint64_t &min, uint64_t &max)
{
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare query extremum timestamp statement. error:", sqlite3_errmsg(db));
        return false;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        int64_t tempMin = sqlite3_column_int64(stmt, col++);
        if (tempMin <= 0) {
            min = 0;
        } else {
            min = tempMin;
        }
        int64_t tempMax = sqlite3_column_int64(stmt, col++);
        if (tempMax <= 0) {
            max = 0;
        } else {
            max = tempMax;
        }
    }
    sqlite3_finalize(stmt);
    return true;
}

bool VirtualClusterDatabase::ExecuteQueryIterationAndCommunicationGroup(std::string &sql,
    std::string &opName, const std::string &rankId, std::string &iteration, std::string &communicationGroup)
{
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare query iteration and communication group statement. error:",
            sqlite3_errmsg(db));
        return false;
    }
    int index = bindStartIndex;
    sqlite3_bind_text(stmt, index++, opName.c_str(), opName.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, index, rankId.c_str(), rankId.length(), SQLITE_TRANSIENT);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        iteration = sqlite3_column_type(stmt, col) == SQLITE_NULL ? "" : sqlite3_column_string(stmt, col++);
        communicationGroup = sqlite3_column_type(stmt, col) == SQLITE_NULL ? "" : sqlite3_column_string(stmt, col++);
    }
    sqlite3_finalize(stmt);
    return true;
}

bool VirtualClusterDatabase::ExecuteQueryAllOperators(Protocol::OperatorDetailsParam &param,
    Protocol::OperatorDetailsResBody &resBody, std::string sql, uint64_t startTime)
{
    sqlite3_stmt *stmt = nullptr;
    std::vector<std::string> orderByFlagVector = {"operatorName", "startTime", "elapseTime", "synchronizationTime",
                                                  "waitTime", "idleTime", "transitTime", "sdmaBw", "rdmaBw",
                                                  "synchronizationTimeRatio", "waitTimeRatio"};
    if (param.orderBy.empty() ||
        std::find(orderByFlagVector.begin(), orderByFlagVector.end(), param.orderBy) ==
        orderByFlagVector.end()) {
        param.orderBy = "elapseTime";
    }
    std::string orderBy = " order by " + param.orderBy;
    std::string order = !param.order.empty() && std::strcmp(param.order.c_str(), "ascend") == 0 ? "ASC" : "DESC";
    sql +=  orderBy + " " + order + " LIMIT ?, ?";
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare query all operators statement. error:", sqlite3_errmsg(db));
        return false;
    }
    int index = bindStartIndex;
    sqlite3_bind_int64(stmt, index++, NumberUtil::CeilingClamp(startTime, static_cast<uint64_t>(INT64_MAX)));
    sqlite3_bind_text(stmt, index++, param.iterationId.c_str(), param.iterationId.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, index++, param.rankId.c_str(), param.rankId.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, index++, param.stage.c_str(), param.stage.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, index++, param.iterationId.c_str(), param.iterationId.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, index++, param.rankId.c_str(), param.rankId.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, index++, param.stage.c_str(), param.stage.length(), SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, index++, (param.currentPage - 1) * param.pageSize);
    sqlite3_bind_int(stmt, index, param.pageSize);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        Protocol::OperatorItem operatorItem;
        operatorItem.operatorName = sqlite3_column_string(stmt, col++);
        operatorItem.startTime = sqlite3_column_double(stmt, col++);
        operatorItem.elapseTime = sqlite3_column_double(stmt, col++);
        operatorItem.transitTime = sqlite3_column_double(stmt, col++);
        operatorItem.synchronizationTime = sqlite3_column_double(stmt, col++);
        operatorItem.waitTime = sqlite3_column_double(stmt, col++);
        operatorItem.idleTime = sqlite3_column_double(stmt, col++);
        operatorItem.synchronizationTimeRatio = sqlite3_column_double(stmt, col++);
        operatorItem.waitTimeRatio = sqlite3_column_double(stmt, col++);
        operatorItem.sdmaBw = sqlite3_column_double(stmt, col++);
        operatorItem.rdmaBw = sqlite3_column_double(stmt, col++);
        resBody.allOperators.emplace_back(operatorItem);
    }
    sqlite3_finalize(stmt);
    return true;
}

bool VirtualClusterDatabase::ExecuteQueryOperatorsCount(Protocol::OperatorDetailsParam &param,
    Protocol::OperatorDetailsResBody &resBody, std::string sql)
{
    sqlite3_stmt *stmt = nullptr;
    int index = bindStartIndex;
    if (!param.iterationId.empty()) {
        sql.append("and iteration_id = ? ");
    }
    if (!param.rankId.empty()) {
        sql.append(" AND rank_id = ? ");
    }
    if (!param.stage.empty()) {
        sql.append(" AND stage_id = ? ");
    }
    sql.append(" group by op_name");
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare query operators count statement. error:", sqlite3_errmsg(db));
        return false;
    }
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

bool VirtualClusterDatabase::ExecuteQueryBandwidthData(Protocol::BandwidthDataParam &param,
    Protocol::BandwidthDataResBody &resBody, std::string sql)
{
    sqlite3_stmt *stmt = nullptr;
    int index = bindStartIndex;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare query bandwidth data statement. error:", sqlite3_errmsg(db));
        return false;
    }
    sqlite3_bind_text(stmt, index++, param.iterationId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, index++, param.rankId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, index++, param.stage.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, index, param.operatorName.c_str(), -1, SQLITE_TRANSIENT);
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

bool VirtualClusterDatabase::ExecuteQueryDistributionData(Protocol::DistributionDataParam &param,
    Protocol::DistributionResBody &resBody, std::string sql)
{
    sqlite3_stmt *stmt = nullptr;
    int index = bindStartIndex;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare distribution data statement. error:", sqlite3_errmsg(db));
        return false;
    }
    sqlite3_bind_text(stmt, index++, param.iterationId.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, index++, param.rankId.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, index++, param.stage.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, index++, param.operatorName.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, index, param.transportType.c_str(), -1, SQLITE_STATIC);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        resBody.distributionData = sqlite3_column_string(stmt, col);
    }
    sqlite3_finalize(stmt);
    return true;
}

bool VirtualClusterDatabase::ExecuteQueryRanksHandler(std::vector<Protocol::IterationsOrRanksObject> &responseBody,
    std::string sql)
{
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Failed to prepare query ranks statement. error:", sqlite3_errmsg(db));
        return false;
    }
    std::map<std::string, std::string> info;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        std::string key = sqlite3_column_string(stmt, col++);
        std::string value = sqlite3_column_string(stmt, col++);
        info.insert({key, value});
    }
    std::string valueRanks = FindValueByKey(info, "ranks", "");
    GetStepsOrRanksObject(valueRanks, responseBody);
    sqlite3_finalize(stmt);
    return true;
}

bool VirtualClusterDatabase::ExecuteQueryOperatorNames(Protocol::OperatorNamesParams &requestParams,
    std::vector<Protocol::OperatorNamesObject> &responseBody, std::string sql)
{
    sqlite3_stmt *stmt = nullptr;
    int index = bindStartIndex;
    std::string iterationId = requestParams.iterationId;
    std::string stage = requestParams.stage;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Failed to prepare query operator names statement. error:", sqlite3_errmsg(db));
        return false;
    }
    sqlite3_bind_text(stmt, index++, iterationId.c_str(), iterationId.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, index, stage.c_str(), stage.length(), SQLITE_TRANSIENT);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        Protocol::OperatorNamesObject object;
        object.operatorName = sqlite3_column_string(stmt, col++);
        responseBody.emplace_back(object);
    }
    sqlite3_finalize(stmt);
    return true;
}

bool VirtualClusterDatabase::ExecuteQueryIterations(std::vector<Protocol::IterationsOrRanksObject> &responseBody,
    std::string sql)
{
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Failed to prepare query iterations statement. error:", sqlite3_errmsg(db));
        return false;
    }
    std::map<std::string, std::string> info;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        std::string key = sqlite3_column_string(stmt, col++);
        std::string value = sqlite3_column_string(stmt, col++);
        info.insert({key, value});
    }
    std::string valueSteps = FindValueByKey(info, "steps", "");
    GetStepsOrRanksObject(valueSteps, responseBody);
    if (responseBody.empty()) {
        ServerLog::Warn("Failed to obtain the number of iteration ids. At least one id must be contained. "
                         "Check whether communication data files exist in the directory.");
    }
    sqlite3_finalize(stmt);
    return true;
}

bool VirtualClusterDatabase::ExecuteQueryDurationList(Protocol::DurationListParams &requestParams,
    std::vector<DurationDo> &durationDoList, std::string sql, uint64_t startTime)
{
    sqlite3_stmt *stmt = nullptr;
    int index = bindStartIndex;
    std::vector<std::string> rankList = requestParams.rankList;
    std::string iterationId = requestParams.iterationId;
    std::string stage = requestParams.stage;
    std::string operatorName = requestParams.operatorName;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Failed to prepare Query Duration List statement. error:", sqlite3_errmsg(db));
        return false;
    }
    sqlite3_bind_int64(stmt, index++, NumberUtil::CeilingClamp(startTime, static_cast<uint64_t>(INT64_MAX)));
    sqlite3_bind_text(stmt, index++, iterationId.c_str(), iterationId.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, index++, stage.c_str(), stage.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, index++, operatorName.c_str(), operatorName.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, index++, iterationId.c_str(), iterationId.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, index++, stage.c_str(), stage.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, index, operatorName.c_str(), operatorName.length(), SQLITE_TRANSIENT);
    std::vector<Protocol::BandwidthStatistic> bwStat = {{"SDMA", 0, 0, DBL_MAX, 0, 0}, {"RDMA", 0, 0, DBL_MAX, 0, 0}};
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        DurationDo object;
        object.rankId = sqlite3_column_string(stmt, col++);
        object.startTime = sqlite3_column_double(stmt, col++);
        object.elapseTime = sqlite3_column_double(stmt, col++);
        object.transitTime = sqlite3_column_double(stmt, col++);
        object.synchronizationTime = sqlite3_column_double(stmt, col++);
        object.waitTime = sqlite3_column_double(stmt, col++);
        object.idleTime = sqlite3_column_double(stmt, col++);
        object.synchronizationTimeRatio = sqlite3_column_double(stmt, col++);
        object.waitTimeRatio = sqlite3_column_double(stmt, col++);
        object.sdmaBw = sqlite3_column_double(stmt, col++);
        object.rdmaBw = sqlite3_column_double(stmt, col++);
        object.sdmaTime = sqlite3_column_double(stmt, col++);
        object.rdmaTime = sqlite3_column_double(stmt, col++);
        durationDoList.emplace_back(object);
    }
    sqlite3_finalize(stmt);
    return true;
}

bool VirtualClusterDatabase::ExecuteQueryOperatorList(Protocol::DurationListParams &requestParams,
    std::vector<OperatorTimeDo> &operatorTimeDoList, const std::string& sql, uint64_t startTime)
{
    sqlite3_stmt *stmt = nullptr;
    int index = bindStartIndex;
    std::string iterationId = requestParams.iterationId;
    std::string stage = requestParams.stage;
    std::string operatorName = requestParams.operatorName;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Failed to prepare Query Operator List statement. error:", sqlite3_errmsg(db));
        return false;
    }
    sqlite3_bind_int64(stmt, index++, NumberUtil::CeilingClamp(startTime, static_cast<uint64_t>(INT64_MAX)));
    sqlite3_bind_text(stmt, index++, iterationId.c_str(), iterationId.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, index++, stage.c_str(), stage.length(), SQLITE_TRANSIENT);
    if (requestParams.operatorName != totalOpInfo) {
        sqlite3_bind_text(stmt, index, operatorName.c_str(), operatorName.length(), SQLITE_TRANSIENT);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        OperatorTimeDo object{};
        object.rankId = sqlite3_column_string(stmt, col++);
        object.operatorName = sqlite3_column_string(stmt, col++);
        int64_t tempStartTime = sqlite3_column_int64(stmt, col++);
        if (tempStartTime <= 0) {
            object.startTime = 0;
        } else {
            object.startTime = tempStartTime;
        }
        int64_t tempElapseTime = sqlite3_column_int64(stmt, col++);
        if (tempElapseTime <= 0) {
            object.elapseTime = 0;
        } else {
            object.elapseTime = tempElapseTime;
        }
        operatorTimeDoList.push_back(object);
    }
    sqlite3_finalize(stmt);
    return true;
}

void VirtualClusterDatabase::GetStepsOrRanksObject(const std::string &jsonStr,
    std::vector<Protocol::IterationsOrRanksObject> &responseBody)
{
    rapidjson::Document json;
    json.Parse(jsonStr.c_str());
    if (!json.IsArray()) {
        return;
    }
    for (auto &item : json.GetArray()) {
        Protocol::IterationsOrRanksObject object;
        object.iterationOrRankId = item.IsString() ? item.GetString() : "";
        responseBody.emplace_back(object);
    }
}

bool VirtualClusterDatabase::ExecuteQueryMatrixSortOpNames(Protocol::OperatorNamesParams &requestParams,
    std::vector<Protocol::OperatorNamesObject> &responseBody, std::string sql)
{
    sqlite3_stmt *stmt = nullptr;
    int index = bindStartIndex;
    std::string iterationId = requestParams.iterationId;
    std::string stage = requestParams.stage;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Failed to prepare Query Matrix Sort OpNames statement. error: ", sqlite3_errmsg(db));
        return false;
    }
    sqlite3_bind_text(stmt, index++, iterationId.c_str(), iterationId.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, index, stage.c_str(), stage.length(), SQLITE_TRANSIENT);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        Protocol::OperatorNamesObject object;
        object.operatorName = sqlite3_column_string(stmt, col++);
        responseBody.emplace_back(object);
    }
    sqlite3_finalize(stmt);
    return true;
}

std::string VirtualClusterDatabase::GetRanksSql(const std::vector<std::string> &rankList)
{
    std::string ranks = "(";
    if (rankList.empty()) {
        return "";
    } else {
        for (size_t i = 0; i < rankList.size(); i++) {
            if (!StringUtil::CheckSqlValid(rankList[i])) {
                ServerLog::Error("There is an SQL injection attack on this parameter. error param: ", rankList[i]);
                return "";
            }

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

bool VirtualClusterDatabase::ExecuteQueryParallelStrategyConfig(std::string &sql,
    ParallelStrategyConfig &config, std::string &level)
{
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare query parallel strategy config statement. error:", sqlite3_errmsg(db));
        return false;
    }
    std::map<std::string, std::string> info;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        std::string key = sqlite3_column_string(stmt, col++);
        std::string value = sqlite3_column_string(stmt, col++);
        info.insert({key, value});
    }
    std::string valueAlgorithm = FindValueByKey(info, "algorithm", "");
    std::string valueDpSize = FindValueByKey(info, "dp_size", "");
    std::string valuePpSize = FindValueByKey(info, "pp_size", "");
    std::string valueTpSize = FindValueByKey(info, "tp_size", "");
    std::string valueCpSize = FindValueByKey(info, "cp_size", "");
    std::string valueEpSize = FindValueByKey(info, "ep_size", "");
    std::string valueLevel = FindValueByKey(info, "level", "");
    config.algorithm = valueAlgorithm;
    config.dpSize = NumberUtil::StringToLongLong(valueDpSize);
    config.ppSize = NumberUtil::StringToLongLong(valuePpSize);
    config.tpSize = NumberUtil::StringToLongLong(valueTpSize);
    config.cpSize = NumberUtil::StringToLongLong(valueCpSize);
    config.epSize = NumberUtil::StringToLongLong(valueEpSize);
    level = valueLevel;
    sqlite3_finalize(stmt);
    return true;
}

bool VirtualClusterDatabase::ExecuteSetParallelStrategyConfig(std::string &sql,
    const ParallelStrategyConfig &config, std::string &level)
{
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare set parallel strategy config statement. error:", sqlite3_errmsg(db));
        return false;
    }
    std::unique_lock<std::recursive_mutex> lock(mutex);
    int index = bindStartIndex;
    std::string stringDpSize = std::to_string(config.dpSize);
    std::string stringPpSize = std::to_string(config.ppSize);
    std::string stringTpSize = std::to_string(config.tpSize);
    std::string stringCpSize = std::to_string(config.cpSize);
    std::string stringEpSize = std::to_string(config.epSize);
    sqlite3_bind_text(stmt, index++, config.algorithm.c_str(), config.algorithm.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, index++, stringDpSize.c_str(), stringDpSize.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, index++, stringPpSize.c_str(), stringPpSize.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, index++, stringTpSize.c_str(), stringTpSize.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, index++, stringCpSize.c_str(), stringCpSize.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, index++, stringEpSize.c_str(), stringEpSize.length(), SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, index++, level.c_str(), level.length(), SQLITE_TRANSIENT);
    auto result = sqlite3_step(stmt);
    if (result != SQLITE_DONE) {
        ServerLog::Error("Fail to update parallel strategy config. ", sqlite3_errmsg(db));
        return false;
    }
    sqlite3_finalize(stmt);
    return true;
}

bool VirtualClusterDatabase::ExecuteGetParallelConfigFromStepTrace(std::string &sql,
    ParallelStrategyConfig &config, std::string &level)
{
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK || stmt == nullptr) {
        ServerLog::Error("Failed to prepare get parallel config statement. error:", sqlite3_errmsg(db));
        return false;
    }
    bool flag = false;
    int64_t ppSize{};
    int64_t dpSize{};
    int64_t prePpIndex{};
    int64_t preDpIndex{};
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        dpSize = sqlite3_column_int64(stmt, col++);
        ppSize = sqlite3_column_int64(stmt, col++);
        config.tpSize = std::max(config.tpSize, (int64_t)sqlite3_column_int64(stmt, col++));
        config.dpSize = std::max(config.dpSize, dpSize);
        config.ppSize = std::max(config.ppSize, ppSize);
        // 通过判断dp和pp哪个先增加，来判断tp-dp-pp还是tp-pp-dp
        if (!flag) {
            if (dpSize > preDpIndex) {
                config.algorithm = MEGATRON_LM_TP_CP_EP_DP_PP_ALG;
                flag = true;
            } else if (ppSize > prePpIndex) {
                config.algorithm = MEGATRON_LM_TP_CP_PP_EP_DP_ALG;
                flag = true;
            }
            preDpIndex = dpSize;
            prePpIndex = ppSize;
        }
    }
    if (config.dpSize <= 0 && config.ppSize <= 0 && config.tpSize <= 0) {
        level = PARALLEL_CONFIG_LEVEL_UNDEFINED;
    } else {
        level = PARALLEL_CONFIG_LEVEL_COLLECTED;
    }
    // 索引从0开始，转化为实际大小时需要+1
    config.dpSize++;
    config.ppSize++;
    config.tpSize++;
    sqlite3_finalize(stmt);
    return true;
}

bool VirtualClusterDatabase::HasFinishedParseLastTime()
{
    return CheckValueFromStatusInfoTable(clusterParseStatus, FINISH_STATUS);
}

bool VirtualClusterDatabase::UpdatesClusterParseStatus(const std::string &status)
{
    return UpdateValueIntoStatusInfoTable(clusterParseStatus, status);
}

bool VirtualClusterDatabase::ExecuteQueryAllPerformanceDataByStep(const std::string &sql,
    const std::string &step, std::unordered_map<std::uint32_t, StepStatistic> &data)
{
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Failed to prepare sql for query all performance data by step.");
        return false;
    }
    if (!step.empty() && step != "All") {
        stmt->BindParams(step);
    }
    auto resultSet = stmt->ExecuteQuery();
    if (resultSet == nullptr) {
        ServerLog::Error("Failed to get result set for query all performance data by step.");
        return false;
    }
    while (resultSet->Next()) {
        StepStatistic one{};
        one.rankId = resultSet->GetString("rank");
        one.computingTime = resultSet->GetDouble("compute");
        one.pureCommunicationTime = resultSet->GetDouble("not_overlap");
        one.overlapCommunicationTime = resultSet->GetDouble("overlap");
        one.communicationTime = resultSet->GetDouble("communication");
        one.freeTime = resultSet->GetDouble("free");
        one.prepareTime = resultSet->GetDouble("preparing");
        one.pureCommunicationExcludeReceiveTime = resultSet->GetDouble("exclude_receive");
        one.npuTotalTime = one.computingTime + one.pureCommunicationTime + one.freeTime;
        uint32_t rankIdNum = StringUtil::StringToUint32(one.rankId);
        if (rankIdNum != UINT32_MAX) {
            data.emplace(rankIdNum, one);
        } else {
            ServerLog::Warn("RankId % could not be converted to a valid uint32_t. This entry will be skipped.",
                            one.rankId.c_str());
        }
    }
    return true;
}

std::vector<std::string> VirtualClusterDatabase::ExecuteGetAllRankFromStepStatisticInfo(std::string &sql)
{
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Failed to prepare sql for query all rank from step statistic info.");
        return {};
    }
    auto resultSet = stmt->ExecuteQuery();
    if (resultSet == nullptr) {
        ServerLog::Error("Failed to get result set for query all rank from step statistic info.");
        return {};
    }
    std::vector<std::string> res;
    while (resultSet->Next()) {
        res.push_back(resultSet->GetString("rankId"));
    }
    return res;
}

std::vector<CommInfoUnderRank> VirtualClusterDatabase::ExecuteGetCommTimeForRankDim(std::string &sql,
                                                                                    const std::string &step)
{
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Failed to prepare sql for query communication time for rank dimension.");
        return {};
    }
    if (step != "" && step != "All") {
        stmt->BindParams(step);
    }
    auto resultSet = stmt->ExecuteQuery();
    if (resultSet == nullptr) {
        ServerLog::Error("Failed to get result for query communication time for rank dimension.");
        return {};
    }
    std::vector<CommInfoUnderRank> res;
    while (resultSet->Next()) {
        CommInfoUnderRank info{};
        info.rankId = resultSet->GetString("rankId");
        info.commTime = resultSet->GetDouble("commTime");
        info.rankSet = resultSet->GetString("rankSet");
        res.push_back(info);
    }
    return res;
}

std::string VirtualClusterDatabase::FindValueByKey(const std::map<std::string, std::string> &info,
                                                   const std::string &key, const std::string defaultValue)
{
    if (info.find(key) == info.end()) {
        return defaultValue;
    }
    return info.at(key);
}
}
}