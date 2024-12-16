/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#ifndef PROFILER_SERVER_OPERATORTABLE_H
#define PROFILER_SERVER_OPERATORTABLE_H
#include "MemoryTableColum.h"
#include "MemoryTable.h"
namespace Dic::Module::Memory {
struct OperatorPO {
    uint64_t id = 0;
    std::string name;
    double size = 0;
    uint64_t allocationTime = 0;
    uint64_t releaseTime = 0;
    double duration = 0;
    uint64_t activeReleaseTime = 0;
    double activeDuration = 0;
    double allocationAllocated = 0;
    double allocationReserve = 0;
    double allocationActive = 0;
    double releaseAllocated = 0;
    double releaseReserve = 0;
    double releaseActive = 0;
    std::string stream;
};
class OperatorTable : public MemoryTable<OperatorPO> {
public:
    OperatorTable() = default;
    ~OperatorTable() override = default;

protected:
    const std::unordered_map<std::string_view, assign> &GetAssignMap() override
    {
        static std::unordered_map<std::string_view, assign> assignMap = {
            { OperatorColumn::ID, SetId },
            { OperatorColumn::NAME, SetName },
            { OperatorColumn::SIZE, SetSize },
            { OperatorColumn::ALLOCATION_TIME, SetAllocationTime },
            { OperatorColumn::RELEASE_TIME, SetReleaseTime },
            { OperatorColumn::DURATION, SetDuration },
            { OperatorColumn::ACTIVE_RELEASE_TIME, SetActiveReleaseTime },
            { OperatorColumn::ACTIVE_DURATION, SetActiveDuration },
            { OperatorColumn::ALLOCATION_ALLOCATED, SetAllocationAllocated },
            { OperatorColumn::ALLOCATION_RESERVE, SetAllocationReserve },
            { OperatorColumn::ALLOCATION_ACTIVE, SetAllocationActive },
            { OperatorColumn::RELEASE_ALLOCATED, SetReleaseAllocated },
            { OperatorColumn::RELEASE_RESERVE, SetReleaseReserve },
            { OperatorColumn::RELEASE_ACTIVE, SetReleaseActive },
            { OperatorColumn::STREAM, SetStream },
        };

        return assignMap;
    }
    const std::string &GetTableName() override
    {
        static std::string tableName = "operator";
        return tableName;
    }
    static void SetId(OperatorPO &operatorPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void SetName(OperatorPO &operatorPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void SetSize(OperatorPO &operatorPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void SetAllocationTime(OperatorPO &operatorPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void SetReleaseTime(OperatorPO &operatorPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void SetDuration(OperatorPO &operatorPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void SetActiveReleaseTime(OperatorPO &operatorPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void SetActiveDuration(OperatorPO &operatorPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void SetAllocationAllocated(OperatorPO &operatorPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void SetAllocationReserve(OperatorPO &operatorPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void SetAllocationActive(OperatorPO &operatorPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void SetReleaseAllocated(OperatorPO &operatorPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void SetReleaseReserve(OperatorPO &operatorPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void SetReleaseActive(OperatorPO &operatorPO, const std::unique_ptr<SqliteResultSet> &resultSet);
    static void SetStream(OperatorPO &operatorPO, const std::unique_ptr<SqliteResultSet> &resultSet);
};
}
#endif // PROFILER_SERVER_OPERATORTABLE_H
