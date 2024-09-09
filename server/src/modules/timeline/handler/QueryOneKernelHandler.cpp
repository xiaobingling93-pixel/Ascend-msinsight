/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
*/
#include "QueryOneKernelHandler.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "ProjectExplorerManager.h"
#include "TraceTime.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;

void QueryOneKernelHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    KernelRequest &request = dynamic_cast<KernelRequest &>(*requestPtr.get());
    std::unique_ptr<OneKernelResponse> responsePtr = std::make_unique<OneKernelResponse>();
    OneKernelResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    WsSession &session = *WsSessionManager::Instance().GetSession();
    if (request.projectName.empty()) {
        ServerLog::Error("project name is empty");
        session.OnResponse(std::move(responsePtr));
        return;
    }
    auto database = DataBaseManager::Instance().GetTraceDatabase(request.params.rankId);
    if (database == nullptr) {
        database = Timeline::DataBaseManager::Instance().GetTraceDatabaseWithOutHost(request.params.rankId);
        if (database == nullptr) {
            ServerLog::Error("Query one kernel failed to get connection.");
            session.OnResponse(std::move(responsePtr));
            return;
        }
    }
    if (!database->QueryKernelDepthAndThread(request.params, response.body, TraceTime::Instance().GetStartTime())) {
        SetResponseResult(response, false);
        ServerLog::Error("Failed to query the operator response data.");
    }

    // 判断是否具备集群数据并且包含集群文件
    if (!Global::ProjectExplorerManager::Instance().IsClusterData(request.projectName)) {
        session.OnResponse(std::move(responsePtr));
        return;
    }

    // 根据通信算子name,startTime查询step、group
    auto clusterDatabase = DataBaseManager::Instance().GetReadClusterDatabase();
    if (database == nullptr) {
        ServerLog::Error("Query one kernel failed to get cluster connection.");
        session.OnResponse(std::move(responsePtr));
        return;
    }
    if (!clusterDatabase->QueryIterationAndCommunicationGroup(request.params, response.body,
        TraceTime::Instance().GetStartTime())) {
        SetResponseResult(response, false);
        ServerLog::Error("Failed to query the operator group and step response data.");
    }

    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}

} // Timeline
} // Module
} // Dic