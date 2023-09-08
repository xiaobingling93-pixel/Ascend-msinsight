/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

#include "CommunicationOperatorDetailsHandler.h"
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

void CommunicationOperatorDetailsHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
{
    ServerLog::Info("request to Communication CommunicationOperatorDetailsHandler");
    Protocol::OperatorDetailsRequest &request =
            dynamic_cast<Protocol::OperatorDetailsRequest &>(*requestPtr.get());
    std::string token = request.token;
    if (!WsSessionManager::Instance().CheckSession(token)) {
        ServerLog::Error("Failed to check session token  , command = ", command);
        return;
    }
    std::unique_ptr<Protocol::OperatorDetailsResponse> responsePtr =
            std::make_unique<Protocol::OperatorDetailsResponse>();
    OperatorDetailsResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    SetResponseResult(response, true);
    // add response to response queue in session
    WsSession &session = *WsSessionManager::Instance().GetSession(token);
    // query data
    auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase();
    database->QueryOperatorsCount(request.params, response.body);
    database->QueryAllOperators(request.params, response.body);
    if (!database->QueryOperatorsCount(request.params, response.body) ||
        !database->QueryAllOperators(request.params, response.body)) {
        SetResponseResult(response, false);
        session.OnResponse(std::move(responsePtr));
        return;
    }
    session.OnResponse(std::move(responsePtr));
}
} // Communication
} // Module
} // Dic