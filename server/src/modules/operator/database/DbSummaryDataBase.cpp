/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "pch.h"
#include "OperatorProtocolRequest.h"
#include "OperatorGroupConverter.h"
#include "OperatorProtocolResponse.h"
#include "OperatorProtocol.h"
#include "WsSessionManager.h"
#include "OperatorProtocolEvent.h"
#include "TraceTime.h"
#include "DataBaseManager.h"
#include "DbSummaryDataBase.h"

namespace Dic::Module::FullDb {
using namespace Server;
using namespace Dic::Module::Timeline;
bool DbSummaryDataBase::OpenDb(const std::string &dbPath, bool clearAllTable)
{
    auto result = Database::OpenDb(dbPath, clearAllTable) && GetMetaVersion();
    blockDimColumnName = isLowCamel ? "blockDim" : "block_dim";
    return result;
}
bool DbSummaryDataBase::QueryComputeOpDetail(Protocol::ComputeDetailParams params,
    std::vector<Protocol::ComputeDetail> &computeDetails)
{
    std::string sql = GenComputeSql(params);
    std::string timeFlag = params.timeFlag;
    uint64_t startTime = NumberUtil::CeilingClamp(Timeline::TraceTime::Instance().GetStartTime(),
                                                  (uint64_t)INT64_MAX);
    double offset = (params.currentPage - 1) * params.pageSize;
    sqlite3_stmt *stmt = nullptr;
    int index = bindStartIndex;

    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query operator detail failed! Failed to prepare sql.", sqlite3_errmsg(db));
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

// LCOV_EXCL_BR_START
std::string DbSummaryDataBase::GenComputeSql(const Protocol::ComputeDetailParams& request)
{
    std::string sql = "SELECT NAME.value AS name, "
                      "OP_TYPE.value as type, "
                      "CASE WHEN startNs == 0 THEN 0 ELSE ROUND((startNs - ?) /(1000.0 * 1000.0), 4) END AS startTime, "
                      "ROUND((endNs - startNs)/1000.0, 2) as duration, "
                      "ROUND((waitNs)/1000.0, 3) as waitTime, " + blockDimColumnName + " as blockDim, "
                      "INPUTSHAPES.value as inputShape, "
                      "INPUTDATATYPES.value as inputDataType, "
                      "INPUTFORMATS.value as inputFormat, "
                      "OUTPUTSHAPES.value as outputShape, "
                      "OUTPUTDATATYPES.value as outputDataType, "
                      "OUTPUTFORMATS.value as outputFormat "
                      "FROM " + TABLE_COMPUTE_TASK_INFO +
                      " JOIN TASK ON COMPUTE_TASK_INFO.globalTaskId = TASK.globalTaskId "
                      " JOIN STRING_IDS AS NAME ON NAME.id = COMPUTE_TASK_INFO.name "
                      " JOIN STRING_IDS AS OP_TYPE ON OP_TYPE.id = COMPUTE_TASK_INFO.opType "
                      " JOIN STRING_IDS AS INPUTSHAPES ON INPUTSHAPES.id = COMPUTE_TASK_INFO.inputShapes "
                      " JOIN STRING_IDS AS INPUTDATATYPES ON INPUTDATATYPES.id = COMPUTE_TASK_INFO.inputDataTypes "
                      " JOIN STRING_IDS AS INPUTFORMATS ON INPUTFORMATS.id = COMPUTE_TASK_INFO.inputFormats "
                      " JOIN STRING_IDS AS OUTPUTSHAPES ON OUTPUTSHAPES.id = COMPUTE_TASK_INFO.outputShapes "
                      " JOIN STRING_IDS AS OUTPUTDATATYPES ON OUTPUTDATATYPES.id = COMPUTE_TASK_INFO.outputDataTypes "
                      " JOIN STRING_IDS AS OUTPUTFORMATS ON OUTPUTFORMATS.id = COMPUTE_TASK_INFO.outputFormats "
                      " JOIN STRING_IDS AS TASKTYPE ON TASKTYPE.id = COMPUTE_TASK_INFO.taskType "
                      " WHERE TASKTYPE.value = ? ";

    if (!StringUtil::CheckSqlValid(request.orderBy)) {
        ServerLog::Error("There is an SQL injection attack on the parameter of orderBy to generate compute sql.");
    } else if (!request.orderBy.empty() && !request.order.empty()) {
        sql += " ORDER by " + request.orderBy + " " + (request.order == "ascend" ? "ASC" : "DESC");
    }
    sql += " LIMIT ? offset ?";
    return sql;
}

bool DbSummaryDataBase::QueryTotalNumByAcceleratorCore(std::string name, int64_t &totalNum)
{
    sqlite3_stmt *stmt = nullptr;
    std::string sql = "SELECT count(*) as nums FROM " + TABLE_COMPUTE_TASK_INFO +
            " WHERE taskType = (select id from STRING_IDS WHERE value = ?)";
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
        ServerLog::Error("Failed to get Duration Info. Msg: ", sqlite3_errmsg(db), " ", result);
        return false;
    }

    int index = bindStartIndex;
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

bool DbSummaryDataBase::ExecSqlGetStatisticInfo(std::string sql,
                                                Protocol::OperatorStatisticReqParams &reqParams,
                                                std::vector<Protocol::OperatorStatisticInfoRes> &res)
{
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Failed to prepare sql to query operator statistic info.", sqlite3_errmsg(db));
        return false;
    }
    auto resultSet =
        stmt->ExecuteQuery(reqParams.topK, reqParams.pageSize, reqParams.pageSize * (reqParams.current - 1));
    if (resultSet == nullptr) {
        ServerLog::Error("Failed to exec sql to query operator statistic info.");
        return false;
    }
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
    std::vector<Protocol::OperatorStatisticInfoRes> res;
    if (!ExecSqlGetStatisticInfo(sql, reqParams, res)) {
        ServerLog::Error("Failed to exec query detail sql.");
        return false;
    }
    std::vector<Protocol::OperatorStatisticCmpInfoRes> cmpRes;
    for (auto &data : res) {
        OperatorStatisticCmpInfoRes tmpInfo;
        tmpInfo.compare = data;
        cmpRes.emplace_back(tmpInfo);
    }
    response.datas = cmpRes;
    return true;
}

bool DbSummaryDataBase::QueryAllOperatorStatisticInfo(Protocol::OperatorStatisticReqParams &reqParams,
                                                      std::vector<Protocol::OperatorStatisticInfoRes> &res)
{
    std::string sql = GenerateQueryStatisticSql(reqParams);
    if (!ExecSqlGetStatisticInfo(sql, reqParams, res)) {
        ServerLog::Error("Failed to exec query detail sql.");
        return false;
    }
    return true;
}

std::string DbSummaryDataBase::GenerateQueryStatisticSql(Protocol::OperatorStatisticReqParams &reqParams)
{
    OperatorGroupConverter::OperatorGroup operatorGroup = Protocol::OperatorGroupConverter::ToEnum(reqParams.group);
    bool isHccl = Protocol::OperatorGroupConverter::IsHccl(reqParams.group);
    std::string sql;
    if (isHccl) {
        sql = " SELECT * FROM (SELECT SUBSTR(NAME.value, 1, INSTR(NAME.value, '__')) as op_type, NAME.value as name,"
            " NULL AS input_shapes,NULL as accelerator_core,"
            " ROUND(SUM(COMMUNICATION_OP.endNs - COMMUNICATION_OP.startNs) / 1000.0, 2) as total_time, COUNT(0) as cnt,"
            " ROUND(SUM(COMMUNICATION_OP.endNs - COMMUNICATION_OP.startNs) / 1000.0 / COUNT(0), 2) as avg_time,"
            " ROUND(max(COMMUNICATION_OP.endNs - COMMUNICATION_OP.startNs) / 1000.0, 2) as max_time,"
            " ROUND(min(COMMUNICATION_OP.endNs - COMMUNICATION_OP.startNs) / 1000.0, 2) as min_time "
            " FROM COMMUNICATION_OP JOIN STRING_IDS AS NAME ON NAME.id = COMMUNICATION_OP.opName "
            " GROUP BY SUBSTR(NAME.value, 1, INSTR(NAME.value, '__'))"
            " ORDER by total_time DESC LIMIT ?) subquery ";
    } else {
        std::string group = operatorGroup == OperatorGroupConverter::OperatorGroup::OP_TYPE_GROUP ?
            "op_type || accelerator_core" :
            R"(name || input_shapes || accelerator_core)";
        sql = " SELECT * FROM ("
            "     SELECT OPTYPE.value AS op_type,NAME.value AS name,"
            "     INPUTSHAPES.value AS input_shapes,TASKTYPE.value AS accelerator_core, "
            "     ROUND(SUM(TASK.endNs - TASK.startNs) / 1000.0, 2) as total_time, COUNT(0) as cnt,"
            "     ROUND(SUM(endNs - startNs) / 1000.0 / COUNT(0), 2) as avg_time,"
            "     ROUND(max(endNs - startNs) / 1000.0, 2) as max_time,"
            "     ROUND(min(endNs - startNs) / 1000.0, 2) as min_time"
            "     FROM  COMPUTE_TASK_INFO "
            "     JOIN STRING_IDS AS NAME ON NAME.id = COMPUTE_TASK_INFO.name"
            "     JOIN STRING_IDS AS OPTYPE ON OPTYPE.id = COMPUTE_TASK_INFO.opType"
            "     JOIN STRING_IDS AS INPUTSHAPES ON INPUTSHAPES.id = COMPUTE_TASK_INFO.inputShapes"
            "     JOIN STRING_IDS AS TASKTYPE ON TASKTYPE.id = COMPUTE_TASK_INFO.taskType"
            "     JOIN TASK ON COMPUTE_TASK_INFO.globalTaskId = TASK.globalTaskId "
            "     WHERE accelerator_core <> 'HCCL'"
            "     GROUP BY " + group + " ORDER by total_time DESC LIMIT ? "
            "     ) subquery ";
    }
    if (!GenerateQueryFiltersSql<OperatorStatisticReqParams>(reqParams, sql)) {
        return "";
    }

    if (!StringUtil::CheckSqlValid(reqParams.orderBy)) {
        ServerLog::Error("There is an SQL injection attack on the parameter of orderBy"
                         "to generate query statistic sql.");
    } else if (!reqParams.orderBy.empty() && !reqParams.order.empty()) {
        sql += " ORDER by " + reqParams.orderBy + " " + (reqParams.order == "ascend" ? "ASC" : "DESC");
    }

    sql += " LIMIT ? OFFSET ?";
    return sql;
}

bool DbSummaryDataBase::QueryStatisticTotalNum(Protocol::OperatorStatisticReqParams &reqParams, int64_t &total)
{
    OperatorGroupConverter::OperatorGroup operatorGroup = Protocol::OperatorGroupConverter::ToEnum(reqParams.group);
    bool isHccl = Protocol::OperatorGroupConverter::IsHccl(reqParams.group);
    std::string sql;
    if (isHccl) {
        sql = "SELECT COUNT(*) as nums "
            "FROM ("
            " SELECT SUBSTR(NAME.value, 1, INSTR(NAME.value, '__')) as opType,"
            " COMMUNICATION_OP.endNs as end_ns,COMMUNICATION_OP.startNs as start_ns "
            " FROM  COMMUNICATION_OP"
            " JOIN STRING_IDS AS NAME ON NAME.id = COMMUNICATION_OP.opName"
            " GROUP BY SUBSTR(NAME.value, 1, INSTR(NAME.value, '__')) "
            " ORDER by (end_ns - start_ns) DESC LIMIT ?"
            ") subquery";
    } else {
        std::string group = operatorGroup == OperatorGroupConverter::OperatorGroup::OP_TYPE_GROUP ?
            "op_type || accelerator_core" :
            R"(name || inputShapes || accelerator_core)";
        sql = " SELECT COUNT(*) as nums"
            " FROM ( "
            "     SELECT deviceId, startNs, endNs,"
            "     TASKTYPE.value AS accelerator_core, "
            "     OPTYPE.value AS op_type, NAME.value AS name FROM COMPUTE_TASK_INFO"
            "     JOIN TASK ON COMPUTE_TASK_INFO.globalTaskId = TASK.globalTaskId "
            "     JOIN STRING_IDS AS TASKTYPE ON TASKTYPE.id = COMPUTE_TASK_INFO.taskType"
            "     JOIN STRING_IDS AS OPTYPE ON OPTYPE.id = COMPUTE_TASK_INFO.opType"
            "     JOIN STRING_IDS AS NAME ON NAME.id = COMPUTE_TASK_INFO.name"
            "     WHERE accelerator_core <> 'HCCL' "
            "     GROUP by " + group +
            "     ORDER by ROUND(SUM(TASK.endNs - TASK.startNs) / 1000.0, 2) DESC LIMIT ?"
            " ) subquery";
    }
    if (!GenerateQueryFiltersSql<OperatorStatisticReqParams>(reqParams, sql)) {
        return false;
    }
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Failed to prepare sql to query statistic total num.", sqlite3_errmsg(db));
        return false;
    }
    auto resultSet = stmt->ExecuteQuery(reqParams.topK);
    if (resultSet == nullptr) {
        ServerLog::Error("Failed to exec query statistic total sql.");
        return false;
    }
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
    std::vector<Protocol::OperatorDetailInfoRes> sqlRes;
    if (!ExecSqlGetDetailInfo(sql, reqParams, sqlRes)) {
        ServerLog::Error("Failed to exec query detail sql.");
        return false;
    }
    std::vector<Protocol::OperatorDetailCmpInfoRes> resultData;
    for (auto &data : sqlRes) {
        OperatorDetailCmpInfoRes tmpInfo;
        tmpInfo.compare = data;
        resultData.emplace_back(tmpInfo);
    }
    response.pmuHeaders = FetchPmuColumnNames();
    response.datas = resultData;
    response.level = OperatorGetLevel(sqlRes);
    return true;
}

bool DbSummaryDataBase::QueryAllOperatorDetailInfo(Protocol::OperatorStatisticReqParams &reqParams,
                                                   std::vector<Protocol::OperatorDetailInfoRes> &res,
                                                   std::string &level)
{
    std::string sql = GenerateAllQueryDetailSql(reqParams);
    if (!ExecSqlGetDetailInfo(sql, reqParams, res)) {
        ServerLog::Error("Failed to exec query detail sql.");
        return false;
    } else {
        level = OperatorGetLevel(res);
    }
    return true;
}

bool DbSummaryDataBase::ExecSqlGetDetailInfo(std::string sql,
                                             Protocol::OperatorStatisticReqParams &reqParams,
                                             std::vector<Protocol::OperatorDetailInfoRes> &res)
{
    if (sql.empty()) {
        ServerLog::Error("Failed to generate query statistic sql.");
        return false;
    }
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Failed to get Detail Info. Msg:", sqlite3_errmsg(db), " ", result);
        return false;
    }
    uint64_t startTime = Timeline::TraceTime::Instance().GetStartTime();
    int index = bindStartIndex;
    sqlite3_bind_int64(stmt, index++, NumberUtil::CeilingClamp(startTime, (uint64_t)INT64_MAX));

    sqlite3_bind_int64(stmt, index++, reqParams.topK);
    if (!reqParams.isCompare) {
        sqlite3_bind_int64(stmt, index++, reqParams.pageSize);
        sqlite3_bind_int64(stmt, index++, (reqParams.current - 1) * reqParams.pageSize);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = 0;
        int columnCount = sqlite3_column_count(stmt);
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
        for (int i = col; i < columnCount; i++) {
            std::string columnValue = sqlite3_column_string(stmt, i);
            one.pmuDatas.push_back(columnValue);
        }
        res.emplace_back(one);
    }
    sqlite3_finalize(stmt);
    return true;
}
// LCOV_EXCL_BR_STOP

bool DbSummaryDataBase::QueryMoreInfoTotalNum(OperatorMoreInfoReqParams &reqParams, int64_t &total)
{
    OperatorGroupConverter::OperatorGroup operatorGroup = Protocol::OperatorGroupConverter::ToEnum(reqParams.group);
    bool isHccl = Protocol::OperatorGroupConverter::IsHccl(reqParams.group);
    std::string sql;
    if (isHccl) {
        std::string name = (operatorGroup == OperatorGroupConverter::OperatorGroup::HCCL_TYPE_GROUP) ?
            " SUBSTR(NAME.value, 1, INSTR(NAME.value, '__')) " : " NAME.value ";
        sql = "SELECT COUNT(*) as nums FROM ( SELECT " + name + " as name FROM COMMUNICATION_OP"
            "   JOIN STRING_IDS AS NAME ON NAME.id = COMMUNICATION_OP.opName WHERE " + name + " = ? ) subquery";
    } else {
        GenerateMoreInfoTotalNumForOther(sql, operatorGroup);
    }
    if (!GenerateQueryFiltersSql<OperatorMoreInfoReqParams>(reqParams, sql)) {
        return false;
    }

    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Failed to get More Total Num. Msg: ", sqlite3_errmsg(db), " ", result);
        return false;
    }
    int index = bindStartIndex;
    if (!isHccl) {
        sqlite3_bind_text(stmt, index++, reqParams.accCore.c_str(), -1, SQLITE_TRANSIENT);
    }
    if (operatorGroup == OperatorGroupConverter::OperatorGroup::OP_TYPE_GROUP ||
        operatorGroup == OperatorGroupConverter::OperatorGroup::HCCL_TYPE_GROUP) {
        sqlite3_bind_text(stmt, index++, reqParams.opType.c_str(), -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_text(stmt, index++, reqParams.opName.c_str(), -1, SQLITE_TRANSIENT);
    }
    if (operatorGroup == OperatorGroupConverter::OperatorGroup::OP_NAME_GROUP ||
        operatorGroup == OperatorGroupConverter::OperatorGroup::OP_INPUT_SHAPE_GROUP) {
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
    OperatorGroupConverter::OperatorGroup operatorGroup = Protocol::OperatorGroupConverter::ToEnum(reqParams.group);
    bool isHccl = Protocol::OperatorGroupConverter::IsHccl(reqParams.group);
    std::string sql;
    if (isHccl) {
        sql = GenerateQueryMoreInfoSqlForHCCL(sql);
    } else {
        sql = GenerateQueryMoreInfoSqlForOther(sql);
    }
    if (operatorGroup == OperatorGroupConverter::OperatorGroup::OP_TYPE_GROUP ||
        operatorGroup == OperatorGroupConverter::OperatorGroup::HCCL_TYPE_GROUP) {
        sql += " WHERE op_type = ?";
    } else {
        sql += " WHERE name = ? ";
    }
    if (operatorGroup == OperatorGroupConverter::OperatorGroup::OP_NAME_GROUP ||
        operatorGroup == OperatorGroupConverter::OperatorGroup::OP_INPUT_SHAPE_GROUP) {
        sql += "AND input_shapes = ?";
    }
    if (!GenerateQueryMoreInfoFilters(reqParams, sql)) {
        return "";
    }

    if (!StringUtil::CheckSqlValid(reqParams.orderBy)) {
        ServerLog::Error("There is an SQL injection attack on the parameter of orderBy"
                         "to generate query more info sql.");
    } else if (!reqParams.orderBy.empty() && !reqParams.order.empty()) {
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
        ServerLog::Error("Failed to get Op More Info. Msg: ", sqlite3_errmsg(db), " ", result);
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
    response.level = OperatorGetLevel(res);
    response.datas = res;
    sqlite3_finalize(stmt);
    return true;
}

void DbSummaryDataBase::BindSqliteParam(sqlite3_stmt *stmt, Protocol::OperatorMoreInfoReqParams &reqParams)
{
    uint64_t startTime = Timeline::TraceTime::Instance().GetStartTime();
    int index = bindStartIndex;
    sqlite3_bind_int64(stmt, index++, NumberUtil::CeilingClamp(startTime, (uint64_t)INT64_MAX));
    OperatorGroupConverter::OperatorGroup operatorGroup = Protocol::OperatorGroupConverter::ToEnum(reqParams.group);
    bool isHccl = Protocol::OperatorGroupConverter::IsHccl(reqParams.group);
    if (!isHccl) {
        sqlite3_bind_text(stmt, index++, reqParams.accCore.c_str(), -1, SQLITE_TRANSIENT);
    }
    if (operatorGroup == OperatorGroupConverter::OperatorGroup::OP_TYPE_GROUP ||
        operatorGroup == OperatorGroupConverter::OperatorGroup::HCCL_TYPE_GROUP) {
        sqlite3_bind_text(stmt, index++, reqParams.opType.c_str(), -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_text(stmt, index++, reqParams.opName.c_str(), -1, SQLITE_TRANSIENT);
    }
    if (operatorGroup == OperatorGroupConverter::OperatorGroup::OP_NAME_GROUP ||
        operatorGroup == OperatorGroupConverter::OperatorGroup::OP_INPUT_SHAPE_GROUP) {
        sqlite3_bind_text(stmt, index++, reqParams.shape.c_str(), -1, SQLITE_TRANSIENT);
    }
    sqlite3_bind_int64(stmt, index++, reqParams.pageSize);
    sqlite3_bind_int64(stmt, index++, (reqParams.current - 1) * reqParams.pageSize);
}

// LCOV_EXCL_BR_START
bool DbSummaryDataBase::QueryCommunicationOpDetail(Protocol::CommunicationDetailParams params,
    std::vector<Protocol::CommunicationDetail> &commDetails)
{
    std::string sql = GetCommSql(params);
    std::string timeFlag = params.timeFlag;
    uint64_t startTime = Timeline::TraceTime::Instance().GetStartTime();
    double offset = (params.currentPage - 1) * params.pageSize;
    sqlite3_stmt *stmt = nullptr;
    int index = bindStartIndex;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Query common detail failed! Failed to prepare sql.", sqlite3_errmsg(db));
        return false;
    }
    sqlite3_bind_int64(stmt, index++, NumberUtil::CeilingClamp(startTime, (uint64_t)INT64_MAX));
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
    std::string sql = "SELECT name, op_type as type, CASE WHEN start_time == 0 THEN 'NA' "
                      "ELSE ROUND((start_time - ?) / (1000.0 * 1000.0), 4) END AS startTime, "
                      "ROUND(duration, 4) as duration, ROUND(wait_time, 4) as waitTime FROM ( "
                      " SELECT NAME.value AS name, OPTYPE.value AS op_type, "
                      " startNs as start_time, ROUND((endNs - startNs)/1000.0, 3) as duration, "
                      " TASKTYPE.value AS taskTypes, 0 as wait_time FROM "
                      + TABLE_COMPUTE_TASK_INFO +
                      "     JOIN STRING_IDS AS NAME ON NAME.id = COMPUTE_TASK_INFO.name"
                      "     JOIN STRING_IDS AS OPTYPE ON OPTYPE.id = COMPUTE_TASK_INFO.opType"
                      "     JOIN STRING_IDS AS TASKTYPE ON TASKTYPE.id = COMPUTE_TASK_INFO.taskType"
                      "     JOIN TASK ON COMPUTE_TASK_INFO.globalTaskId = TASK.globalTaskId "
                      "     WHERE taskTypes = ?"
                      "     GROUP BY TASK.globalTaskId"
                      " ) subquery ";

    if (!StringUtil::CheckSqlValid(request.orderBy)) {
        ServerLog::Error("There is an SQL injection attack on the parameter of orderBy to get common sql.");
    } else if (!request.orderBy.empty() && !request.order.empty()) {
        sql += " ORDER by " + request.orderBy + " " + (request.order == "ascend" ? "ASC" : "DESC");
    }

    sql += " LIMIT ? offset ?";
    return sql;
}

std::string DbSummaryDataBase::GenerateQueryCategoryDurationSql(Protocol::OperatorDurationReqParams &reqParams)
{
    OperatorGroupConverter::OperatorGroup operatorGroup = Protocol::OperatorGroupConverter::ToEnum(reqParams.group);
    if (operatorGroup == OperatorGroupConverter::OperatorGroup::UNKNOWN) {
        ServerLog::Error("Generate query category duration sql failed, unknown operator group.");
        return "";
    }
    bool isHccl = Protocol::OperatorGroupConverter::IsHccl(reqParams.group);
    if (isHccl) {
        return GenerateQueryCategoryDurationSqlForHCCL(operatorGroup);
    } else {
        std::string group;
        std::string name;
        std::string duration;
        if (operatorGroup == OperatorGroupConverter::OperatorGroup::OP_TYPE_GROUP) {
            name = "OPTYPE.value";
            group = " GROUP by op_type || task_type";
            duration = " ROUND(sum(endNs - startNs)/1000.0, 2) as duration";
        } else if (operatorGroup == OperatorGroupConverter::OperatorGroup::OP_NAME_GROUP) {
            name = "NAME.value";
            group = "";
            duration = " ROUND((endNs - startNs)/1000.0, 2) as duration";
        } else {
            name = R"(NAME.value || '[' || INPUTSHAPES.value || ']')";
            group = R"( GROUP by name || '[' || input_shapes || ']' || task_type)";
            duration = " ROUND(sum(endNs - startNs)/1000.0, 2) as duration";
        }
        std::string sql = " SELECT name, duration From ("
            " SELECT " +
            name + " as name," + duration +
            " ,TASKTYPE.value as task_type, OPTYPE.value as op_type, INPUTSHAPES.value as input_shapes"
            " FROM " +
            TABLE_COMPUTE_TASK_INFO +
            " JOIN TASK ON COMPUTE_TASK_INFO.globalTaskId = TASK.globalTaskId "
            "     JOIN STRING_IDS AS NAME ON NAME.id = COMPUTE_TASK_INFO.name"
            "     JOIN STRING_IDS AS OPTYPE ON OPTYPE.id = COMPUTE_TASK_INFO.opType"
            "     JOIN STRING_IDS AS INPUTSHAPES ON INPUTSHAPES.id = COMPUTE_TASK_INFO.inputShapes"
            "     JOIN STRING_IDS AS TASKTYPE ON TASKTYPE.id = COMPUTE_TASK_INFO.taskType"
            " WHERE task_type <> 'HCCL'" +
            group +
            " ORDER BY duration DESC LIMIT ?"
            " ) subquery";
        return sql;
    }
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
            " ROUND((endNs - startNs)/1000.0, 2) as duration" : " ROUND(sum(endNs - startNs)/1000.0, 2) as duration") +
            "     FROM " + TABLE_COMPUTE_TASK_INFO +
            "     JOIN TASK ON COMPUTE_TASK_INFO.globalTaskId = TASK.globalTaskId "
            "     JOIN STRING_IDS AS NAME ON NAME.id = COMPUTE_TASK_INFO.name"
            "     JOIN STRING_IDS AS OPTYPE ON OPTYPE.id = COMPUTE_TASK_INFO.opType"
            "     JOIN STRING_IDS AS INPUTSHAPES ON INPUTSHAPES.id = COMPUTE_TASK_INFO.inputShapes"
            "     JOIN STRING_IDS AS TASKTYPE ON TASKTYPE.id = COMPUTE_TASK_INFO.taskType"
            "     " + (reqParams.group == Protocol::OPERATOR_GROUP ? "" :
            "     GROUP BY " + group) +
            "     ORDER BY duration DESC LIMIT ?"
            " ) subquery" +
            " GROUP by taskTypes"
            " ORDER BY duration DESC";
    return sql;
}

bool DbSummaryDataBase::QueryDetailTotalNum(OperatorStatisticReqParams &reqParams, int64_t &total)
{
    bool isHccl = Protocol::OperatorGroupConverter::IsHccl(reqParams.group);
    std::string sql;
    if (isHccl) {
        sql = " SELECT COUNT(*) as nums FROM ("
            " SELECT *  FROM " +
            TABLE_COMMUNICATION_OP +
            "  LIMIT ?"
            " ) subquery";
    } else {
        sql = " SELECT COUNT(*) as nums"
            " FROM ("
            "     SELECT ROUND((endNs - startNs)/1000.0, 3) as duration, " + blockDimColumnName +
            "     , deviceId as rank_id, streamId as step_id,NAME.value AS name,"
            "     OPTYPE.value AS op_type,TASKTYPE.value as accelerator_core, startNs as start_time"
            " FROM " + TABLE_COMPUTE_TASK_INFO +
            "     JOIN TASK ON COMPUTE_TASK_INFO.globalTaskId = TASK.globalTaskId "
            "     JOIN STRING_IDS AS TASKTYPE ON TASKTYPE.id = COMPUTE_TASK_INFO.taskType"
            "     JOIN STRING_IDS AS OPTYPE ON OPTYPE.id = COMPUTE_TASK_INFO.opType"
            "     JOIN STRING_IDS AS NAME ON NAME.id = COMPUTE_TASK_INFO.name"
            "     WHERE accelerator_core <> 'HCCL'"
            "     ORDER BY duration DESC LIMIT ?"
            " ) subquery";
    }
    if (!GenerateQueryFiltersSql<OperatorStatisticReqParams>(reqParams, sql)) {
        return false;
    }
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Failed to get Detail Total Num. Msg: ", sqlite3_errmsg(db), " ", result);
        return false;
    }
    int index = bindStartIndex;
    sqlite3_bind_int64(stmt, index++, reqParams.topK);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        total = sqlite3_column_int64(stmt, resultStartIndex);
    }
    sqlite3_finalize(stmt);
    return true;
}
// LCOV_EXCL_BR_STOP

std::vector<std::string> DbSummaryDataBase::FetchPmuColumnNames()
{
    std::vector<std::string> columns;
    if (!CheckTableExist(TABLE_TASK_PMU_INFO)) {
        return columns;
    }
    std::string queryColumnSql = "SELECT STRING_IDS.value "
                                  "FROM STRING_IDS "
                                  "WHERE STRING_IDS.id IN ( "
                                  "    SELECT name "
                                  "    FROM TASK_PMU_INFO "
                                  "    WHERE globalTaskId = ( "
                                  "        SELECT globalTaskId "
                                  "        FROM TASK_PMU_INFO "
                                  "        ORDER BY RANDOM() "
                                  "        LIMIT 1 "
                                  "    ) "
                                  ");";
    sqlite3_stmt *stmt = nullptr;
    int result = sqlite3_prepare_v2(db, queryColumnSql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("Failed to get pmu cols Info. Msg:", sqlite3_errmsg(db), " ", result);
        return columns;
    }
    // 执行SQL查询并处理结果
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string colName = sqlite3_column_string(stmt, 0);
        // 表头只能是字母、数字、下划线、短线、空格
        if (!RegexUtil::RegexMatch(colName, R"(^[a-zA-Z0-9\s\-_]+$)")) {
            sqlite3_finalize(stmt);
            ServerLog::Error("There is an SQL injection attack on colName. error colName: %", colName);
            return {};
        }
        columns.push_back(colName);
    }

    // 释放资源
    sqlite3_finalize(stmt);
    return columns;
}

// STRING_IDS 和 TASK_PMU_INFO表联查，用 globalTaskId分组
std::string DbSummaryDataBase::CreatPMUTmpTableSql(std::vector<std::string> cols)
{
    if (cols.empty()) {
        return "";
    }
    std::string convertPmuDataSql = "SELECT i.globalTaskId ";
    for (auto const &col : cols) {
        convertPmuDataSql += ", MAX(CASE WHEN s.value = \'" + col + "\' THEN i.value END) AS " + col;
    }
    convertPmuDataSql += " FROM TASK_PMU_INFO i "
                         " JOIN STRING_IDS s ON i.name = s.id "
                         " GROUP BY i.globalTaskId ";
    
    return " LEFT JOIN ( " + convertPmuDataSql +  " ) AS PMU ON COMPUTE_TASK_INFO.globalTaskId = PMU.globalTaskId ";
}

std::string DbSummaryDataBase::GetPMUTmpTableColSql(const std::vector<std::string> &cols)
{
    if (cols.empty()) {
        return "";
    }
    std::vector<std::string> tmpCols;
    tmpCols.reserve(cols.size()); // 预留空间，避免多次分配内存
    for (const std::string &col : cols) {
        tmpCols.push_back("PMU." + col);
    }
    // 加上临时表的前缀
    return "," + StringUtil::join(tmpCols, ",");
}

// LCOV_EXCL_BR_START
std::string DbSummaryDataBase::GenerateQueryDetailSqlForOperator()
{
    std::vector<std::string> pmuClos = FetchPmuColumnNames();
    // JoinExtraColName、 GetPMUTmpTableColSql、 CreatPMUTmpTableSql 为PMU数据处理，如果没有返回""
    std::string sql = " SELECT rank_id, step_id, name, op_type, accelerator_core,"
        " CASE WHEN start_time == 0 THEN 'NA' ELSE  ROUND((start_time - ?) / (1000.0 * 1000.0), 2)"
        " END AS startTime, duration, wait_time, " + blockDimColumnName + ","
        " input_shapes, input_data_types, input_formats, output_shapes, output_data_types, output_formats "
        + JoinExtraColName(pmuClos) +
        " FROM ("
        "     SELECT " + blockDimColumnName + ", deviceId as rank_id, streamId as step_id,NAME.value AS name,"
        "     OPTYPE.value AS op_type,TASKTYPE.value as accelerator_core, startNs as start_time, "
        "     ROUND((endNs - startNs)/1000.0, 3) as duration, ROUND(waitNs/1000.0, 3) as wait_time, "
        "     INPUTSHAPES.value as input_shapes, INPUTDATATYPES.value as input_data_types, "
        "     INPUTFORMATS.value as input_formats, OUTPUTSHAPES.value as output_shapes, "
        "     OUTPUTDATATYPES.value as output_data_types, OUTPUTFORMATS.value as output_formats " +
              GetPMUTmpTableColSql(pmuClos) +
        " FROM " + TABLE_COMPUTE_TASK_INFO +
        "     JOIN TASK ON COMPUTE_TASK_INFO.globalTaskId = TASK.globalTaskId "
        "     JOIN STRING_IDS AS NAME ON NAME.id = COMPUTE_TASK_INFO.name"
        "     JOIN STRING_IDS AS OPTYPE ON OPTYPE.id = COMPUTE_TASK_INFO.opType"
        "     JOIN STRING_IDS AS TASKTYPE ON TASKTYPE.id = COMPUTE_TASK_INFO.taskType"
        "     JOIN STRING_IDS AS INPUTSHAPES ON INPUTSHAPES.id = COMPUTE_TASK_INFO.inputShapes"
        "     JOIN STRING_IDS AS INPUTDATATYPES ON INPUTDATATYPES.id = COMPUTE_TASK_INFO.inputDataTypes"
        "     JOIN STRING_IDS AS INPUTFORMATS ON INPUTFORMATS.id = COMPUTE_TASK_INFO.inputFormats"
        "     JOIN STRING_IDS AS OUTPUTSHAPES ON OUTPUTSHAPES.id = COMPUTE_TASK_INFO.outputShapes"
        "     JOIN STRING_IDS AS OUTPUTDATATYPES ON OUTPUTDATATYPES.id = COMPUTE_TASK_INFO.outputDataTypes"
        "     JOIN STRING_IDS AS OUTPUTFORMATS ON OUTPUTFORMATS.id = COMPUTE_TASK_INFO.outputFormats " +
              CreatPMUTmpTableSql(pmuClos) +
        "     WHERE accelerator_core <> 'HCCL' ORDER by duration DESC LIMIT ? ) subquery ";
    return sql;
}

std::string DbSummaryDataBase::GenerateAllQueryDetailSql(OperatorStatisticReqParams &reqParams)
{
    bool isHccl = Protocol::OperatorGroupConverter::IsHccl(reqParams.group);
    std::string sql;
    if (isHccl) {
        sql = GenerateQueryDetailSqlForHCCL(sql);
    } else {
        sql = GenerateQueryDetailSqlForOperator();
    }
    if (!GenerateQueryFiltersSql<OperatorStatisticReqParams>(reqParams, sql)) {
        return "";
    }
    if (!StringUtil::CheckSqlValid(reqParams.orderBy)) {
        ServerLog::Error("There is an SQL injection attack on the parameter of orderBy"
                         "to generate all query detail sql.");
    } else if (!reqParams.orderBy.empty() && !reqParams.order.empty()) {
        sql += " ORDER by " + reqParams.orderBy + " " + (reqParams.order == "ascend" ? "ASC" : "DESC");
    }
    return sql;
}

std::string DbSummaryDataBase::GenerateQueryDetailSql(OperatorStatisticReqParams &reqParams)
{
    std::string sql = GenerateAllQueryDetailSql(reqParams);
    if (std::empty(sql)) {
        return sql;
    }
    sql += " LIMIT ? OFFSET ?";
    return sql;
}
// LCOV_EXCL_BR_STOP

std::string DbSummaryDataBase::GenerateQueryCategoryDurationSqlForHCCL(
    const OperatorGroupConverter::OperatorGroup &operatorGroup)
{
    std::string group;
    std::string name;
    std::string duration;
    if (operatorGroup == OperatorGroupConverter::OperatorGroup::HCCL_TYPE_GROUP) {
        name = "SUBSTR(NAME.value, 1, INSTR(NAME.value, '__')) as name ";
        group = " GROUP by SUBSTR(NAME.value, 1, INSTR(NAME.value, '__')) ";
        duration = " ROUND(sum(COMMUNICATION_OP.endNs - COMMUNICATION_OP.startNs)/1000.0, 2) as duration";
    } else if (operatorGroup == OperatorGroupConverter::OperatorGroup::HCCL_NAME_GROUP) {
        name = "NAME.value as name";
        group = "";
        duration = " ROUND((COMMUNICATION_OP.endNs - COMMUNICATION_OP.startNs)/1000.0, 2) as duration";
    }
    std::string sql = " SELECT name, duration From ("
        " SELECT " +
        name + "," + duration + " FROM " + TABLE_COMMUNICATION_OP +
        " JOIN STRING_IDS AS NAME ON NAME.id = COMMUNICATION_OP.opName"
        " " +
        group +
        " ORDER BY duration DESC LIMIT ?"
        " ) subquery";
    return sql;
}

std::string &DbSummaryDataBase::GenerateQueryMoreInfoSqlForHCCL(std::string &sql) const
{
    sql = " SELECT rank_id, step_id, name, op_type, accelerator_core,"
        " CASE WHEN start_time == 0 THEN 'NA' ELSE  ROUND((start_time - ?) / (1000.0 * 1000.0), 2)"
        " END AS startTime, duration, wait_time, NULL AS " + blockDimColumnName + ", NULL AS input_shapes,"
        " NULL AS input_data_types, NULL AS input_formats, NULL AS output_shapes, NULL AS output_data_types,"
        " NULL AS output_formats"
        " FROM ("
        "  SELECT NULL as rank_id, NULL as step_id, NAME.value AS name,"
        "  SUBSTR(NAME.value, 1, INSTR(NAME.value, '__')) as op_type,"
        "  NULL as accelerator_core,COMMUNICATION_OP.startNs as start_time,"
        "  ROUND((COMMUNICATION_OP.endNs - COMMUNICATION_OP.startNs)/1000.0, 3) as duration,"
        "  ROUND(COMMUNICATION_OP.waitNs/1000.0, 3) as wait_time FROM COMMUNICATION_OP"
        "  JOIN STRING_IDS AS NAME ON NAME.id = COMMUNICATION_OP.opName"
        "  ORDER by duration DESC ) subquery ";
    return sql;
}

std::string &DbSummaryDataBase::GenerateQueryMoreInfoSqlForOther(std::string &sql) const
{
    sql = " SELECT rank_id, step_id, name, op_type, accelerator_core,"
          " CASE WHEN start_time == 0 THEN 'NA' ELSE ROUND((start_time - ?) / (1000.0 * 1000.0), 2)"
          " END AS startTime, duration, wait_time, " + blockDimColumnName + ","
          " input_shapes, input_data_types, input_formats, output_shapes, output_data_types, output_formats"
          " FROM ("
          "     SELECT " + blockDimColumnName + ", deviceId as rank_id, streamId as step_id, "
          "     NAME.value AS name,  OPTYPE.value AS op_type,"
          "  TASKTYPE.value as accelerator_core,startNs as start_time,ROUND((endNs - startNs)/1000.0, 3) as duration,"
          "     ROUND((waitNs)/1000.0, 3) as wait_time, INPUTSHAPES.value as input_shapes, "
          "     INPUTDATATYPES.value as input_data_types, "
          "     INPUTFORMATS.value as input_formats, OUTPUTSHAPES.value as output_shapes, "
          "     OUTPUTDATATYPES.value as output_data_types, OUTPUTFORMATS.value as output_formats "
          "     FROM " + TABLE_COMPUTE_TASK_INFO + " JOIN TASK ON COMPUTE_TASK_INFO.globalTaskId = TASK.globalTaskId "
          "     JOIN STRING_IDS AS NAME ON NAME.id = COMPUTE_TASK_INFO.name"
          "     JOIN STRING_IDS AS OPTYPE ON OPTYPE.id = COMPUTE_TASK_INFO.opType"
          "     JOIN STRING_IDS AS TASKTYPE ON TASKTYPE.id = COMPUTE_TASK_INFO.taskType"
          "     JOIN STRING_IDS AS INPUTSHAPES ON INPUTSHAPES.id = COMPUTE_TASK_INFO.inputShapes"
          "     JOIN STRING_IDS AS INPUTDATATYPES ON INPUTDATATYPES.id = COMPUTE_TASK_INFO.inputDataTypes"
          "     JOIN STRING_IDS AS INPUTFORMATS ON INPUTFORMATS.id = COMPUTE_TASK_INFO.inputFormats"
          "     JOIN STRING_IDS AS OUTPUTSHAPES ON OUTPUTSHAPES.id = COMPUTE_TASK_INFO.outputShapes"
          "     JOIN STRING_IDS AS OUTPUTDATATYPES ON OUTPUTDATATYPES.id = COMPUTE_TASK_INFO.outputDataTypes"
          "     JOIN STRING_IDS AS OUTPUTFORMATS ON OUTPUTFORMATS.id = COMPUTE_TASK_INFO.outputFormats"
          "     WHERE accelerator_core = ?"
          "     ORDER by duration DESC ) subquery ";
    return sql;
}

std::string &DbSummaryDataBase::GenerateQueryDetailSqlForHCCL(std::string &sql) const
{
    sql = " SELECT rank_id, step_id, name, op_type, accelerator_core,"
          " CASE WHEN start_time == 0 THEN 'NA' ELSE  ROUND((start_time - ?) / (1000.0 * 1000.0), 2)"
          " END AS startTime, duration, wait_time, NULL AS " + blockDimColumnName + ", NULL AS input_shapes,"
          " NULL AS input_data_types, NULL AS input_formats, NULL AS output_shapes, NULL AS output_data_types,"
          " NULL AS output_formats FROM ("
          "  SELECT NULL as rank_id, NULL as step_id, NAME.value AS name,"
          "  SUBSTR(NAME.value, 1, INSTR(NAME.value, '__')) as op_type,"
          "  NULL as accelerator_core,COMMUNICATION_OP.startNs as start_time,"
          "  ROUND((COMMUNICATION_OP.endNs - COMMUNICATION_OP.startNs)/1000.0, 3) as duration,"
          "  ROUND(COMMUNICATION_OP.waitNs/1000.0, 3) as wait_time FROM COMMUNICATION_OP"
          "  JOIN STRING_IDS AS NAME ON NAME.id = COMMUNICATION_OP.opName"
          "  ORDER by duration DESC LIMIT ? ) subquery ";
    return sql;
}

void DbSummaryDataBase::GenerateMoreInfoTotalNumForOther(std::string &sql,
                                                         OperatorGroupConverter::OperatorGroup opGroup) const
{
    std::string condition = (opGroup == OperatorGroupConverter::OperatorGroup::OP_TYPE_GROUP) ?
                            " op_type = ?" : " name = ? AND input_shapes = ?";
    sql = " SELECT COUNT(*) as nums FROM ("
          "     SELECT NAME.value AS name, INPUTSHAPES.value AS input_shapes, TASKTYPE.value AS accelerator_core, "
          "     OPTYPE.value AS op_type"
          "     FROM " + TABLE_COMPUTE_TASK_INFO +
          "     JOIN TASK ON COMPUTE_TASK_INFO.globalTaskId = TASK.globalTaskId"
          "     JOIN STRING_IDS AS NAME ON NAME.id = COMPUTE_TASK_INFO.name"
          "     JOIN STRING_IDS AS OPTYPE ON OPTYPE.id = COMPUTE_TASK_INFO.opType"
          "     JOIN STRING_IDS AS INPUTSHAPES ON INPUTSHAPES.id = COMPUTE_TASK_INFO.inputShapes"
          "     JOIN STRING_IDS AS TASKTYPE ON TASKTYPE.id = COMPUTE_TASK_INFO.taskType"
          "     WHERE accelerator_core = ? AND" + condition + " ) subquery";
}

template <typename T>
bool DbSummaryDataBase::GenerateQueryFiltersSql(T &reqParams, std::string &sql)
{
    if (reqParams.filters.empty()) {
        return true;
    }
    sql += " WHERE ";
    for (size_t index = 0; index < reqParams.filters.size(); index++) {
        std::pair<std::string, std::string> filter = reqParams.filters[index];
        if (!StringUtil::CheckSqlValid(filter.first) || !StringUtil::CheckSqlValid(filter.second)) {
            ServerLog::Error("There is an SQL injection attack on the parameter of filter"
                             "to generate query filter sql.");
            return false;
        }
        if (index != 0) {
            sql += " AND ";
        }
        sql += filter.first + " LIKE '%" + filter.second + "%' ";
    }
    return true;
}

bool DbSummaryDataBase::GenerateQueryMoreInfoFilters(OperatorMoreInfoReqParams &reqParams, std::string &sql)
{
    for (const auto &filter: reqParams.filters) {
        if (!StringUtil::CheckSqlValid(filter.first) || !StringUtil::CheckSqlValid(filter.second)) {
            ServerLog::Error("There is an SQL injection attack on the parameter of filter"
                             "to generate query more info filters.");
            return false;
        }
        sql += " AND " + filter.first + " LIKE '%" + filter.second + "%' ";
    }
    return true;
}

void DbSummaryDataBase::ParserEnd(const std::string &fileId, bool result, const std::string &msg)
{
    WsSession *session = WsSessionManager::Instance().GetSession();
    if (session == nullptr) {
        ServerLog::Error("Failed to get session for summary callback.");
        return;
    }
    if (fileId.empty()) {
        auto event = std::make_unique<Protocol::ModuleResetEvent>();
        event->moduleName = MODULE_OPERATOR;
        event->result = true;
        event->reset = true;
        session->OnEvent(std::move(event));
    } else {
        auto event = std::make_unique<Protocol::OperatorParseStatusEvent>();
        event->moduleName = MODULE_OPERATOR;
        event->result = true;
        event->data.rankId = fileId;
        event->data.status = result;
        event->data.error = msg;
        session->OnEvent(std::move(event));
    }
}

void DbSummaryDataBase::Reset()
{
    ServerLog::Info("Summary reset. wait task completed.");
    ServerLog::Info("Summary task completed.");
    auto databaseList = Timeline::DataBaseManager::Instance().GetAllSummaryDatabase();
    for (auto &db: databaseList) {
        auto database = dynamic_cast<DbSummaryDataBase*>(db);
        if (database != nullptr) {
            database->CloseDb();
        }
    }
    Timeline::DataBaseManager::Instance().Clear(Timeline::DatabaseType::SUMMARY);
}
}
