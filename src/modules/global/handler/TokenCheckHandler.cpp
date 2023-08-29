/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 */

#include "ServerLog.h"
#include "WsSessionManager.h"
#include "ProtocolErrorCode.h"
#include "TokenCheckHandler.h"

namespace Dic {
namespace Module {
using namespace Dic::Server;
void TokenCheckHandler::HandleRequest(std::unique_ptr<Request> requestPtr)
{
    TokenCheckRequest &request = dynamic_cast<TokenCheckRequest &>(*requestPtr.get());
    std::string sessionToken = request.token;
    ServerLog::Info("Token check start, token = ", StringUtil::AnonymousString(sessionToken));
    if (!WsSessionManager::Instance().CheckSession(sessionToken)) {
        ServerLog::Warn("Failed to check session, token = ", StringUtil::AnonymousString(sessionToken),
            ", command = ", command);
        return;
    }
    WsSession &session = *WsSessionManager::Instance().GetSession(sessionToken);
    std::unique_ptr<TokenCheckResponse> responsePtr = std::make_unique<TokenCheckResponse>();
    TokenCheckResponse &response = *responsePtr.get();
    SetBaseResponse(request, response);

    std::string checkedToken = request.params.checkedToken;
    if (!WsSessionManager::Instance().CheckSession(checkedToken)) {
        error = "Failed to find session by checked token.";
        SetResponseResult(response, false, error, ErrorCode::COMMAND_PARAMS_ERROR);
        session.OnResponse(std::move(responsePtr));
        return;
    } else {
        WsSession &checkedSession = *WsSessionManager::Instance().GetSession(checkedToken);
        response.body.deadTime = checkedSession.GetDeadTime();
        response.body.createTime = checkedSession.GetCreateTime();
        response.body.isSubToken = checkedSession.IsSubSession();
    }
    response.body.checkedToken = checkedToken;
    SetResponseResult(response, true);
    // add response to response queue in session
    session.OnResponse(std::move(responsePtr));
}
} // end of namespace Module
} // Dic