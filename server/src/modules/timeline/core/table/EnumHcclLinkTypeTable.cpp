/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "EnumHcclLinkTypeTable.h"
namespace Dic::Module::Timeline {
void EnumHcclLinkTypeTable::SetId(EnumHcclLinkTypePO &enumHcclLinkTypePO,
    const std::unique_ptr<SqliteResultSet> &resultSet)
{
    enumHcclLinkTypePO.id = resultSet->GetUint64(EnumHcclLinkTypeClumn::ID);
}

void EnumHcclLinkTypeTable::SetName(EnumHcclLinkTypePO &enumHcclLinkTypePO,
    const std::unique_ptr<SqliteResultSet> &resultSet)
{
    enumHcclLinkTypePO.name = resultSet->GetString(EnumHcclLinkTypeClumn::NAME);
}

std::unordered_map<uint64_t, std::string> EnumHcclLinkTypeTable::QueryStrMap(const std::vector<uint64_t> &ids,
    const std::string &fileId)
{
    std::unordered_map<uint64_t, std::string> res;
    std::vector<EnumHcclLinkTypePO> enumHcclLinkTypePO;
    Select(EnumHcclLinkTypeClumn::ID, EnumHcclLinkTypeClumn::NAME)
        .In(EnumHcclLinkTypeClumn::ID, ids)
        .ExcuteQuery(fileId, enumHcclLinkTypePO);
    for (const auto &item : enumHcclLinkTypePO) {
        res[item.id] = item.name;
    }
    return res;
}
}