/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
*/

#include "DataBaseManager.h"
#include "ProjectExplorerManager.h"
#include "QueryCommunicationKernelHandler.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;

bool QueryCommunicationKernelHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    CommunicationKernelRequest &request = dynamic_cast<CommunicationKernelRequest &>(*requestPtr.get());
    std::unique_ptr<CommunicationKernelResponse> responsePtr = std::make_unique<CommunicationKernelResponse>();
    CommunicationKernelResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);

    auto database = DataBaseManager::Instance().GetTraceDatabase(request.params.rankId);
    if (database == nullptr) {
        database = Timeline::DataBaseManager::Instance().GetTraceDatabaseWithOutHost(request.params.rankId);
        if (database == nullptr) {
            SendResponse(std::move(responsePtr), false, "Query communication kernel failed to get connection.");
            return false;
        }
    }
    if (!database->QueryCommunicationKernelInfo(request.params.name, request.params.rankId, response.body)) {
        SendResponse(std::move(responsePtr), false, "Failed to query communication kernel info.");
        return false;
    }

    // 判断是否具备集群数据并且包含集群文件
    if (!Global::ProjectExplorerManager::Instance().IsClusterData(request.projectName)) {
        SendResponse(std::move(responsePtr), false);
        return false;
    }

    // 根据通信算子name,startTime查询step、group
    request.params.rankId = GetRealRankId(request.params.rankId);
    auto clusterDbList = Timeline::DataBaseManager::Instance().GetAllClusterDatabase();
    bool flag = false;
    for (auto &clusterDb: clusterDbList) {
        if (clusterDb == nullptr) {
            continue;
        }
        if (!clusterDb->QueryIterationAndCommunicationGroup(request.params, response.body)) {
            continue;
        }
        flag = true;
        break;
    }
    if (!flag) {
        SendResponse(std::move(responsePtr), false, "Query communication kernel failed to get cluster connection.");
        return false;
    }
    SendResponse(std::move(responsePtr), true);
    return true;
}

std::string QueryCommunicationKernelHandler::GetRealRankId(const std::string &rankId)
{
    if (rankId.empty()) {
        return rankId;
    }
    std::vector<std::string> rankAfterSplit = StringUtil::Split(rankId, " ");
    // 存在 Host + 空格 + rankId的情況，这种情况下需要去掉Host信息
    if (rankAfterSplit.size() > 1) {
        return rankAfterSplit[1];
    }
    return rankId;
}
}
}
}