/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "RanksHandler.h"
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
void RanksHandler::HandleRequest(std::unique_ptr<Protocol::Request> requestPtr)
    {
        RanksRequest &request = dynamic_cast<RanksRequest &>(*requestPtr.get());
        std::string token = request.token;
        if (!WsSessionManager::Instance().CheckSession(token)) {
            ServerLog::Error("Failed to check session token  , command = ", command);
            return;
        }
        WsSession &session = *WsSessionManager::Instance().GetSession(token);
        std::unique_ptr<RanksResponse> responsePtr = std::make_unique<RanksResponse>();
        RanksResponse &response = *responsePtr.get();
        SetBaseResponse(request, response);
        auto database = Timeline::DataBaseManager::Instance().GetClusterDatabase();
        if (!database->QueryRanksHandler(request.params, response.body)) {
            SetResponseResult(response, false);
            ServerLog::Error("Failed to get ranks response data.");
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