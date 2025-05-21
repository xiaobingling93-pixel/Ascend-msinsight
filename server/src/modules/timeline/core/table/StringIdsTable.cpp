/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "StringIdsTable.h"
namespace Dic::Module::Timeline {
void StringIdsTable::SetId(StringIdsPO &stringIdsPO, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    stringIdsPO.id = resultSet->GetUint64(StringIdsColumn::ID);
}

void StringIdsTable::SetValue(StringIdsPO &stringIdsPO, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    stringIdsPO.value = resultSet->GetString(StringIdsColumn::VALUE);
}

std::unordered_map<uint64_t, std::string> StringIdsTable::QueryStrMap(const std::vector<uint64_t> &ids,
    const std::string &fileId)
{
    std::unordered_map<uint64_t, std::string> res;
    std::vector<StringIdsPO> stringIdsPOs;
    Select(StringIdsColumn::ID, StringIdsColumn::VALUE).In(StringIdsColumn::ID, ids).ExcuteQuery(fileId, stringIdsPOs);
    for (const auto &item : stringIdsPOs) {
        res[item.id] = item.value;
    }
    return res;
}

std::unordered_map<uint64_t, std::string> StringIdsTable::QueryStrMapByValues(const std::vector<std::string> &values,
                                                                              const std::string &fileId)
{
    std::unordered_map<uint64_t, std::string> res;
    std::vector<StringIdsPO> stringIdsPOs;
    Select(StringIdsColumn::ID, StringIdsColumn::VALUE)
        .In(StringIdsColumn::VALUE, values)
        .ExcuteQuery(fileId, stringIdsPOs);
    for (const auto &item : stringIdsPOs) {
        res[item.id] = item.value;
    }
    return res;
}
}
