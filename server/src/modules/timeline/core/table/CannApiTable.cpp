/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
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
