/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "DataBaseManager.h"
#include "ServerLog.h"
#include "TraceTime.h"
#include "VirtualTraceDatabase.h"
#include "FusedOpAdvisor.h"

namespace Dic::Module::Advisor {
using namespace Dic::Server;
bool FusedOpAdvisor::Process(const Protocol::APITypeParams &params, Protocol::OperatorFusionResBody &resBody)
{
    auto database = Timeline::DataBaseManager::Instance().GetTraceDatabase(params.rankId);
    if (database == nullptr) {
        ServerLog::Error("Failed to get connection for Fused Operator advice. fileId:", params.rankId);
        return false;
    }
    uint64_t startTime = Timeline::TraceTime::Instance().GetStartTime();
    std::vector<Protocol::KernelBaseInfo> data{};
    Protocol::KernelDetailsParams param = {.orderBy = params.orderBy, .order = params.orderType,
                                           .current = params.currentPage, .pageSize = params.pageSize};

    for (const auto &item : data) {
        Protocol::OperatorFusionData one{};
        one.baseInfo.rankId = params.rankId;
        one.baseInfo.startTime = item.startTime;
        one.baseInfo.duration = item.duration;
        one.baseInfo.pid = item.pid;
        one.baseInfo.tid = item.tid;
        one.name = item.name;
        one.originOpList = item.name;
        one.note = "";
        resBody.datas.emplace_back(one);
    }
    resBody.size = data.size();
    return true;
}
} // Dic::Module::Advisor