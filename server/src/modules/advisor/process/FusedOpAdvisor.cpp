/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "pch.h"
#include "DataBaseManager.h"
#include "TraceTime.h"
#include "AdvisorProcessUtil.h"
#include "FusedOpAdvisor.h"

namespace Dic::Module::Advisor {
using namespace Dic::Server;
bool FusedOpAdvisor::Process(const Protocol::APITypeParams &params, Protocol::OperatorFusionResBody &resBody)
{
    auto database = Timeline::DataBaseManager::Instance().GetTraceDatabaseByRankId(params.rankId);
    if (database == nullptr) {
        ServerLog::Error("Failed to get connection for Fused Operator advice. fileId:", params.rankId);
        return false;
    }
    uint64_t startTime = Timeline::TraceTime::Instance().GetStartTime();
    std::vector<Protocol::FlowLocation> data{};
    Protocol::KernelDetailsParams param = {.orderBy = params.orderBy, .order = params.orderType,
                                           .current = params.currentPage, .pageSize = params.pageSize};
    param.order = params.orderType == "ascend" ? "ASC" : "DESC";
    if (std::count(FUSED_OP_ORDER_BY_NAME_LIST.begin(), FUSED_OP_ORDER_BY_NAME_LIST.end(), params.orderBy) == 0) {
        param.orderBy = "duration";
    }
    for (const auto& item : FUSEABLE_OPERATER_RULE_LIST) {
        std::vector<Protocol::FlowLocation> result{};
        if (!database->QueryFuseableOpData(param, item, result, startTime)) {
            continue;
        }
        if (!result.empty()) {
            data.insert(data.end(), result.begin(), result.end());
        }
    }
    AdvisorProcessUtil::SortFlowLocationData(data, param);
    uint64_t start = param.pageSize * (param.current - 1);
    for (uint64_t i = start; i < start + param.pageSize && i < data.size(); ++i) {
        auto item = data.at(i);
        Protocol::OperatorFusionData one{};
        one.baseInfo.id = item.id;
        one.baseInfo.rankId = params.rankId;
        one.baseInfo.startTime = item.timestamp;
        one.baseInfo.duration = item.duration;
        one.baseInfo.pid = item.pid;
        one.baseInfo.tid = item.tid;
        one.baseInfo.depth = item.depth;
        one.name = item.name;
        one.originOpList = item.type;
        one.fusedOp = item.metaType;
        one.note = item.note;
        resBody.datas.emplace_back(one);
    }
    resBody.size = data.size();
    return true;
}
} // Dic::Module::Advisor