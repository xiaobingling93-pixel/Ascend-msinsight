/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "ServerLog.h"
#include "CommunicationProtocolRequest.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"
#include "CommunicatorGroupQueryHandler.h"

namespace Dic::Module::Communication {
using namespace Dic::Server;
using namespace Dic::Protocol;
void CommunicatorGroupQueryHandler::HandleRequest(std::unique_ptr<Dic::Protocol::Request> requestPtr)
{
    ServerLog::Info("request to Communication CommunicationGroupQueryHandler");
    auto &request = dynamic_cast<Protocol::CommunicatorGroupRequest &>(*requestPtr);
    std::string token = request.token;
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Error("Failed to check session token  , command = ", command);
        return;
    }
    std::unique_ptr<Protocol::CommunicatorGroupResponse> responsePtr =
            std::make_unique<Protocol::CommunicatorGroupResponse>();
    CommunicatorGroupResponse &response = *responsePtr;
    SetBaseResponse(request, response);
    // add response to response queue in session
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    SetResponseResult(response, true);
    auto database = Module::Timeline::DataBaseManager::Instance().GetClusterDatabase();
    if (!database->QueryCommunicationGroup(response.body)) {
        SetResponseResult(response, false);
        ServerLog::Error("Communication CommunicationGroupQueryHandler Failed");
    }
    session.OnResponse(std::move(responsePtr));
}
} // end of namespace Communication
// end of namespace Module
// end of namespace Dic