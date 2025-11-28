/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include "TrackInfoManager.h"
#include "TraceTime.h"
#include "TraceDatabaseSqlConst.h"
#include "ServerLog.h"
#include "TextTraceDatabase.h"

namespace Dic::Module::Timeline {
using namespace Dic::Server;
using namespace Dic::Protocol;

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
}