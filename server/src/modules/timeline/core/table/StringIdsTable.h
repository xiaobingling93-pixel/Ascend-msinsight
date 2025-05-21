/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_STRINGIDSTABLE_H
#define PROFILER_SERVER_STRINGIDSTABLE_H
#include "TextTableColum.h"
#include "Table.h"
namespace Dic::Module::Timeline {
struct StringIdsPO {
    uint64_t id = 0;
    std::string value;
};
class StringIdsTable : public Table<StringIdsPO> {
public:
    StringIdsTable() = default;
    ~StringIdsTable() override = default;
    std::unordered_map<uint64_t, std::string> QueryStrMap(const std::vector<uint64_t> &ids, const std::string &fileId);
    std::unordered_map<uint64_t, std::string> QueryStrMapByValues(const std::vector<std::string> &values,
                                                                  const std::string &fileId);
protected:
    const std::unordered_map<std::string_view, assign> &GetAssignMap() override
    {
        static std::unordered_map<std::string_view, assign> assignMap = {
            { StringIdsColumn::ID, SetId },
            { StringIdsColumn::VALUE, SetValue },
        };
        return assignMap;
    }

    const std::string &GetTableName() override
    {
        static std::string tableName = "STRING_IDS";
        return tableName;
    }
    static void SetId(StringIdsPO &stringIdsPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void SetValue(StringIdsPO &stringIdsPO, const std::unique_ptr<SqliteResultSet> &resultSet);
};
}
#endif // PROFILER_SERVER_STRINGIDSTABLE_H
