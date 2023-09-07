/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "SummaryDataBase.h"
#include "ServerLog.h"

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

bool SummaryDataBase::InsertKernelDetailList(std::vector<Kernel> kernelVec)
{
    sqlite3_stmt *stmt = GetKernelStmt(kernelVec.size());
    if (stmt == nullptr) {
        ServerLog::Error("Failed to get kernel stmt.");
        return false;
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
        return false;
    }
    return true;
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

} // end of namespace Summary
} // end of namespace Module
} // end of namespace Dic