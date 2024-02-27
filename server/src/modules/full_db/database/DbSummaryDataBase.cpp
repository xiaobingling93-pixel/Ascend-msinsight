/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "DbSummaryDataBase.h"
#include "OperatorProtocolRequest.h"
#include "OperatorProtocolResponse.h"
#include "ServerLog.h"
#include "OperatorProtocol.h"
#include "WsSessionManager.h"
#include "OperatorProtocolEvent.h"
#include "TraceTime.h"

namespace Dic::Module::FullDb {
using namespace Server;
using namespace Dic::Module::Timeline;
bool DbSummaryDataBase::QueryComputeDetailHandler(Protocol::ComputeDetailParams params,
                                                  std::vector<Protocol::ComputeDetail> &computeDetails)
{
    std::string sql = GenComputeSql(params);
    std::string timeFlag = params.timeFlag;
    uint64_t startTime = Timeline::TraceTime::Instance().GetStartTime() - Timeline::TraceTime::Instance().GetBaseTime();
    double offset = (params.currentPage - 1) * params.pageSize;
    sqlite3_stmt *stmt = nullptr;
    int index = bindStartIndex;

    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("QueryOperatorDetail failed! Failed to prepare sql.", sqlite3_errmsg(db));
        return false;
    }
    sqlite3_bind_int64(stmt, index++, startTime);
    sqlite3_bind_text(stmt, index++, params.timeFlag.c_str(), params.timeFlag.length(), nullptr);
    sqlite3_bind_double(stmt, index++, params.pageSize);
    sqlite3_bind_double(stmt, index++, offset);
    std::vector<Protocol::ComputeDetail> computeVec;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        Protocol::ComputeDetail computeDetail{};
        computeDetail.name = sqlite3_column_string(stmt, col++);
        computeDetail.type = sqlite3_column_string(stmt, col++);
        computeDetail.startTime = sqlite3_column_string(stmt, col++);
        computeDetail.duration = sqlite3_column_double(stmt, col++);
        computeDetail.waitTime = sqlite3_column_double(stmt, col++);
        computeDetail.blockDim = sqlite3_column_int64(stmt, col++);
        computeDetail.inputShapes = sqlite3_column_string(stmt, col++);
        computeDetail.inputDataTypes = sqlite3_column_string(stmt, col++);
        computeDetail.inputFormats = sqlite3_column_string(stmt, col++);
        computeDetail.outputShapes = sqlite3_column_string(stmt, col++);
        computeDetail.outputDataTypes = sqlite3_column_string(stmt, col++);
        computeDetail.outputFormats = sqlite3_column_string(stmt, col++);
        computeVec.emplace_back(computeDetail);
    }
    computeDetails = computeVec;

    sqlite3_finalize(stmt);
    return true;
}

std::string DbSummaryDataBase::GenComputeSql(const Protocol::ComputeDetailParams& request)
{
    std::string orderList = request.orderBy;
    std::string ascend;
    if (request.order == "ascend") {
        ascend = "ASC";
    } else {
        ascend = "DESC";
    }
    std::string sql = "SELECT NAME.value AS name, "
                      "OP_TYPE.value as type, "
                      "CASE WHEN start == 0 THEN 0 ELSE ROUND((start - ?) / (1000.0 * 1000.0), 4) END AS startTime, "
                      "ROUND(end - start, 2) as duration, "
                      "wait_time as waitTime, "
                      "block_dim as blockDim, "
                      "INPUTSHAPES.value as inputShape, "
                      "INPUTDATATYPES.value as inputDataType, "
                      "INPUTFORMATS.value as inputFormat, "
                      "OUTPUTSHAPES as outputShape, "
                      "OUTPUTDATATYPES as outputDataType, "
                      "OUTPUTFORMATS as outputFormat "
                      "FROM " + TABLE_COMPUTE_TASK_INFO +
                      "JOIN TASK ON COMPUTE_TASK_INFO.correlationId = TASK.correlationId "
                      "JOIN STRING_IDS AS NAME ON NAME.id = COMPUTE_TASK_INFO.name "
                      "JOIN STRING_IDS AS OP_TYPE ON OP_TYPE.id = COMPUTE_TASK_INFO.opType "
                      "JOIN STRING_IDS AS INPUTSHAPES ON INPUTSHAPES.id = COMPUTE_TASK_INFO.inputShapes "
                      "JOIN STRING_IDS AS INPUTDATATYPES ON INPUTDATATYPES.id = COMPUTE_TASK_INFO.inputDataTypes "
                      "JOIN STRING_IDS AS INPUTFORMATS ON INPUTFORMATS.id = COMPUTE_TASK_INFO.inputFormats "
                      "JOIN STRING_IDS AS OUTPUTSHAPES ON OUTPUTSHAPES.id = COMPUTE_TASK_INFO.outputShapes "
                      "JOIN STRING_IDS AS OUTPUTDATATYPES ON OUTPUTDATATYPES.id = COMPUTE_TASK_INFO.outputDataTypes "
                      "JOIN STRING_IDS AS OUTPUTFORMATS ON OUTPUTFORMATS.id = COMPUTE_TASK_INFO.outputFormats "
                      " WHERE COMPUTE_TASK_INFO.taskType = ? ";
    if (!orderList.empty()) {
        sql +=  " ORDER BY " + orderList + " " + ascend;
    }
    sql += " LIMIT ? offset ?";
    return sql;
}

bool DbSummaryDataBase::QueryGetTotalNum(std::string name, int64_t &totalNum)
{
    sqlite3_stmt *stmt = nullptr;
    std::string sql = "SELECT count(*) as nums FROM " + TABLE_COMPUTE_TASK_INFO + " WHERE taskType = ?";
    ServerLog::Debug(sql);
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result == SQLITE_OK) {
        int index = bindStartIndex;
        sqlite3_bind_text(stmt, index++, name.c_str(), name.length(), nullptr);
    } else {
        ServerLog::Error("Get total num failed! Failed to prepare sql.", sqlite3_errmsg(db));
        return false;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        totalNum = sqlite3_column_int64(stmt, resultStartIndex);
    }
    sqlite3_finalize(stmt);
    return true;
}

bool DbSummaryDataBase::QueryOperatorDurationInfo(Protocol::OperatorDurationReqParams &reqParams,
                                                  Protocol::QueryType type,
                                                  std::vector<Protocol::OperatorDurationRes> &datas)
{
    std::string sql;
    if (type == Protocol::QueryType::CATEGORY) {
        sql = GenerateQueryCategoryDurationSql(reqParams);
    } else {
        sql = GenerateQueryComputeUnitDurationSql(reqParams);
    }
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Failed to get Duration Info. Cmd: ", sql, " Msg: ", sqlite3_errmsg(db), " ", result);
        return false;
    }

    int index = bindStartIndex;
    std::string rankId = GetDeviceIdFromCombinationId(reqParams.rankId);
    sqlite3_bind_text(stmt, index++, rankId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, index++, reqParams.topK);

    std::vector<Protocol::OperatorDurationRes> res;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Protocol::OperatorDurationRes one{};
        int col = 0;
        one.name = sqlite3_column_string(stmt, col++);
        one.duration = sqlite3_column_double(stmt, col++);
        if (res.size() >= maxCategorySize) {
            res[maxCategorySize - 1].name = "Others";
            res[maxCategorySize - 1].duration += one.duration;
        } else {
            res.emplace_back(one);
        }
    }
    datas = res;

    sqlite3_finalize(stmt);
    return true;
}

bool DbSummaryDataBase::QueryOperatorStatisticInfo(Protocol::OperatorStatisticReqParams &reqParams,
                                                   Protocol::OperatorStatisticInfoResponse &response)
{
    if (!QueryStatisticTotalNum(reqParams, response.total)) {
        ServerLog::Error("[Operator]Failed to query total num of statistic info.");
        return false;
    }

    std::string sql = GenerateQueryStatisticSql(reqParams);

    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryOperatorStatisticInfo. Failed to prepare sql.", sqlite3_errmsg(db));
        return false;
    }
    std::string rankId = GetDeviceIdFromCombinationId(reqParams.rankId);
    auto resultSet = stmt->ExecuteQuery(rankId, reqParams.pageSize,
                                        reqParams.pageSize * (reqParams.current - 1));
    std::vector<Protocol::OperatorStatisticInfoRes> res;
    while (resultSet->Next()) {
        Protocol::OperatorStatisticInfoRes one{};
        one.opType = resultSet->GetString("op_type");
        one.opName = resultSet->GetString("name");
        one.inputShape = resultSet->GetString("input_shapes");
        one.accCore = resultSet->GetString("accelerator_core");
        one.totalTime = resultSet->GetDouble("total_time");
        one.count = resultSet->GetInt64("cnt");
        one.avgTime = resultSet->GetDouble("avg_time");
        one.maxTime = resultSet->GetDouble("max_time");
        one.minTime = resultSet->GetDouble("min_time");
        res.emplace_back(one);
    }
    response.datas = res;
    return true;
}

std::string DbSummaryDataBase::GenerateQueryStatisticSql(Protocol::OperatorStatisticReqParams &reqParams)
{
    std::string group;
    std::string name;
    if (reqParams.group == Protocol::OP_TYPE_GROUP) {
        group = "op_type || accelerator_core";
        name = "''";
    } else {
        group = R"(name || input_shapes || accelerator_core)";
        name = "name";
    }

    std::string sql =
            " SELECT * FROM ("
            "     SELECT OPTYPE.value AS op_type,"
            "     NAME.value AS name,  "
            "     INPUTSHAPES.value AS input_shapes, "
            "     TASKTYPE.value AS accelerator_core, "
            "     ROUND(SUM(TASK.end - TASK.start), 2) as total_time, COUNT(0) as cnt,"
            "     ROUND(SUM(end - start) / COUNT(0), 2) as avg_time,"
            "     ROUND(max(end - start), 2) as max_time,"
            "     ROUND(min(end - start), 2) as min_time"
            "     FROM  COMPUTE_TASK_INFO "
            "     JOIN STRING_IDS AS NAME ON NAME.id = COMPUTE_TASK_INFO.name"
            "     JOIN STRING_IDS AS OPTYPE ON OPTYPE.id = COMPUTE_TASK_INFO.opType"
            "     JOIN STRING_IDS AS INPUTSHAPES ON INPUTSHAPES.id = COMPUTE_TASK_INFO.inputShapes"
            "     JOIN STRING_IDS AS TASKTYPE ON TASKTYPE.id = COMPUTE_TASK_INFO.taskType"
            "     JOIN TASK ON COMPUTE_TASK_INFO.correlationId = TASK.correlationId "
            "     WHERE deviceId = ? AND accelerator_core <> 'HCCL'"
            "     GROUP BY " + group +
            "     ORDER by total_time DESC LIMIT " + std::to_string(reqParams.topK) +
            "     ) subquery ";

    if (!reqParams.orderBy.empty() && !reqParams.order.empty()) {
        sql += " ORDER by " + reqParams.orderBy + " " + (reqParams.order == "ascend" ? "ASC" : "DESC");
    }

    sql += " LIMIT ? OFFSET ?";
    return sql;
}

bool DbSummaryDataBase::QueryStatisticTotalNum(Protocol::OperatorStatisticReqParams &reqParams, int64_t &total)
{
    std::string group = reqParams.group == Protocol::OP_TYPE_GROUP ?
            "opTypes || taskTypes" : R"(name || inputShapes || taskTypes)";
    std::string sql =
            " SELECT COUNT(*) as nums"
            " FROM ( "
            "     SELECT "
            "     deviceId,"
            "     start, "
            "     end, "
            "     TASKTYPE.value AS taskTypes, "
            "     OPTYPE.value AS opTypes"
            "     FROM COMPUTE_TASK_INFO"
            "     JOIN TASK ON COMPUTE_TASK_INFO.correlationId = TASK.correlationId "
            "     JOIN STRING_IDS AS TASKTYPE ON TASKTYPE.id = COMPUTE_TASK_INFO.taskType"
            "     JOIN STRING_IDS AS OPTYPE ON OPTYPE.id = COMPUTE_TASK_INFO.opType"
            "     WHERE deviceId = ? AND taskTypes <> 'HCCL' "
            "     GROUP by " + group +
            "     ORDER by (end - start) DESC LIMIT ?"
            " ) subquery";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("QueryStatisticTotalNum. Failed to prepare sql.", sqlite3_errmsg(db));
        return false;
    }
    std::string rankId = GetDeviceIdFromCombinationId(reqParams.rankId);
    auto resultSet = stmt->ExecuteQuery(rankId, reqParams.topK);

    while (resultSet->Next()) {
        total = resultSet->GetInt64("nums");
    }
    return true;
}

bool DbSummaryDataBase::QueryOperatorDetailInfo(Protocol::OperatorStatisticReqParams &reqParams,
                                                Protocol::OperatorDetailInfoResponse &response)
{
    if (!QueryDetailTotalNum(reqParams, response.total)) {
        ServerLog::Error("[Operator]Failed to query total num of detail info.");
        return false;
    }
    std::string sql = GenerateQueryDetailSql(reqParams);
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Failed to get Detail Info. Cmd: ", sql, " Msg:", sqlite3_errmsg(db), " ", result);
        return false;
    }
    uint64_t startTime = Timeline::TraceTime::Instance().GetStartTime() - Timeline::TraceTime::Instance().GetBaseTime();
    std::string rankId = GetDeviceIdFromCombinationId(reqParams.rankId);
    int index = bindStartIndex;
    sqlite3_bind_int64(stmt, index++, startTime);
    sqlite3_bind_text(stmt, index++, rankId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, index++, reqParams.topK);
    sqlite3_bind_int64(stmt, index++, reqParams.pageSize);
    sqlite3_bind_int64(stmt, index++, (reqParams.current - 1) * reqParams.pageSize);

    std::vector<OperatorDetailInfoRes> res;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = 0;
        OperatorDetailInfoRes one{};
        one.rankId = sqlite3_column_string(stmt, col++);
        one.stepId = sqlite3_column_string(stmt, col++);
        one.name = sqlite3_column_string(stmt, col++);
        one.type = sqlite3_column_string(stmt, col++);
        one.accCore = sqlite3_column_string(stmt, col++);
        one.startTime = sqlite3_column_string(stmt, col++);
        one.duration = sqlite3_column_double(stmt, col++);
        one.waitTime = sqlite3_column_double(stmt, col++);
        one.blockDim = sqlite3_column_int64(stmt, col++);
        one.inputShape = sqlite3_column_string(stmt, col++);
        one.inputType = sqlite3_column_string(stmt, col++);
        one.inputFormat = sqlite3_column_string(stmt, col++);
        one.outputShape = sqlite3_column_string(stmt, col++);
        one.outputType = sqlite3_column_string(stmt, col++);
        one.outputFormat = sqlite3_column_string(stmt, col++);
        res.emplace_back(one);
    }
    response.level = (res.empty() || res.at(0).inputShape.empty()) ? "l0" : "l1";
    response.datas = res;
    sqlite3_finalize(stmt);
    return true;
}

bool DbSummaryDataBase::QueryMoreInfoTotalNum(OperatorMoreInfoReqParams &reqParams, int64_t &total)
{
    sqlite3_stmt *stmt = nullptr;
    std::string condition =
            (reqParams.group == OP_TYPE_GROUP) ? " op_type = ?" : " name = ? AND input_shapes = ?";

    std::string sql =
            " SELECT COUNT(*) as nums"
            " FROM ("
            "     SELECT "
            "     NAME.value AS name,  "
            "     INPUTSHAPES.value AS input_shapes, "
            "     TASKTYPE.value AS task_type, "
            "     OPTYPE.value AS op_type"
            "     FROM " + TABLE_COMPUTE_TASK_INFO +
            "     JOIN TASK ON COMPUTE_TASK_INFO.correlationId = TASK.correlationId"
            "     JOIN STRING_IDS AS NAME ON NAME.id = COMPUTE_TASK_INFO.name"
            "     JOIN STRING_IDS AS OPTYPE ON OPTYPE.id = COMPUTE_TASK_INFO.opType"
            "     JOIN STRING_IDS AS INPUTSHAPES ON INPUTSHAPES.id = COMPUTE_TASK_INFO.inputShapes"
            "     JOIN STRING_IDS AS TASKTYPE ON TASKTYPE.id = COMPUTE_TASK_INFO.taskType"
            "     WHERE deviceId = ? AND task_type = ? AND" + condition +
            " ) subquery";

    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Failed to get More Total Num. Cmd: ", sql, " Msg: ", sqlite3_errmsg(db), " ", result);
        return false;
    }
    int index = bindStartIndex;
    std::string rankId = GetDeviceIdFromCombinationId(reqParams.rankId);
    sqlite3_bind_text(stmt, index++, rankId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, index++, reqParams.accCore.c_str(), -1, SQLITE_TRANSIENT);
    if (reqParams.group == OP_TYPE_GROUP) {
        sqlite3_bind_text(stmt, index++, reqParams.opType.c_str(), -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_text(stmt, index++, reqParams.opName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, index++, reqParams.shape.c_str(), -1, SQLITE_TRANSIENT);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        total = sqlite3_column_int64(stmt, resultStartIndex);
    }
    sqlite3_finalize(stmt);
    return true;
}

std::string DbSummaryDataBase::GenerateQueryMoreInfoSql(OperatorMoreInfoReqParams &reqParams)
{
    std::string sql =
            " SELECT rank_id, step_id, name, op_type, accelerator_core,"
            " CASE WHEN start_time == 0 THEN 'NA' ELSE ROUND((start_time - ?) / (1000.0 * 1000.0), 2)"
            " END AS startTime, duration, wait_time, block_dim,"
            " input_shapes, input_data_types, input_formats, output_shapes, output_data_types, output_formats"
            " FROM ("
            "     SELECT block_dim, deviceId as rank_id, streamId as step_id, "
            "     NAME.value AS name,  OPTYPE.value AS op_type,"
            "     TASKTYPE.value as accelerator_core, start as start_time, end - start as duration, 0 as wait_time,"
            "     INPUTSHAPES.value as input_shapes, INPUTDATATYPES.value as input_data_types, "
            "     INPUTFORMATS.value as input_formats, OUTPUTSHAPES.value as output_shapes, "
            "     OUTPUTDATATYPES.value as output_data_types, OUTPUTFORMATS.value as output_formats "
            "     FROM " + TABLE_COMPUTE_TASK_INFO +
            "     JOIN TASK ON COMPUTE_TASK_INFO.correlationId = TASK.correlationId "
            "     JOIN STRING_IDS AS NAME ON NAME.id = COMPUTE_TASK_INFO.name"
            "     JOIN STRING_IDS AS OPTYPE ON OPTYPE.id = COMPUTE_TASK_INFO.opType"
            "     JOIN STRING_IDS AS TASKTYPE ON TASKTYPE.id = COMPUTE_TASK_INFO.taskType"
            "     JOIN STRING_IDS AS INPUTSHAPES ON INPUTSHAPES.id = COMPUTE_TASK_INFO.inputShapes"
            "     JOIN STRING_IDS AS INPUTDATATYPES ON INPUTDATATYPES.id = COMPUTE_TASK_INFO.inputDataTypes"
            "     JOIN STRING_IDS AS INPUTFORMATS ON INPUTFORMATS.id = COMPUTE_TASK_INFO.inputFormats"
            "     JOIN STRING_IDS AS OUTPUTSHAPES ON OUTPUTSHAPES.id = COMPUTE_TASK_INFO.outputShapes"
            "     JOIN STRING_IDS AS OUTPUTDATATYPES ON OUTPUTDATATYPES.id = COMPUTE_TASK_INFO.outputDataTypes"
            "     JOIN STRING_IDS AS OUTPUTFORMATS ON OUTPUTFORMATS.id = COMPUTE_TASK_INFO.outputFormats"
            "     WHERE rank_id = ? AND accelerator_core = ?"
            "     ORDER by duration DESC"
            " ) subquery ";
    if (reqParams.group == OP_TYPE_GROUP) {
        sql += " WHERE op_type = ?";
    } else {
        sql += " WHERE name = ? AND input_shapes = ?";
    }
    if (!reqParams.orderBy.empty() && !reqParams.order.empty()) {
        sql += " ORDER by " + reqParams.orderBy + " " + (reqParams.order == "ascend" ? "ASC" : "DESC");
    }

    sql += " LIMIT ? OFFSET ?";
    return sql;
}

bool DbSummaryDataBase::QueryOperatorMoreInfo(OperatorMoreInfoReqParams &reqParams, OperatorMoreInfoResponse &response)
{
    if (!QueryMoreInfoTotalNum(reqParams, response.total)) {
        ServerLog::Error("[Operator]Failed to query total num of more info.");
        return false;
    }
    std::string sql = GenerateQueryMoreInfoSql(reqParams);
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Failed to get Op More Info. Cmd: ", sql, " Msg: ", sqlite3_errmsg(db), " ", result);
        return false;
    }
    BindSqliteParam(stmt, reqParams);

    std::vector<OperatorDetailInfoRes> res;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = 0;
        OperatorDetailInfoRes one{};
        one.rankId = sqlite3_column_string(stmt, col++);
        one.stepId = sqlite3_column_string(stmt, col++);
        one.name = sqlite3_column_string(stmt, col++);
        one.type = sqlite3_column_string(stmt, col++);
        one.accCore = sqlite3_column_string(stmt, col++);
        one.startTime = sqlite3_column_string(stmt, col++);
        one.duration = sqlite3_column_double(stmt, col++);
        one.waitTime = sqlite3_column_double(stmt, col++);
        one.blockDim = sqlite3_column_int64(stmt, col++);
        one.inputShape = sqlite3_column_string(stmt, col++);
        one.inputType = sqlite3_column_string(stmt, col++);
        one.inputFormat = sqlite3_column_string(stmt, col++);
        one.outputShape = sqlite3_column_string(stmt, col++);
        one.outputType = sqlite3_column_string(stmt, col++);
        one.outputFormat = sqlite3_column_string(stmt, col++);
        res.emplace_back(one);
    }
    response.level = (res.empty() || res.at(0).inputShape.empty()) ? "l0" : "l1";
    response.datas = res;
    sqlite3_finalize(stmt);
    return true;
}

void DbSummaryDataBase::BindSqliteParam(sqlite3_stmt *stmt, Protocol::OperatorMoreInfoReqParams &reqParams)
{
    uint64_t startTime = Timeline::TraceTime::Instance().GetStartTime() - Timeline::TraceTime::Instance().GetBaseTime();
    std::string rankId = GetDeviceIdFromCombinationId(reqParams.rankId);
    int index = bindStartIndex;
    sqlite3_bind_int64(stmt, index++, startTime);
    sqlite3_bind_text(stmt, index++, rankId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, index++, reqParams.accCore.c_str(), -1, SQLITE_TRANSIENT);
    if (reqParams.group == Protocol::OP_TYPE_GROUP) {
        sqlite3_bind_text(stmt, index++, reqParams.opType.c_str(), -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_text(stmt, index++, reqParams.opName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, index++, reqParams.shape.c_str(), -1, SQLITE_TRANSIENT);
    }
    sqlite3_bind_int64(stmt, index++, reqParams.pageSize);
    sqlite3_bind_int64(stmt, index++, (reqParams.current - 1) * reqParams.pageSize);
}

bool DbSummaryDataBase::QueryCommDetailHandler(Protocol::CommunicationDetailParams params,
                                               std::vector<Protocol::CommunicationDetail> &commDetails)
{
    std::string sql = GetCommSql(params);
    std::string timeFlag = params.timeFlag;
    uint64_t startTime = Timeline::TraceTime::Instance().GetStartTime() - Timeline::TraceTime::Instance().GetBaseTime();
    double offset = (params.currentPage - 1) * params.pageSize;
    sqlite3_stmt *stmt = nullptr;
    int index = bindStartIndex;

    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("QueryCommDetailHandler failed! Failed to prepare sql.", sqlite3_errmsg(db));
        return false;
    }
    sqlite3_bind_int64(stmt, index++, startTime);
    sqlite3_bind_text(stmt, index++, params.timeFlag.c_str(), params.timeFlag.length(), nullptr);
    sqlite3_bind_double(stmt, index++, params.pageSize);
    sqlite3_bind_double(stmt, index++, offset);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        CommunicationDetail computeDetail{};
        computeDetail.name = sqlite3_column_string(stmt, col++);
        computeDetail.type = sqlite3_column_string(stmt, col++);
        computeDetail.startTime = sqlite3_column_string(stmt, col++);
        computeDetail.duration = sqlite3_column_double(stmt, col++);
        computeDetail.waitTime = sqlite3_column_double(stmt, col++);
        commDetails.emplace_back(computeDetail);
    }

    sqlite3_finalize(stmt);
    return true;
}

std::string DbSummaryDataBase::GetCommSql(const CommunicationDetailParams& request)
{
    std::string order = request.orderBy;
    std::string ascend;
    if (request.order == "ascend") {
        ascend = "ASC";
    } else {
        ascend = "DESC";
    }
    std::string sql = "SELECT name, op_type as type, CASE WHEN start_time == 0 THEN 'NA' "
                      "ELSE ROUND((start_time - ?) / (1000.0 * 1000.0), 4) END AS startTime, "
                      "ROUND(duration, 4) as duration, ROUND(wait_time, 4) as waitTime FROM ( "
                      " SELECT NAME.value AS name, OPTYPE.value AS op_type, "
                      " start as start_time, end - start as duration, "
                      " TASKTYPE.value AS taskTypes, 0 as wait_time FROM "
                      + TABLE_COMPUTE_TASK_INFO +
                      "     JOIN STRING_IDS AS NAME ON NAME.id = COMPUTE_TASK_INFO.name"
                      "     JOIN STRING_IDS AS OPTYPE ON OPTYPE.id = COMPUTE_TASK_INFO.opType"
                      "     JOIN STRING_IDS AS TASKTYPE ON TASKTYPE.id = COMPUTE_TASK_INFO.taskType"
                      "     JOIN TASK ON COMPUTE_TASK_INFO.correlationId = TASK.correlationId "
                      "     WHERE task_type = ?"
                      "     GROUP BY TASK.correlationId"
                      " ) subquery ";
    if (!order.empty()) {
        sql += " ORDER BY " + order + " " + ascend;
    }
    sql += " LIMIT ? offset ?";
    return sql;
}

std::string DbSummaryDataBase::GenerateQueryCategoryDurationSql(Protocol::OperatorDurationReqParams &reqParams)
{
    std::string group;
    std::string name;
    if (reqParams.group == OP_TYPE_GROUP) {
        name = "OPTYPE.value";
        group = "op_type || task_type";
    } else if (reqParams.group == OPERATOR_GROUP) {
        name = "NAME.value";
        group = "name || task_type";
    } else {
        name = R"(NAME.value || '[' || INPUTSHAPES.value || ']')";
        group = R"(name || '[' || input_shapes || ']' || task_type)";
    }

    std::string sql =
            " SELECT name, duration From ("
            " SELECT " + name + " as name," + (reqParams.group == Protocol::OPERATOR_GROUP ?
            " ROUND(end - start, 2) as duration" : " ROUND(sum(end - start), 2) as duration") + ","
            " TASKTYPE.value as task_type, OPTYPE.value as op_type, INPUTSHAPES.value as input_shapes"
            " FROM " + TABLE_COMPUTE_TASK_INFO +
            " JOIN TASK ON COMPUTE_TASK_INFO.correlationId = TASK.correlationId "
            "     JOIN STRING_IDS AS NAME ON NAME.id = COMPUTE_TASK_INFO.name"
            "     JOIN STRING_IDS AS OPTYPE ON OPTYPE.id = COMPUTE_TASK_INFO.opType"
            "     JOIN STRING_IDS AS INPUTSHAPES ON INPUTSHAPES.id = COMPUTE_TASK_INFO.inputShapes"
            "     JOIN STRING_IDS AS TASKTYPE ON TASKTYPE.id = COMPUTE_TASK_INFO.taskType"
            " WHERE deviceId = ? AND task_type <> 'HCCL'" + (reqParams.group == Protocol::OPERATOR_GROUP ?
            " " : " GROUP by " + group) +
            " ORDER BY duration DESC LIMIT ?"
            " ) subquery";
    return sql;
}

std::string DbSummaryDataBase::GenerateQueryComputeUnitDurationSql(Protocol::OperatorDurationReqParams &reqParams)
{
    std::string group;
    if (reqParams.group == OP_TYPE_GROUP) {
        group = "OPTYPE.value || TASKTYPE.value";
    } else if (reqParams.group == OPERATOR_GROUP) {
        group = "NAME.value || TASKTYPE.value";
    } else {
        group = R"(NAME.value || '[' || INPUTSHAPES.value || ']' || TASKTYPE.value)";
    }

    std::string sql =
            " SELECT taskTypes as name, ROUND(SUM(duration), 2) as duration"
            " FROM ("
            "     SELECT " + group + ", TASKTYPE.value as taskTypes, " + (reqParams.group == Protocol::OPERATOR_GROUP ?
            " ROUND(end - start, 2) as duration" : " ROUND(sum(end - start), 2) as duration") +
            "     FROM " + TABLE_COMPUTE_TASK_INFO +
            "     JOIN TASK ON COMPUTE_TASK_INFO.correlationId = TASK.correlationId "
            "     JOIN STRING_IDS AS NAME ON NAME.id = COMPUTE_TASK_INFO.name"
            "     JOIN STRING_IDS AS OPTYPE ON OPTYPE.id = COMPUTE_TASK_INFO.opType"
            "     JOIN STRING_IDS AS INPUTSHAPES ON INPUTSHAPES.id = COMPUTE_TASK_INFO.inputShapes"
            "     JOIN STRING_IDS AS TASKTYPE ON TASKTYPE.id = COMPUTE_TASK_INFO.taskType"
            "     WHERE deviceId = ? AND taskTypes <> 'HCCL'"
            "     GROUP BY " + group +
            "     ORDER BY duration DESC LIMIT ?"
            " ) subquery" +
            " GROUP by taskTypes"
            " ORDER BY duration DESC";
    return sql;
}

bool DbSummaryDataBase::QueryDetailTotalNum(OperatorStatisticReqParams &reqParams, int64_t &total)
{
    sqlite3_stmt *stmt = nullptr;
    std::string sql =
            " SELECT COUNT(*) as nums"
            " FROM ("
            "     SELECT deviceId, TASKTYPE.value as taskTypes, end - start as duration "
            "     FROM "+ TABLE_COMPUTE_TASK_INFO +
            "     JOIN TASK ON COMPUTE_TASK_INFO.correlationId = TASK.correlationId "
            "     JOIN STRING_IDS AS TASKTYPE ON TASKTYPE.id = COMPUTE_TASK_INFO.taskType"
            "     WHERE deviceId = ? AND taskTypes <> 'HCCL'"
            "     ORDER BY duration DESC LIMIT ?"
            " ) subquery";
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Failed to get Detail Total Num. Cmd: ", sql, " Msg: ", sqlite3_errmsg(db), " ", result);
        return false;
    }
    std::string rankId = GetDeviceIdFromCombinationId(reqParams.rankId);
    int index = bindStartIndex;
    sqlite3_bind_text(stmt, index++, rankId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, index++, reqParams.topK);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        total = sqlite3_column_int64(stmt, resultStartIndex);
    }
    sqlite3_finalize(stmt);
    return true;
}

std::string DbSummaryDataBase::GenerateQueryDetailSql(OperatorStatisticReqParams &reqParams)
{
    std::string sql =
            " SELECT rank_id, step_id, name, op_type, accelerator_core,"
            " CASE WHEN start_time == 0 THEN 'NA' ELSE ROUND((start_time - ?) / (1000.0 * 1000.0), 2)"
            " END AS startTime, duration, wait_time, block_dim,"
            " input_shapes, input_data_types, input_formats, output_shapes, output_data_types, output_formats"
            " FROM ("
            "     SELECT block_dim, deviceId as rank_id, streamId as step_id, "
            "     NAME.value AS name,  OPTYPE.value AS op_type,"
            "     TASKTYPE.value as accelerator_core, start as start_time, "
            "     end - start as duration, 0 as wait_time, "
            "     INPUTSHAPES.value as input_shapes, INPUTDATATYPES.value as input_data_types, "
            "     INPUTFORMATS.value as input_formats, OUTPUTSHAPES.value as output_shapes, "
            "     OUTPUTDATATYPES.value as output_data_types, OUTPUTFORMATS.value as output_formats "
            " FROM " + TABLE_COMPUTE_TASK_INFO +
            "     JOIN TASK ON COMPUTE_TASK_INFO.correlationId = TASK.correlationId "
            "     JOIN STRING_IDS AS NAME ON NAME.id = COMPUTE_TASK_INFO.name"
            "     JOIN STRING_IDS AS OPTYPE ON OPTYPE.id = COMPUTE_TASK_INFO.opType"
            "     JOIN STRING_IDS AS TASKTYPE ON TASKTYPE.id = COMPUTE_TASK_INFO.taskType"
            "     JOIN STRING_IDS AS INPUTSHAPES ON INPUTSHAPES.id = COMPUTE_TASK_INFO.inputShapes"
            "     JOIN STRING_IDS AS INPUTDATATYPES ON INPUTDATATYPES.id = COMPUTE_TASK_INFO.inputDataTypes"
            "     JOIN STRING_IDS AS INPUTFORMATS ON INPUTFORMATS.id = COMPUTE_TASK_INFO.inputFormats"
            "     JOIN STRING_IDS AS OUTPUTSHAPES ON OUTPUTSHAPES.id = COMPUTE_TASK_INFO.outputShapes"
            "     JOIN STRING_IDS AS OUTPUTDATATYPES ON OUTPUTDATATYPES.id = COMPUTE_TASK_INFO.outputDataTypes"
            "     JOIN STRING_IDS AS OUTPUTFORMATS ON OUTPUTFORMATS.id = COMPUTE_TASK_INFO.outputFormats"
            "     WHERE rank_id = ? AND accelerator_core <> 'HCCL'"
            "     ORDER by duration DESC LIMIT ?"
            " ) subquery ";
    if (!reqParams.orderBy.empty() && !reqParams.order.empty()) {
        sql += " ORDER by " + reqParams.orderBy + " " + (reqParams.order == "ascend" ? "ASC" : "DESC");
    }
    sql += " LIMIT ? OFFSET ?";
    return sql;
}

void DbSummaryDataBase::ParserEnd(const std::string &token, const std::string &fileId, bool result,
                                  const std::string &msg)
{
    WsSession *session = WsSessionManager::Instance().GetSession(token);
    if (session == nullptr) {
        ServerLog::Error("Failed to get session token for summary callback.");
        return;
    }
    auto event = std::make_unique<Protocol::OperatorParseStatusEvent>();
    event->moduleName = Protocol::ModuleType::OPERATOR;
    event->token = token;
    event->result = true;
    event->data.rankId = fileId;
    event->data.status = result;
    event->data.error = msg;
    session->OnEvent(std::move(event));
}

}