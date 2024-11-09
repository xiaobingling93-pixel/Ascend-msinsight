/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "EnumHcclTransportTypeTable.h"
namespace Dic::Module::Timeline {
void EnumHcclTransportTypeTable::SetId(EnumHcclTransportTypePO &enumHcclTransportTypePO,
    const std::unique_ptr<SqliteResultSet> &resultSet)
{
    enumHcclTransportTypePO.id = resultSet->GetUint64(EnumHcclTransportTypeClumn::ID);
}

void EnumHcclTransportTypeTable::SetName(EnumHcclTransportTypePO &enumHcclTransportTypePO,
    const std::unique_ptr<SqliteResultSet> &resultSet)
{
    enumHcclTransportTypePO.name = resultSet->GetString(EnumHcclTransportTypeClumn::NAME);
}

std::unordered_map<uint64_t, std::string> EnumHcclTransportTypeTable::QueryStrMap(const std::vector<uint64_t> &ids,
    const std::string &fileId)
{
    std::unordered_map<uint64_t, std::string> res;
    std::vector<EnumHcclTransportTypePO> enumHcclTransportTypePO;
    Select(EnumHcclTransportTypeClumn::ID, EnumHcclTransportTypeClumn::NAME)
        .In(EnumHcclTransportTypeClumn::ID, ids)
        .ExcuteQuery(fileId, enumHcclTransportTypePO);
    for (const auto &item : enumHcclTransportTypePO) {
        res[item.id] = item.name;
    }
    return res;
}
}
