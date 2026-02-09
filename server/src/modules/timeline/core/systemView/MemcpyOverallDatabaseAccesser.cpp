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

#include "ServerLog.h"
#include "MemcpyOverallDatabaseAccesser.h"

namespace Dic::Module::Timeline {
DataType MemcpyOverallDatabaseAccesser::GetDatabaseType() const
{
    if (fileId_.empty()) {
        // 如果fileId为空，返回默认类型
        return DataType::TEXT;
    }
    // 或者直接调用DataBaseManager的静态方法
    return DataBaseManager::Instance().GetDataType(fileId_);
}

bool MemcpyOverallDatabaseAccesser::GetMemcpyRecords(const uint64_t startTime, const uint64_t endTime,
    std::vector<MemcpyRecord>& records) const
{
    if (!database_ || fileId_.empty()) {
        return false;
    }

    DataType dataType = GetDatabaseType();

    if (dataType == DataType::TEXT) {
        return GetMemcpyRecordsFromText(startTime, endTime, records);
    } else if (dataType == DataType::DB) {
        return GetMemcpyRecordsFromDb(startTime, endTime, records);
    }
    return false;
}

bool MemcpyOverallDatabaseAccesser::GetMemcpyRecordsFromText(uint64_t startTime, uint64_t endTime,
    std::vector<MemcpyRecord>& records) const
{
    if (!database_) {
        return false;
    }

    try {
        // 构造SQL查询语句，使用参数化查询防止SQL注入
        std::string sql = "SELECT tid, args, start_ns, end_ns FROM slice WHERE name = 'MEMCPY_ASYNC' "
                          "AND start_ns >= ? AND end_ns <= ?";

        auto stmt = database_->CreatPreparedStatement(sql);
        if (stmt == nullptr) {
            // 记录错误日志
            return false;
        }

        // 绑定参数
        stmt->BindParams(startTime, endTime);

        auto resultSet = stmt->ExecuteQuery();
        if (resultSet == nullptr) {
            // 记录错误日志
            return false;
        }

        while (resultSet->Next()) {
            MemcpyRecord record;

            // 获取tid
            record.threadId = resultSet->GetInt32("tid");

            // 获取args并解析operation
            std::string argsStr = resultSet->GetString("args");
            const auto parsed = ParseOperationAndSizeFromJson(argsStr);
            record.memcpyType = parsed.first;
            record.size = parsed.second;

            // 获取时间戳
            record.startTime = resultSet->GetUint64("start_ns");
            record.endTime = resultSet->GetUint64("end_ns");
            record.duration = static_cast<double>(record.endTime - record.startTime);

            records.push_back(record);
        }

        return true;
    } catch (...) {
        return false;
    }
}

bool MemcpyOverallDatabaseAccesser::GetMemcpyRecordsFromDb(uint64_t startTime, uint64_t endTime,
    std::vector<MemcpyRecord>& records) const
{
    if (!database_) {
        return false;
    }

    try {
        // 构造SQL查询语句，关联task表和memcpy_info表，使用参数化查询
        std::string sql = "SELECT t.globalPid, emo.name AS memcpyOperation, mi.size, t.startNs, t.endNs "
                          "FROM task t JOIN memcpy_info mi ON t.globalTaskId = mi.globalTaskId "
                          "LEFT JOIN ENUM_MEMCPY_OPERATION emo ON mi.memcpyOperation = emo.id "
                          "WHERE t.startNs >= ? AND t.endNs <= ?";

        auto stmt = database_->CreatPreparedStatement(sql);
        if (stmt == nullptr) {
            // 记录错误日志
            Server::ServerLog::Error("Querying memcpy records from db has error, Fail to get stmt.");
            return false;
        }

        // 绑定参数
        stmt->BindParams(startTime, endTime);

        auto resultSet = stmt->ExecuteQuery();
        if (resultSet == nullptr) {
            // 记录错误日志
            return false;
        }

        while (resultSet->Next()) {
            MemcpyRecord record;

            // 从globalPid中提取threadId
            uint64_t globalPid = resultSet->GetUint64("globalPid");
            record.threadId = ExtractThreadIdFromGlobalPid(globalPid);

            // 获取memcpyOperation
            record.memcpyType = resultSet->GetString("memcpyOperation");

            // 获取size
            record.size = resultSet->GetUint64("size");

            // 获取时间戳
            record.startTime = resultSet->GetUint64("startNs");
            record.endTime = resultSet->GetUint64("endNs");
            record.duration = static_cast<double>(record.endTime - record.startTime);

            records.push_back(record);
        }

        return true;
    } catch (...) {
        return false;
    }
}

std::pair<std::string, uint64_t>
MemcpyOverallDatabaseAccesser::ParseOperationAndSizeFromJson(const std::string& jsonStr)
{
    std::string error;
    const auto json = JsonUtil::TryParse(jsonStr, error);

    if (!json.has_value() || !error.empty()) {
        return { "", 0 }; // 解析失败，返回空字符串
    }

    std::string operation;
    // 检查并获取operation字段（支持多种可能的键名）
    if (JsonUtil::IsJsonKeyValid(json.value(), "operation")) {
        operation = JsonUtil::GetString(json.value(), "operation");
    } else if (JsonUtil::IsJsonKeyValid(json.value(), "Operation")) {
        operation = JsonUtil::GetString(json.value(), "Operation");
    } else if (JsonUtil::IsJsonKeyValid(json.value(), "OPERATION")) {
        operation = JsonUtil::GetString(json.value(), "OPERATION");
    }

    uint64_t size = 0;
    // 检查并获取 size(B) 字段
    if (JsonUtil::IsJsonKeyValid(json.value(), "size(B)")) {
        size = JsonUtil::GetInteger(json.value(), "size(B)");
    }

    return { operation, size };
}

uint32_t MemcpyOverallDatabaseAccesser::ExtractThreadIdFromGlobalPid(const uint64_t globalPid)
{
    // 前32位是pid，后32位是tid
    return static_cast<uint32_t>(globalPid & 0xFFFFFFFF);
}
}