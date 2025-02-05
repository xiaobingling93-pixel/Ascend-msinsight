/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef PROFILER_SERVER_NPUINFOTABLE_H
#define PROFILER_SERVER_NPUINFOTABLE_H
#include "string"
#include "Table.h"
#include "TextTableColum.h"

namespace Dic::Module::Timeline {
struct NpuInfoPo {
    std::string name;
    uint64_t id = 0;
};

class NpuInfoTable : public Table<NpuInfoPo> {
public:
    NpuInfoTable() = default;
    ~NpuInfoTable() override = default;
private:
    const std::unordered_map<std::string_view, assign> &GetAssignMap() override
    {
        static std::unordered_map<std::string_view, assign> assignMap = {
            { NpuInfoColumn::ID, SetId },
            { NpuInfoColumn::NAME, SetName },
        };
        return assignMap;
    }

    const std::string &GetTableName() override
    {
        static std::string tableName = "NPU_INFO";
        return tableName;
    }
    static void SetId(NpuInfoPo &npuInfoPo, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void SetName(NpuInfoPo &npuInfoPo, const std::unique_ptr<SqliteResultSet> &resultSet);
};
}
#endif // PROFILER_SERVER_NPUINFOTABLE_H
