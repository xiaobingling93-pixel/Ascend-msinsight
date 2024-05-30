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
    param.order = params.orderType == "ascend" ? "ASC" : "DESC";
    if (std::count(AICPU_OP_ORDER_BY_NAME_LIST.begin(), AICPU_OP_ORDER_BY_NAME_LIST.end(), params.orderBy) == 0) {
        param.orderBy = "duration";
    }
    if (!database->QueryAICpuOpCanBeOptimized(param, AICPU_OP_EQUIVALENT_REPLACE,
                                              AICPU_OP_DATATYPE_RULE, data, startTime)) {
        ServerLog::Error("Failed to Query Can Be Optimized AI CPU Op from database. fileId:", params.rankId);
        return false;
    }
    uint64_t start = param.pageSize * (param.current - 1);
    for (uint64_t i = start; i < start + param.pageSize && i < data.size(); ++i) {
        auto item = data.at(i);
        Protocol::AICpuOperatorData one{};
        one.baseInfo.rankId = params.rankId;
        one.baseInfo.startTime = item.startTime;
        one.baseInfo.duration = item.duration;
        one.baseInfo.pid = item.pid;
        one.baseInfo.tid = item.tid;
        one.baseInfo.depth = item.depth;
        one.opName = item.name;
        one.note = "";
        resBody.datas.emplace_back(one);
    }
    resBody.size = data.size();
    return true;
}
} // Dic::Module::Advisor