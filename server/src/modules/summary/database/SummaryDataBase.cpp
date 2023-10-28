/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "SummaryDataBase.h"
#include "ServerLog.h"
#include "SummaryProtocolRequest.h"
#include "SummaryProtocolResponse.h"
#include "TraceTime.h"

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
            "CREATE TABLE " + kernelTable + " (id INTEGER PRIMARY KEY AUTOINCREMENT, step_id TEXT, name TEXT, " +
            "type TEXT, accelerator_core TEXT, start_time INTEGER, duration INTEGER, wait_time INTEGER, " +
            "block_dim INTEGER, input_shapes TEXT, input_data_types TEXT, input_formats TEXT, output_shapes TEXT, " +
            "output_data_types TEXT, output_formats TEXT);";
    return ExecSql(sql);
}

bool SummaryDataBase::InitStmt()
{
    if (hasInitStmt) {
        return true;
    }
    std::string sql = "INSERT INTO " + kernelTable + " (step_id, name, type, accelerator_core, start_time, duration, " +
            "wait_time, block_dim, input_shapes, input_data_types, input_formats, output_shapes, " +
            "output_data_types, output_formats)" + " VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?)";
    for (int i = 0; i < cacheSize - 1; ++i) {
        sql.append(",(?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
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
        sqlite3_finalize(insertKernelStmt);
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
        sqlite3_bind_text(stmt, idx++, event.stepId.c_str(), event.stepId.length(), SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, idx++, event.name.c_str(), event.name.length(), SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, idx++, event.type.c_str(), event.type.length(), SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, idx++, event.acceleratorCore.c_str(), event.acceleratorCore.length(), SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, idx++, event.startTime);
        sqlite3_bind_double(stmt, idx++, event.duration);
        sqlite3_bind_double(stmt, idx++, event.waitTime);
        sqlite3_bind_double(stmt, idx++, event.blockDim);
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

sqlite3_stmt *SummaryDataBase::GetKernelStmt(uint64_t paramLen)
{
    sqlite3_stmt *stmt = nullptr;
    if (paramLen == cacheSize) {
        stmt = insertKernelStmt;
        sqlite3_reset(stmt);
    } else {
        std::string sql = "INSERT INTO " + kernelTable + " (step_id, name, type, accelerator_core, start_time, " +
                "duration, wait_time, block_dim, input_shapes, input_data_types, input_formats, output_shapes, " +
                "output_data_types, output_formats)" + " VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?)";
        for (int i = 0; i < paramLen - 1; ++i) {
            sql.append(",(?,?,?,?,?,?,?,?,?,?,?,?,?,?)");
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
    double startTime = 0;
    double offset = (params.currentPage - 1) * params.pageSize;
    sqlite3_stmt *stmt = nullptr;
    int index = bindStartIndex;

    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        ServerLog::Error("QueryOperatorDetail failed! Failed to prepare sql.", sqlite3_errmsg(db));
        return false;
    }
    sqlite3_bind_text(stmt, index++, params.timeFlag.c_str(), params.timeFlag.length(), nullptr);
    sqlite3_bind_double(stmt, index++, params.pageSize);
    sqlite3_bind_double(stmt, index++, offset);
    std::vector<Protocol::ComputeDetail> computeVec;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        Protocol::ComputeDetail computeDetail{};
        computeDetail.name = sqlite3_column_string(stmt, col++);
        computeDetail.type = sqlite3_column_string(stmt, col++);
        computeDetail.startTime = (static_cast<double>(sqlite3_column_double(stmt, col++))
                                  - Timeline::TraceTime::Instance().GetStartTime() / 1000) / 1000;
        computeDetail.duration = static_cast<double>(sqlite3_column_double(stmt, col++));
        computeDetail.waitTime = static_cast<double>(sqlite3_column_double(stmt, col++));
        computeDetail.blockDim = static_cast<double>(sqlite3_column_double(stmt, col++));
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
        sql = "SELECT name, type, start_time as startTime, duration, wait_time as waitTime, block_dim as blockDim, "
              "input_shapes as inputShapes, input_data_types as inputDataTypes, input_formats as inputFormats, "
              "output_shapes as outputShapes, output_data_types as outputDataTypes, output_formats as outputFormats "
              "FROM " + kernelTable +
              " WHERE accelerator_core = ?  LIMIT ? offset ?";
    } else {
        sql = "SELECT name, type, start_time as startTime, duration, wait_time as waitTime, block_dim as blockDim, "
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
        sql = "SELECT name, type, ROUND((start_time - (? / 1000.0)) / 1000, 4) as startTime, "
              "ROUND(duration, 4) as duration, ROUND(wait_time, 4) as waitTime FROM " + kernelTable +
              " WHERE accelerator_core = ?  LIMIT ? offset ?";
    } else {
        sql = "SELECT name, type, ROUND((start_time - (? / 1000.0)) / 1000, 4) as startTime, "
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

} // end of namespace Summary
} // end of namespace Module
} // end of namespace Dic