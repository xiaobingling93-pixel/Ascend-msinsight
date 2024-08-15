/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_CANNAPITABLE_H
#define PROFILER_SERVER_CANNAPITABLE_H
#include "TextTableColum.h"
#include "Table.h"
namespace Dic::Module::Timeline {
struct CannApiPO {
    uint64_t id = 0;
    uint64_t timestamp = 0;
    uint64_t endTime = 0;
    uint64_t type = 0;
    uint64_t globalTid = 0;
    uint64_t name = 0;
    uint64_t depth = 0;
};
class CannApiTable : public Table<CannApiPO> {
public:
    CannApiTable() = default;
    virtual ~CannApiTable() = default;

protected:
    const std::unordered_map<std::string_view, assign> &GetAssignMap() override
    {
        static std::unordered_map<std::string_view, assign> assignMap = {
            { CannApiColumn::ID, SetId },
            { CannApiColumn::TIMESTAMP, SetTimestamp },
            { CannApiColumn::ENDTIME, SetEndTime },
            { CannApiColumn::TYPE, SetType },
            { CannApiColumn::GLOBAL_TID, SetGlobalTid },
            { CannApiColumn::NAME, SetName },
            { CannApiColumn::DEPTH, SetDepth } };

        return assignMap;
    }
    const std::string &GetTableName() override
    {
        static std::string tableName = "CANN_API";
        return tableName;
    }
    static void SetId(CannApiPO &cannApiPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void SetTimestamp(CannApiPO &cannApiPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void SetEndTime(CannApiPO &cannApiPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void SetType(CannApiPO &cannApiPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void SetGlobalTid(CannApiPO &cannApiPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void SetName(CannApiPO &cannApiPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void SetDepth(CannApiPO &cannApiPO, const std::unique_ptr<SqliteResultSet> &resultSet);
};
}
#endif // PROFILER_SERVER_CANNAPITABLE_H