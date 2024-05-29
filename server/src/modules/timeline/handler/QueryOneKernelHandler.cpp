/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
*/

#include "QueryOneKernelHandler.h"
#include "ServerLog.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "TraceTime.h"
#include "ParserFactory.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;

void QueryOneKernelHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    KernelRequest &request = dynamic_cast<KernelRequest &>(*requestPtr.get());
    if (!WsSessionManager::Instance().CheckSession(request.token)) {
        ServerLog::Warn("Failed to check session, command = ", command);
        return;
    }
    std::unique_ptr<OneKernelResponse> responsePtr = std::make_unique<OneKernelResponse>();
    OneKernelResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    WsSession &session = *WsSessionManager::Instance().GetSession(request.token);
    auto database = DataBaseManager::Instance().GetTraceDatabase(request.params.rankId);
    if (database == nullptr) {
        database = Timeline::DataBaseManager::Instance().GetTraceDatabaseWithOutHost(request.params.rankId);
        if (database == nullptr) {
            ServerLog::Error("Failed to get connection. fileId:", request.params.rankId);
            session.OnResponse(std::move(responsePtr));
            return;
        }
    }
    if (!database->QueryKernelDepthAndThread(request.params, response.body, TraceTime::Instance().GetStartTime())) {
        SetResponseResult(response, false);
        ServerLog::Error("Failed to query the operator response data.");
    }

    if (!DataBaseManager::Instance().curIsCluster) {
        session.OnResponse(std::move(responsePtr));
        return;
    }

    // 根据通信算子name,startTime查询step、group
    auto clusterDatabase = DataBaseManager::Instance().GetReadClusterDatabase();
    if (database == nullptr) {
        ServerLog::Error("Failed to get cluster connection. fileId:", request.params.rankId);
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