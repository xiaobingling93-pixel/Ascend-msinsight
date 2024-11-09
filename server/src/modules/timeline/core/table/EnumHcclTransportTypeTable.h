/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_ENUMHCCLTRANSPORTTYPETABLE_H
#define PROFILER_SERVER_ENUMHCCLTRANSPORTTYPETABLE_H
#include "TextTableColum.h"
#include "Table.h"
namespace Dic::Module::Timeline {
struct EnumHcclTransportTypePO {
    uint64_t id = 0;
    std::string name;
};
class EnumHcclTransportTypeTable : public Table<EnumHcclTransportTypePO> {
public:
    EnumHcclTransportTypeTable() = default;
    ~EnumHcclTransportTypeTable() override = default;
    std::unordered_map<uint64_t, std::string> QueryStrMap(const std::vector<uint64_t> &ids, const std::string &fileId);

protected:
    const std::unordered_map<std::string_view, assign> &GetAssignMap() override
    {
        static std::unordered_map<std::string_view, assign> assignMap = {
            { EnumHcclTransportTypeClumn::ID, SetId },
            { EnumHcclTransportTypeClumn::NAME, SetName },
        };
        return assignMap;
    }

    const std::string &GetTableName() override
    {
        static std::string tableName = "ENUM_HCCL_TRANSPORT_TYPE";
        return tableName;
    }
    static void SetId(EnumHcclTransportTypePO &enumHcclTransportTypePO,
        const std::unique_ptr<SqliteResultSet> &resultSet);
    static void SetName(EnumHcclTransportTypePO &enumHcclTransportTypePO,
        const std::unique_ptr<SqliteResultSet> &resultSet);
};
}
#endif // PROFILER_SERVER_ENUMHCCLTRANSPORTTYPETABLE_H
