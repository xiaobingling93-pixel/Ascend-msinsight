/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

#include <vector>
#include "DataBaseManager.h"
#include "ServerLog.h"
#include "CollectionUtil.h"
#include "ClusterService.h"

namespace Dic {
namespace Module {
namespace Communication {
using namespace Dic::Server;
void ClusterService::QueryIterations(const Protocol::IterationsRequest &request,
                                     Protocol::IterationsOrRanksResponse &response)
{
    auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    if (database == nullptr || !database->QueryIterations(response.body.compare)) {
        ServerLog::Warn("Fail to query compare iterations info.");
    }

    if (!request.params.isCompare) {
        return;
    }

    auto baselineDatabase = Timeline::DataBaseManager::Instance().GetClusterDatabase(BASELINE);
    if (baselineDatabase == nullptr || !baselineDatabase->QueryIterations(response.body.baseline)) {
        ServerLog::Warn("Fail to query baseline iterations info.");
    }
}

void ClusterService::QueryGroupInfo(const Protocol::MatrixGroupRequest &request,
                                    Protocol::MatrixGroupResponse &response)
{
    auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase(COMPARE);
    std::vector<std::string> compareGroupList;
    if (database == nullptr || !database->GetGroups(request.params.iterationId, compareGroupList)) {
        ServerLog::Warn("Fail to query compare group info.");
    }

    auto baselineDatabase = Timeline::DataBaseManager::Instance().GetClusterDatabase(BASELINE);
    std::vector<std::string> baselineGroupList;
    if (request.params.isCompare) {
        if (baselineDatabase == nullptr ||
            !baselineDatabase->GetGroups(request.params.baselineIterationId, baselineGroupList)) {
            ServerLog::Warn("Fail to query baseline group info.");
        }
    }
    // 合并通信域列表
    response.body.groupList = MergeGroupInfo(compareGroupList, baselineGroupList);
    // todo-yqs 并行策略待另一个需求整改后补充
}

std::vector<Protocol::GroupInfo> ClusterService::MergeGroupInfo(std::vector<std::string> &compareGroupList,
                                                                std::vector<std::string> &baselineGroupList)
{
    std::vector<Protocol::GroupInfo> res;
    std::vector<std::string> intersection = CollectionUtil::CalIntersection(compareGroupList, baselineGroupList);
    for (const auto &item: intersection) {
        Protocol::GroupInfo groupInfo = {item, "", "common"};
        res.push_back(groupInfo);
    }

    std::vector<std::string> compareDiff = CollectionUtil::CalDifferenceVector(compareGroupList, intersection);
    for (const auto &item: compareDiff) {
        Protocol::GroupInfo groupInfo = {item, "", "compare"};
        res.push_back(groupInfo);
    }

    std::vector<std::string> baselineDiff = CollectionUtil::CalDifferenceVector(baselineGroupList, intersection);
    for (const auto &item: baselineDiff) {
        Protocol::GroupInfo groupInfo = {item, "", "baseline"};
        res.push_back(groupInfo);
    }
    return res;
}
}
}
}