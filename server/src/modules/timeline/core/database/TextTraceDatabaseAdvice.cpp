/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2024. All rights reserved.
 */

#include "TrackInfoManager.h"
#include "TraceTime.h"
#include "TraceDatabaseSqlConst.h"
#include "ServerLog.h"
#include "TextTraceDatabase.h"
#include "TraceDatabaseHelper.h"

namespace Dic::Module::Timeline {
using namespace Dic::Server;
using namespace Dic::Protocol;

void TextTraceDatabase::ProcessByteAlignmentAnalyzerDataForText(
    std::vector<CommunicationLargeOperatorInfo> &result, std::vector<std::pair<std::string, std::string>> rawData)
{
    bool hasOneHcom = false;
    CommunicationLargeOperatorInfo op;
    for (const auto &item : rawData) {
        if (item.first.find("hcom") == 0) {
            if (hasOneHcom) {
                result.push_back(op);
            } else {
                hasOneHcom = true;
            }
            op.name = item.first;
            op.memcpyTasks.clear();
            op.reduceInlineTasks.clear();
        } else {
            if (!hasOneHcom) {
                continue;
            }
            std::string err;
            std::optional<document_t> jsonOptional = JsonUtil::TryParse(item.second, err);
            if (jsonOptional == std::nullopt) {
                ServerLog::Error("Failed to parse args. ", err);
                continue;
            }
            document_t &json = jsonOptional.value();
            if (!json.IsObject()) {
                ServerLog::Error("Args is not valid json format. raw: %", item.second);
                continue;
            }
            CommunicationSmallOperatorInfo info;
            int64_t tempSize = NumberUtil::StringToLongLong(JsonUtil::GetString(json, "size(Byte)"));
            info.size = (tempSize < 0 ? 0 : static_cast<uint64_t>(tempSize));
            info.transportType = JsonUtil::GetString(json, "transport type");
            info.linkType = JsonUtil::GetString(json, "link type");
            if (item.first.find("Memcpy") == 0) {
                op.memcpyTasks.emplace_back(info);
            } else {
                op.reduceInlineTasks.emplace_back(info);
            }
        }
    }
    result.push_back(op);
}

void TextTraceDatabase::AssembleUnitFlowsBody(Protocol::UnitFlowsBody &responseBody, uint64_t minTimestamp,
    std::unordered_map<std::string, std::vector<FlowPoint>> &flowPointMap)
{
    std::map<std::string, std::vector<Protocol::UnitSingleFlow>> flowMap;
    for (auto &item : flowPointMap) {
        const static int FLOW_COUNT = 2; // from + to
        if (item.second.size() < FLOW_COUNT) {
            continue;
        }
        std::vector<std::unique_ptr<Protocol::UnitSingleFlow>> flowDetailList;
        FlowAnalyzer::SortByFlowIdAndTimestampASC(item.second);
        FlowAnalyzer::ComputeUintFlows(item.second, item.second[0].cat, flowDetailList);
        std::vector<Protocol::UnitCatFlows> unitAllFlow;
        for (const auto &singleFlow: flowDetailList) {
            if (singleFlow->from.timestamp < minTimestamp || singleFlow->to.timestamp < minTimestamp) {
                continue;
            }
            singleFlow->from.timestamp -= minTimestamp;
            singleFlow->to.timestamp -= minTimestamp;
            flowMap[singleFlow->cat].emplace_back(*singleFlow);
        }
    }
    std::vector<Protocol::UnitCatFlows> unitAllFlow;
    for (const auto &item : flowMap) {
        Protocol::UnitCatFlows unitCatFlows;
        unitCatFlows.cat = item.first;
        unitCatFlows.flows = item.second;
        unitAllFlow.emplace_back(unitCatFlows);
    }
    responseBody.unitAllFlows = unitAllFlow;
}

bool TextTraceDatabase::QueryAffinityAPIData(const Protocol::KernelDetailsParams &params,
    const std::set<std::string> &pattern, uint64_t minTimestamp,
    std::map<uint64_t, std::vector<Protocol::FlowLocation>> &data, std::map<uint64_t, std::vector<uint32_t>> &indexes)
{
    auto stmt = CreatPreparedStatement(TextSqlConstant::GenerateAffinityApiTextSql(params));
    if (stmt == nullptr) {
        ServerLog::Error("Failed to prepare sql for Affinity API.");
        return false;
    }
    std::unique_ptr<SqliteResultSet> resultSet;
    if (params.startTime == params.endTime) {
        resultSet = stmt->ExecuteQuery(minTimestamp, minTimestamp);
    } else {
        resultSet = stmt->ExecuteQuery(minTimestamp, minTimestamp, params.startTime + minTimestamp, params.endTime + minTimestamp);
    }
    if (resultSet == nullptr) {
        ServerLog::Error("Failed to get result set for Affinity API data.", stmt->GetErrorMessage());
        return false;
    }
    std::map<uint64_t, std::vector<Protocol::FlowLocation>> filterData;
    while (resultSet->Next()) {
        Protocol::FlowLocation one{};
        uint64_t trackId = resultSet->GetUint64("track");
        one.id = resultSet->GetString("id");
        one.name = resultSet->GetString("name");
        one.timestamp = resultSet->GetUint64("startTime");
        // Protocol::FlowLocation数据结构中只定义start time和duration，绝大多数场景下也是只用上述两个字段，
        // 此处需要比较start time和end time，是个特例，在不修改数据结构的情况下，duration中实际存的是end time，
        // 过滤顶层API后，在根据end time和start time求出duration
        one.duration = resultSet->GetUint64("endTime");
        one.pid = resultSet->GetString("pid");
        one.tid = resultSet->GetString("tid");
        if (data.count(trackId) == 0) {
            filterData.emplace(trackId, std::vector<Protocol::FlowLocation>{});
            data.emplace(trackId, std::vector<Protocol::FlowLocation>{});
            indexes.emplace(trackId, std::vector<uint32_t>{});
        }
        filterData[trackId].emplace_back(one);
    }
    for (const auto &item : filterData) {
        std::vector<Protocol::FlowLocation> originData = item.second;
        TraceDatabaseHelper::FilterTopLevelApi(originData, pattern, data[item.first], indexes[item.first]);
    }
    return true;
}

bool TextTraceDatabase::QueryAffinityOptimizer(const Protocol::KernelDetailsParams &params,
    const std::string &optimizers, std::vector<Protocol::ThreadTraces> &data, uint64_t minTimestamp)
{
    std::string sql = TextSqlConstant::QueryAffinityOptimizerTextSql(optimizers, params);
    auto stmt = CreatPreparedStatement(sql);
    if (stmt == nullptr) {
        ServerLog::Error("Fail to prepare sql for query affinity optimizer.", sqlite3_errmsg(db));
        return false;
    }
    std::unique_ptr<SqliteResultSet> resultSet;
    if (params.startTime == params.endTime) {
        resultSet = stmt->ExecuteQuery(minTimestamp);
    } else {
        resultSet = stmt->ExecuteQuery(minTimestamp, params.startTime + minTimestamp, params.endTime + minTimestamp);
    }
    if (resultSet == nullptr) {
        ServerLog::Error("Failed to get result set for query affinity optimizer.", stmt->GetErrorMessage());
        return false;
    }
    while (resultSet->Next()) {
        Protocol::ThreadTraces one{};
        one.id = resultSet->GetString("id");
        one.startTime = resultSet->GetUint64("startTime");
        one.name = resultSet->GetString("name");
        one.duration = resultSet->GetUint64("duration");
        one.threadId = resultSet->GetString("tid");
        one.pid = resultSet->GetString("pid");
        data.emplace_back(one);
    }
    return true;
}
}