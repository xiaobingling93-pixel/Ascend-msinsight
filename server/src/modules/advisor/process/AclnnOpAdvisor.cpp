/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "pch.h"
#include "DataBaseManager.h"
#include "TraceTime.h"
#include "VirtualTraceDatabase.h"
#include "AclnnOpAdvisor.h"

namespace Dic::Module::Advisor {
using namespace Dic::Server;
bool AclnnOpAdvisor::Process(const Protocol::APITypeParams &params, Protocol::AclnnOperatorResBody &resBody)
{
    auto database = Timeline::DataBaseManager::Instance().GetTraceDatabaseByRankId(params.rankId);
    if (database == nullptr) {
        ServerLog::Error("Failed to get connection in Aclnn op advice. fileId:", params.rankId);
        return false;
    }

    Protocol::KernelDetailsParams param = {.orderBy = params.orderBy, .order = params.orderType,
        .current = params.currentPage, .pageSize = params.pageSize, .rankId = params.rankId};
    param.order = params.orderType == "ascend" ? "ASC" : "DESC";
    if (std::count(SINGLE_OP_ORDER_BY_NAME_LIST.begin(), SINGLE_OP_ORDER_BY_NAME_LIST.end(), params.orderBy) == 0) {
        param.orderBy = "duration";
    }
    if (!AclnnOpProcess(database, param, resBody)) {
        return false;
    }
    resBody.dbPath = database->GetDbPath();
    return true;
}

bool AclnnOpAdvisor::AclnnOpProcess(const std::shared_ptr<Timeline::VirtualTraceDatabase>& database,
    const Protocol::KernelDetailsParams &param, Protocol::AclnnOperatorResBody &resBody)
{
    uint64_t startTime = Timeline::TraceTime::Instance().GetStartTime();
    std::vector<Protocol::KernelBaseInfo> data{};
    if (!database->QueryAclnnOpCountExceedThreshold(param, ACLNN_OP_CNT_THRESHOLD, data, startTime)) {
        ServerLog::Error("Failed to Query Aclnn Op from database which count exceeds .", ACLNN_OP_CNT_THRESHOLD);
        return false;
    }
    uint64_t start = param.pageSize * (param.current - 1);
    for (uint64_t i = start; i < start + param.pageSize && i < data.size(); ++i) {
        auto item = data.at(i);
        Protocol::AclnnOperatorData one{};
        one.baseInfo.id = item.id;
        one.baseInfo.rankId = param.rankId;
        one.baseInfo.startTime = item.startTime;
        one.baseInfo.duration = item.duration;
        one.baseInfo.pid = item.pid;
        one.baseInfo.tid = item.tid;
        one.baseInfo.depth = item.depth;
        one.opName = item.name;
        one.note = "";
        resBody.datas.emplace_back(one);
    }
    resBody.size += data.size();
    return true;
}
} // Dic::Module::Advisor