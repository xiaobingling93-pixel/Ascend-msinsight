/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include "DbTraceDataBase.h"
#include "TrackInfoManager.h"
#include "TraceDatabaseHelper.h"
#include "CounterEventHelper.h"

namespace Dic::Module::FullDb {
using namespace Server;

std::vector<std::string> DbTraceDataBase::GetIdListByFuzzNameFromCache(const std::string &path,
                                                                       const std::string &fuzzName,
                                                                       const bool caseSensitive)
{
    if (stringsCache.count(path) == 0) {
        return {};
    }
    std::vector<std::string> res;
    for (const auto &item: stringsCache.at(path)) {
        if (caseSensitive) {
            if (StringUtil::Contains(item.second, fuzzName)) {
                res.push_back(item.first);
            }
            continue;
        }
        if (StringUtil::ContainsIgnoreCase(item.second, fuzzName)) {
            res.push_back(item.first);
        }
    }
    return res;
}

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
        PROCESS_TYPE::SAMPLE_PMU, PROCESS_TYPE::NIC, PROCESS_TYPE::PCIE, PROCESS_TYPE::HCCS, PROCESS_TYPE::QOS};
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
    std::string curArgs;
    UnitCounterData unitCounterData;
    while (resultSet->Next()) {
        unitCounterData.timestamp = resultSet->GetUint64("startTime");
        unitCounterData.valueJsonStr = resultSet->GetString("args");
        if (unitCounterData.valueJsonStr != curArgs) {
            dataList.emplace_back(unitCounterData);
            curArgs = unitCounterData.valueJsonStr;
        }
    }
    if (!dataList.empty()) {
        dataList = DownSampleUnitCounterData(dataList, counterSampleSize);
        dataList.emplace_back(unitCounterData);
    }
    return true;
}

void DbTraceDataBase::ProcessHostCounterEventsMetadata(const std::string &fileId, std::vector<std::unique_ptr<Protocol::UnitTrack>> &metaData)
{
    CounterEventHelper helper;
    helper.RegisterHostMap();
    for (const auto &element : helper.hostCounterEventMap) {
        std::string progressName = element.second.processName;
        std::string metaDataSQL = helper.GenerateHostMetadataSQL(element.first);
        auto counter = GenerateBaseUnitTrack("label", fileId, progressName, progressName,
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
            auto thread = GenerateBaseUnitTrack("counter", fileId, progressName, progressName,
                                                ENUM_TO_STR(element.first).value());
            thread->metaData.threadId = resultSet->GetString("name");
            thread->metaData.threadName = thread->metaData.threadId;
            thread->metaData.dataType = StringUtil::Split(resultSet->GetString("types"), ",");
            counter->children.emplace_back(std::move(thread));
        }
        metaData.emplace_back(std::move(counter));
    }
}

void DbTraceDataBase::ExecuteQueryUnitFlowsForTable(const std::pair<std::string, std::string> &tableAndSql,
                                                    uint64_t minTimestamp, const std::string &connectionId, const std::vector<uint64_t> &deviceIdList,
                                                    std::vector<FlowLocation> &flowLocations)
{
    auto stmt = CreatPreparedStatement();
    std::string sql = "with constValue as (select ? as minTime, ? as connectionId) " + tableAndSql.second +
        " order by startTime ";
    std::unique_ptr<SqliteResultSet> resultSet;
    if (tableAndSql.first == "COMMUNICATION_OP" && deviceIdList.size() == 1) {
        resultSet = TraceDatabaseHelper::ExecuteQuery(stmt, sql, minTimestamp, connectionId, deviceIdList[0]);
    } else {
        resultSet = TraceDatabaseHelper::ExecuteQuery(stmt, sql, minTimestamp, connectionId);
    }

    while (resultSet->Next()) {
        auto metaType = resultSet->GetString("metaType");
        auto rankId = resultSet->GetString("deviceId");
        for (const auto &item: QueryRankIdAndDeviceMap()) {
            if (rankId == item.second) {
                rankId = item.first;
            }
        }
        rankId = rankId.empty() ? path : QueryHostInfo() + rankId;
        FlowLocation location {
            .tid = resultSet->GetString("tid"), .id = resultSet->GetString("id"),
            .metaType = metaType, .rankId = rankId,
            .depth = resultSet->GetUint32("depth"), .timestamp = resultSet->GetUint64("startTime"),
            .duration = resultSet->GetUint64("duration"), .pid = resultSet->GetString("pid"),
            .name = stringsCache.at(path)[resultSet->GetString("name")]
        };
        if (tableAndSql.first == "TASK") {
            std::string domainId = resultSet->GetString("domainId");
            if (!domainId.empty()) {
                std::string streamId = resultSet->GetString("tid");
                location.tid = streamId + "_" + domainId;
            }
        }
        flowLocations.push_back(location);
    }
}

bool DbTraceDataBase::QueryUnitFlows(const Protocol::UnitFlowsParams &requestParams,
                                     Protocol::UnitFlowsBody &responseBody, uint64_t minTimestamp, uint64_t trackId)
{
    auto stmt = CreatPreparedStatement();
    auto connectionId = TraceDatabaseHelper::QueryConnectionId(stmt, requestParams);
    // connectionId为-1和UINT32_MAX都表示无效值，无连线，因此不能继续搜索，否则会将所有无连线的算子都连起来
    if (!connectionId.has_value() ||
        connectionId.value() == "-1" || connectionId.value() == std::to_string(UINT32_MAX)) {
        return false;
    }
    std::vector<uint64_t> deviceIdList = TraceDatabaseHelper::GetDeviceIdList(requestParams.rankId);
    std::string comSql = deviceIdList.size() == 1 ? COM_OP_UNIT_FLOW_SQL_UNIQUE_DEVICE : COM_OP_UNIT_FLOW_SQL;
    std::vector<FlowLocation> flowLocations;
    std::map<std::string, std::string> sqlMap{{"PYTORCH_API", PYTORCH_UNIT_FLOW_SQL},
        {"CANN_API", CANN_UNIT_FLOW_SQL}, {"TASK", TASK_UNIT_FLOW_SQL},
        {"COMMUNICATION_OP", comSql}, {"MSTX_EVENTS", MSTX_UNIT_FLOW_SQL}};
    for (const auto &item : sqlMap) {
        ExecuteQueryUnitFlowsForTable(item, minTimestamp, connectionId.value(), deviceIdList, flowLocations);
    }
    if (flowLocations.size() < 2) { // 小于2表示没有连线
        return false;
    }
    // 同connectionId的算子按时间排序后相邻的连线
    std::sort(flowLocations.begin(), flowLocations.end(),
        [] (const FlowLocation &a, const FlowLocation &b) { return a.timestamp < b.timestamp;});
    for (auto &item: flowLocations) {
        if (item.rankId == path) {
            item.rankId = requestParams.rankId;
        }
    }
    std::map<std::string, std::vector<UnitSingleFlow>> flowMap;
    for (size_t index = 1; index < flowLocations.size(); index++) {
        UnitSingleFlow singleFlow;
        singleFlow.id = connectionId.value();
        singleFlow.from = flowLocations[index - 1];
        singleFlow.to = flowLocations[index];
        if (singleFlow.from.rankId != singleFlow.to.rankId) {
            continue;
        }
        if (singleFlow.from.metaType == singleFlow.to.metaType && singleFlow.from.metaType == TABLE_API) {
            singleFlow.cat = singleFlow.from.name.find("Enqueue") != std::string::npos ? "async_task_queue" : "fwdbwd";
        }
        singleFlow.cat = singleFlow.from.metaType == TABLE_CANN_API ? "HostToDevice" : singleFlow.cat;
        singleFlow.cat = singleFlow.from.metaType == TABLE_MSTX_EVENTS ? "MsTx" : singleFlow.cat;
        if (singleFlow.from.metaType == TABLE_API && singleFlow.to.metaType == TABLE_CANN_API) {
            singleFlow.cat = "async_npu";
        }
        flowMap[singleFlow.cat].push_back(singleFlow);
    }
    for (const auto &item: flowMap) {
        responseBody.unitAllFlows.push_back({ .cat = item.first, .flows = item.second });
    }
    return true;
}

std::string DbTraceDataBase::GetDeviceIdFromMemoryTable()
{
    auto queryDevice = [this](const std::string &table) -> std::string {
        sqlite3_stmt *stmt = nullptr;
        std::string sql = "SELECT deviceId FROM " + table + " limit 1";
        int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        if (result != SQLITE_OK) {
            sqlite3_finalize(stmt);
            return "";
        }
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string deviceId = sqlite3_column_string(stmt, resultStartIndex);
            sqlite3_finalize(stmt);
            return deviceId;
        }
        sqlite3_finalize(stmt);
        return "";
    };
    if (CheckTableExist(TABLE_MEMORY_RECORD)) {
        return queryDevice(TABLE_MEMORY_RECORD);
    }
    return "";
}

void DbTraceDataBase::QueryDeviceIdInStepTraceTime(std::set<std::string> &deviceIds)
{
    std::string sql = "SELECT DISTINCT deviceId from StepTraceTime";
    sqlite3_stmt* stmt = nullptr;
    int ret = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (ret != SQLITE_OK) {
        ServerLog::Error("Failed to prepare sql for query deviceId in StepTraceTime. Error: ",
                         sqlite3_errmsg(db));
        return;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int col = resultStartIndex;
        std::string deviceId = sqlite3_column_string(stmt, col++);
        deviceIds.insert(deviceId);
    }
    sqlite3_finalize(stmt);
}

bool DbTraceDataBase::QueryThreadSameOperatorsDetails(const Protocol::UnitThreadsOperatorsParams &requestParams,
    Protocol::UnitThreadsOperatorsBody &responseBody, uint64_t minTimestamp,
    const std::vector<uint64_t> &trackIdList)
{
    std::vector<std::string> pidList;
    std::vector<std::string> tidList;
    for (const auto& trackId : trackIdList) {
        TrackInfo trackInfo;
        TrackInfoManager::Instance().GetTrackInfo(trackId, trackInfo);
        if (!StringUtil::CheckSqlValid(trackInfo.threadId)) {
            ServerLog::Error("There is an SQL injection attack in track id. Error param: % ", trackInfo.threadId);
            return false;
        }
        pidList.emplace_back(trackInfo.processId);
        tidList.emplace_back(trackInfo.threadId);
    }
    std::string orderByAndPage = " ORDER BY " + requestParams.orderBy +
                                 (requestParams.order == "descend" ? " DESC " : " ASC ");
    auto stmt = CreatPreparedStatement();
    std::unique_ptr <SqliteResultSet> resultSet;
    try {
        resultSet = TraceDatabaseHelper::QueryThreadSameOperatorsDetails(stmt, requestParams,
            { GetDeviceId(requestParams.rankId), minTimestamp, orderByAndPage, pidList, tidList });
    } catch (DatabaseException &e) {
        ServerLog::Error("Query thread same operators details fail, ", e.What());
        return false;
    }
    if (resultSet == nullptr) {
        return false;
    }
    ExecuteQueryDbThreadSameOperatorsDetails(resultSet, requestParams, responseBody, tidList);
    responseBody.currentPage = requestParams.current;
    responseBody.pageSize = requestParams.pageSize;
    return true;
}

void DbTraceDataBase::ExecuteQueryDbThreadSameOperatorsDetails(const std::unique_ptr<SqliteResultSet> &resultSet,
    const Protocol::UnitThreadsOperatorsParams &requestParams, Protocol::UnitThreadsOperatorsBody &responseBody,
    const std::vector<std::string> tidList)
{
    uint64_t offset = (requestParams.current - 1) > UINT64_MAX / requestParams.pageSize ? 0 :
                      (requestParams.current - 1) * requestParams.pageSize;
    uint64_t count = 0;
    while (resultSet->Next()) {
        int col = resultStartIndex;
        Protocol::SameOperatorsDetails sameOperatorsDetail{};
        sameOperatorsDetail.timestamp = resultSet->GetUint64(col++);
        sameOperatorsDetail.duration = resultSet->GetUint64(col++);
        sameOperatorsDetail.depth = resultSet->GetUint64(col++);
        sameOperatorsDetail.id = resultSet->GetString(col++);
        sameOperatorsDetail.tid = resultSet->GetString(col++);
        if (sameOperatorsDetail.tid.empty()) {  // some process not have tid, use request.tid[0], ex:pytorch
            sameOperatorsDetail.tid = tidList[0];
        }
        sameOperatorsDetail.pid = resultSet->GetString(col++);
        if (!requestParams.startDepth.empty() && !requestParams.endDepth.empty() &&
            !(sameOperatorsDetail.depth >= NumberUtil::StringToUint32(requestParams.startDepth) &&
              sameOperatorsDetail.depth <= NumberUtil::StringToUint32(requestParams.endDepth))) {
            continue;
        }
        if (++count <= offset) {
            continue;
        }
        if (count > offset + requestParams.pageSize) {
            break;
        }
        responseBody.sameOperatorsDetails.emplace_back(sameOperatorsDetail);
    }
}
}
