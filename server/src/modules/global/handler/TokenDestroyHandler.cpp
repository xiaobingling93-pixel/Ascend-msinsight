/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#include "ServerLog.h"
#include "WsSessionManager.h"
#include "TokenDestroyHandler.h"

namespace Dic {
namespace Module {
using namespace Dic::Server;
void TokenDestroyHandler::HandleRequest(std::unique_ptr<Request> requestPtr)
{
    TokenDestroyRequest &request = dynamic_cast<TokenDestroyRequest &>(*requestPtr.get());
    std::string sessionToken = request.token;
    ServerLog::Info("Token destroy start, token = ", StringUtil::AnonymousString(sessionToken));
    if (!WsSessionManager::Instance().CheckSession(sessionToken)) {
        ServerLog::Warn("Failed to check session, token = ", StringUtil::AnonymousString(sessionToken),
            ", command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(sessionToken);
    std::unique_ptr<TokenDestroyResponse> responsePtr = std::make_unique<TokenDestroyResponse>();
    TokenDestroyResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);
    response.body.destroyToken = request.params.destroyToken;
    response.body.destroyTime = TimeUtil::Instance().NowUTC();
    SetResponseResult(response, true);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
    // destroy session at end
    OnDestroySession(request);
}

void TokenDestroyHandler::OnDestroySession(const TokenDestroyRequest &request) const
{
    std::string destroyToken = request.params.destroyToken;
    std::thread(
        [](const std::string &token) {
            if (WsSessionManager::Instance().CheckSession(token)) {
                WsSessionManager::Instance().GetSession(token)->Stop();
            }
        },
        destroyToken)
        .detach();
}
} // end of namespace Module
} // Dic