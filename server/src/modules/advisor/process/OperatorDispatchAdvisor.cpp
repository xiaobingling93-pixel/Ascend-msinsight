/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include "pch.h"
#include "DataBaseManager.h"
#include "TraceTime.h"
#include "AdvisorProcessUtil.h"
#include "OperatorDispatchAdvisor.h"

namespace Dic::Module::Advisor {
using namespace Dic::Server;
bool OperatorDispatchAdvisor::Process(const Protocol::APITypeParams &params, Protocol::OperatorDispatchResBody &resBody)
{
    auto database = Timeline::DataBaseManager::Instance().GetTraceDatabaseByRankId(params.rankId);
    if (database == nullptr) {
        ServerLog::Error("Failed to get connection for Operator Dispatch advice. fileId:", params.rankId);
        return false;
    }
    Protocol::KernelDetailsParams param = {.orderBy = params.orderBy, .order = params.orderType,
                                           .current = params.currentPage, .pageSize = params.pageSize,
                                           .rankId = params.rankId};
    param.order = params.orderType == "ascend" ? "ASC" : "DESC";
    if (std::count(OPERATOR_DISPATCH_ORDER_BY_NAME_LIST.begin(), OPERATOR_DISPATCH_ORDER_BY_NAME_LIST.end(),
                   params.orderBy) == 0) {
        param.orderBy = "duration";
    }
    uint64_t startTime = Timeline::TraceTime::Instance().GetStartTime();
    std::string filePath = Timeline::DataBaseManager::Instance().GetDbPathByRankId(params.rankId);
    std::vector<Protocol::KernelBaseInfo> data{};
    if (!database->QueryOperatorDispatchData(param, data, startTime,
                                             OPERATOR_COMPILE_CNT_THRESHOLD, filePath)) {
        ServerLog::Error("Failed to Query Operator Dispatch data.");
        return false;
    }
    uint64_t start = param.pageSize * (param.current - 1);
    for (uint64_t i = start; i < start + param.pageSize && i < data.size(); ++i) {
        auto item = data.at(i);
        Protocol::OperatorDispatchData one{};
        one.baseInfo.id = item.id;
        one.baseInfo.rankId = item.rankId;
        one.baseInfo.startTime = item.startTime;
        one.baseInfo.duration = item.duration;
        one.baseInfo.pid = item.pid;
        one.baseInfo.tid = item.tid;
        one.baseInfo.depth = item.depth;
        one.opName = item.name;
        one.note = SUGGESTION_NOTE;
        resBody.data.emplace_back(one);
    }
    resBody.size = data.size();
    resBody.dbPath = database->GetDbPath();
    return true;
}
} // Dic::Module::Advisor