/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_ENUMHCCLLINKTYPETABLE_H
#define PROFILER_SERVER_ENUMHCCLLINKTYPETABLE_H
#include "TextTableColum.h"
#include "Table.h"
namespace Dic::Module::Timeline {
struct EnumHcclLinkTypePO {
    uint64_t id = 0;
    std::string name;
};
class EnumHcclLinkTypeTable : public Table<EnumHcclLinkTypePO> {
public:
    EnumHcclLinkTypeTable() = default;
    ~EnumHcclLinkTypeTable() override = default;
    std::unordered_map<uint64_t, std::string> QueryStrMap(const std::vector<uint64_t> &ids, const std::string &fileId);

protected:
    const std::unordered_map<std::string_view, assign> &GetAssignMap() override
    {
        static std::unordered_map<std::string_view, assign> assignMap = {
            { EnumHcclLinkTypeClumn::ID, SetId },
            { EnumHcclLinkTypeClumn::NAME, SetName },
        };
        return assignMap;
    }

    const std::string &GetTableName() override
    {
        static std::string tableName = "ENUM_HCCL_LINK_TYPE";
        return tableName;
    }
    static void SetId(EnumHcclLinkTypePO &enumHcclLinkTypePO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void SetName(EnumHcclLinkTypePO &enumHcclLinkTypePO, const std::unique_ptr<SqliteResultSet> &resultSet);
};
}
#endif // PROFILER_SERVER_ENUMHCCLLINKTYPETABLE_H
