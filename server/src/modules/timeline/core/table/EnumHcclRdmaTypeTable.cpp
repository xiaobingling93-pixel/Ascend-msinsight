/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "EnumHcclRdmaTypeTable.h"
namespace Dic::Module::Timeline {
void EnumHcclRdmaTypeTable::SetId(EnumHcclRdmaTypePO &enumHcclRdmaTypePO,
    const std::unique_ptr<SqliteResultSet> &resultSet)
{
    enumHcclRdmaTypePO.id = resultSet->GetUint64(EnumHcclRdmaTypeClumn::ID);
}

void EnumHcclRdmaTypeTable::SetName(EnumHcclRdmaTypePO &enumHcclRdmaTypePO,
    const std::unique_ptr<SqliteResultSet> &resultSet)
{
    enumHcclRdmaTypePO.name = resultSet->GetString(EnumHcclRdmaTypeClumn::NAME);
}

std::unordered_map<uint64_t, std::string> EnumHcclRdmaTypeTable::QueryStrMap(const std::vector<uint64_t> &ids,
    const std::string &fileId)
{
    std::unordered_map<uint64_t, std::string> res;
    std::vector<EnumHcclRdmaTypePO> enumHcclRdmaTypePO;
    Select(EnumHcclRdmaTypeClumn::ID, EnumHcclRdmaTypeClumn::NAME)
        .In(EnumHcclRdmaTypeClumn::ID, ids)
        .ExcuteQuery(fileId, enumHcclRdmaTypePO);
    for (const auto &item : enumHcclRdmaTypePO) {
        res[item.id] = item.name;
    }
    return res;
}
}