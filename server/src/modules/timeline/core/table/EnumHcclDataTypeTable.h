/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_ENUMHCCLDATATYPE_H
#define PROFILER_SERVER_ENUMHCCLDATATYPE_H
#include "TextTableColum.h"
#include "Table.h"
namespace Dic::Module::Timeline {
struct EnumHcclDataTypePO {
    uint64_t id = 0;
    std::string name;
};
class EnumHcclDataTypeTable : public Table<EnumHcclDataTypePO> {
public:
    EnumHcclDataTypeTable() = default;
    ~EnumHcclDataTypeTable() override = default;
    std::unordered_map<uint64_t, std::string> QueryStrMap(const std::vector<uint64_t> &ids, const std::string &fileId);

protected:
    const std::unordered_map<std::string_view, assign> &GetAssignMap() override
    {
        static std::unordered_map<std::string_view, assign> assignMap = {
            { EnumHcclDataTypeClumn::ID, SetId },
            { EnumHcclDataTypeClumn::NAME, SetName },
        };
        return assignMap;
    }

    const std::string &GetTableName() override
    {
        static std::string tableName = "ENUM_HCCL_DATA_TYPE";
        return tableName;
    }
    static void SetId(EnumHcclDataTypePO &enumHcclDataTypePO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void SetName(EnumHcclDataTypePO &enumHcclDataTypePO, const std::unique_ptr<SqliteResultSet> &resultSet);
};
}
#endif // PROFILER_SERVER_ENUMHCCLDATATYPE_H
