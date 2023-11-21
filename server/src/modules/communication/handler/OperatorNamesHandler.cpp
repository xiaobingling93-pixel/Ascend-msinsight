/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "OperatorNamesHandler.h"
#include "ServerLog.h"
#include "CommunicationProtocolRequest.h"
#include "CommunicationProtocolResponse.h"
#include "WsSessionManager.h"
#include "DataBaseManager.h"

namespace Dic {
namespace Module {
namespace Communication {
using namespace Dic;
using namespace Dic::Server;

void OperatorNamesHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    OperatorNamesRequest &request = dynamic_cast<OperatorNamesRequest &>(*requestPtr.get());
    std::string token = request.token;
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Error("Failed to check session token , command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    std::unique_ptr<OperatorNamesResponse> responsePtr = std::make_unique<OperatorNamesResponse>();
    OperatorNamesResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase();
    if (!database->QueryOperatorNames(request.params, response.body)) {
        SetResponseResult(response, false);
        ServerLog::Error("Failed to get operator names response data.");
        session.OnResponse(std::move(responsePtr));
        return;
    }
    SetResponseResult(response, true);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}

} // Communication
} // Module
} // Dic