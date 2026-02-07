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

#ifndef PROFILER_SERVER_MEMCPYOVERALLDATABASEACCESSER_H
#define PROFILER_SERVER_MEMCPYOVERALLDATABASEACCESSER_H
#include <memory>

#include "DataBaseManager.h"
#include "VirtualTraceDatabase.h"

namespace Dic::Module::Timeline {
    struct MemcpyRecord {
        uint32_t threadId;
        std::string memcpyType;
        uint64_t size;
        double duration;
        uint64_t startTime;
        uint64_t endTime;
    };

    class MemcpyOverallDatabaseAccesser {
    public:
        explicit MemcpyOverallDatabaseAccesser(
            const std::shared_ptr<VirtualTraceDatabase>& database,
            const std::string& fileId)
            : database_(database), fileId_(fileId) {}

        /**
         * @brief 获取指定时间范围内的Memcpy记录
         * @param startTime 起始时间戳
         * @param endTime 结束时间戳
         * @param records 输出的Memcpy记录列表
         * @return 是否成功获取数据
         */
        bool GetMemcpyRecords(uint64_t startTime, uint64_t endTime, std::vector<MemcpyRecord>& records) const;

    private:
        /**
         * @brief 通过DataBaseManager获取当前数据库的数据类型
         * @return 数据库类型
         */
        DataType GetDatabaseType() const;

        /**
         * @brief 从Text数据库获取Memcpy记录
         * @param startTime 起始时间戳
         * @param endTime 结束时间戳
         * @param records 输出的Memcpy记录列表
         * @return 是否成功获取数据
         */
        bool GetMemcpyRecordsFromText(uint64_t startTime, uint64_t endTime, std::vector<MemcpyRecord>& records) const;

        /**
         * @brief 从Db数据库获取Memcpy记录
         * @param startTime 起始时间戳
         * @param endTime 结束时间戳
         * @param records 输出的Memcpy记录列表
         * @return 是否成功获取数据
         */
        bool GetMemcpyRecordsFromDb(uint64_t startTime, uint64_t endTime, std::vector<MemcpyRecord>& records) const;

        /**
         * @brief 解析JSON字符串获取operation字段
         * @param jsonStr JSON字符串
         * @return operation字段值, size:单位B
         */
        static std::pair<std::string, uint64_t> ParseOperationAndSizeFromJson(const std::string& jsonStr);

        /**
         * @brief 从globalPid中提取threadId
         * @param globalPid 包含pid和tid的全局PID
         * @return 提取的threadId
         */
        static uint32_t ExtractThreadIdFromGlobalPid(uint64_t globalPid);

        std::shared_ptr<VirtualTraceDatabase> database_;
        std::string fileId_;
    };
}

#endif //PROFILER_SERVER_MEMCPYOVERALLDATABASEACCESSER_H