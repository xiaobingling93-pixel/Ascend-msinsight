/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "pch.h"
#include "DataBaseManager.h"
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
    auto database = Timeline::DataBaseManager::Instance().GetTraceDatabaseByRankId(params.rankId);
    if (database == nullptr) {
        ServerLog::Error("Failed to get connection. fileId:", params.rankId);
        return false;
    }

    uint64_t startTime = Timeline::TraceTime::Instance().GetStartTime();
    Protocol::KernelDetailsParams param = {.orderBy = params.orderBy, .order = params.orderType,
                                           .current = params.currentPage, .pageSize = params.pageSize};
    param.order = params.orderType == "ascend" ? "ASC" : "DESC";
    if (std::count(AFFINITY_OP_ORDER_BY_NAME_LIST.begin(), AFFINITY_OP_ORDER_BY_NAME_LIST.end(), params.orderBy) == 0) {
        param.orderBy = "duration";
    }
    if (!database->QueryAffinityOptimizer(param, optimizers, data, startTime)) {
        ServerLog::Error("Failed to Query Affinity Optimizer from database.");
        return false;
    }
    uint64_t start = param.pageSize * (param.current - 1);
    auto dbType = Timeline::DataBaseManager::Instance().GetDataType();
    for (uint64_t i = start; i < start + param.pageSize && i < data.size(); ++i) {
        auto item = data.at(i);
        Protocol::AffinityOptimizerData one{};
        one.baseInfo.id = item.id;
        one.baseInfo.rankId = dbType == Timeline::DataType::TEXT ? params.rankId : database->GetDbPath();
        one.baseInfo.startTime = item.startTime;
        one.baseInfo.duration = item.duration;
        one.baseInfo.pid = item.pid;
        one.baseInfo.tid = item.threadId;
        one.baseInfo.depth = item.depth;
        one.originOptimizer = item.name;
        one.replaceOptimizer = OPTIMIZER_MAP.at(item.name); // 上面的查询逻辑能够保证key-value一定存在
        resBody.datas.emplace_back(one);
    }
    resBody.size = data.size();
    resBody.dbPath = database->GetDbPath();
    return true;
}
}
