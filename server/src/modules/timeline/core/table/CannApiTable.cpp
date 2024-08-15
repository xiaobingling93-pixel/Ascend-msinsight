/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "CannApiTable.h"
namespace Dic::Module::Timeline {
void CannApiTable::SetId(CannApiPO &cannApiPO, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    cannApiPO.id = resultSet->GetUint64(CannApiColumn::ID);
}

void CannApiTable::SetTimestamp(CannApiPO &cannApiPO, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    cannApiPO.timestamp = resultSet->GetUint64(CannApiColumn::TIMESTAMP);
}

void CannApiTable::SetEndTime(CannApiPO &cannApiPO, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    cannApiPO.endTime = resultSet->GetUint64(CannApiColumn::ENDTIME);
}

void CannApiTable::SetType(CannApiPO &cannApiPO, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    cannApiPO.type = resultSet->GetUint64(CannApiColumn::TYPE);
}

void CannApiTable::SetGlobalTid(CannApiPO &cannApiPO, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    cannApiPO.globalTid = resultSet->GetUint64(CannApiColumn::GLOBAL_TID);
}

void CannApiTable::SetName(CannApiPO &cannApiPO, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    cannApiPO.name = resultSet->GetUint64(CannApiColumn::NAME);
}

void CannApiTable::SetDepth(CannApiPO &cannApiPO, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    cannApiPO.depth = resultSet->GetUint64(CannApiColumn::DEPTH);
}
}
