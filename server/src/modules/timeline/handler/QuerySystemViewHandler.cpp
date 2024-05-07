/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
*/

#include "QuerySystemViewHandler.h"
#include "ServerLog.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"

namespace Dic {
namespace Module {
namespace Timeline {
using namespace Dic::Server;

void QuerySystemViewHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    SystemViewRequest &request = dynamic_cast<SystemViewRequest &>(*requestPtr.get());
    if (!WsSessionManager::Instance().CheckSession(request.token)) {
        ServerLog::Warn("Failed to check session, command = ", command);
        return;
    }

    std::unique_ptr<SystemViewResponse> responsePtr = std::make_unique<SystemViewResponse>();
    SystemViewResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    WsSession &session = *WsSessionManager::Instance().GetSession(request.token);
    auto database = DataBaseManager::Instance().GetTraceDatabase(request.params.rankId);
    if (database == nullptr) {
        ServerLog::Error("Failed to get connection. fileId:", request.params.rankId);
        session.OnResponse(std::move(responsePtr));
        return;
    }
    if (!database->QuerySystemViewData(request.params, response.body)) {
        SetResponseResult(response, false);
        ServerLog::Error("Failed to get timeline table response data.");
    }
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}

} // Timeline
} // Module
} // Dic