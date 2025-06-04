/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include "DbTraceDataBase.h"
#include "TraceDatabaseHelper.h"
#include "CounterEventHelper.h"

namespace Dic::Module::FullDb {
using namespace Server;

bool DbTraceDataBase::QueryUnitCounter(Protocol::UnitCounterParams &params, uint64_t minTimestamp,
                                       std::vector<Protocol::UnitCounterData> &dataList)
{
    auto stmt = CreatPreparedStatement();
    if (stmt == nullptr) {
        ServerLog::Error("Query_unit_counter. Failed to prepare sql.", sqlite3_errmsg(db));
        return false;
    }
    std::unique_ptr<SqliteResultSet> resultSet;
    const std::vector<PROCESS_TYPE> hostCounterEvents = {PROCESS_TYPE::CPU_USAGE,
        PROCESS_TYPE::HOST_DISK_USAGE, PROCESS_TYPE::HOST_MEM_USAGE, PROCESS_TYPE::HOST_NETWORK_USAGE};
    const std::vector<PROCESS_TYPE> deviceCounterEvents = {PROCESS_TYPE::NIC, PROCESS_TYPE::ROCE,
        PROCESS_TYPE::PCIE, PROCESS_TYPE::HCCS};
    if (std::find(hostCounterEvents.begin(), hostCounterEvents.end(),
                  Timeline::TraceDatabaseHelper::GetProcessType(params.metaType)) != hostCounterEvents.end()) {
        try {
            resultSet = TraceDatabaseHelper::QueryHostUnitCounter(stmt, params, minTimestamp);
        } catch (DatabaseException &e) {
            ServerLog::Error("Query host unit counter failed, ", e.What());
            return false;
        }
    } else if (std::find(deviceCounterEvents.begin(), deviceCounterEvents.end(),
                         Timeline::TraceDatabaseHelper::GetProcessType(params.metaType)) != deviceCounterEvents.end()) {
        try {
            resultSet = TraceDatabaseHelper::QueryDeviceUnitCounter(stmt,
                params, minTimestamp, GetDeviceId(params.rankId));
        } catch (DatabaseException &e) {
            ServerLog::Error("Query device unit counter failed, ", e.What());
            return false;
        }
    } else {
        try {
            resultSet = TraceDatabaseHelper::QueryUnitCounter(stmt, params, minTimestamp, GetDeviceId(params.rankId));
        } catch (DatabaseException &e) {
            ServerLog::Error("Query unit counter failed, ", e.What());
            return false;
        }
    }
    while (resultSet->Next()) {
        Protocol::UnitCounterData unitCounterData;
        unitCounterData.timestamp = resultSet->GetUint64("startTime");
        unitCounterData.valueJsonStr = resultSet->GetString("args");
        dataList.emplace_back(unitCounterData);
    }
    return true;
}

void DbTraceDataBase::ProcessHostCounterEventsMetadata(std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData)
{
    CounterEventHelper helper;
    helper.RegisterHostMap();
    for (const auto &element : helper.hostCounterEventMap) {
        std::string progressName = element.second.progressName;
        std::string metaDataSQL = helper.GenerateHostMetadataSQL(element.first);
        auto counter = GenerateBaseUnitTrack("label", path, progressName, progressName,
                                             ENUM_TO_STR(element.first).value());
        auto stmt = CreatPreparedStatement(metaDataSQL);
        if (!stmt) {
            ServerLog::Error("Failed to query host counter metadata. metaType is %",
                             ENUM_TO_STR(element.first).value());
            continue;
        }
        auto resultSet = stmt->ExecuteQuery();
        if (!resultSet) {
            ServerLog::Error("Failed to get result set of querying counter metadata. metaType is %.",
                             ENUM_TO_STR(element.first).value());
            continue;
        }
        while (resultSet->Next()) {
            auto thread = GenerateBaseUnitTrack("counter", path, progressName, progressName,
                                                ENUM_TO_STR(element.first).value());
            thread->metaData.threadId = resultSet->GetString("name");
            thread->metaData.threadName = thread->metaData.threadId;
            thread->metaData.dataType = StringUtil::Split(resultSet->GetString("types"), ",");
            counter->children.emplace_back(std::move(thread));
        }
        metaData.emplace_back(std::move(counter));
    }
}
}