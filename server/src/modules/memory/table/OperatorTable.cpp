/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "OperatorTable.h"
namespace Dic::Module::Memory {
void OperatorTable::SetId(OperatorPO &operatorPO, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    operatorPO.id = resultSet->GetUint64(OperatorColumn::ID);
}

void OperatorTable::SetName(OperatorPO &operatorPO, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    operatorPO.name = resultSet->GetString(OperatorColumn::NAME);
}

void OperatorTable::SetSize(OperatorPO &operatorPO, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    operatorPO.size = resultSet->GetDouble(OperatorColumn::SIZE);
}

void OperatorTable::SetAllocationTime(OperatorPO &operatorPO, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    operatorPO.allocationTime = resultSet->GetUint64(OperatorColumn::ALLOCATION_TIME);
}

void OperatorTable::SetReleaseTime(OperatorPO &operatorPO, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    operatorPO.releaseTime = resultSet->GetUint64(OperatorColumn::RELEASE_TIME);
}

void OperatorTable::SetDuration(OperatorPO &operatorPO, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    operatorPO.duration = resultSet->GetDouble(OperatorColumn::DURATION);
}

void OperatorTable::SetActiveReleaseTime(OperatorPO &operatorPO, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    operatorPO.activeReleaseTime = resultSet->GetUint64(OperatorColumn::ACTIVE_RELEASE_TIME);
}

void OperatorTable::SetActiveDuration(OperatorPO &operatorPO, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    operatorPO.activeDuration = resultSet->GetDouble(OperatorColumn::ACTIVE_DURATION);
}

void OperatorTable::SetAllocationAllocated(OperatorPO &operatorPO, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    operatorPO.allocationAllocated = resultSet->GetDouble(OperatorColumn::ALLOCATION_ALLOCATED);
}

void OperatorTable::SetAllocationReserve(OperatorPO &operatorPO, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    operatorPO.allocationReserve = resultSet->GetDouble(OperatorColumn::ALLOCATION_RESERVE);
}

void OperatorTable::SetAllocationActive(OperatorPO &operatorPO, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    operatorPO.allocationActive = resultSet->GetDouble(OperatorColumn::ALLOCATION_ACTIVE);
}

void OperatorTable::SetReleaseAllocated(OperatorPO &operatorPO, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    operatorPO.releaseAllocated = resultSet->GetDouble(OperatorColumn::RELEASE_ALLOCATED);
}

void OperatorTable::SetReleaseReserve(OperatorPO &operatorPO, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    operatorPO.releaseReserve = resultSet->GetDouble(OperatorColumn::RELEASE_RESERVE);
}

void OperatorTable::SetReleaseActive(OperatorPO &operatorPO, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    operatorPO.releaseActive = resultSet->GetDouble(OperatorColumn::RELEASE_ACTIVE);
}

void OperatorTable::SetStream(OperatorPO &operatorPO, const std::unique_ptr<SqliteResultSet> &resultSet)
{
    operatorPO.stream = resultSet->GetString(OperatorColumn::STREAM);
}
}
