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
    const std::vector<PROCESS_TYPE> deviceCounterEvents = {PROCESS_TYPE::AI_CORE, PROCESS_TYPE::ACC_PMU,
        PROCESS_TYPE::DDR, PROCESS_TYPE::STARS_SOC, PROCESS_TYPE::NPU_MEM, PROCESS_TYPE::HBM, PROCESS_TYPE::LLC,
        PROCESS_TYPE::SAMPLE_PMU, PROCESS_TYPE::NIC, PROCESS_TYPE::PCIE, PROCESS_TYPE::HCCS};
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
        ServerLog::Error("Counter event type % is not supported.", params.metaType);
        return false;
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
        std::string progressName = element.second.processName;
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

bool DbTraceDataBase::QueryFwdBwdFromMstx(std::vector<Protocol::ThreadTraces> &traceList)
{
    std::string sql = "Select name, startNs, endNs, type from " + TABLE_STEP_TASK_INFO  + " order by startNs";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Failed to prepare sql for query fwd/bwd data from mstx in the DB scenario.");
        return false;
    }
    auto resultSet = stmt->ExecuteQuery();
    if (resultSet == nullptr) {
        ServerLog::Error("Failed to get result set for query fwd/bwd data from mstx.", stmt->GetErrorMessage());
        return false;
    }
    while (resultSet->Next()) {
        std::string name = resultSet->GetString("name");
        uint64_t startNs = resultSet->GetUint64("startNs");
        uint64_t endNs = resultSet->GetUint64("endNs");
        std::string type = std::to_string(resultSet->GetUint64("type"));
        Protocol::ThreadTraces trace = {name, endNs - startNs, startNs, endNs, 0, LANE_FP_BP, "", "", type};
        traceList.push_back(trace);
    }
    return true;
}

bool DbTraceDataBase::QueryP2PCommunicationOpHaveConnectionId(std::vector<Protocol::ThreadTraces> &traceList)
{
    std::string sql = "select str.value as name, op.startNs, op.endNs, op.opConnectionId from " +
        TABLE_COMMUNICATION_OP + " as op LEFT JOIN " + TABLE_STRING_IDS +
        " as str ON str.id = op.opName WHERE LOWER(name) LIKE '%send%' OR LOWER(name) LIKE '%receive%'";
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Failed to prepare sql for query communication op data "
                         "have connection id in the DB scenario.");
        return false;
    }
    auto resultSet = stmt->ExecuteQuery();
    if (resultSet == nullptr) {
        ServerLog::Error("Failed to get result set for query communication op data "
                         "have connection id in the DB scenario.", stmt->GetErrorMessage());
        return false;
    }
    while (resultSet->Next()) {
        Protocol::ThreadTraces trace;
        trace.name = resultSet->GetString("name");
        trace.startTime = resultSet->GetUint64("startNs");
        trace.endTime = resultSet->GetUint64("endNs");
        trace.duration = trace.endTime - trace.startTime;
        if (StringUtil::StartWith(trace.name, "hcom_send")) {
            trace.cname = MARKER_SEND;
        } else if (StringUtil::StartWith(trace.name, "hcom_receive")) {
            trace.cname = MARKER_RECV;
        } else {
            trace.cname = MARKER_BATCH_SEND_RECV;
        }
        trace.opConnectionId = resultSet->GetString("opConnectionId");
        traceList.push_back(trace);
    }
    return true;
}
}