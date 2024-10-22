/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_SESSIONTIMEINFOTABLE_H
#define PROFILER_SERVER_SESSIONTIMEINFOTABLE_H
#include "TextTableColum.h"
#include "Table.h"
namespace Dic::Module::Timeline {
struct HostInfoPO {
    std::string hostUid;
    std::string hostName;
};
class HostInfoTable : public Table<HostInfoPO> {
public:
    HostInfoTable() = default;
    ~HostInfoTable() override = default;
    std::string GetHost(const std::string &fileId);

protected:
    const std::unordered_map<std::string_view, assign> &GetAssignMap() override
    {
        static std::unordered_map<std::string_view, assign> assignMap = {
            { HostInfoColumn::HOST_UID, SetHostUid },
            { HostInfoColumn::HOST_NAME, SetHostName },
        };
        return assignMap;
    }

    const std::string &GetTableName() override
    {
        static std::string tableName = "HOST_INFO";
        return tableName;
    }
    static void SetHostUid(HostInfoPO &hostInfoPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void SetHostName(HostInfoPO &hostInfoPO, const std::unique_ptr<SqliteResultSet> &resultSet);
};
}
#endif // PROFILER_SERVER_SESSIONTIMEINFOTABLE_H
