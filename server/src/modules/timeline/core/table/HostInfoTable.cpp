/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "HostInfoTable.h"
namespace Dic::Module::Timeline {
std::string HostInfoTable::GetHost(const std::string &fileId)
{
    std::string host;
    auto database = DataBaseManager::Instance().GetTraceDatabaseByRankId(fileId);
    if (database == nullptr) {
        ClearThreadLocal();
        return host;
    }
    host = database->QueryHostInfo();
    return host;
}

void HostInfoTable::SetHostUid(HostInfoPO &hostInfoPO, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    hostInfoPO.hostUid = resultSet->GetString(HostInfoColumn::HOST_UID);
}

void HostInfoTable::SetHostName(HostInfoPO &hostInfoPO, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    hostInfoPO.hostName = resultSet->GetString(HostInfoColumn::HOST_NAME);
}
}
