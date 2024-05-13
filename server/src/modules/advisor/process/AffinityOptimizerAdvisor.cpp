/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include "DataBaseManager.h"
#include "ServerLog.h"
#include "TraceTime.h"
#include "VirtualTraceDatabase.h"
#include "AffinityOptimizerAdvisor.h"

namespace Dic::Module::Advisor {
using namespace Dic::Server;
bool AffinityOptimizerAdvisor::Process(const Protocol::APITypeParams& params,
    Protocol::AffinityOptimizerResBody& resBody)
{
    std::string optimizers;
    for (const auto &it : OPTIMIZER_MAP) {
        optimizers += "'" + it.first + "',";
    }
    optimizers.pop_back(); // 去除最后的逗号
    std::vector<Protocol::ThreadTraces> data{};
    auto database = Timeline::DataBaseManager::Instance().GetTraceDatabase(params.rankId);
    if (database == nullptr) {
        ServerLog::Error("Failed to get connection. fileId:", params.rankId);
        return false;
    }

    uint64_t startTime = Timeline::TraceTime::Instance().GetStartTime();
    if (!database->QueryAffinityOptimizer(optimizers, data, startTime)) {
        ServerLog::Error("Failed to Query Affinity Optimizer from database.");
        return false;
    }
    for (const auto& item : data) {
        Protocol::AffinityOptimizerData one{};
        one.baseInfo.rankId = params.rankId;
        one.baseInfo.startTime = item.startTime;
        one.baseInfo.duration = item.duration;
        one.baseInfo.pid = item.id;
        one.baseInfo.tid = item.threadId;
        one.originOptimizer = item.name;
        one.replaceOptimizer = OPTIMIZER_MAP.at(item.name); // 上面的查询逻辑能够保证key-value一定存在
        resBody.datas.emplace_back(one);
    }
    resBody.size = data.size();

    return true;
}
}
