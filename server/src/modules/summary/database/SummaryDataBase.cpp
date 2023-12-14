/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "SummaryDataBase.h"
#include "ServerLog.h"
#include "SummaryProtocolRequest.h"
#include "SummaryProtocolResponse.h"
#include "TraceTime.h"
#include "OperatorProtocol.h"

namespace Dic {
namespace Module {
namespace Summary {
using namespace Server;
SummaryDataBase::~SummaryDataBase()
{
    if (hasInitStmt) {
        ReleaseStmt();
    }
    CloseDb();
}

bool SummaryDataBase::SetConfig()
{
    if (!isOpen) {
        ServerLog::Error("Failed to set config. Database is not open.");
        return false;
    }
    return ExecSql("PRAGMA synchronous = OFF; PRAGMA journal_mode = MEMORY;");
}

bool SummaryDataBase::CreateTable()
{
    if (!isOpen) {
        ServerLog::Error("Failed to set config. Database is not open.");
        return false;
    }
    std::string sql =
        "CREATE TABLE " + kernelTable + " (id INTEGER PRIMARY KEY AUTOINCREMENT, rank_id TEXT, step_id TEXT, " +
        "name TEXT, op_type TEXT, accelerator_core TEXT, start_time INTEGER, duration INTEGER, wait_time INTEGER, " +
        "block_dim INTEGER, input_shapes TEXT, input_data_types TEXT, input_formats TEXT, output_shapes TEXT, " +
        "output_data_types TEXT, output_formats TEXT);" +
        "CREATE INDEX rank_index ON " + kernelTable + " (rank_id);";
    return ExecSql(sql);
}

bool SummaryDataBase::InitStmt()
{
    if (hasInitStmt) {
        return true;
    }
    std::string sql =
            "INSERT INTO " + kernelTable + " (rank_id, step_id, name, op_type, accelerator_core, start_time, " +
            "duration, wait_time, block_dim, input_shapes, input_data_types, input_formats, output_shapes, " +
            "output_data_types, output_formats)" + " VALUES (?,?,?,?,?,round(? * 1000),?,?,?,?,?,?,?,?,?)";
    for (int i = 0; i < cacheSize - 1; ++i) {
        sql.append(",(?,?,?,?,?,round(? * 1000),?,?,?,?,?,?,?,?,?)");
    }
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &insertKernelStmt, nullptr) != SQLITE_OK) {
        ServerLog::Error("Failed to prepare insert kernel detail statement. error:", sqlite3_errmsg(db));
        return false;
    }

    hasInitStmt = true;
    return true;
}

void SummaryDataBase::ReleaseStmt()
{
    if (insertKernelStmt != nullptr) {
        insertKernelStmt = nullptr;
    }
}

void SummaryDataBase::InsertKernelDetailList(std::vector<Kernel> kernelVec)
{
    sqlite3_stmt *stmt = GetKernelStmt(kernelVec.size());
    if (stmt == nullptr) {
        ServerLog::Error("Failed to get kernel stmt.");
        return;
    }
    int idx = bindStartIndex;
    for (const auto &event : kernelVec) {
        sqlite3_bind_text(stmt, idx++, event.rankId.c_str(), event.rankId.length(), SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, idx++, event.stepId.c_str(), event.stepId.length(), SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, idx++, event.name.c_str(), event.name.length(), SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, idx++, event.type.c_str(), event.type.length(), SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, idx++, event.acceleratorCore.c_str(), event.acceleratorCore.length(), SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, idx++, event.startTime);
        sqlite3_bind_double(stmt, idx++, event.duration);
        sqlite3_bind_double(stmt, idx++, event.waitTime);
        sqlite3_bind_int64(stmt, idx++, event.blockDim);
        sqlite3_bind_text(stmt, idx++, event.inputShapes.c_str(), event.inputShapes.length(), SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, idx++, event.inputDataTypes.c_str(), event.inputDataTypes.length(), SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, idx++, event.inputFormats.c_str(), event.inputFormats.length(), SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, idx++, event.outputShapes.c_str(), event.outputShapes.length(), SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, idx++, event.outputDataTypes.c_str(), event.outputDataTypes.length(), SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, idx++, event.outputFormats.c_str(), event.outputFormats.length(), SQLITE_TRANSIENT);
    }
    auto result = sqlite3_step(stmt);
    if (kernelVec.size() != cacheSize) {
        sqlite3_finalize(stmt);
    }
    if (result != SQLITE_DONE) {
        ServerLog::Error("Insert kernel detail fail. ", sqlite3_errmsg(db));
        return;
    }
}

void SummaryDataBase::InsertKernelDetail(Kernel kernel)
{
    kernelCache.emplace_back(kernel);
    if (kernelCache.size() == cacheSize) {
        InsertKernelDetailList(kernelCache);
        kernelCache.clear();
    }
}

void SummaryDataBase::SaveKernelDetail()
{
    if (kernelCache.size() > 0) {
        InsertKernelDetailList(kernelCache);
        kernelCache.clear();
    }
}

bool SummaryDataBase::HasParseKernelFile(const std::string &kernelFile)
{
    return std::count(kernelFiles.begin(), kernelFiles.end(), kernelFile) != 0;
}

void SummaryDataBase::AddParseKernelFile(const std::string &kernelFile)
{
    kernelFiles.emplace_back(kernelFile);
}

void SummaryDataBase::ClearParseKernelFile()
{
    kernelFiles.clear();
}

sqlite3_stmt *SummaryDataBase::GetKernelStmt(uint64_t paramLen)
{
    sqlite3_stmt *stmt = nullptr;
    if (paramLen == cacheSize) {
        stmt = insertKernelStmt;
        sqlite3_reset(stmt);
    } else {
        std::string sql =
                "INSERT INTO " + kernelTable + " (rank_id, step_id, name, op_type, accelerator_core, start_time, " +
                "duration, wait_time, block_dim, input_shapes, input_data_types, input_formats, output_shapes, " +
                "output_data_types, output_formats)" + " VALUES (?,?,?,?,?,round(? * 1000),?,?,?,?,?,?,?,?,?)";
        for (int i = 0; i < paramLen - 1; ++i) {
            sql.append(",(?,?,?,?,?,round(? * 1000),?,?,?,?,?,?,?,?,?)");
        }
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            ServerLog::Error("Failed to prepare insert Kernel stat. error:", sqlite3_errmsg(db));
            return nullptr;
        }
    }
    return stmt;
}

bool SummaryDataBase::QueryComputeDetailHandler(Protocol::ComputeDetailParams params,
                                                std::vector<Protocol::ComputeDetail> &computeDetails)
{
    std::string sql = GenComputeSql(params);
    std::string timeFlag = params.timeFlag;
    uint64_t startTime = Timeline::TraceTime::Instance().GetStartTime();
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
        computeDetail.startTime = sqlite3_column_double(stmt, col++);
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

std::string SummaryDataBase::GenComputeSql(Protocol::ComputeDetailParams request)
{
    std::string orderList = request.orderBy;
    double offset = (request.currentPage - 1) * request.pageSize;
    std::string ascend;
    if (request.order == "ascend") {
        ascend = "ASC";
    } else {
        ascend = "DESC";
    }
    std::string sql = "";
    if (orderList.size() == 0) {
        sql = "SELECT name, op_type as type, ROUND((start_time - ?) / (1000.0 * 1000.0), 4) as startTime, "
              "duration, wait_time as waitTime, block_dim as blockDim, "
              "input_shapes as inputShapes, input_data_types as inputDataTypes, input_formats as inputFormats, "
              "output_shapes as outputShapes, output_data_types as outputDataTypes, output_formats as outputFormats "
              "FROM " + kernelTable +
              " WHERE accelerator_core = ?  LIMIT ? offset ?";
    } else {
        sql = "SELECT name, op_type as type, ROUND((start_time - ?) / (1000.0 * 1000.0), 4) as startTime, duration, "
              "wait_time as waitTime, block_dim as blockDim, "
              "input_shapes as inputShapes, input_data_types as inputDataTypes, input_formats as inputFormats, "
              "output_shapes as outputShapes, output_data_types as outputDataTypes, output_formats as outputFormats "
              "FROM " + kernelTable +
              " WHERE accelerator_core = ?  ORDER BY " + orderList + " " + ascend + " LIMIT ? offset ?";
    }
    return sql;
}

bool SummaryDataBase::QueryGetTotalNum(std::string name, int64_t &totalNum)
{
    sqlite3_stmt *stmt = nullptr;
    std::string sql = "SELECT count(*) as nums FROM " + kernelTable + " WHERE accelerator_core = ?";
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

std::string SummaryDataBase::GetCommSql(Protocol::CommunicationDetailParams request)
{
    std::string order = request.orderBy;
    std::string ascend;
    if (request.order == "ascend") {
        ascend = "ASC";
    } else {
        ascend = "DESC";
    }
    std::string sql = "";
    if (order.size() == 0) {
        sql = "SELECT name, op_type as type, ROUND((start_time - ?) / (1000.0 * 1000.0), 4) as startTime, "
              "ROUND(duration, 4) as duration, ROUND(wait_time, 4) as waitTime FROM " + kernelTable +
              " WHERE accelerator_core = ?  LIMIT ? offset ?";
    } else {
        sql = "SELECT name, op_type as type, ROUND((start_time - ?) / (1000.0 * 1000.0), 4) as startTime, "
              "ROUND(duration, 4) as duration, ROUND(wait_time, 4) as waitTime FROM " + kernelTable +
              " WHERE accelerator_core = ?  ORDER BY " + order + " " + ascend + " LIMIT ? offset ?";
    }
    return sql;
}

bool SummaryDataBase::QueryCommDetailHandler(Protocol::CommunicationDetailParams params,
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
        ServerLog::Error("QueryCommDetailHandler failed! Failed to prepare sql.", sqlite3_errmsg(db));
        return false;
    }
    sqlite3_bind_int64(stmt, index++, startTime);
    sqlite3_bind_text(stmt, index++, params.timeFlag.c_str(), params.timeFlag.length(), nullptr);
    sqlite3_bind_double(stmt, index++, params.pageSize);
    sqlite3_bind_double(stmt, index++, offset);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        Protocol::CommunicationDetail computeDetail{};
        computeDetail.name = sqlite3_column_string(stmt, col++);
        computeDetail.type = sqlite3_column_string(stmt, col++);
        computeDetail.startTime = sqlite3_column_double(stmt, col++);
        computeDetail.duration = sqlite3_column_double(stmt, col++);
        computeDetail.waitTime = sqlite3_column_double(stmt, col++);
        commDetails.emplace_back(computeDetail);
    }

    sqlite3_finalize(stmt);
    return true;
}

    std::string SummaryDataBase::GenerateQueryCategoryDurationSql(Protocol::OperatorDurationReqParams &reqParams)
    {
        std::string group;
        if (reqParams.group == Protocol::OP_TYPE_GROUP) {
            group = "op_type";
        } else if (reqParams.group == Protocol::OPERATOR_GROUP) {
            group = "name";
        } else {
            group = R"(name || '[' || input_shapes || ']')";
        }
        std::string sql =
                " SELECT " + group + " as name, ROUND(sum(duration), 2) as duration" +
                " FROM " + kernelTable +
                " WHERE rank_id = '" + reqParams.rankId + "' AND accelerator_core <> 'HCCL'"
                " GROUP by " + group +
                " ORDER BY duration DESC LIMIT " + std::to_string(reqParams.topK);
        return sql;
    }

    std::string SummaryDataBase::GenerateQueryComputeUnitDurationSql(Protocol::OperatorDurationReqParams &reqParams)
    {
        std::string group;
        if (reqParams.group == Protocol::OP_TYPE_GROUP) {
            group = "op_type";
        } else if (reqParams.group == Protocol::OPERATOR_GROUP) {
            group = "name";
        } else {
            group = R"(name || '[' || input_shapes || ']')";
        }
        std::string sql =
                " SELECT accelerator_core as name, ROUND(SUM(duration), 2) as duration"
                " FROM ("
                "     SELECT " + group + ", accelerator_core, ROUND(SUM(duration), 2) as duration" +
                "     FROM " + kernelTable +
                "     WHERE rank_id = '" + reqParams.rankId + "' AND accelerator_core <> 'HCCL'"
                "     GROUP BY " + group +
                "     ORDER BY duration DESC LIMIT " + std::to_string(reqParams.topK) +
                " ) subquery" +
                " GROUP by accelerator_core"
                " ORDER BY duration DESC";
        return sql;
    }

    bool SummaryDataBase::QueryOperatorDurationInfo(Protocol::OperatorDurationReqParams &reqParams,
        Protocol::QueryType type, std::vector<Protocol::OperatorDurationRes> &datas)
    {
        std::string sql;
        if (type == Protocol::QueryType::CATEGORY) {
            sql = GenerateQueryCategoryDurationSql(reqParams);
        } else {
            sql = GenerateQueryComputeUnitDurationSql(reqParams);
        }

        ServerLog::Info("[Operator]Query Operator Duration Info SQL: ", sql);
        sqlite3_stmt *stmt = nullptr;
        int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        if (result != SQLITE_OK) {
            ServerLog::Error("[Operator]Failed to prepare sql of DurationInfo. ", sqlite3_errmsg(db), " ", result);
            return false;
        }

        std::vector<Protocol::OperatorDurationRes> res;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Protocol::OperatorDurationRes one{};
            int col = 0;
            one.name = sqlite3_column_string(stmt, col++);
            one.duration = sqlite3_column_double(stmt, col++);
            // 限制能够显示的最大数目为50
            if (res.size() >= maxCategorySize) {
                res[maxCategorySize -1].name = "Others";
                res[maxCategorySize -1].duration += one.duration;
            } else {
                res.emplace_back(one);
            }
        }
        datas = res;

        sqlite3_finalize(stmt);
        return true;
    }

    bool SummaryDataBase::QueryStatisticTotalNum(Protocol::OperatorStatisticReqParams &reqParams, int64_t &total)
    {
        sqlite3_stmt *stmt = nullptr;
        std::string sql =
                " SELECT COUNT(*) as nums"
                " FROM ("
                "     SELECT *"
                "     FROM " + kernelTable +
                "     WHERE rank_id = '" + reqParams.rankId + "' AND accelerator_core <> 'HCCL'"
                "     GROUP by " + (reqParams.group == "Operator Type" ? "op_type" : R"(name || input_shapes)") +
                "     ORDER by duration DESC LIMIT " + std::to_string(reqParams.topK) +
                " ) subquery";
        ServerLog::Info("[Operator]Query Statistic Total Num sql: ", sql);
        int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        if (result != SQLITE_OK) {
            ServerLog::Error("[Operator]Failed to prepare sql of StatisticTotalNum.", sqlite3_errmsg(db), " ", result);
            return false;
        }
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            total = sqlite3_column_int64(stmt, resultStartIndex);
        }
        sqlite3_finalize(stmt);
        return true;
    }

    std::string SummaryDataBase::GenerateQueryStatisticSql(Protocol::OperatorStatisticReqParams &reqParams)
    {
        std::string group;
        std::string name;
        if (reqParams.group == Protocol::OP_TYPE_GROUP) {
            group = "op_type";
            name = "''";
        } else {
            group = R"(name || input_shapes)";
            name = "name";
        }
        std::string sql =
                " SELECT * FROM ("
                "     SELECT op_type, " + name + " as name, input_shapes, accelerator_core,"
                "     ROUND(SUM(duration), 2) as total_time, COUNT(0) as cnt,"
                "     ROUND(SUM(duration) / COUNT(0), 2) as avg_time,"
                "     ROUND(max(duration), 2) as max_time,"
                "     ROUND(min(duration), 2) as min_time"
                "     FROM " + kernelTable +
                "     WHERE rank_id = '" + reqParams.rankId + "' AND accelerator_core <> 'HCCL'"
                "     GROUP by " + group +
                "     ORDER by total_time DESC LIMIT " + std::to_string(reqParams.topK) +
                " ) subquery ";

        if (!reqParams.orderBy.empty() && !reqParams.order.empty()) {
            sql += " ORDER by " + reqParams.orderBy + " " + (reqParams.order == "ascend" ? "ASC" : "DESC");
        }

        sql += " LIMIT " + std::to_string(reqParams.pageSize) +
                " OFFSET " + std::to_string(reqParams.pageSize * (reqParams.current - 1));
        return sql;
    }

    bool SummaryDataBase::QueryOperatorStatisticInfo(Protocol::OperatorStatisticReqParams &reqParams,
        Protocol::OperatorStatisticInfoResponse &response)
    {
        if (reqParams.group != Protocol::OP_TYPE_GROUP && reqParams.group != Protocol::INPUT_SHAPE_GROUP) {
            ServerLog::Error("[Operator]Wrong group type of Statistic Info. Group: ", reqParams.group);
            return false;
        }

        if (!QueryStatisticTotalNum(reqParams, response.total)) {
            ServerLog::Error("[Operator]Failed to query total num of statistic info.");
            return false;
        }

        std::string sql = GenerateQueryStatisticSql(reqParams);
        ServerLog::Info("[Operator]Query Operator Statistic Info SQL. ", sql);
        sqlite3_stmt *stmt = nullptr;
        int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        if (result != SQLITE_OK) {
            ServerLog::Error("[Operator]Failed to prepare sql of StatisticInfo.", sqlite3_errmsg(db), " ", result);
            return false;
        }
        std::vector<Protocol::OperatorStatisticInfoRes> res;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int col = 0;
            Protocol::OperatorStatisticInfoRes one{};
            one.opType = sqlite3_column_string(stmt, col++);
            one.opName = sqlite3_column_string(stmt, col++);
            one.inputShape = sqlite3_column_string(stmt, col++);
            one.accCore = sqlite3_column_string(stmt, col++);
            one.totalTime = sqlite3_column_double(stmt, col++);
            one.count = sqlite3_column_int64(stmt, col++);
            one.avgTime = sqlite3_column_double(stmt, col++);
            one.maxTime = sqlite3_column_double(stmt, col++);
            one.minTime = sqlite3_column_double(stmt, col++);
            res.emplace_back(one);
        }
        response.datas = res;
        sqlite3_finalize(stmt);
        return true;
    }

    bool SummaryDataBase::QueryDetailTotalNum(Protocol::OperatorStatisticReqParams &reqParams, int64_t &total)
    {
        sqlite3_stmt *stmt = nullptr;
        std::string sql =
                " SELECT COUNT(*) as nums"
                " FROM ("
                "     SELECT * "
                "     FROM " + kernelTable +
                "     WHERE rank_id = '" + reqParams.rankId + "' AND accelerator_core <> 'HCCL'"
                "     ORDER BY duration DESC"
                "     LIMIT " + std::to_string(reqParams.topK) +
                " ) subquery";

        ServerLog::Info("[Operator]Query Detail Total Num sql: ", sql);
        int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        if (result != SQLITE_OK) {
            ServerLog::Error("[Operator]Failed to prepare sql of Detail Total Num.", sqlite3_errmsg(db), " ", result);
            return false;
        }
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            total = sqlite3_column_int64(stmt, resultStartIndex);
        }
        sqlite3_finalize(stmt);
        return true;
    }

    std::string SummaryDataBase::GenerateQueryDetailSql(Protocol::OperatorStatisticReqParams &reqParams)
    {
        std::string sql =
                " SELECT rank_id, step_id, name, op_type, accelerator_core,"
                " ROUND((start_time - ?) / (1000.0 * 1000.0), 2) as start_time, duration, wait_time, block_dim,"
                " input_shapes, input_data_types, input_formats, output_shapes, output_data_types, output_formats"
                " FROM ("
                "     SELECT * FROM " + kernelTable +
                "     WHERE rank_id = '" + reqParams.rankId + "' AND accelerator_core <> 'HCCL'"
                "     ORDER by duration DESC LIMIT " +  std::to_string(reqParams.topK) +
                " ) subquery ";
        if (!reqParams.orderBy.empty() && !reqParams.order.empty()) {
            sql += " ORDER by " + reqParams.orderBy + " " + (reqParams.order == "ascend" ? "ASC" : "DESC");
        }
        sql += " LIMIT " + std::to_string(reqParams.pageSize) +
                " OFFSET " + std::to_string((reqParams.current - 1) * reqParams.pageSize);
        return sql;
    }

    bool SummaryDataBase::QueryOperatorDetailInfo(Protocol::OperatorStatisticReqParams &reqParams,
        Protocol::OperatorDetailInfoResponse& response)
    {
        if (!QueryDetailTotalNum(reqParams, response.total)) {
            ServerLog::Error("[Operator]Failed to query total num of detail info.");
            return false;
        }
        std::string sql = GenerateQueryDetailSql(reqParams);
        ServerLog::Info("[Operator]Generate Query Detail Sql SQL.", sql);
        sqlite3_stmt *stmt = nullptr;
        int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        if (result != SQLITE_OK) {
            ServerLog::Error("[Operator]Failed to prepare sql of DetailInfo. ", sqlite3_errmsg(db), " ", result);
            return false;
        }
        uint64_t startTime = Timeline::TraceTime::Instance().GetStartTime();
        int index = bindStartIndex;
        sqlite3_bind_int64(stmt, index++, startTime);
        std::vector<Protocol::OperatorDetailInfoRes> res;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int col = 0;
            Protocol::OperatorDetailInfoRes one{};
            one.rankId = sqlite3_column_string(stmt, col++);
            one.stepId = sqlite3_column_string(stmt, col++);
            one.name = sqlite3_column_string(stmt, col++);
            one.type = sqlite3_column_string(stmt, col++);
            one.accCore = sqlite3_column_string(stmt, col++);
            one.startTime = sqlite3_column_double(stmt, col++);
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

    bool SummaryDataBase::QueryMoreInfoTotalNum(Protocol::OperatorMoreInfoReqParams &reqParams, int64_t &total)
    {
        sqlite3_stmt *stmt = nullptr;
        std::string condition = (reqParams.group == Protocol::OP_TYPE_GROUP) ?
                                " op_type = '" + reqParams.opType + "'":
                                " name = '" + reqParams.opName + "' AND input_shapes = '" + reqParams.shape + "'";
        std::string sql =
                " SELECT COUNT(*) as nums"
                " FROM ("
                "     SELECT *"
                "     FROM " + kernelTable +
                "     WHERE rank_id = '" + reqParams.rankId + "' AND" + condition +
                " ) subquery";
        ServerLog::Info("[Operator]Query More Total Num sql: ", sql);
        int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        if (result != SQLITE_OK) {
            ServerLog::Error("[Operator]Failed to prepare sql of More Total Num.", sqlite3_errmsg(db), " ", result);
            return false;
        }
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            total = sqlite3_column_int64(stmt, resultStartIndex);
        }
        sqlite3_finalize(stmt);
        return true;
    }

    std::string SummaryDataBase::GenerateQueryMoreInfoSql(Protocol::OperatorMoreInfoReqParams &reqParams)
    {
        std::string sql =
                " SELECT rank_id, step_id, name, op_type, accelerator_core,"
                " ROUND((start_time - ?) / (1000.0 * 1000.0), 2) as start_time, duration, wait_time, block_dim,"
                " input_shapes, input_data_types, input_formats, output_shapes, output_data_types, output_formats"
                " FROM ("
                "     SELECT * FROM " + kernelTable +
                "     WHERE rank_id = '" + reqParams.rankId + "'"
                "     ORDER by duration DESC"
                " ) subquery ";
        if (reqParams.group == Protocol::OP_TYPE_GROUP) {
            sql += " WHERE op_type = '" + reqParams.opType + "'";
        } else {
            sql += " WHERE name = '" + reqParams.opName + "' AND input_shapes = '" + reqParams.shape + "'";
        }
        if (!reqParams.orderBy.empty() && !reqParams.order.empty()) {
            sql += " ORDER by " + reqParams.orderBy + " " + (reqParams.order == "ascend" ? "ASC" : "DESC");
        }

        sql += " LIMIT " + std::to_string(reqParams.pageSize) +
                " OFFSET " + std::to_string((reqParams.current - 1) * reqParams.pageSize);
        return sql;
    }

    bool SummaryDataBase::QueryOperatorMoreInfo(Protocol::OperatorMoreInfoReqParams &reqParams,
        Protocol::OperatorMoreInfoResponse& response)
    {
        if (reqParams.group != Protocol::OP_TYPE_GROUP && reqParams.group != Protocol::INPUT_SHAPE_GROUP) {
            ServerLog::Error("[Operator]Wrong group type of More Info. Group: ", reqParams.group);
            return false;
        }
        if (!QueryMoreInfoTotalNum(reqParams, response.total)) {
            ServerLog::Error("[Operator]Failed to query total num of more info.");
            return false;
        }

        std::string sql = GenerateQueryMoreInfoSql(reqParams);
        ServerLog::Info("[Operator]QueryOperatorMoreInfo SQL.", sql);
        sqlite3_stmt *stmt = nullptr;
        int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        if (result != SQLITE_OK) {
            ServerLog::Error("[Operator]Failed to prepare sql of QueryOperatorMoreInfo.", sqlite3_errmsg(db));
            return false;
        }
        uint64_t startTime = Timeline::TraceTime::Instance().GetStartTime();
        int index = bindStartIndex;
        sqlite3_bind_int64(stmt, index++, startTime);

        std::vector<Protocol::OperatorDetailInfoRes> res;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int col = 0;
            Protocol::OperatorDetailInfoRes one{};
            one.rankId = sqlite3_column_string(stmt, col++);
            one.stepId = sqlite3_column_string(stmt, col++);
            one.name = sqlite3_column_string(stmt, col++);
            one.type = sqlite3_column_string(stmt, col++);
            one.accCore = sqlite3_column_string(stmt, col++);
            one.startTime = sqlite3_column_double(stmt, col++);
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

} // end of namespace Summary
} // end of namespace Module
} // end of namespace Dic