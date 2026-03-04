/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2026 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */

#include "TextTraceDatabase.h"

namespace Dic::Module::Timeline
{
    using namespace Dic::Server;
    using namespace Dic::Protocol;

    // ==================== TextTraceDatabase 方法实现 ====================
    
    void TextTraceDatabase::CreateFtraceTable()
    {
        if (!isOpen) {
            ServerLog::Error("Failed to create ftrace table. Database is not open.");
            return;
        }

        std::string sql = R"(
            CREATE TABLE IF NOT EXISTS ftrace_analysis (
                track_id INTEGER NOT NULL,
                data_type INTEGER NOT NULL,
                args TEXT,
                PRIMARY KEY (track_id, data_type)
            );
        )";

        std::unique_lock<std::recursive_mutex> lock(mutex);
        ExecSql(sql);
    }

    bool TextTraceDatabase::InsertOrUpdateFtraceStat(const std::vector<FtraceStatisticsData> &dataList)
    {
        if (!isOpen) {
            ServerLog::Error("Failed to insert/update ftrace statistics. Database is not open.");
            return false;
        }
        if (dataList.empty()) {
            return true;
        }
        std::unique_lock<std::recursive_mutex> lock(mutex);
        // 动态生成批量插入的VALUES占位符
        std::string valuePlaceholders;
        for (size_t i = 0; i < dataList.size(); ++i) {
            if (i > 0) {
                valuePlaceholders += ", ";
            }
            valuePlaceholders += "(?, ?, ?)";
        }

        std::string sql = "INSERT INTO ftrace_analysis (track_id, data_type, args) VALUES " 
            + valuePlaceholders 
            + " ON CONFLICT(track_id, data_type) DO UPDATE SET args = excluded.args";

        sqlite3_stmt *stmt = nullptr;
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            ServerLog::Error("Failed to prepare statement: ", sqlite3_errmsg(db));
            return false;
        }

        // 批量绑定参数
        int bindIndex = bindStartIndex;
        for (const auto &data : dataList) {
            sqlite3_bind_int64(stmt, bindIndex++, static_cast<int64_t>(data.trackId));
            sqlite3_bind_int64(stmt, bindIndex++, static_cast<int8_t>(data.dataType));
            sqlite3_bind_text(stmt, bindIndex++, data.GetArgs().c_str(), -1, SQLITE_TRANSIENT);
        }

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            ServerLog::Error("Failed to execute statement: ", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            return false;
        }
        sqlite3_finalize(stmt);
        return true;
    }

    FtraceStatistics TextTraceDatabase::QueryFtraceStatistics(FtraceDataType dataType,
        uint64_t offset, uint64_t limit)
    {
        FtraceStatistics result;

        if (!isOpen) {
            ServerLog::Error("Failed to query ftrace statistics. Database is not open.");
            return result;
        }

        std::unique_lock<std::recursive_mutex> lock(mutex);

        std::string sql = "SELECT track_id, data_type, args, COUNT(*) OVER () AS total_count FROM "
                          "ftrace_analysis WHERE data_type = ? LIMIT ? OFFSET ?";
        sqlite3_stmt *stmt = nullptr;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            ServerLog::Error("Failed to prepare statement: ", sqlite3_errmsg(db));
            return result;
        }
        int bindIndex = bindStartIndex;
        sqlite3_bind_int64(stmt, bindIndex++, static_cast<int64_t>(dataType));
        sqlite3_bind_int64(stmt, bindIndex++, static_cast<int64_t>(limit));
        sqlite3_bind_int64(stmt, bindIndex++, static_cast<int64_t>(offset));

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            FtraceStatisticsData data;
            data.trackId = sqlite3_column_int64(stmt, 0);
            data.dataType = static_cast<FtraceDataType>(sqlite3_column_int(stmt, 1));
            
            std::string argsStr = sqlite3_column_string(stmt, 2);
            if (!argsStr.empty()) {
                data.SetArgs(argsStr);
            }
            result.totalCount = sqlite3_column_int64(stmt, 3);
            
            result.data.push_back(data);
        }

        sqlite3_finalize(stmt);
        return result;
    }
}