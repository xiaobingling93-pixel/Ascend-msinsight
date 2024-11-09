/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "EnumHcclDataTypeTable.h"
namespace Dic::Module::Timeline {
void EnumHcclDataTypeTable::SetId(EnumHcclDataTypePO &enumHcclDataTypePO,
    const std::unique_ptr<SqliteResultSet> &resultSet)
{
    enumHcclDataTypePO.id = resultSet->GetUint64(EnumHcclDataTypeClumn::ID);
}

void EnumHcclDataTypeTable::SetName(EnumHcclDataTypePO &enumHcclDataTypePO,
    const std::unique_ptr<SqliteResultSet> &resultSet)
{
    enumHcclDataTypePO.name = resultSet->GetString(EnumHcclDataTypeClumn::NAME);
}

std::unordered_map<uint64_t, std::string> EnumHcclDataTypeTable::QueryStrMap(const std::vector<uint64_t> &ids,
    const std::string &fileId)
{
    std::unordered_map<uint64_t, std::string> res;
    std::vector<EnumHcclDataTypePO> enumHcclDataTypePO;
    Select(EnumHcclDataTypeClumn::ID, EnumHcclDataTypeClumn::NAME)
        .In(EnumHcclDataTypeClumn::ID, ids)
        .ExcuteQuery(fileId, enumHcclDataTypePO);
    for (const auto &item : enumHcclDataTypePO) {
        res[item.id] = item.name;
    }
    return res;
}
}