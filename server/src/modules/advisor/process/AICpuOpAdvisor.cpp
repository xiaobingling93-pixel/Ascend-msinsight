 /*
  * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
  */

#include "AICpuOpAdvisor.h"
#include "DataBaseManager.h"
#include "ServerLog.h"
#include "TraceTime.h"
#include "VirtualTraceDatabase.h"
namespace Dic::Module::Advisor {
using namespace Dic::Server;
bool AICpuOpAdvisor::Process(const Protocol::APITypeParams &params, Protocol::AICpuOperatorResBody &resBody)
{
    auto database = Timeline::DataBaseManager::Instance().GetTraceDatabase(params.rankId);
    if (database == nullptr) {
        ServerLog::Error("Failed to get connection in AI CPU advice. fileId:", params.rankId);
        return false;
    }
    uint64_t startTime = Timeline::TraceTime::Instance().GetStartTime();
    std::vector<Protocol::KernelBaseInfo> data{};
    Protocol::KernelDetailsParams param = {.orderBy = params.orderBy, .order = params.orderType,
                                           .current = params.currentPage, .pageSize = params.pageSize};
    if (!database->QueryAICpuOpCanBeOptimized(param, AICPU_OP_EQUIVALENT_REPLACE,
                                              AICPU_OP_DATATYPE_RULE, data, startTime)) {
        ServerLog::Error("Failed to Query Long Time AI CPU Op from database.");
        return false;
    }

    for (const auto &item : data) {
        Protocol::AICpuOperatorData one{};
        one.baseInfo.rankId = params.rankId;
        one.baseInfo.startTime = item.startTime;
        one.baseInfo.duration = item.duration;
        one.baseInfo.pid = item.pid;
        one.baseInfo.tid = item.tid;
        one.opName = item.name;
        one.note = "";
        resBody.datas.emplace_back(one);
    }
    resBody.size = data.size();
    return true;
}
} // Dic::Module::Advisor